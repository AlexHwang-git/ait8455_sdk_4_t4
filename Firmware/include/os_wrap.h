//==============================================================================
//
//  File        : os_wrap.h
//  Description : OS wrapper function for uC/OS-II
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================


#ifndef _OS_WRAP_H_
#define _OS_WRAP_H_

#include "includes_fw.h"
// Disable IRQ to lock critical section.
#define IRQ_LOCK(statement)     \
do {    \
    OS_ENTER_CRITICAL();    \
    statement;              \
    OS_EXIT_CRITICAL();     \
} while (0);

void        OSWrap_StatInit(void);
MMP_USHORT  OSWrap_TaskNameSet(MMP_UBYTE prio, char *pname);
MMP_USHORT  OSWrap_EventNameSet(OS_EVENT *pevent, char *pname);


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

#define MMPF_OS_SEMID_MAX   	OS_MAX_EVENTS
#define MMPF_OS_FLAGID_MAX  	OS_MAX_FLAGS
#define MMPF_OS_MUTEXID_MAX  	0
#define MMPF_OS_MQID_MAX   		OS_MAX_QS
#define MMPF_OS_TMRID_MAX   	OS_TMR_CFG_MAX


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef MMP_UBYTE   MMPF_OS_TASKID;
typedef MMP_ULONG   MMPF_OS_SEMID;
typedef MMP_ULONG   MMPF_OS_FLAGID;
typedef MMP_ULONG   MMPF_OS_MUTEXID;
typedef MMP_ULONG   MMPF_OS_MQID;
typedef MMP_ULONG   MMPF_OS_TMRID;

#if OS_FLAGS_NBITS == 16
typedef MMP_USHORT   MMPF_OS_FLAGS;
#endif
#if OS_FLAGS_NBITS == 32
typedef MMP_ULONG    MMPF_OS_FLAGS;
#endif

typedef MMP_USHORT  MMPF_OS_FLAG_WTYPE;
    #define MMPF_OS_FLAG_WAIT_CLR_ALL       OS_FLAG_WAIT_CLR_ALL
    #define MMPF_OS_FLAG_WAIT_CLR_ANY       OS_FLAG_WAIT_CLR_ANY
    #define MMPF_OS_FLAG_WAIT_SET_ALL       OS_FLAG_WAIT_SET_ALL
    #define MMPF_OS_FLAG_WAIT_SET_ANY       OS_FLAG_WAIT_SET_ANY
    #define MMPF_OS_FLAG_CONSUME            OS_FLAG_CONSUME
typedef MMP_USHORT  MMPF_OS_FLAG_OPT;
    #define MMPF_OS_FLAG_CLR                OS_FLAG_CLR
    #define MMPF_OS_FLAG_SET                OS_FLAG_SET


typedef struct {
    MMP_ULONG   pbos;           // lower address
    MMP_ULONG   ptos;           // higer address
	MMP_UBYTE	ubPriority;
} MMPF_TASK_CFG;


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void                MMPF_OS_Initialize(void);
void                MMPF_OS_StartTask(void);
void                MMPF_OS_Sleep(MMP_USHORT usTickCount);

MMPF_OS_TASKID      MMPF_OS_CreateTask(void (*taskproc)(void *param), MMPF_TASK_CFG *task_cfg, void *param);
MMPF_OS_SEMID       MMPF_OS_CreateSem(MMP_UBYTE ubSemValue);
MMP_UBYTE      MMPF_OS_DeleteSem(MMPF_OS_SEMID ulSemId);
MMP_UBYTE      MMPF_OS_ReleaseSem(MMPF_OS_SEMID ulSemId);
MMP_UBYTE      MMPF_OS_AcquireSem(MMPF_OS_SEMID ulSemId, MMP_ULONG ulTimeout);
MMP_UBYTE           MMPF_OS_AcceptSem(MMPF_OS_SEMID ulSemId, MMP_USHORT *usCount);
MMP_UBYTE 			MMPF_OS_QuerySem(MMPF_OS_SEMID ulSemId, MMP_USHORT *usCount);
MMPF_OS_FLAGID      MMPF_OS_CreateEventFlagGrp(MMP_ULONG ulFlagValues);
MMP_UBYTE 			MMPF_OS_SetFlags(MMPF_OS_FLAGID ulFlagID, MMPF_OS_FLAGS flags, MMPF_OS_FLAG_OPT opt);
MMP_UBYTE          MMPF_OS_WaitFlags(MMPF_OS_FLAGID ulFlagID, MMPF_OS_FLAGS flags, MMPF_OS_FLAG_WTYPE waitType, 
						MMP_ULONG ulTimeout, MMPF_OS_FLAGS *ret_flags);
MMP_UBYTE 			MMPF_OS_QueryFlags(MMPF_OS_FLAGID ulFlagID, MMPF_OS_FLAGS *ret_flags);
void*				MMPF_OS_Malloc(int len);
void* 				MMPF_OS_Calloc(int num, int size);
void    			MMPF_OS_MemFree(char *mem_ptr);
MMP_UBYTE      MMPF_OS_SetSem(MMPF_OS_SEMID ulSemId,MMP_USHORT count);
MMP_UBYTE MMPF_OS_ChangePrio(MMP_UBYTE oldprio,MMP_UBYTE newprio);
//------------------------------
// User Specified Configuration
//------------------------------


extern MMPF_TASK_CFG        task_cfg[];


#endif // _OS_WRAP_H_

