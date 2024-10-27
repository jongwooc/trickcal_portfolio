#ifndef _POOL_H_
#define _POOL_H_

void mp1_forward(float *pfPrevData);
void mp1_backprop(float *pPrevDelta);
float *get_mp1_delta();
float *get_mp1_data();void mp2_forward(float *pfPrevData);
void mp2_backprop(float *pPrevDelta);
float *get_mp2_delta();
float *get_mp2_data();

#endif /* ifndef _POOL_H_ */
