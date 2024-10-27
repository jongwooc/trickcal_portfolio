mac: proto
	g++ -m64 -o ilp_DS main.cpp caffe.pb.cc DS.pb.cc -I/Library/gurobi800/mac64/include/ -L/Library/gurobi800/mac64/lib -lgurobi_c++ -lgurobi80 -lm -lprotobuf -lstdc++ -std=c++11

debug: proto
	g++ -m64 -o ilp_DS_debug main_force.cpp caffe.pb.cc DS.pb.cc -I/Library/gurobi800/mac64/include/ -L/Library/gurobi800/mac64/lib -lgurobi_c++ -lgurobi80 -lm -lprotobuf -lstdc++ -std=c++11
linux:	proto
	# building execution file
	g++ -m64 -o ilp_DS main.cpp caffe.pb.cc DS.pb.cc -std=c++11 -I/opt/gurobi800/linux64/include/ -L/opt/gurobi800/linux64/src/build/ -I/usr/local/include -lgurobi_c++ -lgurobi80 -lm -L/usr/local/lib -lprotobuf -lstdc++ 
	# building shared library
	g++ -fPIC -m64 -shared -Wl,-soname,main_porting -o main_porting.so main_porting.cpp caffe.pb.cc DS.pb.cc -std=c++11 -I/opt/gurobi800/linux64/include -L/opt/gurobi800/linux64/src/build -lgurobi_c++ -I/usr/local/include -lgurobi80 -lm -L/usr/local/lib -lprotobuf -lstdc++ 

example:
	g++ -shared -Wl,-soname,porting_example -o porting_example.so -fPIC porting_example.cpp

proto:
	rm *.pb.* && protoc --cpp_out=./ *.proto 
