import sys
from Scheduler import *
from GanttChart import *

from ctypes import *

# def __init__(self, network_info_file, estimation_info_file):
class ILP_batch(Scheduler):
    porting = None
    scheduled_time_dict = None
    num_layer = None
    num_network = None
    objectives = None
    processor_list = ['cpu', 'cpu2', 'cpu3', 'cpu4', 'gpu']

    def __init__(self, network_info_file, estimation_info_file):
        # not used
        Scheduler.__init__(self, network_info_file, estimation_info_file)
        self.porting = CDLL('./main_porting.so')
        self.scheduled_time_dict = {}
        self.num_network = 1
        self.objectives = 1 # 0: Latency, 1: Throughput

    def init_for_schedule(self):
        return

    def do_schedule(self):
        self.porting.ilp('', '', self.objectives)

    def print_result(self):
        print 'objectives: ', self.objectives
        print 'num_layer: ', self.num_layer
        self.porting.get_layer_name_result.restype = c_char_p
        self.porting.get_mapped_result.restype = c_char_p
        self.porting.get_start_time_result.restype = c_float
        self.porting.get_end_time_result.restype = c_float
        self.porting.get_comm_time_result.restype = c_float
        self.porting.get_num_layer.restype = c_int
        self.num_layer = self.porting.get_num_layer()

        for network_index in range(0, self.num_network):
            for layer_index in range(0, self.num_layer):
                layer_dict = {}
                layer_name = self.porting.get_layer_name_result(layer_index)
                mapped = self.porting.get_mapped_result(layer_index)
                start_time = self.porting.get_start_time_result(layer_index)
                end_time = self.porting.get_end_time_result(layer_index)
                comm_time = self.porting.get_comm_time_result(layer_index)
                result_tuple_for_time_table = (layer_name, mapped, start_time, end_time, comm_time)
                if network_index in self.scheduled_time_dict:
                    layer_dict = self.scheduled_time_dict[network_index]
                    layer_dict[layer_index] = result_tuple_for_time_table
                else:
                    layer_dict = {layer_index: result_tuple_for_time_table}
                    self.scheduled_time_dict[network_index] = layer_dict

        result_list = []
        for key1 in self.scheduled_time_dict:
            result_dict = self.scheduled_time_dict[key1]
            for key2 in result_dict:
                result = result_dict[key2]
                result_list.append(result)

        for value in result_list:
            print "Gantt chart value", value, "\n"

        gantt = GanttChart('result_for_ILP_squeeze_1111.png', self.processor_list)
        gantt.add_tasks_all(result_list)
        gantt.draw_gantt_chart()

        return
