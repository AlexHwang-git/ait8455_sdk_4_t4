/**
 @file mmps_3gprecd.h
 @brief Header File for the Host 3GP RECORDER API.
 @author Will Tseng
 @version 1.0
*/

#ifndef _MMPS_3GPRECD_H_
#define _MMPS_3GPRECD_H_

#include "mmp_lib.h"
#include "mmps_dsc.h"
#include "mmps_system.h"

#include "mmpd_mp4venc.h"
#include "mmpd_3gpmgr.h"
#if PCAM_EN==1
#include "pcam_api.h"
#endif
#include "mmpf_mp4venc.h"
//===============================================================================
//
//                               COMPILER OPTION
// 
//===============================================================================


//===============================================================================
// 
//                               CONSTANTS
// 
//===============================================================================
#define VIDEOR_MAX_RAWSTORE_BUFFER_NUM  (3)
#define VIDEOR_MAX_B_FRAME_NUM          (2)

//===============================================================================
// 
//                               STRUCTURES
// 
//===============================================================================

/// Firmware status. For video encode, the sequence is the same as firmware.
typedef enum _MMPS_FW_OPERATION {
    MMPS_FW_OPERATION_NONE = 0x00,						///< FW operation, none
    MMPS_FW_OPERATION_START,							///< FW operation, start
    MMPS_FW_OPERATION_PAUSE,							///< FW operation, pause
    MMPS_FW_OPERATION_RESUME,							///< FW operation, resume
    MMPS_FW_OPERATION_STOP								///< FW operation, stop
} MMPS_FW_OPERATION;

/// Video format
typedef enum _MMPS_3GPRECD_VIDEO_FORMAT {
    MMPS_3GPRECD_VIDEO_FORMAT_OTHERS = 0x00,			///< Video format, none
    MMPS_3GPRECD_VIDEO_FORMAT_H263,						///< Video format, H.263
    MMPS_3GPRECD_VIDEO_FORMAT_MP4V,						///< Video format, MP4V
    MMPS_3GPRECD_VIDEO_FORMAT_H264,                     ///< Video format, H.264
    MMPS_3GPRECD_VIDEO_FORMAT_MJPEG,                    ///< Video format, MJPEG
    MMPS_3GPRECD_VIDEO_FORMAT_YUV422,                    ///< Video format, YUV422
    MMPS_3GPRECD_VIDEO_FORMAT_YUV420,                    ///< Video format, YUV420
    MMPS_3GPRECD_VIDEO_FORMAT_RAW
} MMPS_3GPRECD_VIDEO_FORMAT;

/// Audio format
typedef enum _MMPS_3GPRECD_AUDIO_OPTION {
    MMPS_3GPRECD_AUDIO_AAC_BASE 		= 0x00,			///< Audio AAC format
    MMPS_3GPRECD_AUDIO_AAC_22d05K_64K 	= 0x02,			///< AAC 22.05KHz with 64kbps
    MMPS_3GPRECD_AUDIO_AAC_22d05K_128K,					///< AAC 22.05KHz with 128kbps
    MMPS_3GPRECD_AUDIO_AAC_32K_64K,						///< AAC 32KHz with 64kbps
    MMPS_3GPRECD_AUDIO_AAC_32K_128K,					///< AAC 32KHz with 128kbps
    MMPS_3GPRECD_AUDIO_AAC_44d1K_64K,					///< AAC 44.1KHz with 64kbps
    MMPS_3GPRECD_AUDIO_AAC_44d1K_128K,					///< AAC 44.1KHz with 128kbps
    MMPS_3GPRECD_AUDIO_AMR_BASE 		= 0x10,			///< Audio AMR format
    MMPS_3GPRECD_AUDIO_AMR_4d75K,						///< AMR 4.75KHz with 8kbps
    MMPS_3GPRECD_AUDIO_AMR_5d15K,						///< AMR 5.15KHz with 8kbps
    MMPS_3GPRECD_AUDIO_AMR_12d2K						///< AMR 12.2KHz with 8kbps
} MMPS_3GPRECD_AUDIO_OPTION;

/// AV operation mode. This is for recorder only, used in Initialization
typedef enum _MMPS_3GPRECD_AUDIO_FORMAT {
    MMPS_3GPRECD_AUDIO_FORMAT_AAC = 0x00,    			///< Video encode with AAC audio
    MMPS_3GPRECD_AUDIO_FORMAT_AMR           			///< Video encode with AMR audio
} MMPS_3GPRECD_AUDIO_FORMAT;

/// Global settings
typedef struct _MMPS_3GPRECD_MODES {
    MMP_USHORT usVideoTotalNum;                         ///< total number of enc instance
    MMP_USHORT usWidth[MAX_NUM_ENC_SET];                ///< width of each encode instance
    MMP_USHORT usHeight[MAX_NUM_ENC_SET];               ///< height of each encode instance
} MMPS_3GPRECD_MODES;

//===============================================================================
// 
//                               VARIABLES
// 
//===============================================================================


//===============================================================================
//
//                               FUNCTION PROTOTYPES
// 
//===============================================================================

//------------
//  3gp record
//------------
/** @addtogroup MMPS_VIDRECD
@{
*/

MMP_ERR MMPS_3GPRECD_SetPreviewConfig(MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight,
                                        MMP_USHORT usEncWidth, MMP_USHORT usEncHeight,
                                        MMPF_VIDENC_FRAME_LIST *pFrameList, MMPF_FCTL_LINK *pVideoPipeLink,
                                        MMP_USHORT  usScaleResol, MMP_UBYTE ubEncId);
MMP_ERR MMPS_3GPRECD_StartRecord(void);
MMP_ERR MMPS_3GPRECD_StopRecord(void);
MMP_ERR MMPS_3GPRECD_SetResolution(MMP_USHORT usEncWidth[], MMP_USHORT usEncHeight[], MMP_UBYTE ubEncResolNum);
MMP_ERR MMPS_3GPRECD_InitVideoInfo(void);
MMP_ERR MMPS_3GPRECD_SetMemoryMapH264(MMP_ULONG *ulFBufAddr, MMP_ULONG *ulStackAddr,
                                        MMPF_VIDENC_FRAME_LIST *pInputFrames, MMP_ULONG ulRingBufSize);
MMP_ERR MMPS_3GPRECD_GetPreviewDisplay(MMP_BOOL *bEnable);

/// @}

//===============================================================================
// 
//                               MACRO FUNCTIONS
// 
//===============================================================================


#endif //  _MMPS_3GPRECD_H_

