/// @ait_only
/**
 @file mmpd_3gpmgr.h
 @brief Header File for the Host 3GP MERGER Driver.
 @author Will Tseng
 @version 1.0
*/

#ifndef _MMPD_3GPMGR_H_
#define _MMPD_3GPMGR_H_

#include "ait_config.h"
#include "mmp_lib.h"

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
#if (CHIP == P_V2) || (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define LARGE_FRAME_EN      (1)
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

/// AV operation mode. This is for recorder only, used in Initialization
typedef enum _MMPD_3GPMGR_AUDIO_FORMAT {
    MMPD_3GPMGR_AUDIO_FORMAT_AAC = 0x00,    ///< Video encode with AAC audio
    MMPD_3GPMGR_AUDIO_FORMAT_AMR           ///< Video encode with AMR audio
} MMPD_3GPMGR_AUDIO_FORMAT;

/// 3gp data format
typedef enum _MMPD_3GPMGR_FORMAT {
    MMPD_3GPMGR_FORMAT_OTHERS = 0x00,
    MMPD_3GPMGR_FORMAT_H263,
    MMPD_3GPMGR_FORMAT_MP4V,
    MMPD_3GPMGR_FORMAT_H264,
    MMPD_3GPMGR_FORMAT_MJPEG,
    MMPD_3GPMGR_FORMAT_YUV422,
    MMPD_3GPMGR_FORMAT_YUV420
} MMPD_3GPMGR_FORMAT;

/// Buffer info
typedef struct _MMPD_3GPMGR_REPACKBUF {
    MMP_ULONG ulAvRepackStartAddr;   			///< AV repack buffer start address for 3gp encoder (can be calculated)
    MMP_ULONG ulAvRepackSize;     				///< AV repack buffer size for 3gp encoder (can be calculated)
    MMP_ULONG ulVideoEncSyncAddr;   	      	///< Parameter sync buffer for 3gp encoder (can be calculated)
    MMP_ULONG ulVideoSizeTableAddr;		   		///< Video encode frame table buffer start address (can be calculated)
    MMP_ULONG ulVideoSizeTableSize;     		///< Video encode frame table buffer size (can be calculated)
    MMP_ULONG ulVideoTimeTableAddr;   			///< Video encode time table buffer start address (can be calculated)
    MMP_ULONG ulVideoTimeTableSize;    			///< Video encode time table buffer size (can be calculated)
	MMP_ULONG ulVideoRegStoreAddr;				///< Buffer to store register for frame base encoder
} MMPD_3GPMGR_REPACKBUF;

/// Aux table index
typedef enum _MMPD_3GPMGR_AUX_TABLE {
    MMPD_3GPMGR_AUX_FRAME_TABLE = 0,			///< Video encode aux frame table
    MMPD_3GPMGR_AUX_TIME_TABLE = 1				///< Video encode aux time table
} MMPD_3GPMGR_AUX_TABLE;

typedef struct _MMPD_3GPMGR_AVCOMPRESSEDBUFFER {
	MMP_ULONG ulVideoCompBufStart;
	MMP_ULONG ulVideoCompBufEnd;
	MMP_ULONG ulAudioCompBufStart;
	MMP_ULONG ulAudioCompBufEnd;
} MMPD_3GPMGR_AVCOMPRESSEDBUFFER;

/// Video speed mode
typedef enum _MMPD_3GPMGR_SPEED_MODE {
    MMPD_3GPMGR_SPEED_NORMAL = 0,                      ///< Video speed, normal
    MMPD_3GPMGR_SPEED_SLOW,                            ///< Video speed, slow
    MMPD_3GPMGR_SPEED_FAST                             ///< Video speed, fast
} MMPD_3GPMGR_SPEED_MODE;

/// Video speed mode
typedef enum _MMPD_3GPMGR_SPEED_RATIO {
    MMPD_3GPMGR_SPEED_1X = 0,                          ///< Video speed ratio
    MMPD_3GPMGR_SPEED_2X,
    MMPD_3GPMGR_SPEED_3X,
    MMPD_3GPMGR_SPEED_4X,
    MMPD_3GPMGR_SPEED_5X,
    MMPD_3GPMGR_SPEED_MAX
} MMPD_3GPMGR_SPEED_RATIO;

/// Video frame type
typedef enum _MMPD_3GPMGR_FRAME_TYPE {
    MMPD_3GPMGR_FRAME_TYPE_I = 0,
    MMPD_3GPMGR_FRAME_TYPE_P,
    MMPD_3GPMGR_FRAME_TYPE_B,
    MMPD_3GPMGR_FRAME_TYPE_MAX
} MMPD_3GPMGR_FRAME_TYPE;
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
/** @addtogroup MMPD_3GPMGR
 *  @{
 */

typedef MMP_USHORT (*MMPD_3GPMGR_FileWrite)(void *FileHandle, MMP_UBYTE *ubSrcAddr, MMP_ULONG ulByte, MMP_ULONG *ulCount, MMP_BOOL bAsync);
typedef MMP_USHORT (*MMPD_3GPMGR_FileRead)(void *FileHandle, MMP_UBYTE *uData, MMP_ULONG ulNumBytes, MMP_ULONG *ulReadCount);
typedef MMP_USHORT (*MMPD_3GPMGR_FileSeek)(void *FileHandle, MMP_LONG lOffset, MMP_USHORT ulOrigin);
MMP_ERR MMPD_3GPMGR_SetTargetFrameSize(MMP_USHORT targetFrameSize);
MMP_ERR MMPD_3GPMGR_SyncFTableBufReadPtr(void);
MMP_ERR MMPD_3GPMGR_SyncFTableBufWritePtr(void);
MMP_ERR MMPD_3GPMGR_SyncVideoBufReadPtr(void);
MMP_ERR MMPD_3GPMGR_SyncAudioBufReadPtr(void);
MMP_ERR MMPD_3GPMGR_SetStoragePath(MMP_UBYTE ubEnable);
MMP_ERR MMPD_3GPMGR_SetFileName(MMP_BYTE bFileName[], MMP_USHORT usLength);
MMP_ERR MMPD_3GPMGR_EnableAVSyncEncode(MMP_UBYTE ubEnable);
MMP_ERR MMPD_3GPMGR_SetEncodeCompBuf(MMPD_3GPMGR_AVCOMPRESSEDBUFFER *BufInfo);
MMP_ERR MMPD_3GPMGR_InitializeAVSyncBuf(void);
MMP_ERR MMPD_3GPMGR_Cancel3gpSave2Storage(void);
MMP_ERR MMPD_3GPMGR_Get3gpFileCurSize(MMP_ULONG *ulCurSize);
MMP_ERR MMPD_3GPMGR_Get3gpSaveStatus(MMP_UBYTE *status);
MMP_ERR MMPD_3GPMGR_Get3gpFileSize(MMP_ULONG *filesize);
MMP_ERR MMPD_3GPMGR_SetFileLimit(MMP_ULONG ulFileMax, MMP_ULONG *ulSpace);
MMP_ERR MMPD_3GPMGR_SetFTableExternalBuf(MMP_ULONG ulAddr, MMP_ULONG ulSize);
MMP_ERR MMPD_3GPMGR_SetAVExternalBuf(MMP_ULONG ulAddr, MMP_ULONG ulSize);
MMP_ERR MMPD_3GPMGR_TransferTimeTab2ExternalBuf(void);
MMP_ERR MMPD_3GPMGR_TransferFTable2ExternalBuf(void);
MMP_ERR MMPD_3GPMGR_TransferAV2ExternalBuf(void);
MMP_ULONG MMPD_3GPMGR_GetFTableCaptureLength(void);
MMP_ULONG MMPD_3GPMGR_GetAVdataCaptureLength(void);
MMP_ERR MMPD_3GPMGR_FillTail(void);
MMP_ERR MMPD_3GPMGR_SetAudioAACParam(MMP_ULONG ulSampleRate, MMPD_3GPMGR_AUDIO_FORMAT AudioMode);
MMP_ERR MMPD_3GPMGR_SetAudioAMRParam(MMP_ULONG ulSampleSize, MMPD_3GPMGR_AUDIO_FORMAT AudioMode);
MMP_ERR MMPD_3GPMGR_GetRecordingTime(MMP_ULONG *ulTime);
MMP_ERR MMPD_3GPMGR_AutoCountAudioTime(MMP_ULONG *ulAudioTime);
MMP_ERR MMPD_3GPMGR_SetTimeLimit(MMP_ULONG ulTimeMax);
MMP_ERR MMPD_3GPMGR_PreFill3gpHeader(void);
MMP_ERR MMPD_3GPMGR_GetStatus(MMP_ERR *status);
MMP_UBYTE MMPD_3GPMGR_GetStoragePath(void);
MMPD_3GPMGR_AUDIO_FORMAT MMPD_3GPMGR_GetAudioFormat(void);
MMP_ERR MMPD_3GPMGR_GetHostFsCmdCount(MMP_ULONG *ul3gpCnt, MMP_ULONG *ulFtCnt);
MMP_ERR MMPD_3GPMGR_SetHostFsAsyncMode(MMP_BOOL bAsyncOn);
MMP_ERR MMPD_3GPMGR_SetCallBackFunc(MMPD_3GPMGR_FileWrite FileWrite, MMPD_3GPMGR_FileRead FileRead, MMPD_3GPMGR_FileSeek FileSeek);
MMP_ERR MMPD_3GPMGR_SetFileHandle(void *RecdFileHandle, void *FTableFileHandle);
MMP_ERR MMPD_3GPMGR_SaveAvExtBuf2HostStorage(MMP_BOOL bMovePart);
MMP_ERR MMPD_3GPMGR_SaveFtExtBuf2HostStorage(MMP_BOOL bMovePart);
MMP_ERR MMPD_3GPMGR_SaveTtExtBuf2HostStorage(MMP_BOOL bMovePart);
MMP_ERR MMPD_3GPMGR_SetExtBufMoveSegment(MMP_ULONG ulAvMask, MMP_ULONG ulFtMask, MMP_ULONG ulIntMask);
MMP_ERR MMPD_3GPMGR_ReadFrameTable(MMP_ULONG ulOffset, MMP_ULONG *ulSize);
MMP_ERR MMPD_3GPMGR_ReadTimeTable(MMP_ULONG ulOffset, MMP_ULONG *ulTime);
MMP_ERR MMPD_3GPMGR_SetHeader(void);
MMP_ERR MMPD_3GPMGR_SetGOP(MMP_USHORT usPFrame, MMP_USHORT usBFrame);
MMP_ERR MMPD_3GPMGR_SetMpegFormat(MMPD_3GPMGR_FORMAT MgrFormat);
MMP_ERR MMPD_3GPMGR_SetResolution(MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR MMPD_3GPMGR_SetStopCmdStatus(MMP_UBYTE ubStop);


MMP_ERR MMPD_3GPMGR_SetRepackMiscBuf(MMPD_3GPMGR_REPACKBUF *repackbuf);
MMP_ERR MMPD_3GPMGR_UpdateAsyncReadPtr(MMP_ULONG readSize);
MMP_ERR MMPD_3GPMGR_SetAsyncMode(MMP_BOOL mode);
MMP_ERR MMPD_3GPMGR_SetRecordSpeed(MMPD_3GPMGR_SPEED_MODE SpeedMode, MMPD_3GPMGR_SPEED_RATIO SpeedRatio);
MMP_ERR MMPD_3GPMGR_SetRecordTailSpeed(MMP_BOOL ubHighSpeedEn, MMP_ULONG ulTailInfoAddress, MMP_ULONG ulTailInfoSize);
/// @}
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _INCLUDES_H_
/// @end_ait_only
