from __future__ import absolute_import, division, print_function, unicode_literals

import numpy as np

from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

from . import statistics as stats
from .pipelined_component import PipelinedComponent


class ALU(PipelinedComponent):
    def __init__(self):
        super(ALU, self).__init__()
        self.system_width = cfg.MIDAP.SYSTEM_WIDTH
        self.num_wmem = cfg.MIDAP.WMEM.NUM
        self.name = 'alu'
        self.output_buf = np.zeros((self.num_wmem, self.system_width))

    def setup(self, layer_info):
        super(ALU, self).setup()
        main_op = layer_info.main_op
        self.layer_type = 0
        if isinstance(main_op, PoolOp):
            self.layer_type = 1
        elif isinstance(main_op, SumOp):
            self.layer_type = 2

    def work(self, broadcast_fbuf, wbuf, input_info):
        if input_info is not None:
            stats.run_alu(self.system_width * self.num_wmem)
            if self.layer_type == 0:
                self.output_buf = np.multiply(broadcast_fbuf, wbuf)
            elif self.layer_type == 1:
                self.output_buf[:] = broadcast_fbuf
            else:
                self.output_buf = np.add(broadcast_fbuf, wbuf)
        self.output_info = input_info
