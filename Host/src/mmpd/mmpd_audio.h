/// @ait_only
//==============================================================================
//
//  File        : mmpd_audio.h
//  Description : Audio Control Driver Function
//  Author      : Hnas Liu
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPD_AUDIO_H_
#define _MMPD_AUDIO_H_

#include "ait_config.h"

/** @addtogroup MMPD_AUDIO
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

#define MMPD_AUDIO_PLAY_HANDSHAKE_BUF_SIZE          (24)

#define MMPD_AUDIO_PLAY_R_HPTR_OFST                 (0)
#define MMPD_AUDIO_PLAY_R_PTR_OFST                  (4)
#define MMPD_AUDIO_PLAY_FINISH_SEEK_W               (8)
#define MMPD_AUDIO_PLAY_START_SEEK_W                (10)
#define MMPD_AUDIO_PLAY_FILE_SEEK_OFFSET_L_W        (12)
#define MMPD_AUDIO_PLAY_FILE_SEEK_OFFSET_H_W        (14)
#define MMPD_AUDIO_PLAY_W_HPTR_OFST                 (16)
#define MMPD_AUDIO_PLAY_W_PTR_OFST                  (20)

#define MMPD_AUDIO_REC_HANDSHAKE_BUF_SIZE           (8)
#define MMPD_AUDIO_REC_WRITE_HIGH_PTR_OFFSET_W      (0)
#define MMPD_AUDIO_REC_WRITE_PTR_OFFSET_W           (2)
#define MMPD_AUDIO_REC_READ_HIGH_PTR_OFFSET_W       (4)
#define MMPD_AUDIO_REC_READ_PTR_OFFSET_W            (6)

//Audio in path
#define MMPD_AUDIO_IN_I2S               0x01 ///< audio in using i2s
#define MMPD_AUDIO_IN_AFE_SING			0x02 ///< audio in using internal adc: (AUXL/AUXR)
#define MMPD_AUDIO_IN_AFE_DIFF 			0x03 ///< audio in using internal adc: (LIP/LIN/RIP/RIN)
#define MMPD_AUDIO_IN_AFE_DIFF2SING     0x04 ///< audio in using internal adc: (LIP/RIP)

//Audio out path
#define MMPD_AUDIO_OUT_I2S 				0x01 ///< audio out using i2s
#define MMPD_AUDIO_OUT_AFE_HP 			0x02 ///< audio out using internal dac: HP_OUT (LOUT/ROUT)
#define MMPD_AUDIO_OUT_AFE_HP_INVT 		0x04 ///< audio out using internal dac: HP_OUT Inverter (LOUT/ROUT)
#define MMPD_AUDIO_OUT_AFE_LINE 		0x08 ///< audio out using internal dac: LINE_OUT (LOUT2/ROUT2)
#define MMPD_AUDIO_OUT_AFE_LINE_INVT 	0x10 ///< audio out using internal dac: LINE_OUT Inverter (LOUT2/ROUT2)

typedef enum _MMPD_AUDIO_PLAY_DATAPATH
{
    MMPD_AUDIO_PLAY_FROM_MEM = 0,   ///< memory mode
    MMPD_AUDIO_PLAY_FROM_CARD,      ///< card mode
    MMPD_AUDIO_PLAY_FROM_MMP_MEM    ///< MMP memory mode
} MMPD_AUDIO_PLAY_DATAPATH;

typedef enum _MMPD_AUDIO_RECORD_DATAPATH
{
    MMPD_AUDIO_RECORD_TO_MEM = 0,///< memory mode
    MMPD_AUDIO_RECORD_TO_CARD    ///< card mode
} MMPD_AUDIO_RECORD_DATAPATH;

#define MMPD_AUDIO_SPECTRUM_DATA_SIZE  (64)

#define MMPD_AUDIO_MAX_VOLUME_LEVEL    (39)

#define MMPD_AUDIO_MAX_MIC_VOLUME_LEVEL   (10)

#define OGG_TAG_BUF_SIZE    (512)

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPD_AUDIO_AACPLUS_INFO {
    MMP_ULONG   ulBitRate;      // bit rate
    MMP_USHORT  usSampleRate;   // sampling rate
    MMP_USHORT  usHeaderType;   // header type:0->unknown, 1->ADIF, 2->ADTS
    MMP_USHORT  usChans;        // channel num
    MMP_ULONG	ulTotalTime;    // file total time(unit: sec)
    MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
} MMPD_AUDIO_AACPLUS_INFO;

typedef struct _MMPD_AUDIO_FLAC_INFO {
    MMP_ULONG   ulBitRate;      // bit rate
    MMP_ULONG64   ulTotalSamples; // total samples
    MMP_USHORT  usSampleRate;   // sampling rate
    MMP_USHORT  usChans;        // channel num
    MMP_ULONG	ulTotalTime;    // file total time(unit: sec)
    MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
} MMPD_AUDIO_FLAC_FILE_INFO;

typedef struct _MMPD_AUDIO_MIDI_INFO {
        MMP_USHORT  usSampleRate;   //sampleing rate
        MMP_ULONG   ulTotalTime;    //total time(unit: milli-sec)
        MMP_USHORT  usNotSupport;   // 0 : supported format; 1 : not supported format
} MMPD_AUDIO_MIDI_FILE_INFO;

typedef struct _MMPD_AUDIO_MP3_INFO {
        MMP_ULONG 	ulBitRate;      //unit: bps
        MMP_USHORT  usSampleRate;   //unit: Hz
        MMP_USHORT  usChans;        //1->mono, 2->stereo
        MMP_USHORT  usLayer;        //3->layer 3
        MMP_USHORT  usVersion;      //0->MPEG1, 1->MPEG2, 2->MPEG2.5
        MMP_ULONG	ulTotalTime;    //unit: milli-sec
		MMP_USHORT	usVbr;          //0:CBR, 1:VBR
        MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
        MMP_BOOL    bIsMp3HdFile;   //MMP_TRUE: mp3HD file
} MMPD_AUDIO_MP3_INFO;

typedef struct _MMPD_AUDIO_AMR_INFO {
        MMP_ULONG   ulBitRate;      // bit rate
        MMP_USHORT	usMode;         //0:4.75k, 7:12.2k
        MMP_USHORT	usSampleSize;   //frame size
        MMP_USHORT  usSampleRate;   //sample rate
        MMP_ULONG	ulTotalTime;    //unit: milli-sec
        MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
} MMPD_AUDIO_AMR_INFO;

typedef struct _MMPD_AUDIO_WMA_INFO {
        MMP_ULONG   ulBitRate;      //bit rate
        MMP_ULONG   ulSampleRate;   //sampling rate
        MMP_USHORT  usChans;        //channel num
        MMP_USHORT  usBitsPerSample;//bit per sample
        MMP_ULONG	ulTotalTime;    //unit: milli-sec
        MMP_USHORT  usTagInfo;      //0:no tag, 1:tag exist
        MMP_USHORT  usVbrInfo;      //0:CBR, 1:VBR
        MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
        MMP_USHORT  usVersion;      //0, 1, 2: WMA Std. bitstream,  3: WMA Pro bitstream
} MMPD_AUDIO_WMA_INFO;

typedef struct _MMPD_AUDIO_RA_INFO {
        MMP_ULONG   ulBitRate;      //bit rate
        MMP_USHORT  usSampleRate;   //sampling rate
        MMP_USHORT  usChans;        //channel num
        MMP_ULONG	ulTotalTime;    //unit: milli-sec
        MMP_USHORT  usVbrInfo;      //0:CBR, 1:VBR
        MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
} MMPD_AUDIO_RA_INFO;

typedef struct _MMPD_AUDIO_OGG_INFO {
        MMP_ULONG   ulBitRate;      //bit rate
        MMP_USHORT  usSampleRate;   //sampling rate
        MMP_USHORT  usChans;        //channel num
        MMP_ULONG	ulTotalTime;    //unit: milli-sec
        MMP_USHORT  usTagInfo;      //0:no tag, 1:tag exist
        MMP_USHORT  usVbrInfo;      //0:CBR, 1:VBR
        MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
} MMPD_AUDIO_OGG_INFO;

typedef struct _MMPD_AUDIO_WAV_INFO {
        MMP_ULONG   ulBitRate;      //bit rate
        MMP_ULONG   ulTotalTime;    //unit: milli-sec
        MMP_USHORT  usChans;        //channel num
        MMP_USHORT  usSampleRate;   //sampling rate
        MMP_USHORT  usNewSampleRate;//new sampling rate
        MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
} MMPD_AUDIO_WAV_INFO;


typedef struct {
	    MMP_ULONG	title_len;	
	    MMP_ULONG   version_len;				
	    MMP_ULONG	album_len;		
	    MMP_ULONG	artist_len;					
	    MMP_ULONG   copyright_len;			
	    MMP_ULONG   description_len;		
} MMPD_AUDIO_OGG_TAG_INFO;

typedef struct {
        MMP_ULONG   ulBitRate;      // bit rate
        MMP_USHORT  usSampleRate;   // sampling rate
        MMP_USHORT  usHeaderType;   // header type:0->unknown, 1->ADIF, 2->ADTS
        MMP_USHORT  usChans;        // channel num
        MMP_ULONG	ulTotalTime;    // file total time(unit: sec)
        MMP_USHORT  usNotSupport;   //0 : supported format; 1 : not supported format
} MMPD_AUDIO_AAC_FILE_INFO;

typedef enum _MMPD_AUDIO_I2S_FORMAT
{
    MMPD_AUDIO_MASTER_STD_16 = 0,	///< master standard mode, 16bits
    MMPD_AUDIO_SLAVE_I2S_20         ///< slave i2s mode, 20bits
} MMPD_AUDIO_I2S_FORMAT;

typedef enum _MMPD_AUDIO_ENCODE_FMT
{
    MMPD_AUDIO_VAMR_ENCODE = 0,     ///< video + amr encode mode
    MMPD_AUDIO_VAAC_ENCODE = 1,     ///< video + aac encode mode
    MMPD_AUDIO_AMR_ENCODE = 2,      ///< pure amr encode mode
    MMPD_AUDIO_AAC_ENCODE = 3,      ///< pure aac encode mode
    MMPD_AUDIO_MP3_ENCODE = 4,      ///< pure mp3 encode mode
    MMPD_AUDIO_WAV_ENCODE = 5       ///< pure wav encode mode
} MMPD_AUDIO_ENCODE_FMT;

typedef enum _MMPD_AUDIO_DECODE_FMT
{
    MMPD_AUDIO_MP3_PLAY = 0,		///< mp3 play mmode
    MMPD_AUDIO_MIDI_PLAY = 1,		///< midi play mode
    MMPD_AUDIO_AMR_PLAY = 2,		///< amr play mode
    MMPD_AUDIO_WMA_PLAY = 3,		///< wma play mode
    MMPD_AUDIO_AAC_PLAY = 4,		///< aac play mode
    MMPD_AUDIO_PCM_PLAY = 5,		///< pcm play mode
    MMPD_AUDIO_AACPLUS_PLAY = 7,	///< aac+ play mode
	MMPD_AUDIO_OGG_PLAY = 9,		///< ogg play mode
    MMPD_AUDIO_VAMR_PLAY = 10,		///< video + amr play mode
    MMPD_AUDIO_VAAC_PLAY = 11,		///< video + aac play mode
	MMPD_AUDIO_RA_PLAY = 12,        ///< ra play mode
	MMPD_AUDIO_WAV_PLAY = 13,       ///< wav play mode
	MMPD_AUDIO_VMP3_PLAY = 14,       ///< video + mp3 play mode
    MMPD_AUDIO_AC3_PLAY = 15,		///< ac3 play mmode	
	MMPD_AUDIO_VAC3_PLAY = 16,       ///< video + ac3 play mode	    
	MMPD_AUDIO_VRA_PLAY = 17,       ///< video + ra play mode	    
	MMPD_AUDIO_VWMA_PLAY = 18,
	MMPD_AUDIO_VWAV_PLAY = 19,
	MMPD_AUDIO_FLAC_PLAY = 20
	
} MMPD_AUDIO_DECODE_FMT;

typedef enum _MMPD_AUDIO_LINEIN_CHANNEL
{
    MMPD_AUDIO_LINEIN_DUAL = 0x0,		///< dual channel line in
    MMPD_AUDIO_LINEIN_R = 0x1,			///< Right channel line in
    MMPD_AUDIO_LINEIN_L = 0x2			///< Left channel line in
} MMPD_AUDIO_LINEIN_CHANNEL;

#if (GAPLESS_PLAY_EN == 1)
typedef enum _MMPD_AUDIO_GAPLESS_OP
{
    MMPD_AUDIO_GAPLESS_SEEK     = 0x0001,
    MMPD_AUDIO_GAPLESS_PAUSE    = 0x0002,
    MMPD_AUDIO_GAPLESS_STOP     = 0x0004,
    MMPD_AUDIO_GAPLESS_OP_ALL   = 0x0007
} MMPD_AUDIO_GAPLESS_OP;
#endif

extern MMP_ERR	MMPD_AUDIO_CheckPlayEnd(void);
extern MMP_ERR  MMPD_AUDIO_SetMP3HDEnable(MMP_BOOL bIsMp3HdFile);
extern MMP_ERR  MMPD_AUDIO_GetAACPlusFileInfo(MMPD_AUDIO_AACPLUS_INFO *aacplusinfo);
extern MMP_ERR  MMPD_AUDIO_GetMIDIFileInfo(MMPD_AUDIO_MIDI_FILE_INFO *midiinfo);
extern MMP_ERR  MMPD_AUDIO_GetMP3FileInfo(MMPD_AUDIO_MP3_INFO *mp3info);
extern MMP_ERR  MMPD_AUDIO_GetWMAFileInfo(MMPD_AUDIO_WMA_INFO *wmainfo);
extern MMP_ERR  MMPD_AUDIO_GetRAFileInfo(MMPD_AUDIO_RA_INFO *rainfo);
extern MMP_ERR  MMPD_AUDIO_GetAMRFileInfo(MMPD_AUDIO_AMR_INFO *amrinfo);
extern MMP_ERR  MMPD_AUDIO_GetAACFileInfo(MMPD_AUDIO_AAC_FILE_INFO *aacinfo);
extern MMP_ERR  MMPD_AUDIO_GetWAVFileInfo(MMPD_AUDIO_WAV_INFO *wavinfo);
extern MMP_ERR	MMPD_AUDIO_GetFLACFileInfo(MMPD_AUDIO_FLAC_FILE_INFO *flacinfo);
extern MMP_ERR  MMPD_AUDIO_ReadAACPlusFileInfo(MMPD_AUDIO_AACPLUS_INFO *aacplusinfo);
extern MMP_ERR  MMPD_AUDIO_ReadMIDIFileInfo(MMPD_AUDIO_MIDI_FILE_INFO *midiinfo);
extern MMP_ERR  MMPD_AUDIO_ReadMP3FileInfo(MMPD_AUDIO_MP3_INFO *mp3info);
extern MMP_ERR  MMPD_AUDIO_ReadWMAFileInfo(MMPD_AUDIO_WMA_INFO *wmainfo);
extern MMP_ERR  MMPD_AUDIO_ReadAMRFileInfo(MMPD_AUDIO_AMR_INFO *amrinfo);
extern MMP_ERR  MMPD_AUDIO_ReadOGGFileInfo(MMPD_AUDIO_OGG_INFO *ogginfo);
extern MMP_ERR  MMPD_AUDIO_ReadRAFileInfo(MMPD_AUDIO_RA_INFO *rainfo);
extern MMP_ERR  MMPD_AUDIO_ReadWAVFileInfo(MMPD_AUDIO_WAV_INFO *wavinfo);
extern MMP_ERR	MMPD_AUDIO_ReadFLACFileInfo(MMPD_AUDIO_FLAC_FILE_INFO *flacinfo);
extern MMP_ERR  MMPD_AUDIO_StopAudioPlay(void);
extern MMP_ERR	MMPD_AUDIO_ResumeAudioPlay(void);
extern MMP_ERR	MMPD_AUDIO_PauseAudioPlay(void);
extern MMP_ERR	MMPD_AUDIO_StartAudioPlay(void* , void* );
extern MMP_ERR  MMPD_AUDIO_GetAACPlusPlayTime(MMP_USHORT *usPosRatio,MMP_ULONG *ulMilliSecond);
extern MMP_ERR	MMPD_AUDIO_GetMIDIPlayTime(MMP_USHORT *usTimeRatio,MMP_ULONG *ulMilliSecond);
extern MMP_ERR  MMPD_AUDIO_GetMP3PlayTime(MMP_USHORT *usPosRatio,MMP_ULONG *ulMilliSecond);
extern MMP_ERR  MMPD_AUDIO_GetWMAPlayTime(MMP_USHORT *usPosRatio,MMP_ULONG *ulMilliSecond);
extern MMP_ERR  MMPD_AUDIO_GetRAPlayTime(MMP_USHORT *usPosRatio,MMP_ULONG *ulMilliSecond);
extern MMP_ERR  MMPD_AUDIO_GetOGGPlayTime(MMP_ULONG *ulMilliSecond);
extern MMP_ERR  MMPD_AUDIO_GetWAVPlayTime(MMP_ULONG *ulMilliSecond);
extern MMP_ERR  MMPD_AUDIO_GetAMRPlayTime(MMP_USHORT *usPosRatio,MMP_ULONG *ulMilliSecond);
extern MMP_ERR	MMPD_AUDIO_GetFLACPlayTime(MMP_USHORT *usPosRatio, MMP_ULONG *ulMilliSecond);
extern MMP_ERR	MMPD_AUDIO_SetMIDIPlayTime(MMP_ULONG ulTime);
extern MMP_ERR	MMPD_AUDIO_SetAACPlusPlayTime(MMP_ULONG ulMilliSec);
extern MMP_ERR	MMPD_AUDIO_SetMP3PlayTime(MMP_ULONG ulMilliSec);
extern MMP_ERR	MMPD_AUDIO_SetWMAPlayTime(MMP_ULONG ulTime);
extern MMP_ERR	MMPD_AUDIO_GetWMAVersion(MMP_ULONG *ulVersion);
extern MMP_ERR	MMPD_AUDIO_SetRAPlayTime(MMP_ULONG ulTime);
extern MMP_ERR	MMPD_AUDIO_SetOGGPlayTime(MMP_ULONG ulTime);
extern MMP_ERR  MMPD_AUDIO_SetWAVPlayTime(MMP_ULONG ulMilliSec);
extern MMP_ERR	MMPD_AUDIO_SetAACPlayTime(MMP_ULONG ulMilliSec);
extern MMP_ERR	MMPD_AUDIO_SetAMRPlayTime(MMP_ULONG ulMilliSec);
extern MMP_ERR	MMPD_AUDIO_SetFLACPlayTime(MMP_ULONG ulMilliSec);
extern MMP_ERR  MMPD_AUDIO_GetPlayStatus(MMP_USHORT *usStatus);
extern MMP_ERR	MMPD_AUDIO_SetEncodeFileSize(MMP_ULONG ulSize);
extern MMP_ERR	MMPD_AUDIO_StartAudioRecord(void);
extern MMP_ERR	MMPD_AUDIO_PauseAudioRecord(void);
extern MMP_ERR	MMPD_AUDIO_ResumeAudioRecord(void);
extern MMP_ERR	MMPD_AUDIO_StopAudioRecord(void);
extern MMP_ERR	MMPD_AUDIO_SetPlayPosition(MMP_ULONG ulPos);
extern MMP_ERR	MMPD_AUDIO_EnableABRepeatMode(MMP_USHORT usEnable);
extern MMP_ERR	MMPD_AUDIO_SetPlayVolume(MMP_UBYTE ubSWGain, MMP_UBYTE ubDGain, MMP_UBYTE ubAGain, MMP_BOOL bSWGainOnly);
extern MMP_ERR	MMPD_AUDIO_SetRecordVolume(MMP_UBYTE ubDGain, MMP_UBYTE ubAGain, MMP_BOOL bboostup);
extern MMP_ERR	MMPD_AUDIO_StopMIDINotePlay(void);
extern MMP_ERR	MMPD_AUDIO_StartMIDINotePlay(void);
extern MMP_ERR	MMPD_AUDIO_SetEQType(MMP_USHORT usType);
extern MMP_ERR	MMPD_AUDIO_SetPlayBuf(MMP_ULONG ulAddr,MMP_ULONG ulSize);
extern MMP_ERR  MMPD_AUDIO_SetPlayFileName(MMP_BYTE *bFileName, MMP_ULONG ulNameLength);
extern MMP_ERR  MMPD_AUDIO_SetPlayPath(MMPD_AUDIO_PLAY_DATAPATH ubPath);
extern MMP_ERR  MMPD_AUDIO_SetEncodePath(MMPD_AUDIO_RECORD_DATAPATH ubPath);
extern MMP_ERR  MMPD_AUDIO_SetEncodeFileName(MMP_BYTE *bFileName, MMP_ULONG ulNameLength);
extern MMP_ERR	MMPD_AUDIO_SetEncodeBuf(MMP_ULONG ulAddr,MMP_ULONG ulSize);
extern MMP_ERR	MMPD_AUDIO_SetEncodeMode(MMP_USHORT usMode);
extern MMP_ERR	MMPD_AUDIO_GetEncodeMode(MMP_USHORT *usMode);
extern MMP_ERR  MMPD_AUDIO_GetRecordSize(MMP_ULONG *ulSize);
extern MMP_ERR  MMPD_AUDIO_GetRecordStatus(MMP_USHORT *usStatus);
extern MMP_ERR  MMPD_AUDIO_GetWriteCardStatus(MMP_USHORT *usStatus);
extern MMP_ERR	MMPD_AUDIO_GetPlayFileSize(MMP_ULONG *ulFileSize);
extern MMP_ERR	MMPD_AUDIO_UpdatePlayReadPtr(void);
extern MMP_ERR	MMPD_AUDIO_UpdatePlayWritePtr(void);
extern MMP_ERR	MMPD_AUDIO_SetPlayFileSize(MMP_ULONG ulFileSize);
extern MMP_ERR  MMPD_AUDIO_TransferDataToAIT(MMP_UBYTE *ubAudioPtr, MMP_ULONG ulMaxSize, 
				MMP_ULONG *ulReturnSize);
extern MMP_ERR	MMPD_AUDIO_InitPlayRWPtr(void);
extern MMP_ERR	MMPD_AUDIO_SetPlayHandshakeBuf(MMP_ULONG addr,MMP_ULONG ulSize);
extern MMP_ERR	MMPD_AUDIO_SetRecHandshakeBuf(MMP_ULONG ulAddr,MMP_ULONG ulSize);
extern MMP_ERR	MMPD_AUDIO_UpdateRecWritePtr(void);
extern MMP_ERR  MMPD_AUDIO_TransferDataToHost(MMP_UBYTE *ubAudioPtr, MMP_ULONG ulMaxSize, 
				MMP_ULONG *ulReturnSize);
extern MMP_ERR	MMPD_AUDIO_InitRecRWPtr(void);
extern MMP_ERR	MMPD_AUDIO_SetSBCEncodeEnable(MMP_UBYTE ubEnable,MMP_ULONG ulIntPeriod);
extern MMP_ERR	MMPD_AUDIO_GetSBCEncodeEnable(MMP_BOOL *bEnable);
extern MMP_ERR	MMPD_AUDIO_SetSBCChannelMode(MMP_USHORT usMode);
extern MMP_ERR	MMPD_AUDIO_SetSBCAllocMethod(MMP_USHORT usMethod);
extern MMP_ERR	MMPD_AUDIO_SetSBCNrofBlocks(MMP_USHORT usBlocks);
extern MMP_ERR	MMPD_AUDIO_SetSBCNrofSubbands(MMP_USHORT usSubBands);
extern MMP_ERR	MMPD_AUDIO_SetSBCBitrate(MMP_USHORT usBitRate);
extern MMP_ERR	MMPD_AUDIO_SetSBCBitPool(MMP_USHORT usBitPool);
extern MMP_ERR	MMPD_AUDIO_SetSBCEncBuf(MMP_ULONG ulBufAddr, MMP_ULONG ulBufSize);
extern MMP_ERR  MMPD_AUDIO_GetSBCInfo(  MMP_ULONG *ulOutSampleRate, 
                                        MMP_ULONG *ulOutBitRate,
                                        MMP_ULONG *ulOutFrameSize,
                                        MMP_ULONG *ulSBCFrameCount,
                                        MMP_ULONG *ulSBCSamplesPerFrame,
                                        MMP_ULONG *ulSBCChannelNum,
                                        MMP_ULONG *ulSBCBitpool);

extern MMP_ERR MMPD_AUDIO_SetAC3VHPar(MMP_LONG lm3deff, MMP_LONG ldb, MMP_BOOL bSet_m3deff, MMP_BOOL bSet_db);
                                        
extern MMP_ERR	MMPD_AUDIO_SetWAVEncodeEnable(MMP_UBYTE ubEnable);
extern MMP_ERR  MMPD_AUDIO_GetWAVInfo(MMP_ULONG *ulOutSampleCount);
                                        
extern MMP_ERR  MMPD_AUDIO_TransferMIDIDataToAIT(MMP_UBYTE *audio_ptr, MMP_ULONG ulFileSize);
extern MMP_ERR	MMPD_AUDIO_SetABRepeatMode(MMP_USHORT point , MMP_ULONG pos);
extern MMP_ERR	MMPD_AUDIO_GetPlayPosition(MMP_ULONG *pos);
extern MMP_ERR	MMPD_AUDIO_SetPlaySpectrumBuf(MMP_ULONG ulAddr,MMP_ULONG ulSize);
extern MMP_ERR	MMPD_AUDIO_GetPlaySpectrum(MMP_USHORT *usSpectrum);
extern MMP_ERR	MMPD_AUDIO_AACPlusPlayTime2FileOffset(MMP_ULONG ulMilliSec,MMP_ULONG *ulFileOffset);
extern MMP_ERR	MMPD_AUDIO_MP3PlayTime2FileOffset(MMP_ULONG ulMilliSec,MMP_ULONG *ulFileOffset);
extern MMP_ERR  MMPD_AUDIO_WAVPlayTime2FileOffset(MMP_ULONG ulMilliSec, MMP_ULONG *ulFileOffset);
extern MMP_ERR	MMPD_AUDIO_AMRPlayTime2FileOffset(MMP_ULONG ulMilliSec,MMP_ULONG *ulFileOffset);
extern MMP_ERR	MMPD_AUDIO_SaveAACRecParameter(MMP_ULONG ulMode);
extern MMP_ERR	MMPD_AUDIO_ReadAACRecParameter(MMP_ULONG *ulBitRate, MMP_USHORT *usSampleRate);
extern MMP_ERR	MMPD_AUDIO_SaveMP3RecParameter(MMP_ULONG ulMode);
extern MMP_ERR	MMPD_AUDIO_ReadMP3RecParameter(MMP_ULONG *ulBitRate, MMP_USHORT *usSampleRate);
extern MMP_ERR	MMPD_AUDIO_SaveAMRRecParameter(MMP_ULONG ulMode);
extern MMP_ERR	MMPD_AUDIO_ReadAMRRecParameter(MMP_ULONG *ulBitRate);
extern MMP_ERR  MMPD_AUDIO_SaveWAVRecParameter(MMP_ULONG ulMode);
extern MMP_ERR  MMPD_AUDIO_ReadWAVRecParameter(MMP_USHORT *usSampleRate);
extern MMP_ERR	MMPD_AUDIO_GetPlayStartSeekFlag(MMP_USHORT *usFlag);
extern MMP_ERR	MMPD_AUDIO_GetPlayFinishSeekFlag(MMP_USHORT *usFlag);
extern MMP_ERR	MMPD_AUDIO_SetPlayStartSeekFlag(MMP_USHORT usFlag);
extern MMP_ERR	MMPD_AUDIO_GetPlayFileSeekOffset(MMP_ULONG *ulOffset);
extern MMP_ERR	MMPD_AUDIO_UpdatePlayStreamSize(MMP_ULONG ulFileSize);
extern MMP_ERR	MMPD_AUDIO_UpdatePlayFileRemainSize(MMP_ULONG ulFileSize);
extern MMP_ERR	MMPD_AUDIO_InitPlaySeekFlag(void);
extern MMP_ERR  MMPD_AUDIO_SetAudioTagBuf(MMP_ULONG ulTagAddr,MMP_ULONG ulTagSize);
extern MMP_ERR  MMPD_AUDIO_GetWMATagLen(MMP_ULONG *Len, MMPD_AUDIO_WMA_INFO *wmaproinfo);
extern MMP_ERR  MMPD_AUDIO_GetOGGFileInfo(MMPD_AUDIO_OGG_INFO *ogginfo);
extern MMP_ERR  MMPD_AUDIO_GetOGGTagInfo(MMPD_AUDIO_OGG_TAG_INFO *TagInfo);

extern MMP_ERR  MMPD_AUDIO_SetPlayFileNameBuf(MMP_ULONG ulBufAddr);
extern MMP_ERR  MMPD_AUDIO_SetEncodeFileNameBuf(MMP_ULONG ulBufAddr);
extern MMP_ERR	MMPD_AUDIO_GetAudioPlayBufSize(MMP_ULONG *ulSize);
extern MMP_ERR	MMPD_AUDIO_SetRecReadPtr(void);
extern MMP_ERR	MMPD_AUDIO_SetPlayIntThreshold(MMP_USHORT usThreshold);
extern MMP_ERR	MMPD_AUDIO_SetRecordIntThreshold(MMP_USHORT usThreshold);
extern MMP_ERR  MMPD_AUDIO_SetGraphicEQEnable(MMP_BOOL ubEnable);
extern MMP_ERR	MMPD_AUDIO_SetGraphicEQBand(MMP_USHORT usFreq, MMP_USHORT usQrange, MMP_USHORT usGain);
extern MMP_ERR	MMPD_AUDIO_SetAECEnable(MMP_UBYTE ubEnable);
extern MMP_ERR	MMPD_AUDIO_SetDuplexPath(MMP_USHORT usPath);
extern MMP_ERR	MMPD_AUDIO_SetPlayFileSize(MMP_ULONG ulFileSize);
extern MMP_ERR	MMPD_AUDIO_SetEncodeFileSize(MMP_ULONG ulSize);
extern MMP_ERR  MMPD_AUDIO_DACChargeSpeed(MMP_BOOL bFastCharge);

MMP_ERR	MMPD_AUDIO_SetAudioOutI2SFormat(MMPD_AUDIO_I2S_FORMAT i2sformat);
MMP_ERR	MMPD_AUDIO_SetAudioInI2SFormat(MMPD_AUDIO_I2S_FORMAT i2sformat);
MMP_ERR	MMPD_AUDIO_SetEncodeFormat(MMPD_AUDIO_ENCODE_FMT encodefmt);
MMP_ERR	MMPD_AUDIO_SetPlayFormat(MMPD_AUDIO_DECODE_FMT decodefmt);
MMP_ERR	MMPD_AUDIO_SetInPath(MMP_UBYTE audiopath);
MMP_ERR	MMPD_AUDIO_SetOutPath(MMP_UBYTE audiopath);
MMP_ERR	MMPD_AUDIO_SetLineInChannel(MMPD_AUDIO_LINEIN_CHANNEL lineinchannel);
MMP_ERR MMPD_AUDIO_SetBypassPath(MMP_UBYTE bypasspath);

MMP_ERR	MMPD_AUDIO_SetPlayReadPtr(MMP_ULONG usAudioPlayReadPtr, MMP_ULONG usAudioPlayReadPtrHigh);
MMP_ERR	MMPD_AUDIO_SetPlayWritePtr(MMP_ULONG usAudioPlayWritePtr, MMP_ULONG usAudioPlayWritePtrHigh);
MMP_ERR MMPD_AUDIO_TransferDataHostToDev(void);
MMP_ERR MMPD_AUDIO_InitialMemModePointer(MMP_UBYTE *ubBufAddr, MMP_ULONG ulBufSize, MMP_ULONG ulReadSize, MMP_ULONG ulBufThreshold);
MMP_ERR MMPD_AUDIO_SetMemModeReadPointer(MMP_ULONG ulMemReadPtr, MMP_ULONG ulMemReadPtrHigh);
MMP_ERR MMPD_AUDIO_TransferDataCardToHost(void* FileHandle);
MMP_ERR MMPD_AUDIO_SetPLL(MMP_USHORT usSampleRate);
MMP_ERR MMPD_AUDIO_SetGaplessEnable(MMP_BOOL bEnable, void *nextFileCB, MMP_ULONG param1);
MMP_ERR MMPD_AUDIO_GetGaplessAllowedOp(MMP_USHORT *op);

MMP_ERR MMPD_AUDIO_FlushMp3Cache(void);
MMP_ERR MMPD_AUDIO_InitializeMp3ZIRegion(void);
MMP_ERR MMPD_AUDIO_FlushAudioCache(MMP_ULONG ulRegion);
MMP_ERR MMPD_AUDIO_InitializeAudioZIRegion(MMP_ULONG ulRegion);
MMP_ERR MMPD_AUDIO_TransferAudioDataToHost(MMP_UBYTE *ubAudioPtr, MMP_ULONG ulMaxSize, MMP_ULONG ulFrameSize,
                MMP_ULONG *ulReturnSize);
MMP_ERR	MMPD_AUDIO_SetMemModeFileHostInfo(void* FileHandle, MMP_ULONG ulAudioPlayFileSize);
/// @}
#endif // _MMPD_AUDIO_H_
/// @end_ait_only
