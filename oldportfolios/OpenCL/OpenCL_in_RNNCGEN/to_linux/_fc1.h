#include <string.h>

#include "fully.h"
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

static float buffer[400500 + BATCH * 1000];
static float *pfWeightUpdate = &buffer[0];
static float *pfBiasUpdate = &buffer[400000];
static float *pfDelta = &buffer[400500];
static float *pfData = &buffer[400500 + BATCH * 500];

static inline void bias_update(float *pfBias) {
    int i;

    for (i = 0; i < 500; i++) 
        pfBias[i] += fLearningRate / 64 * pfBiasUpdate[i]; 

    for (i = 0; i < 500; i++) 
        pfBiasUpdate[i] *= MOMENTUM;
}

static inline void weight_update(float *pfWeight) {
    int i;

    for (i = 0; i < 400000; i++)
        pfWeightUpdate[i] -= DECAY / 64 * pfWeight[i];
                
    for (i = 0; i < 400000; i++)
        pfWeight[i] += fLearningRate / 64 * pfWeightUpdate[i];

    for (i = 0; i < 400000; i++)
        pfWeightUpdate[i] *= MOMENTUM;
}

static inline void axpy_forward(float *pfBias) {
    int i, j;
    int nOutput = 500;

    for (i = 0; i < 64; i++) {
        for (j = 0; j < 500; j++) {
            pfData[i * nOutput + j] += pfBias[j];
        }
    }
}

static inline void axpy_backprop() {
    int i, j;
    int nOutput = 500;

    for (i = 0; i < 64; i++) {
        register float *pfTemp = pfDelta + i * nOutput;
        for (j = 0; j < 500; j++) {
            pfBiasUpdate[j] += pfTemp[j];
        }
    }
}
