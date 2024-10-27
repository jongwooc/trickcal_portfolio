#include "gurobi_c++.h"
#include <vector>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "DS.pb.h"
#include "caffe.pb.h"

float const_comm_init = 0;
float const_comm_delay = 1;

float map_alpha = 0.0053;
float map_beta = 569.25;
float unmap_alpha = 0.0024;
float unmap_beta = 526.39;

struct result {
    std::string layer_name;
    std::string mapped;
    float start_time;
    float end_time;
    float comm_time;
};

std::vector<result> return_result   (std::vector<result> _result_set)
{
    return _result_set;
}


int main(int argc, char *argv[])
{
    int mode = 0;
    int num_of_PE = 0;
    int num_of_CPU = 0;
    DS::network * time = new DS::network;
    caffe::NetParameter * caffe_model = new caffe::NetParameter;
    
    //PE profile time
    std::vector<float> cpu_time;
    std::vector<float> gpu_time;
    std::vector<float> dsp_time;
    std::vector<float> npu_time;
    std::vector<float> cpu2_time;
    std::vector<float> cpu3_time;
    std::vector<float> cpu4_time;
    std::vector<std::vector<float>> layer_comm;
    
    
    std::vector<GRBVar> layer_cpu;
    std::vector<GRBVar> layer_cpu2;
    std::vector<GRBVar> layer_cpu3;
    std::vector<GRBVar> layer_cpu4;
    std::vector<GRBVar> layer_gpu;
    std::vector<GRBVar> layer_npu;
    std::vector<GRBVar> layer_dsp;
    //starting cluster does not need to be seperated. So, cpu = cpu2 cluster here, as cpu cluster is the starting cluster.
    //new cpu cluster
    std::vector<GRBVar> pre_cpu;
    std::vector<GRBVar> not_pre_cpu;
    std::vector<GRBVar> CPU_cluster;
    std::vector<GRBVar> to_CPU_transition;
    std::vector<GRBVar> to_CPU_comm;
    //new cpu2 cluster
    std::vector<GRBVar> pre_cpu2;
    std::vector<GRBVar> not_pre_cpu2;
    std::vector<GRBVar> CPU2_cluster;
    std::vector<GRBVar> to_CPU2_transition;
    std::vector<GRBVar> to_CPU2_comm;
    //new cpu3 cluster
    std::vector<GRBVar> pre_cpu3;
    std::vector<GRBVar> not_pre_cpu3;
    std::vector<GRBVar> CPU3_cluster;
    std::vector<GRBVar> to_CPU3_transition;
    std::vector<GRBVar> to_CPU3_comm;
    //new cpu4 cluster
    std::vector<GRBVar> pre_cpu4;
    std::vector<GRBVar> not_pre_cpu4;
    std::vector<GRBVar> CPU4_cluster;
    std::vector<GRBVar> to_CPU4_transition;
    std::vector<GRBVar> to_CPU4_comm;
    //new gpu cluster
    std::vector<GRBVar> pre_gpu;
    std::vector<GRBVar> not_pre_gpu;
    std::vector<GRBVar> GPU_cluster;
    std::vector<GRBVar> to_GPU_transition;
    std::vector<GRBVar> to_GPU_comm;
    
    //new npu cluster
    std::vector<GRBVar> pre_npu;
    std::vector<GRBVar> not_pre_npu;
    std::vector<GRBVar> NPU_cluster;
    std::vector<GRBVar> to_NPU_transition;
    std::vector<GRBVar> to_NPU_comm;
    //new dsp cluster
    std::vector<GRBVar> pre_dsp;
    std::vector<GRBVar> not_pre_dsp;
    std::vector<GRBVar> DSP_cluster;
    std::vector<GRBVar> to_DSP_transition;
    std::vector<GRBVar> to_DSP_comm;
    
    //transitions
    std::vector<GRBVar> transition;
    
    std::vector<GRBVar> map;
    std::vector<GRBVar> unmap;
    
    //time variables for ilp
    std::vector<std::string> layer;
    std::vector<std::vector<std::string>> bottom_layer;
    std::vector<GRBVar> layer_start;
    std::vector<GRBVar> layer_end;
    // [ith layer][jth layer][0] -> ith layer is directly linked && previous layer of jth previous layer, binary
    
    // [ith layer][jth layer][1] -> ith layer && jth layer is both mapped at same PE, binary
    // [ith layer][jth layer][2] -> ith layer && jth layer is both mapped at CPU, binary
    // [ith layer][jth layer][3] -> ith layer && jth layer is both mapped at GPU, binary
    // [ith layer][jth layer][4] -> ith layer && jth layer is both mapped at CPU2, binary
    // [ith layer][jth layer][5] -> ith layer && jth layer is both mapped at CPU3, binary
    // [ith layer][jth layer][6] -> ith layer && jth layer is both mapped at CPU4, binary
    std::vector<std::vector<std::vector<GRBVar>>> condition;
    std::vector<std::pair<std::string,std::vector<int>>> mem_size;
    std::vector<std::vector<result>> result_set;
    std::vector<float> throughput_set;
    
    if (argc != 4)  {
        std::cerr << "Usage:  " << argv[0] << " [caffe_model].prototxt [layer_time_info].prototxt [mode]0/1" << std::endl;
        return -1;
    }
    
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    int model_descriptor = open(argv[1], O_RDONLY);
    mode = std::stoi(argv[3]);
    std::cerr << "mode is " <<mode << std::endl;
    
    google::protobuf::io::FileInputStream model_input(model_descriptor);
    model_input.SetCloseOnDelete( true );
    
    if (!google::protobuf::TextFormat::Parse(&model_input, caffe_model))    {
        std::cerr << "Failed to parse model file!" << std::endl;
        return -1;
    }
    
    int time_descriptor = open(argv[2], O_RDONLY);
    google::protobuf::io::FileInputStream time_input(time_descriptor);
    time_input.SetCloseOnDelete( true );
    
    if (!google::protobuf::TextFormat::Parse(&time_input, time))    {
        std::cerr << "Failed to parse time file!" << std::endl;
        return -1;
    }
    
    
    //check PE num
    if (time->layer(0).has_cpu())    {
        num_of_PE++;
        num_of_CPU++;
    }
    if (time->layer(0).has_gpu())   {
        num_of_PE++;
    }
    if (time->layer(0).has_dsp())    {
        num_of_PE++;
    }
    if (time->layer(0).has_npu())   {
        num_of_PE++;
    }
    if (time->layer(0).has_cpu2())    {
        num_of_PE++;
        num_of_CPU++;
    }
    if (time->layer(0).has_cpu3())    {
        num_of_PE++;
        num_of_CPU++;
    }
    if (time->layer(0).has_cpu4())    {
        num_of_PE++;
        num_of_CPU++;
    }
    
    int temp_i = 0;
    //set layers based on time estimations.
    for (int i = 0; i < caffe_model->layer_size(); i ++)  {
        if (caffe_model->layer(i).name() == time->layer(temp_i).name()) {
            layer.push_back(caffe_model->layer(i).name());
            //set pe time here
            if (time->layer(temp_i).has_cpu())    {
                cpu_time.push_back(time->layer(temp_i).cpu());
            }
            if (time->layer(temp_i).has_gpu())   {
                gpu_time.push_back(time->layer(temp_i).gpu());
            }
            if (time->layer(temp_i).has_dsp())    {
                dsp_time.push_back(time->layer(temp_i).dsp());
            }
            if (time->layer(temp_i).has_npu())   {
                npu_time.push_back(time->layer(temp_i).npu());
            }
            if (time->layer(temp_i).has_cpu2())    {
                cpu2_time.push_back(time->layer(temp_i).cpu2());
            }
            if (time->layer(temp_i).has_cpu3())    {
                cpu3_time.push_back(time->layer(temp_i).cpu3());
            }
            if (time->layer(temp_i).has_cpu4())    {
                cpu4_time.push_back(time->layer(temp_i).cpu4());
            }
            temp_i++;

            std::vector<std::string> temp_bottom;
            std::vector<std::string> temp_bottom_type;
            if (caffe_model->layer(i).bottom_size())    {
                for (int k = 0; k < caffe_model->layer(i).bottom_size(); k ++)   {
                    temp_bottom.push_back(caffe_model->layer(i).bottom(k));
                }
            }
            else    {
                temp_bottom.push_back("start");
            }
            
            for (int j = 0; j < i; j++)   {
                for (int k = 0; k < temp_bottom.size(); k ++)    {
                    if  (temp_bottom[k] == caffe_model->layer(j).name())   {
                        temp_bottom_type.push_back(caffe_model->layer(j).type());
                    }
                }
            }
            
            int while_switch = 1;
            if (temp_bottom_type.size() < 1)    {
                while_switch = 0;
            }
            while (while_switch)   {
                for (int j = 0; j < std::min(2,(int)temp_bottom_type.size()); j ++)   {
                    if (temp_bottom_type[temp_bottom_type.size() -1 -j] == "Concat")    {
                        std::string temp_layer_name = temp_bottom[temp_bottom.size() -1 -j];
                        temp_bottom.pop_back();
                        for (int k = 0; k < i; k ++)   {
                            if  (temp_layer_name == caffe_model->layer(k).name())   {
                                for (int l = 0; l < caffe_model->layer(k).bottom_size(); l ++)   {
                                    temp_bottom.push_back(caffe_model->layer(k).bottom(l));
                                }
                            }
                        }
                    }
                }
                temp_bottom_type.clear();
                for (int j = 0; j < temp_bottom.size(); j++)   {
                    for (int k = 0; k < i; k ++)    {
                        if  (temp_bottom[j] == caffe_model->layer(k).name())   {
                            temp_bottom_type.push_back(caffe_model->layer(k).type());
                        }
                    }
                }
                if (temp_bottom.size() <= 1)   {
                    while_switch = 0;
                }
                else    {
                    if (temp_bottom_type[temp_bottom_type.size()-1] != "Concat" && temp_bottom_type[temp_bottom_type.size()-2] != "Concat")   {
                        while_switch = 0;
                    }
                }
            }

            bottom_layer.push_back(temp_bottom);
        }
    }
    
    // check whether all the time layers are set to layers.
    if (time->layer_size() != layer.size()){
        std::cerr << "Model layers are not matching with time layers!  Check the naming of layers first!" << std::endl;
        return -2;
    }
    
    //set all the initial previous_contidions 0
    for (int i = 0; i < layer.size(); i++){
        std::vector<float> temp_float_vector;
        temp_float_vector.reserve(layer.size());
        
        for (int j = 0; j < layer.size(); j++){
            temp_float_vector.push_back(0.0);
        }
        layer_comm.push_back(temp_float_vector);
    }
    
    //set initial data size
    if (caffe_model->layer(0).has_input_param())    {
        int temp1 = caffe_model->layer(0).input_param().shape(0).dim(1);
        int temp2 = caffe_model->layer(0).input_param().shape(0).dim(2);
        int temp3 = caffe_model->layer(0).input_param().shape(0).dim(3);
        std::vector<int> size = {temp1,temp2,temp3};
        mem_size.push_back(std::make_pair(caffe_model->layer(0).name(),size));
    }
    else if (caffe_model->layer(0).has_transform_param())   {
        int temp2 = caffe_model->layer(0).transform_param().crop_size();
        std::vector<int> size = {3,temp2,temp2};
        mem_size.push_back(std::make_pair(caffe_model->layer(0).name(),size));
    }
    
    //(width_or_height + 2 * pad - kernel_size) / stride + 1
    for (int i = 1; i < caffe_model->layer_size(); i ++) {
        unsigned int temp_output = 0;
        unsigned int temp_width = 0;
        unsigned int temp_height = 0;
        
        for(int k = 0; k < caffe_model->layer(i).bottom_size(); k++)   {
            
            if (mem_size[i-k-1].second[0] > temp_output)  {
                temp_output = mem_size[i-1].second[0];
            }
            if (mem_size[i-k-1].second[1] > temp_width)  {
                temp_width = mem_size[i-1].second[1];
            }
            if (mem_size[i-k-1].second[2] > temp_height)  {
                temp_height = mem_size[i-1].second[2];
            }
        }
        
        //conv_param
        if (caffe_model->layer(i).convolution_param().has_num_output())    {
            temp_output = caffe_model->layer(i).convolution_param().num_output();
        }
        if (caffe_model->layer(i).convolution_param().kernel_size_size())    {
            temp_width -= caffe_model->layer(i).convolution_param().kernel_size(0);
            temp_height -= caffe_model->layer(i).convolution_param().kernel_size(0);
        }
        if (caffe_model->layer(i).convolution_param().pad_size())    {
            temp_width += (2 * caffe_model->layer(i).convolution_param().pad(0));
            temp_height += (2 * caffe_model->layer(i).convolution_param().pad(0));
        }
        if (caffe_model->layer(i).convolution_param().stride_size())    {
            temp_width /= caffe_model->layer(i).convolution_param().stride(0);
            temp_height /= caffe_model->layer(i).convolution_param().stride(0);
        }
        if (caffe_model->layer(i).convolution_param().kernel_size_size())    {
            temp_width += 1;
            temp_height += 1;
        }
        //pooling_param
        if (caffe_model->layer(i).has_pooling_param())    {
            for (int j = 1; j < layer.size(); j ++)    {
                if (caffe_model->layer(i).name() == layer[j])  {
                    temp_output *= bottom_layer[j].size();
                }
            }
            temp_width -= caffe_model->layer(i).pooling_param().kernel_size();
            temp_height -= caffe_model->layer(i).pooling_param().kernel_size();
            
            temp_width += (2 * caffe_model->layer(i). pooling_param().pad());
            temp_height += (2 * caffe_model->layer(i).pooling_param().pad());
            
            temp_width /= caffe_model->layer(i).pooling_param().stride();
            temp_height /= caffe_model->layer(i).pooling_param().stride();
            temp_width += 1;
            temp_height += 1;
        }
        
        //inner_product_param
        if (caffe_model->layer(i).has_inner_product_param())    {
            temp_output = caffe_model->layer(i).inner_product_param().num_output();
            temp_width = 1;
            temp_height = 1;
        }
        std::vector<int> size = {(int)temp_output,(int)temp_width,(int)temp_height};
        std::cerr <<"layer : "<<caffe_model->layer(i).name() <<"out : " <<temp_output <<"width : " << temp_width << "height : " <<temp_height<<std::endl;
        mem_size.push_back(std::make_pair(caffe_model->layer(i).name(),size));
    }
    for (int i = 0; i < bottom_layer.size(); i ++)  {
        for (int j = 0; j < layer.size(); j ++)  {
            for (int l = 0; l < bottom_layer[i].size(); l ++)  {
                if (bottom_layer[i][l] == layer[j]) {
                    for (int k = 0; k < mem_size.size(); k ++)  {
                        if (mem_size[k].first == layer[j]) {
                            std:: cout <<"mem_size layer : " << mem_size[k].first << std::endl;
                            float temp_commtime = 0;
                            temp_commtime = (mem_size[k].second[0] * mem_size[k].second[1] * mem_size[k].second[2]);
                            std:: cout << layer[j] << " to " << layer[i] << "mem_size : " << temp_commtime << std::endl;
                            layer_comm[j][i] = temp_commtime;
                        }
                    }
                    std:: cerr << "map : " <<(map_alpha * layer_comm[j][i] + map_beta) << '\t';
                    std:: cerr << "unmap : " <<(unmap_alpha * layer_comm[j][i] + unmap_beta) << std::endl;
                }
            }
        }
    }
    
    for (int i = 0; i < layer.size(); i++){
        std::vector<GRBVar> temp_condition_set;
        temp_condition_set.reserve(num_of_PE+2);
        
        std::vector<std::vector<GRBVar>> temp_GRB_vector;
        temp_GRB_vector.reserve(layer.size());
        
        for (int j = 0; j < layer.size(); j++){
            temp_GRB_vector.push_back(temp_condition_set);
        }
        condition.push_back(temp_GRB_vector);
    }
    //gurobi start
    try {
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        
        // Create variables
        if (cpu_time.size())    {
            //CPU mapped
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_cpu";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_cpu.push_back(cpu);
                //pre_cpu
                for (int i = 0; i < layer.size(); i ++){
                    std::string temp_string = layer[i];
                    temp_string = "previous_layer_of_" + temp_string + "_cpu";
                    GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                    pre_cpu.push_back(cpu);
                }
                //not_pre_cpu
                for (int i = 0; i < layer.size(); i ++){
                    std::string temp_string = layer[i];
                    temp_string = "previous_" + temp_string + "_not_cpu";
                    GRBVar not_cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                    not_pre_cpu.push_back(not_cpu);
                }
                //CPU_cluster
                for (int i = 0; i < layer.size(); i ++){
                    std::string temp_string = layer[i];
                    temp_string = temp_string + "_CPUclust";
                    GRBVar clust = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                    CPU_cluster.push_back(clust);
                }
                //to_CPU_transition
                for (int i = 0; i < layer.size(); i ++){
                    std::string temp_string = layer[i];
                    temp_string = temp_string + "_to_CPU_transition";
                    GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                    to_CPU_transition.push_back(trans);
                }
                //to_CPU_comm
                for (int i = 0; i < layer.size(); i ++){
                    std::string temp_string = layer[i];
                    temp_string = temp_string + "_to_CPU_comm";
                    GRBVar comm = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                    to_CPU_comm.push_back(comm);
                }
            }
        }
        if (cpu2_time.size())    {
            //CPU2 mapped
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_cpu2";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_cpu2.push_back(cpu);
            }
            //pre_cpu2
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_layer_of_" + temp_string + "_cpu2";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                pre_cpu2.push_back(cpu);
            }
            //not_pre_cpu2
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_" + temp_string + "_not_cpu2";
                GRBVar not_cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                not_pre_cpu2.push_back(not_cpu);
            }
            //CPU2_cluster
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_CPU2clust";
                GRBVar clust = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                CPU2_cluster.push_back(clust);
            }
            //to_CPU2_transition
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU2_transition";
                GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_CPU2_transition.push_back(trans);
            }
            //to_CPU2_comm
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU2_comm";
                GRBVar comm = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_CPU2_comm.push_back(comm);
            }
        }
        if (cpu3_time.size())    {
            //CPU3 mapped
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_cpu3";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_cpu3.push_back(cpu);
            }
            //pre_cpu3
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_layer_of_" + temp_string + "_cpu3";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                pre_cpu3.push_back(cpu);
            }
            //not_pre_cpu3
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_" + temp_string + "_not_cpu3";
                GRBVar not_cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                not_pre_cpu3.push_back(not_cpu);
            }
            //CPU3_cluster
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_CPU3clust";
                GRBVar clust = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                CPU3_cluster.push_back(clust);
            }
            //to_CPU3_transition
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU3_transition";
                GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_CPU3_transition.push_back(trans);
            }
            //to_CPU3_comm
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU3_comm";
                GRBVar comm = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_CPU3_comm.push_back(comm);
            }
        }
        if (cpu4_time.size())    {
            //CPU4 mapped
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_cpu4";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_cpu4.push_back(cpu);
            }
            //pre_cpu4
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_layer_of_" + temp_string + "_cpu4";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                pre_cpu4.push_back(cpu);
            }
            //not_pre_cpu4
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_" + temp_string + "_not_cpu4";
                GRBVar not_cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                not_pre_cpu4.push_back(not_cpu);
            }
            //CPU4_cluster
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_CPU4clust";
                GRBVar clust = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                CPU4_cluster.push_back(clust);
            }
            //to_CPU4_transition
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU4_transition";
                GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_CPU4_transition.push_back(trans);
            }
            //to_CPU4_comm
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU4_comm";
                GRBVar comm = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_CPU4_comm.push_back(comm);
            }
        }
        
        if (gpu_time.size())    {
            //GPU mapped
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_gpu";
                GRBVar gpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_gpu.push_back(gpu);
            }
            //pre_gpu
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_layer_of_" + temp_string + "_gpu";
                GRBVar gpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                pre_gpu.push_back(gpu);
            }
            //not_pre_gpu
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_" + temp_string + "_not_gpu";
                GRBVar not_gpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                not_pre_gpu.push_back(not_gpu);
            }
            //GPU_cluster
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_GPUclust";
                GRBVar clust = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                GPU_cluster.push_back(clust);
            }
            //to_GPU_transition
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_GPU_transition";
                GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_GPU_transition.push_back(trans);
            }
            //to_GPU_comm
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_GPU_comm";
                GRBVar comm = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_GPU_comm.push_back(comm);
            }
        }
        
        if (npu_time.size())    {
            //NPU mapped
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_npu";
                GRBVar npu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_npu.push_back(npu);
            }
            //pre_npu
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_layer_of_" + temp_string + "_npu";
                GRBVar npu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                pre_npu.push_back(npu);
            }
            //not_pre_npu
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_" + temp_string + "_not_npu";
                GRBVar not_npu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                not_pre_npu.push_back(not_npu);
            }
            //NPU_cluster
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_NPUclust";
                GRBVar clust = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                NPU_cluster.push_back(clust);
            }
            //to_NPU_transition
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_NPU_transition";
                GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_NPU_transition.push_back(trans);
            }
            //to_NPU_comm
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU_comm";
                GRBVar comm = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_NPU_comm.push_back(comm);
            }
        }
        
        if (dsp_time.size())    {
            //GPU mapped
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_dsp";
                GRBVar dsp = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_dsp.push_back(dsp);
            }
            //pre_dsp
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_layer_of_" + temp_string + "_dsp";
                GRBVar dsp = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                pre_dsp.push_back(dsp);
            }
            //not_pre_dsp
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = "previous_" + temp_string + "_not_dsp";
                GRBVar not_dsp = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                not_pre_dsp.push_back(not_dsp);
            }
            //DSP_cluster
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_DSPclust";
                GRBVar clust = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                DSP_cluster.push_back(clust);
            }
            //to_DSP_transition
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_DSP_transition";
                GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_DSP_transition.push_back(trans);
            }
            //to_DSP_comm
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_to_CPU_comm";
                GRBVar comm = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                to_DSP_comm.push_back(comm);
            }
        }
        
        //transition
        for (int i = 0; i < layer.size(); i ++){
            std::string temp_string = layer[i];
            temp_string = temp_string + "_trasition";
            GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
            transition.push_back(trans);
        }
        for (int i = 0; i < layer.size(); i ++){
            std::string temp_string = layer[i];
            temp_string = temp_string + "_map";
            GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
            map.push_back(trans);
        }
        for (int i = 0; i < layer.size(); i ++){
            std::string temp_string = layer[i];
            temp_string = temp_string + "_unmap";
            GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
            unmap.push_back(trans);
        }
        
        //start_time
        for (int i = 0; i < layer.size(); i ++){
            std::string temp_string = layer[i];
            temp_string = temp_string + "_start";
            GRBVar start_time = model.addVar(0.0, 999999999.0, 0.0, GRB_CONTINUOUS, temp_string);
            layer_start.push_back(start_time);
        }
        
        //end_time
        for (int i = 0; i < layer.size(); i ++){
            std::string temp_string = layer[i];
            temp_string = temp_string + "_end";
            GRBVar end_time = model.addVar(0.0, 999999999.0, 0.0, GRB_CONTINUOUS, temp_string);
            layer_end.push_back(end_time);
        }
        
        //conditions
        for (int i = 0; i < layer.size(); i ++){
            for (int j = 0; j < layer.size(); j ++){
                std::string temp_string1 = layer[i];
                std::string temp_string2 = layer[j];
                std::string temp_string = temp_string1 + "_is_previous_of_" + temp_string2;
                GRBVar ijpre = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                condition[i][j].push_back(ijpre);
                temp_string = temp_string1 + "_and_" + temp_string2 + "_are_same_PE";
                GRBVar ijsame = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                condition[i][j].push_back(ijsame);
                temp_string = temp_string1 + "_and_" + temp_string2 + "_are_CPU";
                GRBVar ijcpu = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                condition[i][j].push_back(ijcpu);
                temp_string = temp_string1 + "_and_" + temp_string2 + "_are_GPU";
                GRBVar ijgpu = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                condition[i][j].push_back(ijgpu);
                temp_string = temp_string1 + "_and_" + temp_string2 + "_are_CPU2";
                GRBVar ijcpu2 = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                condition[i][j].push_back(ijcpu2);
                temp_string = temp_string1 + "_and_" + temp_string2 + "_are_CPU3";
                GRBVar ijcpu3 = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                condition[i][j].push_back(ijcpu3);
                temp_string = temp_string1 + "_and_" + temp_string2 + "_are_CPU4";
                GRBVar ijcpu4 = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                condition[i][j].push_back(ijcpu4);
            }
        }
        
        //Add max variable
        GRBVar MAX_ob = model.addVar(0.0, 999999999.0, 0.0, GRB_CONTINUOUS, "max (PE)");
        GRBVar min_ob = model.addVar(0.0, 999999999.0, 0.0, GRB_CONTINUOUS, "min (PE)");
        
        // Add constraint: set preij
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < layer.size(); j ++)   {
                float temp_binary = 0.0;
                if (layer_comm[i][j]) {
                    temp_binary = 1.0;
                }
                std::string temp_string = layer[i];
                temp_string = temp_string + " " + layer[j] + "link";
                GRBLinExpr addConstrlhs = 0;
                addConstrlhs += condition[i][j][0];
                model.addConstr(addConstrlhs,'=',temp_binary, temp_string);
            }
        }
        
        GRBLinExpr tempConstrlhs = 0;
        
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_gpu[0];
        model.addConstr(tempConstrlhs,'=',0, "data_does_not_start_gpu");
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_gpu[layer.size()-1];
        model.addConstr(tempConstrlhs,'=',0, "data_does_not_end_gpu");
        

        tempConstrlhs = 0;
        tempConstrlhs += layer_gpu[1];
        model.addConstr(tempConstrlhs,'=',0, "force gpu setting1");

        tempConstrlhs = 0;
        tempConstrlhs += layer_gpu[2];
        model.addConstr(tempConstrlhs,'=',0, "force gpu setting2");

	// Add constraint: layer_PE[i] = 1 guaruntee node has one processor
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string = layer[i];
            temp_string = temp_string + "_node";
            GRBLinExpr addConstrlhs = 0;
            addConstrlhs += layer_cpu[i] + layer_gpu[i];
            if (layer_cpu2.size())  {
                addConstrlhs += layer_cpu2[i];
            }
            if (layer_cpu3.size())  {
                addConstrlhs += layer_cpu3[i];
            }
            if (layer_cpu4.size())  {
                addConstrlhs += layer_cpu4[i];
            }
            if (layer_npu.size())  {
                addConstrlhs += layer_npu[i];
            }
            if (layer_dsp.size())  {
                addConstrlhs += layer_dsp[i];
            }
            model.addConstr(addConstrlhs,'=',1, temp_string);
        }
        
        // Add constraint: 0 = layer0_start_time;
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_start[0];
        model.addConstr(tempConstrlhs,'=',0, "layer0_start_time");
        
        // Add constraint: layer_start+ layer_gpu_n*gputim + layer_cpu_n*cputime = layer_end;
        for (int i = 0; i < layer.size(); i ++)   {
            GRBLinExpr addConstrlhs = 0;
            addConstrlhs += (layer_start[i] + layer_cpu[i] * cpu_time[i] + layer_gpu[i] * gpu_time[i]);
            if (layer_cpu2.size())  {
                addConstrlhs += layer_cpu2[i] * cpu2_time[i];
            }
            if (layer_cpu3.size())  {
                addConstrlhs += layer_cpu3[i] * cpu3_time[i];
            }
            if (layer_cpu4.size())  {
                addConstrlhs += layer_cpu4[i] * cpu4_time[i];
            }
            if (layer_npu.size())  {
                addConstrlhs += layer_npu[i] * npu_time[i];
            }
            if (layer_dsp.size())  {
                addConstrlhs += layer_dsp[i] * dsp_time[i];
            }
            model.addConstr(addConstrlhs,'=',layer_end[i], "layer_end_time");
        }
        
        // Add constraint: pre_cpu
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "pre_CPU";
                    GRBLinExpr addConstrlhs0 = 0;
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        addConstrlhs0 += layer_cpu[j+k];
                    }
                    model.addConstr(addConstrlhs0,'>',pre_cpu[i],temp_string0);
                    j = layer.size();
                }
            }
        }
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_pre_CPU_"+layer[j+k];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += layer_cpu[j+k];
                        model.addConstr(addConstrlhs0,'<',pre_cpu[i],temp_string0);
                    }
                    j = layer.size();
                }
            }
        }
        //not_pre_cpu
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "_pre_~CPU";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += pre_cpu[i] + not_pre_cpu[i];
            model.addConstr(addConstrlhs0,'=',1,temp_string0);
        }
        // Add constraint: CPU_cluster
        for (int i = 1; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "CPU_cluster_start1";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i] + not_pre_cpu[i] - 1;
            model.addConstr(addConstrlhs0,'<',CPU_cluster[i],temp_string0);
        }
        for (int i = 1; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "CPU_cluster_start2";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i];
            model.addConstr(addConstrlhs0,'>',CPU_cluster[i],temp_string0);
        }
        for (int i = 1; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "CPU_cluster_start3";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += not_pre_cpu[i];
            model.addConstr(addConstrlhs0,'>',CPU_cluster[i],temp_string0);
        }
        
        // Add constraint: to_cpu_transition
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "_to_CPU";
            GRBLinExpr addConstrlhs0 = 0;
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    addConstrlhs0 += (1-layer_cpu[j]);
                }
            }
            model.addConstr(addConstrlhs0,'>',to_CPU_transition[i],temp_string0);
        }
        
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "_to_CPU_"+layer[j];
                    GRBLinExpr addConstrlhs0 = 0;
                    addConstrlhs0 += (1-layer_cpu[j]);
                    model.addConstr(addConstrlhs0,'<',to_CPU_transition[i],temp_string0);
                }
            }
        }
        //to_CPU_comm
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU_comm1";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i] + to_CPU_transition[i] - 1;
            model.addConstr(addConstrlhs0,'<',to_CPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU_comm2";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i];
            model.addConstr(addConstrlhs0,'>',to_CPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU_comm3";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += to_CPU_transition[i];
            model.addConstr(addConstrlhs0,'>',to_CPU_comm[i],temp_string0);
        }
        
        // Add constraint: pre_gpu
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "pre_GPU";
                    GRBLinExpr addConstrlhs0 = 0;
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        addConstrlhs0 += layer_gpu[j+k];
                    }
                    model.addConstr(addConstrlhs0,'>',pre_gpu[i],temp_string0);
                    j = layer.size();
                }
            }
        }
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_pre_GPU_"+layer[j+k];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += layer_gpu[j+k];
                        model.addConstr(addConstrlhs0,'<',pre_gpu[i],temp_string0);
                    }
                    j = layer.size();
                }
            }
        }
        //not_pre_gpu
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "_pre_~GPU";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += pre_gpu[i] + not_pre_gpu[i];
            model.addConstr(addConstrlhs0,'=',1,temp_string0);
        }
        // Add constraint: GPU_cluster
        for (int i = 1; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "GPU_cluster_start1";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i] + not_pre_gpu[i] - 1;
            model.addConstr(addConstrlhs0,'<',GPU_cluster[i],temp_string0);
        }
        for (int i = 1; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "GPU_cluster_start2";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i];
            model.addConstr(addConstrlhs0,'>',GPU_cluster[i],temp_string0);
        }
        for (int i = 1; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "GPU_cluster_start3";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += not_pre_gpu[i];
            model.addConstr(addConstrlhs0,'>',GPU_cluster[i],temp_string0);
        }
        //only one GPU cluster
        tempConstrlhs = 0;
        for (int i = 0; i < layer.size(); i ++)   {
            tempConstrlhs += GPU_cluster[i];
        }
        model.addConstr(tempConstrlhs,'=',1,"one GPU cluster");
        // Add constraint: to_GPU_transition
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "_to_GPU";
            GRBLinExpr addConstrlhs0 = 0;
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    addConstrlhs0 += (1-layer_gpu[j]);
                }
            }
            model.addConstr(addConstrlhs0,'>',to_GPU_transition[i],temp_string0);
        }
        
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "_to_GPU_"+layer[j];
                    GRBLinExpr addConstrlhs0 = 0;
                    addConstrlhs0 += (1-layer_gpu[j]);
                    model.addConstr(addConstrlhs0,'<',to_GPU_transition[i],temp_string0);
                }
            }
        }
        //to_GPU_comm
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_GPU_comm1";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i] + to_GPU_transition[i] - 1;
            model.addConstr(addConstrlhs0,'<',to_GPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_GPU_comm2";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i];
            model.addConstr(addConstrlhs0,'>',to_GPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_GPU_comm3";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += to_GPU_transition[i];
            model.addConstr(addConstrlhs0,'>',to_GPU_comm[i],temp_string0);
        }
        if (layer_cpu2.size())  {
            // Add constraint: pre_cpu2
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "pre_CPU2";
                        GRBLinExpr addConstrlhs0 = 0;
                        for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                            addConstrlhs0 += layer_cpu2[j+k];
                        }
                        model.addConstr(addConstrlhs0,'>',pre_cpu2[i],temp_string0);
                        j = layer.size();
                    }
                }
            }
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                            std::string temp_string0 = layer[i];
                            temp_string0 = temp_string0 + "_pre_CPU2_"+layer[j+k];
                            GRBLinExpr addConstrlhs0 = 0;
                            addConstrlhs0 += layer_cpu2[j+k];
                            model.addConstr(addConstrlhs0,'<',pre_cpu2[i],temp_string0);
                        }
                        j = layer.size();
                    }
                }
            }
            //not_pre_cpu2
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "_pre_~CPU2";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += pre_cpu2[i] + not_pre_cpu2[i];
                model.addConstr(addConstrlhs0,'=',1,temp_string0);
            }
            // Add constraint: CPU2_cluster
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster2_start1";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu2[i] + not_pre_cpu2[i] - 1;
                model.addConstr(addConstrlhs0,'<',CPU2_cluster[i],temp_string0);
            }
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster2_start2";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu2[i];
                model.addConstr(addConstrlhs0,'>',CPU2_cluster[i],temp_string0);
            }
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster2_start3";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += not_pre_cpu2[i];
                model.addConstr(addConstrlhs0,'>',CPU2_cluster[i],temp_string0);
            }
            
            // Add constraint: to_CPU2_transition
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "_to_CPU2";
                GRBLinExpr addConstrlhs0 = 0;
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        addConstrlhs0 += (1-layer_cpu2[j]);
                    }
                }
                model.addConstr(addConstrlhs0,'>',to_CPU2_transition[i],temp_string0);
            }
            
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_to_CPU2_"+layer[j];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += (1-layer_cpu2[j]);
                        model.addConstr(addConstrlhs0,'<',to_CPU2_transition[i],temp_string0);
                    }
                }
            }
            //to_CPU2_comm
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU2_comm1";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu2[i] + to_CPU2_transition[i] - 1;
                model.addConstr(addConstrlhs0,'<',to_CPU2_comm[i],temp_string0);
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU2_comm2";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu2[i];
                model.addConstr(addConstrlhs0,'>',to_CPU2_comm[i],temp_string0);
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU2_comm3";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += to_CPU2_transition[i];
                model.addConstr(addConstrlhs0,'>',to_CPU2_comm[i],temp_string0);
            }
            
        }
        if (layer_cpu3.size())  {
            // Add constraint: pre_cpu3
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "pre_CPU3";
                        GRBLinExpr addConstrlhs0 = 0;
                        for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                            addConstrlhs0 += layer_cpu3[j+k];
                        }
                        model.addConstr(addConstrlhs0,'>',pre_cpu3[i],temp_string0);
                        j = layer.size();
                    }
                }
            }
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                            std::string temp_string0 = layer[i];
                            temp_string0 = temp_string0 + "_pre_CPU3_"+layer[j+k];
                            GRBLinExpr addConstrlhs0 = 0;
                            addConstrlhs0 += layer_cpu3[j+k];
                            model.addConstr(addConstrlhs0,'<',pre_cpu3[i],temp_string0);
                        }
                        j = layer.size();
                    }
                }
            }
            //not_pre_cpu3
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "_pre_~CPU3";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += pre_cpu3[i] + not_pre_cpu3[i];
                model.addConstr(addConstrlhs0,'=',1,temp_string0);
            }
            // Add constraint: CPU3_cluster
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster3_start1";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu3[i] + not_pre_cpu3[i] - 1;
                model.addConstr(addConstrlhs0,'<',CPU3_cluster[i],temp_string0);
            }
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster3_start2";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu3[i];
                model.addConstr(addConstrlhs0,'>',CPU3_cluster[i],temp_string0);
            }
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster3_start3";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += not_pre_cpu3[i];
                model.addConstr(addConstrlhs0,'>',CPU3_cluster[i],temp_string0);
            }
            
            // Add constraint: to_CPU3_transition
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "_to_CPU3";
                GRBLinExpr addConstrlhs0 = 0;
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        addConstrlhs0 += (1-layer_cpu3[j]);
                    }
                }
                model.addConstr(addConstrlhs0,'>',to_CPU3_transition[i],temp_string0);
            }
            
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_to_CPU3_"+layer[j];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += (1-layer_cpu3[j]);
                        model.addConstr(addConstrlhs0,'<',to_CPU3_transition[i],temp_string0);
                    }
                }
            }
            //to_CPU3_comm
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU3_comm1";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu3[i] + to_CPU3_transition[i] - 1;
                model.addConstr(addConstrlhs0,'<',to_CPU3_comm[i],temp_string0);
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU3_comm2";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu3[i];
                model.addConstr(addConstrlhs0,'>',to_CPU3_comm[i],temp_string0);
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU3_comm3";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += to_CPU3_transition[i];
                model.addConstr(addConstrlhs0,'>',to_CPU3_comm[i],temp_string0);
            }
            
        }
        if (layer_cpu4.size())  {
            // Add constraint: pre_cpu4
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "pre_CPU4";
                        GRBLinExpr addConstrlhs0 = 0;
                        for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                            addConstrlhs0 += layer_cpu4[j+k];
                        }
                        model.addConstr(addConstrlhs0,'>',pre_cpu4[i],temp_string0);
                        j = layer.size();
                    }
                }
            }
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                            std::string temp_string0 = layer[i];
                            temp_string0 = temp_string0 + "_pre_CPU4_"+layer[j+k];
                            GRBLinExpr addConstrlhs0 = 0;
                            addConstrlhs0 += layer_cpu4[j+k];
                            model.addConstr(addConstrlhs0,'<',pre_cpu4[i],temp_string0);
                        }
                        j = layer.size();
                    }
                }
            }
            //not_pre_cpu4
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "_pre_~CPU4";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += pre_cpu4[i] + not_pre_cpu4[i];
                model.addConstr(addConstrlhs0,'=',1,temp_string0);
            }
            // Add constraint: CPU4_cluster
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster4_start1";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu4[i] + not_pre_cpu4[i] - 1;
                model.addConstr(addConstrlhs0,'<',CPU4_cluster[i],temp_string0);
            }
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster4_start2";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu4[i];
                model.addConstr(addConstrlhs0,'>',CPU4_cluster[i],temp_string0);
            }
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "CPU_cluster4_start3";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += not_pre_cpu4[i];
                model.addConstr(addConstrlhs0,'>',CPU4_cluster[i],temp_string0);
            }
 
            //to_CPU4_transition
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "_to_CPU4";
                GRBLinExpr addConstrlhs0 = 0;
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        addConstrlhs0 += (1-layer_cpu4[j]);
                    }
                }
                model.addConstr(addConstrlhs0,'>',to_CPU4_transition[i],temp_string0);
            }
            
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    if (layer_comm[j][i]) {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_to_CPU4_"+layer[j];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += (1-layer_cpu4[j]);
                        model.addConstr(addConstrlhs0,'<',to_CPU4_transition[i],temp_string0);
                    }
                }
            }
            //to_CPU4_comm
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU4_comm1";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu4[i] + to_CPU4_transition[i] - 1;
                model.addConstr(addConstrlhs0,'<',to_CPU4_comm[i],temp_string0);
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU4_comm2";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += layer_cpu4[i];
                model.addConstr(addConstrlhs0,'>',to_CPU4_comm[i],temp_string0);
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::string temp_string0 = layer[i];
                temp_string0 = temp_string0 + "to_CPU4_comm3";
                GRBLinExpr addConstrlhs0 = 0;
                addConstrlhs0 += to_CPU4_transition[i];
                model.addConstr(addConstrlhs0,'>',to_CPU4_comm[i],temp_string0);
            }
            
        }
        
        
        tempConstrlhs = 0;
        tempConstrlhs += CPU_cluster[0];
        model.addConstr(tempConstrlhs,'=',layer_cpu[0],"start CPU cluster");
        
        if (layer_cpu2.size())  {
            tempConstrlhs = 0;
            tempConstrlhs += CPU2_cluster[0];
            model.addConstr(tempConstrlhs,'=',layer_cpu2[0],"start CPU2 cluster");
        }
        if (layer_cpu3.size())  {
            tempConstrlhs = 0;
            tempConstrlhs += CPU3_cluster[0];
            model.addConstr(tempConstrlhs,'=',layer_cpu3[0],"start CPU3 cluster");
        }
        if (layer_cpu4.size())  {
            tempConstrlhs = 0;
            tempConstrlhs += CPU4_cluster[0];
            model.addConstr(tempConstrlhs,'=',layer_cpu4[0],"start CPU4 cluster");
        }
        
        //cluster numbers
        tempConstrlhs = 0;
        for (int i = 0; i < layer.size(); i ++)   {
            tempConstrlhs += CPU_cluster[i];
            if (layer_cpu2.size())  {
                tempConstrlhs +=  CPU2_cluster[i];
            }
            if (layer_cpu3.size())  {
                tempConstrlhs +=  CPU3_cluster[i];
            }
            if (layer_cpu4.size())  {
                tempConstrlhs +=  CPU4_cluster[i];
            }
        }
        model.addConstr(tempConstrlhs,'=',2>(num_of_CPU)?2:(num_of_CPU),"total CPUs cluster");
        
        
        
        if (layer_cpu2.size())  {
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += CPU_cluster[i];
            }
            model.addConstr(tempConstrlhs,'<',1,"one CPU cluster");
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs +=  CPU2_cluster[i];
            }
            model.addConstr(tempConstrlhs,'<',1,"one CPU2 cluster");
        }
        if (layer_cpu3.size())  {
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs +=  CPU3_cluster[i];
            }
            model.addConstr(tempConstrlhs,'<',1,"one CPU3 cluster");
        }
        if (layer_cpu4.size())  {
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs +=  CPU4_cluster[i];
            }
            model.addConstr(tempConstrlhs,'<',1,"one CPU4 cluster");
        }
        //Add constraint: set map
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "map1";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += to_CPU_comm[i] + pre_gpu[i] - 1;
            if (layer_cpu2.size())  {
                addConstrlhs0 += to_CPU2_comm[i];
            }
            if (layer_cpu3.size())  {
                addConstrlhs0 += to_CPU3_comm[i];
            }
            if (layer_cpu4.size())  {
                addConstrlhs0 += to_CPU4_comm[i];
            }
            model.addConstr(addConstrlhs0,'<',map[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "map2";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += to_CPU_comm[i];
            if (layer_cpu2.size())  {
                addConstrlhs0 += to_CPU2_comm[i];
            }
            if (layer_cpu3.size())  {
                addConstrlhs0 += to_CPU3_comm[i];
            }
            if (layer_cpu4.size())  {
                addConstrlhs0 += to_CPU4_comm[i];
            }
            model.addConstr(addConstrlhs0,'>',map[i],temp_string0);
        }
        
//        for (int i = 0; i < layer.size(); i ++)   {
//            std::string temp_string0 = layer[i];
//            temp_string0 = temp_string0 + "map3";
//            GRBLinExpr addConstrlhs0 = 0;
//            addConstrlhs0 += pre_gpu[i];
//            model.addConstr(addConstrlhs0,'>',map[i],temp_string0);
//        }
        
        //Add constraint: set unmap
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "unmap1";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += to_GPU_comm[i];
            model.addConstr(addConstrlhs0,'=',unmap[i],temp_string0);
        }
        
        //Add constraint: set trasition
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "transition";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += map[i] + unmap[i];
            model.addConstr(addConstrlhs0,'=',transition[i],temp_string0);
        }
        
        // Add constraint: if i,j is connected, jstart - iend + commij * ijdiff >0
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string = layer[j];
                    temp_string = temp_string + "endtime_is_smaller_than" + layer[i] + "start";
                    GRBLinExpr addConstrlhs = 0;
                    addConstrlhs +=  layer_end[j];
                    for (int k = 0; k < i; k ++)   {
                        if (layer_comm[k][i]) {
                            addConstrlhs += (map[i] * (map_alpha * layer_comm[k][i] + map_beta) +  unmap[i] * (unmap_alpha * layer_comm[k][i] + unmap_beta));
                        }
                    }
                    model.addConstr(addConstrlhs,'<',layer_start[i], temp_string);
                }
            }
        }
        
        // Add constraint: ijcpu
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < layer.size(); j ++)   {
                std::string temp_string = layer[j];
                temp_string = temp_string + "_and_" + layer[i] + "_are_CPU_mapped1";
                GRBLinExpr addConstrlhs = 0;
                addConstrlhs += layer_cpu[i] + layer_cpu[j] - 1;
                model.addConstr(addConstrlhs,'<',condition[i][j][2], temp_string);
                
                temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU_mapped2";
                addConstrlhs = 0;
                addConstrlhs += layer_cpu[i];
                model.addConstr(addConstrlhs,'>',condition[i][j][2], temp_string);
                
                temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU_mapped3";
                addConstrlhs = 0;
                addConstrlhs += layer_cpu[j];
                model.addConstr(addConstrlhs,'>',condition[i][j][2], temp_string);
                
                temp_string = layer[j] + "_and_" + layer[i] + "_are_GPU_mapped1";
                addConstrlhs = 0;
                addConstrlhs += layer_gpu[i] + layer_gpu[j] - 1;
                model.addConstr(addConstrlhs,'<',condition[i][j][3], temp_string);
                
                temp_string = layer[j] + "_and_" + layer[i] + "_are_GPU_mapped2";
                addConstrlhs = 0;
                addConstrlhs += layer_gpu[i];
                model.addConstr(addConstrlhs,'>',condition[i][j][3], temp_string);
                
                temp_string = layer[j] + "_and_" + layer[i] + "_are_GPU_mapped3";
                addConstrlhs = 0;
                addConstrlhs += layer_gpu[j];
                model.addConstr(addConstrlhs,'>',condition[i][j][3], temp_string);
                
                if (layer_cpu2.size())  {
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU2_mapped1";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu2[i] + layer_cpu2[j] - 1;
                    model.addConstr(addConstrlhs,'<',condition[i][j][4], temp_string);
                    
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU2_mapped2";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu2[i];
                    model.addConstr(addConstrlhs,'>',condition[i][j][4], temp_string);
                    
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU2_mapped3";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu2[j];
                    model.addConstr(addConstrlhs,'>',condition[i][j][4], temp_string);
                }
                if (layer_cpu3.size())  {
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU3_mapped1";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu3[i] + layer_cpu3[j] - 1;
                    model.addConstr(addConstrlhs,'<',condition[i][j][5], temp_string);
                    
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU3_mapped2";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu3[i];
                    model.addConstr(addConstrlhs,'>',condition[i][j][5], temp_string);
                    
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU3_mapped3";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu3[j];
                    model.addConstr(addConstrlhs,'>',condition[i][j][5], temp_string);
                }
                if (layer_cpu4.size())  {
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU4_mapped1";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu4[i] + layer_cpu4[j] - 1;
                    model.addConstr(addConstrlhs,'<',condition[i][j][6], temp_string);
                    
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU4_mapped2";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu4[i];
                    model.addConstr(addConstrlhs,'>',condition[i][j][6], temp_string);
                    
                    temp_string = layer[j] + "_and_" + layer[i] + "_are_CPU4_mapped3";
                    addConstrlhs = 0;
                    addConstrlhs += layer_cpu4[j];
                    model.addConstr(addConstrlhs,'>',condition[i][j][6], temp_string);
                }
                
                temp_string = temp_string + "_and_" + layer[i] + "_are_same_PE";
                addConstrlhs = 0;
                addConstrlhs += condition[i][j][2] + condition[i][j][3];
                if (layer_cpu2.size())  {
                    addConstrlhs +=  condition[i][j][4];
                }
                if (layer_cpu3.size())  {
                    addConstrlhs +=  condition[i][j][5];
                }
                if (layer_cpu4.size())  {
                    addConstrlhs +=  condition[i][j][6];
                }
                model.addConstr(addConstrlhs,'=',condition[i][j][1], temp_string);
            }
        }
        
        // Add constraint: i,j is different or not? if mapped same processor, start time is greater than end time.
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                std::string temp_string = layer[j];
                temp_string = temp_string + "_are_mapped_same_place_with_" + layer[i];
                //                std::cout << "layer " << layer[j] <<" "<< layer[i] << "link " <<std::endl;
                GRBLinExpr addConstrlhs = 0;
                addConstrlhs += layer_start[i] - layer_end[j] +((1-condition[j][i][1]) * 4999999) ;
                model.addConstr(addConstrlhs,'>',0, temp_string);
            }
        }
        
        
        if (mode) {
            
            //            float map_alpha = 0.0053;
            //            float map_beta = 569.25;
            //            float unmap_alpha = 0.0024;
            //            float unmap_beta = 526.39;
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += cpu_time[i] * layer_cpu[i];
                for (int j = 0; j < layer.size(); j ++)   {
                    if (layer_comm[j][i]) {
                        tempConstrlhs += to_CPU_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                        tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                    }
                }
            }
            model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than CPU");
            
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += gpu_time[i] * layer_gpu[i];
                for (int j = 0; j < layer.size(); j ++)   {
                    if (layer_comm[j][i]) {
                        tempConstrlhs += map[i] * (map_alpha * layer_comm[j][i] + map_beta);
                        tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                    }
                }
            }
            model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than GPU");
            
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += cpu_time[i] * layer_cpu[i];
                for (int j = 0; j < layer.size(); j ++)   {
                    if (layer_comm[j][i]) {
                        tempConstrlhs += to_CPU_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                        tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                    }
                }
            }
            model.addConstr(tempConstrlhs,'>',min_ob, "smaller than CPU");
            
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += gpu_time[i] * layer_gpu[i];
                for (int j = 0; j < layer.size(); j ++)   {
                    if (layer_comm[j][i]) {
                        tempConstrlhs += map[i] * (map_alpha * layer_comm[j][i] + map_beta);
                        tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                    }
                }
            }
            model.addConstr(tempConstrlhs,'>',min_ob, "smaller than GPU");
            
            
            if (layer_cpu2.size())  {
                tempConstrlhs = 0;
                for (int i = 0; i < layer.size(); i ++)   {
                    tempConstrlhs += cpu2_time[i] * layer_cpu2[i];
                    for (int j = 0; j < layer.size(); j ++)   {
                        if (layer_comm[j][i]) {
                            tempConstrlhs += to_CPU2_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                            tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                        }
                    }
                }
                model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than CPU2");
                
                tempConstrlhs = 0;
                for (int i = 0; i < layer.size(); i ++)   {
                    tempConstrlhs += cpu2_time[i] * layer_cpu2[i];
                    for (int j = 0; j < layer.size(); j ++)   {
                        if (layer_comm[j][i]) {
                            tempConstrlhs += to_CPU2_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                            tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                        }
                    }
                }
                model.addConstr(tempConstrlhs,'>',min_ob, "smaller than CPU2");
            }
            if (layer_cpu3.size())  {
                tempConstrlhs = 0;
                for (int i = 0; i < layer.size(); i ++)   {
                    tempConstrlhs += cpu3_time[i] * layer_cpu3[i];
                    for (int j = 0; j < layer.size(); j ++)   {
                        if (layer_comm[j][i]) {
                            tempConstrlhs += to_CPU3_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                            tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                        }
                    }
                }
                model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than CPU3");
                
                tempConstrlhs = 0;
                for (int i = 0; i < layer.size(); i ++)   {
                    tempConstrlhs += cpu3_time[i] * layer_cpu3[i];
                    for (int j = 0; j < layer.size(); j ++)   {
                        if (layer_comm[j][i]) {
                            tempConstrlhs += to_CPU3_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                            tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                        }
                    }
                }
                model.addConstr(tempConstrlhs,'>',min_ob, "smaller than CPU3");
            }
            if (layer_cpu4.size())  {
                tempConstrlhs = 0;
                for (int i = 0; i < layer.size(); i ++)   {
                    tempConstrlhs += cpu4_time[i] * layer_cpu4[i];
                    for (int j = 0; j < layer.size(); j ++)   {
                        if (layer_comm[j][i]) {
                            tempConstrlhs += to_CPU4_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                            tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                        }
                    }
                }
                model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than CPU4");
                
                tempConstrlhs = 0;
                for (int i = 0; i < layer.size(); i ++)   {
                    tempConstrlhs += cpu4_time[i] * layer_cpu4[i];
                    for (int j = 0; j < layer.size(); j ++)   {
                        if (layer_comm[j][i]) {
                            tempConstrlhs += to_CPU4_comm[i] * (map_alpha * layer_comm[j][i] + map_beta);
                            tempConstrlhs += unmap[i] * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                        }
                    }
                }
                model.addConstr(tempConstrlhs,'>',min_ob, "smaller than CPU4");
            }
            
        }
        
        // Set objective: maximize throughput?
        
        if (mode) {
            GRBLinExpr lhs = 0;
            lhs += MAX_ob;
            
            for (int i = 0; i < layer.size(); i ++)   {
                lhs += 0.001 * layer_end[i];
            }
            
            
            model.setObjective(lhs, GRB_MINIMIZE);
        }
        
        // Set objective: minimize last layer's end time
        else {
            GRBLinExpr lhs = 0;
            for (int i = layer.size() -1; i < layer.size(); i ++)   {
                lhs += layer_end[i];
            }
            model.setObjective(lhs, GRB_MINIMIZE);
        }
        
        // Optimize model
        model.optimize();
        //mapped processor
        for (int i = 0; i < layer.size(); i ++)   {
            
            std::cout << layer_cpu[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << (long)(layer_cpu[i].get(GRB_DoubleAttr_X)+0.5) << '\t';
            std::cout << layer_gpu[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << (long)(layer_gpu[i].get(GRB_DoubleAttr_X)+0.5) << '\t';
            if (layer_cpu2.size()) {
                std::cout << layer_cpu2[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << (long)(layer_cpu2[i].get(GRB_DoubleAttr_X)+0.5);
            }
            if (layer_cpu3.size()) {
                std::cout << layer_cpu3[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << (long)(layer_cpu3[i].get(GRB_DoubleAttr_X)+0.5);
            }
            if (layer_cpu4.size()) {
                std::cout << layer_cpu4[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << (long)(layer_cpu4[i].get(GRB_DoubleAttr_X)+0.5);
            }
            std::cout<<std::endl;
        }
        //layer time
        for (int i = 0; i < layer.size(); i ++)   {
            std::cout << layer_start[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << layer_start[i].get(GRB_DoubleAttr_X)<< '\t';
            std::cout << layer_end[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << layer_end[i].get(GRB_DoubleAttr_X) << std::endl;
        }
        
        for (int i = 0; i < layer.size(); i ++)   {
            std::cout << map[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(map[i].get(GRB_DoubleAttr_X))<< '\t';
            std::cout << unmap[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(unmap[i].get(GRB_DoubleAttr_X)) << std::endl;
        }
        
        //debug
        for (int i = 0; i < layer.size(); i ++)   {
            std::cout << CPU_cluster[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(CPU_cluster[i].get(GRB_DoubleAttr_X))<< '\t';
            std::cout << to_CPU_comm[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(to_CPU_comm[i].get(GRB_DoubleAttr_X)) << std::endl;
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::cout << pre_cpu[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(pre_cpu[i].get(GRB_DoubleAttr_X))<< '\t';
            std::cout << to_CPU_transition[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(to_CPU_transition[i].get(GRB_DoubleAttr_X)) << std::endl;
        }
        
        for (int i = 0; i < layer.size(); i ++)   {
            std::cout << GPU_cluster[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(GPU_cluster[i].get(GRB_DoubleAttr_X))<< '\t';
            std::cout << to_GPU_comm[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(to_GPU_comm[i].get(GRB_DoubleAttr_X)) << std::endl;
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::cout << pre_gpu[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(pre_gpu[i].get(GRB_DoubleAttr_X))<< '\t';
            std::cout << to_GPU_transition[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(to_GPU_transition[i].get(GRB_DoubleAttr_X)) << std::endl;
        }
        
        if (layer_cpu2.size())  {
            for (int i = 0; i < layer.size(); i ++)   {
                std::cout << CPU2_cluster[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(CPU2_cluster[i].get(GRB_DoubleAttr_X))<< '\t';
                std::cout << to_CPU2_comm[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(to_CPU2_comm[i].get(GRB_DoubleAttr_X)) << std::endl;
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::cout << pre_cpu2[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(pre_cpu2[i].get(GRB_DoubleAttr_X))<< '\t';
                std::cout << to_CPU2_transition[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(to_CPU2_transition[i].get(GRB_DoubleAttr_X)) << std::endl;
            }
        }

        if (layer_cpu3.size())  {
            for (int i = 0; i < layer.size(); i ++)   {
                std::cout << CPU3_cluster[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(CPU3_cluster[i].get(GRB_DoubleAttr_X))<< '\t';
                std::cout << to_CPU3_comm[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(to_CPU3_comm[i].get(GRB_DoubleAttr_X)) << std::endl;
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::cout << pre_cpu3[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(pre_cpu3[i].get(GRB_DoubleAttr_X))<< '\t';
                std::cout << to_CPU3_transition[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(to_CPU3_transition[i].get(GRB_DoubleAttr_X)) << std::endl;
            }
        }
        if (layer_cpu4.size())  {
            for (int i = 0; i < layer.size(); i ++)   {
                std::cout << CPU4_cluster[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(CPU4_cluster[i].get(GRB_DoubleAttr_X))<< '\t';
                std::cout << to_CPU4_comm[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(to_CPU4_comm[i].get(GRB_DoubleAttr_X)) << std::endl;
            }
            for (int i = 0; i < layer.size(); i ++)   {
                std::cout << pre_cpu4[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(pre_cpu4[i].get(GRB_DoubleAttr_X))<< '\t';
                std::cout << to_CPU4_transition[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << abs(to_CPU4_transition[i].get(GRB_DoubleAttr_X)) << std::endl;
            }
        }
        
        //result output
        if (mode)   {
            throughput_set.push_back(MAX_ob.get(GRB_DoubleAttr_X));
            std::cout << "Obj: MAX " << MAX_ob.get(GRB_DoubleAttr_X)<< std::endl;
        }
        std::cout << "Obj: end time " << layer_end[layer.size()-1].get(GRB_DoubleAttr_X) << std::endl;
        
        //struct
        std::vector<result> temp_result_set;
        for (int i = 0; i < layer.size(); i ++)   {
            
            result temp_result;
            
            temp_result.layer_name = layer[i];
            if ((long)(layer_cpu[i].get(GRB_DoubleAttr_X)+0.5))   {
                temp_result.mapped = "cpu";
            }
            else  {
                temp_result.mapped = "gpu";
            }
            if (layer_cpu2.size()) {
                if ((long)(layer_cpu[i].get(GRB_DoubleAttr_X)+0.5))   {
                    temp_result.mapped = "cpu";
                }
                else if ((long)(layer_cpu2[i].get(GRB_DoubleAttr_X)+0.5))  {
                    temp_result.mapped = "cpu2";
                }
                else  {
                    temp_result.mapped = "gpu";
                }
            }
            if (layer_cpu3.size()) {
                if ((long)(layer_cpu[i].get(GRB_DoubleAttr_X)+0.5))   {
                    temp_result.mapped = "cpu";
                }
                else if ((long)(layer_cpu2[i].get(GRB_DoubleAttr_X)+0.5))  {
                    temp_result.mapped = "cpu2";
                }
                else if ((long)(layer_cpu3[i].get(GRB_DoubleAttr_X)+0.5))  {
                    temp_result.mapped = "cpu3";
                }
                else  {
                    temp_result.mapped = "gpu";
                }
            }
            if (layer_cpu4.size()) {
                if ((long)(layer_cpu[i].get(GRB_DoubleAttr_X)+0.5))   {
                    temp_result.mapped = "cpu";
                }
                else if ((long)(layer_cpu2[i].get(GRB_DoubleAttr_X)+0.5))  {
                    temp_result.mapped = "cpu2";
                }
                else if ((long)(layer_cpu3[i].get(GRB_DoubleAttr_X)+0.5))  {
                    temp_result.mapped = "cpu3";
                }
                else if ((long)(layer_cpu4[i].get(GRB_DoubleAttr_X)+0.5))  {
                    temp_result.mapped = "cpu4";
                }
                else  {
                    temp_result.mapped = "gpu";
                }
            }
            
            temp_result.start_time =  layer_start[i].get(GRB_DoubleAttr_X);
            
            temp_result.end_time = layer_end[i].get(GRB_DoubleAttr_X);
            
            temp_result.comm_time = 0;
            for (int j = 0; j < layer.size(); j ++)   {
                if (layer_comm[j][i]) {
                    temp_result.comm_time += map[i].get(GRB_DoubleAttr_X) * (map_alpha * layer_comm[j][i] + map_beta);
                    temp_result.comm_time += unmap[i].get(GRB_DoubleAttr_X) * (unmap_alpha * layer_comm[j][i] + unmap_beta);
                }
            }
            
            //debug
            std::cout <<temp_result.layer_name <<'\t' <<temp_result.mapped <<'\t' <<temp_result.start_time <<'\t' <<temp_result.end_time <<'\t' <<temp_result.comm_time <<std::endl;
            temp_result_set.push_back(temp_result);
        }
        
        result_set.push_back(temp_result_set);
    }
    
    //exception handler
    catch(GRBException e)   {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
    catch(...)  {
        std::cout << "Exception during optimization" <<  std::endl;
    }
    
    condition.clear();
    layer_end.clear();
    layer_start.clear();
    
    CPU_cluster.clear();
    not_pre_cpu.clear();
    pre_cpu.clear();
    transition.clear();
    GPU_cluster.clear();
    not_pre_gpu.clear();
    pre_gpu.clear();
    
    layer_gpu.clear();
    layer_cpu.clear();
    
    return_result(result_set[0]);
    
    std::cout<<"\n\nfinal result : " << std::endl<<std::endl;
    
    for (int i = 0; i < layer.size(); i ++)   {
        //debug
        std::cout <<result_set[0][i].layer_name <<'\t' <<result_set[0][i].mapped <<'\t' <<result_set[0][i].start_time <<'\t' <<result_set[0][i].end_time <<'\t' <<result_set[0][i].comm_time <<std::endl;
    }
    
    
    //delete
    delete(caffe_model);
    delete(time);
    return 0;
}
