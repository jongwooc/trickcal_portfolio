#ifndef PE_H_
#define PE_H_

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

//classes for variables
#include "CObj.h"
#include "PE.h"
#include "variables.h"

class PE : public Cobj	{
    
    _elements  * element;
    
    
public:
    
    void set_time(float time_data);
    
    PE();
    PE(const PE& _pe);
    virtual ~PE();

};

#endif
