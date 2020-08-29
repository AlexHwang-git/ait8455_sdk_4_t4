/// @ait_only
//==============================================================================
//
//  File        : mmpd_fctl.c
//  Description : Ritian Flow Control driver function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
*  @file mmpd_fctl.c
*  @brief The FLOW control functions
*  @author Penguin Torng
*  @version 1.0
*/
#include "config_fw.h"
#include "mmpf_display.h"

#ifdef BUILD_CE
#undef BUILD_FW
#endif

#include "lib_retina.h"
#include "mmp_lib.h"
#include "mmpf_fctl.h"
#include "mmpd_system.h"
#include "mmph_hif.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_graphics.h"
#include "mmpf_sensor.h"
#include "mmpf_ibc.h"

extern	MMP_ULONG   glPreviewBufAddr[MMPF_IBC_PIPE_MAX][4];
extern	MMP_ULONG   glPreviewUBufAddr[MMPF_IBC_PIPE_MAX][4];
extern	MMP_ULONG   glPreviewVBufAddr[MMPF_IBC_PIPE_MAX][4];
extern  MMP_USHORT  gsPreviewBufWidth[MMPF_IBC_PIPE_MAX][4];
extern  MMP_USHORT  gsPreviewBufHeight[MMPF_IBC_PIPE_MAX][4];
extern  MMP_USHORT  gsPreviewBufCurWidth[MMPF_IBC_PIPE_MAX];
extern  MMP_USHORT  gsPreviewBufCurHeight[MMPF_IBC_PIPE_MAX];
extern  MMP_ULONG   glRotateBufAddr[MMPF_IBC_PIPE_MAX];
extern  MMP_ULONG   glRotateUBufAddr[MMPF_IBC_PIPE_MAX];
extern  MMP_ULONG   glRotateVBufAddr[MMPF_IBC_PIPE_MAX];
extern  MMP_UBYTE   gbRotateBufferCount;

extern  MMP_UBYTE   gbPreviewBufferCount[];
extern 	MMPF_DISPLAY_PREVIEW_MODE   gPreviewMode[];
extern  MMPF_DISPLAY_WINID  gPreviewWinID[];
extern	MMPF_IBC_LINK_TYPE	gIBCLinkType[];
extern  MMP_UBYTE gbIBCLinkEncId[];
extern  MMP_UBYTE gbIBCLinkEncFromGraList[MMPF_IBC_PIPE_MAX][4];
extern  MMP_UBYTE gbIBCLinkEncFromGraNum[MMPF_IBC_PIPE_MAX];

/** @addtogroup MMPD_FCTL
 *  @{
 */
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
MMPF_FCTL_PREVIEWATTRIBUTE m_previewAttribute[MMPF_IBC_PIPE_MAX];

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

/** @brief Transformation of the RAW data image in the frame buffer, The transform
opertaion includes color transform and dimension transform

The function can transfer the picture from one format to another format, and
change the scale ratio of the pictures.

@return It reports the status of the operation.
*/
MMP_ERR MMPF_Fctl_SetPreivewAttributes(MMPF_FCTL_PREVIEWATTRIBUTE *previewattribute)
{
   	MMPF_SCALER_FIT_RANGE 		fitrange;
   	MMPF_ICO_PIPEATTRIBUTE 		ico_pipeattribute;
   	MMPF_IBC_PIPEATTRIBUTE 		ibc_pipeattribute;
   	MMPF_SCALER_COLORMODE  		scal_outcolor;
   	MMP_ULONG					i;
   	MMP_ERR						mmpstatus;

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_TRUE);

	// config IBC module
	ibc_pipeattribute.colorformat = previewattribute->colormode;
	ibc_pipeattribute.ulBaseAddr = previewattribute->ulBaseAddr[0];
	ibc_pipeattribute.ulBaseUAddr = previewattribute->ulBaseUAddr[0];
	ibc_pipeattribute.ulBaseVAddr = previewattribute->ulBaseVAddr[0];
	ibc_pipeattribute.usMirrorWidth = previewattribute->grabctl.usEndX - previewattribute->grabctl.usStartX + 1;
    ibc_pipeattribute.ulLineOffset = 0;
	ibc_pipeattribute.function = previewattribute->IbcLinkFx;
	ibc_pipeattribute.bMirrorEnable = previewattribute->bMirrorEnable;
	ibc_pipeattribute.InputSource   = previewattribute->fctllink.icopipeID;
	MMPF_IBC_SetAttributes(previewattribute->fctllink.ibcpipeID, &ibc_pipeattribute);

	// config ICO module and connect SCAL->ICO
    ico_pipeattribute.inputsel = previewattribute->fctllink.scalerpath;
	ico_pipeattribute.bDlineEn = MMP_TRUE;
	MMPF_ICON_SetAttributes(previewattribute->fctllink.icopipeID, &ico_pipeattribute);

	// config Scaler module
    switch (previewattribute->colormode) {
    case MMPF_IBC_COLOR_RGB565:
        scal_outcolor = MMPF_SCALER_COLOR_RGB565;
        break;
    case MMPF_IBC_COLOR_YUV422_YUYV:
    case MMPF_IBC_COLOR_YUV422_UYVY:
    case MMPF_IBC_COLOR_YUV422:
    case MMPF_IBC_COLOR_I420:
    case MMPF_IBC_COLOR_NV12:
    case MMPF_IBC_COLOR_NV21:
    case MMPF_IBC_COLOR_YUV420_LUMI_ONLY:
        scal_outcolor = MMPF_SCALER_COLOR_YUV422;
        break;
    default:
        return MMP_FCTL_ERR_PARAMETER;
    }
    MMPF_Scaler_SetOutputFormat(previewattribute->fctllink.scalerpath, scal_outcolor);

	fitrange.usInWidth = previewattribute->usInputW;
    fitrange.usInHeight = previewattribute->usInputH;
    MMPF_Scaler_SetEngine(MMP_TRUE, previewattribute->fctllink.scalerpath, &fitrange, &(previewattribute->grabctl));
    #if INIT_SCALE_PATH_EN==1
	// connect ISP->SCAL
	MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, previewattribute->fctllink.scalerpath);
    #endif

	MMPF_Scaler_SetEnable(previewattribute->fctllink.scalerpath, MMP_TRUE);

    gsPreviewBufCurWidth[(MMP_USHORT)previewattribute->fctllink.ibcpipeID] =
	        previewattribute->grabctl.usEndX - previewattribute->grabctl.usStartX + 1;
    gsPreviewBufCurHeight[(MMP_USHORT)previewattribute->fctllink.ibcpipeID] =
            previewattribute->grabctl.usEndY - previewattribute->grabctl.usStartY + 1;
	for (i = 0; i < previewattribute->usBufferCount; i++) {
		glPreviewBufAddr[(MMP_USHORT) previewattribute->fctllink.ibcpipeID][i] = previewattribute->ulBaseAddr[i];
		glPreviewUBufAddr[(MMP_USHORT)previewattribute->fctllink.ibcpipeID][i] = previewattribute->ulBaseUAddr[i];
		glPreviewVBufAddr[(MMP_USHORT)previewattribute->fctllink.ibcpipeID][i] = previewattribute->ulBaseVAddr[i];
		gsPreviewBufWidth[(MMP_USHORT)previewattribute->fctllink.ibcpipeID][i] = 
		            gsPreviewBufCurWidth[(MMP_USHORT)previewattribute->fctllink.ibcpipeID];
		gsPreviewBufHeight[(MMP_USHORT) previewattribute->fctllink.ibcpipeID][i] = 
		            gsPreviewBufCurHeight[(MMP_USHORT)previewattribute->fctllink.ibcpipeID];
	}
    gbPreviewBufferCount[previewattribute->fctllink.ibcpipeID] = (MMP_UBYTE)previewattribute->usBufferCount;

    if (previewattribute->bUseRotateDMA) {
    	for (i = 0; i < previewattribute->usRotateBufCnt; i++) {
			glRotateBufAddr[i]  = previewattribute->ulRotateAddr[i];
			glRotateUBufAddr[i] = previewattribute->ulRotateUAddr[i];
			glRotateVBufAddr[i] = previewattribute->ulRotateVAddr[i];
		}
        gbRotateBufferCount = previewattribute->usRotateBufCnt ;
	}
    
    gIBCLinkType[previewattribute->fctllink.ibcpipeID] = MMPF_IBC_LINK_NONE ;
    gbIBCLinkEncFromGraNum[previewattribute->fctllink.ibcpipeID] = 0;

	m_previewAttribute[previewattribute->fctllink.ibcpipeID] = *previewattribute;

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_FALSE);
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_FALSE);

	return	mmpstatus;
}

/** @brief Transformation of the RAW data image in the frame buffer, The transform
opertaion includes color transform and dimension transform

The function can transfer the picture from one format to another format, and
change the scale ratio of the pictures.

@return It reports the status of the operation.
*/
MMPF_FCTL_PREVIEWATTRIBUTE * MMPF_Fctl_GetPreviewAttributes(MMPF_IBC_PIPEID pipeID)
{
	return &(m_previewAttribute[pipeID]);
}

/** @brief Transformation of the RAW data image in the frame buffer, The transform
opertaion includes color transform and dimension transform

The function can transfer the picture from one format to another format, and
change the scale ratio of the pictures.

@return It reports the status of the operation.
*/
MMP_ERR MMPF_Fctl_SetLPFMaster(MMPF_IBC_PIPEID pipeID)
{
	MMPF_SCALER_FIT_RANGE fitrange;

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);

	fitrange.usInWidth = m_previewAttribute[pipeID].usInputW;
	fitrange.usInHeight = m_previewAttribute[pipeID].usInputH;

    MMPF_Scaler_SetLPF( m_previewAttribute[pipeID].fctllink.scalerpath,
                        &fitrange,
                        &(m_previewAttribute[pipeID].grabctl));

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_FALSE);

	return	MMP_ERR_NONE;
}

/** @brief Transformation of the RAW data image in the frame buffer, The transform
opertaion includes color transform and dimension transform

The function can transfer the picture from one format to another format, and
change the scale ratio of the pictures.

@return It reports the status of the operation.
*/
MMP_ERR MMPF_Fctl_LinkPreviewToVideo(MMPF_IBC_PIPEID pipeID, MMP_UBYTE ubEncId)
{
	if ((m_previewAttribute[pipeID].colormode != MMPF_IBC_COLOR_I420)
        && (m_previewAttribute[pipeID].colormode != MMPF_IBC_COLOR_NV12))
		return	MMP_FCTL_ERR_PARAMETER;

    gIBCLinkType[(MMP_USHORT)pipeID] |= MMPF_IBC_LINK_VIDEO;
    gbIBCLinkEncId[(MMP_UBYTE)pipeID] = ubEncId;

    return MMP_ERR_NONE ;
}

MMP_ERR MMPF_Fctl_LinkPreviewToVidFromGra(MMPF_IBC_PIPEID pipeID, MMP_UBYTE ubEncIdList[], MMP_UBYTE ubEncNum)
{
    MMP_UBYTE   ubStreamNum;

    gIBCLinkType[(MMP_USHORT)pipeID] |= MMPF_IBC_LINK_VID_FROM_GRA;

    for (ubStreamNum = 0; ubStreamNum < ubEncNum; ubStreamNum++) {
        gbIBCLinkEncFromGraList[(MMP_UBYTE)pipeID][ubStreamNum] = ubEncIdList[ubStreamNum];
    }
    gbIBCLinkEncFromGraNum[(MMP_UBYTE)pipeID] = ubEncNum;

    return MMP_ERR_NONE;
}

/** @brief Establish the scaler path and link to FDTC function.

The function link the flow path from sensor to FDTC module, the path used is always
different to preview path.

@return It reports the status of the operation.
*/
MMP_ERR MMPF_Fctl_LinkPreviewToFdtc(MMPF_IBC_PIPEID pipeID)
{
#if FDTC_SUPPORT
    gFDTCAttribute.usFdtcIBCPipe = (MMP_USHORT)pipeID;
	gIBCLinkType[gFDTCAttribute.usFdtcIBCPipe] |= MMPF_IBC_LINK_FDTC;
#endif
    return MMP_ERR_NONE ;
}

MMP_ERR MMPF_Fctl_UnlinkPreview(MMPF_IBC_PIPEID pipeID, MMPF_IBC_LINK_TYPE LinkType)
{
    gIBCLinkType[(MMP_USHORT)pipeID] &= ~LinkType;

    return MMP_ERR_NONE;
}

/** @brief Transformation of the RAW data image in the frame buffer, The transform
opertaion includes color transform and dimension transform

The function can transfer the picture from one format to another format, and
change the scale ratio of the pictures.

@return It reports the status of the operation.
*/
MMP_ERR MMPF_Fctl_EnablePreview(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable, MMP_BOOL bCheckFrameEnd)
{
	MMP_ERR	mmpstatus = 0;

    if (pipeID >= MMPF_IBC_PIPE_MAX) {
        return MMP_FCTL_ERR_PARAMETER;
    }

	if (bEnable) {
    	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_DMA, MMP_TRUE);
		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_TRUE);
		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);
		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_TRUE);

	    mmpstatus = MMPF_Sensor_PreviewMode(bEnable,(MMP_USHORT)pipeID,bCheckFrameEnd);
	}
	else {
        mmpstatus = MMPF_Sensor_PreviewMode(bEnable,(MMP_USHORT)pipeID,bCheckFrameEnd);

        MMPF_Scaler_SetEnable((MMPF_SCALER_PATH)pipeID, MMP_FALSE);

        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_DMA, MMP_FALSE);
		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_FALSE);
		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_FALSE);
		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_FALSE);
	}	

    return mmpstatus;
}				
					

/// @}
/// @end_ait_only

#ifdef BUILD_CE
#define BUILD_FW
#endif


