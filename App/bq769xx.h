#ifndef __BQ769XX_H
#define __BQ769XX_H
#include "sys.h" // 包含系统级头文件

#define BQ769xxAddr   0x08 // BQ769xx系列芯片的I2C从机地址
#define SYS_STAT_RegAddr   0x00 // 系统状态寄存器地址
#define CELLBAL1_RegAddr   0x01 // 电芯均衡寄存器1地址 (控制VC1-VC5)
#define CELLBAL2_RegAddr   0x02 // 电芯均衡寄存器2地址 (控制VC6-VC10)
#define CELLBAL3_RegAddr   0x03 // 电芯均衡寄存器3地址 (控制VC11-VC15)
#define SYS_CTRL1_RegAddr  0x04 // 系统控制寄存器1地址
#define SYS_CTRL2_RegAddr  0x05 // 系统控制寄存器2地址
#define PROTECT1_RegAddr   0x06 // 保护寄存器1地址 (SCD相关)
#define PROTECT2_RegAddr   0x07 // 保护寄存器2地址 (OCD相关)
#define PROTECT3_RegAddr   0x08 // 保护寄存器3地址 (UV/OV延时相关)
#define OV_TRIP_RegAddr    0x09 // 过压保护阈值寄存器地址
#define UV_TRIP_RegAddr    0x0A // 欠压保护阈值寄存器地址
#define VC1_HI_RegAddr     0x0C // 第1节电芯电压高字节寄存器地址 (后续VCx_HI/LO依次递增)
#define ADCGAIN1_RegAddr   0x50 // ADC增益寄存器1地址
#define ADCOFFSET_RegAddr 0x51 // ADC偏移寄存器地址
#define ADCGAIN2_RegAddr   0x59 // ADC增益寄存器2地址

// SYS_CTRL1 寄存器相关位定义
#define   ADC_ENS     1  // ADC使能 (用于SYS_CTRL1寄存器的ADC_EN位)
#define   ADC_DIS    0  // ADC禁用 (用于SYS_CTRL1寄存器的ADC_EN位)
#define   EXTE_TEMP_ON     1  // 选择外部热敏电阻测量 (用于SYS_CTRL1寄存器的TEMP_SEL位)
#define   EXTE_TEMP_OFF    0  // 选择内部芯片温度测量 (用于SYS_CTRL1寄存器的TEMP_SEL位)

// SYS_CTRL2 寄存器相关位定义
#define   ALARM_DELAY_ON   0  // 保护延时正常 (用于SYS_CTRL2寄存器的DELAY_DIS位)
#define   ALARM_DELAY_OFF  1  // 禁用保护延时 (用于SYS_CTRL2寄存器的DELAY_DIS位，用于测试)
#define   CC_CONTINU_DIS  0  // 禁用库仑计连续转换 (用于SYS_CTRL2寄存器的CC_EN位)
#define   CC_CONTINU_EN   1  // 使能库仑计连续转换 (用于SYS_CTRL2寄存器的CC_EN位)
#define   CHG_O0N    1  // 打开充电FET (用于SYS_CTRL2寄存器的CHG_ON位)
#define   CHG_OFF   0  // 关闭充电FET (用于SYS_CTRL2寄存器的CHG_ON位)
#define   DSG_O0N    1  // 打开放电FET (用于SYS_CTRL2寄存器的DSG_ON位)
#define   DSG_OFF   0  // 关闭放电FET (用于SYS_CTRL2寄存器的DSG_ON位)

// PROTECT1 寄存器相关位定义
#define OCD_SCD_LOWER_RANGE   0  // OCD和SCD阈值使用较低输入范围 (用于PROTECT1寄存器的RSNS位)
#define OCD_SCD_HIGH_RANGE    1  // OCD和SCD阈值使用较高输入范围 (用于PROTECT1寄存器的RSNS位)

// 过压/欠压保护阈值相关定义
#define OV_THRESH_BASE	      0x2008 // 过压阈值ADC原始值的基准 (对应10xxxx xxxx1000)
#define UV_THRESH_BASE	      0x1000 // 欠压阈值ADC原始值的基准 (对应01xxxx xxxx0000)
#define OV_THRESH_MAX         4700   // 过压保护阈值最大值 (单位: mV, 近似值)
#define OV_THRESH_MIN         3150   // 过压保护阈值最小值 (单位: mV, 近似值)
#define UV_THRESH_MAX         3100   // 欠压保护阈值最大值 (单位: mV, 近似值)
#define UV_THRESH_MIN         1580   // 欠压保护阈值最小值 (单位: mV, 近似值)

// ADC转换LSB值定义
#define BATVOLTLSB            1532 // 电池包总电压ADC的LSB (单位: uV, 实际计算需要乘以4)
#define TMEPVOLTLSB           382  // 温度和电芯电压ADC的LSB (单位: uV)
#define CRUUVOLTLSB           844  // 库仑计ADC的LSB (单位: uV, 实际是8.44uV)

/*---------清除警报标志位-------*/
#define OCD_CLE      0x01 // 清除过流放电(OCD)警报标志位
#define SCD_CLE      0x02 // 清除短路放电(SCD)警报标志位
#define OV_CLE       0x04 // 清除过压(OV)警报标志位
#define UV_CLE       0x08 // 清除欠压(UV)警报标志位
#define OVRD_ALERT_CLE      0x10 // 清除外部ALERT覆盖警报标志位
#define DEVICE_XREADY_CLE   0x20 // 清除设备内部错误警报标志位
#define CC_READY_CLE   0x80 // 清除库仑计准备就绪警报标志位
/*-------------------------------*/

// 系统参数定义
#define  SYS_CELL_MAX       15    // 系统支持的最大电芯串数
#define  SYS_TEMPEXT_MAX     3    // 系统支持的最大外部热敏电阻数量
#define  SYS_BALACELL_MAX    5    // 每个均衡寄存器控制的最大电芯数量 (例如CELLBAL1控制5个)
/*-------------------------------*/

// 唤醒超时定义
#define  WAKE_TIME_OUT_1S      2   // 唤醒超时计数器值，对应大约1秒 (基于500ms的周期计数)
/*-------------------------------*/

// 库仑计偏移校准值 (根据实际测量调整)
#define  CC_OffEet_ValueN  47   // 库仑计负向偏移校准值 (示例值)
#define  CC_OffEet_ValueP  60   // 库仑计正向偏移校准值 (示例值)
/*-------------------------------*/

extern u8  VoltCellOffSet; // 电芯电压ADC偏移值 (由ADCOFFSET寄存器读取)
extern u32 VoltCellGainUV ; // 电芯电压ADC增益值 (单位uV/LSB, 由ADCGAIN1/2寄存器计算得到)
extern u16 VoltCellGainMV; // 电芯电压ADC增益值 (单位mV/LSB, 仅用于近似显示或计算)
extern u8  BQ769xx_Init_State; // BQ769xx初始化状态标志
extern u8 Bq769xx_Wake_EN; // BQ769xx唤醒使能标志

// PCB（电池保护板）操作序列枚举
typedef enum _ePCB_SEQ
{
	PCB_SEQ_INIT = 0,	// 初始化序列
	PCB_SEQ_WAIT,	    // 等待序列
	PCB_SEQ_CHK,		// 检查序列
	PCB_SEQ_SELECT,		// 选择序列
	PCB_SEQ_MAP,        // 映射序列 (例如保护参数)
	PCB_SEQ_SET,		// 设置序列
	PCB_SEQ_BALA_TIM,	// 均衡时间序列
	PCB_SEQ_MAX			// 最大序列号 (用于数组边界等)
}	ePCB_SEQ;

// BQ769xx 寄存器组结构体定义
typedef struct _Register_Group
{
    // 系统状态寄存器 (SYS_STAT, 0x00)
    union
    {
            struct
            {
                    u8 OCD		:1; // Bit 0: 过流放电 (Overcurrent in Discharge)
                    u8 SCD		:1; // Bit 1: 短路放电 (Short Circuit in Discharge)
                    u8 OV		:1; // Bit 2: 过压 (Overvoltage)
                    u8 UV		:1; // Bit 3: 欠压 (Undervoltage)
                    u8 OVRD_ALERT	:1; // Bit 4: ALERT引脚被外部拉高 (Override Alert)
                    u8 DEVICE_XREADY	:1; // Bit 5: 设备内部错误 (Device XREADY, 1表示错误)
                    u8 WAKE		:1; // Bit 6: (BQ769x0中此位保留RSVD，根据原理图和数据手册，此位非WAKE)
                    u8 CC_READY	:1; // Bit 7: 库仑计数据准备就绪 (Coulomb Counter Ready)
            }StatusBit;
            u8 StatusByte;
    }SysStatus;

    // 电芯均衡寄存器1 (CELLBAL1, 0x01)
    union
    {
            struct
            {
                    u8 RSVD			:3; // Bit 7-5: 保留 (Reserved)
                    u8 CB5			:1; // Bit 4: 电芯5均衡使能 (Cell Balance 5)
                    u8 CB4			:1; // Bit 3: 电芯4均衡使能
                    u8 CB3			:1; // Bit 2: 电芯3均衡使能
                    u8 CB2			:1; // Bit 1: 电芯2均衡使能
                    u8 CB1			:1; // Bit 0: 电芯1均衡使能
            }CellBal1Bit;
            u8 CellBal1Byte;
    }CellBal1;

    // 电芯均衡寄存器2 (CELLBAL2, 0x02)
    union
    {
            struct
            {
                    u8 RSVD			:3; // Bit 7-5: 保留
                    u8 CB10			:1; // Bit 4: 电芯10均衡使能
                    u8 CB9			:1; // Bit 3: 电芯9均衡使能
                    u8 CB8			:1; // Bit 2: 电芯8均衡使能
                    u8 CB7			:1; // Bit 1: 电芯7均衡使能
                    u8 CB6			:1; // Bit 0: 电芯6均衡使能
            }CellBal2Bit;
            u8 CellBal2Byte;
    }CellBal2;

    // 电芯均衡寄存器3 (CELLBAL3, 0x03)
    union
    {
            struct
            {
                    u8 RSVD			:3; // Bit 7-5: 保留
                    u8 CB15			:1; // Bit 4: 电芯15均衡使能
                    u8 CB14			:1; // Bit 3: 电芯14均衡使能
                    u8 CB13			:1; // Bit 2: 电芯13均衡使能
                    u8 CB12			:1; // Bit 1: 电芯12均衡使能
                    u8 CB11			:1; // Bit 0: 电芯11均衡使能
            }CellBal3Bit;
            u8 CellBal3Byte;
    }CellBal3;

    // 系统控制寄存器1 (SYS_CTRL1, 0x04)
    union
    {
            struct
            {
                    u8 SHUT_B		:1; // Bit 0: 关机命令位 B (Shutdown B, 用于进入SHIP模式序列)
                    u8 SHUT_A		:1; // Bit 1: 关机命令位 A (Shutdown A, 用于进入SHIP模式序列)
                    u8 RSVD1		:1; // Bit 2: 保留
                    u8 TEMP_SEL		:1; // Bit 3: 温度源选择 (0: 内部Die温度, 1: 外部热敏电阻TSx)
                    u8 ADC_EN		:1; // Bit 4: ADC使能 (0: 禁用, 1: 使能)
                    u8 RSVD2		:2; // Bit 6-5: 保留
                    u8 LOAD_PRESENT	:1; // Bit 7: 负载检测 (只读, 1: CHG引脚检测到负载)
            }SysCtrl1Bit;
            u8 SysCtrl1Byte;
    }SysCtrl1;

    // 系统控制寄存器2 (SYS_CTRL2, 0x05)
    union
    {
            struct
            {
                    u8 CHG_ON		:1; // Bit 0: 充电FET驱动使能 (0: 关闭, 1: 打开)
                    u8 DSG_ON		:1; // Bit 1: 放电FET驱动使能 (0: 关闭, 1: 打开)
                    u8 WAKE_T		:2; // Bit 3-2: (BQ769x0中此位RSVD，非WAKE_T)
                    u8 WAKE_EN		:1; // Bit 4: (BQ769x0中此位RSVD，非WAKE_EN)
                    u8 CC_ONESHOT	:1; // Bit 5: 库仑计单次转换触发 (0: 无动作, 1: 触发一次转换)
                    u8 CC_EN		:1; // Bit 6: 库仑计连续转换使能 (0: 禁用, 1: 使能)
                    u8 DELAY_DIS	:1; // Bit 7: 禁用保护延时 (0: 正常延时, 1: 禁用延时，用于测试)
            }SysCtrl2Bit;
            u8 SysCtrl2Byte;
    }SysCtrl2;

    // 保护寄存器1 (PROTECT1, 0x06)
    union
    {
            struct
            {
                    u8 SCD_THRESH	:3; // Bit 2-0: 短路放电阈值
                    u8 SCD_DELAY	:2; // Bit 4-3: 短路放电延时
                    u8 RSVD		:2; // Bit 6-5: 保留
                    u8 RSNS		:1; // Bit 7: OCD/SCD电流检测范围选择 (0: 低范围, 1: 高范围)
            }Protect1Bit;
            u8 Protect1Byte;
    }Protect1;

    // 保护寄存器2 (PROTECT2, 0x07)
    union
    {
            struct
            {
                    u8 OCD_THRESH	:4; // Bit 3-0: 过流放电阈值
                    u8 OCD_DELAY	:3; // Bit 6-4: 过流放电延时
                    u8 RSVD		:1; // Bit 7: 保留
            }Protect2Bit;
            u8 Protect2Byte;
    }Protect2;

    // 保护寄存器3 (PROTECT3, 0x08)
    union
    {
            struct
            {
                    u8 RSVD		:4; // Bit 3-0: 保留
                    u8 OV_DELAY		:2; // Bit 5-4: 过压保护延时
                    u8 UV_DELAY		:2; // Bit 7-6: 欠压保护延时
            }Protect3Bit;
            u8 Protect3Byte;
    }Protect3;

    u8 OVTrip;       // 过压保护阈值寄存器 (OV_TRIP, 0x09)
    u8 UVTrip;       // 欠压保护阈值寄存器 (UV_TRIP, 0x0A)
    u8 CCCfg;		 // 库仑计配置寄存器 (CC_CFG, 0x0B)，通常设置为0x19

    // 电芯1电压寄存器 (VC1_HI: 0x0C, VC1_LO: 0x0D)
    union
    {
            struct
            {
                    u8 VC1_HI; // 电压高字节 (D13-D8)
                    u8 VC1_LO; // 电压低字节 (D7-D0)
            }VCell1Byte;
            unsigned short VCell1Word; // 16位电压原始值
    }VCell1;

    // 电芯2电压寄存器 (VC2_HI: 0x0E, VC2_LO: 0x0F)
    union
    {
            struct
            {
									u8 VC2_HI;
									u8 VC2_LO;
            }VCell2Byte;
            unsigned short VCell2Word;
    }VCell2;

    // ... (后续电芯电压寄存器 VCell3 到 VCell15 结构类似) ...

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

    // 电池包总电压寄存器 (BAT_HI: 0x2A, BAT_LO: 0x2B)
    union
    {
            struct
            {
                    u8 BAT_HI; // 总电压高字节 (D15-D8)
                    u8 BAT_LO; // 总电压低字节 (D7-D0)
            }VBatByte;
            unsigned short VBatWord; // 16位总电压原始值
    }VBat;

    // 温度传感器1寄存器 (TS1_HI: 0x2C, TS1_LO: 0x2D)
    union
    {
            struct
            {
                    u8 TS1_HI; // 温度1高字节 (D13-D8)
                    u8 TS1_LO; // 温度1低字节 (D7-D0)
            }TS1Byte;
            unsigned short TS1Word; // 16位温度1原始值
    }TS1;

    // 温度传感器2寄存器 (TS2_HI: 0x2E, TS2_LO: 0x2F)
    union
    {
            struct
            {
                    u8 TS2_HI;
                    u8 TS2_LO;
            }TS2Byte;
            unsigned short TS2Word;
    }TS2;

    // 温度传感器3寄存器 (TS3_HI: 0x30, TS3_LO: 0x31)
    union
    {
            struct
            {
                    u8 TS3_HI;
                    u8 TS3_LO;
            }TS3Byte;
            unsigned short TS3Word;
    }TS3;

    // 库仑计寄存器 (CC_HI: 0x32, CC_LO: 0x33)
    union
    {
            struct
            {
                    u8 CC_HI; // 库仑计高字节 (D15-D8)
                    u8 CC_LO; // 库仑计低字节 (D7-D0)
            }CCByte;
            signed short CCWord; // 16位库仑计原始值 (有符号)
    }CC;

    // ADC增益寄存器1 (ADCGAIN1, 0x50)
    union
    {
            struct
            {
                    u8 RSVD1		:2; // Bit 1-0: 保留
                    u8 ADCGAIN_4_3	:2; // Bit 3-2: ADC增益位[4:3]
                    u8 RSVD2		:4; // Bit 7-4: 保留
            }ADCGain1Bit;
            u8 ADCGain1Byte;
    }ADCGain1;

    u8 ADCOffset; // ADC偏移寄存器 (ADCOFFSET, 0x51)

    // ADC增益寄存器2 (ADCGAIN2, 0x59)
    union
    {
            struct
            {
                    u8 RSVD		:5; // Bit 4-0: 保留
                    u8 ADCGAIN_2_0	:3; // Bit 7-5: ADC增益位[2:0]
            }ADCGain2Bit;
            u8 ADCGain2Byte;
    }ADCGain2;
}RegisterGroup;
extern  RegisterGroup  Bq769xxReg; // BQ769xx寄存器组全局变量

// 函数声明
void  BQ769xx_Wake(void); // 唤醒BQ769xx芯片 (通过TS1引脚)
void BQ769xx_Init(void); // 初始化BQ769xx芯片
void BQ769xx_GetData(void); // 从BQ769xx获取数据 (电压、温度、电流等)
void BQ769xx_GetConfig(void); // 从BQ769xx获取配置参数 (增益、偏移等)
u8 BQ769xx_DSGSET(u8 ONOFF); // 设置放电FET状态 (ON/OFF)
u8 BQ769xx_CHGSET(u8 ONOFF); // 设置充电FET状态 (ON/OFF)
u8 BQ769xx_STATE_GET(void); // 获取BQ769xx的状态
u8 BQ769xx_CC_ONESHOT_SET(u8 ONOFF); // 触发库仑计单次转换
u16  LimitMaxMin(u16 InValue,u16 gMax,u16 gMin); // 限制输入值在最大最小值之间
u8 BQ76940_STAT_CLEAR(u8 statvalue); // 清除BQ769xx的状态标志位
u8 Enter_Ship_Mode(void); // 进入SHIP模式 (超低功耗模式)
u8 Balan_Pack_Check(void); // 检查电池包均衡状态
void BQ769xx_Oper_Comm(void); // BQ769xx操作通信 (主循环中处理)
void Power_Down(void); // 系统断电处理 (可能与进入SHIP模式相关)
void BQ769xx_Wake_Comm(void); // BQ769xx唤醒后通信处理

#endif
