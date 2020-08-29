//==============================================================================
//
//  File        : mmpf_uart.h
//  Description : INCLUDE File for the Firmware Timer Control Driver
//  Author      : Jerry Lai
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_TIMER_H_
#define _MMPF_TIMER_H_

//#include    "includes.h"


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


//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

typedef enum _MMPF_TIMER_ID
{
	MMPF_TIMER_0 =0,
	MMPF_TIMER_1,
	MMPF_TIMER_2
} MMPF_TIMER_ID;


typedef void TimerCallBackFunc(void);              //New for callback function

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//New API: To set AIC source and assign callback function. It will call SetAICISR()
//call RTNA_AIC.... to set the interrupt source.
MMP_ERR    MMPF_Timer_OpenInterrupt(MMPF_TIMER_ID id);             

//New API: To replace following 2 APIs with this one
//RTNA_AIC_IRQ_En or RTNA_AIC_IRQ_Dis to turn on the AIC interrupt status
// and then turn on Timer interrupt in Timer module. (Merge MMPF_TimerEnableIRQ function)
MMP_ERR    MMPF_Timer_EnableInterrupt(MMPF_TIMER_ID id, MMP_BOOL bEnable);
//--MMP_ERR    MMPF_Timer_EnableAICTimerSRC(MMPF_TIMER_TC tc);
//--MMP_ERR    MMPF_Timer_DisableAICTimerSRC(MMPF_TIMER_TC tc);

void MMPF_TIMER1_ISR(void);    //The ISR. It will call callback function.
void MMPF_TIMER2_ISR(void);    

//FW is urge to use MMPF_Timer_OpenMS insted of MMPF_Timer_Open. 
//OpenMS will get CPU frequency and calculate the delay
MMP_ERR MMPF_Timer_Open(MMPF_TIMER_ID id, MMP_ULONG ulMs, TimerCallBackFunc *Func);
MMP_ERR MMPF_Timer_Close(MMPF_TIMER_ID id);
MMP_ERR MMPF_Timer_OpenUs(MMPF_TIMER_ID id, MMP_ULONG ulUs, TimerCallBackFunc *Func);
//MMP_ERR MMPF_Timer_TCSync(void);

//New API: To replace following 2 APIs with this one
// void MMPF_Timer_EnableIRQ(MMPF_TIMER_ID id, MMP_ULONG ulType, MMP_BOOL bEnable);
//--void MMPF_Timer_EnableIRQ(MMPF_TIMER_TC tc, MMP_ULONG type);
//--void MMPF_Timer_DisableIRQ(MMPF_TIMER_TC tc, MMP_ULONG type);



//New API: To get the status of each timer. In case some FW need more then 1 timer
//and need to check which timer triggered the ISR. 
//MMP_UBYTE  MMPF_Timer_GetSR(MMPF_TIMER_TC tc);   


// Please do not use RTNA_TC.... again. RTNA_AIC can be used.
// All of the register access, please enter from AITPS_TCB not AITPS_TC, and remove the
// AITC_BASE_TC0, AITC_BASE_TC1, AITC_BASE_TC2 base declaration
// Please use the newest VSS version to modify the codes.

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================




#endif // _INCLUDES_H_
