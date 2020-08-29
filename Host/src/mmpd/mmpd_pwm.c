/// @ait_only
//==============================================================================
//
//  File        : mmpd_pwm.c
//  Description : MMP PWM Module Control driver function
//  Author      : Rogers Chen
//  Revision    : 1.0
//
//==============================================================================

/**
*  @file mmpd_pwm.c
*  @brief The PWM Module Control functions
*  @author Rogers Chen
*  @version 1.0
*/

#include "mmp_lib.h"
#include "mmp_err.h"
#include "mmph_hif.h"
#include "mmpd_pwm.h"
#include "mmp_reg_gbl.h"
#if (CHIP == PYTHON)||(CHIP==P_V2)
#include "mmp_reg_vif.h"
#endif

#include "ait_utility.h"

/** @addtogroup MMPD_PWM
 *  @{
 */

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

//------------------------------------------------------------------------------
//  Function    : MMPD_PWM_SetAttributes
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function sets the attributes to the specified PWM with its PWM ID

The function sets the attributes to the specified PWM with its PWM ID. These attributes include 
polarity, clock selection, phase selsecion, clock divider and output pin. It is implemented by programming PWM registers to
set those attributes.

  @param[in] pwmID the PWM ID
  @param[in] pwmattribute the PWM attribute
  @return It reports the status of the operation.
*/
#if (CHIP == PYTHON)||(CHIP==P_V2)
MMP_ERR MMPD_PWM_SetAttributes(MMPD_PWM_ID pwmID, MMPD_PWM_ATTRIBUTE *pwmattribute)
{
	MMP_USHORT offset;
    MMP_USHORT tempVal;
    
    offset = pwmID*PWM_OFFSET; 
	MMPH_HIF_RegSetB(PWM_CTL + offset, (pwmattribute->bActiveLow<<6) | (pwmattribute->bClkHighSpeed<<5) | pwmattribute->ubPhaseSel);
	MMPH_HIF_RegSetB(PWM_CLK_DIV + offset, pwmattribute->ubClkDiv);

    //Set Multi-IO control
	switch(pwmID) {
		case MMPD_PWM_0:
			tempVal = MMPH_HIF_RegGetW(GBL_MIO_CTL_0) & (~0x0C02); //clear all PWM0 multi-IO
			switch(pwmattribute->outputpin) { //set new PWM0 multi-IO
				case MMPD_PWM0_OUTPUT_PHD17:
					MMPH_HIF_RegSetW(GBL_MIO_CTL_0, tempVal|0x0002); 
				break;	
				case MMPD_PWM0_OUTPUT_PD0:
					MMPH_HIF_RegSetW(GBL_MIO_CTL_0, tempVal|0x0800);
				break;
				case MMPD_PWM0_OUTPUT_PLCD16:
					MMPH_HIF_RegSetW(GBL_MIO_CTL_0, tempVal|0x0400);
				break;
			}	
		break;
		case MMPD_PWM_1:			
			tempVal = MMPH_HIF_RegGetW(GBL_MIO_CTL_0) & (~0x2004); //clear all PWM1 multi-IO
			switch(pwmattribute->outputpin) { //set new PWM1 multi-IO
				case MMPD_PWM1_OUTPUT_PHGPIOCS:
					MMPH_HIF_RegSetW(GBL_MIO_CTL_0, tempVal|0x0004);
				break;	
				case MMPD_PWM1_OUTPUT_PLCD17:
					MMPH_HIF_RegSetW(GBL_MIO_CTL_0, tempVal|0x2000);
				break;
			}	

			PRINTF("2. Reg0x6916 = 0x%2x  \r\n", MMPH_HIF_RegGetB(0x6916));
	        PRINTF("2. Reg0x6917 = 0x%2x  \r\n", MMPH_HIF_RegGetB(0x6917));
			
		break;
	}		
	return	MMP_ERR_NONE;	
}
#endif
//------------------------------------------------------------------------------
//  Function    : MMPD_PWM_SetEnable
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function enables or disables a PWM device

The function enables or disables a PWM device. It is implemented
by programming PWM registers.

  @param[in] pwmID the PWM ID
  @param[in] bEnable enable or disable PWM
  @return It reports the status of the operation.
*/
#if (CHIP == PYTHON)||(CHIP==P_V2)
MMP_ERR MMPD_PWM_SetEnable(MMPD_PWM_ID pwmID, MMP_BOOL bEnable)
{
	MMP_USHORT offset;
	MMP_UBYTE tempVal;

	offset = pwmID*PWM_OFFSET;
	tempVal = MMPH_HIF_RegGetB(PWM_CTL + offset);
	if(bEnable) {
		MMPH_HIF_RegSetB(PWM_CTL + offset, tempVal | PWM_EN); 
	}
	else {
		MMPH_HIF_RegSetB(PWM_CTL + offset, tempVal & (~PWM_EN)); 
	}	

	return	MMP_ERR_NONE;	
}
#endif
/// @}
/// @end_ait_only

