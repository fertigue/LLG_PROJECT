

#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

#define   ADC1_DR_Address    0x4001244C 
#define  ADCSamp_DEFAULTS     0        // 初始化参数

typedef struct {
unsigned char ia;
}ADCSamp;
extern   ADCSamp     ADCSampPare;






void ADC_Init_Sam(void);
void DMA_Init_Adc(void);
void ADC_GetSample(void);
signed int TemChange(unsigned int uiADCV);



#endif 



