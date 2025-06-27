#ifndef __DATABASE_H
#define __DATABASE_H	 
#include "sys.h"
#include "bq769xx.h"


#define   gRET_OK     0
#define   gRET_NG     1
#define   gTIM_NG     2

#define SYS_MODE_SLEEP     0
#define SYS_MODE_STANDBY    1
#define SYS_MODE_DISCHARGE   2 
#define SYS_MODE_CHARGE      3

#define BMS_MODE_INIT       1
#define BMS_MODE_VCT        2
#define BMS_MODE_CHG_DSG    3 
#define BMS_MODE_RUNING     4
#define BMS_MODE_PROTECT    5

#define gTIMER_FLAG_ON     1
#define gTIMER_FLAG_OFF    0

#define  PACK_CELL_MAX       15      
#define  PACK_TEMPEXT_MAX     3    
#define  PACK_BALACELL_MAX    5    
#define  BQ769xxREG_MAX      55



typedef struct BattPar{
  
  u16 VoltLine;
  i16 CurrLine;
  u16 VoltCell[PACK_CELL_MAX];
  u16 VoltCellDiff;
  u16 VoltCellMax;
  u16 VoltCellMin;
  u16 TempCell[PACK_TEMPEXT_MAX];
  u16 TempPackDiff;
  i16 TempPackMax;
  i16 TempPackMin;
  u16 SOCSys;
  u16 SOCcell[PACK_CELL_MAX];
  u32 CapSoc[PACK_CELL_MAX];
  u16 Cells_Map;
  }sBattPar;


typedef struct ChargePar{
  u16 Volt;
  u16 Curr;
}sChargePar;

typedef struct sStatePar{
  
  u8  RelayDichSW;
  u8  RelayChSW;
}sStatePar;

typedef struct sbalaPar{
	u16 bala_dischg;         //最大两个芯片
	u32 bala_ctrl_sw;           //每一节电芯均衡开关状态
	u32 bala_ctrl_dr;           //每一节已经在均衡电芯的方向
	u8 Bala_force_on;
	u8 bala_state;
	u8 bala_error;	 
}sbalaPar;

typedef struct AlarmPar{
  
   u8 OVP;
   u8 UVP;
   u8 OCP;
   u8 UCP;
   u8 OTP;
   u8 UTP;
   u8 USOC;
   u8 OVLP;
   u8 UVLP;
   
   u8 SCD_BQ;
   u8 OCD_BQ;
   u8 UV_BQ;
   u8 OV_BQ;  
   
  }sAlarmPar;

typedef struct ErrorPar{
  
  u8 I2C_Err;
  u8 Uart_Err;
  u8 RelayCh_Err;
  u8 RelayDich_Err;
  }sErrorPar;

typedef struct Sys_Mod{
  
  u8 sys_mode;
  u8 bms_mode;
  u8 test_mode;
	}sSys_Mod;

typedef struct Softw_Ver 
{
	u8 ver_main;
	u8 ver_sub1;
	u8 ver_sub2;
	u8 ver_sub3;
	}sSoftw_Ver;


typedef struct Gpio_State
{
	u8 IN0_State;
	u8 Charge_Activa_State;
	u8 BQ769xx_Alert_State;
	}sGpio_State;

typedef struct PackCap{
	u32  pack_rated_cap;      
	u32  pack_real_cap;       
	u32  pack_rem_cap; 			
	u32  pack_chg_max_cap;    
	u32  pack_dsg_max_cap;
	u32  pack_cap_dischg;    
  u32  pack_cap_chg;       
}sPackCap;

typedef struct BMSData{
   sBattPar      BattPar; 
   sChargePar    ChargePar;
   sStatePar     StatePar;
	 sbalaPar			 BalaPar;
	 sPackCap      PackCap;
   sAlarmPar     AlarmPar;
   sErrorPar     ErrorPar;   
   sSys_Mod      Sys_Mod;
	 sSoftw_Ver    Softw_Ver;
	 sGpio_State   Gpio_State;
  }sBMSData;

extern  sBMSData  gBMSData;





#endif




