#include "gemm.h"
#include "fixed_point.h"



/*
	nn	nt	tn

C = N*M	N*M	N*M

A = N*P	N*P	P*N

B = P*M	M*P	P*M
*/


inline void conv1_gemm_nn(float *A, float *B, float *C) {
    int i, j, k;

    for (i = 0; i < 20; i++) {
        for (k = 0; k < 25; k++) {
            register float A_PART = A[i * 25 + k];
            for (j = 0; j < 576; j++) {
                C[i * 576 + j] = FADD(C[i * 576 + j], FMUL(A_PART, B[k * 576 + j]));
            }
        }
    }
}


inline void conv1_gemm_nt(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 20; i++) {
		for (j = 0; j < 25; j++) {
			register float sum = 0;
			for (k = 0; k < 576; k++) {
				sum = FADD(sum, FMUL(A[i * 576 + k], B[j * 576 + k]));
			}
			C[i * 25 + j] = FADD(C[i * 25 + j], sum);
		}
	}
}


inline void conv1_gemm_tn(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 25; i++) {
		for (k = 0; k < 20; k++) {
			register float A_PART = A[k * 25 + i];
			for (j = 0; j < 576; j++) {
				C[i * 576 + j] = FADD(C[i * 576 + j], FMUL(A_PART, B[k * 576 + j]));
			}
		}
	}
}

inline void conv2_gemm_nn(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 50; i++) {
		for (k = 0; k < 500; k++) {
			register float A_PART = A[i * 500 + k];
			for (j = 0; j < 64; j++) {
				C[i * 64 + j] = FADD(C[i * 64 + j], FMUL(A_PART, B[k * 64 + j]));
			}
		}
	}
}
inline void conv2_gemm_nt(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 50; i++) {
		for (j = 0; j < 500; j++) {
			register float sum = 0;
			for (k = 0; k < 64; k++) {
				sum = FADD(sum, FMUL(A[i * 64 + k], B[j * 64 + k]));
			}
			C[i * 500 + j] = FADD(C[i * 500 + j], sum);
		}
	}
}

inline void conv2_gemm_tn(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 500; i++) {
		for (k = 0; k < 50; k++) {
			register float A_PART = A[k * 500 + i];
			for (j = 0; j < 64; j++) {
				C[i * 64 + j] = FADD(C[i * 64 + j], FMUL(A_PART, B[k * 64 + j]));
			}
		}
	}
}

inline void fc1_gemm_nn(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 64; i++) {
		for (k = 0; k < 500; k++) {
			register float A_PART = A[i * 500 + k];
			for (j = 0; j < 800; j++) {
				C[i * 800 + j] = FADD(C[i * 800 + j], FMUL(A_PART, B[k * 800 + j]));
			}
		}
	}
}

inline void fc1_gemm_nt(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 64; i++) {
		for (j = 0; j < 500; j++) {
			register float sum = 0;
			for (k = 0; k < 800; k++) {
				sum = FADD(sum, FMUL(A[i * 800 + k], B[j * 800 + k]));
			}
			C[i * 500 + j] = FADD(C[i * 500 + j], sum);
		}
	}
}


inline void fc1_gemm_tn(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 500; i++) {
		for (k = 0; k < 64; k++) {
			register float A_PART = A[k * 500 + i];
			for (j = 0; j < 800; j++) {
				C[i * 800 + j] = FADD(C[i * 800 + j], FMUL(A_PART, B[k * 800 + j]));
			}
		}
	}
}


inline void fc2_gemm_nn(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 64; i++) {
		for (k = 0; k < 10; k++) {
			register float A_PART = A[i * 10 + k];
			for (j = 0; j < 500; j++) {
				C[i * 500 + j] = FADD(C[i * 500 + j], FMUL(A_PART, B[k * 500 + j]));
			}
		}
	}
}
inline void fc2_gemm_nt(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 64; i++) {
		for (j = 0; j < 10; j++) {
			register float sum = 0;
			for (k = 0; k < 500; k++) {
				sum = FADD(sum, FMUL(A[i * 500 + k], B[j * 500 + k]));
			}
			C[i * 10 + j] = FADD(C[i * 10 + j], sum);
		}
	}
}
inline void fc2_gemm_tn(float *A, float *B, float *C) {
	int i, j, k;

	for (i = 0; i < 10; i++) {
		for (k = 0; k < 64; k++) {
			register float A_PART = A[k * 10 + i];
			for (j = 0; j < 500; j++) {
				C[i * 500 + j] = FADD(C[i * 500 + j], FMUL(A_PART, B[k * 500 + j]));
			}
		}
	}
}
