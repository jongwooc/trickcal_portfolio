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

float const_comm_init = 550;
float const_comm_delay = 0.004;



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
    DS::network * time = new DS::network;
    caffe::NetParameter * caffe_model = new caffe::NetParameter;

    //PE profile time
    std::vector<float> cpu_time;
    std::vector<float> gpu_time;
    std::vector<float> dsp_time;
    std::vector<float> npu_time;
    std::vector<float> cpu2_time;
    std::vector<std::pair<int,int>> comm_links;
    std::vector<std::vector<float>> layer_comm;
    
    //add new PE here
    std::vector<GRBVar> layer_cpu;
    std::vector<GRBVar> layer_cpu2;
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
    std::vector<GRBVar> transition1;
    std::vector<GRBVar> transition2;
    
    //time variables for ilp
    std::vector<std::string> layer;
    std::vector<std::vector<std::string>> bottom_layer;
    std::vector<GRBVar> layer_start;
    std::vector<GRBVar> layer_end;
    // [ith layer][jth layer][0] -> ith layer is directly linked && previous layer of jth previous layer, binary
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

    //set layers based on time estimations.
    for (int i = 0; i < time->layer_size(); i ++)  {
        for (int j = 0; j < caffe_model->layer_size(); j ++) {
            if (time->layer(i).name() == caffe_model->layer(j).name()) {
                layer.push_back(time->layer(i).name());
                //set pe time here
                if (time->layer(i).has_cpu())    {
                    cpu_time.push_back(time->layer(i).cpu());
                }
                if (time->layer(i).has_gpu())   {
                    gpu_time.push_back(time->layer(i).gpu());
                }
                if (time->layer(i).has_dsp())    {
                    dsp_time.push_back(time->layer(i).dsp());
                }
                if (time->layer(i).has_npu())   {
                    npu_time.push_back(time->layer(i).npu());
                }
                if (time->layer(i).has_cpu2())    {
                    cpu2_time.push_back(time->layer(i).cpu2());
                }
                std::vector<std::string> temp_bottom;
                if (caffe_model->layer(j).bottom_size())    {
                    for(int k = 0; k < caffe_model->layer(j).bottom_size(); k ++)   {
                        temp_bottom.push_back(caffe_model->layer(j).bottom(k));
                    }
                    bottom_layer.push_back(temp_bottom);
                }
                j = caffe_model->layer_size();
            }
        }
    }

    //check whether each layer has one bottom layer.
    if (bottom_layer.size() != layer.size()){
        std::vector<std::vector<std::string>>::iterator temp_it;
        temp_it = bottom_layer.begin();
        std::vector<std::string> temp_data;
        temp_data.push_back("no data");
        bottom_layer.insert(temp_it,temp_data);
    }

    // check whether all the time layers are set to layers.
    if (time->layer_size() != layer.size()){
//        std::cerr << "Model layers are not matching with time layers!  Check the naming of layers first!" << std::endl;
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
        unsigned int temp_output;
        unsigned int temp_width;
        unsigned int temp_height;
        temp_output = mem_size[i-1].second[0];
        temp_width = mem_size[i-1].second[1];
        temp_height = mem_size[i-1].second[2];

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
                            temp_commtime = (const_comm_init + const_comm_delay * mem_size[k].second[0] * mem_size[k].second[1] * mem_size[k].second[2]);
                            std:: cout <<"mem_size : " << temp_commtime << std::endl;
                            layer_comm[j][i] = temp_commtime;
                        }
                    }
                    comm_links.push_back(std::make_pair(j,i));
                    
                    if (bottom_layer[i].size()-1 == l)  {
                        j = layer.size();
                    }
                }
            }
        }
    }

    for (int i = 0; i < layer.size(); i++){
        std::vector<GRBVar> temp_condition_set;
        temp_condition_set.reserve(5);
        
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
            temp_string = temp_string + "_trasition";
            GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
            transition1.push_back(trans);
        }
        for (int i = 0; i < layer.size(); i ++){
            std::string temp_string = layer[i];
            temp_string = temp_string + "_trasition";
            GRBVar trans = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
            transition2.push_back(trans);
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
            }
        }

        //Add max variable
        GRBVar MAX_ob = model.addVar(0.0, 999999999.0, 0.0, GRB_CONTINUOUS, "max (PE)");
        
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
        tempConstrlhs += layer_cpu[0];
        model.addConstr(tempConstrlhs,'=',1, "data_start_cpu");
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_gpu[0];
        model.addConstr(tempConstrlhs,'=',0, "data_does_not_start_gpu");
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_cpu2[0];
        model.addConstr(tempConstrlhs,'=',0, "data_does_not_start_cpu2");
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_cpu[layer.size()-1];
        model.addConstr(tempConstrlhs,'=',0, "data_not end_cpu");
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_gpu[layer.size()-1];
        model.addConstr(tempConstrlhs,'=',0, "data_does_not_end_gpu");
        
        tempConstrlhs = 0;
        tempConstrlhs += layer_cpu2[layer.size()-1];
        model.addConstr(tempConstrlhs,'=',1, "data_does_end_cpu2");
        
        // Add constraint: layer_PE[i] = 1 guaruntee node has one processor
        for (int i = 1; i < layer.size(); i ++)   {
            std::string temp_string = layer[i];
            temp_string = temp_string + "_node";
            GRBLinExpr addConstrlhs = 0;
            addConstrlhs += layer_cpu[i] + layer_gpu[i];
            if (layer_cpu2.size())  {
                addConstrlhs += layer_cpu2[i];
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
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i] + not_pre_cpu[i] - 1;
            model.addConstr(addConstrlhs0,'<',CPU_cluster[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i];
            model.addConstr(addConstrlhs0,'>',CPU_cluster[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += not_pre_cpu[i];
            model.addConstr(addConstrlhs0,'>',CPU_cluster[i],temp_string0);
        }
        //only one CPU cluster
        tempConstrlhs = 0;
        for (int i = 0; i < layer.size(); i ++)   {
            tempConstrlhs += CPU_cluster[i];
        }
        model.addConstr(tempConstrlhs,'=',0,"one CPU cluster");
        // Add constraint: to_cpu_transition
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "_to_CPU";
                    GRBLinExpr addConstrlhs0 = 0;
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        addConstrlhs0 += (1-layer_cpu[j+k]);
                    }
                    model.addConstr(addConstrlhs0,'>',to_CPU_transition[i],temp_string0);
                    j = layer.size();
                }
            }
        }
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_to_CPU_"+layer[j+k];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += (1-layer_cpu[j+k]);
                        model.addConstr(addConstrlhs0,'<',to_CPU_transition[i],temp_string0);
                    }
                    j = layer.size();
                }
            }
        }
        //to_CPU_comm
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i] + to_CPU_transition[i] - 1;
            model.addConstr(addConstrlhs0,'<',to_CPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu[i];
            model.addConstr(addConstrlhs0,'>',to_CPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU_comm";
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
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i] + not_pre_gpu[i] - 1;
            model.addConstr(addConstrlhs0,'<',GPU_cluster[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i];
            model.addConstr(addConstrlhs0,'>',GPU_cluster[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
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
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "to GPU";
                    GRBLinExpr addConstrlhs0 = 0;
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        addConstrlhs0 += (1 - layer_gpu[j+k]);
                    }
                    model.addConstr(addConstrlhs0,'>',to_GPU_transition[i],temp_string0);
                    j = layer.size();
                }
            }
        }
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_to_GPU_"+layer[j+k];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += (1 - layer_gpu[j+k]);
                        model.addConstr(addConstrlhs0,'<',to_GPU_transition[i],temp_string0);
                    }
                    j = layer.size();
                }
            }
        }
        //to_GPU_comm
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_GPU_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i] + to_GPU_transition[i] - 1;
            model.addConstr(addConstrlhs0,'<',to_GPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_GPU_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_gpu[i];
            model.addConstr(addConstrlhs0,'>',to_GPU_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_GPU_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += to_GPU_transition[i];
            model.addConstr(addConstrlhs0,'>',to_GPU_comm[i],temp_string0);
        }
        
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
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu2[i] + not_pre_cpu2[i] - 1;
            model.addConstr(addConstrlhs0,'<',CPU2_cluster[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu2[i];
            model.addConstr(addConstrlhs0,'>',CPU2_cluster[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "cluster_start";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += not_pre_cpu2[i];
            model.addConstr(addConstrlhs0,'>',CPU2_cluster[i],temp_string0);
        }
        //only one CPU2 cluster
        tempConstrlhs = 0;
        for (int i = 0; i < layer.size(); i ++)   {
            tempConstrlhs += CPU2_cluster[i];
        }
        model.addConstr(tempConstrlhs,'=',1,"one CPU2 cluster");
        // Add constraint: to_CPU2_transition
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "to_CPU2";
                    GRBLinExpr addConstrlhs0 = 0;
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        addConstrlhs0 += (1-layer_cpu2[j+k]);
                    }
                    model.addConstr(addConstrlhs0,'>',to_CPU2_transition[i],temp_string0);
                    j = layer.size();
                }
            }
        }
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    for (int k = 0; k < bottom_layer[i].size(); k ++)   {
                        std::string temp_string0 = layer[i];
                        temp_string0 = temp_string0 + "_to_CPU2_"+layer[j+k];
                        GRBLinExpr addConstrlhs0 = 0;
                        addConstrlhs0 += (1-layer_cpu2[j+k]);
                        model.addConstr(addConstrlhs0,'<',to_CPU2_transition[i],temp_string0);
                    }
                    j = layer.size();
                }
            }
        }
        //to_CPU2_comm
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU2_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu2[i] + to_CPU2_transition[i] - 1;
            model.addConstr(addConstrlhs0,'<',to_CPU2_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU2_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += layer_cpu2[i];
            model.addConstr(addConstrlhs0,'>',to_CPU2_comm[i],temp_string0);
        }
        for (int i = 0; i < layer.size(); i ++)   {
            std::string temp_string0 = layer[i];
            temp_string0 = temp_string0 + "to_CPU2_comm";
            GRBLinExpr addConstrlhs0 = 0;
            addConstrlhs0 += to_CPU2_transition[i];
            model.addConstr(addConstrlhs0,'>',to_CPU2_comm[i],temp_string0);
        }
  

        // Add constraint: if i,j is connected and i is previous of j, jstart - iend + commij * ijdiff >0
        for (int i = 0; i < layer.size(); i ++)   {
            for (int j = 0; j < i; j ++)   {
                if (layer_comm[j][i]) {
                    std::string temp_string = layer[j];
                    temp_string = temp_string + "endtime_is_smaller_than" + layer[i] + "start";
                    GRBLinExpr addConstrlhs = 0;
                    addConstrlhs += layer_start[i] - layer_end[j] - (layer_comm[j][i] * transition[i]);
                    model.addConstr(addConstrlhs,'>',0, temp_string);
                }
            }
        }

        
        if (mode) {
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += cpu_time[i] * layer_cpu[i];
                for (int j = 0; j < layer.size(); j ++) {
                    tempConstrlhs += transition[i] * layer_comm[j][i];
                }
            }
            model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than CPU");

            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += gpu_time[i] * layer_gpu[i];
                for (int j = 0; j < layer.size(); j ++) {
                    tempConstrlhs += transition[i] * layer_comm[j][i];
                }
            }
            model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than GPU");
            
            tempConstrlhs = 0;
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += cpu2_time[i] * layer_cpu2[i];
                for (int j = 0; j < layer.size(); j ++) {
                    tempConstrlhs += transition[i] * layer_comm[j][i];
                }
            }
            model.addConstr(tempConstrlhs,'<',MAX_ob, "greater than CPU2");
        
//        tempConstrlhs = 0;
//        for (int i = layer.size() -1; i < layer.size(); i ++)   {
//            tempConstrlhs += layer_end[i];
//        }
//        model.addConstr(tempConstrlhs,'<',MAX_ob * 3, "latency");
    }
        
        // Set objective: maximize throughput?
    
        if (mode) {
            GRBLinExpr lhs = 0;
            lhs += 1000*MAX_ob + layer_end[layer.size()-1];

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
            << abs(layer_cpu[i].get(GRB_DoubleAttr_X)) << '\t';
            std::cout << layer_gpu[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(layer_gpu[i].get(GRB_DoubleAttr_X)) << '\t';
            std::cout << layer_cpu2[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(layer_cpu2[i].get(GRB_DoubleAttr_X)) << std::endl;
        }
        
        for (int i = 0; i < layer.size(); i ++)   {
            std::cout << transition[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            << abs(transition[i].get(GRB_DoubleAttr_X)) << '\t';
            std::cout << transition1[i].get(GRB_StringAttr_VarName) <<"1  "<< '\t'
            << abs(transition1[i].get(GRB_DoubleAttr_X)) << '\t';
            std::cout << transition2[i].get(GRB_StringAttr_VarName) <<"2  "<< '\t'
            << abs(transition2[i].get(GRB_DoubleAttr_X)) << std::endl;
        }

            //layer time
            for (int i = 0; i < layer.size(); i ++)   {
                std::cout << layer_start[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << layer_start[i].get(GRB_DoubleAttr_X)<< '\t';
                std::cout << layer_end[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
                << layer_end[i].get(GRB_DoubleAttr_X) << std::endl;
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
            temp_result.comm_time = 0;
            for (int j = 0; j < layer.size(); j ++) {
                temp_result.comm_time += transition[i].get(GRB_DoubleAttr_X) * layer_comm[j][i];
            }

            temp_result.layer_name = layer[i];
            if (layer_cpu[i].get(GRB_DoubleAttr_X) > layer_gpu[i].get(GRB_DoubleAttr_X))   {
                temp_result.mapped = "cpu";
            }
            else if (layer_cpu[i].get(GRB_DoubleAttr_X) == layer_gpu[i].get(GRB_DoubleAttr_X))  {
                temp_result.mapped = "cpu2";
            }
            else  {
                temp_result.mapped = "gpu";
            }

            temp_result.start_time =  layer_start[i].get(GRB_DoubleAttr_X);
            temp_result.end_time = layer_end[i].get(GRB_DoubleAttr_X);
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
