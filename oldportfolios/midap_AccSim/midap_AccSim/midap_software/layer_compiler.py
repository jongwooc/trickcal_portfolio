import logging

from generic_op import ConcatOp
from logger import init_logger

from .fmem_info import FMEMInfo


class LayerCompiler(object):
    def __init__(self):
        self.fmem_info = FMEMInfo()
        self.force_init = False
        self.logger = init_logger(self.__class__.__name__, logging.INFO)
        self.layer = None

    def __del__(self):
        del self.fmem_info, self.logger

    def force_setup_layer(self, num_init_banks):
        self.force_init = True
        self.force_init_banks = num_init_banks

    def _force_init_banks(self):
        fmem_info = self.fmem_info
        layer = self.layer
        fragments = layer.get_input_fragments(self.force_init_banks, 0)
        for _, f in enumerate(fragments):
            _ = fmem_info.save_data_to_empty_bank(layer.input[0], f)

    def setup_layer(self, layer):
        self.layer = layer
        self.output_stationary = layer.control_info.output_stationary
        self.input_stationary = layer.control_info.input_stationary

    def _fix_exception(self):
        fmem_info = self.fmem_info
        layer = self.layer
        control_info = layer.control_info
        if control_info.reverse_write:
            self.remain_fix_flag = True

            fmem_info.discard_data_by_layer(layer.name, True)

            control_info.clear_output_mapping()
            self.num_remain_banks += 1
            num_unreserved_bank = fmem_info.get_num_unreserved_bank()
            max_output = num_unreserved_bank - fmem_info.get_num_available_bank()
            if self.num_remain_banks >= max_output:
                control_info.fixed_output = True
                output_fragments = layer.get_output_fragments(num_unreserved_bank - max_output)
                self._set_out_mappings(output_fragments)
        else:
            control_info.fixed_output = True

    def compile_layer(self):
        layer = self.layer

        if isinstance(layer.main_op, ConcatOp):
            return

        if self.force_init:
            self.force_init = False
            self._force_init_banks()
        self._preprocess()
        fmem_info = self.fmem_info
        end = False
        if layer.reduction:
            control_info = layer.control_info
            num_unreserved_bank = fmem_info.get_num_unreserved_bank()
            output_fragments = layer.get_output_fragments(num_unreserved_bank - self.num_remain_banks)
            if not control_info.fixed_output:
                self._set_out_mappings(output_fragments)
        else:
            while not end:
                end = self._do_step()

        # FIXME is this code really need?
        if self.input_stationary <= 0:
            fmem_info.discard_data_by_layer(layer.input[0].name)

        if layer.control_info.reverse_write:
            fmem_info.reverse_mapping(layer.name)

    def _do_step(self):
        raise NotImplementedError

    def _set_out_mappings(self, output_fragments):
        fmem_info = self.fmem_info
        layer = self.layer
        control_info = layer.control_info

        for o_frag in output_fragments[control_info.num_output_mapping:]:
            bank = fmem_info.save_data_to_empty_bank(layer, o_frag)
            if bank is None:  # there is no bank to use
                break
            control_info.add_output_mapping([(bank, o_frag)])

    def _preprocess_input_fmem(self):
        fmem_info = self.fmem_info
        input_layer = self.layer.input[0]

        mapping = fmem_info.get_fmem_mapping_info(input_layer.name)
        fmem_info.reserve_input_banks(mapping, self.input_stationary)
        if mapping:
            self.layer.control_info.add_input_mapping(mapping, True)

    def _preprocess_output_fmem(self):
        fmem_info = self.fmem_info
        control_info = self.layer.control_info
        output_stationary = self.output_stationary

        control_info['fixed_output'] = output_stationary >= 0
        if output_stationary > 0:
            next_layer = self.layer.next[0]
            fmem_info.reserve_output_banks(next_layer, output_stationary)
            mapping = fmem_info.get_fmem_mapping_info(next_layer.name)
            control_info.add_output_mapping([(m[0], m[1]) for m in mapping])

    def _preprocess(self):
        control_info = self.layer.control_info
        fmem_info = self.fmem_info
        control_info.process_flip(
            fmem_info.get_num_unreserved_bank(), self.output_stationary)

        # Setting the initial MIDAP state
        self._preprocess_input_fmem()

        if len(self.layer.next) == 0:
            control_info['fixed_output'] = True
            # self.output_stationary = 0
        else:
            self._preprocess_output_fmem()
