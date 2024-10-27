#ifndef VARIABLES_H_
#define VARIABLES_H_

extern float const_comm_init;
extern float const_comm_delay;


struct result {
    std::string layer_name;
    std::string mapped;
    float start_time;
    float end_time;
    float comm_time;
};

inline std::vector<result> return_result   (std::vector<result> _result_set)
{
    return _result_set;
}

struct _elements {
    //variables for PE
    std::vector<float> process_time;
    std::vector<GRBVar> layer;
    std::vector<GRBVar> pre_same;
    std::vector<GRBVar> not_pre_same;
    std::vector<GRBVar> cluster_start;
    std::vector<GRBVar> to_transition;
    std::vector<GRBVar> PE_comm;
};

struct _dataset   {
    //transition time variables.
    std::vector<std::vector<float>> layer_comm;
    std::vector<GRBVar> map;
    std::vector<GRBVar> unmap;
    
    //time variables for ilp
    std::vector<std::string> layer;
    std::vector<std::vector<std::string>> bottom_layer;
};

struct _expression {
    GRBEnv  * env;
    GRBModel * model;
    //mostly data for ilp constraint expressions.
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
};

struct _result_data   {
    std::vector<std::vector<result>> result_set;
    std::vector<std::pair<std::string,float>> throughput_set;
};

#endif
