#include "ConfigPara.h"
#include "Flash.h"	
#include<string.h>


sBMSConfig   gBMSConfig;
sBMSConfig   gBMSReadConfig;


u8  DataCRC(u8 leng,u8 *ucpStr)
{	
		u8 ucCRC=0,ucN=0;
		for(ucN=0;ucN<leng;ucN++)
		{       
				ucCRC+=*ucpStr;
			  ucpStr++;
		}
		return ucCRC;
}



void InitPara0(void)
{
	 gBMSConfig.DataLeng=sizeof(gBMSConfig);
   
   gBMSConfig.Type.CapaRate=200;      //额定容量2.00AH
   gBMSConfig.Type.VoltSpec=54;       //电压规格
   gBMSConfig.Type.CellNum_Ser=15;     //串联节数
   gBMSConfig.Type.CellNum_Par=1;     //并联节数
   gBMSConfig.Type.TempNum=1;         //温度点个数
   gBMSConfig.Type.ShuntSpec=2500;    //分流器规格2.5MΩ
   gBMSConfig.Type.OCVDaleyTime=600;    
	
	 gBMSConfig.Balan.balanc_start_volt = 3000;
	 gBMSConfig.Balan.balanc_diffe_volt = 100;
	 gBMSConfig.Balan.balanc_number_max = 5;   
	 gBMSConfig.Balan.balanc_oneall_time = 900;	

   gBMSConfig.BQ76Para.SCD_Delay=0x03;
   gBMSConfig.BQ76Para.SCD_Thresh=0x07;
   gBMSConfig.BQ76Para.OCD_Delay=0x07;
   gBMSConfig.BQ76Para.OCD_Thresh=0x0F;
   gBMSConfig.BQ76Para.UV_Delay=0x03;
   gBMSConfig.BQ76Para.OV_Delay=0x03;
   gBMSConfig.BQ76Para.UV_Thresh=2500;
   gBMSConfig.BQ76Para.OV_Thresh=4200;
  
   gBMSConfig.DataCRC=0-DataCRC(gBMSConfig.DataLeng,(u8 *)&gBMSConfig);
}


void WriteAllData(void)
{	
		STMFLASH_Write(FLASH_SAVE_ADDR ,gBMSConfig.DataLeng,  (u16 *)&gBMSConfig);
}


void System_Pare_Get(void)
{
	  u16 ucA=0;  
	  gBMSReadConfig.DataLeng=sizeof(gBMSReadConfig);
    STMFLASH_Read(FLASH_SAVE_ADDR ,gBMSReadConfig.DataLeng,  (u16 *)&gBMSReadConfig);
	  ucA=DataCRC(gBMSReadConfig.DataLeng,(u8 *)&gBMSReadConfig);
	  
    if(ucA!=0)
    {   STMFLASH_Read(FLASH_SAVE_ADDR ,gBMSReadConfig.DataLeng,  (u16 *)&gBMSReadConfig);
				ucA=DataCRC(gBMSReadConfig.DataLeng,(u8 *)&gBMSReadConfig);
        if(ucA!=0)
        {     
              InitPara0();
							WriteAllData();
        }
				else
				{
						memcpy((u8 *)&gBMSConfig, (u8 *)&gBMSReadConfig, gBMSReadConfig.DataLeng);
				}
    }
		else
		{		
				memcpy((u8 *)&gBMSConfig, (u8 *)&gBMSReadConfig, gBMSReadConfig.DataLeng);
		}	
}





































































