__kernel void GEMM_nn_cl(const int Ndim, const int Mdim, const int Pdim,__global float* A, __global float* B, __global float* C,int VECTOR_P_dim) 
{
	int delta_Ndim = get_global_id(0);
	int delta_Mdim = get_global_id(1);
	float sum = 0.f;
	for (int delta_Pdim = 0; delta_Pdim <VECTOR_P_dim; delta_Pdim++)
	{	
		float4 VEC4_A = vload4(delta_Pdim,A+delta_Ndim*Pdim);
		float4 VEC4_B = (float4)(B[(delta_Pdim*Mdim*4)+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+Mdim+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+Mdim+Mdim+delta_Mdim]); 
		
		
		sum += dot(VEC4_A, VEC4_B);
				barrier(CLK_GLOBAL_MEM_FENCE);

	}

	for (int i = 1; i <= Pdim - (VECTOR_P_dim << 2); ++i)
	{
		sum += A[(delta_Ndim+1)*Pdim - i] *B[Mdim*(Pdim-i)+delta_Mdim];	
		barrier(CLK_GLOBAL_MEM_FENCE);	
	}

	C[delta_Ndim*Mdim + delta_Mdim] = sum;		
}


__kernel void GEMM_nt_cl(const int Ndim, const int Mdim, const int Pdim,__global float* A, __global float* B, __global float* C,int VECTOR_P_dim)  
{
	int delta_Ndim = get_global_id(0);
	int delta_Mdim = get_global_id(1);
	float sum = 0.f;
	for (int delta_Pdim = 0; delta_Pdim <VECTOR_P_dim; delta_Pdim++)
	{
		float4 VEC4_A = vload4(delta_Pdim+delta_Ndim*VECTOR_P_dim,A);
		float4 VEC4_B = vload4(delta_Pdim+delta_Mdim*VECTOR_P_dim,B);
		barrier(CLK_GLOBAL_MEM_FENCE);
		sum += dot(VEC4_A, VEC4_B);
		}
	C[delta_Ndim*Mdim + delta_Mdim] = sum;		
}

__kernel void GEMM_tn_cl(const int Ndim, const int Mdim, const int Pdim,__global float* A, __global float* B, __global float* C,int VECTOR_P_dim) {
	int delta_Ndim = get_global_id(0);		int delta_Mdim = get_global_id(1);		float sum = 0.f;
	for (int delta_Pdim = 0; delta_Pdim <VECTOR_P_dim; delta_Pdim++){	
		float4 VEC4_A = (float4)(A[(delta_Pdim*Ndim*4)+delta_Ndim],A[(delta_Pdim*Ndim*4)+Ndim+delta_Ndim],A[(delta_Pdim*Ndim*4)+Ndim+Ndim+delta_Ndim],A[(delta_Pdim*Ndim*4)+Ndim+Ndim+Ndim+delta_Ndim]);float4 VEC4_B = (float4)(B[(delta_Pdim*Mdim*4)+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+Mdim+delta_Mdim],B[(delta_Pdim*Mdim*4)+Mdim+Mdim+Mdim+delta_Mdim]); 
		barrier(CLK_GLOBAL_MEM_FENCE);	sum += dot(VEC4_A, VEC4_B);}
	for (int i = 1; i <= Pdim - (VECTOR_P_dim << 2); ++i){
		sum += A[Ndim*(Pdim-i)+delta_Ndim] *B[Mdim*(Pdim-i)+delta_Mdim];}
	C[delta_Ndim*Mdim + delta_Mdim] = sum;		}