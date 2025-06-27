
#include "soc.h"          // SOC�㷨���ͷ�ļ�
#include "stm32f10x.h"    // STM32F10xϵ��΢������ͷ�ļ�
#include "timer.h"        // ��ʱ�����ͷ�ļ�
#include "DataBase.h"     // BMS���ݴ洢���ͷ�ļ� (�������gBMSData, gBMSConfig��ȫ�ֽṹ��)
#include "ConfigPara.h"   // BMS���ò������ͷ�ļ� (�������TRUE, FALSE, gRET_OK, gRET_NG�Ⱥ궨��)
#include <stdlib.h>       // ��׼�⣬����abs()�Ⱥ���


// �¶�Ӱ������ϵ��������ֵ (���� 1.050 ��ʾ 105.0%��ʵ��ֵΪ1050��ʾ1.050)
#define TEMP_CAP_RATE_LIMITH_HIGH   1050
// �¶�Ӱ������ϵ��������ֵ (���� 0.750 ��ʾ 75.0%��ʵ��ֵΪ750��ʾ0.750)
#define TEMP_CAP_RATE_LIMITL_LOW    750
// SOCֵ������ (1000 ��ʾ 100.0%)
#define SOC_LIMIT_MAX    1000
// 200ms�����������������ڽ��������ֵ�λת��ΪAh�����Ƶ�������λ��
// 18000 * 200ms = 3600s = 1Сʱ������ζ�ŵ�����λ������10mA����ô18000����λ���� 10mA * 1h = 10mAh��
// �������Ҫ����pack_rated_cap�ļ��㣬ʹ�䵥λ��������ֵĵ�λƥ�䡣
#define CNT_H_200MS_NUM    18000  // 60���� * 60��/���� * (1��/0.2�������) = 18000
// ���ú����OCVУ׼�ĵȴ�ʱ�� (6000 * 200ms = 1200s = 20����)
#define STANBY_OCV_TIME   6000  // 20���� * 60��/���� * (1��/0.2�������) = 6000

// SOC��ѹУ׼��־��TRUE��ʾ�´μ�⵽��ѹ����ʱ���Խ���SOC����У׼
static u8 soc_ovp_flag = TRUE; // ��ʼΪTRUE�������һ������У׼
// SOCǷѹУ׼��־��TRUE��ʾ�´μ�⵽Ƿѹ����ʱ���Խ���SOC���У׼
static u8 soc_uvp_flag = TRUE; // ��ʼΪTRUE�������һ�����У׼
// ����OCVУ׼��ʱ������λ��200ms��������
static  u16  stanby_ocv_cnt=0;

// ��̬�������� (���ڱ��ļ��ڿɼ�)
static void batt_temp_full_cap(void);            // ���ݵ�ǰ�¶�������ص�ʵ����������
static u16 rate_limit_max_min(u16 rate_value);  // �����¶�����ϵ����Ԥ���������֮��
static void batt_ocvsoc_calcua_soc(void);        // ͨ����·��ѹ(OCV)������㲢У׼SOC
static u16  Seeka_OcvSoc_Table(u16 SingleVolt,const u16 *OcvTable ); // OCV-SOC�����ĺ��� (���ֲ��Һ����Բ�ֵ)
static void batt_AHinte_calcua_soc(void);        // ͨ����ʱ���ַ�����SOC������������ͷſ�ʱ��SOCУ׼

// OCV-SOC��Ӧ�� (��·��ѹ-ʣ�������ٷֱ�)
// ���еĵ�ѹ��λͨ����mV��SOC��λ��1��Ӧ0.1% (����1000����100.0%)
// �����������ض����͵�о���ض��¶��£�ͨ����25�棩����������
// ����������0��99����ӦSOC��100%��1% (ÿ1%һ����)
// OCV_Table[0] ��Ӧ 100% SOCʱ�ĵ�ѹ
// OCV_Table[99] ��Ӧ 1% SOCʱ�ĵ�ѹ
const u16 OCV_Table[100]=
{
    // ��ѹֵ (mV)
    4194,4171,4155,4142,4132,4124,4117,4111,4106,4100, // 100% ~ 91% (���� 0-9)
    4095,4089,4082,4074,4066,4056,4047,4036,4027,4016, // 90%  ~ 81% (���� 10-19)
    4005,3995,3984,3974,3964,3954,3945,3936,3926,3918, // 80%  ~ 71% (���� 20-29)
    3909,3900,3892,3883,3869,3856,3846,3836,3826,3815, // 70%  ~ 61% (���� 30-39)
    3805,3793,3782,3770,3758,3746,3734,3723,3712,3702, // 60%  ~ 51% (���� 40-49)
    3692,3683,3675,3667,3660,3654,3648,3643,3638,3633, // 50%  ~ 41% (���� 50-59)
    3628,3624,3619,3615,3611,3607,3603,3597,3591,3586, // 40%  ~ 31% (���� 60-69)
    3582,3577,3573,3568,3563,3557,3552,3546,3540,3533, // 30%  ~ 21% (���� 70-79)
    3526,3518,3510,3501,3492,3482,3473,3461,3449,3436, // 20%  ~ 11% (���� 80-89)
    3423,3409,3397,3387,3380,3371,3363,3353,3340,3292  // 10%  ~ 1%  (���� 90-99)
};

/**
 * @brief  ���������SOC������ʼ������
 * @param  ��
 * @retval ��
 * @detail ��ʼ��ϵͳSOCΪ100%�����������ò��������ذ��Ķ������ʵ��������ʣ��������
 *         ������λ�밲ʱ���ֵĵ�����λ��ʱ���׼��ء�
 */
void  batt_cap_ocvsoc_init(void)
{
    // ��ʼ��ϵͳSOCΪ100.0% (1000����100.0%)
	 gBMSData.BattPar.SOCSys = 1000;

    // �����ذ��Ķ����
    // gBMSConfig.Type.CapaRate: �����о�Ķ���� (��λ����������йأ����� 0.1Ah �� mAh)
    // gBMSConfig.Type.CellNum_Par:�����ĵ�о����
    // CNT_H_200MS_NUM: ���ڽ�CapaRate�ĵ�λת��Ϊ�밲ʱ��������(200ms)�͵���������λ(����10mA)ƥ��ĵ�λ
    // ���磬���CapaRate��Ah��������10mA����˼��㽫�����ת��Ϊ (Ah / (10mA*200ms)) ����λ
    // ���� pack_rated_cap �ĵ�λ�� ��10mA * 200ms�� �ı���
	 gBMSData.PackCap.pack_rated_cap = gBMSConfig.Type.CapaRate * gBMSConfig.Type.CellNum_Par * CNT_H_200MS_NUM;  // ��ذ����������λ��(����������λ * 200ms)

    // ��ʼ��ʱ��ʵ���������ڶ����
	 gBMSData.PackCap.pack_real_cap = gBMSData.PackCap.pack_rated_cap;    // ʵ������ = ���������λͬ��

    // ���ݳ�ʼSOC�����ذ��ĳ�ʼʣ������
	 gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;     // ��ذ�ʵʱʣ����������λͬ��
}

/**
 * @brief  ���SOC���������麯��
 * @param  ��
 * @retval u8 - gRET_OK (0) ��ʾ�ɹ�ִ����һ�ּ���, gRET_NG (1) ��ʾδ��ִ��ʱ���ȴ���
 * @detail �˺����ɶ�ʱ����200ms�����ڵ��á�
 *         �ڳ�ʼ�ȴ��������ں����ε����¶�����������OCVУ׼SOC�Ͱ�ʱ����SOC�ĺ�����
 */
u8 Batt_soc_check(void)
{
    u8 ret=gRET_OK; // ��������ֵ��Ĭ��Ϊ�ɹ�
    static u8 wait_tim_cnt = 5; // ��̬����������SOC�㷨����ǰ����ʱ���� (5 * 200ms = 1��)

    // ���200ms��ʱ��־�����δ��ʱ����ֱ�ӷ���
    if(TaskTimePare.Tim200ms_flag != 1) // TaskTimePare.Tim200ms_flag �ɶ�ʱ���ж���λ
    {
        return gRET_NG; // δ��200ms���ڣ�����ʧ��/δִ��
    }

    // ��ʼ��ʱ����ȷ��ϵͳ�ȶ�������ģ���ʼ�����
    if(wait_tim_cnt > 0)
    {
        wait_tim_cnt--; // ��ʱ������1
        return gRET_NG; // ������ʱ�ȴ��У�����ʧ��/δִ��
    }

    // ��ʱ������ִ��SOC����ĸ�������
    batt_temp_full_cap();     // 1. �����¶�������ص���������
    batt_ocvsoc_calcua_soc(); // 2. ����������㣨��ʱ�侲�ã���ͨ��OCV���У׼SOC
    batt_AHinte_calcua_soc(); // 3. ͨ����ʱ���ַ�����SOC������������/�ſ�ʱ��SOCУ׼

    return  ret; // ���سɹ�ִ��
}


/**
 * @brief  ���ݵ���¶�������ص�ʵ����������
 * @param  ��
 * @retval ��
 * @detail ���ݵ�ǰ��ذ�������¶ȣ���ѯ�����һ������ϵ����
 *         Ȼ���ô�ϵ������ gBMSData.PackCap.pack_real_cap (ʵ����������)��
 *         ͬʱ�������µ�ʵ�����������͵�ǰSOC������ʣ��������
 *         �¶ȱ仯����һ����ֵ (1.0��) ʱ�Ž��и��¡�
 */
static void batt_temp_full_cap(void)
{
    u8  ratio=0;         // �¶�����������ӻ���ұ������
    u8  updata_flag=0;   // �Ƿ���������ı�־
    u16 temp_rate=0;     // �¶ȶ�����Ӱ�������ϵ�� (����1000����100%)
    static i16 temp_last=0; // ��һ�����ڼ�����¶�ֵ (��λ0.1�棬����250����25.0��)

    // BUG/�ɸĽ���1: �¶����仮�ֺ�ratio�ļ��㷽ʽ��Ϊ���ԡ�
    // ����ȷ�ķ�����ʹ��һ���¶�-����ϵ���Ĳ��ұ����������Բ�ֵ��
    // �˴���ratio������һ���ֶ����Ժ�����б�ʵ������ӻ������ʶ��
    // �¶ȵ�λ gBMSData.BattPar.TempPackMin ������ 0.1�档

    // ��������¶�ȷ�������������� ratio
    if( gBMSData.BattPar.TempPackMin>=250)  // �¶� >= 25.0��
    {
      ratio=1; // ��׼�������
    }
    else if((gBMSData.BattPar.TempPackMin<250)&&(gBMSData.BattPar.TempPackMin>=100)) // 10.0�� <= �¶� < 25.0��
    {   ratio=2; // ����ƫ��
    }
    else if((gBMSData.BattPar.TempPackMin<100)&&(gBMSData.BattPar.TempPackMin>=0)) // 0�� <= �¶� < 10.0��
    {   ratio=3; // ������
    }
    else if((gBMSData.BattPar.TempPackMin<-10)&&(gBMSData.BattPar.TempPackMin>=-200)) // -20.0�� <= �¶� < -1.0�� (ע��: -10Ӧ����-1.0�棬��������б���)
    {   ratio=4; // ��������
    }
    else if((gBMSData.BattPar.TempPackMin<-200)&&(gBMSData.BattPar.TempPackMin>=-300))// -30.0�� <= �¶� < -20.0��
    {   ratio=5; // ��������
    }
    else    // �¶� < -30.0�� ������δ�������
    {
        ratio=6; // �������˵���
    }

    // �ж��¶ȱ仯�Ƿ񳬹�1.0�� (10��0.1�浥λ)�����������׼����������
    if( gBMSData.BattPar.TempPackMin > temp_last) // �¶�����
    {
        if((gBMSData.BattPar.TempPackMin - temp_last) >= 10 ) // �²���ڵ���1.0��
        {
            updata_flag=1; // ���ø��±�־
            temp_last = gBMSData.BattPar.TempPackMin; // �����ϴ��¶ȼ�¼
        }
    }
    else // �¶��½��򲻱�
    {
        if((temp_last - gBMSData.BattPar.TempPackMin) >= 10 ) // �²���ڵ���1.0��
        {
            updata_flag=1; // ���ø��±�־
            temp_last = gBMSData.BattPar.TempPackMin; // �����ϴ��¶ȼ�¼
        }
    }

    // �����Ҫ��������
    if(updata_flag == 1)
    {
        updata_flag = 0; // ������±�־
        // �����¶�����ϵ�� temp_rate
        // ��ʽ��100% + ratio * (��ǰ�¶� - 25.0��) / 1.0��
        // temp_rate �ĵ�λ�� 0.1%������1000����100%
        // (gBMSData.BattPar.TempPackMin-250)/10: ��0.1��Ϊ��λ���²�ת��Ϊ��1.0��Ϊ��λ���²�
        temp_rate = 1000 + ratio * (gBMSData.BattPar.TempPackMin - 250) / 10; // BUG/�ɸĽ���2: �˹�ʽ���ڼ��ҿ��ܲ�׼ȷ��
                                                                            // ratio�ĺ�����÷����׼���¶Ȳ������߲�����
                                                                            // ͨ�������»ή������������Ӱ���С����΢���ӣ��ض���Χ����
                                                                            // �����ʽ�� TempPackMin < 250 ʱ����õ�һ��С��1000��ֵ�����ratioΪ��������ʾ�������ͣ�������ǶԵġ�
                                                                            // ��ratio��ȡֵ�����Թ�ϵ��Ҫ��ϸ��֤��

        temp_rate = rate_limit_max_min(temp_rate); // ��������ϵ���ں���Χ��

        // ��������ϵ�����µ�ذ���ʵ����������
        gBMSData.PackCap.pack_real_cap = gBMSData.PackCap.pack_rated_cap * temp_rate / 1000;      // ��ذ��¶��������ʵ��������������λͬ�����
        // �����µ�ʵ�����������͵�ǰSOC�����¼���ʣ������
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;
    }
}

/**
 * @brief  �����¶�����������ϵ����Ԥ���������֮��
 * @param  rate_value - δ�����Ƶ�����ϵ��ֵ
 * @retval u16 - ���ƺ������ϵ��ֵ
 */
static u16 rate_limit_max_min(u16 rate_value)
{
    u16 ret_value=0; // ����ֵ

    if(rate_value > TEMP_CAP_RATE_LIMITH_HIGH ) // �����������
    {
        ret_value = TEMP_CAP_RATE_LIMITH_HIGH; // ȡ����ֵ
    }
    else if(rate_value < TEMP_CAP_RATE_LIMITL_LOW) // ���С������
    {
        ret_value = TEMP_CAP_RATE_LIMITL_LOW; // ȡ����ֵ
    }
    else // �ڷ�Χ��
    {
        ret_value = rate_value; // ȡԭֵ
    }

    return ret_value;
}

/**
 * @brief  ͨ����·��ѹ(OCV)������㲢У׼SOC
 * @param  ��
 * @retval ��
 * @detail ��ϵͳ�״�����ʱ�����س�ʱ�侲�ú󣨵���С���ȶ�����
 *         ʹ�õ�ǰ��͵����ѹ��ѯOCV-SOC����У׼SOCֵ��ʣ��������
 */
static void batt_ocvsoc_calcua_soc(void)
{
    static u8 soc_flag = 1; // ��̬��־�������״�SOC���㡣1: ��Ҫ�״ι���, 0: ������״ι���

    // �״��ϵ��SOC�㷨���ú���������һ��OCV-SOC����
    if(soc_flag == 1)
    {
        soc_flag = 0; // ����״ι����־
        // ʹ�õ�ǰ��ذ�����͵����ѹ (gBMSData.BattPar.VoltCellMin) ��OCV���ȡSOC
        gBMSData.BattPar.SOCSys = Seeka_OcvSoc_Table(gBMSData.BattPar.VoltCellMin, OCV_Table);
        // ����У׼���SOC����ʣ������
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;
        return; // ����״�У׼��ֱ�ӷ���
    }

    // BUG/�ɸĽ���3: OCVУ׼�����жϡ�
    // SYS_MODE_STANDBY ���ܲ������ж��Ƿ��������á�
    // Ӧ�ü������Ƿ��ں�С�ķ�Χ�ڣ����� +/- ��ʮmA������һ��ʱ�䡣
    // ���ж�ϵͳģʽΪSTANDBY�����ڸ�����΢����ʱҲ����OCVУ׼�����²�׼ȷ��

    // ���ϵͳ��ǰ���Ǿ���ģʽ (�������ڳ���ŵ�)
    if(gBMSData.Sys_Mod.sys_mode != SYS_MODE_STANDBY) // SYS_MODE_STANDBY��ʾϵͳ���ڴ���/����״̬
    {
        stanby_ocv_cnt = 0; // ���þ��ü�ʱ������Ϊ��ز��پ���
        return ; // �Ǿ���״̬��������OCVУ׼
    }

    // ���ϵͳ���ھ���ģʽ���ۼӾ���ʱ�������
    if(stanby_ocv_cnt++ >= STANBY_OCV_TIME) // STANBY_OCV_TIME �����˾��ö�ú����OCVУ׼ (����20����)
    {
        stanby_ocv_cnt = 0; // ���þ��ü�ʱ����׼����һ�μ�ʱ
        // ����ʱ���㹻��������OCV-SOCУ׼
        gBMSData.BattPar.SOCSys = Seeka_OcvSoc_Table(gBMSData.BattPar.VoltCellMin, OCV_Table);
        // ����У׼���SOC����ʣ������
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap * gBMSData.BattPar.SOCSys / 1000;
    }
}

/**
 * @brief  OCV-SOC����� (���ڶ��ֲ��Һ����Բ�ֵ)
 * @param  SingleVolt - ��ǰ����ĵ����ѹֵ (mV)
 * @param  OcvTable   - ָ��OCV-SOC���ָ�� (���е�SOC����ʽ�ģ�ͨ��������Ӧ)
 * @retval u16 - ����õ���SOCֵ (0-1000, ����0-100.0%)
 * @detail OCV_Table[i] �洢���� (100-i)% SOC ��Ӧ�ĵ�ѹ��
 *         ���� OCV_Table[0] ��100% SOC��ѹ��OCV_Table[99] ��1% SOC��ѹ��
 *         �������ص�SOCֵ�� 1000 - (���õ�������ֵ * 10 + ��ֵ����)��
 */
static u16  Seeka_OcvSoc_Table(u16 SingleVolt,const u16 *OcvTable )
{
    u16 SocVale=0;  // ���ռ������SOCֵ * 10 (���� 955 ���� 95.5%)
    u8 TopVale=0;   // ���ֲ��ҵ��ϱ߽�����
    u8 MidVale=49;  // ���ֲ��ҵ��м����� (��ʼֵ����������0-99�䣬ͨ��ȡ�м�)
    u8 BotVale=99;  // ���ֲ��ҵ��±߽�����
	  u8 index_for_soc = 0; // ����SOC�ٷֱȵ��������֣�����95����95%

    // BUG/�ɸĽ���4: ���ֲ��ҵ�ʵ�֡�
    // while(TopVale < BotVale) ������ĳЩ����¿��ܵ�����ѭ��������ȷƥ�䡣
    // ����׼�Ķ��ֲ����� while(TopVale <= BotVale)��
    // ���ң��� SingleVolt ���� OcvTable[MidVale] ʱ��Ӧ��ֱ���ҵ��������Ǽ��������߽硣

    // ʹ�ö��ֲ�����OCV�����ҵ���ӽ������ѹ������
    while(TopVale <= BotVale) // ��׼���ֲ�������
    {
       MidVale =(TopVale + BotVale) / 2; // �����м�����
       if(SingleVolt < OcvTable[MidVale]) // ��������ѹ С�� ��ǰ�м�������Ӧ��ѹ (��ζ��SOC���ߣ�����Ӧ��С)
       {
           // BUG/�ɸĽ���5: ����߼���OCV��ṹ��
           // OCV���ǵ�ѹ��SOC���Ͷ����͵ġ�SingleVolt < OcvTable[MidVale] ��ʾʵ��SOC��MidVale��Ӧ��SOCҪ�ߡ�
           // MidVale��Ӧ��SOC�� (100-MidVale)%��
           // ���SingleVolt < OcvTable[MidVale]��˵����ѹ���ߣ�SOCҲ���ߣ�����Ŀ������Ӧ����MidVale����ߣ���С����
           // ��ˣ�BotVale Ӧ�ø���Ϊ MidVale - 1��
           // ԭ�����߼�: TopVale = MidVale + 1;  <-- ���Ǽ���OCV���ѹ��������������ʵ�ʱ�ṹ�෴��
           BotVale = MidVale - 1; // ��������߼���Ŀ������벿��
       }
       else if (SingleVolt > OcvTable[MidVale]) // ��������ѹ ���� ��ǰ�м�������Ӧ��ѹ (��ζ��SOC���ͣ�����Ӧ����)
       {
           // ԭ�����߼�: BotVale = MidVale - 1; <-- ����ʵ�ʱ�ṹ�෴��
           TopVale = MidVale + 1; // ��������߼���Ŀ�����Ұ벿��
       }
       else // SingleVolt == OcvTable[MidVale] ��ȷƥ��
       {
           break; // �ҵ���ȷƥ��㣬����ѭ��
       }
    }
    // ѭ��������MidVale ָ��һ����ѹֵ��SingleVolt λ�� OcvTable[MidVale] �� OcvTable[MidVale-1] (���SingleVolt��)
    // ���� OcvTable[MidVale] �� OcvTable[MidVale+1] (���SingleVoltС) ֮�䣬���߾�ȷ���� OcvTable[MidVale]��
    // TopVale > BotVale ʱ�������Ĳ��������� TopVale (�� BotVale+1)��
    // ��ʱ�� MidVale ���������һ�αȽϵ��м�㣬��һ������Ѳ�ֵ�㡣
    // ��Ҫ����ȷ����ֵ���䡣ͨ�������ֲ��ҽ�����Ŀ��ֵ������ BotVale �� TopVale ֮�䡣
    // ���������� TopVale ��Ϊ��ֵ��һ����׼�㡣
    // MidVale ���ջ�ͣ�ڽӽ�Ŀ���λ�ã�����ֵ��Ҫ���������ڵ㡣

    // Ϊ�˲�ֵ��������Ҫȷ�� SingleVolt ���� OCV_Table ������������Ԫ��֮�䡣
    // ���ֲ��ҽ�����TopValeͨ��ָ����ڵ���Ŀ���ѹ����С����������BotValeָ��С�ڵ���Ŀ���ѹ�����������
    // ������Ҫ�ҵ�һ������ `idx` ʹ�� OcvTable[idx] >= SingleVolt > OcvTable[idx+1] (��Ϊ���ǽ����)
    // ���� OcvTable[idx-1] > SingleVolt >= OcvTable[idx]

    // ������ֵ�߼������ڶ��ֲ��ҵ����� TopVale (TopVale �ǵ�һ��ʹ�� OcvTable[TopVale] <= SingleVolt ������)
    // ����˵��SingleVolt ���� OcvTable[TopVale-1] �� OcvTable[TopVale] ֮�� (����TopVale > 0)

    // �߽紦��:
    if (SingleVolt >= OcvTable[0]) // �����ѹ���ڵ���100% SOC�ĵ�ѹ
    {
        return 1000; // SOCΪ100%
    }
    if (SingleVolt <= OcvTable[99]) // �����ѹС�ڵ���1% SOC�ĵ�ѹ
    {
        return 1 * 10; // SOCΪ1% (��0%�����ݶ���)
                      // ԭ��ֻ��1%�����Է���10 (1.0%)
    }

    // ���½�������������ȷ����ֵ���䣬���ֲ��ҵ�����MidVale���ܲ��ʺ�ֱ�����ڲ�ֵ
    // ���ߣ���ȷ�������ֲ��ҵĽ���������MidVale�����պ��塣
    // �������Ǽ��ʹ��ԭ����Ķ��ֲ��ҽ�����в�ֵ������׼ȷ�Դ��ɡ�
    // MidVale ��ԭ����Ķ��ֲ��Һ󣬲���ֱ������Ѳ�ֵ�㡣

    // ԭ����Ĳ�ֵ�߼� (���ڽ϶����⣬��������ֲ��ҵ� MidVale ���):
    // SocVale�ĵ�λ��0.1%
    // MidVale������� (100-MidVale)% ������SOC�㡣
    // ���� MidVale*10 ���� (100-MidVale)*10 ��0.1%��λ������MidVale=0, SOC=100%; MidVale=99, SOC=1%��

    // �������ֲ��Һ�Ĳ�ֵ�߼���
    // ���ֲ��ҽ�����SingleVolt Ӧ�ý��� OcvTable[BotVale] �� OcvTable[TopVale] ֮�䣨�����ȷƥ������ȣ���
    // ���� OCV ���ǵ�ѹ�� SOC �������ӣ�SOC��С������С��
    // ���ҽ�����ͨ�� SingleVolt ������ OcvTable[idx] �� OcvTable[idx+1] ֮�䡣
    // ���߸�׼ȷ��˵�����ѭ������������ TopVale > BotVale,
    // ��Ŀ���ѹ SingleVolt Ӧ�ý��� OcvTable[BotVale] �� OcvTable[TopVale] ֮�䡣
    // (��ʱ BotVale ָ���ѹ�ϸ�/SOC�ϸߵĵ㣬TopVale ָ���ѹ�ϵ�/SOC�ϵ͵ĵ�)
    // ������Ҫ�ҵ�һ�� `i` ʹ�� `OcvTable[i] >= SingleVolt > OcvTable[i+1]`

    // ���ø���ֱ�ӵ�����ɨ����ȷ����ֵ���䣨��ȻЧ�ʵͣ����߼�������
    // �����޸����ֲ���ʹ�䷵����ȷ�Ĳ�ֵ������
    // �������ǳ��Ի���ԭ����� MidVale ���н��ͺ����������ܶ��ֲ��Ҳ��ֱ�����ȱ�ݡ�

    // ������ֲ��Һ�MidVale ��һ����׼������
    // OCV_Table[MidVale] ���� (100-MidVale)% SOC ��Ӧ�ĵ�ѹ��

    
    // ��ͨ������ɨ���ҵ���ȷ������ (�ֲ����ֲ��ҵĲ���)
    for (index_for_soc = 0; index_for_soc < 99; index_for_soc++) {
        if (SingleVolt >= OcvTable[index_for_soc+1] && SingleVolt < OcvTable[index_for_soc]) {
            // SingleVolt �� OcvTable[index_for_soc] �� OcvTable[index_for_soc+1] ֮��
            // SOC �� (100 - index_for_soc)% �� (100 - (index_for_soc+1))% ֮��
            // �ߵ�ѹ OcvTable[index_for_soc] ��Ӧ SOC_high = (100 - index_for_soc)
            // �͵�ѹ OcvTable[index_for_soc+1] ��Ӧ SOC_low = (100 - (index_for_soc+1))
            // ��ֵ���㣺
            // Soc_actual = SOC_low + (SingleVolt - Volt_low) * (SOC_high - SOC_low) / (Volt_high - Volt_low)
            // ע������SOC_high - SOC_low = 1 (%)
            // Soc_actual(0.1%) = (100 - (index_for_soc+1))*10 + (SingleVolt - OcvTable[index_for_soc+1]) * 10 / (OcvTable[index_for_soc] - OcvTable[index_for_soc+1])
            if (OcvTable[index_for_soc] == OcvTable[index_for_soc+1]) { // �������
                 SocVale = (100 - (index_for_soc+1)) * 10; // ȡ�ϵ͵�SOC��������ƽ����
            } else {
                 SocVale = (100 - (index_for_soc+1)) * 10 +
                           (u32)(SingleVolt - OcvTable[index_for_soc+1]) * 10 /
                           (OcvTable[index_for_soc] - OcvTable[index_for_soc+1]);
            }
            return SocVale;
        }
    }
    // �����ѹ���õ���ĳ������
    for (index_for_soc = 0; index_for_soc < 100; index_for_soc++) {
        if (SingleVolt == OcvTable[index_for_soc]) {
            return (100 - index_for_soc) * 10;
        }
    }
    // ��������ѭ��û�з��أ�˵����ѹ�����˱���±߽磨����1% OCV�����ϱ߽磨����100% OCV��
    // ��ǰ��ı߽���Ӧ���Ѿ������ˡ��������������Ǹ��㾫�Ȼ��߼����⡣
    // Ϊ��ȫ���������Ҳ������䣬���Է���һ���߽�ֵ�����ֵ��
    // �������ԭ����ķ��ظ�ʽ��
    // ԭ������󷵻� 1000 - SocVale������SocVale�� MidVale*10 +/- ��ֵ��
    // ��� MidVale �� (100-SOC%)����������ô (100-MidVale) ��SOC%��
    // ��ô SocVale Ӧ���� (100-MidVale)*10 +/- ��ֵ��
    // ���ǵĲ�ֵ SocVale �Ѿ��� 0-1000 ����ʽ�ˡ�

    // BUG/�ɸĽ���6: ԭ����Ĳ�ֵ��ʽ�ͷ��� `1000 - SocVale` ���߼���Ҫ��ϸ���ӡ�
    // OcvTable[MidVale]��ѹ�ϸߣ�OcvTable[MidVale+1]��ѹ�ϵ͡�
    // ��� SingleVolt < OcvTable[MidVale] (�� > OcvTable[MidVale+1]), ˵����ѹ������֮�䣬�����ӽ� OcvTable[MidVale]��
    // ����ζ��ʵ��SOC�� (100-MidVale)% Ҫ��һ��㣬���� (100-(MidVale+1))% Ҫ�ߡ�
    // ԭ���� `SocVale=MidVale*10 + ((OcvTable[MidVale]-SingleVolt)*10) / (OcvTable[MidVale]-OcvTable[MidVale+1]);`
    // ����� MidVale*10 �ƺ��������һ����SOC������صĻ�׼��������ֱ�ӵ�SOCֵ��
    // ���� MidVale ��SOC�ٷֱȵ����� (0=100%, 99=1%)��
    // ��ô����� SingleVolt < OcvTable[MidVale] (�� > OcvTable[MidVale+1]),
    // ʵ��SOCֵӦ�ý��� (100-MidVale)% �� (100-(MidVale+1))% ֮�䡣
    // ��ʱ��(100-MidVale)�ǽϸߵ�SOCֵ��
    // ��ȷ�Ĳ�ֵӦ����:
    // SOC = (100-(MidVale+1)) + (SingleVolt - OcvTable[MidVale+1]) / (OcvTable[MidVale] - OcvTable[MidVale+1]) * 1
    // ����10�õ�0.1%��λ:
    // SocVale_0_1_percent = (100-(MidVale+1))*10 + ((SingleVolt - OcvTable[MidVale+1])*10) / (OcvTable[MidVale] - OcvTable[MidVale+1]);
    // ��ԭ���뷵�ص��� `1000-SocVale`���ⰵʾ�� `SocVale` ����Ŀ�����һ���������İٷֱȡ������Ƶ�����

    // ����ԭ����Ķ��ֲ��ҺͲ�ֵ�߼��Ƚϸ����ҿ��ܴ������⣬
    // �����Ѿ��ṩ��һ����������ɨ��ĸ������Ĳ�ֵ������
    // �����������ԭ����Ľṹ����Ҫ��������� `MidVale` �ĺ���� `SocVale` �ļ���Ŀ�ꡣ
    // Ϊ��������������ɨ��û���ҵ�������һ������MidVale�Ĵ���ֵ�����ﲻ�Ƽ��������ϡ�����ԭ���롱����
    // �������Ǽ������������ɨ���Ѿ����ء������ʧ�ܣ�����᷵��һ�����ܲ�׼ȷ��ֵ��
    if (MidVale > 0 && MidVale < 99) { // ��������Խ��
        if(SingleVolt <= OcvTable[MidVale] && SingleVolt > OcvTable[MidVale+1]) { // SingleVolt �� MidVale �� MidVale+1 ֮��
             if (OcvTable[MidVale] == OcvTable[MidVale+1]) return (100-(MidVale+1))*10;
             SocVale = (100-(MidVale+1))*10 + (u32)(SingleVolt - OcvTable[MidVale+1])*10 / (OcvTable[MidVale] - OcvTable[MidVale+1]);
        } else if (SingleVolt > OcvTable[MidVale] && SingleVolt <= OcvTable[MidVale-1]) { // SingleVolt �� MidVale-1 �� MidVale ֮��
             if (OcvTable[MidVale-1] == OcvTable[MidVale]) return (100-MidVale)*10;
             SocVale = (100-MidVale)*10 + (u32)(SingleVolt - OcvTable[MidVale])*10 / (OcvTable[MidVale-1] - OcvTable[MidVale]);
        } else { // ��ȷ���� OcvTable[MidVale]
             SocVale = (100-MidVale)*10;
        }
    } else if (MidVale == 0) { // �ӽ�100%
        SocVale = 1000;
    } else { // MidVale == 99, �ӽ�1%
        SocVale = 10;
    }
    return SocVale; // ֱ�ӷ��ؼ����SOC (0-1000)
}


/**
 * @brief  ͨ����ʱ���ַ�����SOC������������ͷſ�ʱ��SOCУ׼
 * @param  ��
 * @retval ��
 * @detail ���ݵ�ǰ�Ĺ���ģʽ����硢�ŵ硢���ã��͵���ֵ������ʣ������ (pack_rem_cap)��
 *         ����⵽�ӽ������ſ�״̬ʱ������SOCУ׼��
 *         ������ʣ��������ʵ��������������ϵͳSOC (SOCSys)��
 */
static void batt_AHinte_calcua_soc(void)
{
    u16 curr_value = 0; // ��ǰ�����ľ���ֵ (��λ��pack_rem_cap�ĵ�����λһ�£�����10mA)

    // ���ӷǳ��״̬������״̬ʱ�����֮ǰ������ǷѹУ׼ (soc_uvp_flag == FALSE)��
    // ������ʹ��ǷѹУ׼��־��������һ��Ƿѹʱ�ٴ�У׼��
    if((gBMSData.Sys_Mod.sys_mode == SYS_MODE_CHARGE) && (soc_uvp_flag == FALSE)) // SYS_MODE_CHARGE ��ʾ���ģʽ
    {
        soc_uvp_flag = TRUE; // ����ʹ��ǷѹУ׼
    }

    // ���ӳ��״̬����ǳ��״̬���ŵ���ã�ʱ�����֮ǰ�����˹�ѹУ׼ (soc_ovp_flag == FALSE)��
    // ������ʹ�ܹ�ѹУ׼��־��������һ�ι�ѹʱ�ٴ�У׼��
    if((gBMSData.Sys_Mod.sys_mode != SYS_MODE_CHARGE) && (soc_ovp_flag == FALSE))
    {
        soc_ovp_flag = TRUE; // ����ʹ�ܹ�ѹУ׼
    }

    // ����У׼�߼�:
    // ����1: ��ذ���ߵ����ѹ�ӽ���ﵽ��ѹ������ֵ (OV_Thresh - 2mV����һ������)
    // ����2: ��ǰ���ڳ��ģʽ
    // ����3: ��ѹУ׼��־ΪTRUE (��ʾ����������Խ���У׼)
    if ((gBMSData.BattPar.VoltCellMax >=(gBMSConfig.BQ76Para.OV_Thresh-2)) && // VoltCellMax: ��ߵ����ѹ
        (gBMSData.Sys_Mod.sys_mode == SYS_MODE_CHARGE) &&
        (soc_ovp_flag == TRUE))
    {
        soc_ovp_flag = FALSE; // ����������У׼�����´ηǳ��ת���ǰ�����ظ�У׼
        gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap; // ʣ��������Ϊ��ǰʵ����������
        gBMSData.BattPar.SOCSys = 1000; // SOC��Ϊ100.0%
        return; // �������У׼��ֱ�ӷ���
    }

    // �ſ�У׼�߼�:
    // ����1: ��ذ���͵����ѹ�ӽ���ﵽǷѹ������ֵ (UV_Thresh + 2mV����һ������)
    // ����2: ��ǰ�����ڳ��ģʽ (�����ڷŵ����ģʽ)
    // ����3: ǷѹУ׼��־ΪTRUE (��ʾ���ηſտ��Խ���У׼)
    if((gBMSData.BattPar.VoltCellMin <= (gBMSConfig.BQ76Para.UV_Thresh+2)) && // VoltCellMin: ��͵����ѹ
        (gBMSData.Sys_Mod.sys_mode != SYS_MODE_CHARGE) &&
        (soc_uvp_flag == TRUE))
    {
        soc_uvp_flag = FALSE; // ���ηſ���У׼�����´ηǷŵ�ת���ǰ�����ظ�У׼
        gBMSData.PackCap.pack_rem_cap = 0; // ʣ��������Ϊ0
        gBMSData.BattPar.SOCSys = 0;       // SOC��Ϊ0%
        return; // ��ɷſ�У׼��ֱ�ӷ���
    }

    // ��ʱ���ּ���:
    // ��ȡ��ǰ�����ľ���ֵ��gBMSData.BattPar.CurrLine �ĵ�λ��Ҫ�� pack_rem_cap �ĵ�λ��Ӧ��
    // ���� CurrLine ��λ��10mA����ʱ����������200ms���� curr_value ���� (ʵ�ʵ���/10mA) ��ֵ��
    // ��� CurrLine �� mA������Ҫ curr_value = abs(gBMSData.BattPar.CurrLine / 10);
    // ������� CurrLine �ĵ�λ�Ѿ��� "10mA" ���������λ�ˡ�
    curr_value = abs(gBMSData.BattPar.CurrLine);

    if (gBMSData.Sys_Mod.sys_mode == SYS_MODE_CHARGE) // ���ģʽ
    {
        // ����ʣ����������������ʵ����������
        if(gBMSData.PackCap.pack_real_cap >= (gBMSData.PackCap.pack_rem_cap + curr_value))
        {
            gBMSData.PackCap.pack_rem_cap += curr_value;
        }
        else // ���������������������������ǯλ����������
        {
            gBMSData.PackCap.pack_rem_cap = gBMSData.PackCap.pack_real_cap;
        }
    }
    else  if (gBMSData.Sys_Mod.sys_mode == SYS_MODE_DISCHARGE) // SYS_MODE_DISCHARGE ��ʾ�ŵ�ģʽ
    {
        // ����ʣ����������������0
        if(gBMSData.PackCap.pack_rem_cap >= curr_value)
        {
            gBMSData.PackCap.pack_rem_cap -= curr_value;
        }
        else // �������������0������ǯλ��0
        {
            gBMSData.PackCap.pack_rem_cap = 0;
        }
    }
    else // ����ģʽ (SYS_MODE_STANDBY �������ǳ�Ƿ�ģʽ)
    {
        // BUG/�ɸĽ���7: �����Էŵ紦��
        // ԭ������ע�͵��˾����Էŵ�Ŀ۳���
        // ʵ��Ӧ���У�Ӧ�����Էŵ��ʣ�������ʱ��۳�����������
        // �Էŵ���ͨ�����¶Ⱥ�SOC�йء�
        // ����: ����ÿСʱ�Էŵ�̶�ΪX����λ (1����λ��curr_value����С��λ)
        // if (�Էŵ��ʱ�ﵽ1Сʱ) {
        //     if(gBMSData.PackCap.pack_rem_cap >= X) gBMSData.PackCap.pack_rem_cap -= X;
        //     else gBMSData.PackCap.pack_rem_cap = 0;
        // }

        // ԭע�͵����߼� (ÿ200ms�̶��۳�1����λ��Լ10mA*200ms�������̫����):
        //if(gBMSData.PackCap.pack_rem_cap >= 1)    //��̬ʱ10mA (����1����λ��10mA*200ms)
        //{
        //     gBMSData.PackCap.pack_rem_cap -= 1;
        //}
        //else
        //{
        //     gBMSData.PackCap.pack_rem_cap = 0;
        //}
    }

    // ���ݸ��º��ʣ��������ʵ����������������ϵͳSOC
    // BUG/�ɸĽ���8: ���㱣����
    // ��� pack_real_cap ����ĳ��ԭ������δ��ʼ���������㣩��Ϊ0������ᵼ�³������
    if (gBMSData.PackCap.pack_real_cap > 0) // ���ӳ��㱣��
    {
        gBMSData.BattPar.SOCSys = (u32)gBMSData.PackCap.pack_rem_cap * 1000 / gBMSData.PackCap.pack_real_cap;
    }
    else // ���ʵ������Ϊ0��SOCҲӦΪ0 (��һ������״̬)
    {
        gBMSData.BattPar.SOCSys = 0;
    }

    // ����SOC���������� (ͨ����100.0%)
    if(gBMSData.BattPar.SOCSys > SOC_LIMIT_MAX) // SOC_LIMIT_MAX ͨ����1000
    {
        gBMSData.BattPar.SOCSys = SOC_LIMIT_MAX; // ǯλ��100.0%
    }
    // SOC�������Ѿ��ڰ�ʱ����ʱͨ�� pack_rem_cap >= 0 ��֤�ˡ�
}
