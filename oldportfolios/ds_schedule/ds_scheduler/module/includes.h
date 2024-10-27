#ifndef INCLUDES_H_
#define INCLUDES_H_

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

//funtional classes
#include "ILP_Expressions.h"

#endif
