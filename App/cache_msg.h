

#ifndef _CACHE_MSG_H_
#define _CACHE_MSG_H_
#include "sys.h"

typedef enum 
{
	CACHE_TYPE_MSG_NONE = 0,
	CACHE_TYPE_CAN01_RCV=1,
	CACHE_TYPE_CAN02_RCV=2,
	CACHE_TYPE_CAN03_RCV=3,
	CACHE_TYPE_MSG_MAX=7
}	eCACHE_MSG_TYPE;


u16 cache_msg_init(u8 cachetype);
u16 cache_msg_write(u8 cachetype, u8 *databuf, u16 datasize);
u16 cache_msg_read(u8 cachetype, u8 *databuf, u16 datasize, u16 *pRetsize);

#endif


