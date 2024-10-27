from __future__ import absolute_import, division, print_function, unicode_literals

import numpy as np

from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

from . import statistics as stats
from .pipelined_component import PipelinedComponent


class Activator(PipelinedComponent):
    def __init__(self, memory_manager):
        super(Activator, self).__init__()
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.num_fmem = cfg.MIDAP.FMEM.NUM
        self.memory_manager = memory_manager
        self.bias_buf = np.zeros(self.system_width)
        self.bias_output_buf = np.zeros(self.system_width)
        self.output_buf = np.zeros(self.system_width)
        self.name = 'activation_controller'

    def __del__(self):
        del self.memory_manager
        del self.bias_buf, self.bias_output_buf, self.output_buf

    def setup(self, layer_info):
        super(Activator, self).setup()
        self.main_op = layer_info.main_op
        self.offset = layer_info.offset  # debug param
        self.concurrency = self.num_wmem if layer_info.parallel_type is None else self.system_width
        self.activation = layer_info.main_op.activation
        self.add_bias = isinstance(
            self.main_op, ConvOp) and self.main_op.bias is not None
        self.loaded_bias = -1

    def work(self, input_buf, input_info):
        self.output_info = input_info
        if input_info is not None and input_info.last:
            if self.add_bias:
                # this logic should be seperated (to be done 1 cycle earlier)
                if self.loaded_bias != input_info.filter_idx:
                    self.memory_manager.load_bbuf(
                        self.bias_buf, input_info.filter_idx, self.concurrency)
                    self.loaded_bias = input_info.filter_idx
                self.bias_output_buf = np.add(input_buf, self.bias_buf)
            else:
                self.bias_output_buf[:] = input_buf[:]
            if self.activation is not None:
                if self.activation.lower() == 'leakyrelu':
                    self.output_buf = np.where(
                        self.bias_output_buf > 0, self.bias_output_buf, self.bias_output_buf * 0.01)
                elif 'relu' in self.activation.lower():
                    self.output_buf = np.maximum(self.bias_output_buf, 0)
                    if self.activation.lower() == 'relu6':
                        self.output_buf = np.minimum(self.output_buf, 6)
                elif self.activation.lower() == 'sigmoid':
                    self.output_buf = 1 / (1 + np.exp(-self.bias_output_buf))
                else:
                    raise ValueError(
                        "Unknown acitvation {}".format(self.activation))
            else:
                self.output_buf[:] = self.bias_output_buf
