#pragma once

#ifndef _GEMM_H_
#define _GEMM_H_

#include <CL/cl.h>

void conv1_gemm_nn(float *A, float *B, float *C);
void conv1_gemm_nt(float *A, float *B, float *C);
void conv1_gemm_tn(float *A, float *B, float *C);
void conv2_gemm_nn(float *A, float *B, float *C);
void conv2_gemm_nt(float *A, float *B, float *C);
void conv2_gemm_tn(float *A, float *B, float *C);
void fc1_gemm_nn(float *A, float *B, float *C);
void fc1_gemm_nt(float *A, float *B, float *C);
void fc1_gemm_tn(float *A, float *B, float *C);
void fc2_gemm_nn(float *A, float *B, float *C);
void fc2_gemm_nt(float *A, float *B, float *C);
void fc2_gemm_tn(float *A, float *B, float *C);

//
//float* GEMM_nt(int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C);
//float* GEMM_tn(int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C);
#endif /* ifndef _GEMM_H_ */


////openCL value for Global

typedef struct _openCL_Values {
	//basic items for openCL
	//Devices
	cl_device_id device_id[2];
	cl_uint num_device;
	//Platforms
	cl_platform_id platform_id;
	cl_platform_info platform_info;
	cl_uint num_platform;
	//Context : Where to send Kernel
	cl_context context;
	//Program : Somthing to compile
//	cl_program program;
	cl_program program_nn;
	cl_program program_nt;
	cl_program program_tn;
	cl_program program_transpose;
	//Command : What to do
	cl_command_queue command_queue[2];
	char queue_sign;

	//Extra program for non-host calculator
	cl_kernel kernel[3];

	//Memory for Matrices : all of them are global
	//float mainbuffer1[1024*1024*96];
	//float mainbuffer2[1024 * 1024 * 96];
	//float mainbuffer3[1024 * 1024 * 96];
	cl_mem mainbuffer_1;
	cl_mem mainbuffer_2;
	cl_mem mainbuffer_3;


}openCL_Values;
extern openCL_Values * G_test1;

void init_openCL(openCL_Values * openCL_initializing);
void use_openCL_nn(openCL_Values * openCL_using, int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C);
void use_openCL_nt(openCL_Values * openCL_using, int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C);
void use_openCL_tn(openCL_Values * openCL_using, int _N_dim, int _M_dim, int _P_dim, float *A, float *B, float *C);
void clean_openCL(openCL_Values * openCL_Finishing);