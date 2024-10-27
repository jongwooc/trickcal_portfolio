from __future__ import print_function

import argparse
import os.path

import midap_simulator.statistics as stats
import models.examples as ex
from config import cfg
from generic_op import *
from midap_simulator import *
from midap_software import Compiler, MidapModel


def parse():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input_shape', nargs='+', type=int, required=True)
    parser.add_argument('-oc', '--out_chan', type=int, required=True)
    parser.add_argument('-k', '--kern_info', nargs='+', type=int, required=True)
    parser.add_argument('-l', '--layer_compiler', type=str, choices=['MIN_DRAM_ACCESS', 'HIDE_DRAM_LATENCY'], default='HIDE_DRAM_LATENCY')
    parser.add_argument('-ib', '--init_banks', type=int, default=0)

    parser.add_argument('-b', '--bus_policy', type=str, choices=['WMEM_FIRST', 'FIFO'], default='WMEM_FIRST')
    parser.add_argument('-o', '--output_dir', type=str, default=None)
    parser.add_argument('-a', '--abstract_layer', action="store_true", default=False)

    parser.add_argument('-f', '--fmem_entries', type=int, default=256)
    parser.add_argument('-nb', '--num_banks', type=int, default=4)
    parser.add_argument('--latency', type=int, default=100)
    parser.add_argument('--bandwidth', type=int, default=32)
    return parser.parse_args()


class TestWrapper(object):
    def __init__(self):
        self.cv = GenericConvertor()
        self.midap_model = MidapModel()
        self.cm = Compiler()
        self.midap_simulator = MidapManager()
        self.step_checker = [0, 0, 0]

    def setup_model(self, model_type, model, img_shape=None, image=None):
        if image is not None:
            self.cv.set_image(img_path=image, img_shape=img_shape)
        else:
            self.cv.set_input_tensor(tensor_shape=img_shape)
        if model_type == 'caffe2':
            init_pb = os.path.join('examples/caffe2', model, 'init_net.pb')
            predict_pb = os.path.join('examples/caffe2', model, 'predict_net.pb')
            self.cv.from_caffe2_model(init_pb=init_pb, predict_pb=predict_pb)
        elif model_type == 'tf_builtin':
            self.cv.tensorflow_module(model, builtin=True)
        elif model_type == 'tf_model':
            model_hdf = os.path.join('examples/tf', model + '.hdf5')
            self.cv.tensorflow_module(model_hdf, builtin=False)
        else:
            raise ValueError("model_type must be one of ['caffe2', 'tf_builtin', 'tf_model']")
        self.midap_model.from_generic_op_dict(self.cv.operator_dict)
        self.step_checker[0] = 1
        if self.step_checker[1] > 0:
            del self.cm
            self.cm = Compiler()
            self.step_checker[1] = 0

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

    def compile(self, num_init_banks):
        if self.step_checker[0] == 0:
            print("Please setup the model first")
            return
        self.cm.force_setup(num_init_banks)
        static_info = self.cm.setup(self.midap_model)
        self.step_checker[1] = 1
        if self.step_checker[2] > 0:
            del self.midap_simulator
            self.midap_simulator = MidapManager()
            self.step_checker[2] = 0
        return static_info

    def simulate(self):
        if self.step_checker[0] == 0:
            print("Please setup the model first")
            return
        elif self.step_checker[1] == 0:
            print("Please run compile")
            return
        input_tensor, path_info = self.cm.get_control_info()
        self.midap_simulator.process_network(input_tensor[0], path_info)
        self.step_checker[2] = 1
        return path_info

    def run_all(self, model_type, model, num_init_banks, img_shape=None, image=None, output_dir=None):
        self.__init__()
        if model_type == 'custom':
            self.setup_from_builder(model)
            model = model.name
        else:
            self.setup_model(model_type, model, img_shape, image)
            model = model + str(img_shape)
        static_info = self.compile(num_init_banks)
        path_info = self.simulate()
        if output_dir is not None:
            if not os.path.exists(output_dir):
                os.mkdir(output_dir)
            stats.print_simulate_result(os.path.join(output_dir, model + '_' + model_type + '.txt'), path_info, read=False)
        print("[ {} ]".format(model))
        stats.print_result(path_info, static_info, model)


args = parse()
cfg.MIDAP.CONTROL_STRATEGY.LAYER_COMPILER = args.layer_compiler
cfg.MIDAP.BUS_POLICY = args.bus_policy
cfg.MODEL.ALLOW_ABSTRACT_DATA = args.abstract_layer

# Configuration
cfg.MIDAP.SYSTEM_WIDTH = 64
# cfg.MIDAP.FMEM.SIZE = 256 * 1024
cfg.MIDAP.FMEM.NUM_ENTRIES = args.fmem_entries * 1024
cfg.MIDAP.FMEM.NUM = args.num_banks

cfg.SYSTEM.BANDWIDTH = args.bandwidth  # GB ( * 10^9 byte) / s
cfg.LATENCY.DRAM_READ = args.latency

output_dir = args.output_dir

tr = TestWrapper()

mb = ex.one_layer_example(args.input_shape, args.out_chan, args.kern_info)
tr.run_all("custom", mb, args.init_banks, output_dir=output_dir)
