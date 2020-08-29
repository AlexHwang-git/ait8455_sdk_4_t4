//==============================================================================
//
//  File        : sensor_OV9710.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if defined(DSC_MP3_P_FW)||defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_OV9710

#include "lib_retina.h"
#include "reg_retina.h"

#include "mmp_reg_vif.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_gpio.h"
#include "mmp_reg_rawproc.h"

#include "mmpf_sensor.h"
#include "mmpf_hif.h"
#include "mmpf_i2cm.h"
#include "mmpf_vif.h"
#include "isp_if.h"
#include "mmpf_usbpccam.h"

#include "mmp_reg_i2cm.h"
#include "mmps_3gprecd.h"
#include "sensor_OV9710.h"
#include "hdm_ctl.h"
#include "mmpf_dram.h"

extern MMP_USHORT  gsSensorLCModeWidth;
extern MMP_USHORT  gsSensorLCModeHeight;
extern MMP_USHORT  gsSensorMCModeWidth;
extern MMP_USHORT  gsSensorMCModeHeight;
extern MMP_USHORT  gsCurPreviewMode, gsCurPhaseCount;
extern MMP_USHORT  gsSensorMode;
extern MMP_USHORT  m_usVideoQuality;
extern MMP_USHORT	m_gsISPCoreID;

extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_BOOL    gbAutoFocus;

extern MMP_USHORT m_usResolType;

const ISP_UINT8 Sensor_IQ_CompressedText[] = {
#include "isp_845_iq_data_v1_OV9710.xls.ciq.txt"
};
 
#define  SENSOR_ROTATE_180    0   

//#define SUPPORT_AUTO_FOCUS 0

//#include "sensor_ctl.h"
//#include "preview_ctl.h"
//#include "XDATAmap.h"
//#include "3A_ctl.h"
//#include "OV9710.h"
//#include "motor_ctl.h"

#define BASE_SHUTTER_50HZ		248
#define BASE_SHUTTER_60HZ		207

//------------------------------------------------------------------------------
//  Function    : OV9710_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
extern MMP_BYTE gbSignalType;
static void OV9710_Sensor_PreviewMode(MMP_USHORT usPreviewmode)
{
	gsCurPreviewMode = usPreviewmode;
	//gsCurPhaseCount = usPhasecount;

	switch (usPreviewmode) {
	case 0:
		RTNA_DBG_Str(3, "Sensor 5M (1920x1080) preview mode\r\n");
		ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518
		ISP_IF_SNR_SetBeforeCapture();
		ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1920x1080);
		ISP_IF_3A_Control(ISP_3A_ENABLE); //patrick@100518
		break;

	case 1:
		RTNA_DBG_Str(0, "Sensor preview (1280x720) mode\r\n");
		switch (gsSensorMode) {
		case SENSOR_VIDEO_MODE:
			RTNA_DBG_Str(0, "SENSOR_VIDEO_MODE\r\n");
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

			if  (m_VideoFmt == MMPS_3GPRECD_VIDEO_FORMAT_MJPEG)
				MMPF_Sensor_SetColorTransform(0);//  Y:  0-255, U:  0-255, V:  0-255
			else
				MMPF_Sensor_SetColorTransform(1);//	Y: 16-235, U:  0-255, V:  0-255

			ISP_IF_SNR_SetBeforePreview();
			ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x720);

			#if ENABLE_SKYPE_15_30_FPS// only SKyp mode will change fps , other mode will be fixed FPS.
			if(gbSignalType==0)
				ISP_IF_AE_SetFPS(0);
			else
				ISP_IF_AE_SetFPS(30);
			#else
				ISP_IF_AE_SetFPS(30);
			#endif
 			ISP_IF_3A_Control(ISP_3A_ENABLE);
			break;
		#if defined(DSC_MP3_P_FW)||(DSC_R_EN)
		case SENSOR_DSC_MODE:
			RTNA_DBG_Str(3, "Dsc preview mode\r\n");
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

		//	ISP_IF_IQ_SetColorTransform(0);//julian@091111
		if  (m_VideoFmt == MMPS_3GPRECD_VIDEO_FORMAT_MJPEG) //HB
				MMPF_Sensor_SetColorTransform(0);//  Y:  0-255, U:  0-255, V:  0-255
			else
				MMPF_Sensor_SetColorTransform(1);//	Y: 16-235, U:  0-255, V:  0-255
				
			ISP_IF_SNR_SetBeforePreview();
			ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x960);     //Ted 720p 60fps
			//OV9710_Sensor_SetResolution(ISP_SENSOR_RESOL_1920x1080);
			ISP_IF_3A_Control(ISP_3A_ENABLE);
			break;
		#endif
		}
		break;
	}
}

extern MMP_UBYTE   gbDramID;

extern void GPIO_OutputControl(MMP_UBYTE num, MMP_UBYTE status);
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Initialize
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_Initialize_OV9710(void)
{
	//AITPS_PAD   pPAD  = AITC_BASE_PAD;
	AITPS_VIF   pVIF = AITC_BASE_VIF;
	MMP_UBYTE   snr_id = 0;

	RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_OV9710...Begin\r\n");

	//Init globol variable used by DSC
	gsSensorLCModeWidth     = 1280;
	gsSensorLCModeHeight    = 720;//800;
	// use 1.3M for preview, check sensor preview setting
	gsSensorMCModeWidth     = 1280;
	gsSensorMCModeHeight    = 720;//800;

    #if 0
	pPAD->PAD_PSCK = PAD_E8_CURRENT | PAD_E4_CURRENT | PAD_PULL_HIGH;
	pPAD->PAD_PSDA = PAD_E4_CURRENT | PAD_PULL_HIGH;
	pPAD->PAD_PSEN = PAD_E4_CURRENT | PAD_PULL_HIGH;
	#endif

/*
	#if (CUSTOMER == LGT)  
    GPIO_OutputControl(CAM_EN, TRUE);
	MMPF_OS_Sleep_MS(5);
    #endif

	#if (CUSTOMER == NMG)  
    GPIO_OutputControl(CAM_EN, TRUE);
	MMPF_OS_Sleep_MS(5);
    #endif
*/

    if(gbDramID == MMPF_DRAMID_WINBOND_SDR16){
        GPIO_OutputControl(GPIO_SENSOR_PSEN, MMP_TRUE);
    	MMPF_OS_Sleep_MS(5);
        GPIO_OutputControl(GPIO_SENSOR_PSEN, MMP_FALSE);
    }
    else{
	    MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);
	    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
    	MMPF_OS_Sleep_MS(5);
    	MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_FALSE);//Pin output low
    }

	MMPF_OS_Sleep_MS(5);

	pVIF->VIF_SENSR_CLK_CTL[snr_id] = VIF_SENSR_CLK_EN;
	MMPF_OS_Sleep_MS(5);

	/*Reset Sensor 500mS*/
	MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);

	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);
	MMPF_OS_Sleep_MS(5);

	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);
	MMPF_OS_Sleep_MS(5);

	ISP_IF_SNR_Init();


	RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_OV9710...End\r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_OV9710
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_PowerDown_OV9710(void)
{
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	//AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	//AITPS_MIPI pMIPI = AITC_BASE_MIPI;
	MMP_UBYTE   snr_id = 0;

	
	MMPF_VIF_EnableInputInterface(MMP_FALSE);

#if SUPPORT_AUTO_FOCUS
    VR_Motor_EnterStandby();
    //ISP_IF_MOTOR_WriteI2C((ISP_IF_AF_GetPos() >> 2) | 0x80, (ISP_IF_AF_GetPos() << 6) & 0xc0);
	//ISP_IF_AF_SetPos(0);
    //ISP_IF_MOTOR_GoToAFPos();    
	MMPF_OS_Sleep_MS(10);
#endif

    //MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);
	//MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);  //Pin output high
	//MMPF_OS_Sleep_MS(10);
	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);//Pin output low
	MMPF_OS_Sleep_MS(10);

    if(gbDramID == MMPF_DRAMID_WINBOND_SDR16){
        GPIO_OutputControl(GPIO_SENSOR_PSEN, MMP_TRUE);
    }
    else{
	    MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);
	    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
	}
	MMPF_OS_Sleep_MS(5);

	pVIF->VIF_SENSR_CLK_CTL[snr_id] &= (~ VIF_SENSR_CLK_EN) ;   //output sensor main clock

/*
	#if (CUSTOMER == LGT)  
    GPIO_OutputControl(CAM_EN, FALSE);
	MMPF_OS_Sleep_MS(10);
    #endif

	#if (CUSTOMER == NMG)  
    GPIO_OutputControl(CAM_EN, FALSE);
	MMPF_OS_Sleep_MS(10);
    #endif



#if SUPPORT_AUTO_FOCUS
	#if (CUSTOMER == NMG)  
    GPIO_OutputControl(AF_EN, FALSE);
    #endif

	#if (CUSTOMER == LGT)  
    GPIO_OutputControl(AF_EN, TRUE);
    #endif

#endif
*/

#if 0
	//RTNA_DBG_Str(3, "Config PS_GPIO pin as LOW\r\n");
	pGPIO->GPIO_DATA[3] &= ~(1 << 16);  // Turn Off Sensor Power
#endif
	MMPF_OS_Sleep_MS(20);

	RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_OV9710() \r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_OV9710
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_OV9710(MMP_UBYTE ubFrameRate)
{
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_SetPreviewMode_OV9710(MMP_USHORT usPreviewmode)
{
	OV9710_Sensor_PreviewMode(usPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangePreviewMode_OV9710(
						MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
						MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
	OV9710_Sensor_PreviewMode(usNewPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------

/*static*/ MMP_ERR MMPF_Sensor_ChangeMode_OV9710(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
{
	switch (usPreviewmode) {
	case 0:
		switch (usCapturemode) {
		case 0:
			break;
		case 1:
			break;
		}
		break;
	case 1:
		switch (usCapturemode) {
		case 0:// SXGA preview, 5M capture
			MMPF_Sensor_SetPreviewMode_OV9710(0);
			// wait 3 frame ...
		   // MMPF_Sensor_WaitFrame(5);
			break;
		case 1:
			MMPF_Sensor_SetPreviewMode_OV9710(1);
			// wait 3 frame ...
			//MMPF_Sensor_WaitFrame(3);

			break;
		}
		break;
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_InitializeVIF
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_InitializeVIF_OV9710(void)
{
	AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	MMP_UBYTE   vid = 0;
    #endif

	// register sensor (functions and variables)
	ISP_IF_SNR_Register();
	RTNA_DBG_Str(0, "ISP_IF_SNR_Register...E\r\n");

    #if SUPPORT_AUTO_FOCUS
	#if (CUSTOMER == NMG)  
    GPIO_OutputControl(AF_EN, TRUE);
    #endif
	#if (CUSTOMER == LGT)  
    //GPIO_OutputControl(AF_EN, FALSE);
    #endif
	#if (CUSTOMER == CCN)  
    GPIO_OutputControl(AF_EN, FALSE);
    #endif
	MMPF_OS_Sleep_MS(20);
    #endif

	// register motor
	ISP_IF_MOTOR_Register();

    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
	MMPF_VIF_RegisterInputInterface(MMPF_VIF_PARALLEL);
	pVIF->VIF_SENSR_CTL[vid] = VIF_SENSR_LATCH_PST | VIF_VSYNC_POLAR_NEG | VIF_14BPP_OUT_EN;
    #else
    #if 0
	pMIPI->MIPI_DATA1_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA2_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA3_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA4_SOT = 0x10; // From KW
	#endif
	//#if (CUSTOMER == NMG)  
    //pMIPI->MIPI_CLK_DEL = 0x05;  // MIPI clock lane delay selection [3:0]
    //pMIPI->MIPI_CLK_CFG |= 0x02;  // MIPI clock lane delay
    //#endif
	MMPF_VIF_RegisterInputInterface(MMPF_VIF_MIPI);
    #endif

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_InitializeISP
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_InitializeISP_OV9710(void)
{
	volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;

	// check if ISP Lib and IQ are mismatched.
	if (ISP_IF_VER_CheckIQVer() != MMP_ERR_NONE)	
	    return MMP_ERR_NONE;
	//RTNA_DBG_Str(0, "ISP_IF_3A_Register...S\r\n");

	// register 3A
	ISP_IF_3A_Register();
	//RTNA_DBG_Str(0, "ISP_IF_3A_Register...E\r\n");

	//RTNA_DBG_Str(0, "ISP_IF_3A_Init...S\r\n");
	// initialize 3A
	ISP_IF_3A_Init();

	// set AF type (ISP_AF_TYPE_ONE_SHOT / ISP_AF_TYPE_CONTINUOUS)
	ISP_IF_AF_SetType(ISP_AF_TYPE_CONTINUOUS);
	//RTNA_DBG_Str(0, "ISP_IF_3A_Init...E\r\n");

	if (m_gsISPCoreID == 868) {
		// ISP pipeline selection (PYTHON_V1 only)
		//APICAL_XBYTE[APICAL_CTL_REG1] &= ~(0x80); // clear auto size
		pISP[0x0B] = 0x30;
		pISP[0x09] = 0x0C;
	}

	return  MMP_ERR_NONE;
}

//============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetAFPosition_OV9710
//  Description :
// ---------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_OV9710(MMP_UBYTE ubPos)
{
#if 1
	//if (gISPConfig.AutoFocusMode == VENUS_AF_MODE_MANUAL) {
		//gAFConfig.AFPos = ubPos;
		ISP_IF_AF_SetPos(ubPos);
	//}

            //RTNA_DBG_Str(3, "  ubPos = ");       
            //RTNA_DBG_Short(3, ubPos);
            //RTNA_DBG_Str(3, "\r\n");
#endif
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Do3AOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAWBOperation_OV9710(void)
{
	ISP_IF_R_GetAWBAcc();

	ISP_IF_R_DoAWB();

	return  MMP_ERR_NONE;
}

//ISP_UINT32 gISPFrameCnt;
ISP_UINT32 gISPFrameCnt = 0;
#define FrameDivsor 5 // Gason@20120111, by OV suggestion, under 15 fps, dummy line will work every 5 frame.
ISP_UINT32 frame_div = 5;
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_ST
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_ST_OV9710(void)
{
	//static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;
//ISP_IF_AWB_SetPreviewFastMode
	//RTNA_DBG_Str(0, "Do3AOperation_OV9710()\r\n");

	if (ISP_IF_AE_GetLightCond() <= 48)
		frame_div = 6;
	else 
		frame_div = 6; 
		
//	switch (frame_cnt++ % 6) {
	switch (gISPFrameCnt % frame_div) {
	case 0:
		ISP_IF_AE_Execute();
		{
			ISP_UINT32 H, L;
			ISP_UINT32 new_vsync 	= VR_MAX(ISP_IF_AE_GetVsync(), ISP_IF_AE_GetShutter() + 5);  //+8
			ISP_UINT32 new_shutter 	= VR_MIN(VR_MAX(ISP_IF_AE_GetShutter(), 1), new_vsync - 2);//2);
			
		#if SENSOR_ROTATE_180	
			ISP_UINT8  snrReg0x04 	= 0xC8;
		#else
			ISP_UINT8  snrReg0x04	= 0x08;	
		#endif	
			
			again = ISP_IF_AE_GetAGain();
			dgain = ISP_IF_AE_GetDGain();
									
			
			s_gain = again * dgain / ISP_IF_AE_GetDGainBase();
			
			if (s_gain >= ISP_IF_AE_GetAGainBase() * 16) {
				dgain 	= s_gain * ISP_IF_AE_GetDGainBase() / (ISP_IF_AE_GetAGainBase() * 16);
				s_gain  = ISP_IF_AE_GetAGainBase() * 16;
			} else {
				dgain 	= ISP_IF_AE_GetDGainBase();
			}
			
			if (s_gain >= ISP_IF_AE_GetAGainBase() * 8) {
				H = 7;
			} else if (s_gain >= ISP_IF_AE_GetAGainBase() * 4) {
				H = 3;
			} else if (s_gain >= ISP_IF_AE_GetAGainBase() * 2) {
				H = 1;
			} else {
				H = 0;
			}
			
			L = s_gain * 16 / ISP_IF_AE_GetAGainBase() / (H+1) - 16;
			
			if (L > 15) L = 15;
			
			ISP_IF_SNR_WriteI2C(0x04, snrReg0x04+1);	// Group Latch enable
		
			// set sensor gain
			ISP_IF_SNR_WriteI2C(0x00, (H<<4)+L);
				
			// set sensor shutter & vsync		
			ISP_IF_SNR_WriteI2C(0x3E, new_vsync >> 8);
			ISP_IF_SNR_WriteI2C(0x3D, new_vsync);

			ISP_IF_SNR_WriteI2C(0x16, new_shutter >> 8);
			ISP_IF_SNR_WriteI2C(0x10, new_shutter);
		
			ISP_IF_SNR_WriteI2C(0x04, snrReg0x04);		// End of Group Latch			
 			ISP_IF_SNR_WriteI2C(0xFF, 0xFF);			// Group Latch Send
		}
	case 1:		
		ISP_IF_AE_SetDGain(dgain);
		break;
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_END
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_END_OV9710(void)
{
	switch (gISPFrameCnt++ % 6) {
	case 2:
		ISP_IF_R_GetAEAcc();
		break;
	}

	return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_OV9710(void)
{
#if SUPPORT_AUTO_FOCUS
	if(gbAutoFocus == MMP_TRUE){
		//VR_AF_GetAcc();
		ISP_IF_R_GetAFAcc();
		ISP_IF_R_DoAF();
	}
#endif

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoIQOperation_OV9710
//  Description :
//------------------------------------------------------------------------------
//extern MMP_UBYTE gbSkypeMode;
extern H264_FORMAT_TYPE gbCurH264Type;
static MMP_ERR  MMPF_Sensor_DoIQOperation_OV9710(void)
{
	// switch IQ table if video format is H264
	//PCAM_USB_VIDEO_FORMAT vidFmt = pcam_get_info()->pCamVideoFormat;
	
	//if (vidFmt == PCAM_USB_VIDEO_FORMAT_H264)	ISP_IF_F_SetVideoMode(1);
	//else										ISP_IF_F_SetVideoMode(0);

 
	// set IQ at frame end to ensure frame sync	
	ISP_IF_R_DoIQ();

	//RTNA_DBG_Str(0, "DoIQOperation_OV9710()\r\n");
	
	return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_OV9710(MMP_USHORT usMode)
{
#if 0

	RTNA_DBG_Str(0, "MMPF_SetLightFreq_OV9710 : ");
	RTNA_DBG_Short(0, usMode);
	RTNA_DBG_Str(0, "\r\n");

	/*
	typedef enum {
		VENUS_FLICKER_MODE_OFF              = 0,
		VENUS_FLICKER_MODE_AUTO             = 1,
		VENUS_FLICKER_MODE_60HZ             = 2,
		VENUS_FLICKER_MODE_50HZ             = 3,
		VENUS_FLICKER_MODE_CAL_BASESHUTTER  = 4
	} VENUS_FLICKER_MODE;
	*/

	switch(usMode) {
	case 0:
	case 2:
		//gISPConfig.FlickerMode = VENUS_FLICKER_MODE_60HZ;
		ISP_IF_AE_SetFlicker(ISP_AE_FLICKER_60HZ);
		break;
	case 1:
	case 3:
		//gISPConfig.FlickerMode = VENUS_FLICKER_MODE_50HZ;
		ISP_IF_AE_SetFlicker(ISP_AE_FLICKER_50HZ);
		break;
	default:
		//gISPConfig.FlickerMode = VENUS_FLICKER_MODE_AUTO;
		ISP_IF_AE_SetFlicker(ISP_AE_FLICKER_AUTO);
	}
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_SetStableState_OV9710(MMP_BOOL bStableState)
{
	return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_OV9710(MMP_BOOL bHistEQState)
{

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_OV9710(MMP_USHORT usAddr, MMP_USHORT usData)
{
	//MMPF_I2cm_WriteReg(usAddr, usData);
	 ISP_IF_SNR_WriteI2C(usAddr,usData);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_OV9710(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	//MMPF_I2cm_ReadReg(usAddr, usData);
	*usData = ISP_IF_SNR_ReadI2C(usAddr) ;
	return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_OV9710(MMP_USHORT imgSceneID)
{
#if 0
/*
typedef enum {
	ISP_SCENE_AUTO					= 0,	// scene mode
	ISP_SCENE_PORTRAIT				= 1,	// scene mode
	ISP_SCENE_LANDSCAPE				= 2,	// scene mode
	ISP_SCENE_SPORTS				= 3,	// scene mode
	ISP_SCENE_SUNSET				= 4,	// scene mode
	ISP_SCENE_DUSK					= 5,	// scene mode
	ISP_SCENE_DAWN					= 6,	// scene mode
	ISP_SCENE_NIGHT_SHOT			= 7,	// scene mode
	ISP_SCENE_AGAINST_LIGHT			= 8,	// scene mode
	ISP_SCENE_TEXT					= 9,	// scene mode
	ISP_SCENE_MANUAL				= 10,	// scene mode
	ISP_SCENE_INDOOR				= 11,	// scene mode
	ISP_SCENE_SNOW					= 12,	// scene mode
	ISP_SCENE_FALL					= 13,	// scene mode
	ISP_SCENE_WAVE					= 14,	// scene mode
	ISP_SCENE_FIREWORKS				= 15,	// scene mode
	ISP_SCENE_SHOW_WIN				= 16,	// scene mode
	ISP_SCENE_CANDLE				= 17,	// scene mode
	ISP_SCENE_NONE					= 18	// camera mode
} ISP_SCENE;
*/
	imgSceneID = (imgSceneID % (ISP_SCENE_NONE+1));

	RTNA_DBG_Str(3, "MMPF_SetImageScene_OV9710 : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_OV9710(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
{
#if 0
/*
typedef enum {
	AE_MODE_AUTO						= 0,
	AE_MODE_MANUAL						= 1,
	AE_MODE_ISO							= 2,
	AE_MODE_SHUTTER						= 3
} VENUS_AE_MODE;
*/
/*
typedef enum {
	AE_ISO_AUTO					= 0,
	AE_ISO_50					= 1,
	AE_ISO_100					= 2,
	AE_ISO_200					= 3,
	AE_ISO_400					= 4,
	AE_ISO_800					= 5,
	AE_ISO_1600					= 6,
	AE_ISO_3200					= 7,
	ISP_AE_ISO_NUM
} VENUS_AE_ISO;
*/
    #if 0 //FIXME, VSN_V2
	if (ubAEMode == 3)
		ISP_IF_AE_SetISO(ISP_AE_MODE_AUTO);
	else
	#endif
		ISP_IF_AE_SetISO(ubISOMode);
#endif
	
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAFMode_OV9710(MMP_UBYTE ubAFMode)
{
#if 0
/*
typedef enum {
	VENUS_AF_MODE_AUTO					= 0,
	VENUS_AF_MODE_MANUAL				= 1,
	VENUS_AF_MODE_MACRO					= 2,
	VENUS_AF_MODE_FULL					= 3,
	VENUS_AF_MODE_MOTOR_TEST            = 4,
	VENUS_AF_MODE_NULL					= 5
} VENUS_AF_MODE;
*/
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_OV9710 : ");
	RTNA_DBG_Byte(3, ubAFMode);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.AutoFocusMode = (MMP_UBYTE)ubAFMode;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_AF_SetMode(ubAFMode);
#endif

	return  MMP_ERR_NONE;
}

//======================================================================//
// function : MMPF_Sensor_SetImageEffect                                //
// parameters:                                                          //
// return :                                                             //
// description :                                                        //
//======================================================================//
static MMP_ERR  MMPF_Sensor_SetImageEffect_OV9710(MMP_USHORT imgEffectID)
{
#if 0
/*
typedef enum {
	ISP_IMAGE_EFFECT_NORMAL			= 0,
	ISP_IMAGE_EFFECT_GREY			= 1,
	ISP_IMAGE_EFFECT_SEPIA			= 2,
	ISP_IMAGE_EFFECT_NEGATIVE		= 3,
	ISP_IMAGE_EFFECT_ANTIQUE		= 4,
	ISP_IMAGE_EFFECT_WATERCOLOR		= 5,
	ISP_IMAGE_EFFECT_PORTRAIT		= 6,
	ISP_IMAGE_EFFECT_LANDSCAPE		= 7,
	ISP_IMAGE_EFFECT_SUNSET			= 8,
	ISP_IMAGE_EFFECT_DUSK			= 9,
	ISP_IMAGE_EFFECT_DAWN			= 10,
	ISP_IMAGE_EFFECT_RED			= 11,
	ISP_IMAGE_EFFECT_GREEN			= 12,
	ISP_IMAGE_EFFECT_BLUE			= 13,
	ISP_IMAGE_EFFECT_YELLOW			= 15,
	ISP_IMAGE_EFFECT_EMBOSS			= 17,
	ISP_IMAGE_EFFECT_OIL			= 18,
	ISP_IMAGE_EFFECT_BW				= 19,
	ISP_IMAGE_EFFECT_SKETCH			= 20,
	ISP_IMAGE_EFFECT_CRAYONE		= 21,
	ISP_IMAGE_EFFECT_WHITEBOARD		= 22,
	ISP_IMAGE_EFFECT_BLACKBOARD		= 23,
	ISP_IMAGE_EFFECT_VIVID			= 24,
	ISP_IMAGE_EFFECT_NUM
} ISP_IMAGE_EFFECT;
*/

	imgEffectID = (imgEffectID % (ISP_IMAGE_EFFECT_NUM+1));

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_OV9710 : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_OV9710(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_OV9710 : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

	cAWBtype = ubType;
#endif
	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_OV9710(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_OV9710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);
#endif

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_OV9710(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_OV9710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);
#endif

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_OV9710(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_OV9710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_OV9710(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_OV9710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_OV9710(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_OV9710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);
#endif

	return  MMP_ERR_NONE;
}

/*static MMP_ERR  MMPF_Sensor_SetBacklight_OV9710(MMP_UBYTE ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_OV9710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

//	ISP_IF_F_SetBacklight(ubLevel);
	if(ubLevel > 1){
		ISP_IF_F_SetWDR(ISP_WDR_DISABLE);
	}
	else{
		ISP_IF_F_SetWDR(ISP_WDR_ENABLE);
	}
#endif

	return  MMP_ERR_NONE;
}

*/

static void  MMPF_Sensor_SetCaptureISPSetting_OV9710(MMP_UBYTE usCaptureBegin)
{
#if 0
	AITPS_VIF   pVIF = AITC_BASE_VIF;
	MMP_ULONG   ulVIF_Width, ulVIF_Height;
	MMP_UBYTE   vid = 0;

	/* ToDo: setting IOS and ISP windows*/
	if(MMP_TRUE == usCaptureBegin){
		ulVIF_Width = (pVIF->VIF_GRAB[vid].PIXL_ED - pVIF->VIF_GRAB[vid].PIXL_ST + 1);
		ulVIF_Height = (pVIF->VIF_GRAB[vid].LINE_ED - pVIF->VIF_GRAB[vid].LINE_ST + 1);
		//gSensorFunc.SetISPWindows(ulVIF_Width,ulVIF_Height);
	}
#endif
	
	return;
}

/*Prepare for raw preview zoom*/
static void  MMPF_Sensor_SetISPWindow_OV9710(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
	//gSensorFunc.SetISPWindows(ulWidth,ulHeight);
	return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_OV9710(MMP_USHORT usPreviewmode)
{
#if 0
	switch(usPreviewmode){
		case 0:                 //5M can't preview
			return MMP_TRUE;
			break;
		case 1:                 //1.3M can preview
			return MMP_TRUE;
			break;
	}
#endif
	
	return MMP_TRUE;            //5M and 1.3M always can preview
}

static void SetAFWin_OV9710(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
					MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

	//DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_OV9710(MMP_UBYTE enable)
{
//    if (enable) {
		//RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
		ISP_IF_AF_Control(ISP_AF_START);
		//actually calling SNR_OV9710_SetAutoFocusControl()
//    } else {
//        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
//        ISP_IF_AF_Control(ISP_AF_STOP);
//    }
}

static MMP_ERR MMPF_Sensor_SetAEEnable_OV9710(MMP_UBYTE bEnable)
{
	/*
	if (bEnable) {
		g3AConfig.AE_Enabled     = ON;
	}
	else {
		g3AConfig.AE_Enabled     = OFF;
	}*/

	ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AE, bEnable);

	return MMP_ERR_NONE;
}

static MMP_ERR MMPF_Sensor_SetAWBEnable_OV9710(MMP_UBYTE bEnable)
{
#if 0
	if (bEnable) {
		g3AConfig.AWB_Enabled    = ON;
		g3AConfig.AWBRGB_Enabled = ON;
	}
	else {
		g3AConfig.AWB_Enabled    = OFF;
		g3AConfig.AWBRGB_Enabled = OFF;
	}
#endif

	if(bEnable)
		ISP_IF_AWB_SetMode(ISP_AWB_MODE_AUTO);
	else
		ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);

	return MMP_ERR_NONE;
}
static void SetAEsmooth_OV9710(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_OV9710(MMP_UBYTE ev)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_OV9710 : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	RTNA_DBG_Byte(3, ev);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_AE_SetEV(ev);
#endif

	return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_OV9710     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_OV9710(void)
{
	//return 0;
	return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_OV9710  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_OV9710       (void){return 0;}
void  MMPF_Sensor_SetColorID(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	/*
	#if SENSOR_ROTATE_180
	
	switch(RotateType) {
	case MMPF_SENSOR_ROTATE_NO_ROTATE:
		break;
	case MMPF_SENSOR_ROTATE_RIGHT_90:
		Venus_SetColorID(COLOR_ID_ROTATE_90_DEG);
		break;
	case MMPF_SENSOR_ROTATE_RIGHT_180:
		Venus_SetColorID(COLOR_ID_ROTATE_180_DEG);
		break;
	case MMPF_SENSOR_ROTATE_RIGHT_270:
		Venus_SetColorID(COLOR_ID_ROTATE_270_DEG);
		break;
	}
	
	#else
	switch(RotateType) {
	case MMPF_SENSOR_ROTATE_NO_ROTATE:
		break;
	case MMPF_SENSOR_ROTATE_RIGHT_90:
		Venus_SetColorID(COLOR_ID_ROTATE_90_DEG);
		break;
	case MMPF_SENSOR_ROTATE_RIGHT_180:
		Venus_SetColorID(COLOR_ID_ROTATE_180_DEG);
		break;
	case MMPF_SENSOR_ROTATE_RIGHT_270:
		Venus_SetColorID(COLOR_ID_ROTATE_270_DEG);
		break;
	}
	#endif
	*/
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Set3AStatus
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR	MMPF_Sensor_Set3AStatus_OV9710(MMP_BOOL bEnable)
{
	#if 0
	ISP_HDM_IF_3A_SetInterrupt(bEnable);
	#else
	if (bEnable)
		ISP_IF_3A_Control(ISP_3A_ENABLE);
	else
		ISP_IF_3A_Control(ISP_3A_DISABLE);
	#endif
	return	MMP_ERR_NONE;
}

static void  MMPF_Sensor_SetColorID_OV9710(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}

MMPF_SENSOR_FUNCTION  SensorFunction_OV9710 =
{
	MMPF_Sensor_Initialize_OV9710,
	MMPF_Sensor_InitializeVIF_OV9710,
	MMPF_Sensor_InitializeISP_OV9710,
	MMPF_Sensor_PowerDown_OV9710,
	MMPF_Sensor_SetFrameRate_OV9710,
	MMPF_Sensor_ChangeMode_OV9710,
	MMPF_Sensor_ChangePreviewMode_OV9710,
	MMPF_Sensor_SetPreviewMode_OV9710,
	MMPF_Sensor_SetReg_OV9710,
	MMPF_Sensor_GetReg_OV9710,
	MMPF_Sensor_DoAWBOperation_OV9710,
	MMPF_Sensor_DoAEOperation_ST_OV9710,
	MMPF_Sensor_DoAEOperation_END_OV9710,
	MMPF_Sensor_DoAFOperation_OV9710,
	MMPF_Sensor_DoIQOperation_OV9710,
	MMPF_Sensor_SetImageEffect_OV9710,
	MMPF_SetLightFreq_OV9710,
	MMPF_SetStableState_OV9710,
	MMPF_SetHistEQState_OV9710,
	MMPF_Sensor_SetAFPosition_OV9710,
	SetAFWin_OV9710,
	SetAFEnable_OV9710,
	MMPF_Sensor_SetAEEnable_OV9710,
	MMPF_Sensor_SetAWBEnable_OV9710,
	MMPF_Sensor_SetExposureValue_OV9710,
	SetAEsmooth_OV9710,
	MMPF_Sensor_SetImageScene_OV9710,
	GetAFPosition_OV9710,
	GetAFEnable_OV9710,
	GetExposureValue_OV9710,
	GetAEsmooth_OV9710,
	MMPF_Sensor_SetAWBType_OV9710,
	MMPF_Sensor_SetContrast_OV9710,
	MMPF_Sensor_SetSaturation_OV9710,
	MMPF_Sensor_SetSharpness_OV9710,
	MMPF_Sensor_SetHue_OV9710,
	MMPF_Sensor_SetGamma_OV9710,
 //   MMPF_Sensor_SetBacklight_OV9710,
	MMPF_Sensor_SetAEMode_OV9710,
	MMPF_Sensor_SetAFMode_OV9710,
	MMPF_Sensor_SetCaptureISPSetting_OV9710,
	MMPF_Sensor_SetISPWindow_OV9710,
	MMPF_Sensor_CheckPreviewAbility_OV9710,
	MMPF_Sensor_Set3AStatus_OV9710,
	MMPF_Sensor_SetColorID_OV9710
};


MMP_USHORT SNR_OV9710_Reg_Init_Customer[] = {

			//------------------------------------------------------------
			//Sensor 	: OV9715
			//Sensor Rev	: Rev1B
			//------------------------------------------------------------
			//Setting Rev 	: R2.3
			//Resolution	: 1280x800
			//Framerate	: 30fps
			//Clock  : 21MHz PCLK@24MHz MCLK
			//DVDD : external DVDD
			//AEC/AGC/AWB : manual mode
			//------------------------------------------------------------
			//0x12, 0x80,	//Reset
			//---------------------------------------------------------
			//Core Settings
			//---------------------------------------------------------
			0x1e, 0x07,
			0x5f, 0x18,
			0x69, 0x04,
			0x65, 0x2a,
			0x68, 0x0a,
			0x39, 0x28,
			0x4d, 0x90,
			0xc1, 0x80,
			0x0c, 0x30,
			0x6d, 0x02,  // default : 0x82, new : 0x02 to reduce fixed pattern noise

			0xC2, 0x80,	// [0]: Vsync polarity, 0: low active, 1: high active

			//---------------------------------------------------------
			//DSP
			//---------------------------------------------------------
			0x96, 0xf1,	// DSP options enable, 
			            // [0]: enable ISP
			            // [2]: enable the LENC gain coef module
			            // [3]: enable LENC module
			            // [4]: awb enable
			            // [5]: aeb gain function enable
			            // [6]: enable white pixel removal function
			            // [7]: enable black pixel removal function
			0xbc, 0x68,	// [7]   reserved
			
			// [6:5] bd_sel
			// [4]   th_opt
			// [3:0] thresh_hold
			//---------------------------------------------------------
			//Resolution and Format
			//---------------------------------------------------------
			0x12, 0x00, // [6]: verital downsample, [0]:horizontal downsample
			0x3b, 0x00,	// DSP Downsample, 
			0x97, 0x80,	// [7]   smph_mean
			// [6]   reserved
			// [5:4] smph_drop
			//---- Place generated settings here ----;;
			0x17, 0x25,	 // 0x25*8+7 = 0x12F = 303
			0x18, 0xA2,  // 0xA2*8+0 = 0x510 = 1296
			0x19, 0x01,  // 0x01*4+2 = 6
			0x1a, 0xCA,  // 0xCA*4+2 = 0x32A = 810
			0x03, 0x2A,  // [7:5]: VAEC ceiling 1,2,3,7 frames
			0x32, 0x07,
			
			0x0e, 0x40,	 // [3]: VAEC ON/OFF
			
			0x04, 0x08,  // [7]: mirror, [6]: flip
			
			0x98, 0x00,
			0x99, 0x00,
			0x9a, 0x00,
			0x57, 0x00,
			0x58, 0xCA,  // DSP output vertical size = REG58[7:0]:REG57[1:0]
			0x59, 0xA2,  // DSP output horizontal size = REG59[7:0]:REG57[4:2]
			0x4c, 0x13,  // reserved
			0x4b, 0x36,  // reserved
			0x3d, 0x3c,  // R counter End Point LSB
			0x3e, 0x03,  // R counter End Point MSB
			0xbd, 0xA0,	 // yavg_winh
			0xbe, 0xc8,  // yavg_winv
			//---------------------------------------------------------
			//AWB
			//---------------------------------------------------------
			//
			//---------------------------------------------------------
			//Lens Correction
			//---------------------------------------------------------
			//---- Place lens correction settins here ----;;
			// Lens model  	:
			// Module type	:
			//---------------------------------------------------------
			//YAVG
			//---------------------------------------------------------
			//---- Place generated "WIndows Weight" settings here ----;;
			0x4e, 0x55,	//AVERAGE 
			0x4f, 0x55,	//		
			0x50, 0x55,	//
			0x51, 0x55,	//
			0x24, 0x55,	//Exposure windows
			0x25, 0x40,
			0x26, 0xa1,
			//---------------------------------------------------------
			//Clock
			//---------------------------------------------------------
//#if 0 //CCIR_HD_30FPS	// OFF: 15fps, ON: 30fps
			0x5c, 0x59,//0x59,  // [6:5]: PLL pre-divider, [4:0]: PLL multiplier
			             // CLK1 = XCLK/REG5C[6:5], CLK2 = CLK1*(32-REG5C[4:0])  

			0x5d, 0x00,  // CLK3 = CLK2/(REG5D[3:2]+1)
			0x11, 0x00,//0x01  // [5:0] : system clock divider
			             // SYSCLK = CLK3/((REG11[5:0]+1)*2)
			0x2a, 0x9A,
			0x2b, 0x06,	 // line_pixel_width MSB[15:8]
			
			0x2d, 0x00,  // dummy line LSB[7:0]
			0x2e, 0x00,  // dummy line MSB[15:8]
			
			//---------------------------------------------------------
			//General
			//---------------------------------------------------------
			//0x13, 0x85,
			0x14, 0x40,	//Gain Ceiling 8X

			//0x15, 0x08,
			//---------------------------------------------------------
			//Manual Setting for ISP
			//---------------------------------------------------------
			0x13, 0x00,  //AEC[0],AGC[2] :0-manual, 1-auto/LAEC[3] :0-OFF,1-ON
			0x0d, 0x03,//0x03,  //Manual mode for tp level exposure[1] :1'b0-OFF,1'b0-ON
			0x16, 0x02,  //AEC (r16+r10)
			0x10, 0x60,  //
			0x00, 0x00,  //AGC (r00)
			0x96, 0xc1,  //LCC disable[3][2], AWB[4]-AWB en,AWB[5]-AWB gain en : 0-disable,1-enable
			//0xb6, 0x07,  //AWB[2:0]-awb_fast[2],awb_bias_on[1],awb_bias_plus[0]
			0x41, 0x80,  //BLC target[3:0]
            0x37, 0x02,  //BLC always OFF [2]: 0   2011/7/26 

#if 0 //ENABLE_SENSOR_AWB_FUNCTION			
			0x38, 0x10,	 // [4]: AWB_gain write options	 
			0xb6, 0x07,  // [3]: Enable manual AWB bias
			0x96, 0xf1,  // DPC black[7],DPC white[6]:0-OFF,1-ON, [5]: apply AWB gain 
#else
			0x38, 0x00,	 // [4]: AWB_gain write options	 
			0xb6, 0x08,  // [3]: Enable manual AWB bias
//			0x96, 0xe1,  // DPC black[7],DPC white[6]:0-OFF,1-ON, [5]: apply AWB gain 
			0x96, 0x61,  // DPC black[7],DPC white[6]:0-OFF,1-ON, [5]: apply AWB gain 
#endif			

#if (CUSTOMER == LIT)
			0x63, 0x00,  // [2]: 0, enable internal regulator
#else
			0x63, 0x04,  // [2]: 1, bypass internal regulator
#endif

};
void SNR_OV9710_InitSensor_Customer(void) {
	// implement your initialize sensor routine here ==============================================

	ISP_UINT32 i;

	ISP_IF_SNR_ConfigI2C( SENSOR_I2C_ADDR_OV9710, ISP_I2C_TYPE_1A1D);

	// initialize VIF OPRs
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
	ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_PARALLEL, 22);
#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)	
	ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_SINGLE_0, 22);
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)		
	ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_DUAL_01, 22);
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)		
	ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_QUAD, 22);
#endif	

	//ISP_HDM_IF_EnableSensor(ISP_HDM_SNR_ENABLE_HIGH);

	ISP_IF_SNR_WriteI2C(0x12, 0x80);  // reset
	MMPF_OS_Sleep_MS(50); 

	// set register...........
	RTNA_DBG_Str(0, "set sensor reg\r\n");
	for (i = 0; i < sizeof(SNR_OV9710_Reg_Init_Customer)/4; i++) {
		ISP_IF_SNR_WriteI2C(SNR_OV9710_Reg_Init_Customer[i*2], SNR_OV9710_Reg_Init_Customer[i*2+1]);
		MMPF_OS_Sleep_MS(1);
	}
	MMPF_OS_Sleep_MS(20);
	ISP_IF_AE_SetBaseShutter( BASE_SHUTTER_50HZ, BASE_SHUTTER_60HZ);

	// Set preview resolution as default
	ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x720);
}


void SNR_OV9710_SetSensorGain_Customer(ISP_UINT32 gain)
{
	ISP_UINT16 s_gain;
	ISP_UINT32 H, L;

	s_gain = VR_MIN(VR_MAX(gain, 64), 511); // API input gain range : 64~511, 64=1X

	if (s_gain >= 64 * 8) {
		H = 7;
	} else if (s_gain >= 64 * 4) {
		H = 3;
	} else if (s_gain >= 64 * 2) {
		H = 1;
	} else {
		H = 0;
	}
	
	L = s_gain * 16 / 64 / (H+1) - 16;
	
	if (L > 15) L = 15;	

	// set sensor gain
	ISP_IF_SNR_WriteI2C(0x00, (H<<4)+L);
}

void SNR_OV9710_SetSensorShutter_Customer(ISP_UINT32 shutter, ISP_UINT32 vsync)
{
		ISP_UINT32 new_vsync 	= VR_MAX(vsync, shutter + 5);  //+8
		ISP_UINT32 new_shutter 	= VR_MIN(VR_MAX(shutter, 1), new_vsync - 2);//2);
			
		ISP_IF_SNR_WriteI2C(0x3E, new_vsync >> 8);
		ISP_IF_SNR_WriteI2C(0x3D, new_vsync);

		ISP_IF_SNR_WriteI2C(0x16, new_shutter >> 8);
		ISP_IF_SNR_WriteI2C(0x10, new_shutter);

}

void SNR_OV9710_SetSensorAddMode_Customer(ISP_UINT8 enable)
{

}
MMP_USHORT SNR_OV9710_Reg_1280x720_Customer[] = 
{

			//---------------------------------------------------------
			//Resolution and Format
			//---------------------------------------------------------
			0x12, 0x00, // [6]: verital downsample, [0]:horizontal downsample
			0x3b, 0x00,	// DSP Downsample, 
			0x97, 0x80,	// [7]   smph_mean
			// [6]   reserved
			// [5:4] smph_drop

#if SENSOR_ROTATE_180

			//---- Place generated settings here ----;;
			0x17, 0x25,	 // 0x25*8+7 = 0x12F = 303
			0x18, 0xA2,  // 0xA2*8+0 = 0x510 = 1296
			0x19, 0x00,  // 0x00*4+2 = 2
			0x1a, 0xCA,  // 0xCA*4+2 = 0x32A = 810
			0x03, 0x2A,  // [7:5]: VAEC ceiling 1,2,3,7 frames
			0x32, 0x07,
			
			0x04, 0xC8,  // [7]: mirror, [6]: flip

#else
			
			//---- Place generated settings here ----;;
			0x17, 0x25,	 // 0x25*8+7 = 0x12F = 303
			0x18, 0xA2,  // 0xA2*8+0 = 0x510 = 1296
			0x19, 0x01,  // 0x01*4+2 = 6
			0x1a, 0xCA,  // 0xCA*4+2 = 0x32A = 810
			0x03, 0x2A,  // [7:5]: VAEC ceiling 1,2,3,7 frames
			0x32, 0x07,
			
			0x04, 0x08,  // [7]: mirror, [6]: flip

#endif			
};

void SNR_OV9710_SetSensorResolution_Customer(ISP_SENSOR_RESOL res)
{
	// implement your change sensor resolution routine here =======================================

	MMP_ULONG VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;
    MMP_ULONG i;

	switch (res) {
	case ISP_SENSOR_RESOL_1280x960:
	case ISP_SENSOR_RESOL_1280x720:

		// set register...........
		for (i = 0; i < VR_ARRSIZE(SNR_OV9710_Reg_1280x720_Customer)/2; i++) {
			ISP_IF_SNR_WriteI2C(SNR_OV9710_Reg_1280x720_Customer[i*2], SNR_OV9710_Reg_1280x720_Customer[i*2+1]);
		}

		// set target fps and corresponding sensor vsync
		ISP_IF_SNR_SetFPS(828, 30);//bossino fix m2ts AV sync @0928 ISP_IF_SNR_SetFPS(0x0348, 30);

		// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = 1288;
		VIFGrab_V_Start     = 1 + 40;
		VIFGrab_V_Length    = 722;

		ISP_IF_SNR_SetVIFGrab(VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length);

		// set scaler downsample rate (for scaler calculation)
		ISP_IF_SNR_SetDownSample(1, 1);

		// set color id
        #if SENSOR_ROTATE_180
		ISP_IF_SNR_SetColorID(3);
        #else
		ISP_IF_SNR_SetColorID(0);// mirror is 3
        #endif		

		// set sensor grab range
		ISP_IF_SNR_SetSensorGrab(	0,					//H_Offest
									0,					//V_Offest
									VIFGrab_H_Length,	//H_Length
									VIFGrab_V_Length,	//V_Length
									1,					//H_Scale
									1,					//V_Scale
									VIFGrab_H_Length,	//H_Output
									VIFGrab_V_Length);	//V_Output

		//MMPF_Do_Mirror_Flip();


		break;
	}

	// end of your implementation =================================================================

	// set VIF OPR (Actual sensor output size)
	{
		AITPS_VIF  pVIF = AITC_BASE_VIF;
		volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;
		MMP_UBYTE   vid = 0;

		pVIF->VIF_GRAB[vid].PIXL_ST = VIFGrab_H_Start;
		pVIF->VIF_GRAB[vid].PIXL_ED = VIFGrab_H_Start + VIFGrab_H_Length -1;
		pVIF->VIF_GRAB[vid].LINE_ST = VIFGrab_V_Start ;
		pVIF->VIF_GRAB[vid].LINE_ED = VIFGrab_V_Start + VIFGrab_V_Length -1;
		pVIF->VIF_INT_LINE_NUM_0[vid] = VIFGrab_V_Length - 61;//121;

		if (m_gsISPCoreID == 868) {
			// ISP pipeline selection
			//APICAL_XBYTE[APICAL_CTL_REG1] &= ~(0x80); // clear auto size
			pISP[0x0B] = 0x30;
			pISP[0x09] = 0x0C;

			// IP ColorID setting
			//pISP[0x09] &= ~(0xF0);
			//pISP[0x09] |= (ISP_IF_SNR_GetColorID() == 0 ? 3 : (ISP_IF_SNR_GetColorID() == 1 ? 2 : (ISP_IF_SNR_GetColorID() == 2 ? 1: 0))) * (0x50);
		}
	}
}

void SNR_OV9710_DoAWB_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;

//	switch (frame_cnt++ % FrameDivsor) {
switch (gISPFrameCnt % frame_div) {
//	switch (gISPFrameCnt++ % 6) {
	case 3:
	case 5:
		ISP_IF_AWB_Execute();
		ISP_IF_AWB_SetDGain();
		break;
	}
}

void SNR_OV9710_DoAF_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;

	switch (frame_cnt++ % 1) {
//	switch (gISPFrameCnt++ % 1) {
	case 0:
		//ISP_IF_AF_Execute();
		break;
	}
}

void MOTOR_OV9710_VCM_InitMotor_Customer(void)
{
//	uint16 ret;
}


#if (SENSOR_ID_OV9710== 0)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_OV9710;
#endif

#if (SENSOR_ID_OV9710== 1)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_OV9710;
#endif

#endif  //BIND_SENSOR_OV9710
#endif
