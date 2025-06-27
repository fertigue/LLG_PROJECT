

#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define  RS485_Rce_OFF     PBout(15)=1 
#define  RS485_Rce_ON      PBout(15)=0 

#define UARTNunD       0xFF 
#define START_SYMBOL   0xA8







void Uart1_init(u32 bound);
void Uart1Send(unsigned char STData,unsigned char FunCom,unsigned char LengByte,unsigned char *Data);
void Uart1Run_Pack(void);
void Uart2_init(u32 bound);
void Uart2Send(unsigned char STData,unsigned char FunCom,unsigned char LengByte,unsigned char *Data);
void Uart2Run_Pack(void);
void Usart1SendData(unsigned char Data);
void Usart2SendData(unsigned char Data);

void  Uart1DataDropChk(void);  //´®¿Ú1¼ì²â³¬Ê±
void  Uart2DataDropChk(void);  //´®¿Ú2¼ì²â³¬Ê±

#endif


