
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

//********************ȫ�ֱ���*********************//


//******************ȫ�ֱ���END********************//




//**************�ֲ�����****************************//
static u8 Uart1Recv[UARTNunD]={0};
static u8  Uart1RecvBuf[UARTNunD]={0};
static u8 RBUp1=0;
static u8 RBDown1=0;
static u8 Uart1ACC=0;
static u8 Uart1CntR=0;
static u8 Uart1CntTT=0;
static u8 Uart1Stt=0;
static u8 Uart1_State=0;
static u16  Uart1ErrCountTime=0; //485ͨ�����ݶ�ʧ��ʱ

static u8 Uart2Recv[UARTNunD]={0};
static u8 Uart2RecvBuf[UARTNunD]={0};
static u8 RBUp2=0;
static u8 RBDown2=0;
static u8 Uart2ACC=0;
static u8 Uart2CntR=0;
static u8 Uart2CntTT=0;
static u8 Uart2Stt=0;
static u8 Uart2_State=0;
static u16  Uart2ErrCountTime=0; //232ͨ�����ݶ�ʧ��ʱ
//*************�ֲ�����END****************************//



////////////////////////////////////////UART1//////////////////////////////////////////////////////////////

void Uart1_init(u32 bound)
{
    //GPIO�˿�����
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
	  GPIO_InitTypeDef  GPIO_InitStructure;
	  USART_DeInit(USART1);  //��λ����1
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);
 	  GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
	  
	  //USART1_TX   PA.9
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
 		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //�����������
 		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 		GPIO_Init(GPIOA, &GPIO_InitStructure);
    //USART1_RX	  PA.10
 		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
 		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
 		GPIO_Init(GPIOA,&GPIO_InitStructure);
	  
    //USART ��ʼ������
		USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
		USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
		USART_Init(USART1, &USART_InitStructure); //��ʼ������
 
   //Usart1 NVIC ����
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
		NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
		 
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Usart1_Send_BYTE(unsigned char Data)
{
	  while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
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


void USART1_IRQHandler(void) //����1�жϷ������
{
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
				//Res =USART_ReceiveData(USART2);//(USART1->DR);	//��ȡ���յ�������
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

//*****************��������***************************// 
//��ʼ����  Դ��ַ     ����ָ��    ���ݳ���       ����    CRCУ��
//  A8     SeifAddr    FunCom       Leng          Data     crc
//*****************�ӻ�����***************************// 
//��ʼ����  Դ��ַ    ����ָ��    ���ݳ���       ����    CRCУ��
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
                if(ia==START_SYMBOL)		 //������ʼ�ֽ�
                {		Uart1Stt=1;
										Uart1ACC=ia;	////�ۼ�����
										Uart1CntR=0;	//��ǰ�������ݸ���
										Uart1RecvBuf[Uart1CntR]=ia;
									  Uart1CntR++;
										Uart1ErrCountTime|=0x8000;  //��ʼ��ʱ
                }
                else
                {		Uart1_State|=0x01;		//û�н��յ���ʼ�ֽ�
										Uart1CntR=0;
										Uart1Stt=0;
										Uart1ACC=0;
										Uart1ErrCountTime&=~0x8000;  //ֹͣ��ʱ
                }
                break;
			 case 1:	  
								Uart1Stt++;	           //Դ��ַ																  
                Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;									  										
                Uart1CntR++;
                Uart1ErrCountTime&=~0x7fff;  // ��ʱ����     
                break;																								
       case 2:	
								Uart1Stt++;	   //����ָ����																		  
                Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;									  										
                Uart1CntR++;
                Uart1ErrCountTime&=~0x7fff;  // ��ʱ����     
                break;																					
        case 3:	
								Uart1Stt++;
                Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;
                Uart1CntTT=ia+5;	//���ݸ���  ����һ����ʼ�ֽ� һ��Դ��ַ�ֽ� һ�������ֽ�  һ�����ݳ���  һ��У���ֽ�
                Uart1CntR++;
								Uart1ErrCountTime&=~0x7fff;  // ��ʱ����
                break;																											
        case 4:	
								Uart1ACC+=ia;
                Uart1RecvBuf[Uart1CntR]=ia;	
                Uart1CntR++;
								Uart1ErrCountTime&=~0x7fff;  // ��ʱ����
                if(Uart1CntR>=Uart1CntTT)	//���ݸ���У��
							  {			
											Uart1Stt=0;
											Uart1ErrCountTime=0;  //ֹͣ��ʱ 
											if(Uart1ACC==0)	//CRCУ��
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


void  Uart1DataDropChk(void)   //�жϴ���1ͨ�ų�ʱ
{
		if(Uart1ErrCountTime&0x8000)
		{
				Uart1ErrCountTime++;
				if((Uart1ErrCountTime&0x7fff)>500)    //���ݽ��ճ�ʱ
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
    //GPIO�˿�����
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		GPIO_InitTypeDef  GPIO_InitStructure;
	  
	  USART_DeInit(USART2);  //��λ����1
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);  //ʹ��USART1��GPIOAʱ��
	  
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    //USART ��ʼ������
		USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
		USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
		USART_Init(USART2, &USART_InitStructure); //��ʼ������
    
   //Usart1 NVIC ����
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
		NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
		 
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
}

 
void Usart2SendData(unsigned char Data)
{
	  while((USART2->SR&0X40)==0);//ѭ������,ֱ���������   
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

void USART2_IRQHandler(void)                	             //����2�жϷ������
{
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
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

//*****************��������***************************// 
//��ʼ����  Դ��ַ     ����ָ��    ���ݳ���       ����    CRCУ��
//  A8     SeifAddr    FunCom       Leng          Data     crc
//*****************�ӻ�����***************************// 
//��ʼ����  Դ��ַ    ����ָ��    ���ݳ���       ����    CRCУ��
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
                if(ia==START_SYMBOL)		 //������ʼ�ֽ�
                {		Uart2Stt=1;
										Uart2ACC=ia;	////�ۼ�����
										Uart2CntR=0;	//��ǰ�������ݸ���
										Uart2RecvBuf[Uart2CntR]=ia;
									  Uart2CntR=1;	//��ǰ�������ݸ���
									  Uart1ErrCountTime|=0x8000;  //��ʼ��ʱ
                }
                else
                {		Uart2_State|=0x01;		//û�н��յ���ʼ�ֽ�
										Uart2CntR=0;
										Uart2Stt=0;
										Uart2ACC=0;
									  Uart1ErrCountTime&=~0x8000;  //ֹͣ��ʱ
                }
                break;	
        case 1:	
								Uart2Stt++;	   //�ӻ���ַ																  
                Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;									  										
                Uart2CntR++;
                Uart2ErrCountTime&=~0x7fff;  //��ʱ����       
                break;									
        case 2:	
								Uart2Stt++;	   //����ָ����																		  
                Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;									  										
                Uart2CntR++;
                Uart2ErrCountTime&=~0x7fff;  //��ʱ����       
                break;																					
        case 3:	
								Uart2Stt++;
                Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;
                Uart2CntTT=ia+5;	//���ݸ���  ����һ����ʼ�ֽ� һ��Դ��ַ�ֽ� һ�������ֽ�  һ�����ݳ���  һ��У���ֽ�
                Uart2CntR++;
                Uart2ErrCountTime&=~0x7fff;  //��ʱ����            
                break;																											
        case 4:	
								Uart2ACC+=ia;
                Uart2RecvBuf[Uart2CntR]=ia;	
                Uart2CntR++;
								Uart2ErrCountTime&=~0x7fff;  //��ʱ����    
                if(Uart2CntR>=Uart2CntTT)	//���ݸ���У��
                {			
											Uart2Stt=0;
											Uart2ErrCountTime=0;  //��ʱֹͣ 
											if(Uart2ACC==0)	//CRCУ��
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

void  Uart2DataDropChk(void)    //�жϴ���2ͨ�ų�ʱ
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


