/// @ait_only
/**
 @file mmpd_mp4venc.h
 @brief INCLUDE File of Host VIDEO ENCODE Driver.
 @author Will Tseng
 @version 1.0
*/


#ifndef _MMPD_MP4VENC_H_
#define _MMPD_MP4VENC_H_

#include "mmp_lib.h"
#include "ait_config.h"
#include "mmpd_rawproc.h"
#include "mmpf_mp4venc.h"

/** @addtogroup MMPD_VIDENC
 *  @{
 */

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define MAX_VIDENC_NUM             (MAX_NUM_ENC_SET)

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

/// Firmware status. For video encode, the sequence is the same as firmware.
typedef enum _MMPD_MP4VENC_FW_OP {
    MMPD_MP4VENC_FW_OP_NONE = 0x00,
    MMPD_MP4VENC_FW_OP_START,
    MMPD_MP4VENC_FW_OP_PAUSE,
    MMPD_MP4VENC_FW_OP_RESUME,
    MMPD_MP4VENC_FW_OP_STOP
} MMPD_MP4VENC_FW_OP;

/// Video format
typedef enum _MMPD_MP4VENC_FORMAT {
    MMPD_MP4VENC_FORMAT_OTHERS = 0x00,
    MMPD_MP4VENC_FORMAT_H263,
    MMPD_MP4VENC_FORMAT_MP4V,
    MMPD_MP4VENC_FORMAT_H264,
    MMPD_MP4VENC_FORMAT_MJPEG,
    MMPD_MP4VENC_FORMAT_YUV422,
    MMPD_MP4VENC_FORMAT_YUV420,
    MMPD_MP4VENC_FORMAT_RAW
} MMPD_MP4VENC_FORMAT;

#define	VIDEO_INPUT_FB_MAX_CNT	4

typedef struct _MMPD_MP4VENC_INPUT_BUF {
	MMP_ULONG ulBufCnt;
    MMP_ULONG ulY[VIDEO_INPUT_FB_MAX_CNT];            ///< Video encode input Y buffer0 start address (can be calculated)
    MMP_ULONG ulU[VIDEO_INPUT_FB_MAX_CNT];            ///< Video encode input U buffer1 start address (can be calculated)
    MMP_ULONG ulV[VIDEO_INPUT_FB_MAX_CNT];            ///< Video encode input V buffer1 start address (can be calculated)
} MMPD_MP4VENC_INPUT_BUF;

typedef struct _MMPD_MP4VENC_MISC_BUF {
    MMP_ULONG ulMVBuf;						///< Video encode MV start buffer (can be calculated)
    MMP_ULONG ulSliceLenBuf;                ///< H264 slice length buf
} MMPD_MP4VENC_MISC_BUF;

typedef struct _MMPD_H264ENC_HEADER_BUF {
    MMP_ULONG ulSPSStart;
    MMP_ULONG ulSPSSize;
    MMP_ULONG ulPPSStart;
    MMP_ULONG ulPPSSize;
} MMPD_H264ENC_HEADER_BUF;

typedef struct _MMPD_MP4VENC_BITSTREAM_BUF {
    MMP_ULONG ulStart;      		  		///< Video encode compressed buffer start address (can be calculated)
    MMP_ULONG ulEnd;		          		///< Video encode compressed buffer end address (can be calculated)
} MMPD_MP4VENC_BITSTREAM_BUF;

typedef struct _MMPD_MP4VENC_LINE_BUF {
    MMP_ULONG ulY[12];          ///< ME Y line start address, 12 lines for h264 high profile
    MMP_ULONG ulU[12];		    ///< ME U line start address, 12 lines for h264 high profile UV interleave mode
    MMP_ULONG ulV[12];		    ///< ME Y line start address
    MMP_ULONG ulUP[4];          ///< ME Y up sample, for h264
	MMP_ULONG ulDeblockRow;
} MMPD_MP4VENC_LINE_BUF;

typedef struct _MMPD_MP4VENC_REFGEN_BUF {
    MMP_ULONG ulGenY;			///< Video encode Y generate buffer
    MMP_ULONG ulGenU;			///< Video encode U generate buffer
    MMP_ULONG ulGenV;			///< Video encode V generate buffer
    MMP_ULONG ulRefY;     		///< Video encode Y reference buffer
    MMP_ULONG ulRefU;    		///< Video encode U reference buffer
    MMP_ULONG ulRefV;	   		///< Video encode V reference buffer
} MMPD_MP4VENC_REFGEN_BUF;

typedef struct _MMPD_MP4VENC_VIDEOBUF {
	MMPD_MP4VENC_MISC_BUF	miscbuf;
	MMPD_MP4VENC_LINE_BUF	linebuf;
} MMPD_MP4VENC_VIDEOBUF;

typedef struct _MMPD_VIDENC_FUNCTIONS {
    MMP_ERR (*MMPD_VIDENC_StartCapture)(void);
    MMP_ERR (*MMPD_VIDENC_StopCapture)(void);
    MMP_ERR (*MMPD_VIDENC_ResumeCapture)(void);
    MMP_ERR (*MMPD_VIDENC_PauseCapture)(void);
	MMP_ERR (*MMPD_VIDENC_EnableClock)(MMP_BOOL bEnable);
} MMPD_VIDENC_FUNCTIONS;

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

// Generic API -----------------------------------------------------------------
MMP_ERR MMPD_VIDENC_EnableTimer(MMP_BOOL bEnable);
MMP_ERR MMPD_VIDENC_StartCapture(void);
MMP_ERR MMPD_VIDENC_StopCapture(void);
MMP_ERR MMPD_VIDENC_ResumeCapture(void);
MMP_ERR MMPD_VIDENC_PauseCapture(void);
MMP_ERR MMPD_VIDENC_GetStatus(MMPD_MP4VENC_FW_OP *status);
MMPD_MP4VENC_FW_OP MMPD_VIDENC_CheckStatus(void);
MMP_ERR MMPD_VIDENC_EnableClock(MMP_BOOL bEnable);

// End of Generic API ----------------------------------------------------------

// [MP4] API
MMP_ERR MMPD_MP4VENC_StopCapture(void);
MMP_ERR MMPD_MP4VENC_ResumeCapture(void);
MMP_ERR MMPD_MP4VENC_PauseCapture(void);


// [H264] API
MMP_ERR MMPD_H264ENC_SetMiscBuf(MMPD_MP4VENC_MISC_BUF *miscbuf);
MMP_ERR MMPD_H264ENC_SetLineBuf(MMPD_MP4VENC_LINE_BUF *linebuf);
MMP_ERR MMPD_H264ENC_SetEncodeMode(void);

MMP_ERR MMPD_H264ENC_StartCapture(void);
MMP_ERR MMPD_H264ENC_StopCapture(void);
MMP_ERR MMPD_H264ENC_ResumeCapture(void);
MMP_ERR MMPD_H264ENC_PauseCapture(void);
MMP_ERR MMPD_H264ENC_EnableClock(MMP_BOOL bEnable);
MMP_ERR MMPD_H264ENC_SetIMENum(MMP_UBYTE ubImeNum);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _INCLUDES_H_
/// @end_ait_only

