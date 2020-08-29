/// @ait_only
//==============================================================================
//
//  File        : mmpd_pwm.h
//  Description : INCLUDE File for the Host PWM Driver.
//  Author      : Rogers Chen
//  Revision    : 1.0
//
//==============================================================================


/**
 *  @file mmpd_pwm.h
 *  @brief The header File for the Host PWM Driver
 *  @author Rogers Chen
 *  @version 1.0
 */

#ifndef _MMPD_PWM_H_
#define _MMPD_PWM_H_

#include    "mmp_lib.h"

/** @addtogroup MMPD_PWM
 *  @{
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
#if (CHIP == PYTHON)|| (CHIP==P_V2)
#define PWM_OFFSET 0x1
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPD_PWM_ID
{
    MMPD_PWM_0 = 0,
    MMPD_PWM_1
} MMPD_PWM_ID;

#if (CHIP == PYTHON)|| (CHIP==P_V2)
typedef enum _MMPD_PWM_OUTPUT
{
	MMPD_PWM0_OUTPUT_PHD17 = 0,
	MMPD_PWM0_OUTPUT_PD0,
	MMPD_PWM0_OUTPUT_PLCD16,
    MMPD_PWM1_OUTPUT_PHGPIOCS,
    MMPD_PWM1_OUTPUT_PLCD17
} MMPD_PWM_OUTPUT;

typedef struct _MMPD_PWM_ATTRIBUTE
{
    MMP_BOOL            bActiveLow;
	MMP_BOOL            bClkHighSpeed;
	MMP_UBYTE           ubPhaseSel;
	MMP_UBYTE           ubClkDiv;
	MMPD_PWM_OUTPUT 	outputpin;
} MMPD_PWM_ATTRIBUTE;
#endif


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
#if (CHIP == APOLLO)
MMP_ERR MMPD_PWM_SetAttributes(MMPD_PWM_ID pwmID, MMPD_PWM_ATTRIBUTE *pwmattribute);
MMP_ERR MMPD_PWM_SetEnable(MMPD_PWM_ID pwmID, MMP_BOOL bEnable);
#endif
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================
/// @}

#endif // _INCLUDES_H_
/// @end_ait_only

