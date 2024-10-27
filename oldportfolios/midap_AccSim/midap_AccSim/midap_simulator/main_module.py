from __future__ import absolute_import, division, print_function, unicode_literals

import copy

import numpy as np

from acc_utils.attrdict import AttrDict
from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

from . import statistics as stats
from .activator import Activator
from .adder import Adder
from .alu import ALU


class MainModule():
    def __init__(self, memory_manager):
        self.memory_manager = memory_manager
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.num_fmem = cfg.MIDAP.FMEM.NUM
        # Step 1: Load from the memory
        self.wbuf_mem = np.zeros((self.num_wmem, self.system_width))
        self.fbuf_mem = np.zeros(self.system_width)
        self.step1_info = None
        # Step 2: Fix Alignment & Broadcast
        self.wbuf = np.zeros((self.num_wmem, self.system_width))
        self.fbuf = np.zeros(self.system_width * 2)
        self.broadcast_fbuf = np.zeros(self.system_width)
        self.step2_info = None
        # Step 3, 4, 5: ALU - Adder - Activator
        self.output_buf = np.zeros(self.system_width)
        self.output_info = None
        self.alu = ALU()
        self.adder = Adder()
        self.activator = Activator(memory_manager)
        self.prev_name = ''
        self.start_time = -1
        self.end_time = -1
        self.gantt = False

    def __del__(self):
        del self.memory_manager
        del self.wbuf_mem, self.fbuf_mem, self.output_buf
        del self.alu, self.adder, self.activator

    def setup(self, layer_info):
        control_info = layer_info.control_info
        input_mapping = control_info.input_mapping
        self.main_op = layer_info.main_op
        self.input_mapping = input_mapping
        self.layer_info = layer_info
        self.shape = layer_info.input[0].output_tensor.shape
        if isinstance(self.main_op, PoolOp) and self.main_op.global_pooling == 1:
            self.main_op.k_w, self.main_op.k_h = self.shape[:-1]
        self.output_shape = layer_info.output_tensor.shape
        self.offset = layer_info.offset  # For debugging
        self.alu.setup(layer_info)
        self.adder.setup(layer_info)
        self.activator.setup(layer_info)
        self.load_info = None
        self.fragment_idx = 0
        self.dataflow_info = None
        self.load_buf_info = None
        self.save_buf_info = None
        self.num_gemm_rows = self.shape[-1] // self.system_width if self.main_op.type == 'Gemm' else 1
        self.wmem_last_load = -1
        self.output_loc = (-1, -1)

    def run(self):  # inverted pipeline
        self.activator.work(self.adder.output_buf, self.adder.output_info)
        self.output_buf, self.output_info = self.activator.output_buf, self.activator.output_info
        self.adder.work(self.alu.output_buf, self.alu.output_info)
        self.alu.work(self.broadcast_fbuf, self.wbuf, self.step2_info)
        self.broadcast_step()
        self.load_step()

    def generate_dataflow_info(
            self,
            loc=None,
            filter_idx=None,
            fmem_idx=0,
            fmem_row=-1,
            wmem_row=-1,
            broadcast_offset=0,
            delete_foffset=0,
            delete_boffset=0,
            reset=True,
            last=False,
            junk=False):
        self.dataflow_info = AttrDict(
            {
                'loc': loc,
                'filter_idx': filter_idx,
                'fmem_idx': fmem_idx,
                'fmem_row': fmem_row,
                'wmem_row': wmem_row,
                'broadcast_offset': broadcast_offset,
                'delete_foffset': delete_foffset,
                'delete_boffset': delete_boffset,
                'reset': reset,
                'last': last,
                'junk': junk
            })

    def set_convpool_configuration(self, x, y, filter_idx):
        if self.load_info is not None:
            raise MIDAPError(
                "previous step should be done before calling new set_conv_configuration: {}, {}".format(x, y))
        # print("processing (x,y) = ({}, {})".format(x,y))
        self.load_info = (x, y)
        xx, _ = (
            x, y) if self.layer_info.mapping_func is None else self.layer_info.mapping_func(x, y)
        for idx, (_, (head, tail), _) in enumerate(self.input_mapping):
            if head <= xx and xx < tail:
                self.fragment_idx = idx
        if x < 0:
            self.fragment_idx = 0
        self.filter_idx = filter_idx
        self.set_convpool_output_loc(x, y)

    def set_gemm_configuration(self, x_min, x_max, filter_idx):
        if self.load_info is not None:
            raise MIDAPError(
                "previous step should be done before calling new set_gemm_configuration: {}, {}".format(x_min, x_max))
        self.load_info = (x_min, x_max)
        self.filter_idx = filter_idx

    def set_arithmetic_configuration(self, idx):
        self.filter_idx = -1
        self.load_info = (idx, idx)

    def set_convpool_output_loc(self, x, y):
        self.load_info = (x, y)
        main_op = self.layer_info.main_op
        pivot_x, pivot_y = -main_op.pad_l, -main_op.pad_t
        out_w, out_h, _ = self.layer_info.main_op.output_tensor.shape
        if (x - pivot_x) % main_op.stride != 0 or (y - pivot_y) % main_op.stride != 0:
            raise MIDAPError(
                "Wrong conv location: ({}, {}) for main_op : {}".format(x, y, main_op))
        out_x, out_y = ((x - pivot_x) // main_op.stride,
                        (y - pivot_y) // main_op.stride)
        if out_x < 0 or out_y < 0 or out_x >= out_w or out_y >= out_h:
            raise MIDAPError(
                "Incorrect output location computation: {}".format((x, y, out_x, out_y)))
        self.output_loc = (out_x, out_y)

    def generate_dataflow(self):
        if self.load_info is None:
            while True:
                yield None
        else:
            main_op = self.layer_info.main_op
            op_type = main_op.type
            worker = None
            if op_type == 'Gemm':
                worker = self.gemm_worker()
            elif op_type in ['AveragePool', 'MaxPool', 'Depthwise']:
                worker = self.depthwise_worker()
            elif isinstance(main_op, ConvOp):
                worker = self.conv_yz_worker(
                ) if self.layer_info.mapping_func is None else self.conv_z_worker()
            elif isinstance(main_op, ArithmeticOp):
                worker = self.arithmetic_worker()
            if worker is None:
                raise MIDAPError("Unknown layer : {}".format(self.layer_info))
            for _ in worker:
                fmem_idx, fmem_row = self.dataflow_info.fmem_idx, self.dataflow_info.fmem_row
                self.save_buf_info = copy.copy(self.load_buf_info)
                self.load_buf_info = (fmem_idx, fmem_row)
                yield None
            self.dataflow_info = None
            self.load_info = None

    def get_fmem_info(self, x):
        input_mapping = self.input_mapping
        for fmem_idx, (head, tail), _ in input_mapping[self.fragment_idx:]:
            if head <= x and x < tail:
                return fmem_idx, x - head

    def gemm_worker(self):
        out_w, out_h, _ = self.output_shape
        min_x, max_x = self.load_info
        for x in range(min_x, max_x + 1):
            fmem_idx, effective_x = self.get_fmem_info(x)
            w = x // out_h
            h = x % out_h
            reset = True
            for row_offset in range(self.num_gemm_rows):
                last = row_offset == self.num_gemm_rows - 1
                row = effective_x * self.num_gemm_rows + row_offset
                self.generate_dataflow_info(
                    loc=(w, h),
                    filter_idx=self.filter_idx,
                    fmem_idx=fmem_idx,
                    fmem_row=row,
                    wmem_row=row_offset,
                    reset=reset,
                    last=last
                )
                yield None
                reset = False

    def depthwise_worker(self):
        layer_info = self.layer_info
        mapping_func = layer_info.mapping_func
        valid_func = layer_info.valid_func
        dilation = layer_info.dilation
        main_op = layer_info.main_op
        load_weight = not isinstance(main_op, PoolOp)
        k_h, k_w = main_op.k_h, main_op.k_w
        base_x, base_y = self.load_info
        in_w, in_h, in_c = self.shape
        row = self.filter_idx // self.system_width
        yz_plane_size = in_h * in_c
        in_w *= layer_info.scale_w
        in_h *= layer_info.scale_h
        reset = True

        # duseok
        if self.prev_name != self.layer_info.name:
            if self.gantt and self.start_time != -1:
                print("CIM", self.start_time, self.end_time,
                      self.prev_name.replace('_', '\\\\_'), file=sys.stderr)
            self.start_time = stats.total_cycle()
            self.prev_name = self.layer_info.name
        self.end_time = stats.total_cycle()

        for kx in range(k_w):
            x = base_x + kx * dilation
            if x < 0 or x >= in_w:
                continue
            for ky in range(k_h):
                y = base_y + ky * dilation
                valid_x = in_w - x - 1 if layer_info.control_info.input_flip else x
                if y < 0 or y >= in_h:
                    continue
                if not valid_func(valid_x, y):
                    if not cfg.MIDAP.EFFICENT_LOGIC:
                        stats.increase_cycle()
                    continue
                mapped_x, mapped_y = mapping_func(x, y)
                fmem_idx, effective_x = self.get_fmem_info(mapped_x)
                # Error Checking
                fmem_row = (effective_x * yz_plane_size +
                            mapped_y * in_c) // self.system_width + row
                wmem_row = -1
                if load_weight:
                    wmem_row = (kx * k_h * in_c + ky *
                                in_c) // self.system_width + row
                if not reset:
                    yield None
                self.generate_dataflow_info(
                    loc=self.output_loc,
                    filter_idx=self.filter_idx,
                    fmem_idx=fmem_idx,
                    fmem_row=fmem_row,
                    wmem_row=wmem_row,
                    reset=reset,
                )
                reset = False
        self.dataflow_info.last = True
        yield None

    def conv_z_worker(self):
        layer_info = self.layer_info
        mapping_func = layer_info.mapping_func
        valid_func = layer_info.valid_func
        dilation = layer_info.dilation
        main_op = layer_info.main_op
        k_h, k_w = main_op.k_h, main_op.k_w
        base_x, base_y = self.load_info
        in_w, in_h, in_c = self.shape
        row_per_channel = in_c // self.system_width
        yz_plane_rows = in_h * row_per_channel
        in_w *= layer_info.scale_w
        in_h *= layer_info.scale_h

        if self.prev_name != self.layer_info.name:
            if self.gantt and self.start_time != -1:
                print("CIM", self.start_time, self.end_time,
                      self.prev_name.replace('_', '\\\\_'), file=sys.stderr)
            self.start_time = stats.total_cycle()
            self.prev_name = self.layer_info.name
            # self.prev_latency = 0
        self.end_time = stats.total_cycle()

        reset = True
        for kx in range(k_w):
            x = base_x + kx * dilation
            if x < 0 or x >= in_w:
                continue
            for ky in range(k_h):
                y = base_y + ky * dilation
                valid_x = in_w - x - 1 if layer_info.control_info.input_flip else x
                if y < 0 or y >= in_h:
                    continue
                if not valid_func(valid_x, y):
                    if not cfg.MIDAP.EFFICENT_LOGIC:
                        stats.increase_cycle()
                    continue
                mapped_x, mapped_y = mapping_func(x, y)
                fmem_idx, effective_x = self.get_fmem_info(mapped_x)
                # Error Checking
                fmem_start_row = effective_x * yz_plane_rows + mapped_y * row_per_channel
                wmem_start_row = kx * k_h * row_per_channel + ky * row_per_channel
                # last_y = (y == in_h - 1) or (ky == k_h - 1)
                # last_x = (x == in_w - 1) or (kx == k_w - 1)
                for row in range(row_per_channel):
                    # last = last_x and last_y and row == row_per_channel - 1
                    if not reset:
                        yield None
                    self.generate_dataflow_info(
                        loc=self.output_loc,
                        filter_idx=self.filter_idx,
                        fmem_idx=fmem_idx,
                        fmem_row=fmem_start_row + row,
                        wmem_row=wmem_start_row + row,
                        reset=reset,
                        # last=last
                    )
                    reset = False
        self.dataflow_info.last = True
        yield None

    def conv_yz_worker(self):
        main_op = self.layer_info.main_op
        k_h, k_w = main_op.k_h, main_op.k_w
        base_x, base_y = self.load_info
        in_w, in_h, in_c = self.shape
        row_per_kernel_yz = div_ceil(in_c * k_h, self.system_width)
        yz_plane_size = in_h * in_c

        # duseok
        if self.prev_name != self.layer_info.name:
            if self.gantt and self.start_time != -1:
                print("CIM", self.start_time, self.end_time,
                      self.prev_name.replace('_', '\\\\_'), file=sys.stderr)
            self.start_time = stats.total_cycle()
            self.prev_name = self.layer_info.name
            # self.prev_latency = 0
        self.end_time = stats.total_cycle()

        # Target bank
        reset = True
        for kx in range(k_w):
            x = base_x + kx
            if x < 0 or x >= in_w:
                continue
            last_x = (x == in_w - 1) or (kx == k_w - 1)
            fmem_idx, effective_x = self.get_fmem_info(x)
            # wmem configuration
            start_ky = max(0, -base_y)
            end_ky = min(k_h, in_h - base_y)
            wmem_start_row = kx * row_per_kernel_yz
            wmem_start_row += start_ky * in_c // self.system_width  # when pad skipped
            # For skip operation
            wmem_offset = (start_ky * in_c) % self.system_width
            # assume that Dilation == 1
            fmem_start_address = effective_x * \
                yz_plane_size + (base_y + start_ky) * in_c
            fmem_offset = fmem_start_address % self.system_width
            fmem_start_row = fmem_start_address // self.system_width
            fmem_last_row = div_ceil(
                effective_x * yz_plane_size + (base_y + end_ky) * in_c, self.system_width) - 1
            bubble = wmem_offset - fmem_offset
            num_rows = div_ceil(end_ky * in_c, self.system_width) - \
                (start_ky * in_c // self.system_width)
            self.save_last_run = (fmem_start_row, wmem_start_row, num_rows, fmem_offset,
                                  wmem_offset, end_ky, k_h, row_per_kernel_yz, fmem_last_row, 0)
            # if end_ky < k_h and kx == 0 and k_h == 5:
            #    print("For loc {}: fmem_start_row, wmem_start_row, num_rows, fmem_offset, wmem_offset = {}".format(self.load_info, self.save_last_run))
            for row in range(num_rows):
                if bubble < 0:
                    if self.load_buf_info != (fmem_idx, fmem_start_row):
                        self.generate_dataflow_info(
                            loc=self.output_loc,
                            filter_idx=self.filter_idx,
                            fmem_idx=fmem_idx,
                            fmem_row=fmem_start_row,
                            reset=reset,
                            junk=True
                        )
                        yield None
                    bubble = self.system_width + bubble
                    fmem_start_row += 1
                fmem_row = fmem_last_row if fmem_last_row < fmem_start_row + \
                    row else fmem_start_row + row
                wmem_row = wmem_start_row + row
                cnt = 0
                last = False
                if row == num_rows - 1:
                    cnt = (end_ky * in_c) % self.system_width
                    last = last_x
                self.generate_dataflow_info(
                    loc=self.output_loc,
                    filter_idx=self.filter_idx,
                    fmem_idx=fmem_idx,
                    fmem_row=fmem_row,
                    wmem_row=wmem_row,
                    broadcast_offset=bubble,
                    delete_foffset=wmem_offset,
                    delete_boffset=cnt,
                    reset=reset,
                    last=last,
                )
                yield None
                wmem_offset = 0
                reset = False

    def arithmetic_worker(self):
        main_op = self.layer_info.main_op
        _, in_h, in_c = self.shape
        x, _ = self.load_info
        fmem_idx, effective_x = self.get_fmem_info(x)
        fmem_start_row = effective_x * in_h * in_c // self.system_width
        num_rows = in_h * in_c // self.system_width
        row_per_channel = in_c // self.system_width
        for row in range(num_rows):
            y = row // row_per_channel
            z = (row % row_per_channel) * self.system_width
            wmem_row = row if not main_op.broadcast else row % row_per_channel
            self.generate_dataflow_info(
                loc=(x, y),
                filter_idx=z,
                reset=True,
                last=True,
                fmem_idx=fmem_idx,
                fmem_row=fmem_start_row + row,
                wmem_row=wmem_row
            )
            yield None

    def print(self, print_str, condition=False):
        if condition:
            print(print_str)

    def load_step(self):
        if self.dataflow_info is not None:
            fmem_row = self.dataflow_info.fmem_row
            fmem_idx = self.dataflow_info.fmem_idx
            wmem_row = self.dataflow_info.wmem_row
            if fmem_row > -1:
                self.memory_manager.load_fbuf(
                    self.fbuf_mem, fmem_idx, fmem_row)
            if wmem_row > -1 and self.wmem_last_load != (self.filter_idx, wmem_row):
                self.memory_manager.load_wbuf(self.wbuf_mem, wmem_row)
                self.wmem_last_load = (self.filter_idx, wmem_row)
        self.step1_info = copy.copy(self.dataflow_info)

    def broadcast_step(self):
        if self.step1_info is not None:
            info = self.step1_info
            delete_foffset = info.delete_foffset
            delete_boffset = info.delete_boffset
            broadcast_offset = info.broadcast_offset
            self.shift()
            self.set_broadcast_fbuf(
                broadcast_offset, delete_foffset, delete_boffset)
            self.wbuf[:, :] = self.wbuf_mem

        self.step2_info = copy.copy(self.step1_info)

    def shift(self):
        self.fbuf[self.system_width:] = self.fbuf[:self.system_width]
        self.fbuf[0:self.system_width] = self.fbuf_mem[:]

    def set_broadcast_fbuf(self, offset=0, delete_f=0, delete_b=0):
        if offset > 0:
            self.broadcast_fbuf[:offset] = self.fbuf[-offset:]
            self.broadcast_fbuf[offset:] = self.fbuf[:self.system_width - offset]
        else:
            self.broadcast_fbuf[:] = self.fbuf[:self.system_width]
        if delete_b > 0:
            self.broadcast_fbuf[delete_b:] = np.zeros(
                self.system_width - delete_b)
        if delete_f > 0:
            self.broadcast_fbuf[:delete_f] = np.zeros(delete_f)
