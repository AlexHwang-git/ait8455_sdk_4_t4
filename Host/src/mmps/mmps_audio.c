//==============================================================================
//
//  File        : mmps_audio.c
//  Description : Audio User Application Interface
//  Author      : Hans Liu
//  Revision    : 1.0
//
//==============================================================================
/**
 *  @file mmps_audio.c
 *  @brief The Audio playback/record control functions
 *  @author Hans Liu
 *  @version 1.0
 */

#include "mmp_lib.h"
//#include "mmpd_audiodb.h"
#include "mmps_audio.h"
#include "mmpd_audio.h"
#include "mmph_hif.h"
#include "mmpd_system.h"
#include "mmp_reg_gbl.h"
#include "ait_utility.h"
//#include "mmpd_audioparser.h"
#if (OMA_DRM_EN == 1)
#include "mmpd_omadrm.h"
#endif
/** @addtogroup MMPS_AUDIO
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

#define     SBC_SLOT_NUM            8
#if PCAM_EN==0
/**@brief The audio function card mode memory mode status.

Use @ref MMPS_AUDIO_SetMediaPath to change it state.
*/
static  MMPS_AUDIO_MEDIA_PATH   m_audioPath;

/// Audio in path
static MMPS_AUDIO_IN_PATH   m_audioInPath;
/// Audio out path
static MMPS_AUDIO_OUT_PATH  m_audioOutPath;

/// Audio Memory Allocation Information
#endif
static MMPS_AUDIOMODE_CONFIG_INFO m_AudioBufInfo;
/// Music List File Id
#if (SUPPORT_MTP_FUNC == 1)
static MMP_ULONG       m_usMusicListFileId;
/// Music List Information
static MMPS_AUDIO_MUSICLIST_INFO    m_MusicListListInfo;
///unknown string
static MMP_USHORT  m_UnknownUniStr[]={0x0008,0x0075,0x006E,0x006B,0x006E,0x006F,0x0077,0x006E,0x0000};
#endif
/**@brief The audio function memory file mode ID.

Use @ref MMPS_AUDIO_InitializeMP3Play to change it state.
*/
static	void        *m_ulAudioPlayFileHandle = 0;

static  MMP_BYTE    m_bAudioPlayFileName[AUDIO_MAX_FILE_NAME_SIZE * 2];
static  MMP_USHORT  m_usAudioPlayFileNameLength;
static  MMP_ULONG   m_ulAudioPlayFileSize;
static  MMP_UBYTE   *m_ubAudioPlayBuf;
static  MMP_ULONG   m_ulAudioPlayBufSize;
static	MMP_ULONG	m_ulAudioMemEnd;
static	MMP_ULONG	m_ulAudioMemMapMode;
static  MMP_USHORT  m_AudioPlayVolume = 0;

#if SBC_SUPPORT==1
//SBC
//static  MMP_ULONG   m_ulSBCBitRate;
static  MMP_UBYTE   *m_ucSBCBufStart;
static  MMP_UBYTE   *m_ucSBCBufRead, *m_ucSBCBufWrite;
static  MMP_ULONG   m_ulSBCBufSize;
static  MMP_ULONG   m_ulSBCSamplingRate;
static  MMP_ULONG   m_ulSBCFrameLen;
static  MMP_ULONG   m_ulSBCSamples;
static  MMP_ULONG   m_ulSBCChannels;
static  MMP_ULONG   m_ulSBCSizeSlot[SBC_SLOT_NUM];
static  MMP_ULONG   m_ulSBCSizeReadIdx;
static  MMP_ULONG   m_ulSBCSizeWriteIdx;
static  MMP_ULONG   m_ulSBCSizeNum;
static  MMP_ULONG   m_ulSBCSlotSize;
static  MMP_ULONG   m_ulSBCConsumedData;
static  MMP_ULONG   m_ulSBCStartTime;
static  MMP_ULONG   m_ulSBCMaxTransSize;
#endif
//Wave
#define     WAV_SLOT_NUM            8
#define     MAX_WAV_OUTPUT_SIZE     10 * 1024
#define     MAX_WAV_TRANSFER_SIZE   10 * 1024
#if (WAV_ENC_SUPPORT==1)
static  MMP_UBYTE   *m_ucWAVBufStart;
static  MMP_UBYTE   *m_ucWAVBufRead, *m_ucWAVBufWrite;
static  MMP_ULONG   m_ulWAVBufStart;
static  MMP_ULONG   m_ulWAVBufSize;
static  MMP_ULONG	m_ulWAVSampleCount;
static  MMP_ULONG   m_ulWAVSizeSlot[WAV_SLOT_NUM];
static  MMP_ULONG   m_ulWAVSizeReadIdx;
static  MMP_ULONG   m_ulWAVSizeWriteIdx;
static  MMP_ULONG   m_ulWAVSizeNum;
static  MMP_ULONG   m_ulWAVSlotSize;
static  MMP_ULONG	m_ulWAVSampleRate;
static  MMP_ULONG	m_ulWAVChans;
static	MMP_ULONG	m_ulWAVConsumedData;
static	MMP_ULONG	m_ulWAVStartTime;
static  MMP_LONG    m_lMP3TagSize;
#endif    
#if (AUDIO_HOST_DB_EN == 1)    
//++ for the database
static	MMP_UBYTE	m_ubDBFileName[50];
static	MMP_ULONG	m_ulDBFileNameSize;
static  MMPS_AUDIO_DB_SORT_TYPE	m_sortType = MMPS_AUDIO_DB_SORT_MAX;   
static	MMPD_AUDIODB_BTNODE *m_treeRoot;
static	MMPD_AUDIODB_BFNODE *m_forestRoot;
static  MMP_ULONG	m_ulTreeNodeCount, m_ulForestNodeCount; 
static	MMPS_AUDIO_DB_TRACK* m_trackEntry;
static  MMPS_AUDIO_DB_ENTRY* m_dbEntry;

static	MMP_UBYTE	m_ubFilterString[AUDIODB_MAX_FILE_NAME_SIZE*2];
static	MMP_ULONG	m_ulFilterStringLength;

static	MMP_ULONG   m_ulFilterStart, m_ulFilterEnd;
static	MMP_ULONG	m_ulFilterCurrentCount;

static MMP_ULONG    m_ulOpenedDirID[10];
static MMP_ULONG	m_ulDepth;
static MMP_BYTE     m_bLastDirName[AUDIO_MAX_FILE_NAME_SIZE*3];

static MMP_BOOL		m_bDatabaseDirty = MMP_FALSE;
//-- for the database
#endif
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
#if AUDIO_P_EN==1
MMPS_AUDIO_CODEC            m_AudioCodec;
MMPS_AUDIO_MP3_INFO         m_mp3info;
MMPS_AUDIO_AMR_INFO         m_amrinfo;
MMPS_AUDIO_AACPLUS_INFO     m_aacplusinfo;
MMPS_AUDIO_WMA_INFO         m_wmainfo;
MMPS_AUDIO_OGG_INFO         m_ogginfo;
MMPS_AUDIO_RA_INFO          m_rainfo;
#endif
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
//------------------------------------------------------------------------------
//  Function    : MMPS_AUDIO_GetConfiguration
//  Description :
//------------------------------------------------------------------------------
/**
    @brief  Get Audio Configuration Structure
    @return m_AudioBufInfo
*/
MMPS_AUDIOMODE_CONFIG_INFO*  MMPS_AUDIO_GetConfiguration(void)
{
    return &m_AudioBufInfo;
}

#if 0

MMP_ERR MMPS_AUDIO_DACChargeSpeed(MMP_BOOL bFastCharge)
{
    //return MMP_ERR_NONE;
    return MMPD_AUDIO_DACChargeSpeed(bFastCharge);
}
#endif


//------------------------------------------------------------------------------
//  Function    : MMPS_AUDIO_InitialInPath
//  Description :
//------------------------------------------------------------------------------
/**
    @brief  Initialize Audio In Path
    @param[in] audioinpath specific audio input path
    @return MMP_ERR_NONE
*/
#if PCAM_EN==0
MMP_ERR MMPS_AUDIO_InitialInPath(MMPS_AUDIO_IN_PATH audioinpath)
{
    m_audioInPath = audioinpath;
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_AUDIO_SetLineInChannel
//  Description :
//------------------------------------------------------------------------------
/**
    @brief  Set Line In Channel
    @param[in] lineinchannel specific linein channel (Dual/R/L)
        -   MMPS_AUDIO_LINEIN_DUAL  : LIP/RIP
        -   MMPS_AUDIO_LINEIN_R     : RIP
        -   MMPS_AUDIO_LINEIN_L     : LIP
    @return MMP_ERR_NONE, MMP_SYSTEM_ERR_CMDTIMEOUT
*/
MMP_ERR MMPS_AUDIO_SetLineInChannel(MMPS_AUDIO_LINEIN_CHANNEL lineinchannel)
{
    return MMPD_AUDIO_SetLineInChannel((MMPD_AUDIO_LINEIN_CHANNEL)lineinchannel);
}
//------------------------------------------------------------------------------
//  Function    : MMPS_AUDIO_InitialOutPath
//  Description :
//------------------------------------------------------------------------------
/**
    @brief  Initialize Audio Out Path
    @param[in] audiooutpath specific audio output path
    @return MMP_ERR_NONE
*/
MMP_ERR MMPS_AUDIO_InitialOutPath(MMPS_AUDIO_OUT_PATH audiooutpath)
{
    m_audioOutPath = audiooutpath;
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_AUDIO_GetInPath
//  Description :
//------------------------------------------------------------------------------
/**
    @brief  Get Audio In Path
    @return audio in path
        - MMPS_AUDIO_IN_I2S
        - MMPS_AUDIO_IN_AFE_SING        (AUXL/AUXR)
        - MMPS_AUDIO_IN_AFE_DIFF        (LIP/LIN/RIP/RIN)
        - MMPS_AUDIO_IN_AFE_DIFF2SING   (LIP/RIP)
    @pre Audio in path has beed set either by @ref MMPS_AUDIO_InitialInPath or 
         @ref MMPS_AUDIO_SetInPath
*/
MMPS_AUDIO_IN_PATH  MMPS_AUDIO_GetInPath(void)
{
    return m_audioInPath;
}
//------------------------------------------------------------------------------
//  Function    : MMPS_AUDIO_GetOutPath
//  Description :
//------------------------------------------------------------------------------
/**
    @brief  Get Audio Out Path
    @return audio out path
        - MMPS_AUDIO_OUT_I2S
        - MMPS_AUDIO_OUT_AFE_HP         (LOUT/ROUT)
        - MMPS_AUDIO_OUT_AFE_HP_INVT    (LOUT/ROUT)
        - MMPS_AUDIO_OUT_AFE_LINE       (LOUT2/ROUT2)
        - MMPS_AUDIO_OUT_AFE_LINE_INVT  (LOUT2/ROUT2)
    @pre Audio out path has beed set either by @ref MMPS_AUDIO_InitialOutPath or 
         @ref MMPS_AUDIO_SetOutPath
*/
MMPS_AUDIO_OUT_PATH  MMPS_AUDIO_GetOutPath(void)
{
    return m_audioOutPath;
}
#endif


/// @}
