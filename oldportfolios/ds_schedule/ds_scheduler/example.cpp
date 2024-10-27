/* Copyright 2018, Gurobi Optimization, LLC */

/* This example formulates and solves the following simple MIP model:
 
 minimize     LAYERn + COMMSn
 subject to     LAYERn = CPUn + GPUn = 1
 COMMSn = CPUn * GPUm + CPUm * GPUn : m = n + 1; COOMS = 0 or 1
 
 LAYER, COMMS, CPU, GPU is binary
 */

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

float const_comm_init = 0.5;
float const_comm_delay = 0.001;



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
    
    std::vector<float> cpu_time;
    std::vector<float> gpu_time;
    std::vector<std::pair<int,int>> comm_links;
    std::vector<std::vector<float>> layer_comm;
    std::vector<GRBVar> layer_cpu;
    std::vector<GRBVar> layer_gpu;
    std::vector<std::string> layer;
    std::vector<std::vector<std::string>> bottom_layer;
    std::vector<GRBVar> layer_start;
    std::vector<GRBVar> layer_end;
    // [ith layer][jth layer][0] -> ith layer is directly linked && previous layer of jth previous layer, binary
    // [ith layer][jth layer][1] -> ith layer and jth layer are mapped at same processing unit, binary.
    // [ith layer][jth layer][2] -> ith layer and jth layer are mapped at different processing unit, binary.
    std::vector<std::vector<std::vector<GRBVar>>> condition;
    std::vector<std::pair<std::string,std::vector<int>>> mem_size;
    std::vector<std::vector<result>> result_set;
    
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
                //                std:: cout <<"time layer : " << time->layer(i).name() << "  model layer :  " << caffe_model->layer(j).name() << std::endl;
                layer.push_back(time->layer(i).name());
                cpu_time.push_back(time->layer(i).cpu());
                gpu_time.push_back(time->layer(i).gpu());
                std::vector<std::string> temp_bottom;
                if (caffe_model->layer(j).bottom_size())    {
                    for(int k = 0; k < caffe_model->layer(j).bottom_size(); k ++)   {
                        temp_bottom.push_back(caffe_model->layer(j).bottom(k));
                        std::cerr<<temp_bottom[k]<<std::endl;
                    }
                    bottom_layer.push_back(temp_bottom);
                    
                    //                    bottom_layer.push_back(caffe_model->layer(j).bottom(caffe_model->layer(j).bottom_size()-1));
                }
                j = caffe_model->layer_size();
                std::cerr << i << "layer cpu  " << cpu_time[i]<< "/ gpu  " << gpu_time[i] << std::endl;
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
        unsigned int temp_output;
        unsigned int temp_width;
        unsigned int temp_height;
        temp_output = mem_size[i-1].second[0];
        temp_width = mem_size[i-1].second[1];
        temp_height = mem_size[i-1].second[2];
        std::cerr <<"layer num" << i<<std::endl;
        
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
        std::cerr << mem_size[i].first<<std::endl;
        std::cerr << "[0]" << mem_size[i].second[0]<<std::endl;
        std::cerr << "[1]" << mem_size[i].second[1]<<std::endl;
        std::cerr << "[2]" << mem_size[i].second[2]<<std::endl;
    }
    for (int i = 0; i < bottom_layer.size(); i ++)  {
        for (int j = 0; j < layer.size(); j ++)  {
            for (int l = 0; l < bottom_layer[i].size(); l ++)  {
                std::cerr<< bottom_layer[i][l] << std::endl;
                if (bottom_layer[i][l] == layer[j]) {
                    std:: cout <<"bottom layer : " << layer[j] << "  top layer :  " << layer[i] << std::endl;
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
    for (int i = 0; i < layer.size(); i ++){
        for (int j = 0; j < layer.size(); j ++){
            std:: cout <<"connect_time "<<layer[i]<< " " << layer[j] <<" : "<< layer_comm[i][j] <<std::endl;
        }
    }
    
    for (int test = 0; test < 4; test++) {
        
        for (int i = 0; i < layer.size(); i++){
            std::vector<GRBVar> temp_condition_set;
            temp_condition_set.reserve(3);
            
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
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_cpu";
                GRBVar cpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_cpu.push_back(cpu);
            }
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_gpu";
                GRBVar gpu = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, temp_string);
                layer_gpu.push_back(gpu);
            }
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_start";
                GRBVar start_time = model.addVar(0.0, 99999999.0, 0.0, GRB_CONTINUOUS, temp_string);
                layer_start.push_back(start_time);
            }
            for (int i = 0; i < layer.size(); i ++){
                std::string temp_string = layer[i];
                temp_string = temp_string + "_end";
                GRBVar end_time = model.addVar(0.0, 99999999.0, 0.0, GRB_CONTINUOUS, temp_string);
                layer_end.push_back(end_time);
            }
            for (int i = 0; i < layer.size(); i ++){
                for (int j = 0; j < layer.size(); j ++){
                    std::string temp_string1 = layer[i];
                    std::string temp_string2 = layer[j];
                    std::string temp_string = temp_string1 + "_is_previous_of_" + temp_string2;
                    GRBVar ijpre = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                    temp_string = temp_string1 + "  " + temp_string2 + "  _same_processor";
                    GRBVar ijsamemap = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                    temp_string = temp_string1 + "  "  + temp_string2 + "  _different_processor";
                    GRBVar ijdiffmap = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                    temp_string = temp_string1 + "  "  + temp_string2 + "  _change_processor";
                    GRBVar ijchange = model.addVar(0.0, 1.0 ,0.0, GRB_BINARY, temp_string);
                    condition[i][j].push_back(ijpre);
                    condition[i][j].push_back(ijsamemap);
                    condition[i][j].push_back(ijdiffmap);
                    condition[i][j].push_back(ijchange);
                }
            }
            
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
            model.addConstr(tempConstrlhs,'=',0, "data_start_gpu");
            
            // Add constraint: layer_gpu + layer_cpu = 1 guaruntee node has one processor
            for (int i = 1; i < layer.size(); i ++)   {
                std::string temp_string = layer[i];
                temp_string = temp_string + "_node";
                GRBLinExpr addConstrlhs = 0;
                addConstrlhs += layer_cpu[i] + layer_gpu[i];
                model.addConstr(addConstrlhs,'=',1, temp_string);
            }
            
            // Add constraint: layer_gpu_n + layer_cpu_n = layer_num; guaruntee all layers are scheduled
            for (int i = 0; i < layer.size(); i ++)   {
                tempConstrlhs += layer_cpu[i] + layer_gpu[i];
            }
            model.addConstr(tempConstrlhs,'=',layer.size(), "all_nodes");
            
            // Add constraint: 0 = layer0_start_time;
            
            tempConstrlhs = 0;
            tempConstrlhs += layer_start[0];
            model.addConstr(tempConstrlhs,'=',0, "layer0_start_time");
            
            // Add constraint: layer_start+ layer_gpu_n*gputim + layer_cpu_n*cputime = layer_end;
            for (int i = 0; i < layer.size(); i ++)   {
                GRBLinExpr addConstrlhs = 0;
                addConstrlhs += (layer_start[i] + layer_cpu[i] * cpu_time[i] + layer_gpu[i] * gpu_time[i]);
                model.addConstr(addConstrlhs,'<',layer_end[i], "layer_end_time");
            }
            
            // Add constraint: set ijdiffmap
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < layer.size(); j ++)   {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "_and_" + layer[j] +"_different_mapped_1";
                    GRBLinExpr addConstrlhs0 = 0;
                    addConstrlhs0 += layer_cpu[i] + layer_cpu[j] - condition[i][j][2];
                    model.addConstr(addConstrlhs0,'>',0,temp_string0);
                    
                    std::string temp_string1 = layer[i];
                    temp_string1 = temp_string1 + "_and_" + layer[j] +"_different_mapped_2";
                    GRBLinExpr addConstrlhs1 = 0;
                    addConstrlhs1 += layer_cpu[i] - layer_cpu[j] - condition[i][j][2];
                    model.addConstr(addConstrlhs1,'<',0,temp_string1);
                    
                    std::string temp_string2 = layer[i];
                    temp_string2 = temp_string2 + "_and_" + layer[j] +"_different_mapped_3";
                    GRBLinExpr addConstrlhs2 = 0;
                    addConstrlhs2 += layer_cpu[j] - layer_cpu[i] - condition[i][j][2];
                    model.addConstr(addConstrlhs2,'<',0,temp_string2);
                    
                    std::string temp_string3 = layer[i];
                    temp_string3 = temp_string3 + "_and_" + layer[j] +"_different_mapped_4";
                    GRBLinExpr addConstrlhs3 = 0;
                    addConstrlhs3 += 2 - layer_cpu[i] - layer_cpu[j] - condition[i][j][2];
                    model.addConstr(addConstrlhs3,'>',0,temp_string3);
                    
                    std::string temp_string4 = layer[i];
                    temp_string4 = temp_string4 + "_and_" + layer[j] +"_different_mapped_5";
                    GRBLinExpr addConstrlhs4 = 0;
                    addConstrlhs4 += condition[i][j][2];
                    model.addConstr(addConstrlhs4,'>',0,temp_string4);
                    
                    std::string temp_string5 = layer[i];
                    temp_string5 = temp_string5 + "_and_" + layer[j] +"_different_mapped_6";
                    GRBLinExpr addConstrlhs5 = 0;
                    addConstrlhs5 += condition[i][j][2];
                    model.addConstr(addConstrlhs5,'<',1,temp_string5);
                }
            }
            
            // Add constraint: set ijchange = connected & diff
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < layer.size(); j ++)   {
                    std::string temp_string0 = layer[i];
                    temp_string0 = temp_string0 + "_and_" + layer[j] +"_change_mapped_1";
                    GRBLinExpr addConstrlhs0 = 0;
                    addConstrlhs0 += condition[i][j][0] + condition[i][j][2] - condition[i][j][3];
                    model.addConstr(addConstrlhs0,'<',1,temp_string0);
                    
                    std::string temp_string1 = layer[i];
                    temp_string1 = temp_string1 + "_and_" + layer[j] +"_change_mapped_2";
                    GRBLinExpr addConstrlhs1 = 0;
                    addConstrlhs1 +=condition[i][j][0] - condition[i][j][3];
                    model.addConstr(addConstrlhs1,'>',0,temp_string1);
                    
                    std::string temp_string2 = layer[i];
                    temp_string2 = temp_string2 + "_and_" + layer[j] +"_change_mapped_3";
                    GRBLinExpr addConstrlhs2 = 0;
                    addConstrlhs2 += condition[i][j][2] - condition[i][j][3];
                    model.addConstr(addConstrlhs2,'>',0,temp_string2);
                    
                    std::string temp_string4 = layer[i];
                    temp_string4 = temp_string4 + "_and_" + layer[j] +"_change_mapped_4";
                    GRBLinExpr addConstrlhs4 = 0;
                    addConstrlhs4 += condition[i][j][3];
                    model.addConstr(addConstrlhs4,'>',0,temp_string4);
                    
                    std::string temp_string5 = layer[i];
                    temp_string5 = temp_string5 + "_and_" + layer[j] +"_change_mapped_5";
                    GRBLinExpr addConstrlhs5 = 0;
                    addConstrlhs5 += condition[i][j][3];
                    model.addConstr(addConstrlhs5,'<',1,temp_string5);
                }
            }
            
            // Add constraint: ijsamemap + ijdiffmap  = 1; guaruntee node has one processor
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < layer.size(); j ++)   {
                    std::string temp_string = layer[i];
                    temp_string = temp_string + "_node_and_" + layer[j] + "mapped_same_or_not";
                    GRBLinExpr addConstrlhs = 0;
                    addConstrlhs += 1 - condition[i][j][2];
                    model.addConstr(addConstrlhs,'=', condition[i][j][1], temp_string);
                }
            }
            
            // Add constraint: if i,j is connected and i is previous of j, jstart - iend + commij * ijdiff >0
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    std::string temp_string = layer[j];
                    temp_string = temp_string + "endtime_is_smaller_than" + layer[i] + "start";
                    GRBLinExpr addConstrlhs = 0;
                    //                std::cout << "layer " << j <<" "<< i << "link is " << layer_comm[j][i]<<std::endl;
                    addConstrlhs += layer_start[i] - layer_end[j] - (layer_comm[j][i] * condition[j][i][2]) + ((1-condition[j][i][0]) * 4999999);
                    model.addConstr(addConstrlhs,'>',0, temp_string);
                }
            }
            
            // Add constraint: i,j is different or not? if mapped same processor, start time is greater than end time.
            for (int i = 0; i < layer.size(); i ++)   {
                for (int j = 0; j < i; j ++)   {
                    std::string temp_string = layer[j];
                    temp_string = temp_string + "_are_mapped_same_place_with_" + layer[i];
                    //                std::cout << "layer " << layer[j] <<" "<< layer[i] << "link " <<std::endl;
                    GRBLinExpr addConstrlhs = 0;
                    addConstrlhs += layer_start[i] - layer_end[j] +((condition[j][i][2]) * 4999999) ;
                    model.addConstr(addConstrlhs,'>',0, temp_string);
                }
            }
            
            //data is always cpu
            
            
            // Add constraint: all ijchange = 1
            if (mode) {
                if (test == 0)  {
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        for (int j = 0; j < layer.size(); j ++)   {
                            tempConstrlhs += condition[i][j][3];
                        }
                    }
                    model.addConstr(tempConstrlhs,'<',1, "change time");
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        tempConstrlhs += cpu_time[i] * layer_cpu[i] - gpu_time[i] * layer_gpu[i];
                    }
                    model.addConstr(tempConstrlhs,'>',0, "pipeline");
                }
                if (test == 1)  {
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        for (int j = 0; j < layer.size(); j ++)   {
                            tempConstrlhs += condition[i][j][3];
                        }
                    }
                    model.addConstr(tempConstrlhs,'<',1, "change time");
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        tempConstrlhs += cpu_time[i] * layer_cpu[i] - gpu_time[i] * layer_gpu[i];
                    }
                    model.addConstr(tempConstrlhs,'<',0, "pipeline");
                }
                
                if (test == 2)  {
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        for (int j = 0; j < layer.size(); j ++)   {
                            tempConstrlhs += condition[i][j][3];
                        }
                    }
                    model.addConstr(tempConstrlhs,'<',3, "change time");
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        tempConstrlhs += cpu_time[i] * layer_cpu[i] - gpu_time[i] * layer_gpu[i];
                    }
                    model.addConstr(tempConstrlhs,'>',0, "pipeline");
                }
                if (test == 3)  {
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        for (int j = 0; j < layer.size(); j ++)   {
                            tempConstrlhs += condition[i][j][3];
                        }
                    }
                    model.addConstr(tempConstrlhs,'<',2, "change time");
                    tempConstrlhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        tempConstrlhs += cpu_time[i] * layer_cpu[i] - gpu_time[i] * layer_gpu[i];
                    }
                    model.addConstr(tempConstrlhs,'<',0, "pipeline");
                }
            }
            
            // Set objective: maximize throughput?
            if (mode) {
                if (test %2 == 0)   {
                    GRBLinExpr lhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        lhs += cpu_time[i] * layer_cpu[i] - gpu_time[i] * layer_gpu[i];
                        //                        lhs += gpu_time[i] * layer_gpu[i] - cpu_time[i] * layer_cpu[i];
                    }
                    lhs += layer_end[layer.size() -1];
                    model.setObjective(lhs, GRB_MINIMIZE);
                }
                else   {
                    GRBLinExpr lhs = 0;
                    for (int i = 0; i < layer.size(); i ++)   {
                        //                        lhs += cpu_time[i] * layer_cpu[i] - gpu_time[i] * layer_gpu[i];
                        lhs += gpu_time[i] * layer_gpu[i] - cpu_time[i] * layer_cpu[i];
                    }
                    lhs += layer_end[layer.size() -1];
                    model.setObjective(lhs, GRB_MINIMIZE);
                }
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
            //        //mapped processor
            //            for (int i = 0; i < layer.size(); i ++)   {
            //                std::cout << layer_cpu[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                << abs(layer_cpu[i].get(GRB_DoubleAttr_X)) << '\t';
            //                std::cout << layer_gpu[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                << abs(layer_gpu[i].get(GRB_DoubleAttr_X)) << std::endl;
            //            }
            //
            //            //layer time
            //            for (int i = 0; i < layer.size(); i ++)   {
            //                std::cout << layer_start[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                << layer_start[i].get(GRB_DoubleAttr_X)<< '\t';
            //                std::cout << layer_end[i].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                << layer_end[i].get(GRB_DoubleAttr_X) << std::endl;
            //            }
            //            //for debug
            //            for (int i = 0; i < layer.size(); i ++)   {
            //                for (int j = 0; j < layer.size(); j ++)   {
            //                  std::cout << condition[i][j][0].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                  << abs(condition[i][j][0].get(GRB_DoubleAttr_X)) << '\t';
            //                  std::cout << condition[i][j][1].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                  << abs(condition[i][j][1].get(GRB_DoubleAttr_X)) << '\t';
            //                  std::cout << condition[i][j][2].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                  << abs(condition[i][j][2].get(GRB_DoubleAttr_X)) << '\t';
            //                  std::cout << condition[i][j][3].get(GRB_StringAttr_VarName) <<"  "<< '\t'
            //                  << abs(condition[i][j][3].get(GRB_DoubleAttr_X)) << std::endl;
            //              }
            //            }
            
            //result output
            if (mode)   {
                std::cout << "Obj: throughput 1/" << model.get(GRB_DoubleAttr_ObjVal) /2<< std::endl;
            }
            std::cout << "Obj: end time " << layer_end[layer.size()-1].get(GRB_DoubleAttr_X) << std::endl;
            
            //struct
            std::vector<result> temp_result_set;
            for (int i = 0; i < layer.size(); i ++)   {
                result temp_result;
                temp_result.comm_time = 0;
                if  (i >0  &&
                     (layer_cpu[i].get(GRB_DoubleAttr_X) != layer_cpu[i-1].get(GRB_DoubleAttr_X)
                      && layer_gpu[i].get(GRB_DoubleAttr_X) != layer_gpu[i-1].get(GRB_DoubleAttr_X)) )   {
                         for (int j = 0; j < layer.size(); j ++)   {
                             if (layer_comm[j][i])    {
                                 temp_result.comm_time = layer_comm[j][i];
                             }
                         }
                     }
                temp_result.layer_name = layer[i];
                if (layer_cpu[i].get(GRB_DoubleAttr_X) > layer_gpu[i].get(GRB_DoubleAttr_X))   {
                    temp_result.mapped = "cpu";
                }
                else   {
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
        layer_gpu.clear();
        layer_cpu.clear();
    }
    
    int final_result = 0;
    int temp_out = 9999999;
    for (int i = 0; i < result_set.size(); i++) {
        if (result_set[i][result_set[i].size()-1].end_time < temp_out)    {
            temp_out = result_set[i][result_set[i].size()-1].end_time;
            final_result = i;
        }
    }
    
    return_result(result_set[final_result]);
    
    std::cout<<"final result : " << std::endl<<std::endl;
    
    for (int i = 0; i < layer.size(); i ++)   {
        //debug
        std::cout <<result_set[final_result][i].layer_name <<'\t' <<result_set[final_result][i].mapped <<'\t' <<result_set[final_result][i].start_time <<'\t' <<result_set[final_result][i].end_time <<'\t' <<result_set[final_result][i].comm_time <<std::endl;
    }
    
    
    //delete
    delete(caffe_model);
    delete(time);
    return 0;
}
