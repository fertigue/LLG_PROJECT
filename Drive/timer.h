#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"



typedef struct {	
				
        uint8_t   Tim10ms_count; 	// 10msʱ�ӱ�־
        uint8_t   Tim10ms_flag;   // 10ms��־
        uint8_t   Tim100ms_count; //100ms����
	      uint8_t   Tim100ms_flag;  //100ms�¼���־λ
        uint8_t   Tim200ms_flag;  //200ms�¼���־λ
	      uint8_t   Tim200ms_count; //200ms����
        uint8_t   Tim500ms_count; //500ms����
        uint8_t   Tim500ms_flag; //500ms�¼���־λ
        uint8_t   Tim1s_count;  //1s����
        uint8_t   Tim1s_flag ;  //1s�¼���־λ
        uint8_t   Tim10s_count; //10s����
        uint8_t   Tim10s_flag ; //10s�¼���־λ
        uint8_t   Tim1min_count; //1����
        uint8_t   Tim1min_flag ; //1�����¼���־λ
	   }TaskTime;

#define  TaskTime_DEFAULTS  {0,0,0,0,0,0,0,0,0,0,0,0,0,0}  // ��ʼ������
extern  TaskTime       TaskTimePare;  

//*******************ȫ�ֱ���******************//




//*******************************************//
void  Tim1_Init(u16 arr,u16 psc);
void Systim_Time_Run(void);
void Clear_flag(void);   //����¼���־λ


#endif
