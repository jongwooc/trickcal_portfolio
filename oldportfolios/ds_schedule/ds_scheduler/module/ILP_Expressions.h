//
//  ILP_Constraint.h
//  
//
//  Created by El on 2018. 8. 3..
//

#ifndef ILP_EXPRESSIONS_H_
#define ILP_EXPRESSIONS_H_
#include <map>
#include <string>
#include "PE.h"

class Ilp_Expression{
    //time variables for ilp
    _expression * expression_variables;
    _dataset * data_variables;
    std::map <std::string,PE *> processors;

public:
    
    
    Ilp_Expression();
    Ilp_Expression(int argc,char *argv[]);
    ~Ilp_Expression();
    
};




#endif /* ILP_Constraint_h */
