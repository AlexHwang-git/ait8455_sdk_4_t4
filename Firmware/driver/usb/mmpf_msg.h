#ifndef _MSG_H
#define _MSG_H
#include "os_wrap.h"

#define MSG_BLOCKING       (0) // Blocking call, can't call by ISR
#define MSG_NONBLOCKING    (1) // Non-blocking call, can call by ISR
#define MSG_API            (2) // Function call directly
#define MSG_OVERWR         (3) // Non-blocking , message block will be overwritten by next msg

#define TASK_CRITICAL(statement)    \
do {    \
    OSSchedLock();  \
    statement       \
    OSSchedUnlock();\
} while (0);    

enum {
	SYS_MSG_USB_EP0_TX_INT=0,
	SYS_MSG_USB_EP1_TX_INT,
	SYS_MSG_USB_EP2_TX_INT,
	SYS_MSG_USB_EP3_TX_INT,
	SYS_MSG_USB_EP5_TX_INT,
	SYS_MSG_USB_EP0_RX_INT,
	SYS_MSG_USB_EP1_RX_INT,
	SYS_MSG_USB_EP2_RX_INT,
	SYS_MSG_USB_EP5_RX_INT,
	SYS_MSG_USB_RESET_INT,
	SYS_MSG_USB_SUSPEND_INT,
	SYS_MSG_USB_DMA_EP0_TX_INT,	
	SYS_MSG_USB_DMA_EP1_TX_INT,	
	SYS_MSG_USB_DMA_EP2_TX_INT,	
	SYS_MSG_USB_DMA_EP3_TX_INT,	
	SYS_MSG_USB_DMA_EP0_RX_INT,	
	SYS_MSG_USB_DMA_EP1_RX_INT,	
	SYS_MSG_USB_DMA_EP2_RX_INT,	
	SYS_MSG_USB_DMA_EP3_RX_INT,
#if H264_SIMULCAST_EN
    SYS_MSG_USB_H264_RT_FR_SWITCH,
    SYS_MSG_USB_RESTART_PREVIEW,
#endif 	
    SYS_MSG_USB_RESTART_SENSOR
};

enum {
	USBCTL_MSG_START_PREVIEW=0,
	USBCTL_MSG_STOP_PREVIEW,
	USBCTL_MSG_OTHERS
};
typedef struct _msg_t
{
    MMP_ULONG src_id ;
    MMP_ULONG dst_id ;
    MMP_ULONG msg_id ;
    MMP_ULONG msg_sub_id ; 
    void      *msg_data ;
    MMPF_OS_SEMID *msg_sem;
    MMP_ULONG err ;
} msg_t ;


typedef struct _msg_queue_t
{
    MMP_USHORT rd_i ;
    MMP_USHORT wr_i ;
    MMP_USHORT msg_len;
    MMP_ULONG  *msg_q ;
} msg_queue_t ;
 
#define L1_MSG_SIZE sizeof(msg_t)
#define L1_MSG_Q_SIZE   (20)

#define MSG_QUEUE_NUMS 3
/* PCAM Task use index 0,
   SYS Task use index 1
   USB Control Task 2 */
   
enum {
	MSG_ERROR_NONE = 0,
	MSG_INIT_ERR,
	MSG_CREATE_SEM_ERR,
	MSG_POST_ERR,
	MSG_WAIT_SEM_TIMEOUT,
	MSG_ACQUIRE_SEM_ERR,
	MSG_QUEUE_FULL_ERR
	
};

#define MAX_OVERWR_MSG_NUM  64
typedef struct _overwr_msg_pool_t 
{
    MMP_ULONG msg_cnt ;
    MMP_BOOL  msg_id_enable[MAX_OVERWR_MSG_NUM];
    msg_t     overwr_msg_blk[MAX_OVERWR_MSG_NUM];
} overwr_msg_pool_t ;

msg_t *allocate_msg( int id );
msg_t *get_msg(int id);
msg_t *get_overwr_msg(MMP_ULONG msg_id,MMP_ULONG msg_sub_id);
void free_msg(void *msg, int id);
MMP_USHORT send_msg(msg_t *msg,MMP_USHORT *ret_err, int id);
MMP_USHORT post_msg(msg_t *msg, int id);
MMP_USHORT init_msg_queue(int id, MMPF_OS_FLAGID os_id, MMP_ULONG flags);
MMP_USHORT init_overwr_msg_pool(void);
#endif
