
#include "usart.h"	
#include "sys.h"
#include "adc.h"
#include "timer.h"
#include "TaskFun.h" 
#include "Flash.h"	
#include "gpio.h"  
#include "timer.h" 
#include "delay.h"
#include "cache_msg.h"
#include "Rs485Data.h"

//********************全局变量*********************//


//******************全局变量END********************//




//**************局部变量****************************//
static u8 Uart1Recv[UARTNunD]={0};
static u8  Uart1RecvBuf[UARTNunD]={0};
static u8 RBUp1=0;
static u8 RBDown1=0;
static u8 Uart1ACC=0;
static u8 Uart1CntR=0;
static u8 Uart1CntTT=0;
static u8 Uart1Stt=0;
static u8 Uart1_State=0;
static u16  Uart1ErrCountTime=0; //485通信数据丢失计时

static u8 Uart2Recv[UARTNunD]={0};
static u8 Uart2RecvBuf[UARTNunD]={0};
static u8 RBUp2=0;
static u8 RBDown2=0;
static u8 Uart2ACC=0;
static u8 Uart2CntR=0;
static u8 Uart2CntTT=0;
static u8 Uart2Stt=0;
static u8 Uart2_State=0;
static u16  Uart2ErrCountTime=0; //232通信数据丢失计时
//*************局部变量END****************************//



////////////////////////////////////////UART1//////////////////////////////////////////////////////////////

void Uart1_init(u32 bound)
{
    //GPIO端口设置
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
	  GPIO_InitTypeDef  GPIO_InitStructure;
	  USART_DeInit(USART1);  //复位串口1
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);
 	  GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
	  
	  //USART1_TX   PA.9
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
 		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽输出
 		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 		GPIO_Init(GPIOA, &GPIO_InitStructure);
    //USART1_RX	  PA.10
 		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
 		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
 		GPIO_Init(GPIOA,&GPIO_InitStructure);
	  
    //USART 初始化设置
		USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
		USART_Init(USART1, &USART_InitStructure); //初始化串口
 
   //Usart1 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
		 
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART1, ENABLE);                    //使能串口 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Usart1_Send_BYTE(unsigned char Data)
{
	  while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) Data; 
} 

void Uart1_Send_Pack(unsigned char STData,unsigned char FunCom,unsigned char LengByte,unsigned char *Data)
{
    static u8  i=0;
    static u8  CRCnum=0;
    static u8 SendData[UARTNunD]={0};

    RS485_Rce_OFF;
		SendData[0]=START_SYMBOL;
		CRCnum=START_SYMBOL; 
    SendData[1]=STData;
    CRCnum+=STData; 
    SendData[2]=FunCom;
    CRCnum+=FunCom;
    SendData[3]=LengByte;
    CRCnum+=LengByte;
    for(i=0;i<LengByte;i++)
    {    
         SendData[i+4]=Data[i];
         CRCnum+=Data[i];
    }
    SendData[LengByte+4]=0-CRCnum;
    
    for(i=0;i<(LengByte+5);i++)
    {
			  Usart1_Send_BYTE(SendData[i]); 
    }
		RS485_Rce_ON;
}


void USART1_IRQHandler(void) //串口1中断服务程序
{
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
				//Res =USART_ReceiveData(USART2);//(USART1->DR);	//读取接收到的数据
				if(((RBDown1-RBUp1) & UARTNunD)!=1)
				{    Uart1Recv[RBUp1++]=USART_ReceiveData(USART1);
						 if(RBUp1>= UARTNunD)
						 {
								RBUp1=0;
						 }
				}
			  else
			  {
					   Uart1Recv[RBUp1]=USART_ReceiveData(USART1);
			  }	 
		} 
}

//*****************主机发送***************************// 
//起始数据  源地址     功能指令    数据长度       数据    CRC校验
//  A8     SeifAddr    FunCom       Leng          Data     crc
//*****************从机返回***************************// 
//起始数据  源地址    功能指令    数据长度       数据    CRC校验
//  A8     SeifAddr   FunCom       Leng          Data     crc 
void Uart1Run_Pack(void)
{   
		unsigned char ia=0;
    if(RBDown1!=RBUp1)	         //Uart0 Function UartSD0
    {   ia=Uart1Recv[RBDown1++];		
        if(RBDown1>= UARTNunD)
			  {
					RBDown1=0;
			  }
        switch(Uart1Stt)
        {
        case 0:				         
                if(ia==START_SYMBOL)		 //接收起始字节
                {		Uart1Stt=1;
										Uart1ACC=ia;	////累加数据
										Uart1CntR=0;	//当前接收数据个数
										Uart1RecvBuf[Uart1CntR]=ia;
									  Uart1CntR++;
										Uart1ErrCountTime|=0x8000;  //开始计时
                }
                else
                {		Uart1_State|=0x01;		//没有接收到起始字节
										Uart1CntR=0;
										Uart1Stt=0;
										Uart1ACC=0;
										Uart1ErrCountTime&=~0x8000;  //停止计时
                }
                break;
			 case 1:	  
								Uart1Stt++;	           //源地址																  
                Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;									  										
                Uart1CntR++;
                Uart1ErrCountTime&=~0x7fff;  // 计时清零     
                break;																								
       case 2:	
								Uart1Stt++;	   //功能指令码																		  
                Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;									  										
                Uart1CntR++;
                Uart1ErrCountTime&=~0x7fff;  // 计时清零     
                break;																					
        case 3:	
								Uart1Stt++;
                Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;
                Uart1CntTT=ia+5;	//数据个数  加上一个起始字节 一个源地址字节 一个功能字节  一个数据长度  一个校验字节
                Uart1CntR++;
								Uart1ErrCountTime&=~0x7fff;  // 计时清零
                break;																											
        case 4:	
								Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;	
                Uart1CntR++;
								Uart1ErrCountTime&=~0x7fff;  // 计时清零
                if(Uart1CntR>=Uart1CntTT)	//数据个数校验
							  {			
											Uart1Stt=0;
											Uart1ErrCountTime=0;  //停止计时 
											if(Uart1ACC==0)	//CRC校验
											{    
												(void) Uart1RecvBuf;
											}
											else
											{			
														Uart1CntR=0;
														Uart1Stt=0;
														Uart1ACC=0;
														Uart1_State |=0x02;	   
											}
                 }              
                break;
              default:
                Uart1CntR=0;
                Uart1Stt=0;
                Uart1ACC=0;
                
                Uart1_State |=0x04;//
                break;
       } 	
		}
}


void  Uart1DataDropChk(void)   //判断串口1通信超时
{
		if(Uart1ErrCountTime&0x8000)
		{
				Uart1ErrCountTime++;
				if((Uart1ErrCountTime&0x7fff)>500)    //数据接收超时
				{		
						Uart1CntR=0;
						Uart1Stt=0;
						Uart1ACC=0;		
				}
		}
}


////////////////////////////////////////UART2//////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void Uart2_init(u32 bound)
{
    //GPIO端口设置
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		GPIO_InitTypeDef  GPIO_InitStructure;
	  
	  USART_DeInit(USART2);  //复位串口1
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);  //使能USART1，GPIOA时钟
	  
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    //USART 初始化设置
		USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
		USART_Init(USART2, &USART_InitStructure); //初始化串口
    
   //Usart1 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
		 
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART2, ENABLE);                    //使能串口 
}

 
void Usart2SendData(unsigned char Data)
{
	  while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) Data; 
}

void Uart2Send(unsigned char STData,unsigned char FunCom,unsigned char LengByte,unsigned char* Data)
{
    unsigned char i=0;
    unsigned char CRCnum=0;
    unsigned char SendData[UARTNunD+1]={0};

		RS485_Rce_OFF;
    
    SendData[0]=START_SYMBOL;
		CRCnum=START_SYMBOL;
    SendData[1]=STData;
    CRCnum+=STData;
    SendData[2]=FunCom;
    CRCnum+=FunCom;
    SendData[3]=LengByte;
    CRCnum+=LengByte;
    for(i=0;i<LengByte;i++)
    {    
         SendData[i+4]=Data[i];
         CRCnum+=SendData[i+4];
    }
    SendData[LengByte+4]=0-CRCnum;
    
    for(i=0;i<(LengByte+5);i++)
    {
			  Usart2SendData(SendData[i]); 
    }
		delay_us(100);
		RS485_Rce_ON;
}

void USART2_IRQHandler(void)                	             //串口2中断服务程序
{
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
				if(((RBDown2-RBUp2) & UARTNunD)!=1)
				{    Uart2Recv[RBUp2++]=USART_ReceiveData(USART2);
						 if(RBUp2>= UARTNunD)
						 {
								RBUp2=0;
						 }
				}
			  else
			  {
					   Uart2Recv[RBUp2]=USART_ReceiveData(USART2);
			  }	
		} 
}

//*****************主机发送***************************// 
//起始数据  源地址     功能指令    数据长度       数据    CRC校验
//  A8     SeifAddr    FunCom       Leng          Data     crc
//*****************从机返回***************************// 
//起始数据  源地址    功能指令    数据长度       数据    CRC校验
//  A8     SeifAddr   FunCom       Leng          Data     crc 

void Uart2Run_Pack(void)
{   
		unsigned char ia=0;
    if(RBDown2!=RBUp2)	         //Uart0 Function UartSD0
    {   ia=Uart2Recv[RBDown2++];		
        if(RBDown2>= UARTNunD)
				{
						RBDown2=0;
				}
        switch(Uart2Stt)
        {
        case 0:	
                if(ia==START_SYMBOL)		 //接收起始字节
                {		Uart2Stt=1;
										Uart2ACC=ia;	////累加数据
										Uart2CntR=0;	//当前接收数据个数
										Uart2RecvBuf[Uart2CntR]=ia;
									  Uart2CntR=1;	//当前接收数据个数
									  Uart1ErrCountTime|=0x8000;  //开始计时
                }
                else
                {		Uart2_State|=0x01;		//没有接收到起始字节
										Uart2CntR=0;
										Uart2Stt=0;
										Uart2ACC=0;
									  Uart1ErrCountTime&=~0x8000;  //停止计时
                }
                break;	
        case 1:	
								Uart2Stt++;	   //从机地址																  
                Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;									  										
                Uart2CntR++;
                Uart2ErrCountTime&=~0x7fff;  //计时清零       
                break;									
        case 2:	
								Uart2Stt++;	   //功能指令码																		  
                Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;									  										
                Uart2CntR++;
                Uart2ErrCountTime&=~0x7fff;  //计时清零       
                break;																					
        case 3:	
								Uart2Stt++;
                Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;
                Uart2CntTT=ia+5;	//数据个数  加上一个起始字节 一个源地址字节 一个功能字节  一个数据长度  一个校验字节
                Uart2CntR++;
                Uart2ErrCountTime&=~0x7fff;  //计时清零            
                break;																											
        case 4:	
								Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;	
                Uart2CntR++;
								Uart2ErrCountTime&=~0x7fff;  //计时清零    
                if(Uart2CntR>=Uart2CntTT)	//数据个数校验
                {			
											Uart2Stt=0;
											Uart2ErrCountTime=0;  //计时停止 
											if(Uart2ACC==0)	//CRC校验
											{      
													Rs485_Read_Data(Uart2RecvBuf,sizeof(Uart2RecvBuf) );
											 }
											 else
											 {			
														Uart2CntR=0;
														Uart2Stt=0;
														Uart2ACC=0;
														Uart2_State |=0x02;	//CRC   
											 }
                }              
                break;
        default:
                Uart2CntR=0;
                Uart2Stt=0;
                Uart2ACC=0;
                
                Uart2_State |=0x04;//
                break;
       } 	
    }
}

void  Uart2DataDropChk(void)    //判断串口2通信超时
{
		if(Uart2ErrCountTime&0x8000)
		{
				Uart2ErrCountTime++;
				if((Uart2ErrCountTime&0x7fff)>500)    
				{		
						Uart2CntR=0;
						Uart2Stt=0;
						Uart2ACC=0;		
				}
		}
}


