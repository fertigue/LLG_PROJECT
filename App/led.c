#include "led.h"
#include "timer.h"
#include "DataBase.h"

u8 Led_Run_Cpu(void)
{
		u8 ret = gRET_OK;
		u8 soc_current = 0;
		static u8 soc_flag = 0;
		static u8 soc_last = 0;
		
		if(TaskTimePare.Tim500ms_flag != 1)
		{
				return  gRET_NG;
		}
		
		LedRun_Set_Not();
		soc_current = (gBMSData.BattPar.SOCSys + 5) / 10;
		if(soc_current != soc_last)
		{
				if(soc_current >= 75)
				{
						LedBuzz_SetPort(LED_R, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_G, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_Y, GPIO_DATA_HIGH);
						LedBuzz_SetPort(BUZZER, GPIO_DATA_HIGH);
				}
				else if(soc_current >= 50)
				{
						LedBuzz_SetPort(LED_R, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_G, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_Y, GPIO_DATA_HIGH);
						LedBuzz_SetPort(BUZZER, GPIO_DATA_LOW);  
				}
				else if(soc_current >= 25)
				{
						LedBuzz_SetPort(LED_R, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_G, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_Y, GPIO_DATA_LOW);
						LedBuzz_SetPort(BUZZER, GPIO_DATA_LOW);
				}
				else if(soc_current >= 5)
				{
						LedBuzz_SetPort(LED_R, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_G, GPIO_DATA_LOW);
						LedBuzz_SetPort(LED_Y, GPIO_DATA_LOW);
						LedBuzz_SetPort(BUZZER, GPIO_DATA_LOW);
				}
				else 
				{
						LedBuzz_SetPort(LED_R, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_G, GPIO_DATA_HIGH);
						LedBuzz_SetPort(LED_Y, GPIO_DATA_HIGH);
						if(soc_flag == 1)
						{
								soc_flag = 0;
								LedBuzz_SetPort(BUZZER, GPIO_DATA_HIGH);
						}
						else 
						{
								soc_flag = 1;
								LedBuzz_SetPort(BUZZER, GPIO_DATA_LOW);
						}
				}
				soc_last = soc_current;
		}
		return  ret;
}

void LedRun_Set_On(void)
{
   PBout(14)=0;
}

void LedRun_Set_Off(void)
{
	 PBout(14)=1;
}

void LedRun_Set_Not(void)
{
		//u8 gpio_vlue=0;
		PBout(14)=~PBout(14);
}


u16 LedBuzz_SetPort(u8 led_ch, u8 value)
{
		if(led_ch>=LED_MAX)
		{
				return gRET_NG;
		}
    switch(led_ch)
		{
				case LED_R:
					if(value==GPIO_DATA_LOW)
					{
						  GPIO_ResetBits(GPIOC,GPIO_Pin_13); 
					}
					else
					{
							GPIO_SetBits(GPIOC,GPIO_Pin_13); 
					}
					break;
				case LED_G:
					if(value==GPIO_DATA_LOW)
					{
							  GPIO_ResetBits(GPIOC,GPIO_Pin_14); 
					}
					else
					{
							 GPIO_SetBits(GPIOC,GPIO_Pin_14); 
					}
					break;
			case LED_Y:
					if(value==GPIO_DATA_LOW)
					{
						   GPIO_ResetBits(GPIOC,GPIO_Pin_15); 
					}
					else
					{
							 GPIO_SetBits(GPIOC,GPIO_Pin_15); 
					}
					break;
			case BUZZER:
					if(value==GPIO_DATA_LOW)
					{
								GPIO_ResetBits(GPIOA,GPIO_Pin_0); 
					}
					else
					{
								GPIO_SetBits(GPIOA,GPIO_Pin_0); 
					}
					break;	
      default:
        break;
		}
		return gRET_OK;
}

u16 OutGpio_SetPort(u8 out_ch, u8 value)
{
	  if(out_ch>=OUT_MAX)
		{
				return gRET_NG;
		}
    switch(out_ch)
		{
				case OUT1_GPIO:
					if(value==GPIO_DATA_LOW)
					{
							GPIO_ResetBits(GPIOA,GPIO_Pin_4); 
					}
					else
					{
							GPIO_SetBits(GPIOA,GPIO_Pin_4); 
					}
					break;
				case OUT2_GPIO:
				
					break;
			case OUT3_GPIO:
					
					break;
      default:
        break;
		}
		return gRET_OK;
}

u16 InGpio_GetPort(u8 in_ch, u8* value)
{
	  u8 read_gpio=0;
		if(in_ch>=OUT_MAX)
		{
				return gRET_NG;
		}
    switch(in_ch)
		{
			case IN1_GPIO:
					*value=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
					break;
			case IN2_GPIO:
				
					break;
			case IN3_GPIO:
					
					break;
      default:
        break;
		}
		return read_gpio;
}

void LEDgpio_test(void)
{
	static u8  FANflag=0; 

	if(TaskTimePare.Tim200ms_flag==1)
	{
		if(FANflag==0)
		{
			FANflag=1;
			LedBuzz_SetPort(LED_R, GPIO_DATA_HIGH);
			LedBuzz_SetPort(LED_G, GPIO_DATA_HIGH);
			LedBuzz_SetPort(LED_Y, GPIO_DATA_HIGH);
			LedBuzz_SetPort(BUZZER, GPIO_DATA_HIGH);
			OutGpio_SetPort(OUT1_GPIO, GPIO_DATA_HIGH);
    }
		else
		{ 
			FANflag=0;
			LedBuzz_SetPort(LED_R, GPIO_DATA_LOW);
			LedBuzz_SetPort(LED_G, GPIO_DATA_LOW);
			LedBuzz_SetPort(LED_Y, GPIO_DATA_LOW);
			LedBuzz_SetPort(BUZZER, GPIO_DATA_LOW);
			OutGpio_SetPort(OUT1_GPIO, GPIO_DATA_LOW);
		}
		InGpio_GetPort(IN1_GPIO, &gBMSData.Gpio_State.IN0_State);
		
	}
}









