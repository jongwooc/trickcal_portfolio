from __future__ import division, print_function

from copy import copy

from config import cfg
from generic_op import ArithmeticOp, ConcatOp

from .layer_compiler import LayerCompiler
from .time_line import TimeLine


class HideMemLatency(LayerCompiler):
    def setup_layer(self, path_info):
        if self.layer and not isinstance(self.layer.main_op, ConcatOp) and not self.layer.reduction:
            self.prev_layer = self.layer
        elif not self.layer:
            self.prev_layer = None
        super(HideMemLatency, self).setup_layer(path_info)
        self.time_line = TimeLine(self.layer, self.prev_layer)

    def _preprocess(self):
        self.remain_fix_flag = False
        super(HideMemLatency, self)._preprocess()
        layer = self.layer
        if not (isinstance(layer.main_op, ConcatOp) or isinstance(layer.main_op, ArithmeticOp)):
            self._set_outbank_num()
        else:
            self.num_remain_banks = 1

    def _is_load_end(self):
        input_layer = self.layer.input[0]
        control_info = self.layer.control_info
        load_head = 0 if not control_info.input_mapping else control_info.input_mapping[-1][1][1]
        return load_head == input_layer.output_tensor.shape[0]

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
        min_delay = sys.maxsize
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
            delay = self.time_line.cim_blocking_time
            # print(layer.name, delay, min_delay)
            if delay < min_delay:
                min_delay = delay
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
            del self.time_line
            self.time_line = TimeLine(self.layer, self.prev_layer)
            self.remain_fix_flag = False
            if min_delay == 0:
                break

        self._flip(num_output, min_bank_num)
        # print('[SET]', layer.name, self.num_remain_banks, control_info.reverse_write, control_info.flip)

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

        # all fragments which is loaded or loading
        restricted_fragments = processing_fragments = copy(
            control_info.remain_inputs)
        max_frag_num = len(processing_fragments)
        num_unreserved_bank = fmem_info.get_num_unreserved_bank()
        output_fragments = layer.get_output_fragments(num_unreserved_bank - self.num_remain_banks)
        if not control_info.fixed_output:
            self._set_out_mappings(output_fragments)

            if control_info.num_output_mapping < len(output_fragments):
                restricted_fragments = control_info.limit_processing_fragments(
                    output_fragments, processing_fragments)
                if not restricted_fragments:
                    self._fix_exception()
            else:
                control_info.fixed_output = True  # no more change occurs

        last_flag = (self._is_load_end() and len(control_info.remain_inputs) == len(restricted_fragments))
        bound = control_info.get_out_x_in_fmem(output_fragments, 0) if output_fragments else -1
        process_num = self.time_line.limit_processing_fragments(restricted_fragments, bound, max(max_frag_num - (0 if self._is_load_end() else 1), 1), last_flag)
        processing_fragments = processing_fragments[:process_num]

        # post process
        if not processing_fragments:
            return

        control_info.action.append(('PROCESS', layer.processed_input + len(processing_fragments)))
        last_flag = (self._is_load_end() and len(control_info.remain_inputs) == len(processing_fragments))
        self.time_line.process(processing_fragments, bound, is_last=last_flag)

        for bank, _, _ in processing_fragments:
            control_info.process_input()
            fmem_info.discard_data(bank)

    def _do_load(self):
        fmem_info = self.fmem_info
        layer = self.layer
        input_layer = layer.input[0]
        control_info = layer.control_info

        overlap = not control_info.remain_inputs
        num_available_banks = fmem_info.get_num_available_bank()
        load_head = 0 if not control_info.input_mapping else control_info.input_mapping[-1][1][1]
        input_fragments = layer.get_input_fragments(
            num_available_banks, load_head, overlap=overlap and layer.processed_input > 0)

        if not control_info.fixed_output:
            num_unreserved_bank = fmem_info.get_num_unreserved_bank()
            num_output_bank = num_unreserved_bank - self.num_remain_banks
            input_fragments = control_info.limit_load_fragments(
                num_available_banks, num_output_bank, input_fragments)

        if not input_fragments:
            return False

        control_info.action.append(
            ('LOAD', len(control_info.input_mapping) + len(input_fragments)))

        for fragment in input_fragments:
            bank = fmem_info.save_data_to_empty_bank(input_layer, fragment)
            if bank is None:  # there is no bank to use
                break
            mapping = fmem_info.get_fmem_mapping_info(input_layer.name)
            fmem_info.reserve_input_banks(mapping, self.input_stationary)
            control_info.set_mapping_load_flag(bank)
            control_info.add_input_mapping([[bank, fragment, False]])

        self.time_line.load(input_fragments, self._is_load_end())
        return True
