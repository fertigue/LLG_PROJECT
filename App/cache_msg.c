#include "cache_msg.h" // 可能包含 CACHE_TYPE_*, CAN_RCV_MSG_RSP, TRUE, FALSE 等宏定义和类型定义
#include <string.h>     // 用于 memset 和 memcpy 函数
#include "can.h"         // 此文件中未直接使用，但表明此模块与CAN功能交互 (例如 CAN_RCV_MSG_RSP 结构体的定义可能在此或 cache_msg.h)
#include "usart.h"       // 此文件中未直接使用，但表明可能通过USART进行调试/日志记录
#include "stm32f10x.h"   // STM32F10x 特定的定义，例如 __set_PRIMASK

#define CAN_CACHE_SIZE_MAX	(32) // 定义每个CAN缓存可以容纳的最大消息数量 (必须是2的幂，以便CAN_NEXT_DATAPOS宏通过位与操作正确实现环形缓冲)


/*************************define CAN cache****************************************/
// 静态分配CAN消息的缓存区。每个缓存区可以容纳 CAN_CACHE_SIZE_MAX 条消息。
// CAN_RCV_MSG_RSP 是一个结构体类型，用于存储接收到的CAN消息的具体内容（如ID, DLC, 数据等）。
// 假设 CAN_RCV_MSG_RSP 结构体大小为16字节，则每个缓存区占用 32 * 16B = 512字节。
static CAN_RCV_MSG_RSP cache_can01_buf[CAN_CACHE_SIZE_MAX];	/*512B 内存空间 (基于 CAN_RCV_MSG_RSP 大小的估算)*/
static CAN_RCV_MSG_RSP cache_can02_buf[CAN_CACHE_SIZE_MAX];	/*512B 内存空间 (基于 CAN_RCV_MSG_RSP 大小的估算)*/
static CAN_RCV_MSG_RSP cache_can03_buf[CAN_CACHE_SIZE_MAX];	/*512B 内存空间 (基于 CAN_RCV_MSG_RSP 大小的估算)*/

/*************************define CAN cache write/read position********************/
// 使用 volatile 关键字是因为这些变量可能被中断服务程序 (ISR) 修改，
// 并被主程序循环读取（或反之）。这可以防止编译器进行不当优化，确保每次都从内存中读写这些变量。
static volatile u16 can01_writepos = 0; // CAN1 缓存的写指针 (下一个可写入位置的索引)
static volatile u16 can01_readpos = 0;  // CAN1 缓存的读指针 (下一个可读取位置的索引)
static volatile u16 can02_writepos = 0; // CAN2 缓存的写指针
static volatile u16 can02_readpos = 0;  // CAN2 缓存的读指针
static volatile u16 can03_writepos = 0; // CAN3 缓存的写指针
static volatile u16 can03_readpos = 0;  // CAN3 缓存的读指针

/************************define write position function**************************/
// 宏定义：计算循环缓冲区中的下一个数据位置。
// (x+1) 递增当前位置。
// &(CAN_CACHE_SIZE_MAX-1) 利用位掩码实现模运算（取余），前提是 CAN_CACHE_SIZE_MAX 是2的幂。
// 例如，如果 CAN_CACHE_SIZE_MAX 是 32 (二进制 100000)，则 CAN_CACHE_SIZE_MAX-1 是 31 (二进制 011111)。
// (x+1) & 011111 会将结果限制在 0 到 31 之间，实现环绕效果。
#define CAN_NEXT_DATAPOS(x)		((x+1)&(CAN_CACHE_SIZE_MAX-1))	

/**
 * @brief 初始化指定的CAN消息缓存。
 * @param cachetype: 要初始化的缓存类型 (例如 CACHE_TYPE_CAN01_RCV)。
 *                   这些类型通常在 cache_msg.h 中定义。
 * @return u16: 通常返回 TRUE (值为1) 表示尝试了初始化操作。
 *              注意：当前实现中，即使对于未处理的 cachetype，也返回 TRUE。
 *              对于有效的类型，它会重置读写指针并清零缓存区。
 */
u16 cache_msg_init(u8 cachetype)
{
	u16 ret = TRUE; // 初始化返回值为 TRUE

	// 根据 cachetype 选择要操作的缓存
	switch (cachetype)
	{
	case CACHE_TYPE_CAN01_RCV:
		can01_writepos = 0; // 重置CAN1缓存的写指针
		can01_readpos  = 0; // 重置CAN1缓存的读指针
		memset(cache_can01_buf, 0x00, sizeof(cache_can01_buf)); // 将CAN1缓存区全部清零
		break;
	case CACHE_TYPE_CAN02_RCV:
		can02_writepos = 0; // 重置CAN2缓存的写指针
		can02_readpos  = 0; // 重置CAN2缓存的读指针
		memset(cache_can02_buf, 0x00, sizeof(cache_can02_buf)); // 将CAN2缓存区全部清零
		break;
	case CACHE_TYPE_CAN03_RCV:
		can03_writepos = 0; // 重置CAN3缓存的写指针
		can03_readpos  = 0; // 重置CAN3缓存的读指针
		memset(cache_can03_buf, 0x00, sizeof(cache_can03_buf)); // 将CAN3缓存区全部清零
		break;
	case CACHE_TYPE_MSG_NONE: // 无操作的缓存类型
	case CACHE_TYPE_MSG_MAX:  // 无操作的缓存类型 (可能用作枚举边界)
	default:
		// 对于未明确处理的 cachetype，不执行任何操作。
		// ret = FALSE; // 可以考虑在此处将 ret 设置为 FALSE，以指示未成功初始化。
		break;
	}
	
	return (ret); // 返回操作结果
}

/**
 * @brief 将数据写入指定的CAN消息缓存。
 *        此函数通常在接收到新的CAN消息时被调用（例如在CAN中断服务程序中）。
 *        它处理缓冲区溢出的情况：如果缓冲区已满，则覆盖最旧的未读消息。
 * @param cachetype: 要写入的缓存类型 (例如 CACHE_TYPE_CAN01_RCV)。
 * @param databuf: 指向包含待写入消息数据的缓冲区的指针。
 *                 此数据预期为 CAN_RCV_MSG_RSP 结构体类型或与其内存布局兼容。
 * @param datasize: databuf 中数据的大小（字节数）。
 * @return u16: 如果写入成功则返回 TRUE，否则返回 FALSE (例如，databuf 为 NULL 或 datasize 为 0)。
 */
u16 cache_msg_write(u8 cachetype, u8 *databuf, u16 datasize)
{
	u16 ret = FALSE;     // 初始化返回值为 FALSE
	u8 *pDatabuf = NULL; // 指向缓存区中目标存储位置的指针
	u16 writesize = 0;   // 实际要写入的字节数 (可能会因目标槽位大小限制而被截断)

	// 基本的输入参数有效性检查
	if ((NULL == databuf) || (0 == datasize))
	{
		return (FALSE); // 无效输入，写入失败
	}

	/*禁用中断*/
	// 进入临界区：禁用全局中断，以保护共享的缓存变量（读写指针、缓存内容）
	// 在并发访问（例如主循环和中断服务程序同时访问）时的数据一致性。
	// __set_PRIMASK(1) 会设置 PRIMASK 寄存器，禁止所有可屏蔽中断。
	__set_PRIMASK(1);
	
	switch (cachetype)
	{
	case CACHE_TYPE_CAN01_RCV:
		pDatabuf = (u8 *)&cache_can01_buf[can01_writepos]; // 获取CAN1缓存当前写指针指向的槽位地址
		// 确定实际写入大小：取输入数据大小和单个缓存槽位大小中的较小者，以防写入越界。
		writesize = (datasize > sizeof(CAN_RCV_MSG_RSP)) ? sizeof(CAN_RCV_MSG_RSP) : datasize;
		can01_writepos = CAN_NEXT_DATAPOS(can01_writepos); // 写指针前进到下一个位置（环形）
		
		// 缓存溢出处理：
		// 如果写指针前进后与读指针重合 (can01_writepos == can01_readpos)，
		// 并且该位置（即最旧的数据槽位）的数据仍然是新数据 (NewData == 1, 即未被读取)，
		// 则将读指针也向前推进一个位置。这相当于丢弃（覆盖）了最旧的一条未读消息，为新消息腾出空间。
		if ( (can01_writepos == can01_readpos) && (1 == cache_can01_buf[can01_writepos].NewData) )
		{
			can01_readpos = CAN_NEXT_DATAPOS(can01_readpos); // 丢弃最旧的消息
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
		pDatabuf = NULL; // 对于未处理的类型，不进行写入操作
		break;
	}

	// 如果 pDatabuf 不为 NULL，说明找到了有效的写入目标槽位
	if (NULL != pDatabuf)
	{
		memcpy(pDatabuf, databuf, writesize); // 将输入数据复制到缓存槽位中
		// 假设: databuf 指向的 CAN_RCV_MSG_RSP 结构体中的 NewData 成员
		// 已经被调用者（例如CAN接收中断）设置为1，表示这是一条新数据。
		ret = TRUE; // 标记写入操作成功
	}

	/*启用中断*/
	__set_PRIMASK(0); // 退出临界区：恢复之前的中断状态（使能全局中断）。
	return (ret);	
}

/**
 * @brief 从指定的CAN消息缓存中读取一条消息。
 *        此函数通常在主循环中被调用，用于处理缓存中的消息。
 * @param cachetype: 要读取的缓存类型 (例如 CACHE_TYPE_CAN01_RCV)。
 * @param databuf: 指向用户提供的缓冲区的指针，读取到的消息数据将复制到此缓冲区。
 *                 此缓冲区的大小必须至少为 `datasize`。
 * @param datasize: 期望读取的消息大小。此函数要求 `datasize` 必须精确匹配
 *                  缓存中单个消息结构体 `sizeof(CAN_RCV_MSG_RSP)` 的大小。
 * @param pRetsize: 指向 u16 类型变量的指针，函数将通过此指针返回实际读取到的字节数。
 * @return u16: 如果成功读取到一条消息，则返回 TRUE；否则返回 FALSE (例如缓存为空、
 *              输入指针为NULL、或 `datasize` 不匹配)。
 */
u16 cache_msg_read(u8 cachetype, u8 *databuf, u16 datasize, u16 *pRetsize)
{
  u16 ret = FALSE;    // 初始化返回值为 FALSE
	u8 *pSrcData;       // 指向缓存中源数据（待读取消息）的指针
	u16 readsize = 0;   // 实际读取的字节数

	// 基本的输入参数有效性检查 (主要针对输出参数)
	if ((NULL == databuf) || (NULL == pRetsize))
	{
		return (FALSE); // 无效输入，读取失败
	}

	/*禁用中断*/
	// 进入临界区，保护共享的缓存变量和状态。
	__set_PRIMASK(1); 
	*pRetsize = 0;  // 初始化实际读取字节数为0
	pSrcData = NULL; // 初始化源数据指针为NULL

	switch (cachetype)
	{
	case CACHE_TYPE_CAN01_RCV:
		// 检查条件：
		// 1. 当前读指针指向的槽位是否有新数据 (cache_can01_buf[can01_readpos].NewData == 1)。
		//    NewData 标志由 CAN_RCV_MSG_RSP 结构体定义，用于标记数据是否未被读取。
		// 2. 调用者期望读取的数据大小 (datasize) 是否与缓存中单个消息结构体的大小完全一致。
		if ( (1 == cache_can01_buf[can01_readpos].NewData) && 
		     (sizeof(CAN_RCV_MSG_RSP) == datasize) )
		{
			readsize = datasize; // 设置要读取的字节数
			*pRetsize = readsize; // 通过指针返回实际读取的字节数
			pSrcData = (u8 *)&cache_can01_buf[can01_readpos]; // 获取指向缓存中该消息数据的指针
			cache_can01_buf[can01_readpos].NewData = 0x00; // 关键：将该槽位的 NewData 标志清零，表示数据已被读取。
			can01_readpos = CAN_NEXT_DATAPOS(can01_readpos); // 读指针前进到下一个位置（环形）
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
		// 对于未处理的类型，不执行读取操作，pSrcData 保持为 NULL
		break;
	}

	// 如果 pSrcData 不为 NULL，说明成功找到了可读取的消息
	if (NULL != pSrcData)
	{
		memcpy(databuf, pSrcData, readsize); // 将缓存中的消息数据复制到用户提供的缓冲区
		ret = TRUE; // 标记读取操作成功
	}

	/*启用中断*/
	__set_PRIMASK(0); // 退出临界区，恢复中断。

	return (ret); // 返回读取操作的状态
}
