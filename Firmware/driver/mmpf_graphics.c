//==============================================================================
//
//  File        : mmpf_graphics.c
//  Description : Firmware Graphic Control Function
//  Author      : Ben Lu
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "mmpf_graphics.h"
#include "reg_retina.h"
#include "lib_retina.h"
#include "mmp_reg_graphics.h"
#include "mmp_reg_ibc.h"

#include "mmp_reg_scaler.h"
#include "mmp_reg_icon.h"
#include "mmpf_scaler.h"
#include "mmpf_h264enc.h"
#if H264_SIMULCAST_EN
#include "mmpf_usbuvch264.h"
#endif

#define GRA_SEM_EN      (1)
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
static MMPF_OS_SEMID  gGraphics_SemID;
static GraphicRotDMACallBackFunc *GRAScaleCall = NULL;
MMP_ULONG glGraphicCropX = 0;
MMP_ULONG glGraphicCropY = 0;
MMP_ULONG glGraphicCropWidth = 640;
MMP_ULONG glGraphicCropHeight = 360;
MMP_ULONG glGraphicYAddr ;
MMP_ULONG glGraphicUAddr ;
MMP_ULONG glGraphicVAddr ;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================



//------------------------------------------------------------------------------
//  Function    : MMPF_Graphics_Init
//  Description :
//  Note        : put in sys_task:main() or XX_Task() (ex: dsc_Task)
//------------------------------------------------------------------------------

void MMPF_Graphics_Init(void)
{
    AITPS_AIC   pAIC = AITC_BASE_AIC;
	AITPS_GRA   pGRA = AITC_BASE_GRA; 

	pGRA->GRA_SCAL_INT_CPU_EN = 0;
	pGRA->GRA_SCAL_INT_CPU_SR = pGRA->GRA_SCAL_INT_CPU_SR ;
	
	RTNA_AIC_Open(pAIC, AIC_SRC_GRA, gra_isr_a, AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_GRA);
    
	gGraphics_SemID = MMPF_OS_CreateSem(1);
	
    if(gGraphics_SemID == 0xFF)
    	RTNA_DBG_Str(0, "XXX m_CaptureDoneSem Failed XXX\r\n");
	
    	
   
}
#include "mmpf_usbpccam.h"
#include "mmp_reg_gpio.h"
#if H264_SIMULCAST_EN
extern  GRA_STREAMING_CFG    gGRAStreamingCfg; 
extern  MMP_UBYTE   gbCurIBCBuf[];
extern  MMP_UBYTE	gbExposureDoneFrame[];
extern	MMP_ULONG   glPreviewBufAddr[MMPF_IBC_PIPE_MAX][4];
extern	MMP_ULONG   glPreviewUBufAddr[MMPF_IBC_PIPE_MAX][4];
extern	MMP_ULONG   glPreviewVBufAddr[MMPF_IBC_PIPE_MAX][4];
extern  MMP_UBYTE   gbPreviewBufferCount[];
#endif
extern H264_FORMAT_TYPE gbCurH264Type;
void MMPF_GRA_ISR(void)
{
    AITPS_GRA   pGRA = AITC_BASE_GRA;
    MMP_USHORT  intsrc;
    
    intsrc = pGRA->GRA_SCAL_INT_CPU_SR & pGRA->GRA_SCAL_INT_CPU_EN;
    pGRA->GRA_SCAL_INT_CPU_SR = intsrc;
    
  	//dbg_printf(3, "MMPF_GRA_ISR...0x%x\r\n", intsrc);

	if (intsrc & GRA_YUV420_SCAL_DONE) {
	    pGRA->GRA_SCAL_INT_CPU_EN &= ~(GRA_YUV420_SCAL_DONE);
	    #if GRA_SEM_EN
	    if (MMPF_OS_ReleaseSem(gGraphics_SemID) != OS_NO_ERR) {
		    RTNA_DBG_Str(0, "gGraphics_SemID OSSemPost1: Fail \r\n");
		    return;
	    }
	    #endif
	    if (GRAScaleCall) (*GRAScaleCall)();
	}

	if (intsrc & GRA_SCAL_DONE) {
		pGRA->GRA_SCAL_INT_CPU_EN &= ~(GRA_SCAL_DONE);
	    if (MMPF_OS_ReleaseSem(gGraphics_SemID) != OS_NO_ERR) {
			RTNA_DBG_Str(0, "gGraphics_SemID OSSemPost1: Fail \r\n");
		    return;
	    }
	    if (GRAScaleCall) (*GRAScaleCall)();
	}
}
#if 0
MMP_ERR MMPF_Graphics_Scale_MSB(MMPF_SCALER_PATH mainpipe,
                                MMP_ULONG SrcAddr, MMP_ULONG DstAddr,
                                MMPF_GRAPHICS_COLORDEPTH src_format, 
                                MMPF_GRAPHICS_COLORDEPTH dst_format, 
                                MMP_SHORT src_w,MMP_SHORT src_h, 
                                MMP_SHORT dst_w,MMP_SHORT dst_h)
{
	AITPS_GRA   pGRA = AITC_BASE_GRA;
	AITPS_SCAL  pSCAL = AITC_BASE_SCAL;
	AITPS_IBC   pIBC = AITC_BASE_IBC;
	AITPS_ICOB  pICOB = AITC_BASE_ICOB;
	MMPF_SCALER_FIT_RANGE fitrange;
	MMPF_SCALER_GRABCONTROL grabctl;
    volatile AITPS_IBCP  pIbcPipeCtl;

    switch (src_format) {
    case MMPF_GRAPHICS_COLORDEPTH_YUV420:
    case MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE:
        break;
    default:
        RTNA_DBG_Str0("Gra wrong src color format\r\n");
        return MMP_GRA_ERR_PARAMETER;
    }
    switch (dst_format) {
    case MMPF_GRAPHICS_COLORDEPTH_8:
    case MMPF_GRAPHICS_COLORDEPTH_YUV420:
    case MMPF_GRAPHICS_COLORDEPTH_YUV422:
    case MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE:
    case MMPF_GRAPHICS_COLORDEPTH_YUY2:
        break;
    default:
        RTNA_DBG_Str0("Gra wrong dst color format\r\n");
        return MMP_GRA_ERR_PARAMETER;
    }

	fitrange.fitmode    = MMPF_SCALER_FITMODE_OUT;
	fitrange.usFitResol = 60;
	fitrange.usInWidth  = src_w;
    fitrange.usInHeight = src_h;
	fitrange.usOutWidth = dst_w;
	fitrange.usOutHeight = dst_h;

    MMPF_Scaler_SetOutputFormat(mainpipe, MMPF_SCALER_COLOR_YUV422);
    if (mainpipe == MMPF_SCALER_PATH_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);

        //ctl_reg = pSCAL->SCAL_PATH_CTL;
        //ctl_reg &= ~(SCAL_LPF_PATH_SEL_MASK | SCAL_SCAL0_PATH_SEL_MASK);
        //ctl_reg |= (SCAL_GRA_2_LPF | SCAL_LPF_2_SCAL0);
        //pSCAL->SCAL_PATH_CTL = ctl_reg;
        pSCAL->SCAL_PATH_CTL |= SCAL_GRA_2_SCAL0;

        pSCAL->SCAL_SCAL_0_CTL |= SCAL_SCAL_PATH_EN;
    }
    else if (mainpipe == MMPF_SCALER_PATH_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);

        //ctl_reg = pSCAL->SCAL_PATH_CTL;
        //ctl_reg &= ~(SCAL_LPF1_PATH_SEL_MASK | SCAL_SCAL1_PATH_SEL_MASK);
        //ctl_reg |= (SCAL_GRA_2_LPF1 | SCAL_LPF_2_SCAL1);
        //SCAL->SCAL_PATH_CTL = ctl_reg;
        pSCAL->SCAL_PATH_CTL |= SCAL_GRA_2_SCAL1;

        pSCAL->SCAL_SCAL_1_CTL |= SCAL_SCAL_PATH_EN;
    }
    else {
        pIbcPipeCtl = &(pIBC->IBCP_2);

        pSCAL->SCAL_PATH_2_CTL |= SCAL_GRA_2_SCAL2;

        pSCAL->SCAL_SCAL_2_CTL |= SCAL_SCAL_PATH_EN;
    }

	MMPF_Scaler_SetEngine(MMP_FALSE, mainpipe, &fitrange, &grabctl);
	MMPF_Scaler_SetLPF(mainpipe, &fitrange, &grabctl);   

    //Set dst buffer
    pIbcPipeCtl->IBC_ADDR_Y_ST = DstAddr;
	if ((dst_format != MMPF_GRAPHICS_COLORDEPTH_YUV422) && (dst_format != MMPF_GRAPHICS_COLORDEPTH_YUY2)) {
        pIbcPipeCtl->IBC_ADDR_U_ST = pIbcPipeCtl->IBC_ADDR_Y_ST + dst_w*dst_h;
        pIbcPipeCtl->IBC_ADDR_V_ST = pIbcPipeCtl->IBC_ADDR_U_ST + dst_w*dst_h/4;
	}

    if (mainpipe== MMPF_SCALER_PATH_0) {
        pICOB->ICO_DLINE_CFG[0] = ICO_DLINE_SRC_SEL(0);
        pIBC->IBC_P0_INT_CPU_SR = IBC_INT_FRM_END;
	}
    else if (mainpipe== MMPF_SCALER_PATH_1) {
        pICOB->ICO_DLINE_CFG[1] = ICO_DLINE_SRC_SEL(1);
        pIBC->IBC_P1_INT_CPU_SR = IBC_INT_FRM_END;
	}
	else {
        pICOB->ICO_DLINE_CFG[2] = ICO_DLINE_SRC_SEL(2);
        pIBC->IBC_P2_INT_CPU_SR = IBC_INT_FRM_END;
	}
    #if 0 //dbg
    pSCAL->SCAL_HOST_INT_SR = 0xFF;
    pIBC->IBC_P0_INT_HOST_SR = 0xFF;
    pIBC->IBC_P1_INT_HOST_SR = 0xFF;
    #endif

    pIbcPipeCtl->IBC_SRC_SEL &= ~(IBC_SRC_SEL_MASK|IBC_SRC_YUV420|IBC_420_STORE_CBR|IBC_NV12_EN);
    pIbcPipeCtl->IBC_SRC_SEL |= IBC_SRC_SEL_ICO(mainpipe);
    switch (dst_format) {
 	case MMPF_GRAPHICS_COLORDEPTH_8:
        pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420);
        break;
 	case MMPF_GRAPHICS_COLORDEPTH_YUV420:
        pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420 | IBC_420_STORE_CBR);
        break;
 	case MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE:
        pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420 | IBC_420_STORE_CBR | IBC_NV12_EN);
        break;
	case MMPF_GRAPHICS_COLORDEPTH_YUV422: //for MJPEG
		pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_UYVY;
        break;
    case MMPF_GRAPHICS_COLORDEPTH_YUY2:
		pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_YUYV;
        break;
	default:
		DBG_S(3, "format error\r\n");
        break;
    }
	pIbcPipeCtl->IBC_BUF_CFG |= (IBC_STORE_EN | IBC_ICON_PAUSE_EN |IBC_STORE_PIX_CONT);

	if (mainpipe == MMPF_SCALER_PATH_0) {
        pIBC->IBC_P0_INT_CPU_SR = IBC_INT_FRM_RDY;
        pIBC->IBC_P0_INT_CPU_EN |= IBC_INT_FRM_RDY;
	}
	else if (mainpipe == MMPF_SCALER_PATH_1) {
        pIBC->IBC_P1_INT_CPU_SR = IBC_INT_FRM_RDY;
        pIBC->IBC_P1_INT_CPU_EN |= IBC_INT_FRM_RDY;
	}
	else {
        pIBC->IBC_P2_INT_CPU_SR = IBC_INT_FRM_RDY;
        pIBC->IBC_P2_INT_CPU_EN |= IBC_INT_FRM_RDY;
	}

    //Set Graphics
	if (pGRA->GRA_SCAL_EN & (GRA_YUV420_SCAL_ST)) {
        DBG_S(3, "GRAPHIC BUSY MSB\r\n");
        return 1;
	}

	// for 2x720p @30fps performance
    if ((src_w >= dst_w) && (src_h>=dst_h))
	{
        pGRA->GRA_SCAL_FLOW_CTL = GRA_SCAL_BUSY_CTL;
        pGRA->GRA_SCAL_PIXL_DLY = 1;
		//pGRA->GRA_SCAL_LINE_DLY = 2*src_w + 64;
    }
    else { // scale up case
        MMP_USHORT  usScaleFac;
        AIT_REG_B   *OprNh, *OprMh;

        if (mainpipe == MMPF_SCALER_PATH_0) {
            OprNh = &(pSCAL->SCAL_SCAL_0_H_N);
            OprMh = &(pSCAL->SCAL_SCAL_0_H_M);
        }
        else if (mainpipe == MMPF_SCALER_PATH_1) {
            OprNh = &(pSCAL->SCAL_SCAL_1_H_N);
            OprMh = &(pSCAL->SCAL_SCAL_1_H_M);
        }
        else {
            OprNh = &(pSCAL->SCAL_SCAL_2_H_N);
            OprMh = &(pSCAL->SCAL_SCAL_2_H_M);
        }

        usScaleFac = (*OprNh + *OprMh - 1)/(*OprMh);

        pGRA->GRA_SCAL_FLOW_CTL = GRA_SCAL_DLY_CTL;
        pGRA->GRA_SCAL_PIXL_DLY = ((usScaleFac-1) << 1);
		pGRA->GRA_SCAL_LINE_DLY = (usScaleFac << 1)*dst_w + 64;
    }

	//Set src buffer
	pGRA->GRA_SCAL_ADDR_Y_ST = SrcAddr;
    pGRA->GRA_SCAL_ADDR_U_ST = pGRA->GRA_SCAL_ADDR_Y_ST + src_w*src_h;
	if (src_format == MMPF_GRAPHICS_COLORDEPTH_YUV420) {
    	pGRA->GRA_SCAL_ADDR_V_ST = pGRA->GRA_SCAL_ADDR_U_ST + src_w*src_h/4;
	}
	else {
	    pGRA->GRA_SCAL_ADDR_V_ST = 0;
	}
    pGRA->GRA_SCAL_W = src_w;
	pGRA->GRA_SCAL_H = src_h;  

   // GRAScaleCall = (GraphicRotDMACallBackFunc*)MMPF_Graphics_Scale_MSB;

	//pGRA->GRA_SCAL_INT_CPU_SR = GRA_YUV420_SCAL_DONE;
	//pGRA->GRA_SCAL_INT_CPU_EN = GRA_YUV420_SCAL_DONE;

    switch (src_format) {
    case MMPF_GRAPHICS_COLORDEPTH_YUV420:
        pGRA->GRA_SCAL_EN = GRA_YUV420_SCAL_ST;
        break;
    case MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE:
        pGRA->GRA_SCAL_EN = (GRA_YUV420_SCAL_ST | GRA_YUV420_INTERLEAVE);
        break;
    }

	return MMP_ERR_NONE;
}
#endif

MMP_ERR MMPF_Graphics_SetScaleAttribute(MMPF_GRAPHICS_BUFATTRIBUTE *bufattribute,
                        MMP_ULONG ulScaleBufAddr, MMPF_GRAPHICS_COLORDEPTH incolormode,                       
						MMPF_GRAPHICS_RECT *rect, MMP_USHORT usUpscale, 
						MMPF_GRAPHICS_SCAL_SRC srcsel)
{
	AITPS_GRA   pGRA = AITC_BASE_GRA;
	MMP_ULONG	longtmp;
	MMP_UBYTE   ubRet = 0;
	MMP_USHORT  usCount ;
	
#if GRA_SEM_EN//(SUPPORT_GRA_ZOOM == 0)	    
    ubRet = MMPF_OS_AcquireSem(gGraphics_SemID, GRAPHICS_SEM_TIMEOUT);
    if (ubRet == OS_ERR_PEND_ISR) {
        MMPF_OS_AcceptSem(gGraphics_SemID, &usCount);
        if (usCount == 0) {
            RTNA_DBG_Str(0, "gGraphics_SemID OSSemAccept: Fail0\r\n");
            return MMP_GRA_ERR_BUSY;
        }
    }
    else if (ubRet != OS_NO_ERR) {
        RTNA_DBG_Str(0, "gGraphics_SemID OSSemPend: Fail\r\n");
        return MMP_GRA_ERR_BUSY;
	}
#endif
   	if ( (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420) ||
   	     (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE ) )
   	{
   	
   	#if 1 // TBD
	    longtmp = (bufattribute->ulBaseAddr)
    			+ (bufattribute->usLineOffset * rect->usTop) + rect->usLeft;
        pGRA->GRA_SCAL_ADDR_Y_ST = longtmp;
   	
   	    if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE) {
       		longtmp = (bufattribute->ulBaseUAddr)
        			+ (bufattribute->usLineOffset * (rect->usTop >> 1)) + ((rect->usLeft >> 1) << 1);
	        pGRA->GRA_SCAL_ADDR_U_ST = longtmp;
   	    }
   	    else {
       		longtmp = (bufattribute->ulBaseUAddr)
        			+ ((bufattribute->usLineOffset >> 1) * (rect->usTop >> 1)) + (rect->usLeft >> 1);
	        pGRA->GRA_SCAL_ADDR_U_ST = longtmp;

	       	longtmp = (bufattribute->ulBaseVAddr)
    	    		+ ((bufattribute->usLineOffset >> 1) * (rect->usTop >> 1)) + (rect->usLeft >> 1);
        	pGRA->GRA_SCAL_ADDR_V_ST = longtmp;
    	}

        pGRA->GRA_SCAL_ADDR_Y_OFST = bufattribute->usLineOffset - rect->usWidth;
        if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE) {
   	        pGRA->GRA_SCAL_ADDR_U_OFST = bufattribute->usLineOffset - rect->usWidth;
        }
        else {
            pGRA->GRA_SCAL_ADDR_U_OFST = ((bufattribute->usLineOffset - rect->usWidth) >> 1);
            pGRA->GRA_SCAL_ADDR_V_OFST = pGRA->GRA_SCAL_ADDR_U_OFST;
        }
   	
   	#else
        // setting for graphic engine address. Y address
	    longtmp = (bufattribute->ulBaseAddr)
    			+ (bufattribute->usLineOffset * rect->usTop) + rect->usLeft;
        pGRA->GRA_SCAL_ADDR_Y_ST = longtmp;

        // setting for graphic engine address. U address
   		longtmp = (bufattribute->ulBaseUAddr)
    			+ ((bufattribute->usLineOffset >> 1) * (rect->usTop >> 1)) + (rect->usLeft >> 1);
        pGRA->GRA_SCAL_ADDR_U_ST = longtmp;

    	// setting for graphic engine address. V address
       	longtmp = (bufattribute->ulBaseVAddr)
	    		+ ((bufattribute->usLineOffset >> 1) * (rect->usTop >> 1)) + (rect->usLeft >> 1);
	    // setting for graphic engine
    	pGRA->GRA_SCAL_ADDR_V_ST = longtmp;
    #endif
    
   	}
    else if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422) {
	    longtmp = (bufattribute->ulBaseAddr)
    	    	+ ((bufattribute->usLineOffset) * (rect->usTop)) 
	    	    + ((rect->usLeft) * (2));
	    // setting for graphic engine
    	pGRA->GRA_SCAL_ADDR_Y_ST = longtmp;
        pGRA->GRA_SCAL_ADDR_U_ST = (bufattribute->usLineOffset);

   	}
    else {
        longtmp = (bufattribute->ulBaseAddr)
		    	+ ((bufattribute->usLineOffset) * (rect->usTop)) 
    			+ ((rect->usLeft) * (MMP_USHORT)(bufattribute->colordepth));
    			
        // setting for graphic engine
    	pGRA->GRA_SCAL_ADDR_Y_ST = longtmp;
        pGRA->GRA_SCAL_ADDR_U_ST = (bufattribute->usLineOffset);
    }

    pGRA->GRA_SCAL_W = (rect->usWidth);
    pGRA->GRA_SCAL_H = (rect->usHeight);

	pGRA->GRA_SCAL_UP_FACT = GRA_SCALUP(usUpscale);

	{
		if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_16) {
			pGRA->GRA_SCAL_FMT = GRA_SCAL_MEM_RGB565;
		}		
		else if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_24) {
			pGRA->GRA_SCAL_FMT = GRA_SCAL_MEM_RGB888;
		}		
		else if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422) {
			pGRA->GRA_SCAL_FMT = GRA_SCAL_MEM_YUV422;
		}
		else if ( (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420) ||
		          (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE ) )
		{
			pGRA->GRA_SCAL_FMT = GRA_SCAL_MEM_RGB565;
		}
	}


#if GRA_SEM_EN //(SUPPORT_GRA_ZOOM == 0)
	MMPF_OS_ReleaseSem(gGraphics_SemID);
#endif
    return MMP_ERR_NONE;
}

#if 1// TBD new
MMP_ERR MMPF_Graphics_Scale_P0(MMPF_GRAPHICS_BUFATTRIBUTE src,
                            MMPF_GRAPHICS_BUFATTRIBUTE dst,
                            GraphicRotDMACallBackFunc *GRACallBack,
                            MMP_UBYTE  ubPixelDelay,
                            MMP_USHORT usWaitMs)
{
    AITPS_GRA   pGRA = AITC_BASE_GRA;
    AITPS_IBC   pIBC = AITC_BASE_IBC;
    volatile AITPS_IBCP  pIbcPipeCtl;
    MMP_UBYTE   bIBCPipeNum = 0;
    
    MMP_USHORT  usCount = 0;
    MMP_UBYTE   ubRet = 0;
#if GRA_SEM_EN//(SUPPORT_GRA_ZOOM == 0)	
    ubRet = MMPF_OS_AcquireSem(gGraphics_SemID, usWaitMs);
    
    if(ubRet == 1) { // timeout error
        RTNA_DBG_Str(0, "gGraphics_SemID OSSemPend: Fail\r\n");
        return MMP_GRA_ERR_BUSY;
    } else if (ubRet == 2) { // aquire in ISR
        MMPF_OS_AcceptSem(gGraphics_SemID, &usCount);
        if(usCount == 0){
            RTNA_DBG_Str(0, "gGraphics_SemID OSSemAccept: Fail\r\n");
            return MMP_GRA_ERR_BUSY;
        }
    } 
#endif
       
	pGRA->GRA_SCAL_FLOW_CTL = GRA_SCAL_BUSY_CTL;
	pGRA->GRA_SCAL_PIXL_DLY = ubPixelDelay;

    if (bIBCPipeNum == 0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (bIBCPipeNum == 1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    
    if ((src.colordepth == MMPF_GRAPHICS_COLORDEPTH_16) ||
        (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_24) ||
        (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422)) {
    	if (pGRA->GRA_SCAL_EN & GRA_SCAL_ST) {
    	    DBG_S(3, "GRAPHIC SCAL BUSY\r\n");
    		return MMP_ERR_NONE;
        }
        pGRA->GRA_SCAL_INT_CPU_SR = GRA_SCAL_DONE;
        pGRA->GRA_SCAL_INT_CPU_EN = GRA_SCAL_DONE;
        pGRA->GRA_SCAL_ADDR_Y_ST = src.ulBaseAddr;
        GRAScaleCall = GRACallBack;
        pGRA->GRA_SCAL_EN = GRA_SCAL_ST;
    }
    else if ( (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420) ||
              (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE ) )
    {
        if (pGRA->GRA_SCAL_EN & GRA_YUV420_SCAL_ST) {
    	    DBG_S(3, "GRAPHIC SCAL420 BUSY\r\n");
    	    return MMP_ERR_NONE;
        }
        pGRA->GRA_SCAL_ADDR_Y_ST = src.ulBaseAddr;
        pGRA->GRA_SCAL_ADDR_U_ST = src.ulBaseUAddr;
        pGRA->GRA_SCAL_ADDR_V_ST = src.ulBaseVAddr;
        
        if (dst.ulBaseAddr) {
            pIbcPipeCtl->IBC_ADDR_Y_ST = dst.ulBaseAddr;
            pIbcPipeCtl->IBC_ADDR_U_ST = dst.ulBaseUAddr;
            pIbcPipeCtl->IBC_ADDR_V_ST = dst.ulBaseVAddr;
        }
        GRAScaleCall = GRACallBack;
        pGRA->GRA_SCAL_INT_CPU_SR = GRA_YUV420_SCAL_DONE;
        pGRA->GRA_SCAL_INT_CPU_EN = GRA_YUV420_SCAL_DONE;
        
        if (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420)
            pGRA->GRA_SCAL_EN = GRA_YUV420_SCAL_ST;
        else if (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE)
            pGRA->GRA_SCAL_EN = GRA_YUV420_SCAL_ST | GRA_YUV420_INTERLEAVE;
    }
    else {
        RTNA_DBG_Str(0, "Unsupported color format\r\n");
    }
    return MMP_ERR_NONE;
}
#endif



MMP_ERR MMPF_Graphics_Scale(MMPF_GRAPHICS_BUFATTRIBUTE src,
                            MMPF_GRAPHICS_BUFATTRIBUTE dst,
                            GraphicRotDMACallBackFunc *GRACallBack,
                            MMP_UBYTE  ubPixelDelay,
                            MMP_USHORT usWaitMs)
{
    AITPS_GRA   pGRA = AITC_BASE_GRA;
    AITPS_IBC   pIBC = AITC_BASE_IBC;
    volatile AITPS_IBCP  pIbcPipeCtl;
    MMP_UBYTE   bIBCPipeNum = PIPE_1;
    
    MMP_USHORT  usCount = 0;
    MMP_UBYTE   ubRet = 0;
    //dbg_printf(3,"G:%d\r\n",usb_vc_cur_usof());        
#if GRA_SEM_EN
    ubRet = MMPF_OS_AcquireSem(gGraphics_SemID, usWaitMs);
    
    if(ubRet == 1) { // timeout error
        RTNA_DBG_Str(0, "gGraphics_SemID OSSemPend: Fail\r\n");
        return MMP_GRA_ERR_CMDTIMEOUT;
    } else if (ubRet == 2) { // aquire in ISR
        MMPF_OS_AcceptSem(gGraphics_SemID, &usCount);
        if(usCount == 0){
            RTNA_DBG_Str(0, "GRA Busy\r\n");
            return MMP_GRA_ERR_NOT_IMPLEMENT;
        }
    }
#endif    
	pGRA->GRA_SCAL_FLOW_CTL = GRA_SCAL_BUSY_CTL;
	pGRA->GRA_SCAL_PIXL_DLY = ubPixelDelay;

    if (bIBCPipeNum == 0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (bIBCPipeNum == 1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    
    if ((src.colordepth == MMPF_GRAPHICS_COLORDEPTH_16) ||
        (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_24) ||
        (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422)) {
    	if (pGRA->GRA_SCAL_EN & GRA_SCAL_ST) {
    	    DBG_S(3, "GRAPHIC SCAL BUSY\r\n");
    		return MMP_ERR_NONE;
        }
        pGRA->GRA_SCAL_INT_CPU_SR = GRA_SCAL_DONE;
        pGRA->GRA_SCAL_INT_CPU_EN = GRA_SCAL_DONE;
        pGRA->GRA_SCAL_ADDR_Y_ST = src.ulBaseAddr;
        
        GRAScaleCall = GRACallBack;
        pGRA->GRA_SCAL_EN = GRA_SCAL_ST;
    }
    else if ( (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420) ||
              (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE ) )
    {
        if (pGRA->GRA_SCAL_EN & GRA_YUV420_SCAL_ST) {
    	    DBG_S(3, "GRAPHIC SCAL420 BUSY\r\n");
    	    return MMP_ERR_NONE;
        }
 
#if SUPPORT_GRA_ZOOM       
        //Set src buffer
		//if(gbCurH264Type == FRAMEBASE_H264_YUY2 || gbCurH264Type == INVALID_H264){
		if(1) {
			pGRA->GRA_SCAL_ADDR_Y_ST = src.ulBaseAddr + (glGraphicCropY * src.usWidth) + glGraphicCropX;
		//	pGRA->GRA_SCAL_ADDR_U_ST = src.ulBaseAddr + 1920*1088 + ((glGraphicCropY * 1920) + (glGraphicCropX >> 2));
			pGRA->GRA_SCAL_ADDR_U_ST = src.ulBaseAddr + src.usWidth * src.usHeight + ((glGraphicCropY >> 1) * src.usWidth) + ((glGraphicCropX >> 1) << 1);	
		// 	pGRA->GRA_SCAL_ADDR_V_ST = src.ulBaseAddr + 1920*1080*5/4 + ((glGraphicCropY * 1920 + glGraphicCropX) >> 1);
		
	    	pGRA->GRA_SCAL_W = glGraphicCropWidth;//src.usWidth;
	    	if ((glGraphicCropY != 0) || (glGraphicCropHeight != 0)) {
				pGRA->GRA_SCAL_H = glGraphicCropHeight;
			}
			else {
				pGRA->GRA_SCAL_H = src.usHeight;
			}
			pGRA->GRA_SCAL_ADDR_Y_OFST = src.usWidth - glGraphicCropWidth;
			pGRA->GRA_SCAL_ADDR_U_OFST = src.usWidth - glGraphicCropWidth;
			pGRA->GRA_SCAL_ADDR_V_OFST = src.usWidth - glGraphicCropWidth;
			//dbg_printf(3, "Crop: (X, Y)=(%d, %d)\r\n", glGraphicCropX, glGraphicCropY);	
		}
#else        
        pGRA->GRA_SCAL_ADDR_Y_ST = src.ulBaseAddr;
        pGRA->GRA_SCAL_ADDR_U_ST = src.ulBaseUAddr;
        pGRA->GRA_SCAL_ADDR_V_ST = src.ulBaseVAddr;
#endif
        if (dst.ulBaseAddr) {
            pIbcPipeCtl->IBC_ADDR_Y_ST = dst.ulBaseAddr;
            pIbcPipeCtl->IBC_ADDR_U_ST = dst.ulBaseUAddr;
            pIbcPipeCtl->IBC_ADDR_V_ST = dst.ulBaseVAddr;
        }
        
        GRAScaleCall = GRACallBack;
        pGRA->GRA_SCAL_INT_CPU_SR = GRA_YUV420_SCAL_DONE;
        pGRA->GRA_SCAL_INT_CPU_EN = GRA_YUV420_SCAL_DONE;
        
        if (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420)
            pGRA->GRA_SCAL_EN = GRA_YUV420_SCAL_ST;
        else if (src.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE)
            pGRA->GRA_SCAL_EN = GRA_YUV420_SCAL_ST | GRA_YUV420_INTERLEAVE;
    }
    else {
        RTNA_DBG_Str(0, "Unsupported color format\r\n");
    }
     
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Graphics_SetCrop(MMP_ULONG ulStartX, MMP_ULONG ulStartY,
                              MMP_ULONG ulCropWidth, MMP_ULONG ulCropHeight,
                              MMP_ULONG ulYAddr,MMP_ULONG ulUAddr,MMP_ULONG ulVAddr)
{
	//AITPS_GRA   pGRA = AITC_BASE_GRA;
	glGraphicCropX = ulStartX;
	glGraphicCropY = ulStartY;
	glGraphicCropWidth = ulCropWidth;
	glGraphicCropHeight = ulCropHeight;
	glGraphicYAddr = ulYAddr;
	glGraphicUAddr = ulUAddr;
	glGraphicVAddr = ulVAddr;
	
	//dbg_printf(0,"gra(%x,%x,%x)\r\n",glGraphicYAddr,glGraphicUAddr,glGraphicVAddr);
	return MMP_ERR_NONE;
}                              

#if (CHIP == P_V2)
MMP_ULONG MMPF_Graphic_SetKeyColor(MMP_BOOL bSetFgColor, MMP_ULONG ulColor)
{
    AITPS_GRA   pGRA = AITC_BASE_GRA;
    MMP_ULONG   oldcolor;

    if (bSetFgColor) {
        oldcolor = pGRA->GRA_BLT_FG;
        pGRA->GRA_BLT_FG = ulColor;
    }
    else {
        oldcolor = pGRA->GRA_BLT_BG;
        pGRA->GRA_BLT_BG = ulColor;
    }

    return oldcolor;
}

MMP_ERR MMPF_Graphic_DrawRectToBuf(MMPF_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPF_GRAPHICS_RECT *rect, MMP_BYTE bThickness)
{
    AITPS_GRA   pGRA = AITC_BASE_GRA;
    MMP_ULONG   longtmp;
    MMP_USHORT  cut_width, cut_height;

    if (rect->usLeft >= bufattribute->usWidth || 
        rect->usTop >= bufattribute->usHeight ||
        !rect->usWidth || !rect->usHeight ||
        rect->usWidth <= (2 * bThickness) ||
        rect->usHeight <= (2 * bThickness)) {
        RTNA_DBG_Str(3, "Invalid parameters for graphic drawing\r\n");
        return MMP_GRA_ERR_PARAMETER;
    }

	if ((rect->usWidth + rect->usLeft) > bufattribute->usWidth)
		cut_width = bufattribute->usWidth - rect->usLeft;
    else
        cut_width = rect->usWidth;

	if ((rect->usHeight + rect->usTop) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - rect->usTop;
    else
        cut_height = rect->usHeight;

    if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422)
		pGRA->GRA_BLT_FMT = 1 << 2;
	else
		pGRA->GRA_BLT_FMT = (bufattribute->colordepth - 1) << 2;

    pGRA->GRA_BLT_ROP = GRA_ROP_SRCCOPY & GRA_ROP_MASK;
    pGRA->GRA_BLT_ROP_CTL = GRA_OP_DO_ROP | GRA_SOLID_FILL | GRA_LEFT_TOP_RIGHT;
    pGRA->GRA_BLT_DST_PITCH = bufattribute->usLineOffset;

    // Up Line
    pGRA->GRA_BLT_W = cut_width;
    pGRA->GRA_BLT_H = bThickness;

    if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422)
		longtmp = (bufattribute->ulBaseAddr)
				+ ((bufattribute->usLineOffset) * (rect->usTop))
				+ ((rect->usLeft) * 2);
	else
		longtmp = (bufattribute->ulBaseAddr)
				+ ((bufattribute->usLineOffset) * (rect->usTop))
				+ ((rect->usLeft) * bufattribute->colordepth);
    pGRA->GRA_BLT_DST_ADDR = longtmp;

    pGRA->GRA_BLT_EN = GRA_BLT_ST;
    while(pGRA->GRA_BLT_EN & GRA_BLT_ST);

    // Down Line
    pGRA->GRA_BLT_W = cut_width;
    pGRA->GRA_BLT_H = bThickness;

    if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422)
    	longtmp = (bufattribute->ulBaseAddr)
    			+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - bThickness))
    			+ ((rect->usLeft) * 2);
    else
    	longtmp = (bufattribute->ulBaseAddr)
    			+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - bThickness))
    			+ ((rect->usLeft) * bufattribute->colordepth);
    pGRA->GRA_BLT_DST_ADDR = longtmp;

    pGRA->GRA_BLT_EN = GRA_BLT_ST;
    while(pGRA->GRA_BLT_EN & GRA_BLT_ST);

    // Left Line
    pGRA->GRA_BLT_W = bThickness;
    pGRA->GRA_BLT_H = cut_height - bThickness * 2;

    if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422)
	    longtmp = (bufattribute->ulBaseAddr)
				+ ((bufattribute->usLineOffset) * (rect->usTop + bThickness))
				+ ((rect->usLeft) * 2);
	else
	    longtmp = (bufattribute->ulBaseAddr)
				+ ((bufattribute->usLineOffset) * (rect->usTop + bThickness))
				+ ((rect->usLeft) * bufattribute->colordepth);
    pGRA->GRA_BLT_DST_ADDR = longtmp;

    pGRA->GRA_BLT_EN = GRA_BLT_ST;
    while(pGRA->GRA_BLT_EN & GRA_BLT_ST);

    // Right Line
    pGRA->GRA_BLT_W = bThickness;
    pGRA->GRA_BLT_H = cut_height - bThickness * 2;

    if (bufattribute->colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422)
        longtmp = (bufattribute->ulBaseAddr)
    			+ ((bufattribute->usLineOffset) * (rect->usTop + bThickness))
    			+ ((rect->usLeft + cut_width - bThickness) * 2);
    else
        longtmp = (bufattribute->ulBaseAddr)
    			+ ((bufattribute->usLineOffset) * (rect->usTop + bThickness))
    			+ ((rect->usLeft + cut_width - bThickness) * bufattribute->colordepth);
    pGRA->GRA_BLT_DST_ADDR = longtmp;

    pGRA->GRA_BLT_EN = GRA_BLT_ST;
    while(pGRA->GRA_BLT_EN & GRA_BLT_ST);

    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Graphics_CopyImageBuftoBuf(MMP_ULONG SrcAddr,MMP_ULONG dstAddr,  
						MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty,
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode)
{
    MMP_ULONG   longtmp;
    MMP_USHORT  cut_width, cut_height;
    MMP_USHORT  raster_dir;
    MMP_USHORT  colordepth;
	AITPS_GRA   pGRA = AITC_BASE_GRA;
	MMPF_GRAPHICS_BUFATTRIBUTE srcbufattribute;
    MMPF_GRAPHICS_BUFATTRIBUTE dstbufattribute;
	MMPF_GRAPHICS_RECT srcrect;
	srcrect.usHeight = 720;
	srcrect.usWidth= 1280;
	srcrect.usLeft = usSrcStartx;
	srcrect.usTop = usSrcStarty;

	dstbufattribute.colordepth = MMPF_GRAPHICS_COLORDEPTH_8;
	dstbufattribute.usHeight = 480;
	dstbufattribute.usWidth = 640;
	dstbufattribute.ulBaseAddr = dstAddr;
	dstbufattribute.ulBaseUAddr= dstbufattribute.ulBaseAddr + 	dstbufattribute.usWidth*dstbufattribute.usHeight;
	dstbufattribute.ulBaseVAddr= dstbufattribute.ulBaseUAddr + 	dstbufattribute.usWidth*dstbufattribute.usHeight/4;
	dstbufattribute.usLineOffset = dstbufattribute.usWidth;


	srcbufattribute.colordepth = MMPF_GRAPHICS_COLORDEPTH_8;
	srcbufattribute.ulBaseAddr = SrcAddr;
	srcbufattribute.ulBaseUAddr= srcbufattribute.ulBaseAddr + srcrect.usHeight*srcrect.usWidth;
	srcbufattribute.ulBaseVAddr= srcbufattribute.ulBaseUAddr + srcrect.usHeight*srcrect.usWidth/4;
	srcbufattribute.usHeight = srcrect.usHeight;
	srcbufattribute.usWidth = srcrect.usWidth;
	srcbufattribute.usLineOffset = srcbufattribute.usWidth;
	
	if (srcrect.usLeft >= srcbufattribute.usWidth || 
			srcrect.usTop >= srcbufattribute.usHeight ||
			usDstStartx >= dstbufattribute.usWidth || 
			usDstStarty >= dstbufattribute.usHeight ||
			!srcrect.usWidth || !srcrect.usHeight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if (srcbufattribute.colordepth != dstbufattribute.colordepth) {
        return MMP_GRA_ERR_PARAMETER;
	}			

    if(dstbufattribute.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV420) {
        return MMP_GRA_ERR_PARAMETER;
	}	

    if(dstbufattribute.colordepth == MMPF_GRAPHICS_COLORDEPTH_YUV422) {
        colordepth = 2;
	}	
    else {
        colordepth = dstbufattribute.colordepth;
    }     

	if ((srcrect.usWidth + srcrect.usLeft) > srcbufattribute.usWidth)
		cut_width = srcbufattribute.usWidth - srcrect.usLeft;
	else	
		cut_width = srcrect.usWidth;

	if ((srcrect.usHeight + srcrect.usTop) > srcbufattribute.usHeight)
		cut_height = srcbufattribute.usHeight - srcrect.usTop;
	else	
		cut_height = srcrect.usHeight;

	if ((cut_width + usDstStartx) > dstbufattribute.usWidth)
		cut_width = dstbufattribute.usWidth - usDstStartx;

	if ((cut_height + usDstStarty) > dstbufattribute.usHeight)
		cut_height = dstbufattribute.usHeight - usDstStarty;

    raster_dir = GRA_LEFT_TOP_RIGHT;

	if (srcbufattribute.ulBaseAddr == dstbufattribute.ulBaseAddr) {
		if (srcrect.usLeft == usDstStartx) {
			if (srcrect.usTop > usDstStarty) {
				raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
			}
			else {	// include (srcx == tgtx) and (srcy == tgty)
				raster_dir = GRA_LEFT_BOT_RIGHT;	// checked
			}	
		}	
		else if (srcrect.usTop == usDstStarty) {
			if (srcrect.usLeft > usDstStartx) {
				raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
			}	
			else {		
				raster_dir = GRA_RIGHT_BOT_LEFT;	// checked
			}
		}
		// Detect Overlap	
		else if (srcrect.usLeft > usDstStartx) {
			if ((usDstStartx + srcrect.usWidth) > srcrect.usLeft) {
				//(S:down-right d:upper-left)	
				if (srcrect.usTop > usDstStarty) {
					raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
				}
				//(S:upper-right d:down-left)
				else if (srcrect.usTop < usDstStarty) {
					raster_dir = GRA_LEFT_BOT_RIGHT;	// checked
				}		
			}	
		}
		else if (srcrect.usLeft < usDstStartx) {
			if ((srcrect.usLeft + srcrect.usWidth) > usDstStartx) {
				//(S:down-left d:upper-right)
				if (srcrect.usTop > usDstStarty) {
					raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
				}
				//(S:upper-left d:down-right)
				else if (srcrect.usTop < usDstStarty) {	
					raster_dir = GRA_LEFT_BOT_RIGHT;	// checked
				}
			}	
		}	
	}


	switch (raster_dir) {
		case GRA_LEFT_TOP_RIGHT:
			pGRA->GRA_BLT_ROP_CTL =  GRA_LEFT_TOP_RIGHT;

			// Deal Source Buffer
			longtmp = (srcbufattribute.ulBaseAddr)
					+ ((srcbufattribute.usLineOffset) * (srcrect.usTop)) 
					+ ((srcrect.usLeft) * colordepth);

            pGRA->GRA_BLT_SRC_ADDR= longtmp;
 
        // Deal Destination Buffer
			longtmp = (dstbufattribute.ulBaseAddr)
					+ ((dstbufattribute.usLineOffset) * (usDstStarty))
					+ ((usDstStartx) * colordepth);

        	pGRA->GRA_BLT_DST_ADDR = longtmp;
        break;

    case GRA_RIGHT_BOT_LEFT:
			pGRA->GRA_BLT_ROP_CTL =  GRA_RIGHT_BOT_LEFT;

        // Deal Source Buffer
			longtmp = (srcbufattribute.ulBaseAddr)
				+ ((srcbufattribute.usLineOffset) * (srcrect.usTop + cut_height - 1)) 
				+ ((srcrect.usLeft + cut_width - 1) * colordepth);

        pGRA->GRA_BLT_SRC_ADDR = longtmp;

        // Deal Destination Buffer
			longtmp = (dstbufattribute.ulBaseAddr)
				+ ((dstbufattribute.usLineOffset) * (usDstStarty + cut_height - 1))
				+ ((usDstStartx + cut_width - 1) * colordepth);

        pGRA->GRA_BLT_DST_ADDR = longtmp;
        break;

    case GRA_LEFT_BOT_RIGHT:
			pGRA->GRA_BLT_ROP_CTL = GRA_LEFT_BOT_RIGHT;

        // Deal Source Buffer
			longtmp = (srcbufattribute.ulBaseAddr)
				+ ((srcbufattribute.usLineOffset) * (srcrect.usTop + cut_height - 1)) 
				+ ((srcrect.usLeft) * colordepth);

        pGRA->GRA_BLT_SRC_ADDR= longtmp;

        // Deal Destination Buffer
			longtmp = (dstbufattribute.ulBaseAddr)
				+ ((dstbufattribute.usLineOffset) * (usDstStarty + cut_height - 1))
				+ ((usDstStartx) * colordepth);

        pGRA->GRA_BLT_DST_ADDR= longtmp;
        break;

    case GRA_RIGHT_TOP_LEFT:
			pGRA->GRA_BLT_ROP_CTL= GRA_RIGHT_TOP_LEFT;

        // Deal Source Buffer
			longtmp = (srcbufattribute.ulBaseAddr)
				+ ((srcbufattribute.usLineOffset) * (srcrect.usTop)) 
				+ ((srcrect.usLeft + cut_width - 1) * colordepth);

        pGRA->GRA_BLT_SRC_ADDR= longtmp;

        // Deal Destination Buffer
			longtmp = (dstbufattribute.ulBaseAddr)
				+ ((dstbufattribute.usLineOffset) * (usDstStarty - 1))
				+ ((usDstStartx + cut_width - 1) * colordepth);

        pGRA->GRA_BLT_DST_ADDR= longtmp;
        break;
    }
	//RTNA_DBG_PrintShort(0, cut_width);
	//RTNA_DBG_PrintShort(0, cut_height);
    pGRA->GRA_BLT_W = cut_width;
    pGRA->GRA_BLT_H = cut_height;   
    pGRA->GRA_BLT_FMT= ((colordepth - 1) << 2)	// dst
									| (colordepth - 1);    
    pGRA->GRA_BLT_ROP = ((MMP_UBYTE)ropcode & GRA_ROP_MASK);
    pGRA->GRA_BLT_ROP_CTL |= GRA_OP_DO_ROP | GRA_MEM_2_MEM;

   // if (m_bTransActive)
  //      MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);

    // Deal Source Buffer
	pGRA->GRA_BLT_SRC_PITCH = srcbufattribute.usLineOffset;

    // Deal Destination Buffer
	pGRA->GRA_BLT_DST_PITCH = dstbufattribute.usLineOffset;


    pGRA->GRA_BLT_EN = GRA_BLT_ST;

	//while (pGRA->GRA_BLT_EN & GRA_BLT_ST);

    return MMP_ERR_NONE;
}
#endif


