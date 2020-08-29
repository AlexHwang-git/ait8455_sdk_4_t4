//==============================================================================
//
//  File        : mmpf_audio_task.c32
//  Description : Audio Task Entry
//  Author      : Hans Liu
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "reg_retina.h"
#include "lib_retina.h"

/** @addtogroup MMPF_AUDIO
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

#if defined(ALL_FW)
#if (AUDIO_P_EN)
OS_STK                  AUDIO_Play_Task_Stk[AUDIO_PLAY_STK_SIZE];
extern void             AUDIO_Play_Task(void);
#endif
#if SUPPORT_UAC //(AUDIO_R_EN) || (PCAM_EN)
OS_STK                  AUDIO_Record_Task_Stk[AUDIO_RECORD_STK_SIZE];
extern void             AUDIO_Record_Task(void);
#endif
#if ((AUDIO_P_EN)&&(AUDIO_STREAMING_EN == 1))||(AUDIO_R_EN)
OS_STK                  AUDIO_WriteFile_Task_Stk[AUDIO_WRITEFILE_STK_SIZE];
extern void             AUDIO_Streaming_Task(void);
#endif
#endif

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : AUDIO_Play_Task
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
#if defined(ALL_FW)
#if (AUDIO_P_EN)
void MMPF_AUDIOPLAY_TaskHandler(void *p_arg)
{
    AUDIO_Play_Task();
}
#endif
#if SUPPORT_UAC//(AUDIO_R_EN) || (PCAM_EN)
void MMPF_AUDIORECORD_TaskHandler(void *p_arg)
{
    AUDIO_Record_Task();
}
#endif
#if ((AUDIO_P_EN)&&(AUDIO_STREAMING_EN == 1))||(AUDIO_R_EN)
void MMPF_AUDIOSTREAMING_TaskHandler(void *p_arg)
{
    AUDIO_Streaming_Task();
}
#endif
#endif
/// @}
