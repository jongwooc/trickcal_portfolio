from __future__ import absolute_import, division, print_function, unicode_literals

import logging

import numpy as np

from acc_utils.attrdict import AttrDict
from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

from .pipelined_component import PipelinedComponent


class ReductionModule(PipelinedComponent):
    def __init__(self):
        super(ReductionModule, self).__init__()
        self.name = 'reduction_module'
        self.reduction_type = None
        self.reduction_info = None
        self.reduction_buf = np.zeros(cfg.MIDAP.REDUCTION.NUM_ENTRIES)
        self.reduction_value = 0
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.output_buf = np.zeros(self.system_width)
        self.bypass_flag = True
        self.logger = logging.getLogger(cfg.LOGGER)

    def __del__(self):
        del self.reduction_buf, self.output_buf, self.logger

    def setup(self, layer_info):
        super(ReductionModule, self).setup()
        self.concurrency = self.num_wmem if layer_info.parallel_type is None else self.system_width
        self.reduction_type = None
        self.reduction_value = 0
        self.bypass_flag = True
        if layer_info.have_reduction_layer:
            op = layer_info.next[0].main_op
            if isinstance(op, PoolOp):
                self.reduction_type = 0
                self.reduction_value = layer_info.output_tensor[:, :, 0].size
            elif op.type == 'Softmax':
                self.reduction_type = 1
            else:
                raise ValueError(
                    "Unknown Reduction operation {}".format(layer_info.main_op))
        self.write_mode = False
        self.output_info = None
        self.output_offset = 0
        self.channel_size = layer_info.output_tensor.shape[-1]

    def enable_write(self):
        self.write_mode = True
        if self.reduction_type == 1:
            self.reduction_value = np.sum(
                self.reduction_buf[:self.channel_size])
        self.logger.info("Reduction module: Reduction output generation begins, reduction value = {}".format(
            self.reduction_value))

    def run(self, input_buf, input_info):
        self.work(input_buf, input_info)

    def work(self, input_buf, input_info):
        if not self.write_mode:
            self.output_buf[:] = input_buf[:]
            self.output_info = input_info
            if input_info is None or not input_info.last:
                pass
            elif self.reduction_type == 0:
                loc = input_info.loc
                filter_idx = input_info.filter_idx
                if loc == (0, 0):
                    self.reduction_buf[filter_idx:filter_idx +
                                       self.concurrency] = input_buf[:self.concurrency]
                else:
                    self.reduction_buf[filter_idx:filter_idx + self.concurrency] = np.add(
                        self.reduction_buf[filter_idx:filter_idx + self.concurrency], input_buf[:self.concurrency])
            elif self.reduction_type == 1:
                self.reduction_buf[input_info.filter_idx: input_info.filter_idx +
                                   self.concurrency] = np.exp(input_buf[:self.concurrency])
        else:
            if self.reduction_type is None or self.output_offset >= self.channel_size:
                self.output_info = None
            else:
                output_info = AttrDict()
                output_info.reset = False
                output_info.loc = (0, 0)
                output_info.filter_idx = self.output_offset
                output_info.last = True
                self.output_info = output_info
                self.output_buf[:self.concurrency] = self.reduction_buf[self.output_offset:
                                                                        self.output_offset + self.concurrency] / self.reduction_value
                self.output_offset += self.concurrency
