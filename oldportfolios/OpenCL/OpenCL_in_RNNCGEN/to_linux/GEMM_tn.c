#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define _CRT_SECURE_NO_WARNINGS

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gemm.h"

/*
#define MAX_SOURCE_SIZE 1000	//This number should not be much bigger than actual source size.
*/
float* GEMM_tn(int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C)
{
	//Matrices: A = Ndim*Pdim	B = Pdim*Mdim	C = Ndim*Mdim
	int  Ndim = _N_dim; int Mdim = _M_dim; int Pdim = _P_dim; int VECTOR_P_dim = Pdim >> 2;
	int szA = Ndim*Pdim; int szB = Pdim*Mdim; int szC = Ndim*Mdim;



	//Memory for Matrices : A,B = read only, C = read and write
	cl_mem ReadBufferA;
	cl_mem ReadBufferB;
	cl_mem RwBufferC;

	//Domains
	size_t GlobalDimensions[2];
	size_t LocalDimensions[2];
	//Number of Working Dimensions, NOT "NDRange". 
	cl_uint NDim = 2;

	//Variable for errors. if this variableis non-0, something is wrong.
	int ErrorCode = 0;

	//basic items for openCL
	//Devices
	cl_device_id device_id = NULL;
	cl_uint num_device = 0;
	//Platforms
	cl_platform_id platform_id = NULL;
	cl_platform_info platform_info;
	cl_uint num_platform = 0;
	//Context : Where to send Kernel
	cl_context context = NULL;
	//Program : Somthing to compile
	cl_program program = NULL;
	//Command : What to do
	cl_command_queue command_queue = NULL;
	//Extra program for non-host calculator
	cl_kernel kernel = NULL;


	clGetPlatformIDs(1, &platform_id, &num_platform);
	clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_device);
	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM,(cl_context_properties)platform_id ,0 };
	context = clCreateContext(properties, 1, &device_id, NULL, NULL, &ErrorCode);
	//	fprintf(stderr, "Context error: %d\n", ErrorCode);
	//context = clCreateContext(NULL, num_device, &device_id, NULL, NULL, NULL);


	//if you want to use openCL 1.2, use this funtion instead
	command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ErrorCode);
	//	command_queue = clCreateCommandQueueWithProperties(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ErrorCode);
	//	fprintf(stderr, "Command queue error: %d\n", ErrorCode);






	ReadBufferA = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, sizeof(float)*szA, NULL, NULL);
	ReadBufferB = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, sizeof(float)*szB, NULL, NULL);
	RwBufferC = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, sizeof(float)*szC, NULL, NULL);
	//

	//load cl file to source_str
	FILE * fp;

	size_t source_size;
	char * source_str = calloc(4096, sizeof(char));
	fp = fopen("./generated/gemm.cl", "r");

	if (!fp) {
		fprintf(stderr, "fread error");
		exit(1);
	}
	source_size = fread(source_str, 1, 4096, fp);
	fclose(fp);


	//char * source_str =
	//	"__kernel void GEMM_nn_cl(const int Ndim, const int Mdim, const int Pdim,__global float* A, __global float* B, __global float* C,int VECTOR_P_dim) {\n"\
		//	"int delta_Ndim = get_global_id(0);"\
	//	"int delta_Mdim = get_global_id(1);"\
	//	"float sum = 0.f;\n"\
	//	"for (int delta_Pdim = 0; delta_Pdim <VECTOR_P_dim; delta_Pdim++){	\n"\
	//	"float4 VEC4_A = vload4(delta_Pdim,A+delta_Ndim*Pdim);float4 VEC4_B = (float4)(B[(delta_Pdim*Mdim*4)+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+Mdim+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+Mdim+Mdim+delta_Mdim]); \n"\
	//	"barrier(CLK_GLOBAL_MEM_FENCE);"\
	//	"sum += dot(VEC4_A, VEC4_B);}\n"\
	//	"for (int i = 1; i <= Pdim - (VECTOR_P_dim << 2); ++i){"\
	//	"sum += A[(delta_Ndim+1)*Pdim - i] *B[Mdim*(Pdim-i)+delta_Mdim];}\n"\
	//	"C[delta_Ndim*Mdim + delta_Mdim] = sum;\n"\
	//	"}";

// program build
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, &source_size, &ErrorCode);
	//	fprintf(stderr, "Program error %d\n", ErrorCode);
	//CL_SUCCESS;
	ErrorCode = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	//	fprintf(stderr, "Build Error %d\n", ErrorCode);
	//Create Kernel
	kernel = clCreateKernel(program, "GEMM_tn_cl", &ErrorCode);
	//	fprintf(stderr, "Kernel error %d\n", ErrorCode);
	//Sending arguments to kernel 
	ErrorCode = clSetKernelArg(kernel, 0, sizeof(int), &Ndim);
	//	fprintf(stderr, "Argument0 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(kernel, 1, sizeof(int), &Mdim);
	//	fprintf(stderr, "Argument1 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(kernel, 2, sizeof(int), &Pdim);
	//	fprintf(stderr, "Argument2 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&ReadBufferA);
	//	fprintf(stderr, "Argument3 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&ReadBufferB);
	//	fprintf(stderr, "Argument4 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&RwBufferC);
	//	fprintf(stderr, "Argument5 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(kernel, 6, sizeof(int), (void*)&VECTOR_P_dim);
	//	fprintf(stderr, "Argument6 error %d\n", ErrorCode);

	//Filling Buffer with Matrices
	ErrorCode = clEnqueueWriteBuffer(command_queue, ReadBufferA, CL_TRUE, 0, sizeof(float)*szA, A, 0, NULL, NULL);
	//	fprintf(stderr, "Command queue write BufferA error %d\n", ErrorCode);
	ErrorCode = clEnqueueWriteBuffer(command_queue, ReadBufferB, CL_TRUE, 0, sizeof(float)*szB, B, 0, NULL, NULL);
	//	fprintf(stderr, "Command queue write BufferB error %d\n", ErrorCode);

	//Sending Queues by NDRanges
	GlobalDimensions[0] = (size_t)Ndim; GlobalDimensions[1] = (size_t)Mdim;
	ErrorCode = clEnqueueNDRangeKernel(command_queue, kernel, NDim, NULL, GlobalDimensions, NULL, 0, NULL, NULL);
	//	fprintf(stderr, "Command queue sending error %d\n", ErrorCode);
	clFinish(command_queue);



	ErrorCode = clEnqueueReadBuffer(command_queue, RwBufferC, CL_TRUE, 0, sizeof(float)*szC, C, 0, NULL, NULL);
	//	fprintf(stderr, "Buffer Read error %d\n", ErrorCode);
	free(source_str);
	clFlush(command_queue);
	clFinish(command_queue);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseMemObject(ReadBufferA);
	clReleaseMemObject(ReadBufferB);
	clReleaseMemObject(RwBufferC);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);


	return C;
}
