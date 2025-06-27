#include "bq769xx.h"     // ���� BQ769xx ϵ��оƬ������ͷ�ļ�
#include "DataBase.h"    // �������ݿ⣨���ܴ洢BMSʵʱ���ݡ����õȣ���ͷ�ļ�
#include "ConfigPara.h"  // �������ò�����ͷ�ļ�
#include "iic.h"         // ���� I2C ͨ��Э���ͷ�ļ�
#include "timer.h"       // ������ʱ�����ܵ�ͷ�ļ�
#include "adc.h"         // ���� ADC (ģ��ת����) ���ܵ�ͷ�ļ�
#include "Rs485Data.h"   // ���� RS485 ͨ�����ݵ�ͷ�ļ�
#include "gpio.h"        // ���� GPIO (ͨ���������) ���Ƶ�ͷ�ļ�
#include<string.h>       // ������׼�ַ�����������ͷ�ļ� (�� memcpy)


//-------------ȫ�ֱ���-------------//
RegisterGroup  Bq769xxReg;      // BQ769xx оƬ���мĴ����Ľṹ����������ڴ洢�Ͳ����Ĵ���ֵ
u8  VoltCellOffSet=0;         // ��о��ѹADCԭʼֵ��ƫ���� (��оƬ��ȡ)
u16 VoltCellGainMV = 0;       // ��о��ѹADC���棬��λ: mV/LSB (ʵ���� ��V/LSB / 1000)
u32 VoltCellGainUV = 0;       // ��о��ѹADC���棬��λ: ��V/LSB (��оƬ��ȡ������õ�)
u8  BQ769xx_Init_State=0;     // BQ769xx оƬ��ʼ��״̬��־ (0: δ��ʼ����ʧ��, 1: ��ʼ���ɹ�)
u8  Bq769xx_Wake_EN=0;        // BQ769xx оƬ����ʹ�ܱ�־ (1: ʹ�ܻ�������)
u8  BQ769xxGatherData[40]={0}; // ���ڴ洢��BQ769xxоƬһ���Զ�ȡ���������� (���ѹ���¶ȵ�)
u16 VoltageTemp[3]={0};       // �洢�ⲿ�¶ȴ�����ת��ǰ�ĵ�ѹֵ (���3��)


//------------------------------------//

//-------------�ֲ�����-------------//
u8 bq769xx_Wake_cont=0;       // BQ769xx �������м����������ڿ��ƻ��������ʱ��
//------------------------------------//

// ������������̬���������ڱ��ļ��ڿɼ�
static void pack_temp_max_min(void); // ����һ�����������ڼ����ذ����¶ȵ����ֵ����Сֵ
static void pack_cell_max_min(void); // ����һ�����������ڼ����ذ��е�о��ѹ�����ֵ����Сֵ

/**
 * @brief  ���� BQ769xx оƬ
 * @param  ��
 * @retval ��
 * @detail ͨ���� BQ769xx �� TS1/BOOT ���ŷ���һ���ض�ʱ��������������оƬ
 */
void  BQ769xx_Wake(void)
{
		//--------����-------------------//
		BQ769xx_WAKE_ON;	  // �궨�壬ͨ��������TS1/BOOT����
		delay_ms(1000);		  // ��ʱ1000���룬���ֻ����ź�
		BQ769xx_WAKE_OFF;	  // �궨�壬ͨ��������TS1/BOOT����
}

/**
 * @brief  ���� BQ769xx оƬ�ĵ�о����ӳ��
 * @param  ��
 * @retval ��
 * @detail �������õĴ�����о���� (gBMSConfig.Type.CellNum_Ser)��
 *         ���� gBMSData.BattPar.Cells_Map ��ֵ��
 *         Cells_Map ��һ��λ���룬��ʾ��Щ BQ769xx �ĵ�ѹ����ͨ����ʹ���ˡ�
 */
void  BQ769xx_Set_CellMap(void)
{
   switch(gBMSConfig.Type.CellNum_Ser) // ����ȫ�������еĴ�����о����
   {
	 case 15: // 15��
        gBMSData.BattPar.Cells_Map=0x7FFF; // ��Ӧ BQ76940, ����15����о���붼ʹ��
        break;
     case 14: // 14��
        gBMSData.BattPar.Cells_Map=0x5FFF; // �������� VC5X �� VC10X �е�һ��
        break;
	 case 13:  // 13��
        gBMSData.BattPar.Cells_Map=0x5EFF; // �������� VC5X �� VC10X
        break;
	 case 12:  // 12��
        gBMSData.BattPar.Cells_Map=0x5EF7;
        break;
	 case 11:  // 11��
        gBMSData.BattPar.Cells_Map=0x4EF7;
        break;
     case 10: // 10��
        gBMSData.BattPar.Cells_Map=0x4E77; // ��Ӧ BQ76930, ����10����о���붼ʹ��
        break;
     case 9:  // 9��
        gBMSData.BattPar.Cells_Map=0x4E73;
        break;
      default: // Ĭ��15�� (�����֧�ִ���)
        gBMSData.BattPar.Cells_Map=0x7FFF;
        break;
   }
}

/**
 * @brief  ��ʼ�� BQ769xx оƬ�Ĳ�����RAM
 * @param  ��
 * @retval ��
 * @detail �˺����� RAM ������ Bq769xxReg �ṹ���еĸ����Ĵ���λ��
 *         ��Щֵ�Ժ�ᱻд�뵽 BQ769xx оƬ�С�
 */
void BQ769xx_Init_Para(void)
{
      // ϵͳ���ƼĴ���1 (SYS_CTRL1)
      Bq769xxReg.SysCtrl1.SysCtrl1Bit.ADC_EN=ADC_ENS;          // ADC_EN λ: ʹ��ADC (ͨ����Ϊ1)
      Bq769xxReg.SysCtrl1.SysCtrl1Bit.TEMP_SEL=EXTE_TEMP_ON;    // TEMP_SEL λ: ѡ���ⲿ��������TS1�����¶Ȳ���

      // ϵͳ���ƼĴ���2 (SYS_CTRL2)
      Bq769xxReg.SysCtrl2.SysCtrl2Bit.DELAY_DIS=ALARM_DELAY_ON; // DELAY_DIS λ: 0=ʹ�ܱ����ӳ�, 1=���ñ����ӳ� (�˴��궨����ܱ�ʾʹ��)
      //Bq769xxReg.SysCtrl2.SysCtrl2Bit.CC_EN=CC_CONTINU_DIS;     // CC_EN λ: 0=���ÿ��ؼ� (��ע��)
	  Bq769xxReg.SysCtrl2.SysCtrl2Bit.CC_EN=CC_CONTINU_EN;       // CC_EN λ: 1=ʹ�ܿ��ؼ� (����ģʽ)
      Bq769xxReg.SysCtrl2.SysCtrl2Bit.CHG_ON=CHG_OFF;          // CHG_ON λ: 0=�رճ��FET (��ʼ״̬)
      Bq769xxReg.SysCtrl2.SysCtrl2Bit.DSG_ON=DSG_OFF;          // DSG_ON λ: 0=�رշŵ�FET (��ʼ״̬)

      // �����Ĵ���1 (PROTECT1)
      Bq769xxReg.Protect1.Protect1Bit.RSNS=OCD_SCD_HIGH_RANGE;  // RSNS λ: 1=SCD/OCD���ʹ�øߵ�����Χ
      Bq769xxReg.Protect1.Protect1Bit.SCD_DELAY = gBMSConfig.BQ76Para.SCD_Delay;   // SCD_DELAY λ: ���ö�·�����ӳ�ʱ��

      // �����Ĵ���2 (PROTECT2)
      Bq769xxReg.Protect2.Protect2Bit.OCD_DELAY = gBMSConfig.BQ76Para.OCD_Delay;   // OCD_DELAY λ: ���ù��������ӳ�ʱ��

      // �����Ĵ���1 (PROTECT1) - ����
      Bq769xxReg.Protect1.Protect1Bit.SCD_THRESH =gBMSConfig.BQ76Para.SCD_Thresh; // SCD_THRESH λ: ���ö�·������ֵ

      // �����Ĵ���2 (PROTECT2) - ����
      Bq769xxReg.Protect2.Protect2Bit.OCD_THRESH = gBMSConfig.BQ76Para.OCD_Thresh; // OCD_THRESH λ: ���ù���������ֵ

      // �����Ĵ���3 (PROTECT3)
      Bq769xxReg.Protect3.Protect3Bit.OV_DELAY = gBMSConfig.BQ76Para.OV_Delay;     // OV_DELAY λ: ���ù�ѹ�����ӳ�ʱ��
      Bq769xxReg.Protect3.Protect3Bit.UV_DELAY = gBMSConfig.BQ76Para.UV_Delay;     // UV_DELAY λ: ����Ƿѹ�����ӳ�ʱ��
}

/**
 * @brief  ��ȡ BQ769xx оƬ�� ADC �����ƫ��У׼ֵ
 * @param  ��
 * @retval ��
 * @detail ��оƬ��ȡ ADCGAIN1, ADCGAIN2, ADCOFFSET �Ĵ�����
 *         ��������Щֵ����ʵ�ʵĵ�ѹ�����ƫ�ơ�
 *         Ȼ��������õĹ�ѹ/Ƿѹ��ֵ��У׼ֵ������д�� OVTrip �� UVTrip �Ĵ�����ֵ��
 */
void BQ769xx_Read_Gain(void)
{
      u8  rcd=0; // ���ڴ洢I2C�����ķ���ֵ
      // ͨ��I2C��ȡADC�����ƫ�ƼĴ�����ֵ
      rcd= IIC_ReadByte(BQ769xxAddr, ADCGAIN1_RegAddr, &Bq769xxReg.ADCGain1.ADCGain1Byte  ); // ��ȡADCGAIN1�Ĵ���
      rcd= IIC_ReadByte(BQ769xxAddr, ADCGAIN2_RegAddr,  &Bq769xxReg.ADCGain2.ADCGain2Byte ); // ��ȡADCGAIN2�Ĵ���
      rcd= IIC_ReadByte(BQ769xxAddr, ADCOFFSET_RegAddr, &Bq769xxReg.ADCOffset);             // ��ȡADCOFFSET�Ĵ���

      // ����BQ769xx datasheet�еĹ�ʽ�����ѹ���� (��λ: ��V/LSB)
      VoltCellGainUV = (365 + ((Bq769xxReg.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Bq769xxReg.ADCGain2.ADCGain2Byte & 0xE0)>> 5));
      // ����V/LSBת��ΪmV/LSB (����ֵ)
      VoltCellGainMV = VoltCellGainUV / 1000;
      // ��ѹƫ���� (��λ: LSB, 1 LSB = 1 ADCOFFSET��λ)
      VoltCellOffSet = Bq769xxReg.ADCOffset;

      // �������õĹ�ѹ��ֵ��Ƿѹ��ֵ������ķ�Χ��
      gBMSConfig.BQ76Para.OV_Thresh=LimitMaxMin(gBMSConfig.BQ76Para.OV_Thresh,OV_THRESH_MAX,OV_THRESH_MIN);
      gBMSConfig.BQ76Para.UV_Thresh=LimitMaxMin(gBMSConfig.BQ76Para.UV_Thresh,UV_THRESH_MAX,UV_THRESH_MIN);

      // ����д�� OVTrip �Ĵ�����ֵ (��ѹ��ֵ)
      // ����Ŀ���ѹֵ 4180mV
      Bq769xxReg.OVTrip = (u8)(((((unsigned long)(gBMSConfig.BQ76Para.OV_Thresh - Bq769xxReg.ADCOffset)*1000)/VoltCellGainUV - OV_THRESH_BASE)>>4)&0xff);
      // ����д�� UVTrip �Ĵ�����ֵ (Ƿѹ��ֵ)
      // ����Ŀ��Ƿѹֵ 3200mV
      Bq769xxReg.UVTrip = (u8)(((((unsigned long)(gBMSConfig.BQ76Para.UV_Thresh - Bq769xxReg.ADCOffset)*1000)/VoltCellGainUV - UV_THRESH_BASE)>>4)&0xff);

      // CC_CFG ���ؼ����üĴ�����0x19 ��һ����������
      Bq769xxReg.CCCfg=0x19;

	  (void) rcd; // �����������δʹ�ñ���rcd�ľ���
}

/**
 * @brief  ��RAM��׼���õ����ò���д��BQ769xxоƬ
 * @param  ��
 * @retval u8 - gRET_OK (0) ��ʾ�ɹ�, gRET_NG (1) ��ʾʧ��
 * @detail �� Bq769xxReg �ṹ����ϵͳ���ƺͱ�����ص�����д��оƬ��
 *         д����ض���Щ�Ĵ�������У�飬���У��ʧ�ܻ��������5�Ρ�
 */
u8 BQ769xx_Write_Config(void)
{
     u8  ret=0;      // ��������ֵ��Ĭ��Ϊ0 (�ɹ�)
     u8  rcd=0;      // I2C��������ֵ
     u8  Count=0;    // ���Լ�����
     u8  err=0;      // �����־��0��ʾ�޴���
     u8  bqSysCtrProtectionConfig[11]={0};  // ���ڴ洢�ض��ļĴ���ֵ

     do{
         // ��Bq769xxReg�е�ϵͳ���ƺͱ������� (��8���ֽ�) д��BQ769xxоƬ
         rcd=IIC_WritByteMore(BQ769xxAddr, SYS_CTRL1_RegAddr, &(Bq769xxReg.SysCtrl1.SysCtrl1Byte), 8);    // д�����üĴ���
         // ��BQ769xxоƬ�ض��ղ�д���8���ֽڣ�����У��
         rcd =IIC_ReadByteMore(BQ769xxAddr, SYS_CTRL1_RegAddr, bqSysCtrProtectionConfig, 8);

         // �������б�ע�͵��ˣ�ԭ�������д���У�����Ĵ���
         // rcd=IIC_WritByteMore(BQ769xxAddr, CELLBAL1_RegAddr, &(Bq769xxReg.CellBal1.CellBal1Byte), 11);
         // rcd =IIC_ReadByteMore(BQ769xxAddr, CELLBAL1_RegAddr, bqSysCtrProtectionConfig, 11);

         err = 0; // ���ô����־
         // �Ƚ�д���ֵ�ͻض���ֵ�Ƿ�һ��
         if(bqSysCtrProtectionConfig[0]!=Bq769xxReg.SysCtrl1.SysCtrl1Byte || // SYS_CTRL1
            bqSysCtrProtectionConfig[1]!=Bq769xxReg.SysCtrl2.SysCtrl2Byte || // SYS_CTRL2
            bqSysCtrProtectionConfig[2] != Bq769xxReg.Protect1.Protect1Byte|| // PROTECT1
            bqSysCtrProtectionConfig[3] != Bq769xxReg.Protect2.Protect2Byte|| // PROTECT2
            bqSysCtrProtectionConfig[4] != Bq769xxReg.Protect3.Protect3Byte|| // PROTECT3
            bqSysCtrProtectionConfig[5] != Bq769xxReg.OVTrip                || // OV_TRIP
            bqSysCtrProtectionConfig[6] != Bq769xxReg.UVTrip                || // UV_TRIP
            bqSysCtrProtectionConfig[7] != Bq769xxReg.CCCfg )                // CC_CFG
          {
              err++; // �����һ�£������־��λ
          }
         Count++; // ���Դ�����1
      }while((err!=0)&&(Count<5)); // ����д��������Դ���С��5�����������

     if(err==0) // ������ͨ����err==0��ʾ�ɹ�
     {
          ret=gRET_OK; // ��Ϊ�ɹ�
     }
     else // �������5�κ���Ȼ�д���
     {
          ret=gRET_NG; // ���Ϊʧ��
          // ���RAM�еļĴ�����������ʾ������Ч
          Bq769xxReg.SysCtrl1.SysCtrl1Byte=0;
          Bq769xxReg.SysCtrl2.SysCtrl2Byte=0;
          Bq769xxReg.Protect1.Protect1Byte=0;
          Bq769xxReg.Protect2.Protect2Byte=0;
          Bq769xxReg.Protect3.Protect3Byte=0;
          Bq769xxReg.OVTrip=0;
          Bq769xxReg.UVTrip=0;
          Bq769xxReg.CCCfg=0;
     }
	 (void) rcd; // �����������δʹ�ñ���rcd�ľ���
     return  ret;
}

/**
 * @brief  ��ʼ�� BQ769xx оƬ
 * @param  ��
 * @retval ��
 * @detail ����ִ�����õ�оӳ�䡢��ʼ��������RAM����ȡ����/ƫ�ơ�д�����õ�оƬ�Ȳ��衣
 *         ����д�����õĽ������ BQ769xx_Init_State ״̬��
 */
void BQ769xx_Init(void)
{
      u8  rcd=0; // ���ڴ洢��������ֵ
      //--------���õ�о �¶ȷֲ�(ʵ���ǵ�о����ӳ��)------//
      BQ769xx_Set_CellMap();
     //---------������ʼ��(��RAM)-------------//
      BQ769xx_Init_Para();
     //---------��ȡ��ѹ�����ƫ��-----//
      BQ769xx_Read_Gain();
     //----------��������(д��оƬ)--------------//
      rcd=BQ769xx_Write_Config();

      if(rcd==gRET_OK) // ���д�����óɹ�
      {
          BQ769xx_Init_State=1; // ���ó�ʼ��״̬Ϊ�ɹ�
      }
      else // ���д������ʧ��
      {
          BQ769xx_Init_State=0; // ���ó�ʼ��״̬Ϊʧ��
      }
}

/**
 * @brief  ��������ֵ�����ֵ����Сֵ֮��
 * @param  InValue ����ֵ
 * @param  gMax ��������ֵ
 * @param  gMin �������Сֵ
 * @retval u16 - ���ƺ��ֵ
 */
u16 LimitMaxMin(u16 InValue,u16 gMax,u16 gMin)
{
	u16 valueout=0; // ���ֵ

	valueout=InValue; // Ĭ�����ֵΪ����ֵ
    if(InValue>gMax )   // �������ֵ�������ֵ
    {
        valueout=gMax;  // ���ֵ��Ϊ���ֵ
    }

    if(InValue<gMin )   // �������ֵС����Сֵ
    {
        valueout=gMin;  // ���ֵ��Ϊ��Сֵ
    }
    return valueout;    // �������ƺ��ֵ
}

/**
 * @brief  �� BQ769xx оƬ��ȡʵʱ����
 * @param  ��
 * @retval ��
 * @detail �ú�����200ms��ʱ��־����ʱִ�С�
 *         ͨ��I2Cһ���Զ�ȡ40�ֽڵ����ݣ��������е�о��ѹ����ذ��ܵ�ѹ��
 *         �ⲿ�¶ȴ�������ѹ�Ϳ��ؼƼ���ֵ��
 *         Ȼ�����Щԭʼ���ݽ��д���ת��Ϊʵ�ʵ����������洢��
 */
void BQ769xx_GetData(void)
{
	u8  rcd=0;        // I2C��������ֵ
	u8  ia=0,ib=0;    // ѭ��������
	u16 cells_map=gBMSData.BattPar.Cells_Map; // ��ȡ��ǰ��о����ӳ��
	u8  VoltOffset=0; // ��ʱ�洢��ѹƫ����
	u16 AdcCurr=0;    // �洢ԭʼ�ĵ���ADCֵ

    // ���200ms��ʱ��־δ��λ����ִ�����ݲɼ�
    if(TaskTimePare.Tim200ms_flag != 1)
    {
		return;
	}

	//---------------------�ɼ�BQ769xx����-------------------------------//
	// ��VC1_HI_RegAddr��ʼ��������ȡ40���ֽ�
	rcd= IIC_ReadByteMore(BQ769xxAddr, VC1_HI_RegAddr, BQ769xxGatherData,40);   //.VCell1.VCell1Byte.VC1_HI
	if( rcd==gRET_OK ) // ���I2C��ȡ�ɹ�
	{
		// ����ȡ����40�ֽ����ݿ����� Bq769xxReg �ṹ���ж�Ӧ��λ��
		// ���� Bq769xxReg.VCell1.VCell1Byte.VC1_HI ��һ��ָ��40�ֽڻ�������ָ���������ʼ
		memcpy(&(Bq769xxReg.VCell1.VCell1Byte.VC1_HI), BQ769xxGatherData, 40);
	}
	 //--------------------------��о��ѹ����-----------------------------------//
	ib = 0; // gBMSData.BattPar.VoltCell ���������
	for(ia=0;ia<SYS_CELL_MAX;ia++) // ����BQ769xxоƬ���п��ܵĵ�о����
	{
		if(cells_map&0x01) // ��鵱ǰ��о�����Ƿ�ʹ��
		{
			// ADCOffset ��һ�������ŵ�8λֵ
			if(Bq769xxReg.ADCOffset&0x80) // ���ƫ�����Ƿ�Ϊ��
			{
				VoltOffset= 0x100-Bq769xxReg.ADCOffset; // ���㸺ƫ�Ƶľ���ֵ
				// ��ѹ���㹫ʽ: ( (RawADC_HighByte*256 + RawADC_LowByte) * Gain_��V/LSB ) / 1000 - Offset_mV
				gBMSData.BattPar.VoltCell[ib]=(u16)(((unsigned long)((BQ769xxGatherData[ia*2]*256)+BQ769xxGatherData[ia*2+1])*VoltCellGainUV)/1000-VoltOffset);
			}
			else // ƫ����Ϊ��
			{
				VoltOffset= Bq769xxReg.ADCOffset; // ��ƫ��ֵ
				// ��ѹ���㹫ʽ: ( (RawADC_HighByte*256 + RawADC_LowByte) * Gain_��V/LSB ) / 1000 + Offset_mV
				gBMSData.BattPar.VoltCell[ib]=(u16)(((unsigned long)((BQ769xxGatherData[ia*2]*256)+BQ769xxGatherData[ia*2+1])*VoltCellGainUV)/1000+VoltOffset);
			}
			ib++; // ָ����һ����Ч�ĵ�о��λ
		}
		cells_map=cells_map>>1; // cells_map����һλ�������һ����о����
	}

	//-----------------------------����ܵ�ѹ����-----------------------------------//
	// �ܵ�ѹ������ BQ769xxGatherData �ĵ�30��31�ֽ�
	gBMSData.BattPar.VoltLine=(u16)(((unsigned long)((BQ769xxGatherData[30]*256)+BQ769xxGatherData[31])*BATVOLTLSB)/1000);   // ��λ: MV

	//-----------------------------�¶ȴ���-----------------------------------//
	// �ⲿ�¶ȴ���������
	for(ia=0;ia<SYS_TEMPEXT_MAX;ia++) // SYS_TEMPEXT_MAX ���ⲿ�¶ȴ�����������
	{
		// TSx ���ŵĵ�ѹ LSB = 382 ��V
		VoltageTemp[ia]=((unsigned long)((BQ769xxGatherData[ia*2+32]*256)+BQ769xxGatherData[ia*2+32+1])*382)/1000; // mV
		// ���� TemChange ��������ѹֵת��Ϊʵ���¶�ֵ (��C)
		gBMSData.BattPar.TempCell[ia]=TemChange(VoltageTemp[ia]);
	}

	//-----------------------------��������---------------------------------//
	// �������� (CC_HI, CC_LO) �� BQ769xxGatherData �ĵ�38��39�ֽ�
	AdcCurr=(u16)(BQ769xxGatherData[38]*256)+BQ769xxGatherData[39]; // ��ϳ�16λԭʼADCֵ

	// ����ֵ�Ǵ����ŵ�
	if(AdcCurr&0x8000) // ����Ǹ����� (�ŵ�)
	{
		AdcCurr=0x10000-AdcCurr; // ȡ����õ�����ֵ
		// ��������: (RawADC_Value * LSB_��V_per_Count) / Shunt_Resistor_mOhm
		// CRUUVOLTLSB: ���ؼ�ÿLSB����ĵ�ѹ�ͨ���� 8.44��V/LSB
		// gBMSConfig.Type.ShuntSpec: ��������ֵ (mOhm)
		gBMSData.BattPar.CurrLine=0x10000-(u16)(((unsigned long) AdcCurr*CRUUVOLTLSB)/gBMSConfig.Type.ShuntSpec) ;   // LSB=8.44uV , �������裺ShuntSpec mOhm
	}
	else // ������ (���)
	{
		gBMSData.BattPar.CurrLine=(u16)(((unsigned long)AdcCurr*CRUUVOLTLSB)/gBMSConfig.Type.ShuntSpec) ;   // LSB=8.44uV , �������裺ShuntSpec mOhm
	}

	pack_temp_max_min(); // ���µ�ذ����/����¶�
	pack_cell_max_min(); // ���µ�ذ����/��͵�о��ѹ

	(void) rcd; // �����������δʹ�ñ���rcd�ľ���
}

/**
 * @brief  �� BQ769xx оƬ��ȡ���ú�״̬��Ϣ
 * @param  ��
 * @retval ��
 * @detail �ú�����500ms��ʱ��־����ʱִ�С�
 *         ͨ��I2Cһ���Զ�ȡ12�ֽڵ����ݣ��� SYS_STAT_RegAddr ��ʼ��
 *         ��ȡ�����ݻ´���� Bq769xxReg �ṹ���С�
 */
void BQ769xx_GetConfig(void)
{
	u8  rcd=0; // I2C��������ֵ
	u8  BQ769xxGatherConfig[12]={0}; // �洢��ȡ������/״̬����

	if(TaskTimePare.Tim500ms_flag==1 ) // ���500ms��ʱ��־��λ
    {
		// ��SYS_STAT_RegAddr��ʼ����ȡ12���ֽ�
		rcd= IIC_ReadByteMore(BQ769xxAddr, SYS_STAT_RegAddr, BQ769xxGatherConfig,12);
		if( rcd==gRET_OK ) // ���I2C��ȡ�ɹ�
		{
			// ����ȡ�����ݿ����� Bq769xxReg �ṹ���ж�Ӧ��״̬�Ĵ�������
			memcpy(&(Bq769xxReg.SysStatus.StatusByte), BQ769xxGatherConfig, 12);
		}
	}
}

/**
 * @brief  ���� BQ769xx �ķŵ�FET (DSG FET) ״̬
 * @param  ONOFF 1: ����DSG FET, 0: �ر�DSG FET
 * @retval u8 - gRET_OK (0) ��ʾ�ɹ�, gRET_NG (1) ��ʾʧ�ܻ��������
 */
u8 BQ769xx_DSGSET(u8 ONOFF)
{
    u8 ret=gRET_OK; // ��������ֵ��Ĭ�ϳɹ�
    u8 rcd=0;       // I2C��������ֵ

    if( ONOFF>1) // ������飬ONOFFֻ����0��1
    {
        return gRET_NG ; // �������󣬷���ʧ��
    }
    // ��ȡ��ǰ��ϵͳ���ƼĴ���2 (SYS_CTRL2) ��ֵ
    rcd=IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr, &(Bq769xxReg.SysCtrl2.SysCtrl2Byte));
    if(rcd!=gRET_OK) // �����ȡʧ��
    {
        return gRET_NG; // ����ʧ��
    }

    if(ONOFF==1) // ���Ҫ����DSG FET
    {
          // �� SYS_CTRL2 �Ĵ����� DSG_ON λ (bit 1) ��1
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte|0x02);
    }
    else // ���Ҫ�ر�DSG FET
    {
          // �� SYS_CTRL2 �Ĵ����� DSG_ON λ (bit 1) ��0
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte&0xFD); // 0xFD = 11111101b
    }
    return ret; // ���سɹ�
}

/**
 * @brief  ���� BQ769xx �ĳ��FET (CHG FET) ״̬
 * @param  ONOFF 1: ����CHG FET, 0: �ر�CHG FET
 * @retval u8 - gRET_OK (0) ��ʾ�ɹ�, gRET_NG (1) ��ʾʧ�ܻ��������
 */
u8 BQ769xx_CHGSET(u8 ONOFF)
{
    u8 ret=gRET_OK; // ��������ֵ��Ĭ�ϳɹ�
    u8 rcd=0;       // I2C��������ֵ

    if( ONOFF>1) // ������飬ONOFFֻ����0��1
    {
        return gRET_NG ; // �������󣬷���ʧ��
    }

    // ��ȡ��ǰ��ϵͳ���ƼĴ���2 (SYS_CTRL2) ��ֵ
    rcd = IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr,  &Bq769xxReg.SysCtrl2.SysCtrl2Byte );

    if(rcd!=gRET_OK) // �����ȡʧ��
    {
        return gRET_NG ; // ����ʧ��
    }

    if(ONOFF==1) // ���Ҫ����CHG FET
    {
          // �� SYS_CTRL2 �Ĵ����� CHG_ON λ (bit 0) ��1
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte|0x01 );
     }
     else // ���Ҫ�ر�CHG FET
     {
          // �� SYS_CTRL2 �Ĵ����� CHG_ON λ (bit 0) ��0
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte&0xFE ); // 0xFE = 11111110b
     }
    return ret ; // ���سɹ�
}

/**
 * @brief  ��ȡ BQ769xx оƬ��ϵͳ״̬�Ĵ��� (SYS_STAT) ��ֵ
 * @param  ��
 * @retval u8 - SYS_STAT �Ĵ����ĵ�ǰֵ
 */
u8 BQ769xx_STATE_GET(void)
{
  u8 rcd=0;     // I2C��������ֵ
  u8 BQ_Stat=0; // �洢��ȡ����״ֵ̬

  // ��ȡ SYS_STAT �Ĵ���
  rcd=IIC_ReadByte(BQ769xxAddr, SYS_STAT_RegAddr, &BQ_Stat  );
  (void) rcd;   // �����������δʹ�ñ���rcd�ľ���
  return  BQ_Stat; // ����״ֵ̬
}

/**
 * @brief  ��� BQ769xx оƬϵͳ״̬�Ĵ��� (SYS_STAT) �е�ָ����־λ
 * @param  statvalue - һ��λ���룬ָʾҪ�����Щ״̬��־λ��
 * @retval u8 - gRET_OK (0) �� gRET_NG (1)
 * @detail BQ769xx��SYS_STAT�Ĵ����еı�־λ��ͨ�����Ӧλд'1'������ġ�
 *         �˺�����д�������ض�������Ƿ�����ɹ���
 *         ע�⣺ԭ�����߼��жϳɹ�/ʧ�ܵķ�ʽ�����볣������෴��
 */
u8 BQ769xx_STAT_CLEAR(u8 statvalue)
{
  u8 ret=gRET_OK; // ��������ֵ
  u8 BQ_Stat=0;   // ���ڻض�״ֵ̬
  u8 rcd=0;       // I2C��������ֵ

  // �� SYS_STAT �Ĵ���д�� statvalue��Ŀ������� statvalue ��Ϊ'1'����Щλ
  rcd=IIC_WritByte(BQ769xxAddr, SYS_STAT_RegAddr,statvalue);

  // �ض� SYS_STAT �Ĵ����������������Ƿ�ɹ�
  rcd=IIC_ReadByte(BQ769xxAddr, SYS_STAT_RegAddr, &BQ_Stat );

  // BQ769xx�У�д1���㡣��������BQ_Stat�ж�Ӧstatvalue��λ��Ȼ��1��˵��δ����ɹ���
  // ԭ�����߼������Ҫ�����λ�У���λ��Ȼ��1������OK������������ˣ�����NG��
  // ����ͨ������෴�����ﰴԭ�߼�ע�͡�
  if(BQ_Stat&statvalue) // ���Ҫ�����λ�У���λ��Ȼ��1
  {
      ret=gRET_OK; // ��ԭ�����߼�
  }
  else // ���Ҫ�����λ�����0��
  {
     ret=gRET_NG; // ��ԭ�����߼�
  }

  (void)  rcd; // �����������δʹ�ñ���rcd�ľ���
  return  ret;
}

/**
 * @brief  ʹ BQ769xx ��������ģʽ (Ship Mode / Shutdown Mode)
 * @param  ��
 * @retval u8 - gRET_OK (0) ��ʾ�ɹ�, gRET_NG (1) ��ʾI2C��ȡʧ��
 * @detail ����ģʽ��BQ769xx����͹���״̬��
 *         ��������ģʽ��Ҫ�ض�������д������SYS_CTRL1�Ĵ�����
 *         ע�⣺ԭ�����ж�ȡSYS_CTRL1�浽��SysCtrl2Byte������ע���б�עǱ�����⡣
 */
u8 Enter_Ship_Mode(void)
{
    u8 ret=gRET_OK; // ��������ֵ��Ĭ�ϳɹ�
    u8 rcd=0;       // I2C��������ֵ

    // ע�⣺�����ȡ SYS_CTRL1 �Ĵ��������浽�� Bq769xxReg.SysCtrl2.SysCtrl2Byte��
    // ��ܿ����Ǹ�����Ӧ�ô浽 Bq769xxReg.SysCtrl1.SysCtrl1Byte��
    // ������� Bq769xxReg.SysCtrl1.SysCtrl1Byte ��ֵ����ȷ�ġ�
    rcd = IIC_ReadByte(BQ769xxAddr, SYS_CTRL1_RegAddr,  &Bq769xxReg.SysCtrl1.SysCtrl1Byte ); // ����: Ŀ��ӦΪ SysCtrl1Byte

    if(rcd!=gRET_OK) // �����ȡʧ��
    {
        return gRET_NG; // ����ʧ��
    }

    // ��������ģʽ��һ��: SHUT_A = 1, SHUT_B = 0
    Bq769xxReg.SysCtrl1.SysCtrl1Byte|=0x01;  // SHUT_A (bit 0) ��1
    Bq769xxReg.SysCtrl1.SysCtrl1Byte&=~0x02; // SHUT_B (bit 1) ��0
    IIC_WritByte(BQ769xxAddr, SYS_CTRL1_RegAddr, Bq769xxReg.SysCtrl1.SysCtrl1Byte );

    // ��������ģʽ�ڶ���: SHUT_A = 0, SHUT_B = 1
    Bq769xxReg.SysCtrl1.SysCtrl1Byte&=~0x01; // SHUT_A (bit 0) ��0
    Bq769xxReg.SysCtrl1.SysCtrl1Byte|=0x02;  // SHUT_B (bit 1) ��1
    IIC_WritByte(BQ769xxAddr, SYS_CTRL1_RegAddr, Bq769xxReg.SysCtrl1.SysCtrl1Byte);

    return  ret; // ���سɹ�
}

/**
 * @brief  ���� BQ769xx �Ŀ��ؼ� (CC) ���β���ģʽ
 * @param  ONOFF 1: ����һ�ε���CC����, 0: ��� (ͨ���Զ�����)
 * @retval u8 - gRET_OK (0) ��ʾ�ɹ�, gRET_NG (1) ��ʾʧ�ܻ��������
 */
u8 BQ769xx_CC_ONESHOT_SET(u8 ONOFF)
{
    u8 ret=gRET_OK; // ��������ֵ��Ĭ�ϳɹ�
    u8 rcd=0;       // I2C��������ֵ

    if( ONOFF>1) // �������
    {
        return gRET_NG ; // ��������
    }
    // ��ȡ��ǰ�� SYS_CTRL2 �Ĵ���ֵ
    rcd=IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr, &(Bq769xxReg.SysCtrl2.SysCtrl2Byte));
    if(rcd!=gRET_OK) // �����ȡʧ��
    {
        return gRET_NG;
    }

    if(ONOFF==1) // ���Ҫ����һ�ε��β���
    {
          // �� SYS_CTRL2 �Ĵ����� CC_ONESHOT λ (bit 5) ��1
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte|0x20);
    }
    else // ���Ҫ��� CC_ONESHOT λ
    {
          // �� SYS_CTRL2 �Ĵ����� CC_ONESHOT λ (bit 5) ��0
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte&0xDF); // 0xDF = 11011111b
    }
    // �ض� SYS_CTRL2 �Ĵ���������RAM�еĸ���
	rcd=IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr, &(Bq769xxReg.SysCtrl2.SysCtrl2Byte));
    return ret;
}

/**
 * @brief  д��BQ769xx�ĵ�о����Ĵ��� (CELLBAL1, CELLBAL2, CELLBAL3)
 * @param  cellbalanbyte1 - CELLBAL1 �Ĵ�����ֵ (���Ƶ�о1-5����)
 * @param  cellbalanbyte2 - CELLBAL2 �Ĵ�����ֵ (���Ƶ�о6-10����)
 * @param  cellbalanbyte3 - CELLBAL3 �Ĵ�����ֵ (���Ƶ�о11-15����)
 * @retval u8 - gRET_OK (0) ��ʾ�ɹ�, gRET_NG (1) ��ʾI2C����ʧ�ܻ�У��ʧ��
 */
u8 balance_write_regs(u8 cellbalanbyte1,u8 cellbalanbyte2,u8 cellbalanbyte3 )
{
    u8 ret=gRET_OK;      // ��������ֵ��Ĭ�ϳɹ�
    u8 rcd=0;            // I2C��������ֵ
    u8 write_regs[3]={0}; // �洢Ҫд���3���ֽ�
    u8 read_regs[3]={0};  // �洢�ض���3���ֽ�
    u8 ia=0;             // ѭ��������
    u16 cells_map=gBMSData.BattPar.Cells_Map; // cells_map �ڴ˺�����δ��ʹ��
    (void) cells_map; // �����������δʹ�ñ���cells_map�ľ���
    write_regs[0]=cellbalanbyte1;
    write_regs[1]=cellbalanbyte2;
    write_regs[2]=cellbalanbyte3;
    

    // �� CELLBAL1_RegAddr ��ʼ������д��3���ֽ�
    rcd=IIC_WritByteMore(BQ769xxAddr, CELLBAL1_RegAddr, write_regs,3);

    if( rcd!=gRET_OK ) // ���д��ʧ��
    {
          return  gRET_NG;
    }

    // �ض�д���3���ֽڽ���У��
    rcd=IIC_ReadByteMore(BQ769xxAddr, CELLBAL1_RegAddr, read_regs,3);
    if( rcd!=gRET_OK ) // ����ض�ʧ��
    {
          return  gRET_NG;
    }

    // �Ƚ�д��ֵ�ͻض�ֵ�Ƿ�һ��
    for(ia=0; ia<3; ia++ )
    {
        if(read_regs[ia]!=write_regs[ia])
        {
            ret=gRET_NG; // �����һ�£����Ϊʧ��
            break;       // ��������Ƚ�
        }
    }
	
    return ret;
}

/**
 * @brief  ���߼���о���⿪��ӳ�䵽BQ769xx�������Ĵ���λͼ
 * @param  balaCellSw - �߼���о���⿪��λͼ��bit0�����1���߼���о���Դ����ơ�
 * @param  cellbalaset - ����������洢ת������������Ĵ���λͼ��
 * @detail �˺��������˵�о����ӳ�� (Cells_Map)�����߼���о�ľ�������
 *         ת��Ϊ��BQ769xx��������ľ���λ��
 */
void cell_balan_locatget(u32 balaCellSw, u16* cellbalaset)
{
	u8 ia=0;     // ���ѭ���������������߼���о
    u8 ib=0;     // �ڲ�ѭ��������������BQ769xx��������
    u8 cntset=0; // ��������׷�ٵ�ǰ�ǵڼ����������ӡ�����������
    u16 cells_map=0; // ��ʱ�洢��о����ӳ��
    u32 outcellsw=0; // �洢���յ��������λͼ

	// ����ÿһ���߼���о
	for(ia=0; ia<gBMSConfig.Type.CellNum_Ser; ia++)
	{
		cntset=0; // �����������������������
		cells_map=gBMSData.BattPar.Cells_Map; // ��ȡԭʼ����������ӳ��
		(void)cells_map; // ���⾯��

		// ����BQ769xx���п��ܵ���������
		for(ib=0; ib<SYS_CELL_MAX;ib++)
		{
			if(cells_map&0x01) // �����ǰ��������(ib)�����ӵ�
			{
				if(cntset == ia) // ���������ӵ���������������Ҫ�ҵĵ�ia���߼���о
				{
					// cntset++; // �˴�����cntset���ܹ��磬��ԭ�߼����
					if(balaCellSw&(0x0001<<ia)) // ����ia���߼���о�Ƿ���Ҫ����
					{
						outcellsw |= (0x0001<<ib); // ����Ӧ����������λ(ib)��outcellsw����1
						break; // �ҵ��ˣ������ڲ�ѭ��
					}
				}
				cntset++; // �����Ƿ�ƥ�䣬ֻҪ�����ӵ��������룬cntset��Ӧ����
			 }
			 cells_map=cells_map>>1; // �����һ����������
		}
	}
    *cellbalaset=(u16)outcellsw; // ����������������
    
}

/**
 * @brief  ��ذ������������״̬��
 * @param  ��
 * @retval u8 - gRET_OK (0) ͨ����ʾ״̬����������,
 *              gRET_TM (�ض�ֵ) ���200ms��־δ��,
 *              gRET_NG (1) �����ǿ�ƾ����ϡ�
 * @detail �˺�����200ms������ִ�У������ذ����Զ�������̡�
 */
u8 Balan_Pack_Check(void)
{
    u8 ret=gRET_OK; // ��������ֵ
    u8 ia=0, ib=0;  // ѭ��������
	u16 volta_temp; // ��ʱ��������������ʱ������ѹֵ
	u8 volta_flag;  // ��ʱ��������������ʱ������о����

	static u8 stup_balan=0; // ����״̬����ǰ״̬
	static u16 volta_cell_sort[PACK_CELL_MAX] = {0}; // ���ڴ洢������ĵ�о��ѹ
	static u8 volta_flag_sort[PACK_CELL_MAX] = {0};  // ���ڴ洢������ѹ��Ӧ��ԭʼ��о����
	static u16 balanwait_cnt=0; // ����ȴ�������
	static u16 balantime_cnt=0; // �������ʱ���ļ�����

	// ���200ms��ʱ��־δ��λ����ִ��
    if(TaskTimePare.Tim200ms_flag==0)
    {
      return gRET_TM; // �����ض�ֵ��ʾʱ��δ��
    }

    // ���ǿ�ƾ��⿪������ִ���Զ������߼�
	if(gBMSData.BalaPar.Bala_force_on == 1)
	{
		return gRET_NG; // �����ض�ֵ��ʾ��ǿ�ƾ�����
	}

    switch(stup_balan) // ���ݵ�ǰ����״ִ̬�в���
    {
		case PCB_SEQ_INIT: // 0: ��ʼ��״̬
			gBMSData.BalaPar.bala_state = 0;    // ����״̬��־����
			gBMSData.BalaPar.bala_ctrl_sw = 0;  // �߼���о������ƿ���λͼ����
			gBMSData.BalaPar.bala_dischg=0;     // �������Ĵ���λͼ����

			// ���BQ769xx�ľ���Ĵ�����RAM�еĸ�����д��оƬ
			Bq769xxReg.CellBal1.CellBal1Byte=0;
			Bq769xxReg.CellBal2.CellBal2Byte=0;
			Bq769xxReg.CellBal3.CellBal3Byte=0;
			balance_write_regs(Bq769xxReg.CellBal1.CellBal1Byte, Bq769xxReg.CellBal2.CellBal2Byte, Bq769xxReg.CellBal3.CellBal3Byte );
			stup_balan = PCB_SEQ_WAIT; // ����ȴ�״̬
			break;
		case PCB_SEQ_WAIT: // 1: �ȴ�״̬
			if(++balanwait_cnt > 10) // �ȴ� balanwait_cnt * 200ms
			{
				balanwait_cnt = 0; // ���õȴ�������
				stup_balan = PCB_SEQ_CHK; // ������״̬
			}
			break;
		case PCB_SEQ_CHK: // 2: ����������״̬
			// ����1: ϵͳģʽ�����Ǵ���ģʽ
			if(gBMSData.Sys_Mod.sys_mode != SYS_MODE_STANDBY)
			{
				stup_balan = PCB_SEQ_INIT; // �������򷵻س�ʼ��״̬
				break;
			}
			// ����2: ѹ�����͵�ѹ���
			if(((gBMSData.BattPar.VoltCellMax - gBMSData.BattPar.VoltCellMin) < gBMSConfig.Balan.balanc_diffe_volt)||
			   (gBMSData.BattPar.VoltCellMin < gBMSConfig.Balan.balanc_start_volt))
			{
				stup_balan = PCB_SEQ_INIT; //�������򷵻س�ʼ��״̬
				break;
			}
			stup_balan = PCB_SEQ_SELECT; // �������㣬����ѡ������о״̬
			break;
		case PCB_SEQ_SELECT: // 3: ѡ����Ҫ����ĵ�о״̬
			// ���Ƶ�ѹ��������������
			for(ia=0;ia<gBMSConfig.Type.CellNum_Ser; ia++)
			{
				volta_cell_sort[ia] = gBMSData.BattPar.VoltCell[ia];
				volta_flag_sort[ia] = ia;
			}
            // ð�����򣬰���ѹ�Ӹߵ�������
            for(ia=0;ia<gBMSConfig.Type.CellNum_Ser; ia++)
			{
				for(ib=0;ib<(gBMSConfig.Type.CellNum_Ser -1 - ia); ib++) // �Ż��Ƚϴ���
				{
					if(volta_cell_sort[ib] < volta_cell_sort[ib+1])
					{
						volta_temp = volta_cell_sort[ib+1];
						volta_cell_sort[ib+1] = volta_cell_sort[ib];
						volta_cell_sort[ib] = volta_temp;

						volta_flag = volta_flag_sort[ib+1];
						volta_flag_sort[ib+1] = volta_flag_sort[ib];
						volta_flag_sort[ib] = volta_flag;
					}
				}
			}
			// ѡ��������ѹ��ߵ� N ����о���о���
			for(ia = 0; ia < gBMSConfig.Balan.balanc_number_max; ia++)
			{
				gBMSData.BalaPar.bala_ctrl_sw |= (0x00000001 << volta_flag_sort[ia]);
			}
			stup_balan = PCB_SEQ_MAP; // ����ӳ��״̬
            break;
        case PCB_SEQ_MAP:  // 4: ӳ���߼�����λ���������λ״̬
			cell_balan_locatget(gBMSData.BalaPar.bala_ctrl_sw, &gBMSData.BalaPar.bala_dischg);
			stup_balan = PCB_SEQ_SET; // �������þ���״̬
            break;
        case PCB_SEQ_SET: // 5: ���þ���Ĵ���״̬
			// �����������λͼ����������Ĵ����ֽ�
			Bq769xxReg.CellBal1.CellBal1Byte=gBMSData.BalaPar.bala_dischg&0x1F;
			Bq769xxReg.CellBal2.CellBal2Byte=(gBMSData.BalaPar.bala_dischg>>5)&0x1F;
			Bq769xxReg.CellBal3.CellBal3Byte=(gBMSData.BalaPar.bala_dischg>>10)&0x1F;
			// д��BQ769xxоƬ
			balance_write_regs( Bq769xxReg.CellBal1.CellBal1Byte, Bq769xxReg.CellBal2.CellBal2Byte, Bq769xxReg.CellBal3.CellBal3Byte );
			gBMSData.BalaPar.bala_state = 1; // ���þ���״̬Ϊ�������С�
			stup_balan=PCB_SEQ_BALA_TIM; // ��������ʱ״̬
            break;
        case PCB_SEQ_BALA_TIM: // 6: �����ʱ״̬
			if(++balantime_cnt>900)  // ������� balantime_cnt * 200ms
            {
				stup_balan = PCB_SEQ_INIT; // �ﵽԤ�����ʱ�䣬���س�ʼ��
                balantime_cnt=0; // ���ü�ʱ��
				// ����������״̬
				gBMSData.BalaPar.bala_state = 0;
				gBMSData.BalaPar.bala_ctrl_sw = 0;
				gBMSData.BalaPar.bala_dischg=0;

				Bq769xxReg.CellBal1.CellBal1Byte=0;
				Bq769xxReg.CellBal2.CellBal2Byte=0;
				Bq769xxReg.CellBal3.CellBal3Byte=0;
				balance_write_regs( Bq769xxReg.CellBal1.CellBal1Byte, Bq769xxReg.CellBal2.CellBal2Byte, Bq769xxReg.CellBal3.CellBal3Byte );
            }
            break;
        default: // �쳣״̬�����س�ʼ��
            stup_balan = PCB_SEQ_INIT;
            break;
     }
     return ret;
}

/**
 * @brief  ���㲢���µ�ذ����¶ȵ����ֵ����Сֵ (�������߼�)
 * @param  ��
 * @retval ��
 * @detail ����������Ч���¶ȴ������������ҵ���ߺ�����¶ȡ�
 */
static void pack_temp_max_min(void)
{
    u8 ia;
    u16 temp_max;
    u16 temp_min;

    // ��ʼ�����ֵ����СֵΪ��һ�����������¶�
    if (gBMSConfig.Type.TempNum > 0) { // ȷ��������һ���¶ȴ�����
        temp_max = gBMSData.BattPar.TempCell[0];
        temp_min = gBMSData.BattPar.TempCell[0];
    } else {
        // ���û���¶ȴ���������������һ��Ĭ��ֵ�����ֵ
        gBMSData.BattPar.TempPackMax = 0; // ����һ���ض��Ĵ������
        gBMSData.BattPar.TempPackMin = 0; // ����һ���ض��Ĵ������
        return;
    }

    // �����ӵڶ����¶ȴ�������ʼ�����д�����
    for(ia=1; ia<gBMSConfig.Type.TempNum; ia++)
    {
        if(gBMSData.BattPar.TempCell[ia] > temp_max)
        {
            temp_max = gBMSData.BattPar.TempCell[ia];
        }
        if(gBMSData.BattPar.TempCell[ia] < temp_min)
        {
            temp_min = gBMSData.BattPar.TempCell[ia];
        }
    }

    gBMSData.BattPar.TempPackMax = temp_max; // ���µ�ذ�����¶�
    gBMSData.BattPar.TempPackMin = temp_min; // ���µ�ذ�����¶�
}

/**
 * @brief  ���㲢���µ�ذ��е�о��ѹ�����ֵ����Сֵ
 * @param  ��
 * @retval ��
 * @detail ����������Ч�ĵ�о��ѹ�������ҵ���ߺ���͵�ѹ��
 */
static void pack_cell_max_min(void)
{
	u8 ia;
	u16 volt_max;
	u16 volt_min;

    // ��ʼ�����ֵ����СֵΪ��һ����о�ĵ�ѹ
    if (gBMSConfig.Type.CellNum_Ser > 0) { // ȷ��������һ����о
        volt_max = gBMSData.BattPar.VoltCell[0];
        volt_min = gBMSData.BattPar.VoltCell[0];
    } else {
        gBMSData.BattPar.VoltCellMax = 0;
        gBMSData.BattPar.VoltCellMin = 0;
        return;
    }

	// �����ӵڶ�����о��ʼ�����е�о
	for(ia = 1;ia < gBMSConfig.Type.CellNum_Ser;ia++)
	{
		if(gBMSData.BattPar.VoltCell[ia] > volt_max) // �������Ƚ�volt_max
		{
			volt_max=gBMSData.BattPar.VoltCell[ia];
		}
		if(gBMSData.BattPar.VoltCell[ia] < volt_min) // �������Ƚ�volt_min
		{
			volt_min = gBMSData.BattPar.VoltCell[ia];
		}
	}
	gBMSData.BattPar.VoltCellMax = volt_max; // ���µ�ذ���ߵ����ѹ
	gBMSData.BattPar.VoltCellMin = volt_min; // ���µ�ذ���͵����ѹ
}

// ������Щ�궨���������ط�
// extern u8 Bq769xx_Oper_EN;
// extern u8 Bq769xx_Oper_Type;
// #define OPER_ON 1
// #define OPER_OFF 0
// #define COMM_DSG_ON 1
// #define COMM_DSG_OFF 2
// #define COMM_CHG_ON 3
// #define COMM_CHG_OFF 4
// #define COMM_CLEAR_ALERT 5
// #define COMM_ENTER_SHIP 6

/**
 * @brief  BQ769xx �����������
 * @param  ��
 * @retval ��
 * @detail �ú�����100ms��ʱ��־����ʱ���� Bq769xx_Oper_EN ���ⲿ��λʱִ�С�
 *         ���� Bq769xx_Oper_Type ��ֵ��ִ����Ӧ�Ĳ�����
 */
void  BQ769xx_Oper_Comm(void)
{
    // ���100ms��ʱ��־��λ�����߲���ʹ�ܱ�־ΪON
    if((TaskTimePare.Tim100ms_flag==1)||(Bq769xx_Oper_EN==OPER_ON ) )
    {
		switch (Bq769xx_Oper_Type) // ���ݲ�������ִ��
		{
			case COMM_DSG_ON: // �򿪷ŵ�FET
				BQ769xx_DSGSET(OPER_ON);
				Bq769xx_Oper_EN=OPER_OFF; // �������ʹ�ܱ�־
				Bq769xx_Oper_Type=0;      // �����������
				break;
			case COMM_DSG_OFF: // �رշŵ�FET
				BQ769xx_DSGSET(OPER_OFF);
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_CHG_ON: // �򿪳��FET
				BQ769xx_CHGSET(OPER_ON);
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_CHG_OFF: // �رճ��FET
				BQ769xx_CHGSET(OPER_OFF);
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_CLEAR_ALERT: // ������� (��ǰδʵ�־������)
				// BQ769xx_STAT_CLEAR(ALERT_BITS_TO_CLEAR); // ʾ��
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_ENTER_SHIP: // ��������ģʽ
				Power_Down(); // ����ϵͳ�ϵ纯��
				// Power_Down �ڲ��ᴦ�� Bq769xx_Oper_EN �� Type
				break;
			default: // δ֪��������
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
		}
	}
}

/**
 * @brief  ϵͳ�ϵ纯��
 * @param  ��
 * @retval ��
 * @detail ʹBQ769xxоƬ��������ģʽ��Ȼ��ر���ϵͳ��Դ��
 */
void Power_Down(void)
{
	Enter_Ship_Mode();      // ʹBQ769xx��������ģʽ
	Bq769xx_Oper_EN=OPER_OFF; // �������ʹ�ܱ�־
	Bq769xx_Oper_Type=0;      // �����������
	SYS_POWER_OFF;          // �궨�壬�ر�MCU������ϵͳ��Դ
}

// #define WAKE_TIME_OUT_1S 2 // ���趨�廽�ѳ�ʱ (2 * 500ms = 1s)

/**
 * @brief  BQ769xx �����������
 * @param  ��
 * @retval ��
 * @detail �ú�����500ms��ʱ��־������ Bq769xx_Wake_EN ʹ��ʱִ�С�
 *         ͨ�� bq769xx_Wake_cont ���ƻ�������Ĳ����ͳ���ʱ�䡣
 */
void  BQ769xx_Wake_Comm(void)
{
	// ���500ms��ʱ��־��λ�����һ���ʹ�ܱ�־Ϊ1
	if((TaskTimePare.Tim500ms_flag==1)&&(Bq769xx_Wake_EN==1))
	{
		// bq769xx_Wake_cont �����λ(0x80)����״̬��־
		if(bq769xx_Wake_cont&0x80) // �����ǰ���ڷ��ͻ�������
		{
			// (bq769xx_Wake_cont&0x7F) ���������ʱ��ļ���
			if((bq769xx_Wake_cont&0x7f)>=WAKE_TIME_OUT_1S) // ��������ѴﵽԤ��ʱ��
			{
				BQ769xx_WAKE_OFF;     // ���ͻ������ţ���������
				bq769xx_Wake_cont=0;  // ���û��Ѽ�����
				Bq769xx_Wake_EN=0;    // �������ʹ�ܱ�־
				BQ769xx_Init();       // ���³�ʼ��BQ769xxоƬ
			}
			else  // �������ʱ��δ��
			{
				bq769xx_Wake_cont++; // ���Ӽ���
			}
		}
		else // �����ǰ���ڷ��ͻ�������״̬
		{
			BQ769xx_WAKE_ON;        // ���߻������ţ���ʼ��������
			bq769xx_Wake_cont=0;    // ���������
			bq769xx_Wake_cont|=0x80;// �����λ��1�����Ϊ�����巢���С�
		}
	}
}

