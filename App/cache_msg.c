#include "cache_msg.h" // ���ܰ��� CACHE_TYPE_*, CAN_RCV_MSG_RSP, TRUE, FALSE �Ⱥ궨������Ͷ���
#include <string.h>     // ���� memset �� memcpy ����
#include "can.h"         // ���ļ���δֱ��ʹ�ã���������ģ����CAN���ܽ��� (���� CAN_RCV_MSG_RSP �ṹ��Ķ�������ڴ˻� cache_msg.h)
#include "usart.h"       // ���ļ���δֱ��ʹ�ã�����������ͨ��USART���е���/��־��¼
#include "stm32f10x.h"   // STM32F10x �ض��Ķ��壬���� __set_PRIMASK

#define CAN_CACHE_SIZE_MAX	(32) // ����ÿ��CAN����������ɵ������Ϣ���� (������2���ݣ��Ա�CAN_NEXT_DATAPOS��ͨ��λ�������ȷʵ�ֻ��λ���)


/*************************define CAN cache****************************************/
// ��̬����CAN��Ϣ�Ļ�������ÿ���������������� CAN_CACHE_SIZE_MAX ����Ϣ��
// CAN_RCV_MSG_RSP ��һ���ṹ�����ͣ����ڴ洢���յ���CAN��Ϣ�ľ������ݣ���ID, DLC, ���ݵȣ���
// ���� CAN_RCV_MSG_RSP �ṹ���СΪ16�ֽڣ���ÿ��������ռ�� 32 * 16B = 512�ֽڡ�
static CAN_RCV_MSG_RSP cache_can01_buf[CAN_CACHE_SIZE_MAX];	/*512B �ڴ�ռ� (���� CAN_RCV_MSG_RSP ��С�Ĺ���)*/
static CAN_RCV_MSG_RSP cache_can02_buf[CAN_CACHE_SIZE_MAX];	/*512B �ڴ�ռ� (���� CAN_RCV_MSG_RSP ��С�Ĺ���)*/
static CAN_RCV_MSG_RSP cache_can03_buf[CAN_CACHE_SIZE_MAX];	/*512B �ڴ�ռ� (���� CAN_RCV_MSG_RSP ��С�Ĺ���)*/

/*************************define CAN cache write/read position********************/
// ʹ�� volatile �ؼ�������Ϊ��Щ�������ܱ��жϷ������ (ISR) �޸ģ�
// ����������ѭ����ȡ����֮��������Է�ֹ���������в����Ż���ȷ��ÿ�ζ����ڴ��ж�д��Щ������
static volatile u16 can01_writepos = 0; // CAN1 �����дָ�� (��һ����д��λ�õ�����)
static volatile u16 can01_readpos = 0;  // CAN1 ����Ķ�ָ�� (��һ���ɶ�ȡλ�õ�����)
static volatile u16 can02_writepos = 0; // CAN2 �����дָ��
static volatile u16 can02_readpos = 0;  // CAN2 ����Ķ�ָ��
static volatile u16 can03_writepos = 0; // CAN3 �����дָ��
static volatile u16 can03_readpos = 0;  // CAN3 ����Ķ�ָ��

/************************define write position function**************************/
// �궨�壺����ѭ���������е���һ������λ�á�
// (x+1) ������ǰλ�á�
// &(CAN_CACHE_SIZE_MAX-1) ����λ����ʵ��ģ���㣨ȡ�ࣩ��ǰ���� CAN_CACHE_SIZE_MAX ��2���ݡ�
// ���磬��� CAN_CACHE_SIZE_MAX �� 32 (������ 100000)���� CAN_CACHE_SIZE_MAX-1 �� 31 (������ 011111)��
// (x+1) & 011111 �Ὣ��������� 0 �� 31 ֮�䣬ʵ�ֻ���Ч����
#define CAN_NEXT_DATAPOS(x)		((x+1)&(CAN_CACHE_SIZE_MAX-1))	

/**
 * @brief ��ʼ��ָ����CAN��Ϣ���档
 * @param cachetype: Ҫ��ʼ���Ļ������� (���� CACHE_TYPE_CAN01_RCV)��
 *                   ��Щ����ͨ���� cache_msg.h �ж��塣
 * @return u16: ͨ������ TRUE (ֵΪ1) ��ʾ�����˳�ʼ��������
 *              ע�⣺��ǰʵ���У���ʹ����δ����� cachetype��Ҳ���� TRUE��
 *              ������Ч�����ͣ��������ö�дָ�벢���㻺������
 */
u16 cache_msg_init(u8 cachetype)
{
	u16 ret = TRUE; // ��ʼ������ֵΪ TRUE

	// ���� cachetype ѡ��Ҫ�����Ļ���
	switch (cachetype)
	{
	case CACHE_TYPE_CAN01_RCV:
		can01_writepos = 0; // ����CAN1�����дָ��
		can01_readpos  = 0; // ����CAN1����Ķ�ָ��
		memset(cache_can01_buf, 0x00, sizeof(cache_can01_buf)); // ��CAN1������ȫ������
		break;
	case CACHE_TYPE_CAN02_RCV:
		can02_writepos = 0; // ����CAN2�����дָ��
		can02_readpos  = 0; // ����CAN2����Ķ�ָ��
		memset(cache_can02_buf, 0x00, sizeof(cache_can02_buf)); // ��CAN2������ȫ������
		break;
	case CACHE_TYPE_CAN03_RCV:
		can03_writepos = 0; // ����CAN3�����дָ��
		can03_readpos  = 0; // ����CAN3����Ķ�ָ��
		memset(cache_can03_buf, 0x00, sizeof(cache_can03_buf)); // ��CAN3������ȫ������
		break;
	case CACHE_TYPE_MSG_NONE: // �޲����Ļ�������
	case CACHE_TYPE_MSG_MAX:  // �޲����Ļ������� (��������ö�ٱ߽�)
	default:
		// ����δ��ȷ����� cachetype����ִ���κβ�����
		// ret = FALSE; // ���Կ����ڴ˴��� ret ����Ϊ FALSE����ָʾδ�ɹ���ʼ����
		break;
	}
	
	return (ret); // ���ز������
}

/**
 * @brief ������д��ָ����CAN��Ϣ���档
 *        �˺���ͨ���ڽ��յ��µ�CAN��Ϣʱ�����ã�������CAN�жϷ�������У���
 *        ������������������������������������򸲸���ɵ�δ����Ϣ��
 * @param cachetype: Ҫд��Ļ������� (���� CACHE_TYPE_CAN01_RCV)��
 * @param databuf: ָ�������д����Ϣ���ݵĻ�������ָ�롣
 *                 ������Ԥ��Ϊ CAN_RCV_MSG_RSP �ṹ�����ͻ������ڴ沼�ּ��ݡ�
 * @param datasize: databuf �����ݵĴ�С���ֽ�������
 * @return u16: ���д��ɹ��򷵻� TRUE�����򷵻� FALSE (���磬databuf Ϊ NULL �� datasize Ϊ 0)��
 */
u16 cache_msg_write(u8 cachetype, u8 *databuf, u16 datasize)
{
	u16 ret = FALSE;     // ��ʼ������ֵΪ FALSE
	u8 *pDatabuf = NULL; // ָ�򻺴�����Ŀ��洢λ�õ�ָ��
	u16 writesize = 0;   // ʵ��Ҫд����ֽ��� (���ܻ���Ŀ���λ��С���ƶ����ض�)

	// ���������������Ч�Լ��
	if ((NULL == databuf) || (0 == datasize))
	{
		return (FALSE); // ��Ч���룬д��ʧ��
	}

	/*�����ж�*/
	// �����ٽ���������ȫ���жϣ��Ա�������Ļ����������дָ�롢�������ݣ�
	// �ڲ������ʣ�������ѭ�����жϷ������ͬʱ���ʣ�ʱ������һ���ԡ�
	// __set_PRIMASK(1) ������ PRIMASK �Ĵ�������ֹ���п������жϡ�
	__set_PRIMASK(1);
	
	switch (cachetype)
	{
	case CACHE_TYPE_CAN01_RCV:
		pDatabuf = (u8 *)&cache_can01_buf[can01_writepos]; // ��ȡCAN1���浱ǰдָ��ָ��Ĳ�λ��ַ
		// ȷ��ʵ��д���С��ȡ�������ݴ�С�͵��������λ��С�еĽ�С�ߣ��Է�д��Խ�硣
		writesize = (datasize > sizeof(CAN_RCV_MSG_RSP)) ? sizeof(CAN_RCV_MSG_RSP) : datasize;
		can01_writepos = CAN_NEXT_DATAPOS(can01_writepos); // дָ��ǰ������һ��λ�ã����Σ�
		
		// �����������
		// ���дָ��ǰ�������ָ���غ� (can01_writepos == can01_readpos)��
		// ���Ҹ�λ�ã�����ɵ����ݲ�λ����������Ȼ�������� (NewData == 1, ��δ����ȡ)��
		// �򽫶�ָ��Ҳ��ǰ�ƽ�һ��λ�á����൱�ڶ��������ǣ�����ɵ�һ��δ����Ϣ��Ϊ����Ϣ�ڳ��ռ䡣
		if ( (can01_writepos == can01_readpos) && (1 == cache_can01_buf[can01_writepos].NewData) )
		{
			can01_readpos = CAN_NEXT_DATAPOS(can01_readpos); // ������ɵ���Ϣ
		}
		break;
	case CACHE_TYPE_CAN02_RCV:
		pDatabuf = (u8 *)&cache_can02_buf[can02_writepos];
		writesize = (datasize > sizeof(CAN_RCV_MSG_RSP)) ? sizeof(CAN_RCV_MSG_RSP) : datasize;
		can02_writepos = CAN_NEXT_DATAPOS(can02_writepos);
		if ( (can02_writepos == can02_readpos) && (1 == cache_can02_buf[can02_writepos].NewData) )
		{
			can02_readpos = CAN_NEXT_DATAPOS(can02_readpos);
		}
		break;
	case CACHE_TYPE_CAN03_RCV:
		pDatabuf = (u8 *)&cache_can03_buf[can03_writepos];
		writesize = (datasize > sizeof(CAN_RCV_MSG_RSP)) ? sizeof(CAN_RCV_MSG_RSP) : datasize;
		can03_writepos = CAN_NEXT_DATAPOS(can03_writepos);
		if ( (can03_writepos == can03_readpos) && (1 == cache_can03_buf[can03_writepos].NewData) )
		{
			can03_readpos = CAN_NEXT_DATAPOS(can03_readpos);
		}
		break;
	case CACHE_TYPE_MSG_NONE:
	case CACHE_TYPE_MSG_MAX:
	default:
		pDatabuf = NULL; // ����δ��������ͣ�������д�����
		break;
	}

	// ��� pDatabuf ��Ϊ NULL��˵���ҵ�����Ч��д��Ŀ���λ
	if (NULL != pDatabuf)
	{
		memcpy(pDatabuf, databuf, writesize); // ���������ݸ��Ƶ������λ��
		// ����: databuf ָ��� CAN_RCV_MSG_RSP �ṹ���е� NewData ��Ա
		// �Ѿ��������ߣ�����CAN�����жϣ�����Ϊ1����ʾ����һ�������ݡ�
		ret = TRUE; // ���д������ɹ�
	}

	/*�����ж�*/
	__set_PRIMASK(0); // �˳��ٽ������ָ�֮ǰ���ж�״̬��ʹ��ȫ���жϣ���
	return (ret);	
}

/**
 * @brief ��ָ����CAN��Ϣ�����ж�ȡһ����Ϣ��
 *        �˺���ͨ������ѭ���б����ã����ڴ������е���Ϣ��
 * @param cachetype: Ҫ��ȡ�Ļ������� (���� CACHE_TYPE_CAN01_RCV)��
 * @param databuf: ָ���û��ṩ�Ļ�������ָ�룬��ȡ������Ϣ���ݽ����Ƶ��˻�������
 *                 �˻������Ĵ�С��������Ϊ `datasize`��
 * @param datasize: ������ȡ����Ϣ��С���˺���Ҫ�� `datasize` ���뾫ȷƥ��
 *                  �����е�����Ϣ�ṹ�� `sizeof(CAN_RCV_MSG_RSP)` �Ĵ�С��
 * @param pRetsize: ָ�� u16 ���ͱ�����ָ�룬������ͨ����ָ�뷵��ʵ�ʶ�ȡ�����ֽ�����
 * @return u16: ����ɹ���ȡ��һ����Ϣ���򷵻� TRUE�����򷵻� FALSE (���绺��Ϊ�ա�
 *              ����ָ��ΪNULL���� `datasize` ��ƥ��)��
 */
u16 cache_msg_read(u8 cachetype, u8 *databuf, u16 datasize, u16 *pRetsize)
{
  u16 ret = FALSE;    // ��ʼ������ֵΪ FALSE
	u8 *pSrcData;       // ָ�򻺴���Դ���ݣ�����ȡ��Ϣ����ָ��
	u16 readsize = 0;   // ʵ�ʶ�ȡ���ֽ���

	// ���������������Ч�Լ�� (��Ҫ����������)
	if ((NULL == databuf) || (NULL == pRetsize))
	{
		return (FALSE); // ��Ч���룬��ȡʧ��
	}

	/*�����ж�*/
	// �����ٽ�������������Ļ��������״̬��
	__set_PRIMASK(1); 
	*pRetsize = 0;  // ��ʼ��ʵ�ʶ�ȡ�ֽ���Ϊ0
	pSrcData = NULL; // ��ʼ��Դ����ָ��ΪNULL

	switch (cachetype)
	{
	case CACHE_TYPE_CAN01_RCV:
		// ���������
		// 1. ��ǰ��ָ��ָ��Ĳ�λ�Ƿ��������� (cache_can01_buf[can01_readpos].NewData == 1)��
		//    NewData ��־�� CAN_RCV_MSG_RSP �ṹ�嶨�壬���ڱ�������Ƿ�δ����ȡ��
		// 2. ������������ȡ�����ݴ�С (datasize) �Ƿ��뻺���е�����Ϣ�ṹ��Ĵ�С��ȫһ�¡�
		if ( (1 == cache_can01_buf[can01_readpos].NewData) && 
		     (sizeof(CAN_RCV_MSG_RSP) == datasize) )
		{
			readsize = datasize; // ����Ҫ��ȡ���ֽ���
			*pRetsize = readsize; // ͨ��ָ�뷵��ʵ�ʶ�ȡ���ֽ���
			pSrcData = (u8 *)&cache_can01_buf[can01_readpos]; // ��ȡָ�򻺴��и���Ϣ���ݵ�ָ��
			cache_can01_buf[can01_readpos].NewData = 0x00; // �ؼ������ò�λ�� NewData ��־���㣬��ʾ�����ѱ���ȡ��
			can01_readpos = CAN_NEXT_DATAPOS(can01_readpos); // ��ָ��ǰ������һ��λ�ã����Σ�
		}
		break;
	case CACHE_TYPE_CAN02_RCV:
		if ( (1 == cache_can02_buf[can02_readpos].NewData) && 
		     (sizeof(CAN_RCV_MSG_RSP) == datasize) )
		{
			readsize = datasize;
			*pRetsize = readsize;
			pSrcData = (u8 *)&cache_can02_buf[can02_readpos];
			cache_can02_buf[can02_readpos].NewData = 0x00;
			can02_readpos = CAN_NEXT_DATAPOS(can02_readpos);
		}
		break;
	case CACHE_TYPE_CAN03_RCV:
		if ( (1 == cache_can03_buf[can03_readpos].NewData) && 
		     (sizeof(CAN_RCV_MSG_RSP) == datasize) )
		{
			readsize = datasize;
			*pRetsize = readsize;
			pSrcData = (u8 *)&cache_can03_buf[can03_readpos];
			cache_can03_buf[can03_readpos].NewData = 0x00;
			can03_readpos = CAN_NEXT_DATAPOS(can03_readpos);
		}
		break;
	case CACHE_TYPE_MSG_NONE:
	case CACHE_TYPE_MSG_MAX:
	default:
		// ����δ��������ͣ���ִ�ж�ȡ������pSrcData ����Ϊ NULL
		break;
	}

	// ��� pSrcData ��Ϊ NULL��˵���ɹ��ҵ��˿ɶ�ȡ����Ϣ
	if (NULL != pSrcData)
	{
		memcpy(databuf, pSrcData, readsize); // �������е���Ϣ���ݸ��Ƶ��û��ṩ�Ļ�����
		ret = TRUE; // ��Ƕ�ȡ�����ɹ�
	}

	/*�����ж�*/
	__set_PRIMASK(0); // �˳��ٽ������ָ��жϡ�

	return (ret); // ���ض�ȡ������״̬
}
