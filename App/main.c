#include "gpio.h"
#include "iic.h"
#include "led.h"
#include "sys.h"
#include "timer.h"
#include "adc.h"
#include "can.h"
#include "usart.h" 
#include "TaskFun.h"
#include "Flash.h"	
#include "delay.h"
#include "wdt.h"
#include "bq769xx.h"
#include "ConfigPara.h"
#include "cache_msg.h"
#include "Rs485Data.h"
#include "CanData.h"
#include "key.h"
#include "DataBase.h"
#include "soc.h"

TaskTime     TaskTimePare=TaskTime_DEFAULTS;

void SW_JTAGioConfig(void);
void FLASH_Update_Data(void);
void Sys_Rece_Data(void);
void Sys_Send_Data(void);
void Get_Base_Data(void);
void Mix_Function_Pro(void);
void LEDgpio_test(void);

int main(void)
{	 
	  NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	  SW_JTAGioConfig();
	  Init_GPIO_SYS_Power(); 
		InitPara0();
		//FLASH_Update_Data();
	  //System_Pare_Get();
		Init_GPIO_LED();
        Init_GPIO_KEY();	
		Init_GPIO_IN0();
		Init_GPIO_OUT0();
		Init_GPIO_ALERT();
	  Init_GPIO_BQ769xx_Wake();
		Init_GPIO_R485Pwr();
		Init_GPIO_R485EN();
		Init_GPIO_CanPwr();
		Init_GPIO_IIC(); 
		Init_GPIO_CAN(); 
    Init_GPIO_LED();
    Init_GPIO_ChargeActiva();
	  
		Uart2_init(115200);
	  CAN1_Init(CAN_BAUD_250K);
	  Tim1_Init(7199,9);         //72000000/71999=1k    1ms中断一次	
		delay_init();
	  BQ769xx_Wake();
		BQ769xx_Init();
	  cache_msg_init(CACHE_TYPE_CAN01_RCV);
		batt_cap_ocvsoc_init();
		gBMSData.Sys_Mod.sys_mode=SYS_MODE_STANDBY;
		Init_Watchdog(3,1250);   //(prer,rlr)  Tout=((4*2^prer)*rlr)/40 (ms). = 1000ms
   	while(1) 
	  {   
				/*Get time flag**************/
			  Systim_Time_Run();        
        /*Receive external data******/
        Sys_Rece_Data();              
			  /*Data sent out**************/
			  Sys_Send_Data();
        /* Basic data acquisition****/  
        Get_Base_Data();
        /* Mied function processing**/
        Mix_Function_Pro();
        /*Clear time flag************/
        Clear_flag();
		}
}
  
void SW_JTAGioConfig(void)
{		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
}  

void FLASH_Update_Data(void)
{
 	  FLASH_Unlock();						//解锁 
 	  FLASH_ErasePage(FLASH_SAVE_ADDR);//擦除这个扇区
 	  InitPara0();
 	  WriteAllData();
 	  FLASH_Lock();//上锁 
}

void Sys_Rece_Data(void)
{
	Uart1Run_Pack();
	Uart2Run_Pack();
    Can_Read_Data();
}

void Sys_Send_Data(void)
{
	//Rs485_Send_Test();
	//Bms_Can_Test();
}			

void Get_Base_Data(void)
{
		BQ769xx_GetData();
	    BQ769xx_GetConfig();
	    Balan_Pack_Check();
		Led_Run_Cpu();
	    Watchdog_Feed();
	  //LEDgpio_test();
}

void Mix_Function_Pro(void)
{	  
		BQ769xx_Oper_Comm();
	    BQ769xx_Wake_Comm();
		Sys_Function_Pro();
		Batt_soc_check();
	    Key_Can();
}








