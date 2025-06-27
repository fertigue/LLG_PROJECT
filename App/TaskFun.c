#include "TaskFun.h"
#include "gpio.h"
#include "adc.h"
#include "timer.h"
#include "iic.h"
#include "led.h"
#include "DataBase.h"


//-------------全局变量——-------------//

//------------------------------------//

//-------------局部变量——-------------//
#define SELL_IN_TIME_2H   36000         //200ms     5*60*60*2=36000     2小时  
static  u32 slee_info_cnt=0;
u8 Charge_Activa=0;
//------------------------------------//

static void Sys_Mode_Charge(void);
static u8 Sys_State_Check(void);


void Sys_Function_Pro(void)
{
		Sys_Mode_Charge();
		Sys_State_Check();
}


void Sys_Mode_Charge(void)
{
		if(TaskTimePare.Tim200ms_flag != 1)
		{
				return;
		}
		if(gBMSData.Sys_Mod.sys_mode == SYS_MODE_SLEEP)
		{
			if((gBMSData.BattPar.CurrLine > 20)&&(gBMSData.BattPar.CurrLine < -20))
			{
					gBMSData.Sys_Mod.sys_mode=SYS_MODE_STANDBY;
			}
			return;
		}
		
		if((gBMSData.BattPar.CurrLine<20)&&(gBMSData.BattPar.CurrLine>-20))
		{
				gBMSData.Sys_Mod.sys_mode=SYS_MODE_STANDBY;
				if(slee_info_cnt++ > SELL_IN_TIME_2H  )
				{
						slee_info_cnt=0;
						gBMSData.Sys_Mod.sys_mode=SYS_MODE_SLEEP;
				}	
		}
		else if(gBMSData.BattPar.CurrLine<= -20)
		{
				gBMSData.Sys_Mod.sys_mode=SYS_MODE_DISCHARGE;
				slee_info_cnt=0;
		}
		else if( gBMSData.BattPar.CurrLine>=20 )
		{
				gBMSData.Sys_Mod.sys_mode=SYS_MODE_CHARGE;
				slee_info_cnt=0;
		}
}


static u8 Sys_State_Check(void)
{
		u8 ret=gRET_OK;
		static u8 wait_time_cnt = 10;
		static u8 chg_dsg_off_flag = 1;
	
		if(TaskTimePare.Tim100ms_flag != 1)
		{
				return gRET_NG;
		}

		if(wait_time_cnt > 0)
		{
				wait_time_cnt--;
				return gRET_NG;
		}
		
		if(chg_dsg_off_flag == 1)
		{
				chg_dsg_off_flag = 0;
				BQ769xx_DSGSET(1);
				BQ769xx_CHGSET(1);
		}
		
		return ret;
}

