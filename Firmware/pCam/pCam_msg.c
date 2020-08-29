/*
pcam message queue implementation
*/
//
#include "mmp_lib.h"
#include "os_wrap.h"
#include "pcam_api.h"
#include "pcam_msg.h"
#include "lib_retina.h"
extern MMPF_OS_FLAGID PCAM_Flag ;
//extern MMPF_OS_SEMID  PCAM_Sem ;

/*
Initialize pcam message queue
*/
MMP_USHORT pcam_init_msg_queue(void)
{
    MMP_UBYTE err = PCAM_ERROR_NONE ;

	if( init_msg_queue(0, PCAM_Flag, PCAM_FLAG_MSG) != 0)
		return PCAM_SYS_ERR;
	init_overwr_msg_pool();
    return err;    
}

/*
Allocate pcam message block
*/
pcam_msg_t *pcam_allocate_msg( void )
{
//    MMP_UBYTE err ;
    pcam_msg_t *msg;

	msg = (pcam_msg_t *) allocate_msg(0);
    return (void *)msg ;
}

/*
Free pcam message block
*/
void pcam_free_msg(void *msg)
{
    //MMP_UBYTE err ;
	free_msg(msg, 0);
}

/*
Send pcam message,blocking.
*/
MMP_USHORT pcam_send_msg(pcam_msg_t *msg,MMP_USHORT *ret_err)
{
    MMP_UBYTE err= PCAM_ERROR_NONE;
	
	if(send_msg(msg, ret_err, 0)!=0)
		return PCAM_SYS_ERR;

    return err ;
}


/*
Post pcam message, non-blocking
*/
MMP_USHORT pcam_post_msg(pcam_msg_t *msg)
{
    MMP_UBYTE err = PCAM_ERROR_NONE;

	if(post_msg(msg, 0)!=0)
		return PCAM_SYS_ERR;

	return err;
}

/*
Get mesage block from queue
*/
pcam_msg_t *pcam_get_msg(void)
{
	return (pcam_msg_t *) get_msg(0);
}


pcam_msg_t *pcam_get_overwr_msg(MMP_ULONG msg_id,MMP_ULONG msg_sub_id)
{
	return (pcam_msg_t *)get_overwr_msg(msg_id,msg_sub_id);
}

