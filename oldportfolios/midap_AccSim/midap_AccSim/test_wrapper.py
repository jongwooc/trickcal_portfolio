from __future__ import print_function

import logging
import os.path

import midap_simulator.statistics as stats
from config import cfg
from generic_op import *
from midap_simulator import *
from midap_software import Compiler, MidapModel


class TestWrapper(object):
    def __init__(self):
        self.cv = GenericConvertor()
        self.midap_model = MidapModel()
        self.cm = Compiler()
        self.midap_simulator = MidapManager()
        # it stands for setup model -> compile -> simulate, respectively.
        self.step_checker = [0, 0, 0]
        self.logger = logging.getLogger(cfg.LOGGER)

    def setup_from_builder(self, builder):
        odict = builder.get_operator_dict()
        self.cv.operator_dict = odict
        self.cv.post_process()
        self.midap_model.from_generic_op_dict(odict)
        self.step_checker[0] = 1
        if self.step_checker[1] > 0:
            del self.cm
            self.cm = Compiler()
            self.step_checker[1] = 0
        self.cm.set_op_dict(odict)

    def compile(self):
        if self.step_checker[0] == 0:
            self.logger.error("Please setup the model first")
            return
        static_info = self.cm.setup(self.midap_model)
        self.step_checker[1] = 1
        if self.step_checker[2] > 0:
            del self.midap_simulator
            self.midap_simulator = MidapManager()
            self.step_checker[2] = 0
        return static_info

    def simulate(self):
        if self.step_checker[0] == 0:
            self.logger.error("Please setup the model first")
            return
        elif self.step_checker[1] == 0:
            self.logger.error("Please run compile")
            return
        input_tensor_list, path_info = self.cm.get_control_info()
        init_layer_list = self.midap_model.init_layer
        # stats = self.midap_simulator.process_network(input_tensor, path_info)
        stats = self.midap_simulator.process_network_with_multiple_input(
            input_tensor_list, init_layer_list, path_info)
        self.step_checker[2] = 1
        return path_info, stats

    def run_all(self, model, img_shape=None, output_dir=None, output_option=(True, False, False, False)):
        self.__init__()
        self.setup_from_builder(model)
        model = model.name
        self.logger.info("[ {} ]".format(model))
        static_info = self.compile()
        path_info, stat = self.simulate()
        diff, latency, dram_access = stat
        # print("check stat(Checking info) of network {}: {}".format(model ,stat), file=sys.stderr)
        if diff > 0:
            self.logger.error(
                "Network Result Diff > 0: Functional Problem may occur, network {}".format(model))
        if output_dir is not None:
            if not os.path.exists(output_dir):
                os.mkdir(output_dir)
            read, write, run, etc = output_option
            stats.print_simulate_result(os.path.join(
                output_dir, model + '.txt'), path_info, read=read, write=write, run=run, etc=etc)
        # stats.diff_static_and_simulate(path_info, static_info)
        stats.print_result(path_info, static_info, model)
        return latency, dram_access

    def output_dma_trace(self, trace_file):
        if self.step_checker[2] == 0:
            self.logger.error("Please call it after the simulation")
            return None
        opcode_dict = {
            'START_COMP': 'COMP_START',
            'END_COMP': 'COMP_END',
            'READ': '0',
            'WRITE': '1'
        }
        trace_arr = self.midap_simulator.memory_manager.convert_memory_footprint_to_dma_operation()
        with open(trace_file, 'w') as f:
            for tp, val in trace_arr:
                tp = opcode_dict[tp]
                if tp in ['COMP_START', 'COMP_END']:
                    f.write("{} {}\n".format(tp, int(val)))
                else:
                    rel_time, addr, size = val
                    f.write("{} {} {:x} {}\n".format(
                        tp, int(rel_time), int(addr), int(size)))
