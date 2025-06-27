#include "adc.h"

static u16 ADC_ConvertedValue[5]={0};
static u8 TemNA=6;

//--------20,-16,-12,-8,-4,0,4,8, 100------------//
const static unsigned int TemD[31]=
{
2890,2817,2735,2642,2541,2431,2318,2202,2127,2007,  
1846,1687,1546,1422,1309,1202,1099,1001,907,817,
727,677,613,550,496,452,414,379,345,311,277 
};



void ADC_Init_Sam(void)
{
    ADC_InitTypeDef ADC_InitStructure;	
    /* ADC1 Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    /* ADC1 DeInit */ 
    ADC_DeInit(ADC1);
    /* Initialize ADC structure */
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //����ת������
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 5;     //����ת�����г���Ϊ2
    ADC_Init(ADC1, &ADC_InitStructure);

		RCC_ADCCLKConfig(RCC_PCLK2_Div4); // 72/2   
    //����ת������1��ͨ��10    ����ʱ��>1.6us,(7cycles)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_7Cycles5);
    //����ת������2��ͨ��11   
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_7Cycles5);
    //����ת������1��ͨ��12
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_7Cycles5);
    //����ת������2��ͨ��14   
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 4, ADC_SampleTime_7Cycles5);
    //����ת������2��ͨ��15  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 5, ADC_SampleTime_7Cycles5); 
    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);
    // ����ADC��DMA֧�֣�Ҫʵ��DMA���ܣ������������DMAͨ���Ȳ�����
    ADC_DMACmd(ADC1, ENABLE);
    // ������ADC�Զ�У׼����������ִ��һ�Σ���֤����
    // Enable ADC1 reset calibaration register 
    ADC_ResetCalibration(ADC1);
    // Check the end of ADC1 reset calibration register
    while(ADC_GetResetCalibrationStatus(ADC1));
    
    // Start ADC1 calibaration
    ADC_StartCalibration(ADC1);
    // Check the end of ADC1 calibration
    while(ADC_GetCalibrationStatus(ADC1));
    // ADC�Զ�У׼����---------------
    //������һ��ADת��
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
    //��Ϊ�Ѿ����ú���DMA��������AD�Զ�����ת��������Զ�������RegularConvData_Tab��       
}


void DMA_Init_Adc(void)
{
    DMA_InitTypeDef DMA_InitStructure;
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
 	
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr =ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr =(unsigned int)&ADC_ConvertedValue;	 
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;   
    //BufferSize=2����ΪADCת��������2��ͨ��  
    //������ã�ʹ����1�������RegularConvData_Tab[0]������2�������RegularConvData_Tab[1]
    DMA_InitStructure.DMA_BufferSize =5;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    //ѭ��ģʽ������Bufferд�����Զ��ص���ʼ��ַ��ʼ����
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    //������ɺ�����DMAͨ��
    DMA_Cmd(DMA1_Channel1, ENABLE);
}


void ADC_GetSample(void)
{
  
	

}


//-----------------------------------------------------------------------------
signed int TemChange(unsigned int uiADCV)
{	
	unsigned char ucA=32,TemNK=0;
	signed int uiD=0;
	TemNK=TemNA;
	if(uiADCV<TemD[TemNK])			//
	{	while(--ucA)
		{	if(TemNK<30)
			{	TemNK++;
				if(uiADCV>=TemD[TemNK])
				{	uiD=-200+TemNK*(int)40;
					uiD=uiD-((int)40)*(uiADCV-TemD[TemNK])/(TemD[TemNK-1]-TemD[TemNK]);
					break;
				}
			}
			else 
			{	uiD=1000;		//100?
				break;
			}
		}
	}
	else if(uiADCV>=TemD[TemNK-1])			
	{	while(--ucA)
		{	if(TemNK>1)
			{	TemNK--;
				if(uiADCV<TemD[TemNK-1])
				{	uiD=-200+TemNK*(int)40;
					uiD=uiD-((int)40)*(uiADCV-TemD[TemNK])/(TemD[TemNK-1]-TemD[TemNK]);
					break;
				}
			}
			else 
			{	uiD=-200;		//-30?
				break;
			}
		}
	}
	else
	{	uiD=-200+TemNK*(int)40;
		uiD=uiD-((int)40)*(uiADCV-TemD[TemNK])/(TemD[TemNK-1]-TemD[TemNK]);
	}
	TemNA=TemNK;
	
	if(uiD&0x8000) 
	{   
		 uiD&=~0x8000;
		 uiD=0x8000-uiD;
		 uiD=uiD*90/100;
		 uiD&=0x7fff;
		 //uiD=0x8000-uiD;
		 uiD|=0x8000;
	}
	else 
	{
		uiD=uiD*90/100;
	}

	return uiD;
}



























