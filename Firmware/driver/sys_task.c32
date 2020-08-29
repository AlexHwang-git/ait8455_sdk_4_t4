//==============================================================================
//
//  File        : sys_task.c
//  Description : Main entry function of system
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================


#include "includes_fw.h"
#include "config_fw.h"
#if defined(ALL_FW)
#include "ait_config.h"
#endif
#include "reg_retina.h"
#include "lib_retina.h"
#include "mmp_reg_gpio.h"
#include "mmp_reg_display.h"

#include "mmpf_hif.h"
#include "mmph_hif.h"
#include "mmpf_system.h"
#include "mmpf_display.h"
//#include "mmpf_fs_api.h"
#include "mmpf_pll.h"
#include "mmp_reg_audio.h"
#include "mmpf_boot.h"
#if WATCHDOG_RESET_EN==1
#include "pcam_msg.h"
#endif

#include "mmpf_pll.h"
#include "mmpf_dram.h"
/** @addtogroup MMPF_SYS
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
///////////////////////
// System Task Stack //
///////////////////////
extern OS_STK                  SYS_Task_Stk[];
#if (WATCHDOG_RESET_EN == 0x1)
extern OS_STK           LTASK_Task_Stk[];
#endif
//////////////////
// System Event //
//////////////////
MMPF_OS_FLAGID          SYS_Flag_Hif;


////////////////////////
// System Global Data //
////////////////////////



///////////////////
// External Task //
///////////////////
extern OS_STK           DSC_Task_Stk[];
extern OS_STK           MP4VENC_Task_Stk[];
extern OS_STK           VIDPLAY_Task_Stk[];
extern OS_STK           VIDMGR_Task_Stk[];
extern OS_STK           PARSR3GP_Task_Stk[];
extern OS_STK           VIDDEC_Task_Stk[];
extern OS_STK           SENSOR_Task_Stk[];
extern OS_STK           AUDIO_MidiPlay_Task_Stk[];
extern OS_STK           AUDIO_AmrRecord_Task_Stk[];
extern OS_STK           USB_Task_Stk[];
extern OS_STK           FS_Task_Stk[];
extern OS_STK           AUDIO_WriteFile_Task_Stk[];

extern OS_STK           PCAM_Task_Stk[];

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void                    SYS_Task(void);

static void             CreateTasks(void);
static void             CreateEvents(void);




extern  void MMPF_SWPACK_TaskInit(void);
extern  void MMPF_PCAM_StartUSB(void);

#if defined(UPDATER_FW)
extern int MMPF_USB_StopUSB(void);    
#endif

//==============================================================================
//
//                              MACRO DEFINITIONS
//
//==============================================================================



//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================
void dump_boot_info(void);
void MMPF_SYS_EnableClock(MMP_ULONG ulClkEn,MMP_ULONG ulClk2En);



//------------------------------------------------------------------------------
//  Function    : MMPF_SYS_TaskHandler
//  Description : This is the SYS startup task
//------------------------------------------------------------------------------
void MMPF_SYS_TaskHandler(void *p_arg)
{
    p_arg = p_arg;

    MMPF_BSP_InitializeTimer();
    CreateEvents();                                         // Create the application's kernel objects
    CreateTasks();                                          // Create the other application tasks
#if PCAM_EN==0
    MMPF_SYS_InitializeHIF();                               // Initialize host command interrupt and ISR
#endif
    SYS_Task();
}

#if WATCHDOG_RESET_EN
//MMP_UBYTE gbWatchDogRestart = 1;

void WD_TimeOut()
{
    RTNA_DBG_Str(0, "WD_TimeOut function() \r\n");
    MMPF_SYS_ResetSystem(0);
}

void MMPF_LTASK_TaskHandler(void *p_arg)
{
    #if defined(ALL_FW)&&(SYS_WD_ENABLE == 0x1)
    RTNA_DBG_Str(0, "L_Task() \r\n");

    while(1) {
        MMPF_OS_Sleep(1000);
        MMPF_WD_Kick();
    }
    #endif

//extern MMPF_OS_FLAGID PCAM_Flag ;
//    MMP_ULONG ulValue,flags ;
//
//    while(1) {
//        MMPF_OS_WaitFlags(PCAM_Flag, (PCAM_FLAG_WATCHDOG),
//                     (MMPF_OS_FLAG_WAIT_SET_ANY | MMPF_OS_FLAG_CONSUME),
//                     0, &flags);
//
//        if(flags & PCAM_FLAG_WATCHDOG) {
//            pWD->WD_RE_ST = WD_RESTART;
//            //dbg_printf(3,"*\r\n");
//        }
//    }
}  
#endif
//------------------------------------------------------------------------------
//  Function    : CreateTasks
//  Description : It is called by startup task to create default tasks at
//                initialization.
//------------------------------------------------------------------------------

static void CreateTasks(void)
{

#if defined(ALL_FW)||defined(UPDATER_FW)||defined(MBOOT_FW)
    MMPF_SWPACK_TaskInit();
#endif
}


//------------------------------------------------------------------------------
//  Function    : CreateEvents
//  Description : This function creates the basic system events for OS inter
//                communication.
//------------------------------------------------------------------------------
//extern MMPF_OS_FLAGID 		DSC_UI_Flag;

static void CreateEvents(void)
{
    SYS_Flag_Hif = MMPF_OS_CreateEventFlagGrp(0);
#if PCAM_EN==0    
    DSC_UI_Flag = MMPF_OS_CreateEventFlagGrp(1);
#endif    
}



//------------------------------------------------------------------------------
//  Function    : main
//  Description : This is the standard entry point for C and CRT
//                initialization is done before calling main()
//------------------------------------------------------------------------------

#ifdef BUILD_CE
//#include "mmps_system.h"
//#include "mmpd_system.h"
//#include "mmps_usb.h"

MMPF_OS_SEMID    SYS_Sem_CommandSend;
MMPF_OS_SEMID    SYS_Sem_SifBusy;
#endif //BUILD_CE
extern MMP_ULONG glGroupFreq[];
extern MMP_ULONG glCPUFreq;
extern MMP_ULONG RTNA_CPU_CLK_M ;

extern MMP_UBYTE gbDramID ;
#pragma arm section code = "AbortItcm", rwdata = "AbortItcm",  zidata = "AbortItcm"
volatile MMPF_BOOTINFO *gsBootInfo = (volatile MMPF_BOOTINFO *)0x00000000;
volatile MMPF_BOOTEXINFO *gsBootExInfo = (volatile MMPF_BOOTEXINFO *)0x00000100;

#pragma arm section code, rwdata,  zidata
 
int main(void)
{
   // volatile    MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *)0x80000000;
    MMPF_TASK_CFG   task_cfg;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
   // MMP_ULONG i;
    MMP_UBYTE ret;
   // MMP_UBYTE dramSize,dramVendor;
    // Turn on CPU WFI mode, save 20mA 
#if defined(ALL_FW)
    // Disable JPG Clock when Idle...
    pGBL->GBL_CLK_DIS0 |= GBL_CLK_JPG_DIS;
    #if ISP_GNR_OFF==1
    pGBL->GBL_CLK_DIS2 |= GBL_CLK_GNR_DIS;
    #endif
 
    pGBL->GBL_CPU_CFG |= 0x08;
    
    #if JTAG
    OS_CPU_EnablePowerDown(0);
    #else
    OS_CPU_EnablePowerDown(1);
    #endif
#endif

    
    MMPC_System_InitConfig();
    MMPH_HIF_SetInterface(MMPH_HIF_BYPASS);
    MMPC_Sensor_InitConfig();
    MMPC_AUDIO_InitConfig();

    MMPF_PLL_Setting(MMPF_PLL_480CPU_240G034_166G1_X, MMP_FALSE);
   
    RTNA_CPU_CLK_M = glCPUFreq/1000 ;
    dbg_printf(0,"\r\n\r\nRTNA_CPU_CLK_M : %d MHz\r\n",RTNA_CPU_CLK_M);
    // Switch to 5M configuration
    pGBL->GBL_SOFT_RESOL_SEL = GBL_SENSOR_5M ;


    dump_boot_info();
    MMPF_BSP_Initialize(); 
    MMPF_OS_Initialize();
    SYS_Sem_CommandSend = MMPF_OS_CreateSem(0);
    MMPF_OS_ReleaseSem(SYS_Sem_CommandSend);

    SYS_Sem_SifBusy = MMPF_OS_CreateSem(0);
    MMPF_OS_ReleaseSem(SYS_Sem_SifBusy);

  //  RTNA_DBG_Str0( "H264 PCCAM verion 1.1\r\n");
  //  RTNA_DBG_Str0( "===== H264 PCCAM Build: " __TIME__ " " __DATE__ " =====\r\n");

	#if (SYS_WD_ENABLE == 0x1)
	MMPF_WD_Initialize();
	MMPF_WD_SetTimeOut(31, 1024);
	MMPF_WD_EnableWD(MMP_TRUE, MMP_FALSE, MMP_TRUE, NULL, MMP_TRUE);
	#endif

    
	gbDramID = MMPF_System_ReadCoreID();
	dbg_printf(3, "gbDramID = 0x%x\r\n", gbDramID);
    MMPC_System_DumpSramConfig();
    
    // User specified data
    // Task Table Initialization
    //[TODO] It can do initialization at CRT if CRT exists.
    task_cfg.ubPriority = TASK_SYS_PRIO;
    task_cfg.pbos = (MMP_ULONG)&SYS_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&SYS_Task_Stk[TASK_SYS_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_SYS_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_SYS", &ret);

    #if WATCHDOG_RESET_EN==1
    task_cfg.ubPriority = (OS_LOWEST_PRIO - 2);
    task_cfg.pbos = (INT32U)&LTASK_Task_Stk[0];
    task_cfg.ptos = (INT32U)&LTASK_Task_Stk[LTASK_STK_SIZE - 1];
    MMPF_OS_CreateTask(MMPF_LTASK_TaskHandler, &task_cfg, (void *)0);
    #endif
    
#if 0
    RTNA_DBG_Str0("MMPF_OS_StartTask\r\n");
#endif
    MMPF_OS_StartTask();                                    // Start multitasking (i.e. give control to uC/OS-II)
}



//------------------------------------------------------------------------------
//  Function    : __initial_stackheap
//  Description : This is the default stack/heap initialization function
//------------------------------------------------------------------------------
#pragma import (__use_no_semihosting_swi)

#include <rt_misc.h>

extern unsigned int Load$$TCM_END$$Base;
extern unsigned int Load$$SRAM_END$$Base;
extern unsigned int Image$$ALL_DRAM$$ZI$$Limit;

__value_in_regs struct __initial_stackheap
__user_initial_stackheap(unsigned R0, unsigned SP, unsigned R2, unsigned SL)
{
    struct __initial_stackheap config;


    config.stack_base = (unsigned int)&Load$$TCM_END$$Base; // the end address of TCM
    config.heap_base = (unsigned int)&Image$$ALL_DRAM$$ZI$$Limit;
    return config;
}


void dump_boot_info(void)
{
#if CPU_EXCEPTION_LOG==1  
    MMP_ULONG lr=-1,sp=-1 ;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
#endif
    int i ;
    dbg_printf(3,"Load #FW:%d\r\n",gsBootExInfo->loadfw);
    
    dbg_printf(3,"irq vector:%x\r\n",gsBootInfo->vector[6]);
    dbg_printf(3,"boot sig:%x\r\n",gsBootInfo->sig);
    dbg_printf(3,"boot lr:%x\r\n",gsBootInfo->abort_lr);
    dbg_printf(3,"boot sp:%x\r\n",gsBootInfo->abort_sp);
#if (CHIP==VSN_V3)
    if(gsBootInfo->sig==0x35353438) {
        dbg_printf(3,"New vectors are installed\r\n");
    }
#endif    
#if (CHIP==VSN_V2)
    if(gsBootInfo->sig==0x35313438) {
        dbg_printf(3,"New vectors are installed\r\n");
    }
#endif 
}

void _sys_exit(int return_code)
{

}

void _ttywrch(int ch)
{

}

void __assert(const char *expr, const char *file, int line)
{
}

/// @}
