#include <string.h>

#include "fully.h"
#include "variable.h"
#include "gemm.h"
#include "_fc1.h"

float *get_fc1_delta() { return pfDelta; }
float *get_fc1_data() { return pfData; }

void fc1_forward(float *pfWeight, float *pfBias, float *pfPrevData) {
    int i;
    memset(pfData, 0, sizeof(float) * 32000);
    memset(pfDelta, 0, sizeof(float) * 32000);

    // gemm_nt
    fc1_gemm_nt(pfPrevData, pfWeight, pfData);
//	use_openCL_nt(G_test1, 64, 500, 800, pfPrevData, pfWeight, pfData);

    // axpy
    axpy_forward(pfBias);

    if (0) {
        for (i = 0; i < 32000; i++) {
            pfData[i] = relu(pfData[i]);
        }
    }
}

void fc1_backprop(float *pfWeight, float *pfPrevData, float *pfPrevDelta) {
    int i;

    if (0) {
        for (i = 0; i < 32000; i++) {
            pfDelta[i] *= relu_grad(pfData[i]);
        }
    }

    // axpy_cpu
    axpy_backprop();

    // gemm_tn
    fc1_gemm_tn(pfDelta, pfPrevData, pfWeightUpdate);

    if (pfPrevDelta) {
        // gemm_nn
//        fc1_gemm_nn(pfDelta, pfWeight, pfPrevDelta);
		use_openCL_nn(G_test1, 64, 800, 500, pfDelta, pfWeight, pfPrevDelta);
    }
}

void fc1_update(float *pfWeight, float *pfBias) {
    bias_update(pfBias);
    weight_update(pfWeight);
}
