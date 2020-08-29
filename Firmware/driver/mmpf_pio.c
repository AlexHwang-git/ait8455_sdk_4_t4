//==============================================================================
//
//  File        : mmpf_pio.c
//  Description : PIO Pins Control Interface
//  Author      : Ben Lu
//  Revision    : 1.0
//
//==============================================================================
/**
 *  @file mmpf_pio.c
 *  @brief The PIO pins control functions
 *  @author Ben Lu
 *  @version 1.0
 */
 
#include "config_fw.h"
#include "mmpf_typedef.h"
#include "lib_retina.h"
#include "reg_retina.h"
#include "mmp_err.h"
#include "mmpf_pio.h"
#include "pCam_msg.h"

/** @addtogroup MMPF_PIO
@{
*/

//static MMP_ULONG OutputModeProtect[4];
static MMPF_OS_SEMID  gPIO_GpioModeSemID;
static MMPF_OS_SEMID  gPIO_OutputModeSemID;
static MMPF_OS_SEMID  gPIO_SetDataSemID;
static MMPF_OS_SEMID  gPIO_EnableTrigModeSemID;
static MMPF_OS_SEMID  gPIO_EnableInterruptSemID;
static MMP_ULONG gPIO_BoundingTime[PIO_MAX_PIN_SIZE];
static PioCallBackFunc *gPIO_CallBackFunc[PIO_MAX_PIN_SIZE];


extern MMP_UBYTE		m_gbKeypadCheck[];
//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_Initialize
//  Description :
//------------------------------------------------------------------------------
/** @brief The function registers the interrupt and create related semaphore for PIO pins.

The function registers the interrupt and create related semaphore for PIO pins. 
@return It reports the status of the operation.
*/
MMP_ERR MMPF_PIO_Initialize(void)
{
	MMP_USHORT i = 0;
	AITPS_AIC   pAIC = AITC_BASE_AIC;
    static MMP_BOOL m_bInitialFlag = MMP_FALSE;
    
    if (m_bInitialFlag == MMP_FALSE) {
	    RTNA_AIC_Open(pAIC, AIC_SRC_GPIO, gpio_isr_a,
	                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
	    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_GPIO);

		gPIO_GpioModeSemID =  MMPF_OS_CreateSem(1);
		gPIO_OutputModeSemID =  MMPF_OS_CreateSem(1);
		gPIO_SetDataSemID =  MMPF_OS_CreateSem(1);
		gPIO_EnableTrigModeSemID = MMPF_OS_CreateSem(1);
		gPIO_EnableInterruptSemID = MMPF_OS_CreateSem(1);
		MEMSET0(gPIO_BoundingTime);

		for ( i = 0; i < PIO_MAX_PIN_SIZE ; i++) {
			gPIO_CallBackFunc[i] = NULL;
		}
		m_bInitialFlag = MMP_TRUE;
	}	
	return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_EnableOutputMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set the PIN as GPIO mode (pin should be set as GPIO mode before using GPIO module to control signals)

The function set the PIN as GPIO mode (pin should be set as GPIO mode before using GPIO module to control signals) 
@param[in] piopin is the PIO number, please reference the data structure of MMPF_PIO_REG
@param[in] bEnable is the choice of output mode or input mode
@return It reports the status of the operation.
*/
MMP_ERR MMPF_PIO_EnableGpioMode(MMPF_PIO_REG piopin, MMP_BOOL bEnable)
{
	MMP_UBYTE		ret;
	AITPS_GBL   pGBL = AITC_BASE_GBL;
	
	ret = MMPF_OS_AcquireSem(gPIO_GpioModeSemID, PIO_SEM_TIMEOUT);
	if(bEnable == MMP_TRUE) {
		if (piopin < MMPF_PIO_REG_GPIO8) {
			pGBL->GBL_IO_CTL5 |= (0x1 << piopin);
		}
		else if (piopin < MMPF_PIO_REG_GPIO16) {
			pGBL->GBL_IO_CTL6 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO8)); 
		}
		else if (piopin < MMPF_PIO_REG_GPIO24) {
			pGBL->GBL_IO_CTL7 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO16)); 
		}
		else if (piopin < MMPF_PIO_REG_GPIO32) {
			pGBL->GBL_IO_CTL8 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO24));
		}
		else if (piopin < MMPF_PIO_REG_GPIO40) {	
			pGBL->GBL_IO_CTL9 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO32));
		}
		#if (CHIP == VSN_V2)
		else {
			pGBL->GBL_IO_CTL10 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO40));
		}
		#endif
		#if (CHIP == VSN_V3)
		else if (piopin < MMPF_PIO_REG_GPIO48) {	
			pGBL->GBL_IO_CTL10 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO40));
		}
		else if (piopin < MMPF_PIO_REG_GPIO56) {	
			pGBL->GBL_IO_CTL11 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO48));
		}
		else if (piopin < MMPF_PIO_REG_GPIO64) {	
			pGBL->GBL_IO_CTL12 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO56));
		}
		else {
			pGBL->GBL_IO_CTL13 |= (0x1 << (piopin - MMPF_PIO_REG_GPIO64));
		}
		#endif
	} else {
		if (piopin < MMPF_PIO_REG_GPIO8) {
			pGBL->GBL_IO_CTL5 &= ~(0x1 << piopin);
		}
		else if (piopin < MMPF_PIO_REG_GPIO16) {
			pGBL->GBL_IO_CTL6 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO8)); 
		}
		else if (piopin < MMPF_PIO_REG_GPIO24) {
			pGBL->GBL_IO_CTL7 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO16)); 
		}
		else if (piopin < MMPF_PIO_REG_GPIO32) {
			pGBL->GBL_IO_CTL8 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO24));
		}
		else if (piopin < MMPF_PIO_REG_GPIO40) {
			pGBL->GBL_IO_CTL9 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO32));
		}
		#if (CHIP == VSN_V2)
		else {
			pGBL->GBL_IO_CTL10 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO40));
		}
		#endif
		#if (CHIP == VSN_V3)
		else if (piopin < MMPF_PIO_REG_GPIO48) {
			pGBL->GBL_IO_CTL9 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO40));
		}
		else if (piopin < MMPF_PIO_REG_GPIO56) {
			pGBL->GBL_IO_CTL9 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO48));
		}
		else if (piopin < MMPF_PIO_REG_GPIO64) {
			pGBL->GBL_IO_CTL9 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO56));
		}
		else {
			pGBL->GBL_IO_CTL10 &= ~(0x1 << (piopin - MMPF_PIO_REG_GPIO64));
		}
		#endif
	
	}
	ret = MMPF_OS_ReleaseSem(gPIO_GpioModeSemID);
	return MMP_ERR_NONE;	
}
//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_EnableOutputMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set the PIN as Output mode (bEnable = MMP_TRUE) or Input mode.

The function set the PIN as Output mode (bEnable = MMP_TRUE) or Input mode. 
@param[in] piopin is the PIO number, please reference the data structure of MMPF_PIO_REG
@param[in] bEnable is the choice of output mode or input mode
@return It reports the status of the operation.
*/
MMP_ERR MMPF_PIO_EnableOutputMode(MMPF_PIO_REG piopin, MMP_BOOL bEnable)
{
	MMP_UBYTE bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	MMP_UBYTE IndexAddressInfo = PIO_GET_INDEX(piopin);
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	MMP_UBYTE		ret;  
	
		
	if(bEnable) {
		ret = MMPF_OS_AcquireSem(gPIO_OutputModeSemID, PIO_SEM_TIMEOUT);
		pGPIO->GPIO_EN[IndexAddressInfo] |= (0x1 << bitPostionInfo);
		ret = MMPF_OS_ReleaseSem(gPIO_OutputModeSemID);	
	}
	else {
		ret = MMPF_OS_AcquireSem(gPIO_OutputModeSemID, PIO_SEM_TIMEOUT);
		pGPIO->GPIO_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
		ret = MMPF_OS_ReleaseSem(gPIO_OutputModeSemID);
	}	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_SetData
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set the PIO pin as High or Low (When the pin is at output mode).

The function set the PIO pin as High or Low (When the pin is at output mode).
@param[in] piopin is the PIO number, please reference the data structure of MMPF_PIO_REG
@param[in] outputValue is 0 stands for Low, otherwise it stands for High
@return It reports the status of the operation.
*/
MMP_ERR MMPF_PIO_SetData(MMPF_PIO_REG piopin, MMP_UBYTE outputValue)
{
	MMP_UBYTE bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	MMP_UBYTE IndexAddressInfo = PIO_GET_INDEX(piopin);
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	MMP_UBYTE		ret; 
	
	
	if((pGPIO->GPIO_EN[IndexAddressInfo] & (0x1 << bitPostionInfo)) == 0x0){
		//"Error !!! PIO Input Mode to call  MMPF_PIO_SetData
		return MMP_PIO_ERR_INPUTMODESETDATA;
	}
	
	if(outputValue) {
		ret = MMPF_OS_AcquireSem(gPIO_SetDataSemID, PIO_SEM_TIMEOUT);	
		pGPIO->GPIO_DATA[IndexAddressInfo] |= (0x1 << bitPostionInfo);
		ret = MMPF_OS_ReleaseSem(gPIO_SetDataSemID);
	}
	else {
		ret = MMPF_OS_AcquireSem(gPIO_SetDataSemID, PIO_SEM_TIMEOUT);
		pGPIO->GPIO_DATA[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
		ret = MMPF_OS_ReleaseSem(gPIO_SetDataSemID);
	}	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_GetData
//  Description :
//------------------------------------------------------------------------------
/** @brief The function get the PIO pin's singal. (When the pin is at input mode).

The function get the PIO pin's singal. (When the pin is at input mode).
@param[in] piopin is the PIO number, please reference the data structure of MMPF_PIO_REG
@param[out] returnValue is standing for the High or Low signal.
@return It reports the status of the operation.
*/
MMP_ERR MMPF_PIO_GetData(MMPF_PIO_REG piopin, MMP_UBYTE* returnValue)
{
	MMP_UBYTE bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	MMP_UBYTE IndexAddressInfo = PIO_GET_INDEX(piopin);
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	
	
	if((pGPIO->GPIO_EN[IndexAddressInfo] & (0x1 << bitPostionInfo)) != 0x0){
		//Error !!! PIO Output Mode to call MMPF_PIO_GetData
		return MMP_PIO_ERR_OUTPUTMODEGETDATA;
	}
	
	*returnValue = ((pGPIO->GPIO_DATA[IndexAddressInfo] >> bitPostionInfo) & 0x1);  //AIT_PMP,fix bug,201003
	return MMP_ERR_NONE;
}



//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_GetData
//  Description :
//------------------------------------------------------------------------------
/** @brief The function get the PIO pin's singal. (When the pin is at input mode).

The function get the PIO pin's singal. (When the pin is at input mode).
@param[in] piopin is the PIO number, please reference the data structure of MMPF_PIO_REG
@param[in] trigmode set the pio pin as edge trigger (H2L or L2H) or level trigger (H or L)
@param[out] bEnable make the tirgger settings work or not.
@return It reports the status of the operation.
*/
MMP_ERR MMPF_PIO_EnableTrigMode(MMPF_PIO_REG piopin, MMPF_PIO_TRIGMODE  trigmode, MMP_BOOL bEnable)
{
	MMP_UBYTE bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	MMP_UBYTE IndexAddressInfo = PIO_GET_INDEX(piopin);
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	MMP_UBYTE ret; 
	
	
	if(bEnable) {
		switch(trigmode){
			case MMPF_PIO_TRIGMODE_EDGE_H2L:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				//pGPIO->GPIO_INT_L2H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));	
				pGPIO->GPIO_INT_H2L_EN[IndexAddressInfo] |= (0x1 << bitPostionInfo);
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
			case MMPF_PIO_TRIGMODE_EDGE_L2H:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				//pGPIO->GPIO_INT_H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_H2L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				pGPIO->GPIO_INT_L2H_EN[IndexAddressInfo] |= (0x1 << bitPostionInfo);
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
			case MMPF_PIO_TRIGMODE_LEVEL_H:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				//pGPIO->GPIO_INT_H2L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_L2H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				pGPIO->GPIO_INT_H_EN[IndexAddressInfo] |= (0x1 << bitPostionInfo);
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
			case MMPF_PIO_TRIGMODE_LEVEL_L:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				//pGPIO->GPIO_INT_H2L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_L2H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				//pGPIO->GPIO_INT_H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				pGPIO->GPIO_INT_L_EN[IndexAddressInfo] |= (0x1 << bitPostionInfo);
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
		}
	}
	else {
		switch(trigmode){
			case MMPF_PIO_TRIGMODE_EDGE_H2L:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				pGPIO->GPIO_INT_H2L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
			case MMPF_PIO_TRIGMODE_EDGE_L2H:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				pGPIO->GPIO_INT_L2H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
			case MMPF_PIO_TRIGMODE_LEVEL_H:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				pGPIO->GPIO_INT_H_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
			case MMPF_PIO_TRIGMODE_LEVEL_L:
				ret = MMPF_OS_AcquireSem(gPIO_EnableTrigModeSemID, PIO_SEM_TIMEOUT);
				pGPIO->GPIO_INT_L_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
				ret = MMPF_OS_ReleaseSem(gPIO_EnableTrigModeSemID);
				break;
		}
	}	
	return MMP_ERR_NONE;
}

/*MMP_ERR MMPF_PIO_GetInterruptStatus(MMPF_PIO_REG piopin, MMP_UBYTE *returnStatus)
{
	MMP_UBYTE bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	MMP_UBYTE IndexAddressInfo = PIO_GET_INDEX(piopin);
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	
	*returnStatus = (pGPIO->GPIO_INT_CPU_SR[IndexAddressInfo] & (0x1 << bitPostionInfo));
	return MMP_ERR_NONE;
}

MMP_ERR MMPF_PIO_CleanInterruptStatus(MMPF_PIO_REG piopin)
{
	MMP_UBYTE bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	MMP_UBYTE IndexAddressInfo = PIO_GET_INDEX(piopin);
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	
	pGPIO->GPIO_INT_CPU_SR[IndexAddressInfo] &= (0x1 << bitPostionInfo);
	return MMP_ERR_NONE;
}*/



//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_EnableInterrupt
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set the PIO pin's interrupt actions.

The function set the PIO pin's interrupt actions.
@param[in] piopin is the PIO number, please reference the data structure of MMPF_PIO_REG
@param[in] bEnable stands for enable interrupt or not.
@param[in] boundingTime is used for advanced interrupt settings.
@param[in] CallBackFunc is used by interrupt handler.
@return It reports the status of the operation.
*/
MMP_ERR MMPF_PIO_EnableInterrupt(MMPF_PIO_REG piopin, MMP_BOOL bEnable, MMP_ULONG boundingTime, PioCallBackFunc *CallBackFunc)
{
	MMP_UBYTE bitPostionInfo = piopin & PIO_BITPOSITION_INFO;
	MMP_UBYTE IndexAddressInfo = PIO_GET_INDEX(piopin);
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	MMP_UBYTE ret;
	
	
	if(bEnable) {
		
			gPIO_BoundingTime[piopin] = boundingTime;
			gPIO_CallBackFunc[piopin] = CallBackFunc;	
		
		ret = MMPF_OS_AcquireSem(gPIO_EnableInterruptSemID, PIO_SEM_TIMEOUT);
		pGPIO->GPIO_INT_CPU_EN[IndexAddressInfo] |= (0x1 << bitPostionInfo);
		ret = MMPF_OS_ReleaseSem(gPIO_EnableInterruptSemID);
	}
	else {
		ret = MMPF_OS_AcquireSem(gPIO_EnableInterruptSemID, PIO_SEM_TIMEOUT);
		pGPIO->GPIO_INT_CPU_EN[IndexAddressInfo] &= (~(0x1 << bitPostionInfo));
		ret = MMPF_OS_ReleaseSem(gPIO_EnableInterruptSemID);
		
			gPIO_BoundingTime[piopin] = 0;
			gPIO_CallBackFunc[piopin] = NULL;
		
	}	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_PIO_ISR
//  Description : 
//------------------------------------------------------------------------------
/** @brief PIO pin's interrupt handler function.

PIO pin's interrupt handler function.
@return It reports the status of the operation.
*/

extern  MMP_ULONG   glSystemEvent;
extern MMPF_OS_FLAGID SYS_Flag_Hif;

//#include "mmpf_usb_msg.h"

void MMPF_PIO_ISR(void)
{
	MMP_ULONG i = 0, j = 0;
	MMP_ULONG  intsrc_H = 0, intsrc_L = 0, intsrc_H2L = 0, intsrc_L2H = 0;
	MMPF_PIO_REG piopin = MMPF_PIO_REG_UNKNOWN;
	AITPS_GPIO pGPIO = AITC_BASE_GPIO;
	MMP_UBYTE bit_val;
	
	//To find out the GPIO number and clean the interrupt
	for (i = 0; i < 4; i++) {
		intsrc_H = pGPIO->GPIO_INT_CPU_EN[i] & pGPIO->GPIO_INT_H_SR[i];
		intsrc_L = pGPIO->GPIO_INT_CPU_EN[i] & pGPIO->GPIO_INT_L_SR[i];
		intsrc_H2L = pGPIO->GPIO_INT_CPU_EN[i] & pGPIO->GPIO_INT_H2L_SR[i];
		intsrc_L2H = pGPIO->GPIO_INT_CPU_EN[i] & pGPIO->GPIO_INT_L2H_SR[i];
		if(intsrc_H != 0x0){
			pGPIO->GPIO_INT_H_SR[i] = intsrc_H;  //clean interrupt
			for (j = 1; j <= 0x20; j++){
				if((intsrc_H >> j) == 0) break; 
			}
			piopin = i*0x20 + (j-1);
			break;
		}
		if(intsrc_L != 0x0){
			pGPIO->GPIO_INT_L_SR[i] = intsrc_L;  //clean interrupt
			for (j = 1; j <= 0x20; j++){
				if((intsrc_L >> j) == 0) break; 
			}
			piopin = i*0x20 + (j-1);
			break;
		}
		
		if(intsrc_H2L != 0x0){
			pGPIO->GPIO_INT_H2L_SR[i] = intsrc_H2L;  //clean interrupt
			for (j = 1; j <= 0x20; j++){
				if((intsrc_H2L >> j) == 0) break; 
			}
			piopin = i*0x20 + (j-1);
			break;
		}
		
		if(intsrc_L2H != 0x0){
			pGPIO->GPIO_INT_L2H_SR[i] = intsrc_L2H;  //clean interrupt
			for (j = 1; j <= 0x20; j++){
				if((intsrc_L2H >> j) == 0) break; 
			}
			piopin = i*0x20 + (j-1);
			break;
		}
		
	}

	if (piopin != MMPF_PIO_REG_UNKNOWN) {
		if(gPIO_CallBackFunc[piopin] != NULL) {
			(*gPIO_CallBackFunc[piopin]) (piopin);
		}
	
	}

//    RTNA_DBG_Str(3, "GPIO INT : ");
//    RTNA_DBG_Short(3, piopin);
//    RTNA_DBG_Str(3, "    ");

#if (SUPPORT_ROTARY_ENCODER)
#if (SUPPORT_TIMER_CTL_ENCODER == 0)

	if (piopin != MMPF_PIO_REG_UNKNOWN)
	{
        glSystemEvent &= ~(ROTARY_SWITCH_CW|ROTARY_SWITCH_CCW);
        //dbg_printf(0,"%d\r\n",glSystemEvent);

        /*********************************************************************
         * Refer to datasheet config.1
         * (1) ClockWise Sequence   : A High 2 Low then B Low
         *     (1-1) A(Light+) High 2 Low is trigger on MMPC_GPIO_InitConfig
         *     (1-2) Get B(Light-) PIO status         
         *
         * (2) Counter C.W. Sequence: B High 2 Low then A Low       
         *********************************************************************/

        // Get B (Light-)
        // RTNA_WAIT_MS(4);
        MMPF_PIO_GetData(LIGHT_NEGATIVE, &bit_val);

        // then B low
        if(bit_val == 0)
        {
            dbg_printf(0,"C.W.\r\n");
            glSystemEvent |= ROTARY_SWITCH_CW;

            if(glSystemEvent != 0)
			    MMPF_PCAM_PostMsg(0,TASK_PCAM_PRIO,PCAM_MSG_OTHERS,0,(void *)0);
        }
        else
        {
            dbg_printf(0,"C.C.W.\r\n");
            glSystemEvent |= ROTARY_SWITCH_CCW;

            if(glSystemEvent != 0)
			    MMPF_PCAM_PostMsg(0,TASK_PCAM_PRIO,PCAM_MSG_OTHERS,0,(void *)0);
        }
    }
#endif
#endif

	return;
}

/** @} */ // MMPF_PIO
