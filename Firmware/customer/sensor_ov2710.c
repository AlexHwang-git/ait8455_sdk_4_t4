//==============================================================================
//
//  File        : sensor_OV2710.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if defined(DSC_MP3_P_FW)||defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_OV2710

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

#include "mmp_reg_i2cm.h"
#include "mmps_3gprecd.h"

#include "hdm_ctl.h"

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


const ISP_UINT8 Sensor_IQ_CompressedText[] = {
#if(CHIP == P_V2)
#include "isp_842_iq_data_v1_OV2710.xls.qlz.txt"
#endif
};

//#define SUPPORT_AUTO_FOCUS 0

//#include "sensor_ctl.h"
//#include "preview_ctl.h"
//#include "XDATAmap.h"
//#include "3A_ctl.h"
//#include "OV2710.h"
//#include "motor_ctl.h"

//------------------------------------------------------------------------------
//  Function    : OV2710_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------

static void OV2710_Sensor_PreviewMode(MMP_USHORT usPreviewmode)
{
	gsCurPreviewMode = usPreviewmode;
	//gsCurPhaseCount = usPhasecount;

	switch (usPreviewmode) {
	case 0:
		RTNA_DBG_Str(3, "Sensor 5M (1920x1080) preview mode\r\n");
		ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

		ISP_IF_IQ_SetColorTransform(0);//julian@091111
		ISP_IF_SNR_SetBeforeCapture();
		ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1920x1080);

		ISP_IF_3A_Control(ISP_3A_ENABLE); //patrick@100518
		break;

	case 1:
		RTNA_DBG_Str(3, "Sensor preview (1920x1080) mode\r\n");
		switch (gsSensorMode) {
		#if defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(VIDEO_R_EN)
		case SENSOR_VIDEO_MODE:
			RTNA_DBG_Str(3, "SENSOR_VIDEO_MODE\r\n");
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

//				ISP_IF_IQ_SetColorTransform(1);//julian@091111

			if  (m_VideoFmt == MMPS_3GPRECD_VIDEO_FORMAT_MJPEG) //HB
				ISP_IF_IQ_SetColorTransform(0);//julian@091111
			else
				ISP_IF_IQ_SetColorTransform(0);  // for YUV422/MJPEG mode, tomy@2010_04_30

			ISP_IF_SNR_SetBeforePreview();
			ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1920x1080);

			//ISP_IF_AE_SetFPS(30);
			ISP_IF_3A_Control(ISP_3A_ENABLE);
			break;
		#endif
		#if defined(DSC_MP3_P_FW)||(DSC_R_EN)
		case SENSOR_DSC_MODE:
			RTNA_DBG_Str(3, "Dsc preview mode\r\n");
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

			ISP_IF_IQ_SetColorTransform(0);//julian@091111
			ISP_IF_SNR_SetBeforePreview();
			ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x960);     //Ted 720p 60fps
			//ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1920x1080);
			ISP_IF_3A_Control(ISP_3A_ENABLE);
			break;
		#endif
		}
		break;
	}
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Initialize
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_Initialize_OV2710(void)
{
//  MMP_USHORT usData;
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	AITPS_PAD   pPAD  = AITC_BASE_PAD;
	AITPS_VIF  pVIF = AITC_BASE_VIF;

	RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_OV2710...Begin\r\n");
#if 1
	//RTNA_DBG_Str(0, "Config PS_GPIO pin as pull high IO Pad\r\n");
	// config PS_GPIO as pull high
	//pPAD->PAD_PSGPIO = PAD_E8_CURRENT | PAD_PULL_LOW;
	
	MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);

	#if 1 //PhilipTest@090313
	MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
	//RTNA_WAIT_US((15+125)*10); // 15 us + 6 EXT_CLK (48MHz) = 15 us + 125 ns
	#endif
	
	pGPIO->GPIO_EN[3] |= (1 << 16); // config as output mode
	pGPIO->GPIO_DATA[3] |= (1 << 16);  // Turn On Sensor Power
	RTNA_WAIT_MS(10);

	//Init globol variable used by DSC
	gsSensorLCModeWidth     = 1920;
	gsSensorLCModeHeight    = 1088;//800;
	// use 1.3M for preview, check sensor preview setting
	gsSensorMCModeWidth     = 1920;
	gsSensorMCModeHeight    = 1088;//800;

	pPAD->PAD_PSCK = PAD_E8_CURRENT | PAD_E4_CURRENT | PAD_PULL_HIGH;
	pPAD->PAD_PSDA = PAD_E4_CURRENT | PAD_PULL_HIGH;
	pPAD->PAD_PSEN = PAD_E4_CURRENT | PAD_PULL_HIGH;


	MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_FALSE);//Pin output low
	
	RTNA_WAIT_MS(5);
  
	pVIF->VIF_SENSR_CLK_CTL  = VIF_SENSR_CLK_EN;
	RTNA_WAIT_MS(5);

	/********************************************/
	/*Use GPIO to reset sensor for ev board  */
	/*Caution : It's customer dependent !!!     */
	/********************************************/
	/*Reset Sensor 500mS*/

#endif

	/*Reset Sensor 500mS*/
	#if  (CHIP == P_V2)
	MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);

	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);
	RTNA_WAIT_MS(5);

	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);
	RTNA_WAIT_MS(5);
	#endif

	ISP_IF_SNR_Init();

#if SUPPORT_AUTO_FOCUS &&(AF_EN!=255)
	//RTNA_DBG_Str(0, "Config PGPIO19 pin as pull high IO Pad\r\n");
	// config PGPIO19 as pull high
	pPAD->PAD_GPIO[AF_EN] = AF_EN_PAD_CFG ;
	pGPIO->GPIO_EN[0] |= (1 << AF_EN); // config as output mode
//    pGPIO->GPIO_DATA[0] &= ~(1 << AF_EN);  // Turn On Motor
	pGPIO->GPIO_DATA[0] |= (1 << AF_EN);  // Turn On Motor
	RTNA_WAIT_MS(20);
#endif

	//VR_Motor_Init();

	RTNA_DBG_Str(3, "MMPF_Sensor_Initialize_OV2710...End\r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_OV2710
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_PowerDown_OV2710(void)
{
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	AITPS_MIPI pMIPI = AITC_BASE_MIPI;

	pMIPI->MIPI_DATA1_CFG &= ~(MIPI_CSI2_EN);
	pMIPI->MIPI_DATA2_CFG &= ~(MIPI_CSI2_EN);
	MMPF_VIF_EnableInputInterface(MMP_FALSE);


#if SUPPORT_AUTO_FOCUS && (AF_EN!=255)
	//ISP_IF_MOTOR_MoveToZero();
	//RTNA_WAIT_MS(20);
	//pGPIO->GPIO_DATA[0] |= (1 << AF_EN);  // Turn Off Motor
	pGPIO->GPIO_DATA[0] &= ~(1 << AF_EN);  // Turn Off Motor
#endif


#if (CHIP == P_V2)
	//MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);
	//MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);  //Pin output high
	//RTNA_WAIT_MS(50);
	//MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);//Pin output low
	//RTNA_WAIT_MS(20);
#endif

#if 1
	MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);
	MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
	RTNA_WAIT_MS(5);
#endif

	pVIF->VIF_SENSR_CLK_CTL &= (~ VIF_SENSR_CLK_EN) ;   //output sensor main clock


#if 0
	//RTNA_DBG_Str(3, "Config PS_GPIO pin as LOW\r\n");
	pGPIO->GPIO_DATA[3] &= ~(1 << 16);  // Turn Off Sensor Power
#endif
	RTNA_WAIT_MS(5);

	RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_OV2710() \r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_OV2710
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_OV2710(MMP_UBYTE ubFrameRate)
{
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_SetPreviewMode_OV2710(MMP_USHORT usPreviewmode)
{
	OV2710_Sensor_PreviewMode(usPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangePreviewMode_OV2710(
						MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
						MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
	OV2710_Sensor_PreviewMode(usNewPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------

/*static*/ MMP_ERR MMPF_Sensor_ChangeMode_OV2710(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
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
			MMPF_Sensor_SetPreviewMode_OV2710(0);
			// wait 3 frame ...
		   // MMPF_Sensor_WaitFrame(5);
			break;
		case 1:
			MMPF_Sensor_SetPreviewMode_OV2710(1);
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
static MMP_ERR  MMPF_Sensor_InitializeVIF_OV2710(void)
{
	AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	//RTNA_DBG_Str(0, "ISP_IF_SNR_Register...S\r\n");

	// register sensor (functions and variables)
	ISP_IF_SNR_Register();
	//RTNA_DBG_Str(0, "ISP_IF_SNR_Register...E\r\n");

	// register motor
	ISP_IF_MOTOR_Register();

#if (SENSOR_IF == SENSOR_IF_PARALLEL)	
	MMPF_VIF_RegisterInputInterface(MMPF_VIF_PARALLEL);
	pVIF->VIF_SENSR_CTL      = VIF_SENSR_LATCH_NEG | VIF_HSYNC_POLAR | VIF_12BPP_OUT_EN;
#else
	pMIPI->MIPI_DATA1_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA2_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA3_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA4_SOT = 0x10; // From KW
	MMPF_VIF_RegisterInputInterface(MMPF_VIF_MIPI);
#endif	


	
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_InitializeISP
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_InitializeISP_OV2710(void)
{
	volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;
	
	// check if ISP Lib and IQ are mismatched.
	if (ISP_IF_VER_CheckIQVer() != MMP_ERR_NONE)	return;
	//RTNA_DBG_Str(0, "ISP_IF_3A_Register...S\r\n");
	
	// register 3A
	ISP_IF_3A_Register();
	//RTNA_DBG_Str(0, "ISP_IF_3A_Register...E\r\n");

	//RTNA_DBG_Str(0, "ISP_IF_3A_Init...S\r\n");
	// initialize 3A
	ISP_IF_3A_Init();
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
//  Function    : MMPF_Sensor_SetAFPosition_OV2710
//  Description :
// ---------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_OV2710(MMP_UBYTE ubPos)
{
#if 1
	//if (gISPConfig.AutoFocusMode == VENUS_AF_MODE_MANUAL) {
		//gAFConfig.AFPos = ubPos;
		ISP_IF_AF_SetPos(ubPos);
	//}
#endif
	return  MMP_ERR_NONE;
}


ISP_UINT32 gISPFrameCnt;
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Do3AOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_Do3AOperation_OV2710(void)
{
	//ISP_IF_R_DoAE();
	//RTNA_DBG_Str(0, "Do3AOperation_OV2710()\r\n");

//	static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;

	switch (gISPFrameCnt % 4) {
	case 0:
		ISP_IF_AE_Execute();

		again = ISP_IF_AE_GetAGain();
		dgain = ISP_IF_AE_GetDGain();
		s_gain = VR_MIN(16 * VR_MAX(again, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);

		// special conversion for exposure parameters could be set here ===========================
		if (again > 0x100) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F0)>>2),1);
			again = again & 0x3F0;
		} else if (again > 0x80) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F8)>>2),1);
			again = again & 0x3F8;
		} else {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)again) / VR_MAX((again & 0x3FC),1);
			again = again & 0x3FC;
		}

		if (s_gain > 0x20 && s_gain < 0x40) {
		   dgain = dgain * s_gain / (s_gain & 0xFE);
		   again = again * (s_gain & 0xFE) /s_gain;
		}

		if (s_gain > 0x40 && s_gain < 0x80) {
		   dgain = dgain * s_gain / (s_gain & 0xFC);
		   again = again * (s_gain & 0xFC) /s_gain;
		}
		// end of special conversion ==============================================================

		ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		break;

	case 2:
		//ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		ISP_IF_SNR_SetAGain(again);
		break;
		
	case 3:
		ISP_IF_AWB_Execute();
		break;	
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Set3AOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_Set3AOperation_OV2710(void)
{
	//RTNA_DBG_Str(0, "Do3AOperation_OV2710()\r\n");

//	static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;
	
	// get Acc first
	ISP_IF_R_GetAEAcc();
	if(ISP_IF_AWB_GetVer().MajorVer != 0x03){	ISP_IF_R_GetAWBAcc();	}

	switch (++gISPFrameCnt % 4) {
	case 3:
		again = ISP_IF_AE_GetAGain();
		dgain = ISP_IF_AE_GetDGain();
		s_gain = VR_MIN(16 * VR_MAX(again, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);

		// special conversion for exposure parameters could be set here ===========================
		if (again > 0x100) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F0)>>2),1);
			again = again & 0x3F0;
		} else if (again > 0x80) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F8)>>2),1);
			again = again & 0x3F8;
		} else {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)again) / VR_MAX((again & 0x3FC),1);
			again = again & 0x3FC;
		}

		if (s_gain > 0x20 && s_gain < 0x40) {
		   dgain = dgain * s_gain / (s_gain & 0xFE);
		   again = again * (s_gain & 0xFE) /s_gain;
		}

		if (s_gain > 0x40 && s_gain < 0x80) {
		   dgain = dgain * s_gain / (s_gain & 0xFC);
		   again = again * (s_gain & 0xFC) /s_gain;
		}
		// end of special conversion ==============================================================

		ISP_IF_SNR_SetDGain(dgain);
		ISP_IF_SNR_SetAddMode(ISP_IF_AE_GetAdditionMode());
		
		ISP_IF_AWB_SetDGain();
		break;
	}

	return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_OV2710(void)
{
#if SUPPORT_AUTO_FOCUS
	if(gbAutoFocus == MMP_TRUE){
		//VR_AF_GetAcc();
		ISP_IF_R_GetAFAcc();
		ISP_IF_R_DoAF();
	}
#endif

	//RTNA_DBG_Str(0, "Do3AOperation_OV2710()\r\n");
	
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoIQOperation_OV2710
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoIQOperation_OV2710(void)
{
	// set IQ at frame end to ensure frame sync
	ISP_IF_R_DoIQ();

	//RTNA_DBG_Str(0, "DoIQOperation_OV2710()\r\n");
	
	return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_OV2710(MMP_USHORT usMode)
{
	RTNA_DBG_Str(0, "MMPF_SetLightFreq_OV2710 : ");
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

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_SetStableState_OV2710(MMP_BOOL bStableState)
{
	return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_OV2710(MMP_BOOL bHistEQState)
{

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_OV2710(MMP_USHORT usAddr, MMP_USHORT usData)
{
	//MMPF_I2cm_WriteReg(usAddr, usData);
	 ISP_HDM_IF_SNR_WriteI2C(usAddr,usData);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_OV2710(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	//MMPF_I2cm_ReadReg(usAddr, usData);
	*usData = ISP_HDM_IF_SNR_ReadI2C(usAddr) ;
	return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_OV2710(MMP_USHORT imgSceneID)
{
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

	RTNA_DBG_Str(3, "MMPF_SetImageScene_OV2710 : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_OV2710(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
{
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAEMode_OV2710 : ");
	RTNA_DBG_Byte(3, ubAEMode);
	RTNA_DBG_Str(3, ": ");
	RTNA_DBG_Byte(3, ubISOMode);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//MMPF_Sensor_CheckFrameEnd(1);
	//Venus_SetAEMode(ubAEMode, ubISOMode);
	if (ubAEMode == 3)
		ISP_IF_AE_SetISO(ISP_AE_ISO_AUTO);
	else
		ISP_IF_AE_SetISO(ubISOMode);
	
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAFMode_OV2710(MMP_UBYTE ubAFMode)
{
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_OV2710 : ");
	RTNA_DBG_Byte(3, ubAFMode);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.AutoFocusMode = (MMP_UBYTE)ubAFMode;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_AF_SetMode(ubAFMode);
	return  MMP_ERR_NONE;
}

//======================================================================//
// function : MMPF_Sensor_SetImageEffect                                //
// parameters:                                                          //
// return :                                                             //
// description :                                                        //
//======================================================================//
static MMP_ERR  MMPF_Sensor_SetImageEffect_OV2710(MMP_USHORT imgEffectID)
{
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

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_OV2710 : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_OV2710(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_OV2710 : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

	cAWBtype = ubType;
#endif
	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_OV2710(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_OV2710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_OV2710(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_OV2710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_OV2710(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_OV2710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_OV2710(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_OV2710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_OV2710(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_OV2710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetBacklight_OV2710(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_OV2710 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

/*
	if(ubLevel > 1){
		ISP_IF_F_SetWDR(ISP_WDR_DISABLE);
	}
	else{
		ISP_IF_F_SetWDR(ISP_WDR_ENABLE);
	}
*/

	return  MMP_ERR_NONE;
}



static void  MMPF_Sensor_SetCaptureISPSetting_OV2710(MMP_UBYTE usCaptureBegin)
{
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	MMP_ULONG ulVIF_Width, ulVIF_Height;

	/* ToDo: setting IOS and ISP windows*/
	if(MMP_TRUE == usCaptureBegin){
		ulVIF_Width = (pVIF->VIF_GRAB_PIXL_ED - pVIF->VIF_GRAB_PIXL_ST + 1);
		ulVIF_Height = (pVIF->VIF_GRAB_LINE_ED - pVIF->VIF_GRAB_LINE_ST + 1);
		//gSensorFunc.SetISPWindows(ulVIF_Width,ulVIF_Height);
	}
	
	return;
}

/*Prepare for raw preview zoom*/
static void  MMPF_Sensor_SetISPWindow_OV2710(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
	//gSensorFunc.SetISPWindows(ulWidth,ulHeight);
	return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_OV2710(MMP_USHORT usPreviewmode)
{
	switch(usPreviewmode){
		case 0:                 //5M can't preview
			return MMP_TRUE;
			break;
		case 1:                 //1.3M can preview
			return MMP_TRUE;
			break;
	}
	
	return MMP_TRUE;            //5M and 1.3M always can preview
}

static void SetAFWin_OV2710(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
					MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

	DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_OV2710(MMP_UBYTE enable)
{
#if SUPPORT_AUTO_FOCUS && (AF_EN!=255)
//    if (enable) {
		RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
		ISP_IF_AF_Control(ISP_AF_START);
		//actually calling SNR_OV2710_SetAutoFocusControl()
//    } else {
//        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
//        ISP_IF_AF_Control(ISP_AF_STOP);
//    }
#endif

}

static MMP_ERR MMPF_Sensor_SetAEEnable_OV2710(MMP_UBYTE bEnable)
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

static MMP_ERR MMPF_Sensor_SetAWBEnable_OV2710(MMP_UBYTE bEnable)
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

	ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AWB, bEnable);
	if(bEnable)
		ISP_IF_AWB_SetMode(ISP_AWB_MODE_AUTO);
	else
		ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);

	return MMP_ERR_NONE;
}
static void SetAEsmooth_OV2710(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_OV2710(MMP_UBYTE ev)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_OV2710 : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	RTNA_DBG_Byte(3, ev);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_AE_SetEV(ev);

	return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_OV2710     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_OV2710(void)
{
	//return 0;
	return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_OV2710  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_OV2710       (void){return 0;}
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
static MMP_ERR	MMPF_Sensor_Set3AStatus_OV2710(MMP_BOOL bEnable)
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

static void  MMPF_Sensor_SetColorID_OV2710(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}

MMPF_SENSOR_FUNCTION  SensorFunction_OV2710 =
{
	MMPF_Sensor_Initialize_OV2710,
	MMPF_Sensor_InitializeVIF_OV2710,
	MMPF_Sensor_InitializeISP_OV2710,
	MMPF_Sensor_PowerDown_OV2710,
	MMPF_Sensor_SetFrameRate_OV2710,
	MMPF_Sensor_ChangeMode_OV2710,
	MMPF_Sensor_ChangePreviewMode_OV2710,
	MMPF_Sensor_SetPreviewMode_OV2710,
	MMPF_Sensor_SetReg_OV2710,
	MMPF_Sensor_GetReg_OV2710,
	MMPF_Sensor_Do3AOperation_OV2710,
	MMPF_Sensor_Set3AOperation_OV2710,
	MMPF_Sensor_DoAFOperation_OV2710,
	MMPF_Sensor_DoIQOperation_OV2710,
	MMPF_Sensor_SetImageEffect_OV2710,
	MMPF_SetLightFreq_OV2710,
	MMPF_SetStableState_OV2710,
	MMPF_SetHistEQState_OV2710,
	MMPF_Sensor_SetAFPosition_OV2710,
	SetAFWin_OV2710,
	SetAFEnable_OV2710,
	MMPF_Sensor_SetAEEnable_OV2710,
	MMPF_Sensor_SetAWBEnable_OV2710,
	MMPF_Sensor_SetExposureValue_OV2710,
	SetAEsmooth_OV2710,
	MMPF_Sensor_SetImageScene_OV2710,
	GetAFPosition_OV2710,
	GetAFEnable_OV2710,
	GetExposureValue_OV2710,
	GetAEsmooth_OV2710,
	MMPF_Sensor_SetAWBType_OV2710,
	MMPF_Sensor_SetContrast_OV2710,
	MMPF_Sensor_SetSaturation_OV2710,
	MMPF_Sensor_SetSharpness_OV2710,
	MMPF_Sensor_SetHue_OV2710,
	MMPF_Sensor_SetGamma_OV2710,
 //   MMPF_Sensor_SetBacklight_OV2710,
	MMPF_Sensor_SetAEMode_OV2710,
	MMPF_Sensor_SetAFMode_OV2710,
	MMPF_Sensor_SetCaptureISPSetting_OV2710,
	MMPF_Sensor_SetISPWindow_OV2710,
	MMPF_Sensor_CheckPreviewAbility_OV2710,
	MMPF_Sensor_Set3AStatus_OV2710,
	MMPF_Sensor_SetColorID_OV2710
};


MMP_USHORT SNR_OV2710_Reg_Init_Customer[] = {

		//0x3008, 0x42, // SYSTEM_CTRL0
		0x3103, 0x03, // SCCB_PWUP_DIS
		0x3017, 0x7f, // PAD_OUTPUT ENABLE 1
		0x3018, 0xfc, // PAD_OUTPUT ENABLE 2
		0x3706, 0x61, // ANALOG CONTROL
		//0x3613, 0x44, //
		0x3712, 0x0c, // ANALOG CONTROL
		0x3630, 0x6d, //
#if SENSOR_ROTATE_180
		0x3621, 0x14, // [7]: 1: H-sub, [4]: 0: if 0x3818[6]=1, 1: if 0x3818[6]=0
#else
		0x3621, 0x04, // [7]: 1: H-sub, [4]: 0: if 0x3818[6]=1, 1: if 0x3818[6]=0
#endif
		0x3604, 0x60, //
		0x3603, 0xa7, //
		0x3631, 0x26, //
		0x3600, 0x04, //
		0x3620, 0x37, //
		0x3623, 0x00,
		0x3702, 0x9e, // ANALOG CONTROL
		0x3703, 0x74, // // HREF offset
		0x3704, 0x10, // ANALOG CONTROL
		0x370d, 0x0f,
		0x3713, 0x8b, // ANALOG CONTROL
		0x3714, 0x74,
		0x3710, 0x9e, // ANALOG CONTROL

		//0x3800, 0x01, // HREF Horizontal Start Point High Byte [3:0]
		0x3801, 0xc4,//0x7c, // HREF Horizontal Start Point Low Byte
		//0x3802, 0x00, // HREF Vertical Start Point Low Byte
		//0x3803, 0x0a, // HREF Vertical Start Point Low Byte		==> 0x10

		0x3605, 0x05, //
		0x3606, 0x12, //
		0x302d, 0x90,
		0x370b, 0x40, // ANALOG CONTROL
		0x3716, 0x31,


		0x5181, 0x20,
		0x518f, 0x00,
		0x4301, 0xff,
		0x4303, 0x00,
		0x3a00, 0x00, //0x38, // AEC CONTROL

		0x3a1a, 0x06, // DIFF_MAX
		0x3a18, 0x00,
		0x3a19, 0x7a,
		0x3a13, 0x54,
		0x382e, 0x0f,
		0x381a, 0x1a,//0x00,//0x3c, // TIMING TC HS MIRR ADJ		==> 0x3c
		0x5688, 0x03,
		0x5684, 0x07,
		0x5685, 0xa0,
		0x5686, 0x04,
		0x5687, 0x43,

//#if 0//SENSOR_IF_USING_MIPI
#if (SENSOR_IF != SENSOR_IF_PARALLEL)	
		// for MIPI interface
		0x3017, 0x00,
		0x3018, 0x00,
		0x300e, 0x04, // SC_MIPI_SC_CTRL 0
		0x4801, 0x0f, // MIPI CONTROL 01
#endif

		0x3a0f, 0x40,
		0x3a10, 0x38,
		0x3a1b, 0x48,
		0x3a1e, 0x30,
		0x3a11, 0x90,
		0x3a1f, 0x10,


		0x3500, 0x00, // long_exposure[19:16]
		0x3501, 0x40, // long_exposure[15:8]
		0x3502, 0x00, // long_exposure[7:0]

		0x3503, 0x07, // MANUAL CONTROL, [0]:AEC manual, 0: Auto, 1: manual
					 //                 [1]:AGC manual, 0: Auto, 1: manual
					 //                 [2]:VTS manual, 0: Auto, 1: manual

		0x3406, 0x01,  // manual AWB

		0x350c, 0x00, // VTS_DIFF[15:8], set to zero in manual mode
		0x350d, 0x00, // VTS_DIFF[7:0], set to zero in manual mode
		0x3c01, 0x80, // 5060HZ CONTROL 01
		//0x401c, 0x48, // RSVD
		0x4006, 0x00, // BLC target[9:8]
		0x4007, 0x00, // BLC target[7:0]
		0x4000, 0x01,//0x01, // BLC CONTROL 00, [0]: BLC enable
		0x401d, 0x22,//0x28, // BLC CONTROL 1D

		//0x5046, 0x01, // ISP CONTROL 46, [0]: isp_en, [3]:awbg_en

		//0x3810, 0x00, // TIMING HVOFFS
		//0x3836, 0x00, // TIMING HVPAD
		0x5000, 0x06,//0x02, // ISP CONTROL 00, [1]: wc_en, [2]: bc_en, [7]: lenc_en
		0x5001, 0x00, // ISP CONTROL 01 (AWB), [0]: awb_en
//        0x5002, 0x00, // ISP CONTROL 02, [1]: vap_en
//        0x503d, 0x00, // ISP CONTROL 3D,
//        0x5900, 0x01, // VAP CONTROL 00, [0]: avg_en
//        0x5901, 0x00, // VAP CONTROL 01, [3:2]: hsub_coef, [1:0]: vsub_coef

//        0x5180, 0x00, // AWB CONTROL, [7]: debug mode, [6]: fast_awb

		0x3818, 0x80, // TIMING CONTROL 18, [6]: mirror, [5]: vertical flip, [0]: vsub2, [1]: vsub4
//#if 0//SENSOR_IF_USING_MIPI
#if (SENSOR_IF != SENSOR_IF_PARALLEL)	
		0x3008, 0x04, // Start streaming, [1]:
		0x300e, 0x04, // Start streaming, [1]:
#else
		0x3008, 0x02, // Start streaming, [1]:
		0x300e, 0x02, // Start streaming, [1]:
#endif

		//0x3003, 0x03, // SYSTEM RESET
		//0x4803, 0x50, // MIPI CONTROL 03
		//0x4800, 0x14, // MIPI CONTROL 00
		//0x3815, 0x81, // TIMING TC REG 15
		//0x3003, 0x01, // SYSTEM_RESET 03

#if 0 //SENSOR_IF_USING_MIPI
		// tomy@2010_01_29, reduce MIPI high speed prepare period
		//0x4827, 0x03, // hs_prepare_min, default = 0x32
#endif
		//0x4802, 0x80, // [7]: hs_prepare_sel

//        0x3815, 0x81,
//        0x381c, 0x20, // [4]: r_crop_en, [3:0]: vs_crop[11:8]
//        0x381d, 0x0a,//0x82, // [7:0]: vs_crop[7:0]	==> 0x0a
//        0x381e, 0x01,//0x05, // [3:0]: vh_crop[11:8]	==> 0x01
//        0x381f, 0x20,//0xc0, // [7:0]: vh_crop[7:0]	==> 0x20

//        0x3820, 0x00, // [7]: enable hs crop via x-addr, [4:0]: hs_crop[4:0]
//        0x3821, 0x00,//0x20, // [5:0]: hw_crop[5:0]		==> 0

//        0x3822, 0x03, // [7:0]: ws begin
//        0x3823, 0x03, // [7:0]: ws width
		//0x3824, 0x21, // [5]: hvs_man enable, [4:0]: hrefst_man[12:8]
		//0x3825, 0x2c, // hrefst_man[7:0]


		0x300f, 0x8a, // PLL CONTROL, [2]: R_DIVL 0: one lane, 1: two lanes
		0x3011, 0x0e,//0x0c, // 14*12*4/8 => PCLK = 672/8 = 84MHz
//        0x3011, 0x0a, // 11*10*4 = 440MHz => PCLK = 440/8 = 55MHz
//        0x3011, 0x0f, // 11*15*4 = 660MHz => PCLK = 660/10 = 66MHz
#if 0
		0x3012, 0x02, // [2:0]: R_PREDIV, 010: divide by 2
#else
		0x3012, 0x00, // [2:0]: R_PREDIV, 000: divide by 1
#endif


};
void SNR_OV2710_InitSensor_Customer(void) {
	// implement your initialize sensor routine here ==============================================
	ISP_UINT16 data ;

	ISP_UINT32 i;

	// initialize I2C ID and type
	/*
	VR_SNR_ConfigI2C(	VR_TableN_GetValueByIndex(&gIspTableHeader.Sensor_Misc, 0),
						VR_TableN_GetValueByIndex(&gIspTableHeader.Sensor_Misc, 1));
	*/

	ISP_IF_SNR_ConfigI2C( 0x36, ISP_I2C_TYPE_2A1D);

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

	ISP_HDM_IF_EnableSensor(ISP_HDM_SNR_ENABLE_HIGH);

	ISP_HDM_IF_SNR_WriteI2C(0x3008, 0x82);
	RTNA_WAIT_MS(50); 

	// set register...........
	for (i = 0; i < sizeof(SNR_OV2710_Reg_Init_Customer)/4; i++) {
		ISP_HDM_IF_SNR_WriteI2C(SNR_OV2710_Reg_Init_Customer[i*2], SNR_OV2710_Reg_Init_Customer[i*2+1]);

#if 0
		data = ISP_HDM_IF_SNR_ReadI2C(SNR_OV2710_Reg_Init_Customer[i*2]) ;
		RTNA_DBG_Str3("Reg :");
		RTNA_DBG_Short3(SNR_OV2710_Reg_Init_Customer[i*2]);
		RTNA_DBG_Str3(",");
		RTNA_DBG_Short3(data);
		RTNA_DBG_Str3("\r\n");
#endif
	}

	ISP_IF_AE_SetBaseShutter( 346, 288, 346, 288);

	// Set preview resolution as default
	ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1920x1080);
}

const ISP_UINT8 get_OV2710_gain_Customer[128] = {
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


void SNR_OV2710_SetSensorGain_Customer(ISP_UINT32 gain)
{
	ISP_UINT16 s_gain;
	ISP_UINT32 sensor_gain;

	s_gain = VR_MIN(16 * VR_MAX(gain, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);
	sensor_gain = get_OV2710_gain_Customer[s_gain] ;
	ISP_HDM_IF_SNR_WriteI2C(0x350A, (sensor_gain >> 8) & 0x01);
	ISP_HDM_IF_SNR_WriteI2C(0x350B, sensor_gain & 0xff);

}

void SNR_OV2710_SetSensorShutter_Customer(ISP_UINT32 shutter, ISP_UINT32 vsync)
{
//static int shuttertimes = 30 ;

	// 1 ~ (frame length line - 6)
	//uint32 new_vsync = VR_MIN(VR_MAX(shutter + 6, vsync), 0xFFFF);
	//uint32 new_shutter = VR_MIN(VR_MAX(shutter, 1), new_vsync - 6);
	ISP_UINT32 ExtraVsyncWidth;
	//if(shuttertimes > 0) {
	//    shuttertimes-- ;

	//VR_SNR_WriteI2C(0x3212, 0x00);	// Enable group0


	ISP_HDM_IF_SNR_WriteI2C(0x380E, vsync >> 8);
	ISP_HDM_IF_SNR_WriteI2C(0x380F, vsync);

	if(shutter <= (vsync - 6)){
		ISP_HDM_IF_SNR_WriteI2C(0x350C, 0);
		ISP_HDM_IF_SNR_WriteI2C(0x350D, 0);

		ISP_HDM_IF_SNR_WriteI2C(0x3500, (ISP_UINT8)((shutter >> 12)&0xff));
		ISP_HDM_IF_SNR_WriteI2C(0x3501, (ISP_UINT8)((shutter >> 4)&0xff));
		ISP_HDM_IF_SNR_WriteI2C(0x3502, (ISP_UINT8)((shutter << 4)&0xff));
	}
	else{
		ExtraVsyncWidth = (shutter + 6) - vsync;
		ISP_HDM_IF_SNR_WriteI2C(0x350C, (ISP_UINT8)((ExtraVsyncWidth >> 8) & 0xff));
		ISP_HDM_IF_SNR_WriteI2C(0x350D, (ISP_UINT8)((ExtraVsyncWidth) & 0xff));

		ISP_HDM_IF_SNR_WriteI2C(0x3500, (ISP_UINT8)((shutter >> 12)&0xff));
		ISP_HDM_IF_SNR_WriteI2C(0x3501, (ISP_UINT8)((shutter >> 4)&0xff));
		ISP_HDM_IF_SNR_WriteI2C(0x3502, (ISP_UINT8)((shutter << 4)&0xff));
	}

	//VR_SNR_WriteI2C(0x3212, 0x00);	// End group0
	//VR_SNR_WriteI2C(0x3212, 0xA0);	// Launch group0

	//RTNA_DBG_Str(0,"shuttertimes:");
	//RTNA_DBG_Long(0,shuttertimes);
	//RTNA_DBG_Str(0,"\r\n");
	//}
}

void SNR_OV2710_SetSensorAddMode_Customer(ISP_UINT8 enable)
{
/*	if (ISP_HDM_IF_IsFrameTypePreview()) {
		ISP_UINT8 regval = (enable) ? ISP_HDM_IF_SNR_ReadI2C(0x3016) | 0x40 : ISP_HDM_IF_SNR_ReadI2C(0x3016) & ~0x40;

		ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x01); // grouped_parameter_hold
		ISP_HDM_IF_SNR_WriteI2C(0x3016, regval);
		ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x00); // grouped_parameter_hold
	}
*/
}

MMP_USHORT SNR_OV2710_Reg_1920x1080_Customer[] = {

		0x300f, 0x8a, // PLL CONTROL, [2]: R_DIVL 0: one lane, 1: two lanes, [7:6]: R_SELD5, [1:0]: R_SELD2P5

		0x3010, 0x10, // [7:4]: R_DIVS, [3:0]: R_DIVM

#if SENSOR_ROTATE_180
		0x3621, 0x14, // [7]: 1: H-sub, [4]: 0: if 0x3818[6]=1, 1: if 0x3818[6]=0
#else
		0x3621, 0x04, // [7]: 1: H-sub, [4]: 0: if 0x3818[6]=1, 1: if 0x3818[6]=0
#endif

		0x3804, 0x07, // HREF width = 1928
		0x3805, 0x88,
		0x3806, 0x04, // VREF height = 1092
		0x3807, 0x44,
		0x3808, 0x07, // DVP width = 1928
		0x3809, 0x88,
		0x380a, 0x04, // DVP height = 1092
		0x380b, 0x44,
		0x380c, 0x09, // Total Horizontal Size = 2500
		0x380d, 0x80,//0xC4,
		0x380e, 0x04, // Total Vertical Size = 1120
		0x380f, 0x7F,//0x60,//0x50,
/*
		0x3804, 0x07, // HREF width = 1928
		0x3805, 0x88,
		0x3806, 0x04, // VREF height = 1092
		0x3807, 0x44,
		0x3808, 0x07, // DVP width = 1928
		0x3809, 0x88,
		0x380a, 0x04, // DVP height = 1092
		0x380b, 0x44,
		0x380c, 0x08, // Total Horizontal Size = 2200
		0x380d, 0x98,
		0x380e, 0x04, // Total Vertical Size = 1125
		0x380f, 0x60,//0x50,
*/



//#if SENSOR_ROTATE_180
//		0x3818, 0xe0, // TIMING CONTROL 18, [6]: mirror, [5]: vertical flip, [0]: vsub2, [1]: vsub4
//#else
		0x3818, 0x80, // TIMING CONTROL 18, [6]: mirror, [5]: vertical flip, [0]: vsub2, [1]: vsub4
//#endif
		//0x5002, 0x00, // ISP CONTROL 02, [1]: vap_en
		//0x5900, 0x01, // VAP CONTROL 00, [0]: avg_en
		//0x5901, 0x00, // VAP CONTROL 01, [3:2]: hsub_coef, [1:0]: vsub_coef

};

void SNR_OV2710_SetSensorResolution_Customer(ISP_SENSOR_RESOL res)
{
	// implement your change sensor resolution routine here =======================================

	MMP_ULONG i, VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;
	

	switch (res) {
	case ISP_SENSOR_RESOL_1920x1080:
	case ISP_SENSOR_RESOL_1280x960:
	case ISP_SENSOR_RESOL_1280x720:

		// set register...........
		for (i = 0; i < VR_ARRSIZE(SNR_OV2710_Reg_1920x1080_Customer)/2; i++) {
			ISP_HDM_IF_SNR_WriteI2C(SNR_OV2710_Reg_1920x1080_Customer[i*2], SNR_OV2710_Reg_1920x1080_Customer[i*2+1]);
		}

		// set target fps and corresponding sensor vsync
		ISP_IF_SNR_SetFPS(1120+31, 30);

		// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = 1928;
		VIFGrab_V_Start     = 1;
		VIFGrab_V_Length    = 1092; 
		
		ISP_IF_SNR_SetVIFGrab(VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length);

		// set scaler downsample rate (for scaler calculation)
		ISP_IF_SNR_SetDownSample(1, 1);

		// set color id
		ISP_IF_SNR_SetColorID(3);
		break;
	}

	// end of your implementation =================================================================

	// set VIF OPR (Actual sensor output size)
	{
		AITPS_VIF  pVIF = AITC_BASE_VIF;

		pVIF->VIF_GRAB_PIXL_ST = VIFGrab_H_Start;
		pVIF->VIF_GRAB_PIXL_ED = VIFGrab_H_Start + VIFGrab_H_Length -1;
		pVIF->VIF_GRAB_LINE_ST = VIFGrab_V_Start ;
		pVIF->VIF_GRAB_LINE_ED = VIFGrab_V_Start + VIFGrab_V_Length -1;
		pVIF->VIF_INT_LINE = VIFGrab_V_Length - 61;// 61;//121;		
	}
}
/*
void SNR_OV2710_DoAE_ST_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;

	switch (frame_cnt++ % 6) {
	case 0:
		ISP_IF_AE_Execute();

		again = ISP_IF_AE_GetAGain();
		dgain = ISP_IF_AE_GetDGain();
		s_gain = VR_MIN(16 * VR_MAX(again, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);

		// special conversion for exposure parameters could be set here ===========================
		if (again > 0x100) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F0)>>2),1);
			again = again & 0x3F0;
		} else if (again > 0x80) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F8)>>2),1);
			again = again & 0x3F8;
		} else {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)again) / VR_MAX((again & 0x3FC),1);
			again = again & 0x3FC;
		}

		if (s_gain > 0x20 && s_gain < 0x40) {
		   dgain = dgain * s_gain / (s_gain & 0xFE);
		   again = again * (s_gain & 0xFE) /s_gain;
		}

		if (s_gain > 0x40 && s_gain < 0x80) {
		   dgain = dgain * s_gain / (s_gain & 0xFC);
		   again = again * (s_gain & 0xFC) /s_gain;
		}
		// end of special conversion ==============================================================

		ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		break;

	case 1:
		//ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		ISP_IF_SNR_SetAGain(again);
		break;
	}
}

void SNR_OV2710_DoAE_END_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;

	switch (frame_cnt++ % 6) {
	case 1:
		again = ISP_IF_AE_GetAGain();
		dgain = ISP_IF_AE_GetDGain();
		s_gain = VR_MIN(16 * VR_MAX(again, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);

		// special conversion for exposure parameters could be set here ===========================
		if (again > 0x100) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F0)>>2),1);
			again = again & 0x3F0;
		} else if (again > 0x80) {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)(again>>2)) / VR_MAX(((again & 0x3F8)>>2),1);
			again = again & 0x3F8;
		} else {
			dgain = ((ISP_UINT32)dgain * (ISP_UINT32)again) / VR_MAX((again & 0x3FC),1);
			again = again & 0x3FC;
		}

		if (s_gain > 0x20 && s_gain < 0x40) {
		   dgain = dgain * s_gain / (s_gain & 0xFE);
		   again = again * (s_gain & 0xFE) /s_gain;
		}

		if (s_gain > 0x40 && s_gain < 0x80) {
		   dgain = dgain * s_gain / (s_gain & 0xFC);
		   again = again * (s_gain & 0xFC) /s_gain;
		}
		// end of special conversion ==============================================================

		ISP_IF_SNR_SetDGain(dgain);
		ISP_IF_SNR_SetAddMode(ISP_IF_AE_GetAdditionMode());
		break;
	}
}
*/
void SNR_OV2710_DoAWB_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;

	switch (frame_cnt++ % 6) {
	case 3:
	case 5:
		ISP_IF_AWB_Execute();
		ISP_IF_AWB_SetDGain();
		break;
	}
}

void SNR_OV2710_DoAF_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;

	switch (frame_cnt++ % 1) {
	case 0:
		ISP_IF_AF_Execute();
		break;
	}
}


#if (SENSOR_ID_OV2710== 0)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_OV2710;
#endif

#if (SENSOR_ID_OV2710== 1)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_OV2710;
#endif

#endif  //BIND_SENSOR_OV2710
#endif
