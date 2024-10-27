from __future__ import absolute_import, division, print_function, unicode_literals

import math
from collections import OrderedDict

import numpy as np

from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

from . import statistics as stats


class MemoryManager():
    def __init__(self):
        self.dram_dict = {}
        self.dram_latency = cfg.LATENCY.DRAM_READ
        self.dram_page_size = cfg.SYSTEM.DRAM_PAGE_SIZE
        self.dram_bandwidth = (cfg.SYSTEM.BANDWIDTH * 1000 //
                               cfg.SYSTEM.FREQUENCY) // cfg.SYSTEM.DATA_SIZE
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.busy_timer = 0
        # FMEM setting
        self.input_name = ""
        self.fmem_size = cfg.MIDAP.FMEM.NUM_ENTRIES
        self.num_fmem = cfg.MIDAP.FMEM.NUM
        self.fmem = np.zeros([self.num_fmem, self.fmem_size])
        # input = 0, output = 1, else = -1
        self.fmem_state = [-1 for _ in range(self.num_fmem)]
        self.fmem_valid_timer = [0 for _ in range(self.num_fmem)]
        # WMEM setting - Do not aware WMEM setting
        self.filter = None
        self.filter_type = 0  # 0 : normal / 1: depthwise / 2: tensor
        # normal - load num_wmem filters at once
        # depthwise - load 1 filter at once
        # tensor - load wmem_size
        self.num_filter_set = 0
        self.load_filter_pivot = 0
        self.current_filter_pivot = -1
        self.load_filter_once = False
        self.load_filter_finish = False
        self.filter_size = 0
        self.prepare_layer = None
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.wmem_size = cfg.MIDAP.WMEM.NUM_ENTRIES
        # double buffered WMEM
        self.wmem = np.zeros([2, self.num_wmem, self.wmem_size])
        self.wmem_valid_timer = [0, 0]
        self.wmem_in_use = -1
        # BMMEM setting
        self.bmmem_size = cfg.MIDAP.BMMEM.NUM_ENTRIES
        self.bmmem = np.zeros([2, self.bmmem_size])
        self.bmmem_valid_timer = [0, 0]
        self.bmmem_in_use = -1
        # Set config.py __C.ACTIVATE_DEBUG_MODE = True to show debug msg
        self.debug = cfg.ACTIVATE_DEBUG_MODE

        self.bus_policy = cfg.MIDAP.BUS_POLICY
        self.write_on_dram = False
        self.group_size = 1

        self.dram_addr_dict = OrderedDict()
        self.memory_footprint = []

    def __del__(self):
        del self.dram_dict
        del self.fmem, self.wmem, self.bmmem
        del self.fmem_state, self.fmem_valid_timer
        del self.wmem_valid_timer, self.bmmem_valid_timer
        del self.memory_footprint, self.dram_addr_dict
        del self.filter

    def add_dram_info(self, name, tensor):
        self.dram_dict[name] = tensor
        self.register_dram_addr(name, tensor.size)

    def setup(self, layer_info):
        self.layer_name = layer_info.name
        control_info = layer_info.control_info
        main_op = layer_info.main_op
        self.write_on_dram = layer_info.write_on_dram
        self.load_filter_pivot = 0
        self.num_filter_set = 0
        self.filter_type = 0
        # self.input_name = layer_info.input[0].name
        self.filter_name = None
        self.bias_name = None
        self.dram_setup(layer_info)
        # Debugging information
        # input = 0, output = 1, else = -1
        self.fmem_state = [-1 for _ in range(self.num_fmem)]
        for fmem_idx, _, _ in control_info.input_mapping:
            self.fmem_state[fmem_idx] = 0
        # End
        self.reverse_load = False
        self.load_filter_once = False
        self.load_filter_finish = False
        if isinstance(main_op, ConvOp):
            self.filter = main_op.weight
            self.num_filter_set = self.filter.shape[0]
            self.filter = self.filter.reshape(self.filter.shape[0], -1)
            self.load_filter_once = layer_info.load_filter_once
            self.load_filter_finish = False
            if main_op.type == 'Depthwise':
                self.filter_type = 1
            if main_op.bias is not None:
                self.bmmem_in_use += 1
        elif isinstance(main_op, ArithmeticOp):
            control_info = layer_info.control_info
            wmem_input_layer = layer_info.input[1]
            self.filter = self.dram_dict[wmem_input_layer.name]
            self.filter = self.filter.reshape(self.filter.shape[0], -1)
            self.num_filter_set = self.filter.shape[0]
            input_flip = control_info.input_flip
            self.reverse_load = input_flip != wmem_input_layer.control_info.flip
            self.filter_type = 2
        else:
            self.filter_type = -1
            self.filter = None
            return
        filter_set_size = self.num_wmem
        filter_size = 0 if self.filter is None else self.filter[0, :].size
        if filter_size % self.system_width != 0:
            raise ValueError("Filter size is not padded as well")
        if self.filter_type in [1, 2]:
            filter_set_size = 1
        self.filter_set_size = filter_set_size
        self.filter_size = filter_size
        self.filter_group = control_info.filter_group
        self.filter_group_offset = -1
        self.remain_flag = False
        if self.num_filter_set % self.filter_set_size > 0:
            raise ValueError("Channel Size is not padded as well")
        self.num_filter_set = self.num_filter_set // self.filter_set_size
        if self.filter is not None and layer_info != self.prepare_layer:
            self.wmem_in_use = -1
            self.filter_group_offset = -1
            self.setup_wmem(0, 0)
            if isinstance(main_op, ConvOp) and main_op.bias is not None:
                # META info should be added
                self.setup_bmmem(self.bmmem_in_use % 2, main_op.bias)
        if control_info.prepare is not None:
            self.prepare_layer = control_info.prepare
            if isinstance(self.prepare_layer.main_op, ConvOp) and self.prepare_layer.main_op.bias is not None:
                self.setup_bmmem((self.bmmem_in_use + 1) %
                                 2, self.prepare_layer.main_op.bias)
        else:
            self.prepare_layer = None

    def dram_setup(self, layer_info, current_layer=True):
        input_layer = layer_info.input[0]
        input_name = input_layer.name
        output_name = layer_info.output_name
        filter_name = ''
        bias_name = ''
        self.register_dram_addr(input_name, input_layer.output_tensor.size)
        self.register_dram_addr(output_name, layer_info.output_tensor.size)
        for info in layer_info.extra_output_info:
            size = np.zeros(info.shape).size
            self.register_dram_addr(info.name, size)
        main_op = layer_info.main_op
        if isinstance(main_op, ConvOp):
            if main_op.bias is not None:
                bias_name = layer_info.name + "_bias"
                self.register_dram_addr(bias_name, main_op.bias.size)
            filter_name = layer_info.name + "_weight"
            self.register_dram_addr(filter_name, main_op.weight.size)
        if isinstance(main_op, ArithmeticOp):
            filter_name = layer_info.input[1].name
        if current_layer:
            self.input_name = input_name
            self.output_name = output_name
            self.filter_name = filter_name
            self.bias_name = bias_name

    """ WMEM related functions """

    def change_and_setup_wmem(self, proc_cnt, last_use=False):
        self.filter_group_offset += 1
        if self.filter_group_offset % self.group_size == 0:
            self.filter_group_offset = 0
            self.group_size = self.next_group_size
            self.current_filter_pivot = self.load_filter_pivot
            self.load_filter_pivot += self.group_size
            if not self.remain_flag:
                self.wmem_in_use += 1
#                print("change: cfp, lfp, wmem_in_use = {}, {}, {}".format(self.current_filter_pivot, self.load_filter_pivot, self.wmem_in_use))
            else:
                pass
#                print("remain: cfp, lfp, nfs, wmem_in_use = {}, {}, {}, {}".format(self.current_filter_pivot, self.load_filter_pivot, self.num_filter_set, self.wmem_in_use))
            next_idx = (self.wmem_in_use + 1) % 2
            self.setup_wmem(next_idx, proc_cnt, last_use)
        # print("filter pivot, offset, group_size: {}, {}, {}".format(self.load_filter_pivot, self.filter_group_offset, self.group_size))
        # print("Cur idx = {}, load filter = {}, preparing = {}".format(self.wmem_in_use % 2, self.load_filter_pivot, last_filter and last_use))

    def get_dram_latency(self, size):
        return self.dram_latency * math.ceil(size / self.dram_page_size)

    def setup_wmem(self, wmem_idx, proc_cnt, last_use=False):
        filter_size = self.filter_size
        filter_set_size = self.filter_set_size
        last_filter = self.num_filter_set == self.load_filter_pivot
        if self.load_filter_pivot > self.num_filter_set:
            raise ValueError("Weird load_filter_pivot - error checking")
        last_load = last_filter and last_use
        # print("last filter, last use: {}, {}".format(last_filter, last_use))
        wmem_remain_flag = False
        filter_name = self.filter_name
        if last_filter:
            self.load_filter_pivot = 0
        load_idx = self.load_filter_pivot
        if not last_load:
            # Update group size to current status
            if self.load_filter_once:
                if last_filter:
                    self.load_filter_finish = True
                    wmem_remain_flag = div_ceil(self.num_filter_set, self.filter_group[proc_cnt]) % 2 != 0
            elif last_filter:
                proc_cnt += 1
            next_group_size = min(self.num_filter_set - self.load_filter_pivot, self.filter_group[proc_cnt])
            wmem_remain_flag = wmem_remain_flag or self.num_filter_set <= next_group_size and self.filter_type in [0, 1]
            if self.load_filter_finish or wmem_remain_flag:
                pass
            elif self.filter_type in [0, 1]:
                wmem_pivot = 0 if not self.load_filter_once else self.filter_group[0] * (self.load_filter_pivot // (2 * self.filter_group[0]))
                for i in range(next_group_size):
                    self.wmem[wmem_idx, 0:filter_set_size, (wmem_pivot + i) * self.filter_size: (wmem_pivot + i + 1) * self.filter_size] = \
                        self.filter[(self.load_filter_pivot + i) * filter_set_size: (self.load_filter_pivot + i + 1) * filter_set_size, :]
            elif self.filter_type == 2:
                load_filter_pivot = self.num_filter_set - self.load_filter_pivot - next_group_size if self.reverse_load else self.load_filter_pivot
                self.wmem[wmem_idx, 0, 0:filter_size * next_group_size] = self.filter[load_filter_pivot:load_filter_pivot + next_group_size, :].reshape(-1)
        elif self.prepare_layer is not None:
            if self.load_filter_pivot > 0:
                raise ValueError("it might be not the last load")
            self.load_filter_finish = False
            self.dram_setup(self.prepare_layer, False)
            main_op = self.prepare_layer.main_op
            next_group_size = self.prepare_layer.control_info.filter_group[0]
            if isinstance(main_op, ArithmeticOp):
                reverse_load = self.prepare_layer.control_info.input_flip != self.prepare_layer.input[
                    1].control_info.flip
                tensor = self.dram_dict[self.prepare_layer.input[1].name]
                tensor = tensor.reshape(tensor.shape[0], -1)
                filter_size = tensor[0].size
                filter_set_size = 1
                load_idx = 0 if not reverse_load else tensor.shape[0] - \
                    next_group_size
                self.wmem[wmem_idx, 0, 0:filter_size *
                          next_group_size] = tensor[load_idx:load_idx + next_group_size, :].reshape(-1)
            else:
                load_idx = 0
                tensor = main_op.weight
                tensor = tensor.reshape(tensor.shape[0], -1)
                filter_size = tensor[0].size
                if main_op.type == 'Depthwise':
                    next_group_size = 1
                    filter_set_size = 1
                else:
                    filter_set_size = self.num_wmem
                self.wmem[wmem_idx, 0:filter_set_size,
                          0:filter_size] = tensor[0:filter_set_size, 0:filter_size]
                for i in range(next_group_size):
                    self.wmem[wmem_idx, 0:filter_set_size, i * filter_size:(i + 1) * filter_size] = \
                        tensor[i *
                               filter_set_size: (i + 1) * filter_set_size, :]
        else:
            wmem_remain_flag = True
            next_group_size = 0
        self.next_group_size = next_group_size
        self.remain_flag = wmem_remain_flag
        if wmem_remain_flag or self.load_filter_finish:
            return
        load_address = filter_set_size * filter_size * load_idx
        total_load = filter_set_size * filter_size * next_group_size
        self.mem_req(wmem_idx, filter_name, load_address, total_load)
        stats.wmem_write(div_ceil(total_load, max(
            self.dram_bandwidth, self.system_width)))
        # load_start_time = max(self.busy_timer, stats.total_cycle())
        if self.bus_policy == 'WMEM_FIRST':
            # FIXME higher wmem's priority. need to clean and modularize this code.
            load_start_time = stats.total_cycle() + self.get_dram_latency(total_load)
            wmem_load_time = div_ceil(total_load, self.dram_bandwidth)
            wmem_load_end_time = load_start_time + wmem_load_time
            for i, f_valid in enumerate(self.fmem_valid_timer):
                if f_valid > load_start_time:
                    self.fmem_valid_timer[i] += wmem_load_time
            for i, bm_valid in enumerate(self.bmmem_valid_timer):
                if bm_valid > load_start_time:
                    self.bmmem_valid_timer[i] += wmem_load_time
            self.wmem_valid_timer[wmem_idx] = wmem_load_end_time
            self.busy_timer += max(load_start_time - self.busy_timer, 0) + wmem_load_time
        elif self.bus_policy == 'FIFO':
            load_start_time = max(self.busy_timer, stats.total_cycle() + self.get_dram_latency(total_load))
            self.busy_timer = load_start_time + div_ceil(total_load, self.dram_bandwidth)
            self.wmem_valid_timer[wmem_idx] = self.busy_timer

    def load_wbuf(self, wbuf, row):
        if self.load_filter_once:
            pivot = (self.current_filter_pivot // (self.filter_group[0] * 2)) * self.filter_group[0] + self.filter_group_offset
        else:
            pivot = self.group_size - self.filter_group_offset - 1 if self.reverse_load else self.filter_group_offset
        address = self.system_width * row + pivot * self.filter_size
        cur_idx = self.wmem_in_use % 2
        time_gap = self.wmem_valid_timer[cur_idx] - stats.total_cycle()
        if self.wmem_valid_timer[cur_idx] > 0:
            time_gap = max(0, time_gap)
            self.mem_wait(cur_idx, time_gap)
        if time_gap > 0:
            stats.wait_memory(time_gap)
            stats.wait_dram2wmem(time_gap)
        self.wmem_valid_timer[cur_idx] = -1
        wbuf[:self.filter_set_size, :] = self.wmem[cur_idx,
                                                   :self.filter_set_size, address:address + self.system_width]
        stats.wmem_read(self.filter_set_size)

    def dump_wmem(self, write_file):  # only filter on WMEM (including double-buffered items)
        # 1024KB
        wmem = np.zeros([32, 16, 2 * 1024]).astype(np.int8)
        num_filter = min(32, self.filter.shape[0])
        for i in range(num_filter):
            wmem[i, :, :self.filter[0, 0].size] = self.filter[i].reshape(
                self.num_wmem, -1)
        write_file.write(wmem.reshape(-1).tobytes())

    """ FMEM related functions """

    def load_fmem(self, fmem_idx, info):
        # print("Call: fmem_idx {}, info: {}, time: {}".format(fmem_idx, info, stats.current_cycle()))
        # debug info
        if self.fmem_state[fmem_idx] == 1:
            raise ValueError("Invalid FMEM load - this bank is already in use")
        if self.fmem_state[fmem_idx] != 0:
            self.fmem_state[fmem_idx] = 0
            self.print(
                "FMEM State change notification: {}".format(self.fmem_state))
        # end
        data = self.dram_dict[self.input_name]
        fmem_data = data[info[0]:info[1], :, :]
        read = div_ceil(fmem_data.size, self.system_width) * self.system_width
        # print("Call: fmem_idx {}, info: {}, time: {}, size: {}".format(fmem_idx, info, stats.current_cycle(), fmem_data.size))
        load_address = data[0].size * info[0]
        # stats.dram_read(read)
        self.mem_req(2 + fmem_idx, self.input_name, load_address, read)
        last_cycle = stats.total_cycle()
        load_start_time = max(last_cycle + self.get_dram_latency(read), self.busy_timer)
        self.busy_timer = load_start_time + div_ceil(read, self.dram_bandwidth)  # multiply? add?
        self.fmem_valid_timer[fmem_idx] = self.busy_timer
        fmem_data = fmem_data.reshape(-1)
        if fmem_data.size > self.fmem_size:
            raise MIDAPError("MIDAP FMEM SIZE OVERFLOW ERROR!")
        self.fmem[fmem_idx][:fmem_data.size] = fmem_data
        stats.fmem_write(
            div_ceil(read, max(self.system_width, self.dram_bandwidth)))

    def load_fbuf(self, fbuf, bank_idx, row):
        # debug info
        if self.fmem_state[bank_idx] != 0:
            raise ValueError(
                "Invalid FMEM read - this bank is invalid for reading")
        # end
        address = self.system_width * row
        time_gap = self.fmem_valid_timer[bank_idx] - stats.total_cycle()
        if self.fmem_valid_timer[bank_idx] > 0:
            time_gap = max(0, time_gap)
            self.mem_wait(bank_idx + 2, time_gap)
        if time_gap > 0:
            stats.wait_memory(time_gap)
            stats.wait_dram2fmem(time_gap)
        self.fmem_valid_timer[bank_idx] = -1
        fbuf[0:self.system_width] = self.fmem[bank_idx][address:address +
                                                        self.system_width]
        stats.fmem_read()

    def write_fmem(self, bank_idx, address, data):
        # set debug info
        if self.fmem_state[bank_idx] != 1:
            self.fmem_state[bank_idx] = 1
            self.print(
                "FMEM State change notification: {}".format(self.fmem_state))
        # end
        time_gap = self.fmem_valid_timer[bank_idx] - stats.total_cycle()
        if self.fmem_valid_timer[bank_idx] > 0:
            time_gap = max(0, time_gap)
            self.mem_wait(bank_idx + 2, time_gap)
        if time_gap > 0:
            stats.wait_memory(time_gap)
            stats.wait_fmem2dram(time_gap)
        # Warning
        if address % data.size != 0:
            print("Warning: write data alignment")
        data_size = data.size
        stats.fmem_write()
        self.fmem[bank_idx][address:address + data_size] = data

    def dump_fmem(self, write_file):  # only main FMEM
        write_file.write(self.fmem.tobytes())

    def write_dram(self, data_size):  # DRAM Write
        stats.dram_write(data_size)
        if cfg.LATENCY.INCLUDE_DRAM_WRITE:
            write_start_time = max(
                self.busy_timer, stats.total_cycle() + self.get_dram_latency(data_size))
            self.busy_timer = write_start_time + \
                div_ceil(data_size, self.dram_bandwidth)

    """ BMMEM related """

    def setup_bmmem(self, bmmem_idx, bias):
        load_start_time = max(stats.total_cycle() +
                              self.get_dram_latency(bias.size), self.busy_timer)
        self.busy_timer = load_start_time + \
            div_ceil(bias.size, self.dram_bandwidth)
        self.bmmem_valid_timer[bmmem_idx] = self.busy_timer
        self.bmmem[bmmem_idx, :bias.size] = bias

    def load_bbuf(self, bbuf, address, size):
        bmmem_in_use = self.bmmem_in_use % 2
        time_gap = self.bmmem_valid_timer[bmmem_in_use] - stats.total_cycle()
        if time_gap > 0:
            stats.wait_memory(time_gap)
        bbuf[:size] = self.bmmem[bmmem_in_use, address:address + size]

    def print(self, msg):
        if self.debug:
            print("MemoryManager: " + msg)

    # mem_id: 0, 1: WMEM / 2 ~ N+1 : FMEM
############################### FOR DMA Level Footprint generation (should be ignored for simulation)###############################
    def register_dram_addr(self, name, size):
        if name in self.dram_addr_dict:
            return
        start_addr = sum(self.dram_addr_dict[list(
            self.dram_addr_dict.keys())[-1]]) if len(self.dram_addr_dict) > 0 else 0
        self.dram_addr_dict[name] = (start_addr, size)

    def mem_req(self, mem_id, name, rel_addr, size):
        # import sys
        stats.dram_read(size)
        abs_addr = self.dram_addr_dict[name][0] + rel_addr
        time = stats.total_cycle()
        self.memory_footprint.append(('REQUEST', [time, mem_id, abs_addr, size]))

    def mem_wait(self, mem_id, time_gap):  # Equal to SYNC
        # import sys
        time = stats.total_cycle()
        # print(self.layer_name, 'Wait', [time, mem_id, time_gap], file=sys.stderr)
        self.memory_footprint.append(('WAIT', [time, mem_id, time_gap]))

    def mem_write(self, rel_addr, size, name=None):
        # import sys
        self.write_dram(size)
        if name is None:
            name = self.output_name
        abs_addr = self.dram_addr_dict[name][0] + rel_addr
        time = stats.total_cycle()
        # print(self.layer_name, 'Write', [time, -1, rel_addr, size], file=sys.stderr)
        self.memory_footprint.append(('WRITE', [time, abs_addr, size]))

    def convert_memory_footprint_to_dma_operation(self, request_size=64):
        # DMA Operation: 'REQUEST' & 'Wait for RECEIVE'
        # In this implementation, 'WAIT' is used for [Computation time]
        # Between 'WAIT': Computation time
        # It follows FIFO scheme for read operation
        # And async Write operation, but it preempts current read operation
        dma_operation = []
        self.dma_operation = dma_operation
        # DMA Operation: ['READ'/'WRITE', $REL_TIME, $ADDR, $SIZE] or ['START_COMP'/'END_COMP', $COMP_TIME]
        request_dict = OrderedDict()
        write_queue = []
        wait_start_timer = 0
        estimate_respond_time = request_size / self.dram_bandwidth  # Virtual value
        dma_operation.append(('START_COMP', 0))
        for tp, value in self.memory_footprint:
            if tp == 'REQUEST':
                time, mem_id, abs_addr, size = value
                if mem_id in request_dict:
                    raise ValueError("Request for mem_id {} already exists, {} vs {}".format(
                        mem_id, request_dict[mem_id], [time, abs_addr, size]))
                # request_dict[mem_id] = [time, abs_addr, size]
                request_dict[mem_id] = [abs_addr, size]
                # Since 'REQUEST' is designed to occur right after each synchronization (WAIT)
                # We do not need to look after the exact timing of each request
            elif tp == 'WRITE':
                write_queue.append(value)
            elif tp == 'WAIT':
                time, mem_id, time_gap = value
                rel_time_pivot = wait_start_timer
                comp_time = time - wait_start_timer
                wait_start_timer = time + time_gap
                while rel_time_pivot < time:
                    write_pivot = time + 1
                    if len(write_queue) > 0:
                        write_pivot, write_addr, write_size = write_queue[0]
                    request_id = -1
                    if mem_id in request_dict:
                        request_id = mem_id
                    elif len(request_dict) > 0:
                        request_id = list(request_dict.keys())[0]

                    rw_flag = False  # False: Read, True: Write
                    if request_id == -1:
                        if write_pivot > time:
                            break
                        rw_flag = True
                    elif write_pivot <= time:
                        rw_flag = write_pivot <= rel_time_pivot
                    if rw_flag:
                        rel_time = max(1, math.ceil(
                            write_pivot - rel_time_pivot))
                        dma_operation.append(
                            ('WRITE', [rel_time, write_addr, write_size]))
                        rel_time_pivot = max(rel_time_pivot, write_pivot)
                        write_queue = write_queue[1:]
                    else:
                        read_addr, size = request_dict[request_id]
                        while rel_time_pivot <= min(time, write_pivot):
                            req_size = min(request_size, size)
                            dma_operation.append(
                                ('READ', [1, read_addr, req_size]))
                            rel_time_pivot += estimate_respond_time
                            read_addr += req_size
                            size -= req_size
                        if size > 0:
                            request_dict[request_id] = [read_addr, size]
                        else:
                            del request_dict[request_id]
                if mem_id in request_dict:
                    read_addr, size = request_dict[mem_id]
                    while size > 0:
                        req_size = min(request_size, size)
                        dma_operation.append(
                            ('READ', [1, read_addr, req_size]))
                        read_addr += req_size
                        size -= req_size
                    del request_dict[mem_id]
                dma_operation.append(('END_COMP', comp_time))
                dma_operation.append(('START_COMP', 0))
            else:
                raise ValueError("Unknown processing type: {}".format(tp))
        return dma_operation[:-1]
