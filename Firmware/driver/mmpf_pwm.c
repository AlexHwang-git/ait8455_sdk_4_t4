//==============================================================================
//
//  File        : mmpf_pwm.c
//  Description : PWM control driver
//  Author      : Ben Lu
//  Revision    : 1.0
//
//==============================================================================
/**
 *  @file mmpf_pwm.c
 *  @brief PWM control driver
 *  @author Ben Lu
 *  @version 1.0
 */
 
#include "config_fw.h"
#include "mmpf_typedef.h"
#include "lib_retina.h"
#include "reg_retina.h"
#include "mmp_err.h"
#include "os_wrap.h"
#include "mmp_reg_gpio.h"
#include "mmpf_pio.h"
#include "mmpf_pwm.h"
static PwmCallBackFunc *gPwmCallBack[MMPF_PWM_ID_MAX];
static MMPF_OS_SEMID  	gPWMSemID[MMPF_PWM_ID_MAX];
extern MMPF_OS_MQID  	HighTask_QId;
extern MMP_BOOL m_bVidRecdPreviewStatus[];
extern void pwm_isr_a();
void MMPF_PWM_ISR(void)
{

   AITPS_AIC pAIC = AITC_BASE_AIC;
   AITPS_PWMB pPWM = (AITPS_PWMB)AITC_BASE_PWM;
   MMP_USHORT pwmstatus[2];
      
   pwmstatus[0] = pPWM->PWM[0].PWM_INT_CPU_EN & pPWM->PWM[0].PWM_INT_CPU_SR;
   pwmstatus[1] = pPWM->PWM[1].PWM_INT_CPU_EN & pPWM->PWM[1].PWM_INT_CPU_SR;

	if(pwmstatus[1] != 0){
		pPWM->PWM[1].PWM_INT_CPU_SR = 0x7;
	   	if(m_bVidRecdPreviewStatus[0] || m_bVidRecdPreviewStatus[1]){
			if(pPWM->PWM[1].PWM_PULSE_A_T3 >=0xFE00){
				RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_PWM);
			}
		    else{
		    	pPWM->PWM[1].PWM_PULSE_A_T3 += 0x80;	
			}
   		} else { 
			if(pPWM->PWM[1].PWM_PULSE_A_T3 >= 0x7){
				pPWM->PWM[1].PWM_PULSE_A_T3 -= 0x80;	
			}
		    else{
				RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_PWM);
				pPWM->PWM[1].PWM_CTL = (PWM_PULSE_A_FIRST|PWM_ONE_ROUND|PWM_PULSE_A_NEG|PWM_EN);	
			}
		}
	}
	
	if(pwmstatus[0] != 0){
		pPWM->PWM[0].PWM_INT_CPU_SR = 0x7;   
	} 
   
}

MMP_ERR MMPF_PWM_Initialize(void)
{
#if (CHIP==P_V2)
	MMP_UBYTE i = 0;
	AITPS_AIC pAIC = AITC_BASE_AIC;
    AITPS_GBL pGBL = AITC_BASE_GBL;
	static MMP_BOOL ubInitFlag = MMP_FALSE;
	pGBL->GBL_CLK_EN |= GBL_CLK_I2CM;

	RTNA_WAIT_MS(5);
	if (ubInitFlag == MMP_FALSE) {
		RTNA_AIC_Open(pAIC, AIC_SRC_I2C_PWM, pwm_isr_a,
	                   AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 7);
	    RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_I2C_PWM);
	    
	    for(i = 0; i < MMPF_PWM_ID_MAX; i++) {
	 //   	gPwmCallBack[i] = NULL;
	    //	gPWMSemID[i] = MMPF_OS_CreateSem(1);
	    }
	    ubInitFlag = MMP_TRUE;
    }
	 pGBL->GBL_PWM_1_IO_CTL = 0x8;  // PWM1 => BGPIO9
	 pGBL->GBL_PWM_0_IO_CTL = 0x1;  // PWM0 => BGPIO8
#endif

#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
	AITPS_AIC pAIC = AITC_BASE_AIC;
    AITPS_GBL pGBL = AITC_BASE_GBL;
	static MMP_BOOL ubInitFlag = MMP_FALSE;

	RTNA_WAIT_MS(5);
	if (ubInitFlag == MMP_FALSE) {
		RTNA_AIC_Open(pAIC, AIC_SRC_PWM, pwm_isr_a,
	                   AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 7);
	    RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_PWM);

	    ubInitFlag = MMP_TRUE;
    }
#if (CHIP == VSN_V3)		
	pGBL->GBL_PROBE_CTL |= GBL_JTAG_PDGPIO_SEL;	// JTAG => PDGPIO 0-4
#endif	
	pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_PWM_DIS);	// Enable PWM clock
	pGBL->GBL_IO_CTL2 |= (GBL_PWM1_2_PB_GPIO11|GBL_PWM2_2_PB_GPIO13);	// PWM1 => PBGPIO11, PWM2 => PBGPIO13
	//pGBL->GBL_IO_CTL2 |= GBL_PWM1_2_PSNR_D8;	// PWM1 => PSNRD8
	//dbg_printf(0, "GBL_IO_CTL2 = %x\r\n", pGBL->GBL_IO_CTL2);
	
	// Set PBGPIO11,13 NOT GPIO
	// pGBL->GBL_IO_CTL7 &= ~(0x28);
#endif	
}


MMP_ERR MMPF_PWM_SetAttribe(MMPF_PWM_ATTRIBUTE* uPwmAttribute)
{
	AITPS_PWMB pPWM = (AITPS_PWMB)AITC_BASE_PWM;
	MMP_UBYTE ret = 0xFF;
	//ret = MMPF_OS_AcquireSem(gPWMSemID[uPwmAttribute->uID], PWM_SEM_TIMEOUT);
	if (uPwmAttribute->uPulseID == MMPF_PWM_PULSE_ID_A) {
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T0 = uPwmAttribute->usClkDuty_T0;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T1 = uPwmAttribute->usClkDuty_T1;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T2 = uPwmAttribute->usClkDuty_T2;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T3 = uPwmAttribute->usClkDuty_T3;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_CLK_DIV = uPwmAttribute->usClockDiv;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_NUM = uPwmAttribute->ubNumOfPulses;
	}
	else if (uPwmAttribute->uPulseID == MMPF_PWM_PULSE_ID_B) {
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T0 = uPwmAttribute->usClkDuty_T0;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T1 = uPwmAttribute->usClkDuty_T1;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T2 = uPwmAttribute->usClkDuty_T2;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T3 = uPwmAttribute->usClkDuty_T3;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_CLK_DIV = uPwmAttribute->usClockDiv;
		pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_NUM = uPwmAttribute->ubNumOfPulses;
	}
	
	if (ret == 0) {
		//MMPF_OS_ReleaseSem(gPWMSemID[uPwmAttribute->uID]);
	}

	return MMP_ERR_NONE;
}

MMP_ERR MMPF_PWM_GetAttribe(MMPF_PWM_ATTRIBUTE* uPwmAttribute)
{
	AITPS_PWMB pPWM = (AITPS_PWMB)AITC_BASE_PWM;
	MMP_UBYTE ret = 0xFF;
	//ret = MMPF_OS_AcquireSem(gPWMSemID[uPwmAttribute->uID], PWM_SEM_TIMEOUT);
	if (uPwmAttribute->uPulseID == MMPF_PWM_PULSE_ID_A) {
		uPwmAttribute->usClkDuty_T0 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T0 ;
		uPwmAttribute->usClkDuty_T1 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T1 ;
		uPwmAttribute->usClkDuty_T2 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T2 ;
		uPwmAttribute->usClkDuty_T3 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_T3 ;
		uPwmAttribute->usClockDiv = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_CLK_DIV;
		uPwmAttribute->ubNumOfPulses = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_A_NUM;
	}
	else if (uPwmAttribute->uPulseID == MMPF_PWM_PULSE_ID_B) {
		uPwmAttribute->usClkDuty_T0 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T0;
		uPwmAttribute->usClkDuty_T1 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T1;
		uPwmAttribute->usClkDuty_T2 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T2;
		uPwmAttribute->usClkDuty_T3 = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_T3;
		uPwmAttribute->usClockDiv = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_CLK_DIV;
		uPwmAttribute->ubNumOfPulses = pPWM->PWM[uPwmAttribute->uID].PWM_PULSE_B_NUM;
	}
	
	if (ret == 0) {
		//MMPF_OS_ReleaseSem(gPWMSemID[uPwmAttribute->uID]);
	}

	return MMP_ERR_NONE;
}
MMP_ERR MMPF_PWM_EnableInterrupt(MMPF_PWM_ID uID, MMP_BOOL bEnable, PwmCallBackFunc CallBackFunc, MMPF_PWM_INT IntItem)
{
	MMP_UBYTE 	ret = 0xFF;
	AITPS_PWMB 	pPWM = (AITPS_PWMB)AITC_BASE_PWM;
	//ret = MMPF_OS_AcquireSem(gPWMSemID[uID], PWM_SEM_TIMEOUT);
	if (bEnable == MMP_TRUE) {
		//gPwmCallBack[uID] = CallBackFunc;
		pPWM->PWM[uID].PWM_INT_CPU_EN |= (0x1 << IntItem);
	}
	else {
		pPWM->PWM[uID].PWM_INT_CPU_EN &= ~(0x1 << IntItem);
		//gPwmCallBack[uID] = NULL;
	}
	
	if (ret == 0) {
	//	MMPF_OS_ReleaseSem(gPWMSemID[uID]);
	}
	
	return MMP_ERR_NONE;
}

MMP_ERR MMPF_PWM_ControlSet(MMPF_PWM_ID uID, MMP_UBYTE control)
{
#if (CHIP==P_V2)
	MMP_UBYTE 	ret = 0xFF;
	AITPS_AIC pAIC = AITC_BASE_AIC;
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_PWMB 	pPWM = (AITPS_PWMB)AITC_BASE_PWM;
	//ret = MMPF_OS_AcquireSem(gPWMSemID[uID], PWM_SEM_TIMEOUT);
	if(uID == 0)
		 pGBL->GBL_PWM_0_IO_CTL = 0x1;  // PWM0 => CGPIO8
	if(uID == 1)
		 pGBL->GBL_PWM_1_IO_CTL = 0x8;  // PWM1 => BGPIO9
	RTNA_AIC_IRQ_En(pAIC, AIC_SRC_I2C_PWM);
	pPWM->PWM[uID].PWM_CTL = control;
	if (ret == 0) {
		//MMPF_OS_ReleaseSem(gPWMSemID[uID]);
	}
#endif

#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
	//AITPS_GBL 	pGBL = AITC_BASE_GBL;
	AITPS_AIC 	pAIC = AITC_BASE_AIC;
	AITPS_PWMB 	pPWM = (AITPS_PWMB)AITC_BASE_PWM;
	
	RTNA_AIC_IRQ_En(pAIC, AIC_SRC_PWM);
	pPWM->PWM[uID].PWM_CTL = control;
#endif
	
	return MMP_ERR_NONE;
}