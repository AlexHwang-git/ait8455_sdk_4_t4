/**
 @file mmps_3gprecd.c
 @brief Diamond 3GP Recorder Control Function
 @author Will Tseng
 @version 1.0
*/
#ifdef  BUILD_FW
#include "config_fw.h"
#endif
#include "mmp_lib.h"
#include "lib_retina.h"

#include "mmpf_mp4venc.h"
#include "mmpf_h264enc.h"
#include "mmpf_fctl.h"

#include "mmps_3gprecd.h"

/** @addtogroup MMPS_VIDRECD
@{
*/

#define FILL_CROP_BOTTOM_AREA           (1)
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

/// Video encode system mode
static MMPS_3GPRECD_MODES m_vidrecdModes;

/// Status of video preview
MMP_BOOL m_bVidRecdPreviewStatus[2] ={ MMP_FALSE , MMP_FALSE };

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

/**
@ait_only
*/

//------------------------------------------------------------------------------
/**
 @brief Set video preview mode.

 Video preview has two modes. One is for the system with stack memory. In order
 to suppor VGA, it uses scaler path 0 for preview and scaler path 1 for encode.
 The other one only supports to CIF and uses scaler path 1 for preview and encode.
 @param[in] *inputbuf HW using buffer.
 @param[in] *previewbuf Preview buffer.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetPreviewConfig(MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight,
                                        MMP_USHORT usEncWidth, MMP_USHORT usEncHeight,
                                        MMPF_VIDENC_FRAME_LIST *pFrameList, MMPF_FCTL_LINK *pVideoPipeLink,
                                        MMP_USHORT  usScaleResol, MMP_UBYTE ubEncId)
{
    MMPF_SCALER_FIT_RANGE           fitrange;
    MMPF_SCALER_GRABCONTROL         grabctl;
    MMPF_FCTL_LINK					fctllink;
    MMPF_FCTL_PREVIEWATTRIBUTE  	fctlpreviewattribute;
    MMP_USHORT						i;
    MMPF_VIDENC_CURBUF_MODE_CTL     CurBufCtl;

    MMPF_VIDENC_GetParameter(ubEncId, MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE, (void*)&CurBufCtl);

    fitrange.fitmode    = MMPF_SCALER_FITMODE_OUT;
    fitrange.usFitResol = usScaleResol;
    fitrange.usInWidth  = usSrcWidth;
    fitrange.usInHeight = usSrcHeight;
    fitrange.usOutWidth = usEncWidth;
    fitrange.usOutHeight = usEncHeight;
    MMPF_Scaler_GetBestFitScale(&fitrange, &grabctl);
    fctllink = *pVideoPipeLink;
#if SUPPORT_GRA_ZOOM &&(CHIP==VSN_V3)
    if (fctllink.scalerpath==MMPF_SCALER_PATH_1) {
        if( (usEncHeight==1088) &&  (usEncWidth == 1920 )) {
            grabctl.usStartX = 1 ;
            grabctl.usStartY = 1 ;
            grabctl.usEndX = 1920  ;
            grabctl.usEndY = 1088 ;
        }
    }
#endif

    fctlpreviewattribute.fctllink = fctllink;
    fctlpreviewattribute.usInputW = usSrcWidth;
    fctlpreviewattribute.usInputH = usSrcHeight;
    fctlpreviewattribute.grabctl = grabctl;
    fctlpreviewattribute.usBufferCount = pFrameList[ubEncId].ulFrameCnt;
    for (i = 0; i < fctlpreviewattribute.usBufferCount; i++) {
        fctlpreviewattribute.ulBaseAddr[i] = pFrameList[ubEncId].FrameList[i].ulYAddr;
        fctlpreviewattribute.ulBaseUAddr[i] = pFrameList[ubEncId].FrameList[i].ulUAddr;
        fctlpreviewattribute.ulBaseVAddr[i] = pFrameList[ubEncId].FrameList[i].ulVAddr;
    }
    fctlpreviewattribute.bMirrorEnable = MMP_FALSE;
    fctlpreviewattribute.bUseRotateDMA = MMP_FALSE;
    if (CurBufCtl.InitCurBufMode == MMPF_VIDENC_CURBUF_FRAME) {
        fctlpreviewattribute.IbcLinkFx = MMPF_IBC_FX_TOFB;
        fctlpreviewattribute.colormode = MMPF_IBC_COLOR_NV12;
    }
    else {
        fctlpreviewattribute.IbcLinkFx = MMPF_IBC_FX_H264;
        fctlpreviewattribute.colormode = MMPF_IBC_COLOR_I420;
    }

    MMPF_Fctl_SetPreivewAttributes(&fctlpreviewattribute);
    MMPF_Fctl_LinkPreviewToVideo(fctllink.ibcpipeID, ubEncId);
   	MMPF_Fctl_SetLPFMaster(fctllink.ibcpipeID);

    if (fctlpreviewattribute.colormode == MMPF_IBC_COLOR_NV12) {
        MMPF_VIDENC_SetParameter(ubEncId, MMPF_VIDENC_ATTRIBUTE_INPUT_COLORMODE,
                                    (void*)MMPF_VIDENC_COLORMODE_NV12);
    }
    else {
        MMPF_VIDENC_SetParameter(ubEncId, MMPF_VIDENC_ATTRIBUTE_INPUT_COLORMODE,
                                    (void*)MMPF_VIDENC_COLORMODE_I420);
    }

    return MMP_ERR_NONE;
}

/**
@end_ait_only
*/

//------------------------------------------------------------------------------
/**
 @brief Stop video recording and fill 3GP tail.

 It works after video start, pause and resume.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
 @retval MMP_3GPRECD_ERR_OPEN_FILE_FAILURE Open file failed.
 @retval MMP_3GPRECD_ERR_CLOSE_FILE_FAILURE Close file failed.
*/
MMP_ERR MMPS_3GPRECD_StopRecord(void)
{
extern MMPF_OS_SEMID   gVidRecdSemID;
    MMP_ULONG timeout = 0;
    MMPD_MP4VENC_FW_OP status_fw = MMPD_VIDENC_CheckStatus();

    if ((status_fw == MMPD_MP4VENC_FW_OP_START) ||
        (status_fw == MMPD_MP4VENC_FW_OP_PAUSE) ||
        (status_fw == MMPD_MP4VENC_FW_OP_RESUME) ||
        (status_fw == MMPD_MP4VENC_FW_OP_STOP)) {
        MMPF_VIDENC_Stop();

        RTNA_DBG_Str3("Waiting Stop...\r\n");     
        do {
            MMPD_VIDENC_GetStatus(&status_fw);
            MMPF_OS_Sleep(1);
			timeout ++ ;
			if(timeout >= 1000){
				RTNA_DBG_Str3("Waiting Stop...timeout\r\n");
				MMPF_OS_SetSem(gVidRecdSemID,1);
				break;
			}
        } while (status_fw != MMPD_MP4VENC_FW_OP_STOP);
        RTNA_DBG_Str3("Waiting Stop End\r\n");     

        MMPD_VIDENC_EnableClock(MMP_FALSE);

        return MMP_ERR_NONE;
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
}

//------------------------------------------------------------------------------
/**
 @brief Start video recording.

 It can saves the 3GP file to host memory or memory card.
 @retval MMP_ERR_NONE Success.
 @retval MMP_3GPRECD_ERR_GENERAL_ERROR Not allowed procedure.
*/
//extern MMP_ERR MMPD_VIDENC_GetStatus(MMPD_MP4VENC_FW_OP *status);
MMP_ERR MMPS_3GPRECD_StartRecord(void)
{
    MMPD_MP4VENC_FW_OP status_vid = 0 ;
    MMP_ERR status_mgr;
    MMPD_VIDENC_GetStatus(&status_vid);

    if ((status_vid == MMPD_MP4VENC_FW_OP_NONE) ||
        (status_vid == MMPD_MP4VENC_FW_OP_STOP)) {
        MMPD_VIDENC_EnableClock(MMP_TRUE);
        MMPD_VIDENC_StartCapture();
        do {
            MMPD_VIDENC_GetStatus(&status_vid); 
        } while (status_vid != MMPD_MP4VENC_FW_OP_START);
        if (status_vid == MMPD_MP4VENC_FW_OP_START) {
            return MMP_ERR_NONE;
        }
        else if (status_mgr == MMP_FS_ERR_OPEN_FAIL) {
            return MMP_3GPRECD_ERR_OPEN_FILE_FAILURE;
        }
        else {
            return MMP_ERR_NONE;
        }
    }
    else {
        return MMP_3GPRECD_ERR_GENERAL_ERROR;
    }
}

//------------------------------------------------------------------------------
/**
 @brief Set encoded resolution.
 @param[in] usEncWidth Width for each instance.
 @param[in] usEncHeight Height for each instance.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetResolution(MMP_USHORT usEncWidth[], MMP_USHORT usEncHeight[], MMP_UBYTE ubEncResolNum)
{
    MMP_USHORT i;
    MMP_USHORT usMaxResolIdx;

    if (ubEncResolNum == 0) {
        return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
    }

    if (ubEncResolNum > MAX_VIDENC_NUM) {
        ubEncResolNum = MAX_VIDENC_NUM;
    }

    // swap max width to 1st
    usMaxResolIdx = 0;
    for (i = 0; i < ubEncResolNum; i++) {
        if (usEncWidth[i] > usEncWidth[usMaxResolIdx]) {
            usMaxResolIdx = i;
        }
        if ((usEncWidth[i] & 0x0F) || (usEncHeight[i] & 0x0F)) {
            RTNA_DBG_Str(0, "Error : width/height not 16 alignment !\r\n");
            return MMP_ERR_NONE;
        }
    }
    if (usMaxResolIdx != 0) {
        i = usEncWidth[0];
        usEncWidth[0] = usEncWidth[usMaxResolIdx];
        usEncWidth[usMaxResolIdx] = i;
        i = usEncHeight[0];
        usEncHeight[0] = usEncHeight[usMaxResolIdx];
        usEncHeight[usMaxResolIdx] = i;
    }

    m_vidrecdModes.usVideoTotalNum = ubEncResolNum;
    for (i = 0; i < ubEncResolNum; i++) {
        m_vidrecdModes.usWidth[i] = usEncWidth[i];
        m_vidrecdModes.usHeight[i] = usEncHeight[i];
    }

    MMPF_VIDENC_SetTotalPipeNum(ubEncResolNum);
    for (i = 0; i < ubEncResolNum; i++) {
        MMPF_H264ENC_InitResolution(i, usEncWidth[i], usEncHeight[i]);
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
/**
 @brief Initialize video encoder default info

 @param[in] 
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_InitVideoInfo(void)
{
    MMP_USHORT i;

    for (i = 0; i < MAX_VIDENC_NUM; i++) {
        MMPF_H264ENC_Initilize(i);
        MMPF_VIDMGR_SetOutputPipe(i, 1);
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
/**
 @brief Set memory layout by different resolution and memory type for H264.

 Depends on encoded resolution and memory type to map buffers. It supports two types
 of memory, INTER(823) and STACK(821).
 @param[in/out] ulFBufAddr Available start address of frame buffer.
 @param[in/out] ulStackAddr Available start address of dram buffer.
 @param[out] pInputFrames returns the allocated input frame buffers, 
                no frame will be allocated when realtime mode
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_SetMemoryMapH264(MMP_ULONG *ulFBufAddr, MMP_ULONG *ulStackAddr,
                                        MMPF_VIDENC_FRAME_LIST *pInputFrames, MMP_ULONG ulRingBufSize)
{
    MMP_USHORT                      i, usMaxWidth, usMaxRtWidth, usStreamNum;
    MMP_ULONG                       sram_addr, dram_addr, bufsize;
    MMPD_MP4VENC_VIDEOBUF           videohwbuf;
    MMPF_VIDENC_GOP_CTL             GopCtl;
    MMPF_VIDENC_CURBUF_MODE_CTL     CurBufCtl;
    MMPF_VIDENC_FRAME               CurLineBuf[2];
    MMPF_VIDENC_RESOLUTION          MaxResvResol = {0, 0};
    MMPF_VIDENC_REFGENBUF_MODE      RefGenBufMode;
    MMPF_VIDENC_BUFCFG              MaxRtWidthBufCfg;
    #if (CHIP == VSN_V2)
    MMP_UBYTE                       linebufcount = 6;
    MMP_UBYTE                       up_line_count = 0;
    #endif

    usStreamNum = MMPF_VIDENC_GetTotalPipeNum();

for (i = 0; i < usStreamNum; i++) {
    dbg_printf(0,"H264 W,H=(%d,%d)\r\n",m_vidrecdModes.usWidth[i],m_vidrecdModes.usHeight[i]);
}
    sram_addr = *ulFBufAddr;
    dram_addr = *ulStackAddr;

    for (i = 0, usMaxWidth = 0, usMaxRtWidth = 0; i < usStreamNum; i++) {
        MMP_USHORT  usResvMaxWidth;

        MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_REFGENBUF_MODE, (void*)&RefGenBufMode);
        MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE, (void*)&CurBufCtl);
        MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_RESV_MAX_RESOLUTION, (void*)&MaxResvResol);

        usResvMaxWidth = (m_vidrecdModes.usWidth[i] > MaxResvResol.usWidth)?
                                m_vidrecdModes.usWidth[i]: MaxResvResol.usWidth;

        if ((usMaxWidth < usResvMaxWidth) &&
            (RefGenBufMode != MMPF_VIDENC_REFGENBUF_NONE)) {
            usMaxWidth = m_vidrecdModes.usWidth[i];
        }
        if ((usMaxRtWidth < usResvMaxWidth) && 
            (CurBufCtl.CurBufRTConfig != MMPF_VIDENC_BUFCFG_NONE)) {
            usMaxRtWidth = m_vidrecdModes.usWidth[i];
            MaxRtWidthBufCfg = CurBufCtl.CurBufRTConfig;
        }
    }

    #if (CHIP == P_V2) || (CHIP == VSN_V2)
    if (usMaxWidth == 0) {
        for (i = 0; i < up_line_count; i++) {
            videohwbuf.linebuf.ulUP[i] = 0;
        }
        for (i = 0; i < linebufcount; i++) {
            videohwbuf.linebuf.ulY[i] = 0;
            videohwbuf.linebuf.ulU[i] = 0;
            videohwbuf.linebuf.ulV[i] = 0;
        }
        #if (CHIP == VSN_V2)
        videohwbuf.linebuf.ulDeblockRow = 0;
        #endif
    }
    else {
        for (i = 0; i < up_line_count; i++) {
            videohwbuf.linebuf.ulUP[i] = sram_addr;
            sram_addr += (usMaxWidth << 2);
        }
        for (i = 0; i < linebufcount; i++) {
            videohwbuf.linebuf.ulY[i] = sram_addr;
            sram_addr += (usMaxWidth << 3); //pipeline mode

            videohwbuf.linebuf.ulU[i] = sram_addr;
            sram_addr += (usMaxWidth << 2);
            videohwbuf.linebuf.ulV[i] = sram_addr;
            sram_addr += (usMaxWidth << 2);
        }
        #if (CHIP == VSN_V2)
        videohwbuf.linebuf.ulDeblockRow = sram_addr;
        sram_addr += (usMaxWidth << 3);
        #endif
    }
    #endif //(CHIP == P_V2) || (CHIP == VSN_V2)

    dram_addr = ALIGN32(dram_addr);
    // Set Slice Length Buffer, align32
    videohwbuf.miscbuf.ulSliceLenBuf = dram_addr;
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    dram_addr = dram_addr + 4 * (MAX_SLICE_NUM*10 + 2);
    #endif

    // Set MV buffer, #MB/Frame * #MVs/MB * #byte/MV
    for (i = 0; i < usStreamNum; i++) {
        MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_REFGENBUF_MODE, (void*)&RefGenBufMode);
    	if (RefGenBufMode == MMPF_VIDENC_REFGENBUF_NONE) {
    	    continue;
    	}

    	dram_addr = (dram_addr + 63) >> 6 << 6; //align64
        MMPF_H264ENC_InitMVBuf(dram_addr, MMPF_H264ENC_GetHandle(i));
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        dram_addr += ((m_vidrecdModes.usWidth[i]/16)*(m_vidrecdModes.usHeight[i]/16))*4*4;
        #endif
        #if (CHIP == P_V2)
        dram_addr += ((m_vidrecdModes.usWidth[i]/16)*(m_vidrecdModes.usHeight[i]/16))*16*4;
        #endif
	}

    for (i = 0; i < usStreamNum; i++) {
        MMPF_H264ENC_InitRefList(&dram_addr, MMPF_H264ENC_GetHandle(i));
    }

	if (usMaxRtWidth) {
        switch (MaxRtWidthBufCfg) {
        case MMPF_VIDENC_BUFCFG_SRAM:
            bufsize = (usMaxRtWidth << 4);

            CurLineBuf[0].ulYAddr = sram_addr;
            sram_addr += bufsize;
            CurLineBuf[0].ulUAddr = sram_addr;
            sram_addr += (bufsize >> 2);
            CurLineBuf[0].ulVAddr = sram_addr;
            sram_addr += (bufsize >> 2);

            CurLineBuf[1].ulYAddr = sram_addr;
            sram_addr += bufsize;
            CurLineBuf[1].ulUAddr = sram_addr;
            sram_addr += (bufsize >> 2);
            CurLineBuf[1].ulVAddr = sram_addr;
            sram_addr += (bufsize >> 2);
            break;
        case MMPF_VIDENC_BUFCFG_DRAM:
            bufsize = (usMaxRtWidth << 4);

            CurLineBuf[0].ulYAddr = dram_addr;
            dram_addr += bufsize;
            CurLineBuf[0].ulUAddr = dram_addr;
            dram_addr += (bufsize >> 2);
            CurLineBuf[0].ulVAddr = dram_addr;
            dram_addr += (bufsize >> 2);

            CurLineBuf[1].ulYAddr = dram_addr;
            dram_addr += bufsize;
            CurLineBuf[1].ulUAddr = dram_addr;
            dram_addr += (bufsize >> 2);
            CurLineBuf[1].ulVAddr = dram_addr;
            dram_addr += (bufsize >> 2);
            break;
        case MMPF_VIDENC_BUFCFG_INTLV:
            bufsize = (usMaxRtWidth << 4);

            CurLineBuf[0].ulYAddr = sram_addr;
            sram_addr += bufsize;
            CurLineBuf[0].ulUAddr = sram_addr;
            sram_addr += (bufsize >> 2);
            CurLineBuf[0].ulVAddr = sram_addr;
            sram_addr += (bufsize >> 2);

            CurLineBuf[1].ulYAddr = dram_addr;
            dram_addr += bufsize;
            CurLineBuf[1].ulUAddr = dram_addr;
            dram_addr += (bufsize >> 2);
            CurLineBuf[1].ulVAddr = dram_addr;
            dram_addr += (bufsize >> 2);
            break;
        }
	}
	for (i = 0; i < usStreamNum; i++) {
        MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE, (void*)&CurBufCtl);

        if (CurBufCtl.CurBufFrameConfig == MMPF_VIDENC_BUFCFG_DRAM) { //only support DRAM
            MMP_ULONG   j;
    	    MMP_USHORT  usResvMaxWidth, usResvMaxHeight;
            #if (FILL_CROP_BOTTOM_AREA == 1)
            MMPF_VIDENC_CROPPING    Crop;
            #endif

            MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_GOP_CTL, (void*)&GopCtl);
            MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_RESV_MAX_RESOLUTION, (void*)&MaxResvResol);
            #if (FILL_CROP_BOTTOM_AREA == 1)
            MMPF_VIDENC_GetParameter(i, MMPF_VIDENC_ATTRIBUTE_CROPPING, (void*)&Crop);
            #endif

            usResvMaxWidth = (m_vidrecdModes.usWidth[i] > MaxResvResol.usWidth)?
                                    m_vidrecdModes.usWidth[i]: MaxResvResol.usWidth;
            usResvMaxHeight = (m_vidrecdModes.usHeight[i] > MaxResvResol.usHeight)?
                                    m_vidrecdModes.usHeight[i]: MaxResvResol.usHeight;
            #if (CHIP == VSN_V3)
            pInputFrames[i].ulFrameCnt = 2 + GopCtl.usMaxContBFrameNum;
            #endif
            #if (CHIP == VSN_V2) || (CHIP == P_V2)
            pInputFrames[i].ulFrameCnt = 1 + GopCtl.usMaxContBFrameNum;
            #endif

            bufsize = usResvMaxWidth * usResvMaxHeight;
            for (j = 0; j < pInputFrames[i].ulFrameCnt; j++) {
                pInputFrames[i].FrameList[j].ulYAddr = dram_addr;
                dram_addr += bufsize;

                pInputFrames[i].FrameList[j].ulUAddr = dram_addr;
                dram_addr += (bufsize >> 2);
                pInputFrames[i].FrameList[j].ulVAddr = dram_addr;
                dram_addr += (bufsize >> 2);

                #if (FILL_CROP_BOTTOM_AREA == 1)
                MEMSET((void*)(pInputFrames[i].FrameList[j].ulYAddr +
                        m_vidrecdModes.usWidth[i]*(m_vidrecdModes.usHeight[i]-Crop.usBottom)),
                        0, (m_vidrecdModes.usWidth[i] * Crop.usBottom));
                // Frame mode always NV12
                MEMSET((void*)(pInputFrames[i].FrameList[j].ulUAddr +
                    m_vidrecdModes.usWidth[i]*((m_vidrecdModes.usHeight[i]-Crop.usBottom)>>1)),
                    0x80, (m_vidrecdModes.usWidth[i] * Crop.usBottom));
                #endif
            }
        }
        else if (CurBufCtl.InitCurBufMode == MMPF_VIDENC_CURBUF_FRAME) {
            RTNA_DBG_Str(0, "#Error : Cur frame buf not allocate!\r\n");
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
        if (CurBufCtl.CurBufFrameConfig == MMPF_VIDENC_BUFCFG_NONE) {
            pInputFrames[i].ulFrameCnt = 0;
        }

        if (CurBufCtl.CurBufRTConfig != MMPF_VIDENC_BUFCFG_NONE) {
            MMPF_H264ENC_SetRealtimeLineBuf(MMPF_H264ENC_GetHandle(i), CurLineBuf);
        }
        else if (CurBufCtl.InitCurBufMode == MMPF_VIDENC_CURBUF_RT) {
            RTNA_DBG_Str(0, "#Error : Cur RT buf not allocate!\r\n");
            return MMP_3GPRECD_ERR_UNSUPPORTED_PARAMETERS;
        }
	}

	// set video compressed buffer, 32 byte alignment
	dram_addr = ALIGN32(dram_addr);

    #if (MGR_PROC_EN == 1)
    if (MMPF_VIDMGR_GetRingBufEnable()) {
        MMPF_VIDMGR_SetVideoCompressedBuffer(dram_addr, ulRingBufSize);
    	dram_addr += ulRingBufSize;
    }
    #endif

    MMPD_H264ENC_SetMiscBuf(&(videohwbuf.miscbuf));
    MMPD_H264ENC_SetLineBuf(&(videohwbuf.linebuf));
    MMPD_H264ENC_SetEncodeMode();

	// Log End of H264 address
	*ulFBufAddr = sram_addr;
	*ulStackAddr = dram_addr;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
/**
 @brief Return in video preview status or not.
 @param[in] bEnable preview enable.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_3GPRECD_GetPreviewDisplay(MMP_BOOL *bEnable)
{
    *bEnable = m_bVidRecdPreviewStatus[0] | m_bVidRecdPreviewStatus[1];
    return MMP_ERR_NONE;
}

MMP_ERR MMPS_3GPRECD_GetPreviewDisplayByEpId(MMP_BOOL *bEnable,MMP_UBYTE ep_id)
{
    *bEnable = m_bVidRecdPreviewStatus[ep_id] ;
    return MMP_ERR_NONE;
}

/// @}
