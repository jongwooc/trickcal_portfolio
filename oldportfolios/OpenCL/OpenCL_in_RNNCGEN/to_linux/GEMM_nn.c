#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define _CRT_SECURE_NO_WARNINGS

#include "gemm.h"
#include <stdio.h>
#include <stdlib.h>

void init_openCL(openCL_Values * openCL_initializing)
{
	
	int ErrorCode = 0;

	openCL_initializing->device_id[0] = NULL;
	openCL_initializing->device_id[1] = NULL;

	ErrorCode = clGetPlatformIDs(1, &openCL_initializing->platform_id, &openCL_initializing->num_platform);
	printf("platform num = %d\n\n", openCL_initializing->num_platform);
	ErrorCode = clGetDeviceIDs(openCL_initializing->platform_id, CL_DEVICE_TYPE_GPU, 1, &openCL_initializing->device_id[0],&openCL_initializing->num_device);
	if (ErrorCode) { printf("device0 id error = %d\n\n", ErrorCode); exit(1); }
//	ErrorCode = clGetDeviceIDs(openCL_initializing->platform_id, CL_DEVICE_TYPE_GPU, 1, &openCL_initializing->device_id[1], &openCL_initializing->num_device);
//	if (ErrorCode) { printf("device1 id error = %d\n\n", ErrorCode); exit(1); }

	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM,(cl_context_properties)openCL_initializing->platform_id ,0 };
	openCL_initializing->context = clCreateContext(properties, 1, &openCL_initializing->device_id, NULL, NULL, &ErrorCode);

	fprintf(stderr, "Context error: %d\n", ErrorCode);
	//context = clCreateContext(NULL, num_device, &device_id, NULL, NULL, NULL);

	//if you want to use openCL 1.2, use this funtion instead
	openCL_initializing->command_queue[0] = clCreateCommandQueue(openCL_initializing->context, openCL_initializing->device_id[0], CL_QUEUE_PROFILING_ENABLE, &ErrorCode);
	//	command_queue = clCreateCommandQueueWithProperties(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ErrorCode);
	if (ErrorCode) {	 printf("command queue device0 error = %d\n\n", ErrorCode); exit(1); }
//	openCL_initializing->command_queue[1] = clCreateCommandQueue(openCL_initializing->context, openCL_initializing->device_id[1], CL_QUEUE_PROFILING_ENABLE, &ErrorCode);
//	if (ErrorCode) {	 printf("command queue device1 error = %d\n\n", ErrorCode); exit(1); }





	openCL_initializing->mainbuffer_1 = clCreateBuffer(openCL_initializing->context, CL_MEM_ALLOC_HOST_PTR, 1024*1024*128,NULL, &ErrorCode);
	if (ErrorCode) {		fprintf(stderr, "mainbuffer create error: %d\n", ErrorCode); exit(1);	}
	openCL_initializing->mainbuffer_2 = clCreateBuffer(openCL_initializing->context, CL_MEM_ALLOC_HOST_PTR, 1024 * 1024 * 128, NULL, &ErrorCode);
	if (ErrorCode) {		fprintf(stderr, "mainbuffer create error: %d\n", ErrorCode); exit(1);	}
	openCL_initializing->mainbuffer_3 = clCreateBuffer(openCL_initializing->context, CL_MEM_ALLOC_HOST_PTR, 1024 * 1024 * 128, NULL, &ErrorCode);
	if (ErrorCode) {		fprintf(stderr, "mainbuffer create error: %d\n", ErrorCode); exit(1);	}



	//load cl file to source_str
	//FILE * fp;

	//size_t source_size;
	//char * source_str = calloc(4096, sizeof(char));
	//fp = fopen("./generated/gemm.cl", "r");

	//if (!fp) {
	//	fprintf(stderr, "fread error");
	//	exit(1);
	//}
	//source_size = fread(source_str, 1, 4096, fp);
	//fclose(fp);




	CL_SUCCESS;

	////source _nn///////////////////////////
	char * source_str_nn =
		"__kernel void GEMM_nn_cl(const int Ndim, const int Mdim, const int Pdim,__global float* A, __global float* B, __global float* C,int VECTOR_P_dim) {\n"\
		"int delta_Ndim = get_global_id(0);"\
		"int delta_Mdim = get_global_id(1);"\
		"float sum = 0.f;\n"\
		"if( 0<=delta_Ndim && delta_Ndim< Ndim && 0<=delta_Mdim && delta_Mdim <Mdim)"\
		"for (int delta_Pdim = 0; delta_Pdim <Pdim; delta_Pdim++){	\n"\
		"sum += A[delta_Ndim*Pdim+ delta_Pdim]*B[delta_Pdim*Mdim+delta_Mdim];}\n"\
		"C[delta_Ndim*Mdim + delta_Mdim] += sum;\n"\
		"}";

	fprintf(stderr, "Program build ready\n");
	// program build
	openCL_initializing->program_nn = clCreateProgramWithSource(openCL_initializing->context,1, (const char **)&source_str_nn, NULL, &ErrorCode);
		fprintf(stderr, "Program error %d\n", ErrorCode);
	//CL_SUCCESS;
	ErrorCode = clBuildProgram(openCL_initializing->program_nn,0, NULL, NULL, NULL, NULL);
	fprintf(stderr, "Build Error %d\n", ErrorCode);
	//Create Kernel
	fprintf(stderr, "%p\n", openCL_initializing->kernel[0]);
	openCL_initializing->kernel[0] = clCreateKernel(openCL_initializing->program_nn, "GEMM_nn_cl", &ErrorCode);
	fprintf(stderr, "Kernel GEMM_nn_cl error %d\n", ErrorCode);
	fprintf(stderr, "%p\n", openCL_initializing->kernel[0]);



	////source _nt///////////////////////////
	char * source_str_nt =
		"__kernel void GEMM_nt_cl(const int Ndim, const int Mdim, const int Pdim,__global float* A, __global float* B, __global float* C,int VECTOR_P_dim)  {\n"\
		"int delta_Ndim = get_global_id(0);"\
		"int delta_Mdim = get_global_id(1);"\
		"float sum = 0.f;\n"\
		"if( 0<=delta_Ndim && delta_Ndim< Ndim && 0<=delta_Mdim && delta_Mdim <Mdim)"\
		"	for (int delta_Pdim = 0; delta_Pdim <Pdim; delta_Pdim++){	\n"\
		"		sum += A[delta_Ndim*Pdim+delta_Pdim]*B[delta_Mdim*Pdim+delta_Pdim];}"\
		"	C[delta_Ndim*Mdim + delta_Mdim] += sum;\n}	";

	// program build
	openCL_initializing->program_nt = clCreateProgramWithSource(openCL_initializing->context, 1, (const char **)&source_str_nt, NULL, &ErrorCode);
	fprintf(stderr, "Program error %d\n", ErrorCode);
	//CL_SUCCESS;
	ErrorCode = clBuildProgram(openCL_initializing->program_nt, 0, NULL, NULL, NULL, NULL);
	fprintf(stderr, "Build Error %d\n", ErrorCode);
	//Create Kernel
	openCL_initializing->kernel[1] = clCreateKernel(openCL_initializing->program_nt, "GEMM_nt_cl", &ErrorCode);
	fprintf(stderr, "Kernel GEMM_nt_cl error %d\n", ErrorCode);



		////source _tn///////////////////////////
	char * source_str_tn =
		"__kernel void GEMM_tn_cl(const int Ndim, const int Mdim, const int Pdim,__global float* A, __global float* B, __global float* C,int VECTOR_P_dim)  {\n"\
		"int delta_Ndim = get_global_id(0);"\
		"int delta_Mdim = get_global_id(1);"\
		"float sum = 0.f;\n"\
		"if( 0<=delta_Ndim && delta_Ndim< Ndim && 0<=delta_Mdim && delta_Mdim <Mdim)"\
		"	for (int delta_Pdim = 0; delta_Pdim <Pdim; delta_Pdim++){	\n"\
		"sum += A[delta_Pdim*Ndim+ delta_Ndim]*B[delta_Pdim*Mdim+delta_Mdim];}\n"\
		"C[delta_Ndim*Mdim + delta_Mdim] += sum;\n"\

	// program build
	openCL_initializing->program_tn = clCreateProgramWithSource(openCL_initializing->context, 1, (const char **)&source_str_tn, NULL, &ErrorCode);
	fprintf(stderr, "Program error %d\n", ErrorCode);
	//CL_SUCCESS;
	ErrorCode = clBuildProgram(openCL_initializing->program_tn, 0, NULL, NULL, NULL, NULL);
	fprintf(stderr, "Build Error %d\n", ErrorCode);
	//Create Kernel
	openCL_initializing->kernel[2] = clCreateKernel(openCL_initializing->program_tn, "GEMM_tn_cl", &ErrorCode);
	fprintf(stderr, "Kernel GEMM_tn_cl error %d\n", ErrorCode);

}


void use_openCL_nn(openCL_Values * openCL_using, int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C)
{
	int  Ndim = _N_dim; int Mdim = _M_dim; int Pdim = _P_dim; int VECTOR_P_dim = Pdim >> 2;
	int szA = Ndim*Pdim; int szB = Pdim*Mdim; int szC = Ndim*Mdim;
	int ErrorCode = 0;
	int initvalue = 0;

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_1, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "ntBuffer1 fill error %d\n", ErrorCode); exit(1); }

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_2, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "ntBuffer2 fill error %d\n", ErrorCode); exit(1); }

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_3, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "ntBuffer3 fill error %d\n", ErrorCode); exit(1); }

	ErrorCode = clSetKernelArg(openCL_using->kernel[0], 0, sizeof(int), &Ndim);
	//fprintf(stderr, "Argument0 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[0], 1, sizeof(int), &Mdim);
	//fprintf(stderr, "Argument1 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[0], 2, sizeof(int), &Pdim);
	//fprintf(stderr, "Argument2 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[0], 3, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_1);
	//fprintf(stderr, "Argument3 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[0], 4, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_2);
	//fprintf(stderr, "Argument4 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[0], 5, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_3);
	//fprintf(stderr, "Argument5 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[0], 6, sizeof(int), &VECTOR_P_dim);
	//fprintf(stderr, "Argument6 error %d\n", ErrorCode);



	//Domains
	size_t GlobalDimensions[2];
	//size_t LocalDimensions[2];
	//Number of Working Dimensions, NOT "NDRange". 
	cl_uint NDim = 2;


	//Filling Buffer with Matrices
	ErrorCode = clEnqueueWriteBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_1, CL_TRUE, 0, sizeof(float)*szA, A, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "nnCommand queue write BufferA error %d\n", ErrorCode); exit(1); }
//	clFinish(openCL_using->command_queue[0]);
//	fprintf(stderr, "%p\n", B);
//	fprintf(stderr, "%ld\n", (long)sizeof(float)*szB);
//	clFinish(openCL_using->command_queue[0]);
	ErrorCode = clEnqueueWriteBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_2, CL_TRUE, 0, sizeof(float)*szB, B, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "nnCommand queue write BufferB error %d\n", ErrorCode); exit(1); }
//	clFinish(openCL_using->command_queue[0]);
	//Sending Queues by NDRanges
	GlobalDimensions[0] = (size_t)Ndim; GlobalDimensions[1] = (size_t)Mdim;
	ErrorCode = clEnqueueNDRangeKernel(openCL_using->command_queue[0], openCL_using->kernel[0], NDim, NULL, GlobalDimensions, NULL, 0, NULL, NULL);
//	fprintf(stderr, "Command queue sending error %d\n", ErrorCode);
//	clFinish(openCL_using->command_queue[0]);


	ErrorCode = clEnqueueReadBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_3, CL_TRUE, 0, sizeof(float)*szC, C, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "nnBuffer C Read error %d\n", ErrorCode); exit(1); }
	//	clFinish(openCL_using->command_queue[0]);
}


void use_openCL_nt(openCL_Values * openCL_using, int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C)
{
	int  Ndim = _N_dim; int Mdim = _M_dim; int Pdim = _P_dim; int VECTOR_P_dim = Pdim >> 2;
	int szA = Ndim*Pdim; int szB = Pdim*Mdim; int szC = Ndim*Mdim;
	int ErrorCode = 0;
	int initvalue = 0;

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_1, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "ntBuffer1 fill error %d\n", ErrorCode); exit(1); }

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_2, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "ntBuffer2 fill error %d\n", ErrorCode); exit(1); }

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_3, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "ntBuffer3 fill error %d\n", ErrorCode); exit(1); }



	ErrorCode = clSetKernelArg(openCL_using->kernel[1], 0, sizeof(int), &Ndim);
	//fprintf(stderr, "Argument0 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[1], 1, sizeof(int), &Mdim);
	//fprintf(stderr, "Argument1 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[1], 2, sizeof(int), &Pdim);
	//fprintf(stderr, "Argument2 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[1], 3, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_1);
	//fprintf(stderr, "Argument3 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[1], 4, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_2);
	//fprintf(stderr, "Argument4 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[1], 5, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_3);
	//fprintf(stderr, "Argument5 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[1], 6, sizeof(int), &VECTOR_P_dim);
	//fprintf(stderr, "Argument6 error %d\n", ErrorCode);



	//Domains
	size_t GlobalDimensions[2];
	//size_t LocalDimensions[2];
	//Number of Working Dimensions, NOT "NDRange". 
	cl_uint NDim = 2;


	//Filling Buffer with Matrices
	ErrorCode = clEnqueueWriteBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_1, CL_TRUE, 0, sizeof(float)*szA, A, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "ntCommand queue write BufferA error %d\n", ErrorCode); exit(1); }
	//	fprintf(stderr, "%p\n", B);
//	fprintf(stderr, "%ld\n", (long)sizeof(float)*szB);
//	clFinish(openCL_using->command_queue[0]);
	ErrorCode = clEnqueueWriteBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_2, CL_TRUE, 0, sizeof(float)*szB, B, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "ntCommand queue write BufferB error %d\n", ErrorCode); exit(1); }
	//	clFinish(openCL_using->command_queue[0]);
	//Sending Queues by NDRanges
	GlobalDimensions[0] = (size_t)Ndim; GlobalDimensions[1] = (size_t)Mdim;
	ErrorCode = clEnqueueNDRangeKernel(openCL_using->command_queue[0], openCL_using->kernel[1], NDim, NULL, GlobalDimensions, NULL, 0, NULL, NULL);
//		fprintf(stderr, "Command queue sending error %d\n", ErrorCode);
//	clFinish(openCL_using->command_queue[0]);


	ErrorCode = clEnqueueReadBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_3, CL_TRUE, 0, sizeof(float)*szC, C, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "ntBuffer C Read error %d\n", ErrorCode); exit(1); }
	//	clFinish(openCL_using->command_queue[0]);

}

void use_openCL_tn(openCL_Values * openCL_using, int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C)
{
	int  Ndim = _N_dim; int Mdim = _M_dim; int Pdim = _P_dim; int VECTOR_P_dim = Pdim >> 2;
	int szA = Ndim*Pdim; int szB = Pdim*Mdim; int szC = Ndim*Mdim;
	int ErrorCode = 0;
	int initvalue = 0;

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_1, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "tnBuffer1 fill error %d\n", ErrorCode); exit(1); }

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_2, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "tnBuffer2 fill error %d\n", ErrorCode); exit(1); }

	//ErrorCode = clEnqueueFillBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_3, &initvalue, sizeof(int), 0, 1024 * 1024 * 128, 0, NULL, NULL);
	//if (ErrorCode) { fprintf(stderr, "tnBuffer3 fill error %d\n", ErrorCode); exit(1); }


	ErrorCode = clSetKernelArg(openCL_using->kernel[2], 0, sizeof(int), &Ndim);
	//fprintf(stderr, "Argument0 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[2], 1, sizeof(int), &Mdim);
	//fprintf(stderr, "Argument1 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[2], 2, sizeof(int), &Pdim);
	//fprintf(stderr, "Argument2 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[2], 3, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_1);
	//fprintf(stderr, "Argument3 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[2], 4, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_2);
	//fprintf(stderr, "Argument4 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[2], 5, sizeof(cl_mem), (void*)&openCL_using->mainbuffer_3);
	//fprintf(stderr, "Argument5 error %d\n", ErrorCode);
	ErrorCode = clSetKernelArg(openCL_using->kernel[2], 6, sizeof(int), &VECTOR_P_dim);
	//fprintf(stderr, "Argument6 error %d\n", ErrorCode);



	//Domains
	size_t GlobalDimensions[2];
	//size_t LocalDimensions[2];
	//Number of Working Dimensions, NOT "NDRange". 
	cl_uint NDim = 2;


	//Filling Buffer with Matrices
	ErrorCode = clEnqueueWriteBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_1, CL_TRUE, 0, sizeof(float)*szA, A, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "tnCommand queue write BufferA error %d\n", ErrorCode); exit(1); }
	//	clFinish(openCL_using->command_queue[0]);
	ErrorCode = clEnqueueWriteBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_2, CL_TRUE, 0, sizeof(float)*szB, B, 0, NULL, NULL);
	if (ErrorCode) { fprintf(stderr, "tnCommand queue write BufferB error %d\n", ErrorCode); exit(1); }
	//	clFinish(openCL_using->command_queue[0]);
	//Sending Queues by NDRanges
	GlobalDimensions[0] = (size_t)Ndim; GlobalDimensions[1] = (size_t)Mdim;
	ErrorCode = clEnqueueNDRangeKernel(openCL_using->command_queue[0], openCL_using->kernel[2], NDim, NULL, GlobalDimensions, NULL, 0, NULL, NULL);
	//	fprintf(stderr, "Command queue sending error %d\n", ErrorCode);
	//	clFinish(openCL_using->command_queue[0]);


	ErrorCode = clEnqueueReadBuffer(openCL_using->command_queue[0], openCL_using->mainbuffer_3, CL_TRUE, 0, sizeof(float)*szC, C, 0, NULL, NULL);
	if (ErrorCode) {		fprintf(stderr, "tnBuffer C Read error %d\n", ErrorCode); exit(1);	}
	//	clFinish(openCL_using->command_queue[0]);

}

void clean_openCL(openCL_Values * openCL_Finishing)
{	
	clFlush(openCL_Finishing->command_queue[0]);
	clFinish(openCL_Finishing->command_queue[0]);
	clFlush(openCL_Finishing->command_queue[0]);
	clFinish(openCL_Finishing->command_queue[0]);
	clReleaseKernel(openCL_Finishing->kernel[0]);
	clReleaseKernel(openCL_Finishing->kernel[1]);
	clReleaseKernel(openCL_Finishing->kernel[2]);
	clReleaseProgram(openCL_Finishing->program_nn);
	clReleaseProgram(openCL_Finishing->program_nt);
	clReleaseProgram(openCL_Finishing->program_tn);
	clReleaseMemObject(openCL_Finishing->mainbuffer_1);
	clReleaseMemObject(openCL_Finishing->mainbuffer_2);
	clReleaseMemObject(openCL_Finishing->mainbuffer_3);
	
	clReleaseCommandQueue(openCL_Finishing->command_queue[0]);
	clReleaseContext(openCL_Finishing->context);
}