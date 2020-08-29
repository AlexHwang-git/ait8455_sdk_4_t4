//==============================================================================
//
//  File        : sensor_S5K6A1GX.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if defined(DSC_MP3_P_FW)||defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_S5K6A1GX

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

#include "mmp_reg_i2cm.h"
#include "mmps_3gprecd.h"

extern MMP_USHORT  gsSensorLCModeWidth;
extern MMP_USHORT  gsSensorLCModeHeight;
extern MMP_USHORT  gsSensorMCModeWidth;
extern MMP_USHORT  gsSensorMCModeHeight;
extern MMP_USHORT  gsCurPreviewMode, gsCurPhaseCount;
extern MMP_USHORT  gsSensorMode;
extern MMP_USHORT  m_usVideoQuality;
extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_BOOL    gbAutoFocus;

//#include "sensor_ctl.h"
//#include "preview_ctl.h"
//#include "XDATAmap.h"
//#include "3A_ctl.h"
//#include "S5K6A1GX.h"
//#include "motor_ctl.h"
//------------------------------------------------------------------------------
//  Function    : S5K6A1GX_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
static void S5K6A1GX_Sensor_SetResolution(ISP_SENSOR_RESOL resol)	// Patrick add
{
	ISP_IF_SNR_SetResolution(resol);

	{
		AITPS_VIF  pVIF = AITC_BASE_VIF;
		volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;
		pVIF->VIF_GRAB_PIXL_ST = gIspIfSnrResolInfo.VIFGrab_H_Start ;
		pVIF->VIF_GRAB_PIXL_ED = gIspIfSnrResolInfo.VIFGrab_H_Start + gIspIfSnrResolInfo.VIFGrab_H_Length -1;//gSensorConfig.VIFGrab_H_Length - 1;
		pVIF->VIF_GRAB_LINE_ST = gIspIfSnrResolInfo.VIFGrab_V_Start ;
		pVIF->VIF_GRAB_LINE_ED = gIspIfSnrResolInfo.VIFGrab_V_Start + gIspIfSnrResolInfo.VIFGrab_V_Length -1;//gSensorConfig.VIFGrab_V_Length - 1;
		pVIF->VIF_INT_LINE = gIspIfSnrResolInfo.VIFGrab_V_Length - 61;

		// ISP pipeline selection
		//APICAL_XBYTE[APICAL_CTL_REG1] &= ~(0x80); // clear auto size
		pISP[0x0B] = 0x30;
		pISP[0x09] = 0x0C;

		// IP ColorID setting
		//pISP[0x09] &= ~(0xF0);
		//pISP[0x09] |= (ISP_IF_SNR_GetColorID() == 0 ? 3 : (ISP_IF_SNR_GetColorID() == 1 ? 2 : (ISP_IF_SNR_GetColorID() == 2 ? 1: 0))) * (0x50);
	}
}
//------------------------------------------------------------------------------
//  Function    : OV5650_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
static void S5K6A1GX_Sensor_PreviewMode(MMP_USHORT usPreviewmode, MMP_USHORT usPhasecount)
{
    gsCurPreviewMode = usPreviewmode;
    gsCurPhaseCount = usPhasecount;

    switch (usPreviewmode) {
    case 0:
        RTNA_DBG_Str(3, "Sensor 5M (2560x1920) preview mode\r\n");
		ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

		ISP_IF_IQ_SetColorTransform(0);//julian@091111
		ISP_IF_SNR_SetBeforeCapture();
		S5K6A1GX_Sensor_SetResolution(ISP_SENSOR_RESOL_1280x720);

		ISP_IF_3A_Control(ISP_3A_ENABLE); //patrick@100518
        break;

    case 1:
        RTNA_DBG_Str(3, "Sensor preview (1280x960) mode\r\n");
        switch (gsSensorMode) {
        #if defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(VIDEO_R_EN)
        case SENSOR_VIDEO_MODE:
			RTNA_DBG_Str(3, "SENSOR_VIDEO_MODE\r\n");		
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

			if (m_VideoFmt == MMPS_3GPRECD_VIDEO_FORMAT_H264)
				ISP_IF_IQ_SetColorTransform(1);//julian@091111
			else
				ISP_IF_IQ_SetColorTransform(0);  // for YUV422/MJPEG mode, tomy@2010_04_30

			ISP_IF_SNR_SetBeforePreview();
			S5K6A1GX_Sensor_SetResolution(ISP_SENSOR_RESOL_1280x720);

#if SKYPE_H264_TEST
            ISP_IF_AE_SetFPS(ISP_AE_FPS_30);
#else					
			//ISP_IF_AE_SetFPS(ISP_AE_FPS_AUTO);
            ISP_IF_AE_SetFPS(ISP_AE_FPS_30);
			//ISP_IF_AE_SetFPS(ISP_AE_FPS_25);
			//ISP_IF_AE_SetFPS(ISP_AE_FPS_20);
			//ISP_IF_AE_SetFPS(ISP_AE_FPS_15);
			//ISP_IF_AE_SetFPS(ISP_AE_FPS_10);
			//ISP_IF_AE_SetFPS(ISP_AE_FPS_5);
#endif			
            
			ISP_IF_3A_Control(ISP_3A_ENABLE);
            break;
        #endif
        #if defined(DSC_MP3_P_FW)||(DSC_EN)
        case SENSOR_DSC_MODE:
            RTNA_DBG_Str(3, "Dsc preview mode\r\n");
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

			ISP_IF_IQ_SetColorTransform(0);//julian@091111
			ISP_IF_SNR_SetBeforePreview();
			S5K6A1GX_Sensor_SetResolution(ISP_SENSOR_RESOL_1280x720);

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
static MMP_ERR  MMPF_Sensor_Initialize_S5K6A1GX(void)
{
//  MMP_USHORT usData;
    AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
    AITPS_PAD   pPAD  = AITC_BASE_PAD;
    AITPS_VIF  pVIF = AITC_BASE_VIF;

    RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_S5K6A1GX...Begin\r\n");

#if (CUSTOMER == BIS)
    // sensor PD_12A disable
    pGPIO->GPIO_DATA[0] |= (1 << 12);  // Turn On sensor

    // sensor PD_28A disable
    pGPIO->GPIO_DATA[0] |= (1 << 13);  // Turn On sensor

#endif

    //RTNA_DBG_Str(0, "Config PS_GPIO pin as pull high IO Pad\r\n");
    // config PS_GPIO as pull high
    pPAD->PAD_PSGPIO = PAD_E8_CURRENT | PAD_PULL_LOW;
    pGPIO->GPIO_EN[3] |= (1 << 16); // config as output mode
    pGPIO->GPIO_DATA[3] |= (1 << 16);  // Turn On Sensor Power
    RTNA_WAIT_MS(300);

    //Init globol variable used by DSC
    gsSensorLCModeWidth     = 1280;
    gsSensorLCModeHeight    = 720;
    // use 1.3M for preview, check sensor preview setting
    gsSensorMCModeWidth     = 1280;
    gsSensorMCModeHeight    = 720;


    #if (CHIP == D_V1)||(CHIP == PYTHON)
    pPAD->PAD_PSCK = PAD_E8_CURRENT | PAD_E4_CURRENT | PAD_PULL_HIGH;
    pPAD->PAD_PSDA = PAD_E4_CURRENT | PAD_PULL_HIGH;
    pPAD->PAD_PSEN = PAD_E4_CURRENT | PAD_PULL_HIGH;
    #endif


    MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);

    #if 1 //PhilipTest@090313
    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
    //RTNA_WAIT_US((15+125)*10); // 15 us + 6 EXT_CLK (48MHz) = 15 us + 125 ns
    RTNA_WAIT_MS(1);
    #endif

    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_FALSE);//Pin output low
    //RTNA_WAIT_US(15+125); // 15 us + 6 EXT_CLK (48MHz) = 15 us + 125 ns
    //RTNA_WAIT_US((15+125)*10); // 15 us + 6 EXT_CLK (48MHz) = 15 us + 125 ns
    RTNA_WAIT_MS(1);
    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
    RTNA_WAIT_MS(1);

	#if 0
	// if using AIT824F board, please use these codes to replace the VIF_SIF_SEN control
    pGPIO->GPIO_EN[0] |= (1 << 22);
    pGPIO->GPIO_DATA[0] |= (1 << 22);
    RTNA_WAIT_MS(1);
    pGPIO->GPIO_DATA[0] &= ~(1 << 22);
    RTNA_WAIT_MS(1);
    pGPIO->GPIO_DATA[0] |= (1 << 22);
    RTNA_WAIT_MS(40);
	#endif
    pVIF->VIF_SENSR_CLK_CTL  = VIF_SENSR_CLK_EN;

    /********************************************/
    /*Use GPIO to reset sensor for ev board  */
    /*Caution : It's customer dependent !!!     */
    /********************************************/
    /*Reset Sensor 500mS*/
    #if (CHIP == D_V1)
    pGPIO->GPIO_EN[0] |= (1 << GPIO_SENSOR_RST);
    pGPIO->GPIO_DATA[0] &= ~(1 << GPIO_SENSOR_RST);
    RTNA_WAIT_MS(500);
    pGPIO->GPIO_DATA[0] |= (1 << GPIO_SENSOR_RST);
    RTNA_WAIT_MS(40);
    #endif

    /*Reset Sensor 500mS*/
    #if (CHIP == PYTHON)	
    MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);

    MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);
    RTNA_WAIT_MS(100);

    MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);
    RTNA_WAIT_MS(40);
    #endif

	ISP_IF_SNR_Init();

#if SUPPORT_AUTO_FOCUS  
    //RTNA_DBG_Str(0, "Config PGPIO19 pin as pull high IO Pad\r\n");
    // config PGPIO19 as pull high
    pPAD->PAD_GPIO[AF_EN] = PAD_E8_CURRENT | PAD_PULL_LOW;
    pGPIO->GPIO_EN[0] |= (1 << AF_EN); // config as output mode
//    pGPIO->GPIO_DATA[0] &= ~(1 << AF_EN);  // Turn On Motor
    pGPIO->GPIO_DATA[0] |= (1 << AF_EN);  // Turn On Motor
    RTNA_WAIT_MS(20);
#endif

	//VR_Motor_Init();

    RTNA_DBG_Str(3, "MMPF_Sensor_Initialize_S5K6A1GX...End\r\n");
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_S5K6A1GX
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_PowerDown_S5K6A1GX(void)
{
    AITPS_VIF  pVIF = AITC_BASE_VIF;
    AITPS_GPIO  pGPIO = AITC_BASE_GPIO;

#if (CHIP == D_V1)

	pGPIO->GPIO_EN[0] |= (1 << GPIO_SENSOR_RST);
    pGPIO->GPIO_DATA[0] |= (1 << GPIO_SENSOR_RST);
	RTNA_WAIT_MS(100);
	pGPIO->GPIO_DATA[0] &= ~(1 << GPIO_SENSOR_RST);
	RTNA_WAIT_MS(200);
#endif


#if SUPPORT_AUTO_FOCUS  
	//ISP_IF_MOTOR_MoveToZero();
	RTNA_WAIT_MS(20);
    //pGPIO->GPIO_DATA[0] |= (1 << AF_EN);  // Turn Off Motor
    pGPIO->GPIO_DATA[0] &= ~(1 << AF_EN);  // Turn Off Motor
#endif


#if (CHIP == PYTHON)
    MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);
    MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);  //Pin output high
    RTNA_WAIT_MS(100);
    MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);//Pin output low
    RTNA_WAIT_MS(200);
#endif

#if 1
    MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);
    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_FALSE);  //Pin output high
    RTNA_WAIT_MS(40);
#endif

    pVIF->VIF_SENSR_CLK_CTL &= (~ VIF_SENSR_CLK_EN) ;   //output sensor main clock

#if (CUSTOMER == BIS)
    // sensor PD_12A disable
    pGPIO->GPIO_DATA[0] &= ~(1 << 12);  // Turn Off sensor

    // sensor PD_28A disable
    pGPIO->GPIO_DATA[0] &= ~(1 << 13);  // Turn Off sensor

#endif

#if 1
    //RTNA_DBG_Str(3, "Config PS_GPIO pin as LOW\r\n");
    pGPIO->GPIO_DATA[3] &= ~(1 << 16);  // Turn Off Sensor Power
#endif
    RTNA_WAIT_MS(50);

    RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_S5K6A1GX() \r\n");

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_S5K6A1GX
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_S5K6A1GX(MMP_UBYTE ubFrameRate)
{
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_SetPreviewMode_S5K6A1GX(MMP_USHORT usPreviewmode, MMP_USHORT usPhasecount)
{
    S5K6A1GX_Sensor_PreviewMode(usPreviewmode, usPhasecount);

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangePreviewMode_S5K6A1GX(
                        MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
                        MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
    S5K6A1GX_Sensor_PreviewMode(usNewPreviewmode, usNewPhasecount);

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangeMode_S5K6A1GX(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
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
            MMPF_Sensor_SetPreviewMode_S5K6A1GX(0, 0);
            // wait 3 frame ...
            MMPF_Sensor_WaitFrame(5);
            break;
        case 1:
            MMPF_Sensor_SetPreviewMode_S5K6A1GX(1, 0);
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
static MMP_ERR  MMPF_Sensor_InitializeVIF_S5K6A1GX(void)
{
    AITPS_MIPI  pMIPI = AITC_BASE_MIPI;

    // register sensor (functions and variables)
	ISP_IF_SNR_Register();
	
	// register motor
	ISP_IF_MOTOR_Register();

	// register 3A
	ISP_IF_3A_Register();

	// initialize 3A
	ISP_IF_3A_Init();

#if 0  //SENSOR_IF_MIPI		
    //pMIPI->MIPI_DATA1_DEL = 0x30; // From KW
    //pMIPI->MIPI_DATA2_DEL = 0x30; // From KW    
    
    MMPF_VIF_RegisterInputInterface(MMPF_VIF_MIPI);
#else

    MMPF_VIF_RegisterInputInterface(MMPF_VIF_PARALLEL);

#endif

    
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_InitializeISP
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_InitializeISP_S5K6A1GX(void)
{
    return  MMP_ERR_NONE;
}

//============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetAFPosition_S5K6A1GX
//  Description :
// ---------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_S5K6A1GX(MMP_UBYTE ubPos)
{
#if 1
	//if (gISPConfig.AutoFocusMode == VENUS_AF_MODE_MANUAL) {
		//gAFConfig.AFPos = ubPos;
		ISP_IF_AF_SetPos(ubPos);
	//}
#endif	
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Do3AOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_Do3AOperation_S5K6A1GX(void)
{
	ISP_IF_R_GetAEAcc();
	ISP_IF_R_GetAWBAcc();
	
	//ISP_IF_R_Do3A();
	ISP_IF_R_DoAE();
	ISP_IF_R_DoAWB();
    
    //RTNA_DBG_Str(0, "Do3AOperation_S5K6A1GX()\r\n");

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Do3AOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_S5K6A1GX(void)
{
#if SUPPORT_AUTO_FOCUS
    if(gbAutoFocus == MMP_TRUE){
	    //VR_AF_GetAcc();
	    ISP_IF_R_GetAFAcc();
	    ISP_IF_R_DoAF();
	}
#endif

    //RTNA_DBG_Str(0, "Do3AOperation_S5K6A1GX()\r\n");
    
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoIQOperation_S5K6A1GX
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoIQOperation_S5K6A1GX(void)
{
	// set IQ at frame end to ensure frame sync
	ISP_IF_R_DoIQ();

	//RTNA_DBG_Str(0, "DoIQOperation_OV5650()\r\n");
    
    return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_S5K6A1GX(MMP_USHORT usMode)
{
	RTNA_DBG_Str(0, "MMPF_SetLightFreq_S5K6A1GX : ");
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

static MMP_ERR  MMPF_SetStableState_S5K6A1GX(MMP_BOOL bStableState)
{
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_S5K6A1GX(MMP_BOOL bHistEQState)
{

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_S5K6A1GX(MMP_USHORT usAddr, MMP_USHORT usData)
{
    MMPF_I2cm_WriteReg(usAddr, usData);
    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_S5K6A1GX(MMP_USHORT usAddr, MMP_USHORT *usData)
{
    MMPF_I2cm_ReadReg(usAddr, usData);
    return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_S5K6A1GX(MMP_USHORT imgSceneID)
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

	RTNA_DBG_Str(3, "MMPF_SetImageScene_S5K6A1GX : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_S5K6A1GX(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAEMode_S5K6A1GX : ");
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

static MMP_ERR  MMPF_Sensor_SetAFMode_S5K6A1GX(MMP_UBYTE ubAFMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_S5K6A1GX : ");
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
static MMP_ERR  MMPF_Sensor_SetImageEffect_S5K6A1GX(MMP_USHORT imgEffectID)
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

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_S5K6A1GX : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_S5K6A1GX(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_S5K6A1GX : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

   	cAWBtype = ubType;
#endif
    return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_S5K6A1GX(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_S5K6A1GX : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);

    return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_S5K6A1GX(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_S5K6A1GX : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);

    return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_S5K6A1GX(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_S5K6A1GX : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_S5K6A1GX(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_S5K6A1GX : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_S5K6A1GX(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_S5K6A1GX : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);

    return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetBacklight_S5K6A1GX(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_S5K6A1GX : ");
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


static void  MMPF_Sensor_SetCaptureISPSetting_S5K6A1GX(MMP_UBYTE usCaptureBegin)
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
static void  MMPF_Sensor_SetISPWindow_S5K6A1GX(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
    //gSensorFunc.SetISPWindows(ulWidth,ulHeight);
    return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_S5K6A1GX(MMP_USHORT usPreviewmode)
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

static void SetAFWin_S5K6A1GX(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
                    MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

    DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_S5K6A1GX(MMP_UBYTE enable)
{
//    if (enable) {
        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
        ISP_IF_AF_Control(ISP_AF_START);
        //actually calling SNR_S5K6A1GX_SetAutoFocusControl()
//    } else {
//        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
//        ISP_IF_AF_Control(ISP_AF_STOP);
//    }
}

static MMP_ERR MMPF_Sensor_SetAEEnable_S5K6A1GX(MMP_UBYTE bEnable)  
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

static MMP_ERR MMPF_Sensor_SetAWBEnable_S5K6A1GX(MMP_UBYTE bEnable) 
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
static void SetAEsmooth_S5K6A1GX(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_S5K6A1GX(MMP_UBYTE ev)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_S5K6A1GX : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	RTNA_DBG_Byte(3, ev);
	RTNA_DBG_Str(3, "\r\n");
   
	ISP_IF_AE_SetEV(ev);

    return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_S5K6A1GX     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_S5K6A1GX(void)
{
    //return 0;
    return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_S5K6A1GX  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_S5K6A1GX       (void){return 0;}
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

MMPF_SENSOR_FUNCTION  SensorFunction_S5K6A1GX =
{
    MMPF_Sensor_Initialize_S5K6A1GX,
    MMPF_Sensor_InitializeVIF_S5K6A1GX,
    MMPF_Sensor_InitializeISP_S5K6A1GX,
    MMPF_Sensor_PowerDown_S5K6A1GX,
    MMPF_Sensor_SetFrameRate_S5K6A1GX,
    MMPF_Sensor_ChangeMode_S5K6A1GX,
    MMPF_Sensor_ChangePreviewMode_S5K6A1GX,
    MMPF_Sensor_SetPreviewMode_S5K6A1GX,
    MMPF_Sensor_SetReg_S5K6A1GX,
    MMPF_Sensor_GetReg_S5K6A1GX,
    MMPF_Sensor_Do3AOperation_S5K6A1GX,
    MMPF_Sensor_DoAFOperation_S5K6A1GX,
    MMPF_Sensor_DoIQOperation_S5K6A1GX,
    MMPF_Sensor_SetImageEffect_S5K6A1GX,
    MMPF_SetLightFreq_S5K6A1GX,
    MMPF_SetStableState_S5K6A1GX,
    MMPF_SetHistEQState_S5K6A1GX,
    MMPF_Sensor_SetAFPosition_S5K6A1GX,
    SetAFWin_S5K6A1GX,
    SetAFEnable_S5K6A1GX,
    MMPF_Sensor_SetAEEnable_S5K6A1GX,
    MMPF_Sensor_SetAWBEnable_S5K6A1GX,
    MMPF_Sensor_SetExposureValue_S5K6A1GX,
    SetAEsmooth_S5K6A1GX,
    MMPF_Sensor_SetImageScene_S5K6A1GX,
    GetAFPosition_S5K6A1GX,
    GetAFEnable_S5K6A1GX,
    GetExposureValue_S5K6A1GX,
    GetAEsmooth_S5K6A1GX,
    MMPF_Sensor_SetAWBType_S5K6A1GX,
    MMPF_Sensor_SetContrast_S5K6A1GX,
    MMPF_Sensor_SetSaturation_S5K6A1GX,
    MMPF_Sensor_SetSharpness_S5K6A1GX,
    MMPF_Sensor_SetHue_S5K6A1GX,
    MMPF_Sensor_SetGamma_S5K6A1GX,
    MMPF_Sensor_SetBacklight_S5K6A1GX,
    MMPF_Sensor_SetAEMode_S5K6A1GX,
    MMPF_Sensor_SetAFMode_S5K6A1GX,    
    MMPF_Sensor_SetCaptureISPSetting_S5K6A1GX,
    MMPF_Sensor_SetISPWindow_S5K6A1GX,
    MMPF_Sensor_CheckPreviewAbility_S5K6A1GX
};

#if (SENSOR_ID_S5K6A1GX== 0)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_S5K6A1GX;
#endif

#if (SENSOR_ID_S5K6A1GX== 1)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_S5K6A1GX;
#endif

#endif  //BIND_SENSOR_S5K6A1GX
#endif
