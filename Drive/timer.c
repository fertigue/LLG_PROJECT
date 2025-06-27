

#include "timer.h"
#include "gpio.h"
#include "usart.h"
#include "led.h"

void  Tim1_Init(u16 arr,u16 psc)
{ 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	
 
  TIM_TimeBaseStructure.TIM_Period = arr;                    //    从装比较最大   18k  =   72000/ 6000= 12k 
  TIM_TimeBaseStructure.TIM_Prescaler = psc;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;       //  模式  增
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;                 //  分频
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;      
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_ClearFlag(TIM1, TIM_FLAG_Update);  
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清中断标志位
  TIM_ITConfig(TIM1,TIM_IT_Update ,ENABLE); //打开中断 
  TIM_Cmd(TIM1, ENABLE);//计数开始               
	
	//配置TIM1的更新中断使能
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//指定抢占优先级1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//指定响应优先级0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
 
void TIM1_UP_IRQHandler(void)// 触发ADC中断采样和电机环路控制  0.1MS
{
	  //此程序更新中断一直执行
		if(TIM_GetITStatus(TIM1, TIM_IT_Update)!=RESET)       
    {   
				TIM_ClearFlag(TIM1, TIM_IT_Update); 
				
				TaskTimePare.Tim10ms_count++;	
		}
}

void Systim_Time_Run(void)
{
		if(TaskTimePare.Tim10ms_count>=10)
    {          
				TaskTimePare.Tim10ms_count=0;
				TaskTimePare.Tim10ms_flag = 1;
				if(++TaskTimePare.Tim100ms_count >=10)
				{
						TaskTimePare.Tim100ms_count=0;
						TaskTimePare.Tim100ms_flag=1;
				}
    }
		if(TaskTimePare.Tim100ms_flag==1)
		{
				if(++TaskTimePare.Tim200ms_count >=2)
				{	      
						TaskTimePare.Tim200ms_count=0;
						TaskTimePare.Tim200ms_flag=1;
				}
		}
		
    if(TaskTimePare.Tim100ms_flag==1)
		{		   		 
				if(++TaskTimePare.Tim500ms_count >=5)
				{	      
						TaskTimePare.Tim500ms_count=0;
						TaskTimePare.Tim500ms_flag=1;
				}
		}
		if(TaskTimePare.Tim500ms_flag==1)
		{            
				if(++TaskTimePare.Tim1s_count >=2)
				{	               
					 TaskTimePare.Tim1s_count=0;
					 TaskTimePare.Tim1s_flag=1;
				}
		}
		if(TaskTimePare.Tim1s_flag == 1)
		{      
				if(++TaskTimePare.Tim10s_count >=10)
				{
						TaskTimePare.Tim10s_count = 0;
						TaskTimePare.Tim10s_flag = 1;
				}
		} 
		if(TaskTimePare.Tim10s_flag == 1)
		{                   
				if(++TaskTimePare.Tim1min_count >=6)
				{
						TaskTimePare.Tim1min_count = 0;
						TaskTimePare.Tim1min_flag = 1;
				}
		}	 
}

void Clear_flag(void)   //清除事件标志位
{
	TaskTimePare.Tim10ms_flag=0;
	TaskTimePare.Tim100ms_flag=0;
	TaskTimePare.Tim200ms_flag=0;
	TaskTimePare.Tim500ms_flag=0;
	TaskTimePare.Tim1s_flag=0;
	TaskTimePare.Tim10s_flag=0;
	TaskTimePare.Tim1min_flag=0;
}

/*
void delay_us(unsigned int num)
{
	unsigned int ia=0;
	for(ia=0;ia<num;ia++ )
	{
					__NOP();
				  __NOP();
					__NOP();
				  __NOP();
					__NOP();
					__NOP();
					__NOP();
					__NOP();
	}
}

void delay_ms(unsigned int num)
{
	unsigned int ia=0;
	for(ia=0;ia<num;ia++ )
	{
			delay_us(1000);
	}
}

*/





