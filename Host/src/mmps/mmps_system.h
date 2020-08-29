/**
 *  @file mmps_system.h
 *  @brief Header file the host system API
 *  @author Jerry Tsao, Truman Yang
 *  @version 1.1
 */
#ifndef _MMPS_SYSTEM_H_
#define _MMPS_SYSTEM_H_

#include "mmp_lib.h"
#include "ait_bsp.h"


/** @addtogroup MMPS_System
@{
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


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPS_SYSTEM_OPMODE
{
    MMPS_SYSTEM_OPMODE_BYPASS = 0,		/**< System Bypass mode, LCD bus is bypassed and controlled by B/B, 
    										Firmware is active. */
    MMPS_SYSTEM_OPMODE_ACTIVE,			///< System active mode, LCD bus is controlled by AIT chip, Firmware is active*/
    MMPS_SYSTEM_OPMODE_SUSPEND,			/**< System suspend mode, LCD bus is bypassed and controlled by B/B,
    										Firmware is sleeping. */
    MMPS_SYSTEM_OPMODE_PWRDN			/**< System power down. LCD bus is bypassed and controlled by B/B,
    										Firmware is released. */
} MMPS_SYSTEM_OPMODE;


typedef enum _MMPS_SYSTEM_APMODE
{
    MMPS_SYSTEM_APMODE_NULL = 0, /**<Null parameter. It's used to reset the FirmwareHandler state
                                     machine to download the firmware again.*/
    MMPS_SYSTEM_APMODE_DSC = 1,	 	///< DSC
    MMPS_SYSTEM_APMODE_MP3_PLAY, 		///< MP3 player
    MMPS_SYSTEM_APMODE_VIDEO_AMR_RECORD, ///< AMR video recorder
    MMPS_SYSTEM_APMODE_VIDEO_AAC_RECORD, ///< AAC video recorder
    MMPS_SYSTEM_APMODE_VIDEO_AMR_PLAY, ///< Hardware video with AMR video player
    MMPS_SYSTEM_APMODE_VIDEO_AAC_PLAY, ///< Hardware video with AAC video player
    MMPS_SYSTEM_APMODE_H264_AMR_PLAY, ///< Software H.264 with AMR video player
    MMPS_SYSTEM_APMODE_H264_AAC_PLAY, ///< Software H.264 with AAC video player
    MMPS_SYSTEM_APMODE_VIDEO_AACPLUS_PLAY, ///< Not supported yet
    MMPS_SYSTEM_APMODE_VIDEO_BSAC_PLAY, ///< Not supported yet
    MMPS_SYSTEM_APMODE_AMR, ///< AMR player and recoder without video
    MMPS_SYSTEM_APMODE_AAC_RECORD, ///< AAC player and recoder without video
    MMPS_SYSTEM_APMODE_MP3_RECORD, ///< MP3 recoder without video
    MMPS_SYSTEM_APMODE_MIDI_PLAY, ///< Midi player
    MMPS_SYSTEM_APMODE_AACPLUS_PLAY, ///< AAC+ player
    MMPS_SYSTEM_APMODE_WMA_PLAY, ///< WMA player
    MMPS_SYSTEM_APMODE_OGG_PLAY, ///< OGG Vorbis player
    MMPS_SYSTEM_APMODE_RA_PLAY, ///< RA Vorbis player
    MMPS_SYSTEM_APMODE_WAV_PLAY, ///< WAV player
    MMPS_SYSTEM_APMODE_USB, ///< USB storage
    MMPS_SYSTEM_APMODE_MDTV, ///< Mobile Digital TV
    MMPS_SYSTEM_APMODE_TEST, ///< TEST firmware
    MMPS_SYSTEM_APMODE_ERASE, ///< Nand Flash Initialization
    MMPS_SYSTEM_APMODE_PLL, ///<for change PLL
    MMPS_SYSTEM_APMODE_AVI_MP3_PLAY, ///< Hardware video with MP3 avi video player
    MMPS_SYSTEM_APMODE_MAX /**< It could be used to debug different versions of system mode definition.*/
} MMPS_SYSTEM_APMODE;

typedef enum _MMPS_HIF_INTERFACE {
    MMPS_HIF_16_BIT_MODE = 0,		///<B/B and MMP Interface 16 bit mode
    MMPS_HIF_8_BIT_MODE				///<B/B and MMP Interface 8 bit mode
} MMPS_HIF_INTERFACE;


typedef enum _MMPS_DRAM_TYPE
{
    MMPS_DRAM_TYPE_NONE = 0,	// no stack memory
    MMPS_DRAM_TYPE_1,			// first used
    MMPS_DRAM_TYPE_2,			// second used
    MMPS_DRAM_TYPE_3,			// third used
    MMPS_DRAM_TYPE_EXT,
    MMPS_DRAM_TYPE_AUTO
} MMPS_DRAM_TYPE;

typedef enum _MMPS_DRAM_MODE
{
    MMPS_DRAM_MODE_SDRAM = 0,	// SD RAM
    MMPS_DRAM_MODE_DDR,			// DDR RAM
    MMPS_DRAM_MODE_DDR2
} MMPS_DRAM_MODE;

typedef enum _MMPS_SYSTEM_FWTYPE
{
    MMPS_FWTYPE_SINGLE = 0,		// standalone fw type
    MMPS_FWTYPE_ALL,			// all-in-one
    MMPS_FWTYPE_CVMO,			// all-overwrite (Capture, video, MP3)
    MMPS_FWTYPE_CO				// capture-overwrite
} MMPS_SYSTEM_FWTYPE;

typedef struct __MMPS_SYSTEM_SRAM_MAP {
    MMP_ULONG ulMMUTblAddr,ulMMUTblSize ;
    MMP_ULONG ulJPGLineBufAddr,ulJPGLineBufSize;
    MMP_ULONG ulJPGLineBufAddrOver1080p,ulJPGLineBufSizeOver1080p;
    
    //MMP_ULONG ulCodeAddr,ulCodeSize ;
    MMP_ULONG ulH264Addr,ulH264Size ;
#if ISP_BUF_IN_SRAM
    MMP_ULONG ulISPDmaOprAddr,ulISPDmaOprSize ;
#endif    
    MMP_ULONG ulSramEndAddr;
} MMPS_SYSTEM_SRAM_MAP;

/** @brief Type of by pass pin controller call back function.

The responsibility of this function should set the bypass ping according to the input
@param[in] value Value 1 for high, value 0 for low.
@return If the initalize successful
*/
typedef struct _MMPS_SYSTEM_CONFIG {
    MMPS_SYSTEM_APMODE  apMode;				///<System application mode
    MMPS_DRAM_TYPE		stackMemoryType;	///<Stack memory type inside AIT chip
    MMPS_DRAM_MODE      stackMemoryMode;    // DDR or SDRAM
	MMP_ULONG           ulStackMemoryStart;	///<Stack memory start address
	MMP_ULONG           ulStackMemorySize;	///<Stack memory size
    MMPS_HIF_INTERFACE	hostBusModeSelect;	///<Specify data connection bus width between B/B and AIT chipStack memory type inside AIT chip
    MMPS_SYSTEM_FWTYPE	firmwareType;
    MMP_ULONG           ulMaxReserveBufferSize; ///< Reserve Buffer Size for Audio Operation
    MMP_ULONG           ulMaxReserveChannel; ///< Reserve Buffer Size for Audio Operation
    MMP_ULONG           ulCpuFreq;
    MMP_ULONG           ulG012Freq;
    #if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
    MMP_ULONG           ulG6Freq;           // ASYNC DRAM CLOCK
    #endif
    
    MMPS_SYSTEM_SRAM_MAP sramMap ;
} MMPS_SYSTEM_CONFIG;

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
MMPS_SYSTEM_CONFIG* MMPS_System_GetConfiguration(void);
MMPS_SYSTEM_SRAM_MAP *MMPS_System_GetSramMap(void);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}

#endif // _MMPS_SYSTEM_H_
