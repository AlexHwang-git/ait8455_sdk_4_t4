/// @ait_only
//==============================================================================
//
//  File        : mmpd_ibc.c
//  Description : Ritina IBC Module Control driver function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
*  @file mmpd_ibc.c
*  @brief The IBC Module Control functions
*  @author Penguin Torng
*  @version 1.0
*/

//#ifdef BUILD_CE
//#undef BUILD_FW
//#endif

#include "mmp_lib.h"
#include "mmpf_ibc.h"
#include "mmpd_system.h"
#include "mmph_hif.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_gbl.h"
/** @addtogroup MMPD_IBC
 *  @{
 */

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
IbcCallBackFunc *CallBackFuncIbc[MMPF_IBC_EVENT_MAX][MMPF_IBC_PIPE_MAX];

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================


/** @brief The function sets the attributes to the specified icon with its icon ID

The function sets the attributes to the specified icon with its icon ID. These attributes include icon buffer
starting address, the width, the height and its display location. It is implemented by programming Icon
Controller registers to set those attributes.

  @param[in] usPipeID the IBC ID
  @param[in] pipattribute the IBC attribute buffer
  @return It reports the status of the operation.
*/
MMP_ERR MMPF_IBC_SetAttributes(MMPF_IBC_PIPEID pipeID, 
                        				MMPF_IBC_PIPEATTRIBUTE *pipeattribute)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
	volatile AITPS_IBCP pIbcPipeCtl;

    if (pipeID == MMPF_IBC_PIPE_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (pipeID == MMPF_IBC_PIPE_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else if (pipeID == MMPF_IBC_PIPE_2) {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    else {
        return MMP_IBC_ERR_PARAMETER;
    }

    pIbcPipeCtl->IBC_BUF_CFG &= ~(IBC_STORE_SING_FRAME);
    pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_LINEOFFSET_EN;
    if (pipeattribute->function == MMPF_IBC_FX_JPG) {
        pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
        pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_UYVY;
        pIbcPipeCtl->IBC_BUF_CFG &= ~IBC_STORE_EN;

        pIBC->IBC_JPEG_PIPE_SEL = (pIBC->IBC_JPEG_PIPE_SEL & ~IBC_JPEG_SRC_SEL_MASK) | IBC_JPEG_SRC_SEL(pipeID);
    }
    else if (pipeattribute->function == MMPF_IBC_FX_H264) {
        if (pipeattribute->colorformat == MMPF_IBC_COLOR_NV12) {
            pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420|IBC_420_STORE_CBR|IBC_NV12_EN);
            #if (CHIP == VSN_V3)
            pIbcPipeCtl->IBC_BUF_COLR_FMT_CTL &= ~(IBC_420_FMT_NV21_EN);
            #endif
        }
        else if (pipeattribute->colorformat == MMPF_IBC_COLOR_I420) {
            pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420|IBC_420_STORE_CBR);
            pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_NV12_EN;
        }
        else {
            return MMP_IBC_ERR_PARAMETER;
        }

        pIbcPipeCtl->IBC_BUF_CFG |= IBC_STORE_SING_FRAME;

        pIBC->IBC_IMG_PIPE_CTL = (pIBC->IBC_IMG_PIPE_CTL & ~IBC_H264_SRC_SEL_MASK) | IBC_H264_SRC_SEL(pipeID);
    }
    else if (pipeattribute->function == MMPF_IBC_FX_RING_BUF) {
        pIbcPipeCtl->IBC_ADDR_Y_ST = pipeattribute->ulBaseAddr;

        if (pipeattribute->ulBaseEndAddr >= (pipeattribute->ulBaseAddr + 32)) {
            pIbcPipeCtl->IBC_ADDR_U_ST = pipeattribute->ulBaseEndAddr - 32;
        }
        else {
            return MMP_IBC_ERR_PARAMETER;
        }

        switch (pipeattribute->colorformat) {
        case MMPF_IBC_COLOR_RGB565:
            pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
            break;
        case MMPF_IBC_COLOR_YUV422:
            pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
            pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_UYVY;
            break;
        case MMPF_IBC_COLOR_YUV422_YUYV:
            pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
            pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_YUYV;
            break;
        default:
            return MMP_IBC_ERR_PARAMETER;
        }

        pIbcPipeCtl->IBC_BUF_CFG  |= IBC_RING_BUF_EN;
    }
	else {
        switch (pipeattribute->colorformat) {
            case MMPF_IBC_COLOR_RGB565:
                pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
                break;
            case MMPF_IBC_COLOR_I420:
                pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420|IBC_420_STORE_CBR);
                pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_NV12_EN;
                break;
            case MMPF_IBC_COLOR_YUV420_LUMI_ONLY:
                pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL|IBC_SRC_YUV420) & ~IBC_420_STORE_CBR;
                break;
            case MMPF_IBC_COLOR_NV12:
                pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420|IBC_420_STORE_CBR|IBC_NV12_EN);
                #if (CHIP == VSN_V3)
                pIbcPipeCtl->IBC_BUF_COLR_FMT_CTL &= ~(IBC_420_FMT_NV21_EN);
                #endif
                break;
            #if (CHIP == VSN_V3)
            case MMPF_IBC_COLOR_NV21:
                pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420|IBC_420_STORE_CBR|IBC_NV12_EN);
                pIbcPipeCtl->IBC_BUF_COLR_FMT_CTL |= IBC_420_FMT_NV21_EN;
                break;
            #endif
            case MMPF_IBC_COLOR_YUV422:
                pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
                pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_UYVY;
                break;
            case MMPF_IBC_COLOR_YUV422_YUYV:
                pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
                pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_YUYV;
                break;
            default:
                return MMP_IBC_ERR_PARAMETER;
	    }

	    pIbcPipeCtl->IBC_ADDR_Y_ST = pipeattribute->ulBaseAddr;
	    pIbcPipeCtl->IBC_ADDR_U_ST = pipeattribute->ulBaseUAddr;
	    pIbcPipeCtl->IBC_ADDR_V_ST = pipeattribute->ulBaseVAddr;
	}

	if ((pipeattribute->function == MMPF_IBC_FX_RING_BUF) || (pipeattribute->function == MMPF_IBC_FX_TOFB)) {
        if (pipeattribute->ulLineOffset) {
            pIbcPipeCtl->IBC_LINE_OFST = pipeattribute->ulLineOffset;
            pIbcPipeCtl->IBC_SRC_SEL |= IBC_LINEOFFSET_EN;
        }
	}

    if (pipeattribute->bMirrorEnable) {
        pIbcPipeCtl->IBC_BUF_CFG |= IBC_MIRROR_EN;
        pIbcPipeCtl->IBC_FRM_WIDTH = pipeattribute->usMirrorWidth;
    }
    else {
        pIbcPipeCtl->IBC_BUF_CFG &= ~IBC_MIRROR_EN;
        #if (CHIP == VSN_V2)
        pIbcPipeCtl->IBC_FRM_WIDTH = pipeattribute->usMirrorWidth; //patch for mirror
        #endif
    }

    pIbcPipeCtl->IBC_BUF_CFG |= (IBC_STORE_PIX_CONT | IBC_ICON_PAUSE_EN);
    pIbcPipeCtl->IBC_SRC_SEL = ((pIbcPipeCtl->IBC_SRC_SEL & ~IBC_SRC_SEL_MASK) | IBC_SRC_SEL_ICO(pipeattribute->InputSource));

	return	MMP_ERR_NONE;	
}

/** @brief The function enable store to frame buffer of the IBC

The function enable store to frame buffer of the IBC.

  @param[in] pipeID the IBC ID
  @param[in] bEnable enable or not mirror
  @param[in] usMirrorWidth IBC output frame width
  @return It reports the status of the operation.
*/
MMP_ERR MMPF_IBC_SetStoreEnable(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
	volatile AITPS_IBCP pIbcPipeCtl;

    if (pipeID == MMPF_IBC_PIPE_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (pipeID == MMPF_IBC_PIPE_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else if (pipeID == MMPF_IBC_PIPE_2) {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    else {
        return MMP_IBC_ERR_PARAMETER;
    }

    if (bEnable) {
        pIbcPipeCtl->IBC_BUF_CFG |= IBC_STORE_EN;
    }
    else {
        pIbcPipeCtl->IBC_BUF_CFG &= ~IBC_STORE_EN;
    }
    //if(pipeID==0) {
    //       dbg_printf(3,"IBC#%d :%d\r\n",pipeID,bEnable);
    //}
    return MMP_ERR_NONE;
}

/** @brief The function enable store to frame buffer of the IBC

The function get IBC store enable or not

  @param[in] pipeID the IBC ID
  @return It reports the status of the IBC store bit
*/
MMP_BOOL MMPF_IBC_GetStoreEnable(MMPF_IBC_PIPEID pipeID)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
	volatile AITPS_IBCP pIbcPipeCtl;

    if (pipeID == MMPF_IBC_PIPE_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (pipeID == MMPF_IBC_PIPE_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else if (pipeID == MMPF_IBC_PIPE_2) {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    else {
        return MMP_IBC_ERR_PARAMETER;
    }

    if(pIbcPipeCtl->IBC_BUF_CFG & IBC_STORE_EN ) {
        return MMP_TRUE ;
    }
    return MMP_FALSE;
}


/** @brief The function enable mirror of the IBC

The function enable mirror of the IBC. 

  @param[in] pipeID the IBC ID
  @param[in] bEnable enable or not mirror
  @param[in] usMirrorWidth IBC output frame width
  @return It reports the status of the operation.
*/
MMP_ERR MMPF_IBC_SetMirrorEnable(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable, MMP_USHORT usWidth)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
	volatile AITPS_IBCP pIbcPipeCtl;

    if (pipeID == MMPF_IBC_PIPE_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (pipeID == MMPF_IBC_PIPE_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else if (pipeID == MMPF_IBC_PIPE_2) {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    else {
        return MMP_IBC_ERR_PARAMETER;
    }

    if (bEnable) {
        pIbcPipeCtl->IBC_FRM_WIDTH = usWidth;
        pIbcPipeCtl->IBC_BUF_CFG |= IBC_MIRROR_EN;
    }
    else {
        pIbcPipeCtl->IBC_BUF_CFG &= ~IBC_MIRROR_EN;
    }

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_IBC_SetInterruptEnable(MMPF_IBC_PIPEID pipeID, MMPF_IBC_EVENT event, MMP_BOOL bEnable)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    static MMP_UBYTE ubFlags[MMPF_IBC_EVENT_MAX] = {IBC_INT_FRM_ST, IBC_INT_FRM_RDY, IBC_INT_FRM_END, IBC_INT_PRE_FRM_RDY};

    if (event >= MMPF_IBC_EVENT_MAX) {
        return MMP_IBC_ERR_PARAMETER;
    }

    if (pipeID == MMPF_IBC_PIPE_0) {
        if (bEnable) {
            pIBC->IBC_P0_INT_CPU_SR = ubFlags[event];
            pIBC->IBC_P0_INT_CPU_EN |= ubFlags[event];
        }
        else {
            pIBC->IBC_P0_INT_CPU_EN &= ~(ubFlags[event]);
        }
    }
    else if (pipeID == MMPF_IBC_PIPE_1) {
        if (bEnable) {
            pIBC->IBC_P1_INT_CPU_SR = ubFlags[event];
            pIBC->IBC_P1_INT_CPU_EN |= ubFlags[event];
        }
        else {
            pIBC->IBC_P1_INT_CPU_EN &= ~(ubFlags[event]);
        }
    }
    else if (pipeID == MMPF_IBC_PIPE_2) {
        if (bEnable) {
            pIBC->IBC_P2_INT_CPU_SR = ubFlags[event];
            pIBC->IBC_P2_INT_CPU_EN |= ubFlags[event];
        }
        else {
            pIBC->IBC_P2_INT_CPU_EN &= ~(ubFlags[event]);
        }
    }
    else {
        return MMP_IBC_ERR_PARAMETER;
    }
    //dbg_printf(3,"##IBC Intr:(%x,%x,%x)\r\n",pIBC->IBC_P0_INT_CPU_EN,pIBC->IBC_P1_INT_CPU_EN,pIBC->IBC_P2_INT_CPU_EN );
    return MMP_ERR_NONE;
}

MMP_ULONG MMPF_IBC_GetRingBufWrPtr(MMPF_IBC_PIPEID pipeID)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
	volatile AITPS_IBCP pIbcPipeCtl;

    if (pipeID == MMPF_IBC_PIPE_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (pipeID == MMPF_IBC_PIPE_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else if (pipeID == MMPF_IBC_PIPE_2) {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    else {
        return MMP_IBC_ERR_PARAMETER;
    }

    return pIbcPipeCtl->IBC_WR_PTR;
}

MMP_ERR MMPF_IBC_RegisterIntrCallBack(MMPF_IBC_PIPEID ibcpipe, MMPF_IBC_EVENT event, IbcCallBackFunc *pCallBack)
{
    if (pCallBack) {
        CallBackFuncIbc[event][ibcpipe] = pCallBack;
        return MMP_ERR_NONE;
    }

    return MMP_IBC_ERR_PARAMETER;
}

// --------------------SEAN ADD----------------------
MMP_ERR MMPF_IBC_Reset(MMP_BOOL *pipe_store_en )
{
    MMP_UBYTE i;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    for(i=0;i<MMPF_IBC_PIPE_MAX;i++) {
        pipe_store_en[i] = MMPF_IBC_GetStoreEnable(i);
    }
    pGBL->GBL_RST_CTL01 |= GBL_ICON_IBC_RST ;
    RTNA_WAIT_CYCLE(100);
    pGBL->GBL_RST_CTL01 &= ~GBL_ICON_IBC_RST ;
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_IBC_Restore(MMP_BOOL *pipe_store_en )
{
    MMP_UBYTE i;
    for(i=0;i<MMPF_IBC_PIPE_MAX;i++) {
        if(pipe_store_en[i]==MMP_TRUE) {
            MMPF_IBC_SetStoreEnable( i,MMP_TRUE);
        }
    }
}


MMP_ERR MMPF_IBC_EncoderIBCSrc(MMPF_IBC_PIPEID pipeID, MMPF_IBC_FX function)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;    
    if (function == MMPF_IBC_FX_JPG) {
        pIBC->IBC_JPEG_PIPE_SEL = (pIBC->IBC_JPEG_PIPE_SEL & ~IBC_JPEG_SRC_SEL_MASK) | IBC_JPEG_SRC_SEL(pipeID);
    }
    else if (function == MMPF_IBC_FX_H264) {
        pIBC->IBC_IMG_PIPE_CTL = (pIBC->IBC_IMG_PIPE_CTL & ~IBC_H264_SRC_SEL_MASK)   | IBC_H264_SRC_SEL(pipeID);
    }
    return MMP_ERR_NONE ;
}

/// @}
/// @end_ait_only

//#ifdef BUILD_CE
//#define BUILD_FW
//#endif
