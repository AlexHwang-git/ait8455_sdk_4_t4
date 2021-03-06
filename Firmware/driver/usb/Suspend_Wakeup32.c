//==============================================================================
//
//  File        : Suspend_Wakeup.c
//  Description : USB Suspend and Wakeup function
//  Author      : Robin Feng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "config_fw.h"

#include "mmpf_typedef.h"
#include "mmph_hif.h"
#include "mmps_system.h"
#include "mmpd_system.h"
//#include "mmpf_fs_api.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbpccam.h"
#include "mmpf_pll.h"
#include "mmpf_uart.h"
#include "lib_retina.h"
#include "mmpd_dram.h"
#include "mmpf_sensor.h"
#include "mmp_reg_display.h"
#include "mmpf_pio.h"
#include "mmpf_system.h"
#include "mmp_reg_dram.h"
#include "mmpf_msg.h"
#include "mmp_reg_usb.h"

#define EXIT_PS_CHECK_BY_LED (0) 


MMP_UBYTE gbPowerSavingAtBoot = 0;

#if ((USB_SUSPEND_TEST) || (JTAG == 1))

MMP_UBYTE gbDisbaleUart = 0;


extern MMP_UBYTE gbUSBSuspendFlag;
extern MMP_UBYTE  gbDramType;

void USB_SuspendProcess(void)
{
    static AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_BOOL ispreview ;
    MMP_SHORT timeout=500;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    AITPS_PWM pPWM = AITC_BASE_PWM;
    AITPS_AIC pAIC = AITC_BASE_AIC;
    #if (OS_CRITICAL_METHOD == 3) 
    static OS_CPU_SR   cpu_sr = 0;
    #endif
    
    #if 0// (CHIP==VSN_V2)
        dbg_printf(3,"v2 sleep not test,skip\r\n");
        return ;
    #endif
   // VAR_B(0,pUSB_CTL->USB_POWER );
    MMPS_3GPRECD_GetPreviewDisplay(&ispreview) ;
    //   dbg_printf(3,"#Streaming\r\n");

    if(ispreview==1){
        MMP_UBYTE i;
        STREAM_SESSION *ss;
        for(i=0;i<VIDEO_EP_END();i++) { 
            ss = MMPF_Video_GetStreamSessionByEp(i); 
            if(ss->tx_flag & SS_TX_STREAMING) {
                usb_uvc_stop_preview(ss->ep_id,0) ;
            }
        }
    }
    // sean@2011_03_31 before enter suspend, wait all task enter idle except current task
    MMPF_PCAM_WaitTaskIdle(OS_STAT_FLAG);
    
    //RTNA_DBG_Str(0, "- S.Process -\r\n");
    USB_EnableTaskRunning(0);
    MMPF_SYS_ConfigIOPad();
    OS_ENTER_CRITICAL();
#if WATCHDOG_RESET_EN==1
    MMPF_SYS_WatchDogEn(0);
#endif    
    MMPF_SYS_EnterPSMode(1);
    MMPF_SYS_EnterSelfSleepMode(/*gbDramType*/) ;
    //RTNA_DBG_Str(0, "- S.Process Exit -\r\n");
    MMPF_SYS_EnterPSMode(0);
    OS_EXIT_CRITICAL();
    USB_EnableTaskRunning(1);
#if WATCHDOG_RESET_EN==1
    MMPF_SYS_WatchDogEn(1);
#endif    
}

#pragma arm section code

#endif
