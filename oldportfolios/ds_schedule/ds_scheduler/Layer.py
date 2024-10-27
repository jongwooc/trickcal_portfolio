import google.protobuf
import google.protobuf.text_format
import caffe_pb2
import array
import numpy
import random
import unicodedata
from deap import algorithms
from deap import base
from deap import creator
from deap import tools
from inspect import currentframe

class Layer:
    index = None
    name = None
    mem_size = None
    cpu_time = None
    gpu_time = None
    bottom_layer_list = None 
    top_layer_list = None
    est_info_dict = None
    width = None
    height = None
    num_output = None
    channel = None
    pad = None
    stride = None
    kernel_size = None
    def __init__(self, index, name):
        self.index = index
        self.name = name
        self.bottom_layer_list = []
        self.top_layer_list = []
        self.est_info_dict = {}
        self.mem_size = 0
        self.width = 0
        self.height = 0
        self.num_output = 0
        self.pad = 0
        self.stride = 1
        self.kernel_size = 0

    def get_layer_index(self):
        return self.index
    
    def get_layer_name(self):
        return self.name

    def add_bottom_layer(self, name):
        self.bottom_layer_list.append(name)

    def add_top_layer(self, name):
        self.top_layer_list.append(name)

    def get_top_layer_list(self):
        return self.top_layer_list

    def get_bottom_layer_list(self):
        return self.bottom_layer_list

    def get_est_info_dic(self):
        return self.est_info_dict

    def set_mem_size(self):
        self.mem_size = self.num_output* self.width * self.height

    def set_num_output(self, num_output):
        self.num_output = num_output

    def set_width_height(self, width, height):
        self.width = width
        self.height = height

    def set_pad_stride_kernel_size(self, pad, stride, kernel_size):
        self.pad = pad
        self.stride = stride
        self.kernel_size = kernel_size_

    def set_info(self, num_output, width, height, pad, kernel_size, stride):
        self.num_output = num_output
        self.height = height
        self.width = width
        self.pad = pad
        self.kernel_size = kernel_size
        self.stride = stride

    def get_num_output(self):
        return self.num_output

    def get_width(self):
        return self.width

    def get_height(self):
        return self.height

    def get_pad(self):
        return self.pad

    def get_kernel_size(self):
        return self.kernel_size

    def get_stride(self):
        return self.stride

    def get_mem_size(self):
        return self.mem_size

