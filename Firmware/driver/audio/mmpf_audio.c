//==============================================================================
//
//  File        : audio.c
//  Description : Audio Task Function
//  Author      : Hnas Liu
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_audio_ctl.h"
#include "reg_retina.h"
#include "mmpf_audio.h"
#include "mmpf_i2s_ctl.h"
#if PCAM_EN==1
#include "mmpf_encapi.h"
#endif

#if SUPPORT_UAC==1
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
/** @addtogroup MMPF_AUDIO
@{
*/
extern MMP_UBYTE    gbUseAitADC;
extern MMP_UBYTE    gbUseAitDAC;
extern MMP_ULONG    glAudioRecIntFlag;
extern MMPF_OS_FLAGID   SYS_Flag_Hif;
//extern MMP_USHORT   gsAudioRecordState;
extern MMP_USHORT   gsAudioPlayState;
extern MMP_USHORT	gsAudioEncodeFormat;
extern MMP_USHORT   gsAudioRecEvent;


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
MMP_ULONG glEncTicks,glTimerTicks;
extern MMP_ULONG glAFETicks ;

void AUDIO_Record_Task()
{
    AITPS_AIC   	pAIC = AITC_BASE_AIC;
    MMPF_OS_FLAGS	flags;

    //RTNA_DBG_Str(0, "AUDIO_Record_Task()\r\n");

    gsAudioRecEvent &= ~(EVENT_FIFO_OVER_THRESHOLD);
	#if (MIC_SOURCE == MIC_IN_PATH_I2S)
    	RTNA_AIC_Open(pAIC, AIC_SRC_AUD_FIFO,   i2s_isr_a,
                    	AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE    | 3);
	#elif (MIC_SOURCE == MIC_IN_PATH_AFE)
		RTNA_AIC_Open(pAIC, AIC_SRC_AFE_FIFO,   afe_isr_a,
                    	AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE    | 3);
	#endif
    
    while(1) {
        MMPF_OS_WaitFlags(SYS_Flag_Hif, SYS_FLAG_AUDIOREC,
            MMPF_OS_FLAG_WAIT_SET_ANY| MMPF_OS_FLAG_CONSUME , 0, &flags);
        #if PCAM_EN==1
        if (gsAudioRecEvent & EVENT_FIFO_OVER_THRESHOLD) {
            glEncTicks++;
            gsAudioRecEvent &= ~(EVENT_FIFO_OVER_THRESHOLD);
            MMPF_Audio_EncodeFrame();
        }
        #endif
    }
}
#endif

/// @}
