#include <string.h>

#include "conv.h"
#include "variable.h"
#include "gemm.h"
#include "_conv1.h"

float *get_conv1_delta() { return pfDelta; }
float *get_conv1_data() { return pfData; }

void conv1_forward(float *pfWeight, float *pfBias, float *pfPrevData) {
    int i = 0;
    int nOutputArea = 0, nOutput = 0, nOutChannel = 0;
    int nPrevArea = 0, nPrevChannel = 0;

    nOutputArea = 576;
    nOutput = 11520;
    nOutChannel = 20;

    nPrevArea = 28 * 28;
    nPrevChannel = 1;

    memset(pfData, 0, sizeof(float) * 64 * nOutput);
    memset(pfDelta, 0, sizeof(float) * 64 * nOutput);

    float *pfTempData = pfData;
    for (i = 0; i < 64; i++) {
        // im2col_cpu
        im2col_cpu(pfPrevData, pWorkspace);

        // gemm_nn
//		conv1_gemm_nn(pfWeight, pWorkspace, pfTempData);
		use_openCL_nn(G_test1, 20, 576, 25, pfWeight, pWorkspace, pfTempData);
        pfTempData += nOutChannel * nOutputArea;
        pfPrevData += nPrevChannel * nPrevArea;
    }

    // add_bias
    add_bias(pfData, pfBias);

    if (0) {
        for (i = 0; i < 737280; i++) {
            pfData[i] = relu(pfData[i]);
        }
    }
}

void conv1_backprop(float *pfWeight, float *pfPrevData, float *pfPrevDelta) {
    int i = 0, b = 0;
    int nOutputArea = 0, nOutput = 0;
    int nKernelWeights = 0;
    int nPrevArea = 0, nPrevChannel = 0;

    nOutputArea = 576;
    nOutput = 11520;

    nKernelWeights = 25;

    nPrevArea = 28 * 28;
    nPrevChannel = 1;

    if (0) {
        for (i = 0; i < 737280; i++) {
            pfDelta[i] *= relu_grad(pfData[i]);
        }
    }

    // backward_bias
    backward_bias(pfBiasUpdate, pfDelta);

    float *pfTemp = pfDelta;
    for (b = 0; b < 64; b++) {
        // im2col_cpu
        im2col_cpu(pfPrevData, pWorkspace);
                
        // gemm_nt
//		conv1_gemm_nt(pfTemp, pWorkspace, pfWeightUpdate);
		use_openCL_nt(G_test1, 20, 25, 576, pfTemp, pWorkspace, pfWeightUpdate);
        if (pfPrevDelta) {
            memset(pWorkspace, 0,
                   sizeof(float) *
                       (nKernelWeights * nOutputArea + nOutputArea));

            // gemm_tn
            conv1_gemm_tn(pfWeight, pfTemp, pWorkspace);

            // col2im_cpu
            col2im_cpu(pWorkspace, pfPrevDelta);

            pfPrevDelta += nPrevChannel * nPrevArea;
        }
        pfTemp += nOutput;
        pfPrevData += nPrevChannel * nPrevArea;
    }
}

void conv1_update(float *pfWeight, float *pfBias) {
    bias_update(pfBias);
    weight_update(pfWeight);
}
