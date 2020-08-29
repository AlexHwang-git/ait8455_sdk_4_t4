#ifndef _MMPF_WD_H_
#define _MMPF_WD_H_
#include "os_wrap.h"


typedef struct _AITS_WD {//0xFFFF 8000~800F
    AIT_REG_D   WD_MODE_CTL0;
        #define WD_CTL_ACCESS_KEY       0x2340
        #define WD_INT_EN               0x04 
        #define WD_RT_EN                0x02 
        #define WD_EN                   0x01      
    AIT_REG_D   WD_MODE_CTL1;
        #define WD_CLK_CTL_ACCESS_KEY   0x3700       
    AIT_REG_D   WD_RE_ST;            
        #define WD_RESTART              0xC071
    AIT_REG_D   WD_SR;        
        #define WD_RESET_SR             0x02
        #define WD_OVERFLOW_SR          0x01
} AITS_WD, *AITPS_WD;

typedef enum _WD_CLK_DIVIDER
{
	WD_CLK_MCK_D8    = 0x00, 
	WD_CLK_MCK_D32,
	WD_CLK_MCK_D128,
	WD_CLK_MCK_D1024
}WD_CLK_DIVIDER;

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
typedef void WdCallBackFunc(void);
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
void MMPF_WD_ISR(void);
MMP_ERR MMPF_WD_GetStatus(MMP_ULONG *status);
MMP_ERR MMPF_WD_Initialize(void);
MMP_ERR MMPF_WD_EnableWD(MMP_BOOL bEnable, MMP_BOOL bResetCPU, MMP_BOOL bEnableInterrupt, WdCallBackFunc *CallBackFunc, MMP_BOOL bRestetAllModules);
MMP_ERR MMPF_WD_SetTimeOut(MMP_UBYTE ubCounter, MMP_USHORT clockDiv);
MMP_ERR MMPF_WD_Kick(void);
#endif