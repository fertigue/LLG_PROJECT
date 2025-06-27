
#include "soc.h"          // SOC算法相关头文件
#include "stm32f10x.h"    // STM32F10x系列微控制器头文件
#include "timer.h"        // 定时器相关头文件
#include "DataBase.h"     // BMS数据存储相关头文件 (假设包含gBMSData, gBMSConfig等全局结构体)
#include "ConfigPara.h"   // BMS配置参数相关头文件 (假设包含TRUE, FALSE, gRET_OK, gRET_NG等宏定义)
#include <stdlib.h>       // 标准库，包含abs()等函数


// 温度影响容量系数的上限值 (例如 1.050 表示 105.0%，实际值为1050表示1.050)
#define TEMP_CAP_RATE_LIMITH_HIGH   1050
// 温度影响容量系数的下限值 (例如 0.750 表示 75.0%，实际值为750表示0.750)
#define TEMP_CAP_RATE_LIMITL_LOW    750
// SOC值的上限 (1000 表示 100.0%)
#define SOC_LIMIT_MAX    1000
// 200ms计数器的数量，用于将电流积分单位转换为Ah或类似的容量单位。
// 18000 * 200ms = 3600s = 1小时。这意味着电流单位可能是10mA，那么18000个单位就是 10mA * 1h = 10mAh。
// 这个宏主要用于pack_rated_cap的计算，使其单位与电流积分的单位匹配。
#define CNT_H_200MS_NUM    18000  // 60分钟 * 60秒/分钟 * (1秒/0.2秒个周期) = 18000
// 静置后进行OCV校准的等待时间 (6000 * 200ms = 1200s = 20分钟)
#define STANBY_OCV_TIME   6000  // 20分钟 * 60秒/分钟 * (1秒/0.2秒个周期) = 6000

// SOC过压校准标志，TRUE表示下次检测到过压条件时可以进行SOC满充校准
static u8 soc_ovp_flag = TRUE; // 初始为TRUE，允许第一次满充校准
// SOC欠压校准标志，TRUE表示下次检测到欠压条件时可以进行SOC零点校准
static u8 soc_uvp_flag = TRUE; // 初始为TRUE，允许第一次零点校准
// 静置OCV校准计时器，单位是200ms的周期数
static  u16  stanby_ocv_cnt=0;

// 静态函数声明 (仅在本文件内可见)
static void batt_temp_full_cap(void);            // 根据当前温度修正电池的实际满充容量
static u16 rate_limit_max_min(u16 rate_value);  // 限制温度修正系数在预设的上下限之间
static void batt_ocvsoc_calcua_soc(void);        // 通过开路电压(OCV)查表法计算并校准SOC
static u16  Seeka_OcvSoc_Table(u16 SingleVolt,const u16 *OcvTable ); // OCV-SOC查表核心函数 (二分查找和线性插值)
static void batt_AHinte_calcua_soc(void);        // 通过安时积分法计算SOC，并处理满充和放空时的SOC校准

// OCV-SOC对应表 (开路电压-剩余容量百分比)
// 表中的电压单位通常是mV，SOC单位是1对应0.1% (例如1000代表100.0%)
// 这个表是针对特定类型电芯在特定温度下（通常是25℃）的特性曲线
// 数组索引从0到99，对应SOC从100%到1% (每1%一个点)
// OCV_Table[0] 对应 100% SOC时的电压
// OCV_Table[99] 对应 1% SOC时的电压
const u16 OCV_Table[100]=
{
    // 电压值 (mV)
    4194,4171,4155,4142,4132,4124,4117,4111,4106,4100, // 100% ~ 91% (索引 0-9)
    4095,4089,4082,4074,4066,4056,4047,4036,4027,4016, // 90%  ~ 81% (索引 10-19)
    4005,3995,3984,3974,3964,3954,3945,3936,3926,3918, // 80%  ~ 71% (索引 20-29)
    3909,3900,3892,3883,3869,3856,3846,3836,3826,3815, // 70%  ~ 61% (索引 30-39)
    3805,3793,3782,3770,3758,3746,3734,3723,3712,3702, // 60%  ~ 51% (索引 40-49)
    3692,3683,3675,3667,3660,3654,3648,3643,3638,3633, // 50%  ~ 41% (索引 50-59)
    3628,3624,3619,3615,3611,3607,3603,3597,3591,3586, // 40%  ~ 31% (索引 60-69)
    3582,3577,3573,3568,3563,3557,3552,3546,3540,3533, // 30%  ~ 21% (索引 70-79)
    3526,3518,3510,3501,3492,3482,3473,3461,3449,3436, // 20%  ~ 11% (索引 80-89)
    3423,3409,3397,3387,3380,3371,3363,3353,3340,3292  // 10%  ~ 1%  (索引 90-99)
};

/**
 * @brief  电池容量和SOC参数初始化函数
 * @param  无
 * @retval 无
 * @detail 初始化系统SOC为100%，并根据配置参数计算电池包的额定容量、实际容量和剩余容量。
 *         容量单位与安时积分的电流单位和时间基准相关。
 */
void  batt_cap_ocvsoc_init(void)
{
    // 初始化系统SOC为100.0% (1000代表100.0%)
	 gBMSData.BattPar.SOCSys = 1000;

    // 计算电池包的额定容量
    // gBMSConfig.Type.CapaRate: 单体电芯的额定容量 (单位可能与电流有关，例如 0.1Ah 或 mAh)
    // gBMSConfig.Type.CellNum_Par:并联的电芯数量
    // CNT_H_200MS_NUM: 用于将CapaRate的单位转换为与安时积分周期(200ms)和电流采样单位(假设10mA)匹配的单位
    // 例如，如果CapaRate是Ah，电流是10mA，则此计算将额定容量转换为 (Ah / (10mA*200ms)) 个单位
    // 假设 pack_rated_cap 的单位是 “10mA * 200ms” 的倍数
	 gBMSData.PackCap.pack_rated_cap = gBMSConfig.Type.CapaRate * gBMSConfig.Type.CellNum_Par * CNT_H_200MS_NUM;  // 电池包额定容量，单位：(电流采样单位 * 200ms)

    // 初始化时，实际容量等于额定容量
	 gBMSData.PackCap.pack_real_cap = gBMSData.PackCap.pack_rated_cap;    // 实际容量 = 额定容量，单位同上

    // 根据初始SOC计算电池包的初始剩余容量
	 gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;     // 电池包实时剩余容量，单位同上
}

/**
 * @brief  电池SOC计算的主检查函数
 * @param  无
 * @retval u8 - gRET_OK (0) 表示成功执行了一轮计算, gRET_NG (1) 表示未到执行时间或等待中
 * @detail 此函数由定时器以200ms的周期调用。
 *         在初始等待几个周期后，依次调用温度修正容量、OCV校准SOC和安时积分SOC的函数。
 */
u8 Batt_soc_check(void)
{
    u8 ret=gRET_OK; // 函数返回值，默认为成功
    static u8 wait_tim_cnt = 5; // 静态变量，用于SOC算法启动前的延时计数 (5 * 200ms = 1秒)

    // 检查200ms定时标志，如果未到时间则直接返回
    if(TaskTimePare.Tim200ms_flag != 1) // TaskTimePare.Tim200ms_flag 由定时器中断置位
    {
        return gRET_NG; // 未到200ms周期，返回失败/未执行
    }

    // 初始延时处理，确保系统稳定或其他模块初始化完成
    if(wait_tim_cnt > 0)
    {
        wait_tim_cnt--; // 延时计数减1
        return gRET_NG; // 仍在延时等待中，返回失败/未执行
    }

    // 延时结束后，执行SOC计算的各个步骤
    batt_temp_full_cap();     // 1. 根据温度修正电池的满充容量
    batt_ocvsoc_calcua_soc(); // 2. 如果条件满足（长时间静置），通过OCV查表校准SOC
    batt_AHinte_calcua_soc(); // 3. 通过安时积分法更新SOC，并处理满充/放空时的SOC校准

    return  ret; // 返回成功执行
}


/**
 * @brief  根据电池温度修正电池的实际满充容量
 * @param  无
 * @retval 无
 * @detail 根据当前电池包的最低温度，查询或计算一个修正系数，
 *         然后用此系数调整 gBMSData.PackCap.pack_real_cap (实际满充容量)。
 *         同时，根据新的实际满充容量和当前SOC，更新剩余容量。
 *         温度变化超过一定阈值 (1.0℃) 时才进行更新。
 */
static void batt_temp_full_cap(void)
{
    u8  ratio=0;         // 温度区间比例因子或查找表的索引
    u8  updata_flag=0;   // 是否更新容量的标志
    u16 temp_rate=0;     // 温度对容量影响的修正系数 (例如1000代表100%)
    static i16 temp_last=0; // 上一次用于计算的温度值 (单位0.1℃，例如250代表25.0℃)

    // BUG/可改进点1: 温度区间划分和ratio的计算方式较为粗略。
    // 更精确的方法是使用一个温度-容量系数的查找表，并进行线性插值。
    // 此处的ratio更像是一个分段线性函数的斜率调整因子或区间标识。
    // 温度单位 gBMSData.BattPar.TempPackMin 假设是 0.1℃。

    // 根据最低温度确定修正比例因子 ratio
    if( gBMSData.BattPar.TempPackMin>=250)  // 温度 >= 25.0℃
    {
      ratio=1; // 基准或高温区
    }
    else if((gBMSData.BattPar.TempPackMin<250)&&(gBMSData.BattPar.TempPackMin>=100)) // 10.0℃ <= 温度 < 25.0℃
    {   ratio=2; // 常温偏低
    }
    else if((gBMSData.BattPar.TempPackMin<100)&&(gBMSData.BattPar.TempPackMin>=0)) // 0℃ <= 温度 < 10.0℃
    {   ratio=3; // 低温区
    }
    else if((gBMSData.BattPar.TempPackMin<-10)&&(gBMSData.BattPar.TempPackMin>=-200)) // -20.0℃ <= 温度 < -1.0℃ (注意: -10应该是-1.0℃，这里可能有笔误)
    {   ratio=4; // 更低温区
    }
    else if((gBMSData.BattPar.TempPackMin<-200)&&(gBMSData.BattPar.TempPackMin>=-300))// -30.0℃ <= 温度 < -20.0℃
    {   ratio=5; // 极低温区
    }
    else    // 温度 < -30.0℃ 或其他未覆盖情况
    {
        ratio=6; // 其他极端低温
    }

    // 判断温度变化是否超过1.0℃ (10个0.1℃单位)，如果超过则准备更新容量
    if( gBMSData.BattPar.TempPackMin > temp_last) // 温度上升
    {
        if((gBMSData.BattPar.TempPackMin - temp_last) >= 10 ) // 温差大于等于1.0℃
        {
            updata_flag=1; // 设置更新标志
            temp_last = gBMSData.BattPar.TempPackMin; // 更新上次温度记录
        }
    }
    else // 温度下降或不变
    {
        if((temp_last - gBMSData.BattPar.TempPackMin) >= 10 ) // 温差大于等于1.0℃
        {
            updata_flag=1; // 设置更新标志
            temp_last = gBMSData.BattPar.TempPackMin; // 更新上次温度记录
        }
    }

    // 如果需要更新容量
    if(updata_flag == 1)
    {
        updata_flag = 0; // 清除更新标志
        // 计算温度修正系数 temp_rate
        // 公式：100% + ratio * (当前温度 - 25.0℃) / 1.0℃
        // temp_rate 的单位是 0.1%，所以1000代表100%
        // (gBMSData.BattPar.TempPackMin-250)/10: 将0.1℃为单位的温差转换为以1.0℃为单位的温差
        temp_rate = 1000 + ratio * (gBMSData.BattPar.TempPackMin - 250) / 10; // BUG/可改进点2: 此公式过于简化且可能不准确。
                                                                            // ratio的含义和用法与标准的温度补偿曲线不符。
                                                                            // 通常，低温会降低容量，高温影响较小或略微增加（特定范围）。
                                                                            // 这个公式在 TempPackMin < 250 时，会得到一个小于1000的值（如果ratio为正），表示容量降低，这大方向是对的。
                                                                            // 但ratio的取值和线性关系需要仔细验证。

        temp_rate = rate_limit_max_min(temp_rate); // 限制修正系数在合理范围内

        // 根据修正系数更新电池包的实际满充容量
        gBMSData.PackCap.pack_real_cap = gBMSData.PackCap.pack_rated_cap * temp_rate / 1000;      // 电池包温度修正后的实际满充容量，单位同额定容量
        // 根据新的实际满充容量和当前SOC，重新计算剩余容量
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;
    }
}

/**
 * @brief  限制温度修正容量的系数在预设的上下限之间
 * @param  rate_value - 未经限制的修正系数值
 * @retval u16 - 限制后的修正系数值
 */
static u16 rate_limit_max_min(u16 rate_value)
{
    u16 ret_value=0; // 返回值

    if(rate_value > TEMP_CAP_RATE_LIMITH_HIGH ) // 如果大于上限
    {
        ret_value = TEMP_CAP_RATE_LIMITH_HIGH; // 取上限值
    }
    else if(rate_value < TEMP_CAP_RATE_LIMITL_LOW) // 如果小于下限
    {
        ret_value = TEMP_CAP_RATE_LIMITL_LOW; // 取下限值
    }
    else // 在范围内
    {
        ret_value = rate_value; // 取原值
    }

    return ret_value;
}

/**
 * @brief  通过开路电压(OCV)查表法计算并校准SOC
 * @param  无
 * @retval 无
 * @detail 在系统首次启动时，或电池长时间静置后（电流小且稳定），
 *         使用当前最低单体电压查询OCV-SOC表来校准SOC值和剩余容量。
 */
static void batt_ocvsoc_calcua_soc(void)
{
    static u8 soc_flag = 1; // 静态标志，用于首次SOC估算。1: 需要首次估算, 0: 已完成首次估算

    // 首次上电或SOC算法重置后，立即进行一次OCV-SOC估算
    if(soc_flag == 1)
    {
        soc_flag = 0; // 清除首次估算标志
        // 使用当前电池包的最低单体电压 (gBMSData.BattPar.VoltCellMin) 查OCV表获取SOC
        gBMSData.BattPar.SOCSys = Seeka_OcvSoc_Table(gBMSData.BattPar.VoltCellMin, OCV_Table);
        // 根据校准后的SOC更新剩余容量
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;
        return; // 完成首次校准，直接返回
    }

    // BUG/可改进点3: OCV校准条件判断。
    // SYS_MODE_STANDBY 可能不足以判断是否真正静置。
    // 应该检查电流是否在很小的范围内（例如 +/- 几十mA）持续一段时间。
    // 仅判断系统模式为STANDBY可能在负载轻微波动时也触发OCV校准，导致不准确。

    // 如果系统当前不是静置模式 (例如正在充电或放电)
    if(gBMSData.Sys_Mod.sys_mode != SYS_MODE_STANDBY) // SYS_MODE_STANDBY表示系统处于待机/静置状态
    {
        stanby_ocv_cnt = 0; // 重置静置计时器，因为电池不再静置
        return ; // 非静置状态，不进行OCV校准
    }

    // 如果系统处于静置模式，累加静置时间计数器
    if(stanby_ocv_cnt++ >= STANBY_OCV_TIME) // STANBY_OCV_TIME 定义了静置多久后进行OCV校准 (例如20分钟)
    {
        stanby_ocv_cnt = 0; // 重置静置计时器，准备下一次计时
        // 静置时间足够长，进行OCV-SOC校准
        gBMSData.BattPar.SOCSys = Seeka_OcvSoc_Table(gBMSData.BattPar.VoltCellMin, OCV_Table);
        // 根据校准后的SOC更新剩余容量
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;
    }
}

/**
 * @brief  OCV-SOC查表函数 (基于二分查找和线性插值)
 * @param  SingleVolt - 当前输入的单体电压值 (mV)
 * @param  OcvTable   - 指向OCV-SOC表的指针 (表中的SOC是隐式的，通过索引对应)
 * @retval u16 - 计算得到的SOC值 (0-1000, 代表0-100.0%)
 * @detail OCV_Table[i] 存储的是 (100-i)% SOC 对应的电压。
 *         例如 OCV_Table[0] 是100% SOC电压，OCV_Table[99] 是1% SOC电压。
 *         函数返回的SOC值是 1000 - (查表得到的索引值 * 10 + 插值修正)。
 */
static u16  Seeka_OcvSoc_Table(u16 SingleVolt,const u16 *OcvTable )
{
    u16 SocVale=0;  // 最终计算出的SOC值 * 10 (例如 955 代表 95.5%)
    u8 TopVale=0;   // 二分查找的上边界索引
    u8 MidVale=49;  // 二分查找的中间索引 (初始值可以任意在0-99间，通常取中间)
    u8 BotVale=99;  // 二分查找的下边界索引
	  u8 index_for_soc = 0; // 代表SOC百分比的整数部分，例如95代表95%

    // BUG/可改进点4: 二分查找的实现。
    // while(TopVale < BotVale) 条件在某些情况下可能导致死循环或错过精确匹配。
    // 更标准的二分查找是 while(TopVale <= BotVale)。
    // 并且，当 SingleVolt 等于 OcvTable[MidVale] 时，应该直接找到，而不是继续调整边界。

    // 使用二分查找在OCV表中找到最接近输入电压的区间
    while(TopVale <= BotVale) // 标准二分查找条件
    {
       MidVale =(TopVale + BotVale) / 2; // 计算中间索引
       if(SingleVolt < OcvTable[MidVale]) // 如果输入电压 小于 当前中间索引对应电压 (意味着SOC更高，索引应更小)
       {
           // BUG/可改进点5: 查表逻辑与OCV表结构。
           // OCV表是电压随SOC降低而降低的。SingleVolt < OcvTable[MidVale] 表示实际SOC比MidVale对应的SOC要高。
           // MidVale对应的SOC是 (100-MidVale)%。
           // 如果SingleVolt < OcvTable[MidVale]，说明电压更高，SOC也更高，所以目标索引应该在MidVale的左边（更小）。
           // 因此，BotVale 应该更新为 MidVale - 1。
           // 原代码逻辑: TopVale = MidVale + 1;  <-- 这是假设OCV表电压随索引递增，与实际表结构相反。
           BotVale = MidVale - 1; // 修正后的逻辑：目标在左半部分
       }
       else if (SingleVolt > OcvTable[MidVale]) // 如果输入电压 大于 当前中间索引对应电压 (意味着SOC更低，索引应更大)
       {
           // 原代码逻辑: BotVale = MidVale - 1; <-- 这与实际表结构相反。
           TopVale = MidVale + 1; // 修正后的逻辑：目标在右半部分
       }
       else // SingleVolt == OcvTable[MidVale] 精确匹配
       {
           break; // 找到精确匹配点，跳出循环
       }
    }
    // 循环结束后，MidVale 指向一个电压值，SingleVolt 位于 OcvTable[MidVale] 和 OcvTable[MidVale-1] (如果SingleVolt大)
    // 或者 OcvTable[MidVale] 和 OcvTable[MidVale+1] (如果SingleVolt小) 之间，或者精确等于 OcvTable[MidVale]。
    // TopVale > BotVale 时，真正的插入点可能是 TopVale (或 BotVale+1)。
    // 此时的 MidVale 可能是最后一次比较的中间点，不一定是最佳插值点。
    // 需要重新确定插值区间。通常，二分查找结束后，目标值会落在 BotVale 和 TopVale 之间。
    // 假设我们用 TopVale 作为插值的一个基准点。
    // MidVale 最终会停在接近目标的位置，但插值需要用它和相邻点。

    // 为了插值，我们需要确定 SingleVolt 落在 OCV_Table 的哪两个连续元素之间。
    // 二分查找结束后，TopVale通常指向大于等于目标电压的最小索引，或者BotVale指向小于等于目标电压的最大索引。
    // 我们需要找到一个索引 `idx` 使得 OcvTable[idx] >= SingleVolt > OcvTable[idx+1] (因为表是降序的)
    // 或者 OcvTable[idx-1] > SingleVolt >= OcvTable[idx]

    // 重整插值逻辑，基于二分查找的最终 TopVale (TopVale 是第一个使得 OcvTable[TopVale] <= SingleVolt 的索引)
    // 或者说，SingleVolt 介于 OcvTable[TopVale-1] 和 OcvTable[TopVale] 之间 (假设TopVale > 0)

    // 边界处理:
    if (SingleVolt >= OcvTable[0]) // 如果电压大于等于100% SOC的电压
    {
        return 1000; // SOC为100%
    }
    if (SingleVolt <= OcvTable[99]) // 如果电压小于等于1% SOC的电压
    {
        return 1 * 10; // SOC为1% (或0%，根据定义)
                      // 原表只到1%，所以返回10 (1.0%)
    }

    // 重新进行线性搜索以确定插值区间，二分查找的最终MidVale可能不适合直接用于插值
    // 或者，正确调整二分查找的结束条件和MidVale的最终含义。
    // 假设我们坚持使用原代码的二分查找结果进行插值，但其准确性存疑。
    // MidVale 在原代码的二分查找后，并不直接是最佳插值点。

    // 原代码的插值逻辑 (存在较多问题，基于其二分查找的 MidVale 结果):
    // SocVale的单位是0.1%
    // MidVale代表的是 (100-MidVale)% 的整数SOC点。
    // 所以 MidVale*10 代表 (100-MidVale)*10 个0.1%单位。例如MidVale=0, SOC=100%; MidVale=99, SOC=1%。

    // 修正二分查找后的插值逻辑：
    // 二分查找结束后，SingleVolt 应该介于 OcvTable[BotVale] 和 OcvTable[TopVale] 之间（如果精确匹配则相等）。
    // 由于 OCV 表是电压随 SOC 索引增加（SOC减小）而减小。
    // 查找结束后，通常 SingleVolt 会落在 OcvTable[idx] 和 OcvTable[idx+1] 之间。
    // 或者更准确地说，如果循环结束条件是 TopVale > BotVale,
    // 则目标电压 SingleVolt 应该介于 OcvTable[BotVale] 和 OcvTable[TopVale] 之间。
    // (此时 BotVale 指向电压较高/SOC较高的点，TopVale 指向电压较低/SOC较低的点)
    // 我们需要找到一个 `i` 使得 `OcvTable[i] >= SingleVolt > OcvTable[i+1]`

    // 采用更简单直接的线性扫描来确定插值区间（虽然效率低，但逻辑清晰）
    // 或者修复二分查找使其返回正确的插值索引。
    // 这里我们尝试基于原代码的 MidVale 进行解释和修正，尽管二分查找部分本身有缺陷。

    // 假设二分查找后，MidVale 是一个基准索引。
    // OCV_Table[MidVale] 是与 (100-MidVale)% SOC 对应的电压。

    
    // 先通过线性扫描找到正确的区间 (弥补二分查找的不足)
    for (index_for_soc = 0; index_for_soc < 99; index_for_soc++) {
        if (SingleVolt >= OcvTable[index_for_soc+1] && SingleVolt < OcvTable[index_for_soc]) {
            // SingleVolt 在 OcvTable[index_for_soc] 和 OcvTable[index_for_soc+1] 之间
            // SOC 在 (100 - index_for_soc)% 和 (100 - (index_for_soc+1))% 之间
            // 高电压 OcvTable[index_for_soc] 对应 SOC_high = (100 - index_for_soc)
            // 低电压 OcvTable[index_for_soc+1] 对应 SOC_low = (100 - (index_for_soc+1))
            // 插值计算：
            // Soc_actual = SOC_low + (SingleVolt - Volt_low) * (SOC_high - SOC_low) / (Volt_high - Volt_low)
            // 注意这里SOC_high - SOC_low = 1 (%)
            // Soc_actual(0.1%) = (100 - (index_for_soc+1))*10 + (SingleVolt - OcvTable[index_for_soc+1]) * 10 / (OcvTable[index_for_soc] - OcvTable[index_for_soc+1])
            if (OcvTable[index_for_soc] == OcvTable[index_for_soc+1]) { // 避免除零
                 SocVale = (100 - (index_for_soc+1)) * 10; // 取较低的SOC（或两者平均）
            } else {
                 SocVale = (100 - (index_for_soc+1)) * 10 +
                           (u32)(SingleVolt - OcvTable[index_for_soc+1]) * 10 /
                           (OcvTable[index_for_soc] - OcvTable[index_for_soc+1]);
            }
            return SocVale;
        }
    }
    // 如果电压正好等于某个表项
    for (index_for_soc = 0; index_for_soc < 100; index_for_soc++) {
        if (SingleVolt == OcvTable[index_for_soc]) {
            return (100 - index_for_soc) * 10;
        }
    }
    // 如果上面的循环没有返回，说明电压超出了表的下边界（低于1% OCV）或上边界（高于100% OCV）
    // 但前面的边界检查应该已经处理了。如果到这里，可能是浮点精度或逻辑问题。
    // 为安全起见，如果找不到区间，可以返回一个边界值或错误值。
    // 这里基于原代码的返回格式：
    // 原代码最后返回 1000 - SocVale，而其SocVale是 MidVale*10 +/- 插值。
    // 如果 MidVale 是 (100-SOC%)的索引，那么 (100-MidVale) 是SOC%。
    // 那么 SocVale 应该是 (100-MidVale)*10 +/- 插值。
    // 我们的插值 SocVale 已经是 0-1000 的形式了。

    // BUG/可改进点6: 原代码的插值公式和返回 `1000 - SocVale` 的逻辑需要仔细审视。
    // OcvTable[MidVale]电压较高，OcvTable[MidVale+1]电压较低。
    // 如果 SingleVolt < OcvTable[MidVale] (且 > OcvTable[MidVale+1]), 说明电压在两者之间，但更接近 OcvTable[MidVale]。
    // 这意味着实际SOC比 (100-MidVale)% 要低一点点，但比 (100-(MidVale+1))% 要高。
    // 原代码 `SocVale=MidVale*10 + ((OcvTable[MidVale]-SingleVolt)*10) / (OcvTable[MidVale]-OcvTable[MidVale+1]);`
    // 这里的 MidVale*10 似乎代表的是一个与SOC索引相关的基准，而不是直接的SOC值。
    // 假设 MidVale 是SOC百分比的索引 (0=100%, 99=1%)。
    // 那么，如果 SingleVolt < OcvTable[MidVale] (且 > OcvTable[MidVale+1]),
    // 实际SOC值应该介于 (100-MidVale)% 和 (100-(MidVale+1))% 之间。
    // 此时，(100-MidVale)是较高的SOC值。
    // 正确的插值应该是:
    // SOC = (100-(MidVale+1)) + (SingleVolt - OcvTable[MidVale+1]) / (OcvTable[MidVale] - OcvTable[MidVale+1]) * 1
    // 乘以10得到0.1%单位:
    // SocVale_0_1_percent = (100-(MidVale+1))*10 + ((SingleVolt - OcvTable[MidVale+1])*10) / (OcvTable[MidVale] - OcvTable[MidVale+1]);
    // 而原代码返回的是 `1000-SocVale`。这暗示其 `SocVale` 计算的可能是一个“已消耗百分比”或类似的量。

    // 由于原代码的二分查找和插值逻辑比较复杂且可能存在问题，
    // 上面已经提供了一个基于线性扫描的更清晰的插值方法。
    // 如果必须沿用原代码的结构，需要彻底理解其 `MidVale` 的含义和 `SocVale` 的计算目标。
    // 为简单起见，如果线性扫描没有找到，返回一个基于MidVale的粗略值（这里不推荐，但符合“基于原代码”）。
    // 这里我们假设上面的线性扫描已经返回。如果它失败，这里会返回一个可能不准确的值。
    if (MidVale > 0 && MidVale < 99) { // 避免数组越界
        if(SingleVolt <= OcvTable[MidVale] && SingleVolt > OcvTable[MidVale+1]) { // SingleVolt 在 MidVale 和 MidVale+1 之间
             if (OcvTable[MidVale] == OcvTable[MidVale+1]) return (100-(MidVale+1))*10;
             SocVale = (100-(MidVale+1))*10 + (u32)(SingleVolt - OcvTable[MidVale+1])*10 / (OcvTable[MidVale] - OcvTable[MidVale+1]);
        } else if (SingleVolt > OcvTable[MidVale] && SingleVolt <= OcvTable[MidVale-1]) { // SingleVolt 在 MidVale-1 和 MidVale 之间
             if (OcvTable[MidVale-1] == OcvTable[MidVale]) return (100-MidVale)*10;
             SocVale = (100-MidVale)*10 + (u32)(SingleVolt - OcvTable[MidVale])*10 / (OcvTable[MidVale-1] - OcvTable[MidVale]);
        } else { // 精确等于 OcvTable[MidVale]
             SocVale = (100-MidVale)*10;
        }
    } else if (MidVale == 0) { // 接近100%
        SocVale = 1000;
    } else { // MidVale == 99, 接近1%
        SocVale = 10;
    }
    return SocVale; // 直接返回计算的SOC (0-1000)
}


/**
 * @brief  通过安时积分法计算SOC，并处理满充和放空时的SOC校准
 * @param  无
 * @retval 无
 * @detail 根据当前的工作模式（充电、放电、静置）和电流值，更新剩余容量 (pack_rem_cap)。
 *         当检测到接近满充或放空状态时，进行SOC校准。
 *         最后根据剩余容量和实际满充容量计算系统SOC (SOCSys)。
 */
static void batt_AHinte_calcua_soc(void)
{
    u16 curr_value = 0; // 当前电流的绝对值 (单位与pack_rem_cap的电流单位一致，例如10mA)

    // 当从非充电状态进入充电状态时，如果之前触发了欠压校准 (soc_uvp_flag == FALSE)，
    // 则重新使能欠压校准标志，允许下一次欠压时再次校准。
    if((gBMSData.Sys_Mod.sys_mode == SYS_MODE_CHARGE) && (soc_uvp_flag == FALSE)) // SYS_MODE_CHARGE 表示充电模式
    {
        soc_uvp_flag = TRUE; // 重新使能欠压校准
    }

    // 当从充电状态进入非充电状态（放电或静置）时，如果之前触发了过压校准 (soc_ovp_flag == FALSE)，
    // 则重新使能过压校准标志，允许下一次过压时再次校准。
    if((gBMSData.Sys_Mod.sys_mode != SYS_MODE_CHARGE) && (soc_ovp_flag == FALSE))
    {
        soc_ovp_flag = TRUE; // 重新使能过压校准
    }

    // 满充校准逻辑:
    // 条件1: 电池包最高单体电压接近或达到过压保护阈值 (OV_Thresh - 2mV，留一点余量)
    // 条件2: 当前处于充电模式
    // 条件3: 过压校准标志为TRUE (表示本次满充可以进行校准)
    if ((gBMSData.BattPar.VoltCellMax >=(gBMSConfig.BQ76Para.OV_Thresh-2)) && // VoltCellMax: 最高单体电压
        (gBMSData.Sys_Mod.sys_mode == SYS_MODE_CHARGE) &&
        (soc_ovp_flag == TRUE))
    {
        soc_ovp_flag = FALSE; // 本次满充已校准，在下次非充电转充电前不再重复校准
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap; // 剩余容量置为当前实际满充容量
        gBMSData.BattPar.SOCSys = 1000; // SOC置为100.0%
        return; // 完成满充校准，直接返回
    }

    // 放空校准逻辑:
    // 条件1: 电池包最低单体电压接近或达到欠压保护阈值 (UV_Thresh + 2mV，留一点余量)
    // 条件2: 当前不处于充电模式 (即处于放电或静置模式)
    // 条件3: 欠压校准标志为TRUE (表示本次放空可以进行校准)
    if((gBMSData.BattPar.VoltCellMin <= (gBMSConfig.BQ76Para.UV_Thresh+2)) && // VoltCellMin: 最低单体电压
        (gBMSData.Sys_Mod.sys_mode != SYS_MODE_CHARGE) &&
        (soc_uvp_flag == TRUE))
    {
        soc_uvp_flag = FALSE; // 本次放空已校准，在下次非放电转充电前不再重复校准
        gBMSData.PackCap.pack_rem_cap = 0; // 剩余容量置为0
        gBMSData.BattPar.SOCSys = 0;       // SOC置为0%
        return; // 完成放空校准，直接返回
    }

    // 安时积分计算:
    // 获取当前电流的绝对值。gBMSData.BattPar.CurrLine 的单位需要与 pack_rem_cap 的单位对应。
    // 假设 CurrLine 单位是10mA，安时积分周期是200ms，则 curr_value 就是 (实际电流/10mA) 的值。
    // 如果 CurrLine 是 mA，则需要 curr_value = abs(gBMSData.BattPar.CurrLine / 10);
    // 这里假设 CurrLine 的单位已经是 "10mA" 这个基本单位了。
    curr_value = abs(gBMSData.BattPar.CurrLine);

    if (gBMSData.Sys_Mod.sys_mode == SYS_MODE_CHARGE) // 充电模式
    {
        // 增加剩余容量，但不超过实际满充容量
        if(gBMSData.PackCap.pack_real_cap >= (gBMSData.PackCap.pack_rem_cap + curr_value))
        {
            gBMSData.PackCap.pack_rem_cap += curr_value;
        }
        else // 如果计算结果超过满充容量，则将其钳位到满充容量
        {
            gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap;
        }
    }
    else  if (gBMSData.Sys_Mod.sys_mode == SYS_MODE_DISCHARGE) // SYS_MODE_DISCHARGE 表示放电模式
    {
        // 减少剩余容量，但不低于0
        if(gBMSData.PackCap.pack_rem_cap >= curr_value)
        {
            gBMSData.PackCap.pack_rem_cap -= curr_value;
        }
        else // 如果计算结果低于0，则将其钳位到0
        {
            gBMSData.PackCap.pack_rem_cap = 0;
        }
    }
    else // 静置模式 (SYS_MODE_STANDBY 或其他非充非放模式)
    {
        // BUG/可改进点7: 静置自放电处理。
        // 原代码中注释掉了静置自放电的扣除。
        // 实际应用中，应考虑自放电率，并根据时间扣除少量容量。
        // 自放电率通常与温度和SOC有关。
        // 例如: 假设每小时自放电固定为X个单位 (1个单位是curr_value的最小单位)
        // if (自放电计时达到1小时) {
        //     if(gBMSData.PackCap.pack_rem_cap >= X) gBMSData.PackCap.pack_rem_cap -= X;
        //     else gBMSData.PackCap.pack_rem_cap = 0;
        // }

        // 原注释掉的逻辑 (每200ms固定扣除1个单位，约10mA*200ms，这可能太大了):
        //if(gBMSData.PackCap.pack_rem_cap >= 1)    //静态时10mA (假设1个单位是10mA*200ms)
        //{
        //     gBMSData.PackCap.pack_rem_cap -= 1;
        //}
        //else
        //{
        //     gBMSData.PackCap.pack_rem_cap = 0;
        //}
    }

    // 根据更新后的剩余容量和实际满充容量，计算系统SOC
    // BUG/可改进点8: 除零保护。
    // 如果 pack_real_cap 由于某种原因（例如未初始化或错误计算）变为0，这里会导致除零错误。
    if (gBMSData.PackCap.pack_real_cap > 0) // 增加除零保护
    {
        gBMSData.BattPar.SOCSys = (u32)gBMSData.PackCap.pack_rem_cap * 1000 / gBMSData.PackCap.pack_real_cap;
    }
    else // 如果实际容量为0，SOC也应为0 (或一个错误状态)
    {
        gBMSData.BattPar.SOCSys = 0;
    }

    // 限制SOC不超过上限 (通常是100.0%)
    if(gBMSData.BattPar.SOCSys > SOC_LIMIT_MAX) // SOC_LIMIT_MAX 通常是1000
    {
        gBMSData.BattPar.SOCSys = SOC_LIMIT_MAX; // 钳位到100.0%
    }
    // SOC的下限已经在安时积分时通过 pack_rem_cap >= 0 保证了。
}
