#include "includes.h"
#include "variables.h"

float const_comm_init = 0.5;
float const_comm_delay = 0.0001;

int main(int argc, char *argv[])	{
	
    Ilp_Expression * DS_Scheduler;
    DS_Scheduler = new Ilp_Expression(argc, argv);
    
    
    
    delete(DS_Scheduler);
	return 0;
}
