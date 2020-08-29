//==============================================================================
//
//  File        : sensor_MT9T002.c
//  Description : Firmware Sensor Control File
//  Author      : Philip Lin
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if defined(DSC_MP3_P_FW)||defined(VIDEO_AMR_R_FW)||defined(VIDEO_AAC_R_FW)||(SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_MT9T002

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
SENSOR_I2CM_ID, 0x10, 16, 16, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE,MMP_FALSE, 0, 0, 1, MMPF_I2CM_SPEED_HW_400K, NULL, NULL
#endif
#if VIF_CTL==VIF_CTL_VIF1
SENSOR_I2CM_ID, 0x10, 16, 16, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE,MMP_FALSE, 0, 0, 0/*1*/, MMPF_I2CM_SPEED_HW_400K, NULL, NULL
#endif
};

// sean@2011_02_09   
#define REV_0           (0) // Sequencer rev 0 : for original setting.
#define REV_1           (1) // Sequencer rev 1 : for setting from fremont.
#define REV_2           (2) // Sequencer rev 2 : for MP version.
#define SENSOR_REV      REV_1

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
//extern MMP_ULONG glPccamResolution ;
extern MMP_UBYTE   gbSensorPowerOffMode ; // default : power off mode
 
const ISP_UINT8 Sensor_IQ_CompressedText[] = {
    #if (CHIP == P_V2)
    #include "isp_842_iq_data_v1_MT9T002_LGT.xls.qlz.txt"
    #endif
    #if (CHIP == VSN_V2)
    #include "isp_8451_iq_data_v1_MT9T002.xls.ciq.txt"
    #endif
    #if (CHIP == VSN_V3) // IQ table : V3 TBD
    #if ISP_GNR_OFF==0
    #include "isp_8453_iq_data_v2_MT9T002.xls.ciq.txt" 
    #else
    #include "gnr-off\\isp_8453_iq_data_v2_MT9T002.xls.ciq.txt"
    #endif
    #endif
} ;

#define	SENSOR_ROTATE_180		0   
#define BASE_SHUTTER_50HZ		391
#define BASE_SHUTTER_60HZ		326
MMP_ULONG g_SNR_3135_LineCntPerSec = 1;

void SNR_MT9T002_InitSensor_Customer(void);
void SNR_MT9T002_SetSensorResolution_Customer(ISP_SENSOR_RESOL res);
void MMPF_Sensor_SetSensorGain_MT9T002(MMP_ULONG gain);
void MMPF_Sensor_SetSensorShutter_MT9T002(MMP_ULONG shutter, MMP_ULONG vsync);
void MOTOR_MT9T002_VCM_SetAFPosition_Customer(MMP_USHORT af_pos);

void InitializeMotor(void);
void SetMotorPos(ISP_UINT16 af_pos);
void StopMotor(ISP_UINT16 af_pos);

#define  SENSOR_WIDTH_16_9_BINNING  SENSOR_16_9_W_60FPS //1136//1152 
#define  SENSOR_HEIGHT_16_9_BINNING SENSOR_16_9_H_60FPS  //639//648 

#define  SENSOR_WIDTH_16_9  2304 
#if SENSOR_IN_H > SENSOR_16_9_H
#define  SENSOR_HEIGHT_16_9 (SENSOR_IN_H  ) 
#else
#define  SENSOR_HEIGHT_16_9 (1296  ) 
#endif
//#define  SENSOR_WIDTH_4_3   2048
//#define  SENSOR_HEIGHT_4_3  1536 // 36

#define  SENSOR_WIDTH_BAYER   (2304)//
#define  SENSOR_HEIGHT_BAYER  (1536)  //

MMP_USHORT gsSensorWidthByFOV ;
MMP_USHORT gsSensorHeightByFOV ;
 
//sean@2011_01_24, reduce set sensor resolution if
// resolution is not changed 
ISP_SENSOR_RESOL gsCurSensorEnumRes = ISP_SENSOR_RESOL_NULL ;
//extern PCAM_USB_VIDEO_FORMAT gsVidFmt ;
//extern PCAM_USB_VIDEO_RES gsVidRes ;
//#define SUPPORT_AUTO_FOCUS 0

MMP_USHORT  gsVideoFormat;

//#include "sensor_ctl.h"
//#include "preview_ctl.h"
//#include "XDATAmap.h"
//#include "3A_ctl.h"
//#include "MT9T002.h"
//#include "motor_ctl.h"
//extern MMP_UBYTE gbCurFrameRate[] ;
extern MMP_USHORT gsCurFrameRatex10[];
extern MMP_UBYTE   gbDramID;
static void MT9T002_Sensor_SetColorRangeByStream(void);

static void MT9T002_Sensor_SetColorRangeByStream(void)
{
#if (CHIP==P_V2)

#error Only for 8423


    MMP_UBYTE i ;
    STREAM_SESSION *ss ;
   
    STREAM_CFG *cur_pipe;// = usb_get_cur_image_pipe(0);
    MMP_UBYTE pipe0_cfg , pipe1_cfg , cr_id = 0  ;
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
            if( (cur_pipe->pipe_en&PIPE_EN_MASK)!=(PIPE0_EN|PIPE1_EN) ) {
            // Single stream case
                if( (pipe0_cfg == PIPE_CFG_YUY2) || (pipe1_cfg == PIPE_CFG_YUY2) ) {
                    cr_id = 1 ;
                    dbg_printf(3,"YUY2->color range:");
                }
                else if( (pipe0_cfg == PIPE_CFG_MJPEG) || (pipe1_cfg == PIPE_CFG_MJPEG) ) {
                    cr_id = 0 ;
                    dbg_printf(3,"MJPEG->color range:");
                }
                else if( (pipe0_cfg == PIPE_CFG_H264) || (pipe1_cfg == PIPE_CFG_H264) ) {
                    cr_id = 1 ;
                    dbg_printf(3,"H264->color range:");
                }
            }  else {
            // Dual Stream case
                // YUY2 + H264
                if( (pipe0_cfg == PIPE_CFG_YUY2) && ( pipe1_cfg == PIPE_CFG_H264 ) ) {
                    cr_id = 1 ;
                    dbg_printf(3,"YUY2+H264->color range:");
                }
                // MJPEG + H264
                else if ((pipe0_cfg == PIPE_CFG_MJPEG) && ( pipe1_cfg == PIPE_CFG_H264 )) {
                    cr_id = 0 ;
                    dbg_printf(3,"MJPEG+H264->color range:");
                }
                // YUY2 + MJPEG
                else if ((pipe0_cfg == PIPE_CFG_YUY2) && ( pipe1_cfg == PIPE_CFG_MJPEG )) {
                    cr_id = 0 ;
                    dbg_printf(3,"YUY2+MJPEG->color range:");
                }
            }   
            dbg_printf(3,"%d\r\n",cr_id);
            //MMPF_Sensor_SetColorTransform(cr_id);
        }
    }
#endif    
}

extern MMPF_SENSOR_FUNCTION *gsSensorFunction;
//------------------------------------------------------------------------------
//  Function    : MT9T002_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
static void MT9T002_Sensor_PreviewMode(MMP_USHORT usPreviewmode)
{
	gsCurPreviewMode = usPreviewmode;
	//gsCurPhaseCount = usPhasecount;

	switch (usPreviewmode) {
	case 0:
		dbg_printf(0,"->1152x648\r\n");
        //ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518
        gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
        MT9T002_Sensor_SetColorRangeByStream();
        gSystemMode = VENUS_SYSTEM_MODE_PREVIEW;//ISP_IF_SNR_SetBeforePreview();
		ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
        dbg_printf(0,"gsCurFrameRatex10 : %d\r\n",gsCurFrameRatex10[0]);
        SNR_MT9T002_SetSensorResolution_Customer(ISP_SENSOR_RESOL_800x600);
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
            MT9T002_Sensor_SetColorRangeByStream();
			gSystemMode = VENUS_SYSTEM_MODE_PREVIEW;//ISP_IF_SNR_SetBeforePreview();
			ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
            dbg_printf(0,"gsCurFrameRatex10 : %d\r\n",gsCurFrameRatex10[0]);
            SNR_MT9T002_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);
			//ISP_IF_3A_Control(ISP_3A_ENABLE);
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE);
			break;
 			//set color transform 16 ~ 235
            //MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH_1, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
			
		#endif
		#if defined(DSC_MP3_P_FW)||(DSC_R_EN)
		case SENSOR_DSC_MODE:
			RTNA_DBG_Str(3, "Dsc preview mode\r\n");
			ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518

			MMPF_Sensor_SetColorTransform(0);//julian@091111
			ISP_IF_SNR_SetBeforePreview();
			ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1280x960);     //Ted 720p 60fps
			//ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1920x1080);
			ISP_IF_3A_Control(ISP_3A_ENABLE);
			break;
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
            MT9T002_Sensor_SetColorRangeByStream();
			gSystemMode = VENUS_SYSTEM_MODE_PREVIEW;//ISP_IF_SNR_SetBeforePreview();
			ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
            dbg_printf(0,"gsCurFrameRatex10 : %d\r\n",gsCurFrameRatex10[0]);
            SNR_MT9T002_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);
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

static MMP_ERR  MMPF_Sensor_Initialize_MT9T002(void)
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
	MMPF_PIO_EnableGpioMode(MMPF_PIO_REG_GPIO45, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(MMPF_PIO_REG_GPIO45, MMP_TRUE);
	MMPF_PIO_SetData(MMPF_PIO_REG_GPIO45, MMP_TRUE);	
	RTNA_WAIT_MS(1);
    #endif    
	
	gsCurSensorEnumRes = ISP_SENSOR_RESOL_NULL ;
	//
	// For 4:3 , 16:9 sensor 
	// Only 2048x1536 use 4:3 FOV
	//
    res = GetFirstStreamingRes();
    	
	if(res ==PCCAM_2304_1536) {
        if(m_VideoFmt==MMPS_3GPRECD_VIDEO_FORMAT_RAW) { // sean@2011_01_24 always go into 
	        gsSensorWidthByFOV = SENSOR_WIDTH_BAYER ;
	        gsSensorHeightByFOV = SENSOR_HEIGHT_BAYER ; 
        } else {
            //sean@2011_01_24, change to 2304x1536
            gsSensorWidthByFOV  = SENSOR_WIDTH_BAYER  ;
            gsSensorHeightByFOV = SENSOR_HEIGHT_BAYER ;  
            
	        //gsSensorWidthByFOV = SENSOR_WIDTH_4_3 ;
	        //gsSensorHeightByFOV = SENSOR_HEIGHT_4_3; 
	    }
	} else {
	        gsSensorWidthByFOV = SENSOR_WIDTH_16_9 ;
	        gsSensorHeightByFOV = SENSOR_HEIGHT_16_9;
	}
	
	
#if 1
	
	//Init globol variable used by DSC
	gsSensorLCModeWidth     = gsSensorWidthByFOV;
	gsSensorLCModeHeight    = gsSensorHeightByFOV;//800;
    max_ep_fps = gsCurFrameRatex10[0] ;
    if(max_ep_fps > 300 ) {
    	gsSensorMCModeWidth     = SENSOR_WIDTH_16_9_BINNING;
    	gsSensorMCModeHeight    = SENSOR_HEIGHT_16_9_BINNING;//800;
    
    } else {
    	gsSensorMCModeWidth     = gsSensorWidthByFOV;
    	gsSensorMCModeHeight    = gsSensorHeightByFOV;//800;
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
    

#endif
    dbg_printf(0,"SNR_Init\r\n");

	SNR_MT9T002_InitSensor_Customer();

    cur_res = GetResCfg(res);
    dbg_printf(0,"gsVideoFormat: %d\r\n",gsVideoFormat);
    dbg_printf(0,"cur_res->res_w: %d\r\n",cur_res->res_w);
    dbg_printf(0,"cur_res->res_h: %d\r\n",cur_res->res_h);

    // TODO: Implement the specific resolution here!
    gsSensorFunction->MMPF_Sensor_SetPreviewMode(1);


#if SUPPORT_AUTO_FOCUS && (AF_EN!=255)
	//RTNA_DBG_Str(0, "Config PGPIO19 pin as pull high IO Pad\r\n");
	// config PGPIO19 as pull high
	pPAD->PAD_GPIO[AF_EN] = AF_EN_PAD_CFG ;
	pGPIO->GPIO_EN[0] |= (1 << AF_EN); // config as output mode
//    pGPIO->GPIO_DATA[0] &= ~(1 << AF_EN);  // Turn On Motor
	pGPIO->GPIO_DATA[0] |= (1 << AF_EN);  // Turn On Motor
	RTNA_WAIT_MS(20);
#endif

	// patrick: init motor setting
#if SUPPORT_AUTO_FOCUS	
	pGBL->GBL_CLK_DIS1 &= ~GBL_CLK_PWM_DIS;	// Enable PWM clock
	InitializeMotor();
#endif
	RTNA_DBG_Str(3, "->Sensor.Inited\r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_MT9T002
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_PowerDown_MT9T002(void)
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


#if  SUPPORT_AUTO_FOCUS 
    StopMotor(0);
    //MMPF_OS_Sleep_MS(3); // TBD
    RTNA_WAIT_MS(3);
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
	MMPF_PIO_EnableGpioMode(MMPF_PIO_REG_GPIO45, MMP_TRUE);
	MMPF_PIO_EnableOutputMode(MMPF_PIO_REG_GPIO45, MMP_TRUE);
	MMPF_PIO_SetData(MMPF_PIO_REG_GPIO45, MMP_FALSE);	
    #endif    	
    //	RTNA_WAIT_MS(20);
    }
        
	RTNA_WAIT_MS(1);
	
	pVIF->VIF_SENSR_CLK_CTL[snr_id] &= (~ VIF_SENSR_CLK_EN) ;   //output sensor main clock
	
    if(gbSensorPowerOffMode==0) {
    	// Pull RST / SEN / SCK / SDA to low
        MMPF_Sensor_InitPad();
	    //MMPF_OS_Sleep_MS(20);
	    RTNA_WAIT_MS(20);
	}
	RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_MT9T002() \r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_MT9T002
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_MT9T002(MMP_UBYTE ubFrameRate)
{
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_SetPreviewMode_MT9T002(MMP_USHORT usPreviewmode)
{
	MT9T002_Sensor_PreviewMode(usPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangePreviewMode_MT9T002(
						MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
						MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
	MT9T002_Sensor_PreviewMode(usNewPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------

/*static*/ MMP_ERR MMPF_Sensor_ChangeMode_MT9T002(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
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
			MMPF_Sensor_SetPreviewMode_MT9T002(0);
			// wait 3 frame ...
		   // MMPF_Sensor_WaitFrame(5);
			break;
		case 1:
			MMPF_Sensor_SetPreviewMode_MT9T002(1);
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
static MMP_ERR  MMPF_Sensor_InitializeVIF_MT9T002(void)
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
static MMP_ERR  MMPF_Sensor_InitializeISP_MT9T002(void)
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
	// set AF type (ISP_AF_TYPE_ONE_SHOT / ISP_AF_TYPE_CONTINUOUS)
	ISP_IF_AF_SetType(ISP_AF_TYPE_CONTINUOUS);
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
//  Function    : MMPF_Sensor_SetAFPosition_MT9T002
//  Description :
// ---------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_MT9T002(MMP_UBYTE ubPos)
{
#if SUPPORT_AUTO_FOCUS
	//if (gISPConfig.AutoFocusMode == VENUS_AF_MODE_MANUAL) {
		//gAFConfig.AFPos = ubPos;
		
		//RTNA_DBG_Str0("AF pos = "); RTNA_DBG_Long0(ubPos);RTNA_DBG_Str0("\r\n");
		
		ISP_IF_AF_SetPos(ubPos, 0);
		SetMotorPos(ISP_IF_AF_GetPos(0));
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
static MMP_ERR  MMPF_Sensor_DoAWBOperation_MT9T002(void)
{

	static ISP_UINT32 increase_excute = 12;

	//Renoir speed up 20130306
	if(increase_excute > 0)
	{
		if(increase_excute == 12) {ISP_IF_AWB_SetFastMode(1);}
		increase_excute--;
		ISP_IF_AWB_Execute();
		if(increase_excute == 1) {ISP_IF_AWB_SetFastMode(0);}
	}	
    //Renoir 20130306 end

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
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_ST
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_ST_MT9T002(void)
{	

	ISP_UINT32 FrameC = 0;
	ISP_INT16 T_ccm[9], INccm[9], NEW_ccm[9] ;
	ISP_INT16 i;
	static ISP_INT16 PreColorID, ColorID ;
//							    0x705A	0x705B	0x705C	0x705D	0x705E	0x705F	0x7060	0x7061	0x7062
	static ISP_INT16 pre_ait_ccm[9] = {	420,	-96,	-68,
										-90,	380,	-34,
										-56,	-100,	412
															};
				ISP_INT16 CCMa[9]	= {	388,	-78,	-54,
										-84,	380,	-28,
										-18,	-138,	412
															};
				ISP_INT16 CCMb[9] 	= {	420,	-96,	-68,
										-90,	380,	-34,
										-56,	-100,	412
															};
				ISP_INT16 CCMc[9] 	= { 300,	-42,	-2,
										-90,	380,	-34,
										-90,	-66,	412
															};
								
	ColorID = ISP_IF_IQ_GetID(ISP_IQ_CHECK_CLASS_COLORTEMP);

    if(ISP_IF_3A_GetSwitch(ISP_3A_ALGO_AE) != 1) return MMP_ERR_NONE;


	switch (gISPFrameCnt % DO_AE_GAP){
	case 0:
		ISP_IF_AE_Execute();
		gsSensorFunction->MMPF_Sensor_SetSensorShutter(0, 0);
		gsSensorFunction->MMPF_Sensor_SetSensorGain(ISP_IF_AE_GetGain());
		break;
	case 1:		
	#if WDR_SIM_EN
        gsSensorFunction->MMPF_Sensor_SetSensorShutter(LONG_SHUTTER_RATIO, 0);
    #endif    
		break;
	}

	switch (gISPFrameCnt % 30){

	case 2:
	
		{
			ISP_INT16* tar_ccm = (ColorID > 8) ? CCMc : (ColorID > 4) ? CCMa : CCMb;
			ISP_INT16 step = 2;
			
			for (i = 0; i < 9; i++) {

				if (pre_ait_ccm[i] > tar_ccm[i]) {
					pre_ait_ccm[i] = (pre_ait_ccm[i] - tar_ccm[i] > step) ? pre_ait_ccm[i] - step : tar_ccm[i];
				} else if (pre_ait_ccm[i] < tar_ccm[i]) {
					pre_ait_ccm[i] = (tar_ccm[i] - pre_ait_ccm[i] > step) ? pre_ait_ccm[i] + step : tar_ccm[i];
				}

			}
				
			ISP_IF_IQ_SetCCM(pre_ait_ccm);
		}
	}
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_END
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_END_MT9T002(void)
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
static MMP_ERR  MMPF_Sensor_DoAFOperation_MT9T002(void)
{
/*
#if SUPPORT_AUTO_FOCUS
	if(gbAutoFocus == MMP_TRUE){
		//VR_AF_GetAcc();
		ISP_IF_AF_GetHWAcc(1);
		//ISP_IF_R_DoAF();
	}
#endif
*/
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_FrameStart_MT9T002(void)
{
#if SUPPORT_AUTO_FOCUS //&& (AF_EN!=255)
	if(gbAutoFocus == MMP_TRUE){
		//VR_AF_GetAcc();
		//ISP_IF_R_GetAFAcc();
		//ISP_IF_R_DoAF();
		{
			static ISP_UINT32 frame_cnt = 0;

			switch (frame_cnt++ % 1) {
			case 0:
				ISP_IF_AF_Execute();
				SetMotorPos(ISP_IF_AF_GetPos(0));
				break;
			}
		}
	}
#endif

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoIQOperation_MT9T002
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoIQOperation_MT9T002(void)
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
	
	

	//RTNA_DBG_Str(0, "DoIQOperation_MT9T002()\r\n");

	return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_MT9T002(MMP_USHORT usMode)
{
/*	RTNA_DBG_Str(3, "MMPF_SetLightFreq_MT9T002 : ");
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

static MMP_ERR  MMPF_SetStableState_MT9T002(MMP_BOOL bStableState)
{
	return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_MT9T002(MMP_BOOL bHistEQState)
{

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_MT9T002(MMP_USHORT usAddr, MMP_USHORT usData)
{
	MMPF_I2cm_WriteReg(&m_snr_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_MT9T002(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	*usData = 0xFF;
    MMPF_I2cm_ReadReg(&m_snr_I2cmAttribute, usAddr, usData);
	
	return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_MT9T002(MMP_USHORT imgSceneID)
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

	RTNA_DBG_Str(3, "MMPF_SetImageScene_MT9T002 : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_MT9T002(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAEMode_MT9T002 : ");
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

static MMP_ERR  MMPF_Sensor_SetAFMode_MT9T002(MMP_UBYTE ubAFMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_MT9T002 : ");
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
static MMP_ERR  MMPF_Sensor_SetImageEffect_MT9T002(MMP_USHORT imgEffectID)
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

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_MT9T002 : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_MT9T002(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_MT9T002 : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

	cAWBtype = ubType;
#endif
	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_MT9T002(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_MT9T002 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_MT9T002(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_MT9T002 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_MT9T002(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_MT9T002 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);
	ISP_IF_F_SetCISharpness(ubLevel);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_MT9T002(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_MT9T002 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_MT9T002(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_MT9T002 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetBacklight_MT9T002(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_MT9T002 : ");
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



static void  MMPF_Sensor_SetCaptureISPSetting_MT9T002(MMP_UBYTE usCaptureBegin)
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
static void  MMPF_Sensor_SetISPWindow_MT9T002(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
	//gSensorFunc.SetISPWindows(ulWidth,ulHeight);
	return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_MT9T002(MMP_USHORT usPreviewmode)
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

static void SetAFWin_MT9T002(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
					MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

	DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_MT9T002(MMP_UBYTE enable)
{
#if SUPPORT_AUTO_FOCUS //&& (AF_EN!=255)
//    if (enable) {
		//RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
		ISP_IF_AF_Control(ISP_AF_START);
		//actually calling SNR_MT9T002_SetAutoFocusControl()
//    } else {
//        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
//        ISP_IF_AF_Control(ISP_AF_STOP);
//    }
#endif
}

static MMP_ERR MMPF_Sensor_SetAEEnable_MT9T002(MMP_UBYTE bEnable)
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

static MMP_ERR MMPF_Sensor_SetAWBEnable_MT9T002(MMP_UBYTE bEnable)
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
static void SetAEsmooth_MT9T002(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_MT9T002(MMP_UBYTE ev)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_MT9T002 : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	RTNA_DBG_Byte(3, ev);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_AE_SetEV(ev);

	return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_MT9T002     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_MT9T002(void)
{
	//return 0;
	return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_MT9T002  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_MT9T002       (void){return 0;}
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
static MMP_ERR	MMPF_Sensor_Set3AStatus_MT9T002(MMP_BOOL bEnable)
{
	MMPF_Sensor_3A_SetInterrupt(bEnable);

	if (bEnable)
		ISP_IF_3A_Control(ISP_3A_ENABLE);
	else
		ISP_IF_3A_Control(ISP_3A_DISABLE);

	return	MMP_ERR_NONE;
}

static void  MMPF_Sensor_SetColorID_MT9T002(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}

MMPF_SENSOR_FUNCTION  SensorFunction_MT9T002 =
{
	MMPF_Sensor_Initialize_MT9T002,
	MMPF_Sensor_InitializeVIF_MT9T002,
	MMPF_Sensor_InitializeISP_MT9T002,
	MMPF_Sensor_PowerDown_MT9T002,
	MMPF_Sensor_SetFrameRate_MT9T002,
	MMPF_Sensor_ChangeMode_MT9T002,
	MMPF_Sensor_ChangePreviewMode_MT9T002,
	MMPF_Sensor_SetPreviewMode_MT9T002,
	MMPF_Sensor_SetReg_MT9T002,
	MMPF_Sensor_GetReg_MT9T002,
	MMPF_Sensor_DoAWBOperation_MT9T002,
	MMPF_Sensor_DoAEOperation_ST_MT9T002,
	MMPF_Sensor_DoAEOperation_END_MT9T002,
	MMPF_Sensor_DoAFOperation_MT9T002,
	MMPF_Sensor_DoAFOperation_FrameStart_MT9T002,
	MMPF_Sensor_DoIQOperation_MT9T002,
	MMPF_Sensor_SetImageEffect_MT9T002,
	MMPF_SetLightFreq_MT9T002,
	MMPF_SetStableState_MT9T002,
	MMPF_SetHistEQState_MT9T002,
	MMPF_Sensor_SetAFPosition_MT9T002,
	SetAFWin_MT9T002,
	SetAFEnable_MT9T002,
	MMPF_Sensor_SetAEEnable_MT9T002,
	MMPF_Sensor_SetAWBEnable_MT9T002,
	MMPF_Sensor_SetExposureValue_MT9T002,
	SetAEsmooth_MT9T002,
	MMPF_Sensor_SetImageScene_MT9T002,
	GetAFPosition_MT9T002,
	GetAFEnable_MT9T002,
	GetExposureValue_MT9T002,
	GetAEsmooth_MT9T002,
	MMPF_Sensor_SetAWBType_MT9T002,
	MMPF_Sensor_SetContrast_MT9T002,
	MMPF_Sensor_SetSaturation_MT9T002,
	MMPF_Sensor_SetSharpness_MT9T002,
	MMPF_Sensor_SetHue_MT9T002,
	MMPF_Sensor_SetGamma_MT9T002,
 //   MMPF_Sensor_SetBacklight_MT9T002,
	MMPF_Sensor_SetAEMode_MT9T002,
	MMPF_Sensor_SetAFMode_MT9T002,
	MMPF_Sensor_SetCaptureISPSetting_MT9T002,
	MMPF_Sensor_SetISPWindow_MT9T002,
	MMPF_Sensor_CheckPreviewAbility_MT9T002,
	MMPF_Sensor_Set3AStatus_MT9T002,
	MMPF_Sensor_SetColorID_MT9T002,
	MMPF_Sensor_SetSensorGain_MT9T002,
	MMPF_Sensor_SetSensorShutter_MT9T002
};


#if (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)
//MIPI 1 Lane
ISP_UINT16 SNR_MT9T002_Reg_Init_Customer[] = {
	0x31AE, 0x201,
	0x301A,0x0058, //disable streaming
	0xFFFF, 10, //delay 5us
	0x31AE, 0x201, //Output 1-lane MIPI
	//Configure for Serial Interface
	0x301A,0x0058, //Drive Pins,Parallel Enable,SMIA Serializer Disable
	0x3064, 0x1802, 	//Disable Embedded Data
	//Optimized Gain Configuration
	0x3EE0, 0x1500, 	// DAC_LD_20_21
	0x3EEA,0x001D,
	0x31E0,0x1E01,
	0x3F06,0x046A,
	0x3ED2,0x0186,	// upadate 9/30
	0x3ED4,0x8F2C,	// upadate 9/30
	0x3ED6,0x2244,	// upadate 9/30
	0x3ED8,0x6442,	// upadate 9/30
	0x30BA, 0x002C, //Dither enable
	0x3046, 0x4038,		// Enable Flash Pin
	0x3048, 0x8480,		// Flash Pulse Length
	 0x3ED0, 0x0016, 	// DAC_LD_4_5
	 0x3ED0, 0x0036, 	// DAC_LD_4_5
	 0x3ED0, 0x0076, 	// DAC_LD_4_5
	 0x3ED0, 0x00F6, 	// DAC_LD_4_5
	 0x3ECE, 0x1003, 	// DAC_LD_2_3
	 0x3ECE, 0x100F, 	// DAC_LD_2_3
	 0x3ECE, 0x103F, 	// DAC_LD_2_3
	 0x3ECE, 0x10FF, 	// DAC_LD_2_3
	 0x3ED0, 0x00F6, 	// DAC_LD_4_5
	 0x3ED0, 0x04F6, 	// DAC_LD_4_5
	 0x3ED0, 0x24F6, 	// DAC_LD_4_5
	 0x3ED0, 0xE4F6, 	// DAC_LD_4_5
	 0x3EE6, 0xA480, 	// DAC_LD_26_27
	 0x3EE6, 0xA080, 	// DAC_LD_26_27
	 0x3EE6, 0x8080, 	// DAC_LD_26_27
	 0x3EE6, 0x0080, 	// DAC_LD_26_27
	0x3EE6, 0x0080, 	// DAC_LD_26_27
	0x3EE8, 0x2024, 	// DAC_LD_28_29
	0x30FE, 128, //Noise Pedestal of 128

	//Assuming Input Clock of 24MHz.  Output Clock will be 70Mpixel/s
	 0x302A, 0x0005, 	// VT_PIX_CLK_DIV
	 0x302C, 0x0004, 	// VT_SYS_CLK_DIV
	 0x302E, 0x0002, 	// PRE_PLL_CLK_DIV
	 0x3030, 0x0040, 	// PLL_MULTIPLIER
	 0x3036, 0x000A, 	// OP_PIX_CLK_DIV
	 0x3038, 0x0001, 	// OP_SYS_CLK_DIV
	 0x31AC, 0x0A0A, 	// DATA_FORMAT_BITS
	//MIPI TIMING
	0x31B0, 40,                          // FRAME PREAMBLE
	0x31B2, 14,                          // LINE PREAMBLE
	0x31B4, 0x2743,                      // MIPI TIMING 0
	0x31B6, 0x114E,                      // MIPI TIMING 1
	0x31B8, 0x2049,                      // MIPI TIMING 2
	0x31BA, 0x0186,                      // MIPI TIMING 3
	0x31BC, 0x8005,                      // MIPI TIMING 4
	0x31BE, 0x2003,                      // MIPI CONFIG STATUS
	//0x31B0, 41,
	//Sequencer
// sean@2011_02_09 patch for different sequencer setting.	
#if SENSOR_REV==REV_0	
	0x3088, 0x8000,
	0x3086, 0x4540,
	0x3086, 0x6134,
	0x3086, 0x4A31,
	0x3086, 0x4342,
	0x3086, 0x4560,
	0x3086, 0x2714,
	0x3086, 0x3DFF,
	0x3086, 0x3DFF,
	0x3086, 0x3DEA,
	0x3086, 0x2704,
	0x3086, 0x3D10,
	0x3086, 0x2705,
	0x3086, 0x3D10,
	0x3086, 0x2715,
	0x3086, 0x3527,
	0x3086, 0x053D,
	0x3086, 0x1045,
	0x3086, 0x4027,
	0x3086, 0x0427,
	0x3086, 0x143D,
	0x3086, 0xFF3D,
	0x3086, 0xFF3D,
	0x3086, 0xEA62,
	0x3086, 0x2728,
	0x3086, 0x3627,
	0x3086, 0x083D,
	0x3086, 0x6444,
	0x3086, 0x2C2C,
	0x3086, 0x2C2C,
	0x3086, 0x4B01,
	0x3086, 0x432D,
	0x3086, 0x4643,
	0x3086, 0x1647,
	0x3086, 0x435F,
	0x3086, 0x4F50,
	0x3086, 0x2604,
	0x3086, 0x2684,
	0x3086, 0x2027,
	0x3086, 0xFC53,
	0x3086, 0x0D5C,
	0x3086, 0x0D60,
	0x3086, 0x5754,
	0x3086, 0x1709,
	0x3086, 0x5556,
	0x3086, 0x4917,
	0x3086, 0x145C,
	0x3086, 0x0945,
	0x3086, 0x0017,
	0x3086, 0x0545,
	0x3086, 0x8026,
	0x3086, 0xA627,
	0x3086, 0xF817,
	0x3086, 0x0227,
	0x3086, 0xFA5C,
	0x3086, 0x0B5F,
	0x3086, 0x5307,
	0x3086, 0x5302,
	0x3086, 0x4D28,
	0x3086, 0x6C4C,
	0x3086, 0x0928,
	0x3086, 0x2C28,
	0x3086, 0x294E,
	0x3086, 0x1718,
	0x3086, 0x26A2,
	0x3086, 0x5C03,
	0x3086, 0x1744,
	0x3086, 0x2809,
	0x3086, 0x27F2,
	0x3086, 0x1708,
	0x3086, 0x2809,
	0x3086, 0x1628,
	0x3086, 0x084D,
	0x3086, 0x1A26,
	0x3086, 0x8327,
	0x3086, 0xFA45,
	0x3086, 0xA017,
	0x3086, 0x0727,
	0x3086, 0xFB17,
	0x3086, 0x2945,
	0x3086, 0x8017,
	0x3086, 0x0827,
	0x3086, 0xFA17,
	0x3086, 0x285D,
	0x3086, 0x5300,
	0x3086, 0x17E8,
	0x3086, 0x5302,
	0x3086, 0x1710,
	0x3086, 0x2682,
	0x3086, 0x170E,
	0x3086, 0x484D,
	0x3086, 0x4E28,
	0x3086, 0x094C,
	0x3086, 0x0B17,
	0x3086, 0x5F28,
	0x3086, 0x0927,
	0x3086, 0xF217,
	0x3086, 0x0A28,
	0x3086, 0x0928,
	0x3086, 0x084D,
	0x3086, 0x1A27,
	0x3086, 0xFA26,
	0x3086, 0x025C,
	0x3086, 0x0045,
	0x3086, 0x4027,
	0x3086, 0x9817,
	0x3086, 0x2A4A,
	0x3086, 0x0A43,
	0x3086, 0x160B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x0827,
	0x3086, 0x985D,
	0x3086, 0x5307,
	0x3086, 0x2644,
	0x3086, 0x5C01,
	0x3086, 0x4B12,
	0x3086, 0x4452,
	0x3086, 0x5117,
	0x3086, 0x0260,
	0x3086, 0x184A,
	0x3086, 0x0343,
	0x3086, 0x1604,
	0x3086, 0x4316,
	0x3086, 0x5843,
	0x3086, 0x1659,
	0x3086, 0x4316,
	0x3086, 0x5A43,
	0x3086, 0x165B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x1027,
	0x3086, 0x9817,
	0x3086, 0x2022,
	0x3086, 0x4B12,
	0x3086, 0x442C,
	0x3086, 0x2C2C,
	0x3086, 0x2C00,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
#endif
#if SENSOR_REV==REV_1 // new setting from fremont.
    0x3088, 0x8000,
    0x3086, 0x4540,
    0x3086, 0x6134,
    0x3086, 0x4A31,
    0x3086, 0x4342,
    0x3086, 0x4560,
    0x3086, 0x2714,
    0x3086, 0x3DFF,
    0x3086, 0x3DFF,
    0x3086, 0x3DEA,
    0x3086, 0x2704,
    0x3086, 0x3D10,
    0x3086, 0x2705,
    0x3086, 0x3D10,
    0x3086, 0x2715,
    0x3086, 0x3527,
    0x3086, 0x053D,
    0x3086, 0x1045,
    0x3086, 0x4027,
    0x3086, 0x0427,
    0x3086, 0x143D,
    0x3086, 0xFF3D,
    0x3086, 0xFF3D,
    0x3086, 0xEA62,
    0x3086, 0x2728,
    0x3086, 0x3627,
    0x3086, 0x083D,
    0x3086, 0x6444,
    0x3086, 0x2C2C,
    0x3086, 0x2C2C,
    0x3086, 0x4B01,
    0x3086, 0x432D,
    0x3086, 0x4643,
    0x3086, 0x1647,
    0x3086, 0x435F,
    0x3086, 0x4F50,
    0x3086, 0x2604,
    0x3086, 0x2684,
    0x3086, 0x2027,
    0x3086, 0xFC53,
    0x3086, 0x0D5C,
    0x3086, 0x0D60,
    0x3086, 0x5754,
    0x3086, 0x1709,
    0x3086, 0x5556,
    0x3086, 0x4917,
    0x3086, 0x145C,
    0x3086, 0x0945,
    0x3086, 0x0045,
    0x3086, 0x8026,
    0x3086, 0xA627,
    0x3086, 0xF817,
    0x3086, 0x0227,
    0x3086, 0xFA5C,
    0x3086, 0x0B5F,
    0x3086, 0x5307,
    0x3086, 0x5302,
    0x3086, 0x4D28,
    0x3086, 0x644C,
    0x3086, 0x0928,
    0x3086, 0x2428,
    0x3086, 0x234E,
    0x3086, 0x1718,
    0x3086, 0x26A2,
    0x3086, 0x5C03,
    0x3086, 0x1744,
    0x3086, 0x27F2,
    0x3086, 0x1708,
    0x3086, 0x2803,
    0x3086, 0x2808,
    0x3086, 0x4D1A,
    0x3086, 0x27FA,
    0x3086, 0x2683,
    0x3086, 0x45A0,
    0x3086, 0x1707,
    0x3086, 0x27FB,
    0x3086, 0x1729,
    0x3086, 0x4580,
    0x3086, 0x1708,
    0x3086, 0x27FA,
    0x3086, 0x1728,
    0x3086, 0x2682,
    0x3086, 0x5D17,
    0x3086, 0x0E48 ,
    0x3086, 0x4D4E,
    0x3086, 0x2803,
    0x3086, 0x4C0B,
    0x3086, 0x175F,
    0x3086, 0x27F2,
    0x3086, 0x170A,
    0x3086, 0x2808,
    0x3086, 0x4D1A,
    0x3086, 0x27FA,
    0x3086, 0x2602,
    0x3086, 0x5C00,
    0x3086, 0x4540,
    0x3086, 0x2798,
    0x3086, 0x172A,
    0x3086, 0x4A0A,
    0x3086, 0x4316,
    0x3086, 0x0B43,
    0x3086, 0x279C,
    0x3086, 0x4560,
    0x3086, 0x1707,
    0x3086, 0x279D,
    0x3086, 0x1725,
    0x3086, 0x4540,
    0x3086, 0x1708,
    0x3086, 0x2798,
    0x3086, 0x5D53,
    0x3086, 0x0026,
    0x3086, 0x445C,
    0x3086, 0x014b,
    0x3086, 0x1244,
    0x3086, 0x5251,
    0x3086, 0x1702,
    0x3086, 0x6018,
    0x3086, 0x4A03,
    0x3086, 0x4316,
    0x3086, 0x0443,
    0x3086, 0x1658,
    0x3086, 0x4316,
    0x3086, 0x5943,
    0x3086, 0x165A,
    0x3086, 0x4316,
    0x3086, 0x5B43,
    0x3086, 0x4540,
    0x3086, 0x279C,
    0x3086, 0x4560,
    0x3086, 0x1707,
    0x3086, 0x279D,
    0x3086, 0x1725,
    0x3086, 0x4540,
    0x3086, 0x1710,
    0x3086, 0x2798,
    0x3086, 0x1720,
    0x3086, 0x224B,
    0x3086, 0x1244,
    0x3086, 0x2C2C,
    0x3086, 0x2C2C
#endif
#if SENSOR_REV==REV_2 // MP version dosen't need ~
#endif
	
/*
	//ARRAY READOUT SETTINGS
	0x3004, 0,			// X_ADDR_START
	0x3008, 2303,		// X_ADDR_END
	0x3002, 124,    		// Y_ADDR_START
	0x3006, 1419,		// Y_ADDR_END

	//Sub-sampling
	0x30A2,1,			// X_ODD_INCREMENT
	0x30A6,1,			// Y_ODD_INCREMENT
	0x3040, 0,	//no bin

	//Frame-Timing
	0x300C, 1260,		// LINE_LENGTH_PCK
	0x300A, 1308,//1270,		// FRAME_LENGTH_LINES
	0x3014, 0,		// FINE_INTEGRATION_TIME
	0x3012, 1000,		// Coarse_Integration_Time
	0x3042, 0,			// EXTRA_DELAY
	0x30BA, 0x2C,
	0x301A,0x005C,			// Enable Streaming
*/
};

#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)
//MIPI 2 Lane
MMP_USHORT SNR_MT9T002_Reg_Init_Customer[] = {
	0x31AE, 0x202,
	0x301A,0x0058, //disable streaming
	0xFFFF, 10, //delay 5us
	0x31AE, 0x202, //Output 2-lane MIPI
	//Configure for Serial Interface
	0x301A,0x0058, //Drive Pins,Parallel Enable,SMIA Serializer Disable
	0x3064, 0x1802, 	//Disable Embedded Data
	//Optimized Gain Configuration
	0x3EE0, 0x1500, 	// DAC_LD_20_21
	0x3EEA,0x001D,
	0x31E0,0x1E01,
	0x3F06,0x046A,
	0x3ED2,0x0186,	// upadate 9/30
	0x3ED4,0x8F2C,	// upadate 9/30
	0x3ED6,0x2244,	// upadate 9/30
	0x3ED8,0x6442,	// upadate 9/30
	0x30BA, 0x002C, //Dither enable
	0x3046, 0x4038,		// Enable Flash Pin
	0x3048, 0x8480,		// Flash Pulse Length
	 0x3ED0, 0x0016, 	// DAC_LD_4_5
	 0x3ED0, 0x0036, 	// DAC_LD_4_5
	 0x3ED0, 0x0076, 	// DAC_LD_4_5
	 0x3ED0, 0x00F6, 	// DAC_LD_4_5
	 0x3ECE, 0x1003, 	// DAC_LD_2_3
	 0x3ECE, 0x100F, 	// DAC_LD_2_3
	 0x3ECE, 0x103F, 	// DAC_LD_2_3
	 0x3ECE, 0x10FF, 	// DAC_LD_2_3
	 0x3ED0, 0x00F6, 	// DAC_LD_4_5
	 0x3ED0, 0x04F6, 	// DAC_LD_4_5
	 0x3ED0, 0x24F6, 	// DAC_LD_4_5
	 0x3ED0, 0xE4F6, 	// DAC_LD_4_5
	 0x3EE6, 0xA480, 	// DAC_LD_26_27
	 0x3EE6, 0xA080, 	// DAC_LD_26_27
	 0x3EE6, 0x8080, 	// DAC_LD_26_27
	 0x3EE6, 0x0080, 	// DAC_LD_26_27
	0x3EE6, 0x0080, 	// DAC_LD_26_27
	0x3EE8, 0x2024, 	// DAC_LD_28_29
	0x30FE, 128, //Noise Pedestal of 128

	//Assuming Input Clock of 24MHz.  Output Clock will be 70Mpixel/s
	 0x302A, 0x0005, 	// VT_PIX_CLK_DIV
	 0x302C, 0x0002, 	// VT_SYS_CLK_DIV
	 0x302E, 0x0004, 	// PRE_PLL_CLK_DIV
	 0x3030, 82, 	// PLL_MULTIPLIER
	 0x3036, 0x000A, 	// OP_PIX_CLK_DIV
	 0x3038, 0x0001, 	// OP_SYS_CLK_DIV
	 0x31AC, 0x0A0A, 	// DATA_FORMAT_BITS
	//MIPI TIMING
	0x31B0, 40,                          // FRAME PREAMBLE
	0x31B2, 14,                          // LINE PREAMBLE
	0x31B4, 0x2743,                      // MIPI TIMING 0
	0x31B6, 0x114E,                      // MIPI TIMING 1
	0x31B8, 0x2049,                      // MIPI TIMING 2
	0x31BA, 0x0186,                      // MIPI TIMING 3
	0x31BC, 0x8005,                      // MIPI TIMING 4
	0x31BE, 0x2003,                      // MIPI CONFIG STATUS
	//0x31B0, 41,
	//Sequencer
#if SENSOR_REV==REV_0	
	0x3088, 0x8000,
	0x3086, 0x4540,
	0x3086, 0x6134,
	0x3086, 0x4A31,
	0x3086, 0x4342,
	0x3086, 0x4560,
	0x3086, 0x2714,
	0x3086, 0x3DFF,
	0x3086, 0x3DFF,
	0x3086, 0x3DEA,
	0x3086, 0x2704,
	0x3086, 0x3D10,
	0x3086, 0x2705,
	0x3086, 0x3D10,
	0x3086, 0x2715,
	0x3086, 0x3527,
	0x3086, 0x053D,
	0x3086, 0x1045,
	0x3086, 0x4027,
	0x3086, 0x0427,
	0x3086, 0x143D,
	0x3086, 0xFF3D,
	0x3086, 0xFF3D,
	0x3086, 0xEA62,
	0x3086, 0x2728,
	0x3086, 0x3627,
	0x3086, 0x083D,
	0x3086, 0x6444,
	0x3086, 0x2C2C,
	0x3086, 0x2C2C,
	0x3086, 0x4B01,
	0x3086, 0x432D,
	0x3086, 0x4643,
	0x3086, 0x1647,
	0x3086, 0x435F,
	0x3086, 0x4F50,
	0x3086, 0x2604,
	0x3086, 0x2684,
	0x3086, 0x2027,
	0x3086, 0xFC53,
	0x3086, 0x0D5C,
	0x3086, 0x0D60,
	0x3086, 0x5754,
	0x3086, 0x1709,
	0x3086, 0x5556,
	0x3086, 0x4917,
	0x3086, 0x145C,
	0x3086, 0x0945,
	0x3086, 0x0017,
	0x3086, 0x0545,
	0x3086, 0x8026,
	0x3086, 0xA627,
	0x3086, 0xF817,
	0x3086, 0x0227,
	0x3086, 0xFA5C,
	0x3086, 0x0B5F,
	0x3086, 0x5307,
	0x3086, 0x5302,
	0x3086, 0x4D28,
	0x3086, 0x6C4C,
	0x3086, 0x0928,
	0x3086, 0x2C28,
	0x3086, 0x294E,
	0x3086, 0x1718,
	0x3086, 0x26A2,
	0x3086, 0x5C03,
	0x3086, 0x1744,
	0x3086, 0x2809,
	0x3086, 0x27F2,
	0x3086, 0x1708,
	0x3086, 0x2809,
	0x3086, 0x1628,
	0x3086, 0x084D,
	0x3086, 0x1A26,
	0x3086, 0x8327,
	0x3086, 0xFA45,
	0x3086, 0xA017,
	0x3086, 0x0727,
	0x3086, 0xFB17,
	0x3086, 0x2945,
	0x3086, 0x8017,
	0x3086, 0x0827,
	0x3086, 0xFA17,
	0x3086, 0x285D,
	0x3086, 0x5300,
	0x3086, 0x17E8,
	0x3086, 0x5302,
	0x3086, 0x1710,
	0x3086, 0x2682,
	0x3086, 0x170E,
	0x3086, 0x484D,
	0x3086, 0x4E28,
	0x3086, 0x094C,
	0x3086, 0x0B17,
	0x3086, 0x5F28,
	0x3086, 0x0927,
	0x3086, 0xF217,
	0x3086, 0x0A28,
	0x3086, 0x0928,
	0x3086, 0x084D,
	0x3086, 0x1A27,
	0x3086, 0xFA26,
	0x3086, 0x025C,
	0x3086, 0x0045,
	0x3086, 0x4027,
	0x3086, 0x9817,
	0x3086, 0x2A4A,
	0x3086, 0x0A43,
	0x3086, 0x160B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x0827,
	0x3086, 0x985D,
	0x3086, 0x5307,
	0x3086, 0x2644,
	0x3086, 0x5C01,
	0x3086, 0x4B12,
	0x3086, 0x4452,
	0x3086, 0x5117,
	0x3086, 0x0260,
	0x3086, 0x184A,
	0x3086, 0x0343,
	0x3086, 0x1604,
	0x3086, 0x4316,
	0x3086, 0x5843,
	0x3086, 0x1659,
	0x3086, 0x4316,
	0x3086, 0x5A43,
	0x3086, 0x165B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x1027,
	0x3086, 0x9817,
	0x3086, 0x2022,
	0x3086, 0x4B12,
	0x3086, 0x442C,
	0x3086, 0x2C2C,
	0x3086, 0x2C00,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
#endif
#if SENSOR_REV==REV_1 // new setting from fremont.
    0x3088, 0x8000,
    0x3086, 0x4540,
    0x3086, 0x6134,
    0x3086, 0x4A31,
    0x3086, 0x4342,
    0x3086, 0x4560,
    0x3086, 0x2714,
    0x3086, 0x3DFF,
    0x3086, 0x3DFF,
    0x3086, 0x3DEA,
    0x3086, 0x2704,
    0x3086, 0x3D10,
    0x3086, 0x2705,
    0x3086, 0x3D10,
    0x3086, 0x2715,
    0x3086, 0x3527,
    0x3086, 0x053D,
    0x3086, 0x1045,
    0x3086, 0x4027,
    0x3086, 0x0427,
    0x3086, 0x143D,
    0x3086, 0xFF3D,
    0x3086, 0xFF3D,
    0x3086, 0xEA62,
    0x3086, 0x2728,
    0x3086, 0x3627,
    0x3086, 0x083D,
    0x3086, 0x6444,
    0x3086, 0x2C2C,
    0x3086, 0x2C2C,
    0x3086, 0x4B01,
    0x3086, 0x432D,
    0x3086, 0x4643,
    0x3086, 0x1647,
    0x3086, 0x435F,
    0x3086, 0x4F50,
    0x3086, 0x2604,
    0x3086, 0x2684,
    0x3086, 0x2027,
    0x3086, 0xFC53,
    0x3086, 0x0D5C,
    0x3086, 0x0D60,
    0x3086, 0x5754,
    0x3086, 0x1709,
    0x3086, 0x5556,
    0x3086, 0x4917,
    0x3086, 0x145C,
    0x3086, 0x0945,
    0x3086, 0x0045,
    0x3086, 0x8026,
    0x3086, 0xA627,
    0x3086, 0xF817,
    0x3086, 0x0227,
    0x3086, 0xFA5C,
    0x3086, 0x0B5F,
    0x3086, 0x5307,
    0x3086, 0x5302,
    0x3086, 0x4D28,
    0x3086, 0x644C,
    0x3086, 0x0928,
    0x3086, 0x2428,
    0x3086, 0x234E,
    0x3086, 0x1718,
    0x3086, 0x26A2,
    0x3086, 0x5C03,
    0x3086, 0x1744,
    0x3086, 0x27F2,
    0x3086, 0x1708,
    0x3086, 0x2803,
    0x3086, 0x2808,
    0x3086, 0x4D1A,
    0x3086, 0x27FA,
    0x3086, 0x2683,
    0x3086, 0x45A0,
    0x3086, 0x1707,
    0x3086, 0x27FB,
    0x3086, 0x1729,
    0x3086, 0x4580,
    0x3086, 0x1708,
    0x3086, 0x27FA,
    0x3086, 0x1728,
    0x3086, 0x2682,
    0x3086, 0x5D17,
    0x3086, 0x0E48 ,
    0x3086, 0x4D4E,
    0x3086, 0x2803,
    0x3086, 0x4C0B,
    0x3086, 0x175F,
    0x3086, 0x27F2,
    0x3086, 0x170A,
    0x3086, 0x2808,
    0x3086, 0x4D1A,
    0x3086, 0x27FA,
    0x3086, 0x2602,
    0x3086, 0x5C00,
    0x3086, 0x4540,
    0x3086, 0x2798,
    0x3086, 0x172A,
    0x3086, 0x4A0A,
    0x3086, 0x4316,
    0x3086, 0x0B43,
    0x3086, 0x279C,
    0x3086, 0x4560,
    0x3086, 0x1707,
    0x3086, 0x279D,
    0x3086, 0x1725,
    0x3086, 0x4540,
    0x3086, 0x1708,
    0x3086, 0x2798,
    0x3086, 0x5D53,
    0x3086, 0x0026,
    0x3086, 0x445C,
    0x3086, 0x014b,
    0x3086, 0x1244,
    0x3086, 0x5251,
    0x3086, 0x1702,
    0x3086, 0x6018,
    0x3086, 0x4A03,
    0x3086, 0x4316,
    0x3086, 0x0443,
    0x3086, 0x1658,
    0x3086, 0x4316,
    0x3086, 0x5943,
    0x3086, 0x165A,
    0x3086, 0x4316,
    0x3086, 0x5B43,
    0x3086, 0x4540,
    0x3086, 0x279C,
    0x3086, 0x4560,
    0x3086, 0x1707,
    0x3086, 0x279D,
    0x3086, 0x1725,
    0x3086, 0x4540,
    0x3086, 0x1710,
    0x3086, 0x2798,
    0x3086, 0x1720,
    0x3086, 0x224B,
    0x3086, 0x1244,
    0x3086, 0x2C2C,
    0x3086, 0x2C2C
#endif
#if SENSOR_REV==REV_2 // MP version dosen't need ~
#endif	
/*
	//ARRAY READOUT SETTINGS
	0x3004, 0,			// X_ADDR_START
	0x3008, 2303,		// X_ADDR_END
	0x3002, 124,    		// Y_ADDR_START
	0x3006, 1419,		// Y_ADDR_END

	//Sub-sampling
	0x30A2,1,			// X_ODD_INCREMENT
	0x30A6,1,			// Y_ODD_INCREMENT
	0x3040, 0,	//no bin

	//Frame-Timing
	0x300C, 1260,		// LINE_LENGTH_PCK
	0x300A, 1308,//1270,		// FRAME_LENGTH_LINES
	0x3014, 0,			// FINE_INTEGRATION_TIME
	0x3012, 1000,		// Coarse_Integration_Time
	0x3042, 0,			// EXTRA_DELAY
	0x30BA, 0x2C,
	0x301A,0x005C,			// Enable Streaming
*/
};

#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)
//MIPI 2 Lane
MMP_USHORT SNR_MT9T002_Reg_Init_Customer[] = {
	0x31AE, 0x204,		// 4-lane MIPI Output
	0x301A,0x0058, //disable streaming
	0xFFFF, 10, //delay 5us
	0x31AE, 0x204, //Output 4-lane MIPI
	//Configure for Serial Interface
	0x301A,0x0058, //Drive Pins,Parallel Enable,SMIA Serializer Disable
	0x3064, 0x1802, 	//Disable Embedded Data
	//Optimized Gain Configuration
	0x3EE0, 0x1500, 	// DAC_LD_20_21
	0x3EEA,0x001D,
	0x31E0,0x1E01,
	0x3F06,0x046A,
	0x3ED2,0x0186,	// upadate 9/30
	0x3ED4,0x8F2C,	// upadate 9/30
	0x3ED6,0x2244,	// upadate 9/30
	0x3ED8,0x6442,	// upadate 9/30
	0x30BA, 0x002C, //Dither enable
	0x3046, 0x4038,		// Enable Flash Pin
	0x3048, 0x8480,		// Flash Pulse Length
	 0x3ED0, 0x0016, 	// DAC_LD_4_5
	 0x3ED0, 0x0036, 	// DAC_LD_4_5
	 0x3ED0, 0x0076, 	// DAC_LD_4_5
	 0x3ED0, 0x00F6, 	// DAC_LD_4_5
	 0x3ECE, 0x1003, 	// DAC_LD_2_3
	 0x3ECE, 0x100F, 	// DAC_LD_2_3
	 0x3ECE, 0x103F, 	// DAC_LD_2_3
	 0x3ECE, 0x10FF, 	// DAC_LD_2_3
	 0x3ED0, 0x00F6, 	// DAC_LD_4_5
	 0x3ED0, 0x04F6, 	// DAC_LD_4_5
	 0x3ED0, 0x24F6, 	// DAC_LD_4_5
	 0x3ED0, 0xE4F6, 	// DAC_LD_4_5
	 0x3EE6, 0xA480, 	// DAC_LD_26_27
	 0x3EE6, 0xA080, 	// DAC_LD_26_27
	 0x3EE6, 0x8080, 	// DAC_LD_26_27
	 0x3EE6, 0x0080, 	// DAC_LD_26_27
	0x3EE6, 0x0080, 	// DAC_LD_26_27
	0x3EE8, 0x2024, 	// DAC_LD_28_29
	0x30FE, 128, //Noise Pedestal of 128

	//Assuming Input Clock of 24MHz.  Output Clock will be 70Mpixel/s
	 0x302A, 0x0005, 	// VT_PIX_CLK_DIV
	 0x302C, 0x0001, 	// VT_SYS_CLK_DIV **MIPI4
	 0x302E, 0x0004, 	// PRE_PLL_CLK_DIV
	 0x3030, 64,//80, 	// PLL_MULTIPLIER
	 0x3036, 0x000A, 	// OP_PIX_CLK_DIV
	 0x3038, 0x0001, 	// OP_SYS_CLK_DIV
	 0x31AC, 0x0A0A, 	// DATA_FORMAT_BITS
	//MIPI TIMING
	0x31B0, 40,                          // FRAME PREAMBLE
	0x31B2, 14,                          // LINE PREAMBLE
	0x31B4, 0x2743,                      // MIPI TIMING 0
	0x31B6, 0x114E,                      // MIPI TIMING 1
	0x31B8, 0x2049,                      // MIPI TIMING 2
	0x31BA, 0x0186,                      // MIPI TIMING 3
	0x31BC, 0x8005,                      // MIPI TIMING 4
	0x31BE, 0x2003,                      // MIPI CONFIG STATUS
	//0x31B0, 41,
	//Sequencer
	0x3088, 0x8000,
	0x3086, 0x4540,
	0x3086, 0x6134,
	0x3086, 0x4A31,
	0x3086, 0x4342,
	0x3086, 0x4560,
	0x3086, 0x2714,
	0x3086, 0x3DFF,
	0x3086, 0x3DFF,
	0x3086, 0x3DEA,
	0x3086, 0x2704,
	0x3086, 0x3D10,
	0x3086, 0x2705,
	0x3086, 0x3D10,
	0x3086, 0x2715,
	0x3086, 0x3527,
	0x3086, 0x053D,
	0x3086, 0x1045,
	0x3086, 0x4027,
	0x3086, 0x0427,
	0x3086, 0x143D,
	0x3086, 0xFF3D,
	0x3086, 0xFF3D,
	0x3086, 0xEA62,
	0x3086, 0x2728,
	0x3086, 0x3627,
	0x3086, 0x083D,
	0x3086, 0x6444,
	0x3086, 0x2C2C,
	0x3086, 0x2C2C,
	0x3086, 0x4B01,
	0x3086, 0x432D,
	0x3086, 0x4643,
	0x3086, 0x1647,
	0x3086, 0x435F,
	0x3086, 0x4F50,
	0x3086, 0x2604,
	0x3086, 0x2684,
	0x3086, 0x2027,
	0x3086, 0xFC53,
	0x3086, 0x0D5C,
	0x3086, 0x0D60,
	0x3086, 0x5754,
	0x3086, 0x1709,
	0x3086, 0x5556,
	0x3086, 0x4917,
	0x3086, 0x145C,
	0x3086, 0x0945,
	0x3086, 0x0017,
	0x3086, 0x0545,
	0x3086, 0x8026,
	0x3086, 0xA627,
	0x3086, 0xF817,
	0x3086, 0x0227,
	0x3086, 0xFA5C,
	0x3086, 0x0B5F,
	0x3086, 0x5307,
	0x3086, 0x5302,
	0x3086, 0x4D28,
	0x3086, 0x6C4C,
	0x3086, 0x0928,
	0x3086, 0x2C28,
	0x3086, 0x294E,
	0x3086, 0x1718,
	0x3086, 0x26A2,
	0x3086, 0x5C03,
	0x3086, 0x1744,
	0x3086, 0x2809,
	0x3086, 0x27F2,
	0x3086, 0x1708,
	0x3086, 0x2809,
	0x3086, 0x1628,
	0x3086, 0x084D,
	0x3086, 0x1A26,
	0x3086, 0x8327,
	0x3086, 0xFA45,
	0x3086, 0xA017,
	0x3086, 0x0727,
	0x3086, 0xFB17,
	0x3086, 0x2945,
	0x3086, 0x8017,
	0x3086, 0x0827,
	0x3086, 0xFA17,
	0x3086, 0x285D,
	0x3086, 0x5300,
	0x3086, 0x17E8,
	0x3086, 0x5302,
	0x3086, 0x1710,
	0x3086, 0x2682,
	0x3086, 0x170E,
	0x3086, 0x484D,
	0x3086, 0x4E28,
	0x3086, 0x094C,
	0x3086, 0x0B17,
	0x3086, 0x5F28,
	0x3086, 0x0927,
	0x3086, 0xF217,
	0x3086, 0x0A28,
	0x3086, 0x0928,
	0x3086, 0x084D,
	0x3086, 0x1A27,
	0x3086, 0xFA26,
	0x3086, 0x025C,
	0x3086, 0x0045,
	0x3086, 0x4027,
	0x3086, 0x9817,
	0x3086, 0x2A4A,
	0x3086, 0x0A43,
	0x3086, 0x160B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x0827,
	0x3086, 0x985D,
	0x3086, 0x5307,
	0x3086, 0x2644,
	0x3086, 0x5C01,
	0x3086, 0x4B12,
	0x3086, 0x4452,
	0x3086, 0x5117,
	0x3086, 0x0260,
	0x3086, 0x184A,
	0x3086, 0x0343,
	0x3086, 0x1604,
	0x3086, 0x4316,
	0x3086, 0x5843,
	0x3086, 0x1659,
	0x3086, 0x4316,
	0x3086, 0x5A43,
	0x3086, 0x165B,
	0x3086, 0x4327,
	0x3086, 0x9C45,
	0x3086, 0x6017,
	0x3086, 0x0727,
	0x3086, 0x9D17,
	0x3086, 0x2545,
	0x3086, 0x4017,
	0x3086, 0x1027,
	0x3086, 0x9817,
	0x3086, 0x2022,
	0x3086, 0x4B12,
	0x3086, 0x442C,
	0x3086, 0x2C2C,
	0x3086, 0x2C00,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
	0x3086, 0x0000,
/*
	//ARRAY READOUT SETTINGS
	0x3004, 0,			// X_ADDR_START
	0x3008, 2303,		// X_ADDR_END
	0x3002, 124,    		// Y_ADDR_START
	0x3006, 1419,		// Y_ADDR_END

	//Sub-sampling
	0x30A2,1,			// X_ODD_INCREMENT
	0x30A6,1,			// Y_ODD_INCREMENT
	0x3040, 0,	//no bin

	//Frame-Timing
	0x300C, 1260,		// LINE_LENGTH_PCK
	0x300A, 1308,//1270,		// FRAME_LENGTH_LINES
	0x3014, 0,			// FINE_INTEGRATION_TIME
	0x3012, 1000,		// Coarse_Integration_Time
	0x3042, 0,			// EXTRA_DELAY
	0x30BA, 0x2C,
	0x301A,0x005C,			// Enable Streaming
*/
};

#endif
void SNR_MT9T002_InitSensor_Customer(void) {
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

        if (vid == 0) {
    		pVIF->VIF_0_SENSR_SIF_EN = VIF_SIF_SEN | VIF_SIF_RST;
    		pVIF->VIF_0_SENSR_SIF_DATA = VIF_SIF_RST;
		}
		else {
    		pVIF->VIF_1_SENSR_SIF_EN = VIF_SIF_SEN | VIF_SIF_RST;
    		pVIF->VIF_1_SENSR_SIF_DATA = VIF_SIF_RST;
		}
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
		
		pMIPI->MIPI_DATA0_DLY[vid] =( 0x10 << 8) | 0 ;
		pMIPI->MIPI_DATA1_DLY[vid] =( 0x10 << 8) | 0 ;
		pMIPI->MIPI_DATA2_DLY[vid] =( 0x10 << 8) | 0 ;
		pMIPI->MIPI_DATA3_DLY[vid] =( 0x10 << 8) | 0 ;
		
	}

#if (CHIP==P_V2)
	ISP_HDM_IF_EnableSensor(ISP_HDM_SNR_ENABLE_HIGH);
#endif
	//dbg_printf(3,"SENSOR RST\r\n");
	//RTNA_WAIT_MS(10);
	/*Reset Sensor 500mS*/
	MMPF_VIF_SetPIODir(VIF_SIF_RST, MMP_TRUE);

	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_FALSE);
	//RTNA_WAIT_MS(10/*50*/);
	// t4 : minimum is 1 ms
    RTNA_WAIT_MS(5);
	MMPF_VIF_SetPIOOutput(VIF_SIF_RST, MMP_TRUE);
	
	//RTNA_WAIT_MS(20/*10*/);
	// t5 : 150000 Ext Clk (24MHz) , around 6.5 ms 
	RTNA_WAIT_MS(10);

	// set register..........
	for (i = 0; i < VR_ARRSIZE(SNR_MT9T002_Reg_Init_Customer)/2; i++) {
		if(SNR_MT9T002_Reg_Init_Customer[i*2] == 0xFFFF)
			delay_1ms(SNR_MT9T002_Reg_Init_Customer[i*2+1]);
		else
			gsSensorFunction->MMPF_Sensor_SetReg(SNR_MT9T002_Reg_Init_Customer[i*2], SNR_MT9T002_Reg_Init_Customer[i*2+1]);
            if(MMPF_Sensor_GetErr()) {
                return ;
            }
#if 0
		data = gsSensorFunction->MMPF_Sensor_GetReg(SNR_MT9T002_Reg_Init_Customer[i*2]) ;
		RTNA_DBG_Str3("Reg :");
		RTNA_DBG_Short3(SNR_MT9T002_Reg_Init_Customer[i*2]);
		RTNA_DBG_Str3(",");
		RTNA_DBG_Short3(data);
		RTNA_DBG_Str3("\r\n");
#endif

	}
	dbg_printf(3, "MMPF_I2CM_ID_1: (x%x, x%x)\r\n", pGBL->GBL_IO_CTL0, pGBL->GBL_IO_CTL1);
	//dbg_printf(3, "VIF_1: (x%x, x%x, x%x)\r\n", pVIF->VIF_IN_EN[VIF_CTL], pVIF->VIF_OUT_EN[VIF_CTL], pVIF->VIF_RAW_OUT_EN[VIF_CTL]);    

	// Set preview resolution as default
	//SNR_MT9T002_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);
}

void MMPF_Sensor_SetSensorGain_MT9T002(MMP_ULONG gain)
{
extern MMP_ULONG glCurSensorGain ;
	ISP_UINT16 sensor_again;
	ISP_UINT16 sensor_dgain;
	
	gain = gain * 0x40 / ISP_IF_AE_GetGainBase();

    if(m_VideoFmt==MMPS_3GPRECD_VIDEO_FORMAT_RAW) {
	    return ;
	}
	// sean@2011_03_25, if set gain = cur gain, just return
	if(glCurSensorGain==gain) {
	    return ;
	}
	//sean@2011_01_24 log the gain
	glCurSensorGain = gain ; 
	//dbg_printf(0,"#>Set.Sensor.Gain:%x\r\n",gain);
	// Sensor Gain Mapping
	if 		(gain < 0x80)  {sensor_dgain = gain << 1;	sensor_again = 0x0;}	// 1X ~ 2X

	else if (gain < 0x100)  {sensor_dgain = gain;	sensor_again = 0x10;}		// 2X ~ 4X

	else if (gain < 0x200) {sensor_dgain = gain >> 1;	sensor_again = 0x20;}	// 4X ~ 8X

	else 				{sensor_dgain = gain >> 2;	sensor_again = 0x30;}		// 8X ~16X

	gsSensorFunction->MMPF_Sensor_SetReg(0x305E, sensor_dgain);
	gsSensorFunction->MMPF_Sensor_SetReg(0x3060, sensor_again);

}

void MMPF_Sensor_SetSensorShutter_MT9T002(MMP_ULONG shutter, MMP_ULONG vsync)
{
static MMP_ULONG glCurShutter = 0;
	ISP_UINT32 new_vsync = g_SNR_3135_LineCntPerSec * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
	ISP_UINT32 new_shutter = g_SNR_3135_LineCntPerSec * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();

//20121204 set exposure time in bayer mode
//	if(m_VideoFmt==MMPS_3GPRECD_VIDEO_FORMAT_RAW) {
//	    return ;
//	}
	
	//dbg_printf(0,"#>Set.Sensor(Shutter,Vsync):(%d,%d)\r\n",new_shutter,new_vsync);
    
    if( shutter ) {
        if(glCurShutter) {
            new_shutter = glCurShutter * shutter ;
            gsSensorFunction->MMPF_Sensor_SetReg(0x3012, new_shutter);
        }
    }
    else {
    	new_vsync = VR_MIN(VR_MAX(new_shutter + 3, new_vsync), 0xFFFF);
    	new_shutter = VR_MIN(VR_MAX(new_shutter, 1), new_vsync - 3);
	    gsSensorFunction->MMPF_Sensor_SetReg(0x300A, new_vsync);
	    gsSensorFunction->MMPF_Sensor_SetReg(0x3012, new_shutter);
	    glCurShutter = new_shutter  ;
	}
}

void SNR_MT9T002_SetSensorAddMode_Customer(ISP_UINT8 enable)
{
/*	if (ISP_HDM_IF_IsFrameTypePreview()) {
		ISP_UINT8 regval = (enable) ? gsSensorFunction->MMPF_Sensor_GetReg(0x3016) | 0x40 : gsSensorFunction->MMPF_Sensor_GetReg(0x3016) & ~0x40;

		gsSensorFunction->MMPF_Sensor_SetReg(0x0104, 0x01); // grouped_parameter_hold
		gsSensorFunction->MMPF_Sensor_SetReg(0x3016, regval);
		gsSensorFunction->MMPF_Sensor_SetReg(0x0104, 0x00); // grouped_parameter_hold
	}
*/
}



#if (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)		

ISP_UINT16 SNR_MT9T002_Reg_2304x1296[] = {

	0x3030, 40, 	// PLL_MULTIPLIER  @ 240Mbps/lane
	//0x3030, 82, 	// PLL_MULTIPLIER  @ 490Mbps/lane

    //ARRAY READOUT SETTINGS
    0x3004, 0,			// X_ADDR_START
	0x3008, 2311,//2303,		// X_ADDR_END
	0x3002, 0,//124,    		// Y_ADDR_START
	0x3006, 1299,//1303,//1419,		// Y_ADDR_END

    //Sub-sampling
    0x30A2, 1,			// X_ODD_INCREMENT
    0x30A6, 1,			// Y_ODD_INCREMENT
	0x3040, 0x0000,	// [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning 

    //Frame-Timing
    0x300C, 1260,		// LINE_LENGTH_PCK
    0x300A, 1312,		// FRAME_LENGTH_LINES
    0x3014, 0,			// FINE_INTEGRATION_TIME
    0x3012, 1000,		// Coarse_Integration_Time
    0x3042, 0,			// EXTRA_DELAY
    0x30BA, 0x2C,	    // Digital_Ctrl_Adc_High_Speed
  
    0x301A, 0x000C, //Grouped Parameter Hold = 0x0
};

ISP_UINT16 SNR_MT9T002_Reg_1920x1080[] = {

	0x3030, 40, 	// PLL_MULTIPLIER  @ 240Mbps/lane

    //ARRAY READOUT SETTINGS
	0x3004, 192,			// X_ADDR_START
	0x3008, 2127,		// X_ADDR_END
	0x3002, 232,    		// Y_ADDR_START
	0x3006, 1319,		// Y_ADDR_END

    //Sub-sampling
    0x30A2, 1,			// X_ODD_INCREMENT
    0x30A6, 1,			// Y_ODD_INCREMENT
    0x3040, 0, 	// Row Bin, [12]: Row binning, [13]: column binning

    //Frame-Timing
    0x300C, 1260,//1056,		// LINE_LENGTH_PCK
    0x300A, 1270,//1092,		// FRAME_LENGTH_LINES
    0x3014, 136,			// FINE_INTEGRATION_TIME
    0x3012, 1000,		// Coarse_Integration_Time
    0x3042, 0,			// EXTRA_DELAY
    0x30BA, 0x2C,	// Digital_Ctrl_Adc_High_Speed
  
 	0x301A, 0x000C, //Grouped Parameter Hold = 0x0
};

ISP_UINT16 SNR_MT9T002_Reg_1152x648[] = {

	0x3030, 42, 	// PLL_MULTIPLIER  @ 430Mbps/lane
	//0x3030, 82, 	// PLL_MULTIPLIER  @ 490Mbps/lane

    //ARRAY READOUT SETTINGS
	0x3004, 0,			// X_ADDR_START
	0x3008, 2311,//2303,		// X_ADDR_END
//	0x3002, 120,//124,    		// Y_ADDR_START
//	0x3006, 1423,//1419,		// Y_ADDR_END
	0x3002, 0,//124,    		// Y_ADDR_START
	0x3006, 1303,//1419,		// Y_ADDR_END

    //Sub-sampling
    0x30A2, 3,			// X_ODD_INCREMENT
    0x30A6, 3,			// Y_ODD_INCREMENT
    0x3040, 0x3000, 	// Row-colum Bin, [12]: Row binning, [13]: column binning

    //Frame-Timing
    0x300C, 1250,		// LINE_LENGTH_PCK
    0x300A, 660+8,		// FRAME_LENGTH_LINES
    0x3014, 0,			// FINE_INTEGRATION_TIME
    0x3012, 550,		// Coarse_Integration_Time
    0x3042, 0,			// EXTRA_DELAY
    0x30BA, 0x2C,	// Digital_Ctrl_Adc_High_Speed
  
    0x301A, 0x000C, //Grouped Parameter Hold = 0x0
};

ISP_UINT16 SNR_MT9T002_Reg_2304x648[] = {

	0x3030, 42, 	// PLL_MULTIPLIER  @ 430Mbps/lane
	//0x3030, 82, 	// PLL_MULTIPLIER  @ 490Mbps/lane

    //ARRAY READOUT SETTINGS
	0x3004, 0,			// X_ADDR_START
	0x3008, 2311,//2303,		// X_ADDR_END
//	0x3002, 120,//124,    		// Y_ADDR_START
//	0x3006, 1423,//1419,		// Y_ADDR_END
	0x3002, 0,//124,    		// Y_ADDR_START
	0x3006, 1303,//1419,		// Y_ADDR_END

    //Sub-sampling
    0x30A2, 1,			// X_ODD_INCREMENT
    0x30A6, 3,			// Y_ODD_INCREMENT
    0x3040, 0x1000, 	// Row-colum Bin, [12]: Row binning, [13]: column binning

    //Frame-Timing
    0x300C, 1260,		// LINE_LENGTH_PCK
    0x300A, 660+8,		// FRAME_LENGTH_LINES
    0x3014, 0,			// FINE_INTEGRATION_TIME
    0x3012, 550,		// Coarse_Integration_Time
    0x3042, 0,			// EXTRA_DELAY
    0x30BA, 0x2C,	// Digital_Ctrl_Adc_High_Speed
  
    0x301A, 0x000C, //Grouped Parameter Hold = 0x0
};


#else //MIPI 2-lane

ISP_UINT16 SNR_MT9T002_Reg_2304x1296[] = {
#if SUPPORT_GRA_ZOOM

#if SENSOR_IN_H > SENSOR_16_9_H
	0x3030, 86+4, 	// PLL_MULTIPLIER  @ 516Mbps/lane
#else
	0x3030, 86, 	// PLL_MULTIPLIER  @ 516Mbps/lane
#endif
	
#else
	0x3030, (82+1), 	// PLL_MULTIPLIER  @ 490Mbps/lane
#endif
	//ARRAY READOUT SETTINGS
	// +2 for X axis dead pixel.
	0x3004, 0+2,			// X_ADDR_START
	0x3008, 2311+2,//2303,		// X_ADDR_END
#if SENSOR_IN_H > SENSOR_16_9_H
	0x3002, 100,//0,//124,    		// Y_ADDR_START
	0x3006, 100 + (SENSOR_HEIGHT_16_9 + 4 -1 ),//1299,//1303,//1419,		// Y_ADDR_END
#else	
	0x3002, 122,//0,//124,    		// Y_ADDR_START
	0x3006, 1421,//1299,//1303,//1419,		// Y_ADDR_END
#endif
	//Sub-sampling
	0x30A2, 1,			// X_ODD_INCREMENT
	0x30A6, 1,			// Y_ODD_INCREMENT
	0x3040, 0x0000,	// [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning

	0x300C, 1250,//1248, // LINE_LENGTH_PCK
	0x300A, 1312,//1092,		// FRAME_LENGTH_LINES
	0x3014, 0,		// FINE_INTEGRATION_TIME
	0x3012, (152),//(1000),		// Coarse_Integration_Time
	0x3042, 0,			// EXTRA_DELAY
	0x30BA, 0x2C,

	0x301A, 0x000C, //Grouped Parameter Hold = 0x0

};


ISP_UINT16 SNR_MT9T002_Reg_2304x1536[] = {

	0x3030, 82, 	// PLL_MULTIPLIER  @ 490Mbps/lane

	//ARRAY READOUT SETTINGS
	0x3004, 0+2,			// X_ADDR_START
	0x3008, 2311+2,//2303,		// X_ADDR_END
	0x3002, 0 + 6,//0,//124,    		// Y_ADDR_START
	0x3006, 0 + 6 + (1536 + 4 - 1),//1299,//1303,//1419,		// Y_ADDR_END

	//Sub-sampling
	0x30A2, 1,			// X_ODD_INCREMENT
	0x30A6, 1,			// Y_ODD_INCREMENT
	0x3040, 0x0000,	// [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning
	0x300C, 1250,//1250 ,//1192,////;50,//1248, // LINE_LENGTH_PCK
	0x300A, 1536+4,//,//1092,		// FRAME_LENGTH_LINES
	0x3014, 0,		// FINE_INTEGRATION_TIME
	0x3012, 1000,		// Coarse_Integration_Time
	0x3042, 0,			// EXTRA_DELAY
	0x30BA, 0x2C,

	0x301A, 0x000C, //Grouped Parameter Hold = 0x0

};


ISP_UINT16 SNR_MT9T002_Reg_1920x1080[] = {

	0x3030, 82, 	// PLL_MULTIPLIER  @ 490Mbps/lane

	//ARRAY READOUT SETTINGS
	0x3004, 192,			// X_ADDR_START
	0x3008, 2127,		// X_ADDR_END
	0x3002, 232,    		// Y_ADDR_START
	0x3006, 1319,		// Y_ADDR_END

	//Sub-sampling
	0x30A2, 1,			// X_ODD_INCREMENT
	0x30A6, 1,			// Y_ODD_INCREMENT
	0x3040, 0x0000,	// [15]: flip, [14]: mirror, [12]: Row binning, [13]: column binning

	0x300C, 1260,//1190,		// LINE_LENGTH_PCK
	0x300A, 1270,//1092,		// FRAME_LENGTH_LINES
	0x3014, 0,		// FINE_INTEGRATION_TIME
	0x3012, 1000,		// Coarse_Integration_Time
	0x3042, 0,			// EXTRA_DELAY
	0x30BA, 0x2C,

	0x301A, 0x000C, //Grouped Parameter Hold = 0x0

};

ISP_UINT16 SNR_MT9T002_Reg_1152x648[] = {

	0x3030, 82, 	// PLL_MULTIPLIER @ 490Mbps/lane

    //ARRAY READOUT SETTINGS
	0x3004, 2+0,			// X_ADDR_START
	0x3008, 2+(SENSOR_WIDTH_16_9_BINNING + 4 )* 2 - 1 , //2311,//2303,		// X_ADDR_END
	0x3002, 122,//124,    		// Y_ADDR_START
	0x3006, 122 + (SENSOR_HEIGHT_16_9_BINNING + 4) * 2 - 1 ,//1425,//1419,		// Y_ADDR_END

    //Sub-sampling
    0x30A2, 3,			// X_ODD_INCREMENT
    0x30A6, 3,			// Y_ODD_INCREMENT
    0x3040, 0x3000, 	// Row-colum Bin, [12]: Row binning, [13]: column binning

    //Frame-Timing
    0x300C, 1232,//1216,//1260,//920,		// LINE_LENGTH_PCK
    0x300A, 672,		// FRAME_LENGTH_LINES
    0x3014, 0,			// FINE_INTEGRATION_TIME
    0x3012, 552,		// Coarse_Integration_Time
    0x3042, 0,			// EXTRA_DELAY
    0x30BA, 0x2C,	// Digital_Ctrl_Adc_High_Speed
  
    0x301A, 0x000C, //Grouped Parameter Hold = 0x0
};

#endif

MMP_ULONG VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;

void SNR_MT9T002_SetSensorResolution_Customer(ISP_SENSOR_RESOL res)
{
	// implement your change sensor resolution routine here =======================================

	MMP_ULONG i ;//, VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;
    PCAM_USB_VIDEO_RES ep_res ;
//sean@2011_01_24, if same sensor resolution, just return
    if(gsCurSensorEnumRes==res) {
       // dbg_printf(3,"#Sensor Same Res: %d\r\n",res);
        return ;
    }
    gsCurSensorEnumRes = res ;
    //dbg_printf(3,"#Sensor Res Change : %d\r\n",res);
    ep_res = GetFirstStreamingRes();
    
	switch (res) {
	case ISP_SENSOR_RESOL_1920x1080:
	case ISP_SENSOR_RESOL_1280x960:
	case ISP_SENSOR_RESOL_1280x720:

#if CROP_1080P_FROM_2304_1296
		// set register...........
		for (i = 0; i < VR_ARRSIZE(SNR_MT9T002_Reg_1920x1080)/2; i++) {
			gsSensorFunction->MMPF_Sensor_SetReg(SNR_MT9T002_Reg_1920x1080[i*2], SNR_MT9T002_Reg_1920x1080[i*2+1]);
	    }
#else
		// set register...........
        if(ep_res!=PCCAM_2304_1536) { //sean@2011_01_24
    		for (i = 0; i < VR_ARRSIZE(SNR_MT9T002_Reg_2304x1296)/2; i++) {
    			gsSensorFunction->MMPF_Sensor_SetReg(SNR_MT9T002_Reg_2304x1296[i*2], SNR_MT9T002_Reg_2304x1296[i*2+1]);
    	    }
    	    
    	    
	    } else {
	        RTNA_DBG_Str0("PCCAM_2304_1536...\r\n");
    		for (i = 0; i < VR_ARRSIZE(SNR_MT9T002_Reg_2304x1536)/2; i++) {
                gsSensorFunction->MMPF_Sensor_SetReg(SNR_MT9T002_Reg_2304x1536[i*2], SNR_MT9T002_Reg_2304x1536[i*2+1]);
    	    }
	    }
	    
#endif

		// set target fps and corresponding sensor vsync
#if CROP_1080P_FROM_2304_1296
		ISP_IF_AE_SetMaxSensorFPSx10(30 * 10);
		g_SNR_3135_LineCntPerSec = 1270 * 30;
	
#else
    if(ep_res==PCCAM_2304_1536) { //sean@2011_01_24
		ISP_IF_AE_SetMaxSensorFPSx10(30 * 10);
		g_SNR_3135_LineCntPerSec = 1536 * 30;        
    } else {

		ISP_IF_AE_SetMaxSensorFPSx10(30 * 10);
		#if SUPPORT_GRA_ZOOM
		
		#if SENSOR_IN_H > SENSOR_16_9_H
		g_SNR_3135_LineCntPerSec =  (gsSensorHeightByFOV+ 4 + 50) * 30; ///test20120821 +2 for DMA        
		#else
		g_SNR_3135_LineCntPerSec =  (1340 + /*20*/ 35) * 30; ///test20120821 +2 for DMA        
		#endif
		
		#else
		g_SNR_3135_LineCntPerSec = (1316 + 11 )* 30; ///test20120821 +2 for DMA        
		#endif
    }
#endif		

    ISP_IF_AE_SetShutterBase(g_SNR_3135_LineCntPerSec);
#if CROP_1080P_FROM_2304_1296
	// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = 1936;
		VIFGrab_V_Start     = 1;
		VIFGrab_V_Length    = 1088;

#else

	// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1+4; //shift right 4 pixel, sensor start from 2
		VIFGrab_H_Length    = gsSensorWidthByFOV + 4;
		VIFGrab_V_Start     = 1;
		VIFGrab_V_Length    = gsSensorHeightByFOV+ 4;
		if(m_VideoFmt==MMPS_3GPRECD_VIDEO_FORMAT_RAW) { 
		    VIFGrab_H_Length    = gsSensorWidthByFOV ;
		    VIFGrab_V_Length    = gsSensorHeightByFOV ;
		}
		//dbg_printf(0, "gsSensorWidthByFOV = %d, gsSensorHeightByFOV = %d\r\n", gsSensorWidthByFOV, gsSensorHeightByFOV);
#endif
        
		ISP_IF_AE_SetMinAccLength(VIFGrab_H_Length, VIFGrab_V_Length);        
		ISP_IF_IQ_SetISPInputLength(VIFGrab_H_Length, VIFGrab_V_Length);
		ISP_IF_IQ_SetScalerOutputLength(VIFGrab_H_Length, VIFGrab_V_Length, 0 ,0);	//20121130, pan/tile/zoom	
		//sISP_IF_IQ_SetISPInputLength(640, 480);
		
		// set scaler downsample rate (for scaler calculation)
		//ISP_IF_SNR_SetDownSample(1, 1);

		// set color id
		ISP_IF_IQ_SetColorID(0);
		
		break;

	case ISP_SENSOR_RESOL_800x600:
	case ISP_SENSOR_RESOL_640x480:

#if ENABLE_ROW_BINNING_2304_648
		// set register...........
		for (i = 0; i < VR_ARRSIZE(SNR_MT9T002_Reg_2304x648)/2; i++) {
			gsSensorFunction->MMPF_Sensor_SetReg(SNR_MT9T002_Reg_2304x648[i*2], SNR_MT9T002_Reg_2304x648[i*2+1]);
	    }
#else
		// set register...........
		for (i = 0; i < VR_ARRSIZE(SNR_MT9T002_Reg_1152x648)/2; i++) {
			gsSensorFunction->MMPF_Sensor_SetReg(SNR_MT9T002_Reg_1152x648[i*2], SNR_MT9T002_Reg_1152x648[i*2+1]);
	    }
#endif

		// set sensor vsync
		ISP_IF_AE_SetMaxSensorFPSx10(60 * 10);
		g_SNR_3135_LineCntPerSec = 672 * 60;		

#if ENABLE_ROW_BINNING_2304_648
	// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = 2304 + 8;
		VIFGrab_V_Start     = 1;
		VIFGrab_V_Length    = 648 + 4;

#else
	// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1;
		VIFGrab_H_Length    = SENSOR_WIDTH_16_9_BINNING + 4;
		VIFGrab_V_Start     = 1;
		VIFGrab_V_Length    = SENSOR_HEIGHT_16_9_BINNING + 4;

#endif
        ISP_IF_AE_SetMinAccLength(VIFGrab_H_Length, VIFGrab_V_Length);
        ISP_IF_IQ_SetISPInputLength(VIFGrab_H_Length, VIFGrab_V_Length);
		ISP_IF_IQ_SetScalerOutputLength(VIFGrab_H_Length, VIFGrab_V_Length, 0 ,0);	//20121130, pan/tile/zoom	
        
		// set scaler downsample rate (for scaler calculation)
		//ISP_IF_SNR_SetDownSample(1, 1);

		// set color id
		ISP_IF_IQ_SetColorID(0); //sport-cam 60 fps
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
		dbg_printf(3,"ViF(xs,xe,ys,ye)=(%d,%d,%d,%d)\r\n",pVIF->VIF_GRAB[vid].PIXL_ST,pVIF->VIF_GRAB[vid].PIXL_ED,pVIF->VIF_GRAB[vid].LINE_ST,pVIF->VIF_GRAB[vid].LINE_ED);
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


MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_MT9T002;
#if (TOTAL_SENSOR_NUMBER >= 2)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_MT9T002;
#endif

#endif  //BIND_SENSOR_MT9T002
#endif
