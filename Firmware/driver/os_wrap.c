//==============================================================================
//
//  File        : os_wrap.c
//  Description : OS wrapper functions
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================


#include "includes_fw.h"
#include "os_wrap.h"
#include "reg_retina.h"
#include "lib_retina.h"

/** @addtogroup MMPF_OS
@{
*/

//------------------------------
// User Specified Configuration
//------------------------------

OS_EVENT                *os_sem_tbl[MMPF_OS_SEMID_MAX];
OS_FLAG_GRP             *os_flag_tbl[MMPF_OS_FLAGID_MAX];
// Log the created tasks' prio for USB enum purpose
MMP_UBYTE               os_created_task_prio[OS_MAX_TASKS];
MMP_UBYTE               os_created_tasks ;
extern MMP_ULONG		glHostStatus;


//==============================================================================
//
//                              OS Wrap Functions
//
//==============================================================================
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
/** @brief To do os wrapper layer initialization process, but be called before
any other OS related API.

@return None.
*/
void MMPF_OS_Initialize(void)
{
	MMP_ULONG   i;

    OSInit();

    for (i = 0; i < MMPF_OS_SEMID_MAX; i++) {
        os_sem_tbl[i] = 0x0;
    }

    for (i = 0; i < MMPF_OS_FLAGID_MAX; i++) {
        os_flag_tbl[i] = 0x0;
    }
    os_created_tasks = 0 ;
    for (i = 0; i < OS_MAX_TASKS; i++) {
        os_created_task_prio[i] = 0; 
    }
//    for (i = 0; i < MMPF_OS_MQID_MAX; i++) {
//        os_mq_tbl[i] = 0x0;
//    }
    glHostStatus = 0x0;

}
//------------------------------------------------------------------------------
//  Function    : MMPF_OS_StartTask
//  Description :
//------------------------------------------------------------------------------
/** @brief To start the OS multi-task working.

@return None.
*/
void MMPF_OS_StartTask(void)
{
    OSStart();
}
//------------------------------------------------------------------------------
//  Function    : MMPF_OS_CreateTask
//  Description :
//------------------------------------------------------------------------------
/** @brief This function is used to have OS manage the execution of a task.  Tasks can either
              be created prior to the start of multitasking or by a running task.  A task cannot be
              created by an ISR.

@param[in] taskproc : the task function entry pointer
@param[in] task_cfg : pointert to @ref MMPF_TASK_CFG structure, that deal with stack top/bottom address and task priority.
@param[in] param : input paramter to the taskproc
@retval 0xFE for bad input priority
		0xFF for createtask internal error from OS
		others, return the priority of the task.
*/
MMPF_OS_TASKID  MMPF_OS_CreateTask(void (*taskproc)(void *param), MMPF_TASK_CFG *task_cfg,
                    void *param)
{
    MMP_UBYTE ret;

    if (task_cfg->ubPriority > OS_LOWEST_PRIO) {
        return 0xFE;
    }

    ret = OSTaskCreateExt(taskproc,
                    param,
                    (OS_STK  *)(task_cfg->ptos),
                    task_cfg->ubPriority,
                    task_cfg->ubPriority,
                    (OS_STK  *)(task_cfg->pbos),
                    (task_cfg->ptos - task_cfg->pbos) / sizeof(OS_STK) + 1,
                    (void *) 0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    if (OS_NO_ERR != ret) {
        return 0xFF; //[TBD]
    }
    os_created_task_prio[os_created_tasks++] = task_cfg->ubPriority ;
    return task_cfg->ubPriority;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_OS_CreateSem
//  Description :
//------------------------------------------------------------------------------
/** @brief This function creates a semaphore.

@param[in] ubSemValue : is the initial value for the semaphore.  If the value is 0, no resource is
                        available.  You initialize the semaphore to a non-zero value to specify how many resources are available.
@retval 0xFF for create semaphore internal error from OS
		0xFE the system maximum semaphore counts exceed.
		others, the ID to access the semaphore
*/
MMPF_OS_SEMID MMPF_OS_CreateSem(MMP_UBYTE ubSemValue)
{
    MMP_ULONG   i;
    MMP_UBYTE   ret;
    OS_EVENT    *ev;

    ev = OSSemCreate(ubSemValue);

    if (!ev) {
        while(1) { dbg_printf(3,"sem create failed\r\n"); }
        return 0xFF;
    }

    for (i = 0; i < MMPF_OS_SEMID_MAX; i++) {
        if (os_sem_tbl[i] == 0x0) {
            os_sem_tbl[i] = ev;                            
            return i;
        }
    }
    OSSemDel(ev, OS_DEL_ALWAYS, &ret);
    return 0xFE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_OS_DeleteSem
//  Description :
//------------------------------------------------------------------------------
/** @brief This function deletes a semaphore and readies all tasks pending on the semaphore.

@param[in] ulSemId : The semaphore ID that return by @ref MMPF_OS_CreateSem 
@retval 0xFF for delete semaphore internal error from OS
		0, return delete success.
*/

MMP_UBYTE  MMPF_OS_DeleteSem(MMPF_OS_SEMID ulSemId)
{
    MMP_UBYTE   ret;
    OS_EVENT    *ev;

    if (0x0 != os_sem_tbl[ulSemId]) {
        ev = os_sem_tbl[ulSemId];
        OSSemDel(ev, OS_DEL_ALWAYS, &ret);
        if (ret == OS_NO_ERR) {
            os_sem_tbl[ulSemId] = 0x0;
        }
        else {
            MMPF_DBG_Int(1, ulSemId);
            RTNA_DBG_Str(1, " , ");
            RTNA_DBG_Long(1, ret);
            RTNA_DBG_Str(1, ": MMPF_OS_DeleteSem() fail\r\n");
        }
        return ret;
    }

    return 0;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_OS_AcquireSem
//  Description :
//------------------------------------------------------------------------------
/** @brief This function waits for a semaphore.

@param[in] ulSemId : The semaphore ID that return by @ref MMPF_OS_CreateSem 
@param[in] ulTimeout : is an optional timeout period (in clock ticks).  If non-zero, your task will
                            wait for the resource up to the amount of time specified by this argument.
                            If you specify 0, however, your task will wait forever at the specified
                            semaphore or, until the resource becomes available.
@retval please ref to ucos_ii.h
*/

MMP_UBYTE MMPF_OS_AcquireSem(MMPF_OS_SEMID ulSemId, MMP_ULONG ulTimeout)
{
    MMP_UBYTE       ret;

    if (ulSemId >= MMPF_OS_SEMID_MAX) {
        return 0xFE;
    }

    OSSemPend(os_sem_tbl[ulSemId], ulTimeout, &ret);
#if SUPPORT_GRA_ZOOM    
    if(ret && (ret!=2))
#else    
    if(ret)
#endif
    {    
    /*
        RTNA_DBG_Str(3,"<<OSSemPend err:");
        RTNA_DBG_Byte(3,ret);
        RTNA_DBG_Str(3,">>\r\n");
        RTNA_DBG_Str(3,"<<sem.id:");
        RTNA_DBG_Long(3,ulSemId);
        RTNA_DBG_Str(3,">>\r\n");
    */        
    
    }
    return ret ;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_OS_ReleaseSem
//  Description :
//------------------------------------------------------------------------------
/** @brief This function signals a semaphore

@param[in] ulSemId : The semaphore ID that return by @ref MMPF_OS_CreateSem 
@retval 0xFE for bad input semaphore id,
		0xFF for release semaphore internal error from OS
		0 for getting the resource.
		1 If the semaphore count exceeded its limit.
*/
MMP_UBYTE MMPF_OS_ReleaseSem(MMPF_OS_SEMID ulSemId)
{
    MMP_UBYTE       ret;

    if (ulSemId >= MMPF_OS_SEMID_MAX) {
        return 0xFE;
    }

    ret = OSSemPost(os_sem_tbl[ulSemId]);

    if (ret) {
        RTNA_DBG_Str(3,"<<OSSemPost err:");
        RTNA_DBG_Byte(3,ret);
        RTNA_DBG_Str(3,">>\r\n");
    }
    return ret; 
}

//------------------------------------------------------------------------------
//  Function    : MMPF_OS_AcceptSem
//  Description :
//------------------------------------------------------------------------------
/** @brief This function requests for a semaphore.

@param[in] ulSemId : The semaphore ID that return by @ref MMPF_OS_CreateSem 
@param[out] usCount : The return value
@retval:
    If >0, semaphore value is decremented; value is returned before the decrement.
    If 0, then either resource is unavailable, event did not occur, or null or invalid pointer was passed to the function. 
*/
MMP_UBYTE MMPF_OS_AcceptSem(MMPF_OS_SEMID ulSemId, MMP_USHORT *usCount)
{

    if (ulSemId >= MMPF_OS_SEMID_MAX) {
        return 0xFE;
    }

    *usCount = OSSemAccept(os_sem_tbl[ulSemId]);
        
    return 0;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_OS_QuerySem
//  Description :
//------------------------------------------------------------------------------
/** @brief This function obtains count about a semaphore

@param[in] ulSemId : The semaphore ID that return by @ref MMPF_OS_CreateSem 
@param[out] usCount : The count of the ulSemId
@retval 0xFE for bad input semaphore id,
		0xFF for query semaphore internal error from OS
		0 for no error
*/
MMP_UBYTE MMPF_OS_QuerySem(MMPF_OS_SEMID ulSemId, MMP_USHORT *usCount)
{
    OS_SEM_DATA 	semDataP;
    MMP_UBYTE       ret;

    if (ulSemId >= MMPF_OS_SEMID_MAX) {
        return 0xFE;
    }

    ret = OSSemQuery(os_sem_tbl[ulSemId], &semDataP);
    *usCount = semDataP.OSCnt;

    switch (ret) {
	case OS_NO_ERR:
		return 0;
    default:
        return 0xFF;
    }
}



MMP_UBYTE MMPF_OS_SetSem(MMPF_OS_SEMID semID,MMP_USHORT count)
{
    MMP_UBYTE       ret;

    if (semID >= MMPF_OS_SEMID_MAX) {
        return 0xFE; //[TBD]
    }
    OSSemSet(os_sem_tbl[semID],count,&ret);

    if(ret) {
        RTNA_DBG_Str(3,"<<OSSemSet err:");
        RTNA_DBG_Byte(3,ret);
        RTNA_DBG_Str(3,">>\r\n");
    
    }
    return ret ;    
}

//------------------------------------------------------------------------------
//  Function    : MMPF_OS_WaitFlags
//  Description :
//------------------------------------------------------------------------------
/** @brief This function waits for a semaphore.

@param[in] ulFlagID : The Flag ID that return by @ref MMPF_OS_CreateEventFlagGrp 
@param[in] flags : Is a bit pattern indicating which bit(s) (i.e. flags) you wish to wait for.
@param[in] waitType : specifies whether you want ALL bits to be set or ANY of the bits to be set.
                            You can specify the following argument:

                            MMPF_OS_FLAG_WAIT_CLR_ALL   You will wait for ALL bits in 'mask' to be clear (0)
                            MMPF_OS_FLAG_WAIT_SET_ALL   You will wait for ALL bits in 'mask' to be set   (1)
                            MMPF_OS_FLAG_WAIT_CLR_ANY   You will wait for ANY bit  in 'mask' to be clear (0)
                            MMPF_OS_FLAG_WAIT_SET_ANY   You will wait for ANY bit  in 'mask' to be set   (1)

                            NOTE: Add MMPF_OS_FLAG_CONSUME if you want the event flag to be 'consumed' by
                                  the call.  Example, to wait for any flag in a group AND then clear
                                  the flags that are present, set 'wait_type' to:

                                  MMPF_OS_FLAG_WAIT_SET_ANY + MMPF_OS_FLAG_CONSUME
@param[in] ulTimeout : is an optional timeout (in clock ticks) that your task will wait for the
                            desired bit combination.  If you specify 0, however, your task will wait
                            forever at the specified event flag group or, until a message arrives.
@param[out] ret_flags : The flags in the event flag group that made the task ready or, 0 if a timeout or an error
		*              occurred.
@retval 0xFE for bad input flag id,
		0xFF for wait flag internal error from OS
		0 for getting the flag.
		1 for time out happens
*/

MMP_UBYTE MMPF_OS_WaitFlags(MMPF_OS_FLAGID ulFlagID, MMPF_OS_FLAGS flags, MMPF_OS_FLAG_WTYPE waitType, 
							MMP_ULONG ulTimeout, MMPF_OS_FLAGS *ret_flags)
{
    MMP_UBYTE       ret;

    if (ulFlagID >= MMPF_OS_FLAGID_MAX) {
        return 0xFE;
    }

    *ret_flags = OSFlagPend(os_flag_tbl[ulFlagID], flags, waitType, ulTimeout, &ret);


    switch (ret) {
	case OS_NO_ERR:
		return 0;
	case OS_TIMEOUT:
		return 1;
    default:
        return 0xFF;
    }
}

//------------------------------------------------------------------------------
//  Function    : MMPF_OS_SetFlags
//  Description :
//------------------------------------------------------------------------------
/** @brief This function is called to set or clear some bits in an event flag group.  The bits to
              set or clear are specified by a 'bit mask'.

@param[in] ulFlagID : The flag ID that return by @ref MMPF_OS_CreateEventFlagGrp 
@param[in] flags : If 'opt' (see below) is MMPF_OS_FLAG_SET, each bit that is set in 'flags' will
                   set the corresponding bit in the event flag group.
				   If 'opt' (see below) is MMPF_OS_FLAG_CLR, each bit that is set in 'flags' will
				   CLEAR the corresponding bit in the event flag group.
@param[in] opt : MMPF_OS_FLAG_CLR for flag clear
				 MMPF_OS_FLAG_SET for flag set	
@retval 0xFE for bad input semaphore id,
		0xFF for setflag internal error from OS
		0 for calling was successfull
*/

MMP_UBYTE MMPF_OS_SetFlags(MMPF_OS_FLAGID ulFlagID, MMPF_OS_FLAGS flags, MMPF_OS_FLAG_OPT opt)
{
    MMP_UBYTE       ret;

    if (ulFlagID >= MMPF_OS_FLAGID_MAX) {
        return 0xFE;
    }

    OSFlagPost(os_flag_tbl[ulFlagID], flags, opt, &ret);

    switch (ret) {
    case OS_NO_ERR:
    	return 0;
    default:
        return 0xFF;
    }
}

//------------------------------------------------------------------------------
//  Function    : MMPF_OS_QueryFlags
//  Description :
//------------------------------------------------------------------------------
/** @brief This function obtains count about a semaphore

@param[in] ulFlagID : The flag ID that return by @ref MMPF_OS_CreateEventFlagGrp 
@param[out] ret_flags : The current value of the event flag group.
@retval 0xFE for bad input flag id,
		0xFF for query flag internal error from OS
		0 for no error
*/
MMP_UBYTE MMPF_OS_QueryFlags(MMPF_OS_FLAGID ulFlagID, MMPF_OS_FLAGS *ret_flags)
{
    MMP_UBYTE		ret;

    if (ulFlagID >= MMPF_OS_FLAGID_MAX) {
        return ulFlagID;
    }

    *ret_flags = OSFlagQuery(os_flag_tbl[ulFlagID], &ret);

    switch (ret) {
	case OS_NO_ERR:
		return 0;
    default:
        return 0xFF;
    }
}

//------------------------------------------------------------------------------
//  Function    : MMPF_OS_CreateEventFlagGrp
//  Description :
//------------------------------------------------------------------------------
/** @brief This function creates a flag.

@retval 0xFF for create flag internal error from OS
		0xFE the system maximum flag counts exceed.
		others, the ID to access the flag
*/
MMPF_OS_FLAGID MMPF_OS_CreateEventFlagGrp(MMP_ULONG ulFlagValues)
{
    MMP_ULONG		i;
    MMP_UBYTE		ret;
    OS_FLAG_GRP		*flags;

    flags = OSFlagCreate(ulFlagValues, &ret);

    if (!flags) {
        while(1) { dbg_printf(3,"flag create failed\r\n"); }
        return 0xFF;
    }

    for (i = 0; i < MMPF_OS_FLAGID_MAX; i++) {
        if (os_flag_tbl[i] == 0x0) {
            os_flag_tbl[i] = flags;
            return i;
        }
    }

    OSFlagDel(flags, OS_DEL_ALWAYS, &ret);
    return 0xFE;
}


/*
MMPF_OS_MQID MMPF_OS_CreateMQueue(void **msg, MMP_UBYTE ubQueueSize)
{
    MMP_ULONG   i;
    MMP_UBYTE   ret;
    OS_EVENT    *ev;

    ev = OSQCreate(msg, ubQueueSize);

    if (!ev) {
        return 0xFF;
    }

    for (i = 0; i < MMPF_OS_MQID_MAX; i++) {
        if (os_mq_tbl[i] == 0x0) {
            os_mq_tbl[i] = ev;
            return i;
        }
    }

    OSQDel(ev, OS_DEL_ALWAYS, &ret);
    return 0xFE;
}
*/


void MMPF_OS_Sleep(MMP_USHORT usTickCount)
{
    OSTimeDly(usTickCount);
}

MMP_UBYTE MMPF_OS_ChangePrio(MMP_UBYTE oldprio,MMP_UBYTE newprio)
{
    MMP_UBYTE err = 0 ;
 #if (SUPPORT_UAC==1) //&& (CHIP==P_V2)    

    err = OSTaskChangePrio((INT8U)oldprio,(INT8U)newprio);
    dbg_printf(3,"[OS/Prio] : %d - > %d\r\n",oldprio,newprio );
    MMPF_OS_Sleep(1);
#endif    
    return err ;  
     
}

/** @} */ // MMPF_OS