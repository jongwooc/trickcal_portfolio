from __future__ import absolute_import, division, print_function, unicode_literals

import logging

import numpy as np

import midap_simulator.statistics as stats
from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *
from midap_software import *

from .main_module import MainModule
from .memory_manager import MemoryManager
from .post_module import PostModule
from .reduction_module import ReductionModule


class MidapManager():
    # MidapManager processes each layer based on given control sequence
    def __init__(self, *args, **kwargs):
        # initialize submodules (cfg)
        self.memory_manager = MemoryManager()
        self.main_module = MainModule(self.memory_manager)
        self.reduction_module = ReductionModule()
        self.post_module = PostModule(self.memory_manager)
        self.stat = stats.init()
        self.print_stats = True
        self.diff_dict = {}
        self.dump = False
        self.filter_idx = 0
        self.num_filter_set = 0
        self.num_fmem = cfg.MIDAP.FMEM.NUM
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.logger = logging.getLogger(cfg.LOGGER)

    def __del__(self):
        del self.memory_manager, self.main_module, self.reduction_module, self.post_module, self.diff_dict, self.logger

    def process_network(self, input_data, path_info):
        self.input_data = input_data
        self.memory_manager.add_dram_info(
            path_info[0].input[0].name, self.input_data)
        return self._process_network(path_info)

    def process_network_with_multiple_input(self, input_data_list, init_layer_list, path_info):
        if len(input_data_list) != len(init_layer_list):
            raise ValueError("Invalid input")
        for data, name in zip(input_data_list, init_layer_list):
            self.memory_manager.add_dram_info(name, data)
        return self._process_network(path_info)

    def _process_network(self, path_info):
        for idx, layer_info in enumerate(path_info):
            self.process_layer(layer_info, idx)
        stats.end_simulation()
        self.memory_manager.mem_wait(-1, 0)
        last_diff = 0 if path_info[-1].output_name not in self.diff_dict else np.sum(
            self.diff_dict[path_info[-1].output_name])
        stat = self.stat.GLOBAL
        latency = stat.CLOCK
        dram_access = stat.READ.DRAM + stat.WRITE.DRAM
        return last_diff, latency, dram_access

    def process_layer(self, layer_info, layer_idx=None):  # layer_idx : debugging info
        self.layer_info = layer_info
        self.control_info = layer_info.control_info
        self.main_op = layer_info.main_op
        print("Process: {}\nmain_op: {} reduction_op: {}\n".format(layer_info.name, self.main_op,
                                                                   None if not self.layer_info.have_reduction_layer else self.layer_info.next[0].name))
        print("Action: {}".format(self.control_info.action))
        print("Expected input mapping: {}\n Expected output mapping: {}".format(
            self.control_info.input_mapping, self.control_info.output_mapping))
        self.input_tensor = self.layer_info.input[0].output_tensor
        self.output_tensor = self.layer_info.output_tensor
        self.input_shape = self.input_tensor.shape
        self.output_shape = self.output_tensor.shape
        if isinstance(self.main_op, ConcatOp) or self.layer_info.reduction:
            self.compare_result()
            return None
        self.main_module.setup(layer_info)
        self.reduction_module.setup(layer_info)
        self.post_module.setup(layer_info)
        self.memory_manager.setup(layer_info)
        self.filter_idx = 0
        self.output_parallelism = self.num_wmem if self.layer_info.parallel_type is None else self.system_width
        self.num_filter_set = self.main_op.output_tensor.shape[-1]
        if isinstance(self.layer_info.main_op, ArithmeticOp):
            self.process_arithmetic_layer()
        else:
            self.process_general_layer()
        self.finish_computation()
        if self.layer_info.have_reduction_layer:
            self.reduction_module.enable_write()
            self.post_module.setup(self.layer_info.next[0])
            self.finish_computation()
        self.compare_result()
        stats.set_macs(layer_info.main_op.get_macs())
        stats.update(self.layer_info.name, self.print_stats)
        print("-------------------------------------------------------------------------------------------------------------")

    def compare_result(self):
        output_name = self.layer_info.output_name
        head, tail = self.layer_info.offset
        n = self.layer_info.sub_op.output_tensor if self.layer_info.sub_op is not None else self.layer_info.main_op.output_tensor
        n = np.flip(n, axis=0) if self.control_info.flip else n
        new_tail = head + n.shape[-1]
        p = self.memory_manager.dram_dict[output_name]
        diff = np.abs(n - p[:, :, head:new_tail])
        abs_arr = np.abs(n) + np.abs(p[:, :, head:new_tail])
        abs_arr = np.where(abs_arr > 0, abs_arr, 1)
        diff_ratio = np.true_divide(diff, abs_arr)
        diff_arr = np.where(diff_ratio < 0.01, 0, 1)
        diff_value = np.sum(diff_arr)
        print("Function Simulation result - layer: {}, diff: {} / {}".format(
            self.layer_info.name, diff_value, diff_arr.size))
        if diff_value > 0:
            self.diff_dict[self.layer_info.name] = diff_arr
        self.diff_dict[self.layer_info.name + '_diff'] = diff

    def get_min_max_height(self):
        main_op = self.layer_info.main_op
        if main_op.type == 'Gemm':
            return -1, -1
        y_min = 0
        y_max = self.input_shape[1] * self.layer_info.scale_h
        if isinstance(main_op, ConvPoolOpBase):
            y_min = -main_op.pad_t
            y_max += main_op.pad_b
            y_max -= main_op.k_h

        return y_min, y_max

    def get_min_max_width(self, head_idx, tail_idx, next_on_chip=False):
        main_op = self.layer_info.main_op
        scale = self.layer_info.scale_w
        input_mapping = self.control_info.input_mapping
        head_info = input_mapping[head_idx][1]
        tail_info = input_mapping[tail_idx - 1][1]
        x_min = head_info[0] * scale
        x_max = tail_info[1] * scale - 1
        if isinstance(main_op, ConvPoolOpBase) and main_op.type != 'Gemm':
            x_limit = self.input_shape[0] * scale - main_op.k_w + main_op.pad_r
            if head_idx == 0:
                x_min -= main_op.pad_l
            if tail_idx == len(input_mapping):
                x_max += main_op.pad_r
            if not next_on_chip:
                x_max -= main_op.k_w - 1
            remain = (x_min + main_op.pad_l) % main_op.stride
            if remain > 0:
                x_min = x_min + remain
            x_max = min(x_max, x_limit)
        self.logger.debug(
            "get_min_max_width: x_min, x_max = {}, {}".format(x_min, x_max))
        return x_min, x_max

    def process_general_layer(self):
        num_fragments_on_fmem = self.control_info.num_initial_fragments
        input_mapping = self.control_info.input_mapping
        action = self.control_info.action
        # Multiplication at (0,0) is a reference point (nxn conv includes (0,0) ~ (n-1, n-1))
        self.y_min, self.y_max = self.get_min_max_height()
        cur_idx = 0
        proc_cnt = 0
        if self.main_op.type == 'Depthwise':
            self.memory_manager.change_and_setup_wmem(0, True)
        for action_type, tail_idx in action:
            if action_type == 'LOAD':
                self.logger.info("process_general_layer: LOAD ({}, {})".format(
                    num_fragments_on_fmem, tail_idx))
                for idx in range(num_fragments_on_fmem, tail_idx):
                    stats.increase_cycle()
                    fmem_idx, info, _ = input_mapping[idx]
                    self.memory_manager.load_fmem(fmem_idx, info)
                num_fragments_on_fmem = tail_idx
            else:
                last_flag = tail_idx == len(input_mapping)
                self.filter_idx = 0
                self.logger.info(
                    "process_general_layer: PROCESS ({}, {})".format(cur_idx, tail_idx))
                x_min, x_max = self.get_min_max_width(
                    cur_idx, tail_idx, tail_idx < num_fragments_on_fmem)
                while self.filter_idx + self.output_parallelism < self.num_filter_set:
                    if isinstance(self.main_op, ConvOp) and self.layer_info.parallel_type is None:
                        self.memory_manager.change_and_setup_wmem(
                            proc_cnt, last_flag)
                    self.run_module(x_min, x_max)
                    self.filter_idx += self.output_parallelism
                if isinstance(self.main_op, ConvOp) and self.layer_info.parallel_type is None:
                    self.memory_manager.change_and_setup_wmem(
                        proc_cnt, last_flag)
                load_cnt = 0
                for idx in range(cur_idx, tail_idx):
                    x_min, x_max = self.get_min_max_width(
                        idx, idx + 1, idx + 1 < num_fragments_on_fmem)
                    self.run_module(x_min, x_max)
                    if input_mapping[idx][2]:
                        fmem_idx, info, _ = input_mapping[num_fragments_on_fmem + load_cnt]
                        self.memory_manager.load_fmem(fmem_idx, info)
                        load_cnt += 1
                num_fragments_on_fmem += load_cnt
                cur_idx = tail_idx
                proc_cnt += 1

    def run_module(self, x_min, x_max):
        self.logger.debug(
            "run_module: clock = {}".format(stats.current_cycle()))
        self.memory_manager.mem_wait(-1, 0)  # To prevent reverse ordering
        if self.main_op.type == 'Gemm':
            self.main_module.set_gemm_configuration(
                x_min, x_max, self.filter_idx)
            self.work()
        elif isinstance(self.main_op, ConvPoolOpBase):
            self.convpool_module(x_min, x_max)
        else:
            raise MIDAPError(
                "Unknown layer or not implemented: {}".format(self.layer_info))

    def convpool_module(self, x_min, x_max):
        stride = self.main_op.stride
        for x in range(x_min, x_max + 1, stride):
            for y in range(self.y_min, self.y_max + 1, stride):
                self.main_module.set_convpool_configuration(
                    x, y, self.filter_idx)
                self.work()

    def process_arithmetic_layer(self):
        num_fragments_on_fmem = self.control_info.num_initial_fragments
        input_mapping = self.control_info.input_mapping
        action = self.control_info.action
        cur_idx = 0
        proc_cnt = 0
        if self.main_op.broadcast:
            self.memory_manager.change_and_setup_wmem(proc_cnt, True)
        for action_type, tail_idx in action:
            if action_type == 'LOAD':
                self.logger.info("process_arithmetic_layer: LOAD ({}, {})".format(
                    num_fragments_on_fmem, tail_idx))
                for idx in range(num_fragments_on_fmem, tail_idx):
                    stats.increase_cycle()
                    fmem_idx, info, _ = input_mapping[idx]
                    self.memory_manager.load_fmem(fmem_idx, info)
                num_fragments_on_fmem = tail_idx
            else:
                self.logger.info(
                    "process_arithmetic_layer: PROCESS ({}, {})".format(cur_idx, tail_idx))
                for idx in range(cur_idx, tail_idx):
                    x_min, x_max = self.get_min_max_width(idx, idx + 1, True)
                    if not self.main_op.broadcast:
                        check = self.memory_manager.load_filter_pivot + \
                            self.memory_manager.filter_group_offset
                        if check != x_min and check != -1:
                            self.logger.warning(
                                "Residual layer processing - incorrect plane loading {}(expected) vs {}".format(x_min, check))
                    for x in range(x_min, x_max + 1):
                        self.logger.debug(
                            "Processing yz plane idx: {}".format(x))
                        if not self.main_op.broadcast:
                            self.memory_manager.change_and_setup_wmem(
                                proc_cnt, True)
                        self.main_module.set_arithmetic_configuration(x)
                        self.work()
                    if input_mapping[idx][2]:
                        fmem_idx, info, _ = input_mapping[num_fragments_on_fmem]
                        self.memory_manager.load_fmem(fmem_idx, info)
                        num_fragments_on_fmem += 1
                cur_idx = tail_idx
                proc_cnt += 1

    def work(self):
        work = self.main_module.generate_dataflow()
        for _ in work:
            stats.increase_cycle()
            self.post_module.run(
                self.reduction_module.output_buf, self.reduction_module.output_info)
            self.reduction_module.run(
                self.main_module.output_buf, self.main_module.output_info)
            self.main_module.run()

    def finish_computation(self):
        while not self.post_module.end_flag:
            stats.increase_cycle()
            self.post_module.run(
                self.reduction_module.output_buf, self.reduction_module.output_info)
            self.reduction_module.run(
                self.main_module.output_buf, self.main_module.output_info)
            self.main_module.run()
