//==============================================================================
//
//  File        : mmpf_system.c
//  Description : MMPF_SYS functions
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"

#include "mmpf_hif.h"
#include "mmpf_pll.h"
#include "mmpf_dma.h"
#include "mmpf_display.h"
#include "mmpf_system.h"
#if (defined(ALL_FW)||defined(UPDATER_FW))&&(SYS_SELF_SLEEP_ENABLE == 0x1)
#include "mmpf_audio_ctl.h"
#include "mmp_reg_audio.h"
#include "mmp_reg_vif.h"
#include "mmpf_pio.h"
#include "mmp_reg_usb.h"
#include "mmpf_dram.h"
#endif
#include "mmp_reg_display.h"
#include "mmpf_msg.h"
#include "mmps_system.h"
#include "mmpf_boot.h"
#include "mmpf_usbvend.h"
#include "ucos_ii.h"
#include "mmpf_usbuvc.h"
#include "mmpf_wd.h"

#include "mmpf_usbuvch264.h"

/** @addtogroup MMPF_SYS
@{
*/
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
OS_STK                  SYS_Task_Stk[TASK_SYS_STK_SIZE]; // put into sram
#if WATCHDOG_RESET_EN==1
OS_STK                  LTASK_Task_Stk[LTASK_STK_SIZE];
#endif

extern MMPF_OS_FLAGID   SYS_Flag_Hif;
extern MMPF_PLL_MODE    gPLLMode;
/// @deprecated Customer and project ID
extern MMP_CUSTOMER  gbCustomer;
/// @brief Human-maintained release version
extern MMP_RELEASE_VERSION gbFwVersion;

extern MMP_BOOL    gbHasInitAitDAC;

MMP_UBYTE	m_gbSystemCoreID;
MMP_USHORT	m_gsISPCoreID;
#if ((DSC_R_EN)&&(FDTC_SUPPORT == 1))
extern MMPF_OS_FLAGID   FDTC_Flag;
#include "mmpf_fdtc.h"
#endif
#if (APP_EN)
extern MMPF_OS_FLAGID   APP_Flag;
#include "mmpf_appapi.h"
#endif


//==============================================================================
//
//                         MODULE VARIABLES
//
//==============================================================================
/// @brief Compiler build version
MMP_SYSTEM_BUILD_VERSION mmpf_buildVersion = {__DATE__, __TIME__};

/// @brief Define the firmware name to be printed for debugging
#if defined (ALL_FW)
	#define FIRMWARE_NAME "ALL"
#else
    #define FIRMWARE_NAME "UNKNOWN FW (Modify mmpf_system.c)"
#endif

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================
#pragma arm section code = "EnterSelfSleepMode", rwdata = "EnterSelfSleepMode",  zidata = "EnterSelfSleepMode"
MMP_ERR MMPF_SYS_SwitchVCore(MMPF_SYS_VCORETYPE vcore)
{
// CGPIO17 : GPIO59
// Low : Low Voltage
// High : High Voltage
    static AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	static AITPS_GBL   pGBL = AITC_BASE_GBL;
	static MMPF_PIO_REG piopin = MMPF_PIO_REG_GPIO59 ;
	static MMP_UBYTE bitPostionInfo;// = piopin & PIO_BITPOSITION_INFO;
	static MMP_UBYTE IndexAddressInfo ;//= PIO_GET_INDEX(piopin);
	bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	IndexAddressInfo = PIO_GET_INDEX(piopin);
	pGBL->GBL_IO_CTL12 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO56));
	if(vcore==MMPF_SYS_SLEEP_VCORE) {
	// Low
	    pGPIO->GPIO_DATA[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));        
	}
	else {
	// High
	    pGPIO->GPIO_DATA[IndexAddressInfo] |= (0x1 << bitPostionInfo);
	}
	// Enable
	pGPIO->GPIO_EN[IndexAddressInfo] |= (0x1 << bitPostionInfo);
    return MMP_ERR_NONE;
}
#pragma arm section code, rwdata,  zidata

//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_ResetSystem
//  Description :
//------------------------------------------------------------------------------
/** @brief The function is used to reset system and system re-run from ROM code.

 The function is used to reset system and system re-run from ROM code.
@return It reports the status of the operation.
*/
#pragma O0
MMP_ERR	MMPF_SYS_ResetSystem(MMP_UBYTE ubSpecialCase)
{
#if !defined(UPDATER_FW)
	//AITPS_GBL pGBL = AITC_BASE_GBL;
	#if (OS_CRITICAL_METHOD == 3)
	OS_CPU_SR   cpu_sr = 0;
	#endif
	
	#if (CHIP == VSN_V2)
	MMP_ULONG i;
	void (*FW_Entry)(void) = NULL;
	volatile 	MMP_UBYTE *ROM_BASE_B =   (volatile MMP_UBYTE *)0x80002000; //used for copy ROM
	volatile 	MMP_UBYTE *ROM_BASE_B_2 = (volatile MMP_UBYTE *)0x80003000;	//used for copy ROM
	MMP_ULONG ulTcmSize = 0x1000, ulSramSize = 0x3000;
	volatile 	MMP_UBYTE *TCM_BASE_B = (volatile MMP_UBYTE *)0x0;			
	volatile 	MMP_UBYTE *SRAM_BASE_B = (volatile MMP_UBYTE *)0x100000;
	#endif
	#if (CHIP == VSN_V3)
	void (*FW_Entry)(void) = NULL;
	AITPS_WD 	pWD = AITC_BASE_WD;
	MMP_ULONG counter = 0x0;
	#endif
	
	AITPS_GBL   pGBL = AITC_BASE_GBL;
	
	OS_ENTER_CRITICAL();
	
	#if (SYS_WD_ENABLE == 0x1)
	MMPF_WD_Kick();
	#endif
	
  	if(ubSpecialCase != 0x1) {
  		MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_VI, MMP_TRUE);
  	}
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_ISP, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_JPG, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_SCAL, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_GPIO, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_AUD, MMP_TRUE);
    
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_RAW, MMP_TRUE);
  	MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_DMA, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_I2C, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_USB, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_H264, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_IBC, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_GRA, MMP_TRUE);
    
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_SD, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_PWM, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_PSPI, MMP_TRUE);
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_USB_PHY, MMP_TRUE);
	MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_CPU_SRAM, MMP_TRUE);
	MMPF_MMU_FlushDCache();
	AT91F_DisableDCache();
	AT91F_DisableICache();
	AT91F_DisableMMU();
	
	
	pGBL->GBL_CLK_DIS0 = (MMP_UBYTE)(~(GBL_CLK_MCI_DIS | GBL_CLK_VI_DIS| GBL_CLK_JPG_DIS | GBL_CLK_CPU_DIS | GBL_CLK_GPIO_DIS));
	pGBL->GBL_CLK_DIS1 = (MMP_USHORT)(~(GBL_CLK_DRAM_DIS | GBL_CLK_PWM_DIS | GBL_CLK_I2C_DIS | GBL_CLK_DMA_DIS | GBL_CLK_USB_DIS | GBL_CLK_CPU_PHL_DIS));
	#if (CHIP == VSN_V3)
	pGBL->GBL_CLK_DIS2 = (MMP_UBYTE)(~(GBL_CLK_CIDC_DIS | GBL_CLK_GNR_DIS | GBL_CLK_COLOR_DIS)) ;
	#endif
	
	
	pGBL->GBL_CLK_DIS1 &= ~GBL_CLK_BS_SPI_DIS;
	#if (CHIP == VSN_V2)
	for(i = 0; i < ulTcmSize; i++) {
		TCM_BASE_B[i] = ROM_BASE_B[i];
	}
	
	for(i = 0; i < ulSramSize; i++) {
		SRAM_BASE_B[i] = ROM_BASE_B_2[i];
	}
	
	FW_Entry = (void (*)(void))(0x0);
	FW_Entry();	//enter the firmware entry
	while(1) {
	
	}
	#endif
	
	#if (CHIP == VSN_V3)
	//Change the Boot Strapping as ROM boot
	pGBL->GBL_CHIP_CTL |= MOD_BOOT_STRAP_EN;
    pGBL->GBL_CHIP_CFG = ROM_BOOT_MODE;
    pGBL->GBL_CHIP_CTL &= ~MOD_BOOT_STRAP_EN;
	
	//Turn-off watch dog
 	if((pWD->WD_MODE_CTL0 & WD_EN)!= 0x0) {
 		RTNA_DBG_Str(0, "\r\nTurn-off WD !!\r\n");
  	    pWD->WD_MODE_CTL0 = 0x2340;
 	}
	
	//VSN_V3, CPU access ROM code have HW bug, so we use reset ROM controller to archieve this purpose. 
	//Note: The busy waiting is necessary !!!  ROM controller need some time to re-load ROM code.
	MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_CPU_PERIF, MMP_FALSE);
	for(counter = 0x3FFFF; counter > 0 ; counter --) {
	}
	
	if(ubSpecialCase == 0x1) {
		FW_Entry = (void (*)(void))(0x0);
		FW_Entry();	//enter the firmware entry
		while(1);
	}
	
	
	//Finally, use watch-dog do timeout-reset, this purpose is to reset PLL as normal speed for serial-flash acessing issue.
	MMPF_WD_SetTimeOut(31, 128);
	MMPF_WD_EnableWD(MMP_TRUE, MMP_TRUE, MMP_FALSE, NULL, MMP_TRUE);
	#endif
	
	
	
	
	
	#if 0  //The following test code is used CPU watch dog to archieve system reset
	//Change the Boot Strapping as ROM boot
	pGBL->GBL_CHIP_CTL |= MOD_BOOT_STRAP_EN;
    pGBL->GBL_CHIP_CFG = ROM_BOOT_MODE;
    pGBL->GBL_CHIP_CTL &= ~MOD_BOOT_STRAP_EN;
    
	MMPF_WD_Initialize();
	
	MMPF_WD_SetTimeOut(31, 128);
	
	MMPF_WD_EnableWD(MMP_TRUE, MMP_TRUE, MMP_FALSE, NULL, MMP_TRUE);
	#endif
#endif //#if !defined(UPDATER_FW)
	return MMP_ERR_NONE;
}
#pragma

//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_ResetHModule
//  Description :
//------------------------------------------------------------------------------
/** @brief The function is used to reset HW modules.

The function is used to reset HW modules. 
@param[in] moduletype is used to select which modules to reset.
@param[in] bResetRegister is used enable/disable reset module's registers. 
@return It reports the status of the operation.
*/
MMP_ERR MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE moduletype, MMP_BOOL bResetRegister)
{
	AITPS_GBL pGBL = AITC_BASE_GBL;
	
	if(moduletype <= MMPF_SYS_HMODULETYPE_GRA) {
		if(bResetRegister) {
			pGBL->GBL_RST_REG_EN |= (0x1 << moduletype);
		}
		pGBL->GBL_RST_CTL01 |= (0x1 << moduletype);
		MMPF_PLL_WaitCount(0x20);  //Note: 0x20 is just the test value used in PCAM project
		pGBL->GBL_RST_CTL01 &= ~(0x1 << moduletype);
		if(bResetRegister) {
			pGBL->GBL_RST_REG_EN &= ~(0x1 << moduletype);
		}
	}
	else {
		MMP_UBYTE	ubModuleType1 = (moduletype - MMPF_SYS_HMODULETYPE_CPU_SRAM);
		if((bResetRegister) && (ubModuleType1 > 0) && (ubModuleType1 > 0) && (ubModuleType1 < 4)) {
			pGBL->GBL_RST_CTL2 |= (0x1 << (ubModuleType1 + 4));
		}
		pGBL->GBL_RST_CTL2 |= (0x1 << ubModuleType1);
		MMPF_PLL_WaitCount(0x20);
		pGBL->GBL_RST_CTL2 &= ~(0x1 << ubModuleType1);
		if((bResetRegister) && (ubModuleType1 > 0) && (ubModuleType1 > 0) && (ubModuleType1 < 4)) {
			pGBL->GBL_RST_CTL2 &= ~(0x1 << (ubModuleType1 + 4));
		}
	}
	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_SetWakeUpEvent
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set the wake up event when CPU enter sleep

The function set the PIN as Output mode (bEnable = MMP_TRUE) or Input mode.
@param[in] bEnable is used to turn on/off wake up event.
@param[in] event is used to select wake up event type. 
@param[in] piopin is the PIO number, please reference the data structure of MMPF_PIO_REG
@param[in] polarity is used for GPIO event wake up. To set high(0)/low(1) level wake up.
@return It reports the status of the operation.
*/
MMP_ERR MMPF_SYS_SetWakeUpEvent(MMP_BOOL bEnable, MMPF_WAKEUP_EVNET event, MMPF_PIO_REG piopin, MMP_UBYTE polarity)
{
#if (defined(ALL_FW)||defined(UPDATER_FW))&&(SYS_SELF_SLEEP_ENABLE == 0x1)
	MMP_UBYTE ubShiftBit = 0x0;
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	
	if(bEnable == MMP_TRUE) {
		if(event == MMPF_WAKEUP_GPIO) {	
			switch(piopin) {
				case MMPF_PIO_REG_GPIO0:
					ubShiftBit = GBL_WAKE_GPIO0;
					break;
				case MMPF_PIO_REG_GPIO6:
					ubShiftBit = GBL_WAKE_GPIO6;
					break;
				#if (CHIP == VSN_V3)
				case MMPF_PIO_REG_GPIO23:
					ubShiftBit = GBL_WAKE_GPIO23;
					break;
				case MMPF_PIO_REG_GPIO50:
					ubShiftBit = GBL_WAKE_GPIO50;
					break;
				case MMPF_PIO_REG_GPIO63:
					ubShiftBit = GBL_WAKE_GPIO63;
					break;
				#endif
				default:
					return MMP_SYSTEM_ERR_FORMAT;
					break; 
			}
			
			MMPF_PIO_Initialize();
			MMPF_PIO_EnableGpioMode(piopin, MMP_TRUE);	
			MMPF_PIO_EnableOutputMode(piopin, MMP_FALSE);
				
			if(polarity == 0x1) { //High level wake up
				pGBL->GBL_GPIO_WAKE_INT_POLARITY &= (~ubShiftBit);
			}
			else { //Low level wake up
				pGBL->GBL_GPIO_WAKE_INT_POLARITY |= (ubShiftBit);
			}
			
			pGBL->GBL_WAKE_CTL |= ubShiftBit;
		}
		else if(event == MMPF_WAKEUP_USB_RESUME) {
			if((pUSB_CTL->USB_POWER & 0x1) == 0x0) {
				pUSB_CTL->USB_POWER |= 0x1;
			}
			pGBL->GBL_WAKE_CTL |= GBL_WAKE_USB_RESUME;
		}
		else {
			return MMP_SYSTEM_ERR_FORMAT;
		} 
	}
	else {
		if(event == MMPF_WAKEUP_GPIO) {
			switch(piopin) {
				case MMPF_PIO_REG_GPIO0:
					ubShiftBit = GBL_WAKE_GPIO0;
					break;
				case MMPF_PIO_REG_GPIO6:
					ubShiftBit = GBL_WAKE_GPIO6;
					break;
				#if (CHIP == VSN_V3)
				case MMPF_PIO_REG_GPIO23:
					ubShiftBit = GBL_WAKE_GPIO23;
					break;
				case MMPF_PIO_REG_GPIO50:
					ubShiftBit = GBL_WAKE_GPIO50;
					break;
				case MMPF_PIO_REG_GPIO63:
					ubShiftBit = GBL_WAKE_GPIO63;
					break;
				#endif
				default:
					return MMP_SYSTEM_ERR_FORMAT;
					break; 
			}

			MMPF_PIO_EnableGpioMode(piopin, MMP_FALSE);
			pGBL->GBL_WAKE_CTL &= ~ubShiftBit;
		}
		else if(event == MMPF_WAKEUP_USB_RESUME) {
			pGBL->GBL_WAKE_CTL &= ~GBL_WAKE_USB_RESUME;
		}
		else {
			return MMP_SYSTEM_ERR_FORMAT;
		} 
	}
#else
	RTNA_DBG_Str(0, "CPU wake up event un-support ! \r\n");
#endif
	return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_EnterSelfSleepMode
//  Description :
//------------------------------------------------------------------------------
/** @brief This function is used to do some operations before CPU sleep and make CPU sleep.
           After CPU wake up, the CPU start to run the code after it enters sleep.
           
    NOTE1: This function should co-work with MMPF_SYS_EnterPSMode !!!!
    NOTE2: It can not be placed into DRAM because DRAM will enter slef refresh mode !!
        
@return It reports the status of the operation.
*/
#pragma arm section code = "EnterSelfSleepMode", rwdata = "EnterSelfSleepMode",  zidata = "EnterSelfSleepMode"
MMP_ERR MMPF_SYS_EnterSelfSleepMode(void)
{
	
#if (defined(ALL_FW)||defined(UPDATER_FW))&&(SYS_SELF_SLEEP_ENABLE == 0x1)
	#if (OS_CRITICAL_METHOD == 3)
	static OS_CPU_SR   cpu_sr = 0;
	#endif
	static MMP_UBYTE    reset_sig;
	static MMP_UBYTE    ubClkDis0 = 0x0, ubClkDis2 = 0x0;
	static MMP_USHORT   usClkDis1 = 0x0;
	static MMP_ULONG    counter = 0, ulTempISR = 0x0;
	static AITPS_AIC 	pAIC = AITC_BASE_AIC;
	static AITPS_GBL    pGBL = AITC_BASE_GBL;
	static AITPS_USB_CTL pUSB = AITC_BASE_USBCTL ;
	//RTNA_DBG_Str(0, "Enter Sleep mode \r\n");
	static MMP_USHORT  usb_int ;// = pUSB_CTL->USB_INT_EVENT_SR & pUSB_CTL->USB_INT_EVENT_EN ;
	OS_ENTER_CRITICAL();
	ulTempISR = pAIC->AIC_IMR;
	pAIC->AIC_IDCR = 0xFFFFFFFF;
	
	#if (CHIP == VSN_V2)
	MMPF_SYS_SetWakeUpEvent(MMP_TRUE, MMPF_WAKEUP_GPIO, MMPF_PIO_REG_GPIO6, 0x0);
	#endif
	#if (CHIP == VSN_V3)
	//MMPF_SYS_SetWakeUpEvent(MMP_TRUE, MMPF_WAKEUP_GPIO, MMPF_PIO_REG_GPIO23, 0x1);
	#endif
	MMPF_SYS_SetWakeUpEvent(MMP_TRUE, MMPF_WAKEUP_USB_RESUME, 0x0, 0x0);
	
	ubClkDis0 = pGBL->GBL_CLK_DIS0;
	usClkDis1 = pGBL->GBL_CLK_DIS1;
	#if (CHIP == VSN_V3)
	ubClkDis2 = pGBL->GBL_CLK_DIS2;
	#endif
	
	MMPF_DRAM_SetSelfRefresh(MMP_TRUE);
	// ?????
	//if( (pUSB->USB_POWER & 0x02) == 0 ) {
	//    reset_sig = 1 ;
	//    goto exit_ps ;    
	//}
	
	usb_int = pUSB->USB_INT_EVENT_SR & pUSB->USB_INT_EVENT_EN ;
	
	if( usb_int & (RESUME_INT_BIT | RESET_INT_BIT)) {
	    MMPF_SYS_SwitchVCore(MMPF_SYS_ACTIVE_VCORE); // Fixed bug when rapid exit suspend
	    reset_sig = 1 ;
	    goto exit_ps ;
	}
		
	pGBL->GBL_CLK_DIS0 = (MMP_UBYTE)(~(GBL_CLK_MCI_DIS | GBL_CLK_CPU_DIS | GBL_CLK_GPIO_DIS));
	pGBL->GBL_CLK_DIS1 = (MMP_USHORT)(~(GBL_CLK_DRAM_DIS | GBL_CLK_USB_DIS | GBL_CLK_CPU_PHL_DIS));
	#if (CHIP == VSN_V3)
	pGBL->GBL_CLK_DIS2 = (MMP_UBYTE)(GBL_CLK_AUD_CODEC_DIS | GBL_CLK_CIDC_DIS | GBL_CLK_GNR_DIS | GBL_CLK_SM_DIS | GBL_CLK_COLOR_DIS); 
	#endif
		
	pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_ENTER_SELF_SLEEP_MODE;

	//When set 0x8F88 enter sleep mode, CPU still alive in a short time
	//So, need to insert some useless commands before setting 0x8F88 as zero
	#pragma O0
	for(counter = 0x1000; counter > 0 ; counter --) {
	}
	#pragma
		
	pGBL->GBL_LCD_BYPASS_CTL0 &= (~GBL_ENTER_SELF_SLEEP_MODE);
	MMPF_SYS_SwitchVCore(MMPF_SYS_ACTIVE_VCORE);
	pGBL->GBL_CLK_DIS0 = ubClkDis0;
	pGBL->GBL_CLK_DIS1 = usClkDis1;
	#if (CHIP == VSN_V3)
	pGBL->GBL_CLK_DIS2 = ubClkDis2; 
	#endif
	
exit_ps:
	
	MMPF_DRAM_SetSelfRefresh(MMP_FALSE);

	#if (CHIP == VSN_V2)
	MMPF_SYS_SetWakeUpEvent(MMP_FALSE, MMPF_WAKEUP_GPIO, MMPF_PIO_REG_GPIO6, 0x0);
	#endif
	#if (CHIP == VSN_V3)
	//MMPF_SYS_SetWakeUpEvent(MMP_FALSE, MMPF_WAKEUP_GPIO, MMPF_PIO_REG_GPIO23, 0x1);
	#endif
	MMPF_SYS_SetWakeUpEvent(MMP_FALSE, MMPF_WAKEUP_USB_RESUME, 0x0, 0x0);
	
	pAIC->AIC_IECR = ulTempISR;
	OS_EXIT_CRITICAL();
	
	if(reset_sig) {
        //VAR_B(0,USB_REG_BASE_B[USB_POWER_B] );
	    RTNA_DBG_Str(0, "-ExPS.Reset-\r\n");
	} else {
	    RTNA_DBG_Str(0, "-ExPS-\r\n");
	}
	
	#else
	RTNA_DBG_Str(0, "CPU slef sleep mode un-support ! \r\n");
#endif
	return MMP_ERR_NONE;

}
#pragma arm section code, rwdata,  zidata
//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_EnterPSMode
//  Description : called by the task with host command queue to process
//                command for System group
//------------------------------------------------------------------------------
// 0 : Exit Power Saving Mode
// 1 : Enter Power Saving Mode
// 2 : Power On Initial the unused Moudle
MMP_ERR MMPF_SYS_EnterPSMode(MMP_BOOL bEnterPSMode)
{
#define DISABLE_USB_PHY (0) // Change back to 0 (never set 1 now)
extern MMP_UBYTE  gbUsbHighSpeed;
	#if (defined(ALL_FW)||defined(UPDATER_FW))&&(SYS_SELF_SLEEP_ENABLE == 0x1)
	AITPS_GBL   pGBL = AITC_BASE_GBL;
	AITPS_USB_DMA   pUSBDMA = AITC_BASE_USBDMA;
	AITPS_MIPI pMIPI = AITC_BASE_MIPI;
	volatile MMP_UBYTE* REG_BASE_B = (volatile MMP_UBYTE*)0x80000000;
	static MMP_BOOL ubMipiUseLan0 = MMP_FALSE, ubMipiUseLan1 = MMP_FALSE;
	
	pGBL->GBL_CLK_DIS0 &= ~( GBL_CLK_VI_DIS | GBL_CLK_AUD_DIS )  ;
	 
	if(bEnterPSMode == MMP_TRUE) {
	
    	#if (CHIP==VSN_V3)
        {
            AITPS_AFE   pAFE    = AITC_BASE_AFE;
            pAFE->AFE_ADC_CTL_REG4 &= (~ADC_MIC_BIAS_ON);
    	}
    	#endif
		//MIPI RX
		REG_BASE_B[0x6160] &= 0xFB; // MIPI DPHY0 use HW design OPR to control Power-down
		REG_BASE_B[0x6170] &= 0xFB;  // MIPI DPHY1 use HW design OPR to control Power-down
        #if DISABLE_USB_PHY==1
        //USB PHY
        //sean@2012_09_12,for FS suspend/resume test 
        if(gbUsbHighSpeed) {
            SPI_Write(0x00, 0x0B00);
            SPI_Write(0x0A, 0x8520); 
            SPI_Write(0x0C, 0xff40); // 0xff40
        }
        #endif
        
        
        
#if  SUSPEND_LEVEL==0
    	pGBL->GBL_LCD_BYPASS_CTL0 &= ~GBL_LCD_BYPASS_PWN_DPLL;
        pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_LCD_BYPASS_CLK_ACTIVE;
        pGBL->GBL_LCD_BYPASS_CTL0 &= ~GBL_XTAL_OFF_BYPASS;
#elif SUSPEND_LEVEL==1
    	pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_LCD_BYPASS_PWN_DPLL;
        pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_LCD_BYPASS_CLK_ACTIVE;
        pGBL->GBL_LCD_BYPASS_CTL0 &= ~GBL_XTAL_OFF_BYPASS;
#elif SUSPEND_LEVEL==2
    	pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_LCD_BYPASS_PWN_DPLL;
        pGBL->GBL_LCD_BYPASS_CTL0 &= ~GBL_LCD_BYPASS_CLK_ACTIVE;
        pGBL->GBL_LCD_BYPASS_CTL0 &= ~GBL_XTAL_OFF_BYPASS;
#elif SUSPEND_LEVEL==3        
		pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_LCD_BYPASS_PWN_DPLL;
		pGBL->GBL_LCD_BYPASS_CTL0 &= (~ GBL_LCD_BYPASS_CLK_ACTIVE);
		pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_XTAL_OFF_BYPASS;
#endif		
		MMPF_SYS_SwitchVCore(MMPF_SYS_SLEEP_VCORE);
	}
	else {
	
		pGBL->GBL_LCD_BYPASS_CTL0 &= (~GBL_LCD_BYPASS_PWN_DPLL);
		pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_LCD_BYPASS_CLK_ACTIVE;
		pGBL->GBL_LCD_BYPASS_CTL0 &= (~GBL_XTAL_OFF_BYPASS);
		//USB PHY
        #if DISABLE_USB_PHY==1
        //USB PHY
        if(gbUsbHighSpeed) {
            SPI_Write(0x0C, 0x0000);
            SPI_Write(0x0A, 0x0020);
            SPI_Write(0x00, 0x0300);
            RTNA_WAIT_US(100); // wait 100 us for 0x0B
        }
        #endif
        
        
    	#if (CHIP==VSN_V3)
        {
            AITPS_AFE   pAFE    = AITC_BASE_AFE;
            pAFE->AFE_ADC_CTL_REG4 |= ADC_MIC_BIAS_ON;
    	}
    	#endif
    
          
	}
	pGBL->GBL_CLK_DIS0 |= ( GBL_CLK_VI_DIS | GBL_CLK_AUD_DIS);
	
	#endif //#if defined(ALL_FW)
	return  MMP_ERR_NONE;

}

MMP_ERR MMPF_SYS_ConfigIOPad(void)
{
#if (CHIP==P_V2)
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) (0x80000000);
    // Turn off LED.
    AITPS_GBL pGBL = AITC_BASE_GBL ;
    #if 1
    // PWM mode : LED & Motor switch to GPIO mode
    pGBL->GBL_PWM_1_IO_CTL &= ~0x8;  // PWM1 => BGPIO9
    pGBL->GBL_PWM_0_IO_CTL &= ~0x1;  // PWM0 => CGPIO8
    // enable BGPIO9/CGPIO8 as output
    // enable output
    REG_BASE_B[0x9401] |= 0x02 ;
    REG_BASE_B[0x9405] |= 0x01 ;
    // output low
    REG_BASE_B[0x9411] &= ~0x02 ;  
    REG_BASE_B[0x9415] &= ~0x01 ;  
    #endif
    
#if 1 // JTAG , Pull High
    
    REG_BASE_B[0x9D0A]=0x24;
    REG_BASE_B[0x9D0B]=0x24;
    REG_BASE_B[0x9D0C]=0x24;
    REG_BASE_B[0x9D0D]=0x24;
    REG_BASE_B[0x9D0E]=0x24;
    REG_BASE_B[0x9D0F]=0x24;

#endif

#endif
    
#if (CHIP==VSN_V3)
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) (0x80000000);

    AITPS_GBL pGBL = AITC_BASE_GBL ;
    #if 1
    // PWM mode : LED & Motor switch to GPIO mode
    //pGBL->GBL_CLK_DIS0 &= ~GBL_CLK_GPIO_DIS;	// Enable GPIO clock
    pGBL->GBL_CLK_DIS1 |= GBL_CLK_PWM_DIS;		// Disable PWM clock
	pGBL->GBL_IO_CTL2 &= ~(GBL_PWM1_2_PB_GPIO11|GBL_PWM2_2_PB_GPIO13);
    
    // enable BGPIO11/13 as output
    REG_BASE_B[0x692C] |= 0x28 ;
    // enable output
    REG_BASE_B[0x6602] |= 0x28 ;
    // output low
    REG_BASE_B[0x6612] &= ~0x28 ;        
    #endif
    
	#if 1 // JTAG , Pull High    
    REG_BASE_B[0x5DE0]=0x24;
    REG_BASE_B[0x5DE1]=0x24;
    REG_BASE_B[0x5DE2]=0x24;
    REG_BASE_B[0x5DE3]=0x24;
    REG_BASE_B[0x5DE4]=0x24;
	#endif
	//dbg_printf(3, "disable PWM\r\n");
	
	// Disable UART 0 TX
	//pGBL->GBL_IO_CTL1  &= ~GBL_UART_TX_PAD0;
	//MMPF_PIO_EnableGpioMode(MMPF_PIO_REG_GPIO0, MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(MMPF_PIO_REG_GPIO0, MMP_TRUE);
	//MMPF_PIO_SetData(MMPF_PIO_REG_GPIO0, MMP_FALSE);	
	
	
#endif    
    
    return MMP_ERR_NONE ;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_EnterBypassMode
//  Description : called by the task with host command queue to process
//                command for System group
//------------------------------------------------------------------------------
MMP_ERR MMPF_SYS_EnterBypassMode(MMP_BOOL bEnterBypassMode)
{
    AITPS_GBL   pGBL = AITC_BASE_GBL;

    if (bEnterBypassMode) {
        pGBL->GBL_LCD_BYPASS_CTL0 &= (~GBL_LCD_BYPASS_PWN_DPLL);
        pGBL->GBL_LCD_BYPASS_CTL0 |= GBL_LCD_BYPASS_CLK_ACTIVE;
    }
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_SuspendCPU
//  Description : called by the task with host command input to suspend
//                the OS timer
//------------------------------------------------------------------------------
MMP_ERR MMPF_SYS_SuspendCPU(MMP_BOOL bSuspend)
{
    AITPS_AIC   pAIC = AITC_BASE_AIC;

    if (bSuspend) {
		RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_TC0);
    }
    else {
		RTNA_AIC_IRQ_En(pAIC, AIC_SRC_TC0);
    }

    return  MMP_ERR_NONE;
}

#include "mmpf_ibc.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_jpeg.h"

MMP_ERR MMPF_SYS_ResetSensor(MMP_UBYTE ep_id)
{
extern MMP_USHORT gsCurFrameRatex10[];
extern MMP_USHORT  gsCurrentSensor ;
    MMP_ERR err ;
    
    MMP_BOOL  ibc[MMPF_IBC_PIPE_MAX] ,jpeg_done;
    STREAM_CFG *cur_stream = usb_get_cur_image_pipe_by_epid(ep_id);
   
    MMP_UBYTE pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_stream->pipe_cfg);
    AITPS_JPG   pJPG = AITC_BASE_JPG;
        
    dbg_printf(0,"*pipe_cfg : %d\r\n",pipe0_cfg);
    if(pipe0_cfg!=PIPE_CFG_MJPEG) {
        MMPF_IBC_Reset(ibc);
    }
    else {
        pJPG->JPG_INT_CPU_SR  = JPG_INT_ENC_DONE ;
        pJPG->JPG_INT_CPU_EN &= ~JPG_INT_ENC_DONE ;
    
    }
    
    // gating sensor data
    MMPF_VIF_EnableInputInterface(MMP_FALSE);

    //
    // power down sensor
    MMPF_Sensor_PowerDown(gsCurrentSensor,SEN_POWER_OFF);
    // initialize sensor
    
    if(gsCurFrameRatex10[0] > 300) {
        MMPS_Sensor_Initialize(gsCurrentSensor, 0, 2);
    } else {
        MMPS_Sensor_Initialize(gsCurrentSensor, 1, 2);
    }
    
    MMPF_VIF_EnableOutput( MMP_FALSE); //Gason@1228, uv issue
    MMPF_VIF_EnableInput(MMP_TRUE);   //Gason@1228, uv issue
    MMPF_VIF_EnableIntMode( MMP_TRUE); //Gason@1228, uv issue
    err = MMPF_Sensor_CheckFrameStart(1);
    if(err==0) {
	    MMPF_VIF_EnableInput( MMP_FALSE);
	    MMPF_VIF_EnableIntMode( MMP_FALSE );
        if(pipe0_cfg!=PIPE_CFG_MJPEG) {  
            MMPF_IBC_Restore(ibc);
        }
    
    
    }
    //dbg_printf(0,"*EpId : %d\r\n",ep_id);
     
    
    
    // enable sensor data
    MMPF_VIF_EnableInputInterface(MMP_TRUE);
    MMPF_OS_Sleep(100);
    cur_stream->pipe_b[0].total_rd = 0 ;
    cur_stream->pipe_b[0].total_wr = 0 ;
    cur_stream->pipe_b[0].wr_index = 0 ;
    cur_stream->pipe_b[0].rd_index = 0 ;
    if(pipe0_cfg==PIPE_CFG_MJPEG) {
         pJPG->JPG_CTL |= JPG_ENC_EN ;
         while(pJPG->JPG_CTL & JPG_ENC_EN) {
            dbg_printf(0,"J");
         }
        pJPG->JPG_INT_CPU_SR  = JPG_INT_ENC_DONE ;
        pJPG->JPG_INT_CPU_EN  = JPG_INT_ENC_DONE ;
        MMPF_Video_EnableJpeg();
    }
    
    
}


//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_InitializeHIF
//  Description : Initialize host command ISR
//------------------------------------------------------------------------------
#if PCAM_EN==0

MMP_ERR MMPF_SYS_InitializeHIF(void)
{
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_GBL   pGBL = AITC_BASE_GBL;

    pGBL->GBL_HOST2CPU_INT_SR = GBL_HOST2CPU_INT;
    pGBL->GBL_HOST2CPU_INT_EN |= GBL_HOST2CPU_INT;

    RTNA_AIC_Open(pAIC, AIC_SRC_HOST, hif_isr_a,
                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE    | 3);

    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_HOST);

    return MMP_ERR_NONE;
}
/** @}*/ //end of MMPF_SYS




/** @addtogroup MMPF_HIF
@{
*/

/** @brief Host command interrupt service routine
*/
void MMPF_HIF_HifISR(void)
{
    MMP_USHORT  command;

    AITPS_GBL   pGBL = AITC_BASE_GBL;

    pGBL->GBL_HOST2CPU_INT_SR = GBL_HOST2CPU_INT;

    command = MMPF_HIF_GetCmd();

    switch (command & GRP_MASK) {
    case GRP_SYS:
        MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_SYS, MMPF_OS_FLAG_SET);
        break;
    case GRP_SENSOR:
	    #if (((DSC_R_EN))&&(FDTC_SUPPORT == 1))
    	if ((command & FUNC_MASK) == 0x0E) {
        	MMPF_OS_SetFlags(FDTC_Flag, SYS_FLAG_FDTC, MMPF_OS_FLAG_SET);
        }
        else {
        #endif
        	MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_SENSOR, MMPF_OS_FLAG_SET);
		#if (((DSC_R_EN))&&(FDTC_SUPPORT == 1))
        }
        #endif
        break;
    case GRP_DSC:
        MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_DSC, MMPF_OS_FLAG_SET);
        break;
    case GRP_VID:
        if ((command & FUNC_MASK) < 0x10) {
            MMP_USHORT usFunc = command & (GRP_MASK | FUNC_MASK);
            if (usFunc == HIF_CMD_PARSER) {
                MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_PSR3GP, MMPF_OS_FLAG_SET);
            }
            else if (usFunc == HIF_CMD_VIDEO_DECODER){
                MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_VID_DEC, MMPF_OS_FLAG_SET);
            }
            else if (usFunc == HIF_CMD_MFD){
                MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_VID_DEC, MMPF_OS_FLAG_SET);
            }
            else {
                MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_VIDPLAY, MMPF_OS_FLAG_SET);
            }
        }
        // Will's code
        else if ((command & FUNC_MASK) == 0x13){
            MMP_USHORT usFunc = command & (GRP_MASK | FUNC_MASK);
            if (usFunc == HIF_CMD_H264_DEC){
                MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_VID_DEC, MMPF_OS_FLAG_SET);
            }
        }
        //end
        else {
            if ((command & FUNC_MASK) & 0x08) { // 0x18 & 0x19
                MMPF_OS_SetFlags(SYS_Flag_Hif, CMD_FLAG_VIDRECD, MMPF_OS_FLAG_SET);
            }
            else { // 0x10 & 0x11
                MMPF_OS_SetFlags(SYS_Flag_Hif, CMD_FLAG_MGR3GP, MMPF_OS_FLAG_SET);
            }
        }
        break;
    case GRP_AUD:
        if ((command & FUNC_MASK) < 0x10) {
            MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_AUDIOPLAY, MMPF_OS_FLAG_SET);
        }
        else {
            MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_AUDIOREC, MMPF_OS_FLAG_SET);
        }
        break;
    case GRP_STG:
        break;
    case GRP_USB:
        if((command & (GRP_MASK|FUNC_MASK)) == HIF_CMD_MSDC_ACK){
            //RTNA_DBG_Str0("get ack\r\n");
            MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_MEMDEV_ACK, MMPF_OS_FLAG_SET);
            MMPF_HIF_ClearCmd();
        }
        else
        MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_USB, MMPF_OS_FLAG_SET);
        break;
    case GRP_USR:
        if ((command & FUNC_MASK) <= 0x02) {
            MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_FS, MMPF_OS_FLAG_SET);
        }
        else {
#if defined(BROGENT_FW)
            if (((command & FUNC_MASK) >= 0x03) && ((command & FUNC_MASK) <= 0x08)) {
            MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_BROGENT1, MMPF_OS_FLAG_SET);
            }
#endif /* defined(BROGENT_FW) */
            #if (APP_EN)
    	    if ((command & FUNC_MASK) == 0x09) {
        	    MMPF_OS_SetFlags(APP_Flag, SYS_FLAG_APP, MMPF_OS_FLAG_SET);
            }
            #endif
        }
        break;
    }

}
#endif
/** @} */ // end of MMPF_HIF

//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_GetFWEndAddr
//  Description : called by the task with host command queue to process
//                command for System group
//------------------------------------------------------------------------------

#if defined(ALL_FW)
extern unsigned int Image$$ALL_DRAM$$ZI$$Limit;
#endif
MMP_ERR MMPF_SYS_GetFWEndAddr(MMP_ULONG *ulEndAddr)
{
	#if defined(ALL_FW)
	*ulEndAddr = ((MMP_ULONG)&Image$$ALL_DRAM$$ZI$$Limit + 0xFFF) & (~0xFFF);
	#endif

    return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_GetAudio1StartAddr
//  Description : called by the task with host command queue to process
//                command for System group
//------------------------------------------------------------------------------
MMP_ERR MMPF_SYS_GetAudioStartAddr(MMP_ULONG coderange, MMP_ULONG *ulEndAddr)
{
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : SYS_Task
//  Description : SYS Task Function
//------------------------------------------------------------------------------
//#ifdef BUILD_CE
//#include "mmph_hif.h"
//extern MMP_BOOL bWaitForSysCommandDone;
//#endif




msg_t *MMPF_SYS_MakeMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    msg_t *msg;
    msg = allocate_msg(src_id);
    if(!msg ){
        return (msg_t *)0 ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    return msg ; 
}

msg_t *MMPF_SYS_GetMsg(void)
{
    MMPF_OS_FLAGS flags;
    msg_t *msg=0;
    /*
    Get message first in case there has multiple messages
    */
    #if 1
    msg = (msg_t *)get_msg(1);
    if(msg) {
        return msg ;
    }
    #endif
    /*
    Wait mesage event
    */
    MMPF_OS_WaitFlags(SYS_Flag_Hif, SYS_FLAG_SYS,
                    MMPF_OS_FLAG_WAIT_SET_ANY | MMPF_OS_FLAG_CONSUME, 0, &flags);

    /*if(flags & PCAM_FLAG_OVERWR_MSG) {
        msg = (pcam_msg_t *)get_overwr_msg(1);
        if(msg) {
            return msg ;
        }
    }     */
    if(flags & SYS_FLAG_SYS) {
        msg = (msg_t *)get_msg(1);
        if(msg) {
            return msg ;
        }
    }
    return (msg_t *)0;
}
extern void UsbEp0IntHandler(void);
//extern  MMP_UBYTE   USBDevAddr;
//extern MMPF_OS_SEMID    gUSBDMASem[] ;
extern MMP_UBYTE gbUSBSuspendFlag ;
#if USB_SUSPEND_TEST
extern volatile MMP_UBYTE gbUSBSuspendEvent ;
#endif

MMP_UBYTE  gbDramType = 0;
void SYS_Task(void)
{
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    MMPF_OS_FLAGS   flags;
    msg_t *msg=0 ;
    MMP_USHORT err = MSG_ERROR_NONE ;

    //gUSBDMASem[0] = MMPF_OS_CreateSem(1);
    //gUSBDMASem[1] = MMPF_OS_CreateSem(1);

    RTNA_DBG_Str(3, "SYS_Task()...\r\n");
    //MMPF_PIO_Initialize();
    MMPC_GPIO_InitConfig();

    init_msg_queue(1, SYS_Flag_Hif, SYS_FLAG_SYS);


    #if (CHIP == P_V2)
    m_gbSystemCoreID = pGBL->GBL_CHIP_NAME[8];
    m_gsISPCoreID = 888;
    #endif
    #if (CHIP == VSN_V2)
    m_gbSystemCoreID = pGBL->GBL_CHIP_VER;
    m_gsISPCoreID = 989;
    gbDramType = 0;
    #endif

   // dbg_printf(0,"Core Id:%x,ISP Id:%d\r\n",m_gbSystemCoreID,m_gsISPCoreID);
#if DUAL_UART_DBG_EN==1
    MMPF_Uart_InitISR();
#endif
    
	#if (DSC_R_EN)||(VIDEO_R_EN)||(VIDEO_P_EN)
    MMPF_DMA_Initialize();
    #endif
    
	#if SUPPORT_GRA_ZOOM//PCCAM_MSB==1
	MMPF_Graphics_Init();
	#endif
    while (TRUE) {
        // add task event for usb interrupt here.
        msg = MMPF_SYS_GetMsg();
        if(!msg) {
            continue ;
        }
        switch(msg->msg_id) {
            case SYS_MSG_USB_RESTART_SENSOR:
            {
                MMP_UBYTE ep = 0;
                if( (MMP_UBYTE *)msg->msg_data==0) {
                    ep = 0;
                }
                else {
                    ep = 1;
                }
                MMPF_SYS_ResetSensor(ep);
                dbg_printf(0,"--restart-snr\r\n");
                break;
            }
            #if H264_SIMULCAST_EN
            case SYS_MSG_USB_RESTART_PREVIEW:
            {
                MMP_UBYTE ep = 0;
                if( (MMP_UBYTE *)msg->msg_data==0) {
                    ep = 0;
                }
                else {
                    ep = 1;
                }
                
                UVCX_RestartPreview(ep);
                
            }   
            break;
            case SYS_MSG_USB_H264_RT_FR_SWITCH:
            {
                ENCODE_BUF_MODE mode;
                if( (MMP_BOOL *)msg->msg_data==0 ) {
                    mode = REALTIME_MODE ;
                }    
                else {
                    mode = FRAME_MODE ;
                }
                UVCX_SwitchH264EncodeBufferMode(mode);
                break ;
            }
            #endif
            case SYS_MSG_USB_EP0_TX_INT:
                UsbEp0IntHandler();
                break;
            case SYS_MSG_USB_RESET_INT:
                //USBDevAddr=0;
                 
                UsbRestIntHandler();
                RTNA_DBG_Str(0,"UsbRestIntHandler\r\n"); 
                break;
            case SYS_MSG_USB_SUSPEND_INT:
                #if USB_SUSPEND_TEST==1
                if(gbUSBSuspendEvent){
                    gbUSBSuspendEvent = 0;
                    if(gbUSBSuspendFlag == 0){
                        RTNA_DBG_Str(0, "-S3-\r\n");
                        gbUSBSuspendFlag = 1;
                        USB_SuspendProcess() ;
                    }
                }
                #endif

                break ;

            case SYS_MSG_USB_EP3_TX_INT:
                UsbEp3TxIntHandler(); // audio use fifo
                break;
            case SYS_MSG_USB_EP1_TX_INT:
                UsbEp1TxIntHandler();
                break;
            case SYS_MSG_USB_EP2_TX_INT:
                UsbEp2TxIntHandler();
                break;
            case SYS_MSG_USB_DMA_EP1_RX_INT:
                UsbEp1RxIntHandler();
                break;
            case SYS_MSG_USB_DMA_EP2_TX_INT:
                UsbEp2TxIntHandler();
                break;
            case SYS_MSG_USB_DMA_EP1_TX_INT:
                UsbEp1TxIntHandler();
                break;
            #if SUPPORT_PCSYNC
            case SYS_MSG_USB_EP5_TX_INT:
                UsbEp5TxIntHandler() ;
                break;
            case SYS_MSG_USB_EP5_RX_INT:
                UsbEp5RxIntHandler();
                break;
            #endif
            default:
                RTNA_DBG_Str(0, "unregconize USB intr\r\n");
                break;
        }
		msg->err = err;
        /*Handler processed done*/
        /*Release semaphore*/
       /* if(msg->msg_sem) {
            MMPF_OS_ReleaseSem(*msg->msg_sem);
        } else {
            free_msg(msg, 1);
        }*/

        free_msg(msg, 1);
    }
}

static char *gbSysCurFBPtr[2] = {0,0};
static int  glSysResetFB[2]   = {1,1} ;
static MMP_ULONG glSysFBPartionStart[2] ;
static MMP_ULONG glSysFBPartionEnd[2] ;

void MMPF_SYS_InitFB(MMP_UBYTE partition_id) 
{
#define FB_PARTITION_1_SIZE (16*1024)*1024
    MMPS_SYSTEM_CONFIG *pSysCfg = (MMPS_SYSTEM_CONFIG *)MMPS_System_GetConfiguration() ;
    if(partition_id==0) {
        MMP_ULONG fw_end;
        MMPF_SYS_GetFWEndAddr(&fw_end);
        glSysFBPartionStart[0] = fw_end;
        if(UVC_VCD()==bcdVCD10) {
            glSysFBPartionEnd[0]   = pSysCfg->ulStackMemoryStart + pSysCfg->ulStackMemorySize ;
        } else {
            glSysFBPartionEnd[0]   = pSysCfg->ulStackMemoryStart + FB_PARTITION_1_SIZE ;
        }
    } 
    if(partition_id==1) {
        if( UVC_VCD()==bcdVCD15 ) { // Open for simulcast
            glSysFBPartionStart[1] = pSysCfg->ulStackMemoryStart + FB_PARTITION_1_SIZE ;
            glSysFBPartionEnd[1]   = pSysCfg->ulStackMemoryStart + pSysCfg->ulStackMemorySize  ;
        } else {
            dbg_printf(3,"@Init FB Err partition id\r\n");
            glSysFBPartionStart[1] = glSysFBPartionEnd[1] = 0 ;
        }
    }   
    dbg_printf(3,"FB %d, [start,end] = (%x,%x)\r\n",partition_id,glSysFBPartionStart[partition_id],glSysFBPartionEnd[partition_id] );
    glSysResetFB[partition_id] = 1 ;
}

void *MMPF_SYS_AllocFB(char *tag,MMP_ULONG size, MMP_USHORT align,MMP_UBYTE partition_id)
{
    char *ptr ;
    MMP_ULONG alignSize;
    //MMP_ULONG dramEnd ;
    //MMPS_SYSTEM_CONFIG *pSysCfg = (MMPS_SYSTEM_CONFIG *)MMPS_System_GetConfiguration() ;
    //dramEnd = pSysCfg->ulStackMemoryStart  + pSysCfg->ulStackMemorySize ;
    TASK_CRITICAL(
        if(glSysResetFB[partition_id]) {
            //MMP_ULONG fw_end;
            //MMPF_SYS_GetFWEndAddr(&fw_end);
            gbSysCurFBPtr[partition_id] = (char *) glSysFBPartionStart[partition_id] ;  
            glSysResetFB[partition_id] = 0 ;         
        } 
        //ptr = (char *)gbSysCurFBPtr[partition_id];
        
        ptr = (char *)(((MMP_ULONG)gbSysCurFBPtr[partition_id] + (MMP_ULONG)align - 1) & ((MMP_ULONG) -align));
        
        alignSize = ( size +(MMP_ULONG) align - 1) & ((MMP_ULONG) -align ) ;
        gbSysCurFBPtr[partition_id] += alignSize ;
        if( (MMP_ULONG)gbSysCurFBPtr[partition_id] > glSysFBPartionEnd[partition_id] /*dramEnd*/) {
            dbg_printf(0,"+++Dram allocfailed(%x)+++\r\n",gbSysCurFBPtr[partition_id]);
        }
    )
    dbg_printf(3,"@[%d][%s](Addr,Size) : (%x,%x)\r\n",partition_id,tag,(MMP_ULONG)ptr,alignSize);
    

    return (void *)ptr ;
    
}

void MMPF_SYS_SetCurFBAddr(char *ptr,MMP_UBYTE partition_id)
{
    TASK_CRITICAL(
        gbSysCurFBPtr[partition_id] = ptr ;
    )
}

void *MMPF_SYS_GetCurFBAddr(MMP_LONG *freesize,MMP_UBYTE partition_id)
{
    MMP_ULONG dramEnd ,usedsize ;
    MMPS_SYSTEM_CONFIG *pSysCfg = (MMPS_SYSTEM_CONFIG *)MMPS_System_GetConfiguration() ;
    
    TASK_CRITICAL(
        if(glSysResetFB[partition_id]) {
            //MMP_ULONG fw_end;
            //MMPF_SYS_GetFWEndAddr(&fw_end);
            gbSysCurFBPtr[partition_id] = (char *) glSysFBPartionStart[partition_id] ;   
            glSysResetFB[partition_id] = 0 ;         
        }
    )
    
    usedsize = (MMP_ULONG)gbSysCurFBPtr[partition_id]- glSysFBPartionStart[partition_id]  ;
    *freesize = (MMP_LONG)( (glSysFBPartionEnd[partition_id] - glSysFBPartionStart[partition_id]) - usedsize) ;
    dbg_printf(3,"FB %d,used : %d, free : %d, addr :%x\r\n",partition_id,usedsize,*freesize,gbSysCurFBPtr[partition_id]);
    return gbSysCurFBPtr[partition_id] ;
}

void MMPF_SYS_GetFBAddrArea(MMP_LONG *start,MMP_LONG *end,MMP_UBYTE partition_id)
{
    if(partition_id==0) {
        *start = glSysFBPartionStart[0] ;
        *end   = glSysFBPartionEnd[0];
    }
    else {
        *start = glSysFBPartionStart[1] ;
        *end   = glSysFBPartionEnd[1];
    }
}

extern volatile MMPF_BOOTINFO *gsBootInfo ;
#pragma arm section code = "AbortItcm", rwdata = "AbortItcm",  zidata = "AbortItcm"

void AbortHandler(void)
{
    static int i ;
    static volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) 0x80000000;
    static AITPS_GBL 	pGBL = AITC_BASE_GBL;
#if WATCHDOG_RESET_EN && CPU_EXCEPTION_LOG
    static AITPS_WD 	pWD = AITC_BASE_WD;
    static MMP_ULONG lr,sp ;
#endif     
// Dram Invert 0xAC76 = 0
#if(CHIP==P_V2)
    REG_BASE_B[0xAC76] = 0;// 0xD8 ; // Just for test....
    REG_BASE_B[0x8D02] &= ~0x01 ;
    REG_BASE_B[0x8D03] &= ~0x08 ;
    REG_BASE_B[0x9401] = 0x02 ;
#endif
#if(CHIP==VSN_V3)
    //REG_BASE_B[0x6E70] = 0x38 ;
#endif
    
    dbg_printf(0,"*** CPU Exception ***\r\n");
    dbg_printf(0,"Abort LR:%x\r\n",gsBootInfo->abort_lr);
    dbg_printf(0,"Abort SP:%x\r\n",gsBootInfo->abort_sp);
#if WATCHDOG_RESET_EN && CPU_EXCEPTION_LOG
    write_cpu_exception(gsBootInfo->abort_lr,gsBootInfo->abort_sp);
#endif    
#if 1//(CHIP==P_V2)

    pGBL->GBL_CLK_DIS1 |= GBL_CLK_PWM_DIS;
    pGBL->GBL_IO_CTL2 &= ~(GBL_PWM1_2_PB_GPIO11|GBL_PWM2_2_PB_GPIO13); // Disable PWM
    pGBL->GBL_IO_CTL7 |= (0x08);    // BGPIO11 as GPIO
    REG_BASE_B[0x6602] |= 0x08 ; // GPIO output mode
    while(1) {
        REG_BASE_B[0x6612] = (i & 1)?0x08:0x00 ;
        i++ ;
        RTNA_WAIT_MS(200);
    }	
#endif
        
}
#pragma arm section code, rwdata,  zidata

void MMPF_SYS_WatchDogEn(MMP_BOOL en)
{
#if WATCHDOG_RESET_EN==1
    MMPF_WD_EnableWD(en,MMP_FALSE,TRUE,0,TRUE);
    
#endif
}

#if DIV_0_RESET_EN==1

 void dump_div0_info(void)
{
    SAVE_LRSP();
    AbortHandler();           
}
// Integer DIV 0 detector
extern void $Super$$__rt_div0(void);
extern long SYS_FLOAT ;
extern OS_TCB *OSTCBCur ;

void $Sub$$__rt_div0(void)
{
    dbg_printf(0, "[%s]Div 0\r\n",OSTCBCur->OSTCBTaskName);
    $Super$$__rt_div0();
    // Force Watch dog timeout
    dump_div0_info();
    while(1); 
}

#endif

#if 1//
#include "mmp_reg_jpeg.h"
#include "mmp_reg_graphics.h"
#include "mmp_reg_icon.h"
#include "mmp_reg_dram.h"
#include "mmp_reg_sf.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_i2cm.h"
#include "mmp_reg_rawproc.h"
#include "mmp_reg_audio.h"
#include "mmp_reg_dma.h"
#include "mmp_reg_uart.h"

void MMPF_SYS_DumpReg(MMP_ULONG opr_startaddr, MMP_UBYTE *buf ,MMP_USHORT *size)
{
    volatile MMP_UBYTE *REG_BASE_B ;
    AITPS_GBL   pGBL  = AITC_BASE_GBL;
    MMP_UBYTE    clk0,clk2 ;
    MMP_USHORT   clk1,i ;
    clk0 = pGBL->GBL_CLK_DIS0 ;
    clk1 = pGBL->GBL_CLK_DIS1 ;
    clk2 = pGBL->GBL_CLK_DIS2 ;
    if( (opr_startaddr & 0x80000000) ) {
        opr_startaddr &= ~0x80000000 ;
    }

     // Check if it is GBL
    #if 1
    if ( (opr_startaddr >= GBL_BASE) && (opr_startaddr < (GBL_BASE + sizeof(AITS_GBL)))) {
        if ( *size > sizeof(AITS_GBL) ) {
            *size = sizeof(AITS_GBL) ;
        }
    }
    else if ((opr_startaddr >= VIF_BASE) && (opr_startaddr < (VIF_BASE + sizeof(AITS_VIF)))) {
        if ( *size > sizeof(AITS_VIF) ) {
            *size = sizeof(AITS_VIF) ;
        }
        pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_VI_DIS);
    }
    else if ((opr_startaddr >= JPG_BASE) && (opr_startaddr < (JPG_BASE + sizeof(AITS_JPG)))) {
        if ( *size > sizeof(AITS_JPG) ) {
            *size = sizeof(AITS_JPG) ;
        }
        pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_JPG_DIS);
    }
    else if ((opr_startaddr >= GRA_BASE) && (opr_startaddr < (GRA_BASE + sizeof(AITS_GRA)))) {
        if ( *size > sizeof(AITS_GRA) ) {
            *size = sizeof(AITS_GRA) ;
        }
        pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_GRA_DIS);
    }
    else if ((opr_startaddr >= ICO_BASE) && (opr_startaddr < (ICO_BASE + sizeof(AITS_ICOB)))) {
        if ( *size > sizeof(AITS_ICOB) ) {
            *size = sizeof(AITS_ICOB) ;
        }
        pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_ICON_DIS);
    }
    else if ((opr_startaddr >= DRAM_BASE ) && (opr_startaddr < (DRAM_BASE + sizeof(AITS_DRAM)))) {
        if ( *size > sizeof(AITS_DRAM) ) {
            *size = sizeof(AITS_DRAM) ;
        }
    }
    else if ((opr_startaddr >= SIF_BASE ) && (opr_startaddr < (SIF_BASE + sizeof(AITS_SIF)))) {
        if ( *size > sizeof(AITS_SIF) ) {
            *size = sizeof(AITS_SIF) ;
        }
    }
    else if ((opr_startaddr >= IBC_BASE ) && (opr_startaddr < (IBC_BASE + sizeof(AITS_IBC)))) {
        if ( *size > sizeof(AITS_IBC) ) {
            *size = sizeof(AITS_IBC) ;
        }
        pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_IBC_DIS);
    }
    else if ((opr_startaddr >= GPIO_BASE ) && (opr_startaddr < (GPIO_BASE + sizeof(AITS_GPIO)))) {
        if ( *size > sizeof(AITS_GPIO) ) {
            *size = sizeof(AITS_GPIO) ;
        }
        pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_GPIO_DIS);
    }
    else if ((opr_startaddr >= SCAL_BASE ) && (opr_startaddr < (SCAL_BASE + sizeof(AITS_SCAL)))) {
        if ( *size > sizeof(AITS_SCAL) ) {
            *size = sizeof(AITS_SCAL) ;
        }
        pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_SCAL_DIS);
    }
    else if ((opr_startaddr >= I2CM_BASE ) && (opr_startaddr < (I2CM_BASE + sizeof(AITS_I2CM)))) {
        if ( *size > sizeof(AITS_I2CM) ) {
            *size = sizeof(AITS_I2CM) ;
        }
        pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_VI_DIS);
        pGBL->GBL_CLK_DIS1 &= (~ GBL_CLK_I2C_DIS);
    }
    else if ((opr_startaddr >= RAWPROC_BASE ) && (opr_startaddr < (RAWPROC_BASE + sizeof(AITS_RAWPROC)))) {
        if ( *size > sizeof(AITS_RAWPROC) ) {
            *size = sizeof(AITS_RAWPROC) ;
        }
        pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_RAW_DIS);
    }
    else if ((opr_startaddr >= PWM_BASE ) && (opr_startaddr < (PWM_BASE + sizeof(AITS_PWM)))) {
        if ( *size > sizeof(AITS_PWM) ) {
            *size = sizeof(AITS_PWM) ;
        }
        pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_PWM_DIS);
    }
    else if ((opr_startaddr >= DMA_BASE ) && (opr_startaddr < (DMA_BASE + sizeof(AITS_DMA)))) {
        if ( *size > sizeof(AITS_DMA) ) {
            *size = sizeof(AITS_DMA) ;
        }
        pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_DMA_DIS);
    }
    else if ((opr_startaddr >= UARTB_BASE ) && (opr_startaddr < (UARTB_BASE + sizeof(AITS_UARTB)))) {
        if ( *size > sizeof(AITS_UARTB) ) {
            *size = sizeof(AITS_UARTB) ;
        }
    }    
    else if ((opr_startaddr >= AUD_BASE ) && (opr_startaddr < (AUD_BASE + sizeof(AITS_AUD)))) {
        if ( *size > sizeof(AITS_AUD) ) {
            *size = sizeof(AITS_AUD) ;
        }
        pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_AUD_DIS);
        pGBL->GBL_CLK_DIS2 &= (~GBL_CLK_AUD_CODEC_DIS);
    }
    else if ((opr_startaddr >= AFE_BASE ) && (opr_startaddr < (AFE_BASE + sizeof(AITS_AFE)))) {
        if ( *size > sizeof(AITS_AFE) ) {
            *size = sizeof(AITS_AFE) ;
        }
        pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_AUD_DIS);
        pGBL->GBL_CLK_DIS2 &= (~GBL_CLK_AUD_CODEC_DIS);
    }
    
    opr_startaddr |= 0x80000000 ;
    REG_BASE_B = (volatile MMP_UBYTE *)opr_startaddr ;
    
    for(i=0;i< *size ;i++) {
        buf[i] = REG_BASE_B[i] ;
    }

    #endif
    pGBL->GBL_CLK_DIS0 = clk0 ;
    pGBL->GBL_CLK_DIS1 = clk1 ;
    pGBL->GBL_CLK_DIS2 = clk2 ;
	dbg_printf(3,"Get opr addr : %x,size : %d\r\n",	 opr_startaddr , *size );
    
}
#endif

/** @}*/ //end of MMPF_SYS
