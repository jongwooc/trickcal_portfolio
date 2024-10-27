#include "PE.h"


void PE::set_time(float time_data) {
    element->process_time.push_back(time_data);
    std::cerr << element->process_time.back()<<std::endl;
}

PE::PE() {
    element = new _elements;
    
}
PE::PE(const PE& _pe){ }
PE::~PE() {
    
}
