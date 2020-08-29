/// @ait_only
//==============================================================================
//
//  File        : mmpd_system.c
//  Description : Ritian System Control Device Driver Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpd_system.c
 *  @brief The header File for the Host System Control Device Driver Function
 *  @author Penguin Torng
 *  @version 1.0
 */
#include "config_fw.h"

#ifdef BUILD_CE
#undef BUILD_FW
#endif

#include "mmp_lib.h"
#include "lib_retina.h"

#include "mmpd_system.h"
#include "mmph_hif.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_dram.h"
#include "mmp_reg_gpio.h"
#include "mmpd_dram.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbuvch264.h"
#include "mmpf_pll.h"
//#include "ait_utility.h"
/** @addtogroup MMPD_System
 *  @{
 */

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

/**@brief The PLL mode setting.
*/
//static	MMPD_SYSTEM_PLL_MODE m_PLLMode = MMPD_SYSTEM_PLL_MODE_NUMBER;

/**@brief The Core ID of the chip
*/
static	MMP_UBYTE	m_gbSystemCoreID;

/**@brief Keep the reference count of the clock module
*/
static  MMP_ULONG   ulVIRefCount        = 0;
static  MMP_ULONG   ulIspRefCount       = 0;
static	MMP_ULONG	ulAudioRefCount     = 0;
static	MMP_ULONG	ulGraphicsRefCount  = 0;
static	MMP_ULONG	ulIBCRefCount       = 0;
static	MMP_ULONG	ulJPGRefCount       = 0;
static	MMP_ULONG	ulScalerRefCount    = 0;
static	MMP_ULONG	ulIconRefCount      = 0;
#if (PCAM_EN == 0x0)
static	MMP_ULONG	ulVideoRefCount     = 0;
#endif
static  MMP_ULONG   ulH264RefCount      = 0;
static  MMP_ULONG   ulDMARefCount = 0;
static  MMP_ULONG   ulRawprocRefCount   = 0;
static	MMP_ULONG	ulGpioRefCount    = 0;
#if (CHIP == P_V2)
static  MMP_ULONG   ulMP3DRefCount   	= 0;
#endif
#if (CHIP == P_V2)
static  MMP_ULONG   ulMFDRefCount   	= 0;
#endif	

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPD_System_ResetVariables
//  Description :
//------------------------------------------------------------------------------
/** @brief The function reset all file-scope variables to default value

The function reset all file-scope variables to default value
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_System_ResetVariables()
{
    m_PLLMode = MMPD_SYSTEM_PLL_MODE_NUMBER;
    ulVIRefCount = 0;
    ulAudioRefCount = 0;
    ulGraphicsRefCount = 0;
    ulIBCRefCount = 0;
    ulJPGRefCount = 0;
    ulScalerRefCount = 0;
    ulIconRefCount = 0;
    ulVideoRefCount = 0;
    ulRawprocRefCount = 0;

    return MMP_ERR_NONE;
}
//  Function    : MMPD_System_SetHostBusMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function sets the current host bus mode

The function uses the current host bus mode defined in MMPS_System_Config.hostbusmodeselect
and prepares the internal access functions for register, memory and FIFO under the bus mode. The
function needs to be called before any accesses to the devices.

@return It reports the status of the operation.
*/
MMP_ERR    MMPD_System_SetHostBusMode(MMPD_HIF_INTERFACE hifmode)
{
    ///Set host interface to 16 or 8 bits mode.
    if (hifmode == MMPD_HIF_8_BIT_MODE) {
        MMPH_HIF_SetInterface(MMPH_HIF_8_BIT_MODE);
	}else{
        MMPH_HIF_SetInterface(MMPH_HIF_16_BIT_MODE);
	}

    return  MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPD_System_SetBypassMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function enables or disables the bypass mode

The function enables or disables the bypass mode by sending host command request
@param[in] bypassmode enable or disable bypass mode
@return It reports the status of the operation.
*/
MMP_ERR    MMPD_System_SetBypassMode(MMPD_SYSTEM_BYPASS_MODE bypassmode)
{
    if (bypassmode == MMPD_SYSTEM_ENTER_BYPASS) {
        MMPH_HIF_CmdSend(ENTER_BYPASS_MODE | HIF_CMD_SET_BYPASS_MODE);
    }
    else {
        MMPH_HIF_CmdSend(EXIT_BYPASS_MODE | HIF_CMD_SET_BYPASS_MODE);
    }
    
    return  MMPC_System_CheckMMPStatus(SYSTEM_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_System_SetPSMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function sets the power saving mode

The function sets the power saving mode
@param[in] psmode the power saving mode
@return It reports the status of the operation.
*/
MMP_ERR MMPD_System_SetPSMode(MMPD_SYSTEM_PS_MODE psmode)
{
    if (psmode == MMPD_SYSTEM_ENTER_PS) {
       	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_AUD, MMP_TRUE);
    
		//PRINTF("MMPD_System_SetPSMode: ENTER\r\n");
        MMPH_HIF_RegSetW(HOSTCMD_CMD, ENTER_PS_MODE | HIF_CMD_SET_PS_MODE);
    }
    else {
		//PRINTF("MMPD_System_SetPSMode: EXIT\r\n");
        MMPH_HIF_RegSetW(HOSTCMD_CMD, EXIT_PS_MODE | HIF_CMD_SET_PS_MODE);
    }

    RTNA_WAIT_MS(100); //Must wait

    if (MMPH_HIF_RegGetW(HOSTCMD_CMD)) {
        //PRINTF("MMPD_System_SetPSMode: must wait more time 1. 0x%X\r\n", MMPH_HIF_RegGetW(HOSTCMD_CMD));
        return	MMP_SYSTEM_ERR_SETPSMODE;
    }

    if (MMPH_HIF_CmdGetStatusL() & SYSTEM_CMD_IN_EXEC) {
        //PRINTF("MMPD_System_SetPSMode: must wait more time 2\r\n");
        //PRINTF("MMPH_HIF_CmdGetStatusL() = 0%X\r\n", MMPH_HIF_CmdGetStatusL());
        return	MMP_SYSTEM_ERR_SETPSMODE;
    }
    
    #if (CHIP == P_V2)||(CHIP == VSN_V2)
    if (psmode == MMPD_SYSTEM_ENTER_PS) {
    	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_AUD, MMP_FALSE);
    }
    #endif

    return  MMP_ERR_NONE;
}
#endif


//------------------------------------------------------------------------------
//  Function    : MMPD_System_EnableClock
//  Description :
//------------------------------------------------------------------------------
/** @brief The function enables or disables the specified clock

The function enables or disables the specified clock from the clock type input by programming the
Global controller registers.

@param[in] ulClockType the clock type to be selected
@param[in] bEnableclock enable or disable the clock
@return It reports the status of the operation.
*/
MMP_ERR  MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE clocktype, MMP_BOOL bEnableclock)
{
   	AITPS_GBL   pGBL  = AITC_BASE_GBL;
	#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr = 0;
	#endif
    if (bEnableclock) {
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_VI) {
            OS_ENTER_CRITICAL();
        	ulVIRefCount++;
        	if(ulVIRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_VI_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_ISP) {
            OS_ENTER_CRITICAL();
        	ulIspRefCount++;
        	if(ulIspRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_ISP_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_AUD) {
        	OS_ENTER_CRITICAL();
        	ulAudioRefCount++;
        	if(ulAudioRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_AUD_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_JPG) {    	
        	OS_ENTER_CRITICAL();
        	ulJPGRefCount++;
        	if(ulJPGRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_JPG_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_SCAL) {
        	OS_ENTER_CRITICAL();
        	ulScalerRefCount++;
        	if(ulScalerRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_SCAL_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_GPIO) {
        	OS_ENTER_CRITICAL();
        	ulGpioRefCount++;
        	if(ulGpioRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_GPIO_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }

        if (clocktype == MMPD_SYSTEM_CLK_MODULE_GRA) {
        	OS_ENTER_CRITICAL();
        	ulGraphicsRefCount++;
        	if(ulGraphicsRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_GRA_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_IBC) {
        	OS_ENTER_CRITICAL();
        	ulIBCRefCount++;
        	if(ulIBCRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_IBC_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_ICON) {
        	OS_ENTER_CRITICAL();
        	ulIconRefCount++;
        	if(ulIconRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_ICON_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_H264) {
        	OS_ENTER_CRITICAL();
        	ulH264RefCount++;
        	if(ulH264RefCount == 0x1) {
        		pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_H264_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_DMA) {
            OS_ENTER_CRITICAL();
        	ulDMARefCount++;
        	if(ulDMARefCount == 0x1) {
        		pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_DMA_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_RAWPROC) {
            OS_ENTER_CRITICAL();
        	ulRawprocRefCount++;
        	if(ulRawprocRefCount == 0x1) {
        		pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_RAW_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
	}
    else {
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_VI) {
            OS_ENTER_CRITICAL();
            if (ulVIRefCount) {
        	ulVIRefCount--;
        	}
        	if(ulVIRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS0 |= (GBL_CLK_VI_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_ISP) {
            OS_ENTER_CRITICAL();
        	if (ulIspRefCount) {
        	    ulIspRefCount--;
        	}
        	if(ulIspRefCount == 0) {
        		pGBL->GBL_CLK_DIS0 |= (GBL_CLK_ISP_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_AUD) {
        	OS_ENTER_CRITICAL();
        	if (ulAudioRefCount) {
        	ulAudioRefCount--;
        	}
        	if(ulAudioRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS0 |= (GBL_CLK_AUD_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_JPG) {    	
        	OS_ENTER_CRITICAL();
        	if (ulJPGRefCount) {
        	ulJPGRefCount--;
        	}
        	if(ulJPGRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS0 |= (GBL_CLK_JPG_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_SCAL) {
        	OS_ENTER_CRITICAL();
        	if (ulScalerRefCount) {
        	ulScalerRefCount--;
        	}
        	if(ulScalerRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS0 |= (GBL_CLK_SCAL_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_GPIO) {
        	OS_ENTER_CRITICAL();
        	if (ulGpioRefCount) {
        	ulGpioRefCount--;
        	}
        	if(ulGpioRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS0 |= (GBL_CLK_GPIO_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_GRA) {
        	OS_ENTER_CRITICAL();
        	if (ulGraphicsRefCount) {
        	ulGraphicsRefCount--;
        	}
        	if(ulGraphicsRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS1 |= (GBL_CLK_GRA_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_IBC) {
        	OS_ENTER_CRITICAL();
        	if (ulIBCRefCount) {
        	ulIBCRefCount--;
        	}
        	if(ulIBCRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS1 |= (GBL_CLK_IBC_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_ICON) {
        	OS_ENTER_CRITICAL();
        	if (ulIconRefCount) {
        	ulIconRefCount--;
        	}
        	if(ulIconRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS1 |= (GBL_CLK_ICON_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_H264) {
        	OS_ENTER_CRITICAL();
        	if (ulH264RefCount) {
        	ulH264RefCount--;
        	}
        	if(ulH264RefCount == 0x0) {
        		pGBL->GBL_CLK_DIS1 |= (GBL_CLK_H264_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_DMA) {
            OS_ENTER_CRITICAL();
            if (ulDMARefCount) {
        	ulDMARefCount--;
        	}
        	if(ulDMARefCount == 0x0) {
        		pGBL->GBL_CLK_DIS1 |= (GBL_CLK_DMA_DIS);
        	}
        	OS_EXIT_CRITICAL();
        }
        if (clocktype == MMPD_SYSTEM_CLK_MODULE_RAWPROC) {
            OS_ENTER_CRITICAL();
            if (ulRawprocRefCount) {
        	ulRawprocRefCount--;
        	}
        	if(ulRawprocRefCount == 0x0) {
        		pGBL->GBL_CLK_DIS1 |= (GBL_CLK_RAW_DIS);
        	}
        	OS_EXIT_CRITICAL();
            
        }
    }

    return  MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPD_System_GetClock
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the clock status of all modules

@param[out] ulClockType The current clock setting.
@return Always return success.
*/
MMP_ERR  MMPD_System_GetClock(MMP_ULONG *ulClockType)
{
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_System_ResetHModule
//  Description :
//------------------------------------------------------------------------------
/** @brief The function resets hardware module

The function resets hardware module
@param[in] moduletype the module
@param[in] bResetRegister MMP_TRUE: reset the registers and state of the module; MMP_FALSE: only reset the state of the module.
@return It reports the status of the operation.
*/

MMP_ERR  MMPD_System_ResetHModule(MMPD_SYSTEM_HMODULETYPE moduletype, MMP_BOOL bResetRegister)
{
	AITPS_GBL   pGBL  = AITC_BASE_GBL;
	/*#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr = 0;
	#endif*/
	
	if(moduletype < MMPD_SYSTEM_HMODULETYPE_CPU_SRAM) {
    	pGBL->GBL_RST_CTL01 |= ((MMP_USHORT)1 << moduletype);
    	if(bResetRegister == MMP_TRUE) {
    		pGBL->GBL_RST_REG_EN |= ((MMP_USHORT)1 << moduletype);
    	}
    }
    else {
    	pGBL->GBL_RST_CTL2 |=  ((MMP_UBYTE)1 << (moduletype - MMPD_SYSTEM_HMODULETYPE_CPU_SRAM));
    	if (bResetRegister == MMP_TRUE) {
    		if((moduletype == MMPD_SYSTEM_HMODULETYPE_SD) ||(moduletype == MMPD_SYSTEM_HMODULETYPE_PWM) ||
    			(moduletype == MMPD_SYSTEM_HMODULETYPE_PSPI)) {
    			pGBL->GBL_RST_CTL2 |=  ((MMP_UBYTE)1 << (moduletype - MMPD_SYSTEM_HMODULETYPE_CPU_SRAM + 4));	
    		}
    	}
    }
    //MMPC_System_WaitMmp(100);
    RTNA_WAIT_CYCLE(100); // sean@2011_04_20

    if(moduletype < MMPD_SYSTEM_HMODULETYPE_CPU_SRAM) {
    	pGBL->GBL_RST_CTL01 &= (~((MMP_USHORT)1 << moduletype));
    	if(bResetRegister == MMP_TRUE) {
    		pGBL->GBL_RST_REG_EN &= (~((MMP_USHORT)1 << moduletype));
    	}
    }
	else {
		pGBL->GBL_RST_CTL2 &=  ~((MMP_UBYTE)1 << (moduletype - MMPD_SYSTEM_HMODULETYPE_CPU_SRAM));
    	if (bResetRegister == MMP_TRUE) {
    		if((moduletype == MMPD_SYSTEM_HMODULETYPE_SD) ||(moduletype == MMPD_SYSTEM_HMODULETYPE_PWM) ||
    			(moduletype == MMPD_SYSTEM_HMODULETYPE_PSPI)) {
    			pGBL->GBL_RST_CTL2 &=  ~((MMP_UBYTE)1 << (moduletype - MMPD_SYSTEM_HMODULETYPE_CPU_SRAM + 4));	
    		}
    	}
	}
    return  MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPD_System_ResetCPU
//  Description : 
//------------------------------------------------------------------------------
/** @brief Reset the CPU or let CPU free run.

The reason why seperate CPU reset from MMPD_System_ResetHModule is that the CPU
might not free run right away. Now it's only used in firmware downloading.
@param[in] bLock Lock CPU or not.
@return The status of the the function call.
*/
/*
MMP_ERR MMPD_System_ResetCPU(MMP_BOOL bLock)
{
    MMP_ULONG i;
    // do the software reset
    if (bLock) {
        // clear CPU boot status
        MMPH_HIF_RegSetL(HOSTCMD_STS, (MMPH_HIF_CmdGetStatusL() & (~CPU_BOOT_FINISH)));

        #if (CHIP == P_V2)
        MMPH_HIF_RegSetB(GBL_RST_SW_EN, GBL_MODULE_CPU);
        #endif
		// patch for during download RGB LCD is working
		#if defined(UPDATER_FW)
		MMPD_System_ResetHModule(MMPD_SYSTEM_HMODULETYPE_MCI, MMP_FALSE);
        #else
		MMPD_Display_SetDisplayPause(MMPD_DISPLAY_PRM_CTL, MMP_TRUE);
		MMPD_System_ResetHModule(MMPD_SYSTEM_HMODULETYPE_MCI, MMP_FALSE);
		MMPD_Display_SetDisplayPause(MMPD_DISPLAY_PRM_CTL, MMP_FALSE);

		#endif
		MMPC_System_WaitMmp(100);

		return MMP_ERR_NONE;
    } 
    else {
        // CPU free run
        PRINTF("Firmware Booting...");
		MMPH_HIF_RegSetB(GBL_ARM_PWR_DOWN, GBL_CPU_WFI_CLKOFF_EN); 
        #if (CHIP == P_V2)
        MMPH_HIF_RegSetB(GBL_RST_SW_DIS, GBL_MODULE_CPU);
        #endif

        // Add CPU time out
        for (i = 0; i < 0x10000; i++) {
            if ((MMPH_HIF_CmdGetStatusL() & CPU_BOOT_FINISH)) {
                PRINTF("OK\r\n");
               	return MMP_ERR_NONE;
            }
        }
        PRINTF("Failed\r\n");
    }
    return MMP_SYSTEM_ERR_CPUBOOT;
}
*/
//------------------------------------------------------------------------------
//  Function    : MMPD_System_SetCPUMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function enables or disables the specified clock

The function enables or disables the specified clock from the clock type input by programming the
Global controller registers.

@param[in] ulPllSrc the PLL src to be selected. (0x00: GBL_CPU_SRC_DPLL0, 0x10: GBL_CPU_SRC_DPLL1; 0x20: GBL_CPU_SRC_DPLL2)
@param[in] bAsyncEn enable or disable the synchronous mode
@param[in] ulCPUFreq 1000*(CPU frequency) (ex: 133MHz -> 133000) 
@param[in] ulMCIFreq 1000*(MCI frequency) (ex: 96.2MHz -> 96200)
@return It reports the status of the operation.
*/
/*
MMP_ERR MMPD_System_SetCPUMode(MMP_ULONG ulPllSrc, MMP_BOOL bAsyncEn, MMP_ULONG ulCPUFreq, MMP_ULONG ulMCIFreq)

{
	MMP_UBYTE	reg;

	// hold CPU
	#ifndef BUILD_CE
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_CPU, MMP_FALSE);
	#endif
	
	reg = MMPH_HIF_RegGetB(GBL_CLK_PATH_CTL2) & ~(GBL_CPU_SRC_MUX);

#if (CHIP == P_V2)
	if (ulPllSrc < MMPD_SYSTEM_CPU_PLL4) {
		MMPH_HIF_RegSetB(GBL_CLK_PATH_CTL2, reg | ulPllSrc << 4); 
		MMPH_HIF_RegSetB(GBL_CLK_SRC_HIGH, MMPH_HIF_RegGetB(GBL_CLK_SRC_HIGH) &~ GBL_CPU_SEL_PLL_HIGH);
	}
	else {
		MMPH_HIF_RegSetB(GBL_CLK_PATH_CTL2, reg); 
		MMPH_HIF_RegSetB(GBL_CLK_SRC_HIGH, MMPH_HIF_RegGetB(GBL_CLK_SRC_HIGH) | GBL_CPU_SEL_PLL_HIGH);		
	}
#endif	
	reg = MMPH_HIF_RegGetB(GBL_ARM_CLK_CTL);
	if (bAsyncEn) {
        if(ulMCIFreq > 2*ulCPUFreq)
		    MMPH_HIF_RegSetB(GBL_ARM_CLK_CTL, reg | MCI_FREQ_LT_2_AMR_FREQ | ARM_ASYNC_MODE_EN);
        else
            MMPH_HIF_RegSetB(GBL_ARM_CLK_CTL, (reg & ~MCI_FREQ_LT_2_AMR_FREQ) | ARM_ASYNC_MODE_EN);
	}            
	else
		MMPH_HIF_RegSetB(GBL_ARM_CLK_CTL, reg & ~ARM_ASYNC_MODE_EN);

	// release CPU
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_CPU, MMP_TRUE);

    return  MMP_ERR_NONE;
}
*/
//------------------------------------------------------------------------------
//  Function    : MMPD_System_ConfigTCM
//  Description : 
//------------------------------------------------------------------------------
/** @brief Config the TCM size.

@param[in] usITCM ITCM can be 0, 16, 32, 48,64. (KB). The left would be DTCM
@remarks There is no I00_D64 config. Input 0 to ignore this value.
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_System_ConfigTCM(MMP_USHORT usITCM)
{
    switch (usITCM) {
        return MMP_SYSTEM_ERR_NOT_SUPPORT;
    }
    return MMP_ERR_NONE;
}
#endif


//------------------------------------------------------------------------------
//  Function    : MMPD_System_TestMemory
//  Description : Memory test for internal SRAM from 0x10_0000 to 0x19_0000
//------------------------------------------------------------------------------
/** @brief The function runs memory test for internal memory according to the current configuration flag

The function runs memory test for internal memory according to the current configuration flag
@return It reports the status of the operation.
*/
#define TCM_1_MEMORY_START   0x0        /// Internal TCM memory start
#define TCM_1_MEMORY_SIZE    0x2000

#define TCM_2_MEMORY_START   0x80000    /// Internal TCM memory start
#define TCM_2_MEMORY_SIZE    0x2000

#define INTERNAL_MEMORY_START   0x100000

#if	(CHIP == P_V2)||(CHIP == VSN_V2)
#define INTERNAL_MEMORY_SIZE    0xA0000
#endif
/*
MMP_ERR    MMPD_System_TestMemory(MMPD_SYSTEM_MEMTEST memtestmode)
{
    MMP_ULONG   i;

    if(memtestmode == MMPD_SYSTEM_MEMTEST_TCM || memtestmode == MMPD_SYSTEM_MEMTEST_ALL) {
       	MMPH_HIF_RegSetB(GBL_TCM_DNLD_EN, 0x01);                          // Enable HOST TCM access

        PRINTF("TCM 1 memory phase test\r\n");
        for (i = TCM_1_MEMORY_START; i < (TCM_1_MEMORY_START + TCM_1_MEMORY_SIZE); i+=4)
            MMPH_HIF_MemSetL(i, 0x55AA55AA);

        for (i = TCM_1_MEMORY_START; i < (TCM_1_MEMORY_START + TCM_1_MEMORY_SIZE); i+=4)
            if (MMPH_HIF_MemGetL(i) != 0x55AA55AA) {
                PRINTF("***********error***********\r\n");
                PRINTF("mem : 0x%X. Set as 0x55AA55AA. Get as 0x%X\r\n", i, MMPH_HIF_MemGetL(i));
                //return MMP_SYSTEM_ERR_MEMORY_TEST_FAIL;
            }

        PRINTF("TCM 2 memory phase test\r\n");
        for (i = TCM_2_MEMORY_START; i < (TCM_2_MEMORY_START + TCM_2_MEMORY_SIZE); i+=4)
            MMPH_HIF_MemSetL(i, 0xAA55AA55);

        for (i = TCM_2_MEMORY_START; i < (TCM_2_MEMORY_START + TCM_2_MEMORY_SIZE); i+=4)
            if (MMPH_HIF_MemGetL(i) != 0xAA55AA55) {
                PRINTF("***********error***********\r\n");
                PRINTF("mem : 0x%X. Set as 0xAA55AA55. Get as 0x%X\r\n", i, MMPH_HIF_MemGetL(i));
                //return MMP_SYSTEM_ERR_MEMORY_TEST_FAIL;
            }

       	MMPH_HIF_RegSetB(GBL_TCM_DNLD_EN, 0x0);
    }

    if(memtestmode == MMPD_SYSTEM_MEMTEST_FB || memtestmode == MMPD_SYSTEM_MEMTEST_ALL) {
    	PRINTF("internal memory phase 1 test\r\n");
    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i+=2)
    		MMPH_HIF_MemSetW(i, 0x0000);

    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i+=2)
    		if (MMPH_HIF_MemGetW(i) != 0x0000) {
    			PRINTF("***********error***********\r\n");
    			PRINTF("mem : 0x%X. Set as 0x0000. Get as 0x%X\r\n", i, MMPH_HIF_MemGetW(i));
                //return MMP_SYSTEM_ERR_MEMORY_TEST_FAIL;
    		}

    	PRINTF("internal memory phase 2 test\r\n");
    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i+=2)
    		MMPH_HIF_MemSetW(i, 0xFFFF);

    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i+=2)
    		if (MMPH_HIF_MemGetW(i) != 0xFFFF) {
    			PRINTF("***********error***********\r\n");
    			PRINTF("mem : 0x%X. Set as 0xFFFF. Get as 0x%X\r\n", i, MMPH_HIF_MemGetW(i));
                //return MMP_SYSTEM_ERR_MEMORY_TEST_FAIL;
    		}

    	PRINTF("internal memory phase 3 test\r\n");
    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i++)
    		MMPH_HIF_MemSetB(i, 0x55);

    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i++)
    		if (MMPH_HIF_MemGetB(i) != 0x55) {
    			PRINTF("***********error***********\r\n");
    			PRINTF("mem : 0x%X. Set as 0x55. Get as 0x%X\r\n", i, MMPH_HIF_MemGetB(i));
                //return MMP_SYSTEM_ERR_MEMORY_TEST_FAIL;
    		}

    	PRINTF("internal memory phase 4 test\r\n");
    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i++)
    		MMPH_HIF_MemSetB(i, 0xAA);

    	for (i = INTERNAL_MEMORY_START; i < (INTERNAL_MEMORY_START+INTERNAL_MEMORY_SIZE); i++)
    		if (MMPH_HIF_MemGetB(i) != 0xAA) {
    			PRINTF("***********error***********\r\n");
    			PRINTF("mem : 0x%X. Set as 0xAA. Get as 0x%X\r\n", i, MMPH_HIF_MemGetB(i));
                //return MMP_SYSTEM_ERR_MEMORY_TEST_FAIL;
    		}
    }
    return MMP_ERR_NONE;
}
*/

//------------------------------------------------------------------------------
//  Function    : MMPD_System_DownloadFirmware
//  Description :
//------------------------------------------------------------------------------
/** @brief The function downloads firmware into the code memory.

The function downloads firmware into the code memory.
@param[in] ulDevaddr the memory address in MMP
@param[in] usFwaddr the memory address ar host
@param[in] ulLength the firmware size
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR    MMPD_System_DownloadFirmware(MMP_ULONG ulDevaddr,
                        MMP_USHORT *usFwaddr, MMP_ULONG ulLength)
{
    MMP_ULONG		tmp;
    MMP_ULONG		i;
    MMP_ULONG		fwval;
    
    if (ulLength) {
        PRINTF("Downloading length 0x%05X, ", ulLength);

        //[HWLIMIT] jerrytsao
        // When accessing TCM, using auto increment RD/WR mode and disable
        // host interface auto flush is necessary.
        // 1.Increment mode activate the 8-byte ping-pong write buffer.
        // 2.TCM only support DWORD access. (4x bytes)
        // 3.Any other host interface access to TCM is not allowed.

        if (ulDevaddr == 0 || ulDevaddr == 0x80000) {
            MMPH_HIF_RegSetB(GBL_TCM_DNLD_EN, 0x01); // Enable HOST TCM access
        }

#pragma O0
		MMPH_HIF_MemCopyHostToDev(ulDevaddr, (MMP_UBYTE *)usFwaddr, ulLength);
	    PRINTF("Verifying...");
	    for (i = 0; i < (ulLength >> 1); i+=2) {
   	    	tmp = MMPH_HIF_MemGetL(ulDevaddr + (i << 1));
   	    	fwval = *(usFwaddr + i) + ((MMP_ULONG)*(usFwaddr + i + 1) << 16);
   	    	if (tmp != fwval) {
       	    	PRINTF("0x%X : 0x%X, 0x%X\r\n", i, tmp, fwval);
      	    	return	MMP_SYSTEM_ERR_VERIFY_FW;
	        }
	        
   		}
#pragma
    	PRINTF("Done\r\n");

        if (ulDevaddr == 0 || ulDevaddr == 0x80000) {
            MMPH_HIF_RegSetB(GBL_TCM_DNLD_EN, 0x00); // Disable HOST TCM access
        }
    }

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_System_SetPLL
//  Description :
//------------------------------------------------------------------------------
/** @brief The function configures the PLL

The function configures the PLL
@param[in] PLLMode the PLL mode
@return It reports the status of the operation.
*/
MMP_ERR    MMPD_System_SetPLL(MMPD_SYSTEM_PLL_MODE PLLMode)
{
    MMP_BOOL KeepG0;

	if(m_PLLMode == MMPD_SYSTEM_PLL_ExtClkOutput || m_PLLMode == MMPD_SYSTEM_PLL_MODE_NUMBER) {
		KeepG0 = MMP_FALSE;
 	}
	else {
		KeepG0 = MMP_TRUE;
	}

	//AIT_PMP,20100312
	KeepG0 = MMP_TRUE;
	m_PLLMode = PLLMode;


	#ifdef BUILD_CE
    MMPF_PLL_Setting(PLLMode, (MMP_BOOL)KeepG0);
	#else
    MMPH_HIF_CmdSetParameterL(0, (MMP_ULONG)PLLMode);
	MMPH_HIF_CmdSetParameterL(4, (MMP_ULONG)KeepG0);
    MMPH_HIF_CmdSend(PLL_SETTING | HIF_CMD_SYSTEM_SET_PLL);
    MMPH_HIF_RegSetW(HOSTCMD_CMD, PLL_SETTING | HIF_CMD_SYSTEM_SET_PLL);
    #endif

#if 0	
    MMPH_HIF_CmdSetParameterL(0, (MMP_ULONG)PLLMode);
	MMPH_HIF_CmdSetParameterL(4, (MMP_ULONG)KeepG0);
	#ifdef BUILD_CE
    //MMPH_HIF_CmdSend(PLL_SETTING | HIF_CMD_SYSTEM_SET_PLL);
    MMPF_PLL_Setting(PLLMode, (MMP_BOOL)KeepG0);
	#else
    MMPH_HIF_RegSetW(HOSTCMD_CMD, PLL_SETTING | HIF_CMD_SYSTEM_SET_PLL);
    #endif
#endif
       
    RTNA_WAIT_MS(60); // Truman061023, save downloading time
#if PCAM_EN==0
    if (MMPH_HIF_RegGetW(HOSTCMD_CMD)) {
        PRINTF("MMPD_System_SetPLL: must wait more time 1\r\n");
        return	MMP_SYSTEM_ERR_SETPLL;
    }

    if (MMPH_HIF_CmdGetStatusL() & SYSTEM_CMD_IN_EXEC) {
        PRINTF("MMPD_System_SetPLL: must wait more time 2\r\n");
        return	MMP_SYSTEM_ERR_SETPLL;
    }
#endif
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_System_GetPLL
//  Description :
//------------------------------------------------------------------------------
/** @brief The function get the PLL mode

The function get the PLL mode
@param[out] PLLMode the PLL mode
@return It reports the status of the operation.
*/
MMP_ERR    MMPD_System_GetPLL(MMPD_SYSTEM_PLL_MODE *PLLMode)
{
	*PLLMode = m_PLLMode;
	
	return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_System_SetPLL
//  Description :
//------------------------------------------------------------------------------
#if 0
MMP_ERR    MMPD_System_SetPLLPowerUp(MMPD_SYSTEM_PLL_NO PLLNo, MMP_BOOL PowerUp)
{
    MMP_ULONG i;

    MMPH_HIF_CmdSetParameterL(0, (MMP_ULONG)PLLNo);
    MMPH_HIF_CmdSetParameterL(4, (MMP_ULONG)PowerUp);

    if (PowerUp == MMP_TRUE) {
        MMPH_HIF_RegSetW(HOSTCMD_CMD, PLL_POWER_UP | HIF_CMD_SYSTEM_SET_PLL);

        for (i=0; i<0x100000; i++); //Must wait

        if (MMPH_HIF_RegGetW(HOSTCMD_CMD)) {
            PRINTF("MMPD_System_SetPLLPowerUp must wait more time");
        }

        if (MMPH_HIF_CmdGetStatusL() & SYSTEM_CMD_IN_EXEC) {
            PRINTF("MMPD_System_SetPLLPowerUp must wait more time");
        }
    }
    else {// Power Down
        MMPH_HIF_CmdSend(PLL_POWER_UP | HIF_CMD_SYSTEM_SET_PLL);
        return MMPC_System_CheckMMPStatus(SYSTEM_CMD_IN_EXEC);
    }

    return MMP_ERR_NONE;
}
#endif

#if PCAM_EN==0

//------------------------------------------------------------------------------
//  Function    : MMPD_System_GetFWEndAddress
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get the end address of download firmware.
 
 This API gets the end address of current downloaded firmware. It can optimize the
 memory usage without ldsfile setting.
 @param[out] ulAddress Start address of firmware available buffer.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_System_GetFWEndAddress(MMP_ULONG *ulAddress)
{
	MMP_ERR	mmpstatus;

    MMPH_HIF_CmdSend(FW_END | HIF_CMD_GET_FW_ADDR);
    mmpstatus = MMPC_System_CheckMMPStatus(SYSTEM_CMD_IN_EXEC);
    if (mmpstatus == MMP_ERR_NONE) {
	    *ulAddress = MMPH_HIF_CmdGetParameterL(0);
	}	    

	return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_System_GetAudioFWStartAddress
//  Description :
//------------------------------------------------------------------------------
/**
 @brief Get the start address of the audio code
 
 This API gets the start address of audio codes.
 @param[in] ulRegion Region 1 or 2
 @param[out] ulAddress Start address of audio .
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_System_GetAudioFWStartAddress(MMP_ULONG ulRegion, MMP_ULONG *ulAddress)
{
	MMP_ERR	mmpstatus;

    MMPH_HIF_CmdSetParameterL(0, ulRegion);
    MMPH_HIF_CmdSend(AUDIO_START | HIF_CMD_GET_FW_ADDR);
    mmpstatus = MMPC_System_CheckMMPStatus(SYSTEM_CMD_IN_EXEC);
    if (mmpstatus == MMP_ERR_NONE) {
	    *ulAddress = MMPH_HIF_CmdGetParameterL(0);
	}	    

	return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_System_CheckVersion
//  Description :
//------------------------------------------------------------------------------
/** @brief Check the version of the firmware and the MMPD library.

This function Get the compile-made version from the firmware and this function.
Then it displayed on the screen if possible.
@pre This function should be called after PLL setting so that the firmware could output the FW name correctly.
@return It reports the status of the operation. Now it always return success.
*/
#if PCAM_EN==0
MMP_ERR MMPD_System_CheckVersion(void)
{
    MMP_USHORT i;
    MMPD_SYSTEM_BUILD_VERSION buildVersion;
    MMP_ERR		mmpstatus;
    
    MMPH_HIF_CmdSend(HIF_CMD_VERSION_CONTROL | FW_RELEASE_TIME);
	mmpstatus = MMPC_System_CheckMMPStatus(SYSTEM_CMD_IN_EXEC);
    if (mmpstatus) {
	    return mmpstatus;
	}	        
    for (i = 0; i <= COMPILER_TIME_LEN; i++) {
        buildVersion.szTime[i] = MMPH_HIF_CmdGetParameterB(i);
    }

    MMPH_HIF_CmdSend(HIF_CMD_VERSION_CONTROL | FW_RELEASE_DATE);
	mmpstatus = MMPC_System_CheckMMPStatus(SYSTEM_CMD_IN_EXEC);
    if (mmpstatus) {
	    return mmpstatus;
	}	        

    for (i = 0; i <= COMPILER_DATE_LEN; i++) {
        buildVersion.szDate[i] = MMPH_HIF_CmdGetParameterB(i);
    }
    ///@warning might have compiler porting issue of it doesn't support compiler time and date string
    PRINTF("MMPD Library build: " __TIME__ " "__DATE__"\r\n");
    PRINTF("Firmware build: %s %s\r\n", buildVersion.szTime, buildVersion.szDate);
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_System_SendEchoCommand
//  Description :
//------------------------------------------------------------------------------
/** @brief Check the command interrupt still workable.

This function Get the command data with host parameter0 and can use to check HOST command interface.
@pre This function should be called after download firmware success.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_System_SendEchoCommand(void)
{
	u_int		timeout = 0;
	
	MMPH_HIF_CmdSend(HIF_CMD_ECHO | GET_INTERNAL_STATUS);
	do {
		timeout++;
	} while (MMPH_HIF_CmdGetStatusL() & SYSTEM_CMD_IN_EXEC && timeout < 0x10000);

	if (timeout >= 0x10000) {
		PRINTF("ECHO fail\r\n");
		return	MMP_SYSTEM_ERR_HW;
	}

	if (MMPH_HIF_CmdGetParameterW(0) != (HIF_CMD_ECHO | GET_INTERNAL_STATUS)) {
		PRINTF("ECHO fail\r\n");
		return	MMP_SYSTEM_ERR_HW;	
	}		
	
	return MMP_ERR_NONE;
}
#endif
 
//------------------------------------------------------------------------------
//  Function    : MMPD_System_ReadCoreID
//  Description :
//------------------------------------------------------------------------------
/** @brief Check the chip code ID

This function Get the code ID of the chip
@return It reports the status of the operation.
*/
MMP_UBYTE MMPD_System_ReadCoreID(void)
{
    AITPS_GBL pGBL = AITC_BASE_GBL;
    m_gbSystemCoreID = pGBL->GBL_CHIP_VER;
    RTNA_DBG_Str3("[SYS] : Core Id : ");
    RTNA_DBG_Byte3(m_gbSystemCoreID);
    RTNA_DBG_Str3("\r\n");
    return m_gbSystemCoreID;
}

/**
 @brief Fine tune MCI priority to fit VGA size encoding. It's an access issue.
 @param[in] ubMode Mode.
 @retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPD_System_TuneMCIPriority(MMP_UBYTE ubMode)
{
    MMP_ULONG ulBase = 0x80007700, addr;
    extern MMP_UBYTE gbCurH264Type ;

    RTNA_DBG_Str3("MCI mode : ");
    RTNA_DBG_Byte3(ubMode);
    RTNA_DBG_Str3("\r\n");
    dbg_printf(3,"<-- MCI mode : %d -->\r\n", ubMode );
     
#if 1// TEST CODE
    MMPH_HIF_RegSetB(ulBase + 0x2a, 0x1C); // DMA 1
    MMPH_HIF_RegSetB(ulBase + 0x31, 0x1D); // DMA 2
#endif
    if (ubMode == 3) {
        MMPH_HIF_RegSetB(ulBase + 0x11, MMPH_HIF_RegGetB(ulBase+0x11)|0xc0 ); // old : 0x80, 0xc0 : scaling up 
        
        #if 1// Add for H.264
        MMPH_HIF_RegSetB(ulBase + 0x2B, 0x1F); // h264e dm rd dram
        MMPH_HIF_RegSetB(ulBase + 0x2C, 0x1F); // h264e inter rd
        //MMPH_HIF_RegSetB(ulBase + 0x2D, 0x1F); // h264e vlc wr
        MMPH_HIF_RegSetB(ulBase + 0x2E, 0x1F); // h264e dblk wr
        #endif
        MMPH_HIF_RegSetB(ulBase + 0x35, 0x1E); // Graphic req for graphic busy.
        MMPH_HIF_RegSetB(ulBase + 0x29, 0x1D); // jpeg wr dram
        MMPH_HIF_RegSetB(ulBase + 0x2B, 0x1F); // h264e dm rd dram
        MMPH_HIF_RegSetB(ulBase + 0x27, 0x11); // jpeg compress r/w
        MMPH_HIF_RegSetB(ulBase + 0x24, 0x1F); // ibc 0
        MMPH_HIF_RegSetB(ulBase + 0x25, 0x1F); // ibc 1
//        MMPH_HIF_RegSetB(ulBase + 0x26, 0x1D); // ibc 2
        MMPH_HIF_RegSetB(ulBase + 0x26, 0x1F); // ibc 2
        
        #if 1 // Add for scaling up test
        // yuy2 1080p + h264 1080p , turn off or icon delay line broken
        MMPH_HIF_RegSetB(ulBase + 0x66, 0x7); // ibc 2 na
        MMPH_HIF_RegSetB(ulBase + 0x86, 0x7); // ibc 2 row hit
        MMPH_HIF_RegSetB(ulBase + 0xA6, 0x7); // ibc 2 conti
        #endif
        #if 1 // Add for scaling up test
        // yuy2 1080p + h264 1080p , turn off or icon delay line broken
        MMPH_HIF_RegSetB(ulBase + 0x64, 0x7); // ibc 0 na
        MMPH_HIF_RegSetB(ulBase + 0x84, 0x7); // ibc 0 row hit
        MMPH_HIF_RegSetB(ulBase + 0xA4, 0x11); // ibc 0 conti
        #endif
        
        
        #if 1 // for simulcast reset IBC 1
            MMPH_HIF_RegSetB(ulBase + 0x65, 0x11); // ibc 1 na
            MMPH_HIF_RegSetB(ulBase + 0x85, 0x11); // ibc 1 row hit
            MMPH_HIF_RegSetB(ulBase + 0xA5, 0x11); // ibc 1 conti
        #endif
        
        #if 1// HIGH profile turning 
	    ISP_IF_IQ_SetOpr(0x7078, 1, 2);
	    ISP_IF_IQ_SetOpr(0x707A, 1, 0x20);
        #endif
        
	}
    else if (ubMode == 2) {
        MMPH_HIF_RegSetB(ulBase + 0x11, MMPH_HIF_RegGetB(ulBase+0x11)|0xc0 ); // old : 0x80, 0xc0 : scaling up 
        
        #if 1// Add for H.264
        MMPH_HIF_RegSetB(ulBase + 0x2B, 0x1F); // h264e dm rd dram
        MMPH_HIF_RegSetB(ulBase + 0x2C, 0x1F); // h264e inter rd
        //MMPH_HIF_RegSetB(ulBase + 0x2D, 0x1F); // h264e vlc wr
        MMPH_HIF_RegSetB(ulBase + 0x2E, 0x1F); // h264e dblk wr
        #endif
        MMPH_HIF_RegSetB(ulBase + 0x35, 0x1E); // Graphic req for graphic busy.
        MMPH_HIF_RegSetB(ulBase + 0x29, 0x1D); // jpeg wr dram
        MMPH_HIF_RegSetB(ulBase + 0x2B, 0x1F); // h264e dm rd dram
        MMPH_HIF_RegSetB(ulBase + 0x27, 0x11); // jpeg compress r/w
        MMPH_HIF_RegSetB(ulBase + 0x24, 0x1F); // ibc 0
        MMPH_HIF_RegSetB(ulBase + 0x25, 0x1F); // ibc 1
//        MMPH_HIF_RegSetB(ulBase + 0x26, 0x1D); // ibc 2
        MMPH_HIF_RegSetB(ulBase + 0x26, 0x1F); // ibc 2
        



        if( UVCX_IsSimulcastH264() ) {
        #if 1 // Add for scaling up test
            // yuy2 1080p + h264 1080p , turn off or icon delay line broken
            if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                MMPH_HIF_RegSetB(ulBase + 0x66, 0x11); // ibc 2 na
                MMPH_HIF_RegSetB(ulBase + 0x86, 0x11); // ibc 2 row hit
                MMPH_HIF_RegSetB(ulBase + 0xA6, 0x11); // ibc 2 conti
                dbg_printf(0,"--ibc2 mci : frame \r\n");
            } else {    
            
                MMPH_HIF_RegSetB(ulBase + 0x66, 0x7); // ibc 2 na
                MMPH_HIF_RegSetB(ulBase + 0x86, 0x7); // ibc 2 row hit
                MMPH_HIF_RegSetB(ulBase + 0xA6, 0x7); // ibc 2 conti
                 dbg_printf(0,"--ibc2 mci : realtime \r\n");
            }
        
        #endif
        //MMPH_HIF_RegSetB(ulBase + 0x24, 0x1e);
        #if 1 // Add for scaling up test
            MMPH_HIF_RegSetB(ulBase + 0x65, 0x7); // ibc 1 na
            MMPH_HIF_RegSetB(ulBase + 0x85, 0x7); // ibc 1 row hit
            MMPH_HIF_RegSetB(ulBase + 0xA5, 0x7); // ibc 1 conti
        #endif
        
        
        }
        else {
        #if 1 // Add for scaling up test
            // yuy2 1080p + h264 1080p , turn off or icon delay line broken
            MMPH_HIF_RegSetB(ulBase + 0x66, 0x7); // ibc 2 na
            MMPH_HIF_RegSetB(ulBase + 0x86, 0x7); // ibc 2 row hit
            MMPH_HIF_RegSetB(ulBase + 0xA6, 0x7); // ibc 2 conti
        #endif
        #if 1 // for simulcast reset IBC 1
            MMPH_HIF_RegSetB(ulBase + 0x65, 0x11); // ibc 1 na
            MMPH_HIF_RegSetB(ulBase + 0x85, 0x11); // ibc 1 row hit
            MMPH_HIF_RegSetB(ulBase + 0xA5, 0x11); // ibc 1 conti
        #endif
        }
        #if 1// HIGH profile turning 
	    ISP_IF_IQ_SetOpr(0x7078, 1, 2);
	    ISP_IF_IQ_SetOpr(0x707A, 1, 0x20);
        #endif
        
	}
	else if (ubMode == 1) {
        for (addr = ulBase+0x20; addr <= (ulBase+0x36); addr++) {
            MMPH_HIF_RegSetB(addr, 0x50);
        }
        MMPH_HIF_RegSetB(ulBase + 0x11, MMPH_HIF_RegGetB(ulBase+0x11)&~0x80);
	}

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_System_GetGroupFreq
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the group freq.
@param[in]  ubGroup Select the group.
@param[out] ulGroupFreq The current group freq.
@return Always return success.
*/
#if PCAM_EN==0
MMP_ERR  MMPD_System_GetGroupFreq(MMP_UBYTE ubGroup, MMP_ULONG *ulGroupFreq)
{
	MMP_ERR	mmpstatus;

	MMPH_HIF_CmdSetParameterL(0, ubGroup);
	MMPH_HIF_CmdSend(HIF_CMD_SYSTEM_GET_GROUP_FREQ);
	mmpstatus = MMPC_System_CheckMMPStatus(SYSTEM_CMD_IN_EXEC);
	if (mmpstatus == MMP_ERR_NONE) 
		*ulGroupFreq = MMPH_HIF_CmdGetParameterL(4);
    
    return  mmpstatus;
}
#endif




/// @}
/// @end_ait_only

#ifdef BUILD_CE
#define BUILD_FW
#endif

