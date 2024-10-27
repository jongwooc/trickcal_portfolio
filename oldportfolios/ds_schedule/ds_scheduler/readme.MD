[Build ILP in linux]
 You can build ILP by entering 'make linux'.
 'ilp_DS' is a execution file of ILP. You can execute the 'ILP scheduler' by typing './ilp_DS [model_file] [estimation_file] [objectives(0: latency, 1: throughput)]'

[Porting ILP(C++) to GanttChart(Python)]
 You can get a shared library by entering 'make linux'
 'main_porting.so' is a shared library used for execution in Python code(ILP.py). It not only executes the 'ILP scheduler' but also draws 'Gantt chart' for the schedule.
 You can execute it by typing 'python Scheduler.py -s ILP'.
