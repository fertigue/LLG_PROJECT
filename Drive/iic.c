#include "iic.h"
#include "gpio.h"
#include "TaskFun.h"
#include "timer.h"
#include "adc.h"
#include "Flash.h"

 
//***************全局变量******************//
unsigned char  HW_VERSION[17]={"软件版本:1.1    \0"};
unsigned char  SW_VERSION[17]={"硬件版本:1.1    \0"};
unsigned char  DE_VERSION[17]={"设备版本:2.0    \0"};
//**************全局变量END****************//


//***************局部变量******************//


//***************局部变量END***************//



void Sda_Set_Out_Mode(void)
{
	//IIC_DATA   
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Sda_Set_In_Mode(void)
{
	//IIC_DATA   
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}


unsigned char IICCheckAck(void) 
{   unsigned char ret=gRET_OK;
    unsigned char cnt=0;
    
    Sda_Set_In_Mode();
    delay_us(2);
    SCK_OUT_HIGH;
    delay_us(2);
    while(SDA_IN_DATA==1)
    {
        cnt++;
        if(cnt>250)
        {
          return  gRET_NG; 
        }
    }
    SCK_OUT_LOW;
    return  ret; 
}                

void IICSendAck(void)    //????????
{   
    SDA_OUT_LOW; 
    Sda_Set_Out_Mode();
    SCK_OUT_HIGH;
    delay_us(2);
    SCK_OUT_LOW;
    delay_us(2);
    SDA_OUT_HIGH;            
}

void IICSendNoAck(void)    //????????
{   
    SDA_OUT_HIGH; 
    Sda_Set_Out_Mode();
    SCK_OUT_HIGH;
    delay_us(2);
    SCK_OUT_LOW;
    delay_us(2);
    SDA_OUT_HIGH;             
}

unsigned char IICStart(void)    //??????
{    unsigned char ret=gRET_OK;
     SDA_OUT_HIGH; 
	   SCK_OUT_HIGH;
     Sda_Set_Out_Mode();
	   delay_us(2);
		 SDA_OUT_LOW;
	   delay_us(2);
     SCK_OUT_LOW;
     return  ret;
}

void IICStop(void)    //??????
{      
     SCK_OUT_LOW; 
	   SDA_OUT_LOW;
     Sda_Set_Out_Mode();
	   delay_us(2);
     SCK_OUT_HIGH;
     delay_us(2);
     SDA_OUT_HIGH;              
}


unsigned char IICSendByte(unsigned char Data) 
{
   unsigned char ret=gRET_OK;
   unsigned char  ia=0;
   for(ia=0;ia<8;ia++) 
   {
       if(Data&0x80) 
       {
          SDA_OUT_HIGH;
       } 
       else 
       {  SDA_OUT_LOW;
       }
       Sda_Set_Out_Mode();
       delay_us(2);
       SCK_OUT_HIGH; 
       delay_us(2);
       SCK_OUT_LOW; 
       
       Data=Data<<1;
   }
   
   if(IICCheckAck()!= gRET_OK) 
   {
       return gRET_NG;     //??????
   } 
   return  ret; 
}

unsigned char IICReceByte(void) 
{
     //unsigned char ret=gRET_OK;
     unsigned char  ia=0;
     unsigned char  readDATA=0;
     Sda_Set_In_Mode();
     for(ia=0;ia<8;ia++) 
     {
        SCK_OUT_HIGH;
        delay_us(2);
        
        readDATA=readDATA<<1;
        if(SDA_IN_DATA)
        {
            readDATA|=0x01;
        } 
        else 
        {
             readDATA&=~0x01;
        }
        SCK_OUT_LOW; 
        delay_us(2);
     }
     return  readDATA;
}





u8 IIC_CRC8Bytes(u8 *ptr, u8 len,u8 key)
{
    u8 i;          
    u8 crc=0;      
    while(len--!=0)           
    {
        for(i=0x80; i!=0; i/=2)    
        {
            if((crc & 0x80) != 0)    
            {
                    crc *= 2;      
                    crc ^= key;    
            }
            else
                    crc *= 2;

            if((*ptr & i)!=0)
                    crc ^= key;
        }
        ptr++;
    }
    return(crc);
}

unsigned char IIC_WritByte(unsigned char SloveAddr,  unsigned char RegAddr,  unsigned char Data) 
{   
     unsigned char  ret=gRET_OK;
     //unsigned char  CRC=0;
     unsigned char  rcd=0;
     unsigned char  BuffData[3]={0};
		 unsigned char  crc=0;

     IICStart();

     rcd=IICSendByte((SloveAddr<<1)&0xFE);
     BuffData[0]=((SloveAddr<<1)&0xFE);
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }

     rcd=IICSendByte(RegAddr);
     BuffData[1]=RegAddr;
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     rcd=IICSendByte(Data);
     BuffData[2]=Data;
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     crc=IIC_CRC8Bytes(BuffData,3,0x07);
     rcd=IICSendByte(crc);
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }

     IICStop();
     
     return ret;
}



unsigned char  IIC_ReadByte(unsigned char SloveAddr,  unsigned char RegAddr , unsigned char* ReadData )
{    
    unsigned char ret=gRET_OK;
    unsigned int  rcd=0;
    unsigned char BuffData[3]={0};
    unsigned char crc=0;

     IICStart();                        
     rcd=IICSendByte((SloveAddr<<1)&0xFE);   
     delay_us(5); 
     rcd=IICSendByte(RegAddr);               
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     IICStart();     
     rcd=IICSendByte((SloveAddr<<1)|0x01);   
     BuffData[0]=(SloveAddr<<1)|0x01;
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     BuffData[1]=IICReceByte();
     IICSendAck();
     delay_us(5); 
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     BuffData[2]=IICReceByte();
     IICSendNoAck();
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     IICStop();
     
     crc=IIC_CRC8Bytes(BuffData, 2,0x07);
     if(BuffData[2]!=crc)
     {
        return  gRET_NG;
     }
     
     *ReadData=BuffData[1];
     return   ret ;
}


unsigned char IIC_WritByteMore(unsigned char SloveAddr,  unsigned char RegAddr,  unsigned char *DataSend, unsigned char Long ) 
{    
     unsigned char  ret=gRET_OK;
     unsigned char  crc=0,ia=0;
     unsigned char  BuffData[3]={0};
     unsigned char  rcd=0;
     
     IICStart();
     rcd=IICSendByte((SloveAddr<<1)&0xFE);
     BuffData[0]=SloveAddr<<1;
     delay_us(5);
     if(rcd!=gRET_OK )
    {
        return gRET_NG;
     }
     
     rcd=IICSendByte(RegAddr);
     BuffData[1]=RegAddr;
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     rcd=IICSendByte(DataSend[0]);
     BuffData[2]=DataSend[0];
     delay_us(5);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     crc=IIC_CRC8Bytes(BuffData,3,0x07);
     rcd=IICSendByte(crc);
     delay_us(5);
     if(rcd!=gRET_OK )
     {
       return gRET_NG;
     }
     
     for(ia=1;ia<Long;ia++)
     {  
        rcd=IICSendByte(DataSend[ia]);
        crc=IIC_CRC8Bytes(&DataSend[ia],1,0x07);
        rcd=IICSendByte(crc);
        delay_us(5);
       if(rcd!=gRET_OK )
       {
          return gRET_NG;
       }
     }
     IICStop();
     
     return   ret ;
}


unsigned char  IIC_ReadByteMore(unsigned char SloveAddr,  unsigned char RegAddr,unsigned char *DataRece, unsigned char Long )
{   
		unsigned char ret=gRET_OK;
		unsigned int  rcd=0;
		unsigned int  ia=0;
		unsigned char  crc=0;
		unsigned char  DataRead[111]={0};

     IICStart();                        
     rcd=IICSendByte((SloveAddr<<1)&0xFE);         
     delay_us(5); 
     rcd=IICSendByte(RegAddr);               
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     
     IICStart();   
     rcd=IICSendByte((SloveAddr<<1)|0x01);        
     DataRead[0]=((SloveAddr<<1)|0x01);
     if(rcd!=gRET_OK )
     {
        return gRET_NG;
     }
     delay_us(5); 
     
     for(ia=0;ia<Long;ia++)
     {
         DataRead[ia*2+1]=IICReceByte();
         IICSendAck();
         delay_us(5); 
         
         DataRead[ia*2+2]=IICReceByte();
         if(ia==(Long-1))
         {
             IICSendNoAck();
         }
         else
         {
             IICSendAck();
         }
        delay_us(5);      

     }
     IICStop();

     crc=IIC_CRC8Bytes(DataRead, 2,0x07); 
     if(DataRead[2]!=crc)
     {
        ret=gRET_NG;
     }
     
     for(ia=1;ia<Long;ia++)
     {
        crc=IIC_CRC8Bytes(&DataRead[ia*2+1], 1,0x07); 
        if(DataRead[ia*2+2]!=crc)
        {
           ret=gRET_NG;
        }
     } 
     
     if(ret==gRET_OK)
     {  
        for(ia=0;ia<Long;ia++)
        {
             DataRece[ia]=DataRead[ia*2+1];
        }
     }
     return   ret;
}


