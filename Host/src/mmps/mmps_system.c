//==============================================================================
//
//  File        : mmps_system.c
//  Description : Ritian System Control function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
@file mmps_system.c
@brief The System Control functions
@author Penguin Torng, Truman Yang
@version 1.0
*/

#ifdef BUILD_CE
#include "config_fw.h"
#undef BUILD_FW
#endif

#include "mmp_lib.h"
#include "mmps_system.h"
#include "mmpd_system.h"
//#include "mmpd_dram.h"
#include "mmph_hif.h"
#include "mmp_reg_gbl.h"
#include "mmpd_audio.h"
#include "ait_utility.h"
/** @addtogroup MMPS_System
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
/**@brief The system configuration

Use @ref MMPS_System_GetConfiguration to assign the field value of it.
You should read this functions for more information.
*/
static MMPS_SYSTEM_CONFIG	m_systemConfig;
/**@brief The system operation mode.
         
Use @ref MMPS_System_SetOperationMode to set it.
And use @ref MMPS_System_GetOperationMode to get the current mode.
You should read these functions for more information.
*/
static MMPS_SYSTEM_OPMODE	m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;
/**@brief The system application mode.
         
Use @ref MMPS_System_SetApplicationMode to set it.
And use @ref MMPS_System_GetApplicationMode to get the current mode.
You should read these functions for more information.
*/
static MMPS_SYSTEM_APMODE	m_systemApMode = MMPS_SYSTEM_APMODE_NULL;
/**@brief The system application mode before power down
         
Use @ref MMPS_System_SetOperationMode to set it.
*/
static MMPS_SYSTEM_APMODE	m_systemApModePrePowerDown = MMPS_SYSTEM_APMODE_NULL;

/**@brief The system ram is occupied by some hardware application or not
         
Use @ref MMPS_System_SetRamOverwrite /@ref MMPS_System_SetAudioOverwrite and @ref MMPS_System_SetApplicationMode to set it.
*/
static MMP_BOOL	m_bRamOverwrite = MMP_FALSE;
static MMP_BOOL	m_bAudio1Overwrite = MMP_FALSE;
static MMP_BOOL	m_bAudio2Overwrite = MMP_FALSE;

/**@brief Keep the audio frame start address, and the preview frame start address
         
Use @ref MMPS_System_SetAudioFrameStart to set it, and use 
@ref MMPS_System_GetAudioFrameStart to retrieve it
*/
static MMP_ULONG	m_ulAudioFrameStart;
static MMP_ULONG	m_ulPreviewFrameStart;

/**@brief Keep the audio code start address
         
Use @ref MMPS_System_SetAudioOverwrite to retrieve it
*/

static	MMP_ULONG	m_ulAudio1StartAddress;
static	MMP_ULONG	m_ulAudio2StartAddress;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPS_System_ResetVariables
//  Description :
//------------------------------------------------------------------------------
/** @brief The function will reset system related file-scope variables

@return The result of the default reset.
*/
#if PCAM_EN==0
static MMP_ERR  MMPS_System_ResetVariables()
{
    m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;
    m_systemApMode = MMPS_SYSTEM_APMODE_NULL;
    m_systemApModePrePowerDown = MMPS_SYSTEM_APMODE_NULL;
    m_bRamOverwrite = MMP_FALSE;
    m_bAudio1Overwrite = MMP_FALSE;
    m_bAudio2Overwrite = MMP_FALSE;
    MMPD_System_ResetVariables();
    return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetConfiguration
//  Description :
//------------------------------------------------------------------------------
/** @brief The function gets the current system configuration for the host application

The function gets the current system configuration for reference by the host application. The current
configuration can be accessed from output parameter pointer. The function calls
MMPD_System_GetConfiguration to get the current settings from Host Device Driver Interface.

@return It return the pointer of the system configuration data structure.
*/
MMPS_SYSTEM_CONFIG* MMPS_System_GetConfiguration(void)
{
    return &m_systemConfig;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_System_InitializeModules
//  Description :
//------------------------------------------------------------------------------
/** @brief This function initializes the modules in the system

The function initializes the modules in the system.
Start up process:\n
-# Call MMPC_System_InitializeMMP Initialize system interface on host (bus, chip reset, chip bypass, etc).
-# Call this function to
    -Call MMPD_System_InitializeModules to setup host interface other register-set dependent things.
     Such as Host interface type and e-fuse. Refer MMPD_System_InitializeModules for more information.
    -Run the test of internal memory by input parameter.
    -Reset the clocks for all of the blocks in the system.
    -Initialize LCD controller and may also initalize the panel. This is defined in the utility callback functions.
-# Call MMPS_System_SetApplicationMode to download a firmware and start an AIT function
-# (Optional)Initialize the host side file system so that you could get the file list.
@pre Normally the function can be called after a successful configuration setup.
    Use MMPS_System_GetConfiguration to get the configuration pointer to set it up.
@pre AIT should be initlized. Maybe by MMPC_System_InitializeMMP.
@pre This function should always in active mode and 
      the customer platform might initialize the LCD before AIT.
@post AIT could download the firmwares.
@return It reports the status of the operation.
@retval MMPS_SYSTEM_SUCCESS success
The function completes the following tasks:
*/
/*
MMP_ERR  MMPS_System_InitializeModules(void)
{
    MMP_ERR 	retstatus;
	MMP_UBYTE	coreID;

    MMPS_System_ResetVariables();
	MMPC_System_InitConfig();

    // Force to set active mode
    MMPC_System_SetBypassPin(MMP_FALSE);
    m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;

    ///Set host interface to 16 or 8 bits mode.
    switch (m_systemConfig.hostBusModeSelect) {
    case MMPS_HIF_16_BIT_MODE:
	  	MMPD_System_SetHostBusMode(MMPD_HIF_16_BIT_MODE);
	  	break;
    case MMPS_HIF_8_BIT_MODE:
	  	MMPD_System_SetHostBusMode(MMPD_HIF_8_BIT_MODE);
	  	break;
    }

	// check the code ID
	coreID = MMPD_System_ReadCoreID();
	PRINTF("coreID = 0x%X\r\n", coreID);

	// enable all clock
	#if (AUTO_CLK_GATING_EN == 1)
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_BASIC, MMP_TRUE);
	#else		
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ALL, MMP_TRUE);
	#endif


	// ++ P_V1 Only
	#if (CHIP == PYTHON)
	if (coreID == 0x0) {
		m_systemConfig.ulCpuFreq = 162;
		m_systemConfig.ulG012Freq = 162;
		m_systemConfig.ulG6Freq = 162;	
		m_systemConfig.stackMemoryMode = MMPS_DRAM_MODE_SDRAM;
	}
	else if (coreID == 0x31) {
		m_systemConfig.stackMemoryMode = MMPS_DRAM_MODE_DDR;
	}
	// ++ P_65C
	else if (coreID == 0x35) {
		m_systemConfig.stackMemoryMode = MMPS_DRAM_MODE_DDR;
	}
	// -- P_65C
	else {
		PRINTF("Please check device ID\r\n");
	}
	#endif
	// -- P_V1 Only
    // Reset the application mode to download again
    retstatus = MMPS_System_SetApplicationMode(MMPS_SYSTEM_APMODE_PLL);

    if (retstatus != MMP_ERR_NONE) {
        return retstatus;
    }
	switch (m_systemConfig.stackMemoryType) {
	case MMPS_DRAM_TYPE_1:
		#if (CHIP == D_V1)
		retstatus = MMPD_DRAM_Initialize(MMPD_DRAM_TYPE_1, &(m_systemConfig.ulStackMemorySize), m_systemConfig.ulG012Freq, m_systemConfig.stackMemoryMode);
		#endif
		break;			
	case MMPS_DRAM_TYPE_2:
		#if (CHIP == D_V1)
		retstatus = MMPD_DRAM_Initialize(MMPD_DRAM_TYPE_2, &(m_systemConfig.ulStackMemorySize), m_systemConfig.ulG012Freq, m_systemConfig.stackMemoryMode);
		#endif
		break;			
	case MMPS_DRAM_TYPE_3:
		#if (CHIP == D_V1)
		retstatus = MMPD_DRAM_Initialize(MMPD_DRAM_TYPE_3, &(m_systemConfig.ulStackMemorySize), m_systemConfig.ulG012Freq, m_systemConfig.stackMemoryMode);
		#endif
		break;
	case MMPS_DRAM_TYPE_AUTO:
		#if (CHIP == D_V1)
		retstatus = MMPD_DRAM_Initialize(MMPD_DRAM_TYPE_AUTO, &(m_systemConfig.ulStackMemorySize), 0, m_systemConfig.stackMemoryMode);
		#endif
		#if (CHIP == PYTHON)||(CHIP == P_V2)
		retstatus = MMPD_DRAM_Initialize(MMPD_DRAM_TYPE_AUTO, &(m_systemConfig.ulStackMemorySize), m_systemConfig.ulG6Freq, m_systemConfig.stackMemoryMode);
		#endif
		break;
	case MMPS_DRAM_TYPE_NONE:
	    m_systemConfig.ulStackMemorySize = 0;
	    break;
	}
	if (retstatus != MMP_ERR_NONE)
		return retstatus;

    // Initial System Module Configuration
    MMPC_Sensor_InitConfig();
    MMPC_Display_InitConfig();
    MMPC_DSC_InitConfig();
    MMPC_3GPRECD_InitConfig();
    //MMPC_3GPPLAY_InitConfig();
    MMPC_USB_InitConfig();
    MMPC_AUDIO_InitConfig();
    MMPC_VIDPHONE_InitConfig();

    return  MMP_ERR_NONE;
}
*/
//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetApplicationMode
//  Description :
//------------------------------------------------------------------------------
/**
@brief The function will control operation mode, switch firmware and set PLL

@param [in] apmode The Application mode. Use this value to download different firmware. When change this value, this
            function load different firmware. When use MMPS_SYSTEM_MODE_MAX it reset the firmware counter so that it will
            restart the firmware downloading next time.
            Use NULL to use the current MMPS_SYSTEM_CONFIG setting.
@return The result of the system function call.
@retval MMP_SYSTEM_ERR_SETAPMODE When the system is in bypass mode or download firmware fail
@retval MMP_ERR_NONE for success.
*/
/*
MMP_ERR  MMPS_System_SetApplicationMode(MMPS_SYSTEM_APMODE apmode)
{
    MMP_ERR		retstatus;
    MMP_ULONG	hostAddress, fwSize;
    MMP_BOOL	redownload = 1;
    MMP_UBYTE 	*pFirmwareAddress;
    MMP_UBYTE 	*curAddr;
    MMP_UBYTE	nFirmware, i;
	MMP_ULONG 	ulFirmwareSize, ulTargetAddress;

    
    //to Active Operation Mode
    if (m_systemOpMode == MMPS_SYSTEM_OPMODE_PWRDN) {
        // by pass set low
		MMPC_System_SetBypassPin(MMP_FALSE);
    	MMPD_DRAM_SetPowerDown(MMP_FALSE);
    	m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;
    }
    if (m_systemOpMode == MMPS_SYSTEM_OPMODE_SUSPEND) {
        MMPS_System_SetOperationMode(MMPS_SYSTEM_OPMODE_ACTIVE);
    }
    else if (m_systemOpMode == MMPS_SYSTEM_OPMODE_BYPASS) {
    	PRINTF("System OpMode Change Violation\r\n");
        return  MMP_SYSTEM_ERR_SETAPMODE;
    }
    
    if (apmode == MMPS_SYSTEM_APMODE_NULL || apmode == MMPS_SYSTEM_APMODE_MAX) {
        m_systemApMode = apmode;
    	return MMP_ERR_NONE;
	}

    if (m_systemApMode == apmode) {
        return MMP_ERR_NONE;
    }

	MMPC_System_CheckFirmwareReload(m_systemApMode, apmode, &redownload);
	
	if (!redownload) {
		if (apmode == MMPS_SYSTEM_APMODE_MP3_PLAY && 
			((m_bRamOverwrite == MMP_TRUE) || (m_bAudio1Overwrite == MMP_TRUE))) {
		    retstatus = MMPC_System_LoadFirmware(apmode, &hostAddress, &fwSize);
   			if (MMP_ERR_NONE != retstatus) {
	        	return retstatus;
		    }
		    
		    pFirmwareAddress = (MMP_UBYTE*)hostAddress;
		    curAddr = pFirmwareAddress + 8; // number of firmware

		    nFirmware = *curAddr;
		    curAddr += 2;
		    for (i = 0; i < nFirmware; i++) {
		        ///Portability: Use memcpy to get info directly from the file.
		        MEMCPY (&ulTargetAddress, curAddr, 4);
		        curAddr += 4;

		        MEMCPY (&ulFirmwareSize, curAddr, 4);
		        curAddr += 4; // begin of this firmware binary

				if (m_bRamOverwrite == MMP_TRUE) {
					if (ulTargetAddress == 0x100000) {
				        retstatus = MMPD_System_DownloadFirmware(ulTargetAddress, (MMP_USHORT *)curAddr, ulFirmwareSize);
					}					        
				}
				if (m_bAudio1Overwrite == MMP_TRUE) {					        
					if (ulTargetAddress == m_ulAudio1StartAddress) {
				        retstatus = MMPD_System_DownloadFirmware(ulTargetAddress, (MMP_USHORT *)curAddr, ulFirmwareSize);
					}					        
				}
				curAddr += ulFirmwareSize; // to next firmware start
		        if (MMP_ERR_NONE != retstatus) {
       				return retstatus;
				}
		    }
		    
		    if (m_bRamOverwrite == MMP_TRUE) {
			    MMPD_AUDIO_InitializeMp3ZIRegion();
			    m_bRamOverwrite = MMP_FALSE;
			}				    
		    if (m_bAudio1Overwrite == MMP_TRUE) {
			    MMPD_AUDIO_InitializeAudioZIRegion(1);
			    m_bAudio1Overwrite = MMP_FALSE;
			}				    
		}			

		if (MMPS_System_GetConfiguration()->firmwareType == MMPS_FWTYPE_CO) {
			if ((apmode == MMPS_SYSTEM_APMODE_VIDEO_AMR_RECORD ||
					apmode == MMPS_SYSTEM_APMODE_VIDEO_AAC_RECORD ||
					apmode == MMPS_SYSTEM_APMODE_VIDEO_AMR_PLAY ||
					apmode == MMPS_SYSTEM_APMODE_VIDEO_AAC_PLAY ||
					apmode == MMPS_SYSTEM_APMODE_AMR ||
					apmode == MMPS_SYSTEM_APMODE_AAC_RECORD ||
					apmode == MMPS_SYSTEM_APMODE_AACPLUS_PLAY) && 
					m_bAudio1Overwrite == MMP_TRUE) {
				retstatus = MMPC_System_LoadFirmware(apmode, &hostAddress, &fwSize);
    			if (MMP_ERR_NONE != retstatus) {
		        	return retstatus;
			    }

			    pFirmwareAddress = (MMP_UBYTE*)hostAddress;
			    curAddr = pFirmwareAddress + 8; // number of firmware

			    nFirmware = *curAddr;
			    curAddr += 2;
			    for (i = 0; i < nFirmware; i++) {
			        ///Portability: Use memcpy to get info directly from the file.
			        MEMCPY (&ulTargetAddress, curAddr, 4);
			        curAddr += 4;

			        MEMCPY (&ulFirmwareSize, curAddr, 4);
			        curAddr += 4; // begin of this firmware binary

					if (ulTargetAddress == m_ulAudio1StartAddress) {
				        retstatus = MMPD_System_DownloadFirmware(ulTargetAddress, (MMP_USHORT *)curAddr, ulFirmwareSize);
					}				        
					curAddr += ulFirmwareSize; // to next firmware start
			        if (MMP_ERR_NONE != retstatus) {
           				return retstatus;
					}
			    }
			    
			    MMPD_AUDIO_InitializeAudioZIRegion(1);
			    
			    m_bAudio1Overwrite = MMP_FALSE;
			}			


			if ((apmode == MMPS_SYSTEM_APMODE_WMA_PLAY ||
				apmode == MMPS_SYSTEM_APMODE_OGG_PLAY ||
				apmode == MMPS_SYSTEM_APMODE_RA_PLAY ||
				apmode == MMPS_SYSTEM_APMODE_WAV_PLAY ||
				apmode == MMPS_SYSTEM_APMODE_MP3_RECORD) && 
				((m_bAudio1Overwrite == MMP_TRUE) || (m_bAudio2Overwrite == MMP_TRUE))) {
				retstatus = MMPC_System_LoadFirmware(apmode, &hostAddress, &fwSize);
    			if (MMP_ERR_NONE != retstatus) {
		        	return retstatus;
			    }

			    pFirmwareAddress = (MMP_UBYTE*)hostAddress;
			    curAddr = pFirmwareAddress + 8; // number of firmware

			    nFirmware = *curAddr;
			    curAddr += 2;
			    for (i = 0; i < nFirmware; i++) {
			        ///Portability: Use memcpy to get info directly from the file.
			        MEMCPY (&ulTargetAddress, curAddr, 4);
			        curAddr += 4;

			        MEMCPY (&ulFirmwareSize, curAddr, 4);
			        curAddr += 4; // begin of this firmware binary

					if (m_bAudio1Overwrite == MMP_TRUE) {
						if (ulTargetAddress == m_ulAudio1StartAddress) {
					        retstatus = MMPD_System_DownloadFirmware(ulTargetAddress, (MMP_USHORT *)curAddr, ulFirmwareSize);
						}					        
					}				        
					if (m_bAudio2Overwrite == MMP_TRUE) {
						if (ulTargetAddress == m_ulAudio2StartAddress) {
					        retstatus = MMPD_System_DownloadFirmware(ulTargetAddress, (MMP_USHORT *)curAddr, ulFirmwareSize);
						}					        
					}				        
					
					curAddr += ulFirmwareSize; // to next firmware start
			        if (MMP_ERR_NONE != retstatus) {
           				return retstatus;
					}
			    }
			    
			    if (m_bAudio1Overwrite == MMP_TRUE) {
				    MMPD_AUDIO_InitializeAudioZIRegion(1);
				    m_bAudio1Overwrite = MMP_FALSE;
				}				    
			    if (m_bAudio2Overwrite == MMP_TRUE) {
				    MMPD_AUDIO_InitializeAudioZIRegion(2);
				    m_bAudio2Overwrite = MMP_FALSE;
				}				    
			}			
		}	
	}
    
	if (redownload) {
		m_bRamOverwrite = MMP_FALSE;
		m_bAudio1Overwrite = MMP_FALSE;
		m_bAudio2Overwrite = MMP_FALSE;
	
		MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL0, MMP_FALSE, 
								MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
								MMPS_System_GetConfiguration()->ulG012Freq * 1000);
	
		MMPH_HIF_RegSetB(GBL_ARM_PWR_DOWN, (MMPH_HIF_RegGetB(GBL_ARM_PWR_DOWN) & ~GBL_CPU_WFI_CLKOFF_EN)); 
		MMPS_System_SendEchoCommand();
    
	    ///- Reset CPU, don't reset register
    	MMPD_System_ResetCPU(MMP_TRUE); // alway return success

    	///- Use call back funtion to get load the firmware into the memory and get the size and address
	    retstatus = MMPC_System_LoadFirmware(apmode, &hostAddress, &fwSize);
    	if (MMP_ERR_NONE != retstatus) {
        	return retstatus;
	    }

    	retstatus = MMPS_System_DownloadFirmware(hostAddress);
    	if (MMP_ERR_NONE == retstatus) {
        	//CPU free run
	        retstatus = MMPD_System_ResetCPU(MMP_FALSE);
    	    if (MMP_ERR_NONE != retstatus) {
	    	    PRINTF("Error on switching firmware.\r\nThe system might hang in any firmware command.\r\n");
	        	return	retstatus;
			}
		}		
	}		
	m_systemApMode = apmode;
	MMPS_System_SendEchoCommand();

	if (redownload) {
	    //Set PLL
    	switch (apmode) {
	    case MMPS_SYSTEM_APMODE_VIDEO_AMR_PLAY:
    	case MMPS_SYSTEM_APMODE_VIDEO_AAC_PLAY:
    	case MMPS_SYSTEM_APMODE_H264_AMR_PLAY:
	    case MMPS_SYSTEM_APMODE_H264_AAC_PLAY:
	    case MMPS_SYSTEM_APMODE_AVI_MP3_PLAY:
			#if	(CHIP == D_V1)
			if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
		        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_192CPU_12G34_X);
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012_192CPU_12G34_X);
		    }
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 120) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_120G012_192CPU_12G34_X);
		    }		    
		    else
		    {
		    	PRINTF("Currently no support other PLL\r\n");
		    }
			MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
								MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
								MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			#endif
			#if	(CHIP == PYTHON)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 192) {
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G0126_192CPU_12G35_54G4_X);
			    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96)
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G0126_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 133)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G0126_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 144)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_192CPU_12G35_54G4_X);					
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 156)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_156G0126_192CPU_12G35_54G4_X);
				else {
					PRINTF("NO support pll\r\n");
				}
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 364){
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_364CPU_12G35_54G4_166G6_X);
			    else
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G0126_364CPU_12G35_54G46_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL0, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_144CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}		
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G0126_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {
				PRINTF("NO support pll\r\n");
			}
			#endif
			#if	(CHIP == P_V2)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 192) {
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012678_192CPU_12G35_54G4_X);
			    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96)
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012678_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 133)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G012678_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 144)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_192CPU_12G35_54G4_X);					
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 156)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_156G012678_192CPU_12G35_54G4_X);
				else {
					PRINTF("NO support pll\r\n");
				}
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 364){
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G01278_364CPU_12G35_54G4_166G6_X);
			    else
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G012678_364CPU_12G35_54G46_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL0, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_144CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}		
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G012678_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {
				PRINTF("NO support pll\r\n");
			}
			#endif	        
	        break;
	    case MMPS_SYSTEM_APMODE_MDTV:
        	#if(CHIP == D_V1)
			if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
		        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_192CPU_12G34_X);
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96) {
                retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012_192CPU_12G34_X);
		    }
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 120) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_120G012_192CPU_12G34_X);
		    }		    
		    else
		    {
		    	PRINTF("Currently no support other PLL\r\n");
		    }
			MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
								MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
								MMPS_System_GetConfiguration()->ulG012Freq * 1000);
    	    #endif
        	#if(CHIP == PYTHON)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G0126_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {	        	
		        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G0126_192CPU_12G35_54G4_X);
	        	MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 192000, 96000);
			}
    	    #endif
        	#if (CHIP == P_V2)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G012678_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {	        	
		        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012678_192CPU_12G35_54G4_X);
	        	MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 192000, 96000);
			}
    	    #endif
        	break;
	    case MMPS_SYSTEM_APMODE_USB:
            #if (CHIP == D_V1)
			if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
		        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_192CPU_12G34_X);
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012_192CPU_12G34_X);
		    }
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 120) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_120G012_192CPU_12G34_X);
		    }		    
		    else
		    {
		    	PRINTF("Currently no support other PLL\r\n");
		    }
			MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
								MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
								MMPS_System_GetConfiguration()->ulG012Freq * 1000);
            #endif
    	    #if (CHIP == PYTHON)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);				
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G0126_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);				
			}
			else
			{    	    
	        	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G0126_192CPU_12G35_54G4_X);
	        	MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 192000, 96000);
			}
	        #endif
    	    #if (CHIP == P_V2)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);				
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G012678_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);				
			}
			else
			{    	    
	        	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012678_192CPU_12G35_54G4_X);
	        	MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 192000, 96000);
			}
	        #endif
    	    break;
	    case MMPS_SYSTEM_APMODE_VIDEO_AMR_RECORD:
    	case MMPS_SYSTEM_APMODE_VIDEO_AAC_RECORD:
	    case MMPS_SYSTEM_APMODE_DSC:
	    case MMPS_SYSTEM_APMODE_MP3_PLAY:
            #if (CHIP == D_V1)
			if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
		        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_192CPU_12G34_X);
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012_192CPU_12G34_X);
		    }
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 120) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_120G012_192CPU_12G34_X);
		    }		    
		    else
		    {
		    	PRINTF("Currently no support other PLL\r\n");
            }
			MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
								MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
								MMPS_System_GetConfiguration()->ulG012Freq * 1000);
            #endif
			#if	(CHIP == PYTHON)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 192) {
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G0126_192CPU_12G35_54G4_X);
			    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96)
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G0126_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 133)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G0126_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 144)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_192CPU_12G35_54G4_X);					
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 156)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_156G0126_192CPU_12G35_54G4_X);
				else
					PRINTF("NO support pll\r\n");
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 364){
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_364CPU_12G35_54G4_166G6_X);
			    else
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G0126_364CPU_12G35_54G46_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL0, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G0126_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {
				PRINTF("NO support pll\r\n");
			}
			#endif
			#if	(CHIP == P_V2)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 192) {
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012678_192CPU_12G35_54G4_X);
			    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96)
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012678_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 133)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G012678_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 144)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_192CPU_12G35_54G4_X);					
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 156)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_156G012678_192CPU_12G35_54G4_X);
				else
					PRINTF("NO support pll\r\n");
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 364){
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G01278_364CPU_12G35_54G4_166G6_X);
			    else
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G012678_364CPU_12G35_54G46_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL0, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G012678_162CPU_12G345_X);
	        
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {
				PRINTF("NO support pll\r\n");
			}
			#endif
	        break;
	    default:
            #if (CHIP == D_V1)
			if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
		        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_192CPU_12G34_X);
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012_192CPU_12G34_X);
		    }
		    else if (MMPS_System_GetConfiguration()->ulG012Freq == 120) {
		    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_120G012_192CPU_12G34_X);
		    }		    
		    else
		    {
		    	PRINTF("Currently no support other PLL\r\n");
		    }
			MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
								MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
								MMPS_System_GetConfiguration()->ulG012Freq * 1000);
            #endif
			#if	(CHIP == PYTHON)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 192) {
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G0126_192CPU_12G35_54G4_X);
			    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96)
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G0126_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 133)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G0126_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 144)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_192CPU_12G35_54G4_X);					
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 156)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_156G0126_192CPU_12G35_54G4_X);
				else {
					PRINTF("NO support pll\r\n");
				}
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 364){
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012_364CPU_12G35_54G4_166G6_X);
			    else
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G0126_364CPU_12G35_54G46_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL0, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G0126_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}			
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G0126_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {
				PRINTF("NO support pll\r\n");
			}
			#endif
			#if	(CHIP == P_V2)
			if (MMPS_System_GetConfiguration()->ulCpuFreq == 192) {
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G012678_192CPU_12G35_54G4_X);
			    else if (MMPS_System_GetConfiguration()->ulG012Freq == 96)
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_96G012678_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 133)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G012678_192CPU_12G35_54G4_X);
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 144)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_192CPU_12G35_54G4_X);					
				else if (MMPS_System_GetConfiguration()->ulG012Freq == 156)
					retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_156G012678_192CPU_12G35_54G4_X);
				else {
					PRINTF("NO support pll\r\n");
				}
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL1, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 364){
				if (MMPS_System_GetConfiguration()->ulG012Freq == 132) 
			        retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_132G01278_364CPU_12G35_54G4_166G6_X);
			    else
			    	retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_133G012678_364CPU_12G35_54G46_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL0, MMP_TRUE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 144){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_144G012678_144CPU_12G345_X);
				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}			
			else if (MMPS_System_GetConfiguration()->ulCpuFreq == 162){
				retstatus = MMPD_System_SetPLL(MMPD_SYSTEM_PLL_13_162G012678_162CPU_12G345_X);

				MMPD_System_SetCPUMode(MMPD_SYSTEM_CPU_PLL4, MMP_FALSE, 
									MMPS_System_GetConfiguration()->ulCpuFreq * 1000, 
									MMPS_System_GetConfiguration()->ulG012Freq * 1000);
			}								
			else {
				PRINTF("NO support pll\r\n");
			}
			#endif
        	break;
	    }

	    if (retstatus != MMP_ERR_NONE)
    		return	retstatus;

	    if (apmode != MMPS_SYSTEM_APMODE_PLL) {
		     MMPS_System_AllocateFSBuffer(apmode);
	    }
	
	    MMPD_System_CheckVersion();
	    MMPS_System_SendEchoCommand();
	}	    
    return  MMP_ERR_NONE;
}
*/
//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetApplicationMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function will retrive current firmware name

@return The result of the current application (FW) mode
*/
#if PCAM_EN==0
MMP_ERR  MMPS_System_GetApplicationMode(MMPS_SYSTEM_APMODE *apmode)
{
    *apmode = m_systemApMode;
    return	MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetOperationMode
//  Description :
//------------------------------------------------------------------------------
/** @brief Get current operation mode

Get the operation mode from this module.
The main reason to get this mode is for LCD.
LCD must not send any data or commands to the LCD in Active mode.
@return Current Operation mode.
@pre The operatoin mode might not always reflect the real status. All customer flow should
call MMPS_System_SetOperationMode to maintain integrity of this status.
*/
MMP_ERR  MMPS_System_GetOperationMode(MMPS_SYSTEM_OPMODE *opmode)
{
	*opmode = m_systemOpMode;
    return	MMP_ERR_NONE;
}
#endif // PCAM_EN==0
//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetOperationMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function sets possible system operation mode

The function sets possible system operation mode defined in MMPS_System_OpMode by calling
MMPD_System_SetOperationMode. Three operation modes can be supported Bypass, Active,
Power Down. (Bypass <--> Active <--> Power Down)

@param[in] systemopmode the system operation mode
@retval MMPS_SYSTEM_SUCCESS success
@retval MMPS_SYSTEM_SUCCESS
@retval MMPS_SYSTEM_FAILURE_SETCONFIG
@retval MMPS_SYSTEM_FAILURE_INITIALMODULE
@retval MMPS_SYSTEM_FAILURE_INITIALLCD
@retval MMPS_SYSTEM_FAILURE_INITIALFILEIO
@retval MMPS_SYSTEM_FAILURE_SETOPMODE
@retval MMPS_SYSTEM_FAILURE_TESTMEMORY
@retval MMPS_SYSTEM_FAILURE_CMDTIMEOUT
@return It reports the status of the operation.
*/
/*
MMP_ERR  MMPS_System_SetOperationMode(MMPS_SYSTEM_OPMODE systemopmode)
{
    if (m_systemOpMode == MMPS_SYSTEM_OPMODE_ACTIVE) {
        if (systemopmode == MMPS_SYSTEM_OPMODE_BYPASS) {//Active -> Bypass
            MMPD_System_SetBypassMode(MMPD_SYSTEM_ENTER_BYPASS);
            MMPC_System_SetBypassPin(MMP_TRUE);
            m_systemOpMode = MMPS_SYSTEM_OPMODE_BYPASS;
        }
        else if (systemopmode == MMPS_SYSTEM_OPMODE_SUSPEND) {//Active -> Suspend, DRAM in selfrefresh mode
            MMPD_System_SetPSMode(MMPD_SYSTEM_ENTER_PS);
			
			#if	(CHIP == D_V1)
//			MMPH_HIF_RegSetB(GBL_ARM_PWR_DOWN, (MMPH_HIF_RegGetB(GBL_ARM_PWR_DOWN) & ~GBL_CPU_WFI_CLKOFF_EN)); 
//			MMPS_System_SendEchoCommand();
			#endif
//			MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_CPU, MMP_FALSE);
			MMPD_DRAM_SetSelfRefresh(MMP_TRUE);
            
            MMPC_System_SetBypassPin(MMP_TRUE);
            MMPC_System_SetCoreVoltage(MMP_FALSE);
            m_systemOpMode = MMPS_SYSTEM_OPMODE_SUSPEND;
        }
        else if (systemopmode == MMPS_SYSTEM_OPMODE_PWRDN) {//Active -> PowerSaving, DRAM in deeppower mode
            MMPD_System_SetPSMode(MMPD_SYSTEM_ENTER_PS);
			
			MMPH_HIF_RegSetB(GBL_ARM_PWR_DOWN, (MMPH_HIF_RegGetB(GBL_ARM_PWR_DOWN) & ~GBL_CPU_WFI_CLKOFF_EN)); 
			MMPS_System_SendEchoCommand();

			MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_CPU, MMP_FALSE);
			MMPD_DRAM_SetPowerDown(MMP_TRUE);
			m_systemApModePrePowerDown = m_systemApMode;	
			MMPS_System_SetApplicationMode(MMPS_SYSTEM_APMODE_NULL);
            
            MMPC_System_SetBypassPin(MMP_TRUE);
            MMPC_System_SetCoreVoltage(MMP_FALSE);
            m_systemOpMode = MMPS_SYSTEM_OPMODE_PWRDN;
        }
        else {
            return  MMP_SYSTEM_ERR_SETOPMODE;
        }
    }
    else if (m_systemOpMode == MMPS_SYSTEM_OPMODE_BYPASS) {
        if (systemopmode == MMPS_SYSTEM_OPMODE_ACTIVE) {//Bypass -> Active
            MMPC_System_SetBypassPin(MMP_FALSE);
            MMPD_System_SetBypassMode(MMPD_SYSTEM_EXIT_BYPASS);
            m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;
        }
        else if (systemopmode == MMPS_SYSTEM_OPMODE_PWRDN) {//Bypass -X-> PowerSaving
            PRINTF("MMPS_System_SetOperationMode: state change error. (Bypass mode can't change to PowerSaving mode) \r\n");
            return  MMP_SYSTEM_ERR_SETOPMODE;
        }
        else {
            return  MMP_SYSTEM_ERR_SETOPMODE;
        }
    }
    else if (m_systemOpMode == MMPS_SYSTEM_OPMODE_SUSPEND) {
        if (systemopmode == MMPS_SYSTEM_OPMODE_ACTIVE) {//Suspend -> Active
            MMPC_System_SetCoreVoltage(MMP_TRUE);
            // by pass set low
            MMPC_System_SetBypassPin(MMP_FALSE);
			
			MMPD_DRAM_SetSelfRefresh(MMP_FALSE);
//			MMPH_HIF_RegSetB(GBL_ARM_PWR_DOWN, GBL_CPU_WFI_CLKOFF_EN);
//			MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_CPU, MMP_TRUE);
            
            MMPD_System_SetPSMode(MMPD_SYSTEM_EXIT_PS);
            m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;
        }
        else if (systemopmode == MMPS_SYSTEM_OPMODE_BYPASS) {//PowerSaving -X-> Bypass
            PRINTF("MMPS_System_SetOperationMode: state change error. (PowerSaving mode can't change to Bypass mode) \r\n");
            return  MMP_SYSTEM_ERR_SETOPMODE;
        }
        else {
            return  MMP_SYSTEM_ERR_SETOPMODE;
        }
    }
    else if (m_systemOpMode == MMPS_SYSTEM_OPMODE_PWRDN) {
        if (systemopmode == MMPS_SYSTEM_OPMODE_ACTIVE) {//PowerSaving -> Active
            MMPC_System_SetCoreVoltage(MMP_TRUE);
			MMPS_System_SetApplicationMode(m_systemApModePrePowerDown);
            
            m_systemOpMode = MMPS_SYSTEM_OPMODE_ACTIVE;
        }
        else if (systemopmode == MMPS_SYSTEM_OPMODE_BYPASS) {//PowerSaving -X-> Bypass
            PRINTF("MMPS_System_SetOperationMode: state change error. (PowerSaving mode can't change to Bypass mode) \r\n");
            return  MMP_SYSTEM_ERR_SETOPMODE;
        }
        else {
            return  MMP_SYSTEM_ERR_SETOPMODE;
        }
    }
    else {
        return  MMP_SYSTEM_ERR_SETOPMODE;
    }

    return  MMP_ERR_NONE;
}
*/

//------------------------------------------------------------------------------
//  Function    : MMPS_System_DownloadFirmware
//  Description :
//------------------------------------------------------------------------------
/** @brief Download the firmware package from the memory

Download the firmware from the memory. The firmware would be wrapped into one file in our
firmware package format. See the following illustration.
@image html "image/firmware package.png"
@pre The firmware has to be loaded into the host memory
@param[in] ulFirmwareAddress The host address of the firmware
@return The status of the system function call.
@retval MMPS_SYSTEM_SUCCESS The firmware is finished.
@retval MMPS_SYSTEM_FAILURE_DOWNLOAD_FW This function encounter some error while downloading the firmware
@retval MMPS_SYSTEM_FAILURE_NOT_IMPLEMENTED This source is not supported.
*/
#if PCAM_EN==0
MMP_ERR MMPS_System_DownloadFirmware(MMP_ULONG ulFirmwareAddress)
{
    MMP_UBYTE nFirmware, i;
    MMP_ERR   retstatus;
    MMP_ULONG ulFirmwareSize, ulTargetAddress;
    MMP_UBYTE *pFirmwareAddress = (MMP_UBYTE*)ulFirmwareAddress;
    MMP_UBYTE *curAddr = pFirmwareAddress + 8; // number of firmware

    nFirmware = *curAddr;
    curAddr += 2; // to begining of the first firmware
    for (i = 0; i < nFirmware; i++) {
        ///Portability: Use memcpy to get info directly from the file.
        MEMCPY (&ulTargetAddress, curAddr, 4);
        curAddr += 4;

        MEMCPY (&ulFirmwareSize, curAddr, 4);
        curAddr += 4; // begin of this firmware binary

        retstatus = MMPD_System_DownloadFirmware(ulTargetAddress, (MMP_USHORT *)curAddr, ulFirmwareSize);
        curAddr += ulFirmwareSize; // to next firmware start
        if (MMP_ERR_NONE != retstatus) {
            return retstatus;
        }
    }

    return MMP_ERR_NONE;
}
#endif
//------------------------------------------------------------------------------
//  Function    : MMPS_System_DelayTime
//  Description :
//------------------------------------------------------------------------------
/** @brief The function delays the specified delay time input in milli-seconds.

The function delays the specified delay time input in milli-seconds. The implementation is system
dependent.
@param[in] ulDelaytime the delay time
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR    MMPS_System_DelayTime(MMP_ULONG ulDelaytime)
{
    MMP_ULONG i;
    for(i=0; i<ulDelaytime; i++);

    return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetAudioFrameStart
//  Description :
//------------------------------------------------------------------------------
/** 
    @brief  Keep the frame end address
    @return MMPS_SYSTEM_SUCCESS
*/
MMP_ERR	MMPS_System_SetAudioFrameStart(MMP_ULONG ulFBEndAddr)
{
	m_ulAudioFrameStart = ulFBEndAddr;
	m_ulPreviewFrameStart = ulFBEndAddr - (MMPS_System_GetConfiguration()->ulMaxReserveBufferSize *
											MMPS_System_GetConfiguration()->ulMaxReserveChannel);
	PRINTF("==m_ulAudioFrameStart== 0x%X\r\n", m_ulAudioFrameStart);
	PRINTF("==m_ulPreviewFrameStart== 0x%X\r\n", m_ulPreviewFrameStart);
	
	return	MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetAudioFrameStart
//  Description :
//------------------------------------------------------------------------------
/** 
    @brief  Return the frame end address
    @return MMPS_SYSTEM_SUCCESS
*/
MMP_ERR	MMPS_System_GetAudioFrameStart(MMP_ULONG audio_id, MMP_ULONG *ulFBEndAddr)
{

	 *ulFBEndAddr = m_ulAudioFrameStart - (audio_id * MMPS_System_GetConfiguration()->ulMaxReserveBufferSize);
	
	return	MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetPreviewFrameStart
//  Description :
//------------------------------------------------------------------------------
/** 
    @brief  Return the frame end address
    @return MMPS_SYSTEM_SUCCESS
*/
MMP_ERR	MMPS_System_GetPreviewFrameStart(MMP_ULONG *ulFBEndAddr)
{

	 *ulFBEndAddr = m_ulPreviewFrameStart;
	
	return	MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPS_System_AllocateFSBuffer
//  Description :
//------------------------------------------------------------------------------
/** 
    @brief  Application-mode Memory Allocation
    @param[in] apMode : application mode
    @return MMPS_SYSTEM_SUCCESS
*/
#if PCAM_EN==0
MMP_ERR MMPS_System_AllocateFSBuffer(const MMPS_SYSTEM_APMODE apMode)
{#if !defined(MBOOT_FW)
	#if (FS_INPUT_ENCODE == UCS2)
	MMP_BYTE	volume[32];
	#endif

    MMP_ULONG	buf_start, buf_size, cur_buf;
    MMP_ULONG   name_buf_size, parameter_buf_size, nand_buffer_require;
	MMP_ULONG	max_file_count, max_dir_count;

    #if defined(UPDATER_FW)
    buf_start = MMPS_System_GetConfiguration()->ulStackMemoryStart + 
                MMPS_System_GetConfiguration()->ulStackMemorySize;
    buf_size = MMPS_System_GetConfiguration()->ulStackMemorySize;
    #else
    MMPS_Display_GetOsdLocation(&buf_start, &buf_size);
    #endif

    #if (FS_INPUT_ENCODE == UCS2)
    name_buf_size = 0x300;
    #elif (FS_INPUT_ENCODE == UTF8)
    name_buf_size = 0x200;
    #endif
    parameter_buf_size = 8*1024;
    cur_buf = buf_start;

   	cur_buf -= name_buf_size;  
   	cur_buf -= parameter_buf_size;
    MMPD_FS_SetFileNameDmaAddr(cur_buf, name_buf_size, cur_buf + name_buf_size, parameter_buf_size);

    cur_buf -= 512;
    MMPD_FS_SetSDTmpMemory(cur_buf, 512);

	#if (OMA_DRM_EN == 1)
    cur_buf -= 16*12;
    MMPD_FS_SetDRMIVMemory(cur_buf, 16*12);
	#endif

	MMPD_FS_GetSMMEMAlloc(&nand_buffer_require);
   	cur_buf -= nand_buffer_require;
	MMPD_FS_SetSMMemory(cur_buf, nand_buffer_require);

	MMPD_FS_GetMaxFileOpenSupport(&max_file_count);
	MMPD_FS_GetMaxDirOpenSupport(&max_dir_count);	
	MMPD_FS_SetFATGlobalBufAddr(cur_buf - 8*1024 - (max_file_count + max_dir_count)*1024, 8*1024);
	MMPS_FS_Initialize();
   	MMPS_System_SetAudioFrameStart(cur_buf - 8*1024 - (max_file_count + max_dir_count)*1024);

	#if (FS_INPUT_ENCODE == UCS2)
	uniStrcpy(volume, (const char *)L"SD:\\");
	MMPS_FS_MountVolume(volume, uniStrlen((const short *)volume));
	uniStrcpy(volume, (const char *)L"SM:\\");
	MMPS_FS_MountVolume(volume, uniStrlen((const short *)volume));
	#elif (FS_INPUT_ENCODE == UTF8)
	MMPS_FS_MountVolume("SD:\\", STRLEN("SD:\\"));
	MMPS_FS_MountVolume("SM:\\", STRLEN("SM:\\"));
	#endif
#endif
    
    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_System_SendEchoCommand
//  Description :
//------------------------------------------------------------------------------
/** @brief The function will check current host interface workable

@return The result of the current host command interface is work or not.
*/

MMP_ERR  MMPS_System_SendEchoCommand()
{
	if (m_systemApMode != MMPS_SYSTEM_APMODE_NULL)
		return MMPD_System_SendEchoCommand();
	return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetRamOverwrite
//  Description :
//------------------------------------------------------------------------------
/** @brief The function will set the data in ram to be overwrite by dsc capture or other hardware application

@return The result of the current host command interface is work or not.
*/
MMP_ERR  MMPS_System_SetRamOverwrite(MMP_BOOL bOverwrite)
{
	if (bOverwrite) {
		MMPD_AUDIO_FlushMp3Cache();
	}	
	#endif
	m_bRamOverwrite = bOverwrite;
	return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPS_System_SetAudioOverwrite
//  Description :
//------------------------------------------------------------------------------
/** @brief The function will set the data in audio to be overwrite by dsc capture or other hardware application

@return The result of the current host command interface is work or not.
*/
MMP_ERR  MMPS_System_SetAudioOverwrite(MMP_ULONG ulRegion, MMP_BOOL bOverwrite)
{
    #if !(defined(UPDATER_FW)||defined(MBOOT_FW))
	MMPD_System_GetAudioFWStartAddress(1, &m_ulAudio1StartAddress);
	MMPD_System_GetAudioFWStartAddress(2, &m_ulAudio2StartAddress);
	
	if (bOverwrite) {
		MMPD_AUDIO_FlushAudioCache(ulRegion);
		MMPD_AUDIO_FlushAudioCache(2);
	}	
	#endif

	if (ulRegion == 1) {
		m_bAudio1Overwrite = bOverwrite;
	}
	else if (ulRegion == 2) {
		m_bAudio2Overwrite = bOverwrite;
	}
	
	return MMP_ERR_NONE;
}
#endif
/// @}

#ifdef BUILD_CE
#define BUILD_FW
#endif
