import sys
import google.protobuf
import google.protobuf.text_format
import caffe_pb2
from Layer import *
import array
import numpy
import random
import unicodedata
from deap import algorithms
from deap import base
from deap import creator
from deap import tools
from inspect import currentframe
from optparse import OptionParser

class Scheduler:
    network_info_file = None
    estimation_info_file = None
    net = None
    est = None
    est_info = None
    mem_init_delay = None
    mem_unit_delay = None
    layer_dict = {}
    mem_transition_time_dict = {}

    def __init__(self, network_info_file, estimation_info_file):
        self.network_info_file = network_info_file
        self.estimation_info_file = estimation_info_file
        self.mem_init_delay = 0.5
        self.mem_unit_delay = 0.0001

        self.read_prototxt()
        self.save_layer_dict_from_net()
        self.read_estimation_info()
        self.save_est_info_from_est()
        # self.print_layer_info()
        self.set_info_for_network()
        self.set_mem_transfer_time_table()

    def get_line_number():
        cf = currentframe()
        return cf.f_back.f_lineno

    def get_string_from_else(self, uni):
        if isinstance(uni, str):
            return uni
        elif isinstance(uni, unicode):
            return unicodedata.normalize('NFKD', uni).encode('ascii', 'ignore')
        elif isinstance(uni, google.protobuf.pyext._message.RepeatedScalarContainer):
            return str(uni[0])
        else:
            return str(uni[0])

    def read_prototxt(self):
        self.net = caffe_pb2.NetParameter()
        f = open(self.network_info_file, "r")
        google.protobuf.text_format.Merge(f.read(), self.net)
        f.close()

    def get_bottom_index(self, layer_index, layer_name_list):
        bottom = self.get_string_from_else(self.net.layer[layer_index].bottom)
        src = layer_name_list.index(bottom)
        return src

    def get_top_index(self, layer_index, layer_name_list):
        top = self.get_string_from_else(self.net.layer[layer_index].top)
        dst = layer_name_list.index(top)
        return dst

    def check_top_bottom_is_equal(self, layer_index, layer_name_list):
        if self.net.layer[layer_index].bottom and self.net.layer[layer_index].top:
            bottom_index = self.get_bottom_index(layer_index, layer_name_list)
            top_index = self.get_top_index(layer_index, layer_name_list)
            if bottom_index == top_index:
                return True
            else:
                return False

    def set_top_layer_list(self, layer_name, bottom_layer_name):
        layer = self.layer_dict[layer_name]
        for key in self.layer_dict:
            # print("layer_name: " + layer_name + " key: " + key)
            top_layer = self.layer_dict[key]
            bottom_layer_list = top_layer.get_bottom_layer_list()
            if bottom_layer_name in bottom_layer_list:
                layer.add_top_layer(key)

    def check_concat_layer(self, i):
        if self.net.layer[i].type == "Concat":
            return True
        else:
            return False

    def connect_layers(self):
        layer_num = len(self.net.layer)
        layer_name_list = []
        concat_dict = {}
        for i in range(0, layer_num):
            is_concat = self.check_concat_layer(i)
            if is_concat:
                bottom_layer_list = []
                for j in range(0, len(self.net.layer[i].bottom)):
                    bottom_layer_name = self.get_string_from_else(self.net.layer[i].bottom[j])
                    bottom_layer_list.append(bottom_layer_name)
                concat_dict[self.get_string_from_else(self.net.layer[i].name)] = bottom_layer_list
            layer_name_list.append(self.net.layer[i].name)

        for i in range(0, layer_num):
            is_concat = self.check_concat_layer(i)
            is_equal = self.check_top_bottom_is_equal(i, layer_name_list)
            if not is_equal and not is_concat:
                layer_name = self.get_string_from_else(self.net.layer[i].name)
                layer = self.layer_dict[layer_name]
                if layer.bottom_layer_list:
                    bottom_layer_name = self.get_string_from_else(layer.bottom_layer_list[0])
                    if bottom_layer_name in concat_dict:
                        layer.bottom_layer_list = concat_dict[bottom_layer_name]
                        for key in layer.bottom_layer_list:
                            key_layer = self.layer_dict[key]
                            key_layer.add_top_layer(layer_name)

    def save_layer_dict_from_net(self):
        layer_num = len(self.net.layer)
        layer_name_list = []

        for i in range(0, layer_num):
            layer_name_list.append(self.net.layer[i].name)

        index = 0
        for i in range(0, layer_num):
            is_concat = self.check_concat_layer(i)
            is_equal = self.check_top_bottom_is_equal(i, layer_name_list)
            if not is_equal and not is_concat:
                layer = Layer(index, self.get_string_from_else(self.net.layer[i].name))
                self.layer_dict[self.get_string_from_else(self.net.layer[i].name)] = layer
                if self.net.layer[i].bottom:
                    if(isinstance(self.net.layer[i].bottom, list)):
                        for j in range(0, len(self.net.layer[i].bottom)):
                            bottom_layer_name = self.get_string_from_else(self.net.layer[i].bottom[j])
                            layer.add_bottom_layer(bottom_layer_name)
                    elif(isinstance(self.net.layer[i].bottom, google.protobuf.pyext._message.RepeatedScalarContainer)):
                        bottom_layer_name = self.get_string_from_else(self.net.layer[i].bottom)
                        layer.add_bottom_layer(bottom_layer_name)
                    else:
                        print("type is wrong, line: " +  get_line_number())
                index += 1

        for i in range(0, layer_num):
            is_concat = self.check_concat_layer(i)
            is_equal = self.check_top_bottom_is_equal(i, layer_name_list)
            if not is_equal and not is_concat:
                if self.net.layer[i].top:
                    if(isinstance(self.net.layer[i].top, list)):
                        for j in range(0, len(self.net.layer[i].top)):
                            top_layer_name = self.get_string_from_else(self.net.layer[i].top[j])
                            self.set_top_layer_list(self.get_string_from_else(self.net.layer[i].name), top_layer_name)
                    elif(isinstance(self.net.layer[i].top, google.protobuf.pyext._message.RepeatedScalarContainer)):
                        top_layer_name = self.get_string_from_else(self.net.layer[i].top)
                        self.set_top_layer_list(self.get_string_from_else(self.net.layer[i].name), top_layer_name)
                    else:
                        print("type is wrong, line: " +  get_line_number())

        self.connect_layers()

    def print_layer_info(self):
        for name in self.layer_dict:
            layer = self.layer_dict[name]
            print("layer_name: " + name)
            est_dic = layer.get_est_info_dic()
            top_layer_list = layer.get_top_layer_list()
            print("top_layer_list")
            print(top_layer_list)
            bottom_layer_list = layer.get_bottom_layer_list()

    def read_estimation_info(self):
        self.est = caffe_pb2.NetParameter()
        f = open(self.estimation_info_file, "r")
        google.protobuf.text_format.Merge(f.read(), self.est)
        f.close()

    def save_est_info_from_est(self):
        layer_num = len(self.est.layer)
        layer_name_index_dict = {}
        for i in range(0, len(self.net.layer)):
            layer_name_index_dict[self.get_string_from_else(self.net.layer[i].name)] = i
        
        for j in range(0, layer_num):
            name = self.get_string_from_else(self.est.layer[j].name)
            if name in self.layer_dict:
                layer = self.layer_dict[name]
                dic = layer.get_est_info_dic()
                if self.est.layer[j].cpu:
                    dic['cpu'] = self.est.layer[j].cpu
                else:
                    dic['cpu'] = 0
                if self.est.layer[j].gpu:
                    dic['gpu'] = self.est.layer[j].gpu
                else:
                    dic['gpu'] = 0
                if self.est.layer[j].time_unit:
                    dic['time_unit'] = self.get_string_from_else(self.est.layer[j].time_unit)
            else:
                layer_index = layer_name_index_dict[self.get_string_from_else(self.est.layer[j].name)]
                bottom_layer_name = self.get_string_from_else(self.net.layer[layer_index].bottom)
                bottom_layer = self.layer_dict[bottom_layer_name]
                bottom_dic = bottom_layer.get_est_info_dic()
                bottom_dic['cpu'] = bottom_dic['cpu'] + self.est.layer[j].cpu
                bottom_dic['gpu'] = bottom_dic['gpu'] + self.est.layer[j].gpu

    def get_width_or_height_for_conv_and_pool(self, width_or_height, pad, kernel_size, stride):
        return (width_or_height + 2 * pad - kernel_size) / stride + 1

    def set_info_for_network(self):
        layer_num = len(self.net.layer)
        for i in range(0, layer_num):
            if self.get_string_from_else(self.net.layer[i].name) in self.layer_dict:
                layer = self.layer_dict[self.get_string_from_else(self.net.layer[i].name)]
                if self.net.layer[i].type == "Input":
                    width = self.net.layer[i].input_param.shape[0].dim[2]
                    height = self.net.layer[i].input_param.shape[0].dim[3]
                    layer.set_info(1, width, height, 0, 0, 1)
                    self.start_layer_name = self.get_string_from_else(self.net.layer[i].name)
                elif self.net.layer[i].type == "Convolution":
                    num_output = self.net.layer[i].convolution_param.num_output
                    kernel_size = int(self.net.layer[i].convolution_param.kernel_size[0])
                    if self.net.layer[i].convolution_param.stride:
                        stride = int(self.net.layer[i].convolution_param.stride[0])
                    else:
                        stride = 1
                    if self.net.layer[i].convolution_param.pad:
                        pad = int(self.net.layer[i].convolution_param.pad[0])
                    else:
                        pad = 0
                    bottom_layer_list = layer.get_bottom_layer_list()
                    for bottom_layer_name in bottom_layer_list:
                        bottom_layer = self.layer_dict[bottom_layer_name]
                        bottom_width = bottom_layer.get_width()
                        bottom_height = bottom_layer.get_height()
                        width = self.get_width_or_height_for_conv_and_pool(bottom_width, pad, kernel_size, stride)
                        height = self.get_width_or_height_for_conv_and_pool(bottom_height, pad, kernel_size, stride)
                        break;
                    layer.set_info(num_output, width, height, pad, kernel_size, stride)
                elif self.net.layer[i].type == "Pooling":
                    kernel_size = self.net.layer[i].pooling_param.kernel_size
                    if self.net.layer[i].pooling_param.stride:
                        stride = self.net.layer[i].pooling_param.stride
                    else:
                        stride = 1
                    if self.net.layer[i].pooling_param.pad:
                        pad = self.net.layer[i].pooling_param.pad
                    else:
                        pad = 0
                    bottom_layer_list = layer.get_bottom_layer_list()
                    for bottom_layer_name in bottom_layer_list:
                        bottom_layer = self.layer_dict[bottom_layer_name]
                        num_output = bottom_layer.get_num_output()
                        bottom_width = bottom_layer.get_width()
                        bottom_height = bottom_layer.get_height()
                        width = self.get_width_or_height_for_conv_and_pool(bottom_width, pad, kernel_size, stride)
                        height = self.get_width_or_height_for_conv_and_pool(bottom_height, pad, kernel_size, stride)
                        break;
                    layer.set_info(num_output, width, height, pad, kernel_size, stride)
                elif self.net.layer[i].type == "InnerProduct":
                    num_output = self.net.layer[i].inner_product_param.num_output
                    layer.set_info(num_output, 1, 1, 0, 0, 1)
                else:
                    bottom_layer_list = layer.get_bottom_layer_list()
                    for bottom_layer_name in bottom_layer_list:
                        bottom_layer = self.layer_dict[bottom_layer_name]
                        num_output = bottom_layer.get_num_output()
                        width = bottom_layer.get_width()
                        height = bottom_layer.get_height()
                        break;
                    layer.set_info(num_output, width, height, 0, 0, 1)
                layer.set_mem_size()

    def get_mem_transfer_time_from_cpu_to_gpu(self, layer_name):
        layer = self.layer_dict[layer_name]
        return self.mem_init_delay + self.mem_unit_delay * layer.get_mem_size()

    def get_mem_transfer_time_from_gpu_to_cpu(self, layer_name):
        layer = self.layer_dict[layer_name]
        return self.mem_init_delay + self.mem_unit_delay * layer.get_mem_size()

    def get_zero_for_mem_transfer_time(self, layer_name):
        return 0

    def set_mem_transfer_time_table(self):
        dic_for_cpu = {'gpu': self.get_mem_transfer_time_from_cpu_to_gpu}
        dic_for_cpu['cpu'] = self.get_zero_for_mem_transfer_time
        self.mem_transition_time_dict['cpu'] = dic_for_cpu
        dic_for_gpu = {'cpu': self.get_mem_transfer_time_from_gpu_to_cpu}
        dic_for_gpu['gpu'] = self.get_zero_for_mem_transfer_time
        self.mem_transition_time_dict['gpu'] = dic_for_gpu

    def do_schedule(self):
        print("Error: not yet completed schedule function")

    def print_result(self):
        print("Error: not yet completed schedule print function")


# parse command line options
def parse_options():
    usage = """usage: %prog [options]"""
    parser = OptionParser(usage=usage)
    parser.add_option("-s", "--schedule_method", dest="sched_method", help="set scheduler method")
    parser.add_option("-n", "--network", dest="net", default="squeezenet_cfg_to_prototxt.prototxt", help="set network prototxt file path", metavar="FILE")
    parser.add_option("-e", "--estimation", dest="est", default="squeezenet_estimation_cpu_core_1111.prototxt", help="set estimated time information file path", metavar="FILE")
    (options, args) = parser.parse_args()

    if options.sched_method is None:
        print("please insert scheule method: (ListScheduler, GA or ILP)")
        parser.print_help()
        sys.exit(2)
    
    return options, args

if __name__ == '__main__':
#    scheduler = Scheduler("./lenet.prototxt", "./lenet_estimation.prototxt")
#    scheduler.read_prototxt()
#    scheduler.save_layer_dict_from_net()
#    scheduler.read_estimation_info()
#    scheduler.save_est_info_from_est()
#    scheduler.print_layer_info()
#    scheduler.set_info_for_network()
    options, args = parse_options()

    sched_module = __import__(options.sched_method)
    sched_class = getattr(sched_module, options.sched_method)
    sched = sched_class(options.net, options.est)
    sched.print_layer_info()
    sched.init_for_schedule()
    sched.do_schedule()
    sched.print_result()
