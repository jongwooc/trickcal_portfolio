from __future__ import print_function

from collections import deque

from acc_utils.errors import CompilerError
from config import cfg
from midap_software.layer_compiler import LayerCompiler

from .fmem_info import FMEMInfo


class DoubleBufferFMEMInfo(FMEMInfo):
    def __del__(self):
        del self.in_available_queue, self.out_available_queue, self.mapping_info

    def reset(self):
        self.in_available_queue = deque([i for i in range(cfg.MIDAP.FMEM.NUM // 2)])
        self.out_available_queue = deque([i for i in range(cfg.MIDAP.FMEM.NUM // 2, cfg.MIDAP.FMEM.NUM)])
        self.mapping_info = []

    def reverse_in_out_queue(self):
        self.out_available_queue, self.in_available_queue = self.in_available_queue, self.out_available_queue

    def discard_data_by_layer(self, name, reverse_order=False):
        discard_list = []
        for idx, data in enumerate(self.mapping_info):
            n = data[0]
            if n == name:
                discard_list.append(idx)

        for idx in (reversed(discard_list) if reverse_order else discard_list):
            _, bank, _ = self.mapping_info[idx]
            if reverse_order:
                raise CompilerError("This case cannot happen")
            else:
                self.in_available_queue.append(bank)

        for idx in reversed(discard_list):
            del self.mapping_info[idx]

    def discard_data(self, bank):
        discard_idx = -1
        for idx, data in enumerate(self.mapping_info):
            b = data[1]
            if b == bank:
                discard_idx = idx
                break

        self.in_available_queue.append(bank)
        del self.mapping_info[discard_idx]

    def get_num_available_bank(self):
        return len(self.in_available_queue)

    def save_data_to_empty_bank(self, layer, data):
        raise CompilerError("This case cannot happen")

    def _pop_available_in_bank(self):
        if not self.in_available_queue:
            return None
        bank = self.in_available_queue.popleft()
        return bank

    def save_data_to_empty_in_bank(self, layer, data):
        name = layer.name

        # FIXME check that data is already in fmem.
        for n, b, d in self.mapping_info:
            if name == n and d == data:
                return None

        bank = self._pop_available_in_bank()
        if bank is not None:
            self._save_data(name, bank, data)
        return bank

    def _pop_available_out_bank(self):
        if not self.out_available_queue:
            return None
        bank = self.out_available_queue.popleft()
        return bank

    def save_data_to_empty_out_bank(self, layer, data):
        name = layer.name

        # FIXME check that data is already in fmem.
        for n, b, d in self.mapping_info:
            if name == n and d == data:
                return None

        bank = self._pop_available_out_bank()
        if bank is not None:
            self._save_data(name, bank, data)
        return bank

    def get_num_unreserved_bank(self):
        return cfg.MIDAP.FMEM.NUM // 2


class DoubleBufferCompiler(LayerCompiler):
    def __init__(self):
        self.fmem_info = DoubleBufferFMEMInfo()
        self.force_init = False
        self.debug = False
        self.num_remain_banks = 0

    def __del__(self):
        del self.fmem_info

    def _preprocess_input_fmem(self):
        fmem_info = self.fmem_info
        input_layer = self.layer.input[0]

        mapping = fmem_info.get_fmem_mapping_info(input_layer.name)
        if mapping:
            self.layer.control_info.add_input_mapping(mapping, True)

    def _preprocess_output_fmem(self):
        pass

    def _preprocess(self):
        import sys
        control_info = self.layer.control_info
        control_info.process_flip(sys.maxsize, -1)

        if control_info.output_stationary >= 0:
            control_info['fixed_output'] = True

        # no stationary
        self.input_stationary  = control_info.input_stationary  = 0
        # def is_no_layer_branch(layer):
        #     if layer.next:
        #         next_layer = layer.next[0]
        #         return (next_layer.main_op.type in ['Concat', 'Sum', 'Mul'])
        #     else:
        #         return False

        # self.input_stationary  = control_info.input_stationary  = control_info.input_stationary if is_no_layer_branch(self.layer) else 0
        self.output_stationary = control_info.output_stationary = -1

        # Setting the initial MIDAP state
        self._preprocess_input_fmem()
        if len(self.layer.next) == 0:
            control_info['fixed_output'] = True

    def _set_out_mappings(self, output_fragments):
        fmem_info = self.fmem_info
        layer = self.layer
        control_info = layer.control_info

        for o_frag in output_fragments[control_info.num_output_mapping:]:
            bank = fmem_info.save_data_to_empty_out_bank(layer, o_frag)
            if bank is None:  # there is no bank to use
                break
            control_info.add_output_mapping([(bank, o_frag)])

    def _is_load_end(self):
        input_layer = self.layer.input[0]
        control_info = self.layer.control_info
        load_head = 0 if not control_info.input_mapping else control_info.input_mapping[-1][1][1]
        return load_head == input_layer.output_tensor.shape[0]

    def compile_layer(self):
        super(DoubleBufferCompiler, self).compile_layer()
        self.fmem_info.reverse_in_out_queue()

    def _do_step(self):
        layer = self.layer
        control_info = layer.control_info

        if self._is_load_end() or not self._do_load():
            self._do_operation()

        if control_info.input_mapping and not control_info.remain_inputs:
            return not layer.get_input_fragments(1, control_info.input_mapping[-1][1][1])

        return False

    def _fix_exception(self):
        raise CompilerError("This case cannot happen")

    def _do_operation(self):
        fmem_info = self.fmem_info
        layer = self.layer
        control_info = layer.control_info

        processing_fragments = control_info.remain_inputs[:1]  # Remain input mappings

        if not control_info.fixed_output:
            output_fragments = layer.get_output_fragments(cfg.MIDAP.FMEM.NUM // 2)
            self._set_out_mappings(output_fragments)

        # post process
        if not processing_fragments:
            return

        control_info.action.append(('PROCESS', layer.processed_input + 1))

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

        fragments = layer.get_input_fragments(num_fragments, load_head, overlap=overlap and layer.processed_input > 0)

        if not fragments:
            return False

        control_info.action.append(('LOAD', len(control_info.input_mapping) + len(fragments)))

        for fragment in fragments:
            bank = fmem_info.save_data_to_empty_in_bank(input_layer, fragment)
            if bank is None:  # there is no bank to use
                break
            control_info.set_mapping_load_flag(bank)
            control_info.add_input_mapping([[bank, fragment, False]])

        return True
