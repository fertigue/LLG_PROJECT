#include "CanData.h"
#include "can.h"
#include "cache_msg.h"
#include "timer.h"
#include "bq769xx.h"
#include "DataBase.h"
#include "Rs485Data.h"
#include <cstdlib>
#include <string.h>

static i16 Can_Read_handle_msg(PCAN_RCV_MSG_RSP pmsg);
static void bms_recvmsg_pc(u32 can_id, u8 *msgbuf);
static void PostMsg_GainOffset(void);
static void PostMsg_BMS(void);
static void PostMsg_BQ769xxConfig(void);
static void PostMsg_Comm_Pro(u8 *databuf);


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
u16 Bms_Can_dbug(u32 can_id, u8 *msgbuf)
{
	u8 msg[CAN_MSG_LEN]={0};
  u8 ia=0;
	if (NULL == msgbuf)
	{
		return (gRET_NG);
	}
	
	for(ia=0; ia<CAN_MSG_LEN; ia++)
	{
      msg[ia]=*msgbuf;
			msgbuf++;
	}
	 
	Can_Send_Data(1, CAN_EXT_FRAME, can_id, CAN_MSG_LEN, msg);
	return (gRET_OK);
}

u16 Bms_Can_Param(u8 func_id, u8 *msgbuf, u8 size)
{
  u32 can_id;

	if (NULL == msgbuf)
	{
		return (gRET_NG);
	}
	can_id = MSGID(CAN_ID_BASE_18, func_id, 0x01, BASE_ADDR_SLOVE_BMS);
	
	Can_Send_Data(1, CAN_EXT_FRAME, can_id, size, msgbuf);
	return (gRET_OK);
}

u16 Bms_Can_Test(void)
{
	u8 msgbuf[CAN_MSG_LEN] = {0x00};
	u32 can_id;
	u8 func_id=0x01;
	if(TaskTimePare.Tim200ms_flag==1)
	{
		msgbuf[0]=1;
		msgbuf[1]=2;
		msgbuf[2]=3;
		msgbuf[3]=4;
		msgbuf[4]=5;
		msgbuf[5]=6;
		msgbuf[6]=7;
		msgbuf[7]=8;
		  
		can_id = MSGID(CAN_ID_BASE_18, func_id, BASE_ADDR_PC, BASE_ADDR_SLOVE_BMS);
		Bms_Can_dbug(can_id, msgbuf);

	}
	return (gRET_OK);
}



//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
u16 Can_Read_Data(void)
{
		CAN_RCV_MSG_RSP msg;
		u16 size = 0;
		while (TRUE)
		{
				if (TRUE == cache_msg_read(CACHE_TYPE_CAN01_RCV, (u8 *)&msg, sizeof(CAN_RCV_MSG_RSP), &size))
				{
						Can_Read_handle_msg(&msg);
				}
				else
				{
					break;
				}
		}
		
		while (TRUE)
		{
				if (TRUE == cache_msg_read(CACHE_TYPE_CAN02_RCV, (u8 *)&msg, sizeof(CAN_RCV_MSG_RSP), &size))
				{
						Can_Read_handle_msg(&msg);
				}
				else
				{
					break;
				}
		}

		while (TRUE)
		{
				if (TRUE == cache_msg_read(CACHE_TYPE_CAN03_RCV, (u8 *)&msg, sizeof(CAN_RCV_MSG_RSP), &size))
				{
						Can_Read_handle_msg(&msg);
				}
				else
				{
					break;
				}
		}
		return (gRET_OK);
		
}



i16 Can_Read_handle_msg(PCAN_RCV_MSG_RSP pmsg)
{
		if (pmsg==NULL)
		{
			return (gRET_NG);
		}

		switch (BASEID(pmsg->Id))
		{
			case BASE_ADDR_PC&0xF0:	
				bms_recvmsg_pc(pmsg->Id, pmsg->DataArr);
				break;
			case BASE_ADDR_MIAN_BMS&0xF0:	

				break;
			case BASE_ADDR_E102&0xF0:	

				break;
			case BASE_ADDR_CHG&0xF0:	

				break;
			case BASE_ADDR_SLOVE_BMS&0xF0:				
			
				break;
			default:  break;
	  }
		return (gRET_OK);
}



void bms_recvmsg_pc(u32 can_id, u8 *msgbuf)
{
		if (NULL == msgbuf)
		{
			return;
		}
		
		switch (PFUNCID(can_id))
		{
			case MPFUNCID(CAN_ID_BASE_18, FUNC_ID_PARAM1):
				PostMsg_GainOffset();
				break;
			case MPFUNCID(CAN_ID_BASE_18, FUNC_ID_PARAM2):
				PostMsg_BMS();
				break;
			case MPFUNCID(CAN_ID_BASE_18, FUNC_ID_PARAM3):
				PostMsg_BQ769xxConfig();
				break;
			case MPFUNCID(CAN_ID_BASE_18, FUNC_ID_PARAM4):
				PostMsg_Comm_Pro(msgbuf);
				break;
			default: 
				break;
		}
}

void PostMsg_GainOffset(void)
{
	u8 msgbuf[8];
	u32 can_id=0;
	u16 ia=0;
	u16 ib=0;
	u16 ia_Line=0;
	u8 DataBuffMsg[50]={0};
    
	DataBuffMsg[0]=(u8)((VoltCellGainUV&0xFF00)>>8);
	DataBuffMsg[1]=(u8)(VoltCellGainUV&0x00ff);
	DataBuffMsg[2]=VoltCellOffSet;
	for (ia = 0; ia < DATA_GAINOFFSET_SIZE; ia += DATA_PACK_SIZE)
	{
			ia_Line++;
			msgbuf[0] = ia_Line;
			msgbuf[1] = 1;
		  if((ia+DATA_PACK_SIZE)<= DATA_GAINOFFSET_SIZE)
			{
					memcpy(&msgbuf[2], &DataBuffMsg[ia], DATA_PACK_SIZE);
				  can_id = MSGID(CAN_ID_BASE_18, FUNC_ID_PARAM1, BASE_ADDR_PC, BASE_ADDR_SLOVE_BMS);
					Can_Send_Data(1, CAN_EXT_FRAME, can_id, CAN_MSG_LEN, msgbuf);
				  delay_ms(10);
			}
			else 
			{	
					memcpy(&msgbuf[2], &DataBuffMsg[ia], (DATA_GAINOFFSET_SIZE-ia));
					for(ib=0; ib<(DATA_PACK_SIZE-(DATA_GAINOFFSET_SIZE-ia)); ib++)
				  {
						msgbuf[(DATA_GAINOFFSET_SIZE-ia)+ib+2]=0xFF;
					}
				  can_id = MSGID(CAN_ID_BASE_18, FUNC_ID_PARAM1, BASE_ADDR_PC, BASE_ADDR_SLOVE_BMS);
					Can_Send_Data(1, CAN_EXT_FRAME, can_id, CAN_MSG_LEN, msgbuf);	
			}
	}
}

void PostMsg_BMS(void)
{
	u8 msgbuf[8];
	u32 can_id=0;
	u16 ia=0;
	u16 ib=0;
	u16 ia_Line=0;
	u8 DataBuffMsg[46]={0};
   
	for( ia=0; ia<15; ia++ )
	{
		DataBuffMsg[2*ia]=(gBMSData.BattPar.VoltCell[ia]&0xFF00)>>8;
		DataBuffMsg[2*ia+1]= gBMSData.BattPar.VoltCell[ia]&0xFF;
	}
	DataBuffMsg[30]=(gBMSData.BattPar.VoltLine)>>8;
	DataBuffMsg[31]=gBMSData.BattPar.VoltLine&0xFF;
	DataBuffMsg[32]=(gBMSData.BattPar.TempCell[0])>>8;
	DataBuffMsg[33]=gBMSData.BattPar.TempCell[0]&0xFF;
	DataBuffMsg[34]=(gBMSData.BattPar.TempCell[1])>>8;
	DataBuffMsg[35]=gBMSData.BattPar.TempCell[1]&0xFF;
	DataBuffMsg[36]=(gBMSData.BattPar.TempCell[2])>>8;
	DataBuffMsg[37]=gBMSData.BattPar.TempCell[2]&0xFF;
	DataBuffMsg[38]=(gBMSData.BattPar.CurrLine)>>8;
	DataBuffMsg[39]=gBMSData.BattPar.CurrLine&0xFF;
	DataBuffMsg[40]=(gBMSData.BattPar.SOCSys)>>8;
	DataBuffMsg[41]=gBMSData.BattPar.SOCSys&0xFF;
	DataBuffMsg[42]=(gBMSData.BalaPar.bala_ctrl_sw>>24)&0xff;
	DataBuffMsg[43]=(gBMSData.BalaPar.bala_ctrl_sw>>16)&0xff;
	DataBuffMsg[44]=(gBMSData.BalaPar.bala_ctrl_sw>>8)&0xff;
	DataBuffMsg[45]=gBMSData.BalaPar.bala_ctrl_sw&0xff;
	for (ia = 0; ia < DATA_BMS_SIZE; ia += DATA_PACK_SIZE)
	{
			ia_Line++;
			msgbuf[0] = ia_Line;
			msgbuf[1] = 2;
		  if((ia+DATA_PACK_SIZE)<= DATA_BMS_SIZE)
			{
					memcpy(&msgbuf[2], &DataBuffMsg[ia], DATA_PACK_SIZE);
				  can_id = MSGID(CAN_ID_BASE_18, FUNC_ID_PARAM2, BASE_ADDR_PC, BASE_ADDR_SLOVE_BMS);
					Can_Send_Data(1, CAN_EXT_FRAME, can_id, CAN_MSG_LEN, msgbuf);
				  delay_ms(10);
			}
			else 
			{	
					memcpy(&msgbuf[2], &DataBuffMsg[ia], (DATA_BMS_SIZE-ia));
					for(ib=0; ib<(DATA_PACK_SIZE-(DATA_BMS_SIZE-ia)); ib++)
				  {
						msgbuf[(DATA_BMS_SIZE-ia)+ib+2]=0xFF;
					}
				  can_id = MSGID(CAN_ID_BASE_18, FUNC_ID_PARAM2, BASE_ADDR_PC, BASE_ADDR_SLOVE_BMS);
					Can_Send_Data(1, CAN_EXT_FRAME, can_id, CAN_MSG_LEN, msgbuf);	
			}
	 }
}

void PostMsg_BQ769xxConfig(void)
{
  u8 msgbuf[8];
	u32 can_id=0;
	u16 ia=0;
	u16 ib=0;
	u16 ia_Line=0;
	u8 DataBuffMsg[50]={0};
   
	DataBuffMsg[0]= Bq769xxReg.SysStatus.StatusByte;
	DataBuffMsg[1]= Bq769xxReg.CellBal1.CellBal1Byte;
	DataBuffMsg[2]= Bq769xxReg.CellBal2.CellBal2Byte;
	DataBuffMsg[3]= Bq769xxReg.CellBal3.CellBal3Byte;
	DataBuffMsg[4]= Bq769xxReg.SysCtrl1.SysCtrl1Byte;
	DataBuffMsg[5]= Bq769xxReg.SysCtrl2.SysCtrl2Byte;
	DataBuffMsg[6]= Bq769xxReg.Protect1.Protect1Byte;
	DataBuffMsg[7]= Bq769xxReg.Protect2.Protect2Byte;
	DataBuffMsg[8]= Bq769xxReg.Protect3.Protect3Byte;
	DataBuffMsg[9]= Bq769xxReg.OVTrip;
	DataBuffMsg[10]= Bq769xxReg.UVTrip;
	DataBuffMsg[11]= Bq769xxReg.CCCfg;
	
	for (ia = 0; ia < DATA_BMS769XXCONFIG_SIZE; ia += DATA_PACK_SIZE)
	{
			ia_Line++;
			msgbuf[0] = ia_Line;
			msgbuf[1] = 3;
		  if((ia+DATA_PACK_SIZE)<= DATA_BMS769XXCONFIG_SIZE)
			{
					memcpy(&msgbuf[2], &DataBuffMsg[ia], DATA_PACK_SIZE);
				  can_id = MSGID(CAN_ID_BASE_18, FUNC_ID_PARAM3, BASE_ADDR_PC, BASE_ADDR_SLOVE_BMS);
					Can_Send_Data(1, CAN_EXT_FRAME, can_id, CAN_MSG_LEN, msgbuf);
				  delay_ms(10);
			}
			else 
			{	
					memcpy(&msgbuf[2], &DataBuffMsg[ia], (DATA_GAINOFFSET_SIZE-ia));
				  for(ib=0; ib<(DATA_PACK_SIZE-(DATA_BMS_SIZE-ia)); ib++)
				  {
						msgbuf[(DATA_BMS_SIZE-ia)+ib+2]=0xFF;
					}
				  can_id = MSGID(CAN_ID_BASE_18, FUNC_ID_PARAM3, BASE_ADDR_PC, BASE_ADDR_SLOVE_BMS);
					Can_Send_Data(1, CAN_EXT_FRAME, can_id, CAN_MSG_LEN, msgbuf);	
			}
	 }
}

void PostMsg_Comm_Pro(u8 *databuf)
{
		u8 typecomm=0;
		typecomm=databuf[0];
	  switch (typecomm)
		{
		case COMM_DSG_ON:
			Bq769xx_Oper_EN=OPER_ON;
			Bq769xx_Oper_Type=COMM_DSG_ON;
			break;
		case COMM_DSG_OFF:
			Bq769xx_Oper_EN=OPER_ON;
			Bq769xx_Oper_Type=COMM_DSG_OFF;
			break;
		case COMM_CHG_ON:
			Bq769xx_Oper_EN=OPER_ON;
			Bq769xx_Oper_Type=COMM_CHG_ON;
			break;
		case COMM_CHG_OFF:
		  Bq769xx_Oper_EN=OPER_ON;
			Bq769xx_Oper_Type=COMM_CHG_OFF;
		  break;	
		case COMM_CLEAR_ALERT:
			Bq769xx_Oper_EN=OPER_ON;
		  Bq769xx_Oper_Type=COMM_CLEAR_ALERT;
			break;
		case COMM_ENTER_SHIP:
			Bq769xx_Oper_EN=OPER_ON;
			Bq769xx_Oper_Type=COMM_ENTER_SHIP;
			break;
		default: break;
		}
}
































































































































