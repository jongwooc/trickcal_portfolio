from __future__ import absolute_import, division, print_function, unicode_literals

import numpy as np

from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

from . import statistics as stats
from .pipelined_component import PipelinedComponent


class Adder(PipelinedComponent):
    def __init__(self):
        super(Adder, self).__init__()
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.name = 'adder'
        self.output_buf = np.zeros(self.system_width)
        self.csatree_buf = np.zeros(self.num_wmem)
        self.accumulator_buf = np.zeros(self.system_width)

    def setup(self, layer_info):
        super(Adder, self).setup()
        main_op = layer_info.main_op
        self.main_op = main_op
        self.shape = layer_info.input[0].output_tensor.shape
        self.adder_type = 0
        if layer_info.parallel_type is not None:
            if isinstance(main_op, ConvOp):
                self.adder_type = 1
            else:
                self.adder_type = 2 if main_op.type == 'AveragePool' else 3
        self.adder_count = 0

    def work(self, input_buf, input_info):  # input buf : multiplier results
        if input_info is None or input_info.junk:
            pass
        elif self.adder_type == 0:  # CSATREE
            partial_sum = np.sum(input_buf, axis=1)
            if input_info.reset:
                self.csatree_buf[:] = partial_sum[:]
            else:
                self.csatree_buf = np.add(self.csatree_buf, partial_sum)
            if input_info.last:
                #print("loc: {}, sum: {}".format(input_info.loc, self.csatree_buf[0]))
                self.output_buf[0:self.num_wmem] = self.csatree_buf[:]
        else:
            if input_info.reset:
                self.accumulator_buf[:] = input_buf[0, :]
                self.adder_count = 1
            elif self.adder_type < 3:
                self.accumulator_buf = np.add(
                    self.accumulator_buf, input_buf[0, :])
                self.adder_count += 1
            else:
                self.accumulator_buf = np.maximum(
                    self.accumulator_buf, input_buf[0, :])
            if input_info.last:
                if self.adder_type == 2:
                    self.output_buf[:] = np.true_divide(
                        self.accumulator_buf, self.adder_count)
                else:
                    self.output_buf[:] = self.accumulator_buf
        self.output_info = input_info
