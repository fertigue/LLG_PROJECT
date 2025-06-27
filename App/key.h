#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY_SW  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)


typedef struct {
	   unsigned char   Key_num[5];
	   unsigned short  KeyDataK;
	   unsigned char   Key_count[5];
	   unsigned char   Key_switch[5];
	   }KEY_Pare;

extern   KEY_Pare    KEYPare;


void Key_Can(void);
		    
#endif
