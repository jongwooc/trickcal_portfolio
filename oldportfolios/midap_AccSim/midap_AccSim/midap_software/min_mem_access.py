from __future__ import print_function

from copy import copy
from functools import reduce

from config import cfg
from midap_software.layer_compiler import LayerCompiler


class MinMemAccess(LayerCompiler):
    def _preprocess(self):
        from generic_op import ConvOp
        self.load_flag = False
        super(MinMemAccess, self)._preprocess()
        self.num_remain_banks = 1
        layer = self.layer
        op = layer.main_op
        if isinstance(op, ConvOp):
            self._set_outbank_num()

    def _calc_dram_access_by_weight(self):
        layer  = self.layer
        op     = layer.main_op
        action = layer.control_info.action

        process_num = 1 if layer.is_weight_in_wmem else reduce(lambda x, y: x + y, [0] + [1 if a[0] == 'PROCESS' else 0 for a in action])
        # process_num = reduce(lambda x, y: x + y, [0] + [1 if a[0] == 'PROCESS' else 0 for a in action]) if op.type != 'Depthwise' else 1
        return (op.weight.size * process_num)

    def _calc_dram_access_by_outfeature(self):
        import numpy as np
        layer     = self.layer
        out_shape = layer.get_output_shape()
        mapping   = layer.control_info.output_mapping

        num_out_banks = len(mapping)
        reduced_width = layer.num_planes_per_fmem * num_out_banks

        max_row = reduce(lambda x, y: y[1][1] if x < y[1][1] else x, [0] + mapping)
        assert (reduced_width >= max_row)
        return (max(out_shape[0] - reduced_width, 0)) * np.prod(out_shape[1:]) * 2

    def _flip(self, num_output, min_bank_num):
        layer = self.layer
        control_info = layer.control_info
        fmem_info = self.fmem_info

        num_available_bank = fmem_info.get_num_unreserved_bank()
        if num_output < layer.require_fmem and num_output < num_available_bank - min_bank_num:
            min_bank_num = num_available_bank - num_output
        self.num_remain_banks = min_bank_num
        # TODO clean code
        if control_info.output_stationary < 0:
            reverse_write = control_info.reverse_write = layer.require_fmem > num_available_bank - min_bank_num
            input_layer = layer.input[0]
            input_flip = control_info.input_flip = input_layer.control_info.flip
            control_info.flip = not input_flip if reverse_write else input_flip

    def _set_outbank_num(self):
        import sys
        from copy import deepcopy
        layer = self.layer
        fmem_info = self.fmem_info

        min_bank_num = 1
        min_access = sys.maxsize
        fmem_info_bak = deepcopy(fmem_info)
        num_output = layer.require_fmem
        for n in range(min_bank_num, cfg.MIDAP.FMEM.NUM - len(layer.control_info.output_mapping)):
            control_info = layer.control_info

            in_mapping = deepcopy(control_info.input_mapping)
            out_mapping = deepcopy(control_info.output_mapping)
            fixed = control_info.fixed_output
            reverse_write = control_info.reverse_write
            flip = control_info.flip

            end = False
            num_available_bank = fmem_info.get_num_unreserved_bank()
            self._flip(min(layer.require_fmem, num_available_bank - n), n)
            while not end:
                end = self._do_step()
            w = self._calc_dram_access_by_weight()
            of = self._calc_dram_access_by_outfeature()
            if w + of < min_access:
                min_access = w + of
                min_bank_num = n
                num_output = len(control_info.output_mapping)

            del self.fmem_info
            fmem_info = self.fmem_info = deepcopy(fmem_info_bak)
            layer.processed_input = 0
            del layer.control_info['input_mapping'], layer.control_info['remain_inputs'], layer.control_info['output_mapping']
            layer.control_info.input_mapping = in_mapping
            layer.control_info.remain_inputs = deepcopy(in_mapping)
            layer.control_info.output_mapping = out_mapping
            layer.control_info.num_output_mapping = len(out_mapping)
            layer.control_info.fixed_output = fixed
            layer.control_info.action = []
            layer.control_info.reverse_write = reverse_write
            layer.control_info.flip = flip

        self._flip(num_output, min_bank_num)

    def _is_load_end(self):
        input_layer = self.layer.input[0]
        control_info = self.layer.control_info
        load_head = 0 if not control_info.input_mapping else control_info.input_mapping[-1][1][1]
        return load_head == input_layer.output_tensor.shape[0]

    def _do_step(self):
        layer = self.layer
        control_info = layer.control_info

        if self._is_load_end() or not self._do_load():
            self._do_operation()

        if control_info.input_mapping and not control_info.remain_inputs:
            return not layer.get_input_fragments(1, control_info.input_mapping[-1][1][1])

        return False

    def _do_operation(self):
        fmem_info = self.fmem_info
        layer = self.layer
        control_info = layer.control_info

        processing_fragments = copy(
            control_info.remain_inputs)  # Remain input mappings
        if not control_info.fixed_output:
            num_unreserved_bank = fmem_info.get_num_unreserved_bank()
            output_fragments = layer.get_output_fragments(num_unreserved_bank - self.num_remain_banks)
            self._set_out_mappings(output_fragments)

            if control_info.num_output_mapping < len(output_fragments):
                processing_fragments = control_info.limit_processing_fragments(
                    output_fragments, processing_fragments)
                if not processing_fragments:
                    self._fix_exception()
            else:
                control_info.fixed_output = True  # no more change occurs

        # post process
        if not processing_fragments:
            return

        control_info.action.append(
            ('PROCESS', layer.processed_input + len(processing_fragments)))

        for bank, _, _ in processing_fragments:
            control_info.process_input()
            fmem_info.discard_data(bank)

    def _do_load(self):
        fmem_info = self.fmem_info
        layer = self.layer
        input_layer = layer.input[0]
        control_info = layer.control_info

        overlap = not control_info.remain_inputs
        num_fragments = fmem_info.get_num_available_bank()
        load_head = 0 if not control_info.input_mapping else control_info.input_mapping[-1][1][1]
        if num_fragments == 0 or load_head == input_layer.output_tensor.shape[0]:
            return False

        fragments = layer.get_input_fragments(
            num_fragments, load_head, overlap=overlap and layer.processed_input > 0)
        if not control_info.fixed_output:
            num_unreserved_bank = fmem_info.get_num_unreserved_bank()
            num_output_bank = num_unreserved_bank - self.num_remain_banks
            fragments = control_info.limit_load_fragments(num_fragments, num_output_bank, fragments)
            # num_unreserved_bank = fmem_info.get_num_unreserved_bank()
            # fragments = control_info.limit_load_fragments(num_fragments, num_unreserved_bank - 1, fragments)

        if not fragments:
            return False

        control_info.action.append(
            ('LOAD', len(control_info.input_mapping) + len(fragments)))

        for fragment in fragments:
            bank = fmem_info.save_data_to_empty_bank(input_layer, fragment)
            if bank is None:  # there is no bank to use
                break
            mapping = fmem_info.get_fmem_mapping_info(input_layer.name)
            fmem_info.reserve_input_banks(mapping, self.input_stationary)
            control_info.set_mapping_load_flag(bank)
            control_info.add_input_mapping([[bank, fragment, False]])

        return True
