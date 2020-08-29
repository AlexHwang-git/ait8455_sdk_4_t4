//==============================================================================
//
//  File        : sensor_ov4689.c
//  Description : Firmware Sensor Control File
//  Author      : 
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if defined(DSC_MP3_P_FW)||defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_OV4689

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
#include "mmpf_usbpccam.h"
#include "mmpf_dram.h"
#include "mmpd_system.h"
#include "mmpf_usbuvc.h"

#include "sensor_OV4689.h"
#include "Led_ctl.h"

#define WDR_SIM_EN      (0)

#if WDR_SIM_EN
#define DO_AE_GAP       (2)
#define LONG_SHUTTER_RATIO  (3) // 3x ratio of normal shutter
#else
#define DO_AE_GAP       (3)
#define LONG_SHUTTER_RATIO  (0) // disable long shutter mode
#endif

MMPF_I2CM_ATTRIBUTE m_snr_I2cmAttribute = {
#if VIF_CTL==VIF_CTL_VIF0
SENSOR_I2CM_ID, SENSOR_I2C_ADDR_OV4689, 16, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE,MMP_FALSE, 0, 0, 1, MMPF_I2CM_SPEED_HW_250K, NULL, NULL
#endif
#if VIF_CTL==VIF_CTL_VIF1
SENSOR_I2CM_ID, 0x10, 16, 16, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE,MMP_FALSE, 0, 0, 0/*1*/, MMPF_I2CM_SPEED_HW_400K, NULL, NULL
#endif
};

extern MMP_USHORT  gsSensorLCModeWidth;
extern MMP_USHORT  gsSensorLCModeHeight;
extern MMP_USHORT  gsSensorMCModeWidth;
extern MMP_USHORT  gsSensorMCModeHeight;
extern MMP_USHORT  gsCurPreviewMode, gsCurPhaseCount;
extern MMP_USHORT  gsSensorMode;
extern MMP_USHORT  m_usVideoQuality;
extern MMP_USHORT  m_gsISPCoreID;

extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_BOOL    gbAutoFocus;
extern MMP_UBYTE   gbSensorPowerOffMode ; // default : power off mode

extern MMP_ULONG   glPccamResolution;
extern MMP_LONG    AFNearAdjustValue;
extern MMP_LONG    AFInfiAdjustValue;

const ISP_UINT8 Sensor_IQ_CompressedText[] = {
    #if (CHIP == P_V2)
    #include "isp_8453_iq_data_v2_OV4689.xls.ciq.txt"
    #endif
    #if (CHIP == VSN_V2)
    #include "isp_8453_iq_data_v2_OV4689.xls.ciq.txt"
    #endif
    #if (CHIP == VSN_V3) // IQ table : V3 TBD
    #if ISP_GNR_OFF==0
    #include "isp_8453_iq_data_v2_OV4689.xls.ciq.txt" 
    #else
    #include "isp_8453_iq_data_v2_OV4689.xls.ciq.txt"
    #endif
    #endif
} ;

MMP_ULONG g_SNR_ov4689_LineCntPerSec = 1;

void SNR_OV4689_InitSensor_Customer(void);
void SNR_OV4689_SetSensorResolution_Customer(ISP_SENSOR_RESOL res);
void MMPF_Sensor_SetSensorGain_OV4689(MMP_ULONG gain);
void MMPF_Sensor_SetSensorShutter_OV4689(MMP_ULONG shutter, MMP_ULONG vsync);

#if SUPPORT_AUTO_FOCUS
void MOTOR_OV4689_VCM_InitMotor_Customer(void);
void MOTOR_OV4689_VCM_SetAFPosition_Customer(MMP_USHORT af_pos);
void MOTOR_OV4689_VCM_SetAFHomePosition_Customer(void);
#endif

#define  SENSOR_WIDTH_16_9_BINNING  SENSOR_16_9_W_60FPS  // 1280
#define  SENSOR_HEIGHT_16_9_BINNING SENSOR_16_9_H_60FPS  // 720

#define  SENSOR_WIDTH_16_9  1920 
#if SENSOR_IN_H > SENSOR_16_9_H
#define  SENSOR_HEIGHT_16_9 (SENSOR_IN_H) 
#else
#define  SENSOR_HEIGHT_16_9 (1080) 
#endif

MMP_USHORT gsSensorWidthByFOV ;
MMP_USHORT gsSensorHeightByFOV ;
 
ISP_SENSOR_RESOL gsCurSensorEnumRes = ISP_SENSOR_RESOL_NULL ;

extern MMP_USHORT  gsCurFrameRatex10[];
extern MMP_UBYTE   gbDramID;
extern MMPF_SENSOR_FUNCTION *gsSensorFunction;

MMP_USHORT  gsVideoFormat;

//------------------------------------------------------------------------------
//  Function    : OV4689_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
static void OV4689_Sensor_PreviewMode(MMP_USHORT usPreviewmode)
{
	gsCurPreviewMode = usPreviewmode;
	//gsCurPhaseCount = usPhasecount;

	switch (usPreviewmode) {
	case 0:
		dbg_printf(0,"->1152x648\r\n");
        //ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518
        gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
        gSystemMode = VENUS_SYSTEM_MODE_PREVIEW;//ISP_IF_SNR_SetBeforePreview();
		ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
        dbg_printf(0,"gsCurFrameRatex10 : %d\r\n",gsCurFrameRatex10[0]);
        SNR_OV4689_SetSensorResolution_Customer(ISP_SENSOR_RESOL_800x600);
        //set color transform 16 ~ 235
        //MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH_1, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
        //ISP_IF_3A_Control(ISP_3A_ENABLE);
        gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE);
        break;

	case 1:
		dbg_printf(0,"->1920x1080\r\n");
		switch (gsSensorMode) {
		#if defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(VIDEO_R_EN)
		case SENSOR_VIDEO_MODE:
			//RTNA_DBG_Str(3, "SENSOR_VIDEO_MODE\r\n");
			//ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
			gSystemMode = VENUS_SYSTEM_MODE_PREVIEW;//ISP_IF_SNR_SetBeforePreview();
			ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
            dbg_printf(0,"(1) gsCurFrameRatex10 : %d\r\n",gsCurFrameRatex10[0]);
            SNR_OV4689_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);
			//ISP_IF_3A_Control(ISP_3A_ENABLE);
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE);
			break;
 			//set color transform 16 ~ 235
            //MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH_1, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
			
		#endif
		}
		break;

	case 2:
		dbg_printf(0,"->MJ 1280x720\r\n");
		switch (gsSensorMode) {
		#if defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(VIDEO_R_EN)
		case SENSOR_VIDEO_MODE:
			//RTNA_DBG_Str(3, "SENSOR_VIDEO_MODE\r\n");
			//ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
			gSystemMode = VENUS_SYSTEM_MODE_PREVIEW;//ISP_IF_SNR_SetBeforePreview();
			ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
            dbg_printf(0,"(2) gsCurFrameRatex10 : %d\r\n",gsCurFrameRatex10[0]);
            SNR_OV4689_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1280x720);
			//ISP_IF_3A_Control(ISP_3A_ENABLE);
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE);
			break;
 			//set color transform 16 ~ 235
            //MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH_1, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
		#endif
		}
		break;

	default:
		dbg_printf(0,"->Default 1920x1080\r\n");
		switch (gsSensorMode) {
		#if defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(VIDEO_R_EN)
		case SENSOR_VIDEO_MODE:
			//RTNA_DBG_Str(3, "SENSOR_VIDEO_MODE\r\n");
			//ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
			gSystemMode = VENUS_SYSTEM_MODE_PREVIEW;//ISP_IF_SNR_SetBeforePreview();
			ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
            dbg_printf(0,"(Def) gsCurFrameRatex10 : %d\r\n",gsCurFrameRatex10[0]);
            SNR_OV4689_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);
			//ISP_IF_3A_Control(ISP_3A_ENABLE);
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE);
			break;
 			//set color transform 16 ~ 235
            //MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH_1, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
		#endif
		}
		break;

	}
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Initialize
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_Initialize_OV4689(void)
{
    MMP_USHORT  max_ep_fps ;
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	AITPS_PAD   pPAD  = AITC_BASE_PAD;
	AITPS_VIF	pVIF  = AITC_BASE_VIF;
	AITPS_GBL	pGBL  = AITC_BASE_GBL ;
	
	STREAM_SESSION *ss;
	PCAM_USB_VIDEO_RES res ;
    RES_TYPE_CFG *cur_res;

	#if (CHIP==P_V2)
    pPAD->PAD_PSGPIO = PAD_E8_CURRENT | PAD_PULL_LOW;
    pGPIO->GPIO_EN[3] |= (1 << 16); // config as output mode
	pGPIO->GPIO_DATA[3] |= (1 << 16);  // Turn On Sensor Power
	RTNA_WAIT_MS(1);
	#endif
    #if (CHIP==VSN_V3)
    //Pin(CGPIO5->SEN_PWR_EN) output HIGH
	MMPF_PIO_EnableGpioMode(CAM_PWR_EN, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(CAM_PWR_EN, MMP_TRUE);
	MMPF_PIO_SetData(CAM_PWR_EN, MMP_TRUE);	
	RTNA_WAIT_MS(1);
    #endif    
	
	gsCurSensorEnumRes = ISP_SENSOR_RESOL_NULL ;

    res = GetFirstStreamingRes();

    gsSensorWidthByFOV = SENSOR_WIDTH_16_9 ;  // 1920
    gsSensorHeightByFOV = SENSOR_HEIGHT_16_9; // 1080

	//Init globol variable used by DSC
	gsSensorLCModeWidth     = gsSensorWidthByFOV;  // 1920
	gsSensorLCModeHeight    = gsSensorHeightByFOV; // 1080

    max_ep_fps = gsCurFrameRatex10[0] ;

    if(max_ep_fps > 300 ) {
    	gsSensorMCModeWidth     = SENSOR_WIDTH_16_9_BINNING;  // 1280
    	gsSensorMCModeHeight    = SENSOR_HEIGHT_16_9_BINNING; // 720
    
    } else {
    	gsSensorMCModeWidth     = gsSensorWidthByFOV;  // 1920
    	gsSensorMCModeHeight    = gsSensorHeightByFOV; // 1080
    } 	
    
    #if (CHIP==P_V2)
	pPAD->PAD_PSCK = PAD_E8_CURRENT | PAD_E4_CURRENT | PAD_PULL_HIGH;
	pPAD->PAD_PSDA = PAD_E4_CURRENT | PAD_PULL_HIGH;
	pPAD->PAD_PSEN = PAD_E4_CURRENT | PAD_PULL_HIGH;
    #endif
    #if (CHIP==VSN_V2)||(CHIP==VSN_V3)
    pGBL->GBL_IO_CFG_PSNR[0] = 0x0A ;
    // dbg_printf(3,"Pull Up SEN/SCK/SDA:%x\r\n",pGBL->GBL_IO_CFG_PSNR2);
    // Pull SCK/SDA/SEN to Low
    pGBL->GBL_IO_CFG_PSNR2[0] = PAD_E8_CURRENT | PAD_E4_CURRENT | PAD_PULL_HIGH;
    pGBL->GBL_IO_CFG_PSNR2[1] = PAD_E4_CURRENT | PAD_PULL_HIGH;
    pGBL->GBL_IO_CFG_PSNR2[2] = PAD_E4_CURRENT | PAD_PULL_HIGH;
    #endif

    dbg_printf(0,"SNR_Init\r\n");

	SNR_OV4689_InitSensor_Customer();

    cur_res = GetResCfg(res);
    dbg_printf(0,"gsVideoFormat: %d\r\n",gsVideoFormat);
    dbg_printf(0,"cur_res->res_w: %d\r\n",cur_res->res_w);
    dbg_printf(0,"cur_res->res_h: %d\r\n",cur_res->res_h);


    if( (gsVideoFormat == PCAM_USB_VIDEO_FORMAT_MJPEG) && 
        ((cur_res->res_w == 1280) && (cur_res->res_h == 720)) )
    {
        dbg_printf(0,"MJPEG 1280x720\r\n");
        //gsSensorFunction->MMPF_Sensor_SetPreviewMode(2);
		gsSensorFunction->MMPF_Sensor_SetPreviewMode(1);
    }
    else
    {
        dbg_printf(0,"Others...\r\n");
        gsSensorFunction->MMPF_Sensor_SetPreviewMode(1);
    }


#if SUPPORT_AUTO_FOCUS && (AF_EN!=255)
    MMPF_PIO_EnableGpioMode(AF_EN, MMP_TRUE);
    MMPF_PIO_EnableOutputMode(AF_EN, MMP_TRUE);
    MMPF_PIO_SetData(AF_EN, MMP_TRUE);
    RTNA_WAIT_MS(20);
#endif

    MMPF_PIO_EnableGpioMode(PWR_LED, MMP_TRUE);
    MMPF_PIO_EnableOutputMode(PWR_LED, MMP_TRUE);
    MMPF_PIO_SetData(PWR_LED, MMP_TRUE);
    RTNA_WAIT_MS(1);


#if SUPPORT_AUTO_FOCUS
    MOTOR_OV4689_VCM_InitMotor_Customer();
#endif

#if SUPPORT_LED_DRIVER
    //MMPF_PIO_EnableGpioMode(LED_HWEN, MMP_TRUE);
    //MMPF_PIO_EnableOutputMode(LED_HWEN, MMP_TRUE);
    //MMPF_PIO_SetData(LED_HWEN, MMP_TRUE);
    //RTNA_WAIT_MS(20);

    // Init LED Driver
	LED_Driver_Init();

    // Brightness 1~0x7ff(2047)
    LED_Driver_SetBrightnessCode(LM36922_DEF_BRIGHT);

    // Get Brightness code
    LED_Driver_GetBrightnessCode();

    // Turn On LED
	LED_Driver_Enable(1);
	RTNA_WAIT_MS(1);
#endif

	RTNA_DBG_Str(3, "->Sensor.Inited\r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_OV4689
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_PowerDown_OV4689(void)
{
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	AITPS_MIPI pMIPI = AITC_BASE_MIPI;
	AITPS_GBL  pGBL = AITC_BASE_GBL;
	MMP_UBYTE   snr_id = 0;
	MMP_USHORT data;
    // Wait Sensor Task to become IDLE
    MMPF_Sensor_WaitTaskIdle();
    
    gsSensorFunction->MMPF_Sensor_GetReg(0x301A, &data);   
    //Put standby mode
    data = data & ~0x04 ;
    gsSensorFunction->MMPF_Sensor_SetReg(0x301A,data);
	
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    for(snr_id=0;snr_id < VIF_NUM;snr_id++) {
        pMIPI->MIPI_CLK_CFG[snr_id] &= ~(MIPI_CSI2_EN);
        pMIPI->MIPI_DATA0_CFG[snr_id] &= ~MIPI_DAT_LANE_EN ;
        pMIPI->MIPI_DATA1_CFG[snr_id] &= ~MIPI_DAT_LANE_EN ;
    }
    #endif
    #if (CHIP == P_V2)
	pMIPI->MIPI_DATA1_CFG &= ~(MIPI_CSI2_EN);
	pMIPI->MIPI_DATA2_CFG &= ~(MIPI_CSI2_EN);
	#endif
	//MMPF_VIF_EnableInputInterface(MMP_FALSE);

#if SUPPORT_AUTO_FOCUS && (AF_EN!=255)
    MMPF_PIO_EnableGpioMode(AF_EN, MMP_TRUE);
    MMPF_PIO_EnableOutputMode(AF_EN, MMP_TRUE);
    MMPF_PIO_SetData(AF_EN, MMP_FALSE);
    RTNA_WAIT_MS(20);
#endif

	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);//Pin output low
	//MMPF_OS_Sleep_MS(3);
	RTNA_WAIT_MS(3);

    if(gbSensorPowerOffMode==0) {
	#if (CHIP==VSN_V2)
        if(gbDramID == MMPF_DRAMID_WINBOND_SDR16){
            MMPF_PIO_SetData(GPIO_SENSOR_PSEN,FALSE);
            MMPF_PIO_EnableOutputMode(GPIO_SENSOR_PSEN,TRUE);
        }
        else{
            MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);
            MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);  //Pin output high
        }

	#endif 

	#if (CHIP==VSN_V3)
	    MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);
	    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_FALSE);  //Pin output LOW
	#endif   	
    }
    
    if(gbSensorPowerOffMode==0) {
    #if (CHIP==P_V2)
    	pGPIO->GPIO_DATA[3] &= ~(1 << 16);  // Turn Off Sensor Power
    #endif
    #if (CHIP==VSN_V3)
	//Pin(CGPIO5->SEN_PWR_EN) output LOW
	MMPF_PIO_EnableGpioMode(CAM_PWR_EN, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(CAM_PWR_EN, MMP_TRUE);
	MMPF_PIO_SetData(CAM_PWR_EN, MMP_FALSE);
    #endif
    //	RTNA_WAIT_MS(20);
    }

#if SUPPORT_LED_DRIVER
	//MMPF_PIO_EnableGpioMode(LED_HWEN, MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(LED_HWEN, MMP_TRUE);
	//MMPF_PIO_SetData(LED_HWEN, MMP_FALSE);
	//RTNA_WAIT_MS(20);

    // Turn Off LED
	LED_Driver_Enable(0);	
#endif
        
	RTNA_WAIT_MS(1);
	
	pVIF->VIF_SENSR_CLK_CTL[snr_id] &= (~ VIF_SENSR_CLK_EN) ;   //output sensor main clock
	
    if(gbSensorPowerOffMode==0) {
    	// Pull RST / SEN / SCK / SDA to low
        MMPF_Sensor_InitPad();
	    //MMPF_OS_Sleep_MS(20);
	    RTNA_WAIT_MS(20);
	}
	RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_OV4689() \r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_OV4689
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_OV4689(MMP_UBYTE ubFrameRate)
{
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR MMPF_Sensor_SetPreviewMode_OV4689(MMP_USHORT usPreviewmode)
{
	OV4689_Sensor_PreviewMode(usPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR MMPF_Sensor_ChangePreviewMode_OV4689(
						MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
						MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
	OV4689_Sensor_PreviewMode(usNewPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------
/*static*/ MMP_ERR MMPF_Sensor_ChangeMode_OV4689(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
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
			MMPF_Sensor_SetPreviewMode_OV4689(0);
			// wait 3 frame ...
		   // MMPF_Sensor_WaitFrame(5);
			break;
		case 1:
			MMPF_Sensor_SetPreviewMode_OV4689(1);
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
static MMP_ERR  MMPF_Sensor_InitializeVIF_OV4689(void)
{
	AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
	
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	MMP_UBYTE   vid = 0;
    #endif
	
	//RTNA_DBG_Str(3, "ISP_IF_SNR_Register...S\r\n");
	//ISP_IF_SNR_Register();
    //RTNA_DBG_Str(3, "ISP_IF_SNR_Register...E\r\n");

	
    #if (CHIP==P_V2)
	pMIPI->MIPI_DATA1_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA2_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA3_SOT = 0x10; // From KW
	pMIPI->MIPI_DATA4_SOT = 0x10; // From KW
    #endif
    
    
    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
    MMPF_VIF_RegisterInputInterface(MMPF_VIF_PARALLEL);
    pVIF->VIF_SENSR_CTL[vid] = VIF_SENSR_LATCH_NEG | VIF_HSYNC_POLAR | VIF_12BPP_OUT_EN;
    #else
    MMPF_VIF_RegisterInputInterface(MMPF_VIF_MIPI);
    #endif

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_InitializeISP
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_InitializeISP_OV4689(void)
{
	volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;

	// register sensor (functions and variables)
	RTNA_DBG_Str(0, "ISP_IF_LIB_Init...\r\n");
	ISP_IF_LIB_Init();
	
	// check if ISP Lib and IQ are mismatched.
	if (ISP_IF_LIB_CompareIQVer() != MMP_ERR_NONE) {
		RTNA_DBG_Str(0, "Wrong ISP lib version!\r\n");
	    return MMP_ERR_NONE;
	}

	//RTNA_DBG_Str(0, "ISP_IF_3A_Init...S\r\n");
	// initialize 3A
	ISP_IF_3A_Init();

#if SUPPORT_AUTO_FOCUS
	// set AF type (ISP_AF_TYPE_ONE_SHOT / ISP_AF_TYPE_CONTINUOUS)
	ISP_IF_AF_SetType(ISP_AF_TYPE_CONTINUOUS);
#endif

	//RTNA_DBG_Str(0, "ISP_IF_3A_Init...E\r\n");

#if (CHIP == P_V2)
	if (m_gsISPCoreID == 868) {
		// ISP pipeline selection (PYTHON_V1 only)
		//APICAL_XBYTE[APICAL_CTL_REG1] &= ~(0x80); // clear auto size
		pISP[0x0B] = 0x30;
		pISP[0x09] = 0x0C;
	}
#endif

	return  MMP_ERR_NONE;
}

//============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetAFPosition_OV4689
//  Description :
// ---------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_OV4689(MMP_UBYTE ubPos)
{
#if SUPPORT_AUTO_FOCUS
	//if (gISPConfig.AutoFocusMode == VENUS_AF_MODE_MANUAL) {
		//gAFConfig.AFPos = ubPos;
		if(gbAutoFocus == MMP_FALSE){
		//RTNA_DBG_Str0("AF pos = "); RTNA_DBG_Long0(ubPos);RTNA_DBG_Str0("\r\n");
		
		ISP_IF_AF_SetPos(ubPos, 8/*0*/);

        MOTOR_OV4689_VCM_SetAFPosition_Customer(ISP_IF_AF_GetPos(0));

		}
	//}
#endif
	return  MMP_ERR_NONE;
}
// sean@2011_01_24 3A interface changed for AE hunting issue [START]

ISP_UINT32 gISPFrameCnt;
extern MMP_ULONG	AWBAcc_Buffer[256* 5];
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Do3AOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAWBOperation_OV4689(void)
{
	static ISP_UINT32 increase_excute = 12;

	if(increase_excute > 0)
	{
		if(increase_excute == 12) {ISP_IF_AWB_SetFastMode(1);}
		increase_excute--;
		ISP_IF_AWB_Execute();
		if(increase_excute == 1) {ISP_IF_AWB_SetFastMode(0);}
	}	

	switch (gISPFrameCnt % 3){
	case 1:
		//ISP_IF_AWB_GetHWAcc(1);
		ISP_IF_AWB_Execute();
		ISP_IF_IQ_SetAWBGains(ISP_IF_AWB_GetGainR(), ISP_IF_AWB_GetGainG(), ISP_IF_AWB_GetGainB(), ISP_IF_AWB_GetGainBase());		
		break;
	case 2:		
		ISP_IF_CALI_Execute();
		break;
	}

	return  MMP_ERR_NONE;
}

ISP_UINT32 gISPFrameCnt = 0;
#define FrameDivsor 5 // Gason@20120111, by OV suggestion, under 15 fps, dummy line will work every 5 frame.
ISP_UINT32 frame_div = 5;
ISP_UINT32 dgain;
#define ISP_DGAIN_BASE		(0x200)
#define MAX_SENSOR_GAIN		(16)
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_ST
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_ST_OV4689(void)
{
    if(ISP_IF_3A_GetSwitch(ISP_3A_ALGO_AE) != 1) return MMP_ERR_NONE;

	switch (gISPFrameCnt % DO_AE_GAP)
	{
		case 0:
			ISP_IF_AE_Execute();
			gsSensorFunction->MMPF_Sensor_SetSensorShutter(0, 0);
			gsSensorFunction->MMPF_Sensor_SetSensorGain(ISP_IF_AE_GetGain());
		break;
		case 1:
   
		break;
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_END
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_END_OV4689(void)
{
	gISPFrameCnt++;
    if(ISP_IF_3A_GetSwitch(ISP_3A_ALGO_AE) != 1) return MMP_ERR_NONE;

	switch (gISPFrameCnt % DO_AE_GAP){
	case 0:
		ISP_IF_AE_GetHWAcc(1);
		//RTNA_DBG_Str(0, "ISP_IF_AE_GetHWAcc ()\r\n");
		break;
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_OV4689(void)
{
#if SUPPORT_AUTO_FOCUS
	if(gbAutoFocus == MMP_TRUE){
		//VR_AF_GetAcc();
		ISP_IF_AF_GetHWAcc(1);
		//ISP_IF_R_DoAF();
	}
#endif

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_FrameStart_OV4689(void)
{
#if SUPPORT_AUTO_FOCUS //&& (AF_EN!=255)
	if(gbAutoFocus == MMP_TRUE){
		//VR_AF_GetAcc();
		//ISP_IF_R_GetAFAcc();
		//ISP_IF_R_DoAF();
		{
           switch (gISPFrameCnt % 1) {
            case 0:
                ISP_IF_AF_Execute();
                MOTOR_OV4689_VCM_SetAFPosition_Customer(ISP_IF_AF_GetPos(0));
                break;
            }
		}
	}
#endif

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoIQOperation_OV4689
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoIQOperation_OV4689(void)
{
    MMP_USHORT i,w[2] = {0,0} ,h[2] = {0,0}; // endpoint 0 & 1
    STREAM_SESSION *ss ;
    STREAM_CFG *cur_pipe;
    MMP_UBYTE pipe0_cfg , pipe1_cfg  ;
    

	if(gISPFrameCnt % 3 != 0) //20131016
		return;
    
    pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_pipe->pipe_cfg);//(cur_pipe->pipe_cfg & PIPE_CFG_MASK) ;
    pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_pipe->pipe_cfg);//( (cur_pipe->pipe_cfg >> 4) & PIPE_CFG_MASK) ;


    for(i=0;i< VIDEO_EP_END();i++) {
        ss = MMPF_Video_GetStreamSessionByEp(i);
        if(ss->tx_flag & SS_TX_STREAMING) {
            cur_pipe = usb_get_cur_image_pipe_by_epid(i);
        } else {
            cur_pipe = 0 ;
        }
        if(cur_pipe) {
            PIPE_PATH pipe = PIPE_0 ;
            if( PIPE_EN(cur_pipe->pipe_en) & PIPE1_EN)  {
                pipe = PIPE_1 ;            
            }
            else if( PIPE_EN(cur_pipe->pipe_en) & PIPE0_EN)  {
                pipe = PIPE_0 ;
            }
            else {
                pipe = PIPE_2 ;
            }
            w[i] = cur_pipe->pipe_w[pipe] ;
            h[i] = cur_pipe->pipe_h[pipe] ;
        }
    }

	// set IQ at frame end to ensure frame sync
	ISP_IF_IQ_SetAll();
	//////////////////20121228
	if(w[1] && h[1]) {
	
		if((w[1]==352) && (h[1]==288)){
			ISP_IF_IQ_SetCIGainRatio(256); //x128
			ISP_IF_IQ_SetCIYMHClipRatio(256);//x128
			ISP_IF_IQ_SetYEESclGainRatio(128);
		
		}else if((w[1]==640) && (h[1]==360)){
			ISP_IF_IQ_SetCIGainRatio(42); //x128
			ISP_IF_IQ_SetCIYMHClipRatio(42);
			ISP_IF_IQ_SetYEESclGainRatio(128);

		}
	
	    //ISP_IF_IQ_SetMTF(w[1],h[1]);  //res = PCAM_USB_VIDEO_RES GetResIndexByEp(0/1);
	    //dbg_printf(3,"[MTF]:(w,h) = (%d,%d)\r\n",w[1],h[1]);
	}
	else {
		if((w[0]==352) && (h[0]==288)){
			ISP_IF_IQ_SetCIGainRatio(256); //x128
			ISP_IF_IQ_SetCIYMHClipRatio(256);//x128
		
		}else if((w[0]==640) && (h[0]==360)){
			ISP_IF_IQ_SetCIGainRatio(42); //x128
			ISP_IF_IQ_SetCIYMHClipRatio(42);
		}	
	    //ISP_IF_IQ_SetMTF(w[0],h[0]);
	    //dbg_printf(3,"[MTF]:(w,h) = (%d,%d)\r\n",w[0],h[0]);
	}
	  
//	gnrOffset[0] = 0;
//	gnrOffset[1] = 2;
//	gnrOffset[2] = -3;
//	gnrOffset[3] = 4;
//	gnrOffset[4] = -5;
//	gnrOffset[5] = 6;
//	gnrOffset[6] = -7;
//	gnrOffset[7] = 8;	
//	ISP_IF_IQ_SetGNRGainOffset(gnrOffset); //test 20130520
	
	ISP_IF_IQ_CheckBypass();

	//RTNA_DBG_Str(0, "DoIQOperation_OV4689()\r\n");

	return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_OV4689(MMP_USHORT usMode)
{
/*	RTNA_DBG_Str(3, "MMPF_SetLightFreq_OV4689 : ");
	RTNA_DBG_Short(3, usMode);
	RTNA_DBG_Str(3, "\r\n");
*/
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

static MMP_ERR  MMPF_SetStableState_OV4689(MMP_BOOL bStableState)
{
	return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_OV4689(MMP_BOOL bHistEQState)
{

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_OV4689(MMP_USHORT usAddr, MMP_USHORT usData)
{
	MMPF_I2cm_WriteReg(&m_snr_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_OV4689(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	*usData = 0xFF;
    MMPF_I2cm_ReadReg(&m_snr_I2cmAttribute, usAddr, usData);
	
	return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_OV4689(MMP_USHORT imgSceneID)
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

	RTNA_DBG_Str(3, "MMPF_SetImageScene_OV4689 : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_OV4689(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAEMode_OV4689 : ");
	RTNA_DBG_Byte(3, ubAEMode);
	RTNA_DBG_Str(3, ": ");
	RTNA_DBG_Byte(3, ubISOMode);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//MMPF_Sensor_CheckFrameEnd(1);
	//Venus_SetAEMode(ubAEMode, ubISOMode);
	if (ubAEMode == 3)
		ISP_IF_AE_SetISO(0);		//sean@2011_01_24 ISP_AE_ISO_AUTO
	else
		ISP_IF_AE_SetISO(ubISOMode);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAFMode_OV4689(MMP_UBYTE ubAFMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_OV4689 : ");
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
static MMP_ERR  MMPF_Sensor_SetImageEffect_OV4689(MMP_USHORT imgEffectID)
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

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_OV4689 : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_OV4689(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_OV4689 : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

	cAWBtype = ubType;
#endif
	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_OV4689(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_OV4689 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_OV4689(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_OV4689 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_OV4689(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_OV4689 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);
	ISP_IF_F_SetCISharpness(ubLevel);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_OV4689(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_OV4689 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_OV4689(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_OV4689 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetBacklight_OV4689(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_OV4689 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

/* sean@2011_01_24, comment out
	if(ubLevel > 1){
		ISP_IF_F_SetWDR(ISP_WDR_DISABLE);
	}
	else{
		ISP_IF_F_SetWDR(ISP_WDR_ENABLE);
	}
*/
	
	return  MMP_ERR_NONE;
}

static void  MMPF_Sensor_SetCaptureISPSetting_OV4689(MMP_UBYTE usCaptureBegin)
{
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	MMP_ULONG ulVIF_Width, ulVIF_Height;
#if (CHIP==P_V2)
	/* ToDo: setting IOS and ISP windows*/
	if(MMP_TRUE == usCaptureBegin){
		ulVIF_Width = (pVIF->VIF_GRAB_PIXL_ED - pVIF->VIF_GRAB_PIXL_ST + 1);
		ulVIF_Height = (pVIF->VIF_GRAB_LINE_ED - pVIF->VIF_GRAB_LINE_ST + 1);
		//gSensorFunc.SetISPWindows(ulVIF_Width,ulVIF_Height);
	}
#endif

	return;
}

/*Prepare for raw preview zoom*/
static void  MMPF_Sensor_SetISPWindow_OV4689(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
	//gSensorFunc.SetISPWindows(ulWidth,ulHeight);
	return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_OV4689(MMP_USHORT usPreviewmode)
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

static void SetAFWin_OV4689(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
					MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

	DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_OV4689(MMP_UBYTE enable)
{
#if SUPPORT_AUTO_FOCUS //&& (AF_EN!=255)
    if (enable) {
		RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
		ISP_IF_AF_Control(ISP_AF_START);
		//actually calling SNR_OV4689_SetAutoFocusControl()
    } else {
        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
        ISP_IF_AF_Control(ISP_AF_STOP);
    }
#endif
}

static MMP_ERR MMPF_Sensor_SetAEEnable_OV4689(MMP_UBYTE bEnable)
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

static MMP_ERR MMPF_Sensor_SetAWBEnable_OV4689(MMP_UBYTE bEnable)
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
static void SetAEsmooth_OV4689(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_OV4689(MMP_UBYTE ev)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_OV4689 : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	RTNA_DBG_Byte(3, ev);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_AE_SetEV(ev);

	return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_OV4689     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_OV4689(void)
{
	//return 0;
	return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_OV4689  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_OV4689       (void){return 0;}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Set3AStatus
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR	MMPF_Sensor_Set3AStatus_OV4689(MMP_BOOL bEnable)
{
	MMPF_Sensor_3A_SetInterrupt(bEnable);

	if (bEnable)
		ISP_IF_3A_Control(ISP_3A_ENABLE);
	else
		ISP_IF_3A_Control(ISP_3A_DISABLE);

	return	MMP_ERR_NONE;
}

static void  MMPF_Sensor_SetColorID_OV4689(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
    MMP_USHORT read_value;

	switch(RotateType)
	{
        case MMPF_SENSOR_ROTATE_NO_ROTATE:
            gsSensorFunction->MMPF_Sensor_GetReg(0x3820, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3820, read_value & ~(0x06));
            gsSensorFunction->MMPF_Sensor_GetReg(0x3821, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3821, read_value | 0x06);
            ISP_IF_IQ_SetColorID(2);
            break;
        
        case MMPF_SENSOR_ROTATE_RIGHT_90:
            break;
        
        case MMPF_SENSOR_ROTATE_RIGHT_180:
            gsSensorFunction->MMPF_Sensor_GetReg(0x3820, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3820, read_value | (0x02/*0x06*/));
            gsSensorFunction->MMPF_Sensor_GetReg(0x3821, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3821, read_value & ~(0x06));
            ISP_IF_IQ_SetColorID(0);
            break;
        
        case MMPF_SENSOR_ROTATE_RIGHT_270:
            break;
        
        case MMPF_SENSOR_ROTATE_H_MIRROR:
            gsSensorFunction->MMPF_Sensor_GetReg(0x3820, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3820, read_value & ~(0x06));
            gsSensorFunction->MMPF_Sensor_GetReg(0x3821, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3821, read_value & ~(0x06));
            ISP_IF_IQ_SetColorID(2);
            break;
        
        case MMPF_SENSOR_ROTATE_V_FLIP:
            gsSensorFunction->MMPF_Sensor_GetReg(0x3820, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3820, read_value | (0x02/*0x06*/));
            gsSensorFunction->MMPF_Sensor_GetReg(0x3821, &read_value);
            gsSensorFunction->MMPF_Sensor_SetReg(0x3821, read_value | (0x06));
            ISP_IF_IQ_SetColorID(0);
            break;
	}

    ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);

	return;
}

MMPF_SENSOR_FUNCTION  SensorFunction_OV4689 =
{
	MMPF_Sensor_Initialize_OV4689,
	MMPF_Sensor_InitializeVIF_OV4689,
	MMPF_Sensor_InitializeISP_OV4689,
	MMPF_Sensor_PowerDown_OV4689,
	MMPF_Sensor_SetFrameRate_OV4689,
	MMPF_Sensor_ChangeMode_OV4689,
	MMPF_Sensor_ChangePreviewMode_OV4689,
	MMPF_Sensor_SetPreviewMode_OV4689,
	MMPF_Sensor_SetReg_OV4689,
	MMPF_Sensor_GetReg_OV4689,
	MMPF_Sensor_DoAWBOperation_OV4689,
	MMPF_Sensor_DoAEOperation_ST_OV4689,
	MMPF_Sensor_DoAEOperation_END_OV4689,
	MMPF_Sensor_DoAFOperation_OV4689,
	MMPF_Sensor_DoAFOperation_FrameStart_OV4689,
	MMPF_Sensor_DoIQOperation_OV4689,
	MMPF_Sensor_SetImageEffect_OV4689,
	MMPF_SetLightFreq_OV4689,
	MMPF_SetStableState_OV4689,
	MMPF_SetHistEQState_OV4689,
	MMPF_Sensor_SetAFPosition_OV4689,
	SetAFWin_OV4689,
	SetAFEnable_OV4689,
	MMPF_Sensor_SetAEEnable_OV4689,
	MMPF_Sensor_SetAWBEnable_OV4689,
	MMPF_Sensor_SetExposureValue_OV4689,
	SetAEsmooth_OV4689,
	MMPF_Sensor_SetImageScene_OV4689,
	GetAFPosition_OV4689,
	GetAFEnable_OV4689,
	GetExposureValue_OV4689,
	GetAEsmooth_OV4689,
	MMPF_Sensor_SetAWBType_OV4689,
	MMPF_Sensor_SetContrast_OV4689,
	MMPF_Sensor_SetSaturation_OV4689,
	MMPF_Sensor_SetSharpness_OV4689,
	MMPF_Sensor_SetHue_OV4689,
	MMPF_Sensor_SetGamma_OV4689,
 //   MMPF_Sensor_SetBacklight_OV4689,
	MMPF_Sensor_SetAEMode_OV4689,
	MMPF_Sensor_SetAFMode_OV4689,
	MMPF_Sensor_SetCaptureISPSetting_OV4689,
	MMPF_Sensor_SetISPWindow_OV4689,
	MMPF_Sensor_CheckPreviewAbility_OV4689,
	MMPF_Sensor_Set3AStatus_OV4689,
	MMPF_Sensor_SetColorID_OV4689,
	MMPF_Sensor_SetSensorGain_OV4689,
	MMPF_Sensor_SetSensorShutter_OV4689
};

// RES_2688x1520_696Mbps(30fps)
MMP_USHORT SNR_OV4689_Reg_Init_Customer[] = {
    0x0103	,	0x01	,			
    0x3638	,	0x00	,			
    0x0300	,	0x00	,			
    0x0302	,	0x1d	,	//	for	696Mbps
    0x0303	,	0x00	,			
    0x0304	,	0x03	,			
    0x030b	,	0x00	,			
    0x030d	,	0x1e	,			
    0x030e	,	0x04	,			
    0x030f	,	0x01	,			
    0x0312	,	0x01	,			
    0x031e	,	0x00	,			
    0x3000	,	0x20	,			
    0x3002	,	0x00	,			
    0x3018	,	0x32	,	//	for	2-lane
    0x3020	,	0x93	,			
    0x3021	,	0x03	,			
    0x3022	,	0x01	,			
    0x3031	,	0x0a	,			
    0x303f	,	0x0c	,			
    0x3305	,	0xf1	,			
    0x3307	,	0x04	,			
    0x3309	,	0x29	,			
    0x3500	,	0x00	,			
    0x3501	,	0x60	,			
    0x3502	,	0x00	,			
    0x3503	,	0x04	,			
    0x3504	,	0x00	,			
    0x3505	,	0x00	,			
    0x3506	,	0x00	,			
    0x3507	,	0x00	,			
    0x3508	,	0x00	,			
    0x3509	,	0x80	,			
    0x350a	,	0x00	,			
    0x350b	,	0x00	,			
    0x350c	,	0x00	,			
    0x350d	,	0x00	,			
    0x350e	,	0x00	,			
    0x350f	,	0x80	,			
    0x3510	,	0x00	,			
    0x3511	,	0x00	,			
    0x3512	,	0x00	,			
    0x3513	,	0x00	,			
    0x3514	,	0x00	,			
    0x3515	,	0x80	,			
    0x3516	,	0x00	,			
    0x3517	,	0x00	,			
    0x3518	,	0x00	,			
    0x3519	,	0x00	,			
    0x351a	,	0x00	,			
    0x351b	,	0x80	,			
    0x351c	,	0x00	,			
    0x351d	,	0x00	,			
    0x351e	,	0x00	,			
    0x351f	,	0x00	,			
    0x3520	,	0x00	,			
    0x3521	,	0x80	,			
    0x3522	,	0x08	,			
    0x3524	,	0x08	,			
    0x3526	,	0x08	,			
    0x3528	,	0x08	,			
    0x352a	,	0x08	,			
    0x3602	,	0x00	,			
    0x3603	,	0x40	,			
    0x3604	,	0x02	,			
    0x3605	,	0x00	,			
    0x3606	,	0x00	,			
    0x3607	,	0x00	,			
    0x3609	,	0x12	,			
    0x360a	,	0x40	,			
    0x360c	,	0x08	,			
    0x360f	,	0xe5	,			
    0x3608	,	0x8f	,			
    0x3611	,	0x00	,			
    0x3613	,	0xf7	,			
    0x3616	,	0x58	,			
    0x3619	,	0x99	,			
    0x361b	,	0x60	,			
    0x361c	,	0x7a	,			
    0x361e	,	0x79	,			
    0x361f	,	0x02	,			
    0x3632	,	0x00	,			
    0x3633	,	0x10	,			
    0x3634	,	0x10	,			
    0x3635	,	0x10	,			
    0x3636	,	0x15	,			
    0x3646	,	0x86	,			
    0x364a	,	0x0b	,			
    0x3700	,	0x17	,			
    0x3701	,	0x22	,			
    0x3703	,	0x10	,			
    0x370a	,	0x37	,			
    0x3705	,	0x00	,			
    0x3706	,	0x63	,			
    0x3709	,	0x3c	,			
    0x370b	,	0x01	,			
    0x370c	,	0x30	,			
    0x3710	,	0x24	,			
    0x3711	,	0x0c	,			
    0x3716	,	0x00	,			
    0x3720	,	0x28	,			
    0x3729	,	0x7b	,			
    0x372a	,	0x84	,			
    0x372b	,	0xbd	,			
    0x372c	,	0xbc	,			
    0x372e	,	0x52	,			
    0x373c	,	0x0e	,			
    0x373e	,	0x33	,			
    0x3743	,	0x10	,			
    0x3744	,	0x88	,			
    0x3745	,	0xc0	,			
    0x374a	,	0x43	,			
    0x374c	,	0x00	,			
    0x374e	,	0x23	,			
    0x3751	,	0x7b	,			
    0x3752	,	0x84	,			
    0x3753	,	0xbd	,			
    0x3754	,	0xbc	,			
    0x3756	,	0x52	,			
    0x375c	,	0x00	,			
    0x3760	,	0x00	,			
    0x3761	,	0x00	,			
    0x3762	,	0x00	,			
    0x3763	,	0x00	,			
    0x3764	,	0x00	,			
    0x3767	,	0x04	,			
    0x3768	,	0x04	,			
    0x3769	,	0x08	,			
    0x376a	,	0x08	,			
    0x376b	,	0x20	,			
    0x376c	,	0x00	,			
    0x376d	,	0x00	,			
    0x376e	,	0x00	,			
    0x3773	,	0x00	,			
    0x3774	,	0x51	,			
    0x3776	,	0xbd	,			
    0x3777	,	0xbd	,			
    0x3781	,	0x18	,			
    0x3783	,	0x25	,			
    0x3798	,	0x1b	,			
    0x3800	,	0x00	,			
    0x3801	,	0x08	,			
    0x3802	,	0x00	,			
    0x3803	,	0x04	,			
    0x3804	,	0x0a	,			
    0x3805	,	0x97	,			
    0x3806	,	0x05	,			
    0x3807	,	0xfb	,			
    0x3808	,	0x0a	,   // H_OUTPUT_SIZE 0xa80 = 2688
    0x3809	,	0x80	,			
    0x380a	,	0x05	,	// V_OUTPUT_SIZE 0x5f0 = 1520
    0x380b	,	0xf0	,			
    0x380c	,	0x0a	,	//	for	30fps   TIMING_HTS 0xa0a = 2570
    0x380d	,	0x0a	,	//	for	30fps
    0x380e	,	0x06	,	//	for	30fps   TIMING_VTS 0x614 = 1556
    0x380f	,	0x14	,	//	for	30fps
    0x3810	,	0x00	,			
    0x3811	,	0x08	,			
    0x3812	,	0x00	,			
    0x3813	,	0x04	,			
    0x3814	,	0x01	,			
    0x3815	,	0x01	,			
    0x3819	,	0x01	,			
    0x3820	,	0x00	,			
    0x3821	,	0x06	,			
    0x3829	,	0x00	,			
    0x382a	,	0x01	,			
    0x382b	,	0x01	,			
    0x382d	,	0x7f	,			
    0x3830	,	0x04	,			
    0x3836	,	0x01	,			
    0x3837	,	0x00	,			
    0x3841	,	0x02	,			
    0x3846	,	0x08	,			
    0x3847	,	0x07	,			
    0x3d85	,	0x36	,			
    0x3d8c	,	0x71	,			
    0x3d8d	,	0xcb	,			
    0x3f0a	,	0x00	,			
    0x4000	,	0xf1	,			
    0x4001	,	0x40	,			
    0x4002	,	0x04	,			
    0x4003	,	0x14	,			
    0x400e	,	0x00	,			
    0x4011	,	0x00	,			
    0x401a	,	0x00	,			
    0x401b	,	0x00	,			
    0x401c	,	0x00	,			
    0x401d	,	0x00	,			
    0x401f	,	0x00	,			
    0x4020	,	0x00	,			
    0x4021	,	0x10	,			
    0x4022	,	0x07	,			
    0x4023	,	0xcf	,			
    0x4024	,	0x09	,			
    0x4025	,	0x60	,			
    0x4026	,	0x09	,			
    0x4027	,	0x6f	,			
    0x4028	,	0x00	,			
    0x4029	,	0x02	,			
    0x402a	,	0x06	,			
    0x402b	,	0x04	,			
    0x402c	,	0x02	,			
    0x402d	,	0x02	,			
    0x402e	,	0x0e	,			
    0x402f	,	0x04	,			
    0x4302	,	0xff	,			
    0x4303	,	0xff	,			
    0x4304	,	0x00	,			
    0x4305	,	0x00	,			
    0x4306	,	0x00	,			
    0x4308	,	0x02	,			
    0x4500	,	0x6c	,			
    0x4501	,	0xc4	,			
    0x4502	,	0x40	,			
    0x4503	,	0x01	,			
    0x4601	,	0xA7	,			
    0x4800	,	0x04	,			
    0x4813	,	0x08	,			
    0x481f	,	0x40	,			
    0x4829	,	0x78	,			
    0x4837	,	0x16	,	//	for	Global
    0x4b00	,	0x2a	,			
    0x4b0d	,	0x00	,			
    0x4d00	,	0x04	,			
    0x4d01	,	0x42	,			
    0x4d02	,	0xd1	,			
    0x4d03	,	0x93	,			
    0x4d04	,	0xf5	,			
    0x4d05	,	0xc1	,			
    0x5000	,	0xf3	,			
    0x5001	,	0x11	,			
    0x5004	,	0x00	,			
    0x500a	,	0x00	,			
    0x500b	,	0x00	,			
    0x5032	,	0x00	,			
    0x5040	,	0x00	,			
    0x5050	,	0x0c	,			
    0x5500	,	0x00	,			
    0x5501	,	0x10	,			
    0x5502	,	0x01	,			
    0x5503	,	0x0f	,			
    0x8000	,	0x00	,			
    0x8001	,	0x00	,			
    0x8002	,	0x00	,			
    0x8003	,	0x00	,			
    0x8004	,	0x00	,			
    0x8005	,	0x00	,			
    0x8006	,	0x00	,			
    0x8007	,	0x00	,			
    0x8008	,	0x00	,			
    0x3638	,	0x00	,			
    0x0100	,	0x01	,			   
};


void SNR_OV4689_InitSensor_Customer(void) {
	// implement your initialize sensor routine here ==============================================
	ISP_UINT16 data ;
	ISP_UINT32 i;
	AITPS_GBL  pGBL = AITC_BASE_GBL;

	// tx ms 10~ 100ms , before turn on clock and wait power to stable
    RTNA_WAIT_MS(10);
	// initialize VIF OPRs
	//ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_PARALLEL, 22);
	{
		ISP_HDM_VIF_IF vif_if;
		//AITPS_GBL  pGBL = AITC_BASE_GBL;
		AITPS_VIF  pVIF = AITC_BASE_VIF;
		AITPS_MIPI pMIPI = AITC_BASE_MIPI;
		MMP_ULONG ulSENSORCLK;
		MMP_UBYTE   vid = VIF_CTL;
		
#if (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)	
	vif_if = ISP_HDM_VIF_IF_MIPI_SINGLE_0;
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)		
	vif_if = ISP_HDM_VIF_IF_MIPI_DUAL_01;
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)		
	vif_if = ISP_HDM_VIF_IF_MIPI_QUAD;
#endif

		pVIF->VIF_INT_HOST_EN[vid] = 0;
		pVIF->VIF_INT_CPU_SR[vid] = VIF_INT_ALL;
		pVIF->VIF_OUT_EN[vid] = 0;
		//pVIF->VIF_DEFT_EN = 0;
		pVIF->VIF_RAW_OUT_EN[vid] = 0;
		pVIF->VIF_FRME_SKIP_NO[vid] = 0;
		pVIF->VIF_FRME_SKIP_EN[vid] = 0;
		#if (CHIP == P_V2)
		pVIF->VIF_IGBT_EN[vid] = 1;
		#endif

		// Add VIF1 PAD setting
      	if (vid == VIF_CTL_VIF1) {	
      		pGBL->GBL_IO_CTL0 |= GBL_VIF1_PAD0;	
      		pVIF->VIF_RAW_OUT_EN[vid] |= (VIF_1_TO_ISP);
      	}

		//RTNA_DBG_Str(0,"ISP_HDM_IF_InitVIF\r\n");
		MMPF_PLL_GetGroupFreq(4, &ulSENSORCLK);
        dbg_printf(0, "ulSENSORCLK = %d\r\n", ulSENSORCLK);

		switch(ulSENSORCLK){
			case 96000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D4; // 24MHz
				RTNA_DBG_Str3("96MHz DIV 4\r\n");
				break;
			case 144000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D6; 
				RTNA_DBG_Str3("Sensor DIV 6 for 144MHz\r\n");
				break;
			case 132000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D11;  // 132/11 = 12M
				break;
			case 166000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D7;
				break;
			case 192000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D8;
				//RTNA_DBG_Str3("192MHz DIV 8\r\n");
				break;
			case 216000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D9;
				RTNA_DBG_Str3("216MHz DIV 9\r\n");
				break;
			case 240000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D10;
				RTNA_DBG_Str3("240MHz DIV 10\r\n");
				break;
			case 120000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D5;
				RTNA_DBG_Str3("120MHz DIV 5\r\n")
				break;
			case 264000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D11;
				RTNA_DBG_Str3("264MHz DIV 11\r\n")
				break;
			default:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D12;  // 144/12 = 12M
				break;
		}

		pVIF->VIF_SENSR_CTL[vid] = VIF_SENSR_LATCH_PST | VIF_PIXL_ID_POLAR | VIF_VSYNC_POLAR_NEG ;//| VIF_12BPP_OUT_EN;

		pVIF->VIF_YUV_CTL[vid] = VIF_YUV_FORMAT_VUY;
        MMPF_VIF_SetPIODir(VIF_SIF_SEN, MMP_TRUE);

	    //MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);
	    RTNA_WAIT_MS(5);
	    MMPF_VIF_SetPIOOutput(VIF_SIF_SEN, MMP_TRUE);
        /*
        if (vid == 0) {
    		pVIF->VIF_0_SENSR_SIF_EN = VIF_SIF_SEN | VIF_SIF_RST;
    		pVIF->VIF_0_SENSR_SIF_DATA = VIF_SIF_RST;
		}
		else {
    		pVIF->VIF_1_SENSR_SIF_EN = VIF_SIF_SEN | VIF_SIF_RST;
    		pVIF->VIF_1_SENSR_SIF_DATA = VIF_SIF_RST;
		}
		*/
		
	   MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);
	   MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);
       RTNA_WAIT_MS(5);
	   MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);

        //dbg_printf(3,"MCLK on\r\n");
		pVIF->VIF_SENSR_CLK_CTL[vid] = VIF_SENSR_CLK_EN;
		pVIF->VIF_OPR_UPD[vid] = VIF_OPR_UPD_EN | VIF_OPR_UPD_FRAME;

		if (vif_if==ISP_HDM_VIF_IF_PARALLEL) {
			RTNA_DBG_Str0("ISP_HDM_VIF_IF_PARALLEL\r\n");
		}
		else {
		    pMIPI->MIPI_DATA0_CFG[vid] = ((pMIPI->MIPI_DATA0_CFG[vid] & ~MIPI_DAT_SRC_SEL_MASK) | MIPI_DAT_SRC_PHY_1) | MIPI_DAT_LANE_EN;
		    if (vif_if == ISP_HDM_VIF_IF_MIPI_DUAL_01) {
                pMIPI->MIPI_DATA1_CFG[vid] = ((pMIPI->MIPI_DATA1_CFG[vid] & ~MIPI_DAT_SRC_SEL_MASK) | MIPI_DAT_SRC_PHY_2) | MIPI_DAT_LANE_EN;
			}
			#if 0//(CHIP == P_V2)
			else if(vif_if==ISP_HDM_VIF_IF_MIPI_QUAD){
			    pMIPI->MIPI_DATA2_CFG = MIPI_CSI2_EN | MIPI_SRC_SEL_2;
			    pMIPI->MIPI_DATA3_CFG = MIPI_CSI2_EN | MIPI_SRC_SEL_3;
			    pMIPI->MIPI_DATA4_CFG = MIPI_CSI2_EN | MIPI_SRC_SEL_4;
			}
			#endif
		}

        // MIPI Data Delay
		pMIPI->MIPI_DATA0_CFG[vid] |= MIPI_DAT_DLY_EN;
		pMIPI->MIPI_DATA1_CFG[vid] |= MIPI_DAT_DLY_EN;
		pMIPI->MIPI_DATA2_CFG[vid] |= MIPI_DAT_DLY_EN;
		pMIPI->MIPI_DATA3_CFG[vid] |= MIPI_DAT_DLY_EN;
		pMIPI->MIPI_DATA0_DLY[vid] =( 0x10 << 8) | 0 ;
		pMIPI->MIPI_DATA1_DLY[vid] =( 0x10 << 8) | 0 ;
		pMIPI->MIPI_DATA2_DLY[vid] =( 0x10 << 8) | 0 ;
		pMIPI->MIPI_DATA3_DLY[vid] =( 0x10 << 8) | 0 ;

	}

#if (CHIP==P_V2)
	ISP_HDM_IF_EnableSensor(ISP_HDM_SNR_ENABLE_HIGH);
#endif

	
	//RTNA_WAIT_MS(20/*10*/);
	// t5 : 150000 Ext Clk (24MHz) , around 6.5 ms 
	RTNA_WAIT_MS(10);

#if 0
	// set register..........
	for (i = 0; i < VR_ARRSIZE(SNR_OV4689_Reg_Init_Customer)/2; i++) {
		if(SNR_OV4689_Reg_Init_Customer[i*2] == 0xFFFF)
			delay_1ms(SNR_OV4689_Reg_Init_Customer[i*2+1]);
		else
			gsSensorFunction->MMPF_Sensor_SetReg(SNR_OV4689_Reg_Init_Customer[i*2], SNR_OV4689_Reg_Init_Customer[i*2+1]);
            if(MMPF_Sensor_GetErr()) {
                return ;
            }

        /*
		data = gsSensorFunction->MMPF_Sensor_GetReg(SNR_OV4689_Reg_Init_Customer[i*2]) ;
		RTNA_DBG_Str3("Reg :");
		RTNA_DBG_Short3(SNR_OV4689_Reg_Init_Customer[i*2]);
		RTNA_DBG_Str3(",");
		RTNA_DBG_Short3(data);
		RTNA_DBG_Str3("\r\n");
        */
	}
	dbg_printf(3, "MMPF_I2CM_ID_1: (x%x, x%x)\r\n", pGBL->GBL_IO_CTL0, pGBL->GBL_IO_CTL1);
	//dbg_printf(3, "VIF_1: (x%x, x%x, x%x)\r\n", pVIF->VIF_IN_EN[VIF_CTL], pVIF->VIF_OUT_EN[VIF_CTL], pVIF->VIF_RAW_OUT_EN[VIF_CTL]);    

	// Set preview resolution as default
	// SNR_OV4689_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);
#endif

}

void MMPF_Sensor_SetSensorGain_OV4689(MMP_ULONG gain)
{
    ISP_UINT16 sensor_H;
    ISP_UINT16 sensor_L;
    MMP_ULONG gainbase = ISP_IF_AE_GetGainBase();    
    gain = VR_MIN(VR_MAX(gain, gainbase), gainbase*16-1);
    
    /* Sensor Gain Mapping */
    if ((gain>>8) < 2)
    {
        sensor_H = 0;
        sensor_L = (gain * 128) >> 8;                 // 1X ~ 2X
    }
    else if ((gain >> 8) < 4)
    {
        sensor_H = 1;   
        sensor_L = ((gain * 64) - (256 * 8)) >> 8;    // 2X ~ 4X
    }       
    else if ((gain >> 8) < 8)
    {
        sensor_H = 3;   
        sensor_L = ((gain * 32) - (256 * 12)) >> 8;   // 4X ~ 8X
    }   
    else
    {
        sensor_H = 7;
        sensor_L = ((gain * 16) - (256 * 8)) >> 8;    // 8X ~16X
    }
    
    gsSensorFunction->MMPF_Sensor_SetReg(0x3508, sensor_H);
    gsSensorFunction->MMPF_Sensor_SetReg(0x3509, sensor_L);
}

void MMPF_Sensor_SetSensorShutter_OV4689(MMP_ULONG shutter, MMP_ULONG vsync)
{
    ISP_UINT32 new_vsync   = g_SNR_ov4689_LineCntPerSec * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
    ISP_UINT32 new_shutter = g_SNR_ov4689_LineCntPerSec * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();
    
    new_vsync   = ISP_MIN(ISP_MAX((new_shutter + 4), new_vsync), 0xFFFF);
    new_shutter = ISP_MIN(ISP_MAX(new_shutter, 1), (new_vsync - 4));
    
    // dbg_printf(0,"#>Set.Sensor(Shutter,Vsync):(%d,%d)\r\n",new_shutter,new_vsync);
    
    // gsSensorFunction->MMPF_Sensor_SetReg(0x380E, (new_vsync >> 8));
    // gsSensorFunction->MMPF_Sensor_SetReg(0x380F, new_vsync);
    
    gsSensorFunction->MMPF_Sensor_SetReg(0x3500, (ISP_UINT8)((new_shutter >> 12) & 0xFF));
    gsSensorFunction->MMPF_Sensor_SetReg(0x3501, (ISP_UINT8)((new_shutter >> 4) & 0xFF));
    gsSensorFunction->MMPF_Sensor_SetReg(0x3502, (ISP_UINT8)((new_shutter << 4) & 0xFF));
}

void SNR_OV4689_SetSensorAddMode_Customer(ISP_UINT8 enable)
{
/*	if (ISP_HDM_IF_IsFrameTypePreview()) {
		ISP_UINT8 regval = (enable) ? gsSensorFunction->MMPF_Sensor_GetReg(0x3016) | 0x40 : gsSensorFunction->MMPF_Sensor_GetReg(0x3016) & ~0x40;

		gsSensorFunction->MMPF_Sensor_SetReg(0x0104, 0x01); // grouped_parameter_hold
		gsSensorFunction->MMPF_Sensor_SetReg(0x3016, regval);
		gsSensorFunction->MMPF_Sensor_SetReg(0x0104, 0x00); // grouped_parameter_hold
	}
*/
}

MMP_ULONG VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;

void SNR_OV4689_SetSensorResolution_Customer(ISP_SENSOR_RESOL res)
{
	// implement your change sensor resolution routine here =======================================

	MMP_ULONG i ;//, VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;
    PCAM_USB_VIDEO_RES ep_res ;

    if(gsCurSensorEnumRes==res) {
       // dbg_printf(3,"#Sensor Same Res: %d\r\n",res);
        return ;
    }
    gsCurSensorEnumRes = res ;
    //dbg_printf(3,"#Sensor Res Change : %d\r\n",res);
    ep_res = GetFirstStreamingRes();

    dbg_printf(0,"@ res: x%x\r\n",res);
    
	switch (res) {
	case ISP_SENSOR_RESOL_1920x1080:
	case ISP_SENSOR_RESOL_1280x960:
	case ISP_SENSOR_RESOL_1280x720:

		// set register...........
        for (i = 0; i < VR_ARRSIZE(SNR_OV4689_Reg_Init_Customer)/2; i++) {
        	gsSensorFunction->MMPF_Sensor_SetReg(SNR_OV4689_Reg_Init_Customer[i*2], SNR_OV4689_Reg_Init_Customer[i*2+1]);
        }

		// set target fps and corresponding sensor vsync
       ISP_IF_AE_SetMaxSensorFPSx10(30 * 10);
       g_SNR_ov4689_LineCntPerSec = (1544 )* 30; ///test20120821 +2 for DMA


        ISP_IF_AE_SetShutterBase(g_SNR_ov4689_LineCntPerSec);

    	// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1; //shift right 4 pixel, sensor start from 2
		VIFGrab_H_Length    = 2688 - 4;
		VIFGrab_V_Start     = 1;
		VIFGrab_V_Length    = 1520 - 4;
        
		ISP_IF_AE_SetMinAccLength(VIFGrab_H_Length, VIFGrab_V_Length);        
		ISP_IF_IQ_SetISPInputLength(VIFGrab_H_Length, VIFGrab_V_Length);
		ISP_IF_IQ_SetScalerOutputLength(VIFGrab_H_Length, VIFGrab_V_Length, 0 ,0);	//20121130, pan/tile/zoom	
		//sISP_IF_IQ_SetISPInputLength(640, 480);
		
		// set scaler downsample rate (for scaler calculation)
		//ISP_IF_SNR_SetDownSample(1, 1);

		// set color id
		ISP_IF_IQ_SetColorID(2);
		
		break;

	}

		// calcuate shading ratio
		{
			ISP_UINT32 base_x, base_y;
			ISP_UINT32 x_scale_n = 0x200;
			ISP_UINT32 y_scale_n = 0x200;
			ISP_IF_IQ_GetCaliBase(&base_x, &base_y);
			
			x_scale_n = VIFGrab_H_Length * 0x200 / base_x;
			y_scale_n = VIFGrab_V_Length * 0x200 / base_y;
			
			ISP_IF_IQ_SetCaliRatio(	x_scale_n,	//x_scale_n, 
									0x200,		//x_scale_m, 
									0,			//x_offset, 
									y_scale_n,	//y_scale_n, 
									0x200,		//y_scale_m, 
									0);			//y_offset
		}
	// end of your implementation =================================================================

	// set VIF OPR (Actual sensor output size)
	{
		AITPS_VIF  pVIF = AITC_BASE_VIF;
		volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;
		MMP_UBYTE   vid = VIF_CTL;
        #if (CHIP==P_V2)
		pVIF->VIF_GRAB_PIXL_ST = VIFGrab_H_Start;
		pVIF->VIF_GRAB_PIXL_ED = VIFGrab_H_Start + VIFGrab_H_Length -1;
		pVIF->VIF_GRAB_LINE_ST = VIFGrab_V_Start ;
		pVIF->VIF_GRAB_LINE_ED = VIFGrab_V_Start + VIFGrab_V_Length -1;
		pVIF->VIF_INT_LINE = VIFGrab_V_Length - 61;// 61;//121;
		#endif
		#if (CHIP==VSN_V2)|| (CHIP==VSN_V3)
		pVIF->VIF_GRAB[vid].PIXL_ST = VIFGrab_H_Start;
		pVIF->VIF_GRAB[vid].PIXL_ED = VIFGrab_H_Start + VIFGrab_H_Length -1;
		pVIF->VIF_GRAB[vid].LINE_ST = VIFGrab_V_Start ;
		pVIF->VIF_GRAB[vid].LINE_ED = VIFGrab_V_Start + VIFGrab_V_Length -1;
		pVIF->VIF_INT_LINE_NUM_0[vid] = VIFGrab_V_Length - 61;//121;

		#endif
		dbg_printf(0,"ViF(xs,xe,ys,ye)=(%d,%d,%d,%d)\r\n",pVIF->VIF_GRAB[vid].PIXL_ST,pVIF->VIF_GRAB[vid].PIXL_ED,pVIF->VIF_GRAB[vid].LINE_ST,pVIF->VIF_GRAB[vid].LINE_ED);
	}
	
	
	//================
	// set iq (NR, Edge, CCM, Gamma, etc.) and functions (saturation, contrast, sharpness, hue, etc.)
	ISP_IF_IQ_SetAll();

	// SetBeforePreview and SetBeforeCapture should be called after getting new configurations of the current resolution
	if (gSystemMode  == VENUS_SYSTEM_MODE_SNAPSHOT) {
		// update AE before capture
		ISP_IF_AE_UpdateBeforeCapture();

		// update AWB before capture
		ISP_IF_AWB_UpdateBeforeCapture();

		// special settings for capture could be set here =============================================

		// end of special settings ====================================================================
	} else if (gSystemMode == VENUS_SYSTEM_MODE_PREVIEW) {
		// update AE before preview
		ISP_IF_AE_UpdateBeforePreview();

		// update AWB before preview
		ISP_IF_AWB_UpdateBeforePreview();
	}

	// set direction (color ID, orientation, etc.)
	ISP_IF_IQ_SetDirection(ISP_IQ_DIRECTION_ORIGINAL);

	// set exposure parameters

	{	
		//ISP_IF_IQ_SetAEGain(ISP_IF_AE_GetDGain(),ISP_IF_AE_GetGainBase());
		gsSensorFunction->MMPF_Sensor_SetSensorGain(ISP_IF_AE_GetGain());
		gsSensorFunction->MMPF_Sensor_SetSensorShutter(0, 0);
	}
	// update ISP window
	ISP_IF_IQ_UpdateInputSize();

	ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AE, 1);
    ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AWB, 1);
    ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AF, 1);	
	// check if modules are bypassed
	ISP_IF_IQ_CheckBypass();

	// force update iq to hardware
	ISP_IF_IQ_UpdateOprtoHW(ISP_IQ_SWITCH_ALL, 1);
	
}

#if SUPPORT_AUTO_FOCUS
void MOTOR_OV4689_VCM_InitMotor_Customer(void)
{

    //DW9714, 10 bit data
#if 0 //(CUSTOMER == RAZ)
    if ( AFInfiAdjustValue==0xFF || AFNearAdjustValue==0xFF )
    {
	ISP_IF_AF_SetSearchRange(0x18 * 4, 0x78 * 4, 0x18 * 4, 0x78 * 4, 10);
	ISP_IF_AF_SetPos(0x18, 10);
    }
    else
    {
	ISP_IF_AF_SetSearchRange(AFInfiAdjustValue * 4, AFNearAdjustValue * 4, AFInfiAdjustValue * 4, AFNearAdjustValue * 4, 10);
	ISP_IF_AF_SetPos(AFInfiAdjustValue, 10);   
    }
#else
    ISP_IF_AF_SetSearchRange(0x08 * 4, 0x78 * 4, 0x08 * 4, 0x78 * 4, 10);
    ISP_IF_AF_SetPos(0x08, 10);
#endif

    dbg_printf(0, "###MOTOR_OV4689_VCM_InitMotor_Customer###\r\n");

}

void MOTOR_OV4689_VCM_SetAFPosition_Customer(MMP_USHORT af_pos)
{
    //AF_10BIT_INTERFACE
    // =================================
    // SDA = --------+--------+--------
    //       ---11--W|S-987654|3210----
    //          ID      addr     data
    //
    // addr = (gsAFPos >> 4) & 0x3f
    // data = (gsAFPos << 4) & 0xf0
    // =================================
    //dbg_printf(0, "x%x\r\n",af_pos);

    MMPF_MOTOR_WriteI2C((af_pos >> 4) & 0x3f, (af_pos << 4) & 0xf0);

    //dbg_printf(3,"###MOTOR_OV4689_VCM_SetAFPosition_Customer(%d)###\r\n", af_pos);
}

void MOTOR_OV4689_VCM_SetAFHomePosition_Customer(void)
{
	MMP_USHORT af_pos;
	
	af_pos = ISP_IF_AF_GetPos(0);
    //AF_10BIT_INTERFACE
    // =================================
    // SDA = --------+--------+--------
    //       ---11--W|S-987654|3210----
    //          ID      addr     data
    //
    // addr = (gsAFPos >> 4) & 0x3f
    // data = (gsAFPos << 4) & 0xf0
    // =================================
    
    while (af_pos >= 20)
    {
    	af_pos -= 20;
	    MMPF_MOTOR_WriteI2C((af_pos >> 4) & 0x3f, (af_pos << 4) & 0xf0);
	    MMPF_OS_Sleep(10);
	}

    //dbg_printf(3,"###MOTOR_OV4689_VCM_SetAFHomePosition_Customer(%d)###\r\n", af_pos);

}

#endif


MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_OV4689;
#if (TOTAL_SENSOR_NUMBER >= 2)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_OV4689;
#endif

#endif  //BIND_SENSOR_OV4689
#endif
