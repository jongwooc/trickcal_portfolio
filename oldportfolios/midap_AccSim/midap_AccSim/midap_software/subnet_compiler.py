from __future__ import absolute_import, division, print_function, unicode_literals

import logging
import sys
from collections import defaultdict
from functools import reduce

import numpy as np

from config import cfg
from generic_op import FC, ArithmeticOp, ConcatOp, ConvOp, PoolOp
from midap_software import midap_path
from midap_software.double_buffer_compiler import DoubleBufferCompiler
from midap_software.hide_mem_latency import HideMemLatency
from midap_software.layer_block import BlockBuilder, LayerBlock
from midap_software.min_mem_access import MinMemAccess


class SubNetCompiler(dict):
    def __init__(self, *args, **kwargs):
        super(SubNetCompiler, self).__init__(*args, **kwargs)

        # HW Configuration
        self.width = cfg.MIDAP.SYSTEM_WIDTH
        self.num_fmem = cfg.MIDAP.FMEM.NUM
        # how many elements can be located in one fmem bank
        self.fmem_size = cfg.MIDAP.FMEM.NUM_ENTRIES * 1024
        self.fmem_height = self.fmem_size // self.width
        self.wmem_num = cfg.MIDAP.WMEM.NUM

        # Policy
        self.first_layer_strategy = cfg.MIDAP.CONTROL_STRATEGY.FIRST_LAYER
        if cfg.MIDAP.CONTROL_STRATEGY.LAYER_COMPILER == 'MIN_DRAM_ACCESS':
            self.layer_compiler = MinMemAccess()
        elif cfg.MIDAP.CONTROL_STRATEGY.LAYER_COMPILER == 'HIDE_DRAM_LATENCY':
            self.layer_compiler = HideMemLatency()
        elif cfg.MIDAP.CONTROL_STRATEGY.LAYER_COMPILER == 'DOUBLE_BUFFER':
            self.layer_compiler = DoubleBufferCompiler()

        self.model = None
        self.debug = False
        self.path_checker = defaultdict(int)

        self.init_logger(logging.DEBUG if self.debug else logging.INFO)
        self.block_organizer = BlockBuilder()

    def __del__(self):
        del self.layer_compiler, self.block_organizer

    def init_logger(self, level=logging.INFO):
        self.logger = logging.getLogger('SubNetCompiler')
        self.logger.setLevel(level)

        if not self.logger.handlers:
            ch = logging.StreamHandler()
            ch.setLevel(level)

            # , datefmt='%y/%m/%d %H:%M')
            formatter = logging.Formatter(
                "[ %(levelname)8s | %(filename)20s:%(lineno)3d ]  %(message)s")
            ch.setFormatter(formatter)
            self.logger.addHandler(ch)

    def force_setup(self, num_init_banks):
        self.layer_compiler.force_setup_layer(num_init_banks)

    def setup(self, midap_model):
        self.model = midap_model
        self.logger.debug("-" * 80)
        self.logger.debug("|{:^78}|".format('Determine Path & Stationary'))
        self.logger.debug("-" * 80)
        self.logger.debug("|{:^20s}|{:^15s}|{:^20s}|{:^20s}|".format(
            "Name", "Op Type", "Input Shape", "Output Shape"))
        self.logger.debug("-" * 80)
        self.determine_path_and_stationary()
        self.logger.debug("-" * 80)

        self.logger.debug("|{:^78}|".format('Generating Control Code'))
        self.generate_control_info()
        self.logger.debug("-" * 80)

        self.logger.debug("|{:^78}|".format('Post Processing'))
        self.post_process()
        self.logger.debug("-" * 80)

    def get_model_control_info(self):
        # return self.processing_order[0].input[0].output_tensor, self.processing_order
        input_tensor_list = [
            self.model[l].output_tensor for l in self.model.init_layer]
        return input_tensor_list, self.processing_order

    def post_process(self):
        # generate detail control information for each layer
        prev_layer = self.processing_order[0]
        for layer_info in self.processing_order[1:]:
            if isinstance(layer_info.main_op, ConvOp):
                prev_layer.control_info['prepare'] = layer_info
                prev_layer = layer_info
            if isinstance(layer_info.main_op, ArithmeticOp):
                if layer_info.main_op.broadcast and prev_layer in layer_info.input:
                    prev_layer.control_info['prepare'] = None
                else:
                    prev_layer.control_info['prepare'] = layer_info
                prev_layer = layer_info
            if isinstance(layer_info.main_op, FC):
                main_op = layer_info.main_op
                input_shape = layer_info.input[0].output_tensor.shape
                main_op.k_w, main_op.k_h = input_shape[0], input_shape[1]
                weight = main_op.weight
                weight = weight.reshape(
                    weight.shape[0], -1, input_shape[0], input_shape[1]).transpose(0, 3, 2, 1)
                weight = weight.reshape(weight.shape[0], input_shape[0], -1)
                if layer_info.control_info.input_flip:
                    weight = np.flip(weight, axis=1)
                main_op.weight = weight
        for layer_info in self.processing_order:
            last_process = 0
            for action, fragment in layer_info.control_info.action:
                if action == 'LOAD':
                    continue
                fragment_size = fragment - last_process
                last_process = fragment
                filter_load = self.get_filter_load(layer_info, fragment_size)
                layer_info.control_info.filter_group.append(filter_load)

    def get_filter_load(self, layer_info, fragment_size):
        if isinstance(layer_info.main_op, PoolOp):
            return 0
        if cfg.MIDAP.CONTROL_STRATEGY.FILTER_LOAD == 'ONE':
            return 1
        elif cfg.MIDAP.CONTROL_STRATEGY.FILTER_LOAD == 'MAXIMUM':
            target_filter = None
            if isinstance(layer_info.main_op, ConvOp):
                target_filter = layer_info.main_op.weight
                # NWHC
            elif isinstance(layer_info.main_op, ArithmeticOp):
                target_filter = layer_info.input[1].output_tensor
                # WHC
            else:
                raise ValueError("Undefined filter size")
            filter_size = target_filter[0].size
            num_filter = target_filter.shape[0]
            if layer_info.parallel_type is None:
                num_filter = num_filter // cfg.MIDAP.WMEM.NUM
            maximum_load = min(
                num_filter, cfg.MIDAP.WMEM.NUM_ENTRIES // filter_size)
            if maximum_load == 0:
                raise ValueError("Too big filter size")
            return maximum_load
        else:
            self.determine_filter_load(layer_info, fragment_size)

    def determine_filter_load(self, layer_info, fragment_size):
        pass  # TODO: Should be implemented

    def determine_path_and_stationary(self):
        # input_blob = list(self.model.keys())[0]  # TODO --> self.model.init_layer
        # input_layer = self.model[input_blob]  # TODO --> [self.model[x] for x in input_blob]
        input_blob = self.model.init_layer
        input_layer = [self.model[x] for x in input_blob]

        paths = self.block_organizer.make_block_path(input_layer)
        po = []
        for v in paths:
            v.log_print(self.logger.debug)
            if isinstance(v, LayerBlock):
                po.extend(v.get_ordered_path())
            else:
                po.append(v)
        self.processing_order = [v for v in po if v.main_op.type != 'HEAD']

        # self.processing_order, _ = self.determine_processing_order(input_layer, [], debug_print=True)
        # self.processing_order = self.processing_order[1:]  # Input blob is data blob
        # print('Gold', [l.name for l in self.processing_order])


################################# OLD Implementation ( < 1.4.0, Deprecated) ################################


    def determine_input_stationary(self, num_input_fragments, paths):
        maximum_stationary = min(num_input_fragments, self.num_fmem - 1)
        input_stationary = []
        for path in paths[:-1]:
            minimum_overhead = (-1, sys.maxsize)  # idx, overhead
            path_cost = [l.require_fmem for l in path]
            for stationary in reversed(range(maximum_stationary + 1)):
                input_overhead = (num_input_fragments - stationary)
                num_available_slot = self.num_fmem - stationary - 1
                intermediate_overhead = reduce(
                    lambda x, y: x + max(y - num_available_slot, 0), [0] + path_cost)
                overhead = input_overhead + intermediate_overhead
                if overhead < minimum_overhead[1]:
                    minimum_overhead = (stationary, overhead)
            input_stationary.append(minimum_overhead[0])
        input_stationary.append(0)
        return input_stationary

    def generate_control_info(self):
        for layer in self.processing_order:
            # print("============================================== [ {:^12s} ] ==============================================".format(layer.name))
            self.layer_compiler.setup_layer(layer)
            self.layer_compiler.compile_layer()
            # print("=============================================================================================================\n")
        # print("Control Manager: Control info generation is finished")

    def check_available(self, layer_info, unifying):
        if layer_info.main_op.type in ['Concat', 'Sum', 'Mul']:
            check = len(layer_info.input)
            if isinstance(layer_info.main_op, ArithmeticOp) and layer_info.main_op.broadcast:
                check = 1
            if self.path_checker[layer_info.name] == check:
                if not unifying:
                    raise ValueError(
                        "Layer is referred incorrectly: {}".format(layer_info))
                return True
            else:
                if not unifying:
                    self.path_checker[layer_info.name] += 1
                return False
        else:
            return True

    def determine_processing_order(self, layer_info, po, unifying=False):
        # TODO make this code simple
        if not self.check_available(layer_info, unifying):
            # It must be stalled for postprocessing - it means that some diversed branches are unified
            return po, layer_info

        po.append(layer_info)
        if len(layer_info.next) == 0:
            return po, None
        elif len(layer_info.next) == 1:
            if layer_info.have_reduction_layer:
                print("Reduction Configuration - single-branch")
                layer_info.control_info.os = 0
            return self.determine_processing_order(layer_info.next[0], po)

        reduction_path = None
        paths = [self.determine_processing_order(
            next_layer, []) for next_layer in layer_info.next]
        if layer_info.have_reduction_layer:
            print("Reduction Configuration - multi-branch")
            reduction_path = paths[0]
            paths = paths[1:]
        out_paths, inner_paths, unifying_layer = midap_path.classify_paths(
            paths)

        self.logger.debug("Branch diversed at {:^10s}: # of out_path = {}, inner path = {}, unifying_layer = {}".format(
            layer_info.name, len(out_paths), len(inner_paths), unifying_layer.name))
        if unifying_layer is not None:
            if isinstance(unifying_layer.main_op, ArithmeticOp) and not unifying_layer.main_op.broadcast:
                ordered_input = [
                    path[-1] if len(path) > 0 else layer_info for path in inner_paths]
                ordered_input[0].next.remove(unifying_layer)
                unifying_layer.input = list(reversed(ordered_input))
        else:  # if all path is output
            pass

        input_stationary = self.determine_input_stationary(
            layer_info.require_fmem, out_paths + inner_paths)
        if reduction_path is not None:
            if reduction_path[1] is not None and unifying_layer is not None and unifying_layer != reduction_path[1]:
                raise ValueError(
                    "Reduction path is not met with original branch")
            if reduction_path[1] is not None:
                unifying_layer = reduction_path[1]
            inner_paths.append(reduction_path[0])
            input_stationary.append(0)

        # Process out_paths
        for (stationary, path) in zip(input_stationary[:len(out_paths)], out_paths):
            if len(path) == 0:
                continue
            po = po + path
            path[0].control_info.input_stationary = stationary
            for l in path[:-1]:
                l.require_fmem += stationary

        if unifying_layer is None:
            return po, None

        memory_op_os_available = True
        max_cost = 0
        for (stationary, path) in zip(input_stationary[len(out_paths):], inner_paths):
            if len(path) == 0:
                memory_op_os_available = False
                continue
            po = po + path
            path[0].control_info.input_stationary = stationary
            max_cost = max(max_cost, stationary)
            for l in path[:-1]:
                l.require_fmem += stationary
                max_cost = max(max_cost, l.require_fmem)

        os = 0 if not memory_op_os_available else max(
            0, self.num_fmem - 1 - max_cost)
        # reduction
        if reduction_path is not None:
            path = reduction_path[0]
            path[0].control_info.output_stationary = 0
            os = 0

        os = min(unifying_layer.require_fmem, os)
        if isinstance(unifying_layer.main_op, ConcatOp):
            if self.check_available(unifying_layer, True):
                print("Output stationary configuration for the unifying layer {}".format(
                    unifying_layer.name))
                for l in unifying_layer.input:
                    l.require_fmem += os
                    l.control_info.output_stationary = os
                    print(l.name, 'Required', l.require_fmem,
                          'Out Stationary', os)
        elif isinstance(unifying_layer.main_op, ArithmeticOp):
            if len(inner_paths[0]) > 0:
                inner_paths[0][-1].control_info.output_stationary = 0

        self.print("Unified path: {}, Unifying layer: {}".format(
            [x.name for x in po], unifying_layer))
        return self.determine_processing_order(unifying_layer, po, True)
################################# OLD Implementation END ( < 1.4.0, Deprecated) ################################
