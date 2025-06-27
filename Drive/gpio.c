
#include "gpio.h"
#include "TaskFun.h"

//******************SYS Power************************************************//
void Init_GPIO_SYS_Power(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	 	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	
	 GPIO_SetBits(GPIOB,GPIO_Pin_5);	
}																							
		
//*******PC10-LED1****PA15-LED2**********************************************//
void Init_GPIO_LED(void)
{   
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 
	 //LED//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_2;				 
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	 GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	 GPIO_SetBits(GPIOB,GPIO_Pin_14|GPIO_Pin_2);	
	 
	 //LEDR LEDG LEDY//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOC, &GPIO_InitStructure);	  
	 GPIO_ResetBits(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15); 
   
	 //buzzer//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	  
	 GPIO_ResetBits(GPIOA,GPIO_Pin_0); 
}

//**************************************************************************// 
void Init_GPIO_KEY(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	 //KEY //
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	
	 //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	   
	 GPIO_ResetBits(GPIOA,GPIO_Pin_5);
}

//**************************************************************************// 
void Init_GPIO_IN0(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	 	
	 //IN0 //
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	  
	 GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

void Init_GPIO_OUT0(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
		
	 //OUT0//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	 GPIO_Init(GPIOA, &GPIO_InitStructure);
	 GPIO_ResetBits(GPIOA,GPIO_Pin_4);
}

void Init_GPIO_ALERT(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
		
	 //ALERT//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	 GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Init_GPIO_BQ769xx_Wake(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	 //Wake//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	 GPIO_Init(GPIOB, &GPIO_InitStructure);
	 GPIO_ResetBits(GPIOB,GPIO_Pin_3);
}

void Init_GPIO_ChargeActiva(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	 	
	 //IN0 //
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	
	 //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	
	 GPIO_ResetBits(GPIOB,GPIO_Pin_13);
}

u16 ChargeActiva_Read_Gpio(u8 *activa)
{
   u16 rcd=0;
	 *activa=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);
	 return rcd;
}


void Init_GPIO_R485Pwr(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
		
	 //R485PWR//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	
   GPIO_ResetBits(GPIOA,GPIO_Pin_8); 	
}

void Init_GPIO_R485EN(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	 //R485EN//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	
   GPIO_ResetBits(GPIOB,GPIO_Pin_15); 	
}

void Init_GPIO_CanPwr(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
		
	 //CANPER//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	
   GPIO_ResetBits(GPIOA,GPIO_Pin_15); 	
}

void Init_GPIO_Uart2(void)
{
	//PA9-UART_485TXD     PA10-UART_485RXD        PA11-485EN   //     
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1 , ENABLE);
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AF_PP; 		
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
  GPIO_Init(GPIOA, &GPIO_InitStructure);		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU; 		
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Init_GPIO_IIC(void ) 
{
		//IIC_DATA   IIC_SCK
// 	GPIO_InitTypeDef GPIO_InitStructure;
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
// 	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Init_GPIO_CAN(void ) 
{
  GPIO_InitTypeDef GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
  //  CAN TX
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
  //  CAN RX  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
}

