from __future__ import print_function

from acc_utils.attrdict import AttrDict


class ControlInfo(AttrDict):
    def __init__(self, layer):
        super(ControlInfo, self).__init__({
            'input_stationary': -1,
            'output_stationary': -1,
            'flip': False,
            'input_flip': False,
            'action': [],
            'filter_group': [],
            'prepare': None,
            # [fmem_idx, (head, tail), load_new_fragment?]
            'input_mapping': [],
            'output_mapping': [],
            'num_initial_fragments': 0,
            'reverse_write': False
        })

        # for compile
        self.layer = layer
        self.remain_inputs = []
        self.fixed_output = None
        self.num_output_mapping = 0

    def process_flip(self, num_available_bank, output_stationary):
        layer = self.layer
        # XXX assume that the first input layer remains its output feature map on FMEM
        self.input_layer = self.layer.input[0]
        input_flip = self.input_layer.control_info.flip

        if output_stationary < 0:
            # input_size = self.input_layer.get_output_size()
            # output_size = layer.get_output_size()
            # and input_size < output_size
            reverse_write = layer.require_fmem >= num_available_bank
            flip = not input_flip if reverse_write else input_flip
        else:
            flip = False
            reverse_write = (flip != input_flip)

        self.flip = flip
        self.reverse_write = reverse_write
        self.input_flip = input_flip

        if input_flip:
            layer.main_op.flip_operation()
            if layer.sub_op is not None:
                layer.sub_op.flip_operation()

    # function about input/output mapping
    def add_input_mapping(self, inputs, is_initial=False):
        self.input_mapping.extend(inputs)
        self.remain_inputs.extend(inputs)
        if is_initial:
            self.num_initial_fragments += len(inputs)

    def process_input(self):
        del self.remain_inputs[0]
        self.layer.processed_input += 1

    def add_output_mapping(self, outputs):
        self.output_mapping.extend(outputs)
        self.num_output_mapping += len(outputs)

    def clear_output_mapping(self):
        self.output_mapping = []
        self.num_output_mapping = 0

    def set_mapping_load_flag(self, fmem_id):
        for info in self.input_mapping:
            if info[0] == fmem_id:
                info[2] = True

    def get_out_x_in_fmem(self, output_fragments, len_output):
        return output_fragments[len_output][1] if self.reverse_write else output_fragments[len_output][0]

    # return input fragments to process, considering fmem size
    def limit_processing_fragments(self, output_fragments, processing_fragments):
        layer = self.layer
        len_output = self.num_output_mapping
        if len_output >= len(output_fragments):
            len_output = len(output_fragments) - 1
        criteria = self.get_out_x_in_fmem(output_fragments, len_output)
        num_inputs_to_process = 0
        for _, fragment, _ in processing_fragments:
            output_x = layer.get_output_x(
                fragment, num_inputs_to_process + 1 < len(processing_fragments))
            if (self.reverse_write and output_x < criteria) or (not self.reverse_write and output_x >= criteria):
                break
            num_inputs_to_process += 1
        return processing_fragments[:num_inputs_to_process]

    # return input fragments to load, considering fmem size
    def limit_load_fragments(self, num_available_banks, max_output_banks, input_banks):
        layer = self.layer

        output_fragments = layer.get_output_fragments(max_output_banks)

        overlap = self.remain_inputs
        first_fragment = input_banks[0] if not overlap else self.remain_inputs[0][1]
        output_x = layer.get_output_x(first_fragment, True)

        output_required = 0
        for head, tail in output_fragments:
            # print('[ Limit Load ]', output_x, (head, tail), self.reverse_write)
            if (output_x >= head and not self.reverse_write) or (output_x < tail and self.reverse_write):
                output_required += 1
            else:
                break
        output_required -= len(self.output_mapping)
        slots_for_output = num_available_banks - len(input_banks)
        output_required -= slots_for_output

        if output_required > num_available_banks:
            input_banks = []
            # raise ValueError("unavailable scheduling")
        elif output_required > 0:
            input_banks = input_banks[:-output_required]
        return input_banks
