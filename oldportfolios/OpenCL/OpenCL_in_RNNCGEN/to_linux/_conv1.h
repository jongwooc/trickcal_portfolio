#include <string.h>

#include "conv.h"
#include "variable.h"
#include "gemm.h"

#if (1)
static float relu(float x) { return x <= 0 ? 0 : x; }
static float relu_grad(float x) { return x <= 0 ? 0 : 1; }
#endif
#if (0)
static float leaky_relu(float x) { return x <= 0 ? x * 0.1 : x; }
static float leaky_relu_grad(float x) { return x <= 0 ? 0.1 : 1; }
#endif

float pWorkspace[19600];

static float buffer[520 + BATCH * 23040];
static float *pfWeightUpdate = &buffer[0];
static float *pfBiasUpdate = &buffer[500];
static float *pfDelta = &buffer[520];
static float *pfData = &buffer[520 + BATCH * 11520];

static inline void add_bias(float *output, float *biases) {
    int b = 0, i = 0, j = 0;
    int nOutputArea = 0, nOutChannel = 0;

    nOutputArea = 576;
    nOutChannel = 20;

    for (b = 0; b < 64; b++) {
        for (i = 0; i < 20; i++) {
            for (j = 0; j < 576; j++) {
                output[(b * nOutChannel + i) * nOutputArea + j] += biases[i];
            }
        }
    }
}

static inline void backward_bias(float *bias_updates, float *delta) {
    int b = 0, i = 0, j = 0;
    int nOutputArea = 0, nOutChannel = 0;

    nOutputArea = 576;
    nOutChannel = 20;

    for (b = 0; b < 64; b++) {
        for (i = 0; i < 20; i++) {
            for (j = 0; j < 576; j++) {
                bias_updates[i] += delta[nOutputArea * (i + b * nOutChannel) + j];
            }
        }
    }
}

static inline void col2im_cpu(float *data_col, float *data_im) {
    int nKernelSize = 0, nStride = 0;
    int nPrevHeight = 0, nPrevWidth = 0;

    int c = 0, h = 0, w = 0;
    int w_offset = 0, h_offset = 0, c_im = 0;
    int im_row = 0, im_col = 0, col_index = 0;
    int height_col = 0, width_col = 0;

    nKernelSize = 5;
    nStride = 1;
    nPrevHeight = 28;
    nPrevWidth = 28;

    height_col = (nPrevHeight - nKernelSize + 2 * 0) / nStride + 1;
    width_col = (nPrevWidth - nKernelSize + 2 * 0) / nStride + 1;

    for (c = 0; c < 25; c++) {
        w_offset = c % nKernelSize;
        h_offset = (c / nKernelSize) % nKernelSize;
        c_im = c / nKernelSize / nKernelSize;
        for (h = 0; h < (28 - 5 + 2 * 0) / 1 + 1; h++) {
            for (w = 0; w < (28 - 5 + 2 * 0) / 1 + 1; w++) {
                im_row = h_offset + h * nStride;
                im_col = w_offset + w * nStride;
                col_index = (c * height_col + h) * width_col + w;
                if (!(im_row < 0 || im_col < 0 || im_row >= nPrevHeight || im_col >= nPrevWidth))
                   data_im[im_col + nPrevWidth * (im_row + nPrevHeight * c_im)] += data_col[col_index];
            }
        }
    }
}

static inline void im2col_cpu(float *data_im, float *data_col) {
    int nKernelSize = 0, nStride = 0;
    int nPrevHeight = 0, nPrevWidth = 0;

    int c = 0, h = 0, w = 0;
    int w_offset = 0, h_offset = 0, c_im = 0;
    int im_row = 0, im_col = 0, col_index = 0;
    int height_col = 0, width_col = 0;

    nKernelSize = 5;
    nStride = 1;
    nPrevHeight = 28;
    nPrevWidth = 28;

    height_col = (nPrevHeight - nKernelSize + 2 * 0) / nStride + 1;
    width_col = (nPrevWidth - nKernelSize + 2 * 0) / nStride + 1;

    for (c = 0; c < 25; c++) {
        w_offset = c % nKernelSize;
        h_offset = (c / nKernelSize) % nKernelSize;
        c_im = c / nKernelSize / nKernelSize;
        for (h = 0; h < (28 - 5 + 2 * 0) / 1 + 1; h++) {
            for (w = 0; w < (28 - 5 + 2 * 0) / 1 + 1; w++) {
                im_row = h_offset + h * nStride;
                im_col = w_offset + w * nStride;
                col_index = (c * height_col + h) * width_col + w;
                data_col[col_index] = 
                    (im_row < 0 || im_col < 0 || im_row >= nPrevHeight || im_col >= nPrevWidth) ?
                    0 : data_im[im_col + nPrevWidth * (im_row + nPrevHeight * c_im)];
            }
        }
    }
}

static inline void bias_update(float *pfBias) {
    int i;
    
    for (i = 0; i < 20; i++) 
        pfBias[i] += fLearningRate / 64 * pfBiasUpdate[i];

    for (i = 0; i < 20; i++) 
        pfBiasUpdate[i] *= MOMENTUM;
}

static inline void weight_update(float *pfWeight) {
    int i;

    for (i = 0; i < 500; i++) 
        pfWeightUpdate[i] -= DECAY / 64 * pfWeight[i];

    for (i = 0; i < 500; i++) 
        pfWeight[i] += fLearningRate / 64 * pfWeightUpdate[i];

    for (i = 0; i < 500; i++) 
        pfWeightUpdate[i] *= MOMENTUM;
}
