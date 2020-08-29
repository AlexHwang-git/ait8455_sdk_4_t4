//==============================================================================
//
//  File        : bsp.c
//  Description : Board support package source code
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================


#include "includes_fw.h"

#include "reg_retina.h"
#include "lib_retina.h"
#include "mmpf_pll.h"
#if WATCHDOG_RESET_EN==1
#include "pcam_msg.h"
#endif
/** @addtogroup BSP
@{
*/

#define USING_PROTECT_MODE                      //[jerry] for debugger variant


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define  IRQ_STK_SIZE           (256*3) // entry count
#define  FIQ_STK_SIZE           8

#ifdef ARMUL
MMP_ULONG   TickCtr;
#endif // ARMUL


//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

MMP_ULONG   *pIRQStkTop;
MMP_ULONG   *pFIQStkTop;

static MMP_ULONG m_systick ;
static MMP_ULONG m_systickclockbase ;

//==============================================================================
//
//                              PROTOTYPES
//
//==============================================================================

void  SetStackPointers(void);
//void MMPF_BSP_KickOffWatchDog(void);

#ifdef ARMUL
void  INT_Initialize(void);
#endif


//------------------------------------------------------------------------------
//  Function    : MMPF_BSP_Initialize
//  Description : Initialize BSP hardware configuration.
//------------------------------------------------------------------------------

MMP_ERR MMPF_BSP_Initialize(void)
{

    //[jerry] declare 32bit for compiler alignment.
    // 0807 swap array. because FIQ is no used
    static MMP_ULONG    FIQStackPool[FIQ_STK_SIZE];
    static MMP_ULONG    IRQStackPool[IRQ_STK_SIZE];

#ifdef ARMUL
    ARMULPS_INT pINT = ARMULC_BASE_INT;
    ARMULPS_TMR pTMR = ARMULC_BASE_TMR;
#else

#endif // ARMUL

    pIRQStkTop = &IRQStackPool[IRQ_STK_SIZE - 1];
    pFIQStkTop = &FIQStackPool[FIQ_STK_SIZE - 1];

#ifdef ARMUL
    pINT->IRQENSET = 0x00000000;
    pINT->FIQENSET = 0x00000000;
    pTMR->T1CTRL = 0x00000000;
    pTMR->T2CTRL = 0x00000000;
#else
    //AITC_BASE_MC->MC_RCR = 1;                           /* Remap SRAM to 0x00000000                            */
#endif // ARMUL


    MMPF_BSP_InitializeInt();                               // Initialize the interrupt controller
    SetStackPointers();                         /* Initialize the default and exception stacks         */


    //RTNA_Init();

    return MMP_ERR_NONE; //[TBD]
}

//------------------------------------------------------------------------------
//  Function    : MMPF_BSP_InitializeInt
//  Description : Initialize(reset) BSP interrupts
//------------------------------------------------------------------------------

MMP_ERR MMPF_BSP_InitializeInt(void)
{
#ifdef ARMUL
    *ARMULR_INT_IRQENSET = 0x00000000;
    *ARMULR_INT_FIQENSET = 0x00000000;
    INT_Initialize();
#else
    int i;

    AITPS_AIC pAIC = AITC_BASE_AIC;

    RTNA_AIC_IRQ_DisAll(pAIC);                  // Disable ALL interrupts
    RTNA_AIC_IRQ_ClearAll(pAIC);                // Clear ALL interrups if any pending

    #ifdef USING_PROTECT_MODE
    pAIC->AIC_DBR = AIC_DBG_EN;
    #else
    pAIC->AIC_DBR = 0;                          // Disable Protect Mode
    #endif

    for (i = 0; i < 8; i++) {
        RTNA_AIC_IRQ_EOI(pAIC);                 // End of all pending interrupt.
    }
                                                // 0xE51FFF20 is opcode of (ldr pc,[pc,#-0xf20])      */
    *(MMP_ULONG *)0x00000018L = 0xE51FFF20;        // IRQ exception vector - install redirection to AIC   */


#endif // ARMUL

    return MMP_ERR_NONE; //[TBD]
}


//------------------------------------------------------------------------------
//  Function    : MMPF_BSP_InitializeTimer
//  Description : Initialize hardware tick timer interrupt
//------------------------------------------------------------------------------

MMP_ERR MMPF_BSP_InitializeTimer(void)
{
//extern MMP_ULONG glCPUFreq;

#ifdef ARMUL
    *ARMULR_TMR_T1LOAD = 0x00002710;                                    // set timer tick 10000 cycle/tick
    *ARMULR_TMR_T1CTRL = ARMULC_TMR_CTRL_EN | ARMULC_TMR_CTRL_PERIOD;   // enable period timer interrupt
    *ARMULR_INT_IRQENSET = 0x00000010;                                  // enable TIMER1 source interrupt
#else
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_TC    pTC0 = AITC_BASE_TC0;
    AITPS_TCB   pTCB = AITC_BASE_TCB;
    MMP_ULONG   ulMClk = 0;

    //------------------------------
    // Timer Counter Initialization
    //------------------------------

    // System Timer initialization
    // Initialize TC0 to generate 10000 tick/sec

  
  
    #ifdef USING_PROTECT_MODE                       //[jerry] for debugger variant
    RTNA_TC_DBG_En(pTCB);
    #endif

    RTNA_AIC_Open(pAIC, AIC_SRC_TC0, OS_CPU_IRQ_ISR,
                   AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 7);

    #if (CHIP == VSN_V2)
    MMPF_PLL_GetCPUFreq(&ulMClk);
    RTNA_TC_Open(pTC0, TC_CPCTRG | TC_CLK_MCK_D8, 
                    ((ulMClk>>1)*1000/8)/OS_TICKS_PER_SEC);
    #endif
    #if (CHIP == VSN_V3)
    MMPF_PLL_GetGroupFreq(0, &ulMClk);
    RTNA_TC_Open(pTC0, TC_CPCTRG | TC_CLK_MCK_D8, 
                    ((ulMClk>>1)*1000/8)/OS_TICKS_PER_SEC);
    #endif

    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_TC0);
    RTNA_TC_SYNC(pTCB);
    RTNA_TC_IRQ_En(pTC0, TC_CPCS);
// V3 : TBD
    m_systick = 0 ;
    m_systickclockbase = /*glCPUFreq*/ USB_STC_CLK_KHZ / pTC0->TC_RC ;
   // dbg_printf(3,"ulMCLk : %d,m_systickclockbase:%d\r\n",ulMClk,m_systickclockbase);
#endif // ARMUL


    return MMP_ERR_NONE; //[TBD]
}

//------------------------------------------------------------------------------
//  Function    : MMPF_BSP_TimerHandler
//  Description : BSP tick timer interrupt handler
//------------------------------------------------------------------------------
MMP_ERR MMPF_BSP_TimerHandler(void)
{
extern MMP_BOOL MMPF_Detect_Sensor(void);
#if HEARTBEAT_LED_EN==1
extern MMP_USHORT USB_CheckTaskAlive(MMP_USHORT heartbeat);

static MMP_ULONG ind_bl2_toggle = 0;
static MMP_UBYTE ind_bl2_on_off = 0 ;
#endif

    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_TC    pTC0 = AITC_BASE_TC0;

    m_systick++;

    OSTimeTick();                       //  call OSTimeTick()

    #if OS_TMR_EN > 0
    OSTmrSignal();
    #endif

    #ifdef USING_PROTECT_MODE
    pAIC->AIC_IVR = 0x0;                // Write IVR to end interrupt (protect mode used)
    #endif
    
    MMPF_Detect_Sensor() ;
    
    #if HEARTBEAT_LED_EN==1
    if( !(ind_bl2_toggle & 0x3FF) ) {
        USB_CheckTaskAlive(ind_bl2_on_off & 1 );
        ind_bl2_on_off++;
    }
    ind_bl2_toggle++;
    #endif
    #if WATCHDOG_RESET_EN
    //MMPF_BSP_KickOffWatchDog();
    #endif
    
     
    RTNA_TC_SR_Clear(pTC0);            // clear interrupt status register
    pAIC->AIC_ICCR = 0x1 << AIC_SRC_TC0;    // Disable TC0 Interrupt on AIC

    pAIC->AIC_EOICR = 0x0;        // End of interrupt handler

    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_BSP_IntHandler
//  Description : BSP interrupt handler
//------------------------------------------------------------------------------

MMP_ERR MMPF_BSP_IntHandler(void)
{
    return	MMPF_BSP_TimerHandler();
}

//------------------------------------------------------------------------------
//  Function    : ARM_ExceptionHandler
//  Description : ARM exception handler
//------------------------------------------------------------------------------
void ARM_ExceptionHandler(MMP_ULONG exceptID, MMP_ULONG lr)
{
    switch(exceptID) {
    case 0x04:
        RTNA_DBG_Str(0, "Undefined Instruction at");
        break;
    case 0x08:
        RTNA_DBG_Str(0, "Software Interrupt at");
        break;
    case 0x0C:
        RTNA_DBG_Str(0, "Prefetch Abort at");
        break;
    case 0x10:
        RTNA_DBG_Str(0, "Data Abort at");
        break;
    default:
        RTNA_DBG_Str(0, "Unknow Exception at");
        break;
    }
    RTNA_DBG_Long(0, lr);
    RTNA_DBG_Str(0, "\r\n");
    while(1);
}

void MMPF_OSTaskStatHook(void)
{
#if OS_TASK_STAT_EN > 0
//    dbg_printf(0,"CPU Usage:%d\r\n",OSCPUUsage);
#endif
}

/** @brief Get OS tick number
@retval system tick
@note Return in tick, not in ms.
*/
MMP_ULONG MMPF_BSP_GetTick(void)
{
    return m_systick;
}

/** @brief Get CPU counter for the OS tick timer.

This would be the fractional part. The clock shall be reset after trigger the Timer ISR.
@retval The clock of the OS tick timer
*/
MMP_ULONG MMPF_BSP_GetTickClock(void)
{
    AITPS_TC pTC0 = AITC_BASE_TC0 ;
    return pTC0->TC_CVR * m_systickclockbase ;
}

//void MMPF_BSP_KickOffWatchDog(void)
//{
//#define WATCHDOG_CLEAN_PERIOD (500) // clean watchdog timer by 50 ms period
//#if WATCHDOG_RESET_EN==1
//extern MMPF_OS_FLAGID PCAM_Flag ;
//static int watchdog_period = 0 ;    
//    watchdog_period++ ;
//    if(watchdog_period>=WATCHDOG_CLEAN_PERIOD) {
//        MMPF_OS_SetFlags(PCAM_Flag, PCAM_FLAG_WATCHDOG, MMPF_OS_FLAG_SET);
//        watchdog_period = 0; 
//    }
//#endif

//}

/** @} */ // end of BSP