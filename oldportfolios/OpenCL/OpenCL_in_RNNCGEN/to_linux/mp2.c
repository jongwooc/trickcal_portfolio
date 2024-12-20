#include <string.h>
#include <float.h>

#include "pool.h"
#include "variable.h"

static float buffer[3 * BATCH * 800];
static float *pfData = &buffer[0];
static int *pnIndex = (int *)&buffer[BATCH * 800];
static float *pfDelta = &buffer[2 * BATCH * 800];

float *get_mp2_delta() { return pfDelta; }
float *get_mp2_data() { return pfData; }

#if 1
static float max_pool(float *pfPrevData, int anArgs[3], int *pnIndex) {
    int nPrevWidth = 0, nPrevHeight = 0, nPrevChannel = 0;
    int nStride = 0, nSize = 0;
    int nLoopWidth = 0, nLoopHeight = 0;

    nPrevWidth = 8;
    nPrevHeight = 8;
    nPrevChannel = 50;

    nStride = 2;
    nSize = 2;

    float fResult = -FLT_MAX;
    int szIdx = -1;
    for (nLoopHeight = 0; nLoopHeight < nSize; nLoopHeight++) {
        int nHeightOffset = anArgs[2] * nStride + nLoopHeight;
        for (nLoopWidth = 0; nLoopWidth < nSize; nLoopWidth++) {
            int nWidthOffset = anArgs[3] * nStride + nLoopWidth;
            int nOffset = anArgs[0] * nPrevChannel * nPrevWidth * nPrevHeight +
                          anArgs[1] * nPrevWidth * nPrevHeight +
                          nHeightOffset * nPrevWidth + nWidthOffset;
            int valid = (nHeightOffset >= 0 && nHeightOffset < nPrevHeight &&
                         nWidthOffset >= 0 && nWidthOffset < nPrevWidth);
            float fVal = (valid != 0) ? pfPrevData[nOffset] : -FLT_MAX;

            szIdx = (fResult < fVal) ? nOffset : szIdx;
            fResult = (fResult < fVal) ? fVal : fResult;
        }
    }
    *pnIndex = szIdx;

    return fResult;
}
#endif

#if 0
static float avg_pool(float *pfPrevData, int anArgs[3], int *pnIndex) {
    int nPrevWidth = 0, nPrevHeight = 0, nPrevChannel = 0;
    int nStride = 0, nSize = 0;
    int nLoopWidth = 0, nLoopHeight = 0;

    nPrevWidth = 8;
    nPrevHeight = 8;
    nPrevChannel = 50;

    nStride = 2;
    nSize = 2;

    float fResult = -FLT_MAX;
    int szIdx = -1;
    float nSum = 0;
    for (nLoopHeight = 0; nLoopHeight < nSize; nLoopHeight++) {
        int nHeightOffset = anArgs[2] * nStride + nLoopHeight;
        for (nLoopWidth = 0; nLoopWidth < nSize; nLoopWidth++) {
            int nWidthOffset = anArgs[3] * nStride + nLoopWidth;
            int nOffset = anArgs[0] * nPrevChannel * nPrevWidth * nPrevHeight +
                          anArgs[1] * nPrevWidth * nPrevHeight +
                          nHeightOffset * nPrevWidth + nWidthOffset;
            int valid = (nHeightOffset >= 0 && nHeightOffset < nPrevHeight &&
                         nWidthOffset >= 0 && nWidthOffset < nPrevWidth);
            float fVal = (valid != 0) ? pfPrevData[nOffset] : -FLT_MAX;

            nSum += fVal;
            szIdx = nOffset;
        }
    }
    *pnIndex = szIdx;

    fResult = nSum / (nSize * nSize);
    return fResult;
}
#endif

void mp2_forward(float *pfPrevData) {
    int i, j, k, l;
    memset(pfDelta, 0, sizeof(float) * 51200);

    for (i = 0; i < 64; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 4; k++) {
                for (l = 0; l < 4; l++) {
                    int anArgs[4] = {i, j, k, l};
                    int idx = i * 50 * 4 * 4 + j * 4 * 4 + k * 4 + l;
                    pfData[idx] = max_pool(pfPrevData, anArgs, &pnIndex[idx]);
                }
            }
        }
    }
}

void mp2_backprop(float *pfPrevDelta) {
    int i;
#if 0
    int nWidth;
    int nSize;
#endif

    if (pfPrevDelta == NULL)
        return;

#if 0
    nSize = 2;
    nWidth = 4;
#endif

    for (i = 0; i < 51200; i++) {
        int nIndex = pnIndex[i];
#if 1
        pfPrevDelta[nIndex] += pfDelta[i];
#endif
#if 0
        int j = 0, k = 0;
        for (j = 0; j < nSize; j++)
            for (k = 0; k < nSize; k++)
                pfPrevDelta[nIndex - j * nWidth - k] += pfDelta[j] / (nSize * nSize);
#endif
    }
}
