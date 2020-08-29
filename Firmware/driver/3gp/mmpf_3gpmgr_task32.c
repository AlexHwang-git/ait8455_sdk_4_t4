//==============================================================================
//
//  File        : mmpf_3gpmgr_task.c32
//  Description : 3GP merger task entry function
//  Author      : Will Tseng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

OS_STK  VIDMGR_Task_Stk[VIDMGR_STK_SIZE];

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

extern void VidMergr_Task(void *p_arg);



//------------------------------------------------------------------------------
//  Function    : Mergr3GP_Task_Entry
//  Description : Task entry function for 3GP file merge
//------------------------------------------------------------------------------
void MMPF_VIDMGR_TaskHandler(void *p_arg)
{
    VidMergr_Task(p_arg);
}

