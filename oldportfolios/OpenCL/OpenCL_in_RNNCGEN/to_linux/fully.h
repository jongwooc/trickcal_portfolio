#ifndef _FULLY_H_
#define _FULLY_H_

void fc1_forward(float *pfWeight, float *pfBias, float *pfPrevData);
void fc1_backprop(float *pfWeight, float *pfPrevData, float *pPrevDelta);
void fc1_update(float *pfWeight, float *pfBias);
float *get_fc1_delta();
float *get_fc1_data();
void fc2_forward(float *pfWeight, float *pfBias, float *pfPrevData);
void fc2_backprop(float *pfWeight, float *pfPrevData, float *pPrevDelta);
void fc2_update(float *pfWeight, float *pfBias);
float *get_fc2_delta();
float *get_fc2_data();


#endif /* ifndef _FULLY_H_ */
