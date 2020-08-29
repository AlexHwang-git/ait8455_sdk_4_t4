/// @ait_only
//==============================================================================
//
//  File        : mmpd_system.h
//  Description : INCLUDE File for the Host System Control Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpd_system.h
 *  @brief The header File for the Host System Control Driver
 *  @author Penguin Torng
 *  @version 1.0
 */

#ifndef _MMPD_SYSTEM_H_
#define _MMPD_SYSTEM_H_
/** @addtogroup MMPD_System
 *  @{
 */

#include "ait_config.h"

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

typedef enum _MMPD_SYSTEM_HMODULETYPE
{
	MMPD_SYSTEM_HMODULETYPE_CPU_PERIF  = 0x0,
    MMPD_SYSTEM_HMODULETYPE_VIF  = 0x1,
    MMPD_SYSTEM_HMODULETYPE_ISP  = 0x2,
    MMPD_SYSTEM_HMODULETYPE_JPG  = 0x3,
    MMPD_SYSTEM_HMODULETYPE_SCAL  = 0x4,
    MMPD_SYSTEM_HMODULETYPE_GPIO  = 0x5,
    MMPD_SYSTEM_HMODULETYPE_AUD  = 0x6,
    MMPD_SYSTEM_HMODULETYPE_DRAM  = 0x7,
     
    MMPD_SYSTEM_HMODULETYPE_MCI  = 0x8,
    MMPD_SYSTEM_HMODULETYPE_RAW  = 0x9,
    MMPD_SYSTEM_HMODULETYPE_DMA  = 0xA,
    MMPD_SYSTEM_HMODULETYPE_I2C  = 0xB,
    MMPD_SYSTEM_HMODULETYPE_USB  = 0xC,
    MMPD_SYSTEM_HMODULETYPE_H264  = 0xD,
    MMPD_SYSTEM_HMODULETYPE_IBC  = 0xE,
    MMPD_SYSTEM_HMODULETYPE_GRA  = 0xF,
     
    MMPD_SYSTEM_HMODULETYPE_CPU_SRAM  = 0x10,
    MMPD_SYSTEM_HMODULETYPE_SD  = 0x11,
    MMPD_SYSTEM_HMODULETYPE_PWM  = 0x12,
    MMPD_SYSTEM_HMODULETYPE_PSPI  = 0x13,
    MMPD_SYSTEM_HMODULETYPE_USB_PHY = 0x14,
    MMPD_SYSTEM_HMODULETYPE_VI = MMPD_SYSTEM_HMODULETYPE_VIF //Need to be replaced by MMPD_SYSTEM_HMODULETYPE_VIF
} MMPD_SYSTEM_HMODULETYPE;

typedef enum _MMPD_SYSTEM_CLK_MODULE
{
    MMPD_SYSTEM_CLK_MODULE_CPU  = 0,
    MMPD_SYSTEM_CLK_MODULE_CPU_PHL  = 1,
    MMPD_SYSTEM_CLK_MODULE_AUD  = 2,
    MMPD_SYSTEM_CLK_MODULE_SD   = 3,
    MMPD_SYSTEM_CLK_MODULE_ICON = 4,
    MMPD_SYSTEM_CLK_MODULE_GPIO = 5,
    MMPD_SYSTEM_CLK_MODULE_GRA  = 6,
    MMPD_SYSTEM_CLK_MODULE_USB  = 8,
    MMPD_SYSTEM_CLK_MODULE_IBC  = 9,
    MMPD_SYSTEM_CLK_MODULE_I2CM = 10,
    MMPD_SYSTEM_CLK_MODULE_VID  = 11,
    MMPD_SYSTEM_CLK_MODULE_DLINE= 12,
    MMPD_SYSTEM_CLK_MODULE_SCAL = 13,
    MMPD_SYSTEM_CLK_MODULE_JPG  = 14,
    MMPD_SYSTEM_CLK_MODULE_VI   = 15,
    MMPD_SYSTEM_CLK_MODULE_ISP  = 16,
    MMPD_SYSTEM_CLK_MODULE_TV   = 17,
    MMPD_SYSTEM_CLK_MODULE_DBLK = 18,
    MMPD_SYSTEM_CLK_MODULE_BS_SPI  = 19,
    MMPD_SYSTEM_CLK_MODULE_SM   = 20,
    MMPD_SYSTEM_CLK_MODULE_MCI  = 21,
    MMPD_SYSTEM_CLK_MODULE_DRAM = 22,
    MMPD_SYSTEM_CLK_MODULE_PAD  = 23,
    MMPD_SYSTEM_CLK_MODULE_DMA  = 24,
    MMPD_SYSTEM_CLK_MODULE_SPI  = 25,
    MMPD_SYSTEM_CLK_MODULE_PCM  = 26,
    MMPD_SYSTEM_CLK_MODULE_CCIR  = 27,
    MMPD_SYSTEM_CLK_MODULE_UART  = 28,
    MMPD_SYSTEM_CLK_MODULE_HOST  = 29,
    MMPD_SYSTEM_CLK_MODULE_MS  = 30,
    MMPD_SYSTEM_CLK_MODULE_H264 = 31,
    MMPD_SYSTEM_CLK_MODULE_RAWPROC = 32,
    MMPD_SYSTEM_CLK_MODULE_APICAL = 33,
    MMPD_SYSTEM_CLK_MODULE_MP3D = 34,
    MMPD_SYSTEM_CLK_MODULE_MFD  = 35,
    MMPD_SYSTEM_CLK_MODULE_CABAC = 36,
    MMPD_SYSTEM_CLK_MODULE_BASIC = 0xFE,
    MMPD_SYSTEM_CLK_MODULE_ALL = 0xFF
} MMPD_SYSTEM_CLK_MODULE;
typedef enum _MMPD_SYSTEM_BYPASS_MODE
{
    MMPD_SYSTEM_ENTER_BYPASS = 0,
    MMPD_SYSTEM_EXIT_BYPASS
} MMPD_SYSTEM_BYPASS_MODE;

typedef enum _MMPD_SYSTEM_PS_MODE
{
    MMPD_SYSTEM_ENTER_PS = 0,
    MMPD_SYSTEM_EXIT_PS
} MMPD_SYSTEM_PS_MODE;

typedef enum _MMPD_SYSTEM_PLL_NO {
    MMPD_SYSTEM_PLL_0 = 0,
    MMPD_SYSTEM_PLL_1,
    MMPD_SYSTEM_PLL_2,
    MMPD_SYSTEM_PLL_NULL
} MMPD_SYSTEM_PLL_NO;

typedef enum _MMPD_HIF_INTERFACE {
    MMPD_HIF_16_BIT_MODE = 0,
    MMPD_HIF_8_BIT_MODE
} MMPD_HIF_INTERFACE;

typedef enum _MMPD_SYSTEM_MEMTEST
{
    MMPD_SYSTEM_MEMTEST_TCM = 0,
    MMPD_SYSTEM_MEMTEST_FB,
    MMPD_SYSTEM_MEMTEST_ALL
} MMPD_SYSTEM_MEMTEST;

typedef enum _MMPD_SYSTEM_CPUPLLSRC
{
    MMPD_SYSTEM_CPU_PLL0 = 0,
    MMPD_SYSTEM_CPU_PLL1,
    MMPD_SYSTEM_CPU_PLL2,
    MMPD_SYSTEM_CPU_PLL3,
    MMPD_SYSTEM_CPU_PLL4    
} MMPD_SYSTEM_CPUPLLSRC;
//Copy from mmpf_pll.h
typedef enum _MMPD_SYSTEM_PLL_MODE
{
#if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
	MMPD_SYSTEM_PLL_13_96G0_78G12678_54G34_X = 0,
    MMPD_SYSTEM_PLL_13_96G0_78G12678_48G3_X,
    MMPD_SYSTEM_PLL_13_96G0_156CPU_54G1234678_X,
    MMPD_SYSTEM_PLL_13_96G0_39CPU_54G1234678_X,
    MMPD_SYSTEM_PLL_13_96G012678_192CPU_12G35_54G4_X,
    MMPD_SYSTEM_PLL_13_132G012678_192CPU_12G35_54G4_X,
    MMPD_SYSTEM_PLL_13_133G012678_192CPU_12G35_54G4_X,
    MMPD_SYSTEM_PLL_13_144G012678_192CPU_12G35_54G4_X,
    MMPD_SYSTEM_PLL_13_156G012678_192CPU_12G35_54G4_X,
	MMPD_SYSTEM_PLL_13_132G012678_364CPU_12G35_54G4X,
	MMPD_SYSTEM_PLL_13_132G01278_364CPU_12G35_54G4_166G6_X,
	MMPD_SYSTEM_PLL_13_133G012678_364CPU_12G35_54G46_X,
	MMPD_SYSTEM_PLL_13_144G012678_144CPU_12G345_X,
	MMPD_SYSTEM_PLL_13_162G012678_162CPU_12G345_X,
	//for PLL3 to Audio(G5)
    MMPD_SYSTEM_PLL_AUDIO_48K,	
	MMPD_SYSTEM_PLL_AUDIO_44d1K,  	
	MMPD_SYSTEM_PLL_AUDIO_32K, 
	MMPD_SYSTEM_PLL_AUDIO_24K, 
	MMPD_SYSTEM_PLL_AUDIO_22d05K, 
	MMPD_SYSTEM_PLL_AUDIO_16K,
	MMPD_SYSTEM_PLL_AUDIO_12K,
	MMPD_SYSTEM_PLL_AUDIO_11d025K,
	MMPD_SYSTEM_PLL_AUDIO_8K,

	MMPD_SYSTEM_PLL_ExtClkOutput,
    MMPD_SYSTEM_PLL_MODE_NUMBER
#endif
} MMPD_SYSTEM_PLL_MODE;

typedef enum _MMPD_SYSTEM_PLL_HDMI_MODE
{
	MMPD_SYSTEM_PLL_HDMI_27MHZ,
	MMPD_SYSTEM_PLL_HDMI_74_25MHZ,
	MMPD_SYSTEM_PLL_HDMI_SYNC_DISPLAY
} MMPD_SYSTEM_PLL_HDMI_MODE;

/////////////////////////////////////////////////////////////////////////
//EX1:PLL_13_96d2G0_78G12_48G3_X
//        13-> pmclk=13MHz         
//           96d2G0-> PLL0 output 96.2MHz and connect with G0 
//                  78G12-> PLL1 output 78MHz and connect with G1 and G2
//                        48G3-> PLL2 output 48MHz and connect with G3
//                             X-> No serial PLL 
//    -------------------------------------------------------------------
//    pmclk(13MHz)-> PLL0 -> 96.2MHz -> G0
//    pmclk(13MHz)-> PLL1 -> 78  MHz -> G1 G2
//    pmclk(13MHz)-> PLL2 -> 48  MHz -> G3
//   
//
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//EX2:PLL_13_96G0_78G12_48G3_2L01
//        13-> pmclk=13MHz         
//           96G0-> PLL0 output 96MHz and connect with G0 
//                78G12-> PLL1 output 78MHz and connect with G1 and G2
//                      48G3-> PLL2 output 48MHz and connect with G3
//                           2L01-> Serial PLL: PLL2->PLL0 and PLL2->PLL1 
//    -------------------------------------------------------------------
//    PLL2(48MHz) -> PLL0 -> 96MHz -> G0
//    PLL2(48MHz) -> PLL1 -> 78MHz -> G1 G2
//    pmclk(13MHz)-> PLL2 -> 48MHz -> G3
//   
//
/////////////////////////////////////////////////////////////////////////

//These declaration should be sync with fw mmpf_system.h
#define COMPILER_DATE_LEN (12) /// "mmm dd yyyy"
#define COMPILER_TIME_LEN (10) /// "hh:mm:ss" 9 bytes but word alignment
/** @brief Compiler build version

This version is build by compiler.
The human modified version might not correct. Use this version to compare
the version.
*/
typedef struct MMPD_SYSTEM_BUILD_VERSION {
    MMP_UBYTE szDate[COMPILER_DATE_LEN]; /// "mmm dd yyyy"
    MMP_UBYTE szTime[COMPILER_TIME_LEN]; /// "hh:mm:ss" 9 bytes
} MMPD_SYSTEM_BUILD_VERSION;



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
MMP_ERR MMPD_System_ResetVariables(void);
MMP_ERR MMPD_System_SetHostBusMode(MMPD_HIF_INTERFACE hifmode);

MMP_ERR MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE clocktype, MMP_BOOL bEnableclock);
MMP_ERR MMPD_System_GetClock(MMP_ULONG *ulClockType);
MMP_ERR MMPD_System_ResetHModule(MMPD_SYSTEM_HMODULETYPE moduletype, MMP_BOOL bResetRegister);

MMP_ERR MMPD_System_TestRegister(void);
MMP_ERR MMPD_System_ResetCPU (const MMP_BOOL bLock);
MMP_ERR MMPD_System_SetCPUMode(MMP_ULONG ulPllSrc, MMP_BOOL bAsyncEn, MMP_ULONG ulCPUFreq, MMP_ULONG ulMCIFreq);

MMP_ERR MMPD_System_TestMemory(MMPD_SYSTEM_MEMTEST memtestmode);

MMP_ERR MMPD_System_DownloadFirmware(MMP_ULONG ulDevaddr,
						MMP_USHORT *usFwaddr, MMP_ULONG ulLength);
MMP_ERR MMPD_System_ConfigTCM (const MMP_USHORT ITCM);
MMP_ERR MMPD_System_SetPLL(MMPD_SYSTEM_PLL_MODE PLLMode);
MMP_ERR MMPD_System_GetPLL(MMPD_SYSTEM_PLL_MODE *PLLMode);
MMP_ERR MMPD_System_SetBypassMode(MMPD_SYSTEM_BYPASS_MODE bypassmode);
MMP_ERR MMPD_System_SetPSMode(MMPD_SYSTEM_PS_MODE psmode);
MMP_ERR MMPD_System_GetFWEndAddress(MMP_ULONG *ulAddress);
MMP_ERR MMPD_System_GetAudioFWStartAddress(MMP_ULONG ulRegion, MMP_ULONG *ulAddress);
MMP_ERR MMPD_System_CheckVersion(void);
MMP_ERR MMPD_System_SendEchoCommand(void);
MMP_UBYTE MMPD_System_ReadCoreID(void);
MMP_ERR MMPD_System_TuneMCIPriority(MMP_UBYTE ubMode);
MMP_ERR  MMPD_System_GetGroupFreq(MMP_UBYTE ubGroup, MMP_ULONG *ulGroupFreq);
MMP_ERR    MMPD_System_ChangeHDMIPLL(MMPD_SYSTEM_PLL_HDMI_MODE PLLMode, MMP_BOOL bEnable);
MMP_ERR  MMPD_System_MarchCTest(void);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _MMPD_SYSTEM_H_
/// @end_ait_only

