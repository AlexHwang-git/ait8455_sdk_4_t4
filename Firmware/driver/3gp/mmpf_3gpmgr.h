/**
 @file mmpf_3gpmgr.h
 @brief Header function of 3gp processor
 @author Will Tseng, Truman Yang
 @version 1.1 Add constant and struction section to sync with the host
 @version 1.0 Original Version
*/


#ifndef _MMPF_3GPMGR_H_
#define _MMPF_3GPMGR_H_

#include "includes_fw.h"
#include "mmpf_mts.h"
//==============================================================================
//                                IPC SECTION
//==============================================================================
/** @addtogroup MMPF_VIDMGR
@{
*/

/** @name IPC
Inter Process Communication section. 
All definition and declaration here are used for host MMP inter process communication.
This section should be sync with the host.
@{
*/
    /// @}
/** @} */ // end of 3GPP module
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

#define NO_GOP_FLAG                                 0

#define MMPF_3GPMGR_SAVE_AV_PART                	(~(MMP_ULONG)0xfff)   		///< mask of moving 2K bytes AV data
#define MMPF_3GPMGR_SAVE_AV_FULL                	((MMP_ULONG)0xffffffff)     ///< mask of moving all AV data
#define MMPF_3GPMGR_SAVE_FT_PART                	0   ///< flag of moving 512 bytes frame table
#define MMPF_3GPMGR_SAVE_FT_FULL                	1   ///< flag of moving all frame table

#define MMPF_VIDMGR_FRAME_QUEUE_SIZE                (96)  ///< queue of frame size for data transfer

#define MAX_PARSET_BUF_SIZE     (256)
#define MAX_SLICE_HDR_NUM       (2)
#define MAX_SLICE_NUM           (500)//(72*3)
#define MAX_SLICE_HDR_SIZE      (32)
#define MAX_XHDR_SIZE           (MAX_SLICE_HDR_SIZE*3)

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

/// Video frame type
typedef enum _MMPF_3GPMGR_FRAME_TYPE {
    MMPF_3GPMGR_FRAME_TYPE_I = 0,
    MMPF_3GPMGR_FRAME_TYPE_P,
    MMPF_3GPMGR_FRAME_TYPE_B,
    MMPF_3GPMGR_FRAME_TYPE_MAX
} MMPF_3GPMGR_FRAME_TYPE;

typedef enum _MMPF_VIDMGR_DESC_FLAG {
    MMPF_VIDMGR_DESC_FLAG_NONE = 0x00000000,
    MMPF_VIDMGR_DESC_FLAG_SOF = 0x00000001,
    MMPF_VIDMGR_DESC_FLAG_EOF = 0x00000002,
    MMPF_VIDMGR_DESC_FLAG_IDR = 0x00000004
} MMPF_VIDMGR_DESC_FLAG;

typedef struct _MMPF_VIDMGR_MTS_PAYLOAD {
    MMP_USHORT  usPesHeaderLen;
    MMP_USHORT  usTsHeaderLen;
    MMP_UBYTE   PesHeader[MMPF_MTS_MAX_PAYLOAD_SIZE];
    MMP_UBYTE   TsPacket[MTS_TS_PACKET_SIZE];
    MMP_UBYTE   PcrPacket[MTS_TS_PACKET_SIZE];
    MMP_ULONG   ulPts;
    MMP_ULONG   ulDts;
    MMP_UBYTE   ubPtsMsb32;
    MMP_UBYTE   ubDtsMsb32;
    MMP_UBYTE   ubContinuityCount;
    MMP_USHORT  usTsPktPos;
    MMPF_MTS_TIME_INFO TimeInfo;
    MMPF_MTS_TIME_INFO PcrInfo;
} MMPF_VIDMGR_MTS_PAYLAOD;

typedef struct _MMPF_VIDMGR_CODED_DESC {
    MMP_ULONG   ulSliceNum;
    MMP_ULONG   ulOutputSize;
    MMP_ULONG   ulHwCodedSize;
    MMP_UBYTE   (*XhdrBuf)[MAX_XHDR_SIZE];
    MMP_ULONG   *XhdrSize;
    MMP_ULONG   *DataBufAddr;
    MMP_ULONG   *DataSize;
    MMP_ULONG   ulTimestamp;
    MMP_ULONG   ulCbrPaddingByte ;
    MMPF_3GPMGR_FRAME_TYPE  FrameType;
    MMPF_VIDMGR_DESC_FLAG   Flags;
    MMP_USHORT  usLayerId;
    MMP_UBYTE   ubEncId;
} MMPF_VIDMGR_CODED_DESC;

typedef struct _MMPF_VIDMGR_HANDLE {
    #if (MGR_PROC_EN == 1)
    MMPF_VIDMGR_CODED_DESC  DescQueue[MMPF_VIDMGR_FRAME_QUEUE_SIZE];
    MMP_ULONG   DescQueueWrIdx;
    MMP_ULONG   DescQueueWrWrap;
    MMP_ULONG   DescQueueRdIdx;
    MMP_ULONG   DescQueueRdWrap;
    #endif
    MMP_ULONG   ulSequenceCnt;
    MMP_UBYTE   ubMgrId;
} MMPF_VIDMGR_HANDLE;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void        MMPF_VIDMGR_Initialization(MMP_UBYTE ubMgrId);
MMP_ERR     MMPF_VIDMGR_CheckDataReady(MMPF_VIDMGR_HANDLE *MgrHandle);
MMPF_VIDMGR_HANDLE *MMPF_VIDMGR_GetHandle(MMP_UBYTE ubMgrId);
MMP_ERR     MMPF_VIDMGR_GetDescQueueDepth (MMP_ULONG *ulDepth, MMPF_VIDMGR_HANDLE *pMgr);
MMPF_VIDMGR_CODED_DESC *MMPF_VIDMGR_GetDescQueueWrPtr (MMPF_VIDMGR_HANDLE *pMgr);
MMPF_VIDMGR_CODED_DESC *MMPF_VIDMGR_GetDescQueueRdPtr (MMPF_VIDMGR_HANDLE *pMgr);
void        MMPF_VIDMGR_UpdateDescQueueWrIdx (MMPF_VIDMGR_HANDLE *pMgr);
void        MMPF_VIDMGR_UpdateDescQueueRdIdx (MMPF_VIDMGR_HANDLE *pMgr);
void        MMPF_VIDMGR_SetVideoCompressedBuffer(MMP_ULONG startaddr, MMP_ULONG bufsize);
void		MMPF_VIDMGR_SetTimeCounter(MMP_ULONG timecnt);
MMP_ULONG	MMPF_VIDMGR_GetTimeCounter(void);

MMP_ERR     MMPF_VIDMGR_GetWriteBufInfo(MMP_UBYTE ubEncId, MMP_ULONG *ulStartAddr,
                                        MMP_ULONG *ulLowBound, MMP_ULONG *ulHighBound,
                                        MMP_ULONG *ulMaxFreeSize);
MMP_ERR     MMPF_VIDMGR_UpdateVideoWritePtr (MMP_ULONG CurSize);
void        MMPF_VIDMGR_UpdateVideoReadPtr (MMP_ULONG CurSize);
MMP_ERR     MMPF_VIDMGR_AlignVideoWritePtr (void);
void        MMPF_VIDMGR_AlignVideoReadPtr (void);
void        MMPF_VIDMGR_SetMtsMuxModeEnable (MMP_BOOL tmp);
MMP_BOOL    MMPF_VIDMGR_GetMtsMuxModeEnable (void);
MMP_BOOL    MMPF_VIDMGR_GetRingBufEnable(void);
MMP_ERR     MMPF_VIDMGR_SetRingBufEnable(MMP_BOOL bEnable);
MMP_UBYTE   MMPF_VIDMGR_SetOutputPipe (MMP_UBYTE ubEid, MMP_UBYTE ubPipe);
MMP_UBYTE   MMPF_VIDMGR_GetOutputPipe (MMP_UBYTE ubEid);
MMP_ERR     MMPF_VIDMGR_StartMts (MMP_UBYTE ubEncId);
MMP_ERR MMPF_VIDMGR_PrepareUvcHeader (MMP_ULONG ulPayloadSize, MMP_ULONG ulTimestamp,
                                      MMP_USHORT usLayerId, MMP_BOOL bIdrFlag, MMP_BOOL bEOF, MMP_BOOL bSOF, void *EncHandle,
                                      MMP_ULONG ulPadBytes);
MMP_ERR     MMPF_VIDMGR_SendCodedData(MMPF_VIDMGR_CODED_DESC *pDesc);
MMP_ERR MMPF_VIDMGR_SetDropStreamId(MMP_UBYTE sid);
#endif
