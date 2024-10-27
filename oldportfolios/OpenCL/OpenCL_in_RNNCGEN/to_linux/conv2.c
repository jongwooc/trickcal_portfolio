#include <string.h>

#include "conv.h"
#include "variable.h"
#include "gemm.h"
#include "_conv2.h"

float *get_conv2_delta() { return pfDelta; }
float *get_conv2_data() { return pfData; }

void conv2_forward(float *pfWeight, float *pfBias, float *pfPrevData) {
    int i = 0;
    int nOutputArea = 0, nOutput = 0, nOutChannel = 0;
    int nPrevArea = 0, nPrevChannel = 0;

    nOutputArea = 64;
    nOutput = 3200;
    nOutChannel = 50;

    nPrevArea = 12 * 12;
    nPrevChannel = 20;

    memset(pfData, 0, sizeof(float) * 64 * nOutput);
    memset(pfDelta, 0, sizeof(float) * 64 * nOutput);

    float *pfTempData = pfData;
    for (i = 0; i < 64; i++) {
        // im2col_cpu
        im2col_cpu(pfPrevData, pWorkspace);

        // gemm_nn
//        conv2_gemm_nn(pfWeight, pWorkspace, pfTempData);
		use_openCL_nn(G_test1, 50, 64, 500, pfWeight, pWorkspace, pfTempData);
    
        pfTempData += nOutChannel * nOutputArea;
        pfPrevData += nPrevChannel * nPrevArea;
    }

    // add_bias
    add_bias(pfData, pfBias);

    if (0) {
        for (i = 0; i < 204800; i++) {
            pfData[i] = relu(pfData[i]);
        }
    }
}

void conv2_backprop(float *pfWeight, float *pfPrevData, float *pfPrevDelta) {
    int i = 0, b = 0;
    int nOutputArea = 0, nOutput = 0;
    int nKernelWeights = 0;
    int nPrevArea = 0, nPrevChannel = 0;

    nOutputArea = 64;
    nOutput = 3200;

    nKernelWeights = 500;

    nPrevArea = 12 * 12;
    nPrevChannel = 20;

    if (0) {
        for (i = 0; i < 204800; i++) {
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
//        conv2_gemm_nt(pfTemp, pWorkspace, pfWeightUpdate);
		use_openCL_nt(G_test1, 50, 500, 64, pfTemp, pWorkspace, pfWeightUpdate);
        if (pfPrevDelta) {
            memset(pWorkspace, 0,
                   sizeof(float) *
                       (nKernelWeights * nOutputArea + nOutputArea));

            // gemm_tn
            conv2_gemm_tn(pfWeight, pfTemp, pWorkspace);

            // col2im_cpu
            col2im_cpu(pWorkspace, pfPrevDelta);

            pfPrevDelta += nPrevChannel * nPrevArea;
        }
        pfTemp += nOutput;
        pfPrevData += nPrevChannel * nPrevArea;
    }
}

void conv2_update(float *pfWeight, float *pfBias) {
    bias_update(pfBias);
    weight_update(pfWeight);
}
