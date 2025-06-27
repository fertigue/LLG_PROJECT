#ifndef __can_H
#define __CAN_H	
#include "sys.h"

//brp :波特率分频器.范围:1~1024;
#define CAN_BRP_1   1
#define CAN_BRP_2   2
#define CAN_BRP_3   3
#define CAN_BRP_4   4
#define CAN_BRP_5   5
#define CAN_BRP_6   6
#define CAN_BRP_7   7
#define CAN_BRP_8   8
#define CAN_BRP_9   9
#define CAN_BRP_10   10
#define CAN_BRP_11   11
#define CAN_BRP_12   12
#define CAN_BRP_13   13
#define CAN_BRP_14   14
#define CAN_BRP_15   15
#define CAN_BRP_16   16
//波特率=Fpclk1/((tsjw+tbs1+tbs2)*brp);

/*****************define handle CAN related respond result************/
#define CAN_RET_OK		(0)
#define CAN_RET_ERR		(1)
#define CAN_Mode_NORMAL	(0)
#define CAN_Mode_LOOP		(1)


/*****************define CAN related property*************************/
#define CAN_DLC_MAX		(8)		/*DLC Max Value*/
#define	CAN_STD_FRAME	(0)		/*CAN Standerd format (11bit ID)*/
#define	CAN_EXT_FRAME	(1)		/*CAN Extended format (29bit ID)*/

typedef enum 
{
	CAN_CH_01 = 1,
	CAN_CH_02,
	CAN_CH_03,
	CAN_CH_MAX
}	eCAN_CH;

typedef enum 
{
	CAN_BAUD_125K = 1,
	CAN_BAUD_250K,
	CAN_BAUD_500K,
	CAN_BAUD_800K,
	CAN_BAUD_1000K,
	CAN_BAUD_MAX
}	eCAN_BAUDRATE;

typedef struct _CAN_MSG_BAUD
{
	u8 CAN_SJW_TQ;
	u8 CAN_TBS1_TQ;
	u8 CAN_TBS2_TQ;				
	u16 CAN_BRP;				
}CAN_MSG_BAUD, *PCAN_MSG_BAUD;

typedef struct _CAN_RCV_MSG_RSP
{
	u32 Id;
	u8 DataArr[CAN_DLC_MAX];/*CAN message buffer*/
	u8 DataLen;				/*CAN message data size*/
	u8 NewData;				/*message data flag*/
}CAN_RCV_MSG_RSP, *PCAN_RCV_MSG_RSP;


u16 CAN1_Init(u8 baud);
u16 Can_Send_Data(u8 chno , u8 frame_type, u32 msgID, u8 msgsize, u8 *msg);

#endif 



