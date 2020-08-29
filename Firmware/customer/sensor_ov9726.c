//==============================================================================
//
//  File        : sensor_OV9726.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if defined(DSC_MP3_P_FW)||defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_OV9726

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
#include "sensor_OV9726.h"
#include "hdm_ctl.h"
#include "mmpf_dram.h"

#define G2_CLK                      (120)
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
#include "isp_845_iq_data_v1_OV9726.xls.ciq.txt"
};
 
#define  SENSOR_ROTATE_180    0   

#define  PIXEL_RATE_60M      (1)
#define  PIXEL_RATE_42M      (2)

#define VOLTAGE33V 0
#define VOLTAGE28V 1
#define VOLTAGE18V 2

#define SENSOR_VOLTAGE      VOLTAGE18V
//#if (G1_CLK < 120000)
    #define  SENSOR_PIXEL_RATE          (PIXEL_RATE_42M)
//#else
//    #define  SENSOR_PIXEL_RATE          (PIXEL_RATE_60M)
//#endif

//#define SUPPORT_AUTO_FOCUS 0

//#include "sensor_ctl.h"
//#include "preview_ctl.h"
//#include "XDATAmap.h"
//#include "3A_ctl.h"
//#include "OV9726.h"
//#include "motor_ctl.h"

#define BASE_SHUTTER_50HZ		251
#define BASE_SHUTTER_60HZ		209

//------------------------------------------------------------------------------
//  Function    : OV9726_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
extern MMP_BYTE gbSignalType;
static void OV9726_Sensor_PreviewMode(MMP_USHORT usPreviewmode)
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
			//OV9726_Sensor_SetResolution(ISP_SENSOR_RESOL_1920x1080);
			ISP_IF_3A_Control(ISP_3A_ENABLE);
			break;
		#endif
		}
		break;
	}
}
extern void GPIO_OutputControl(MMP_UBYTE num, MMP_UBYTE status);
extern MMP_UBYTE   gbDramID;
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Initialize
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_Initialize_OV9726(void)
{
	//AITPS_PAD   pPAD  = AITC_BASE_PAD;
	AITPS_VIF   pVIF = AITC_BASE_VIF;
	MMP_UBYTE   snr_id = 0;

	RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_OV9726...Begin\r\n");

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


	RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_OV9726...End\r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_OV9726
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_PowerDown_OV9726(void)
{
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	//AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	//AITPS_MIPI pMIPI = AITC_BASE_MIPI;
	MMP_UBYTE   snr_id = 0;

	
	ISP_HDM_IF_SNR_WriteI2C(0x0100,0);
	MMPF_OS_Sleep_MS(33);
	
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

	RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_OV9726() \r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_OV9726
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_OV9726(MMP_UBYTE ubFrameRate)
{
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_SetPreviewMode_OV9726(MMP_USHORT usPreviewmode)
{
	OV9726_Sensor_PreviewMode(usPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangePreviewMode_OV9726(
						MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
						MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
	OV9726_Sensor_PreviewMode(usNewPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------

/*static*/ MMP_ERR MMPF_Sensor_ChangeMode_OV9726(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
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
			MMPF_Sensor_SetPreviewMode_OV9726(0);
			// wait 3 frame ...
		   // MMPF_Sensor_WaitFrame(5);
			break;
		case 1:
			MMPF_Sensor_SetPreviewMode_OV9726(1);
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
static MMP_ERR  MMPF_Sensor_InitializeVIF_OV9726(void)
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
	pVIF->VIF_SENSR_CTL[vid] = VIF_SENSR_LATCH_NEG | VIF_HSYNC_POLAR | VIF_12BPP_OUT_EN;
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
static MMP_ERR  MMPF_Sensor_InitializeISP_OV9726(void)
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
//  Function    : MMPF_Sensor_SetAFPosition_OV9726
//  Description :
// ---------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_OV9726(MMP_UBYTE ubPos)
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
static MMP_ERR  MMPF_Sensor_DoAWBOperation_OV9726(void)
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
static MMP_ERR  MMPF_Sensor_DoAEOperation_ST_OV9726(void)
{
	static ISP_UINT32 again, dgain, s_gain, shutter, vsync;

	if (ISP_IF_AE_GetLightCond() <= 48)
		frame_div = 5;
	else 
		frame_div = 5; 
		
//	switch (frame_cnt++ % 6) {
	switch (gISPFrameCnt % frame_div) {
	case 0:
		ISP_IF_AE_Execute();
		{
			ISP_UINT32 H, L;
			ISP_UINT32 new_vsync   = ISP_IF_AE_GetVsync();	
			ISP_UINT32 new_shutter = ISP_IF_AE_GetShutter();
			
			again = ISP_IF_AE_GetAGain();
			dgain = ISP_IF_AE_GetDGain();
									
			
			
			//if fix 15fps, set vsnyc & shutter to 1/15s.
			if (ISP_IF_AE_GetBaseShutter() == BASE_SHUTTER_50HZ) {
				vsync   = VR_MAX(new_vsync, new_shutter + 5);
				shutter = VR_MIN(VR_MAX(new_shutter, 1), vsync - 2);

				if (new_shutter >= 6*ISP_IF_AE_GetBaseShutter())
					vsync = 6*ISP_IF_AE_GetBaseShutter() + 5;
/*				
(*(MMP_UBYTE*)0x80000580) = vsync & 0xff;
(*(MMP_UBYTE*)0x80000581) = (vsync>>8) & 0xff;
(*(MMP_UBYTE*)0x80000582) = 0xc1;
*/
			}
			else {
			
				if (ISP_IF_AE_GetFPS() == 15) {
					again   = VR_MAX(ISP_IF_AE_GetShutter() * again / (8 * ISP_IF_AE_GetBaseShutter()), ISP_IF_AE_GetAGainBase());
					shutter = 8 * ISP_IF_AE_GetBaseShutter();
					vsync   = shutter + 5;
				}
				else {
					vsync   = VR_MAX(new_vsync, new_shutter + 5);
					shutter = VR_MIN(VR_MAX(new_shutter, 1), vsync - 2);
				}
			}

			//calc sensor gain
			s_gain = again * dgain / ISP_IF_AE_GetDGainBase();
			
			if (s_gain >= ISP_IF_AE_GetAGainBase() * 16) {
				dgain 	= s_gain * ISP_IF_AE_GetDGainBase() / 16;
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
			ISP_HDM_IF_SNR_WriteI2C(0x0104, 1); // Gason@20120111,by OV suggestion, group hold for sync shutter & gain
			
			ISP_HDM_IF_SNR_WriteI2C(0x0340, vsync >> 8);
			ISP_HDM_IF_SNR_WriteI2C(0x0341, vsync);
			ISP_HDM_IF_SNR_WriteI2C(0x0202, shutter >> 8);
			ISP_HDM_IF_SNR_WriteI2C(0x0203, shutter);				
			ISP_HDM_IF_SNR_WriteI2C(0x0205, (H<<4)+L);

			ISP_HDM_IF_SNR_WriteI2C(0x0104, 0);// Gason@20120111, by OV suggestion, group release for sync shutter & gain
		}
		break;
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_END
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_END_OV9726(void)
{
	switch (gISPFrameCnt++ % frame_div) {
	case 1:
		ISP_IF_SNR_SetDGain(ISP_IF_AE_GetDGainBase());
		break;
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
static MMP_ERR  MMPF_Sensor_DoAFOperation_OV9726(void)
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
//  Function    : MMPF_Sensor_DoIQOperation_OV9726
//  Description :
//------------------------------------------------------------------------------
//extern MMP_UBYTE gbSkypeMode;
extern H264_FORMAT_TYPE gbCurH264Type;
static MMP_ERR  MMPF_Sensor_DoIQOperation_OV9726(void)
{
	// switch IQ table if video format is H264
	//PCAM_USB_VIDEO_FORMAT vidFmt = pcam_get_info()->pCamVideoFormat;
	
	//if (vidFmt == PCAM_USB_VIDEO_FORMAT_H264)	ISP_IF_F_SetVideoMode(1);
	//else										ISP_IF_F_SetVideoMode(0);

    #if 0 //FIXME vsn_v2 not exist
    if (gbCurH264Type ==  FRAMEBASE_H264_YUY2) //REAL Skype
        ISP_IF_F_SetVideoMode(1); // Skype mode ¢DI1
    else          
        ISP_IF_F_SetVideoMode(0); // Other mode ¢DI 0
 	#endif
 
	// set IQ at frame end to ensure frame sync	
	ISP_IF_R_DoIQ();

	//RTNA_DBG_Str(0, "DoIQOperation_OV9726()\r\n");
	
	return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_OV9726(MMP_USHORT usMode)
{
#if 0

	RTNA_DBG_Str(0, "MMPF_SetLightFreq_OV9726 : ");
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

static MMP_ERR  MMPF_SetStableState_OV9726(MMP_BOOL bStableState)
{
	return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_OV9726(MMP_BOOL bHistEQState)
{

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_OV9726(MMP_USHORT usAddr, MMP_USHORT usData)
{
	//MMPF_I2cm_WriteReg(usAddr, usData);
	 ISP_HDM_IF_SNR_WriteI2C(usAddr,usData);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_OV9726(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	//MMPF_I2cm_ReadReg(usAddr, usData);
	*usData = ISP_HDM_IF_SNR_ReadI2C(usAddr) ;
	return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_OV9726(MMP_USHORT imgSceneID)
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

	RTNA_DBG_Str(3, "MMPF_SetImageScene_OV9726 : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_OV9726(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
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

static MMP_ERR  MMPF_Sensor_SetAFMode_OV9726(MMP_UBYTE ubAFMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_OV9726 : ");
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
static MMP_ERR  MMPF_Sensor_SetImageEffect_OV9726(MMP_USHORT imgEffectID)
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

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_OV9726 : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_OV9726(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_OV9726 : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

	cAWBtype = ubType;
#endif
	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_OV9726(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_OV9726 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);
#endif

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_OV9726(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_OV9726 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);
#endif

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_OV9726(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_OV9726 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_OV9726(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_OV9726 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);
#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_OV9726(MMP_SHORT ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_OV9726 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);
#endif

	return  MMP_ERR_NONE;
}

/*static MMP_ERR  MMPF_Sensor_SetBacklight_OV9726(MMP_UBYTE ubLevel)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_OV9726 : ");
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

static void  MMPF_Sensor_SetCaptureISPSetting_OV9726(MMP_UBYTE usCaptureBegin)
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
static void  MMPF_Sensor_SetISPWindow_OV9726(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
	//gSensorFunc.SetISPWindows(ulWidth,ulHeight);
	return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_OV9726(MMP_USHORT usPreviewmode)
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

static void SetAFWin_OV9726(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
					MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

	//DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_OV9726(MMP_UBYTE enable)
{
//    if (enable) {
		//RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
		ISP_IF_AF_Control(ISP_AF_START);
		//actually calling SNR_OV9726_SetAutoFocusControl()
//    } else {
//        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
//        ISP_IF_AF_Control(ISP_AF_STOP);
//    }
}

static MMP_ERR MMPF_Sensor_SetAEEnable_OV9726(MMP_UBYTE bEnable)
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

static MMP_ERR MMPF_Sensor_SetAWBEnable_OV9726(MMP_UBYTE bEnable)
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
static void SetAEsmooth_OV9726(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_OV9726(MMP_UBYTE ev)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_OV9726 : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	RTNA_DBG_Byte(3, ev);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_AE_SetEV(ev);
#endif

	return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_OV9726     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_OV9726(void)
{
	//return 0;
	return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_OV9726  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_OV9726       (void){return 0;}
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
static MMP_ERR	MMPF_Sensor_Set3AStatus_OV9726(MMP_BOOL bEnable)
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

static void  MMPF_Sensor_SetColorID_OV9726(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}

MMPF_SENSOR_FUNCTION  SensorFunction_OV9726 =
{
	MMPF_Sensor_Initialize_OV9726,
	MMPF_Sensor_InitializeVIF_OV9726,
	MMPF_Sensor_InitializeISP_OV9726,
	MMPF_Sensor_PowerDown_OV9726,
	MMPF_Sensor_SetFrameRate_OV9726,
	MMPF_Sensor_ChangeMode_OV9726,
	MMPF_Sensor_ChangePreviewMode_OV9726,
	MMPF_Sensor_SetPreviewMode_OV9726,
	MMPF_Sensor_SetReg_OV9726,
	MMPF_Sensor_GetReg_OV9726,
	MMPF_Sensor_DoAWBOperation_OV9726,
	MMPF_Sensor_DoAEOperation_ST_OV9726,
	MMPF_Sensor_DoAEOperation_END_OV9726,
	MMPF_Sensor_DoAFOperation_OV9726,
	MMPF_Sensor_DoIQOperation_OV9726,
	MMPF_Sensor_SetImageEffect_OV9726,
	MMPF_SetLightFreq_OV9726,
	MMPF_SetStableState_OV9726,
	MMPF_SetHistEQState_OV9726,
	MMPF_Sensor_SetAFPosition_OV9726,
	SetAFWin_OV9726,
	SetAFEnable_OV9726,
	MMPF_Sensor_SetAEEnable_OV9726,
	MMPF_Sensor_SetAWBEnable_OV9726,
	MMPF_Sensor_SetExposureValue_OV9726,
	SetAEsmooth_OV9726,
	MMPF_Sensor_SetImageScene_OV9726,
	GetAFPosition_OV9726,
	GetAFEnable_OV9726,
	GetExposureValue_OV9726,
	GetAEsmooth_OV9726,
	MMPF_Sensor_SetAWBType_OV9726,
	MMPF_Sensor_SetContrast_OV9726,
	MMPF_Sensor_SetSaturation_OV9726,
	MMPF_Sensor_SetSharpness_OV9726,
	MMPF_Sensor_SetHue_OV9726,
	MMPF_Sensor_SetGamma_OV9726,
 //   MMPF_Sensor_SetBacklight_OV9726,
	MMPF_Sensor_SetAEMode_OV9726,
	MMPF_Sensor_SetAFMode_OV9726,
	MMPF_Sensor_SetCaptureISPSetting_OV9726,
	MMPF_Sensor_SetISPWindow_OV9726,
	MMPF_Sensor_CheckPreviewAbility_OV9726,
	MMPF_Sensor_Set3AStatus_OV9726,
	MMPF_Sensor_SetColorID_OV9726
};


MMP_USHORT SNR_OV9726_Reg_Init_Customer[] = {


		// @@ WXGA_RAW_30FPS MIPI 1280x720 (720P)

		0x0103, 0x01,
		//0x3021, 0x20,  // [5]: 0: use internal 1.5V LDO(default), 1: use external 1.5V LDO 
		0x3026, 0x00,
		0x3027, 0x00,
		0x3002, 0xe8,
		0x3004, 0x03,
		0x3005, 0xff,
		
		0x3703, 0x42,  // reserved
		0x3704, 0x10,  // reserved
		0x3705, 0x45,  // reserved
		0x3603, 0xaa,  // reserved
		0x3632, 0x2f,  // reserved
		0x3620, 0x66,  // reserved
		0x3621, 0xc0,  // reserved
		
//        0x0340, 0x03,  // frame_length_lines[15:8]
//        0x0341, 0x48,  // frame_length_lines[7:0]
//        0x0342, 0x06,  // line_length_pck[15:8]
//        0x0343, 0x80,  // line_length_pck[7:0]
//        0x0202, 0x03,
//        0x0203, 0x43,
		
		0x3833, 0x04,  // reserved
		0x3835, 0x02,  // reserved
		0x4702, 0x04,  // reserved
		0x4704, 0x00,
		0x4706, 0x08,
		0x5052, 0x01,  // reserved
		0x3819, 0x6e,  // reserved
		0x3817, 0x94,  // reserved
		0x3a18, 0x00,  // AEC gain ceiling
		0x3a19, 0x7f,  // AEC gain ceiling
		0x404e, 0x7e,  // reserved
#if SENSOR_VOLTAGE == VOLTAGE33V
		0x3631, 0x70,  // reserved
		0x3633, 0x90,  // reserved
#endif
#if (SENSOR_VOLTAGE == VOLTAGE28V) || (SENSOR_VOLTAGE == VOLTAGE18V)
		0x3631, 0x52,  // reserved
		0x3633, 0x50,  // reserved
#endif
		0x3630, 0xd2,  // reserved
		0x3604, 0x08,  // reserved
		0x3601, 0x40,  // reserved
		0x3602, 0x14,  // reserved
		0x3610, 0xa0,  // reserved
		0x3612, 0x20,  // reserved
		
		0x034c, 0x05,  // 1296
		0x034d, 0x10,
		0x034e, 0x03,  // 736
		0x034f, 0x28,
//		0x0347, 0x28,
		 
		
#if (G2_CLK == 120)
#if (SENSOR_PIXEL_RATE == PIXEL_RATE_60M)  // for 60M pixel/sec
		0x0340, 0x03,  // frame_length_lines[15:8]
		0x0341, 0x48,  // frame_length_lines[7:0]
		
		0x0342, 0x07,  // line_length_pck[15:8]  // for input clock = 24MHz @ 30fps
		0x0343, 0x72,  // line_length_pck[7:0]
#endif		
#if (SENSOR_PIXEL_RATE == PIXEL_RATE_42M)  // for 42M pixel/sec
		0x0340, 0x03,  // frame_length_lines[15:8]
		0x0341, 0x48,  // frame_length_lines[7:0]
		
		0x0342, 0x06,  // line_length_pck[15:8]  // for input clock = 24MHz @ 30fps
		0x0343, 0x82,  // line_length_pck[7:0]
#endif
#endif
#if (G2_CLK == 132)
		0x0340, 0x03,  // frame_length_lines[15:8]
		0x0341, 0x48,  // frame_length_lines[7:0]
		0x0342, 0x06,  // line_length_pck[15:8]  // for input clock = 22MHz @ 30fps
		0x0343, 0xD4,  // line_length_pck[7:0]
#endif
		
		0x0202, 0x03,
		0x0203, 0x43,
		
		0x0301, 0x0A,  // vt_pix_clk_div
		0x0303, 0x01,  // vt_sys_clk_div

		0x3002, 0x00,
		0x3004, 0x00,
		0x3005, 0x00,
		
		0x4801, 0x0f,
		0x4803, 0x05,
		0x4601, 0x16,
		0x3014, 0x05,  // [2]: MIPI enable
#if (SENSOR_PIXEL_RATE == PIXEL_RATE_60M)  // 60M pixels/sec
		0x3104, 0x80,  // use two PLLs, PLL1 and PLL2
#endif
#if (SENSOR_PIXEL_RATE == PIXEL_RATE_42M)  // 42M pixels/sec	
		0x3104, 0x20,  // use single PLL
#endif
		// tomy@2010_05_18, reduce MIPI high speed prepare period
		0x4827, 0x03, // hs_prepare_min, default = 0x32
		0x4802, 0x80, // [7]: hs_prepare_sel

#if (SENSOR_PIXEL_RATE == PIXEL_RATE_60M)  // 60M pixels/sec
		0x0305, 0x04,
		0x0307, 0x64, //0x64, // change 60M/sec to 42M/sec
		0x300c, 0x02,  // reserved
		0x300d, 0x20,  // reserved
#endif		
#if (SENSOR_PIXEL_RATE == PIXEL_RATE_42M)  // 42M pixels/sec	
		0x0305, 0x04,
		0x0307, 0x46, //0x64, // change 60M/sec to 42M/sec
		0x300c, 0x02,  // reserved
		0x300d, 0x1c,  // reserved
#endif
		0x300e, 0x01,  // reserved
		0x3010, 0x01,  // reserved
		0x460e, 0x81,  // [7]: MIPI mode 2 enable
		0x0101, 0x01,  // [0]: mirror on/off, [1]: flip on/off
		
		0x3707, 0x14,  // reserved
		0x3622, 0x9f,  // reserved
		0x3400, 0x05,
		// Sensor function disable
		0x3a00, 0x08,  // night mode off
		0x5047, 0x63,  // [0]: BLC enable, [1]: AWB gain enable, [5]: LENC bias plus
		0x4002, 0x45,  // [6]: BLC auto enable
		0x5000, 0x02,  // [1]: WPC enable, [2]: BPC enable, [7]: LENC correction enable
		0x5001, 0x00,  // [0]: AWB enable
		0x3406, 0x01,  // AWB disable
		0x3503, 0x13,  // AGC/AEC disable
		0x4005, 0x18,  // [4]: output black line enable
		0x0100, 0x01,  // streaming

/*
		0x3a0f, 0x64,
		0x3a10, 0x54,
		0x3a11, 0xc2,
		0x3a1b, 0x64,
		0x3a1e, 0x54,
		0x3a1a, 0x05,
*/


};
void SNR_OV9726_InitSensor_Customer(void) {
	// implement your initialize sensor routine here ==============================================

	ISP_UINT32 i;

	ISP_IF_SNR_ConfigI2C( SENSOR_I2C_ADDR_OV9726, ISP_I2C_TYPE_2A1D);

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

	// set register...........
	RTNA_DBG_Str(0, "set sensor reg\r\n");
	for (i = 0; i < sizeof(SNR_OV9726_Reg_Init_Customer)/4; i++) {
		ISP_HDM_IF_SNR_WriteI2C(SNR_OV9726_Reg_Init_Customer[i*2], SNR_OV9726_Reg_Init_Customer[i*2+1]);
		MMPF_OS_Sleep_MS(3);
	}
	MMPF_OS_Sleep_MS(20);
	ISP_IF_AE_SetBaseShutter( BASE_SHUTTER_50HZ, BASE_SHUTTER_60HZ);

	// Set preview resolution as default
	ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x720);
}

const ISP_UINT8 get_OV9726_gain_Customer[128] = 
{
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,

	0x00, //0x10
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x07,
	0x08,
	0x09,
	0x0a,
	0x0b,
	0x0c,
	0x0d,
	0x0e,
	0x0f,


	0x11,//0x20
	0x11,
	0x12,
	0x12,
	0x13,
	0x13,
	0x14,
	0x14,
	0x15,
	0x15,
	0x16,
	0x16,
	0x17,
	0x17,
	0x18,
	0x18,


	0x19,//0x30
	0x19,
	0x1a,
	0x1a,
	0x1b,
	0x1b,
	0x1c,
	0x1c,
	0x1d,
	0x1d,
	0x1e,
	0x1e,
	0x1f,
	0x1f,
	0x31,
	0x31,

	0x31,//0x40
	0x31,
	0x31,
	0x31,
	0x32,
	0x32,
	0x32,
	0x32,
	0x33,
	0x33,
	0x33,
	0x33,
	0x34,
	0x34,
	0x34,
	0x34,

	0x35,//0x50
	0x35,
	0x35,
	0x35,
	0x36,
	0x36,
	0x36,
	0x36,
	0x37,
	0x37,
	0x37,
	0x37,
	0x38,
	0x38,
	0x38,
	0x38,

	0x39,//0x60
	0x39,
	0x39,
	0x39,
	0x3a,
	0x3a,
	0x3a,
	0x3a,
	0x3b,
	0x3b,
	0x3b,
	0x3b,
	0x3c,
	0x3c,
	0x3c,
	0x3c,

	0x3d,//0x70
	0x3d,
	0x3d,
	0x3d,
	0x3e,
	0x3e,
	0x3e,
	0x3e,
	0x3f,
	0x3f,
	0x3f,
	0x3f,
	0x71,
	0x71,
	0x71,
	0x71,
};


void SNR_OV9726_SetSensorGain_Customer(ISP_UINT32 gain)
{
	ISP_UINT16 s_gain;
	ISP_UINT32 sensor_gain;

	s_gain = VR_MIN(16 * VR_MAX(gain, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);
	sensor_gain = get_OV9726_gain_Customer[s_gain];
	ISP_HDM_IF_SNR_WriteI2C(0x0205, sensor_gain & 0xff);
}

void SNR_OV9726_SetSensorShutter_Customer(ISP_UINT32 shutter, ISP_UINT32 vsync)
{
		ISP_UINT32 new_vsync =  VR_MAX(vsync, shutter + 5);  //+8
		ISP_UINT32 new_shutter = VR_MIN(VR_MAX(shutter, 1), new_vsync - 2);
	//	ISP_UINT32 ExtraVsyncWidth;

		ISP_HDM_IF_SNR_WriteI2C(0x0340, new_vsync >> 8);
		ISP_HDM_IF_SNR_WriteI2C(0x0341, new_vsync);

		ISP_HDM_IF_SNR_WriteI2C(0x0202, new_shutter >> 8);
		ISP_HDM_IF_SNR_WriteI2C(0x0203, new_shutter);


}

void SNR_OV9726_SetSensorAddMode_Customer(ISP_UINT8 enable)
{

}
#if 0
MMP_USHORT SNR_OV9726_Reg_1280x720_Customer[] = 
{
		// @@ WXGA_RAW_30FPS MIPI 1280x720 (720P)

		0x0103, 0x01,
		0x3026, 0x00,
		0x3027, 0x00,
		0x3002, 0xe8,
		0x3004, 0x03,
		0x3005, 0xff,
		
		0x3703, 0x42,  // reserved
		0x3704, 0x10,  // reserved
		0x3705, 0x45,  // reserved
		0x3603, 0xaa,  // reserved
		0x3632, 0x2f,  // reserved
		0x3620, 0x66,  // reserved
		0x3621, 0xc0,  // reserved
		
//        0x0340, 0x03,  // frame_length_lines[15:8]
//        0x0341, 0x48,  // frame_length_lines[7:0]
//        0x0342, 0x06,  // line_length_pck[15:8]
//        0x0343, 0x80,  // line_length_pck[7:0]
//        0x0202, 0x03,
//        0x0203, 0x43,
		
		0x3833, 0x04,  // reserved
		0x3835, 0x02,  // reserved
		0x4702, 0x04,  // reserved
		0x4704, 0x00,
		0x4706, 0x08,
		0x5052, 0x01,  // reserved
		0x3819, 0x6e,  // reserved
		0x3817, 0x94,  // reserved
		0x3a18, 0x00,  // AEC gain ceiling
		0x3a19, 0x7f,  // AEC gain ceiling
		0x404e, 0x7e,  // reserved
		0x3631, 0x70,  // reserved
		0x3633, 0x90,  // reserved
		0x3630, 0xd2,  // reserved
		0x3604, 0x08,  // reserved
		0x3601, 0x40,  // reserved
		0x3602, 0x14,  // reserved
		0x3610, 0xa0,  // reserved
		0x3612, 0x20,  // reserved

#if 0
		0x034c, 0x05,  // 1296
		0x034d, 0x10,
		0x034e, 0x02,  // 736
		0x034f, 0xe0,
		0x0347, 0x28,
#else
		0x034c, 0x05,  // 1296
		0x034d, 0x10,
		0x034e, 0x03,  // 808
		0x034f, 0x28,
		//0x0347, 0x28,

#endif
		 
		0x0340, 0x03,  // frame_length_lines[15:8]
		0x0341, 0x48,  // frame_length_lines[7:0]
		
#if 1//(G2_CLK == 144)
		0x0342, 0x07,  // line_length_pck[15:8]  // for input clock = 24MHz @ 30fps
		0x0343, 0x72,  // line_length_pck[7:0]
#endif
#if 0//(G2_CLK == 132)
		0x0342, 0x06,  // line_length_pck[15:8]  // for input clock = 22MHz @ 30fps
		0x0343, 0xD4,  // line_length_pck[7:0]
#endif
//        0x0342, 0x05,  // line_length_pck[15:8]  // for input clock = 18MHz @ 30fps
//        0x0343, 0x96,  // line_length_pck[7:0]
		
		0x0202, 0x03,
		0x0203, 0x43,
		
		0x0301, 0x0A,  // vt_pix_clk_div
		0x0303, 0x01,  // vt_sys_clk_div

		0x3002, 0x00,
		0x3004, 0x00,
		0x3005, 0x00,
		
		0x4801, 0x0f,
		0x4803, 0x05,
		0x4601, 0x16,
		0x3014, 0x05,  // [2]: MIPI enable
		0x3104, 0x80,  // reserved

		// tomy@2010_05_18, reduce MIPI high speed prepare period
		0x4827, 0x03, // hs_prepare_min, default = 0x32
		0x4802, 0x80, // [7]: hs_prepare_sel
		
		0x0305, 0x04,
		0x0307, 0x64,
		
		0x300c, 0x02,  // reserved
		0x300d, 0x20,  // reserved
		0x300e, 0x01,  // reserved
		0x3010, 0x01,  // reserved
		0x460e, 0x81,  // [7]: MIPI mode 2 enable
		0x0101, 0x01,  // [0]: mirror on/off, [1]: flip on/off
		
		0x3707, 0x14,  // reserved
		0x3622, 0x9f,  // reserved

#if 0
		0x0601, 0x02,  // test pattern
#endif


		// Sensor function disable
		0x3a00, 0x08,//0x78,  // night mode off
		0x5047, 0x63,  // [0]: BLC enable, [1]: AWB gain enable, [5]: LENC bias plus
		0x4002, 0x45,  // [6]: BLC auto enable
		0x5000, 0x06,  // [1]: WPC enable, [2]: BPC enable, [7]: LENC correction enable
		0x5001, 0x00,  // [0]: AWB enable
		0x3406, 0x01,  // AWB disable
		0x3503, 0x13,  // AGC/AEC disable
		0x4005, 0x18,  // [4]: output black line enable
		0x0100, 0x01,  // streaming


};
#endif
//extern void MMPF_Do_Mirror_Flip();
void SNR_OV9726_SetSensorResolution_Customer(ISP_SENSOR_RESOL res)
{
	// implement your change sensor resolution routine here =======================================

	MMP_ULONG VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;


	switch (res) {
	case ISP_SENSOR_RESOL_1280x960:
	case ISP_SENSOR_RESOL_1280x720:

		// set register...........
	/*	for (i = 0; i < VR_ARRSIZE(SNR_OV9726_Reg_1280x720_Customer)/2; i++) {
			ISP_HDM_IF_SNR_WriteI2C(SNR_OV9726_Reg_1280x720_Customer[i*2], SNR_OV9726_Reg_1280x720_Customer[i*2+1]);
		}
*/
		// set target fps and corresponding sensor vsync
		ISP_IF_SNR_SetFPS(0x0348, 30);//bossino fix m2ts AV sync @0928 ISP_IF_SNR_SetFPS(0x0348, 30);

		// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 7;
		VIFGrab_H_Length    = 1282;
		VIFGrab_V_Start     = 43;
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

void SNR_OV9726_DoAWB_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;

 ISP_UINT8 Fdiv;
 Fdiv = (frame_div >= 6) ? 10 : 3;
//	switch (frame_cnt++ % FrameDivsor) {
//  switch (gISPFrameCnt % frame_div) {
switch (gISPFrameCnt % Fdiv) {
//	switch (gISPFrameCnt++ % 6) {
 //case 3:
 //case 5:
 case 2:
		ISP_IF_AWB_Execute();
		ISP_IF_AWB_SetDGain();
		break;
	}
}

void SNR_OV9726_DoAF_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;

	switch (frame_cnt++ % 1) {
//	switch (gISPFrameCnt++ % 1) {
	case 0:
		//ISP_IF_AF_Execute();
		break;
	}
}

void MOTOR_OV9726_VCM_InitMotor_Customer(void)
{
//	uint16 ret;
}


#if (SENSOR_ID_OV9726== 0)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_OV9726;
#endif

#if (SENSOR_ID_OV9726== 1)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_OV9726;
#endif

#endif  //BIND_SENSOR_OV9726
#endif
