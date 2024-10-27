import sys
from Scheduler import *
from GanttChart import *

class ListScheduler(Scheduler):
    processor_list = None
    processor_curr_time_dict = None
    visited_dict = None
    ready_list = None
    result_list = None
    layer_processor_mapping_dict = None
    layer_end_time_dict = None

    def __init__(self, network_info_file, estimation_info_file):
        self.processor_list = ['cpu', 'gpu']
        self.processor_curr_time_dict = {}
        for key in self.processor_list:
            self.processor_curr_time_dict[key] = 0
        self.visited_dict = {}
        self.ready_list = []
        self.result_list = []
        self.layer_processor_mapping_dict = {}
        self.layer_end_time_dict = {}
        self.layer_end_time_dict = {}
        Scheduler.__init__(self, network_info_file, estimation_info_file)

    def init_for_schedule(self):
        for key in self.layer_dict:
            layer = self.layer_dict[key]
            self.visited_dict[key] = False
            self.layer_end_time_dict[key] = 0

    def update_ready_list(self):
        for key in self.layer_dict:
            if not self.visited_dict[key]:
                layer = self.layer_dict[key]
                bottom_layer_list = layer.get_bottom_layer_list()
                flag = True
                for bottom_layer_name in bottom_layer_list:
                    if not self.visited_dict[bottom_layer_name]:
                        flag = False
                        break
                if flag:
                    self.ready_list.append(key)

    def get_selected_processor_and_duration(self, layer_name):
        layer = self.layer_dict[layer_name]
        bottom_layer_list = layer.get_bottom_layer_list()
        candidate_dict = {}
        for key in layer.est_info_dict:
            if key in self.processor_list:
                exe_time = layer.est_info_dict[key]
                mem_transition_time = 0
                bottom_end_time_list = [0]
                for bottom_key in bottom_layer_list:
                    bottom_processor = self.layer_processor_mapping_dict[bottom_key]
                    if key == bottom_processor:
                        mem_transition_time += 0
                    elif bottom_processor in self.mem_transition_time_dict:
                        mem_transition_time += self.mem_transition_time_dict[bottom_processor][key](bottom_key)
                        # mem_transition_time += self.dic_for_processor[key](bottom_key)
                    bottom_end_time_list.append(self.layer_end_time_dict[bottom_key])
                candidate_list = []
                start_time = mem_transition_time + max(self.processor_curr_time_dict[key], max(bottom_end_time_list))
                end_time = start_time + exe_time 
                candidate_list.append(end_time)
                candidate_list.append(start_time)
                candidate_list.append(mem_transition_time)
                candidate_dict[key] = candidate_list
                # max(self.processor_curr_time_dict[key], self.layer_end_time_dict[bottom_key]) + exe_time + mem_transition_time
        key_min = min(candidate_dict.keys(), key=(lambda k: candidate_dict[k][0]))
        return key_min, candidate_dict[key_min][1], candidate_dict[key_min][0], candidate_dict[key_min][2]

    def do_schedule(self):
        self.ready_list.append(self.start_layer_name)
        while len(self.ready_list) != 0:
            layer_name = self.ready_list.pop(0)
            layer = self.layer_dict[layer_name]
            key_min, start_time, end_time, mem_transition_time = self.get_selected_processor_and_duration(layer_name)
            # layer_name, processor_name, start_time, end_time, mem_transition_time
            self.result_list.append((layer_name, key_min, start_time, end_time, mem_transition_time))
            self.layer_processor_mapping_dict[layer_name] = key_min
            self.layer_end_time_dict[layer_name] = end_time
            self.processor_curr_time_dict[key_min] += (end_time - start_time + mem_transition_time)
            self.visited_dict[layer_name] = True
            self.update_ready_list()
            
    def print_result(self):
        gantt = GanttChart('result.png', self.processor_list)
        gantt.add_tasks_all(self.result_list)
        gantt.draw_gantt_chart()
        # print(self.result_list)
