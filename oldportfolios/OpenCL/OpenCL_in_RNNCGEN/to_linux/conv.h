#ifndef _CONV_H_
#define _CONV_H_

void conv1_forward(float *pfWeight, float *pfBias, float *pfPrevData);
void conv1_backprop(float *pfWeight, float *pfPrevData, float *pPrevDelta);
void conv1_update(float *pfWeight, float *pfBias);
float *get_conv1_delta();
float *get_conv1_data();
void conv2_forward(float *pfWeight, float *pfBias, float *pfPrevData);
void conv2_backprop(float *pfWeight, float *pfPrevData, float *pPrevDelta);
void conv2_update(float *pfWeight, float *pfBias);
float *get_conv2_delta();
float *get_conv2_data();


#endif /* ifndef _CONV_H_ */
