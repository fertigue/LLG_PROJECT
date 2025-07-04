#ifndef __FLASH_H
#define __FLASH_H
#include "sys.h"


#define STM32_FLASH_SIZE 64 	 		   //所选STM32的FLASH容量大小(单位为K) CPU是STM32F103R8T6  64k Flash  共64页  每页1Kbyt
#define STM32_FLASH_WREN 1           //使能FLASH写入(0，不是能;1，使能)
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
#define FLASH_SAVE_ADDR  0x0800fc00 	//设置FLASH 保存地址(必须为偶数) 从第63K的FLASH写入
#define STM_SECTOR_SIZE 1024 //字节



void STMFLASH_Write(u32 WriteAddr,u16 NumToWrite,u16 *pBuffer);		  //从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr, u16 NumToRead, u16 *pBuffer);  	//从指定地址开始读出指定长度的数据












#endif










