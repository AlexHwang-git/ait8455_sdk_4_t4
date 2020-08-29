//==============================================================================
//
//  File        : mmpf_scaler.c
//  Description : Firmware Display Control Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_vif.h" 
#include "mmpf_scaler.h"
#include "mmpf_hif.h"
#if (SENSOR_EN)
#include "mmpf_sensor.h"
#include "mmpf_rawproc.h"
#endif
#include "mmpf_ibc.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc.h"

/** @addtogroup MMPF_Scaler
@{
*/

static MMPF_SCALER_ZOOM_INFO m_scalerZoomInfo[TOTAL_SCALER_PATH];
static MMPF_SCALER_PANTILT_INFO m_scalerPanTiltInfo[TOTAL_SCALER_PATH];
MMP_UBYTE      gbZoomPathSel;
MMP_UBYTE      gbPanTiltPathSel ;
MMPF_SCALER_SWITCH_PATH m_scalerSwitch[TOTAL_SCALER_PATH];
MMP_USHORT gsScalerRefInH ;
MMP_USHORT gsScalerRefOutH[TOTAL_SCALER_PATH];
extern	MMP_UBYTE	m_gbSystemCoreID;
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
//  Function    : MMPF_Scaler_SetEngine
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL gbTestEnGrabOut= 1;

MMP_ERR MMPF_Scaler_SetEngine(MMP_BOOL bUserdefine, MMPF_SCALER_PATH pathsel,
                        MMPF_SCALER_FIT_RANGE *fitrange, MMPF_SCALER_GRABCONTROL *grabctl)
{
#define GRABOUT_CROP_MODE   (1)
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    MMP_USHORT  y_scale, x_scale;
    MMP_ULONG	max_scale_width;
    MMP_USHORT  unscale_width, unscale_height;
    AIT_REG_W   *OprGrabInHStart, *OprGrabInHEnd, *OprGrabInVStart, *OprGrabInVEnd;
    AIT_REG_W   *OprGrabOutHStart, *OprGrabOutHEnd, *OprGrabOutVStart, *OprGrabOutVEnd;
    AIT_REG_B   *OprNh, *OprMh, *OprNv, *OprMv;
    AIT_REG_B   *OprHWeight, *OprVWeight;
    AIT_REG_B   *OprScaleCtl;
    
    MMP_USHORT  scaleN,scaleM ;
    MMP_USHORT  scaler_ref_h ,scaler_real_h;
#if SENSOR_IN_H > SENSOR_16_9_H
    scaler_ref_h  = gsScalerRefInH ;
    scaler_real_h = gsScalerRefOutH[pathsel] ;
#else
    scaler_ref_h = fitrange->usInHeight ;
    scaler_real_h = fitrange->usOutHeight;
#endif    
    // For 800x600 , 160x120 H264
#if SENSOR_IN_H > SENSOR_16_9_H
    if(MMPF_Scaler_GetCurInfo(pathsel)->usSensorHeight==SENSOR_16_9_H) {
        scaler_real_h = fitrange->usOutHeight;
    }
	if(MMPF_Scaler_GetCurInfo(pathsel)->usSensorHeight==1536) {
		scaler_ref_h = fitrange->usOutHeight;
	}

#endif    

    if (pathsel == MMPF_SCALER_PATH_0) {
        OprScaleCtl     = &(pSCAL->SCAL_SCAL_0_CTL);
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_0_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_0_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_0_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_0_V_ED);
        OprGrabOutHStart= &(pSCAL->SCAL_GRAB_OUT_0_H_ST);
        OprGrabOutHEnd  = &(pSCAL->SCAL_GRAB_OUT_0_H_ED);
        OprGrabOutVStart= &(pSCAL->SCAL_GRAB_OUT_0_V_ST);
        OprGrabOutVEnd  = &(pSCAL->SCAL_GRAB_OUT_0_V_ED);
        OprNh           = &(pSCAL->SCAL_SCAL_0_H_N);
        OprMh           = &(pSCAL->SCAL_SCAL_0_H_M);
        OprNv           = &(pSCAL->SCAL_SCAL_0_V_N);
        OprMv           = &(pSCAL->SCAL_SCAL_0_V_M);
        OprHWeight      = &(pSCAL->SCAL_SCAL_0_H_WT);
        OprVWeight      = &(pSCAL->SCAL_SCAL_0_V_WT);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        OprScaleCtl     = &(pSCAL->SCAL_SCAL_1_CTL);
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_1_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_1_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_1_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_1_V_ED);
        OprGrabOutHStart= &(pSCAL->SCAL_GRAB_OUT_1_H_ST);
        OprGrabOutHEnd  = &(pSCAL->SCAL_GRAB_OUT_1_H_ED);
        OprGrabOutVStart= &(pSCAL->SCAL_GRAB_OUT_1_V_ST);
        OprGrabOutVEnd  = &(pSCAL->SCAL_GRAB_OUT_1_V_ED);
        OprNh           = &(pSCAL->SCAL_SCAL_1_H_N);
        OprMh           = &(pSCAL->SCAL_SCAL_1_H_M);
        OprNv           = &(pSCAL->SCAL_SCAL_1_V_N);
        OprMv           = &(pSCAL->SCAL_SCAL_1_V_M);
        OprHWeight      = &(pSCAL->SCAL_SCAL_1_H_WT);
        OprVWeight      = &(pSCAL->SCAL_SCAL_1_V_WT);
    }
    else {
        OprScaleCtl     = &(pSCAL->SCAL_SCAL_2_CTL);
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_2_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_2_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_2_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_2_V_ED);
        OprGrabOutHStart= &(pSCAL->SCAL_GRAB_OUT_2_H_ST);
        OprGrabOutHEnd  = &(pSCAL->SCAL_GRAB_OUT_2_H_ED);
        OprGrabOutVStart= &(pSCAL->SCAL_GRAB_OUT_2_V_ST);
        OprGrabOutVEnd  = &(pSCAL->SCAL_GRAB_OUT_2_V_ED);
        OprNh           = &(pSCAL->SCAL_SCAL_2_H_N);
        OprMh           = &(pSCAL->SCAL_SCAL_2_H_M);
        OprNv           = &(pSCAL->SCAL_SCAL_2_V_N);
        OprMv           = &(pSCAL->SCAL_SCAL_2_V_M);
        OprHWeight      = &(pSCAL->SCAL_SCAL_2_H_WT);
        OprVWeight      = &(pSCAL->SCAL_SCAL_2_V_WT);
    }
    
    if (bUserdefine == MMP_TRUE) {
        if(fitrange==0) {
    		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_OUT_0_H_ST) + (pathsel << 3)) = grabctl->usStartX;
    		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_OUT_0_H_ED) + (pathsel << 3)) = grabctl->usEndX;
    		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_OUT_0_V_ST) + (pathsel << 3)) = grabctl->usStartY;
    		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_OUT_0_V_ED) + (pathsel << 3)) = grabctl->usEndY;
    		*OprGrabOutHStart = grabctl->usStartX;
    		*OprGrabOutHEnd   = grabctl->usEndX;
    		*OprGrabOutVStart = grabctl->usStartY;
    		*OprGrabOutVEnd   = grabctl->usEndY;
			//dbg_printf(3, "No fitrange!!\r\n");
            return MMP_ERR_NONE;
        }
    }
    else {  // bUserdefine == MMP_FALSE
        grabctl->usScaleM = fitrange->usFitResol;

        #if 1 //scaling up v1
        if (fitrange->usInWidth >= fitrange->usOutWidth) {
            x_scale = (fitrange->usOutWidth * grabctl->usScaleM + fitrange->usInWidth - 1) / fitrange->usInWidth;
        }
        else {
            x_scale = (fitrange->usOutWidth * grabctl->usScaleM + fitrange->usInWidth - 2) / (fitrange->usInWidth - 1);
        }
        if (fitrange->usInHeight >= fitrange->usOutHeight) {
            y_scale = (fitrange->usOutHeight * grabctl->usScaleM + fitrange->usInHeight - 1) / fitrange->usInHeight;
        }
        else {
            y_scale = (fitrange->usOutHeight * grabctl->usScaleM + fitrange->usInHeight - 2) / (fitrange->usInHeight - 1);
        }
        //dbg_printf(0, "P#%d scale (X, Y) = (%d, %d)\r\n", pathsel, x_scale, y_scale);
        #else
        y_scale = (fitrange->usOutHeight * grabctl->usScaleM + fitrange->usInHeight - 1) / fitrange->usInHeight;
        x_scale = (fitrange->usOutWidth * grabctl->usScaleM + fitrange->usInWidth - 1) / fitrange->usInWidth;
        #endif

        if (fitrange->fitmode == MMPF_SCALER_FITMODE_OUT) {
            if (x_scale > y_scale)
                grabctl->usScaleN = x_scale;
            else
                grabctl->usScaleN = y_scale;
        }
        else if (fitrange->fitmode == MMPF_SCALER_FITMODE_IN) {
			RTNA_DBG_Str(3, "MMPF_SCALER_FITMODE_IN not support\r\n");
        }
        #if SENSOR_IN_H > SENSOR_16_9_H
        if(fitrange->usInHeight < scaler_ref_h) {
            grabctl->usStartX = (fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM - fitrange->usOutWidth) / 2 + 1;
            grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
            grabctl->usStartY = (fitrange->usInHeight * grabctl->usScaleN / grabctl->usScaleM - fitrange->usOutHeight) / 2 + 1;
            grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        } else {
            grabctl->usStartX = (fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM - fitrange->usOutWidth) / 2 + 1;
            grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
            grabctl->usStartY = (scaler_ref_h * grabctl->usScaleN / grabctl->usScaleM - scaler_real_h) / 2 + 1;
            grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        
        }
        #else
        grabctl->usStartX = (fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM - fitrange->usOutWidth) / 2 + 1;
        grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
        grabctl->usStartY = (fitrange->usInHeight * grabctl->usScaleN / grabctl->usScaleM - fitrange->usOutHeight) / 2 + 1;
        grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        #endif
        
        if(x_scale > grabctl->usScaleM) {
        //    dbg_printf(3,"grab calc 2(%d,%d,%d,%d),N[%d]\r\n",grabctl->usStartX,grabctl->usEndX,grabctl->usStartY,grabctl->usEndY ,x_scale);
        
        }
    }

	if (pathsel == MMPF_SCALER_PATH_0) {
        max_scale_width = SCALER_PATH0_MAX_WIDTH;	

    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_SCAL_0_H) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_SCAL_0_V) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_OUT_0_H) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_OUT_0_V) = SCAL_DNSAMP_NONE;
	}
	else if (pathsel == MMPF_SCALER_PATH_1) {
        max_scale_width = SCALER_PATH1_MAX_WIDTH;	

    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_SCAL_1_H) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_SCAL_1_V) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_OUT_1_H) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_OUT_1_V) = SCAL_DNSAMP_NONE;
	}
    else {
        max_scale_width = SCALER_PATH2_MAX_WIDTH;	

    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_SCAL_2_H) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_SCAL_2_V) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_OUT_2_H) = SCAL_DNSAMP_NONE;
    	*(AIT_REG_B *)&(pSCAL->SCAL_DNSAMP_OUT_2_V) = SCAL_DNSAMP_NONE;
    }
    
    *(AIT_REG_B *)((MMP_ULONG)&(pSCAL->SCAL_EDGE_0_CTL)) |= SCAL_EDGE_BYPASS;
    *(AIT_REG_B *)((MMP_ULONG)&(pSCAL->SCAL_EDGE_1_CTL)) |= SCAL_EDGE_BYPASS;
    *(AIT_REG_B *)((MMP_ULONG)&(pSCAL->SCAL_EDGE_2_CTL)) |= SCAL_EDGE_BYPASS;

    (*OprScaleCtl) &= ~(SCAL_SCAL_BYPASS);


    if(grabctl->usScaleN) {
        scaleN = grabctl->usScaleN ;
        scaleM = grabctl->usScaleM ;
  //      if (grabctl->usScaleN > grabctl->usScaleM) {
  //  	    (*OprScaleCtl) |=  (SCAL_UP_EARLY_ST);
  //      }
  //      else {
  //  	    (*OprScaleCtl) &=  ~(SCAL_UP_EARLY_ST);
  //      }
        *OprNh  = grabctl->usScaleN;
        *OprMh  = grabctl->usScaleM;
        *OprNv  = grabctl->usScaleN;
        *OprMv  = grabctl->usScaleM;
    
    } else {
    #if USB_LYNC_TEST_MODE
        scaleN = grabctl->usScaleXN ;
        scaleM = grabctl->usScaleXM ;
        if (grabctl->usScaleYN > grabctl->usScaleYM) {
    	    (*OprScaleCtl) |=  (SCAL_UP_EARLY_ST);
        }
        else {
    	    (*OprScaleCtl) &=  ~(SCAL_UP_EARLY_ST);
        }
        *OprNh  = grabctl->usScaleXN;
        *OprMh  = grabctl->usScaleXM;
        *OprNv  = grabctl->usScaleYN;
        *OprMv  = grabctl->usScaleYM;
    #endif
    }
    
    //dbg_printf(3,"scaler.engine : (N/M) : (%d/%d)\r\n",scaleN,scaleM);
    (*OprHWeight) &= ~(SCAL_SCAL_WT_AVG);
    (*OprVWeight) &= ~(SCAL_SCAL_WT_AVG);

    if ((fitrange->usInWidth * scaleN / scaleM > max_scale_width)||(scaleN > scaleM)){
        if ((grabctl->usScaleN > grabctl->usScaleM)) { //scale up function: UP((length_h-1)*Nh/Mh) * UP((length_v-1)*Nv/Mv)
            MMP_ULONG x_st = (grabctl->usStartX*scaleM + scaleN -1)/scaleN;
            MMP_ULONG y_st = (grabctl->usStartY*scaleM + scaleN -1)/scaleN;

            unscale_width  = 3 + ((grabctl->usEndX - grabctl->usStartX) * scaleM ) / scaleN;
            unscale_height = 3 + ((grabctl->usEndY - grabctl->usStartY) * scaleM ) / scaleN;
            if ((x_st + unscale_width - 1) > fitrange->usInWidth) {
            //unscale_width = fitrange->usInWidth - x_st + 1;
            // patrick remove for GRA case
            	unscale_width = fitrange->usInWidth;
            }
            if ((y_st + unscale_height - 1) > fitrange->usInHeight) {
            //unscale_height = fitrange->usInHeight - y_st + 1;
            // patrick remove for GRA case
            	unscale_height = fitrange->usInHeight;
            }
            //dbg_printf(3, "scale up function:Fit(%d,%d),Unscale(%d,%d)\r\n",fitrange->usInWidth,fitrange->usInHeight,unscale_width,unscale_height);
        }
        else { //scale down function: DOWN((length_h)*Nh/Mh) * DOWN((length)*Nv/Mv)
           // dbg_printf(3, "scale down function.\r\n");
            unscale_width = ((grabctl->usEndX - grabctl->usStartX + 1)  * scaleM + (scaleN - 1)) / scaleN;
            unscale_height = ((grabctl->usEndY - grabctl->usStartY + 1) * scaleM + (scaleN - 1)) / scaleN;
        }

        #if 1//(DSC_GRAB_CENTER)
        
        if(scaleN > scaleM) {
            *OprGrabInHStart    = (fitrange->usInWidth-unscale_width)/2 + 1;
            *OprGrabInHEnd      = *OprGrabInHStart + unscale_width - 1;
            *OprGrabInVStart    = (fitrange->usInHeight -unscale_height)/2 + 1;
            *OprGrabInVEnd      = *OprGrabInVStart + unscale_height - 1;
        } else {
			MMP_SHORT y = 0;
            *OprGrabInHStart    = (fitrange->usInWidth-unscale_width)/2 + 1;
            *OprGrabInHEnd      = *OprGrabInHStart + unscale_width - 1;
            //*OprGrabInVStart    = ( /*fitrange->usInHeight*/scaler_ref_h -unscale_height)/2 + 1;
			y  = ( scaler_ref_h -unscale_height)/2 + 1;
			if(y < 1) y = 1 ;
			*OprGrabInVStart    = y;
            *OprGrabInVEnd      = y + /* *OprGrabInVStart*/ + unscale_height - 1;
        }        
        #else
        *OprGrabInHStart    = (grabctl->usStartX*scaleM + scaleN -1)/scaleN;
        *OprGrabInHEnd      = *OprGrabInHStart + unscale_width - 1;
        *OprGrabInVStart    = (grabctl->usStartY*scaleM + scaleN -1)/scaleN;
        *OprGrabInVEnd      = *OprGrabInVStart + unscale_height - 1;
        #endif

		*OprGrabOutHStart   = 1;
		*OprGrabOutHEnd     = grabctl->usEndX - grabctl->usStartX + 1;
		*OprGrabOutVStart   = 1;
		*OprGrabOutVEnd     = grabctl->usEndY - grabctl->usStartY + 1;
	}
	else {
        *OprGrabInHStart    = 1;
        *OprGrabInHEnd      = fitrange->usInWidth;
        *OprGrabInVStart    = 1;
        *OprGrabInVEnd      = fitrange->usInHeight;

		#if 0//(DSC_GRAB_CENTER) // for pan/tilt, set #if 0
		
		if(grabctl->usScaleN) {
    		unscale_width = (fitrange->usInWidth * scaleN)/scaleM;
    		unscale_height = (fitrange->usInHeight * scaleN)/scaleM;
    		*OprGrabOutHStart   = (unscale_width- (grabctl->usEndX - grabctl->usStartX + 1))/2 + 1;
    		*OprGrabOutHEnd     = *OprGrabOutHStart + (grabctl->usEndX - grabctl->usStartX);
    		*OprGrabOutVStart   = (unscale_height- (grabctl->usEndY - grabctl->usStartY + 1))/2 + 1;
    		*OprGrabOutVEnd     = *OprGrabOutVStart + (grabctl->usEndY - grabctl->usStartY);
		
		} 
		else {
    		*OprGrabOutHStart   = grabctl->usStartX;
    		*OprGrabOutHEnd     = grabctl->usEndX;
    		*OprGrabOutVStart   = grabctl->usStartY;
    		*OprGrabOutVEnd     = grabctl->usEndY;
		} 
	
		#else
		*OprGrabOutHStart   = grabctl->usStartX;
		*OprGrabOutHEnd     = grabctl->usEndX;
		*OprGrabOutVStart   = grabctl->usStartY;
		*OprGrabOutVEnd     = grabctl->usEndY;
		#endif
    }
	//Jared test
	#if 0
	*OprGrabInHStart =  (SENSOR_16_9_W - fitrange->usInWidth)/2+1 ;
	*OprGrabInHEnd = *OprGrabInHStart + fitrange->usInWidth -1;
	*OprGrabInVStart =  (SENSOR_16_9_H - fitrange->usInHeight)/2+1 ;
	*OprGrabInVEnd = *OprGrabInVStart + fitrange->usInHeight -1;	
		*OprGrabOutHStart   = *OprGrabInHStart;
		*OprGrabOutHEnd     = *OprGrabOutHStart + 1920 -1 ;//fitrange->usOutWidth -1;
		*OprGrabOutVStart   = *OprGrabInVStart;
		*OprGrabOutVEnd     = *OprGrabOutVStart + 1080 -1;//fitrange->usOutHeight -1;
	#endif
    if(gbTestEnGrabOut) { 
    dbg_printf(0,"--grab%d in(%d,%d,%d,%d),[%d/%d]\r\n",pathsel,*OprGrabInHStart,*OprGrabInHEnd,*OprGrabInVStart,*OprGrabInVEnd ,*OprNh,*OprMh);
    dbg_printf(0,"--grab%d ot(%d,%d,%d,%d),[%d/%d]\r\n",pathsel,*OprGrabOutHStart,*OprGrabOutHEnd,*OprGrabOutVStart,*OprGrabOutVEnd ,*OprNv,*OprMh);
    }
    if ((*OprNh)*5 < (*OprMh)*3) {
        *OprHWeight = (*OprNh) >> 1;
        *OprVWeight = (*OprNv) >> 1;
    }
    else {
        *OprHWeight = 0;
        *OprVWeight = 0;
    }
	
#if 1  // Modify for early scale up   
    //if ((*OprNh > *OprMh) || (*OprNv > *OprMv)) { 
    *OprScaleCtl |= SCAL_UP_EARLY_ST;
/*
    if ((*OprNh == *OprMh) || (*OprNv == *OprMv)) {
    	*OprScaleCtl |= SCAL_SCAL_BYPASS;  
    }
    else {
    	*OprScaleCtl &= ~SCAL_SCAL_BYPASS;  
    }  	
*/
    //}
#endif
   return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPDF_Scaler_SetLPF
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function set the LPF coefficient according to the grab control
		
The function calculates the best grab parameter..
@param[in] pathsel the scaling path in Scaler
@param[in] fitrange the fit range parameters
@param[in] grabctl the grab control parameters
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Scaler_SetLPF(MMPF_SCALER_PATH pathsel, MMPF_SCALER_FIT_RANGE *fitrange,
                            MMPF_SCALER_GRABCONTROL *grabctl)
{
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    MMP_ULONG	max_scale_width;
    MMP_USHORT  scaleN ,scaleM ;
    AIT_REG_W   *OprGrabInHStart, *OprGrabInHEnd;
    AIT_REG_W   *OprGrabInVStart, *OprGrabInVEnd;
    AIT_REG_B   *OprNh, *OprMh, *OprNv, *OprMv;
    AIT_REG_B   *OprHWeight, *OprVWeight;
    
    max_scale_width = LPF_MAX_WIDTH;

    if (pathsel == MMPF_SCALER_PATH_0) {
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_0_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_0_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_0_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_0_V_ED);
        OprNh           = &(pSCAL->SCAL_SCAL_0_H_N);
        OprMh           = &(pSCAL->SCAL_SCAL_0_H_M);
        OprNv           = &(pSCAL->SCAL_SCAL_0_V_N);
        OprMv           = &(pSCAL->SCAL_SCAL_0_V_M);
        OprHWeight      = &(pSCAL->SCAL_SCAL_0_H_WT);
        OprVWeight      = &(pSCAL->SCAL_SCAL_0_V_WT);

        pSCAL->SCAL_LPF_AUTO_CTL    |= SCAL_LPF0_EN;

        pSCAL->SCAL_GRAB_LPF_H_ST   = 1;
        pSCAL->SCAL_GRAB_LPF_H_ED   = fitrange->usInWidth;
        pSCAL->SCAL_GRAB_LPF_V_ST   = 1;
        pSCAL->SCAL_GRAB_LPF_V_ED   = fitrange->usInHeight;

        pSCAL->SCAL_DNSAMP_LPF_H    = SCAL_DNSAMP_NONE;
        pSCAL->SCAL_DNSAMP_LPF_V    = SCAL_DNSAMP_NONE;

        pSCAL->SCAL_LPF_CTL         = SCAL_LPF_BYPASS;
        pSCAL->SCAL_LPF_SCAL_CTL    = SCAL_LPF_DNSAMP_NONE;
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_1_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_1_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_1_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_1_V_ED);
        OprNh           = &(pSCAL->SCAL_SCAL_1_H_N);
        OprMh           = &(pSCAL->SCAL_SCAL_1_H_M);
        OprNv           = &(pSCAL->SCAL_SCAL_1_V_N);
        OprMv           = &(pSCAL->SCAL_SCAL_1_V_M);
        OprHWeight      = &(pSCAL->SCAL_SCAL_1_H_WT);
        OprVWeight      = &(pSCAL->SCAL_SCAL_1_V_WT);
        
        pSCAL->SCAL_LPF_AUTO_CTL    |= SCAL_LPF1_EN;	

        pSCAL->SCAL_GRAB_LPF1_H_ST  = 1;
        pSCAL->SCAL_GRAB_LPF1_H_ED  = fitrange->usInWidth;
        pSCAL->SCAL_GRAB_LPF1_V_ST  = 1;
        pSCAL->SCAL_GRAB_LPF1_V_ED  = fitrange->usInHeight;

        pSCAL->SCAL_DNSAMP_LPF1_H   = SCAL_DNSAMP_NONE;
        pSCAL->SCAL_DNSAMP_LPF1_V   = SCAL_DNSAMP_NONE;
 
        pSCAL->SCAL_LPF1_SCAL_CTL   = SCAL_LPF_DNSAMP_NONE;
    }
    else {
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_2_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_2_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_2_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_2_V_ED);
        OprNh           = &(pSCAL->SCAL_SCAL_2_H_N);
        OprMh           = &(pSCAL->SCAL_SCAL_2_H_M);
        OprNv           = &(pSCAL->SCAL_SCAL_2_V_N);
        OprMv           = &(pSCAL->SCAL_SCAL_2_V_M);
        OprHWeight      = &(pSCAL->SCAL_SCAL_2_H_WT);
        OprVWeight      = &(pSCAL->SCAL_SCAL_2_V_WT);

        pSCAL->SCAL_LPF2_AUTO_CTL    |= SCAL_LPF2_EN;	

        pSCAL->SCAL_GRAB_LPF2_H_ST  = 1;
        pSCAL->SCAL_GRAB_LPF2_H_ED  = fitrange->usInWidth;
        pSCAL->SCAL_GRAB_LPF2_V_ST  = 1;
        pSCAL->SCAL_GRAB_LPF2_V_ED  = fitrange->usInHeight;

        pSCAL->SCAL_DNSAMP_LPF2_H   = SCAL_DNSAMP_NONE;
        pSCAL->SCAL_DNSAMP_LPF2_V   = SCAL_DNSAMP_NONE;

        pSCAL->SCAL_LPF2_SCAL_CTL   = SCAL_LPF_DNSAMP_NONE;
     }

    if (fitrange->usInWidth > max_scale_width) {
    	if (pathsel == MMPF_SCALER_PATH_0) {
    	    pSCAL->SCAL_LPF_AUTO_CTL &= ~SCAL_LPF0_EN;
    	}
    	else if (pathsel == MMPF_SCALER_PATH_1) {
    	    pSCAL->SCAL_LPF_AUTO_CTL &= ~SCAL_LPF1_EN;
    	}
    	else {
    	    pSCAL->SCAL_LPF2_AUTO_CTL &= ~SCAL_LPF2_EN;
    	}
        return	MMP_ERR_NONE;                      
    }
    scaleN = grabctl->usScaleN ;
    scaleM = grabctl->usScaleM ;
#if USB_LYNC_TEST_MODE        
    if(scaleN==0) {
        //scaleN=(grabctl->usScaleXN > grabctl->usScaleYN) ?grabctl->usScaleXN:grabctl->usScaleYN ;
        scaleN = grabctl->usScaleYN ;
        scaleM = grabctl->usScaleYM ;
    }
#endif 
	if (scaleN >= scaleM) {
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        if (pathsel == MMPF_SCALER_PATH_0) {
            pSCAL->SCAL_LPF_CTL  = SCAL_LPF_BYPASS;
        }    
        #endif
	} 
   	else if (2*(scaleN) >= scaleM) {	//	1->1/2
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        if (pathsel == MMPF_SCALER_PATH_0) {
            pSCAL->SCAL_LPF_CTL  = SCAL_LPF_BYPASS;
        }     
        #endif
	}
	else if (4*(scaleN) >= scaleM) {	//	1/2->1/4
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ST) + (pathsel << 3)) = 
		//		ALIGN2(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ST) + (pathsel << 3))) >> 1;
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ST) + (pathsel << 3)) = 
		//		ALIGN2(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ST) + (pathsel << 3))) >> 1;
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ED) + (pathsel << 3)) = 
		//		ALIGN2(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ED) + (pathsel << 3))) >> 1;
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ED) + (pathsel << 3)) = 
		//		ALIGN2(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ED) + (pathsel << 3))) >> 1;
        (*OprGrabInHStart)  = ALIGN2(*OprGrabInHStart)  >> 1;
        (*OprGrabInHEnd)    = ALIGN2(*OprGrabInHEnd)    >> 1;
        (*OprGrabInVStart)  = ALIGN2(*OprGrabInVStart)  >> 1;
        (*OprGrabInVEnd)    = ALIGN2(*OprGrabInVEnd)    >> 1;
				
        //(*(AIT_REG_B *)((MMP_ULONG)&(pSCAL->SCAL_SCAL_0_H_N) + ((pathsel)?6:0))) <<= 1; 
        //(*(AIT_REG_B *)((MMP_ULONG)&(pSCAL->SCAL_SCAL_0_V_N) + ((pathsel)?6:0))) <<= 1; 
        (*OprNh) = (*OprNh) << 1;
        (*OprNv) = (*OprNv) << 1;

        if (pathsel == MMPF_SCALER_PATH_0) {
            pSCAL->SCAL_LPF_SCAL_CTL = SCAL_LPF_DNSAMP_1_2 | 
                                       SCAL_LPF_Y_L1_EN | 
                                       SCAL_LPF_UV_EN;
        }
        else if (pathsel == MMPF_SCALER_PATH_1) {
            pSCAL->SCAL_LPF1_SCAL_CTL = SCAL_LPF_DNSAMP_1_2 | 
                                        SCAL_LPF_Y_L1_EN | 
                                        SCAL_LPF_UV_EN;
        }
        else {
            pSCAL->SCAL_LPF2_SCAL_CTL = SCAL_LPF_DNSAMP_1_2 | 
                                        SCAL_LPF_Y_L1_EN | 
                                        SCAL_LPF_UV_EN;
        }
	}
	else {	//	1/4
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ST) + (pathsel << 3)) = 
		//		ALIGN4(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ST) + (pathsel << 3))) >> 2;
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ST) + (pathsel << 3)) = 
		//		ALIGN4(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ST) + (pathsel << 3))) >> 2;
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ED) + (pathsel << 3)) = 
		//		ALIGN4(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_H_ED) + (pathsel << 3))) >> 2;
		//*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ED) + (pathsel << 3)) = 
		//		ALIGN4(*(AIT_REG_W *)((MMP_ULONG)&(pSCAL->SCAL_GRAB_SCAL_0_V_ED) + (pathsel << 3))) >> 2;
        (*OprGrabInHStart)  = ALIGN4(*OprGrabInHStart)  >> 2;
        (*OprGrabInHEnd)    = ALIGN4(*OprGrabInHEnd)    >> 2;
        (*OprGrabInVStart)  = ALIGN4(*OprGrabInVStart)  >> 2;
        (*OprGrabInVEnd)    = ALIGN4(*OprGrabInVEnd)    >> 2;
				
        //(*(AIT_REG_B *)((MMP_ULONG)&(pSCAL->SCAL_SCAL_0_H_N) + ((pathsel)?6:0)) ) <<= 2; 
        //(*(AIT_REG_B *)((MMP_ULONG)&(pSCAL->SCAL_SCAL_0_V_N) + ((pathsel)?6:0)) ) <<= 2; 
        (*OprNh) = (*OprNh) << 2;
        (*OprNv) = (*OprNv) << 2;

        if (pathsel == MMPF_SCALER_PATH_0) {
            pSCAL->SCAL_LPF_SCAL_CTL = SCAL_LPF_DNSAMP_1_4 | 
                               SCAL_LPF_Y_L1_EN | 
                               SCAL_LPF_Y_L2_EN |
                               SCAL_LPF_UV_EN;
        }
        else if (pathsel == MMPF_SCALER_PATH_1) {
            pSCAL->SCAL_LPF1_SCAL_CTL = SCAL_LPF_DNSAMP_1_4 |
                                SCAL_LPF_Y_L1_EN |
                                SCAL_LPF_Y_L2_EN |
                                SCAL_LPF_UV_EN;
        }
        else {
            pSCAL->SCAL_LPF2_SCAL_CTL = SCAL_LPF_DNSAMP_1_4 |
                                SCAL_LPF_Y_L1_EN |
                                SCAL_LPF_Y_L2_EN |
                                SCAL_LPF_UV_EN;
        }
	}

	if (pathsel == MMPF_SCALER_PATH_0) {
		if ((pSCAL->SCAL_LPF_CTL) || (pSCAL->SCAL_LPF_SCAL_CTL))
			pSCAL->SCAL_LPF_AUTO_CTL |=SCAL_LPF0_EN;
		else
			pSCAL->SCAL_LPF_AUTO_CTL &= (~SCAL_LPF0_EN);
	}
	else if (pathsel == MMPF_SCALER_PATH_1) {
		if (pSCAL->SCAL_LPF1_SCAL_CTL)
			pSCAL->SCAL_LPF_AUTO_CTL |= SCAL_LPF1_EN;
		else
			pSCAL->SCAL_LPF_AUTO_CTL &= (~SCAL_LPF1_EN);
	}
	else {
		if (pSCAL->SCAL_LPF2_SCAL_CTL)
			pSCAL->SCAL_LPF2_AUTO_CTL |= SCAL_LPF2_EN;
		else
			pSCAL->SCAL_LPF2_AUTO_CTL &= (~SCAL_LPF2_EN);
	}

    if ((*OprNh)*5 < (*OprMh)*3) {
        *OprHWeight = (*OprNh) >> 1;
        *OprVWeight = (*OprNv) >> 1;
    }
    else {
        *OprHWeight = 0;
        *OprVWeight = 0;
    }

    return  MMP_ERR_NONE;    
}

MMP_ERR MMPF_Scaler_SetOutputFormat(MMPF_SCALER_PATH pathsel, MMPF_SCALER_COLORMODE outcolor)
{
    AITPS_SCAL      pSCAL = AITC_BASE_SCAL;
    AIT_REG_B       *pOutCtlOpr;

    if (pathsel == MMPF_SCALER_PATH_0) {
        pOutCtlOpr = (AIT_REG_B *)&(pSCAL->SCAL_OUT_0_CTL);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        pOutCtlOpr = (AIT_REG_B *)&(pSCAL->SCAL_OUT_1_CTL);
    }
    else {
        pOutCtlOpr = (AIT_REG_B *)&(pSCAL->SCAL_OUT_2_CTL);
    }

    switch (outcolor) {
    #if 0
    case MMPF_SCALER_COLOR_RGB565:
        (*pOutCtlOpr) &= ~SCAL_OUT_YUV_EN;
        //MMPH_HIF_RegSetB(SCAL_OUT_0_CTL + offset, SCAL_OUT_RGB_EN |
        //    SCAL_OUT_FMT_RGB565 |
		//	SCAL_OUT_DITHER_EN |
		//	SCAL_OUT_RGB);
        break;
    #endif
	case MMPF_SCALER_COLOR_YUV422:
	    (*pOutCtlOpr) |= (SCAL_OUT_YUV_EN|SCAL_OUT_FMT_YUV422);
        break;	
    #if 0
    case MMPF_SCALER_COLOR_RGB888:
        MMPH_HIF_RegSetB(SCAL_OUT_0_CTL + offset, SCAL_OUT_RGB_EN |
			SCAL_OUT_FMT_RGB888 |
			SCAL_OUT_RGB);
        break;
    case MMPF_SCALER_COLOR_YUV444:
        MMPH_HIF_RegSetB(SCAL_OUT_0_CTL + offset, SCAL_OUT_YUV_EN |
            SCAL_OUT_FMT_YUV444);
        break;
    #endif
    default:
        return MMP_SCALER_ERR_PARAMETER;
    }

    return MMP_ERR_NONE;
}

#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
static MMP_SHORT m_ScalerColorOffset_BT601[3] = {
    16, 128, 128
};
static MMP_SHORT m_ScalerColorMatrix_BT601[3][3] = {
    {219,   0,   0},
    {  0, 224,   0},
    {  0,   0, 224}
};
static MMP_SHORT m_ScalerColorClip_BT601[3][2] = {
    { 16, 235},
    { 16, 240},
    { 16, 240}
};
#endif

MMP_ERR MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH pathsel, MMP_BOOL bEnable, MMPF_SCALER_COLRMTX_MODE MatrixMode)
{
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AITPS_SCAL pSCAL = AITC_BASE_SCAL;
    volatile AITS_SCAL_MTX *pMtxOpr;
    volatile AITS_SCAL_COLR_CLIP *pClipOpr;
    MMP_ULONG   i;
    MMP_LONG    s_coeff;
   // dbg_printf(3,"scaler.color: path:%d,enable:%d,mode:%d\r\n",pathsel,bEnable,MatrixMode);
    if (pathsel == MMPF_SCALER_PATH_0) {
        pMtxOpr = &(pSCAL->SCAL_P0_CT_MTX);
        pClipOpr = &(pSCAL->SCAL_P0_MTX_CLIP);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        pMtxOpr = &(pSCAL->SCAL_P1_CT_MTX);
        pClipOpr = &(pSCAL->SCAL_P1_MTX_CLIP);
    }
    else {
        pMtxOpr = &(pSCAL->SCAL_P2_CT_MTX);
        pClipOpr = &(pSCAL->SCAL_P2_MTX_CLIP);
    }

    if (bEnable) {
        switch (MatrixMode) {
        case MMPF_SCALER_COLRMTX_BT601:
            pMtxOpr->MTX_COEFF_ROW1_MSB &= ~(SCAL_MTX_COL_COEFF_MSB_MASK);
            pMtxOpr->MTX_COEFF_ROW2_MSB &= ~(SCAL_MTX_COL_COEFF_MSB_MASK);
            pMtxOpr->MTX_COEFF_ROW3_MSB &= ~(SCAL_MTX_COL_COEFF_MSB_MASK);
            for (i = 0; i < 3; i++) {
                s_coeff = SIGN_BIT_CONVERT(m_ScalerColorMatrix_BT601[0][i], 10);
                pMtxOpr->MTX_COEFF_ROW1[i] = (s_coeff & 0xFF);
                pMtxOpr->MTX_COEFF_ROW1_MSB |= (SCAL_MTX_COL_COEFF_MSB(s_coeff, i));

                s_coeff = SIGN_BIT_CONVERT(m_ScalerColorMatrix_BT601[1][i], 10);
                pMtxOpr->MTX_COEFF_ROW2[i] = (s_coeff & 0xFF);
                pMtxOpr->MTX_COEFF_ROW2_MSB |= (SCAL_MTX_COL_COEFF_MSB(s_coeff, i));

                s_coeff = SIGN_BIT_CONVERT(m_ScalerColorMatrix_BT601[2][i], 10);
                pMtxOpr->MTX_COEFF_ROW3[i] = (s_coeff & 0xFF);
                pMtxOpr->MTX_COEFF_ROW3_MSB |= (SCAL_MTX_COL_COEFF_MSB(s_coeff, i));
            }

            s_coeff = SIGN_BIT_CONVERT(m_ScalerColorOffset_BT601[0], 9);
            pMtxOpr->MTX_Y_OFST = (s_coeff & 0xFF);
            if (s_coeff & 0x100) {
                pMtxOpr->COLRMTX_CTL |= SCAL_MTX_Y_OFST_MSB;
            }
            else {
                pMtxOpr->COLRMTX_CTL &= ~(SCAL_MTX_Y_OFST_MSB);
            }

            s_coeff = SIGN_BIT_CONVERT(m_ScalerColorOffset_BT601[1], 9);
            pMtxOpr->MTX_U_OFST = (s_coeff & 0xFF);
            if (s_coeff & 0x100) {
                pMtxOpr->COLRMTX_CTL |= SCAL_MTX_U_OFST_MSB;
            }
            else {
                pMtxOpr->COLRMTX_CTL &= ~(SCAL_MTX_U_OFST_MSB);
            }

            s_coeff = SIGN_BIT_CONVERT(m_ScalerColorOffset_BT601[2], 9);
            pMtxOpr->MTX_V_OFST = (s_coeff & 0xFF);
            if (s_coeff & 0x100) {
                pMtxOpr->COLRMTX_CTL |= SCAL_MTX_V_OFST_MSB;
            }
            else {
                pMtxOpr->COLRMTX_CTL &= ~(SCAL_MTX_V_OFST_MSB);
            }

            pClipOpr->OUT_Y_MIN = m_ScalerColorClip_BT601[0][0];
            pClipOpr->OUT_Y_MAX = m_ScalerColorClip_BT601[0][1];
            pClipOpr->OUT_U_MIN = m_ScalerColorClip_BT601[1][0];
            pClipOpr->OUT_U_MAX = m_ScalerColorClip_BT601[1][1];
            pClipOpr->OUT_V_MIN = m_ScalerColorClip_BT601[2][0];
            pClipOpr->OUT_V_MAX = m_ScalerColorClip_BT601[2][1];
            break;
        case MMPF_SCALER_COLRMTX_FULLRANGE:
            pMtxOpr->COLRMTX_CTL &= ~(SCAL_COLRMTX_EN);
            return MMP_ERR_NONE;
        default:
            return MMP_SCALER_ERR_PARAMETER;
        }
        pMtxOpr->COLRMTX_CTL |= SCAL_COLRMTX_EN;
    }
    else {
        pMtxOpr->COLRMTX_CTL &= ~(SCAL_COLRMTX_EN);
    }

    #endif

    return MMP_ERR_NONE;
}


MMP_ERR MMPF_Scaler_SetOutputRange(MMPF_SCALER_PATH pathsel,
                                    MMP_UBYTE y_min,MMP_UBYTE y_max,
                                    MMP_UBYTE u_min,MMP_UBYTE u_max,
                                    MMP_UBYTE v_min,MMP_UBYTE v_max)
{
    AITPS_SCAL pSCAL = AITC_BASE_SCAL;
    AIT_REG_B   *OprScalCtl;
    volatile AITS_SCAL_COLR_CLIP *pClipOpr;
    if (pathsel == MMPF_SCALER_PATH_0) {
        pClipOpr = &(pSCAL->SCAL_P0_MTX_CLIP);
        OprScalCtl = &(pSCAL->SCAL_SCAL_0_CTL);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        pClipOpr = &(pSCAL->SCAL_P1_MTX_CLIP);
        OprScalCtl = &(pSCAL->SCAL_SCAL_1_CTL);
    }
    else {
        pClipOpr = &(pSCAL->SCAL_P2_MTX_CLIP);
        OprScalCtl = &(pSCAL->SCAL_SCAL_2_CTL);
    }
    if( *OprScalCtl & SCAL_SCAL_PATH_EN) {
        pClipOpr->OUT_Y_MIN = y_min;
        pClipOpr->OUT_Y_MAX = y_max;
        pClipOpr->OUT_U_MIN = u_min;
        pClipOpr->OUT_U_MAX = u_max;
        pClipOpr->OUT_V_MIN = v_min;
        pClipOpr->OUT_V_MAX = v_max;
        dbg_printf(0,"pipe.s[%d] = %d,%d,%d,%d,%d,%d\r\n",pathsel,
                        pClipOpr->OUT_Y_MIN,pClipOpr->OUT_Y_MAX,
                        pClipOpr->OUT_U_MIN,pClipOpr->OUT_U_MAX,
                        pClipOpr->OUT_V_MIN,pClipOpr->OUT_V_MAX);
                        
    }
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Scaler_GetOutputRange(MMPF_SCALER_PATH pathsel,
                                    MMP_UBYTE *y_min,MMP_UBYTE *y_max,
                                    MMP_UBYTE *u_min,MMP_UBYTE *u_max,
                                    MMP_UBYTE *v_min,MMP_UBYTE *v_max)
{
    AITPS_SCAL pSCAL = AITC_BASE_SCAL;
    volatile AITS_SCAL_COLR_CLIP *pClipOpr;
    AIT_REG_B   *OprScalCtl;
    if (pathsel == MMPF_SCALER_PATH_0) {
        pClipOpr = &(pSCAL->SCAL_P0_MTX_CLIP);
        OprScalCtl = &(pSCAL->SCAL_SCAL_0_CTL);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        pClipOpr = &(pSCAL->SCAL_P1_MTX_CLIP);
        OprScalCtl = &(pSCAL->SCAL_SCAL_1_CTL);
    }
    else {
        pClipOpr = &(pSCAL->SCAL_P2_MTX_CLIP);
        OprScalCtl = &(pSCAL->SCAL_SCAL_2_CTL);
    }
    if( *OprScalCtl & SCAL_SCAL_PATH_EN) {
        *y_min = pClipOpr->OUT_Y_MIN;
        *y_max = pClipOpr->OUT_Y_MAX;
        *u_min = pClipOpr->OUT_U_MIN;
        *u_max = pClipOpr->OUT_U_MAX;
        *v_min = pClipOpr->OUT_V_MIN;
        *v_max = pClipOpr->OUT_V_MAX;
        dbg_printf(0,"pipe.g[%d] = %d,%d,%d,%d,%d,%d\r\n",pathsel,
                        *y_min,*y_max,
                        *u_min,*u_max,
                        *v_min,*v_max);
    } 
    return MMP_ERR_NONE;
}

MMP_UBYTE MMPF_Scaler_GetMatrixCtl(MMPF_SCALER_PATH pathsel)
{
    AITPS_SCAL pSCAL = AITC_BASE_SCAL;
    volatile AITS_SCAL_MTX *pMtxOpr;
    if (pathsel == MMPF_SCALER_PATH_0) {
        pMtxOpr = &(pSCAL->SCAL_P0_CT_MTX);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        pMtxOpr = &(pSCAL->SCAL_P1_CT_MTX);
    }
    else {
        pMtxOpr = &(pSCAL->SCAL_P2_CT_MTX);
    }
    return pMtxOpr->COLRMTX_CTL ;
}

void MMPF_Scaler_SetMatrixCtl(MMPF_SCALER_PATH pathsel,MMP_UBYTE ctl)
{
    AITPS_SCAL pSCAL = AITC_BASE_SCAL;
    volatile AITS_SCAL_MTX *pMtxOpr;
    if (pathsel == MMPF_SCALER_PATH_0) {
        pMtxOpr = &(pSCAL->SCAL_P0_CT_MTX);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        pMtxOpr = &(pSCAL->SCAL_P1_CT_MTX);
    }
    else {
        pMtxOpr = &(pSCAL->SCAL_P2_CT_MTX);
    }
    pMtxOpr->COLRMTX_CTL = ctl ;
}



//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_SetZoomParams
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_SetZoomParams(MMPF_SCALER_PATH pathsel, MMPF_SCALER_GRABCONTROL *grabctl,
                        MMP_USHORT usInputWidth, MMP_USHORT usInputHeight, MMP_USHORT usStepX, MMP_USHORT usStepY)
{
    m_scalerZoomInfo[pathsel].usStepX = usStepX;
    m_scalerZoomInfo[pathsel].usStepY = usStepY;
    m_scalerZoomInfo[pathsel].usInputWidth = usInputWidth;
    m_scalerZoomInfo[pathsel].usInputHeight = usInputHeight;
    m_scalerZoomInfo[pathsel].grabCtl = *grabctl;
    m_scalerZoomInfo[pathsel].usBaseN = grabctl->usScaleN;
#if USB_LYNC_TEST_MODE
    m_scalerZoomInfo[pathsel].usBaseXN = grabctl->usScaleXN;
    m_scalerZoomInfo[pathsel].usBaseYN = grabctl->usScaleYN;
    m_scalerZoomInfo[pathsel].usBaseXM = grabctl->usScaleXM;
    m_scalerZoomInfo[pathsel].usBaseYM = grabctl->usScaleYM;
#endif

#if 0
    dbg_printf(3,"m_scalerZoomInfo[%d].usStepX : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usStepX);
    dbg_printf(3,"m_scalerZoomInfo[%d].usStepY : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usStepY);
    dbg_printf(3,"m_scalerZoomInfo[%d].usInputWidth : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usInputWidth);
    dbg_printf(3,"m_scalerZoomInfo[%d].usInputHeight : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usInputHeight);
    dbg_printf(3,"m_scalerZoomInfo[%d].usSensorWidth : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usSensorWidth);
    dbg_printf(3,"m_scalerZoomInfo[%d].usSensorHeight : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usSensorHeight);
    dbg_printf(3,"m_scalerZoomInfo[%d].usBaseN : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usBaseN);
    dbg_printf(3,"usScaleN:%d\r\n",grabctl->usScaleN);
    dbg_printf(3,"usScaleM:%d\r\n",grabctl->usScaleM);
    dbg_printf(3,"usStartX:%d\r\n",grabctl->usStartX);
    dbg_printf(3,"usStartY:%d\r\n",grabctl->usStartY);
    dbg_printf(3,"usEndX:%d\r\n",grabctl->usEndX);
    dbg_printf(3,"usEndY:%d\r\n",grabctl->usEndY);
    dbg_printf(3,"grabCtlIn.usScaleN:%d\r\n",m_scalerZoomInfo[pathsel].grabCtlIn.usStartX);
    dbg_printf(3,"grabCtlIn.usScaleM:%d\r\n",m_scalerZoomInfo[pathsel].grabCtlIn.usStartY);
    dbg_printf(3,"grabCtlIn.usStartX:%d\r\n",m_scalerZoomInfo[pathsel].grabCtlIn.usEndX);
    dbg_printf(3,"grabCtlIn.usStartY:%d\r\n",m_scalerZoomInfo[pathsel].grabCtlIn.usEndY);
#endif    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_SetZoomRange
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_SetZoomRange(MMPF_SCALER_PATH pathsel, MMP_USHORT usZoomMin, MMP_USHORT usZoomMax)
{
    
    m_scalerZoomInfo[pathsel].usRangeMinN = usZoomMin;
    m_scalerZoomInfo[pathsel].usRangeMaxN = usZoomMax;
    //dbg_printf(3,"m_scalerZoomInfo[%d].usRangeMinN : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usRangeMaxN);
    //dbg_printf(3,"m_scalerZoomInfo[%d].usRangeMaxN : %d\r\n",pathsel,m_scalerZoomInfo[pathsel].usRangeMaxN);
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Scaler_SetZoomRangeEx(MMPF_SCALER_PATH pathsel, MMP_USHORT usZoomMinX, MMP_USHORT usZoomMaxX,MMP_USHORT usZoomMinY, MMP_USHORT usZoomMaxY)
{
#if USB_LYNC_TEST_MODE    
    m_scalerZoomInfo[pathsel].usRangeMinXN = usZoomMinX;
    m_scalerZoomInfo[pathsel].usRangeMaxXN = usZoomMaxX;
    m_scalerZoomInfo[pathsel].usRangeMinYN = usZoomMinY;
    m_scalerZoomInfo[pathsel].usRangeMaxYN = usZoomMaxY;
#endif    
    return MMP_ERR_NONE ;
}


MMPF_SCALER_PANTILT_INFO* MMPF_Scaler_GetCurPanTiltInfo(MMPF_SCALER_PATH PathSel)
{
    return &m_scalerPanTiltInfo[PathSel] ;
}

//MMP_USHORT gsPanEnd = 0,gsTiltEnd = 0 ;
MMP_ERR MMPF_Scaler_SetPanTiltParams(MMPF_SCALER_PATH pathsel,MMP_LONG targetPan,MMP_LONG targetTilt,MMP_USHORT steps,MMP_LONG *pan_off,MMP_LONG *tilt_off)
{

#define TILT_DIR_X     (-1)
#define PAN_DIR_X      ( 1)
    AITPS_SCAL  pSCAL    = AITC_BASE_SCAL;
    MMPF_SCALER_ZOOM_INFO  *scalerInfo ,panTiltScalerInfo ; 
    MMPF_SCALER_GRABCONTROL *grab ; 
    MMP_LONG    pan_min,tilt_min,off_x,off_y ;
    MMP_USHORT limitW,limitH, grabW,grabH ;
    MMP_USHORT curStartX,curStartY, curEndX,curEndY;
    MMP_USHORT inGrabW,inGrabH ;
    MMP_USHORT scaler_ref_h,grab_ref_h ,grab_start_y =0 ;
	MMP_USHORT scaler_real_h ;
	
	scalerInfo = MMPF_Scaler_GetCurInfo(pathsel);	
	
#if SENSOR_IN_H > SENSOR_16_9_H
    scaler_ref_h = gsScalerRefInH;//SENSOR_16_9_H;// gsScalerRefH[pathsel];
	scaler_real_h = gsScalerRefOutH[pathsel] ;
	
#else
    scaler_ref_h = scalerInfo->usInputHeight ;
#endif
    if(pathsel==MMPF_SCALER_PATH_0) {
        curStartX = pSCAL->SCAL_GRAB_SCAL_0_H_ST ;
        curStartY = pSCAL->SCAL_GRAB_SCAL_0_V_ST ;
        curEndX   = pSCAL->SCAL_GRAB_SCAL_0_H_ED ;
        curEndY   = pSCAL->SCAL_GRAB_SCAL_0_V_ED ;
    } 
    else if(pathsel==MMPF_SCALER_PATH_1){
        curStartX = pSCAL->SCAL_GRAB_SCAL_1_H_ST ;
        curStartY = pSCAL->SCAL_GRAB_SCAL_1_V_ST ;
        curEndX   = pSCAL->SCAL_GRAB_SCAL_1_H_ED ;
        curEndY   = pSCAL->SCAL_GRAB_SCAL_1_V_ED ;
    }    
    else {
        curStartX = pSCAL->SCAL_GRAB_SCAL_2_H_ST ;
        curStartY = pSCAL->SCAL_GRAB_SCAL_2_V_ST ;
        curEndX   = pSCAL->SCAL_GRAB_SCAL_2_H_ED ;
        curEndY   = pSCAL->SCAL_GRAB_SCAL_2_V_ED ;
    }
    inGrabW = curEndX-curStartX + 1 ;
    inGrabH = curEndY-curStartY + 1 ;
    
   // dbg_printf(3,"[%d]Pos(Pan,Tilt) = (%d,%d)\r\n",pathsel,targetPan,targetTilt);
   // dbg_printf(3,"PanTilt.GrabIn(%d,%d)-(%d,%d),(W,H)=(%d,%d)\r\n",curStartX,curEndX,curStartY,curEndY,inGrabW,inGrabH);
    panTiltScalerInfo = *MMPF_Scaler_GetCurInfo(pathsel) ;
 
    if(steps==0) {
        steps      = m_scalerPanTiltInfo[pathsel].usStep ;
        targetPan  = m_scalerPanTiltInfo[pathsel].ulTargetPan;
        targetTilt = m_scalerPanTiltInfo[pathsel].ulTargetTilt;
        
    } else {
        m_scalerPanTiltInfo[pathsel].usStep = steps ;
    //
    // pipe 2 is as the soruce of GRA engine
    // In this case, we need to update pipe0 and pipe1 
    //
#if SUPPORT_GRA_ZOOM
        if(pathsel==MMPF_SCALER_PATH_2) {
            m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].usStep =  m_scalerPanTiltInfo[pathsel].usStep ; 
            m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].usStep =  m_scalerPanTiltInfo[pathsel].usStep ; 
        }
        else {
            m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].usStep =  m_scalerPanTiltInfo[pathsel].usStep ;
        }
#endif    
        
    }
    
    if(steps==0) {
        return MMP_SCALER_ERR_PARAMETER ;
    }

	m_scalerPanTiltInfo[pathsel].ulTargetPan = targetPan;
	m_scalerPanTiltInfo[pathsel].ulTargetTilt= targetTilt;
    dbg_printf(0,"[%d]Pos(Pan,Tilt) = (%d,%d)\r\n",pathsel,targetPan,targetTilt);
    dbg_printf(0,"PanTilt.GrabIn(%d,%d)-(%d,%d),(W,H)=(%d,%d)\r\n",curStartX,curEndX,curStartY,curEndY,inGrabW,inGrabH);


#if SUPPORT_GRA_ZOOM
    //
    // pipe 2 is as the soruce of GRA engine
    // In this case, we need to update pipe0 and pipe1 
    //
    if(pathsel==MMPF_SCALER_PATH_2) {
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].usStep      = m_scalerPanTiltInfo[pathsel].usStep ;      
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].ulTargetPan = m_scalerPanTiltInfo[pathsel].ulTargetPan ;      
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].ulTargetTilt= m_scalerPanTiltInfo[pathsel].ulTargetTilt ;      
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].usStep      = m_scalerPanTiltInfo[pathsel].usStep ;      
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].ulTargetPan = m_scalerPanTiltInfo[pathsel].ulTargetPan ;      
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].ulTargetTilt= m_scalerPanTiltInfo[pathsel].ulTargetTilt ;      
        
    }
    else {
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].usStep      = m_scalerPanTiltInfo[pathsel].usStep ;      
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].ulTargetPan = m_scalerPanTiltInfo[pathsel].ulTargetPan ;      
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].ulTargetTilt= m_scalerPanTiltInfo[pathsel].ulTargetTilt ;      
    
    }
#endif    
    
    scalerInfo = &panTiltScalerInfo;
    grab = &scalerInfo->grabCtl ;
	if(pathsel!=MMPF_SCALER_PATH_2) {
	    if(MMPF_Scaler_GetCurInfo(pathsel)->usSensorHeight==SENSOR_16_9_H) {
			MMPF_SCALER_GRABCONTROL *grabctl = &MMPF_Scaler_GetCurInfo(pathsel)->grabCtl ;
	        scaler_real_h = grabctl->usEndY - grabctl->usStartY + 1;
	    }
	}
    /*
    dbg_printf(3,"Input.GrabIn : (%d,%d)-(%d,%d)\r\n",
                scalerInfo->grabCtlIn.usStartX,scalerInfo->grabCtlIn.usEndX,
                scalerInfo->grabCtlIn.usStartY,scalerInfo->grabCtlIn.usEndY);
    dbg_printf(3,"Input.GrabOut : (%d,%d)-(%d,%d)\r\n",
                scalerInfo->grabCtl.usStartX,scalerInfo->grabCtl.usEndX,
                scalerInfo->grabCtl.usStartY,scalerInfo->grabCtl.usEndY);
    */
    // Don't support zoom if resolutio is scaling up
    if(scalerInfo->grabCtl.usScaleN > scalerInfo->grabCtl.usScaleM) {
        //dbg_printf(0,"PanTilt ( N,M) : (%x,%x)\r\n",scalerInfo->grabCtl.usScaleN,scalerInfo->grabCtl.usScaleM);
        m_scalerPanTiltInfo[pathsel].usPanEnd = 2 ;
        m_scalerPanTiltInfo[pathsel].usTiltEnd = 2;
        m_scalerPanTiltInfo[pathsel].grabCtl = panTiltScalerInfo.grabCtl ;
        *pan_off = 0 ;
        *tilt_off = 0 ;
        return MMP_ERR_NONE ;
    }
    
    m_scalerPanTiltInfo[pathsel].bGrabSrc = 0 ; // Grab OUT
    
    if(scalerInfo->usBaseN) {
        MMP_BOOL  grabsrc = MMP_FALSE;
        //MMP_SHORT outW = pSCAL->SCAL_GRAB_OUT_1_H_ED - pSCAL->SCAL_GRAB_OUT_1_H_ST + 1;
        //MMP_SHORT outH = pSCAL->SCAL_GRAB_OUT_1_V_ED - pSCAL->SCAL_GRAB_OUT_1_V_ST + 1;
        if(pathsel!=MMPF_SCALER_PATH_2) {
            grabsrc = MMPF_Scaler_IfGrabInput(pathsel,scalerInfo->usSensorWidth,scalerInfo->grabCtl.usScaleN,scalerInfo->grabCtl.usScaleM);
        }
        if( (pathsel==MMPF_SCALER_PATH_2)|| grabsrc/*((pathsel==MMPF_SCALER_PATH_1) && (outW==1920) && (outH==1088) ) */) {
            m_scalerPanTiltInfo[pathsel].bGrabSrc = 1 ; // Grab IN
            grab = &scalerInfo->grabCtlIn ;
           // dbg_printf(3,"OOXX-Pan/Tilt  by Grab In...\r\n");
            grab->usStartX = curStartX;
            grab->usStartY = curStartY;
            grab->usEndX = curEndX;
            grab->usEndY = curEndY ;
        #if 1
            limitW = scalerInfo->usSensorWidth ;
            #if SENSOR_IN_H > SENSOR_16_9_H
            limitH = scaler_ref_h;//SENSOR_16_9_H;
            //grab_start_y = grab->usStartY ;
            #else
            limitH = scalerInfo->usSensorHeight;
            #endif
            grabW = grab->usEndX - grab->usStartX + 1;
            grabH = grab->usEndY - grab->usStartY + 1;
            // Centerialize grab range
            grab->usStartX = 1 + (limitW - grabW) / 2 ;
            grab->usEndX   = grab->usStartX + grabW - 1;
            grab->usStartY = 1 + (limitH - grabH) / 2 ;
            grab->usEndY   = grab->usStartY + grabH - 1;
            // Set to center
            curStartX = 1 ;
            curEndX   = limitW ;
            curStartY = 1 ;
            curEndY   = limitH ;
            
        #endif    
        }
        else {
            
            limitW = (scalerInfo->usInputWidth  * grab->usScaleN)/  grab->usScaleM;
            limitH = (scaler_ref_h * grab->usScaleN)/  grab->usScaleM;
            
// Set to center for 720p skew image( zoom in<->out)
            curStartX = 1 ;
            curEndX   = scalerInfo->usSensorWidth ;
            curStartY = 1 ;
            #if SENSOR_IN_H > SENSOR_16_9_H
            curEndY   =  scaler_ref_h;//SENSOR_16_9_H;   
            #else
            curEndY   = scalerInfo->usSensorHeight ;            
            #endif
        }
    } else {
#if USB_LYNC_TEST_MODE
        limitW = (scalerInfo->usInputWidth  *  grab->usScaleXN)/ grab->usScaleXM;
        limitH = (scalerInfo->usInputHeight *  grab->usScaleYN)/ grab->usScaleYM;
#endif    
    }
    
    grabW = grab->usEndX - grab->usStartX ;
    grabH = grab->usEndY - grab->usStartY ;
#if SENSOR_IN_H > SENSOR_16_9_H
    if(scalerInfo->usBaseN) {
        if(m_scalerPanTiltInfo[pathsel].bGrabSrc==1) {
            grab_ref_h = grabH ;
        } 
        else {
            grab_ref_h = scaler_real_h - 1 ;//gsScalerRefOutH[pathsel] - 1;
        
        }
    } 
    else {
        grab_ref_h = grabH ;
    }
#endif    
    //dbg_printf(3,"@ Limit(W,H):(%d,%d),Grab(W,H):(%d,%d),steps:%d\r\n",limitW,limitH,grabW,grabH,steps);
    off_x = ( (limitW - (grabW + 1) ) ) / steps ; 
    
#if SENSOR_IN_H > SENSOR_16_9_H
    off_y = ( (limitH - (grab_ref_h + 1) ) ) / steps ;
#else
    off_y = ( (limitH - (grabH + 1) ) ) / steps ;
#endif
    //dbg_printf(3,"#pan,tilt step size:%d,%d\r\n",off_x,off_y);
    
    if(off_x) {
        pan_min  = curStartX / off_x ;
        *pan_off  =  targetPan * PAN_DIR_X- pan_min ;
    } else {
        *pan_off = 0;
    }
    if(off_y) {
        tilt_min = curStartY / off_y ; 
        *tilt_off =  targetTilt * TILT_DIR_X - tilt_min;                           
    } else {
        *tilt_off = 0 ;
    }
    //dbg_printf(3,"target pan : %d,*pan_off = %d\r\n", targetPan , *pan_off);
   

    off_x = ( (limitW - (grabW + 1) )* (*pan_off) )  / steps ; 
#if SENSOR_IN_H > SENSOR_16_9_H
    off_y = ( (limitH - (grab_ref_h + 1) )* (*tilt_off) ) / steps ;
#else
    off_y = ( (limitH - (grabH + 1) )* (*tilt_off) ) / steps ;
#endif    
    #if 0
    
    dbg_printf(3,"@ Grab(X0,Y0,X1,Y1):(%d,%d,%d,%d),Off(X,Y):(%d,%d)\r\n",
    grab->usStartX,
    grab->usStartY,
    grab->usEndX,
    grab->usEndY,
    off_x,off_y);
    #endif    

    if(*pan_off > 0 ) { // ->
        if( (limitW - grab->usEndX ) > off_x ) {
            grab->usStartX += off_x ;
		    grab->usEndX = grab->usStartX + grabW;
		    m_scalerPanTiltInfo[pathsel].usPanEnd = 0 ;
        } else {
            grab->usStartX = limitW - grabW ;
            grab->usEndX = limitW ;
            if( m_scalerPanTiltInfo[pathsel].usPanEnd==0) {
                 m_scalerPanTiltInfo[pathsel].usPanEnd = 1 ;
            }    
        }
    } else if(*pan_off < 0){ // <-
        off_x = 0 - off_x ;
        
        if( grab->usStartX > off_x ) {
            grab->usStartX -= off_x ;
             m_scalerPanTiltInfo[pathsel].usPanEnd = 0;
        }  else {
            grab->usStartX = 1;
            if( m_scalerPanTiltInfo[pathsel].usPanEnd==0) {
                 m_scalerPanTiltInfo[pathsel].usPanEnd = 1 ;
            }    
        }
        grab->usEndX = grab->usStartX + grabW ;
    }
    
    if(*tilt_off < 0 ) { // ^
        off_y = 0 - off_y ;
		if (grab->usStartY > off_y) {
			grab->usStartY -= off_y;
			grab->usEndY = grab->usStartY + grabH;
			 m_scalerPanTiltInfo[pathsel].usTiltEnd = 0;
		}
		else {
			grab->usStartY = 1;
			grab->usEndY = grab->usStartY + grabH;
			if( m_scalerPanTiltInfo[pathsel].usTiltEnd==0) {
			     m_scalerPanTiltInfo[pathsel].usTiltEnd = 1 ;
			}
		}        		
    } else if(*tilt_off > 0){ // down
        MMP_USHORT end_ref_y ;
#if SENSOR_IN_H > SENSOR_16_9_H
        if(scalerInfo->usBaseN) {
            if(m_scalerPanTiltInfo[pathsel].bGrabSrc == 1) {
                end_ref_y = grab->usEndY ;//????
            } else {
                end_ref_y = grab->usStartY + scaler_real_h - 1;//gsScalerRefOutH[pathsel] - 1 ;
            }
        } else {
            end_ref_y = grab->usEndY;
        }    
#else
        end_ref_y = grab->usEndY; 
#endif
  
		if ((limitH - end_ref_y/*grab->usEndY*/) > off_y) {
			grab->usStartY += off_y;
			grab->usEndY = grab->usStartY + grabH;
			 m_scalerPanTiltInfo[pathsel].usTiltEnd = 0 ;
		}
		else {
			grab->usStartY = limitH - grabH;
			grab->usEndY = limitH;
			if( m_scalerPanTiltInfo[pathsel].usTiltEnd==0) {
			     m_scalerPanTiltInfo[pathsel].usTiltEnd = 1 ;
			}
		}
    
    }
    m_scalerPanTiltInfo[pathsel].ulTargetPan  = targetPan ;
    m_scalerPanTiltInfo[pathsel].ulTargetTilt = targetTilt ;
   
    if ( m_scalerPanTiltInfo[pathsel].usPanEnd==2) {
        *pan_off = 0 ;
        //RTNA_DBG_Str3("PanEnd...\r\n");
    }
    else {
        //RTNA_DBG_Str3("CanPan...\r\n");
    }
    if( m_scalerPanTiltInfo[pathsel].usTiltEnd==2) {
        *tilt_off = 0;
    }
    if(1 /**pan_off || *tilt_off*/) {
        m_scalerPanTiltInfo[pathsel].grabCtl   = panTiltScalerInfo.grabCtl ;
        #if 1
        if(m_scalerPanTiltInfo[pathsel].bGrabSrc == 1 ) {
            m_scalerPanTiltInfo[pathsel].grabCtlIn = panTiltScalerInfo.grabCtlIn ;
            
            dbg_printf(GRA_DBG_LEVEL,"#[%d]PT-in:(%d,%d)-(%d,%d)\r\n",pathsel,
                        m_scalerPanTiltInfo[pathsel].grabCtlIn.usStartX,
                        m_scalerPanTiltInfo[pathsel].grabCtlIn.usStartY,
                        m_scalerPanTiltInfo[pathsel].grabCtlIn.usEndX,
                        m_scalerPanTiltInfo[pathsel].grabCtlIn.usEndY);
                         
        } else {
           
            dbg_printf(GRA_DBG_LEVEL,"#[%d]PT:(%d,%d)-(%d,%d)\r\n",pathsel,
                        m_scalerPanTiltInfo[pathsel].grabCtl.usStartX,
                        m_scalerPanTiltInfo[pathsel].grabCtl.usStartY,
                        m_scalerPanTiltInfo[pathsel].grabCtl.usEndX,
                        m_scalerPanTiltInfo[pathsel].grabCtl.usEndY);
                       
        }
        #endif
    }
#if SUPPORT_GRA_ZOOM
    if(pathsel==MMPF_SCALER_PATH_2) {
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].ulTargetPan  =  m_scalerPanTiltInfo[pathsel].ulTargetPan ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].ulTargetPan  =  m_scalerPanTiltInfo[pathsel].ulTargetPan ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].ulTargetTilt =  m_scalerPanTiltInfo[pathsel].ulTargetTilt ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].ulTargetTilt =  m_scalerPanTiltInfo[pathsel].ulTargetTilt ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].usPanEnd     =  m_scalerPanTiltInfo[pathsel].usPanEnd ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].usPanEnd     =  m_scalerPanTiltInfo[pathsel].usPanEnd ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].usTiltEnd    =  m_scalerPanTiltInfo[pathsel].usTiltEnd ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].usTiltEnd    =  m_scalerPanTiltInfo[pathsel].usTiltEnd ; 
    }
    else {
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].ulTargetPan  =  m_scalerPanTiltInfo[pathsel].ulTargetPan ;
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].ulTargetTilt =  m_scalerPanTiltInfo[pathsel].ulTargetTilt ;
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].usPanEnd     =  m_scalerPanTiltInfo[pathsel].usPanEnd ; 
        m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].usTiltEnd    =  m_scalerPanTiltInfo[pathsel].usTiltEnd ; 
    }
#endif    
    
    return MMP_ERR_NONE ;
}

MMP_ERR MMPF_Scaler_SetDigitalPanTilt(MMPF_SCALER_PATH pathsel)
{
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    gbPanTiltPathSel |= (1) << pathsel ;
    
    MMPF_HIF_SetCmdStatus(IMAGE_UNDER_PANTILT);  
    //sean@2010_12_08  
	pVIF->VIF_INT_CPU_SR[vid]  = ( VIF_INT_FRM_END  );
	pVIF->VIF_INT_CPU_EN[vid] |= ( VIF_INT_FRM_END  );
    return MMP_ERR_NONE ;	
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_SetDigitalZoom
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_SetDigitalZoom(MMPF_SCALER_PATH pathsel, MMPF_SCALER_ZOOMDIRECTION zoomdir, 
                                              MMP_USHORT zoomrate, MMP_USHORT startOP)
{
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;
    if(startOP) {
        gbZoomPathSel |=  (1)<< pathsel;
        dbg_printf(3,"S-Zoom:%d\r\n",gbZoomPathSel);
    }
    m_scalerZoomInfo[pathsel].direction = zoomdir;
    m_scalerZoomInfo[pathsel].usZoomRate = zoomrate;    
    
    if ((zoomdir == MMPF_SCALER_ZOOMIN) || (zoomdir == MMPF_SCALER_ZOOMOUT)) {
        if(startOP == MMP_TRUE){
            MMPF_HIF_SetCmdStatus(IMAGE_UNDER_ZOOM);
    		pVIF->VIF_INT_CPU_SR[vid] = ( VIF_INT_FRM_END  ) ;
    		pVIF->VIF_INT_CPU_EN[vid] |=( VIF_INT_FRM_END  );
        }
    }
    else if (zoomdir == MMPF_SCALER_ZOOMSTOP) {
        //RTNA_DBG_Str(3, "MMPF_Scaler_SetDigitalZoom():ZOOM_STOP_OP\r\n");
    }
    
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Scaler_EnableZoom(MMPF_SCALER_PATH pathsel)
{
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    gbZoomPathSel |=  (1)<< pathsel;
    dbg_printf(GRA_DBG_LEVEL,"E-Zoom:%x\r\n",gbZoomPathSel);
    MMPF_HIF_SetCmdStatus(IMAGE_UNDER_ZOOM);
    pVIF->VIF_INT_CPU_SR[vid] = ( VIF_INT_FRM_END  ) ;
    pVIF->VIF_INT_CPU_EN[vid] |=( VIF_INT_FRM_END  );
    return MMP_ERR_NONE ;
} 

MMP_ERR MMPF_Scaler_DisableZoom(MMPF_SCALER_PATH pathsel)
{
    gbZoomPathSel &= ~(1<<pathsel);
    dbg_printf(GRA_DBG_LEVEL,"D-Zoom:%x\r\n",gbZoomPathSel);
    return MMP_ERR_NONE ;
} 


//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_SetDigitalZoomOP
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_SetDigitalZoomOP(MMPF_SCALER_PATH pathsel, MMPF_SCALER_ZOOMDIRECTION zoomdir, 
                                              MMP_USHORT zoomrate)
{
    gbZoomPathSel |= (1) << pathsel;
    dbg_printf(GRA_DBG_LEVEL,"S-Zoom:%x\r\n",gbZoomPathSel);
     
    m_scalerZoomInfo[pathsel].direction = zoomdir;
    m_scalerZoomInfo[pathsel].usZoomRate = zoomrate;    
   
    return MMPF_Scaler_SetZoomOP();
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_AdjustSubScaler
//  Description : 
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_AdjustSubScaler(MMPF_SCALER_PATH mainpipe, MMPF_SCALER_PATH subpipe)
{
    MMPF_SCALER_GRABCONTROL grabctl;
    MMPF_SCALER_FIT_RANGE fitrange;

	if(m_scalerZoomInfo[mainpipe].grabCtl.usScaleN >= m_scalerZoomInfo[mainpipe].grabCtl.usScaleM){
        //do nothing
    } 
    else if (2*(m_scalerZoomInfo[mainpipe].grabCtl.usScaleN) >= m_scalerZoomInfo[mainpipe].grabCtl.usScaleM) {	//	1->1/2
        //do nothing
    }
    else if (4*(m_scalerZoomInfo[mainpipe].grabCtl.usScaleN) >= m_scalerZoomInfo[mainpipe].grabCtl.usScaleM) {	//	1/2->1/4
        fitrange.usInWidth = m_scalerZoomInfo[subpipe].usInputWidth >> 1;
        fitrange.usInHeight = m_scalerZoomInfo[subpipe].usInputHeight >> 1;
        grabctl = m_scalerZoomInfo[subpipe].grabCtl;
        grabctl.usScaleN <<= 1;
        MMPF_Scaler_SetEngine(MMP_TRUE, subpipe, &fitrange, &grabctl);
    }
    else {	//	1/4
        fitrange.usInWidth = m_scalerZoomInfo[subpipe].usInputWidth >> 2;
        fitrange.usInHeight = m_scalerZoomInfo[subpipe].usInputHeight >> 2;
        grabctl = m_scalerZoomInfo[subpipe].grabCtl;
        grabctl.usScaleN <<= 2;
        MMPF_Scaler_SetEngine(MMP_TRUE, subpipe, &fitrange, &grabctl);
    }	
    return MMP_ERR_NONE;    
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_CalculateZoomInfo
//  Description : the function calculate zoom N offset and grab offset
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_CalculateZoomInfo(MMPF_SCALER_PATH PathSel, MMP_UBYTE  *pbN_offset)
{
    if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMIN) {
        if (m_scalerZoomInfo[PathSel].grabCtl.usScaleN == 
             m_scalerZoomInfo[PathSel].usRangeMaxN) {
           // RTNA_DBG_Str(3, "S-S-P\r\n");
            gbZoomPathSel &= ~(1<<PathSel);
            return MMP_ERR_NONE;
        }
        else {
            // N (grabCtl.usScaleN) is increasing to Max N
            // (Max N - Min N) Maybe 1.>  2.=  3.< (usZoomRate)
            if (m_scalerZoomInfo[PathSel].usZoomRate > 
                 (m_scalerZoomInfo[PathSel].usRangeMaxN  - 
                  m_scalerZoomInfo[PathSel].grabCtl.usScaleN)) {
                *pbN_offset = m_scalerZoomInfo[PathSel].usRangeMaxN - 
                           m_scalerZoomInfo[PathSel].grabCtl.usScaleN;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleN = 
                m_scalerZoomInfo[PathSel].usRangeMaxN;
            } 
            else {
              *pbN_offset = m_scalerZoomInfo[PathSel].usZoomRate;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleN += 
                m_scalerZoomInfo[PathSel].usZoomRate;
            }
        }
    }
    else if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMOUT) {
        if (m_scalerZoomInfo[PathSel].grabCtl.usScaleN == 
             m_scalerZoomInfo[PathSel].usRangeMinN) {

            gbZoomPathSel &= ~(1<<PathSel);
            return MMP_ERR_NONE;
        }
        else {
           //N (grabCtl.usScaleN) is decreasing to Min N
            // (Max N - Min N) Maybe 1.>  2.=  3.< (usZoomRate)
            if (m_scalerZoomInfo[PathSel].usZoomRate > 
                 (m_scalerZoomInfo[PathSel].grabCtl.usScaleN - 
                  m_scalerZoomInfo[PathSel].usRangeMinN)) {
                *pbN_offset = m_scalerZoomInfo[PathSel].grabCtl.usScaleN - 
                           m_scalerZoomInfo[PathSel].usRangeMinN;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleN = 
                m_scalerZoomInfo[PathSel].usRangeMinN;
            }
            else {
                *pbN_offset = m_scalerZoomInfo[PathSel].usZoomRate;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleN -=
                m_scalerZoomInfo[PathSel].usZoomRate;
            }
        }
    }
    else if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMSTOP) {
            gbZoomPathSel &= ~(1<<PathSel);
            return MMP_ERR_NONE;
    }

    //Calculate new grabCtl
    if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMIN) {
        m_scalerZoomInfo[PathSel].grabCtl.usStartX += (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offset));
        m_scalerZoomInfo[PathSel].grabCtl.usEndX   += (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offset));
        m_scalerZoomInfo[PathSel].grabCtl.usStartY += (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offset));
        m_scalerZoomInfo[PathSel].grabCtl.usEndY   += (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offset));
    }
    else if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMOUT) {
        m_scalerZoomInfo[PathSel].grabCtl.usStartX -= (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offset));
        m_scalerZoomInfo[PathSel].grabCtl.usEndX   -= (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offset));
        m_scalerZoomInfo[PathSel].grabCtl.usStartY -= (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offset));
        m_scalerZoomInfo[PathSel].grabCtl.usEndY   -= (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offset));
    }

	
    return MMP_ERR_NONE;
}


MMP_ERR MMPF_Scaler_CalculateZoomInfoEx(MMPF_SCALER_PATH PathSel, MMP_UBYTE  *pbN_offsetX,MMP_UBYTE  *pbN_offsetY)
{
#if USB_LYNC_TEST_MODE

    if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMIN) {
        if ( (m_scalerZoomInfo[PathSel].grabCtl.usScaleXN ==  m_scalerZoomInfo[PathSel].usRangeMaxXN) &&
             (m_scalerZoomInfo[PathSel].grabCtl.usScaleYN ==  m_scalerZoomInfo[PathSel].usRangeMaxYN) ) {
           // RTNA_DBG_Str(3, "S-S-P\r\n");
            gbZoomPathSel &= ~(1<<PathSel);
            return MMP_ERR_NONE;
        }
        else {
            // N (grabCtl.usScaleN) is increasing to Max N
            // (Max N - Min N) Maybe 1.>  2.=  3.< (usZoomRate)
            if (m_scalerZoomInfo[PathSel].usZoomRate > 
                 (m_scalerZoomInfo[PathSel].usRangeMaxXN  - 
                  m_scalerZoomInfo[PathSel].grabCtl.usScaleXN)) {
                *pbN_offsetX = m_scalerZoomInfo[PathSel].usRangeMaxXN - 
                           m_scalerZoomInfo[PathSel].grabCtl.usScaleXN;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleXN = 
                m_scalerZoomInfo[PathSel].usRangeMaxXN;
            } 
            else {
                *pbN_offsetX = m_scalerZoomInfo[PathSel].usZoomRate;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleXN += 
                m_scalerZoomInfo[PathSel].usZoomRate;
            }
            if (m_scalerZoomInfo[PathSel].usZoomRate > 
                 (m_scalerZoomInfo[PathSel].usRangeMaxYN  - 
                  m_scalerZoomInfo[PathSel].grabCtl.usScaleYN)) {
                *pbN_offsetY = m_scalerZoomInfo[PathSel].usRangeMaxYN - 
                           m_scalerZoomInfo[PathSel].grabCtl.usScaleYN;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleYN = 
                m_scalerZoomInfo[PathSel].usRangeMaxYN;
            } 
            else {
                *pbN_offsetY = m_scalerZoomInfo[PathSel].usZoomRate;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleYN += 
                m_scalerZoomInfo[PathSel].usZoomRate;
            }
        }
    }
    else if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMOUT) {
        if ( (m_scalerZoomInfo[PathSel].grabCtl.usScaleXN == m_scalerZoomInfo[PathSel].usRangeMinXN)&&
             (m_scalerZoomInfo[PathSel].grabCtl.usScaleYN == m_scalerZoomInfo[PathSel].usRangeMinYN) ) {
            gbZoomPathSel &= ~(1<<PathSel);
            return MMP_ERR_NONE;
        }
        else {
            //N (grabCtl.usScaleN) is decreasing to Min N
            // (Max N - Min N) Maybe 1.>  2.=  3.< (usZoomRate)
            if (m_scalerZoomInfo[PathSel].usZoomRate > 
                 (m_scalerZoomInfo[PathSel].grabCtl.usScaleXN - 
                  m_scalerZoomInfo[PathSel].usRangeMinXN)) {
                *pbN_offsetX = m_scalerZoomInfo[PathSel].grabCtl.usScaleXN - 
                           m_scalerZoomInfo[PathSel].usRangeMinXN;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleXN = 
                m_scalerZoomInfo[PathSel].usRangeMinXN;
            }
            else {
                *pbN_offsetX = m_scalerZoomInfo[PathSel].usZoomRate;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleXN -=
                m_scalerZoomInfo[PathSel].usZoomRate;
            }

            if (m_scalerZoomInfo[PathSel].usZoomRate > 
                 (m_scalerZoomInfo[PathSel].grabCtl.usScaleYN - 
                  m_scalerZoomInfo[PathSel].usRangeMinYN)) {
                *pbN_offsetY = m_scalerZoomInfo[PathSel].grabCtl.usScaleYN - 
                           m_scalerZoomInfo[PathSel].usRangeMinYN;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleYN = 
                m_scalerZoomInfo[PathSel].usRangeMinYN;
            }
            else {
                *pbN_offsetY = m_scalerZoomInfo[PathSel].usZoomRate;
                
                m_scalerZoomInfo[PathSel].grabCtl.usScaleYN -=
                m_scalerZoomInfo[PathSel].usZoomRate;
            }

        }
    }
    else if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMSTOP) {
            gbZoomPathSel &= ~(1<<PathSel);
            return MMP_ERR_NONE;
    }

    //Calculate new grabCtl
    if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMIN) {
        m_scalerZoomInfo[PathSel].grabCtl.usStartX += (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offsetX));
        m_scalerZoomInfo[PathSel].grabCtl.usEndX   += (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offsetX));
        m_scalerZoomInfo[PathSel].grabCtl.usStartY += (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offsetY));
        m_scalerZoomInfo[PathSel].grabCtl.usEndY   += (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offsetY));
    }
    else if (m_scalerZoomInfo[PathSel].direction == MMPF_SCALER_ZOOMOUT) {
        m_scalerZoomInfo[PathSel].grabCtl.usStartX -= (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offsetX));
        m_scalerZoomInfo[PathSel].grabCtl.usEndX   -= (m_scalerZoomInfo[PathSel].usStepX * (*pbN_offsetX));
        m_scalerZoomInfo[PathSel].grabCtl.usStartY -= (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offsetY));
        m_scalerZoomInfo[PathSel].grabCtl.usEndY   -= (m_scalerZoomInfo[PathSel].usStepY * (*pbN_offsetY));
    }

	dbg_printf(3,"#offX = %d,offY = %d\r\n",*pbN_offsetX,*pbN_offsetY);
#endif
    return MMP_ERR_NONE;
}


MMP_ERR MMPF_Scaler_SetPanTiltOP(void)
{
    AITPS_VIF               pVIF = AITC_BASE_VIF;
    MMPF_SCALER_GRABCONTROL    tmpgrabCtl,tmpgrabInCtl;  
    MMPF_SCALER_PATH    mainpipe, subpipe;
    MMP_UBYTE   vid = VIF_CTL;
    MMPF_SCALER_FIT_RANGE   fitrange;
    if( gbPanTiltPathSel == 0 ){  
        MMPF_HIF_ClearCmdStatus(IMAGE_UNDER_PANTILT);
        //RTNA_DBG_Str(3, "Pan...End\r\n");
        if(gbZoomPathSel==0 ) {
        // sean@2010_12_08
            pVIF->VIF_INT_CPU_EN[vid] &= ~(VIF_INT_FRM_END);
        }
        return MMP_ERR_NONE;
    }
    else {
        //dbg_printf(3,"SET PANTILT OP\r\n");

        if(  gbPanTiltPathSel & ( 1 << MMPF_SCALER_PATH_0 ) ) {
            
            if(m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].bGrabSrc==0) {
                tmpgrabCtl  = m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].grabCtl ;
                MMPF_Scaler_SetEngine(MMP_TRUE, MMPF_SCALER_PATH_0, 0, &tmpgrabCtl);
            }       
            else {
                tmpgrabInCtl = m_scalerPanTiltInfo[MMPF_SCALER_PATH_0].grabCtlIn ; 
                MMPF_Scaler_AccessGrabArea(MMP_TRUE,MMPF_SCALER_PATH_0,&tmpgrabInCtl,0);
            }
        }
        if(  gbPanTiltPathSel & ( 1 << MMPF_SCALER_PATH_1 ) ) {
            if(m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].bGrabSrc==0) {
                tmpgrabCtl = m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].grabCtl ;
                MMPF_Scaler_SetEngine(MMP_TRUE, MMPF_SCALER_PATH_1, 0, &tmpgrabCtl);
            }       
            else {
                tmpgrabInCtl = m_scalerPanTiltInfo[MMPF_SCALER_PATH_1].grabCtlIn ; 
                MMPF_Scaler_AccessGrabArea(MMP_TRUE,MMPF_SCALER_PATH_1,&tmpgrabInCtl,0);
            }
        }
        if(  gbPanTiltPathSel & ( 1 << MMPF_SCALER_PATH_2 ) ) {
            if(m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].bGrabSrc==0) {
                tmpgrabCtl = m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].grabCtl ;
                MMPF_Scaler_SetEngine(MMP_TRUE, MMPF_SCALER_PATH_2, 0, &tmpgrabCtl);
            }       
            else {
                tmpgrabInCtl = m_scalerPanTiltInfo[MMPF_SCALER_PATH_2].grabCtlIn ; 
                MMPF_Scaler_AccessGrabArea(MMP_TRUE,MMPF_SCALER_PATH_2,&tmpgrabInCtl,0);
            }
        }
        gbPanTiltPathSel = 0 ;
    }
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_SetZoomOP
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_SetZoomOP(void)
{
	AITPS_VIF               pVIF = AITC_BASE_VIF;

    MMP_UBYTE               N_offset=0,N_offsetX=0,N_offsetY=0;
    MMPF_SCALER_FIT_RANGE   fitrange;
    #if(DSC_SCALE_GRAB_IN_RAW_EN)
    MMP_BOOL                bRawPathPreview;
    MMP_BOOL                bRawStoreDownsample;
    #endif
    MMPF_SCALER_GRABCONTROL    tmpgrabCtl;  
    MMPF_SCALER_PATH    mainpipe, subpipe;
    MMP_LONG            pan_off,tilt_off;
    MMP_UBYTE           vid = VIF_CTL;
    
	pVIF->VIF_INT_CPU_SR[vid] = ( VIF_INT_FRM_END  );

	#if(DSC_SCALE_GRAB_IN_RAW_EN)
    MMPF_Sensor_GetParam(MMPF_SENSOR_RAWPATH_PREVIEW_ENABLE  , &bRawPathPreview);
    MMPF_Sensor_GetParam(MMPF_SENSOR_RAWPATH_STORE_DOWNSAMPLE, &bRawStoreDownsample);
    #endif
#if SCALER_ZOOM_LEVEL!=BEST_LEVEL
    /*Set Scaler pipe0*/
    if(gbZoomPathSel &  (1<<MMPF_SCALER_PATH_0)){
        if(m_scalerZoomInfo[MMPF_SCALER_PATH_0].usBaseN) {
	        MMPF_Scaler_CalculateZoomInfo(MMPF_SCALER_PATH_0, &N_offset);
	    } else {
	        MMPF_Scaler_CalculateZoomInfoEx(MMPF_SCALER_PATH_0, &N_offsetX,&N_offsetY);
	    }
    }

    /*Set Scaler pipe1*/
    if(gbZoomPathSel &  (1<<MMPF_SCALER_PATH_1)){
    
        if(m_scalerZoomInfo[MMPF_SCALER_PATH_1].usBaseN) {
	        MMPF_Scaler_CalculateZoomInfo(MMPF_SCALER_PATH_1, &N_offset);
	    } else {
	        MMPF_Scaler_CalculateZoomInfoEx(MMPF_SCALER_PATH_1, &N_offsetX,&N_offsetY);
	    }
	    
    }

#if(CHIP==VSN_V2)||(CHIP==VSN_V3)
    /*Set Scaler pipe1*/
    if(gbZoomPathSel &  (1<<MMPF_SCALER_PATH_2)){
    
        if(m_scalerZoomInfo[MMPF_SCALER_PATH_2].usBaseN) {
	        MMPF_Scaler_CalculateZoomInfo(MMPF_SCALER_PATH_2, &N_offset);
	    } else {
	        MMPF_Scaler_CalculateZoomInfoEx(MMPF_SCALER_PATH_2, &N_offsetX,&N_offsetY);
	    }
	    
    }
#endif

#endif

    /*No zoom, stop VIF interrupt*/
    if(gbZoomPathSel == 0){     
        if(gbPanTiltPathSel==0) {
            pVIF->VIF_INT_CPU_EN[vid] &= ~(VIF_INT_FRM_END);
    	}
        MMPF_HIF_ClearCmdStatus(IMAGE_UNDER_ZOOM);
        return MMP_ERR_NONE;
    }
    /*adjust the LPF, scaler pipe0, pipe1 if enable both pipe*/
    else if (gbZoomPathSel  ==  ((1<<MMPF_SCALER_PATH_1)|(1<<MMPF_SCALER_PATH_0)) ) {
        MMP_BOOL grapath;// = MMP_FALSE;
        MMPF_SCALER_ZOOM_INFO *scalinfo;
        MMPF_SCALER_SOURCE scalsrc_to ;
        MMP_BYTE pipe;
        
        STREAM_CFG *cur_stream;
        MMP_UBYTE   grapath_en;       
        
	    for (pipe = MMPF_SCALER_PATH_0; pipe < MMPF_SCALER_PATH_2; pipe++){
	        grapath = MMP_FALSE;
	        
	        cur_stream = usb_get_cur_image_pipe(pipe);
	        grapath_en = MMPF_Video_IsLoop2GRA(cur_stream);
	        scalinfo = (MMPF_SCALER_ZOOM_INFO *)MMPF_Scaler_GetCurInfo(pipe);
	        
	        if(grapath_en) {
	            if(scalinfo->grabCtl.usScaleN > scalinfo->grabCtl.usScaleM) {
	                        grapath = MMP_TRUE ;
	                        scalsrc_to = MMPF_SCALER_SOURCE_GRA ;
	            } else {
	                MMPF_SCALER_SOURCE scalsrc = MMPF_Scaler_GetPath(pipe);
	                if(scalsrc==MMPF_SCALER_SOURCE_GRA) {
	                    scalsrc_to = MMPF_SCALER_SOURCE_ISP ;    
	                    grapath = MMP_TRUE ;
	                } else {
	                    MMPF_Scaler_SetZoomSinglePipe(pipe);
	                }
	            }
	        } else {
	            MMPF_Scaler_SetZoomSinglePipe(pipe);
	        }
	        
	        if(grapath) {
	           // dbg_printf(3,"[ZOOMOP] : scalsrc : %d\r\n",scalsrc_to);
	            MMPF_SCALER_SWITCH_PATH *scalpath = MMPF_Scaler_GetPathInfo(pipe);//
	            if(scalpath->flowctl==SCAL_FLOW_NONE) {
	                MMPF_Scaler_SwitchPath(MMP_TRUE, pipe,scalsrc_to,MMP_FALSE);
	            }
	            else {
	                dbg_printf(3,"<NRS(%d):%d>\r\n",pipe,scalpath->flowctl);
	            }
	        }
	        else {
	#if SCALER_ZOOM_LEVEL==BEST_LEVEL        
	            MMPF_Scaler_DisableZoom(pipe);
	#endif      
	        }
		}

    }
    /*only one pipe zoom*/        
    else {  
        MMP_ERR err ;
        MMP_BOOL grapath = MMP_FALSE;
        MMPF_SCALER_SOURCE scalsrc_to ;
        MMPF_SCALER_ZOOM_INFO *scalinfo;
        
        STREAM_CFG *cur_stream;
        MMP_UBYTE   grapath_en;// = MMPF_Video_IsLoop2GRA(cur_stream);
        mainpipe = gbZoomPathSel>>1;
        cur_stream = usb_get_cur_image_pipe(mainpipe);
        grapath_en = MMPF_Video_IsLoop2GRA(cur_stream);
        
        //dbg_printf(3, "VS:pipe = %d, PathSel = %d, grapath_en = %d\r\n", mainpipe, gbZoomPathSel, grapath_en);
        
        scalinfo = (MMPF_SCALER_ZOOM_INFO *)MMPF_Scaler_GetCurInfo(mainpipe);
        if(grapath_en) {
            if(scalinfo->grabCtl.usScaleN > scalinfo->grabCtl.usScaleM) {
                grapath = MMP_TRUE ;
                scalsrc_to = MMPF_SCALER_SOURCE_GRA ;
            } else {
                MMPF_SCALER_SOURCE scalsrc = MMPF_Scaler_GetPath(mainpipe);
                if(scalsrc==MMPF_SCALER_SOURCE_GRA) {
                    scalsrc_to = MMPF_SCALER_SOURCE_ISP ;    
                    grapath = MMP_TRUE ;
                } else {
                    MMPF_Scaler_SetZoomSinglePipe(mainpipe);
                }
            }
        } else {
            MMPF_Scaler_SetZoomSinglePipe(mainpipe);
        }
        
        if(grapath) {
           // dbg_printf(3,"[ZOOMOP] : scalsrc : %d\r\n",scalsrc_to);
            MMPF_SCALER_SWITCH_PATH *scalpath = MMPF_Scaler_GetPathInfo(mainpipe);//
            //dbg_printf(3,"VS.FCTL:%d\r\n",scalpath->flowctl);
            if(scalpath->flowctl==SCAL_FLOW_NONE) {
                MMPF_Scaler_SwitchPath(MMP_TRUE, mainpipe,scalsrc_to,MMP_FALSE);
            }
            else {
                dbg_printf(3,"<-NRTS:%d>\r\n",scalpath->flowctl);
            }
        }
        else {
#if SCALER_ZOOM_LEVEL==BEST_LEVEL 
            MMPF_Scaler_DisableZoom(mainpipe);
#endif      
        }
    }
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Scaler_SetZoomSinglePipe(MMPF_SCALER_PATH mainpipe)
{
    MMP_BOOL            grabsrc ;       
    MMP_LONG            pan_off,tilt_off;
    MMPF_SCALER_GRABCONTROL    tmpgrabCtl;  
    MMPF_SCALER_FIT_RANGE   fitrange;
    fitrange.usInWidth = m_scalerZoomInfo[mainpipe].usInputWidth;
    fitrange.usInHeight = m_scalerZoomInfo[mainpipe].usInputHeight;
    
    tmpgrabCtl = m_scalerZoomInfo[mainpipe].grabCtl;
    if( tmpgrabCtl.usScaleN > tmpgrabCtl.usScaleM) {
        return MMP_SCALER_BY_GRAPHIC ;
    }    

    #if(DSC_SCALE_GRAB_IN_RAW_EN)
    if ( bRawPathPreview && bRawStoreDownsample ) {
        MMPF_RAWPROC_SetZoomOffset(m_scalerZoomInfo[mainpipe].direction,(m_scalerZoomInfo[mainpipe].usStepX * N_offset));
    }
    if ( bRawPathPreview ) {
        MMPF_RAWPROC_SetMainPipeZoomOP(bRawStoreDownsample, m_scalerZoomInfo,mainpipe,N_offset,&fitrange,&tmpgrabCtl);
    }
    #endif
    
    #if SUPPORT_DIGITAL_PAN
    grabsrc = MMPF_Scaler_IfGrabInput(mainpipe,m_scalerZoomInfo[mainpipe].usSensorWidth,tmpgrabCtl.usScaleN,tmpgrabCtl.usScaleM);
    if(grabsrc==0) {
        // sean@2010_09_20, canculate pantilt
        if(MMPF_Scaler_SetPanTiltParams(mainpipe,0,0,0,&pan_off,&tilt_off)==MMP_ERR_NONE) {
            if(pan_off || tilt_off) {
                tmpgrabCtl = m_scalerPanTiltInfo[mainpipe].grabCtl;
            } else {
                if( (m_scalerPanTiltInfo[mainpipe].usPanEnd==2 ) || (m_scalerPanTiltInfo[mainpipe].usTiltEnd==2)) {
                    tmpgrabCtl = m_scalerPanTiltInfo[mainpipe].grabCtl; 
                }
            }
            
        }
    }
    #endif
	dbg_printf(0,"fitrange %d , %d , %d , %d \r\n",fitrange.usInWidth,fitrange.usInHeight,fitrange.usOutWidth,fitrange.usOutHeight);
	//dbg_printf(0,"tmpgrabCtl %d , %d , %d , %d \r\n",tmpgrabCtl->usInWidth,tmpgrabCtl->usInHeight,tmpgrabCtl->usOutWidth,tmpgrabCtl->usOutHeight);
    MMPF_Scaler_SetEngine(MMP_TRUE, mainpipe, &fitrange, &tmpgrabCtl);
    MMPF_Scaler_SetLPF(mainpipe, &fitrange, &tmpgrabCtl);
    // Grab source, adjust pan/tilt again
    #if SUPPORT_DIGITAL_PAN
    if(grabsrc) {
        if(MMPF_Scaler_SetPanTiltParams(mainpipe,0,0,0,&pan_off,&tilt_off)==MMP_ERR_NONE) {
            MMPF_SCALER_PANTILT_INFO *paninfo = MMPF_Scaler_GetCurPanTiltInfo(mainpipe) ;
            
            if(pan_off || tilt_off) {
                MMPF_Scaler_AccessGrabArea(MMP_TRUE,mainpipe,&paninfo->grabCtlIn,0);
            } 
            else {
                if( (paninfo->usPanEnd==2 ) || (paninfo->usTiltEnd==2)) {
                    MMPF_Scaler_AccessGrabArea(MMP_TRUE,mainpipe,&paninfo->grabCtlIn,0);
                }
            }
        }
    }
    #endif
    
    #if(DSC_SCALE_GRAB_IN_RAW_EN)
    if ( bRawPathPreview ) {
        MMPF_RAWPROC_CalcRawFetchTiming(mainpipe, MMPF_RAWPROC_DISTRIBUTION);
    }
    #endif
    return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_GetCurInfo
//  Description :
//------------------------------------------------------------------------------
MMPF_SCALER_ZOOM_INFO* MMPF_Scaler_GetCurInfo(MMPF_SCALER_PATH pathsel)
{
    MMPF_SCALER_GRABCONTROL *grabctl = &m_scalerZoomInfo[pathsel].grabCtl;
    //dbg_printf(3,"grabctl(%d) : %x,N/M=%d / %d\r\n",pathsel,grabctl,grabctl->usScaleN,grabctl->usScaleM);
    return &m_scalerZoomInfo[pathsel];
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Scaler_GetZoomInfo
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_GetZoomInfo(MMPF_SCALER_PATH pathsel, MMP_USHORT *usBaseN, MMP_USHORT *usCurrentN)
{
    *usBaseN = m_scalerZoomInfo[pathsel].usBaseN;
    *usCurrentN = m_scalerZoomInfo[pathsel].grabCtl.usScaleN;
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Scaler_SetZoomSensorInput(MMPF_SCALER_PATH pathsel,MMP_USHORT sensor_w,MMP_USHORT sensor_h)
{
    m_scalerZoomInfo[pathsel].usSensorWidth = sensor_w ;
    m_scalerZoomInfo[pathsel].usSensorHeight = sensor_h ;
 
    return  MMP_ERR_NONE;  
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_GetZoomOutPos
//  Description : in capture mode, sensor resolution is different with preview mode.
//               It must set the scale according to capture mode resolution
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_GetZoomOutPos(MMPF_SCALER_PATH pathsel, MMPF_SCALER_GRABCONTROL *SetgrabCtl)
{
    SetgrabCtl->usStartX = m_scalerZoomInfo[pathsel].grabCtl.usStartX;
    SetgrabCtl->usEndX = m_scalerZoomInfo[pathsel].grabCtl.usEndX;
    SetgrabCtl->usStartY = m_scalerZoomInfo[pathsel].grabCtl.usStartY;
    SetgrabCtl->usEndY = m_scalerZoomInfo[pathsel].grabCtl.usEndY;
    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_GetScaleUpHBlanking
//  Description : get the exact Hblanking value when scaling up
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_GetScaleUpHBlanking(MMPF_SCALER_PATH pathsel, MMP_BYTE bPixelDelay, MMP_ULONG *pHBlanking)
{
	AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    AIT_REG_B   *OprScaleCtl;
    MMP_ULONG ScalerInWidth, tmp0, tmp1;
    MMP_USHORT ScaleHM, ScaleHN, ScaleVM, ScaleVN;

    if (pathsel == MMPF_SCALER_PATH_0) {
        OprScaleCtl = &(pSCAL->SCAL_SCAL_0_CTL);

        ScalerInWidth = pSCAL->SCAL_GRAB_SCAL_0_H_ED - pSCAL->SCAL_GRAB_SCAL_0_H_ST + 1;

        ScaleHN = pSCAL->SCAL_SCAL_0_H_N;
        ScaleHM = pSCAL->SCAL_SCAL_0_H_M;
        ScaleVN = pSCAL->SCAL_SCAL_0_V_N;
        ScaleVM = pSCAL->SCAL_SCAL_0_V_M;
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        OprScaleCtl = &(pSCAL->SCAL_SCAL_1_CTL);

        ScalerInWidth = pSCAL->SCAL_GRAB_SCAL_1_H_ED - pSCAL->SCAL_GRAB_SCAL_1_H_ST + 1;

        ScaleHN = pSCAL->SCAL_SCAL_1_H_N;
        ScaleHM = pSCAL->SCAL_SCAL_1_H_M;
        ScaleVN = pSCAL->SCAL_SCAL_1_V_N;
        ScaleVM = pSCAL->SCAL_SCAL_1_V_M;
    }
    else {
        OprScaleCtl = &(pSCAL->SCAL_SCAL_2_CTL);

        ScalerInWidth = pSCAL->SCAL_GRAB_SCAL_2_H_ED - pSCAL->SCAL_GRAB_SCAL_2_H_ST + 1;

        ScaleHN = pSCAL->SCAL_SCAL_2_H_N;
        ScaleHM = pSCAL->SCAL_SCAL_2_H_M;
        ScaleVN = pSCAL->SCAL_SCAL_2_V_N;
        ScaleVM = pSCAL->SCAL_SCAL_2_V_M;
    }
    
    if((ScaleHN > ScaleHM) || (ScaleVN > ScaleVM)){
        if (0) {//FIXME vsn_v2( *OprScaleCtl & SCAL_SCAL_DBL_FIFO) {
            tmp1 = ( ((MMP_ULONG)(ScaleHN * ScaleVN)/ (ScaleHM*ScaleVM)) - (MMP_ULONG)(bPixelDelay+1));
            tmp1 ++;
            tmp1 = ScalerInWidth *  tmp1;
            
            tmp0 = ScalerInWidth *( ((MMP_ULONG)(ScaleHN + ScaleHM -1)/ScaleHM)* ((MMP_ULONG)(ScaleVN + ScaleVM -1)/ScaleVM) - (MMP_ULONG)(bPixelDelay+1)) + 64 + 16;     //16 is tolerance 
            
            *pHBlanking = (tmp0 > tmp1) ?  tmp0 : tmp1;     
        }
        else{
            *pHBlanking = ScalerInWidth *((MMP_ULONG)(ScaleHN + ScaleHM -1)/ScaleHM)* ((MMP_ULONG)(ScaleVN + ScaleVM -1)/ScaleVM) + 64 + 16;     //16 is tolerance 
        }
    }
    else{
        *pHBlanking = 16;          // ISP line delay must have 3 pixel at least. 0x10 is tolerance 
    }

    return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_EnableDNSMP
//  Description : When change sensor resolution, enable downsample function before frame enter scale
//------------------------------------------------------------------------------
MMP_ERR MMPF_Scaler_EnableDNSMP(MMPF_SCALER_PATH pathsel, MMP_BYTE bDownSample)
{
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;

    if (pathsel == MMPF_SCALER_PATH_0) {
    	pSCAL->SCAL_DNSAMP_SCAL_0_H = bDownSample;
    	pSCAL->SCAL_DNSAMP_SCAL_0_V = bDownSample;
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
    	pSCAL->SCAL_DNSAMP_SCAL_1_H = bDownSample;
    	pSCAL->SCAL_DNSAMP_SCAL_1_V = bDownSample;
    }
    else {
    	pSCAL->SCAL_DNSAMP_SCAL_2_H = bDownSample;
    	pSCAL->SCAL_DNSAMP_SCAL_2_V = bDownSample;
    }

    return MMP_ERR_NONE;
}


/** @brief The function calculates the best grab parameter.

The function calculates the best grab parameter..
@param[in] fitrange the fit range parameters
@param[out] grabctl the grab control parameters
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Scaler_GetBestFitScale(MMPF_SCALER_FIT_RANGE *fitrange,
                                               MMPF_SCALER_GRABCONTROL *grabctl)
{
    MMP_USHORT  x_scale, y_scale,scaler_ref_h,scaler_real_h;
#if SENSOR_IN_H > SENSOR_16_9_H
    scaler_ref_h = gsScalerRefInH;
    scaler_real_h = gsScalerRefOutH[1];
#else
    scaler_ref_h = fitrange->usInHeight;
    scaler_real_h = fitrange->usOutHeight ;
#endif
    // For 800x600 , 160x120 H264
#if SENSOR_IN_H > SENSOR_16_9_H
    if(MMPF_Scaler_GetCurInfo(1)->usSensorHeight==SENSOR_16_9_H) {
        scaler_real_h = fitrange->usOutHeight;
    }
#endif
    
    grabctl->usScaleM = fitrange->usFitResol;

    y_scale = (fitrange->usOutHeight * grabctl->usScaleM + fitrange->usInHeight - 1) / fitrange->usInHeight;
    x_scale = (fitrange->usOutWidth * grabctl->usScaleM + fitrange->usInWidth - 1) / fitrange->usInWidth;

	if ((fitrange->usOutHeight > fitrange->usInHeight) || ( fitrange->usOutWidth > fitrange->usInWidth)) {
		// scale up case
		if (fitrange->fitmode == MMPF_SCALER_FITMODE_OUT) {
	    	while ((((fitrange->usInHeight - 1) * y_scale + grabctl->usScaleM - 1) / grabctl->usScaleM)
	    			< fitrange->usOutHeight)
    			y_scale++;
	    	while ((((fitrange->usInWidth - 1) * x_scale + grabctl->usScaleM - 1) / grabctl->usScaleM)
	    			< fitrange->usOutWidth)
    			x_scale++;
		}
		else {
	    	while ((((fitrange->usInHeight - 1) * y_scale + grabctl->usScaleM - 1) / grabctl->usScaleM)
	    			> fitrange->usOutHeight)
    			y_scale--;
	    	while ((((fitrange->usInWidth - 1) * x_scale + grabctl->usScaleM - 1) / grabctl->usScaleM)
	    			> fitrange->usOutWidth)
    			x_scale--;
		}	    		
	}
	else {
		// scale down case
		if (fitrange->fitmode == MMPF_SCALER_FITMODE_OUT) {
		    while (((fitrange->usInHeight) * y_scale) < (fitrange->usOutHeight * grabctl->usScaleM))
    			y_scale++;
		    while (((fitrange->usInWidth) * x_scale) < (fitrange->usOutWidth * grabctl->usScaleM))
	    		x_scale++;
		}
		else {
	    	while (((fitrange->usInHeight) * y_scale) > (fitrange->usOutHeight * grabctl->usScaleM))
    			y_scale--;
	    	while (((fitrange->usInWidth) * x_scale) > (fitrange->usOutWidth * grabctl->usScaleM))
    			x_scale--;
		}	    		
	}
	
    if (fitrange->fitmode == MMPF_SCALER_FITMODE_OUT) {
		if (x_scale > y_scale) {
       	   	grabctl->usScaleN = x_scale;
		}
       	else {
       	   	grabctl->usScaleN = y_scale;
       	}
   	}
	else {
       	if (x_scale > y_scale) {
   		 	grabctl->usScaleN = y_scale;
       	}
        else {
   	        grabctl->usScaleN = x_scale;
       	}
   	}

    if (x_scale > grabctl->usScaleM || y_scale > grabctl->usScaleM) {
		if (fitrange->fitmode == MMPF_SCALER_FITMODE_OUT) {
		    if(fitrange->usInHeight < scaler_ref_h) {    
    	    	grabctl->usStartX = ((fitrange->usInWidth - 1) * grabctl->usScaleN / grabctl->usScaleM
    		    	                    - fitrange->usOutWidth) / 2 + 1;
    	    	grabctl->usStartY = (( fitrange->usInHeight - 1) * grabctl->usScaleN / grabctl->usScaleM
    								    - fitrange->usOutHeight) / 2 + 1;
    		    grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
    		    grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
		    }
		    else {
    	    	grabctl->usStartX = ((fitrange->usInWidth - 1) * grabctl->usScaleN / grabctl->usScaleM
    		    	                    - fitrange->usOutWidth) / 2 + 1;
    	    	grabctl->usStartY = (( scaler_ref_h - 1) * grabctl->usScaleN / grabctl->usScaleM
    								    - fitrange->usOutHeight) / 2 + 1;
    		    grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
    		    grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
		    
		    }
		}
		else {
    	    grabctl->usStartX = 1;
       		grabctl->usStartY = 1;
		    grabctl->usEndX = ((fitrange->usInWidth - 1) * grabctl->usScaleN + grabctl->usScaleM - 1) / grabctl->usScaleM;
	    	grabctl->usEndY = ((fitrange->usInHeight - 1) * grabctl->usScaleN + grabctl->usScaleM - 1) / grabctl->usScaleM;
		}				    
	}
    else {
		if (fitrange->fitmode == MMPF_SCALER_FITMODE_OUT) {
        	grabctl->usStartX = (fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM
   	                            - fitrange->usOutWidth) / 2 + 1;
        	grabctl->usStartY = (scaler_ref_h * grabctl->usScaleN / grabctl->usScaleM
                                - scaler_real_h ) / 2 + 1;
		    grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
    		grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
		}
		else {
	        grabctl->usStartX = 1;
       		grabctl->usStartY = 1;
		    grabctl->usEndX = fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM;
	    	grabctl->usEndY = fitrange->usInHeight * grabctl->usScaleN / grabctl->usScaleM;
		}	    	
	}

    return  MMP_ERR_NONE;    
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Scaler_SetEnable
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function enables or disables an icon on the display device

The function enables or disables icon with the specified icon ID on the display device. It is implemented
by programming Icon Controller registers.

  @param[in] usPipeID the IBC ID
  @param[in] bEnable enable or disable IBC
  @return It reports the status of the operation.
*/
MMP_ERR MMPF_Scaler_SetEnable(MMPF_SCALER_PATH pathsel, MMP_BOOL bEnable)
{
    AITPS_SCAL  pSCAL = AITC_BASE_SCAL;
    AIT_REG_B   *OprScalCtl;

	//MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);
    
    if (pathsel == MMPF_SCALER_PATH_0) {
        OprScalCtl = &(pSCAL->SCAL_SCAL_0_CTL);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        OprScalCtl = &(pSCAL->SCAL_SCAL_1_CTL);
    }
    else {
        OprScalCtl = &(pSCAL->SCAL_SCAL_2_CTL);
    }

    if (bEnable == MMP_TRUE) {
        (*OprScalCtl) |= SCAL_SCAL_PATH_EN;
    }
    else {
        (*OprScalCtl) &= ~SCAL_SCAL_PATH_EN;
    }

	//MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_FALSE);

	return	MMP_ERR_NONE;	
}

/** @brief The function set the LPF coefficient according to the grab control
		
The function calculates the best grab parameter..
@param[in] pathsel the scaling path in Scaler
@param[in] grabctl the grab control parameters
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE source, MMPF_SCALER_PATH pathsel)
{
    AITPS_SCAL  pSCAL = AITC_BASE_SCAL;

	//MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);

    switch (source) {
    case MMPF_SCALER_SOURCE_ISP:
        if (pathsel == MMPF_SCALER_PATH_0) {
            //ctl_reg &= ~(SCAL_LPF_PATH_SEL_MASK | SCAL_SCAL0_PATH_SEL_MASK);
            //ctl_reg |= (SCAL_ISP_2_LPF | SCAL_LPF_2_SCAL0);
            pSCAL->SCAL_PATH_CTL &= ~SCAL_GRA_2_SCAL0;
        }            
        else if (pathsel == MMPF_SCALER_PATH_1) {
            //ctl_reg &= ~(SCAL_LPF1_PATH_SEL_MASK | SCAL_SCAL1_PATH_SEL_MASK);
            //ctl_reg |= (SCAL_ISP_2_LPF1 | SCAL_LPF_2_SCAL1);
            pSCAL->SCAL_PATH_CTL &= ~SCAL_GRA_2_SCAL1;
        }
        else {
            pSCAL->SCAL_PATH_2_CTL &= ~SCAL_GRA_2_SCAL2;
        }
        break;
    #if (CHIP == P_V2)
    case MMPF_SCALER_SOURCE_JPG:
        if (pathsel == MMPF_SCALER_PATH_0) {
            ctl_reg &= ~(SCAL_SCAL0_PATH_SEL_MASK|SCAL_LPF_PATH_SEL_MASK);
            ctl_reg |= SCAL_JPG_2_LPF;
        }
        else if (pathsel == MMPF_SCALER_PATH_1) {
            ctl_reg &= ~(SCAL_SCAL1_PATH_SEL_MASK|SCAL_LPF1_PATH_SEL_MASK);
            ctl_reg |= SCAL_JPG_2_LPF1;
        }
		break;
    #endif
    case MMPF_SCALER_SOURCE_GRA:
        if (pathsel == MMPF_SCALER_PATH_0) {
            //ctl_reg &= ~(SCAL_SCAL0_PATH_SEL_MASK|SCAL_LPF_PATH_SEL_MASK);
            //ctl_reg |= SCAL_GRA_2_LPF;
            pSCAL->SCAL_PATH_CTL |= SCAL_GRA_2_SCAL0;
        }
        else if (pathsel == MMPF_SCALER_PATH_1) {
            //ctl_reg &= ~(SCAL_SCAL1_PATH_SEL_MASK|SCAL_LPF1_PATH_SEL_MASK);
            //ctl_reg |= SCAL_GRA_2_LPF1;
            pSCAL->SCAL_PATH_CTL |= SCAL_GRA_2_SCAL1;
        }
        else {
            pSCAL->SCAL_PATH_2_CTL |= SCAL_GRA_2_SCAL2;
        }
        break;        
    }

	//MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_FALSE);
    //dbg_printf(0,"Path[%d] %s->SCAL\r\n",pathsel,(source==MMPF_SCALER_SOURCE_ISP)?"ISP":"GRA");

    return  MMP_ERR_NONE;    
}


MMPF_SCALER_SOURCE MMPF_Scaler_GetPath(MMPF_SCALER_PATH pathsel)
{
    AITPS_SCAL  pSCAL = AITC_BASE_SCAL;
    MMP_UBYTE scale_src[] = { SCAL_GRA_2_SCAL0,SCAL_GRA_2_SCAL1,SCAL_GRA_2_SCAL2 } ;
    if(pathsel==MMPF_SCALER_PATH_0) {
        if(pSCAL->SCAL_PATH_CTL & SCAL_GRA_2_SCAL0) {
            return MMPF_SCALER_SOURCE_GRA ;
        }
    }  
    else if(pathsel==MMPF_SCALER_PATH_1) {
        if(pSCAL->SCAL_PATH_CTL & SCAL_GRA_2_SCAL1) {
            return MMPF_SCALER_SOURCE_GRA ;
        }
    }
    else {
        if(pSCAL->SCAL_PATH_2_CTL & SCAL_GRA_2_SCAL2) {
            return MMPF_SCALER_SOURCE_GRA ;
        }
    }    
    return MMPF_SCALER_SOURCE_ISP ; 
}

/*
set = TRUE, prepare scaler path switch
set = FALSE, update scaler path
*/
MMP_ERR MMPF_Scaler_SwitchPath(MMP_BOOL set,MMPF_SCALER_PATH pathsel,MMPF_SCALER_SOURCE src,MMP_BOOL wait_end)
{
extern MMP_BOOL gbGRAEn[] ;

//extern MMP_USHORT	gsZoomParam,ZOOM_RATIO_BASE;
    MMP_ULONG cpu_sr;
    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(pathsel);
    MMP_UBYTE pipe_cfg = CUR_PIPE_CFG(pathsel,cur_pipe->pipe_cfg) ;
    MMPF_SCALER_GRABCONTROL grab ;
    if(pipe_cfg==PIPE_CFG_H264) {
        //dbg_printf(3,"< H264 >\r\n");
    }
    else if(pipe_cfg==PIPE_CFG_MJPEG) {
        //dbg_printf(3,"< JPG >\r\n");
    }
    else if(pipe_cfg==PIPE_CFG_YUY2) {
        //dbg_printf(3,"< YUY2 >\r\n");
    }
    if( (!set) &&(m_scalerSwitch[pathsel].set) ) {
        MMPF_SCALER_SOURCE cur_src = MMPF_Scaler_GetPath(m_scalerSwitch[pathsel].path) ;
        if(cur_src!=m_scalerSwitch[pathsel].src) {
            MMP_BOOL pipe_store_en[MMPF_IBC_PIPE_MAX] = { 0, 0 ,0 } ;
            //MMPF_Scaler_SetPath(m_scalerSwitch.src, m_scalerSwitch.path);
            
            pipe_store_en[GRA_SRC_PIPE] = TRUE ;
            dbg_printf(GRA_DBG_LEVEL, "I:try pipe%d->%s!\r\n",m_scalerSwitch[pathsel].path,(m_scalerSwitch[pathsel].src==MMPF_SCALER_SOURCE_GRA)?"GRA":"ISP" );
            if(1/*pipe_cfg!=PIPE_CFG_YUY2*/) {
                //grab = m_scalerZoomInfo[pathsel].grabCtl;
                if(m_scalerSwitch[pathsel].flowctl==SCAL_FLOW_NONE) {
                    dbg_printf(GRA_DBG_LEVEL,">I:wait ENC end\r\n");
                    if(m_scalerSwitch[pathsel].forceswitch==MMP_FALSE) {
                        m_scalerSwitch[pathsel].flowctl = SCAL_WAIT_ENC_END ;
                    } else {
                        m_scalerSwitch[pathsel].flowctl = SCAL_ENC_END;
                        m_scalerSwitch[pathsel].forceswitch = MMP_FALSE ;
                    }
               }
                if((m_scalerSwitch[pathsel].src==MMPF_SCALER_SOURCE_GRA)&&(m_scalerSwitch[pathsel].flowctl == SCAL_ENC_END )) {
                    dbg_printf(GRA_DBG_LEVEL,">I:ENC end%d->GRA...\r\n",pathsel);
                    MMPF_Scaler_SetPath(m_scalerSwitch[pathsel].src, m_scalerSwitch[pathsel].path);
                    m_scalerSwitch[pathsel].flowctl= SCAL_FLOW_NONE ;
                    //return MMP_ERR_NONE;
                } 
                else if((m_scalerSwitch[pathsel].src==MMPF_SCALER_SOURCE_ISP)&&(m_scalerSwitch[pathsel].flowctl == SCAL_READY_TO_SWITCH )){
                    dbg_printf(GRA_DBG_LEVEL,">I:S_R_T_S%d->ISP...\r\n",pathsel);
                    MMPF_Scaler_SetPath(m_scalerSwitch[pathsel].src, m_scalerSwitch[pathsel].path);
                    m_scalerSwitch[pathsel].flowctl= SCAL_FLOW_NONE ;
                    gbGRAEn[pathsel] = FALSE ;
                } 
                else {
                    return MMP_ERR_NONE ;
                }
            } else {
                if(m_scalerSwitch[pathsel].src==MMPF_SCALER_SOURCE_GRA) {
                    MMPF_Scaler_SetPath(m_scalerSwitch[pathsel].src, m_scalerSwitch[pathsel].path);
                    m_scalerSwitch[pathsel].flowctl= SCAL_FLOW_NONE ;
                }
                else if((m_scalerSwitch[pathsel].src==MMPF_SCALER_SOURCE_ISP)&&(m_scalerSwitch[pathsel].flowctl == SCAL_READY_TO_SWITCH )){
                    dbg_printf(GRA_DBG_LEVEL,">I:YUY2->ISP : %d,%d\r\n",m_scalerSwitch[pathsel].src,usb_vc_cur_usof());
                    MMPF_Scaler_SetPath(m_scalerSwitch[pathsel].src, m_scalerSwitch[pathsel].path);
                    m_scalerSwitch[pathsel].flowctl= SCAL_FLOW_NONE ;
                    gbGRAEn[pathsel] = FALSE ;
                }
                else {
                    //dbg_printf(GRA_DBG_LEVEL,">I:YUY2->GRA : %d,%d\r\n",m_scalerSwitch[pathsel].src,usb_vc_cur_usof());
                    return MMP_ERR_NONE ;
                }
            }
            MMPF_IBC_SetStoreEnable(GRA_SRC_PIPE,pipe_store_en[GRA_SRC_PIPE] );
        }
        m_scalerSwitch[pathsel].set = MMP_FALSE ;
        return MMP_ERR_NONE ;     
    }
    
    if(set) {
        // wait_end : 1, H264 dynamic change resolution only
        if(wait_end) {
            m_scalerSwitch[pathsel].set = MMP_TRUE ;
            m_scalerSwitch[pathsel].path = pathsel ;
            m_scalerSwitch[pathsel].src = src;  
            m_scalerSwitch[pathsel].flowctl = SCAL_ENC_END ; //fixed dynamic change resolution
            while(m_scalerSwitch[pathsel].set) {
                MMPF_OS_Sleep(1);
                dbg_printf(GRA_DBG_LEVEL,"<Switch...>\r\n");
            }
        }
        else {
            grab = m_scalerZoomInfo[pathsel].grabCtl;
            if( (m_scalerSwitch[pathsel].m != grab.usScaleM ) || (m_scalerSwitch[pathsel].n != grab.usScaleN )) {
                IRQ_LOCK(
                    m_scalerSwitch[pathsel].m = grab.usScaleM ;
                    m_scalerSwitch[pathsel].n = grab.usScaleN ;
                    m_scalerSwitch[pathsel].set = MMP_TRUE ;  
                    m_scalerSwitch[pathsel].path = pathsel ;
                    m_scalerSwitch[pathsel].src = src;  
                    if(m_scalerSwitch[pathsel].flowctl!=SCAL_WAIT_ENC_END) {
                    //    m_scalerSwitch.flowctl = SCAL_FLOW_NONE ;
                    }
                )
                dbg_printf(GRA_DBG_LEVEL,"<I:Set(N/M) : %d / %d\r\n", m_scalerSwitch[pathsel].n, m_scalerSwitch[pathsel].m);
            }
            else {
             //   dbg_printf(3,"Oops N[%d/%d] - O[%d/%d]\r\n",grab.usScaleN,grab.usScaleM,m_scalerSwitch[pathsel].n,m_scalerSwitch[pathsel].m);
            }
        }
       // dbg_printf(3,"GRA[%d]->M,N : %d,%d\r\n",m_scalerSwitch.src,m_scalerSwitch.m,m_scalerSwitch.n);
        return MMP_ERR_NONE ;
    }
    
}


MMPF_SCALER_SWITCH_PATH *MMPF_Scaler_GetPathInfo(MMPF_SCALER_PATH pathsel)
{
    return &m_scalerSwitch[pathsel];
}

void MMPF_Scaler_ResetPathInfo(MMPF_SCALER_PATH pathsel)
{
    MMP_USHORT i;
    //for(i=0;i<2;i++) {
    if(pathsel==GRA_DST_PIPE_1) {
        m_scalerSwitch[0].flowctl = SCAL_FLOW_NONE;
        m_scalerSwitch[0].m = ZOOM_RATIO_BASE;
        m_scalerSwitch[0].n = ZOOM_RATIO_BASE;
        m_scalerSwitch[0].forceswitch = MMP_FALSE ;
        m_scalerSwitch[0].set = MMP_FALSE ;
    }
    else if(pathsel==GRA_DST_PIPE_2) {
        m_scalerSwitch[1].flowctl = SCAL_FLOW_NONE;
        m_scalerSwitch[1].m = ZOOM_RATIO_BASE;
        m_scalerSwitch[1].n = ZOOM_RATIO_BASE;
        m_scalerSwitch[1].forceswitch = MMP_FALSE ;
        m_scalerSwitch[1].set = MMP_FALSE ;
    }
    
}

void MMPF_Scaler_SetForceSwitchPath(MMPF_SCALER_PATH pathsel)
{
    if(pathsel==GRA_DST_PIPE_1) {
        m_scalerSwitch[0].forceswitch = MMP_TRUE ;
    }
    else if(pathsel==GRA_DST_PIPE_2) {
        m_scalerSwitch[1].forceswitch = MMP_TRUE ;
    }
}

MMP_ERR MMPF_Scaler_CheckZoomComplete(MMPF_SCALER_PATH pathsel, MMP_BOOL *bComplete)
{
#if 0 // Check gbZoomPathSel instead of this
    if (MMPH_HIF_CmdGetStatusL() & IMAGE_UNDER_ZOOM)
        *bComplete = MMP_FALSE;
    else
        *bComplete = MMP_TRUE;
#else
extern MMP_UBYTE      gbZoomPathSel;
    *bComplete = (gbZoomPathSel==0)?MMP_TRUE : MMP_FALSE ;
#endif    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Scaler_SetPixelLineDelay
//  Description : Set scaler pixel and line delay
//------------------------------------------------------------------------------
/** @brief Set scaler pixel and line delay. Pixel and line delay only useful when scaling up
*/
MMP_ERR MMPF_Scaler_SetPixelLineDelay(MMPF_SCALER_PATH pathsel, MMP_UBYTE ubPixelDelay, MMP_UBYTE ubLineDelay)
{
    AITPS_SCAL  pSCAL	= AITC_BASE_SCAL;
    
    if(pathsel == MMPF_SCALER_PATH_0){
        pSCAL->SCAL_P0_PIXL_DELAY = ubPixelDelay;
        pSCAL->SCAL_P0_LINE_DELAY = ubLineDelay;
    }else if(pathsel == MMPF_SCALER_PATH_1){
        pSCAL->SCAL_P1_PIXL_DELAY = ubPixelDelay;
        pSCAL->SCAL_P1_LINE_DELAY = ubLineDelay;
    }else if(pathsel == MMPF_SCALER_PATH_2){
        pSCAL->SCAL_P2_PIXL_DELAY = ubPixelDelay;
        pSCAL->SCAL_P2_LINE_DELAY = ubLineDelay;
    }
	return MMP_ERR_NONE;
}

MMP_UBYTE MMPF_Scaler_GetLineDelay(MMPF_SCALER_PATH pathsel)
{
    AITPS_SCAL  pSCAL	= AITC_BASE_SCAL;
    
    if(pathsel == MMPF_SCALER_PATH_0){
        return pSCAL->SCAL_P0_LINE_DELAY ;
    }else if(pathsel == MMPF_SCALER_PATH_1){
        return pSCAL->SCAL_P1_LINE_DELAY ;
    }else if(pathsel == MMPF_SCALER_PATH_2){
        return pSCAL->SCAL_P2_LINE_DELAY ;
    }
    return 0;
}

MMP_ERR MMPF_Scaler_CheckPanTiltComplete(MMPF_SCALER_PATH pathsel, MMP_BOOL *bComplete)
{
    if (MMPF_HIF_GetCmdStatus(IMAGE_UNDER_PANTILT))
        *bComplete = MMP_FALSE;
    else
        *bComplete = MMP_TRUE;
    
    return MMP_ERR_NONE;
}

MMP_UBYTE MMPF_Scaler_CheckUpdatePath(void)
{
    
    MMP_UBYTE update = 0 ,grapath;
    update = gbZoomPathSel | gbPanTiltPathSel ;
#if SUPPORT_GRA_ZOOM

    if( update & (1<<MMPF_SCALER_PATH_0) ) {
        if( MMPF_Scaler_GetPath(GRA_DST_PIPE_1)==MMPF_SCALER_SOURCE_GRA ) {
            return (1 << GRA_SRC_PIPE ) ; // path is change to graphic loop
        }
    }
    
    if( update & (1<<MMPF_SCALER_PATH_1) ) {
        if( MMPF_Scaler_GetPath(GRA_DST_PIPE_2)==MMPF_SCALER_SOURCE_GRA ) {
            return (1 << GRA_SRC_PIPE )  ; // path is change to graphic loop
        }
    }
    //
    // pipe 2 is used for graphic source
    // AE window reset
    if( update & (1<<MMPF_SCALER_PATH_2) ) {
        return (1 << GRA_SRC_PIPE )  ;
    }
#endif    
    return update ;
}

#if SCALER_ZOOM_LEVEL==BEST_LEVEL

RATIO_TBL  ratio_tbl[/*6112*/] =
{
6,96,81,
8,126,82,
5,78,83,
7,107,84,
5,76,85,
5,75,86,
6,89,87,
6,88,88,
8,116,89,
7,100,90,
5,71,91,
5,70,92,
5,69,93,
7,96,94,
8,109,95,
6,81,96,
9,120,97,
8,105,98,
5,65,99,
9,116,100,
5,64,101,
8,101,102,
6,75,103,
9,112,104,
10,123,105,
7,85,106,
9,109,107,
9,108,108,
10,118,109,
8,94,110,
6,70,111,
6,69,112,
7,80,113,
9,102,114,
9,101,115,
11,122,116,
8,88,117,
11,120,118,
7,76,119,
10,108,120,
8,85,121,
7,74,122,
11,115,123,
8,83,124,
10,103,125,
9,92,126,
10,102,127,
9,91,128,
8,80,129,
11,109,130,
8,79,131,
11,108,132,
7,68,133,
12,116,134,
10,96,135,
8,76,136,
7,66,137,
9,84,138,
10,93,139,
12,111,140,
11,101,141,
11,100,142,
8,72,144,
12,107,145,
8,71,146,
10,88,147,
10,87,148,
9,78,149,
10,86,150,
13,111,151,
8,68,152,
9,76,153,
13,109,154,
9,75,155,
14,116,156,
14,115,157,
15,123,158,
15,122,159,
11,89,160,
8,64,162,
14,111,163,
10,79,164,
11,86,165,
14,109,166,
12,93,167,
14,108,168,
11,84,169,
10,76,170,
11,83,171,
16,120,172,
15,112,173,
16,119,174,
10,74,175,
9,66,176,
17,124,177,
15,109,178,
13,94,179,
17,122,180,
14,100,181,
11,78,182,
13,92,183,
15,105,185,
16,111,186,
14,97,187,
15,103,188,
14,96,189,
10,68,190,
13,88,191,
18,121,192,
13,87,193,
18,120,194,
13,86,195,
18,119,196,
16,105,197,
11,72,198,
18,117,199,
11,71,200,
12,77,201,
18,115,202,
11,70,203,
15,95,204,
19,120,205,
11,69,206,
12,75,207,
18,112,208,
16,99,209,
19,117,210,
17,104,211,
11,67,212,
12,73,213,
18,109,214,
12,72,216,
20,119,217,
17,101,218,
12,71,219,
15,88,220,
20,117,221,
17,99,222,
19,110,223,
17,98,224,
12,69,225,
18,103,226,
16,91,227,
12,68,228,
14,79,229,
16,90,230,
20,112,231,
19,106,232,
18,100,233,
13,72,234,
18,99,235,
23,126,236,
20,109,237,
16,87,238,
22,119,239,
18,97,240,
14,75,241,
17,91,242,
16,85,243,
23,122,244,
21,111,245,
20,105,246,
21,110,247,
23,120,248,
15,78,249,
23,119,250,
13,67,251,
21,108,252,
17,87,253,
23,117,254,
13,66,255,
22,111,256,
25,126,257,
20,100,259,
23,114,261,
14,69,262,
13,64,263,
20,98,264,
16,78,265,
21,102,266,
25,121,267,
24,116,268,
26,125,269,
15,72,270,
18,86,271,
24,114,272,
23,109,273,
25,118,274,
20,94,275,
23,108,276,
18,84,277,
17,79,278,
25,116,279,
21,97,280,
23,106,281,
24,110,282,
14,64,283,
20,91,284,
26,118,285,
17,77,286,
18,81,288,
17,76,289,
15,67,290,
25,111,291,
23,102,292,
17,75,293,
25,110,294,
18,79,295,
24,105,296,
22,96,297,
26,113,298,
27,117,299,
16,69,300,
23,99,301,
28,120,302,
29,124,303,
19,81,304,
25,106,305,
26,110,306,
19,80,307,
20,84,308,
16,67,309,
23,96,310,
25,104,311,
28,116,312,
22,91,313,
24,99,314,
29,119,315,
22,90,316,
26,106,317,
27,110,318,
17,69,319,
22,89,320,
31,125,321,
20,80,324,
30,119,326,
26,103,327,
17,67,328,
31,122,329,
26,102,330,
23,90,331,
28,109,332,
26,101,333,
32,124,334,
29,112,335,
26,100,336,
19,73,337,
24,92,338,
33,126,339,
31,118,340,
24,91,341,
32,121,342,
26,98,343,
17,64,344,
20,75,345,
19,71,346,
30,112,347,
29,108,348,
31,115,349,
33,122,350,
29,107,351,
22,81,352,
21,77,353,
26,95,354,
31,113,355,
25,91,356,
29,105,357,
21,76,358,
28,101,359,
32,115,360,
29,104,361,
33,118,362,
32,114,363,
29,103,364,
20,71,365,
26,92,366,
19,67,367,
35,123,368,
20,70,370,
35,122,371,
25,87,372,
36,125,373,
26,90,374,
31,107,375,
25,86,376,
32,110,377,
19,65,378,
36,123,379,
27,92,380,
30,102,381,
31,105,382,
21,71,383,
30,101,384,
36,121,385,
37,124,386,
26,87,387,
27,90,388,
37,123,389,
38,126,390,
29,96,391,
20,66,392,
31,102,393,
32,105,394,
29,95,395,
34,111,396,
35,114,397,
32,104,398,
37,120,399,
21,68,400,
22,71,401,
32,103,402,
24,77,403,
34,109,404,
20,64,405,
27,86,406,
33,105,407,
34,108,408,
24,76,409,
38,120,410,
34,107,411,
35,110,412,
38,119,413,
23,72,414,
33,103,415,
37,115,416,
29,90,417,
31,96,418,
33,102,419,
36,111,420,
40,123,421,
31,95,422,
32,98,423,
39,119,424,
23,70,425,
26,79,426,
31,94,427,
38,115,428,
39,117,432,
37,110,435,
33,98,436,
28,83,437,
23,68,438,
39,115,439,
35,103,440,
32,94,441,
27,79,442,
26,76,443,
37,108,444,
34,99,445,
40,116,446,
38,110,447,
37,107,448,
35,101,449,
25,72,450,
31,89,451,
37,106,452,
42,120,453,
33,94,454,
32,91,455,
43,122,456,
36,102,457,
35,99,458,
33,93,459,
43,121,460,
42,118,461,
45,126,462,
34,95,463,
29,81,464,
32,89,465,
36,100,466,
44,122,467,
26,72,468,
33,91,469,
37,102,470,
44,121,471,
27,74,472,
34,93,473,
37,101,474,
44,120,475,
32,87,476,
42,114,477,
41,111,478,
44,119,479,
40,108,480,
29,78,481,
32,86,482,
41,110,483,
34,91,484,
39,104,486,
32,85,487,
29,77,488,
31,82,489,
28,74,490,
33,87,491,
41,108,492,
48,126,493,
37,97,494,
47,123,495,
31,81,496,
28,73,497,
25,65,498,
37,96,499,
39,101,500,
36,93,501,
43,111,502,
47,121,503,
44,113,504,
46,118,505,
25,64,506,
36,92,507,
42,107,508,
44,112,509,
41,104,510,
47,119,511,
34,86,512,
42,106,513,
29,73,514,
33,83,515,
45,113,516,
42,105,518,
45,112,520,
39,97,521,
50,124,522,
42,104,523,
34,84,524,
28,69,525,
41,101,526,
48,118,527,
33,81,528,
49,120,529,
43,105,530,
41,100,531,
30,73,532,
35,85,533,
47,114,534,
31,75,535,
36,87,536,
39,94,537,
49,118,538,
47,113,539,
35,84,540,
51,122,541,
41,98,542,
39,93,543,
50,119,544,
53,126,545,
35,83,546,
52,123,547,
36,85,548,
28,66,549,
34,80,550,
37,87,551,
52,122,552,
35,82,553,
36,84,555,
52,121,556,
43,100,557,
31,72,558,
44,102,559,
35,81,560,
49,113,561,
43,99,562,
40,92,563,
34,78,564,
55,126,565,
45,103,566,
53,121,567,
50,114,568,
40,91,569,
33,75,570,
52,118,571,
34,77,572,
50,113,573,
35,79,574,
36,81,576,
41,92,577,
50,112,578,
51,114,579,
52,116,580,
35,78,581,
40,89,582,
54,120,583,
55,122,584,
33,73,585,
52,115,586,
34,75,587,
49,108,588,
45,99,589,
51,112,590,
52,114,591,
53,116,592,
38,83,593,
39,85,594,
34,74,595,
35,76,596,
41,89,597,
48,104,598,
37,80,599,
57,123,600,
45,97,601,
40,86,602,
48,103,603,
35,75,604,
36,77,605,
37,79,606,
45,96,607,
38,81,608,
55,117,609,
33,70,610,
58,123,611,
52,110,612,
53,112,613,
46,97,614,
47,99,615,
58,122,616,
40,84,617,
32,67,618,
44,92,619,
56,117,620,
46,96,621,
48,100,622,
50,104,623,
54,112,624,
42,87,625,
59,122,626,
45,93,627,
33,68,628,
34,70,629,
37,76,630,
39,80,631,
61,125,632,
45,92,633,
48,98,634,
50,102,635,
56,114,636,
60,122,637,
32,65,638,
38,77,639,
40,81,640,
50,101,641,
62,125,642,
51,102,648,
59,117,653,
50,99,654,
45,89,655,
39,77,656,
35,69,657,
61,120,658,
59,116,659,
55,108,660,
50,98,661,
47,92,662,
42,82,663,
40,78,664,
38,74,665,
53,103,666,
51,99,667,
48,93,668,
46,89,669,
44,85,670,
58,112,671,
54,104,672,
40,77,673,
38,73,674,
49,94,675,
36,69,676,
35,67,677,
45,86,678,
54,103,679,
53,101,680,
51,97,681,
49,93,682,
39,74,683,
66,125,684,
46,87,685,
45,85,686,
35,66,687,
43,81,688,
66,124,689,
65,122,690,
48,90,691,
39,73,692,
61,114,693,
52,97,694,
44,82,695,
36,67,696,
35,65,697,
55,102,698,
41,76,699,
60,111,700,
46,85,701,
39,72,702,
63,116,703,
56,103,704,
55,101,705,
60,110,706,
65,119,707,
35,64,708,
63,115,709,
51,93,710,
62,113,711,
55,100,712,
49,89,713,
59,107,714,
69,125,715,
63,114,716,
67,121,717,
51,92,718,
50,90,720,
69,124,721,
58,104,722,
48,86,723,
38,68,724,
61,109,725,
65,116,726,
69,123,727,
50,89,728,
49,87,729,
71,126,730,
44,78,731,
39,69,732,
47,83,733,
51,90,734,
42,74,735,
50,88,736,
66,116,737,
53,93,738,
69,121,739,
64,112,740,
51,89,742,
66,115,743,
58,101,744,
61,106,745,
57,99,746,
64,111,747,
67,116,748,
63,109,749,
73,126,750,
58,100,751,
54,93,752,
68,117,753,
71,122,754,
67,115,755,
73,125,756,
52,89,757,
62,106,758,
41,70,759,
54,92,760,
67,114,761,
73,124,762,
53,90,763,
46,78,764,
62,105,765,
55,93,766,
58,98,767,
64,108,768,
38,64,769,
66,111,770,
72,121,771,
56,94,772,
71,119,773,
55,92,774,
64,107,775,
60,100,777,
74,123,779,
53,88,780,
41,68,781,
64,106,782,
75,124,783,
46,76,784,
60,99,785,
71,117,786,
48,79,787,
73,120,788,
53,87,789,
50,82,790,
47,77,791,
74,121,792,
49,80,793,
57,93,794,
70,114,795,
51,83,796,
72,117,797,
53,86,798,
66,107,799,
63,102,800,
68,110,801,
78,126,802,
49,79,803,
72,116,804,
41,66,805,
61,98,806,
76,122,807,
78,125,808,
75,120,810,
72,115,811,
47,75,812,
54,86,813,
44,70,814,
61,97,815,
63,100,816,
41,65,817,
72,114,818,
74,117,819,
57,90,820,
52,82,821,
59,93,822,
68,107,823,
42,66,824,
79,124,825,
58,91,826,
53,83,827,
78,122,828,
64,100,829,
50,78,830,
43,67,831,
52,81,832,
81,126,833,
65,101,834,
49,76,835,
60,93,836,
42,65,837,
77,119,838,
79,122,839,
70,108,840,
50,77,841,
78,120,842,
54,83,843,
43,66,844,
62,95,845,
81,124,846,
68,104,847,
53,81,848,
59,90,849,
63,96,850,
67,102,851,
75,114,852,
81,123,853,
60,91,854,
66,100,855,
72,109,856,
47,71,857,
55,83,858,
67,101,859,
77,116,860,
54,81,864,
83,124,867,
69,103,868,
57,85,869,
47,70,870,
74,110,871,
66,98,872,
64,95,873,
81,120,874,
75,111,875,
69,102,876,
63,93,877,
80,118,878,
76,112,879,
72,106,880,
68,100,881,
62,91,882,
77,113,883,
73,107,884,
69,101,885,
78,114,886,
50,73,887,
83,121,888,
46,67,889,
68,99,890,
66,96,891,
53,77,892,
71,103,893,
69,100,894,
67,97,895,
56,81,896,
72,104,897,
52,75,898,
68,98,899,
82,118,900,
48,69,901,
87,125,902,
69,99,903,
81,116,904,
88,126,905,
86,123,906,
63,90,907,
61,87,908,
80,114,909,
52,74,910,
45,64,911,
81,115,912,
55,78,913,
72,102,914,
53,75,915,
75,106,916,
46,65,917,
78,110,918,
88,124,919,
81,114,920,
69,97,921,
79,111,922,
72,101,923,
77,108,924,
80,112,925,
88,123,927,
81,113,928,
71,99,929,
56,78,930,
69,96,931,
82,114,932,
90,125,933,
80,111,934,
83,115,935,
86,119,936,
55,76,937,
76,105,938,
66,91,939,
74,102,940,
69,95,941,
56,77,942,
91,125,943,
59,81,944,
70,96,945,
65,89,946,
57,78,947,
90,123,948,
63,86,949,
55,75,950,
58,79,951,
72,98,952,
78,106,953,
56,76,954,
59,80,955,
93,126,956,
65,88,957,
71,96,958,
77,104,959,
60,81,960,
89,120,961,
52,70,962,
84,113,963,
93,125,964,
79,106,965,
85,114,966,
53,71,967,
80,107,968,
83,111,969,
69,92,972,
82,109,974,
76,101,975,
52,69,976,
89,118,977,
71,94,978,
62,82,979,
87,115,980,
75,99,981,
69,91,982,
63,83,983,
79,104,984,
73,96,985,
70,92,986,
80,105,987,
77,101,988,
71,93,989,
55,72,990,
52,68,991,
72,94,992,
82,107,993,
56,73,994,
63,82,995,
60,78,996,
77,100,997,
94,122,998,
81,105,999,
95,123,1000,
51,66,1001,
89,115,1002,
96,124,1003,
69,89,1004,
83,107,1005,
66,85,1006,
77,99,1008,
74,95,1009,
85,109,1010,
96,123,1011,
50,64,1012,
61,78,1013,
83,106,1014,
76,97,1015,
98,125,1016,
95,121,1017,
88,112,1018,
96,122,1019,
63,80,1020,
78,99,1021,
56,71,1022,
94,119,1023,
68,86,1024,
91,115,1025,
99,125,1026,
92,116,1027,
73,92,1028,
58,73,1029,
97,122,1030,
74,93,1031,
90,113,1032,
63,79,1033,
79,99,1034,
68,85,1036,
101,126,1038,
73,91,1039,
53,66,1040,
94,117,1041,
74,92,1042,
66,82,1043,
58,72,1044,
71,88,1045,
84,104,1046,
59,73,1047,
55,68,1048,
85,105,1049,
60,74,1050,
73,90,1051,
95,117,1052,
78,96,1053,
70,86,1054,
79,97,1055,
66,81,1056,
80,98,1057,
67,82,1058,
103,126,1059,
54,66,1060,
77,94,1061,
91,111,1062,
64,78,1063,
69,84,1064,
97,118,1065,
93,113,1066,
103,125,1067,
89,108,1068,
66,80,1069,
57,69,1070,
81,98,1071,
72,87,1072,
87,105,1073,
97,117,1074,
83,100,1075,
59,71,1076,
69,83,1077,
94,113,1078,
75,90,1080,
101,121,1081,
76,91,1082,
97,116,1083,
87,104,1084,
93,111,1085,
57,68,1086,
73,87,1087,
84,100,1088,
100,119,1089,
90,107,1090,
64,76,1091,
86,102,1092,
92,109,1093,
87,103,1094,
60,71,1095,
88,104,1096,
83,98,1097,
61,72,1098,
95,112,1099,
90,106,1100,
85,100,1101,
97,114,1102,
63,74,1103,
92,108,1104,
64,75,1105,
70,82,1106,
106,124,1107,
65,76,1108,
107,125,1109,
102,119,1110,
79,92,1112,
73,85,1113,
92,107,1114,
105,122,1115,
62,72,1116,
100,116,1117,
82,95,1118,
108,125,1119,
70,81,1120,
58,67,1121,
71,82,1122,
98,113,1123,
72,83,1124,
99,114,1125,
73,84,1126,
67,77,1127,
101,116,1128,
68,78,1129,
109,125,1130,
62,71,1131,
90,103,1132,
91,104,1134,
85,97,1135,
100,114,1136,
79,90,1137,
65,74,1138,
80,91,1139,
66,75,1140,
59,67,1141,
97,110,1142,
60,68,1143,
106,120,1144,
84,95,1145,
69,78,1146,
93,105,1147,
70,79,1148,
63,71,1149,
111,125,1150,
88,99,1152,
97,109,1153,
98,110,1154,
66,74,1155,
108,121,1156,
92,103,1157,
101,113,1158,
68,76,1159,
111,124,1160,
95,106,1161,
87,97,1162,
70,78,1163,
106,118,1164,
89,99,1165,
108,120,1166,
82,91,1167,
92,102,1168,
83,92,1169,
84,93,1170,
113,125,1171,
95,105,1172,
96,106,1173,
77,85,1174,
107,118,1175,
79,87,1176,
99,109,1177,
90,99,1178,
111,122,1179,
82,90,1180,
114,125,1181,
94,103,1182,
84,92,1183,
85,93,1184,
107,117,1185,
65,71,1186,
109,119,1187,
111,121,1188,
67,73,1189,
102,111,1190,
80,87,1191,
81,88,1192,
105,114,1193,
59,64,1194,
71,77,1195,
84,91,1196,
73,79,1197,
74,80,1198,
112,121,1199,
63,68,1200,
114,123,1201,
77,83,1202,
78,84,1203,
92,99,1204,
106,114,1205,
108,116,1206,
68,73,1207,
111,119,1208,
70,75,1209,
85,91,1210,
86,92,1211,
116,124,1212,
118,126,1213,
104,111,1214,
91,97,1215,
76,81,1216,
62,66,1217,
63,67,1218,
64,68,1219,
65,69,1220,
66,70,1221,
100,106,1222,
101,107,1223,
85,90,1224,
87,92,1225,
71,75,1226,
107,113,1227,
73,77,1228,
111,117,1229,
75,79,1230,
114,120,1231,
116,122,1232,
118,124,1233,
101,106,1234,
61,64,1235,
83,87,1236,
85,89,1237,
108,113,1238,
88,92,1239,
111,116,1240,
68,71,1241,
116,121,1242,
71,74,1243,
120,125,1244,
74,77,1245,
75,78,1246,
102,106,1247,
104,108,1248,
107,111,1249,
109,113,1250,
113,117,1251,
115,119,1252,
118,122,1253,
120,124,1254,
92,95,1255,
64,66,1256,
97,100,1257,
101,104,1258,
103,106,1259,
108,111,1260,
111,114,1261,
75,77,1262,
78,80,1263,
122,125,1264,
123,126,1265,
87,89,1266,
90,92,1267,
93,95,1268,
94,96,1269,
99,101,1270,
103,105,1271,
107,109,1272,
111,113,1273,
117,119,1274,
123,125,1275,
66,67,1276,
68,69,1277,
74,75,1278,
76,77,1279,
81,82,1280,
86,87,1281,
95,96,1282,
103,104,1283,
115,116,1284,
121,122,1285,
120,120,1296,
#if SUPPORT_GRA_ZOOM
89,88,1325,
73,72,1331,
90,88,1340,
57,56,1342,
82,80,1344,
91,88,1354,
66,64,1356,
41,40,1360,
58,56,1365,
75,72,1367,
92,88,1369,
33,32,1376,
93,88,1384,
76,72,1385,
59,56,1388,
42,40,1393,
68,64,1397,
94,88,1399,
51,48,1403,
86,80,1409,
60,56,1411,
95,88,1413,
69,64,1417,
78,72,1421,
43,40,1425,
96,88,1428,
52,48,1430,
61,56,1434,
70,64,1437,
79,72,1439,
88,80,1441,
97,88,1443,
44,40,1457,
99,88,1472,
90,80,1474,
81,72,1475,
72,64,1478,
63,56,1480,
54,48,1484,
100,88,1487,
91,80,1490,
82,72,1493,
36,32,1498,
101,88,1502,
64,56,1504,
92,80,1506,
27,24,1511,
102,88,1516,
74,64,1518,
93,80,1522,
65,56,1527,
84,72,1529,
103,88,1531,
56,48,1538,
104,88,1546,
85,72,1547,
66,56,1550,
95,80,1554,
76,64,1559,
105,88,1560,
57,48,1565,
96,80,1571,
67,56,1573,
106,88,1575,
77,64,1579,
87,72,1583,
97,80,1587,
107,88,1590,
58,48,1592,
68,56,1596,
78,64,1599,
88,72,1601,
98,80,1603,
108,88,1605,
59,48,1619,
110,88,1634,
100,80,1635,
90,72,1637,
80,64,1639,
70,56,1642,
60,48,1646,
111,88,1649,
50,40,1652,
91,72,1655,
40,32,1660,
112,88,1663,
71,56,1665,
102,80,1668,
30,24,1673,
113,88,1678,
82,64,1680,
51,40,1684,
72,56,1689,
93,72,1691,
114,88,1693,
104,80,1700,
115,88,1708,
94,72,1709,
73,56,1712,
52,40,1716,
84,64,1720,
116,88,1722,
63,48,1727,
106,80,1733,
74,56,1735,
117,88,1737,
85,64,1741,
96,72,1745,
53,40,1749,
118,88,1752,
64,48,1754,
75,56,1758,
86,64,1761,
97,72,1763,
108,80,1765,
119,88,1766,
32,24,1781,
121,88,1796,
110,80,1797,
99,72,1799,
88,64,1801,
77,56,1804,
66,48,1808,
122,88,1811,
111,80,1813,
100,72,1817,
44,32,1822,
123,88,1825,
78,56,1827,
112,80,1830,
33,24,1835,
124,88,1840,
90,64,1842,
113,80,1846,
79,56,1850,
102,72,1853,
125,88,1855,
45,32,1862,
103,72,1871,
80,56,1874,
57,40,1878,
92,64,1882,
104,72,1889,
116,80,1894,
81,56,1897,
46,32,1903,
105,72,1907,
58,40,1911,
70,48,1916,
82,56,1920,
94,64,1923,
106,72,1925,
118,80,1927,
35,24,1943,
120,80,1959,
108,72,1961,
96,64,1963,
84,56,1966,
72,48,1970,
60,40,1975,
109,72,1979,
48,32,1983,
85,56,1989,
122,80,1992,
36,24,1997,
98,64,2004,
61,40,2008,
86,56,2012,
111,72,2015,
99,64,2024,
112,72,2033,
87,56,2035,
62,40,2040,
100,64,2044,
37,24,2051,
88,56,2059,
50,32,2064,
114,72,2069,
63,40,2072,
76,48,2078,
89,56,2082,
102,64,2085,
115,72,2087,
77,48,2105,
117,72,2123,
104,64,2125,
91,56,2128,
78,48,2132,
65,40,2137,
118,72,2141,
105,64,2145,
92,56,2151,
39,24,2159,
106,64,2166,
66,40,2170,
93,56,2174,
120,72,2177,
53,32,2186,
121,72,2195,
94,56,2197,
67,40,2202,
108,64,2206,
40,24,2213,
95,56,2220,
109,64,2226,
123,72,2231,
68,40,2234,
82,48,2240,
96,56,2244,
110,64,2247,
124,72,2249,
111,64,2267,
112,64,2287,
98,56,2290,
84,48,2294,
70,40,2299,
56,32,2307,
99,56,2313,
42,24,2321,
114,64,2327,
71,40,2331,
100,56,2336,
57,32,2348,
101,56,2359,
72,40,2364,
116,64,2368,
87,48,2375,
102,56,2382,
58,32,2388,
73,40,2396,
88,48,2402,
103,56,2405,
118,64,2408,
74,40,2429,
120,64,2449,
105,56,2452,
90,48,2456,
75,40,2461,
60,32,2469,
106,56,2475,
91,48,2483,
122,64,2489,
76,40,2493,
107,56,2498,
92,48,2510,
108,56,2521,
77,40,2526,
124,64,2530,
46,24,2537,
109,56,2544,
62,32,2550,
78,40,2558,
94,48,2564,
110,56,2567,
95,48,2590,
112,56,2614,
96,48,2617,
80,40,2623,
64,32,2631,
113,56,2637,
48,24,2644,
81,40,2655,
114,56,2660,
98,48,2671,
115,56,2683,
82,40,2688,
49,24,2698,
116,56,2706,
66,32,2712,
83,40,2720,
100,48,2725,
117,56,2729,
67,32,2752,
119,56,2775,
102,48,2779,
85,40,2785,
68,32,2793,
120,56,2799,
103,48,2806,
86,40,2817,
121,56,2822,
69,32,2833,
122,56,2845,
87,40,2849,
105,48,2860,
123,56,2868,
70,32,2874,
88,40,2882,
106,48,2887,
124,56,2891,
71,32,2914,
108,48,2941,
90,40,2947,
72,32,2955,
54,24,2968,
91,40,2979,
73,32,2995,
92,40,3011,
55,24,3022,
74,32,3036,
93,40,3044,
112,48,3049,
75,32,3076,
114,48,3103,
95,40,3108,
76,32,3117,
57,24,3130,
96,40,3141,
77,32,3157,
97,40,3173,
58,24,3184,
78,32,3198,
98,40,3206,
118,48,3211,
59,24,3238,
120,48,3265,
100,40,3270,
80,32,3278,
60,24,3292,
101,40,3303,
81,32,3319,
102,40,3335,
61,24,3346,
82,32,3359,
103,40,3367,
124,48,3373,
83,32,3400,
105,40,3432,
84,32,3440,
63,24,3454,
106,40,3465,
85,32,3481,
107,40,3497,
64,24,3508,
86,32,3521,
108,40,3529,
87,32,3562,
110,40,3594,
88,32,3602,
66,24,3616,
111,40,3626,
89,32,3643,
112,40,3659,
67,24,3670,
90,32,3683,
113,40,3691,
68,24,3724,
115,40,3756,
92,32,3764,
69,24,3778,
116,40,3788,
93,32,3805,
117,40,3821,
70,24,3832,
94,32,3845,
118,40,3853,
95,32,3885,
120,40,3918,
96,32,3926,
72,24,3939,
121,40,3950,
97,32,3966,
122,40,3983,
73,24,3993,
98,32,4007,
123,40,4015,
74,24,4047,
125,40,4080,
100,32,4088,
75,24,4101,
101,32,4128,
76,24,4155,
102,32,4169,
77,24,4209,
104,32,4250,
78,24,4263,
105,32,4290,
79,24,4317,
106,32,4331,
80,24,4371,
108,32,4412,
81,24,4425,
109,32,4452,
82,24,4479,
110,32,4493,
83,24,4533,
112,32,4573,
84,24,4587,
113,32,4614,
85,24,4641,
114,32,4654,
86,24,4695,
116,32,4735,
87,24,4749,
117,32,4776,
88,24,4803,
118,32,4816,
89,24,4857,
120,32,4897,
90,24,4911,
121,32,4938,
91,24,4965,
122,32,4978,
92,24,5019,
124,32,5059,
93,24,5073,
125,32,5100,
94,24,5127,
95,24,5180,
96,24,5234,
97,24,5288,
98,24,5342,
99,24,5396,
100,24,5450,
101,24,5504,
102,24,5558,
103,24,5612,
104,24,5666,
105,24,5720,
106,24,5774,
107,24,5828,
108,24,5882,
109,24,5936,
110,24,5990,
111,24,6044,
112,24,6098,
113,24,6152,
114,24,6206,
115,24,6260,
116,24,6314,
117,24,6368,
118,24,6422,
119,24,6475,
120,24,6529,
121,24,6583,
122,24,6637,
123,24,6691,
124,24,6745,
125,24,6799,
#endif
125,24,6799,
};
MMP_USHORT ratio_tbl_items[MMPF_SCALER_PATH_MAX];
MMP_USHORT ratio_tbl_base_n_item[MMPF_SCALER_PATH_MAX];
MMP_USHORT ratio_tbl_cur_item[MMPF_SCALER_PATH_MAX];

MMP_USHORT MMPF_Scaler_CreateScalerTable(void)
{
#define BASE_M 64
#define END_M  128
#define BASE_N 4
static MMP_BOOL ratio_tbl_inited = 0 ;

	MMP_USHORT i,j,m,n ,c1=0,c = 0;
	unsigned int ratio ,ratio_x,ratio_y;
	RATIO_TBL item ;
	MMP_USHORT total_entries ; //= sizeof(ratio_tbl) / sizeof(RATIO_TBL) ;
	return sizeof(ratio_tbl) / sizeof(RATIO_TBL) ; 

}

RATIO_TBL *MMPF_Scaler_GetBestM(MMPF_SCALER_PATH pipe,MMP_USHORT *item_at,MMP_USHORT in_w,MMP_USHORT in_h,MMP_USHORT out_w,MMP_USHORT out_h,MMP_USHORT user_ratio)
{
#define MATCH_RATIO_N   (0)
	MMP_USHORT i, more_lines = 0, n ;
	unsigned int ratio ,ratio_x,ratio_y;
	RATIO_TBL *tbl;
	ratio_tbl_items[pipe] = MMPF_Scaler_CreateScalerTable();
	
	ratio_x = (CAL_BASE * out_w) / in_w;
	ratio_y = (CAL_BASE * out_h) / in_h;
	if(user_ratio) {
	    ratio = user_ratio;
	} else {
	    ratio = (ratio_x > ratio_y) ? ratio_x : ratio_y ;
	}
	ratio = ((ratio + 1) >> 1) << 1;	
	dbg_printf(0,"Cur Res Ratio : %d\r\n",ratio);
	*item_at = 0;
	for ( i = 0; i < ratio_tbl_items[pipe] ; i++) {
		if(ratio_tbl[i].ratio >= (MMP_USHORT)ratio) {
		    *item_at = i + more_lines ;
		    ratio_tbl_base_n_item[pipe] = *item_at;
		    ratio_tbl_cur_item[pipe] = ratio_tbl_base_n_item[pipe] ;
		    tbl = &ratio_tbl[*item_at];
#if MATCH_RATIO_N==0		    
			return tbl;//&ratio_tbl[*item_at];
#else
            n = MMPF_Scaler_GetNbyM( tbl->m,in_w,in_h,out_w,out_h) ;
            if(n == tbl->n) {
                return tbl ;
            }
            else {
                dbg_printf(0,"xx -- ratio : %d,n=%d,m=%d\r\n",ratio,tbl->n,tbl->m); 
            }
#endif			
		}
	}
	return 0 ;
}

MMP_USHORT MMPF_Scaler_GetNbyM(MMP_USHORT m,MMP_USHORT in_w,MMP_USHORT in_h,MMP_USHORT out_w,MMP_USHORT out_h)
{
    MMP_USHORT  n, y_scale, x_scale;
    if (in_w >= out_w) {
        x_scale = (out_w * m + in_w - 1) / in_w;
    }
    else {
        x_scale = (out_w * m + in_w - 2) / (in_w - 1);
    }
    if (in_h >= out_h) {
        y_scale = (out_h * m + in_h - 1) / in_h;
    }
    else {
        y_scale = (out_h * m + in_h - 2) / (in_h - 1);
    }
    if (x_scale > y_scale)
        n = x_scale;
    else
        n = y_scale;
    return n;
}

RATIO_TBL *MMPF_Scaler_GetBaseRatioTbl(MMPF_SCALER_PATH pipe)
{
    return &ratio_tbl[ratio_tbl_base_n_item[pipe] ];
}

RATIO_TBL *MMPF_Scaler_GetCurRatioTbl(MMPF_SCALER_PATH pipe)
{
    return &ratio_tbl[ratio_tbl_cur_item[pipe]];
}

MMP_USHORT MMPF_Scaler_GetRatioStartItem(MMPF_SCALER_PATH pipe)
{
    return ratio_tbl_base_n_item[pipe] ;
}


MMP_USHORT MMPF_Scaler_GetRatioCurItem(MMPF_SCALER_PATH pipe)
{
    return ratio_tbl_cur_item[pipe] ;
}

void MMPF_Scaler_SetRatioCurItem(MMPF_SCALER_PATH pipe,MMP_USHORT item)
{
    ratio_tbl_cur_item[pipe] = item;
}

RATIO_TBL *MMPF_Scaler_GetRatioTblByRatio(MMPF_SCALER_PATH pipe,MMP_USHORT new_ratio,MMP_BOOL move_ratio_item)
{

    MMP_SHORT i,old_item = ratio_tbl_cur_item[pipe];
    MMP_SHORT end = ratio_tbl_base_n_item[pipe] ;
    MMP_SHORT a,b ;
    
    RATIO_TBL *cur_tbl  = MMPF_Scaler_GetCurRatioTbl(pipe) ;
    //dbg_printf(3,"[%d]Ratio %d->%d,from #%d\r\n",move_ratio_item,cur_tbl->ratio,new_ratio,old_item);
    if(!move_ratio_item) {
        end = 0 ;
    }
    if(new_ratio >= cur_tbl->ratio) {
        for(i=ratio_tbl_cur_item[pipe];i<ratio_tbl_items[pipe];i++) {
            if(ratio_tbl[i].ratio >= new_ratio) {
#if 0
                a = ratio_tbl[i].ratio - new_ratio ;
                b = new_ratio - ratio_tbl[i-1].ratio ;
                if(a > b) {
                    i = i - 1 ;
                }
#endif
                if(move_ratio_item) {   
                    ratio_tbl_cur_item[pipe] = i;
                }
                break;
            }
        }
        // protect out of range
        if(i==ratio_tbl_items[pipe]) {
            i = ratio_tbl_items[pipe] - 1 ;
            if(move_ratio_item) {
                ratio_tbl_cur_item[pipe] = i;
            }
        }
    } else {
       
        for(i=ratio_tbl_cur_item[pipe];i >= end;i--) {
            if(ratio_tbl[i].ratio <= new_ratio) {
#if 0
                a = new_ratio - ratio_tbl[i].ratio ;
                b = ratio_tbl[i+1].ratio - new_ratio  ;
                if(a > b) {
                    i = i + 1;
                }
#endif
                if(move_ratio_item) {   
                    ratio_tbl_cur_item[pipe] = i + 1 ;//i;
                }
                break;
            }
        }
        // protect out of range
        if(i < end) {
            i = end ;
            if(move_ratio_item) {
                ratio_tbl_cur_item[pipe] = i ;
            }
        }
        
    }
    //dbg_printf(3,"[%d]item# %d->%d, (%d/%d)\r\n",ratio_tbl_items,old_item, i,ratio_tbl[i].n,ratio_tbl[i].m);
    
    return &ratio_tbl[i];
}
//
//  lync_ratio :
//  100 : w/h = 1
//  92 : w/h = 0.92
MMP_ERR MMPF_Scaler_GetBestGrabRange(MMPF_SCALER_PATH pipe,MMP_USHORT item,MMPF_SCALER_FIT_RANGE *fitrange,MMPF_SCALER_GRABCONTROL *grabctl,MMPF_SCALER_GRABCONTROL *grabinctl,MMP_USHORT lync_ratio)
{
	RATIO_TBL  *ratio_item ;
    MMP_USHORT  x_scale, y_scale;
    MMP_USHORT  unscale_width, unscale_height;
    MMP_ULONG	max_scale_width;
    MMP_BOOL    up_scale = FALSE ;
    MMP_USHORT scaler_ref_h = gsScalerRefInH;
    MMP_USHORT scaler_real_h = gsScalerRefOutH[pipe];
    MMP_SHORT startx=0,starty=0 ;
    
    // For 800x600 , 160x120 H264
#if SENSOR_IN_H > SENSOR_16_9_H
    if(MMPF_Scaler_GetCurInfo(pipe)->usSensorHeight==SENSOR_16_9_H) {
        scaler_real_h = fitrange->usOutHeight;
    }
#endif    
    if( (item >= ratio_tbl_items[pipe]) || (item < ratio_tbl_base_n_item[pipe] )) {
        dbg_printf(3,"item is out of ratio tbl :%d\r\n",item);
        return MMP_SCALER_ERR_PARAMETER ;
    }
    
    if(lync_ratio==0) {
        if (grabctl->usScaleN==0) {
            lync_ratio = USB_LYNC_CIF_RATIO ;
        }
        else {
            lync_ratio = 100;
        }
    }
    
    ratio_item = &ratio_tbl[item] ;
    if(pipe == MMPF_SCALER_PATH_0) {
        max_scale_width = SCALER_PATH0_MAX_WIDTH;	    
    }
    else if(pipe == MMPF_SCALER_PATH_1) {
        max_scale_width = SCALER_PATH1_MAX_WIDTH;	    
    }
    else {
        max_scale_width = SCALER_PATH2_MAX_WIDTH;	
    }
    
    if(lync_ratio==100) {
    	fitrange->usFitResol = ratio_item->m ;
    	grabctl->usScaleN = ratio_item->n ;
        grabctl->usScaleM = fitrange->usFitResol ;
        //if(grabctl->usScaleN <= grabctl->usScaleM) {
    	    y_scale = grabctl->usScaleN ;
    	    x_scale = grabctl->usScaleN ;
        //} else {
        //    y_scale = (fitrange->usOutHeight * grabctl->usScaleM + fitrange->usInHeight - 1) / fitrange->usInHeight;
        //   x_scale = (fitrange->usOutWidth * grabctl->usScaleM + fitrange->usInWidth - 1) / fitrange->usInWidth;
        //}
        if (fitrange->fitmode == MMPF_SCALER_FITMODE_OUT) {
    		if (x_scale > y_scale) {
           	   	grabctl->usScaleN = x_scale;
    		}
           	else {
           	   	grabctl->usScaleN = y_scale;
           	}
       	}
    	else {
           	if (x_scale > y_scale) {
       		 	grabctl->usScaleN = y_scale;
           	}
            else {
       	        grabctl->usScaleN = x_scale;
           	}
       	}

        //if (x_scale > grabctl->usScaleM || y_scale > grabctl->usScaleM) {
        up_scale = (x_scale > grabctl->usScaleM || y_scale > grabctl->usScaleM) ;
        
        if ((fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM > max_scale_width)|| (up_scale)){
    		if (up_scale) {
                MMP_ULONG x_st = (grabctl->usStartX*grabctl->usScaleM + grabctl->usScaleN -1)/grabctl->usScaleN;
                MMP_ULONG y_st = (grabctl->usStartY*grabctl->usScaleM + grabctl->usScaleN -1)/grabctl->usScaleN;

                unscale_width  = 3 + ((grabctl->usEndX - grabctl->usStartX) * grabctl->usScaleM ) / grabctl->usScaleN;
                unscale_height = 3 + ((grabctl->usEndY - grabctl->usStartY) * grabctl->usScaleM ) / grabctl->usScaleN;
                if ((x_st + unscale_width - 1) > fitrange->usInWidth) {
                    unscale_width = fitrange->usInWidth - x_st + 1;
                }
                if ((y_st + unscale_height - 1) > fitrange->usInHeight) {
                    unscale_height = fitrange->usInHeight - y_st + 1;
                }
    		}
    		else {
                unscale_width = ((grabctl->usEndX - grabctl->usStartX + 1) * grabctl->usScaleM + (grabctl->usScaleN - 1)) / grabctl->usScaleN;
                unscale_height = ((grabctl->usEndY - grabctl->usStartY + 1) * grabctl->usScaleM + (grabctl->usScaleN - 1)) / grabctl->usScaleN;
    		}
    		if(up_scale) {
                grabinctl->usStartX    = (fitrange->usInWidth-unscale_width)/2 + 1;
                grabinctl->usEndX      = grabinctl->usStartX + unscale_width - 1;
                grabinctl->usStartY    = (fitrange->usInHeight-unscale_height)/2 + 1;
                grabinctl->usEndY      = grabinctl->usStartY + unscale_height - 1;
        	} else {


                startx    = (fitrange->usInWidth-unscale_width)/2 + 1;
                starty    = (scaler_ref_h-unscale_height)/2 + 1;
	            if( (startx <= 0) || (starty <= 0) ) {
	                 //dbg_printf(3,"grabin--startx : %d, starty = %d\r\n",startx,starty);
	                 //goto try_again;
	                 return MMP_SCALER_ERR_N ;
	                
	            }
			
                grabinctl->usStartX    = (fitrange->usInWidth-unscale_width)/2 + 1;
                grabinctl->usEndX      = grabinctl->usStartX + unscale_width - 1;
                grabinctl->usStartY    = (scaler_ref_h-unscale_height)/2 + 1;
                grabinctl->usEndY      = grabinctl->usStartY + unscale_height - 1;
        	}	
    	}
        else {
            
            grabinctl->usStartX    = 1;
            grabinctl->usEndX      = fitrange->usInWidth;
            grabinctl->usStartY    = 1;
            grabinctl->usEndY      = fitrange->usInHeight;
        #if SENSOR_IN_H > SENSOR_16_9_H
        {
        	startx = (MMP_SHORT)(fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM
   	                            - fitrange->usOutWidth) / 2 + 1;
        	starty = (MMP_SHORT)( scaler_ref_h  * grabctl->usScaleN / grabctl->usScaleM
                                - scaler_real_h) / 2 + 1;
            
            if( (startx <= 0) || (starty <= 0) ) {
                dbg_printf(3,"--startx : %d, starty = %d\r\n",startx,starty);
                 //goto try_again;
                 return MMP_SCALER_ERR_N ;
                
            }
        
        	grabctl->usStartX = (fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM
   	                            - fitrange->usOutWidth) / 2 + 1;
        	grabctl->usStartY = (scaler_ref_h * grabctl->usScaleN / grabctl->usScaleM
                                - scaler_real_h) / 2 + 1;
		    grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
    		grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        }
        #else
        	startx = (MMP_SHORT)(fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM
   	                            - fitrange->usOutWidth) / 2 + 1;
        	starty = (MMP_SHORT)(fitrange->usInHeight * grabctl->usScaleN / grabctl->usScaleM
                                - fitrange->usOutHeight) / 2 + 1;
            
            if( (startx <= 0) || (starty <= 0) ) {
                 //goto try_again;
                 return MMP_SCALER_ERR_N ;
                
            }
        	grabctl->usStartX = (fitrange->usInWidth * grabctl->usScaleN / grabctl->usScaleM
   	                            - fitrange->usOutWidth) / 2 + 1;
        	grabctl->usStartY = (fitrange->usInHeight * grabctl->usScaleN / grabctl->usScaleM
                                - fitrange->usOutHeight) / 2 + 1;
		    grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
    		grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
       #endif
                    
    	}
    	
    } 
    else {
    #if USB_LYNC_TEST_MODE 
        MMP_USHORT new_x_ratio ;
        RATIO_TBL *new_x_tbl ;
        new_x_ratio = ( ratio_item->ratio * lync_ratio ) / 100 ;
        new_x_tbl = MMPF_Scaler_GetRatioTblByRatio(pipe,new_x_ratio,MMP_FALSE);
        grabctl->usScaleN = 0;
        grabctl->usScaleM = 0;
        grabctl->usScaleXN = new_x_tbl->n ;
        grabctl->usScaleXM = new_x_tbl->m ;
        grabctl->usScaleYN = ratio_item->n ;
        grabctl->usScaleYM = ratio_item->m ;
    	x_scale = grabctl->usScaleXN ;
    	y_scale = grabctl->usScaleYN ;
        if (x_scale > grabctl->usScaleXM ) {
	    	grabctl->usStartX = ((fitrange->usInWidth - 1) * grabctl->usScaleXN / grabctl->usScaleXM
		    	                    - fitrange->usOutWidth) / 2 + 1;
		    grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
        }
        else {
        	grabctl->usStartX = (fitrange->usInWidth * grabctl->usScaleXN / grabctl->usScaleXM
   	                            - fitrange->usOutWidth) / 2 + 1;
		    grabctl->usEndX = grabctl->usStartX + fitrange->usOutWidth - 1;
        }
        #if SENSOR_IN_H > SENSOR_16_9_H
        if (y_scale > grabctl->usScaleYM ) {
	    	grabctl->usStartY = ((scaler_ref_h - 1) * grabctl->usScaleYN / grabctl->usScaleYM
								    - fitrange->usOutHeight) / 2 + 1;
            grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        }
        else {
        	grabctl->usStartY = (scaler_ref_h * grabctl->usScaleYN / grabctl->usScaleYM
                                - fitrange->usOutHeight) / 2 + 1;
    		grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        }
        
        #else
        if (y_scale > grabctl->usScaleYM ) {
	    	grabctl->usStartY = ((fitrange->usInHeight - 1) * grabctl->usScaleYN / grabctl->usScaleYM
								    - fitrange->usOutHeight) / 2 + 1;
            grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        }
        else {
        	grabctl->usStartY = (fitrange->usInHeight * grabctl->usScaleYN / grabctl->usScaleYM
                                - fitrange->usOutHeight) / 2 + 1;
    		grabctl->usEndY = grabctl->usStartY + fitrange->usOutHeight - 1;
        }
        #endif
        
       // dbg_printf(3,"Lync xy ratio : %d\r\n",(new_x_tbl->ratio * 100) / ratio_item->ratio );
    #endif    
    }
    return  MMP_ERR_NONE;    
}
#endif


MMP_ERR MMPF_Scaler_AccessGrabArea(MMP_BOOL set,MMPF_SCALER_PATH pathsel,MMPF_SCALER_GRABCONTROL *grabin,MMPF_SCALER_GRABCONTROL *grabout)
{
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    AIT_REG_W   *OprGrabInHStart, *OprGrabInHEnd, *OprGrabInVStart, *OprGrabInVEnd;
    AIT_REG_W   *OprGrabOutHStart, *OprGrabOutHEnd, *OprGrabOutVStart, *OprGrabOutVEnd;
    
    if (pathsel == MMPF_SCALER_PATH_0) {
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_0_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_0_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_0_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_0_V_ED);
        OprGrabOutHStart= &(pSCAL->SCAL_GRAB_OUT_0_H_ST);
        OprGrabOutHEnd  = &(pSCAL->SCAL_GRAB_OUT_0_H_ED);
        OprGrabOutVStart= &(pSCAL->SCAL_GRAB_OUT_0_V_ST);
        OprGrabOutVEnd  = &(pSCAL->SCAL_GRAB_OUT_0_V_ED);
    }
    else if (pathsel == MMPF_SCALER_PATH_1) {
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_1_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_1_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_1_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_1_V_ED);
        OprGrabOutHStart= &(pSCAL->SCAL_GRAB_OUT_1_H_ST);
        OprGrabOutHEnd  = &(pSCAL->SCAL_GRAB_OUT_1_H_ED);
        OprGrabOutVStart= &(pSCAL->SCAL_GRAB_OUT_1_V_ST);
        OprGrabOutVEnd  = &(pSCAL->SCAL_GRAB_OUT_1_V_ED);
    }
    else {
        OprGrabInHStart = &(pSCAL->SCAL_GRAB_SCAL_2_H_ST);
        OprGrabInHEnd   = &(pSCAL->SCAL_GRAB_SCAL_2_H_ED);
        OprGrabInVStart = &(pSCAL->SCAL_GRAB_SCAL_2_V_ST);
        OprGrabInVEnd   = &(pSCAL->SCAL_GRAB_SCAL_2_V_ED);
        OprGrabOutHStart= &(pSCAL->SCAL_GRAB_OUT_2_H_ST);
        OprGrabOutHEnd  = &(pSCAL->SCAL_GRAB_OUT_2_H_ED);
        OprGrabOutVStart= &(pSCAL->SCAL_GRAB_OUT_2_V_ST);
        OprGrabOutVEnd  = &(pSCAL->SCAL_GRAB_OUT_2_V_ED);
    }

    if(set) {
        if(grabin) {
            *OprGrabInHStart    = grabin->usStartX;
            *OprGrabInHEnd      = grabin->usEndX;
            *OprGrabInVStart    = grabin->usStartY;
            *OprGrabInVEnd      = grabin->usEndY;
        }
        if(grabout) {
            *OprGrabOutHStart   = grabout->usStartX;
            *OprGrabOutHEnd     = grabout->usEndX;
            *OprGrabOutVStart  = grabout->usStartY;
            *OprGrabOutVEnd     = grabout->usEndY;
        }
    }
    else {
        if(grabin) {
            grabin->usStartX = *OprGrabInHStart ;
            grabin->usEndX   = *OprGrabInHEnd   ;
            grabin->usStartY = *OprGrabInVStart ;
            grabin->usEndY   = *OprGrabInVEnd   ;
        }
        if(grabout) {
            grabout->usStartX = *OprGrabOutHStart ;
            grabout->usEndX   = *OprGrabOutHEnd   ;
            grabout->usStartY = *OprGrabOutVStart ;
            grabout->usEndY   = *OprGrabOutVEnd   ;
        }
        
        
    }

    return MMP_ERR_NONE;
}


MMP_BOOL MMPF_Scaler_IfGrabInput(MMPF_SCALER_PATH pathsel,MMP_USHORT w,MMP_USHORT n,MMP_USHORT m)
{
    MMP_USHORT max_scale_width ;
    
    if( !m || !n) {
        return MMP_FALSE;
    }
	if (pathsel == MMPF_SCALER_PATH_0) {
        max_scale_width = SCALER_PATH0_MAX_WIDTH;	
	}
	else if (pathsel == MMPF_SCALER_PATH_1) {
        max_scale_width = SCALER_PATH1_MAX_WIDTH;	
	}
    else {
        max_scale_width = SCALER_PATH2_MAX_WIDTH;	
    }
    if ((w * n / m > max_scale_width)) {
       // dbg_printf(3,"--need grab in\r\n");
        return MMP_TRUE; 
    }    
    return MMP_FALSE ;

}

//
// ratio = ratioN / ratioM
// ratioN  > ratioM : scaling up
// ratioN <= ratioM : scaling down
// return ratioN = 0 & ratioM =0 --> scaler is disabled
MMP_ERR MMPF_Scaler_GetCurRatio(MMPF_SCALER_PATH pathsel,MMP_USHORT *ratioN,MMP_USHORT *ratioM)
{
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    
    if (pathsel == MMPF_SCALER_PATH_0) {
        if(pSCAL->SCAL_SCAL_0_CTL & SCAL_SCAL_PATH_EN) {
            *ratioN = pSCAL->SCAL_SCAL_0_H_N ;
            *ratioM = pSCAL->SCAL_SCAL_0_H_M ;
        }
        else {
            *ratioN = *ratioM = 0;
        }
    }
    else if(pathsel == MMPF_SCALER_PATH_1) {
        if(pSCAL->SCAL_SCAL_1_CTL & SCAL_SCAL_PATH_EN) {
            *ratioN = pSCAL->SCAL_SCAL_1_H_N ;
            *ratioM = pSCAL->SCAL_SCAL_1_H_M ;
        } else {
            *ratioN = *ratioM = 0 ;
        }    
    
    }
    else{
        if(pSCAL->SCAL_SCAL_2_CTL & SCAL_SCAL_PATH_EN) {
            *ratioN = pSCAL->SCAL_SCAL_2_H_N ;
            *ratioM = pSCAL->SCAL_SCAL_2_H_M ;
        } else {
            *ratioN = *ratioM = 0 ;
        }
    }
    return MMP_ERR_NONE;
}
//
// edge_core  : 0~63
// edge_value : 0~63
// 
MMP_ERR MMPF_Scaler_SetEdge(MMPF_SCALER_PATH pathsel,MMP_UBYTE edge_core,MMP_UBYTE edge_val)
{
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    MMPF_Scaler_EnableEdge(pathsel,MMP_TRUE);
    if (pathsel == MMPF_SCALER_PATH_0) {
        pSCAL->SCAL_EDGE_0_CORE = edge_core & 0x3F ;
        pSCAL->SCAL_EDGE_0_GAIN = edge_val  & 0x3F ;
    }
    else if(pathsel == MMPF_SCALER_PATH_1) {
        pSCAL->SCAL_EDGE_1_CORE = edge_core & 0x3F ;
        pSCAL->SCAL_EDGE_1_GAIN = edge_val  & 0x3F ;
    }
    else{
        pSCAL->SCAL_EDGE_2_CORE = edge_core & 0x3F ;
        pSCAL->SCAL_EDGE_2_GAIN = edge_val  & 0x3F ;
    }  
    return MMP_ERR_NONE;  
}

MMP_ERR MMPF_Scaler_GetEdge(MMPF_SCALER_PATH pathsel,MMP_UBYTE *edge_core,MMP_UBYTE *edge_val)
{
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    MMPF_Scaler_EnableEdge(pathsel,MMP_TRUE);
    if (pathsel == MMPF_SCALER_PATH_0) {
        *edge_core = pSCAL->SCAL_EDGE_0_CORE ;
        *edge_val  = pSCAL->SCAL_EDGE_0_GAIN ;
    }
    else if(pathsel == MMPF_SCALER_PATH_1) {
        *edge_core = pSCAL->SCAL_EDGE_1_CORE ;
        *edge_val  = pSCAL->SCAL_EDGE_1_GAIN ;
    }
    else{
        *edge_core = pSCAL->SCAL_EDGE_2_CORE ;
        *edge_val  = pSCAL->SCAL_EDGE_2_GAIN ;
    }  
    return MMP_ERR_NONE;  
}

MMP_ERR MMPF_Scaler_EnableEdge(MMPF_SCALER_PATH pathsel,MMP_BOOL enable)
{
    AITPS_SCAL   pSCAL    = AITC_BASE_SCAL;
    if (pathsel == MMPF_SCALER_PATH_0) {
        if( enable) {
            pSCAL->SCAL_EDGE_0_CTL &= ~SCAL_EDGE_BYPASS ;
        }
        else {
            pSCAL->SCAL_EDGE_0_CTL |= SCAL_EDGE_BYPASS ;
        }
    }
    else if(pathsel == MMPF_SCALER_PATH_1) {
        if( enable) {
            pSCAL->SCAL_EDGE_1_CTL &= ~SCAL_EDGE_BYPASS ;
        }
        else {
            pSCAL->SCAL_EDGE_1_CTL |= SCAL_EDGE_BYPASS ;
        }
    }
    else{
        if( enable) {
            pSCAL->SCAL_EDGE_2_CTL &= ~SCAL_EDGE_BYPASS ;
        }
        else {
            pSCAL->SCAL_EDGE_2_CTL |= SCAL_EDGE_BYPASS ;
        }
    }  
    
}

MMP_ERR MMPF_Scaler_SetRefInHeight(MMP_USHORT scaler_ref_h)
{
    gsScalerRefInH = scaler_ref_h;//SENSOR_16_9_H;//scaler_ref_h ;
}

MMP_ERR MMPF_Scaler_SetRefOutHeight(MMPF_SCALER_PATH pathsel,MMP_USHORT scaler_real_h)
{
    gsScalerRefOutH[pathsel] = scaler_real_h ;
}


MMP_BOOL MMPF_Scaler_IsScalingUp(MMPF_SCALER_PATH pathsel,MMP_USHORT ratio)
{
extern MMP_UBYTE STREAM_EP_H264  ;

  //  MMPF_SCALER_PATH pathsel = MMPF_SCALER_PATH_1 ,end ;
    RATIO_TBL *base_tbl,*new_tbl ;
    MMP_BOOL enable ,scalup[2] = {0,0},check_scal[2] = {0,0};
    PCAM_USB_ZOOM  zoomInfo = { 0,1 ,0,0 };
    MMPF_SCALER_ZOOM_INFO  scalerInfo ;
    STREAM_CFG *cur_pipe ;
    //= usb_get_cur_image_pipe_by_epid(pathsel);
    MMP_UBYTE gra_path ;
    MMP_BOOL lossy_up ;  
    MMP_USHORT scaler_input_h,item_at ,user_ratio;
    MMP_ULONG target_val ;
    if(pathsel==MMPF_SCALER_PATH_1) {
        cur_pipe = usb_get_cur_image_pipe_by_epid(STREAM_EP_H264);
    }
    else {
        cur_pipe = usb_get_cur_image_pipe_by_epid(STREAM_EP_MJPEG);
    }
    


    ratio_tbl_items[pathsel] = MMPF_Scaler_CreateScalerTable();
    lossy_up = MMPF_Video_IsLoop2GRA(cur_pipe)?MMP_TRUE : MMP_FALSE ;
    scaler_input_h = GetScalerInHeight(cur_pipe->pipe_w[pathsel],cur_pipe->pipe_h[pathsel]);
    user_ratio = GetScalerUserRatio(cur_pipe->pipe_w[pathsel],cur_pipe->pipe_h[pathsel]);
    zoomInfo.RangeMin = zoomInfo.RangeMax = ratio ;
    
    scalerInfo.usInputWidth  = SENSOR_16_9_W ;
    scalerInfo.usInputHeight = scaler_input_h ;
    scalerInfo.usSensorWidth = SENSOR_16_9_W ;
    scalerInfo.usSensorHeight = scaler_input_h ;
       
    
    scalerInfo.grabCtl.usStartX = 1;
    scalerInfo.grabCtl.usStartY = 1;
    scalerInfo.grabCtl.usEndX = scalerInfo.grabCtl.usStartX +  cur_pipe->pipe_w[pathsel] - 1 ;
    scalerInfo.grabCtl.usEndY = scalerInfo.grabCtl.usStartY +  cur_pipe->pipe_h[pathsel] - 1 ;
    dbg_printf(3,"<%d,%d> : <%d,%d,%d,%d>\r\n",scalerInfo.usInputWidth,scalerInfo.usInputHeight,
                    scalerInfo.grabCtl.usStartX ,
                    scalerInfo.grabCtl.usStartY ,
                    scalerInfo.grabCtl.usEndX,
                    scalerInfo.grabCtl.usEndY );
                    
    base_tbl = MMPF_Scaler_GetBestM(pathsel,&item_at,scalerInfo.usInputWidth ,scalerInfo.usInputHeight,cur_pipe->pipe_w[pathsel],cur_pipe->pipe_h[pathsel],user_ratio);
    
    
    GetScalerZoomGrab(pathsel,0,&scalerInfo,&zoomInfo,lossy_up);
    
    new_tbl = MMPF_Scaler_GetCurRatioTbl(pathsel);
    
    dbg_printf(0,"#<Scaler %d> : %d / %d\r\n",pathsel,new_tbl->n ,new_tbl->m);
    if (new_tbl->n> new_tbl->m) {
        return MMP_TRUE ;
    }
    return MMP_FALSE ;
}
/// @}