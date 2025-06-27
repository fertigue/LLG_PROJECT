#ifndef __FLASH_H
#define __FLASH_H
#include "sys.h"


#define STM32_FLASH_SIZE 64 	 		   //��ѡSTM32��FLASH������С(��λΪK) CPU��STM32F103R8T6  64k Flash  ��64ҳ  ÿҳ1Kbyt
#define STM32_FLASH_WREN 1           //ʹ��FLASHд��(0��������;1��ʹ��)
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
#define FLASH_SAVE_ADDR  0x0800fc00 	//����FLASH �����ַ(����Ϊż��) �ӵ�63K��FLASHд��
#define STM_SECTOR_SIZE 1024 //�ֽ�



void STMFLASH_Write(u32 WriteAddr,u16 NumToWrite,u16 *pBuffer);		  //��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr, u16 NumToRead, u16 *pBuffer);  	//��ָ����ַ��ʼ����ָ�����ȵ�����












#endif










