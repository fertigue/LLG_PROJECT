#ifndef __RS485DATA_H
#define __RS485DATA_H	 
#include "sys.h"

#define  OPER_ON  1
#define  OPER_OFF 0

extern u8 Bq769xx_Oper_EN;
extern u8 Bq769xx_Oper_Type;

typedef enum 
{
	COMM_MIN=0,
	COMM_DSG_ON =1,
	COMM_DSG_OFF=2,
	COMM_CHG_ON=3,
	COMM_CHG_OFF=4,
	COMM_CLEAR_ALERT=5,
	COMM_ENTER_SHIP=6,
	COMM_MAX=7,
}	eCOMM_TYPE;




void Rs485_Send_Test(void);
u16 Rs485_Read_Data(u8* resp_buf,u8 recvsize );


#endif


