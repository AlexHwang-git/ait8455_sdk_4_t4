//==============================================================================
//
//  File        : mmpf_vif.c
//  Description : MMPF_VIF functions
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_rawproc.h"
#include "mmp_reg_gbl.h"
#include "mmpf_vif.h"
/** @addtogroup MMPF_VIF
@{
*/
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
MMPF_VIF_IN_TYPE m_VIFInterface[MMPF_VIF_MDL_MAX] = {MMPF_VIF_PARALLEL, MMPF_VIF_PARALLEL};
//==============================================================================
//
//                          MODULE VARIABLES
//
//==============================================================================
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
//*----------------------------------------------------------------------------
//* Function Name       : MMPF_VIF_SetPIOOutput
//* Input Parameters    : <mask>   = bit mask identifying the PIOs
//*                     : <config> = mask identifying the PIOs configuration
//* Output Parameters   : none
//* Functions called    : none
//*----------------------------------------------------------------------------
MMP_ERR	MMPF_VIF_SetPIODir(MMP_UBYTE mask, MMP_BOOL bOutput)
{
    AITPS_VIF   pVIF    = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;
    if (bOutput == MMP_TRUE) {
        if (vid == 0) {
            pVIF->VIF_0_SENSR_SIF_EN |= mask;
        }
        else {
            pVIF->VIF_1_SENSR_SIF_EN |= mask;
        }
    }
    else {
        if (vid == 0) {
            pVIF->VIF_0_SENSR_SIF_EN &= ~mask;
        }
        else {
            pVIF->VIF_1_SENSR_SIF_EN &= ~mask;
        }
    }
	return MMP_ERR_NONE;
}
//*----------------------------------------------------------------------------
//* Function Name       : MMPF_VIF_SetPIOOutput
//* Input Parameters    : <mask>   = bit mask identifying the PIOs
//*                     : <config> = mask identifying the PIOs configuration
//* Output Parameters   : none
//* Functions called    : none
//*----------------------------------------------------------------------------
MMP_ERR	MMPF_VIF_SetPIOOutput(MMP_UBYTE mask, MMP_BOOL bSetHigh)
{
    AITPS_VIF   pVIF    = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    if (bSetHigh == MMP_TRUE) {
        if (vid == 0) {
            pVIF->VIF_0_SENSR_SIF_DATA |= mask;
        }
        else {
            pVIF->VIF_1_SENSR_SIF_DATA |= mask;
        }
    }
    else {
        if (vid == 0) {
            pVIF->VIF_0_SENSR_SIF_DATA &= ~mask;
        }
        else {
            pVIF->VIF_1_SENSR_SIF_DATA &= ~mask;
        }
    }
	return MMP_ERR_NONE;
}
//*----------------------------------------------------------------------------
//* Function Name       : MMPF_VIF_GetPIOOutput
//* Input Parameters    : <mask>   = bit mask identifying the PIOs
//*                     : <config> = mask identifying the PIOs configuration
//* Output Parameters   : none
//* Functions called    : none
//*----------------------------------------------------------------------------
MMP_BOOL	MMPF_VIF_GetPIOOutput(MMP_UBYTE mask)
{
    AITPS_VIF   pVIF    = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    if (vid == 0) {
        return (pVIF->VIF_0_SENSR_SIF_DATA & mask)? MMP_TRUE: MMP_FALSE;
    }
    else {
        return (pVIF->VIF_1_SENSR_SIF_DATA & mask)? MMP_TRUE: MMP_FALSE;
    }
}


MMP_ERR	MMPF_VIF_RegisterInputInterface(MMPF_VIF_IN_TYPE type)
{
    MMP_UBYTE   vid = VIF_CTL;

    if(type == MMPF_VIF_MIPI){
        RTNA_DBG_Str0("Set MIPI mode\r\n");
    }
    else{
        RTNA_DBG_Str0("Set CCIR mode\r\n");
    }
    
    m_VIFInterface[vid] = type;

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIF_IsInterfaceEnable(MMP_BOOL *bEnable)
{
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
    MMP_UBYTE   vid = VIF_CTL;
    
    *bEnable = MMP_FALSE;
    
    if (m_VIFInterface[vid] == MMPF_VIF_MIPI) {
        if (pMIPI->MIPI_CLK_CFG[vid] & MIPI_CSI2_EN) {
            *bEnable = MMP_TRUE;
        }
    }
    else {
        if (pVIF->VIF_IN_EN[vid]) {
            *bEnable = MMP_TRUE;
        }
    }
    
   dbg_printf(3,"<Is.VIF : %d>\r\n",*bEnable);
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIF_EnableInputInterface(MMP_BOOL enable)
{
    AITPS_GBL  pGBL = AITC_BASE_GBL;
    AITPS_VIF  pVIF = AITC_BASE_VIF;
    AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
    MMP_UBYTE   vid = VIF_CTL;
    
    dbg_printf(3,"<VIF_%d : %d>\r\n", vid, enable);
    if(enable == MMP_TRUE){
        if(m_VIFInterface[vid] == MMPF_VIF_MIPI){		
            pVIF->VIF_OUT_EN[vid] = 0;
            pMIPI->MIPI_CLK_CFG[vid] |= MIPI_CSI2_EN;
            pVIF->VIF_OUT_EN[vid] = 1;
            //dbg_printf(3, "VIF_1: (x%x, x%x, x%x)\r\n", pVIF->VIF_IN_EN[VIF_CTL], pVIF->VIF_OUT_EN[VIF_CTL], pVIF->VIF_RAW_OUT_EN[VIF_CTL]);
        }
        else
        {        
            pVIF->VIF_OUT_EN[vid] = 0;
            pVIF->VIF_IN_EN[vid] = 1;
            pVIF->VIF_OUT_EN[vid] = 1;
        }
    }
    else{
        if (m_VIFInterface[vid] == MMPF_VIF_MIPI) {
        	if (vid == VIF_CTL_VIF1) {	
        	    pGBL->GBL_IO_CTL0 &= ~GBL_VIF1_BASE;
        	}
            pVIF->VIF_OUT_EN[vid] = 0;
            pMIPI->MIPI_CLK_CFG[vid] &= ~MIPI_CSI2_EN;
        }
        else {
            pVIF->VIF_IN_EN[vid] = 0;
        }    
    }
    
	return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIF_EnableOutput(MMP_BOOL enable)
{
    AITPS_VIF  pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    if(enable == MMP_TRUE){
        pVIF->VIF_OUT_EN[vid] = 1;
    }
    else{
        pVIF->VIF_OUT_EN[vid] = 0;
    }

	return MMP_ERR_NONE;
}


MMP_ERR MMPF_VIF_EnableInput(MMP_BOOL bEnable)
{
    AITPS_VIF  		pVIF 	= AITC_BASE_VIF;
    AITPS_MIPI  	pMIPI 	= AITC_BASE_MIPI;
    MMP_UBYTE       vid = VIF_CTL;
    
    if (bEnable == MMP_TRUE) 
    {
        if(m_VIFInterface[vid] == MMPF_VIF_MIPI)
        {                  
            pMIPI->MIPI_CLK_CFG[vid] 	|= MIPI_CSI2_EN;
        }
        else
        {   
            pVIF->VIF_IN_EN[vid] 	= 1;
    	}
    }
    else 
    {
        if (m_VIFInterface[vid] == MMPF_VIF_MIPI) 
        {	
            pMIPI->MIPI_CLK_CFG[vid] &= ~MIPI_CSI2_EN;
        }
        else 
        {
            pVIF->VIF_IN_EN[vid] = 0;
        }
    }

	return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIF_EnableIntMode( MMP_BOOL bEnable)
{
    AITPS_VIF  		pVIF 	= AITC_BASE_VIF;
    MMP_UBYTE       vid = VIF_CTL;

    if(bEnable == MMP_TRUE)
    {
        pVIF->VIF_INT_MODE[vid] 	= VIF_INT_EVERY_FRM;
    }
    else
    {
        pVIF->VIF_INT_MODE[vid] 	= 0;
    }

	return MMP_ERR_NONE;
}


MMP_ERR MMPF_VIF_SetGrabRange(MMP_UBYTE ubVid, MMPF_VIF_GRAB_INFO *pGrab)
{
    AITPS_VIF  pVIF = AITC_BASE_VIF;

    pVIF->VIF_GRAB[ubVid].PIXL_ST = pGrab->usStartX;
    pVIF->VIF_GRAB[ubVid].PIXL_ED = pGrab->usStartX + pGrab->usGrabWidth - 1;
    pVIF->VIF_GRAB[ubVid].LINE_ST = pGrab->usStartY;
    pVIF->VIF_GRAB[ubVid].LINE_ED = pGrab->usStartY + pGrab->usGrabHeight - 1;

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_VIF_GetVIFGrabResolution(MMP_ULONG *ulWidth, MMP_ULONG *ulHeight)
{
    AITPS_VIF  pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    *ulWidth = (pVIF->VIF_GRAB[vid].PIXL_ED - pVIF->VIF_GRAB[vid].PIXL_ST + 1);
    *ulHeight = (pVIF->VIF_GRAB[vid].LINE_ED - pVIF->VIF_GRAB[vid].LINE_ST + 1);

	return MMP_ERR_NONE;
}
/** @}*/ //end of MMPF_I2CM
