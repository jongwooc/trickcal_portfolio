#include <string.h>

#include "fully.h"
#include "variable.h"
#include "gemm.h"
#include "_fc2.h"

float *get_fc2_delta() { return pfDelta; }
float *get_fc2_data() { return pfData; }

void fc2_forward(float *pfWeight, float *pfBias, float *pfPrevData) {
    int i;
    memset(pfData, 0, sizeof(float) * 640);
    memset(pfDelta, 0, sizeof(float) * 640);

    // gemm_nt
    fc2_gemm_nt(pfPrevData, pfWeight, pfData);
//	use_openCL_nt(G_test1, 64, 10, 500, pfPrevData, pfWeight, pfData);
    // axpy
    axpy_forward(pfBias);

    if (0) {
        for (i = 0; i < 640; i++) {
            pfData[i] = relu(pfData[i]);
        }
    }
}

void fc2_backprop(float *pfWeight, float *pfPrevData, float *pfPrevDelta) {
    int i;

    if (0) {
        for (i = 0; i < 640; i++) {
            pfDelta[i] *= relu_grad(pfData[i]);
        }
    }

    // axpy_cpu
    axpy_backprop();

    // gemm_tn
    fc2_gemm_tn(pfDelta, pfPrevData, pfWeightUpdate);

    if (pfPrevDelta) {
        // gemm_nn
//        fc2_gemm_nn(pfDelta, pfWeight, pfPrevDelta);
		use_openCL_nn(G_test1, 64,500,10, pfDelta, pfWeight, pfPrevDelta);
    }
}

void fc2_update(float *pfWeight, float *pfBias) {
    bias_update(pfBias);
    weight_update(pfWeight);
}
