#include "stm32f10x.h"
#include "key.h"
#include "bq769xx.h"
#include "timer.h"
KEY_Pare    KEYPare;
 
//***************全局变量***************//

//**************全局变量END***************//
 

//***************局部变量***************//

//***************局部变量END*************//



void Key_Can(void)
{
	//*******************************************
	if(TaskTimePare.Tim10ms_flag==1)
	{
			KEYPare.Key_num[0]= GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
			if(KEYPare.Key_num[0]==0)		 
			{  
				if(KEYPare.Key_count[0]<200 ){KEYPare.Key_count[0]++;}
				else { KEYPare.Key_count[0]=200;} 
					
				if(( KEYPare.Key_count[0] >=2)&&( KEYPare.Key_count[0] <200))
				{
					KEYPare.Key_switch[0]=1;  
				}
				else if( KEYPare.Key_count[0] ==200)
				{
					    Power_Down();
							KEYPare.KeyDataK|=0x0100;	
				}
				else
				{	KEYPare.Key_switch[0]=0;
				}
			}
			else
			{  if( KEYPare.Key_count[0] !=0)
				 {	 KEYPare.Key_count[0] =0;
						 if(KEYPare.Key_switch[0]==1)
						 {	KEYPare.KeyDataK|=0x01;
								KEYPare.Key_switch[0]=0;
						 }
				 }
			}
	}
}












