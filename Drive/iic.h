

#ifndef __ICC_H
#define __ICC_H	 
#include "sys.h"


#define  SDA_IN_DATA       PBin(7)
#define  SDA_OUT_HIGH      PBout(7)=1
#define  SDA_OUT_LOW       PBout(7)=0  

#define  SCK_OUT_HIGH      PBout(6)=1
#define  SCK_OUT_LOW       PBout(6)=0

void Sda_Set_Out_Mode(void);
void Sda_Set_In_Mode(void);
u8 IIC_Check_Ack(void); 
void IIC_Send_Ack(void); 
void IIC_Send_NoAck(void); 
void IIC_Start(void); 
void IIC_Stop(void);  
u8 IIC_Send_Byte(u8 Data);
u8 IIC_Rece_Byte(void);
u8  IIC_WritByte(u8 SloveAddr,u8 RegAddr, u8 Data);
u8  IIC_ReadByte(u8 SloveAddr,  u8 RegAddr , u8* ReadData );
u8  IIC_WritByteMore(u8 SloveAddr,  u8 RegAddr,  u8 *DataSend, u8 Long ); 
u8  IIC_ReadByteMore(u8 SloveAddr,  u8 RegAddr,u8 *DataRece, u8 Long );			


#endif
