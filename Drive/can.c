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
//CAN初始化
//tsjw:重新同步跳跃时间单元.范围:1~3; CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//tbs2:时间段2的时间单元.范围:1~8;
//tbs1:时间段1的时间单元.范围:1~16;	  CAN_BS1_1tq ~CAN_BS1_16tq
//brp :波特率分频器.范围:1~1024;(实际要加1,也就是1~1024) tq=(brp)*tpclk1
//注意以上参数任何一个都不能设为0,否则会乱.
//波特率=Fpclk1/((tsjw+tbs1+tbs2)*brp);
//mode:0,普通模式;1,回环模式;
//Fpclk1的时钟在初始化的时候设置为36M,如果设置CAN_Normal_Init(1,8,7,5,1);
//则波特率为:36M/((1+8+7)*5)=450Kbps
//u16 CAN1_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
//返回值:0,初始化OK;
//////////////////////////////////////////////////////////////////////////////////
u16 CAN1_Init(u8 baud)
{
	

		CAN_InitTypeDef        CAN_InitStructure;
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;
		NVIC_InitTypeDef  NVIC_InitStructure;
              											 
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟	
    
	  CAN1_Baud_Switch( baud);
	  
	  CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);
 	  //CAN单元设置
 	  CAN_InitStructure.CAN_TTCM=DISABLE;						 //非时间触发通信模式  //
 	  CAN_InitStructure.CAN_ABOM=DISABLE;						 //软件自动离线管理	 //
  	CAN_InitStructure.CAN_AWUM=DISABLE;						 //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)//
  	CAN_InitStructure.CAN_NART=ENABLE;//ENABLE;						 	//禁止报文自吨匦炉传送 //
  	CAN_InitStructure.CAN_RFLM=DISABLE;						 //报文不锁定,新的覆盖旧的 // 
  	CAN_InitStructure.CAN_TXFP=DISABLE;						 //优先级由报文标识符决定 //
  	CAN_InitStructure.CAN_Mode= CAN_Mode_NORMAL;	 //模式设置： mode:0,普通模式;1,回环模式; //
  	//设置波特率
//   	CAN_InitStructure.CAN_SJW=baud_tbs.CAN_SJW_TQ   ;				     //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位  CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//   	CAN_InitStructure.CAN_BS1=baud_tbs.CAN_TBS1_TQ;               //Tbs1=tbs1+1个时间单位CAN_BS1_1tq ~CAN_BS1_16tq
//   	CAN_InitStructure.CAN_BS2=baud_tbs.CAN_TBS2_TQ;               //Tbs2=tbs2+1个时间单位CAN_BS2_1tq ~	CAN_BS2_8tq
//   	CAN_InitStructure.CAN_Prescaler=baud_tbs.CAN_BRP; ;            //分频系数(Fdiv)为brp+1	//
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;		    
		CAN_InitStructure.CAN_BS1=CAN_BS1_15tq;		   
		CAN_InitStructure.CAN_BS2=CAN_BS2_8tq;		 
		CAN_InitStructure.CAN_Prescaler =6;    //  72/ (1+15+8)/6/2 =250
		
		CAN_Init(CAN1, &CAN_InitStructure);             // 初始化CAN1 
    
   	CAN_FilterInitStructure.CAN_FilterNumber=0;	  //过滤器0
 	  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; 
  	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32位 
  	CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;////32位ID
  	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
  	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;//32位MASK
  	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
  	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//过滤器0关联到FIFO0
 	  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //激活过滤器0
  	CAN_FilterInit(&CAN_FilterInitStructure);//滤波器初始化	    
  
  	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
		
	  CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0消息挂号中断允许.	

		TxMessage.StdId = 0 ;    //     
		TxMessage.ExtId = 0x1811f411;
		TxMessage.RTR = CAN_RTR_DATA;  // 0  CAN_RTR_DATA
		TxMessage.IDE =CAN_ID_EXT ;   //  0  CAN_ID_EXT
		TxMessage.DLC = 8;  
	  return (gRET_OK);
}   
 
//tsjw:重新同步跳跃时间单元.范围:1~3; CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
//tbs2:时间段2的时间单元.范围:1~8;
//tbs1:时间段1的时间单元.范围:1~16;	  CAN_BS1_1tq ~CAN_BS1_16tq
//波特率=Fpclk1/((tsjw+tbs1+tbs2)*brp);
//则波特率为:36M/((1+8+7)*5)=450Kbps
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

//中断服务函数			    
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  	CanRxMsg RxMessage;
	  CAN_RCV_MSG_RSP sCanBuf0;				// read buffer
	
	  CAN_Receive(CAN1, 0, &RxMessage);
	  
		Can_Convert_Data( &RxMessage,  &sCanBuf0);
	  
	  cache_msg_write(CACHE_TYPE_CAN01_RCV, (u8 *)&sCanBuf0, sizeof(CAN_RCV_MSG_RSP));
}

//can口接收数据查询
//buf:数据缓存区;	 
//返回值:0,无数据被收到;
//其他,接收的数据长度;
u8 Can_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
	if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		//没有接收到数据,直接退出 
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);//读取数据	
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
		
		TxMessage.StdId=0;		    // 标准标识符为0
		TxMessage.ExtId=msgID;		// 设置扩展标示符（29位）
		if(frame_type==CAN_EXT_FRAME)
		{
				TxMessage.IDE=CAN_ID_EXT;			  // 使用扩展标识符  
		}
		else
		{
				TxMessage.IDE=CAN_ID_STD;			  // 使用标准标识符  
		}
		TxMessage.RTR=CAN_RTR_DATA;		            // 消息类型为数据帧，一帧8位
		TxMessage.DLC=msgsize;	// 发送两帧信息
		for(ia=0;ia<msgsize;ia++)
		{
				TxMessage.Data[ia]=msg[ia];		 // 第一帧信息      
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







