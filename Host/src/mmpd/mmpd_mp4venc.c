/// @ait_only
/**
 @file mmpd_mp4venc.c
 @brief Retina Video Encoder Control Driver Function
 @author Will Tseng
 @version 1.0
*/
#include "config_fw.h"

#ifdef BUILD_CE
#undef BUILD_FW
#endif

#include "mmp_lib.h"
#include "mmpd_mp4venc.h"
#include "mmpd_system.h"
#include "mmpd_rawproc.h"
#include "mmph_hif.h"
#include "mmp_reg_video.h"
#include "mmp_reg_h264enc.h"
#include "mmp_reg_h264dec.h"
#include "mmp_reg_gbl.h"
#include "lib_retina.h"

#if PCAM_EN==1
#include "mmpf_mp4venc.h"
#include "mmpf_h264enc.h"
extern MMP_ERR MMPF_VIDENC_Stop(void);
extern MMP_ERR MMPF_H264ENC_Resume(void);
extern MMP_ERR MMPF_VIDENC_Pause(void);
extern void MMPF_VIDENC_TimerOpen(void);
extern void MMPF_VIDENC_TimerClose(void);
#endif

/** @addtogroup MMPD_VIDENC
 *  @{
 */

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

extern MMPD_VIDENC_FUNCTIONS m_VidencFunction_H264;     ///< set of H264 encode functions

MMPD_VIDENC_FUNCTIONS *glVidencFunction = &m_VidencFunction_H264; ///< default use H264 encode

static MMPD_MP4VENC_FW_OP m_VidRecdStatus;				///< status of firmware video engine, include START, PAUSE, RESUME and STOP

static MMP_UBYTE    m_ubIMENum = 0;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

/**
 @brief Start timer for recording
 @param[in] bEnable enable/disable timer
 @retval MMP_ERR_NONE Success
*/
MMP_ERR MMPD_VIDENC_EnableTimer(MMP_BOOL bEnable)
{
    MMP_ERR	mmpstatus = MMP_ERR_NONE;

    if (bEnable) {
        MMPF_VIDENC_TimerOpen();
    }
    else {
        MMPF_VIDENC_TimerClose();
    }

    return mmpstatus;
}

// Operation related API
/**
 @brief Encoder Driver interface API
 @retval MMP_ERR_NONE on sucess.
*/
MMP_ERR MMPD_VIDENC_StartCapture(void)
{
    return glVidencFunction->MMPD_VIDENC_StartCapture();
}
/**
 @brief Encoder Driver interface API
 @retval MMP_ERR_NONE on sucess.
*/
MMP_ERR MMPD_VIDENC_StopCapture(void)
{
    return glVidencFunction->MMPD_VIDENC_StopCapture();
}
/**
 @brief Encoder Driver interface API
 @retval MMP_ERR_NONE on sucess.
*/
MMP_ERR MMPD_VIDENC_ResumeCapture(void)
{
    return glVidencFunction->MMPD_VIDENC_ResumeCapture();
}
/**
 @brief Encoder Driver interface API
 @retval MMP_ERR_NONE on sucess.
*/
MMP_ERR MMPD_VIDENC_PauseCapture(void)
{
    return glVidencFunction->MMPD_VIDENC_PauseCapture();
}

/**
 @brief Encoder Driver interface API
 @retval MMPD_MP4VENC_FW_OP_NONE
 @retval MMPD_MP4VENC_FW_OP_START
 @retval MMPD_MP4VENC_FW_OP_PAUSE
 @retval MMPD_MP4VENC_FW_OP_RESUME
 @retval MMPD_MP4VENC_FW_OP_STOP
*/
MMP_ERR MMPD_VIDENC_GetStatus(MMPD_MP4VENC_FW_OP *status)
{
    m_VidRecdStatus =  MMPF_VIDENC_GetStatus();
	*status = m_VidRecdStatus;
	return MMP_ERR_NONE;
}

/**
 @brief Check host kept video status.
 @retval MMPD_MP4VENC_FW_OP_NONE
 @retval MMPD_MP4VENC_FW_OP_START
 @retval MMPD_MP4VENC_FW_OP_PAUSE
 @retval MMPD_MP4VENC_FW_OP_RESUME
 @retval MMPD_MP4VENC_FW_OP_STOP
*/
MMPD_MP4VENC_FW_OP MMPD_VIDENC_CheckStatus(void)
{
    return m_VidRecdStatus;
}

/**
 @brief Encoder Driver interface API
 @retval MMP_ERR_NONE on sucess.
*/
MMP_ERR MMPD_VIDENC_EnableClock(MMP_BOOL bEnable)
{
    return glVidencFunction->MMPD_VIDENC_EnableClock(bEnable);
}


/**
 @brief Set video module clock

 This function will turn on/off the video module clock. Video module is also shared with 
 decoder, so the final decision of the clock should be controlled by system.
 @param[in] bEnable Turn On/Off the VID module clock
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_EnableClock(MMP_BOOL bEnable)
{
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, bEnable);
	#if (CHIP == P_V2)
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_CABAC, bEnable);
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_AUD, bEnable);
	#endif

    return MMP_ERR_NONE;
}

/**
 @brief Set slice length, MV buffer.
 @param[in] *miscbuf Pointer of encode buffer structure.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetMiscBuf(MMPD_MP4VENC_MISC_BUF *miscbuf)
{
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_TRUE);

    MMPH_HIF_RegSetL(H264ENC_SLICE_LEN_BUF_ADDR, miscbuf->ulSliceLenBuf);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_FALSE);

    return MMP_ERR_NONE;
}

/**
 @brief Set video line buffer.

 Line buffers are used for motion estimation then need dedicated frame buffers
 for best performance.
 @param[in] *linebuf Pointer of encode buffer structure.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetLineBuf(MMPD_MP4VENC_LINE_BUF *linebuf)
{
    #if (CHIP == P_V2) || (CHIP == VSN_V2)
    MMP_USHORT  i;
    #endif
    #if (CHIP == P_V2)
    MMP_ULONG   ulUpSampleAddr[4] = { H264ENC_HALFPEL_UPSAMPLE0_ADDR,
                                    H264ENC_HALFPEL_UPSAMPLE1_ADDR,
                                    H264ENC_HALFPEL_UPSAMPLE2_ADDR,
                                    H264ENC_HALFPEL_UPSAMPLE3_ADDR };
    #endif

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_TRUE);

    #if (CHIP == VSN_V2)
    for (i = 0; i < 4; i++) {
        MMPH_HIF_RegSetL(H264ENC_LINE_Y_0 + i*4, linebuf->ulY[i]);
        MMPH_HIF_RegSetL(H264ENC_LINE_U_0 + i*4, linebuf->ulU[i]);
        MMPH_HIF_RegSetL(H264ENC_LINE_V_0 + i*4, linebuf->ulV[i]);
    }
    for (i = 0; i < 2; i++) {
        MMPH_HIF_RegSetL(H264ENC_LINE_Y_4 + i*4, linebuf->ulY[i+4]);
        MMPH_HIF_RegSetL(H264ENC_LINE_U_4 + i*4, linebuf->ulU[i+4]);
        MMPH_HIF_RegSetL(H264ENC_LINE_V_4 + i*4, linebuf->ulV[i+4]);
    }
    MMPH_HIF_RegSetL(H264_DBLK_UP_ROW_ST, linebuf->ulDeblockRow);
    #endif

    #if (CHIP == P_V2)
    for (i = 0; i < 4; i++) {
        MMPH_HIF_RegSetL(ulUpSampleAddr[i], linebuf->ulUP[i]);
        MMPH_HIF_RegSetL(H264ENC_LINE_Y_0 + i*4, linebuf->ulY[i]);
        MMPH_HIF_RegSetL(H264ENC_LINE_Y_4 + i*4, linebuf->ulY[i+4]);
        MMPH_HIF_RegSetL(H264ENC_LINE_Y_8 + i*4, linebuf->ulY[i+8]);
        MMPH_HIF_RegSetL(H264ENC_LINE_UV_0 + i*4, linebuf->ulU[i]);
        MMPH_HIF_RegSetL(H264ENC_LINE_UV_4 + i*4, linebuf->ulU[i+4]);
        MMPH_HIF_RegSetL(H264ENC_LINE_UV_8 + i*4, linebuf->ulU[i+8]);
    }
    #endif

    MMPH_HIF_RegSetB(H264ENC_FRAME_CTL,
                        MMPH_HIF_RegGetB(H264ENC_FRAME_CTL) & ~H264_Y_LINE_DIS);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_FALSE);

	return MMP_ERR_NONE;
}

/**
 @brief Enable encoder OPR setting
 @param[in] ubDram Unused now.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_SetEncodeMode(void)
{
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_TRUE);

    // Motion Estimation Setting
    MMPH_HIF_RegSetW(H264ENC_ME_COMPLEXITY              , 2);
    MMPH_HIF_RegSetW(H264ENC_ME_REFINE_COUNT            , 0x1FFF);
    MMPH_HIF_RegSetW(H264ENC_ME_PART_LIMIT_COUNT        , 0x7FFF);
    #if 1//(LOW_BITRATE == 1)
    MMPH_HIF_RegSetW(H264ENC_ME_PART_COST_THRES         , 0);
	#else
    MMPH_HIF_RegSetW(H264ENC_ME_PART_COST_THRES         , 256);
	#endif
    MMPH_HIF_RegSetW(H264ENC_ME_NO_SUBBLOCK             , 0);
    MMPH_HIF_RegSetB(H264ENC_FIRST_P_AFTER_I_IDX        , 1); // only 1st P after 1st I

    MMPH_HIF_RegSetB(H264ENC_ME_EARLY_STOP_THRES        , 15);

    MMPH_HIF_RegSetW(H264ENC_ME_STOP_THRES_UPPER_BOUND  , 512/2);
    MMPH_HIF_RegSetW(H264ENC_ME_STOP_THRES_LOWER_BOUND  , 256/2);
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    MMPH_HIF_RegSetW(H264ENC_ME_SKIP_THRES_UPPER_BOUND  , 512);
    MMPH_HIF_RegSetW(H264ENC_ME_SKIP_THRES_LOWER_BOUND  , 256);
    #endif
    #if (CHIP == P_V2)
    MMPH_HIF_RegSetW(H264ENC_ME_SKIP_THRES_UPPER_BOUND  , 1024);//julian dbg for iq
    MMPH_HIF_RegSetW(H264ENC_ME_SKIP_THRES_LOWER_BOUND  , 256);//julian dbg for iq
    #endif

    //Mode Decision
    //(min_cost > inter_cost_thr) && (((intra_cost * intra_cost_adj) >> 4) < (min_cost+inter_cost_bias))
    //TRUE    : intra
    //FALSE   : inter

    //Intra setting
    MMPH_HIF_RegSetW(H264ENC_TRANS_CTL                  , CBP_CTL_EN);
    MMPH_HIF_RegSetB(H264ENC_LUMA_COEFF_COST            , (0x5 << 4) | 0x4);
    MMPH_HIF_RegSetB(H264ENC_CHROMA_COEFF_COST          , 0x4);
    MMPH_HIF_RegSetB(H264ENC_COST_LARGER_THAN_ONE       , 0x9);
    MMPH_HIF_RegSetB(H264ENC_INTRA_PRED_MODE            , INTRA_PRED_IN_INTER_SLICE_EN);
    
    //SPS/PPS
    MMPH_HIF_RegSetB(H264ENC_GAPS_IN_FRAME_NUM_ALLOWED  , GAPS_IN_FRAME_NUM_DIS);

    MMPH_HIF_RegSetB(H264ENC_FRAME_CROPPING_FLAG        , 0);
    MMPH_HIF_RegSetB(H264ENC_FRAME_CROPPING_LEFT        , 0);
    MMPH_HIF_RegSetB(H264ENC_FRAME_CROPPING_RIGHT       , 0);
    MMPH_HIF_RegSetB(H264ENC_FRAME_CROPPING_TOP         , 0);
    MMPH_HIF_RegSetB(H264ENC_FRAME_CROPPING_BOTTOM      , 0);
    MMPH_HIF_RegSetB(H264ENC_ME16X16_MAX_MINUS_1        , 0);
    //MMPH_HIF_RegSetB(H264ENC_ME_INTER_CTL, MMPH_HIF_RegGetB(H264ENC_ME_INTER_CTL)|SKIP_CAND_INCR_ME_STEP);
    MMPH_HIF_RegSetB(H264ENC_ME_INTER_CTL, MMPH_HIF_RegGetB(H264ENC_ME_INTER_CTL)|SKIP_CAND_INCR_ME_STEP_IME);
    MMPH_HIF_RegSetB(H264ENC_IME16X16_MAX_MINUS_1       , m_ubIMENum);

    #if (CHIP == VSN_V3)
    MMPH_HIF_RegSetB(H264DEC_VLD_DATA_INFO, MMPH_HIF_RegGetB(H264DEC_VLD_DATA_INFO)|H264_REC_NV12);
    #endif
    #if (CHIP == VSN_V2)
    MMPH_HIF_RegSetB(H264DEC_VLD_DATA_INFO, MMPH_HIF_RegGetB(H264DEC_VLD_DATA_INFO)&~H264_REC_NV12);
    #endif
    #if (CHIP == P_V2)
    MMPH_HIF_RegSetB(H264DEC_VLD_FRM_DATA_INFO_REG, MMPH_HIF_RegGetB(H264DEC_VLD_FRM_DATA_INFO_REG)|H264_CURR_NV12);
    MMPH_HIF_RegSetB(H264DEC_VLD_DATA_INFO, MMPH_HIF_RegGetB(H264DEC_VLD_DATA_INFO)|H264_REC_NV12);
    MMPH_HIF_RegSetB(H264ENC_DIRECT_COST_ADJUST, 15); // B direct mode cost
    MMPH_HIF_RegSetB(H264ENC_IME_COST_WEIGHT0, 0x64);
    MMPH_HIF_RegSetB(H264ENC_IME_COST_WEIGHT1, 0x86);
    #endif

    MMPH_HIF_RegSetB(H264ENC_VLC_CTRL2, MMPH_HIF_RegGetB(H264ENC_VLC_CTRL2)|H264E_INSERT_EP3_EN);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_FALSE);

    return MMP_ERR_NONE;
}

/**
 @brief Send host command to start firmware video encoding.

 Update the host status and reset the counters of AV size buffer and frame size
 table.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_StartCapture(void)
{
	MMP_ERR	mmpstatus = MMP_ERR_NONE;

    MMPH_HIF_RegSetB(H264DEC_FRAME_CTL, AVC_DEC_ENABLE|AVC_ENC_ENABLE); // Set H264 encode mode

    MMPF_H264ENC_Start();
    
    return mmpstatus;
}


/**
 @brief Send host command to stop firmware video encoding and update the host status.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_StopCapture(void)
{
    return MMPF_VIDENC_Stop();
}

/**
 @brief Send host command to resume firmware video encoding and update the host status.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_ResumeCapture(void)
{
    return MMPF_H264ENC_Resume();
}

/**
 @brief Send host command to pause firmware video encoding and update the host status.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_H264ENC_PauseCapture(void)
{
    return MMPF_VIDENC_Pause();
}

MMP_ERR MMPD_H264ENC_SetIMENum(MMP_UBYTE ubImeNum)
{
    if(ubImeNum > 15) {
        ubImeNum = 15 ;
    }
    #if SUPPORT_GRA_ZOOM
    m_ubIMENum =  0;
    #else
    m_ubIMENum =  ubImeNum;
    #endif
    
    #if SUPPORT_PCSYNC
    if(pcsync_is_preview()) {
        m_ubIMENum = 0;
        dbg_printf(0," m_ubIMENum : %d\r\n",m_ubIMENum);   
    }
    #endif
    return MMP_ERR_NONE;
}

/**
 @brief Send host command to stop firmware video encoding and update the host status.
 @retval MMP_ERR_NONE Success.
*/

MMP_ERR MMPD_MP4VENC_StopCapture(void)
{
    return MMPF_VIDENC_Stop(); 
}

/**
 @brief Send host command to resume firmware video encoding and update the host status.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_MP4VENC_ResumeCapture(void)
{
    return MMPF_H264ENC_Resume() ;
}

/**
 @brief Send host command to pause firmware video encoding and update the host status.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_MP4VENC_PauseCapture(void)
{
    return MMPF_VIDENC_Pause() ;
}

// Initail encoder function interface
MMPD_VIDENC_FUNCTIONS m_VidencFunction_H264 = {
    MMPD_H264ENC_StartCapture,
    MMPD_H264ENC_StopCapture,
    MMPD_H264ENC_ResumeCapture,
    MMPD_H264ENC_PauseCapture,
	MMPD_H264ENC_EnableClock,
};


/// @}
/// @end_ait_only

#ifdef BUILD_CE
#define BUILD_FW
#endif

