//==============================================================================
//
//  File        : mmpf_mp4venc_task.c32
//  Description : Video encoder task entry function
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

OS_STK  MP4VENC_Task_Stk[MP4VENC_STK_SIZE];


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

extern void MP4VENC_Task(void *p_arg);


//------------------------------------------------------------------------------
//  Function    : VidRecd_Task_Entry
//  Description : Task entry function for Video Encoder
//------------------------------------------------------------------------------

void MMPF_MP4VENC_TaskHandler(void *p_arg)
{
    MP4VENC_Task(p_arg);
}
