/**
 @file mmpf_3gpmgr.c
 @brief Control functions of 3GP merger
 @author Will Tseng
 @version 1.0
*/

#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_3gpmgr.h"
#include "mmpf_mp4venc.h"
#include "mmpf_h264enc.h"
#include "mmpf_dma.h"

#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc.h"

/** @addtogroup MMPF_VIDMGR
@{
*/
#define MAX_MGR_NUM     (1)

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

#if (MGR_PROC_EN == 1)
MMP_BOOL        bVidMgrRingBufEn = MMP_TRUE;        ///< video always use ring buf
#else
const MMP_BOOL  bVidMgrRingBufEn = MMP_FALSE;
#endif
#if (MGR_PROC_EN == 1) && (MTS_MUXER_EN == 1)
MMP_BOOL        bVidMgrMtsMuxEn = MMP_FALSE;        ///< enable mpeg transport stream or not
#else
const MMP_BOOL  bVidMgrMtsMuxEn = MMP_FALSE;
#endif

MMP_UBYTE       bVidMgrDstPipeId[MAX_NUM_ENC_SET];  ///< use pipe 0 or 1 slot buffer

#if (MTS_MUXER_EN == 1)
///< MPEG transport stream
MMPF_MTS_PSI                MtsPatConfig;
MMPF_MTS_PSI                MtsPmtConfig;
MMP_ULONG                   glMtsPatPayloadLen;
MMP_ULONG                   glMtsPmtPayloadLen;
MMP_ULONG                   glMtsSitPayloadLen;
MMP_UBYTE                   MtsPatPayload[MTS_TS_PACKET_SIZE];
MMP_UBYTE                   MtsPmtPayload[MTS_TS_PACKET_SIZE];
MMP_UBYTE                   MtsSitPayload[MTS_TS_PACKET_SIZE];
MMPF_VIDMGR_MTS_PAYLAOD     MtsPayload[MAX_NUM_ENC_SET]; ///< for each encoder
MMPF_DMA_M_LOFFS_DATA       MtsPacketSplitOffset = {184, 184, 184, 188};
#endif

MMPF_VIDMGR_HANDLE          m_VidMgrHandle[MAX_MGR_NUM];

#if (MGR_PROC_EN == 1)
MMP_ULONG       glVidRecdCompBufAddr;               ///< start address of video encoded compressed buffer
MMP_ULONG       glVidRecdCompBufSize;               ///< size of video encoded compressed buffer
MMP_ULONG       glVidRecdCompBufReadPtr;           	///< read-out address of video encoded compressed buffer
MMP_ULONG       glVidRecdCompBufWritePtr;          	///< write-in address of video encoded compressed buffer
MMP_ULONG       glVidRecdCompBufReadWrap;          	///< wrap times of read ptr
MMP_ULONG       glVidRecdCompBufWriteWrap;         	///< wrap times of write ptr
MMP_ULONG		glVidRecdCompBufFreeSize;			///< maximum free size for next frame

static MMPF_OS_SEMID        gCatDMASemID;
static volatile MMP_BOOL    m_bMgrDmaMoveWorkingFlag = MMP_FALSE;
#endif

MMP_ULONG		gl3gpTimeCntInMS;					///< timer counter in ms

extern MMPF_OS_FLAGID                   SYS_Flag_Hif;
extern volatile MMP_ULONG               PTS; //sean@2011_01_22 add timestamp

void usb_fill_payload_header (MMP_UBYTE eid,MMP_UBYTE *frame_ptr, MMP_ULONG framelength, MMP_ULONG frameseq,
                                MMP_ULONG flag, MMP_ULONG timestamp, MMP_USHORT w, MMP_USHORT h,
                                MMP_USHORT framerate, MMP_UBYTE ubPipe);
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

/**
 @brief Initialization before video start.

 This function initialize frame size queue, end frame done flag, video frame and
 chunk counter, audio frame and chunk counter, read/write ptr of video and audio
 compressed buffer, read/write ptr of AV repack buffer and frame table buffer,
 sync buffer for memory mode, the counter of I and P frames in one cycle.
*/
void MMPF_VIDMGR_Initialization(MMP_UBYTE ubMgrId)
{
	gl3gpTimeCntInMS = 0;

    #if (MGR_PROC_EN == 1)
    glVidRecdCompBufReadPtr     = 0;
	glVidRecdCompBufWritePtr    = 0;
	glVidRecdCompBufReadWrap    = 0;
	glVidRecdCompBufWriteWrap   = 0;
	glVidRecdCompBufFreeSize    = glVidRecdCompBufSize;

    m_VidMgrHandle[ubMgrId].DescQueueRdIdx = 0;
    m_VidMgrHandle[ubMgrId].DescQueueRdWrap = 0;
    m_VidMgrHandle[ubMgrId].DescQueueWrIdx = 0;
    m_VidMgrHandle[ubMgrId].DescQueueWrWrap = 0;
    #endif
    m_VidMgrHandle[ubMgrId].ulSequenceCnt = 1;
}

/**
 @brief Set current encode done encoder id, and set flag to info mgr task
 @param[in] ubEncID ID of the encoder
 @retval MMP_ERR_NONE Success.
*/
#if (MGR_PROC_EN == 1)
MMP_ERR MMPF_VIDMGR_CheckDataReady(MMPF_VIDMGR_HANDLE *MgrHandle)
{
    if (MgrHandle->ubMgrId== 0) {
        MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_MGR_0, MMPF_OS_FLAG_SET);
    }
    else { //TODO
    }

    return MMP_ERR_NONE;
}
#endif


MMPF_VIDMGR_HANDLE *MMPF_VIDMGR_GetHandle(MMP_UBYTE ubMgrId)
{
    if (ubMgrId >= MAX_MGR_NUM) {
        ubMgrId = 0;
    }
    return &(m_VidMgrHandle[ubMgrId]);
}

#if (MGR_PROC_EN == 1)
MMP_ERR MMPF_VIDMGR_GetDescQueueDepth (MMP_ULONG *ulDepth, MMPF_VIDMGR_HANDLE *pMgr)
{
    #if (OS_CRITICAL_METHOD == 3)
    OS_CPU_SR cpu_sr = 0;
    #endif
    MMP_ULONG   ulRdWrap, ulWrWrap, ulRdIdx, ulWrIdx;

    OS_ENTER_CRITICAL();
    ulRdWrap    = pMgr->DescQueueRdWrap;
    ulRdIdx     = pMgr->DescQueueRdIdx;
    ulWrWrap    = pMgr->DescQueueWrWrap;
    ulWrIdx     = pMgr->DescQueueWrIdx;
    OS_EXIT_CRITICAL();

    *ulDepth = 0;
    if (ulRdWrap > ulWrWrap) {
        return MMP_VIDMGR_ERR_DESCQ_UNDERFLOW;
    }
    else if (ulRdWrap == ulWrWrap) {
        if (ulRdIdx > ulWrIdx) {
            return MMP_VIDMGR_ERR_DESCQ_UNDERFLOW;
        }
        else {
            *ulDepth = ulWrIdx - ulRdIdx;
            return MMP_ERR_NONE;
        }
    }
    else if ((ulRdWrap+1) == ulWrWrap) {
        if (ulRdIdx < ulWrIdx) {
            return MMP_VIDMGR_ERR_DESCQ_OVERFLOW;
        }
        else {
            *ulDepth = MMPF_VIDMGR_FRAME_QUEUE_SIZE - ulRdIdx + ulWrIdx;
            return MMP_ERR_NONE;
        }
    }
    else {
        return MMP_VIDMGR_ERR_DESCQ_OVERFLOW;
    }
}

MMPF_VIDMGR_CODED_DESC *MMPF_VIDMGR_GetDescQueueWrPtr (MMPF_VIDMGR_HANDLE *pMgr)
{
    return &(pMgr->DescQueue[pMgr->DescQueueWrIdx]);
}

MMPF_VIDMGR_CODED_DESC *MMPF_VIDMGR_GetDescQueueRdPtr (MMPF_VIDMGR_HANDLE *pMgr)
{
    return &(pMgr->DescQueue[pMgr->DescQueueRdIdx]);
}

void MMPF_VIDMGR_UpdateDescQueueWrIdx (MMPF_VIDMGR_HANDLE *pMgr)
{
    pMgr->DescQueueWrIdx++;
    if (pMgr->DescQueueWrIdx >= MMPF_VIDMGR_FRAME_QUEUE_SIZE) {
        pMgr->DescQueueWrIdx = 0;
        pMgr->DescQueueWrWrap++;
    }
}

void MMPF_VIDMGR_UpdateDescQueueRdIdx (MMPF_VIDMGR_HANDLE *pMgr)
{
    pMgr->DescQueueRdIdx++;
    if (pMgr->DescQueueRdIdx >= MMPF_VIDMGR_FRAME_QUEUE_SIZE) {
        pMgr->DescQueueRdIdx = 0;
        pMgr->DescQueueRdWrap++;
    }
}
#endif

/**
 @brief Get the start address and size of video compressed buffer from host
 parameters.
 @param[in] startaddr Start address.
 @param[in] bufsize Buffer size.
*/
#if (MGR_PROC_EN == 1)
void MMPF_VIDMGR_SetVideoCompressedBuffer(MMP_ULONG startaddr, MMP_ULONG bufsize)
{
    glVidRecdCompBufAddr = startaddr;
    glVidRecdCompBufSize = bufsize;
}
#endif

/**
 @brief Set number to time counter.
 @param[in] timecnt Initial value.
*/
void MMPF_VIDMGR_SetTimeCounter(MMP_ULONG timecnt)
{
	gl3gpTimeCntInMS = timecnt;
}

/**
 @brief Get time counter.
 @return Time counter.
*/
MMP_ULONG MMPF_VIDMGR_GetTimeCounter(void)
{
	return gl3gpTimeCntInMS;
}

/**
 @brief callback for of H264ENC dma cat done

 This callback clear the flag
*/
#if (MGR_PROC_EN == 1)
void MMPF_VIDMGR_CatDMADone(void)
{
    m_bMgrDmaMoveWorkingFlag = MMP_FALSE;

	if (MMPF_OS_ReleaseSem(gCatDMASemID) != OS_NO_ERR) {
		RTNA_DBG_Str(0, "gCatDMASemID OSSemPost: Fail \r\n");
	}
}
#endif

MMP_ERR MMPF_VIDMGR_GetTxBufInfo(MMP_UBYTE ubEncId, MMP_ULONG *ulStartAddr,
                                    MMP_ULONG *ulLowBound, MMP_ULONG *ulHighBound,
                                    MMP_ULONG *ulMaxFreeSize)
{
    MMP_ULONG   ulSlotStart, ulSlotSize;
    MMP_USHORT  usSlotNum;

    MMPF_Video_GetSlotInfo(1, bVidMgrDstPipeId[ubEncId], &ulSlotStart, &ulSlotSize, &usSlotNum);

    MMPF_Video_GetBufBound(bVidMgrDstPipeId[ubEncId], ulLowBound, ulHighBound);

    *ulStartAddr = (MMP_ULONG)MMPF_Video_CurWrPtr(bVidMgrDstPipeId[ubEncId]) + FRAME_PAYLOAD_HEADER_SZ;

    #if (SLOT_RING == 1)
    *ulMaxFreeSize = ulSlotSize*MMPF_Video_GetEmptySlotNum(bVidMgrDstPipeId[ubEncId]);
    *ulMaxFreeSize = (*ulMaxFreeSize > FRAME_PAYLOAD_HEADER_SZ)? (*ulMaxFreeSize - FRAME_PAYLOAD_HEADER_SZ): 0;
    #else
    if (MMPF_Video_IsFull(bVidMgrDstPipeId[ubEncId])) {
        *ulMaxFreeSize = 0;
    }
    else {
        *ulMaxFreeSize = ulSlotSize;
        *ulMaxFreeSize = (*ulMaxFreeSize > FRAME_PAYLOAD_HEADER_SZ)? (*ulMaxFreeSize - FRAME_PAYLOAD_HEADER_SZ): 0;
    }
    #endif

    return MMP_ERR_NONE;
}

/**
 @brief Get the buf info for write next frame.

 @retval Write Address.
*/
MMP_ERR MMPF_VIDMGR_GetWriteBufInfo(MMP_UBYTE ubEncId, MMP_ULONG *ulStartAddr,
                                    MMP_ULONG *ulLowBound, MMP_ULONG *ulHighBound,
                                    MMP_ULONG *ulMaxFreeSize)
{
    MMP_ERR ret = MMP_ERR_NONE;

    #if (MGR_PROC_EN == 1)
    if (bVidMgrRingBufEn) {
        *ulStartAddr = glVidRecdCompBufAddr + glVidRecdCompBufWritePtr;

	    *ulLowBound = glVidRecdCompBufAddr;
	    *ulHighBound = glVidRecdCompBufAddr + glVidRecdCompBufSize;

        if (glVidRecdCompBufWriteWrap == glVidRecdCompBufReadWrap) {
            if (glVidRecdCompBufWritePtr < glVidRecdCompBufReadPtr) {
                *ulMaxFreeSize = glVidRecdCompBufFreeSize = 0;
                ret = MMP_3GPMGR_ERR_AVBUF_UNDERFLOW;
            }
            else {
                glVidRecdCompBufFreeSize = glVidRecdCompBufSize - glVidRecdCompBufWritePtr + glVidRecdCompBufReadPtr;
                *ulMaxFreeSize = glVidRecdCompBufFreeSize;
            }
        }
        else if (glVidRecdCompBufWriteWrap == (glVidRecdCompBufReadWrap+1)) {
            if (glVidRecdCompBufWritePtr > glVidRecdCompBufReadPtr) {
                *ulMaxFreeSize = glVidRecdCompBufFreeSize = 0;
                ret = MMP_3GPMGR_ERR_AVBUF_OVERFLOW;
            }
            else {
                glVidRecdCompBufFreeSize = glVidRecdCompBufReadPtr - glVidRecdCompBufWritePtr;
                *ulMaxFreeSize = glVidRecdCompBufFreeSize;
            }
        }
        else {
            *ulMaxFreeSize = 0;
            ret = MMP_3GPMGR_ERR_AVBUF_FAILURE;
        }
	}
	else
	#endif
	{
	    MMPF_VIDMGR_GetTxBufInfo(ubEncId, ulStartAddr, ulLowBound, ulHighBound, ulMaxFreeSize);
	}

	return ret;
}

/**
 @brief Update video ring buffer current write pointer
 @param[in] CurSize current write out frame size
*/
#if (MGR_PROC_EN == 1)
MMP_ERR MMPF_VIDMGR_UpdateVideoWritePtr (MMP_ULONG CurSize)
{
	if (bVidMgrRingBufEn) {
        glVidRecdCompBufWritePtr += CurSize;
        if (glVidRecdCompBufWritePtr >= glVidRecdCompBufSize) {
        	glVidRecdCompBufWritePtr -= glVidRecdCompBufSize;
        	glVidRecdCompBufWriteWrap++;
        }
        if (CurSize > glVidRecdCompBufFreeSize) {
        	RTNA_DBG_Str(0, "Frame size over available area.\r\n");
        	RTNA_DBG_PrintLong(0, CurSize);
        	RTNA_DBG_PrintLong(0, glVidRecdCompBufFreeSize);
        	return MMP_3GPMGR_ERR_AVBUF_OVERFLOW;
        }
    }

    return MMP_ERR_NONE;
}

/**
 @brief Update video ring buffer current read pointer
 @param[in] CurSize current read out frame size
*/
void MMPF_VIDMGR_UpdateVideoReadPtr (MMP_ULONG CurSize)
{
	if (bVidMgrRingBufEn) {
        glVidRecdCompBufReadPtr += CurSize;
        if (glVidRecdCompBufReadPtr >= glVidRecdCompBufSize) {
            glVidRecdCompBufReadPtr -= glVidRecdCompBufSize;
        	glVidRecdCompBufReadWrap++;
        }
    }
}

/**
 @brief Align video ring buffer current write pointer
 @param[in] usAlignment MUST be 2's power
*/
MMP_ERR MMPF_VIDMGR_AlignVideoWritePtr (void)
{
	if (bVidMgrRingBufEn) {
        MMP_ULONG ulAlignOffset = glVidRecdCompBufAddr + glVidRecdCompBufWritePtr;

        ulAlignOffset = (ALIGN32(ulAlignOffset) - ulAlignOffset);
        if (ulAlignOffset) {
            return MMPF_VIDMGR_UpdateVideoWritePtr(ulAlignOffset);
        }
    }

    return MMP_ERR_NONE;
}

/**
 @brief Align video ring buffer current read pointer
 @param[in] usAlignment MUST be 2's power
*/
void MMPF_VIDMGR_AlignVideoReadPtr (void)
{
	if (bVidMgrRingBufEn) {
        MMP_ULONG ulAlignOffset = glVidRecdCompBufAddr + glVidRecdCompBufReadPtr;

        ulAlignOffset = (ALIGN32(ulAlignOffset) - ulAlignOffset);
        if (ulAlignOffset) {
            MMPF_VIDMGR_UpdateVideoReadPtr(ulAlignOffset);
        }
    }
}
#endif

/**
 @brief Transfer from compress to slot buffer
 @param[in] CurSize current read out frame size
*/
void MMPF_VIDMGR_TransferToSlotBuf (MMP_ULONG *ulDst, MMP_ULONG ulDstLow, MMP_ULONG ulDstHigh, MMP_ULONG ulSrc, MMP_ULONG ulLength)
{
    MMP_ULONG trans_size;

    #if (SLOT_RING == 1)
    if (((*ulDst) + ulLength) > ulDstHigh) {
        trans_size = ulDstHigh - (*ulDst);
    }
    else
    #endif
    {
        trans_size = ulLength;
    }
    MEMCPY ((void*)(*ulDst), (void*)ulSrc, trans_size);
    (*ulDst) += trans_size;

    #if (SLOT_RING == 1)
    if (*ulDst == ulDstHigh) {
        *ulDst = ulDstLow;
    }
    if (trans_size < ulLength) {
        MEMCPY ((void*)ulDstLow, (void*)(ulSrc+trans_size), ulLength-trans_size);
        (*ulDst) = ulDstLow + ulLength - trans_size;
    }
    #endif
}

/**
 @brief Get MPEG transport stream mux mode enable or not
 @retval Enable or not
*/
MMP_BOOL MMPF_VIDMGR_GetMtsMuxModeEnable (void)
{
    return bVidMgrMtsMuxEn;
}

void MMPF_VIDMGR_SetMtsMuxModeEnable (MMP_BOOL tmp)
{
    #if (MGR_PROC_EN == 1) && (MTS_MUXER_EN == 1)
    bVidMgrMtsMuxEn = tmp;
    #endif
}

/**
 @brief enable ring buf or not
 @param[in] bEnable enable or not ring buffer
 @retval MMP_ERR_NONE Success.
*/
MMP_BOOL MMPF_VIDMGR_GetRingBufEnable(void)
{
    return bVidMgrRingBufEn;
}

/**
 @brief enable ring buf or not
 @param[in] bEnable enable or not ring buffer
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPF_VIDMGR_SetRingBufEnable(MMP_BOOL bEnable)
{
    #if (MGR_PROC_EN == 1)
    bVidMgrRingBufEn = bEnable;
    #endif

    return MMP_ERR_NONE;
}

MMP_UBYTE MMPF_VIDMGR_SetOutputPipe (MMP_UBYTE ubEid, MMP_UBYTE ubPipe)
{
    if (ubEid >= MAX_NUM_ENC_SET) {
        return 0;
    }

    bVidMgrDstPipeId[ubEid] = ubPipe;

    return MMP_ERR_NONE;
}

MMP_UBYTE MMPF_VIDMGR_GetOutputPipe (MMP_UBYTE ubEid)
{
    if (ubEid >= MAX_NUM_ENC_SET) {
        return 0;
    }

    return bVidMgrDstPipeId[ubEid];
}

MMP_ERR MMPF_VIDMGR_StartMts (MMP_UBYTE ubEncId)
{
    #if (MTS_MUXER_EN == 1)
    MMP_ULONG   total_size = 0, header_size, free_size;
    MMP_ULONG   dst_addr, dst_low, dst_high, pkt_tail_size;
    MMPF_MTS_TS ts_config;
    MMP_UBYTE   TsHeader[8];
    MMPF_H264ENC_ENC_INFO   *pEnc = MMPF_H264ENC_GetHandle(ubEncId);

    if (ubEncId >= MAX_NUM_ENC_SET) {
        return MMP_3GPMGR_ERR_PARAMETER;
    }

    MMPF_VIDMGR_GetTxBufInfo(ubEncId, &dst_addr, &dst_low, &dst_high, &free_size);

    ts_config.usPID             = MTS_PID_PAT;
    ts_config.bPayloadUnitStart = 1;
    ts_config.ubContinuity      = 0;
    ts_config.bAdaptationCtl    = MMP_FALSE;
    ts_config.bHeaderSection    = MMP_TRUE;
    ts_config.bPcrFlag          = MMP_FALSE;
    header_size = MMPF_MTS_GenerateTsHeader(TsHeader, &ts_config);
    MMPF_VIDMGR_TransferToSlotBuf (&dst_addr, dst_low, dst_high, (MMP_ULONG)TsHeader, header_size);
    MMPF_VIDMGR_TransferToSlotBuf (&dst_addr, dst_low, dst_high, (MMP_ULONG)MtsPatPayload, glMtsPatPayloadLen);
    if ((header_size + glMtsPatPayloadLen) > MTS_TS_PACKET_SIZE) {
        while (1) {
            RTNA_DBG_Str(0, "Error: not support too large table 1\r\n");
        }
    }
    else {
        pkt_tail_size = MTS_TS_PACKET_SIZE - header_size - glMtsPatPayloadLen;
        if (pkt_tail_size) {
            MEMSET((void*)dst_addr, 0xFF, pkt_tail_size);
        }
        dst_addr += pkt_tail_size;
    }
    total_size += MTS_TS_PACKET_SIZE;

    ts_config.usPID             = MTS_PID_PMT;
    header_size = MMPF_MTS_GenerateTsHeader(TsHeader, &ts_config);
    MMPF_VIDMGR_TransferToSlotBuf (&dst_addr, dst_low, dst_high, (MMP_ULONG)TsHeader, header_size);
    MMPF_VIDMGR_TransferToSlotBuf (&dst_addr, dst_low, dst_high, (MMP_ULONG)MtsPmtPayload, glMtsPmtPayloadLen);
    if ((header_size + glMtsPmtPayloadLen) > MTS_TS_PACKET_SIZE) {
        while (1) {
            RTNA_DBG_Str(0, "Error: not support too large table 1\r\n");
        }
    }
    else {
        pkt_tail_size = MTS_TS_PACKET_SIZE - header_size - glMtsPmtPayloadLen;
        if (pkt_tail_size) {
            MEMSET((void*)dst_addr, 0xFF, pkt_tail_size);
        }
        dst_addr += pkt_tail_size;
    }
    total_size += MTS_TS_PACKET_SIZE;

    ts_config.usPID             = MTS_PID_NIT;
    ts_config.ubContinuity      = 1;
    ts_config.bHeaderSection    = MMP_FALSE;
    header_size = MMPF_MTS_GenerateTsHeader(TsHeader, &ts_config);
    MMPF_VIDMGR_TransferToSlotBuf (&dst_addr, dst_low, dst_high, (MMP_ULONG)TsHeader, header_size);
    MMPF_VIDMGR_TransferToSlotBuf (&dst_addr, dst_low, dst_high, (MMP_ULONG)MtsSitPayload, glMtsSitPayloadLen);
    if ((header_size + glMtsSitPayloadLen) > MTS_TS_PACKET_SIZE) {
        while (1) {
            RTNA_DBG_Str(0, "Error: not support too large table 1\r\n");
        }
    }
    else {
        pkt_tail_size = MTS_TS_PACKET_SIZE - header_size - glMtsSitPayloadLen;
        if (pkt_tail_size) {
            MEMSET((void*)dst_addr, 0xFF, pkt_tail_size);
        }
        dst_addr += pkt_tail_size;
    }
    total_size += MTS_TS_PACKET_SIZE;

    MMPF_VIDMGR_PrepareUvcHeader(total_size, PTS, 0, MMP_FALSE, MMP_TRUE, pEnc);

        MMPF_Video_UpdateWrPtr(bVidMgrDstPipeId[ubEncId]);

    #if MTS_INSERT_PCR_EN
    ts_config.usPID             = MTS_PID_PCR_0;
    ts_config.bPayloadUnitStart = 0;
    ts_config.ubContinuity      = 0;
    ts_config.bAdaptationCtl    = MMP_TRUE;
    ts_config.bHeaderSection    = MMP_FALSE;
    ts_config.bPcrFlag          = MMP_TRUE;
    MMPF_MTS_GenerateTsHeader(MtsPayload[ubEncId].PcrPacket, &ts_config);
    #endif

    ts_config.usPID             = MTS_PID_ES_VIDEO_0;
    ts_config.bPayloadUnitStart = 1;
    ts_config.ubContinuity      = 1;
    ts_config.bAdaptationCtl    = MMP_FALSE;
    ts_config.bHeaderSection    = MMP_FALSE;
    ts_config.bPcrFlag          = MMP_FALSE;
    MtsPayload[ubEncId].usTsHeaderLen = 
        MMPF_MTS_GenerateTsHeader(MtsPayload[ubEncId].TsPacket, &ts_config);

    MtsPayload[ubEncId].usPesHeaderLen = 
        MMPF_MTS_GeneratePesVidHeader(MtsPayload[ubEncId].PesHeader);

    #if 1 //FPS = 25.000
        #if MTS_INSERT_PCR_EN
        MtsPayload[ubEncId].PcrInfo.ulTimeDelta = 3600;//9360;
        MtsPayload[ubEncId].PcrInfo.ulTimeDecimalDelta = 0;
        #endif
    MtsPayload[ubEncId].TimeInfo.ulTimeDelta = 3600;
    MtsPayload[ubEncId].TimeInfo.ulTimeDecimalDelta = 0;
    #elif 0 //FPS = 30.000
    MtsPayload[ubEncId].TimeInfo.ulTimeDelta = 3000;
    MtsPayload[ubEncId].TimeInfo.ulTimeDecimalDelta = 0;
    #elif 0 //FPS = 14.985
    MtsPayload[ubEncId].TimeInfo.ulTimeDelta = 6060;
    MtsPayload[ubEncId].TimeInfo.ulTimeDecimalDelta = 60606;
    #elif 0 //FPS = 23.970
    MtsPayload[ubEncId].TimeInfo.ulTimeDelta = 3754;
    MtsPayload[ubEncId].TimeInfo.ulTimeDecimalDelta = 69337;
    #endif

    MtsPayload[ubEncId].TimeInfo.ubTimeMsb32 = 0;
    MtsPayload[ubEncId].TimeInfo.ulTime = 54000000;
    MtsPayload[ubEncId].TimeInfo.ulTimeDecimal = 0;

    MtsPayload[ubEncId].ulPts = MtsPayload[ubEncId].TimeInfo.ulTime;
    MtsPayload[ubEncId].ulDts = MtsPayload[ubEncId].ulPts
                                - MtsPayload[ubEncId].TimeInfo.ulTimeDelta;
    MtsPayload[ubEncId].ubPtsMsb32 = 0;
    MtsPayload[ubEncId].ubDtsMsb32 = 0;

    #if MTS_INSERT_PCR_EN
    MtsPayload[ubEncId].PcrInfo.ubTimeMsb32 = 0;
    MtsPayload[ubEncId].PcrInfo.ulTime      = MtsPayload[ubEncId].TimeInfo.ulTime //53955000;
                                            - MtsPayload[ubEncId].TimeInfo.ulTimeDelta*15;//15 frames
    MtsPayload[ubEncId].PcrInfo.ulTimeDecimal= 0;
    #endif

    MtsPayload[ubEncId].ubContinuityCount = ts_config.ubContinuity;
    MtsPayload[ubEncId].usTsPktPos = 0;

    #endif //#if (MGR_PROC_EN == 1)

    return MMP_ERR_NONE;
}

#if (MGR_PROC_EN == 1)
static MMP_ERR MMPF_VIDMGR_MoveData(MMP_ULONG ulSrcAddr, MMP_ULONG ulDstAddr,
                                    MMP_ULONG ulCount, MMPF_DMA_M_LOFFS_DATA* ptrLineOffset)
{
    if (MMPF_OS_AcquireSem(gCatDMASemID, 0) != OS_NO_ERR) {
        RTNA_DBG_Str(0, "#Error : gCatDMASemID OSSemPend fail\r\n");
    }

    m_bMgrDmaMoveWorkingFlag = MMP_TRUE;

    if (MMPF_DMA_MoveData0(ulSrcAddr, ulDstAddr, ulCount, ptrLineOffset,
                            (DmaCallBackFunc*)MMPF_VIDMGR_CatDMADone)) {
        RTNA_DBG_Str(0, "#Error : Mgr DMA move fail\r\n");
    }

    return MMP_ERR_NONE;
}
#endif

MMP_ERR MMPF_VIDMGR_PrepareUvcHeader (MMP_ULONG ulPayloadSize, MMP_ULONG ulTimestamp,
                                      MMP_USHORT usLayerId, MMP_BOOL bIdrFlag, MMP_BOOL bEOF, MMP_BOOL bSOF,
                                      void *EncHandle,MMP_ULONG ulPadBytes)
{
    MMP_ULONG  fps, uvc_flag = 0;
    MMPF_H264ENC_ENC_INFO *pEnc = (MMPF_H264ENC_ENC_INFO *)EncHandle;

    #if (FLAG_EOF_IN_LAST_SLICE == 1)
    if (bIdrFlag) {
        uvc_flag |= H264_UVC_PH_FLAG_IDR;
    }
    if (bEOF) {
        uvc_flag |= H264_UVC_PH_FLAG_EOF;
    }
    if( bSOF ) {
        uvc_flag |= H264_UVC_PH_FLAG_SOF ;
    }
    #endif
    uvc_flag = (uvc_flag & ~(H264_UVC_PH_LAYER_ID_MASK)) | H264_UVC_PH_LAYER_ID(usLayerId);
    
#if H264_SIMULCAST_EN
    uvc_flag |= ( pEnc->eid << 8);
#endif    
   //dbg_printf(3,"uvcflag : %x\r\n",uvc_flag);
    
    fps = (pEnc->ulMaxFpsInc)? pEnc->ulMaxFpsRes/pEnc->ulMaxFpsInc
            : (MMP_USHORT)ISP_IF_AE_GetRealFPS();

    usb_fill_payload_header (pEnc->eid,(MMP_UBYTE *)MMPF_Video_CurWrPtr(bVidMgrDstPipeId[pEnc->eid]),
        ulPayloadSize, pEnc->MgrHandle->ulSequenceCnt, uvc_flag,
        ulTimestamp,
        (pEnc->mb_w << 4) - pEnc->crop.usLeft - pEnc->crop.usRight,
        (pEnc->mb_h << 4) - pEnc->crop.usBottom - pEnc->crop.usTop,
        fps, bVidMgrDstPipeId[pEnc->eid]);
    #if H264_CBR_PADDING_EN==1 // CBR bitstuffing   
    usb_uvc_fill_cbr_padding_bytes((MMP_UBYTE *)MMPF_Video_CurWrPtr(bVidMgrDstPipeId[pEnc->eid]),ulPadBytes);
    #endif
    return MMP_ERR_NONE;
}

MMP_UBYTE gbDropStreamId = SIMCAST_DROP_ID ;

MMP_ERR MMPF_VIDMGR_SetDropStreamId(MMP_UBYTE sid)
{
    gbDropStreamId = sid ;    
}


MMP_ERR MMPF_VIDMGR_SendCodedData(MMPF_VIDMGR_CODED_DESC *pDesc)
{
    #if SIMCAST_DROP == 1
    if (pDesc->ubEncId == gbDropStreamId) { // abort one of simcast streams
    }
    else
    #endif
    {
        MMPF_H264ENC_ENC_INFO   *pEnc = MMPF_H264ENC_GetHandle(pDesc->ubEncId);
        #if H264_CBR_PADDING_EN
        if(!pEnc->cbr_padding_en) {
            pDesc->ulCbrPaddingByte = 0 ;
        }
        #else
        pDesc->ulCbrPaddingByte = 0 ;
        #endif
        MMPF_VIDMGR_PrepareUvcHeader(pDesc->ulOutputSize, pDesc->ulTimestamp, pDesc->usLayerId,
                                    (pDesc->Flags&MMPF_VIDMGR_DESC_FLAG_IDR)? MMP_TRUE: MMP_FALSE,
                                    (pDesc->Flags&MMPF_VIDMGR_DESC_FLAG_EOF)? MMP_TRUE: MMP_FALSE, 
                                    (pDesc->Flags&MMPF_VIDMGR_DESC_FLAG_SOF)? MMP_TRUE: MMP_FALSE,
                                    pEnc,pDesc->ulCbrPaddingByte);
        MMPF_Video_UpdateWrPtr((PIPE_PATH)bVidMgrDstPipeId[pDesc->ubEncId]);
        MMPF_Video_SignalFrameDone(pDesc->ubEncId,(PIPE_PATH)bVidMgrDstPipeId[pDesc->ubEncId]);
        pEnc->MgrHandle->ulSequenceCnt++;
    }

    return MMP_ERR_NONE;
}

#if (MGR_PROC_EN == 1)
MMP_ERR MMPF_VIDMGR_MgrCodedData (MMP_ULONG ulDestAddr, MMP_ULONG ulDestLow, MMP_ULONG ulDestHigh,
                                      MMP_ULONG ulMaxSize, MMP_ULONG ulSliceIdx, MMPF_VIDMGR_CODED_DESC *pCurDesc,
                                      MMP_ULONG *ulOutputSize)
{
    MMP_ULONG   ulTotalSize, ulSizeSum = 0;
    MMP_ULONG   ulSrcAddr, ulSrcDataLowBd, ulSrcDataHighBd;
    #if (MTS_MUXER_EN == 1)
    MMP_USHORT  usMtsPktCnt = 0;
    MMPF_VIDMGR_MTS_PAYLAOD *pMtsPayload = &(MtsPayload[pCurDesc->ubEncId]);
    #endif
    MMPF_H264ENC_ENC_INFO   *pEnc = MMPF_H264ENC_GetHandle(pCurDesc->ubEncId);

    #if (MTS_MUXER_EN == 1)
    if (bVidMgrMtsMuxEn) {
        MMP_ULONG packet_data_size = MTS_TS_PACKET_SIZE - pMtsPayload->usPesHeaderLen;

        if (packet_data_size == 0) {
            RTNA_DBG_Str(0, "Error TS header len\r\n");
        }
        ulTotalSize = pCurDesc->XhdrSize[ulSliceIdx] + pCurDesc->DataSize[ulSliceIdx];
        if (pCurDesc->Flags & MMPF_VIDMGR_DESC_FLAG_SOF) { //only 1st slice carries PES header
            ulTotalSize += (pMtsPayload->usPesHeaderLen + MMPF_H264ENC_GetAudNaluLen());
        }
        ulTotalSize = ((ulTotalSize+packet_data_size-1)/packet_data_size); //packet count
        ulTotalSize *= MTS_TS_PACKET_SIZE; //total size
    }
    else
    #endif
    {
        #ifdef MAX_SLICE_HDR_NUM
        ulTotalSize =  pCurDesc->DataSize[ulSliceIdx];
        if(ulSliceIdx < MAX_SLICE_HDR_NUM) {
            ulTotalSize += pCurDesc->XhdrSize[ulSliceIdx] ;
        }
        #else
        ulTotalSize = pCurDesc->XhdrSize[ulSliceIdx] + pCurDesc->DataSize[ulSliceIdx];
        #endif
    }
    *ulOutputSize   = 0;

    ulSrcAddr       = pCurDesc->DataBufAddr[ulSliceIdx];
    ulSrcDataLowBd  = pEnc->cur_frm_bs_low_bd;
    ulSrcDataHighBd = pEnc->cur_frm_bs_high_bd;

    if (ulDestAddr >= ulDestHigh) {
        ulDestAddr -= (ulDestHigh - ulDestLow);
    }

    // repack extention header and rest of slice data
    if (ulTotalSize > ulMaxSize) {
    	RTNA_DBG_Long(0, ulMaxSize);
    	RTNA_DBG_Long(0, ulTotalSize);
        RTNA_DBG_Str(0, "slot overflow\r\n");
        return MMP_3GPMGR_ERR_AVBUF_FULL;
    }

    #if (MTS_MUXER_EN == 1)
    if (bVidMgrMtsMuxEn) {
        #if MTS_INSERT_PCR_EN
        if (ulSliceIdx == 0) { // only 1st slice contains PES header
            MMPF_MTS_UpdateTsPcr(&(pMtsPayload->PcrPacket[TS_PCR_PAYLOAD_OFFSET]),
                                    &(pMtsPayload->PcrInfo));
            MMPF_VIDMGR_TransferToSlotBuf(&ulDestAddr, ulDestLow,
                                            ulDestHigh, (MMP_ULONG)pMtsPayload->PcrPacket,
                                            MTS_TS_PACKET_SIZE);
            usMtsPktCnt++;
            ulSizeSum += MTS_TS_PACKET_SIZE;
        }
        #endif

        //TS header
        MMPF_MTS_UpdateTsHeader (pMtsPayload->TsPacket, MMP_TRUE, pMtsPayload->ubContinuityCount++, 0);
        pMtsPayload->usTsPktPos = pMtsPayload->usTsHeaderLen;

        if (ulSliceIdx == 0) { // only 1st slice contains PES header
            //PES header
            MMPF_MTS_UpdatePesTimestamp (
                &(pMtsPayload->PesHeader[PES_PTS_PAYLOAD_OFFSET]),
                pMtsPayload->ubPtsMsb32,
                pMtsPayload->ulPts);
            MMPF_MTS_UpdatePesTimestamp (
                &(pMtsPayload->PesHeader[PES_DTS_PAYLOAD_OFFSET]),
                pMtsPayload->ubDtsMsb32,
                pMtsPayload->ulDts);
            MEMCPY (
                pMtsPayload->TsPacket + pMtsPayload->usTsPktPos,
                pMtsPayload->PesHeader,
                pMtsPayload->usPesHeaderLen);
            pMtsPayload->usTsPktPos += pMtsPayload->usPesHeaderLen;

            //Aud header
            MMPF_H264ENC_GenerateAudHeader (pMtsPayload->TsPacket + pMtsPayload->usTsPktPos, pCurDesc->FrameType);
            pMtsPayload->usTsPktPos += MMPF_H264ENC_GetAudNaluLen();
        }
    }
    #endif //#if (MTS_MUXER_EN == 1)

    #if (MTS_MUXER_EN == 1)
    if (bVidMgrMtsMuxEn) {
        MMP_ULONG trans_size, rest_size, slice_data_offset = 0;

        if (pMtsPayload->usTsPktPos >= MTS_TS_PACKET_SIZE) {
            RTNA_DBG_Str(0, "Error, header>=188, not support\r\n");
        }

        for (rest_size = pCurDesc->XhdrSize[ulSliceIdx]; rest_size > 0; rest_size -= trans_size) {
            if ((pMtsPayload->usTsPktPos + rest_size) > MTS_TS_PACKET_SIZE) {
                trans_size = MTS_TS_PACKET_SIZE - pMtsPayload->usTsPktPos;
            }
            else {
                trans_size = rest_size;
            }

            MEMCPY ( pMtsPayload->TsPacket + pMtsPayload->usTsPktPos, 
                    (void*)(pCurDesc->XhdrBuf[ulSliceIdx] + pCurDesc->XhdrSize[ulSliceIdx] - rest_size),
                    trans_size);
            pMtsPayload->usTsPktPos += trans_size;

            if (pMtsPayload->usTsPktPos == MTS_TS_PACKET_SIZE) { //new packet
                MMPF_VIDMGR_TransferToSlotBuf (&ulDestAddr, ulDestLow, ulDestHigh,
                    (MMP_ULONG)pMtsPayload->TsPacket, pMtsPayload->usTsPktPos);
                usMtsPktCnt++;
                ulSizeSum += MTS_TS_PACKET_SIZE;

                MMPF_MTS_UpdateTsHeader (pMtsPayload->TsPacket, MMP_FALSE, pMtsPayload->ubContinuityCount++, 0);
                pMtsPayload->usTsPktPos = pMtsPayload->usTsHeaderLen;
            }
        }

        if (pCurDesc->DataSize[ulSliceIdx]) {
            if (pMtsPayload->usTsPktPos > pMtsPayload->usTsHeaderLen) { // fill last packet with comp data
                MMP_ULONG pkt_rest_size = MTS_TS_PACKET_SIZE - pMtsPayload->usTsPktPos;

                if (pkt_rest_size > pCurDesc->DataSize[ulSliceIdx]) {
                    slice_data_offset = pCurDesc->DataSize[ulSliceIdx];
                }
                else {
                    slice_data_offset = pkt_rest_size;
                }

                MMPF_VIDMGR_TransferToSlotBuf (&ulDestAddr, ulDestLow, ulDestHigh,
                                                (MMP_ULONG)pMtsPayload->TsPacket,
                                                pMtsPayload->usTsPktPos);

                if ((ulSrcAddr + slice_data_offset) > ulSrcDataHighBd) {
                    trans_size = ulSrcDataHighBd - ulSrcAddr;
                }
                else {
                    trans_size = slice_data_offset;
                }

                MMPF_VIDMGR_TransferToSlotBuf (&ulDestAddr, ulDestLow, ulDestHigh,
                                                ulSrcAddr, trans_size);
                ulSrcAddr += trans_size;
                if (ulSrcAddr == ulSrcDataHighBd) {
                    ulSrcAddr = ulSrcDataLowBd;
                }

                if (trans_size < slice_data_offset) {
                    MMPF_VIDMGR_TransferToSlotBuf (&ulDestAddr, ulDestLow, ulDestHigh,
                                                    ulSrcAddr,
                                                    slice_data_offset - trans_size);
                    ulSrcAddr = ulSrcDataLowBd + slice_data_offset - trans_size;
                }
                if (slice_data_offset < pkt_rest_size) { //fill stuffing bytes
                    MEMSET((void*)ulDestAddr, 0, pkt_rest_size - slice_data_offset);
                    ulDestAddr += (pkt_rest_size - slice_data_offset);
                    //TODO : add SLOT_RING support
                }
                usMtsPktCnt++;
                ulSizeSum += MTS_TS_PACKET_SIZE;
            } //ulSrcAddr is increased
            //DMA move multiple segments
            if (slice_data_offset < pCurDesc->DataSize[ulSliceIdx]) {
                //ulSrcAddr         : current src addr
                //slice_data_offset : slice data moved size
                //pEnc->cur_slice_data_addr[*ulSliceIdx]    : slice start
                //pEnc->cur_slice_data_len[*ulSliceIdx]     : slice total size
                //ulDestAddr : current dst addr
                MMP_ULONG   trans_pkt_cnt, trans_tail_size;
                MMP_SHORT   ts_payload_size, i;

                ts_payload_size = MTS_TS_PACKET_SIZE - pMtsPayload->usTsHeaderLen;
                if (ts_payload_size == 0) {
                    RTNA_DBG_Str(0, "Error TS payload size\r\n");
                }

                rest_size = pCurDesc->DataSize[ulSliceIdx] - slice_data_offset;
                if ((ulSrcAddr + rest_size) > ulSrcDataHighBd) {
                    trans_size = ulSrcDataHighBd - ulSrcAddr;
                }
                else {
                    trans_size = rest_size;
                }
                trans_pkt_cnt = (trans_size+ts_payload_size-1)/ts_payload_size;
                trans_tail_size = (trans_pkt_cnt*ts_payload_size)-trans_size; //last packet size

                //TODO: add SLOT_RING support
                if (MMPF_VIDMGR_MoveData (ulSrcAddr, ulDestAddr + pMtsPayload->usTsHeaderLen,
                                            trans_size, &MtsPacketSplitOffset)) {
                    RTNA_DBG_Str(0, "TS DMA FAILED\r\n");
                }

                // fill TS packet header
                for (i = 0; i < trans_pkt_cnt; i++) {
                    MMPF_MTS_UpdateTsHeader(pMtsPayload->TsPacket, MMP_FALSE,
                                            pMtsPayload->ubContinuityCount++, 0);
                    //TODO : add SLOT_RING support
                    MMPF_VIDMGR_TransferToSlotBuf(&ulDestAddr, ulDestLow, ulDestHigh,
                                                    (MMP_ULONG)pMtsPayload->TsPacket,
                                                    pMtsPayload->usTsHeaderLen);
                    if (i == (trans_pkt_cnt-1)) {
                        ulDestAddr += (ts_payload_size-trans_tail_size);
                    }
                    else {
                        ulDestAddr += ts_payload_size;
                    }
                }
                ulSrcAddr += trans_size;
                if (ulSrcAddr == ulSrcDataHighBd) {
                    ulSrcAddr = ulSrcDataLowBd;
                }
                rest_size -= trans_size;

                if (trans_size < (pCurDesc->DataSize[ulSliceIdx]-slice_data_offset)) { //ring
                    if (trans_tail_size) {
                        MMP_ULONG   last_coded_size;

                        if (rest_size >= trans_tail_size) {
                            last_coded_size = trans_tail_size;
                        }
                        else {
                            last_coded_size = rest_size;
                        }
                        MMPF_VIDMGR_TransferToSlotBuf (&ulDestAddr, ulDestLow, ulDestHigh,
                                                        ulSrcAddr, last_coded_size);
                        ulSrcAddr += last_coded_size;
                        rest_size -= last_coded_size;
                        if (last_coded_size < trans_tail_size) {
                            MEMSET((void*)ulDestAddr, 0, trans_tail_size-last_coded_size);
                            ulDestAddr += (trans_tail_size-last_coded_size);
                        }
                    }
                    ulSizeSum += (MTS_TS_PACKET_SIZE*trans_pkt_cnt);

                    if (rest_size) {
                        trans_pkt_cnt = (rest_size+ts_payload_size-1)/ts_payload_size;
                        trans_tail_size = (trans_pkt_cnt*ts_payload_size)-rest_size;

                        if (MMPF_VIDMGR_MoveData(ulSrcAddr, ulDestAddr + pMtsPayload->usTsHeaderLen,
                                                    rest_size, &MtsPacketSplitOffset)) {
                            RTNA_DBG_Str(0, "TS DMA FAILED\r\n");
                        }

                        for (i = 0; i < trans_pkt_cnt; i++) {
                            MMPF_MTS_UpdateTsHeader(pMtsPayload->TsPacket, MMP_FALSE,
                                                    pMtsPayload->ubContinuityCount++, 0);
                            //TODO : add SLOT_RING support
                            MMPF_VIDMGR_TransferToSlotBuf (&ulDestAddr, ulDestLow, ulDestHigh,
                                                            (MMP_ULONG)pMtsPayload->TsPacket,
                                                            pMtsPayload->usTsHeaderLen);
                            if (i == (trans_pkt_cnt-1)) {
                                ulDestAddr += (ts_payload_size-trans_tail_size);
                            }
                            else {
                                ulDestAddr += ts_payload_size;
                            }
                        }
                        ulSrcAddr += rest_size; //should not ring
                        rest_size -= rest_size;
                        //ulDestAddr -= trans_tail_size; //not yet moved

                        if (trans_tail_size) {
                            MEMSET ((void*)ulDestAddr, 0, trans_tail_size);
                            ulDestAddr += trans_tail_size;
                        }

                        ulSizeSum += (MTS_TS_PACKET_SIZE*trans_pkt_cnt);
                    }
                }
                else { //only 1 segment data
                    if (trans_tail_size) {
                        MEMSET((void*)ulDestAddr, 0, trans_tail_size);//stuffing byte
                        ulDestAddr += trans_tail_size;
                    }
                    ulSizeSum += (MTS_TS_PACKET_SIZE*trans_pkt_cnt);
                }
            }
        }
    }
    else
    #endif //#if (MTS_MUXER_EN == 1)
    {
    
        #ifdef MAX_SLICE_HDR_NUM
        if ( (ulSliceIdx < MAX_SLICE_HDR_NUM) && (pCurDesc->XhdrSize[ulSliceIdx])  )
        #else
        if (pCurDesc->XhdrSize[ulSliceIdx])
        #endif
        {
            MMP_ULONG trans_size;
            #if (SLOT_RING == 1)
            if ((ulDestAddr+pCurDesc->XhdrSize[ulSliceIdx]) > ulDestHigh) {
                trans_size = ulDestHigh - ulDestAddr;
            }
            else
            #endif
            {
                trans_size = pCurDesc->XhdrSize[ulSliceIdx];
            }

            MEMCPY ((void*)ulDestAddr,
                    (void*)(pCurDesc->XhdrBuf[ulSliceIdx]),
                    trans_size);
            ulDestAddr += trans_size;

            #if (SLOT_RING == 1)
            if (ulDestAddr == ulDestHigh) {
                ulDestAddr = ulDestLow;
            }
            if (trans_size < pCurDesc->XhdrSize[ulSliceIdx]) {
                MEMCPY ((void*)ulDestLow,
                        (void*)(pCurDesc->XhdrBuf[ulSliceIdx]+trans_size),
                        pCurDesc->XhdrSize[ulSliceIdx]-trans_size);
                ulDestAddr = ulDestLow + pCurDesc->XhdrSize[ulSliceIdx] - trans_size;
            }
            #endif
            ulSizeSum += pCurDesc->XhdrSize[ulSliceIdx];
        }

        if (pCurDesc->DataSize[ulSliceIdx]) {
            MMP_ULONG src_size0, src_size1, dst_trans_size;

            if ((pCurDesc->DataBufAddr[ulSliceIdx]+pCurDesc->DataSize[ulSliceIdx]) > ulSrcDataHighBd) {
                src_size0 = ulSrcDataHighBd - pCurDesc->DataBufAddr[ulSliceIdx];
                src_size1 = pCurDesc->DataSize[ulSliceIdx] - src_size0;
            }
            else {
                src_size0 = pCurDesc->DataSize[ulSliceIdx];
                src_size1 = 0;
            }

            #if (SLOT_RING == 1)
            if ((ulDestAddr + src_size0) > ulDestHigh) {
                dst_trans_size = ulDestHigh - ulDestAddr;
            }
            else
            #endif
            {
                dst_trans_size = src_size0;
            }
            if (MMPF_VIDMGR_MoveData(pCurDesc->DataBufAddr[ulSliceIdx],
                                    ulDestAddr, dst_trans_size, NULL)) {
                RTNA_DBG_Str(0, "MGR CAT DMA FAILED 0a\r\n");
            }
            ulDestAddr += dst_trans_size;

            #if (SLOT_RING == 1)
            if (ulDestAddr == ulDestHigh) {
                ulDestAddr = ulDestLow;
            }
            if (dst_trans_size < src_size0) {
                if (MMPF_VIDMGR_MoveData(pCurDesc->DataBufAddr[ulSliceIdx] + dst_trans_size,
                                        ulDestLow, src_size0 - dst_trans_size, NULL)) {
                    RTNA_DBG_Str(0, "MGR CAT DMA FAILED 0b\r\n");
                }
                ulDestAddr = ulDestLow + src_size0 - dst_trans_size;
            }
            #endif
            if (src_size1) {
                #if (SLOT_RING == 1)
                if ((ulDestAddr + src_size1) > ulDestHigh) {
                    dst_trans_size = ulDestHigh - ulDestAddr;
                }
                else
                #endif
                {
                    dst_trans_size = src_size1;
                }
                if (MMPF_VIDMGR_MoveData(ulSrcDataLowBd, ulDestAddr, dst_trans_size, NULL)) {
                    RTNA_DBG_Str(0, "MGR CAT DMA FAILED 1a\r\n");
                }
                ulDestAddr += dst_trans_size;

                #if (SLOT_RING == 1)
                if (ulDestAddr == ulDestHigh) {
                    ulDestAddr = ulDestLow;
                }
                if (dst_trans_size < src_size1) {
                    if (MMPF_VIDMGR_MoveData(ulSrcDataLowBd + dst_trans_size,
                                            ulDestLow, src_size1 - dst_trans_size, NULL)) {
                        RTNA_DBG_Str(0, "MGR CAT DMA FAILED 1b\r\n");
                    }
                    ulDestAddr = ulDestLow + src_size1 - dst_trans_size;
                }
                #endif
            }
            ulSizeSum += pCurDesc->DataSize[ulSliceIdx];
        }
    }

    *ulOutputSize = ulSizeSum;

    return MMP_ERR_NONE;
}
#endif //#if (MGR_PROC_EN == 1)

#if (MGR_PROC_EN == 1)
MMP_ERR MMPF_VIDMGR_ProcDesc(MMPF_VIDMGR_CODED_DESC *pCurDesc)
{
#define MGR_MOVE_TIMEOUT_MAX_CNT    (10000000)
    MMP_ULONG   i, MgrOutputSize, ulDestAddr, ulDestLow, ulDestHigh, ulAvaFreeSize, ulCurSize;
    MMP_ERR     MgrStat = MMP_ERR_NONE;

    if (MMPF_Video_IsFull(bVidMgrDstPipeId[pCurDesc->ubEncId])) {
        MMPF_VIDENC_SetParameter(pCurDesc->ubEncId, MMPF_VIDENC_ATTRIBUTE_FORCE_I,
                                    (void*)MMPF_VIDENC_PICCTL_IDR_RESYNC);
        RTNA_DBG_Str(0, "#Error : Mgr found no empty slot\r\n");
        return MMP_3GPMGR_ERR_AVBUF_FULL;
    }

    MMPF_VIDMGR_GetTxBufInfo (pCurDesc->ubEncId, &ulDestAddr, &ulDestLow, &ulDestHigh,
                                &ulAvaFreeSize);

    for (i = 0, MgrOutputSize = 0; i < pCurDesc->ulSliceNum; i++) {
         MgrStat = MMPF_VIDMGR_MgrCodedData(ulDestAddr + MgrOutputSize,
                            ulDestLow, ulDestHigh, ulAvaFreeSize - MgrOutputSize,
                            i, pCurDesc, &ulCurSize);
        if (MgrStat == MMP_ERR_NONE) {
            MgrOutputSize += ulCurSize;
        }
        else {
            break;
        }
    }

    if (MgrStat == MMP_ERR_NONE) {
        MMP_ULONG   ulTimeoutCnt;

        pCurDesc->ulOutputSize = MgrOutputSize;

        for (ulTimeoutCnt = 0; (m_bMgrDmaMoveWorkingFlag == MMP_TRUE); ulTimeoutCnt++) {
            if (ulTimeoutCnt >= MGR_MOVE_TIMEOUT_MAX_CNT) {
                RTNA_DBG_Str(0, "#Error : Mgr DMA timeout\r\n");
                return MMP_VIDMGR_ERR_DMA_TIMEOUT;
            }
        }

        MMPF_VIDMGR_SendCodedData(pCurDesc);

        MMPF_VIDMGR_UpdateVideoReadPtr(pCurDesc->ulHwCodedSize);
        if (pCurDesc->Flags & MMPF_VIDMGR_DESC_FLAG_EOF) {
            MMPF_VIDMGR_AlignVideoReadPtr();
        }
    }
    else {
        MMPF_VIDENC_SetParameter(pCurDesc->ubEncId, MMPF_VIDENC_ATTRIBUTE_FORCE_I,
                                    (void*)MMPF_VIDENC_PICCTL_IDR_RESYNC);
        RTNA_DBG_Str(0, "#Error : Mgr no enough free size\r\n");
    }

    return MgrStat;
}
#endif

/**
 @brief Main routine of video merger task.
*/
void VidMergr_Task(void *p_arg)
{
	MMPF_OS_FLAGS   flags;
	MMP_UBYTE       ubMgrId;

    RTNA_DBG_Str(0, "VidMergr_Task()\r\n");

    #if (MGR_PROC_EN == 1)
	gCatDMASemID = MMPF_OS_CreateSem(1);
	#endif

    for (ubMgrId = 0; ubMgrId < MAX_MGR_NUM; ubMgrId++) {
        #if (MGR_PROC_EN == 1)
        m_VidMgrHandle[ubMgrId].DescQueueRdIdx = 0;
        m_VidMgrHandle[ubMgrId].DescQueueRdWrap = 0;
        m_VidMgrHandle[ubMgrId].DescQueueWrIdx = 0;
        m_VidMgrHandle[ubMgrId].DescQueueWrWrap = 0;
        #endif
        m_VidMgrHandle[ubMgrId].ulSequenceCnt = 1;
        m_VidMgrHandle[ubMgrId].ubMgrId = ubMgrId;
    }
    #if (MTS_MUXER_EN == 1)
    {
        MMP_USHORT      PmtPids[1] = {MTS_PID_PMT};
        MMP_UBYTE       *pDesc;
        MMP_ULONG       ulDescLen;

        MMPF_MTS_InitPatConfig(&MtsPatConfig, PmtPids, 1);
        MMPF_MTS_GeneratePsiTable(MtsPatPayload, &glMtsPatPayloadLen, &MtsPatConfig);

        MMPF_MTS_GetPmtProgramDesc(&pDesc, &ulDescLen);
        MMPF_MTS_InitPmtConfig(&MtsPmtConfig, MTS_PID_PCR_0, pDesc, ulDescLen);
        MMPF_MTS_GetPmtVidDesc(&pDesc, &ulDescLen);
        MMPF_MTS_AddEsToPmt(&MtsPmtConfig, MMPF_MTS_STREAM_TYPE_H264, MTS_PID_ES_VIDEO_0, pDesc, ulDescLen);
        MMPF_MTS_GetPmtAudDesc(&pDesc, &ulDescLen, MMPF_MTS_STREAM_TYPE_AAC);
        MMPF_MTS_AddEsToPmt(&MtsPmtConfig, MMPF_MTS_STREAM_TYPE_AAC, MTS_PID_ES_AUDIO_0, NULL, 0);
        MMPF_MTS_GeneratePsiTable(MtsPmtPayload, &glMtsPmtPayloadLen, &MtsPmtConfig);
        MMPF_MTS_GenerateSit(MtsSitPayload, &glMtsSitPayloadLen);
    }
    #endif

	#if MGR_DBG_MSG
	{
	    MMP_ULONG i;
	    RTNA_DBG_Str(0, " Dump MTS info .............\r\n");
	    RTNA_DBG_Str(0, " PAT section .............\r\n");
	    RTNA_DBG_Short(0, glMtsPatPayloadLen);
	    RTNA_DBG_Str(0, " byte\r\n");
	    for (i = 0; i < glMtsPatPayloadLen; i++) {
	        RTNA_DBG_Byte(0, MtsPatPayload[i]);
	        if ((i & 0x0F) == 0x0F) {
	            RTNA_DBG_Str(0, "\r\n");
	        }
	    }
	    RTNA_DBG_Str(0, "\r\n PMT section .............\r\n");
	    RTNA_DBG_Short(0, glMtsPmtPayloadLen);
	    RTNA_DBG_Str(0, " byte\r\n");
	    for (i = 0; i < glMtsPmtPayloadLen; i++) {
	        RTNA_DBG_Byte(0, MtsPmtPayload[i]);
	        if ((i & 0x0F) == 0x0F) {
	            RTNA_DBG_Str(0, "\r\n");
	        }
	    }
	    RTNA_DBG_Str(0, "\r\n TS section .............\r\n");
	    RTNA_DBG_Short(0, MtsPayload[0].usTsHeaderLen);
	    RTNA_DBG_Str(0, " byte\r\n");
	    for (i = 0; i < MtsPayload[0].usTsHeaderLen; i++) {
	        RTNA_DBG_Byte(0, MtsPayload[0].TsPacket[i]);
	        if ((i & 0x0F) == 0x0F) {
	            RTNA_DBG_Str(0, "\r\n");
	        }
	    }
	    RTNA_DBG_Str(0, "\r\n PES section .............\r\n");
	    RTNA_DBG_Short(0, MtsPayload[0].usPesHeaderLen);
	    RTNA_DBG_Str(0, " byte\r\n");
	    for (i = 0; i < MtsPayload[0].usPesHeaderLen; i++) {
	        RTNA_DBG_Byte(0, MtsPayload[0].PesHeader[i]);
	        if ((i & 0x0F) == 0x0F) {
	            RTNA_DBG_Str(0, "\r\n");
	        }
	    }
	}
	#endif

    while (TRUE) {
        MMPF_OS_WaitFlags ( SYS_Flag_Hif, SYS_FLAG_MGR_0,
                            (MMPF_OS_FLAG_WAIT_SET_ANY|MMPF_OS_FLAG_CONSUME),
                            0, &flags);

        #if (MGR_PROC_EN == 1)
        if (flags & SYS_FLAG_MGR_0) {
            MMPF_VIDMGR_CODED_DESC *CurDesc;
            MMP_ULONG   ulAvaliableDesc = 0;
            MMP_ERR     queue_stat, move_stat;

            queue_stat = MMPF_VIDMGR_GetDescQueueDepth(&ulAvaliableDesc, &(m_VidMgrHandle[0]));
            if (queue_stat != MMP_ERR_NONE) {
                RTNA_DBG_Long(0, queue_stat);
                RTNA_DBG_Str(0, " #Error : VIDMGR queue status\r\n");
            }

            if ((queue_stat == MMP_ERR_NONE) && (ulAvaliableDesc > 0)) {
                CurDesc = MMPF_VIDMGR_GetDescQueueRdPtr(&(m_VidMgrHandle[0]));
                move_stat = MMPF_VIDMGR_ProcDesc(CurDesc);
                if (move_stat == MMP_ERR_NONE) {
                    MMPF_VIDMGR_UpdateDescQueueRdIdx(&(m_VidMgrHandle[0]));
                    if (CurDesc->Flags & MMPF_VIDMGR_DESC_FLAG_EOF) {
                        MMPF_VIDENC_UpdateWorkingBufRdIdx(MMPF_H264ENC_GetHandle(CurDesc->ubEncId));
                        #if (MTS_MUXER_EN == 1)
                        if (bVidMgrMtsMuxEn) {
                            MtsPayload[CurDesc->ubEncId].ulDts      = MtsPayload[CurDesc->ubEncId].ulPts;
                            MtsPayload[CurDesc->ubEncId].ubDtsMsb32 = MtsPayload[CurDesc->ubEncId].ubPtsMsb32;
                            MMPF_MTS_IncreamentPesTimestamp (&(MtsPayload[CurDesc->ubEncId].TimeInfo));
                            MtsPayload[CurDesc->ubEncId].ulPts      = MtsPayload[CurDesc->ubEncId].TimeInfo.ulTime;
                            MtsPayload[CurDesc->ubEncId].ubPtsMsb32 = MtsPayload[CurDesc->ubEncId].TimeInfo.ubTimeMsb32;
                            #if MTS_INSERT_PCR_EN
                            MMPF_MTS_IncreamentTsPcr (&(MtsPayload[CurDesc->ubEncId].PcrInfo));
                            #endif
                        }
                        #endif
                    }
                    ulAvaliableDesc--;
                    if (ulAvaliableDesc) {
                        MMPF_VIDMGR_CheckDataReady(&(m_VidMgrHandle[0]));
                    }
                }
                else if (move_stat == MMP_3GPMGR_ERR_AVBUF_FULL) {
                    if (MMPF_VIDENC_GetStatus() == MMPF_MP4VENC_FW_STATUS_STOP) {
                        MMPF_VIDMGR_Initialization(0);
                    }
                    if (ulAvaliableDesc) {
                        MMPF_OS_Sleep_MS(8);
                        MMPF_VIDMGR_CheckDataReady(&(m_VidMgrHandle[0]));
                    }
                }
            }
        }
        #endif

    }
}
/// @}
