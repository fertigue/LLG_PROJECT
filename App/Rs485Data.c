#include "Rs485Data.h"
#include "cache_msg.h"
#include "timer.h"
#include "usart.h"	
#include "bq769xx.h"
#include "DataBase.h"

#define RS485_BMS_ID  0x11

#define RS485_FUN_COMM1  0x01
#define RS485_FUN_COMM2  0x02
#define RS485_FUN_COMM3  0x03
#define RS485_FUN_COMM4  0x04

u8 Bq769xx_Oper_EN=0;
u8 Bq769xx_Oper_Type=0;

static u16 Rs485_Rec_PCdata(u8* PCdtaBuff ,  u8 byte_size);
void Send_GainOffset_Data(void);
void Send_BMS_Data(void);
void Send_BQ769xxConfig_Data(void);
static u16 PcComm_Pro(u8* dtaBuff, u8 size);


void Rs485_Send_Test(void)
{		
		u8 DataBuff[3]={0};
		if(TaskTimePare.Tim500ms_flag==1)
		{    
				 DataBuff[0]=1;
				 DataBuff[1]=2;
				 DataBuff[2]=3;
				 Uart2Send(RS485_BMS_ID,RS485_FUN_COMM1,sizeof(DataBuff),DataBuff);
		}
}

u16 Rs485_Read_Data(u8* resp_buf,u8 size_buf )
{
		u8 rsp_val;
		u16 ret=0;
		if ((resp_buf==NULL)||(0 == size_buf))
		{
			return (gRET_NG);
		}
		rsp_val=resp_buf[1];
		switch (rsp_val)
		{
			case RS485_BMS_ID:
				Rs485_Rec_PCdata(resp_buf , size_buf);
				break;
			default: 
				Rs485_Rec_PCdata(resp_buf , size_buf);
				break;
		}
		return (ret);
}

u16  Rs485_Rec_PCdata(u8* PCdtaBuff, u8 size_buf)
{
    u8 typedata=0;
    u16 ret=0;
	
		typedata=PCdtaBuff[2];
		switch (typedata)
		{
		case RS485_FUN_COMM1:
			Send_GainOffset_Data();
			break;
		case RS485_FUN_COMM2:
			Send_BMS_Data();
			break;
		case RS485_FUN_COMM3:
			Send_BQ769xxConfig_Data();
			break;
		case RS485_FUN_COMM4:
		  PcComm_Pro( PCdtaBuff, size_buf);
			break;
		default: break;
		}
		return (ret);
}


void Send_GainOffset_Data(void)
{ 	
	 u8 DataBuff[3]={0};
	 DataBuff[0]=(u8)((VoltCellGainUV&0xFF00)>>8);
	 DataBuff[1]=(u8)(VoltCellGainUV&0x00ff);
	 DataBuff[2]=VoltCellOffSet;
	 Uart2Send(RS485_BMS_ID,RS485_FUN_COMM1,sizeof(DataBuff),DataBuff);
}

void Send_BMS_Data(void)
{
		u8 DataBuff[46]={0};
		u8 ia=0;

		for( ia=0; ia<15; ia++ )
		{
			DataBuff[2*ia]=(gBMSData.BattPar.VoltCell[ia]&0xFF00)>>8;
			DataBuff[2*ia+1]= gBMSData.BattPar.VoltCell[ia]&0xFF;
		}
		DataBuff[30]=(gBMSData.BattPar.VoltLine)>>8;
		DataBuff[31]=gBMSData.BattPar.VoltLine&0xFF;
		DataBuff[32]=(gBMSData.BattPar.TempCell[0])>>8;
		DataBuff[33]=gBMSData.BattPar.TempCell[0]&0xFF;
		DataBuff[34]=(gBMSData.BattPar.TempCell[1])>>8;
		DataBuff[35]=gBMSData.BattPar.TempCell[1]&0xFF;
		DataBuff[36]=(gBMSData.BattPar.TempCell[2])>>8;
		DataBuff[37]=gBMSData.BattPar.TempCell[2]&0xFF;
		DataBuff[38]=(gBMSData.BattPar.CurrLine)>>8;
		DataBuff[39]=gBMSData.BattPar.CurrLine&0xFF;
		DataBuff[40]=(gBMSData.BattPar.SOCSys)>>8;
		DataBuff[41]=gBMSData.BattPar.SOCSys&0xFF;
		DataBuff[42]=(gBMSData.BalaPar.bala_ctrl_sw>>24)&0xff;
		DataBuff[43]=(gBMSData.BalaPar.bala_ctrl_sw>>16)&0xff;
		DataBuff[44]=(gBMSData.BalaPar.bala_ctrl_sw>>8)&0xff;
		DataBuff[45]=gBMSData.BalaPar.bala_ctrl_sw&0xff;
	  Uart2Send(RS485_BMS_ID,RS485_FUN_COMM2,sizeof(DataBuff),DataBuff);
}


void Send_BQ769xxConfig_Data(void)
{
	  u8 DataBuff[12]={0};

		DataBuff[0]= Bq769xxReg.SysStatus.StatusByte;
		DataBuff[1]= Bq769xxReg.CellBal1.CellBal1Byte;
		DataBuff[2]= Bq769xxReg.CellBal2.CellBal2Byte;
		DataBuff[3]= Bq769xxReg.CellBal3.CellBal3Byte;
		DataBuff[4]= Bq769xxReg.SysCtrl1.SysCtrl1Byte;
		DataBuff[5]= Bq769xxReg.SysCtrl2.SysCtrl2Byte;
		DataBuff[6]= Bq769xxReg.Protect1.Protect1Byte;
		DataBuff[7]= Bq769xxReg.Protect2.Protect2Byte;
		DataBuff[8]= Bq769xxReg.Protect3.Protect3Byte;
		DataBuff[9]= Bq769xxReg.OVTrip;
		DataBuff[10]= Bq769xxReg.UVTrip;
		DataBuff[11]= Bq769xxReg.CCCfg;

    Uart2Send(RS485_BMS_ID,RS485_FUN_COMM3,sizeof(DataBuff),DataBuff);
}

u16 PcComm_Pro(u8* dtaBuff, u8 size)
{   
		u8 typecomm =0;
		u16 ret=0;
		typecomm=dtaBuff[4];
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
		return (ret);
}



























