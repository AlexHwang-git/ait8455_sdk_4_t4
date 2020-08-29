/*
usb_ctl message queue implementation
*/
//
#include "mmp_lib.h"
#include "os_wrap.h"
#include "mmpf_hif.h"
#include "mmpf_msg.h"
#include "lib_retina.h"


static void msg_queue_init(msg_queue_t *queue,void *q_addr,MMP_USHORT q_size);
static MMP_USHORT msg_queue_is_full(msg_queue_t *queue);
static void msg_queue_put(msg_queue_t *queue,msg_t *msg);
static MMP_USHORT msg_queue_is_empty(msg_queue_t *queue);
static msg_t *msg_queue_get(msg_queue_t *queue);


//static msg_t        overwr_msg_blk[MSG_QUEUE_NUMS];
static msg_t        msg_data[MSG_QUEUE_NUMS][L1_MSG_Q_SIZE] ;
static msg_queue_t  msg_queue[MSG_QUEUE_NUMS];

static OS_MEM  				*msg_mem_handle[MSG_QUEUE_NUMS] = { (OS_MEM *)0,(OS_MEM *)0,  (OS_MEM *)0 }  ;
static MMPF_OS_FLAGID msg_os_flags[MSG_QUEUE_NUMS];
static MMP_ULONG 			msg_task_flags[MSG_QUEUE_NUMS];
static MMP_ULONG 			msg_buffer[MSG_QUEUE_NUMS][L1_MSG_Q_SIZE+1] ;
static MMP_USHORT 		msg_timeout = 10 * 1000 ; /*10 seconds timeout*/
static MMPF_OS_SEMID  sem[MSG_QUEUE_NUMS] = { 0xFF , 0xFF, 0xFF };
static overwr_msg_pool_t overwr_msg_pool;

static MMP_ULONG cpu_sr ;

MMP_USHORT init_overwr_msg_pool(void)
{
    MMP_USHORT i;
    msg_t *msg;
    overwr_msg_pool.msg_cnt = 0;
    for(i=0;i<MAX_OVERWR_MSG_NUM;i++) {
        overwr_msg_pool.msg_id_enable[i] = MMP_FALSE;
        msg = &overwr_msg_pool.overwr_msg_blk[i];
        if(msg) {
            msg->src_id = (MMP_ULONG)-1;
            msg->dst_id = (MMP_ULONG)-1;
            msg->msg_id = (MMP_ULONG)-1;
            msg->msg_sub_id = (MMP_ULONG)-1;
            msg->msg_data = (void *)0;
            msg->msg_sem = (MMPF_OS_SEMID *)0;
            msg->err =  MSG_ERROR_NONE ;
        }
    }
    return MSG_ERROR_NONE ;
}

/*
Initialize message queue
*/
MMP_USHORT init_msg_queue(int id, MMPF_OS_FLAGID os_id, MMP_ULONG flags)
{
    MMP_UBYTE err = MSG_ERROR_NONE ;
    int i=0;
    
    msg_mem_handle[id] = OSMemCreate((void *)msg_data[id],L1_MSG_Q_SIZE,L1_MSG_SIZE,&err);
    if(err) {
      return MSG_INIT_ERR;
    }
    msg_queue_init(&msg_queue[id],(void *)msg_buffer[id],L1_MSG_Q_SIZE+1);
    
    msg_os_flags[id] = os_id;
    msg_task_flags[id] = flags;
	
	
    return err;
}

/*
Allocate usb_ctl message block
*/
msg_t *allocate_msg( int id )
{
    MMP_UBYTE err ;
    msg_t *msg=NULL;
    msg = (msg_t *)OSMemGet(msg_mem_handle[id],&err);

    if( err == OS_NO_ERR ) {
        msg->src_id = (MMP_ULONG)-1;
        msg->dst_id = (MMP_ULONG)-1;
        msg->msg_id = (MMP_ULONG)-1;
        msg->msg_sub_id = (MMP_ULONG)-1;
        msg->msg_data = (void *)0;
        msg->msg_sem = (MMPF_OS_SEMID *)0;
        msg->err =  MSG_ERROR_NONE ;
    }
    return (void *)msg ;
}

/*
Free usb_ctl message block
*/
void free_msg(void *msg, int id)
{
    MMP_UBYTE err ;
	if( (((MMP_ULONG) msg) - (MMP_ULONG)&(msg_data[id]) ) <=  ((L1_MSG_Q_SIZE-1)*L1_MSG_SIZE)  )
    err = OSMemPut(msg_mem_handle[id],msg);  
}

/*
Send usb_ctl message,blocking.
*/
MMP_USHORT send_msg(msg_t *msg,MMP_USHORT *ret_err, int id)
{
    MMP_UBYTE err;
    
    /*
    Create semaphore
    */
	sem[id] = MMPF_OS_CreateSem(0);
	if(sem[id]==0xFF) {
        RTNA_DBG_Str(3,"<<msg create sem err>>\r\n");
        return MSG_CREATE_SEM_ERR ;
    }  
    /*
    Pass semaphore to handler for release
    */
    msg->msg_sem = (MMPF_OS_SEMID *)&sem[id] ;
    
    err = post_msg(msg, id) ;
    
    if(err) {
        MMPF_OS_DeleteSem(sem[id]);
        return MSG_POST_ERR ;
    }    
    /*
    Wait 10 seconds for semaphore released (MMPF_Task())
    */
    err = MMPF_OS_AcquireSem(sem[id],msg_timeout);
    if(err) {
        RTNA_DBG_Str(3,"<<usb_ctl get sem err,msg id:");
        RTNA_DBG_Short(3,msg->msg_id);
        RTNA_DBG_Str(3,",");
        RTNA_DBG_Short(3,msg->msg_sub_id);
        RTNA_DBG_Str(3,">>\r\n");
        MMPF_OS_DeleteSem(sem[id]);
        
        if(err==OS_TIMEOUT) {
            return MSG_WAIT_SEM_TIMEOUT ;
        }
        return MSG_ACQUIRE_SEM_ERR ;
    }
    *ret_err = msg->err ;
    /*
    Delete semaphore
    */
    MMPF_OS_DeleteSem(sem[id]);
    return MSG_ERROR_NONE ;
}


/*
Post usb_ctl message, non-blocking
*/
MMP_USHORT post_msg(msg_t *msg, int id)
{
    MMP_UBYTE err ;
    if(msg_queue_is_full(&msg_queue[id])) {
        RTNA_DBG_Str(3,"MSD ID:");
        RTNA_DBG_Long(3, id);
        RTNA_DBG_Str(3," queue is full\r\n");
        return MSG_QUEUE_FULL_ERR ;
    }        
    msg_queue_put(&msg_queue[id],msg);

	err = MMPF_OS_SetFlags(msg_os_flags[id],msg_task_flags[id], MMPF_OS_FLAG_SET);
    if(err) {
        dbg_printf(3,"postmsgerr:%d\r\n",err);
    }
    return err ;
}

/*
Get mesage block from queue
*/
msg_t *get_msg(int id)
{
    if(msg_queue_is_empty(&msg_queue[id])) {
        return (msg_t *)0;
    }
    return msg_queue_get(&msg_queue[id]) ;
}


msg_t *get_overwr_msg(MMP_ULONG msg_id,MMP_ULONG msg_sub_id)
{
    msg_t *msg ;
    MMP_USHORT i;
    MMP_ULONG cpu_sr;
    MMP_BOOL  set = MMP_TRUE ;
    MMP_SHORT msg_addr = -1 ;
    if(msg_id==(MMP_ULONG)-1) {
        for(i=0;i<MAX_OVERWR_MSG_NUM;i++) {
            if(overwr_msg_pool.msg_id_enable[i]==TRUE) {
                msg_addr = i ;
                msg = &overwr_msg_pool.overwr_msg_blk[msg_addr];    
                set = MMP_FALSE ;
                //dbg_printf(0,"get owr_msg:[%d,%d]\r\n",msg->msg_id,msg->msg_sub_id);
                goto exit ;

            }
        }
        return (msg_t *)0;
    }
    else {
        for(i=0;i<MAX_OVERWR_MSG_NUM;i++) {
            msg = &overwr_msg_pool.overwr_msg_blk[i];
            if( (msg->msg_id==msg_id) && (msg->msg_sub_id==msg_sub_id) ) {
                msg_addr = i ;
                //dbg_printf(0,"put owr_msg:[%d,%d]\r\n",msg_id,msg_sub_id);
                goto exit;
            }
        }
        if(overwr_msg_pool.msg_cnt >= MAX_OVERWR_MSG_NUM) {
            return 0;
        } 
        else {
            msg = &overwr_msg_pool.overwr_msg_blk[overwr_msg_pool.msg_cnt] ;
            msg_addr = overwr_msg_pool.msg_cnt ;
            overwr_msg_pool.msg_cnt++;
        }
    }
exit:
    IRQ_LOCK(
        overwr_msg_pool.msg_id_enable[msg_addr] = set ; 
    )
    return msg;
}



static void msg_queue_init(msg_queue_t *queue,void *q_addr,MMP_USHORT q_size)
{
    MMP_USHORT i;
    queue->rd_i = 0;
    queue->wr_i = 0;
    queue->msg_len = q_size ;
    queue->msg_q = (MMP_ULONG *)q_addr ;
    for(i=0;i<queue->msg_len;i++) {
        queue->msg_q[i] = 0 ;
    }
}

/*
Test if queue is full or not
*/
static MMP_USHORT msg_queue_is_full(msg_queue_t *queue)
{
    MMP_USHORT ret = 0;
    MMP_USHORT len ;
    
    IRQ_LOCK(
        len = queue->rd_i ;
        if(len==0) len = queue->msg_len ;
        if((len - queue->wr_i)==1) {
            ret = 1;
        }
    )
    
    return ret ;    
}

/*
Test if queue is empty or not
*/
static MMP_USHORT msg_queue_is_empty(msg_queue_t *queue)
{
    MMP_USHORT ret = 0;
    
    IRQ_LOCK(
        if(queue->rd_i == queue->wr_i) {
            ret = 1 ;
        }
    )
    return ret;
}

/*
Enqueue message
*/
static void msg_queue_put(msg_queue_t *queue,msg_t *msg)
{
    IRQ_LOCK(
        queue->msg_q[queue->wr_i] = (MMP_ULONG)msg ;
        if(queue->wr_i >= (queue->msg_len-1)) {
            queue->wr_i = 0 ;
        } else {
            queue->wr_i++;
        }      
    )
}

/*
Dequeue message
*/
static  msg_t *msg_queue_get(msg_queue_t *queue)
{
    msg_t *msg ;
    IRQ_LOCK(
        msg = (msg_t *)queue->msg_q[queue->rd_i] ;
        queue->rd_i++;
        if(queue->rd_i >= queue->msg_len) {
            queue->rd_i-=queue->msg_len ;
        }
    )
    return msg;
}
