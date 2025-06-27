

#ifndef __CANDATA_H
#define __CANDATA_H	 
#include "sys.h"

#define CAN_MSG_LEN  (8)

#define MSGID(base, func_id, dst_id, key_id) (u32)((base)|(func_id<<16)|(dst_id<<8)|(key_id&0xFF))
#define MPFUNCID(base, func_id) (u16)((base >> 16) |(func_id&0xFF))


#define FUNCID(canid)   (u8)((canid >> 16) & 0xFF)
#define PFUNCID(canid) (u16)((canid >> 16) & 0xFFFF)	/*include priority*/
#define DESTID(canid)   (u8)((canid >> 8) & 0xFF)
#define SRCID(canid)    (u8)(canid & 0xFF)
#define BASEID(canid)  (u8)(canid & 0xF0)

/*CAN-ID:XX-FUNC ID-DST-SRC*/
#define CAN_ID_BASE_10	(0x10000000)    //alarm
#define CAN_ID_BASE_18	(0x18000000)    //data
#define CAN_ID_BASE_1A	(0x1A000000)    //dbug

//Target Address(Key ID Base)
#define BASE_ADDR_PC		      (0xFC)	/*FC Device Client*/
#define BASE_ADDR_MIAN_BMS		(0xA8)
#define BASE_ADDR_E102	      (0xB0)
#define BASE_ADDR_CHG		      (0xCE)
#define BASE_ADDR_SLOVE_BMS	  (0x11)

#define FUNC_ID_PARAM1    0x01
#define FUNC_ID_PARAM2    0x02
#define FUNC_ID_PARAM3    0x03
#define FUNC_ID_PARAM4    0x04

#define   DATA_PACK_SIZE  6
#define   DATA_GAINOFFSET_SIZE  3
#define   DATA_BMS_SIZE  46
#define   DATA_BMS769XXCONFIG_SIZE  12
#define   DATA_COMM_PRO_SIZE  0

// #define FUNC_ID_PACK_CURR	 (0x10)	
// #define FUNC_ID_PACK_VOLT  (0x20)	
// #define FUNC_ID_PACK_SOC  (0x2A)	
// #define FUNC_ID_CELLC1		(0x30)	
// #define FUNC_ID_CELLC2		(0x31)	
// #define FUNC_ID_CELLC3	  (0x32)	
// #define FUNC_ID_CELLC4		(0x33)	
// #define FUNC_ID_CELLC5	  (0x34)	
// #define FUNC_ID_CELLC6		(0x35)	
// #define FUNC_ID_CELL_SOC1		(0x40)	
// #define FUNC_ID_CELL_SOC2		(0x41)	
// #define FUNC_ID_CELL_SOC3	  (0x50)	
// #define FUNC_ID_PACK_TEMP1		(0x50)	
// #define FUNC_ID_PACK_TEMP2		(0x51)	
// #define FUNC_ID_PACK_FAULT1		(0x60)	
// #define FUNC_ID_PACK_FAULT2		(0x61)	
// #define FUNC_ID_PACK_STATE1		(0x70)	
// #define FUNC_ID_PACK_STATE2		(0x71)	
// #define FUNC_ID_PACK_MODE		  (0x80)	


u16 Can_Read_Data(void);
u16 Bms_Can_Param(u8 func_id, u8 *msgbuf, u8 size);
u16 Bms_postmsg_param(u32 can_id, u8 *msgbuf);
u16 Bms_Can_Test(void);
#endif


