#include "bq769xx.h"     // 包含 BQ769xx 系列芯片驱动的头文件
#include "DataBase.h"    // 包含数据库（可能存储BMS实时数据、配置等）的头文件
#include "ConfigPara.h"  // 包含配置参数的头文件
#include "iic.h"         // 包含 I2C 通信协议的头文件
#include "timer.h"       // 包含定时器功能的头文件
#include "adc.h"         // 包含 ADC (模数转换器) 功能的头文件
#include "Rs485Data.h"   // 包含 RS485 通信数据的头文件
#include "gpio.h"        // 包含 GPIO (通用输入输出) 控制的头文件
#include<string.h>       // 包含标准字符串处理函数的头文件 (如 memcpy)


//-------------全局变量-------------//
RegisterGroup  Bq769xxReg;      // BQ769xx 芯片所有寄存器的结构体变量，用于存储和操作寄存器值
u8  VoltCellOffSet=0;         // 电芯电压ADC原始值的偏移量 (从芯片读取)
u16 VoltCellGainMV = 0;       // 电芯电压ADC增益，单位: mV/LSB (实际是 μV/LSB / 1000)
u32 VoltCellGainUV = 0;       // 电芯电压ADC增益，单位: μV/LSB (从芯片读取并计算得到)
u8  BQ769xx_Init_State=0;     // BQ769xx 芯片初始化状态标志 (0: 未初始化或失败, 1: 初始化成功)
u8  Bq769xx_Wake_EN=0;        // BQ769xx 芯片唤醒使能标志 (1: 使能唤醒序列)
u8  BQ769xxGatherData[40]={0}; // 用于存储从BQ769xx芯片一次性读取的批量数据 (如电压、温度等)
u16 VoltageTemp[3]={0};       // 存储外部温度传感器转换前的电压值 (最多3个)


//------------------------------------//

//-------------局部变量-------------//
u8 bq769xx_Wake_cont=0;       // BQ769xx 唤醒序列计数器，用于控制唤醒脉冲的时长
//------------------------------------//

// 函数声明：静态函数，仅在本文件内可见
static void pack_temp_max_min(void); // 声明一个函数，用于计算电池包中温度的最大值和最小值
static void pack_cell_max_min(void); // 声明一个函数，用于计算电池包中电芯电压的最大值和最小值

/**
 * @brief  唤醒 BQ769xx 芯片
 * @param  无
 * @retval 无
 * @detail 通过向 BQ769xx 的 TS1/BOOT 引脚发送一个特定时长的脉冲来唤醒芯片
 */
void  BQ769xx_Wake(void)
{
		//--------激活-------------------//
		BQ769xx_WAKE_ON;	  // 宏定义，通常是拉高TS1/BOOT引脚
		delay_ms(1000);		  // 延时1000毫秒，保持唤醒信号
		BQ769xx_WAKE_OFF;	  // 宏定义，通常是拉低TS1/BOOT引脚
}

/**
 * @brief  设置 BQ769xx 芯片的电芯连接映射
 * @param  无
 * @retval 无
 * @detail 根据配置的串联电芯数量 (gBMSConfig.Type.CellNum_Ser)，
 *         设置 gBMSData.BattPar.Cells_Map 的值。
 *         Cells_Map 是一个位掩码，表示哪些 BQ769xx 的电压采样通道被使用了。
 */
void  BQ769xx_Set_CellMap(void)
{
   switch(gBMSConfig.Type.CellNum_Ser) // 根据全局配置中的串联电芯数量
   {
	 case 15: // 15串
        gBMSData.BattPar.Cells_Map=0x7FFF; // 对应 BQ76940, 所有15个电芯输入都使用
        break;
     case 14: // 14串
        gBMSData.BattPar.Cells_Map=0x5FFF; // 可能跳过 VC5X 或 VC10X 中的一个
        break;
	 case 13:  // 13串
        gBMSData.BattPar.Cells_Map=0x5EFF; // 可能跳过 VC5X 和 VC10X
        break;
	 case 12:  // 12串
        gBMSData.BattPar.Cells_Map=0x5EF7;
        break;
	 case 11:  // 11串
        gBMSData.BattPar.Cells_Map=0x4EF7;
        break;
     case 10: // 10串
        gBMSData.BattPar.Cells_Map=0x4E77; // 对应 BQ76930, 所有10个电芯输入都使用
        break;
     case 9:  // 9串
        gBMSData.BattPar.Cells_Map=0x4E73;
        break;
      default: // 默认15串 (或最大支持串数)
        gBMSData.BattPar.Cells_Map=0x7FFF;
        break;
   }
}

/**
 * @brief  初始化 BQ769xx 芯片的参数到RAM
 * @param  无
 * @retval 无
 * @detail 此函数在 RAM 中设置 Bq769xxReg 结构体中的各个寄存器位，
 *         这些值稍后会被写入到 BQ769xx 芯片中。
 */
void BQ769xx_Init_Para(void)
{
      // 系统控制寄存器1 (SYS_CTRL1)
      Bq769xxReg.SysCtrl1.SysCtrl1Bit.ADC_EN=ADC_ENS;          // ADC_EN 位: 使能ADC (通常设为1)
      Bq769xxReg.SysCtrl1.SysCtrl1Bit.TEMP_SEL=EXTE_TEMP_ON;    // TEMP_SEL 位: 选择外部热敏电阻TS1进行温度测量

      // 系统控制寄存器2 (SYS_CTRL2)
      Bq769xxReg.SysCtrl2.SysCtrl2Bit.DELAY_DIS=ALARM_DELAY_ON; // DELAY_DIS 位: 0=使能保护延迟, 1=禁用保护延迟 (此处宏定义可能表示使能)
      //Bq769xxReg.SysCtrl2.SysCtrl2Bit.CC_EN=CC_CONTINU_DIS;     // CC_EN 位: 0=禁用库仑计 (旧注释)
	  Bq769xxReg.SysCtrl2.SysCtrl2Bit.CC_EN=CC_CONTINU_EN;       // CC_EN 位: 1=使能库仑计 (连续模式)
      Bq769xxReg.SysCtrl2.SysCtrl2Bit.CHG_ON=CHG_OFF;          // CHG_ON 位: 0=关闭充电FET (初始状态)
      Bq769xxReg.SysCtrl2.SysCtrl2Bit.DSG_ON=DSG_OFF;          // DSG_ON 位: 0=关闭放电FET (初始状态)

      // 保护寄存器1 (PROTECT1)
      Bq769xxReg.Protect1.Protect1Bit.RSNS=OCD_SCD_HIGH_RANGE;  // RSNS 位: 1=SCD/OCD检测使用高电流范围
      Bq769xxReg.Protect1.Protect1Bit.SCD_DELAY = gBMSConfig.BQ76Para.SCD_Delay;   // SCD_DELAY 位: 设置短路保护延迟时间

      // 保护寄存器2 (PROTECT2)
      Bq769xxReg.Protect2.Protect2Bit.OCD_DELAY = gBMSConfig.BQ76Para.OCD_Delay;   // OCD_DELAY 位: 设置过流保护延迟时间

      // 保护寄存器1 (PROTECT1) - 继续
      Bq769xxReg.Protect1.Protect1Bit.SCD_THRESH =gBMSConfig.BQ76Para.SCD_Thresh; // SCD_THRESH 位: 设置短路保护阈值

      // 保护寄存器2 (PROTECT2) - 继续
      Bq769xxReg.Protect2.Protect2Bit.OCD_THRESH = gBMSConfig.BQ76Para.OCD_Thresh; // OCD_THRESH 位: 设置过流保护阈值

      // 保护寄存器3 (PROTECT3)
      Bq769xxReg.Protect3.Protect3Bit.OV_DELAY = gBMSConfig.BQ76Para.OV_Delay;     // OV_DELAY 位: 设置过压保护延迟时间
      Bq769xxReg.Protect3.Protect3Bit.UV_DELAY = gBMSConfig.BQ76Para.UV_Delay;     // UV_DELAY 位: 设置欠压保护延迟时间
}

/**
 * @brief  读取 BQ769xx 芯片的 ADC 增益和偏移校准值
 * @param  无
 * @retval 无
 * @detail 从芯片读取 ADCGAIN1, ADCGAIN2, ADCOFFSET 寄存器，
 *         并根据这些值计算实际的电压增益和偏移。
 *         然后根据配置的过压/欠压阈值和校准值，计算写入 OVTrip 和 UVTrip 寄存器的值。
 */
void BQ769xx_Read_Gain(void)
{
      u8  rcd=0; // 用于存储I2C操作的返回值
      // 通过I2C读取ADC增益和偏移寄存器的值
      rcd= IIC_ReadByte(BQ769xxAddr, ADCGAIN1_RegAddr, &Bq769xxReg.ADCGain1.ADCGain1Byte  ); // 读取ADCGAIN1寄存器
      rcd= IIC_ReadByte(BQ769xxAddr, ADCGAIN2_RegAddr,  &Bq769xxReg.ADCGain2.ADCGain2Byte ); // 读取ADCGAIN2寄存器
      rcd= IIC_ReadByte(BQ769xxAddr, ADCOFFSET_RegAddr, &Bq769xxReg.ADCOffset);             // 读取ADCOFFSET寄存器

      // 根据BQ769xx datasheet中的公式计算电压增益 (单位: μV/LSB)
      VoltCellGainUV = (365 + ((Bq769xxReg.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Bq769xxReg.ADCGain2.ADCGain2Byte & 0xE0)>> 5));
      // 将μV/LSB转换为mV/LSB (近似值)
      VoltCellGainMV = VoltCellGainUV / 1000;
      // 电压偏移量 (单位: LSB, 1 LSB = 1 ADCOFFSET单位)
      VoltCellOffSet = Bq769xxReg.ADCOffset;

      // 限制配置的过压阈值和欠压阈值在允许的范围内
      gBMSConfig.BQ76Para.OV_Thresh=LimitMaxMin(gBMSConfig.BQ76Para.OV_Thresh,OV_THRESH_MAX,OV_THRESH_MIN);
      gBMSConfig.BQ76Para.UV_Thresh=LimitMaxMin(gBMSConfig.BQ76Para.UV_Thresh,UV_THRESH_MAX,UV_THRESH_MIN);

      // 计算写入 OVTrip 寄存器的值 (过压阈值)
      // 例如目标过压值 4180mV
      Bq769xxReg.OVTrip = (u8)(((((unsigned long)(gBMSConfig.BQ76Para.OV_Thresh - Bq769xxReg.ADCOffset)*1000)/VoltCellGainUV - OV_THRESH_BASE)>>4)&0xff);
      // 计算写入 UVTrip 寄存器的值 (欠压阈值)
      // 例如目标欠压值 3200mV
      Bq769xxReg.UVTrip = (u8)(((((unsigned long)(gBMSConfig.BQ76Para.UV_Thresh - Bq769xxReg.ADCOffset)*1000)/VoltCellGainUV - UV_THRESH_BASE)>>4)&0xff);

      // CC_CFG 库仑计配置寄存器，0x19 是一个常用配置
      Bq769xxReg.CCCfg=0x19;

	  (void) rcd; // 避免编译器对未使用变量rcd的警告
}

/**
 * @brief  将RAM中准备好的配置参数写入BQ769xx芯片
 * @param  无
 * @retval u8 - gRET_OK (0) 表示成功, gRET_NG (1) 表示失败
 * @detail 将 Bq769xxReg 结构体中系统控制和保护相关的配置写入芯片。
 *         写入后会回读这些寄存器进行校验，如果校验失败会重试最多5次。
 */
u8 BQ769xx_Write_Config(void)
{
     u8  ret=0;      // 函数返回值，默认为0 (成功)
     u8  rcd=0;      // I2C操作返回值
     u8  Count=0;    // 重试计数器
     u8  err=0;      // 错误标志，0表示无错误
     u8  bqSysCtrProtectionConfig[11]={0};  // 用于存储回读的寄存器值

     do{
         // 将Bq769xxReg中的系统控制和保护参数 (共8个字节) 写入BQ769xx芯片
         rcd=IIC_WritByteMore(BQ769xxAddr, SYS_CTRL1_RegAddr, &(Bq769xxReg.SysCtrl1.SysCtrl1Byte), 8);    // 写入配置寄存器
         // 从BQ769xx芯片回读刚才写入的8个字节，用于校验
         rcd =IIC_ReadByteMore(BQ769xxAddr, SYS_CTRL1_RegAddr, bqSysCtrProtectionConfig, 8);

         // 下面两行被注释掉了，原意可能是写入和校验均衡寄存器
         // rcd=IIC_WritByteMore(BQ769xxAddr, CELLBAL1_RegAddr, &(Bq769xxReg.CellBal1.CellBal1Byte), 11);
         // rcd =IIC_ReadByteMore(BQ769xxAddr, CELLBAL1_RegAddr, bqSysCtrProtectionConfig, 11);

         err = 0; // 重置错误标志
         // 比较写入的值和回读的值是否一致
         if(bqSysCtrProtectionConfig[0]!=Bq769xxReg.SysCtrl1.SysCtrl1Byte || // SYS_CTRL1
            bqSysCtrProtectionConfig[1]!=Bq769xxReg.SysCtrl2.SysCtrl2Byte || // SYS_CTRL2
            bqSysCtrProtectionConfig[2] != Bq769xxReg.Protect1.Protect1Byte|| // PROTECT1
            bqSysCtrProtectionConfig[3] != Bq769xxReg.Protect2.Protect2Byte|| // PROTECT2
            bqSysCtrProtectionConfig[4] != Bq769xxReg.Protect3.Protect3Byte|| // PROTECT3
            bqSysCtrProtectionConfig[5] != Bq769xxReg.OVTrip                || // OV_TRIP
            bqSysCtrProtectionConfig[6] != Bq769xxReg.UVTrip                || // UV_TRIP
            bqSysCtrProtectionConfig[7] != Bq769xxReg.CCCfg )                // CC_CFG
          {
              err++; // 如果不一致，错误标志置位
          }
         Count++; // 重试次数加1
      }while((err!=0)&&(Count<5)); // 如果有错误且重试次数小于5，则继续重试

     if(err==0) // 修正：通常是err==0表示成功
     {
          ret=gRET_OK; // 认为成功
     }
     else // 如果重试5次后仍然有错误
     {
          ret=gRET_NG; // 标记为失败
          // 清空RAM中的寄存器副本，表示配置无效
          Bq769xxReg.SysCtrl1.SysCtrl1Byte=0;
          Bq769xxReg.SysCtrl2.SysCtrl2Byte=0;
          Bq769xxReg.Protect1.Protect1Byte=0;
          Bq769xxReg.Protect2.Protect2Byte=0;
          Bq769xxReg.Protect3.Protect3Byte=0;
          Bq769xxReg.OVTrip=0;
          Bq769xxReg.UVTrip=0;
          Bq769xxReg.CCCfg=0;
     }
	 (void) rcd; // 避免编译器对未使用变量rcd的警告
     return  ret;
}

/**
 * @brief  初始化 BQ769xx 芯片
 * @param  无
 * @retval 无
 * @detail 依次执行设置电芯映射、初始化参数到RAM、读取增益/偏移、写入配置到芯片等步骤。
 *         根据写入配置的结果设置 BQ769xx_Init_State 状态。
 */
void BQ769xx_Init(void)
{
      u8  rcd=0; // 用于存储函数返回值
      //--------设置电芯 温度分布(实际是电芯连接映射)------//
      BQ769xx_Set_CellMap();
     //---------参数初始化(到RAM)-------------//
      BQ769xx_Init_Para();
     //---------读取电压增益和偏移-----//
      BQ769xx_Read_Gain();
     //----------参数配置(写入芯片)--------------//
      rcd=BQ769xx_Write_Config();

      if(rcd==gRET_OK) // 如果写入配置成功
      {
          BQ769xx_Init_State=1; // 设置初始化状态为成功
      }
      else // 如果写入配置失败
      {
          BQ769xx_Init_State=0; // 设置初始化状态为失败
      }
}

/**
 * @brief  限制输入值在最大值和最小值之间
 * @param  InValue 输入值
 * @param  gMax 允许的最大值
 * @param  gMin 允许的最小值
 * @retval u16 - 限制后的值
 */
u16 LimitMaxMin(u16 InValue,u16 gMax,u16 gMin)
{
	u16 valueout=0; // 输出值

	valueout=InValue; // 默认输出值为输入值
    if(InValue>gMax )   // 如果输入值大于最大值
    {
        valueout=gMax;  // 输出值设为最大值
    }

    if(InValue<gMin )   // 如果输入值小于最小值
    {
        valueout=gMin;  // 输出值设为最小值
    }
    return valueout;    // 返回限制后的值
}

/**
 * @brief  从 BQ769xx 芯片获取实时数据
 * @param  无
 * @retval 无
 * @detail 该函数在200ms定时标志触发时执行。
 *         通过I2C一次性读取40字节的数据，包括所有电芯电压、电池包总电压、
 *         外部温度传感器电压和库仑计计数值。
 *         然后对这些原始数据进行处理，转换为实际的物理量并存储。
 */
void BQ769xx_GetData(void)
{
	u8  rcd=0;        // I2C操作返回值
	u8  ia=0,ib=0;    // 循环计数器
	u16 cells_map=gBMSData.BattPar.Cells_Map; // 获取当前电芯连接映射
	u8  VoltOffset=0; // 临时存储电压偏移量
	u16 AdcCurr=0;    // 存储原始的电流ADC值

    // 如果200ms定时标志未置位，则不执行数据采集
    if(TaskTimePare.Tim200ms_flag != 1)
    {
		return;
	}

	//---------------------采集BQ769xx数据-------------------------------//
	// 从VC1_HI_RegAddr开始，连续读取40个字节
	rcd= IIC_ReadByteMore(BQ769xxAddr, VC1_HI_RegAddr, BQ769xxGatherData,40);   //.VCell1.VCell1Byte.VC1_HI
	if( rcd==gRET_OK ) // 如果I2C读取成功
	{
		// 将读取到的40字节数据拷贝到 Bq769xxReg 结构体中对应的位置
		// 假设 Bq769xxReg.VCell1.VCell1Byte.VC1_HI 是一个指向40字节缓冲区的指针或数组起始
		memcpy(&(Bq769xxReg.VCell1.VCell1Byte.VC1_HI), BQ769xxGatherData, 40);
	}
	 //--------------------------电芯电压处理-----------------------------------//
	ib = 0; // gBMSData.BattPar.VoltCell 数组的索引
	for(ia=0;ia<SYS_CELL_MAX;ia++) // 遍历BQ769xx芯片所有可能的电芯输入
	{
		if(cells_map&0x01) // 检查当前电芯输入是否被使用
		{
			// ADCOffset 是一个带符号的8位值
			if(Bq769xxReg.ADCOffset&0x80) // 检查偏移量是否为负
			{
				VoltOffset= 0x100-Bq769xxReg.ADCOffset; // 计算负偏移的绝对值
				// 电压计算公式: ( (RawADC_HighByte*256 + RawADC_LowByte) * Gain_μV/LSB ) / 1000 - Offset_mV
				gBMSData.BattPar.VoltCell[ib]=(u16)(((unsigned long)((BQ769xxGatherData[ia*2]*256)+BQ769xxGatherData[ia*2+1])*VoltCellGainUV)/1000-VoltOffset);
			}
			else // 偏移量为正
			{
				VoltOffset= Bq769xxReg.ADCOffset; // 正偏移值
				// 电压计算公式: ( (RawADC_HighByte*256 + RawADC_LowByte) * Gain_μV/LSB ) / 1000 + Offset_mV
				gBMSData.BattPar.VoltCell[ib]=(u16)(((unsigned long)((BQ769xxGatherData[ia*2]*256)+BQ769xxGatherData[ia*2+1])*VoltCellGainUV)/1000+VoltOffset);
			}
			ib++; // 指向下一个有效的电芯槽位
		}
		cells_map=cells_map>>1; // cells_map右移一位，检查下一个电芯输入
	}

	//-----------------------------电池总电压处理-----------------------------------//
	// 总电压数据在 BQ769xxGatherData 的第30和31字节
	gBMSData.BattPar.VoltLine=(u16)(((unsigned long)((BQ769xxGatherData[30]*256)+BQ769xxGatherData[31])*BATVOLTLSB)/1000);   // 单位: MV

	//-----------------------------温度处理-----------------------------------//
	// 外部温度传感器数据
	for(ia=0;ia<SYS_TEMPEXT_MAX;ia++) // SYS_TEMPEXT_MAX 是外部温度传感器的数量
	{
		// TSx 引脚的电压 LSB = 382 μV
		VoltageTemp[ia]=((unsigned long)((BQ769xxGatherData[ia*2+32]*256)+BQ769xxGatherData[ia*2+32+1])*382)/1000; // mV
		// 调用 TemChange 函数将电压值转换为实际温度值 (°C)
		gBMSData.BattPar.TempCell[ia]=TemChange(VoltageTemp[ia]);
	}

	//-----------------------------电流处理---------------------------------//
	// 电流数据 (CC_HI, CC_LO) 在 BQ769xxGatherData 的第38和39字节
	AdcCurr=(u16)(BQ769xxGatherData[38]*256)+BQ769xxGatherData[39]; // 组合成16位原始ADC值

	// 电流值是带符号的
	if(AdcCurr&0x8000) // 如果是负电流 (放电)
	{
		AdcCurr=0x10000-AdcCurr; // 取补码得到绝对值
		// 电流计算: (RawADC_Value * LSB_μV_per_Count) / Shunt_Resistor_mOhm
		// CRUUVOLTLSB: 库仑计每LSB代表的电压差，通常是 8.44μV/LSB
		// gBMSConfig.Type.ShuntSpec: 采样电阻值 (mOhm)
		gBMSData.BattPar.CurrLine=0x10000-(u16)(((unsigned long) AdcCurr*CRUUVOLTLSB)/gBMSConfig.Type.ShuntSpec) ;   // LSB=8.44uV , 采样电阻：ShuntSpec mOhm
	}
	else // 正电流 (充电)
	{
		gBMSData.BattPar.CurrLine=(u16)(((unsigned long)AdcCurr*CRUUVOLTLSB)/gBMSConfig.Type.ShuntSpec) ;   // LSB=8.44uV , 采样电阻：ShuntSpec mOhm
	}

	pack_temp_max_min(); // 更新电池包最高/最低温度
	pack_cell_max_min(); // 更新电池包最高/最低电芯电压

	(void) rcd; // 避免编译器对未使用变量rcd的警告
}

/**
 * @brief  从 BQ769xx 芯片获取配置和状态信息
 * @param  无
 * @retval 无
 * @detail 该函数在500ms定时标志触发时执行。
 *         通过I2C一次性读取12字节的数据，从 SYS_STAT_RegAddr 开始。
 *         读取的数据会拷贝到 Bq769xxReg 结构体中。
 */
void BQ769xx_GetConfig(void)
{
	u8  rcd=0; // I2C操作返回值
	u8  BQ769xxGatherConfig[12]={0}; // 存储读取的配置/状态数据

	if(TaskTimePare.Tim500ms_flag==1 ) // 如果500ms定时标志置位
    {
		// 从SYS_STAT_RegAddr开始，读取12个字节
		rcd= IIC_ReadByteMore(BQ769xxAddr, SYS_STAT_RegAddr, BQ769xxGatherConfig,12);
		if( rcd==gRET_OK ) // 如果I2C读取成功
		{
			// 将读取的数据拷贝到 Bq769xxReg 结构体中对应的状态寄存器部分
			memcpy(&(Bq769xxReg.SysStatus.StatusByte), BQ769xxGatherConfig, 12);
		}
	}
}

/**
 * @brief  设置 BQ769xx 的放电FET (DSG FET) 状态
 * @param  ONOFF 1: 开启DSG FET, 0: 关闭DSG FET
 * @retval u8 - gRET_OK (0) 表示成功, gRET_NG (1) 表示失败或参数错误
 */
u8 BQ769xx_DSGSET(u8 ONOFF)
{
    u8 ret=gRET_OK; // 函数返回值，默认成功
    u8 rcd=0;       // I2C操作返回值

    if( ONOFF>1) // 参数检查，ONOFF只能是0或1
    {
        return gRET_NG ; // 参数错误，返回失败
    }
    // 读取当前的系统控制寄存器2 (SYS_CTRL2) 的值
    rcd=IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr, &(Bq769xxReg.SysCtrl2.SysCtrl2Byte));
    if(rcd!=gRET_OK) // 如果读取失败
    {
        return gRET_NG; // 返回失败
    }

    if(ONOFF==1) // 如果要开启DSG FET
    {
          // 将 SYS_CTRL2 寄存器的 DSG_ON 位 (bit 1) 置1
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte|0x02);
    }
    else // 如果要关闭DSG FET
    {
          // 将 SYS_CTRL2 寄存器的 DSG_ON 位 (bit 1) 清0
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte&0xFD); // 0xFD = 11111101b
    }
    return ret; // 返回成功
}

/**
 * @brief  设置 BQ769xx 的充电FET (CHG FET) 状态
 * @param  ONOFF 1: 开启CHG FET, 0: 关闭CHG FET
 * @retval u8 - gRET_OK (0) 表示成功, gRET_NG (1) 表示失败或参数错误
 */
u8 BQ769xx_CHGSET(u8 ONOFF)
{
    u8 ret=gRET_OK; // 函数返回值，默认成功
    u8 rcd=0;       // I2C操作返回值

    if( ONOFF>1) // 参数检查，ONOFF只能是0或1
    {
        return gRET_NG ; // 参数错误，返回失败
    }

    // 读取当前的系统控制寄存器2 (SYS_CTRL2) 的值
    rcd = IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr,  &Bq769xxReg.SysCtrl2.SysCtrl2Byte );

    if(rcd!=gRET_OK) // 如果读取失败
    {
        return gRET_NG ; // 返回失败
    }

    if(ONOFF==1) // 如果要开启CHG FET
    {
          // 将 SYS_CTRL2 寄存器的 CHG_ON 位 (bit 0) 置1
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte|0x01 );
     }
     else // 如果要关闭CHG FET
     {
          // 将 SYS_CTRL2 寄存器的 CHG_ON 位 (bit 0) 清0
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte&0xFE ); // 0xFE = 11111110b
     }
    return ret ; // 返回成功
}

/**
 * @brief  获取 BQ769xx 芯片的系统状态寄存器 (SYS_STAT) 的值
 * @param  无
 * @retval u8 - SYS_STAT 寄存器的当前值
 */
u8 BQ769xx_STATE_GET(void)
{
  u8 rcd=0;     // I2C操作返回值
  u8 BQ_Stat=0; // 存储读取到的状态值

  // 读取 SYS_STAT 寄存器
  rcd=IIC_ReadByte(BQ769xxAddr, SYS_STAT_RegAddr, &BQ_Stat  );
  (void) rcd;   // 避免编译器对未使用变量rcd的警告
  return  BQ_Stat; // 返回状态值
}

/**
 * @brief  清除 BQ769xx 芯片系统状态寄存器 (SYS_STAT) 中的指定标志位
 * @param  statvalue - 一个位掩码，指示要清除哪些状态标志位。
 * @retval u8 - gRET_OK (0) 或 gRET_NG (1)
 * @detail BQ769xx的SYS_STAT寄存器中的标志位是通过向对应位写'1'来清除的。
 *         此函数在写操作后会回读，检查是否清除成功。
 *         注意：原代码逻辑判断成功/失败的方式可能与常规理解相反。
 */
u8 BQ769xx_STAT_CLEAR(u8 statvalue)
{
  u8 ret=gRET_OK; // 函数返回值
  u8 BQ_Stat=0;   // 用于回读状态值
  u8 rcd=0;       // I2C操作返回值

  // 向 SYS_STAT 寄存器写入 statvalue，目标是清除 statvalue 中为'1'的那些位
  rcd=IIC_WritByte(BQ769xxAddr, SYS_STAT_RegAddr,statvalue);

  // 回读 SYS_STAT 寄存器，检查清除操作是否成功
  rcd=IIC_ReadByte(BQ769xxAddr, SYS_STAT_RegAddr, &BQ_Stat );

  // BQ769xx中，写1清零。如果清除后，BQ_Stat中对应statvalue的位仍然是1，说明未清除成功。
  // 原代码逻辑：如果要清除的位中，有位仍然是1，返回OK。如果都清零了，返回NG。
  // 这与通常理解相反。这里按原逻辑注释。
  if(BQ_Stat&statvalue) // 如果要清除的位中，有位仍然是1
  {
      ret=gRET_OK; // 按原代码逻辑
  }
  else // 如果要清除的位都变成0了
  {
     ret=gRET_NG; // 按原代码逻辑
  }

  (void)  rcd; // 避免编译器对未使用变量rcd的警告
  return  ret;
}

/**
 * @brief  使 BQ769xx 进入运输模式 (Ship Mode / Shutdown Mode)
 * @param  无
 * @retval u8 - gRET_OK (0) 表示成功, gRET_NG (1) 表示I2C读取失败
 * @detail 运输模式是BQ769xx的最低功耗状态。
 *         进入运输模式需要特定的两步写操作到SYS_CTRL1寄存器。
 *         注意：原代码中读取SYS_CTRL1存到了SysCtrl2Byte，已在注释中标注潜在问题。
 */
u8 Enter_Ship_Mode(void)
{
    u8 ret=gRET_OK; // 函数返回值，默认成功
    u8 rcd=0;       // I2C操作返回值

    // 注意：这里读取 SYS_CTRL1 寄存器，但存到了 Bq769xxReg.SysCtrl2.SysCtrl2Byte。
    // 这很可能是个笔误，应该存到 Bq769xxReg.SysCtrl1.SysCtrl1Byte。
    // 假设后续 Bq769xxReg.SysCtrl1.SysCtrl1Byte 的值是正确的。
    rcd = IIC_ReadByte(BQ769xxAddr, SYS_CTRL1_RegAddr,  &Bq769xxReg.SysCtrl1.SysCtrl1Byte ); // 修正: 目标应为 SysCtrl1Byte

    if(rcd!=gRET_OK) // 如果读取失败
    {
        return gRET_NG; // 返回失败
    }

    // 进入运输模式第一步: SHUT_A = 1, SHUT_B = 0
    Bq769xxReg.SysCtrl1.SysCtrl1Byte|=0x01;  // SHUT_A (bit 0) 置1
    Bq769xxReg.SysCtrl1.SysCtrl1Byte&=~0x02; // SHUT_B (bit 1) 清0
    IIC_WritByte(BQ769xxAddr, SYS_CTRL1_RegAddr, Bq769xxReg.SysCtrl1.SysCtrl1Byte );

    // 进入运输模式第二步: SHUT_A = 0, SHUT_B = 1
    Bq769xxReg.SysCtrl1.SysCtrl1Byte&=~0x01; // SHUT_A (bit 0) 清0
    Bq769xxReg.SysCtrl1.SysCtrl1Byte|=0x02;  // SHUT_B (bit 1) 置1
    IIC_WritByte(BQ769xxAddr, SYS_CTRL1_RegAddr, Bq769xxReg.SysCtrl1.SysCtrl1Byte);

    return  ret; // 返回成功
}

/**
 * @brief  设置 BQ769xx 的库仑计 (CC) 单次测量模式
 * @param  ONOFF 1: 触发一次单次CC测量, 0: 清除 (通常自动清零)
 * @retval u8 - gRET_OK (0) 表示成功, gRET_NG (1) 表示失败或参数错误
 */
u8 BQ769xx_CC_ONESHOT_SET(u8 ONOFF)
{
    u8 ret=gRET_OK; // 函数返回值，默认成功
    u8 rcd=0;       // I2C操作返回值

    if( ONOFF>1) // 参数检查
    {
        return gRET_NG ; // 参数错误
    }
    // 读取当前的 SYS_CTRL2 寄存器值
    rcd=IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr, &(Bq769xxReg.SysCtrl2.SysCtrl2Byte));
    if(rcd!=gRET_OK) // 如果读取失败
    {
        return gRET_NG;
    }

    if(ONOFF==1) // 如果要触发一次单次测量
    {
          // 将 SYS_CTRL2 寄存器的 CC_ONESHOT 位 (bit 5) 置1
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte|0x20);
    }
    else // 如果要清除 CC_ONESHOT 位
    {
          // 将 SYS_CTRL2 寄存器的 CC_ONESHOT 位 (bit 5) 清0
          IIC_WritByte(BQ769xxAddr, SYS_CTRL2_RegAddr, Bq769xxReg.SysCtrl2.SysCtrl2Byte&0xDF); // 0xDF = 11011111b
    }
    // 回读 SYS_CTRL2 寄存器，更新RAM中的副本
	rcd=IIC_ReadByte(BQ769xxAddr, SYS_CTRL2_RegAddr, &(Bq769xxReg.SysCtrl2.SysCtrl2Byte));
    return ret;
}

/**
 * @brief  写入BQ769xx的电芯均衡寄存器 (CELLBAL1, CELLBAL2, CELLBAL3)
 * @param  cellbalanbyte1 - CELLBAL1 寄存器的值 (控制电芯1-5均衡)
 * @param  cellbalanbyte2 - CELLBAL2 寄存器的值 (控制电芯6-10均衡)
 * @param  cellbalanbyte3 - CELLBAL3 寄存器的值 (控制电芯11-15均衡)
 * @retval u8 - gRET_OK (0) 表示成功, gRET_NG (1) 表示I2C操作失败或校验失败
 */
u8 balance_write_regs(u8 cellbalanbyte1,u8 cellbalanbyte2,u8 cellbalanbyte3 )
{
    u8 ret=gRET_OK;      // 函数返回值，默认成功
    u8 rcd=0;            // I2C操作返回值
    u8 write_regs[3]={0}; // 存储要写入的3个字节
    u8 read_regs[3]={0};  // 存储回读的3个字节
    u8 ia=0;             // 循环计数器
    u16 cells_map=gBMSData.BattPar.Cells_Map; // cells_map 在此函数中未被使用
    (void) cells_map; // 避免编译器对未使用变量cells_map的警告
    write_regs[0]=cellbalanbyte1;
    write_regs[1]=cellbalanbyte2;
    write_regs[2]=cellbalanbyte3;
    

    // 从 CELLBAL1_RegAddr 开始，连续写入3个字节
    rcd=IIC_WritByteMore(BQ769xxAddr, CELLBAL1_RegAddr, write_regs,3);

    if( rcd!=gRET_OK ) // 如果写入失败
    {
          return  gRET_NG;
    }

    // 回读写入的3个字节进行校验
    rcd=IIC_ReadByteMore(BQ769xxAddr, CELLBAL1_RegAddr, read_regs,3);
    if( rcd!=gRET_OK ) // 如果回读失败
    {
          return  gRET_NG;
    }

    // 比较写入值和回读值是否一致
    for(ia=0; ia<3; ia++ )
    {
        if(read_regs[ia]!=write_regs[ia])
        {
            ret=gRET_NG; // 如果不一致，标记为失败
            break;       // 无需继续比较
        }
    }
	
    return ret;
}

/**
 * @brief  将逻辑电芯均衡开关映射到BQ769xx物理均衡寄存器位图
 * @param  balaCellSw - 逻辑电芯均衡开关位图。bit0代表第1个逻辑电芯，以此类推。
 * @param  cellbalaset - 输出参数，存储转换后的物理均衡寄存器位图。
 * @detail 此函数考虑了电芯连接映射 (Cells_Map)，将逻辑电芯的均衡请求
 *         转换为对BQ769xx物理输入的均衡位。
 */
void cell_balan_locatget(u32 balaCellSw, u16* cellbalaset)
{
	u8 ia=0;     // 外层循环计数器，遍历逻辑电芯
    u8 ib=0;     // 内层循环计数器，遍历BQ769xx物理输入
    u8 cntset=0; // 计数器，追踪当前是第几个“已连接”的物理输入
    u16 cells_map=0; // 临时存储电芯连接映射
    u32 outcellsw=0; // 存储最终的物理均衡位图

	// 遍历每一个逻辑电芯
	for(ia=0; ia<gBMSConfig.Type.CellNum_Ser; ia++)
	{
		cntset=0; // 重置已连接物理输入计数器
		cells_map=gBMSData.BattPar.Cells_Map; // 获取原始的物理连接映射
		(void)cells_map; // 避免警告

		// 遍历BQ769xx所有可能的物理输入
		for(ib=0; ib<SYS_CELL_MAX;ib++)
		{
			if(cells_map&0x01) // 如果当前物理输入(ib)是连接的
			{
				if(cntset == ia) // 如果这个连接的物理输入是我们要找的第ia个逻辑电芯
				{
					// cntset++; // 此处递增cntset可能过早，但原逻辑如此
					if(balaCellSw&(0x0001<<ia)) // 检查第ia个逻辑电芯是否需要均衡
					{
						outcellsw |= (0x0001<<ib); // 将对应的物理输入位(ib)在outcellsw中置1
						break; // 找到了，跳出内层循环
					}
				}
				cntset++; // 无论是否匹配，只要是连接的物理输入，cntset都应增加
			 }
			 cells_map=cells_map>>1; // 检查下一个物理输入
		}
	}
    *cellbalaset=(u16)outcellsw; // 将结果存入输出参数
    
}

/**
 * @brief  电池包均衡检查与控制状态机
 * @param  无
 * @retval u8 - gRET_OK (0) 通常表示状态机正常运行,
 *              gRET_TM (特定值) 如果200ms标志未到,
 *              gRET_NG (1) 如果被强制均衡打断。
 * @detail 此函数以200ms的周期执行，管理电池包的自动均衡过程。
 */
u8 Balan_Pack_Check(void)
{
    u8 ret=gRET_OK; // 函数返回值
    u8 ia=0, ib=0;  // 循环计数器
	u16 volta_temp; // 临时变量，用于排序时交换电压值
	u8 volta_flag;  // 临时变量，用于排序时交换电芯索引

	static u8 stup_balan=0; // 均衡状态机当前状态
	static u16 volta_cell_sort[PACK_CELL_MAX] = {0}; // 用于存储待排序的电芯电压
	static u8 volta_flag_sort[PACK_CELL_MAX] = {0};  // 用于存储排序后电压对应的原始电芯索引
	static u16 balanwait_cnt=0; // 均衡等待计数器
	static u16 balantime_cnt=0; // 均衡进行时长的计数器

	// 如果200ms定时标志未置位，则不执行
    if(TaskTimePare.Tim200ms_flag==0)
    {
      return gRET_TM; // 返回特定值表示时间未到
    }

    // 如果强制均衡开启，则不执行自动均衡逻辑
	if(gBMSData.BalaPar.Bala_force_on == 1)
	{
		return gRET_NG; // 返回特定值表示被强制均衡打断
	}

    switch(stup_balan) // 根据当前均衡状态执行操作
    {
		case PCB_SEQ_INIT: // 0: 初始化状态
			gBMSData.BalaPar.bala_state = 0;    // 均衡状态标志清零
			gBMSData.BalaPar.bala_ctrl_sw = 0;  // 逻辑电芯均衡控制开关位图清零
			gBMSData.BalaPar.bala_dischg=0;     // 物理均衡寄存器位图清零

			// 清空BQ769xx的均衡寄存器在RAM中的副本并写入芯片
			Bq769xxReg.CellBal1.CellBal1Byte=0;
			Bq769xxReg.CellBal2.CellBal2Byte=0;
			Bq769xxReg.CellBal3.CellBal3Byte=0;
			balance_write_regs(Bq769xxReg.CellBal1.CellBal1Byte, Bq769xxReg.CellBal2.CellBal2Byte, Bq769xxReg.CellBal3.CellBal3Byte );
			stup_balan = PCB_SEQ_WAIT; // 进入等待状态
			break;
		case PCB_SEQ_WAIT: // 1: 等待状态
			if(++balanwait_cnt > 10) // 等待 balanwait_cnt * 200ms
			{
				balanwait_cnt = 0; // 重置等待计数器
				stup_balan = PCB_SEQ_CHK; // 进入检查状态
			}
			break;
		case PCB_SEQ_CHK: // 2: 检查均衡条件状态
			// 条件1: 系统模式必须是待机模式
			if(gBMSData.Sys_Mod.sys_mode != SYS_MODE_STANDBY)
			{
				stup_balan = PCB_SEQ_INIT; // 不满足则返回初始化状态
				break;
			}
			// 条件2: 压差和最低电压检查
			if(((gBMSData.BattPar.VoltCellMax - gBMSData.BattPar.VoltCellMin) < gBMSConfig.Balan.balanc_diffe_volt)||
			   (gBMSData.BattPar.VoltCellMin < gBMSConfig.Balan.balanc_start_volt))
			{
				stup_balan = PCB_SEQ_INIT; //不满足则返回初始化状态
				break;
			}
			stup_balan = PCB_SEQ_SELECT; // 条件满足，进入选择均衡电芯状态
			break;
		case PCB_SEQ_SELECT: // 3: 选择需要均衡的电芯状态
			// 复制电压和索引用于排序
			for(ia=0;ia<gBMSConfig.Type.CellNum_Ser; ia++)
			{
				volta_cell_sort[ia] = gBMSData.BattPar.VoltCell[ia];
				volta_flag_sort[ia] = ia;
			}
            // 冒泡排序，按电压从高到低排列
            for(ia=0;ia<gBMSConfig.Type.CellNum_Ser; ia++)
			{
				for(ib=0;ib<(gBMSConfig.Type.CellNum_Ser -1 - ia); ib++) // 优化比较次数
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
			// 选择排序后电压最高的 N 个电芯进行均衡
			for(ia = 0; ia < gBMSConfig.Balan.balanc_number_max; ia++)
			{
				gBMSData.BalaPar.bala_ctrl_sw |= (0x00000001 << volta_flag_sort[ia]);
			}
			stup_balan = PCB_SEQ_MAP; // 进入映射状态
            break;
        case PCB_SEQ_MAP:  // 4: 映射逻辑均衡位到物理均衡位状态
			cell_balan_locatget(gBMSData.BalaPar.bala_ctrl_sw, &gBMSData.BalaPar.bala_dischg);
			stup_balan = PCB_SEQ_SET; // 进入设置均衡状态
            break;
        case PCB_SEQ_SET: // 5: 设置均衡寄存器状态
			// 分配物理均衡位图到三个均衡寄存器字节
			Bq769xxReg.CellBal1.CellBal1Byte=gBMSData.BalaPar.bala_dischg&0x1F;
			Bq769xxReg.CellBal2.CellBal2Byte=(gBMSData.BalaPar.bala_dischg>>5)&0x1F;
			Bq769xxReg.CellBal3.CellBal3Byte=(gBMSData.BalaPar.bala_dischg>>10)&0x1F;
			// 写入BQ769xx芯片
			balance_write_regs( Bq769xxReg.CellBal1.CellBal1Byte, Bq769xxReg.CellBal2.CellBal2Byte, Bq769xxReg.CellBal3.CellBal3Byte );
			gBMSData.BalaPar.bala_state = 1; // 设置均衡状态为“均衡中”
			stup_balan=PCB_SEQ_BALA_TIM; // 进入均衡计时状态
            break;
        case PCB_SEQ_BALA_TIM: // 6: 均衡计时状态
			if(++balantime_cnt>900)  // 均衡持续 balantime_cnt * 200ms
            {
				stup_balan = PCB_SEQ_INIT; // 达到预设均衡时间，返回初始化
                balantime_cnt=0; // 重置计时器
				// 清理均衡相关状态
				gBMSData.BalaPar.bala_state = 0;
				gBMSData.BalaPar.bala_ctrl_sw = 0;
				gBMSData.BalaPar.bala_dischg=0;

				Bq769xxReg.CellBal1.CellBal1Byte=0;
				Bq769xxReg.CellBal2.CellBal2Byte=0;
				Bq769xxReg.CellBal3.CellBal3Byte=0;
				balance_write_regs( Bq769xxReg.CellBal1.CellBal1Byte, Bq769xxReg.CellBal2.CellBal2Byte, Bq769xxReg.CellBal3.CellBal3Byte );
            }
            break;
        default: // 异常状态，返回初始化
            stup_balan = PCB_SEQ_INIT;
            break;
     }
     return ret;
}

/**
 * @brief  计算并更新电池包中温度的最大值和最小值 (已修正逻辑)
 * @param  无
 * @retval 无
 * @detail 遍历所有有效的温度传感器读数，找到最高和最低温度。
 */
static void pack_temp_max_min(void)
{
    u8 ia;
    u16 temp_max;
    u16 temp_min;

    // 初始化最大值和最小值为第一个传感器的温度
    if (gBMSConfig.Type.TempNum > 0) { // 确保至少有一个温度传感器
        temp_max = gBMSData.BattPar.TempCell[0];
        temp_min = gBMSData.BattPar.TempCell[0];
    } else {
        // 如果没有温度传感器，可以设置一个默认值或错误值
        gBMSData.BattPar.TempPackMax = 0; // 或者一个特定的错误代码
        gBMSData.BattPar.TempPackMin = 0; // 或者一个特定的错误代码
        return;
    }

    // 遍历从第二个温度传感器开始的所有传感器
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

    gBMSData.BattPar.TempPackMax = temp_max; // 更新电池包最高温度
    gBMSData.BattPar.TempPackMin = temp_min; // 更新电池包最低温度
}

/**
 * @brief  计算并更新电池包中电芯电压的最大值和最小值
 * @param  无
 * @retval 无
 * @detail 遍历所有有效的电芯电压读数，找到最高和最低电压。
 */
static void pack_cell_max_min(void)
{
	u8 ia;
	u16 volt_max;
	u16 volt_min;

    // 初始化最大值和最小值为第一个电芯的电压
    if (gBMSConfig.Type.CellNum_Ser > 0) { // 确保至少有一个电芯
        volt_max = gBMSData.BattPar.VoltCell[0];
        volt_min = gBMSData.BattPar.VoltCell[0];
    } else {
        gBMSData.BattPar.VoltCellMax = 0;
        gBMSData.BattPar.VoltCellMin = 0;
        return;
    }

	// 遍历从第二个电芯开始的所有电芯
	for(ia = 1;ia < gBMSConfig.Type.CellNum_Ser;ia++)
	{
		if(gBMSData.BattPar.VoltCell[ia] > volt_max) // 修正：比较volt_max
		{
			volt_max=gBMSData.BattPar.VoltCell[ia];
		}
		if(gBMSData.BattPar.VoltCell[ia] < volt_min) // 修正：比较volt_min
		{
			volt_min = gBMSData.BattPar.VoltCell[ia];
		}
	}
	gBMSData.BattPar.VoltCellMax = volt_max; // 更新电池包最高单体电压
	gBMSData.BattPar.VoltCellMin = volt_min; // 更新电池包最低单体电压
}

// 假设这些宏定义在其他地方
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
 * @brief  BQ769xx 操作命令处理函数
 * @param  无
 * @retval 无
 * @detail 该函数在100ms定时标志触发时，或 Bq769xx_Oper_EN 被外部置位时执行。
 *         根据 Bq769xx_Oper_Type 的值，执行相应的操作。
 */
void  BQ769xx_Oper_Comm(void)
{
    // 如果100ms定时标志置位，或者操作使能标志为ON
    if((TaskTimePare.Tim100ms_flag==1)||(Bq769xx_Oper_EN==OPER_ON ) )
    {
		switch (Bq769xx_Oper_Type) // 根据操作类型执行
		{
			case COMM_DSG_ON: // 打开放电FET
				BQ769xx_DSGSET(OPER_ON);
				Bq769xx_Oper_EN=OPER_OFF; // 清除操作使能标志
				Bq769xx_Oper_Type=0;      // 清除操作类型
				break;
			case COMM_DSG_OFF: // 关闭放电FET
				BQ769xx_DSGSET(OPER_OFF);
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_CHG_ON: // 打开充电FET
				BQ769xx_CHGSET(OPER_ON);
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_CHG_OFF: // 关闭充电FET
				BQ769xx_CHGSET(OPER_OFF);
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_CLEAR_ALERT: // 清除警报 (当前未实现具体操作)
				// BQ769xx_STAT_CLEAR(ALERT_BITS_TO_CLEAR); // 示例
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
			case COMM_ENTER_SHIP: // 进入运输模式
				Power_Down(); // 调用系统断电函数
				// Power_Down 内部会处理 Bq769xx_Oper_EN 和 Type
				break;
			default: // 未知操作类型
				Bq769xx_Oper_EN=OPER_OFF;
				Bq769xx_Oper_Type=0;
				break;
		}
	}
}

/**
 * @brief  系统断电函数
 * @param  无
 * @retval 无
 * @detail 使BQ769xx芯片进入运输模式，然后关闭主系统电源。
 */
void Power_Down(void)
{
	Enter_Ship_Mode();      // 使BQ769xx进入运输模式
	Bq769xx_Oper_EN=OPER_OFF; // 清除操作使能标志
	Bq769xx_Oper_Type=0;      // 清除操作类型
	SYS_POWER_OFF;          // 宏定义，关闭MCU或其他系统电源
}

// #define WAKE_TIME_OUT_1S 2 // 假设定义唤醒超时 (2 * 500ms = 1s)

/**
 * @brief  BQ769xx 唤醒命令处理函数
 * @param  无
 * @retval 无
 * @detail 该函数在500ms定时标志触发且 Bq769xx_Wake_EN 使能时执行。
 *         通过 bq769xx_Wake_cont 控制唤醒脉冲的产生和持续时间。
 */
void  BQ769xx_Wake_Comm(void)
{
	// 如果500ms定时标志置位，并且唤醒使能标志为1
	if((TaskTimePare.Tim500ms_flag==1)&&(Bq769xx_Wake_EN==1))
	{
		// bq769xx_Wake_cont 的最高位(0x80)用作状态标志
		if(bq769xx_Wake_cont&0x80) // 如果当前正在发送唤醒脉冲
		{
			// (bq769xx_Wake_cont&0x7F) 是脉冲持续时间的计数
			if((bq769xx_Wake_cont&0x7f)>=WAKE_TIME_OUT_1S) // 如果脉冲已达到预设时长
			{
				BQ769xx_WAKE_OFF;     // 拉低唤醒引脚，结束脉冲
				bq769xx_Wake_cont=0;  // 重置唤醒计数器
				Bq769xx_Wake_EN=0;    // 清除唤醒使能标志
				BQ769xx_Init();       // 重新初始化BQ769xx芯片
			}
			else  // 如果脉冲时长未到
			{
				bq769xx_Wake_cont++; // 增加计数
			}
		}
		else // 如果当前不在发送唤醒脉冲状态
		{
			BQ769xx_WAKE_ON;        // 拉高唤醒引脚，开始发送脉冲
			bq769xx_Wake_cont=0;    // 清零计数器
			bq769xx_Wake_cont|=0x80;// 将最高位置1，标记为“脉冲发送中”
		}
	}
}

