

#ifndef __CONFIGPARA_H
#define __CONFIGPARA_H	 
#include "sys.h"

#define EP_HEADER_ADDR   0x4000  

/*
gBMSConfig.BQ76Para.SCD_Delay=0;
                    SET: 0x00?70us
                    SET: 0x01?100us
                    SET: 0x02?200us
                    SET: 0x03?200us
gBMSConfig.BQ76Para.SCD_Thresh=0;
                            RSNS=1                RSNS=0
                    SET: 0x00?44mv         0x00?22mv
                    SET: 0x01?67mv         0x01?33mv
                    SET: 0x02?89mv         0x02?44mv
                    SET: 0x03?111mv        0x03?56mv
                    SET: 0x04?133mv        0x04?67mv
                    SET: 0x05?155mv        0x05?78mv
                    SET: 0x06?178mv        0x06?89mv
                    SET: 0x07?200mv        0x07?100mv

gBMSConfig.BQ76Para.OCD_Delay=0;
                    SET: 0x00?8ms
                    SET: 0x01?20ms
                    SET: 0x02?40ms
                    SET: 0x03?80ms
                    SET: 0x04?160ms
                    SET: 0x05?320ms
                    SET: 0x06?640ms
                    SET: 0x07?1280ms
gBMSConfig.BQ76Para.OCD_Thresh=0;
                            RSNS=1            RSNS=0
                    SET: 0x00?17mv        0x00?8mv
                    SET: 0x01?22mv        0x01?11mv
                    SET: 0x02?28mv        0x02?14mv
                    SET: 0x03?33mv        0x03?17mv
                    SET: 0x04?39mv        0x04?19mv
                    SET: 0x05?44mv        0x05?22mv
                    SET: 0x06?50mv        0x06?25mv
                    SET: 0x07?56mv        0x07?28mv
                    SET: 0x08?61mv        0x00?31mv
                    SET: 0x09?67mv        0x01?33mv
                    SET: 0x0A?72mv        0x02?36mv
                    SET: 0x0B?78mv        0x03?39mv
                    SET: 0x0C?83mv        0x04?42mv
                    SET: 0x0D?89mv        0x05?44mv
                    SET: 0x0E?94mv        0x06?47mv
                    SET: 0x0F?100mv       0x07?50mv

gBMSConfig.BQ76Para.UV_Delay=0;
                    SET: 0x00?1s
                    SET: 0x01?4s
                    SET: 0x02?8s
                    SET: 0x03?16s
gBMSConfig.BQ76Para.OV_Delay=0;
                    SET: 0x00?1s
                    SET: 0x01?4s
                    SET: 0x02?8s
                    SET: 0x03?16s

gBMSConfig.BQ76Para.UV_Thresh=1580;     
                    ??: MV
gBMSConfig.BQ76Para.OV_Thresh=4180;
                    ??: MV
*/


 typedef struct System_Type
 {
		u64 CapaRate;      //��ذ���о����  
		u8 VoltSpec;       // ��ذ���ѹ���
		u8 CellNum_Ser;    //��ذ�����
	 	u8 CellNum_Par;    //��ذ�����
		u8 TempNum;        //��ذ��¶�����
		u16 ShuntSpec;     //��ذ����������
		u16 OCVDaleyTime;   //��ֹ֮�� S/���OCVУ׼ 
  }sSystem_Type;

typedef struct System_Balan
{ 
	u16 balanc_start_volt;
	u16 balanc_diffe_volt;
	u16 balanc_number_max;   
	u16 balanc_oneall_time;	
}sSystem_Balan;

 typedef struct System_Alar_BQ76Para{
  u8 SCD_Delay;      //�ŵ��·��ʱʱ��
  u8 SCD_Thresh;     //�ŵ��·��ֵ
  u8 OCD_Delay;      //�ŵ������ʱ
  u8 OCD_Thresh;     //�ŵ������ֵ
  u8 UV_Delay;       //����Ƿѹ��ʱ
  u8 OV_Delay;       //�����ѹ��ʱ
  u16 UV_Thresh;     //����Ƿѹ��ֵ
  u16 OV_Thresh;     //�����ѹ��ֵ     
 }sSystem_BQ76Para;


 typedef struct BMSConfig{
   
   u8 DataLeng;
   sSystem_Type        Type;      
   sSystem_Balan       Balan ; 
   sSystem_BQ76Para    BQ76Para;
   u8  DataCRC;
  }sBMSConfig;

extern  sBMSConfig   gBMSConfig;


	
	
void InitPara0(void);
void WriteAllData(void);
void System_Pare_Get(void);


	
	
#endif




