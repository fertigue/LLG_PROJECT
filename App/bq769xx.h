#ifndef __BQ769XX_H
#define __BQ769XX_H
#include "sys.h" // ����ϵͳ��ͷ�ļ�

#define BQ769xxAddr   0x08 // BQ769xxϵ��оƬ��I2C�ӻ���ַ
#define SYS_STAT_RegAddr   0x00 // ϵͳ״̬�Ĵ�����ַ
#define CELLBAL1_RegAddr   0x01 // ��о����Ĵ���1��ַ (����VC1-VC5)
#define CELLBAL2_RegAddr   0x02 // ��о����Ĵ���2��ַ (����VC6-VC10)
#define CELLBAL3_RegAddr   0x03 // ��о����Ĵ���3��ַ (����VC11-VC15)
#define SYS_CTRL1_RegAddr  0x04 // ϵͳ���ƼĴ���1��ַ
#define SYS_CTRL2_RegAddr  0x05 // ϵͳ���ƼĴ���2��ַ
#define PROTECT1_RegAddr   0x06 // �����Ĵ���1��ַ (SCD���)
#define PROTECT2_RegAddr   0x07 // �����Ĵ���2��ַ (OCD���)
#define PROTECT3_RegAddr   0x08 // �����Ĵ���3��ַ (UV/OV��ʱ���)
#define OV_TRIP_RegAddr    0x09 // ��ѹ������ֵ�Ĵ�����ַ
#define UV_TRIP_RegAddr    0x0A // Ƿѹ������ֵ�Ĵ�����ַ
#define VC1_HI_RegAddr     0x0C // ��1�ڵ�о��ѹ���ֽڼĴ�����ַ (����VCx_HI/LO���ε���)
#define ADCGAIN1_RegAddr   0x50 // ADC����Ĵ���1��ַ
#define ADCOFFSET_RegAddr 0x51 // ADCƫ�ƼĴ�����ַ
#define ADCGAIN2_RegAddr   0x59 // ADC����Ĵ���2��ַ

// SYS_CTRL1 �Ĵ������λ����
#define   ADC_ENS     1  // ADCʹ�� (����SYS_CTRL1�Ĵ�����ADC_ENλ)
#define   ADC_DIS    0  // ADC���� (����SYS_CTRL1�Ĵ�����ADC_ENλ)
#define   EXTE_TEMP_ON     1  // ѡ���ⲿ����������� (����SYS_CTRL1�Ĵ�����TEMP_SELλ)
#define   EXTE_TEMP_OFF    0  // ѡ���ڲ�оƬ�¶Ȳ��� (����SYS_CTRL1�Ĵ�����TEMP_SELλ)

// SYS_CTRL2 �Ĵ������λ����
#define   ALARM_DELAY_ON   0  // ������ʱ���� (����SYS_CTRL2�Ĵ�����DELAY_DISλ)
#define   ALARM_DELAY_OFF  1  // ���ñ�����ʱ (����SYS_CTRL2�Ĵ�����DELAY_DISλ�����ڲ���)
#define   CC_CONTINU_DIS  0  // ���ÿ��ؼ�����ת�� (����SYS_CTRL2�Ĵ�����CC_ENλ)
#define   CC_CONTINU_EN   1  // ʹ�ܿ��ؼ�����ת�� (����SYS_CTRL2�Ĵ�����CC_ENλ)
#define   CHG_O0N    1  // �򿪳��FET (����SYS_CTRL2�Ĵ�����CHG_ONλ)
#define   CHG_OFF   0  // �رճ��FET (����SYS_CTRL2�Ĵ�����CHG_ONλ)
#define   DSG_O0N    1  // �򿪷ŵ�FET (����SYS_CTRL2�Ĵ�����DSG_ONλ)
#define   DSG_OFF   0  // �رշŵ�FET (����SYS_CTRL2�Ĵ�����DSG_ONλ)

// PROTECT1 �Ĵ������λ����
#define OCD_SCD_LOWER_RANGE   0  // OCD��SCD��ֵʹ�ýϵ����뷶Χ (����PROTECT1�Ĵ�����RSNSλ)
#define OCD_SCD_HIGH_RANGE    1  // OCD��SCD��ֵʹ�ýϸ����뷶Χ (����PROTECT1�Ĵ�����RSNSλ)

// ��ѹ/Ƿѹ������ֵ��ض���
#define OV_THRESH_BASE	      0x2008 // ��ѹ��ֵADCԭʼֵ�Ļ�׼ (��Ӧ10xxxx xxxx1000)
#define UV_THRESH_BASE	      0x1000 // Ƿѹ��ֵADCԭʼֵ�Ļ�׼ (��Ӧ01xxxx xxxx0000)
#define OV_THRESH_MAX         4700   // ��ѹ������ֵ���ֵ (��λ: mV, ����ֵ)
#define OV_THRESH_MIN         3150   // ��ѹ������ֵ��Сֵ (��λ: mV, ����ֵ)
#define UV_THRESH_MAX         3100   // Ƿѹ������ֵ���ֵ (��λ: mV, ����ֵ)
#define UV_THRESH_MIN         1580   // Ƿѹ������ֵ��Сֵ (��λ: mV, ����ֵ)

// ADCת��LSBֵ����
#define BATVOLTLSB            1532 // ��ذ��ܵ�ѹADC��LSB (��λ: uV, ʵ�ʼ�����Ҫ����4)
#define TMEPVOLTLSB           382  // �¶Ⱥ͵�о��ѹADC��LSB (��λ: uV)
#define CRUUVOLTLSB           844  // ���ؼ�ADC��LSB (��λ: uV, ʵ����8.44uV)

/*---------���������־λ-------*/
#define OCD_CLE      0x01 // ��������ŵ�(OCD)������־λ
#define SCD_CLE      0x02 // �����·�ŵ�(SCD)������־λ
#define OV_CLE       0x04 // �����ѹ(OV)������־λ
#define UV_CLE       0x08 // ���Ƿѹ(UV)������־λ
#define OVRD_ALERT_CLE      0x10 // ����ⲿALERT���Ǿ�����־λ
#define DEVICE_XREADY_CLE   0x20 // ����豸�ڲ����󾯱���־λ
#define CC_READY_CLE   0x80 // ������ؼ�׼������������־λ
/*-------------------------------*/

// ϵͳ��������
#define  SYS_CELL_MAX       15    // ϵͳ֧�ֵ�����о����
#define  SYS_TEMPEXT_MAX     3    // ϵͳ֧�ֵ�����ⲿ������������
#define  SYS_BALACELL_MAX    5    // ÿ������Ĵ������Ƶ�����о���� (����CELLBAL1����5��)
/*-------------------------------*/

// ���ѳ�ʱ����
#define  WAKE_TIME_OUT_1S      2   // ���ѳ�ʱ������ֵ����Ӧ��Լ1�� (����500ms�����ڼ���)
/*-------------------------------*/

// ���ؼ�ƫ��У׼ֵ (����ʵ�ʲ�������)
#define  CC_OffEet_ValueN  47   // ���ؼƸ���ƫ��У׼ֵ (ʾ��ֵ)
#define  CC_OffEet_ValueP  60   // ���ؼ�����ƫ��У׼ֵ (ʾ��ֵ)
/*-------------------------------*/

extern u8  VoltCellOffSet; // ��о��ѹADCƫ��ֵ (��ADCOFFSET�Ĵ�����ȡ)
extern u32 VoltCellGainUV ; // ��о��ѹADC����ֵ (��λuV/LSB, ��ADCGAIN1/2�Ĵ�������õ�)
extern u16 VoltCellGainMV; // ��о��ѹADC����ֵ (��λmV/LSB, �����ڽ�����ʾ�����)
extern u8  BQ769xx_Init_State; // BQ769xx��ʼ��״̬��־
extern u8 Bq769xx_Wake_EN; // BQ769xx����ʹ�ܱ�־

// PCB����ر����壩��������ö��
typedef enum _ePCB_SEQ
{
	PCB_SEQ_INIT = 0,	// ��ʼ������
	PCB_SEQ_WAIT,	    // �ȴ�����
	PCB_SEQ_CHK,		// �������
	PCB_SEQ_SELECT,		// ѡ������
	PCB_SEQ_MAP,        // ӳ������ (���籣������)
	PCB_SEQ_SET,		// ��������
	PCB_SEQ_BALA_TIM,	// ����ʱ������
	PCB_SEQ_MAX			// ������к� (��������߽��)
}	ePCB_SEQ;

// BQ769xx �Ĵ�����ṹ�嶨��
typedef struct _Register_Group
{
    // ϵͳ״̬�Ĵ��� (SYS_STAT, 0x00)
    union
    {
            struct
            {
                    u8 OCD		:1; // Bit 0: �����ŵ� (Overcurrent in Discharge)
                    u8 SCD		:1; // Bit 1: ��·�ŵ� (Short Circuit in Discharge)
                    u8 OV		:1; // Bit 2: ��ѹ (Overvoltage)
                    u8 UV		:1; // Bit 3: Ƿѹ (Undervoltage)
                    u8 OVRD_ALERT	:1; // Bit 4: ALERT���ű��ⲿ���� (Override Alert)
                    u8 DEVICE_XREADY	:1; // Bit 5: �豸�ڲ����� (Device XREADY, 1��ʾ����)
                    u8 WAKE		:1; // Bit 6: (BQ769x0�д�λ����RSVD������ԭ��ͼ�������ֲᣬ��λ��WAKE)
                    u8 CC_READY	:1; // Bit 7: ���ؼ�����׼������ (Coulomb Counter Ready)
            }StatusBit;
            u8 StatusByte;
    }SysStatus;

    // ��о����Ĵ���1 (CELLBAL1, 0x01)
    union
    {
            struct
            {
                    u8 RSVD			:3; // Bit 7-5: ���� (Reserved)
                    u8 CB5			:1; // Bit 4: ��о5����ʹ�� (Cell Balance 5)
                    u8 CB4			:1; // Bit 3: ��о4����ʹ��
                    u8 CB3			:1; // Bit 2: ��о3����ʹ��
                    u8 CB2			:1; // Bit 1: ��о2����ʹ��
                    u8 CB1			:1; // Bit 0: ��о1����ʹ��
            }CellBal1Bit;
            u8 CellBal1Byte;
    }CellBal1;

    // ��о����Ĵ���2 (CELLBAL2, 0x02)
    union
    {
            struct
            {
                    u8 RSVD			:3; // Bit 7-5: ����
                    u8 CB10			:1; // Bit 4: ��о10����ʹ��
                    u8 CB9			:1; // Bit 3: ��о9����ʹ��
                    u8 CB8			:1; // Bit 2: ��о8����ʹ��
                    u8 CB7			:1; // Bit 1: ��о7����ʹ��
                    u8 CB6			:1; // Bit 0: ��о6����ʹ��
            }CellBal2Bit;
            u8 CellBal2Byte;
    }CellBal2;

    // ��о����Ĵ���3 (CELLBAL3, 0x03)
    union
    {
            struct
            {
                    u8 RSVD			:3; // Bit 7-5: ����
                    u8 CB15			:1; // Bit 4: ��о15����ʹ��
                    u8 CB14			:1; // Bit 3: ��о14����ʹ��
                    u8 CB13			:1; // Bit 2: ��о13����ʹ��
                    u8 CB12			:1; // Bit 1: ��о12����ʹ��
                    u8 CB11			:1; // Bit 0: ��о11����ʹ��
            }CellBal3Bit;
            u8 CellBal3Byte;
    }CellBal3;

    // ϵͳ���ƼĴ���1 (SYS_CTRL1, 0x04)
    union
    {
            struct
            {
                    u8 SHUT_B		:1; // Bit 0: �ػ�����λ B (Shutdown B, ���ڽ���SHIPģʽ����)
                    u8 SHUT_A		:1; // Bit 1: �ػ�����λ A (Shutdown A, ���ڽ���SHIPģʽ����)
                    u8 RSVD1		:1; // Bit 2: ����
                    u8 TEMP_SEL		:1; // Bit 3: �¶�Դѡ�� (0: �ڲ�Die�¶�, 1: �ⲿ��������TSx)
                    u8 ADC_EN		:1; // Bit 4: ADCʹ�� (0: ����, 1: ʹ��)
                    u8 RSVD2		:2; // Bit 6-5: ����
                    u8 LOAD_PRESENT	:1; // Bit 7: ���ؼ�� (ֻ��, 1: CHG���ż�⵽����)
            }SysCtrl1Bit;
            u8 SysCtrl1Byte;
    }SysCtrl1;

    // ϵͳ���ƼĴ���2 (SYS_CTRL2, 0x05)
    union
    {
            struct
            {
                    u8 CHG_ON		:1; // Bit 0: ���FET����ʹ�� (0: �ر�, 1: ��)
                    u8 DSG_ON		:1; // Bit 1: �ŵ�FET����ʹ�� (0: �ر�, 1: ��)
                    u8 WAKE_T		:2; // Bit 3-2: (BQ769x0�д�λRSVD����WAKE_T)
                    u8 WAKE_EN		:1; // Bit 4: (BQ769x0�д�λRSVD����WAKE_EN)
                    u8 CC_ONESHOT	:1; // Bit 5: ���ؼƵ���ת������ (0: �޶���, 1: ����һ��ת��)
                    u8 CC_EN		:1; // Bit 6: ���ؼ�����ת��ʹ�� (0: ����, 1: ʹ��)
                    u8 DELAY_DIS	:1; // Bit 7: ���ñ�����ʱ (0: ������ʱ, 1: ������ʱ�����ڲ���)
            }SysCtrl2Bit;
            u8 SysCtrl2Byte;
    }SysCtrl2;

    // �����Ĵ���1 (PROTECT1, 0x06)
    union
    {
            struct
            {
                    u8 SCD_THRESH	:3; // Bit 2-0: ��·�ŵ���ֵ
                    u8 SCD_DELAY	:2; // Bit 4-3: ��·�ŵ���ʱ
                    u8 RSVD		:2; // Bit 6-5: ����
                    u8 RSNS		:1; // Bit 7: OCD/SCD������ⷶΧѡ�� (0: �ͷ�Χ, 1: �߷�Χ)
            }Protect1Bit;
            u8 Protect1Byte;
    }Protect1;

    // �����Ĵ���2 (PROTECT2, 0x07)
    union
    {
            struct
            {
                    u8 OCD_THRESH	:4; // Bit 3-0: �����ŵ���ֵ
                    u8 OCD_DELAY	:3; // Bit 6-4: �����ŵ���ʱ
                    u8 RSVD		:1; // Bit 7: ����
            }Protect2Bit;
            u8 Protect2Byte;
    }Protect2;

    // �����Ĵ���3 (PROTECT3, 0x08)
    union
    {
            struct
            {
                    u8 RSVD		:4; // Bit 3-0: ����
                    u8 OV_DELAY		:2; // Bit 5-4: ��ѹ������ʱ
                    u8 UV_DELAY		:2; // Bit 7-6: Ƿѹ������ʱ
            }Protect3Bit;
            u8 Protect3Byte;
    }Protect3;

    u8 OVTrip;       // ��ѹ������ֵ�Ĵ��� (OV_TRIP, 0x09)
    u8 UVTrip;       // Ƿѹ������ֵ�Ĵ��� (UV_TRIP, 0x0A)
    u8 CCCfg;		 // ���ؼ����üĴ��� (CC_CFG, 0x0B)��ͨ������Ϊ0x19

    // ��о1��ѹ�Ĵ��� (VC1_HI: 0x0C, VC1_LO: 0x0D)
    union
    {
            struct
            {
                    u8 VC1_HI; // ��ѹ���ֽ� (D13-D8)
                    u8 VC1_LO; // ��ѹ���ֽ� (D7-D0)
            }VCell1Byte;
            unsigned short VCell1Word; // 16λ��ѹԭʼֵ
    }VCell1;

    // ��о2��ѹ�Ĵ��� (VC2_HI: 0x0E, VC2_LO: 0x0F)
    union
    {
            struct
            {
									u8 VC2_HI;
									u8 VC2_LO;
            }VCell2Byte;
            unsigned short VCell2Word;
    }VCell2;

    // ... (������о��ѹ�Ĵ��� VCell3 �� VCell15 �ṹ����) ...

    union
    {
            struct
            {
                    u8 VC3_HI;
                    u8 VC3_LO;
            }VCell3Byte;
            unsigned short VCell3Word;
    }VCell3;

    union
    {
            struct
            {
                    u8 VC4_HI;
                    u8 VC4_LO;
            }VCell4Byte;
            unsigned short VCell4Word;
    }VCell4;

    union
    {
            struct
            {
                    u8 VC5_HI;
                    u8 VC5_LO;
            }VCell5Byte;
            unsigned short VCell5Word;
    }VCell5;

    union
    {
            struct
            {
                    u8 VC6_HI;
                    u8 VC6_LO;
            }VCell6Byte;
            unsigned short VCell6Word;
    }VCell6;

    union
    {
            struct
            {
                    u8 VC7_HI;
                    u8 VC7_LO;
            }VCell7Byte;
            unsigned short VCell7Word;
    }VCell7;

    union
    {
            struct
            {
                    u8 VC8_HI;
                    u8 VC8_LO;
            }VCell8Byte;
            unsigned short VCell8Word;
    }VCell8;

    union
    {
            struct
            {
                    u8 VC9_HI;
                    u8 VC9_LO;
            }VCell9Byte;
            unsigned short VCell9Word;
    }VCell9;

    union
    {
            struct
            {
                    u8 VC10_HI;
                    u8 VC10_LO;
            }VCell10Byte;
            unsigned short VCell10Word;
    }VCell10;

    union
    {
            struct
            {
                    u8 VC11_HI;
                    u8 VC11_LO;
            }VCell11Byte;
            unsigned short VCell11Word;
    }VCell11;

    union
    {
            struct
            {
                    u8 VC12_HI;
                    u8 VC12_LO;
            }VCell12Byte;
            unsigned short VCell12Word;
    }VCell12;

    union
    {
            struct
            {
                    u8 VC13_HI;
                    u8 VC13_LO;
            }VCell13Byte;
            unsigned short VCell13Word;
    }VCell13;

    union
    {
            struct
            {
                    u8 VC14_HI;
                    u8 VC14_LO;
            }VCell14Byte;
            unsigned short VCell14Word;
    }VCell14;

    union
    {
            struct
            {
                    u8 VC15_HI;
                    u8 VC15_LO;
            }VCell15Byte;
            unsigned short VCell15Word;
    }VCell15;

    // ��ذ��ܵ�ѹ�Ĵ��� (BAT_HI: 0x2A, BAT_LO: 0x2B)
    union
    {
            struct
            {
                    u8 BAT_HI; // �ܵ�ѹ���ֽ� (D15-D8)
                    u8 BAT_LO; // �ܵ�ѹ���ֽ� (D7-D0)
            }VBatByte;
            unsigned short VBatWord; // 16λ�ܵ�ѹԭʼֵ
    }VBat;

    // �¶ȴ�����1�Ĵ��� (TS1_HI: 0x2C, TS1_LO: 0x2D)
    union
    {
            struct
            {
                    u8 TS1_HI; // �¶�1���ֽ� (D13-D8)
                    u8 TS1_LO; // �¶�1���ֽ� (D7-D0)
            }TS1Byte;
            unsigned short TS1Word; // 16λ�¶�1ԭʼֵ
    }TS1;

    // �¶ȴ�����2�Ĵ��� (TS2_HI: 0x2E, TS2_LO: 0x2F)
    union
    {
            struct
            {
                    u8 TS2_HI;
                    u8 TS2_LO;
            }TS2Byte;
            unsigned short TS2Word;
    }TS2;

    // �¶ȴ�����3�Ĵ��� (TS3_HI: 0x30, TS3_LO: 0x31)
    union
    {
            struct
            {
                    u8 TS3_HI;
                    u8 TS3_LO;
            }TS3Byte;
            unsigned short TS3Word;
    }TS3;

    // ���ؼƼĴ��� (CC_HI: 0x32, CC_LO: 0x33)
    union
    {
            struct
            {
                    u8 CC_HI; // ���ؼƸ��ֽ� (D15-D8)
                    u8 CC_LO; // ���ؼƵ��ֽ� (D7-D0)
            }CCByte;
            signed short CCWord; // 16λ���ؼ�ԭʼֵ (�з���)
    }CC;

    // ADC����Ĵ���1 (ADCGAIN1, 0x50)
    union
    {
            struct
            {
                    u8 RSVD1		:2; // Bit 1-0: ����
                    u8 ADCGAIN_4_3	:2; // Bit 3-2: ADC����λ[4:3]
                    u8 RSVD2		:4; // Bit 7-4: ����
            }ADCGain1Bit;
            u8 ADCGain1Byte;
    }ADCGain1;

    u8 ADCOffset; // ADCƫ�ƼĴ��� (ADCOFFSET, 0x51)

    // ADC����Ĵ���2 (ADCGAIN2, 0x59)
    union
    {
            struct
            {
                    u8 RSVD		:5; // Bit 4-0: ����
                    u8 ADCGAIN_2_0	:3; // Bit 7-5: ADC����λ[2:0]
            }ADCGain2Bit;
            u8 ADCGain2Byte;
    }ADCGain2;
}RegisterGroup;
extern  RegisterGroup  Bq769xxReg; // BQ769xx�Ĵ�����ȫ�ֱ���

// ��������
void  BQ769xx_Wake(void); // ����BQ769xxоƬ (ͨ��TS1����)
void BQ769xx_Init(void); // ��ʼ��BQ769xxоƬ
void BQ769xx_GetData(void); // ��BQ769xx��ȡ���� (��ѹ���¶ȡ�������)
void BQ769xx_GetConfig(void); // ��BQ769xx��ȡ���ò��� (���桢ƫ�Ƶ�)
u8 BQ769xx_DSGSET(u8 ONOFF); // ���÷ŵ�FET״̬ (ON/OFF)
u8 BQ769xx_CHGSET(u8 ONOFF); // ���ó��FET״̬ (ON/OFF)
u8 BQ769xx_STATE_GET(void); // ��ȡBQ769xx��״̬
u8 BQ769xx_CC_ONESHOT_SET(u8 ONOFF); // �������ؼƵ���ת��
u16  LimitMaxMin(u16 InValue,u16 gMax,u16 gMin); // ��������ֵ�������Сֵ֮��
u8 BQ76940_STAT_CLEAR(u8 statvalue); // ���BQ769xx��״̬��־λ
u8 Enter_Ship_Mode(void); // ����SHIPģʽ (���͹���ģʽ)
u8 Balan_Pack_Check(void); // ����ذ�����״̬
void BQ769xx_Oper_Comm(void); // BQ769xx����ͨ�� (��ѭ���д���)
void Power_Down(void); // ϵͳ�ϵ紦�� (���������SHIPģʽ���)
void BQ769xx_Wake_Comm(void); // BQ769xx���Ѻ�ͨ�Ŵ���

#endif
