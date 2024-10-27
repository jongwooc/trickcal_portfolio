from __future__ import absolute_import, division, print_function, unicode_literals

import logging
from collections import deque

import numpy as np

from acc_utils.attrdict import AttrDict
from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

from . import statistics as stats
from .pipelined_component import PipelinedComponent


class PostModule(PipelinedComponent):
    def __init__(self, memory_manager):
        super(PostModule, self).__init__()
        self.name = 'post_module'
        self.sub_type = -1
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.num_fmem = cfg.MIDAP.FMEM.NUM
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.memory_manager = memory_manager
        self.save_buf = np.zeros(self.system_width)
        self.save_info = None
        self.obuf = np.zeros(self.system_width)
        self.write_buf = np.zeros(self.system_width)
        self.write_info = None
        self.write_type = 0
        self.abstract_write = False
        self.logger = logging.getLogger(cfg.LOGGER)
        # self.quantization = False

    def __del__(self):
        del self.memory_manager
        del self.save_buf, self.obuf, self.write_buf, self.logger

    def setup(self, layer_info):
        super(PostModule, self).setup()
        control_info = layer_info.control_info
        self.output_mapping = control_info.output_mapping
        self.concurrency = self.num_wmem if layer_info.parallel_type is None else self.system_width
        self.layer_name = layer_info.name
        self.output_name = layer_info.output_name
        self.offset = layer_info.offset
        self.input_shape = layer_info.main_op.output_tensor.shape
        self.output_shape = layer_info.output_tensor.shape
        sub_op = layer_info.sub_op
        self.sub_op = sub_op
        self.k_w, self.k_h = (1, 1)
        self.pad_t, self.pad_b, self.pad_l, self.pad_r = (0, 0, 0, 0)
        self.stride = 1
        if sub_op is None:
            self.sub_type = 0
        elif isinstance(sub_op, UpsampleOp):
            self.sub_type = 2 if sub_op.algorithm == 'Zero' else 1
            if self.sub_type == 2:
                self.logger.warning(
                    "'Zero' Type upsampling may not be available for the real hardware implementation.. please enable cfg.MODEL.ALLOW_ABSTRACT_DATA option. (-a)")
            self.k_w, self.k_h, self.pad_t, self.pad_b, self.pad_l, self.pad_r = sub_op.k_w, sub_op.k_h, sub_op.pad_t, sub_op.pad_b, sub_op.pad_l, sub_op.pad_r
            self.stride = sub_op.stride
        else:
            raise ValueError(
                "This write operation is not yet implemented: {}".format(self.sub_op))
        # Write property
        self.reverse_write = control_info.reverse_write
        self.flip = control_info.flip
        self.input_flip = control_info.input_flip
        self.write_on_dram = layer_info.write_on_dram
        self.write_type = 1 if self.offset[0] % self.concurrency != 0 or self.offset[
            1] % self.concurrency != 0 or self.offset[1] > self.output_shape[-1] else 0
        if self.write_type == 1:
            self.write_size = min(self.num_wmem, self.system_width)
            larger_size = max(self.num_wmem, self.system_width)
            self.logger.info("Write module splits the {} size main module output into {} size chunks due to the alignment issue.".format(
                larger_size, self.write_size))
        else:
            self.write_size = self.concurrency
        if self.output_name in self.memory_manager.dram_dict:
            self.logger.info(
                "Output data {0} is already referred. Please check layer {0} type is Concatenate.".format(self.output_name))
            self.output_data = self.memory_manager.dram_dict[self.output_name]
        else:
            self.output_data = np.zeros(self.output_shape)
            self.memory_manager.dram_dict[self.output_name] = self.output_data
        self.logger.info("Output data name: {}, shape: {}".format(
            self.output_name, self.output_shape))
        self.extra_output_info = layer_info.extra_output_info
        for info in self.extra_output_info:
            self.logger.info(
                "Extra output data name: {} (Write on DRAM)".format(info.name))
            if info.name not in self.memory_manager.dram_dict:
                self.memory_manager.dram_dict[info.name] = np.zeros(info.shape)
        self.end_flag = False
        self.write_info = None
        self.abstract_write = False
        if layer_info.require_fmem == 0:
            self.abstract_write = True
            self.logger.info(
                "PostModule: Main Operator {} does not have effective output - it may have reduction logic")
        self.patch_sequence = deque()

    def run(self, input_buf, input_info):
        self.write()
        self.work()
        if input_info is not None and input_info.last:
            while len(self.patch_sequence) > 0:
                stats.increase_cycle()
                self.write()
                self.work()
            self.update_queue(input_buf, input_info)

    def work(self):
        if len(self.patch_sequence) == 0:
            return None
        patch = self.patch_sequence.popleft()
        self.set_write(patch)

    def update_queue(self, input_buf, input_info):  # Override
        if self.sub_type == 0:
            self.obuf[:] = input_buf
            input_info.reset = False
            self.set_write(input_info)
        else:
            self.obuf[:] = input_buf
            self.save_info = input_info
            x, y = self.save_info.loc
            filter_idx = self.save_info.filter_idx
            reset = False
            order = reversed(
                range(self.k_w)) if self.input_flip else range(self.k_w)
            for kx in order:
                for ky in range(self.k_h):
                    patch = AttrDict({'loc': (
                        self.k_w * x + kx, self.k_h * y + ky), 'filter_idx': filter_idx, 'reset': reset})
                    self.patch_sequence.append(patch)
                    reset = self.sub_type == 2

    def get_fmem_info(self, x):
        fmem_idx = -1
        for idx, (head, tail) in self.output_mapping:
            if head <= x and x < tail:
                fmem_idx = idx
                effective_x = x - head
                return fmem_idx, effective_x
        return -1, -1

    def set_write(self, write_info):
        while self.write_info is not None:
            stats.increase_cycle()
            self.write()
        self.write_buf[:] = self.obuf
        self.write_info = write_info
        self.write_info.write_offset = 0

    def write(self):
        if self.write_info is None:
            return None
        x, y = self.write_info.loc
        out_w, out_h, out_c = self.output_shape
        if x >= out_w or y >= out_h:
            self.write_info = None
            return None
        if self.reverse_write:
            x = out_w - x - 1
        temp_x = out_w - x - 1 if self.flip else x
        fmem_idx, effective_x = self.get_fmem_info(x)
        reset = self.write_info.reset
        filter_idx = self.write_info.filter_idx
        write_offset = self.write_info.write_offset
        offset, max_z = self.offset
        head = offset + filter_idx
        tail = min(out_c, head + self.concurrency)
        if reset:
            self.write_buf[:] = np.zeros(self.write_buf.size)
        if write_offset == 0:
            self.output_data[x, y, head:tail] = self.write_buf[0:tail - head]
            for info in self.extra_output_info:
                extra_head = info.offset + filter_idx
                extra_tail = min(info.shape[-1], extra_head + self.concurrency)
                self.memory_manager.dram_dict[info.name][temp_x, y,
                                                         extra_head:extra_tail] = self.write_buf[0:tail - head]
                rel_addr = temp_x * \
                    info.shape[-1] * info.shape[-2] + \
                    y * info.shape[-1] + extra_head
                size = extra_tail - extra_head
                self.memory_manager.mem_write(rel_addr, size, info.name)
        data_size = tail - head
        rel_addr = x * out_h * out_c + y * out_c + head + write_offset
        if fmem_idx >= 0:
            address = effective_x * out_h * out_c + y * out_c + head + write_offset
            self.memory_manager.write_fmem(
                fmem_idx, address, self.write_buf[write_offset:write_offset + self.write_size])
            # print("Write fmem: data loc {}, (fmem_idx, effective_x, addr): ({}, {}, {})".format((x,y), fmem_idx, effective_x, address))
            if self.write_on_dram:
                self.memory_manager.mem_write(rel_addr, self.write_size)
            write_offset += self.write_size
        else:
            self.status = 0
            if not self.abstract_write:
                self.memory_manager.mem_write(rel_addr, tail - head)
            write_offset += (tail - head)
        if not self.write_on_dram and write_offset > data_size:
            raise ValueError("Weird write {}: {}".format(
                write_offset, data_size))
        elif write_offset >= data_size:  # Finish writing
            last_x = x == 0 if self.reverse_write else x == out_w - 1
            last_y = y == out_h - 1
            last_z = head + self.concurrency >= max_z
            self.end_flag = last_x and last_y and last_z
            self.last_process = (x, y, tail)
            self.write_info = None
        else:
            self.write_info.write_offset = write_offset
