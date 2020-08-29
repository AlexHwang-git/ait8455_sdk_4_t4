//==============================================================================
//
//  File        : sensor_IMX045ES.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if defined(DSC_MP3_P_FW)||defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_IMX045ES

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

extern MMP_USHORT m_usResolType;

const ISP_UINT8 Sensor_IQ_CompressedText[] = {
#if(CHIP == P_V2)
#include "isp_842_iq_data_v1_IMX045ES.xls.qlz.txt"
#endif
};

//#define SUPPORT_AUTO_FOCUS 0

//#include "sensor_ctl.h"
//#include "preview_ctl.h"
//#include "XDATAmap.h"
//#include "3A_ctl.h"
//#include "IMX045ES.h"
//#include "motor_ctl.h"

//------------------------------------------------------------------------------
//  Function    : IMX045ES_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
static void IMX045ES_Sensor_PreviewMode(MMP_USHORT usPreviewmode)
{
    gsCurPreviewMode = usPreviewmode;
	//gsCurPhaseCount = usPhasecount;

    switch (usPreviewmode) {
    case 0:
        RTNA_DBG_Str(3, "Sensor 5M (2560x1920) preview mode\r\n");
		ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

		ISP_IF_IQ_SetColorTransform(0);//julian@091111
		ISP_IF_SNR_SetBeforeCapture();
		ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_2560x1920);

		ISP_IF_3A_Control(ISP_3A_ENABLE); //patrick@100518
        break;

    case 1:
        RTNA_DBG_Str(3, "Sensor preview (1280x960) mode\r\n");
        switch (gsSensorMode) {
        #if defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(VIDEO_R_EN)
        case SENSOR_VIDEO_MODE:
			RTNA_DBG_Str(3, "SENSOR_VIDEO_MODE\r\n");
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

//				ISP_IF_IQ_SetColorTransform(1);//julian@091111

			if  (m_VideoFmt == MMPS_3GPRECD_VIDEO_FORMAT_MJPEG) //HB
				ISP_IF_IQ_SetColorTransform(0);//julian@091111
			else
				ISP_IF_IQ_SetColorTransform(1);  // for YUV422/MJPEG mode, tomy@2010_04_30

			ISP_IF_SNR_SetBeforePreview();
			ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x960);

#if 1
			if (m_VideoFmt == MMPS_3GPRECD_VIDEO_FORMAT_H264){
                if(m_usResolType == VIDRECD_RESOL_1920x1088){  // 1080p
                    VR_AE_SetSensorVsync(gIspIfSnrResolInfo.SensorVsync*30/FULLHD_VIDEO_ENCODE_MAX_FRAME_RATE, gIspIfSnrResolInfo.TargetFPS);        
                }
                else{
                    VR_AE_SetSensorVsync(gIspIfSnrResolInfo.SensorVsync, gIspIfSnrResolInfo.TargetFPS);        
                }
            }
#endif  

			ISP_IF_AE_SetFPS(0);
			//ISP_IF_AE_SetFPS(30);

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
			ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x960);

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
static MMP_ERR  MMPF_Sensor_Initialize_IMX045ES(void)
{
//  MMP_USHORT usData;
    AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
    AITPS_PAD   pPAD  = AITC_BASE_PAD;
    AITPS_VIF  pVIF = AITC_BASE_VIF;

    RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_IMX045ES...Begin\r\n");

    //Init globol variable used by DSC
    gsSensorLCModeWidth     = 2560;
    gsSensorLCModeHeight    = 1920;
#if SKYPE_FOV
    // use 1.3M for preview, check sensor preview setting
    gsSensorMCModeWidth     = 1280;
    gsSensorMCModeHeight    = 720;
#else
    // use 1.3M for preview, check sensor preview setting
    gsSensorMCModeWidth     = 1280;
    gsSensorMCModeHeight    = 960;
#endif

    pPAD->PAD_PSCK = PAD_E8_CURRENT | PAD_E4_CURRENT | PAD_PULL_HIGH;
    pPAD->PAD_PSDA = PAD_E4_CURRENT | PAD_PULL_HIGH;
    pPAD->PAD_PSEN = PAD_E4_CURRENT | PAD_PULL_HIGH;


    MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);

    #if 1 //PhilipTest@090313
    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
    //RTNA_WAIT_US((15+125)*10); // 15 us + 6 EXT_CLK (48MHz) = 15 us + 125 ns
    #endif

    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_FALSE);//Pin output low
    //RTNA_WAIT_US(15+125); // 15 us + 6 EXT_CLK (48MHz) = 15 us + 125 ns
    //RTNA_WAIT_US((15+125)*10); // 15 us + 6 EXT_CLK (48MHz) = 15 us + 125 ns
    RTNA_WAIT_MS(1);
    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high

    RTNA_WAIT_MS(5);

    pVIF->VIF_SENSR_CLK_CTL  = VIF_SENSR_CLK_EN;
	RTNA_WAIT_MS(5);

    /********************************************/
    /*Use GPIO to reset sensor for ev board  */
    /*Caution : It's customer dependent !!!     */
    /********************************************/
    /*Reset Sensor 500mS*/
 
    /*Reset Sensor 500mS*/
	#if  (CHIP == P_V2)
    MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);

    MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);
	RTNA_WAIT_MS(5);

    MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);
	RTNA_WAIT_MS(5);
    #endif

	ISP_IF_SNR_Init();

	//VR_Motor_Init();

    RTNA_DBG_Str(3, "MMPF_Sensor_Initialize_IMX045ES...End\r\n");
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_IMX045ES
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_PowerDown_IMX045ES(void)
{
    AITPS_VIF  pVIF = AITC_BASE_VIF;
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	AITPS_MIPI pMIPI = AITC_BASE_MIPI;

	pMIPI->MIPI_DATA1_CFG &= ~(MIPI_CSI2_EN);
	pMIPI->MIPI_DATA2_CFG &= ~(MIPI_CSI2_EN);
	MMPF_VIF_EnableInputInterface(MMP_FALSE);


#if SUPPORT_AUTO_FOCUS
    GPIO_OutputControl(AF_EN, TRUE);
#endif


#if (CHIP == P_V2)
    /*Reset Sensor 500mS*/
    MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);
   // MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);  //Pin output high
   // RTNA_WAIT_MS(10/*100*/);
    MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);//Pin output low
    RTNA_WAIT_MS(2/*10*//*100*/);
#endif

    MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);
    //MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
    //RTNA_WAIT_MS(10/*40*/);
    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_FALSE);//Pin output low
    RTNA_WAIT_MS(2/*10*//*40*/);

    pVIF->VIF_SENSR_CLK_CTL &= (~ VIF_SENSR_CLK_EN) ;   //output sensor main clock


    RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_IMX045ES() \r\n");

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_IMX045ES
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_IMX045ES(MMP_UBYTE ubFrameRate)
{
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_SetPreviewMode_IMX045ES(MMP_USHORT usPreviewmode)
{
    IMX045ES_Sensor_PreviewMode(usPreviewmode);

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangePreviewMode_IMX045ES(
                        MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
                        MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
    IMX045ES_Sensor_PreviewMode(usNewPreviewmode);

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangeMode_IMX045ES(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
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
            MMPF_Sensor_SetPreviewMode_IMX045ES(0);
            // wait 3 frame ...
            //MMPF_Sensor_WaitFrame(5);
            break;
        case 1:
            MMPF_Sensor_SetPreviewMode_IMX045ES(1);
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
static MMP_ERR  MMPF_Sensor_InitializeVIF_IMX045ES(void)
{
    AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	AITPS_PAD   pPAD  = AITC_BASE_PAD;
	//RTNA_DBG_Str(0, "ISP_IF_SNR_Register...S\r\n");

    // register sensor (functions and variables)
	ISP_IF_SNR_Register();
	RTNA_DBG_Str(0, "ISP_IF_SNR_Register...E\r\n");

#if SUPPORT_AUTO_FOCUS
    GPIO_OutputControl(AF_EN, FALSE);
	RTNA_WAIT_MS(20);
#endif

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
static MMP_ERR  MMPF_Sensor_InitializeISP_IMX045ES(void)
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
//  Function    : MMPF_Sensor_SetAFPosition_IMX045ES
//  Description :
// ---------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_IMX045ES(MMP_UBYTE ubPos)
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
static MMP_ERR  MMPF_Sensor_DoAWBOperation_IMX045ES(void)
{

	if(ISP_IF_AWB_GetVer().MajorVer != 0x03){	ISP_IF_R_GetAWBAcc();	}

	ISP_IF_R_DoAWB();
	
	//RTNA_DBG_Str(0, "DoAWBOperation_IMX045ES()\r\n");

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_ST
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_ST_IMX045ES(void)
{
	//ISP_IF_R_DoAE();
	//RTNA_DBG_Str(0, "Do3AOperation_IMX045ES()\r\n");

	static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;

	switch (frame_cnt++ % 6) {
	case 0:
		ISP_IF_AE_Execute();

		again = ISP_IF_AE_GetAGain();
		dgain = ISP_IF_AE_GetDGain();
		s_gain = VR_MIN(16 * VR_MAX(again, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);
/*
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
*/
		ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		ISP_IF_SNR_SetAGain(again);
		break;

	case 2:
		//ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		//ISP_IF_SNR_SetAGain(again);
		break;
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_END
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_END_IMX045ES(void)
{
	//ISP_IF_R_DoAE();
	//RTNA_DBG_Str(0, "Do3AOperation_IMX045ES()\r\n");

	static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;
	
	ISP_IF_R_GetAEAcc();

	switch (frame_cnt++ % 6) {
	case 2:
		again = ISP_IF_AE_GetAGain();
		dgain = ISP_IF_AE_GetDGain();
		s_gain = VR_MIN(16 * VR_MAX(again, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);
/*
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
*/
		ISP_IF_SNR_SetDGain(dgain);
		ISP_IF_SNR_SetAddMode(ISP_IF_AE_GetAdditionMode());
		break;
	}

	return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_IMX045ES(void)
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
//  Function    : MMPF_Sensor_DoIQOperation_IMX045ES
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoIQOperation_IMX045ES(void)
{
	// set IQ at frame end to ensure frame sync
	ISP_IF_R_DoIQ();

	//RTNA_DBG_Str(0, "DoIQOperation_IMX045ES()\r\n");
    
    return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_IMX045ES(MMP_USHORT usMode)
{
    RTNA_DBG_Str(0, "MMPF_SetLightFreq_IMX045ES : ");
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

static MMP_ERR  MMPF_SetStableState_IMX045ES(MMP_BOOL bStableState)
{
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_IMX045ES(MMP_BOOL bHistEQState)
{

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_IMX045ES(MMP_USHORT usAddr, MMP_USHORT usData)
{
	//MMPF_I2cm_WriteReg(usAddr, usData);
	 ISP_HDM_IF_SNR_WriteI2C(usAddr,usData);
    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_IMX045ES(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	//MMPF_I2cm_ReadReg(usAddr, usData);
	*usData = ISP_HDM_IF_SNR_ReadI2C(usAddr) ;
    return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_IMX045ES(MMP_USHORT imgSceneID)
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

	RTNA_DBG_Str(3, "MMPF_SetImageScene_IMX045ES : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_IMX045ES(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAEMode_IMX045ES : ");
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

static MMP_ERR  MMPF_Sensor_SetAFMode_IMX045ES(MMP_UBYTE ubAFMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_IMX045ES : ");
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
static MMP_ERR  MMPF_Sensor_SetImageEffect_IMX045ES(MMP_USHORT imgEffectID)
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

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_IMX045ES : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_IMX045ES(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_IMX045ES : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

   	cAWBtype = ubType;
#endif
    return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_IMX045ES(MMP_SHORT ubLevel)
{
/*
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_IMX045ES : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");
*/
	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);

    return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_IMX045ES(MMP_SHORT ubLevel)
{
/*
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_IMX045ES : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");
*/
	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);

    return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_IMX045ES(MMP_SHORT ubLevel)
{
/*
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_IMX045ES : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");
*/
	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_IMX045ES(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_IMX045ES : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_IMX045ES(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_IMX045ES : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetBacklight_IMX045ES(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_IMX045ES : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

//	ISP_IF_F_SetBacklight(ubLevel);
    if(ubLevel > 1){
		ISP_IF_F_SetWDR(ISP_WDR_DISABLE);
    }
    else{
		ISP_IF_F_SetWDR(ISP_WDR_ENABLE);
    }

    return  MMP_ERR_NONE;
}


static void  MMPF_Sensor_SetCaptureISPSetting_IMX045ES(MMP_UBYTE usCaptureBegin)
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
static void  MMPF_Sensor_SetISPWindow_IMX045ES(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
	//gSensorFunc.SetISPWindows(ulWidth,ulHeight);
    return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_IMX045ES(MMP_USHORT usPreviewmode)
{
    switch(usPreviewmode){
        case 0:                 //5M can't preview
            return MMP_FALSE;           
            break;
        case 1:                 //1.3M can preview
            return MMP_TRUE;           
            break;
    }
    
    return MMP_TRUE;            //5M and 1.3M always can preview
}

static void SetAFWin_IMX045ES(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
                    MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

    DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_IMX045ES(MMP_UBYTE enable)
{
//    if (enable) {
		RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
		ISP_IF_AF_Control(ISP_AF_START);
        //actually calling SNR_IMX045ES_SetAutoFocusControl()
//    } else {
//        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
//        ISP_IF_AF_Control(ISP_AF_STOP);
//    }
}

static MMP_ERR MMPF_Sensor_SetAEEnable_IMX045ES(MMP_UBYTE bEnable)  
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

static MMP_ERR MMPF_Sensor_SetAWBEnable_IMX045ES(MMP_UBYTE bEnable) 
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
static void SetAEsmooth_IMX045ES(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_IMX045ES(MMP_UBYTE ev)
{
	//RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_IMX045ES : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	//RTNA_DBG_Byte(3, ev);
	//RTNA_DBG_Str(3, "\r\n");
   
	ISP_IF_AE_SetEV(ev);

    return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_IMX045ES     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_IMX045ES(void)
{
    //return 0;
	return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_IMX045ES  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_IMX045ES       (void){return 0;}
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
static MMP_ERR	MMPF_Sensor_Set3AStatus_IMX045ES(MMP_BOOL bEnable)
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

static void  MMPF_Sensor_SetColorID_IMX045ES(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}

MMPF_SENSOR_FUNCTION  SensorFunction_IMX045ES =
{
    MMPF_Sensor_Initialize_IMX045ES,
    MMPF_Sensor_InitializeVIF_IMX045ES,
    MMPF_Sensor_InitializeISP_IMX045ES,
    MMPF_Sensor_PowerDown_IMX045ES,
    MMPF_Sensor_SetFrameRate_IMX045ES,
    MMPF_Sensor_ChangeMode_IMX045ES,
    MMPF_Sensor_ChangePreviewMode_IMX045ES,
    MMPF_Sensor_SetPreviewMode_IMX045ES,
    MMPF_Sensor_SetReg_IMX045ES,
    MMPF_Sensor_GetReg_IMX045ES,
    MMPF_Sensor_DoAWBOperation_IMX045ES,
    MMPF_Sensor_DoAEOperation_ST_IMX045ES,
    MMPF_Sensor_DoAEOperation_END_IMX045ES,
    MMPF_Sensor_DoAFOperation_IMX045ES,
    MMPF_Sensor_DoIQOperation_IMX045ES,
    MMPF_Sensor_SetImageEffect_IMX045ES,
    MMPF_SetLightFreq_IMX045ES,
    MMPF_SetStableState_IMX045ES,
    MMPF_SetHistEQState_IMX045ES,
    MMPF_Sensor_SetAFPosition_IMX045ES,
    SetAFWin_IMX045ES,
    SetAFEnable_IMX045ES,
    MMPF_Sensor_SetAEEnable_IMX045ES,
    MMPF_Sensor_SetAWBEnable_IMX045ES,
    MMPF_Sensor_SetExposureValue_IMX045ES,
    SetAEsmooth_IMX045ES,
    MMPF_Sensor_SetImageScene_IMX045ES,
    GetAFPosition_IMX045ES,
    GetAFEnable_IMX045ES,
    GetExposureValue_IMX045ES,
    GetAEsmooth_IMX045ES,
    MMPF_Sensor_SetAWBType_IMX045ES,
    MMPF_Sensor_SetContrast_IMX045ES,
    MMPF_Sensor_SetSaturation_IMX045ES,
    MMPF_Sensor_SetSharpness_IMX045ES,
    MMPF_Sensor_SetHue_IMX045ES,
    MMPF_Sensor_SetGamma_IMX045ES,
//    MMPF_Sensor_SetBacklight_IMX045ES,
    MMPF_Sensor_SetAEMode_IMX045ES,
    MMPF_Sensor_SetAFMode_IMX045ES,    
    MMPF_Sensor_SetCaptureISPSetting_IMX045ES,
    MMPF_Sensor_SetISPWindow_IMX045ES,
    MMPF_Sensor_CheckPreviewAbility_IMX045ES,
	MMPF_Sensor_Set3AStatus_IMX045ES,
	MMPF_Sensor_SetColorID_IMX045ES
};


MMP_USHORT SNR_IMX045ES_Reg_Init_Customer[] = {
	// PLL settings
	0x0305, 0x02, // pre_pll_clk_div
//	0x0307, 0x24, // pll_multiplier, input clock = 24MHz
	0x0307, 0x48, // pll_multiplier, input clock = 12MHz 
	0x302B, 0x53, // PLL Oscillation Stable Wait Time

	0x0101, 0x03, // image_orientation // for 180 rotate
	0x300A, 0x80,
#if 1//SENSOR_OFFSET_ON
	0x300B, 0x00,
#endif
	0x3014, 0x08, // Y_OPBADDR_START_DI
	0x3015, 0x37, // Y_OPBADDR_END_DI
	0x3017, 0x40,
	0x301C, 0x41,//0x01, // OUTIF driving current - 0x81:0.5mA | 0x41:1mA | 0x01:2mA
	0x3031, 0x28, // Test register
	0x3040, 0x00,
	0x3041, 0x60, // Test register
	0x3051, 0x24, // Test register
	0x3053, 0x34, // Test register
	0x3055, 0x3B, // Test register
	0x3057, 0xC0, // Test register
	0x3060, 0x30, // Low power test
	0x3065, 0x00, // Low power test
	0x30AA, 0x88,
	0x30AB, 0x1C,
	0x30B0, 0x32,
	0x30B2, 0x83,
	0x30D3, 0x04,
#if 1//SENSOR_OFFSET_ON
	0x30E7, 0x04,
#endif
	0x310E, 0xDD,
	0x31A4, 0xD8, // Timing adjustment register
	0x31A6, 0x17, // Timing adjustment register
	0x31AC, 0xCF, // Timing adjustment register
	0x31AE, 0xF1, // Timing adjustment register
	0x31B4, 0xD8, // Timing adjustment register
	0x31B6, 0x17, // Timing adjustment register
	0x3304, 0x02, // Test register
	0x3305, 0x02, // Test register
	0x3306, 0x0A, // Test register
	0x3307, 0x02, // Test register
	0x3308, 0x11, // Test register
	0x3309, 0x04, // Test register
	0x330A, 0x05, // Test register
	0x330B, 0x00, // Test register
	0x330C, 0x05, // Test register
	0x330D, 0x02, // Test register
	0x330E, 0x01, // Test register

	0x0340, 0x03, // frame_length_lines
	0x0341, 0xF7, // frame_length_lines
	0x034C, 0x05, // x_output_size
	0x034D, 0x18, // x_output_size(1304d)
	0x034E, 0x03, // y_output_size
	0x034F, 0xD4, // y_output_size(980d)
	0x0381, 0x01, // x_even_inc
	0x0383, 0x03, // x_odd_inc
	0x0385, 0x01, // y_even_inc
	0x0387, 0x03, // y_odd_inc
	0x3016, 0x46, // VMODEADD
	0x30E8, 0x86, // HADDAVE[7]
	0x3301, 0x80, // MIPI setting (0: dual lane ; 1,2: single lane) , [7]: 1 for clock DIV2

	0x0100, 0x01, // Streaming
};
void SNR_IMX045ES_InitSensor_Customer(void) {
	// implement your initialize sensor routine here ==============================================
	ISP_UINT16 data ;

	ISP_UINT32 i;

	// initialize I2C ID and type
	/*
	VR_SNR_ConfigI2C(	VR_TableN_GetValueByIndex(&gIspTableHeader.Sensor_Misc, 0),
						VR_TableN_GetValueByIndex(&gIspTableHeader.Sensor_Misc, 1));
	*/

	ISP_IF_SNR_ConfigI2C( 0x1A, ISP_I2C_TYPE_2A1D);

	// initialize VIF OPRs
	ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_DUAL_01, 22);

	ISP_HDM_IF_EnableSensor(ISP_HDM_SNR_ENABLE_HIGH);

	// set register...........
	for (i = 0; i < sizeof(SNR_IMX045ES_Reg_Init_Customer)/4; i++) {
		ISP_HDM_IF_SNR_WriteI2C(SNR_IMX045ES_Reg_Init_Customer[i*2], SNR_IMX045ES_Reg_Init_Customer[i*2+1]);
	}

	VR_AE_SetBaseShutter( 304, 254, 304, 254);

	// Set preview resolution as default
	ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x960);
}



void SNR_IMX045ES_SetSensorGain_Customer(ISP_UINT32 gain)
{
	ISP_UINT32 sensor_gain = VR_MIN(256 - (256 * ISP_IF_AE_GetAGainBase() + (gain-1)) / VR_MAX(gain, ISP_IF_AE_GetAGainBase()), 224); //Change Again setting for rounding error

	ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x01); // grouped_parameter_hold
	ISP_HDM_IF_SNR_WriteI2C(0x0204, (sensor_gain >> 8) & 0xff);
	ISP_HDM_IF_SNR_WriteI2C(0x0205, sensor_gain & 0xff);
	ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x00); // grouped_parameter_hold

}

void SNR_IMX045ES_SetSensorShutter_Customer(ISP_UINT32 shutter, ISP_UINT32 vsync)
{
	// 1 ~ (frame length line - 3)
	ISP_UINT32 new_vsync = VR_MIN(VR_MAX(shutter + 3, gIspIfSnrResolInfo.SensorVsync), 0xFFFF);
	ISP_UINT32 new_shutter = VR_MIN(VR_MAX(shutter, 1), new_vsync - 3);

	ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x01); // grouped_parameter_hold
	ISP_HDM_IF_SNR_WriteI2C(0x0340, new_vsync >> 8);
	ISP_HDM_IF_SNR_WriteI2C(0x0341, new_vsync);

	ISP_HDM_IF_SNR_WriteI2C(0x0202, new_shutter >> 8);
	ISP_HDM_IF_SNR_WriteI2C(0x0203, new_shutter);
	ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x00); // grouped_parameter_hold
}

void SNR_IMX045ES_SetSensorAddMode_Customer(ISP_UINT8 enable)
{
	if (ISP_HDM_IF_IsFrameTypePreview()) {
		ISP_UINT8 regval = (enable) ? ISP_HDM_IF_SNR_ReadI2C(0x3016) | 0x40 : ISP_HDM_IF_SNR_ReadI2C(0x3016) & ~0x40;

		ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x01); // grouped_parameter_hold
		ISP_HDM_IF_SNR_WriteI2C(0x3016, regval);
		ISP_HDM_IF_SNR_WriteI2C(0x0104, 0x00); // grouped_parameter_hold
	}

}

MMP_USHORT SNR_IMX045ES_Reg_2560x1920_Customer[] = {
	0x0104, 0x01, // Grouped Parameter Hold = 0x01

	0x0305, 0x04, // pre_pll_clk_div

	0x0340, 0x07, // frame_length_lines
	0x0341, 0xEE, // frame_length_lines
	0x0342, 0x0B, // frame_length_lines
	0x0343, 0x16, // frame_length_lines
	0x034C, 0x0A, // x_output_size
	0x034D, 0x30, // x_output_size(2608d)
	0x034E, 0x07, // y_output_size
	0x034F, 0xA8, // y_output_size(1960d)
	0x0381, 0x01, // x_even_inc
	0x0383, 0x01, // x_odd_inc
	0x0385, 0x01, // y_even_inc
	0x0387, 0x01, // y_odd_inc
	0x3016, 0x06, // VMODEADD
	0x30E8, 0x06, // HADDAVE[7]
	0x3301, 0x00, // MIPI setting (0: dual lane ; 1,2: single lane) , [7]: 1 for clock DIV2

	0x0104, 0x00, // Grouped Parameter Hold = 0x00
};

MMP_USHORT SNR_IMX045ES_Reg_1280x960_Customer[] = {
	0x0104, 0x01, // Grouped Parameter Hold = 0x01

	0x0305, 0x02, // pre_pll_clk_div

	0x0340, 0x03, // frame_length_lines
	0x0341, 0xF7, // frame_length_lines
	0x0342, 0x0B, // frame_length_lines
	0x0343, 0x16, // frame_length_lines
	0x034C, 0x05, // x_output_size
	0x034D, 0x18, // x_output_size(1304d)
	0x034E, 0x03, // y_output_size
	0x034F, 0xD4, // y_output_size(980d)
	0x0381, 0x01, // x_even_inc
	0x0383, 0x03, // x_odd_inc
	0x0385, 0x01, // y_even_inc
	0x0387, 0x03, // y_odd_inc
	0x3016, 0x46, // VMODEADD
	0x30E8, 0x86, // HADDAVE[7]
	0x3301, 0x80, // MIPI setting (0: dual lane ; 1,2: single lane) , [7]: 1 for clock DIV2

	0x0104, 0x00, // Grouped Parameter Hold = 0x00
};

void SNR_IMX045ES_SetSensorResolution_Customer(ISP_SENSOR_RESOL res)
{
	// implement your change sensor resolution routine here =======================================

	MMP_ULONG i, VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;


	switch (res) {
	case ISP_SENSOR_RESOL_2560x1920:

		// set register...........
		for (i = 0; i < VR_ARRSIZE(SNR_IMX045ES_Reg_2560x1920_Customer)/2; i++) {
			VR_SNR_WriteI2C(SNR_IMX045ES_Reg_2560x1920_Customer[i*2], SNR_IMX045ES_Reg_2560x1920_Customer[i*2+1]);
		}

		// set target fps and corresponding sensor vsync
		ISP_IF_SNR_SetFPS(0x07EE, 30);

		// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = 2592;
		VIFGrab_V_Start     = 1 + 20;
		VIFGrab_V_Length    = 1944;

		ISP_IF_SNR_SetVIFGrab(VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length);

		// set scaler downsample rate (for scaler calculation)
		ISP_IF_SNR_SetDownSample(1, 1);

		// set color id
		ISP_IF_SNR_SetColorID(0);

		break;

	case ISP_SENSOR_RESOL_1920x1080:
	case ISP_SENSOR_RESOL_1280x960:
	case ISP_SENSOR_RESOL_1280x720:

		// set register...........
		for (i = 0; i < VR_ARRSIZE(SNR_IMX045ES_Reg_1280x960_Customer)/2; i++) {
			ISP_HDM_IF_SNR_WriteI2C(SNR_IMX045ES_Reg_1280x960_Customer[i*2], SNR_IMX045ES_Reg_1280x960_Customer[i*2+1]);
		}

		// set target fps and corresponding sensor vsync
		ISP_IF_SNR_SetFPS(0x03F7, 30);

#if SKYPE_FOV
		// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = 1296;
		VIFGrab_V_Start     = 1 + 12 + 120;
		VIFGrab_V_Length    = 728;
#else
		// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = 1296;
		VIFGrab_V_Start     = 1 + 12;
		VIFGrab_V_Length    = 972;
#endif

		ISP_IF_SNR_SetVIFGrab(VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length);

		// set scaler downsample rate (for scaler calculation)
		ISP_IF_SNR_SetDownSample(1, 1);

		// set color id
		ISP_IF_SNR_SetColorID(0);
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
void SNR_IMX045ES_DoAE_ST_Customer(void)
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
		// end of special conversion ==============================================================

		ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		ISP_IF_SNR_SetAGain(again);
		break;

	case 1:
		//ISP_IF_SNR_SetShutter(ISP_IF_AE_GetShutter(), ISP_IF_AE_GetVsync());
		//ISP_IF_SNR_SetAGain(again);
		break;
	}
}

void SNR_IMX045ES_DoAE_END_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;
	static ISP_UINT32 again, dgain, s_gain;

	switch (frame_cnt++ % 6) {
	case 1:
		again = ISP_IF_AE_GetAGain();
		dgain = ISP_IF_AE_GetDGain();
		s_gain = VR_MIN(16 * VR_MAX(again, ISP_IF_AE_GetAGainBase()) / ISP_IF_AE_GetAGainBase() , 127);

		// special conversion for exposure parameters could be set here ===========================
		// end of special conversion ==============================================================

		ISP_IF_SNR_SetDGain(dgain);
		ISP_IF_SNR_SetAddMode(ISP_IF_AE_GetAdditionMode());
		break;
	}
}
*/
void SNR_IMX045ES_DoAWB_Customer(void)
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

void SNR_IMX045ES_DoAF_Customer(void)
{
	static ISP_UINT32 frame_cnt = 0;

	switch (frame_cnt++ % 1) {
	case 0:
		ISP_IF_AF_Execute();
		break;
	}
}


#if (SENSOR_ID_IMX045ES== 0)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_IMX045ES;
#endif

#if (SENSOR_ID_IMX045ES== 1)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_IMX045ES;
#endif

#endif  //BIND_SENSOR_IMX045ES
#endif
