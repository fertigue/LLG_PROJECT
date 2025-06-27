#include "can.h"
#include "CanData.h"
#include <cstdlib>
#include <stddef.h>
#include <string.h>
#include "CanData.h"
#include "cache_msg.h"

CanTxMsg     TxMessage; 
CAN_MSG_BAUD  baud_tbs;

static void CAN1_Baud_Switch( u8 baud );
static u16 Can_Convert_Data( CanRxMsg*  RxMsg, PCAN_RCV_MSG_RSP   qCanmsg); 
//static u16 Can_Send_Msg(void);


//////////////////////////////////////////////////////////////////////////////////
//CAN��ʼ��
//tsjw:����ͬ����Ծʱ�䵥Ԫ.��Χ:1~3; CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//tbs2:ʱ���2��ʱ�䵥Ԫ.��Χ:1~8;
//tbs1:ʱ���1��ʱ�䵥Ԫ.��Χ:1~16;	  CAN_BS1_1tq ~CAN_BS1_16tq
//brp :�����ʷ�Ƶ��.��Χ:1~1024;(ʵ��Ҫ��1,Ҳ����1~1024) tq=(brp)*tpclk1
//ע�����ϲ����κ�һ����������Ϊ0,�������.
//������=Fpclk1/((tsjw+tbs1+tbs2)*brp);
//mode:0,��ͨģʽ;1,�ػ�ģʽ;
//Fpclk1��ʱ���ڳ�ʼ����ʱ������Ϊ36M,�������CAN_Normal_Init(1,8,7,5,1);
//������Ϊ:36M/((1+8+7)*5)=450Kbps
//u16 CAN1_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
//����ֵ:0,��ʼ��OK;
//////////////////////////////////////////////////////////////////////////////////
u16 CAN1_Init(u8 baud)
{
	

		CAN_InitTypeDef        CAN_InitStructure;
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;
		NVIC_InitTypeDef  NVIC_InitStructure;
              											 
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��	
    
	  CAN1_Baud_Switch( baud);
	  
	  CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);
 	  //CAN��Ԫ����
 	  CAN_InitStructure.CAN_TTCM=DISABLE;						 //��ʱ�䴥��ͨ��ģʽ  //
 	  CAN_InitStructure.CAN_ABOM=DISABLE;						 //����Զ����߹���	 //
  	CAN_InitStructure.CAN_AWUM=DISABLE;						 //˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)//
  	CAN_InitStructure.CAN_NART=ENABLE;//ENABLE;						 	//��ֹ�����Զ���¯���� //
  	CAN_InitStructure.CAN_RFLM=DISABLE;						 //���Ĳ�����,�µĸ��Ǿɵ� // 
  	CAN_InitStructure.CAN_TXFP=DISABLE;						 //���ȼ��ɱ��ı�ʶ������ //
  	CAN_InitStructure.CAN_Mode= CAN_Mode_NORMAL;	 //ģʽ���ã� mode:0,��ͨģʽ;1,�ػ�ģʽ; //
  	//���ò�����
//   	CAN_InitStructure.CAN_SJW=baud_tbs.CAN_SJW_TQ   ;				     //����ͬ����Ծ���(Tsjw)Ϊtsjw+1��ʱ�䵥λ  CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//   	CAN_InitStructure.CAN_BS1=baud_tbs.CAN_TBS1_TQ;               //Tbs1=tbs1+1��ʱ�䵥λCAN_BS1_1tq ~CAN_BS1_16tq
//   	CAN_InitStructure.CAN_BS2=baud_tbs.CAN_TBS2_TQ;               //Tbs2=tbs2+1��ʱ�䵥λCAN_BS2_1tq ~	CAN_BS2_8tq
//   	CAN_InitStructure.CAN_Prescaler=baud_tbs.CAN_BRP; ;            //��Ƶϵ��(Fdiv)Ϊbrp+1	//
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;		    
		CAN_InitStructure.CAN_BS1=CAN_BS1_15tq;		   
		CAN_InitStructure.CAN_BS2=CAN_BS2_8tq;		 
		CAN_InitStructure.CAN_Prescaler =6;    //  72/ (1+15+8)/6/2 =250
		
		CAN_Init(CAN1, &CAN_InitStructure);             // ��ʼ��CAN1 
    
   	CAN_FilterInitStructure.CAN_FilterNumber=0;	  //������0
 	  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; 
  	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32λ 
  	CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;////32λID
  	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
  	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;//32λMASK
  	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
  	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//������0������FIFO0
 	  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //���������0
  	CAN_FilterInit(&CAN_FilterInitStructure);//�˲�����ʼ��	    
  
  	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // �����ȼ�Ϊ1
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // �����ȼ�Ϊ0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
		
	  CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0��Ϣ�Һ��ж�����.	

		TxMessage.StdId = 0 ;    //     
		TxMessage.ExtId = 0x1811f411;
		TxMessage.RTR = CAN_RTR_DATA;  // 0  CAN_RTR_DATA
		TxMessage.IDE =CAN_ID_EXT ;   //  0  CAN_ID_EXT
		TxMessage.DLC = 8;  
	  return (gRET_OK);
}   
 
//tsjw:����ͬ����Ծʱ�䵥Ԫ.��Χ:1~3; CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//tbs2:ʱ���2��ʱ�䵥Ԫ.��Χ:1~8;
//tbs1:ʱ���1��ʱ�䵥Ԫ.��Χ:1~16;	  CAN_BS1_1tq ~CAN_BS1_16tq
//������=Fpclk1/((tsjw+tbs1+tbs2)*brp);
//������Ϊ:36M/((1+8+7)*5)=450Kbps
void  CAN1_Baud_Switch( u8 baud)
{
		switch(baud)
		{
				case CAN_BAUD_125K:
					baud_tbs.CAN_SJW_TQ=CAN_SJW_3tq;
					baud_tbs.CAN_TBS1_TQ=CAN_BS1_16tq;
				  baud_tbs.CAN_TBS2_TQ=CAN_BS2_8tq;
					baud_tbs.CAN_BRP=CAN_BRP_12;
					break ;
				case CAN_BAUD_250K:
					baud_tbs.CAN_SJW_TQ=CAN_SJW_3tq;
				  baud_tbs.CAN_TBS1_TQ=CAN_BS1_16tq;
					baud_tbs.CAN_TBS2_TQ=CAN_BS2_8tq;
					baud_tbs.CAN_BRP=CAN_BRP_6;
					break ;
			 	case CAN_BAUD_500K:
					baud_tbs.CAN_SJW_TQ=CAN_SJW_3tq;
					baud_tbs.CAN_TBS1_TQ=CAN_BS1_15tq;
				  baud_tbs.CAN_TBS2_TQ=CAN_BS2_7tq;
					baud_tbs.CAN_BRP=CAN_BRP_3;
					break ;
				case CAN_BAUD_800K:
					baud_tbs.CAN_SJW_TQ=CAN_SJW_4tq;
					baud_tbs.CAN_TBS1_TQ=CAN_BS1_11tq;
				  baud_tbs.CAN_TBS2_TQ=CAN_BS2_5tq;
					baud_tbs.CAN_BRP=CAN_BRP_3; 
					break ;
				case CAN_BAUD_1000K:
					baud_tbs.CAN_SJW_TQ=CAN_BS1_2tq;
					baud_tbs.CAN_TBS1_TQ=CAN_BS1_11tq;
				  baud_tbs.CAN_TBS2_TQ=CAN_BS2_6tq;
					baud_tbs.CAN_BRP=CAN_BRP_2; 
					break ;
		}
}

//�жϷ�����			    
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  	CanRxMsg RxMessage;
	  CAN_RCV_MSG_RSP sCanBuf0;				// read buffer
	
	  CAN_Receive(CAN1, 0, &RxMessage);
	  
		Can_Convert_Data( &RxMessage,  &sCanBuf0);
	  
	  cache_msg_write(CACHE_TYPE_CAN01_RCV, (u8 *)&sCanBuf0, sizeof(CAN_RCV_MSG_RSP));
}

//can�ڽ������ݲ�ѯ
//buf:���ݻ�����;	 
//����ֵ:0,�����ݱ��յ�;
//����,���յ����ݳ���;
u8 Can_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
	if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		//û�н��յ�����,ֱ���˳� 
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);//��ȡ����	
	for(i=0;i<8;i++)
	buf[i]=RxMessage.Data[i];  
	return RxMessage.DLC;	
}





u16 Can_Send_Data(u8 chno , u8 frame_type, u32 msgID, u8 msgsize, u8* msg)
{
	  u16 ret=0;
	  u8 ia=0;
	  //CanTxMsg   TxMessage ; 
	  if(msg==NULL)
		{
				return gRET_NG;
		}
		if((frame_type!=CAN_EXT_FRAME)&&(frame_type!=(u8)CAN_STD_FRAME) )
		{
				return gRET_NG;
		}
		
		TxMessage.StdId=0;		    // ��׼��ʶ��Ϊ0
		TxMessage.ExtId=msgID;		// ������չ��ʾ����29λ��
		if(frame_type==CAN_EXT_FRAME)
		{
				TxMessage.IDE=CAN_ID_EXT;			  // ʹ����չ��ʶ��  
		}
		else
		{
				TxMessage.IDE=CAN_ID_STD;			  // ʹ�ñ�׼��ʶ��  
		}
		TxMessage.RTR=CAN_RTR_DATA;		            // ��Ϣ����Ϊ����֡��һ֡8λ
		TxMessage.DLC=msgsize;	// ������֡��Ϣ
		for(ia=0;ia<msgsize;ia++)
		{
				TxMessage.Data[ia]=msg[ia];		 // ��һ֡��Ϣ      
		}
		switch (chno) 
		{
			case CAN_CH_01: // Channel1
				//ret =Can_Send_Msg();
			  ret =CAN_Transmit(CAN1, &TxMessage);   
				if (ret==gRET_OK) 
				{
				}
				else  
				{
				}
				break;
			case CAN_CH_02: // Channel2
			  
				break;
			case CAN_CH_03: // Channel3
			
				break;
			default:
				return (gRET_NG);
		}
	
	return (ret);
}

u16 Can_Convert_Data( CanRxMsg*  RxMsg, PCAN_RCV_MSG_RSP   qCanmsg)
{
		u8 ia=0;
	  if(RxMsg==NULL)
		{
        return gRET_NG;
		}
	
		if(RxMsg->IDE==CAN_Id_Standard)
		{
				qCanmsg->Id =RxMsg->StdId;
		}
		else
		{
				qCanmsg->Id =RxMsg->ExtId ;
		}
		
		qCanmsg->DataLen=RxMsg->DLC;
		
		for(ia=0; ia<RxMsg->DLC; ia++ )
		{
				qCanmsg->DataArr[ia]=RxMsg->Data[ia];
		}
		qCanmsg->NewData=1;
		
		return gRET_OK;
}







