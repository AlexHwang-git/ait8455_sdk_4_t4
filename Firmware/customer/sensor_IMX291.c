//==============================================================================
//
//  File        : sensor_IMX291.c
//  Description : Firmware Sensor Control File
//  Author      : gason mo
//  Revision    : 1.0
//
//=============================================================================

#include "includes_fw.h"
#if (SENSOR_EN)
#include "config_fw.h"

#if BIND_SENSOR_IMX291

#include "lib_retina.h"
#include "reg_retina.h"

#include "mmp_reg_vif.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_gpio.h"
#include "mmp_reg_rawproc.h"

#include "mmpf_sensor.h"
#include "mmpf_pll.h"
#include "mmpf_hif.h"
#include "mmpf_i2cm.h"
#include "mmpf_vif.h"
#include "isp_if.h"
#include "mmpf_usbpccam.h"

#include "mmp_reg_i2cm.h"
#include "mmps_3gprecd.h"

#include "hdm_ctl.h"
#define VIPER           (0)
#define HD_PREVIEW      (0)

#define SENSOR_RESO_NUM             (3)

MMP_USHORT  gsSensorPreviewWidth[SENSOR_RESO_NUM], gsSensorPreviewHeight[SENSOR_RESO_NUM];
MMP_ULONG g_SNR_LineCntPerSec = 1;

extern MMP_USHORT  gsCurPreviewMode, gsCurPhaseCount;
extern MMP_USHORT  gsSensorMode;
extern MMP_USHORT  m_usVideoQuality;
extern MMP_USHORT	m_gsISPCoreID;

extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_BOOL    gbAutoFocus;
extern MMP_BOOL    m_bISP3AStatus;

extern MMP_USHORT gsEnableTestPattern;
extern MMP_USHORT gsTestPatternMode;
extern MMP_USHORT m_usResolType;
extern MMP_ULONG   glPccamResolution;
extern H264_FORMAT_TYPE gbCurH264Type;
extern void InitializeMotor(void);
extern void GPIO_OutputControl(MMP_UBYTE num, MMP_UBYTE status);
extern void SetMotorPos(MMP_USHORT pos);
extern void dbg_printf(unsigned long level, char *fmt, ...);
extern void delay_1us(MMP_ULONG us);



const __align(4) ISP_UINT8 Sensor_IQ_CompressedText[] = {
#if (CHIP == VSN_V3)||(CHIP == MERCURY)
//#include "isp_8453_iq_data_v2_IMX208.xls.ciq.txt"
#include "isp_8453_iq_data_v2_IMX291_20170725.xls.ciq.txt"
#endif
};

#define SENSOR_I2C_ADDR_IMX291    0x1A

MMPF_I2CM_ATTRIBUTE m_snr_I2cmAttribute = {
SENSOR_I2CM_ID, 0x1A, 16, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE,MMP_FALSE, 0, 0, 1, MMPF_I2CM_SPEED_HW_400K, NULL, NULL

};
#if SUPPORT_IR_LED
MMP_UBYTE  gbNightMode = 0;   
MMP_ERR MMPF_ICR_Control(MMP_USHORT NightMode);
#endif


void SNR_IMX291_InitSensor_Customer(void);
void SNR_IMX291_SetSensorResolution_Customer(ISP_SENSOR_RESOL res);
MMPF_SENSOR_ISP_MODE gSensorISPMode = MMPF_SENSOR_MODE_INIT;

#define TEST_SENSOR_PRATE 1
//#define SUPPORT_AUTO_FOCUS 0

//#include "sensor_ctl.h"
//#include "preview_ctl.h"
//#include "XDATAmap.h"
//#include "3A_ctl.h"
//#include "IMX291.h"
//#include "motor_ctl.h"

unsigned char metering_weight_tbl[128] = 
{
	//ISP_AE_METERING_CENTER
	0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00,
	0x00, 0x00, 0x33, 0x34, 0x43, 0x33, 0x00, 0x00,
	0x00, 0x00, 0x33, 0x34, 0x43, 0x33, 0x00, 0x00,
	0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,	
};

//------------------------------------------------------------------------------
//  Function    : IMX291_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
extern MMPF_SENSOR_FUNCTION *gsSensorFunction ;
extern MMP_USHORT ST5WorkingMode; // 0 is 3M , 1 is 5M.

static void IMX291_Sensor_PreviewMode(MMP_USHORT usPreviewmode)
{
	gsCurPreviewMode = usPreviewmode;
	//gsCurPhaseCount = usPhasecount;
	ISP_IF_AE_SetFPS(0);
	switch (usPreviewmode) {
	case 0:
		RTNA_DBG_Str(3, "Sensor preview0 (1920x1080) mode\r\n");
		gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);//patrick@100518
		ISP_IF_IQ_SetSysMode(0); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....

		//ISP_IF_SNR_SetResolution(ISP_SENSOR_RESOL_1920x1080);
		SNR_IMX291_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);

	//	ISP_IF_AE_SetFPS(30);

		gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE); //patrick@100518
		break;

	case 1:
		RTNA_DBG_Str(3, "Sensor preview1 (1920x1080) mode\r\n");
			RTNA_DBG_Str(0, "SENSOR_VIDEO_MODE\r\n");
			//ISP_IF_3A_Control(ISP_3A_DISABLE); //patrick@100518
			gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
			//ISP_IF_SNR_SetBeforePreview();
			gSensorISPMode = MMPF_SENSOR_PREVIEW;
			ISP_IF_IQ_SetSysMode(1); //Steven NOTE:please reference excel define: 0=capture, 1=preview, 2=video....
			SNR_IMX291_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);

            ISP_IF_AE_SetFPS(0); //auto fps
            gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE);

		break;
	}
	ISP_IF_AE_SetSysMode(0);
	ISP_IF_AF_SetSysMode(0);
	ISP_IF_AWB_SetSysMode(0);		
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Initialize
//  Description :
//------------------------------------------------------------------------------
extern PCAM_USB_INFO *pcam_get_info(void);
ISP_UINT32 gISPFrameCnt = 0;

static MMP_ERR  MMPF_Sensor_Initialize_IMX291(void)
{
    #if (CHIP == P_V2)
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
	AITPS_PAD   pPAD  = AITC_BASE_PAD;
    #endif
	#if (SENSOR_SUPPORT_SCALER == 1)
	MMPF_SENSOR_SCALE_STATE ScaleStaust;
	#endif
	gISPFrameCnt = 0;

	RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_IMX291...Begin\r\n");

    //Init globol variable used by DSC
	/*gsSensorPreviewWidth[0] = 1280;
	gsSensorPreviewHeight[0] = 720;
    // use 1.3M for preview, check sensor preview setting
    gsSensorPreviewWidth[1] = 1280;
    gsSensorPreviewHeight[1] = 720;

    gsSensorPreviewWidth[2] = 1280;
    gsSensorPreviewHeight[2] = 720;
    */
    
    //Init globol variable used by DSC
	gsSensorPreviewWidth[0] = 1920;
	gsSensorPreviewHeight[0] = 1088;
    // use 1.3M for preview, check sensor preview setting
    gsSensorPreviewWidth[1] = 1920;
    gsSensorPreviewHeight[1] = 1088;

    gsSensorPreviewWidth[2] = 1920;
    gsSensorPreviewHeight[2] = 1088;
	
    //MMPF_Sensor_SetScaleResol(1920, 1088);

    //MMPF_Sensor_SaveDefModeInfo(gsSensorPreviewWidth, gsSensorPreviewHeight, SENSOR_RESO_NUM);
	
    //ScaleStaust = MMPF_SENSOR_SCALE_STATE_DOWN;
    //MMPF_Sensor_SetParam(MMPF_SENSOR_SCALE_STATE_INFO, (void*)&ScaleStaust);

	//ISP_IF_SNR_Init();
	SNR_IMX291_InitSensor_Customer();

	gsCurPreviewMode = 1;
    gsSensorFunction->MMPF_Sensor_SetPreviewMode(gsCurPreviewMode);

#if SUPPORT_AUTO_FOCUS	
	InitializeMotor();
#endif
	RTNA_DBG_Str(0, "MMPF_Sensor_Initialize_IMX291...End\r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_PowerDown_IMX291
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_PowerDown_IMX291(void)
{
	AITPS_VIF   pVIF = AITC_BASE_VIF;
    #if (CHIP == P_V2)
	AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
    #endif
	AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
	MMP_UBYTE   vid = 0;


    #if (CHIP == VSN_V3)||(CHIP == MERCURY)
    pMIPI->MIPI_DATA0_CFG[vid] &= ~(MIPI_CSI2_EN);
    #endif
    #if (CHIP == P_V2)
	pMIPI->MIPI_DATA1_CFG &= ~(MIPI_CSI2_EN);
	pMIPI->MIPI_DATA2_CFG &= ~(MIPI_CSI2_EN);
	#endif
	MMPF_VIF_EnableInputInterface(MMPF_VIF_MDL_ID0, MMP_FALSE);

    #if SUPPORT_AUTO_FOCUS
    GPIO_OutputControl(AF_EN, FALSE);
    #endif

    #if SUPPORT_IR_LED
    // Control ICR
	MMPF_PIO_SetData(MMPF_PIO_REG_GPIO1, 1);  // Set ENB to high to turn off ICR
    #endif


	#if (CUSTOMER == FOX)
	//GPIO_OutputControl(SEN_PWR_EN, FALSE);
	//GPIO_OutputControl(VCORE, FALSE);
	//GPIO_OutputControl(LED_EN, FALSE);
	#endif

//    #if (CHIP == P_V2)
    MMPF_VIF_SetPIODir(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
	MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);  //Pin output high
	MMPF_OS_Sleep_MS(10);
	MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_FALSE);//Pin output low
	MMPF_OS_Sleep_MS(10);
//    #endif

    #if 1 // TBD
	MMPF_VIF_SetPIODir(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_TRUE);
	//MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_TRUE);  //Pin output high
    //RTNA_WAIT_MS(1);
    MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_FALSE);//Pin output low
	MMPF_OS_Sleep_MS(1);
	#endif

	pVIF->VIF_SENSR_CLK_CTL[vid] &= (~ VIF_SENSR_CLK_EN) ;   //output sensor main clock

    #if (CHIP == P_V2)
	pGPIO->GPIO_DATA[3] &= ~(1 << 16);  // Turn Off Sensor Power
    #endif

	RTNA_DBG_Str3( "MMPF_Sensor_PowerDown_IMX291() \r\n");
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetFrameRate_IMX291
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_SetFrameRate_IMX291(MMP_UBYTE ubFrameRate)
{
	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_SetPreviewMode_IMX291(MMP_USHORT usPreviewmode)
{
	IMX291_Sensor_PreviewMode(usPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetPreviewMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangePreviewMode_IMX291(
						MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
						MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount)
{
	IMX291_Sensor_PreviewMode(usNewPreviewmode);

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ChangeMode
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR MMPF_Sensor_ChangeMode_IMX291(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode)
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
			MMPF_Sensor_SetPreviewMode_IMX291(0);
			break;
		case 1:
			MMPF_Sensor_SetPreviewMode_IMX291(1);
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
static MMP_ERR  MMPF_Sensor_InitializeVIF_IMX291(void)
{
    #if (CHIP == P_V2)||(CHIP == VSN_V3)||(CHIP == MERCURY)
	AITPS_MIPI  pMIPI = AITC_BASE_MIPI;
	#if (CHIP == VSN_V3)||(CHIP == MERCURY)
	MMP_UBYTE   vid = 0;
	#endif
    //#if (SENSOR_IF == SENSOR_IF_PARALLEL)
	AITPS_VIF   pVIF = AITC_BASE_VIF;
    //#endif
	#endif
	MMPF_VIF_IF vif_if;
	MMP_ULONG   ulSENSORCLK;

    #if SUPPORT_AUTO_FOCUS
    GPIO_OutputControl(AF_EN, TRUE);
	RTNA_WAIT_MS(20);
    #endif

    #if SUPPORT_IR_LED
    // Control IR LED
	MMPF_PIO_EnableGpioMode(MMPF_PIO_REG_GPIO19, 1);  // BGPIO11
	MMPF_PIO_EnableOutputMode(MMPF_PIO_REG_GPIO19, 1);
    #endif

    #if SUPPORT_IR_LED
    // Control ICR
	MMPF_PIO_EnableGpioMode(MMPF_PIO_REG_GPIO1, 1); // AGPIO1 -> ENB
	MMPF_PIO_EnableOutputMode(MMPF_PIO_REG_GPIO1, 1);
	MMPF_PIO_EnableGpioMode(MMPF_PIO_REG_GPIO7, 1); // AGPIO7 -> FBC
	MMPF_PIO_EnableOutputMode(MMPF_PIO_REG_GPIO7, 1);
	MMPF_PIO_SetData(MMPF_PIO_REG_GPIO1, 0);  // Set ENB to low to turn on ICR
    #endif

	#if (CUSTOMER == FOX)
	//GPIO_OutputControl(SEN_PWR_EN, TRUE);
	//GPIO_OutputControl(VCORE, TRUE);
	//GPIO_OutputControl(LED_EN, TRUE);
	#endif


    #if (SENSOR_IF == SENSOR_IF_PARALLEL)
	MMPF_VIF_RegisterInputInterface(MMPF_VIF_MDL_ID0, MMPF_VIF_PARALLEL);
	pVIF->VIF_SENSR_CTL[vid] = VIF_SENSR_LATCH_NEG;
    #else
    #if (CHIP == VSN_V3)
	pMIPI->MIPI_DATA0_DLY[vid] |= (0x21 << 8); // From KW
	pMIPI->MIPI_DATA1_DLY[vid] |= (0x21 << 8); // From KW
	#endif
    #if (CHIP == P_V2)
	pMIPI->MIPI_DATA1_SOT = 0x02; // From KW
	pMIPI->MIPI_DATA2_SOT = 0x02; // From KW
	pMIPI->MIPI_DATA3_SOT = 0x02; // From KW
	pMIPI->MIPI_DATA4_SOT = 0x02; // From KW
	#endif
	MMPF_VIF_RegisterInputInterface(MMPF_VIF_MDL_ID0, MMPF_VIF_MIPI);
    #endif
    
    
    MMPF_VIF_SetPIODir(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
	//MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
	//MMPF_OS_Sleep_MS(5);
	MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_FALSE);
	MMPF_OS_Sleep_MS(5);
    


	MMPF_VIF_SetPIODir(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_TRUE);

	MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_TRUE);  //Pin output high
	#if (CHIP == P_V2)
	pGPIO->GPIO_EN[3] |= (1 << 16); // config as output mode
	pGPIO->GPIO_DATA[3] |= (1 << 16);  // Turn On Sensor Power
	#endif
	MMPF_OS_Sleep_MS(10);

    //MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_FALSE);//Pin output low
	//MMPF_OS_Sleep_MS(5);

	pVIF->VIF_SENSR_CLK_CTL[vid] = VIF_SENSR_CLK_EN;
	MMPF_OS_Sleep_MS(5);

	/*Reset Sensor 500mS*/
	MMPF_VIF_SetPIODir(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
	//MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
	//MMPF_OS_Sleep_MS(5);
	//MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_FALSE);
	MMPF_OS_Sleep_MS(5);

	MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
	MMPF_OS_Sleep_MS(20);
		

	//ISP_HDM_IF_SNR_ConfigI2C( 0x36, ISP_I2C_TYPE_2A1D);

	// initialize VIF OPRs
#if (SENSOR_IF == SENSOR_IF_PARALLEL)
	vif_if = MMPF_VIF_IF_PARALLEL;		//ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_PARALLEL, 22);
#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)	
	vif_if = MMPF_VIF_IF_MIPI_SINGLE_0;	//ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_SINGLE_0, 22);
#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)		
	vif_if = MMPF_VIF_IF_MIPI_DUAL_01;	//ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_DUAL_01, 22);
#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)		
	vif_if = MMPF_VIF_IF_MIPI_QUAD;		//ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_QUAD, 22);
#endif	

	MMPF_OS_Sleep_MS(1);

	{
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

		RTNA_DBG_Str(0,"ISP_HDM_IF_InitVIF\r\n");
		MMPF_PLL_GetGroupFreq(4, &ulSENSORCLK);

		switch(ulSENSORCLK){
			case 96000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D4; // 24MHz
				RTNA_DBG_Str3("96MHz DIV 4\r\n");
				break;
			case 144000:
				RTNA_DBG_Str3("Sensor DIV 6 for 144MHz\r\n");
				#if (BIND_SENSOR_IMX291 == 1)
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D8;
				#else
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D6; 
				#endif
				break;

			case 132000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D11;  // 132/11 = 12M
				break;
			case 166000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D7;
				break;
			case 192000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D8;
				RTNA_DBG_Str3("192MHz DIV 8\r\n");
				break;
			case 216000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D9;
				RTNA_DBG_Str3("216MHz DIV 9\r\n");
				break;
			case 240000:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D6;
				break;
			case 264000:
				//pVIF->VIF_SENSR_CLK_FREQ[vid] = 0x15;
				//pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D11;
				//RTNA_DBG_Str3("264MHz DIV 11\r\n");
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D7;  
				RTNA_DBG_Str3("264MHz DIV 7\r\n");
				break;
			case 120000:
				#if (BIND_SENSOR_IMX291==1)
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D10;
				RTNA_DBG_Str3("120MHz DIV 9\r\n")
				#else
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D5;
				RTNA_DBG_Str3("120MHz DIV 5\r\n")
				#endif
				break;
			default:
				pVIF->VIF_SENSR_CLK_FREQ[vid] = VIF_SENSR_CLK_PLL_D12;  // 144/12 = 12M
				break;
		}

       
		//pVIF->VIF_SENSR_CTL[vid] = VIF_LINE_ID_POLAR | VIF_PIXL_ID_POLAR;
		

		pVIF->VIF_YUV_CTL[vid] = VIF_YUV_FORMAT_YUV;

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
		pVIF->VIF_SENSR_CLK_CTL[vid] = VIF_SENSR_CLK_EN;
		pVIF->VIF_OPR_UPD[vid] = VIF_OPR_UPD_EN | VIF_OPR_UPD_FRAME;

		if (vif_if==MMPF_VIF_IF_PARALLEL) {
			RTNA_DBG_Str0("MMPF_VIF_IF_PARALLEL\r\n");
		}
		else {
            //#if ((BIND_SENSOR_IMX291) && (SENSOR_IF == SENSOR_IF_MIPI_1_LANE) && (CUSTOMER == LGT)) || (BIND_SENSOR_IMX045ES)
		    //pMIPI->MIPI_DATA0_CFG[vid] = MIPI_CSI2_EN | MIPI_SRC_SEL_1;  // for old AIT840 MIPI sensor board
            //#elif ((BIND_SENSOR_IMX291) && (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)) && (CUSTOMER == NMG)
		    //pMIPI->MIPI_DATA0_CFG[vid] = MIPI_CSI2_EN | MIPI_SRC_SEL_3;  // for customer's PCB
            //#else
            #if VIPER
            pMIPI->MIPI_DATA0_CFG[vid] = ((pMIPI->MIPI_DATA0_CFG[vid] & ~MIPI_DAT_SRC_SEL_MASK) | MIPI_DAT_SRC_PHY_0) | MIPI_CSI2_EN;
            #else
		    pMIPI->MIPI_DATA0_CFG[vid] = ((pMIPI->MIPI_DATA0_CFG[vid] & ~MIPI_DAT_SRC_SEL_MASK) | MIPI_DAT_SRC_PHY_1) | MIPI_CSI2_EN;
            #endif
            //#endif
		    if (vif_if == MMPF_VIF_IF_MIPI_DUAL_01) {
                #if VIPER
                pMIPI->MIPI_DATA1_CFG[vid] = ((pMIPI->MIPI_DATA1_CFG[vid] & ~MIPI_DAT_SRC_SEL_MASK) | MIPI_DAT_SRC_PHY_1 ) | MIPI_CSI2_EN;
                #else
			    pMIPI->MIPI_DATA1_CFG[vid] = ((pMIPI->MIPI_DATA1_CFG[vid] & ~MIPI_DAT_SRC_SEL_MASK) | MIPI_DAT_SRC_PHY_2 ) | MIPI_CSI2_EN;
                #endif
			}
			#if (CHIP == P_V2)
			else if(vif_if==ISP_HDM_VIF_IF_MIPI_QUAD){
			    pMIPI->MIPI_DATA2_CFG = MIPI_CSI2_EN | MIPI_SRC_SEL_2;
			    pMIPI->MIPI_DATA3_CFG = MIPI_CSI2_EN | MIPI_SRC_SEL_3;
			    pMIPI->MIPI_DATA4_CFG = MIPI_CSI2_EN | MIPI_SRC_SEL_4;
			}
			#endif
		}
		
	}


	return MMP_ERR_NONE;
}

//ISP_UINT32 sensorOTPvalue;
ISP_UINT8  sensorOTPvalue[4];

void SNR_IMX291_InitSensor_Customer(void);
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_InitializeISP
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_InitializeISP_IMX291(void)
{
    #if (CHIP == P_V2)
	volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;
	#endif

/*
{
	{
	
		AITPS_VIF   pVIF = AITC_BASE_VIF;
		MMP_UBYTE   vid = 0;

		MMPF_VIF_SetPIODir(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_TRUE);

		MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_TRUE);  //Pin output high
		MMPF_OS_Sleep_MS(10);

	    MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_SEN, MMP_FALSE);//Pin output low
		MMPF_OS_Sleep_MS(5);

//		pVIF->VIF_SENSR_CLK_CTL[vid] = VIF_SENSR_CLK_EN;
		MMPF_OS_Sleep_MS(5);

		MMPF_VIF_SetPIODir(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
		//MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
		//MMPF_OS_Sleep_MS(5);
		MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_FALSE);
		MMPF_OS_Sleep_MS(5);

		MMPF_VIF_SetPIOOutput(MMPF_VIF_MDL_ID0, VIF_SIF_RST, MMP_TRUE);
		MMPF_OS_Sleep_MS(20);
	}

	//SNR_IMX291_InitSensor_Customer();
	{
		ISP_HDM_IF_SNR_ConfigI2C( 0x36, ISP_I2C_TYPE_2A1D);

		// initialize VIF OPRs
	#if (SENSOR_IF == SENSOR_IF_PARALLEL)	
		ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_PARALLEL, 22);
	#elif (SENSOR_IF == SENSOR_IF_MIPI_1_LANE)	
		ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_SINGLE_0, 22);
	#elif (SENSOR_IF == SENSOR_IF_MIPI_2_LANE)		
		//ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_DUAL_01, 22);
	#elif (SENSOR_IF == SENSOR_IF_MIPI_4_LANE)		
		ISP_HDM_IF_InitVIF(ISP_HDM_VIF_IF_MIPI_QUAD, 22);
	#endif	

		MMPF_OS_Sleep_MS(1);

	}
	
	MMPF_Sensor_SetSensorReg(0x3008, 0x82);
	MMPF_OS_Sleep_MS(1);
}

*/
	// register sensor (functions and variables)
	RTNA_DBG_Str(0, "ISP_IF_LIB_Init...\r\n");
	ISP_IF_LIB_Init();
	
	// check if ISP Lib and IQ are mismatched.
	if (ISP_IF_LIB_CompareIQVer() != MMP_ERR_NONE) {
		RTNA_DBG_Str(0, "Wrong ISP lib version!\r\n");
	    return MMP_ERR_NONE;
	}

	//RTNA_DBG_Str(0, "ISP_IF_3A_Register...\r\n");
	//ISP_IF_3A_Register();

	RTNA_DBG_Str(0, "ISP_IF_3A_Init...\r\n");
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
//  Function    : MMPF_Sensor_SetAFPosition_IMX291
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR MMPF_Sensor_SetAFPosition_IMX291(MMP_USHORT ubPos)
{
#if SUPPORT_AUTO_FOCUS	
	//if (gISPConfig.AutoFocusMode == VENUS_AF_MODE_MANUAL) {
		//gAFConfig.AFPos = ubPos;
		//ISP_IF_AF_SetPos(ubPos);
	//}
	//ISP_HDM_IF_MOTOR_WriteI2C(0x05,ubPos * 2);
			ISP_IF_AF_SetPos(ubPos, 8);
			MOTOR_IMX291_VCM_SetAFPosition_Customer(ISP_IF_AF_GetPos(0));
            RTNA_DBG_Str(3, "  ubPos = ");
            RTNA_DBG_Short(3, ubPos);
            RTNA_DBG_Str(3, "\r\n");
#endif
	return  MMP_ERR_NONE;
}
#define FrameDivsor 3
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Do3AOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAWBOperation_IMX291(void)
{
	static ISP_UINT32 increase_excute = 12;
	if(increase_excute > 0)
	{
		if(increase_excute == 12) {ISP_IF_AWB_SetFastMode(1);}
		increase_excute--;
		ISP_IF_AWB_Execute();
		ISP_IF_IQ_SetAWBGains(ISP_IF_AWB_GetGainR(), ISP_IF_AWB_GetGainG(), ISP_IF_AWB_GetGainB(), ISP_IF_AWB_GetGainBase());				
		if(increase_excute == 1) {ISP_IF_AWB_SetFastMode(0);}

		return MMP_ERR_NONE;
	}	


	switch (gISPFrameCnt % FrameDivsor){
	case 1:
		ISP_IF_AWB_Execute();
		ISP_IF_IQ_SetAWBGains(ISP_IF_AWB_GetGainR(), ISP_IF_AWB_GetGainG(), ISP_IF_AWB_GetGainB(), ISP_IF_AWB_GetGainBase());		
		break;
	case 2:		
		ISP_IF_CALI_Execute();
		break;
	}

	return  MMP_ERR_NONE;
}

static ISP_UINT32 dgain, s_gain;
#define MAX_SENSOR_GAIN 4096
#define ISP_DGAIN_BASE 0x200
static ISP_UINT32 isp_gain = ISP_DGAIN_BASE;
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_ST
//  Description :
//------------------------------------------------------------------------------
extern MMP_SHORT do_release_mjpeg_image ;

static MMP_ERR  MMPF_Sensor_DoAEOperation_ST_IMX291(void)
{

    if(ISP_IF_3A_GetSwitch(ISP_3A_ALGO_AE) != 1) return MMP_ERR_NONE;
	
    if(do_release_mjpeg_image>0) 
        return 0;

   	switch (gISPFrameCnt % FrameDivsor) {
	case 0:
		ISP_IF_AE_Execute();
		
		s_gain = ISP_MAX(ISP_IF_AE_GetGain(), ISP_IF_AE_GetGainBase());

		if (s_gain >= ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN) {
			dgain = ISP_DGAIN_BASE * s_gain / ISP_IF_AE_GetGainBase() / MAX_SENSOR_GAIN;
			s_gain = ISP_IF_AE_GetGainBase() * MAX_SENSOR_GAIN;
		} else {
			dgain = ISP_DGAIN_BASE;
		}
//dbg_printf(0,"%d %d %d %d %x %x %x\r\n",ISP_IF_AE_GetVsync(), ISP_IF_AE_GetVsyncBase(), ISP_IF_AE_GetShutter(), ISP_IF_AE_GetShutterBase(), s_gain, ISP_IF_AE_GetDbgData(0), ISP_IF_AE_GetDbgData(1));

//dbg_printf(0,"%d %d\r\n",ISP_IF_AE_GetFPS(), ISP_IF_AE_GetRealFPS());
//dbg_printf(0,"%x %x %x\r\n",ISP_IF_AWB_GetGainR(), ISP_IF_AWB_GetGainG(), ISP_IF_AWB_GetGainB());
		gsSensorFunction->MMPF_Sensor_SetSensorShutter(0, 0);
		gsSensorFunction->MMPF_Sensor_SetSensorGain(s_gain);
		break;	
	}

	return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAEOperation_END
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAEOperation_END_IMX291(void)
{
	if(do_release_mjpeg_image>0){ gISPFrameCnt++;  return 0; }
	
	//RTNA_DBG_Str(0,"x");
	gISPFrameCnt++;
    if(ISP_IF_3A_GetSwitch(ISP_3A_ALGO_AE) != 1) return MMP_ERR_NONE;
	
		ISP_IF_AE_GetHWAcc(1);
		ISP_IF_AWB_GetHWAcc(1);
   	switch (gISPFrameCnt % FrameDivsor) {
		case 1:
			ISP_IF_IQ_SetAEGain(isp_gain, ISP_DGAIN_BASE);
		break;
	}
	return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_Sensor_DoAFOperation_IMX291(void)
{
#if SUPPORT_AUTO_FOCUS	
	if(gbAutoFocus == MMP_TRUE){

	    ISP_IF_AF_GetHWAcc(0);
		ISP_IF_AF_Execute();
		SetMotorPos(ISP_IF_AF_GetPos((MMP_UBYTE)0));
	}
    //RTNA_DBG_Str(0, "Do3AOperation_IMX291()\r\n");
#endif
	return  MMP_ERR_NONE;
}

/*
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoAFOperation_FrameStart
//  Description :
//------------------------------------------------------------------------------

static MMP_ERR  MMPF_Sensor_DoAFOperation_FrameStart_IMX291(void)
{
#if SUPPORT_AUTO_FOCUS
	if(gbAutoFocus == MMP_TRUE){
		//VR_AF_GetAcc();
		//ISP_IF_R_GetAFAcc();
		//ISP_IF_R_DoAF();
		{
			static ISP_UINT32 frame_cnt = 0;

			switch (frame_cnt++ % 2) {
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
*/

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_DoIQOperation_IMX291
//  Description :
//------------------------------------------------------------------------------
extern H264_FORMAT_TYPE gbCurH264Type;
static MMP_ERR  MMPF_Sensor_DoIQOperation_IMX291(void)
{
	// switch IQ table if video format is H264
	//PCAM_USB_VIDEO_FORMAT vidFmt = pcam_get_info()->pCamVideoFormat;
    if(do_release_mjpeg_image>0 && do_release_mjpeg_image<5){ //Gason@20130110, force to do IQ.		
        ISP_IF_IQ_SetAll();
		ISP_IF_IQ_CheckBypass(); 		
		
		return MMP_ERR_NONE;
	}
	
	switch (gISPFrameCnt % 2) {
	case 0:
		break;

	case 1:
		// set iq (NR, Edge, CCM, Gamma, etc.) and functions (saturation, contrast, sharpness, hue, etc.)
		ISP_IF_IQ_SetAll();
		ISP_IF_IQ_CheckBypass();
		break;
	} 
    return  MMP_ERR_NONE;
}

// Steven ADD
//------------------------------------------------------------------------------
//  Function    : MMPF_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR  MMPF_SetLightFreq_IMX291(MMP_USHORT usMode)
{

	RTNA_DBG_Str(0, "MMPF_SetLightFreq_IMX291 : ");
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

static MMP_ERR  MMPF_SetStableState_IMX291(MMP_BOOL bStableState)
{
	return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SetHistEQState_IMX291(MMP_BOOL bHistEQState)
{

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetReg_IMX291(MMP_USHORT usAddr, MMP_USHORT usData)
{
	//MMPF_I2cm_WriteReg(usAddr, usData);
	//ISP_HDM_IF_SNR_WriteI2C(usAddr,usData);
	MMPF_I2cm_WriteReg(&m_snr_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_GetReg_IMX291(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	*usData = 0xFF;
	//MMPF_I2cm_ReadReg(usAddr, usData);
	//*usData = ISP_HDM_IF_SNR_ReadI2C(usAddr) ;
    MMPF_I2cm_ReadReg(&m_snr_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}


static MMP_ERR  MMPF_Sensor_SetImageScene_IMX291(MMP_USHORT imgSceneID)
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
    #if 0
	imgSceneID = (imgSceneID % (ISP_SCENE_NONE+1));

	RTNA_DBG_Str(3, "MMPF_SetImageScene_IMX291 : ");
	RTNA_DBG_Byte(3, imgSceneID);
	RTNA_DBG_Str(3, "\r\n");

	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
		MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetScene(imgSceneID);
	#endif

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAEMode_IMX291(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)
{
	if (ubAEMode == 3)
		ISP_IF_AE_SetISO(0);
	else
		ISP_IF_AE_SetISO(ubISOMode);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAFMode_IMX291(MMP_UBYTE ubAFMode)
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
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAFMode_IMX291 : ");
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
static MMP_ERR  MMPF_Sensor_SetImageEffect_IMX291(MMP_USHORT imgEffectID)
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

	RTNA_DBG_Str(3, "MMPF_Sensor_SetImageEffect_IMX291 : ");
	RTNA_DBG_Byte(3, imgEffectID);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ImageEffect = (MMP_USHORT)imgEffectID;
	//if (g3AConfig.Control3A == ISP_3A_ENABLE)
	//	MMPF_Sensor_CheckFrameEnd(1);
	ISP_IF_F_SetImageEffect(imgEffectID);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetAWBType_IMX291(MMP_UBYTE ubType)
{
#if 0
	RTNA_DBG_Str(3, "MMPF_Sensor_SetAWBType_IMX291 : ");
	RTNA_DBG_Byte(3, ubType);
	RTNA_DBG_Str(3, "\r\n");

	cAWBtype = ubType;
#endif
	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetContrast_IMX291(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_Contrast_IMX291 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.ContrastLevel = ubLevel;
	ISP_IF_F_SetContrast(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSaturation_IMX291(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSaturation_IMX291 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SaturationLevel = ubLevel;
	ISP_IF_F_SetSaturation(ubLevel);

	return  MMP_ERR_NONE;
}
static MMP_ERR  MMPF_Sensor_SetSharpness_IMX291(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetSharpness_IMX291 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	//gISPConfig.SharpnessLevel = ubLevel;
	ISP_IF_F_SetSharpness(ubLevel);
	//ISP_IF_F_SetCISharpness(ubLevel);
	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetHue_IMX291(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetHue_IMX291 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetHue(ubLevel);

	return  MMP_ERR_NONE;
}

static MMP_ERR  MMPF_Sensor_SetGamma_IMX291(MMP_SHORT ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetGamma_IMX291 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

	ISP_IF_F_SetGamma(ubLevel);

	return  MMP_ERR_NONE;
}

#if 0
static MMP_ERR  MMPF_Sensor_SetBacklight_IMX291(MMP_UBYTE ubLevel)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetBacklight_IMX291 : ");
	RTNA_DBG_Byte(3, ubLevel);
	RTNA_DBG_Str(3, "\r\n");

//	ISP_IF_F_SetBacklight(ubLevel);
	if(ubLevel > 1){
		ISP_IF_F_SetWDR(0);
	}
	else{
		ISP_IF_F_SetWDR(255);
	}

	return  MMP_ERR_NONE;
}
#endif

static void  MMPF_Sensor_SetCaptureISPSetting_IMX291(MMP_UBYTE usCaptureBegin)
{
	AITPS_VIF   pVIF = AITC_BASE_VIF;
	MMP_ULONG   ulVIF_Width, ulVIF_Height;
	MMP_UBYTE   vid = 0;

	/* ToDo: setting IOS and ISP windows*/
	if(MMP_TRUE == usCaptureBegin){
		ulVIF_Width = (pVIF->VIF_GRAB[vid].PIXL_ED - pVIF->VIF_GRAB[vid].PIXL_ST + 1);
		ulVIF_Height = (pVIF->VIF_GRAB[vid].LINE_ED - pVIF->VIF_GRAB[vid].LINE_ST + 1);
		//gSensorFunc.SetISPWindows(ulVIF_Width,ulVIF_Height);
	}
	
	return;
}

/*Prepare for raw preview zoom*/
static void  MMPF_Sensor_SetISPWindow_IMX291(MMP_ULONG ulWidth, MMP_ULONG ulHeight)
{
	//gSensorFunc.SetISPWindows(ulWidth,ulHeight);
	return;
}

/*Check AIT can preview in this resolution*/
static MMP_UBYTE  MMPF_Sensor_CheckPreviewAbility_IMX291(MMP_USHORT usPreviewmode)
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

static void SetAFWin_IMX291(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
					MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight)
{

	//DBG_S3("SetAFWin (TODO)\r\n");
}

//static MMP_BOOL VGAPreview = MMP_TRUE;
static void SetAFEnable_IMX291(MMP_UBYTE enable)
{
#if SUPPORT_AUTO_FOCUS //&& (AF_EN!=255)
//    if (enable) {
		//RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_START\r\n");
		ISP_IF_AF_Control(ISP_AF_START);
		//actually calling SNR_IMX291_SetAutoFocusControl()
//    } else {
//        RTNA_DBG_Str(3, "VENUS_ISP_AUTO_FOCUS_STATUS_STOP\r\n");
//        ISP_IF_AF_Control(ISP_AF_STOP);
//    }
#endif
}

static MMP_ERR MMPF_Sensor_SetAEEnable_IMX291(MMP_UBYTE bEnable)
{
	/*
	if (bEnable) {
		g3AConfig.AE_Enabled     = ON;
	}
	else {
		g3AConfig.AE_Enabled     = OFF;
	}*/

	//ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AE, bEnable);

	return MMP_ERR_NONE;
}

static MMP_ERR MMPF_Sensor_SetAWBEnable_IMX291(MMP_UBYTE bEnable)
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
static void SetAEsmooth_IMX291(MMP_UBYTE smooth)  {return;}
static MMP_ERR MMPF_Sensor_SetExposureValue_IMX291(MMP_UBYTE ev)
{
	RTNA_DBG_Str(3, "MMPF_Sensor_SetExposureValue_IMX291 : ");
	//RTNA_DBG_Byte(3, gISPConfig.EV);
	//RTNA_DBG_Str(3, " : ");
	RTNA_DBG_Byte(3, ev);
	RTNA_DBG_Str(3, "\r\n");

	//ISP_IF_AE_SetEV(ev);

	return MMP_ERR_NONE;
}

static MMP_UBYTE GetAFPosition_IMX291     (void){ return 0;/*volatile MMP_UBYTE *GPIO_BASE_B = (volatile MMP_UBYTE *)0x80009400;  return GPIO_BASE_B[0xB0];*/}


static MMP_UBYTE GetAFEnable_IMX291(void)
{
	//return 0;
	return ISP_IF_AF_GetResult();
}

static MMP_UBYTE GetExposureValue_IMX291  (void){return ISP_IF_AE_GetEV();}
static MMP_UBYTE GetAEsmooth_IMX291       (void){return 0;}
void  MMPF_Sensor_SetColorID(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_Set3AStatus
//  Description :
//------------------------------------------------------------------------------
static MMP_ERR	MMPF_Sensor_Set3AStatus_IMX291(MMP_BOOL bEnable)
{
    m_bISP3AStatus = bEnable;
    dbg_printf(0, "enable 3A:%d!!!!\r\n", bEnable);
	//MMPF_Sensor_3A_SetInterrupt(bEnable);
	
	if (bEnable)
		ISP_IF_3A_Control(ISP_3A_ENABLE);
	else
		ISP_IF_3A_Control(ISP_3A_DISABLE);
	
	return	MMP_ERR_NONE;
}

static void  MMPF_Sensor_SetColorID_IMX291(MMPF_SENSOR_ROTATE_TYPE RotateType)
{
	ISP_IF_IQ_SetDirection((ISP_UINT8)RotateType);
	return;
}

const ISP_UINT32 imx322_GainTable[241] = {
	256,
	265,
	274,
	284,
	294,
	304,
	315,
	326,
	338,
	350,
	362,
	375,
	388,
	402,
	416,
	431,
	446,
	461,
	478,
	495,
	512,
	530,
	549,
	568,
	588,
	609,
	630,
	653,
	676,
	699,
	724,
	750,
	776,
	803,
	832,
	861,
	891,
	923,
	955,
	989,
	1024,
	1060,
	1097,
	1136,
	1176,
	1218,
	1261,
	1305,
	1351,
	1399,
	1448,
	1499,
	1552,
	1607,
	1663,
	1722,
	1783,
	1846,
	1911,
	1978,
	2048,
	2120,
	2195,
	2272,
	2353,
	2435,
	2521,
	2610,
	2702,
	2798,
	2896,
	2998,
	3104,
	3214,
	3327,
	3444,
	3566,
	3692,
	3822,
	3956,
	4096,
	4240,
	4390,
	4545,
	4705,
	4871,
	5043,
	5221,
	5405,
	5595,
	5793,
	5997,
	6208,
	6427,
	6654,
	6889,
	7132,
	7383,
	7643,
	7913,
	8192,
	8481,
	8780,
	9090,
	9410,
	9742,
	10086,
	10441,
	10809,
	11191,
	11585,
	11994,
	12417,
	12855,
	13308,
	13777,
	14263,
	14766,
	15287,
	15826,
	16384,
	16962,
	17560,
	18179,
	18820,
	19484,
	20171,
	20882,
	21619,
	22381,
	23170,
	23988,
	24834,
	25709,
	26616,
	27554,
	28526,
	29532,
	30574,
	31652,
	32768,
	33924,
	35120,
	36358,
	37641,
	38968,
	40342,
	41765,
	43238,
	44762,
	46341,
	47975,
	49667,
	51419,
	53232,
	55109,
	57052,
	59064,
	61147,
	63304,
	65536,
	67847,
	70240,
	72717,
	75281,
	77936,
	80684,
	83530,
	86475,
	89525,
	92682,
	95950,
	99334,
	102837,
	106464,
	110218,
	114105,
	118129,
	122295,
	126607,
	131072,
	135694,
	140480,
	145434,
	150562,
	155872,
	161369,
	167059,
	172951,
	179050,
	185364,
	191901,
	198668,
	205674,
	212927,
	220436,
	228210,
	236257,
	244589,
	253214,
	262144,
	271389,
	280959,
	290867,
	301124,
	311744,
	322737,
	334118,
	345901,
	358099,
	370728,
	383801,
	397336,
	411348,
	425854,
	440872,
	456419,
	472515,
	489178,
	506429,
	524288,
	542777,
	561918,
	581734,
	602249,
	623487,
	645474,
	668237,
	691802,
	716199,
	741455,
	767603,
	794672,
	822696,
	851708,
	881744,
	912839,
	945030,
	978356,
	1012858,
	1048576
};

void MMPF_Sensor_SetSensorGain_IMX291(MMP_ULONG ulGain)
{
	ISP_UINT8 i, sensor_gain;
	
	for( i = 0; i< 241; i++)
	{
		if (ulGain >= imx322_GainTable[i])
			sensor_gain = i;
		else
			break;
	}

	isp_gain = dgain * ulGain / imx322_GainTable[sensor_gain];

	MMPF_Sensor_SetSensorReg(0x3014, sensor_gain & 0xff);
}

void MMPF_Sensor_SetSensorShutter_IMX291(MMP_ULONG shutter, MMP_ULONG vsync)
{
	ISP_UINT32 new_vsync = g_SNR_LineCntPerSec * ISP_IF_AE_GetVsync() / ISP_IF_AE_GetVsyncBase();
	ISP_UINT32 new_shutter = g_SNR_LineCntPerSec * ISP_IF_AE_GetShutter() / ISP_IF_AE_GetShutterBase();

	// ***************g_SNR_LineCntPerSec = 1 sec hsync.***********

    new_vsync 	= ISP_MIN(ISP_MAX(new_shutter, new_vsync), 0xFFFF);
	new_shutter = ISP_MIN(ISP_MAX(new_shutter, 1), (new_vsync - 2));
		
    if(m_VideoFmt==MMPS_3GPRECD_VIDEO_FORMAT_RAW) {
	    return ;
	}

	MMPF_Sensor_SetSensorReg( 0x3018, ((( new_vsync ) >> 0 ) & 0xFF));
	MMPF_Sensor_SetSensorReg( 0x3019, ((( new_vsync ) >> 8 ) & 0xFF));
	MMPF_Sensor_SetSensorReg( 0x301A, ((( new_vsync ) >> 16 ) & 0xFF));

	MMPF_Sensor_SetSensorReg( 0x3020, ((( new_vsync - (new_shutter+1)) >> 0  ) & 0xFF));
	MMPF_Sensor_SetSensorReg( 0x3021, ((( new_vsync - (new_shutter+1)) >> 8  ) & 0xFF));
	MMPF_Sensor_SetSensorReg( 0x3022, ((( new_vsync - (new_shutter+1)) >> 16 ) & 0xFF));
}
MMPF_SENSOR_FUNCTION  SensorFunction_IMX291 =
{
	MMPF_Sensor_Initialize_IMX291,
	MMPF_Sensor_InitializeVIF_IMX291,
	MMPF_Sensor_InitializeISP_IMX291,
	MMPF_Sensor_PowerDown_IMX291,
	MMPF_Sensor_SetFrameRate_IMX291,
	MMPF_Sensor_ChangeMode_IMX291,
	MMPF_Sensor_ChangePreviewMode_IMX291,
	MMPF_Sensor_SetPreviewMode_IMX291,
	MMPF_Sensor_SetReg_IMX291,
	MMPF_Sensor_GetReg_IMX291,
	MMPF_Sensor_DoAWBOperation_IMX291,
	MMPF_Sensor_DoAEOperation_ST_IMX291,
	MMPF_Sensor_DoAEOperation_END_IMX291,
	MMPF_Sensor_DoAFOperation_IMX291,
	//MMPF_Sensor_DoAFOperation_FrameStart_IMX291,
	MMPF_Sensor_DoIQOperation_IMX291,
	MMPF_Sensor_SetImageEffect_IMX291,
	MMPF_SetLightFreq_IMX291,
	MMPF_SetStableState_IMX291,
	MMPF_SetHistEQState_IMX291,
	MMPF_Sensor_SetAFPosition_IMX291,
	SetAFWin_IMX291,
	SetAFEnable_IMX291,
	MMPF_Sensor_SetAEEnable_IMX291,
	MMPF_Sensor_SetAWBEnable_IMX291,
	MMPF_Sensor_SetExposureValue_IMX291,
	SetAEsmooth_IMX291,
	MMPF_Sensor_SetImageScene_IMX291,
	GetAFPosition_IMX291,
	GetAFEnable_IMX291,
	GetExposureValue_IMX291,
	GetAEsmooth_IMX291,
	MMPF_Sensor_SetAWBType_IMX291,
	MMPF_Sensor_SetContrast_IMX291,
	MMPF_Sensor_SetSaturation_IMX291,
	MMPF_Sensor_SetSharpness_IMX291,
	MMPF_Sensor_SetHue_IMX291,
	MMPF_Sensor_SetGamma_IMX291,
 //   MMPF_Sensor_SetBacklight_IMX291,
	MMPF_Sensor_SetAEMode_IMX291,
	MMPF_Sensor_SetAFMode_IMX291,
	MMPF_Sensor_SetCaptureISPSetting_IMX291,
	MMPF_Sensor_SetISPWindow_IMX291,
	MMPF_Sensor_CheckPreviewAbility_IMX291,
	MMPF_Sensor_Set3AStatus_IMX291,
	MMPF_Sensor_SetColorID_IMX291,
	MMPF_Sensor_SetSensorGain_IMX291,
	MMPF_Sensor_SetSensorShutter_IMX291
};

MMP_USHORT SNR_IMX291_Reg_Init_Customer[] = {

	0x3003, 0x01,



};

void SNR_IMX291_InitSensor_Customer(void) 
{
	// implement your initialize sensor routine here ==============================================
	ISP_UINT32 i;

    RTNA_DBG_Str3("SNR_IMX291_InitSensor_Customer\r\n");

	//MMPF_Sensor_SetSensorReg(0x3008, 0x82);


	// set register...........
	for (i = 0; i < ISP_ARRSIZE(SNR_IMX291_Reg_Init_Customer)/2; i++) {
		MMPF_Sensor_SetSensorReg(SNR_IMX291_Reg_Init_Customer[i*2], SNR_IMX291_Reg_Init_Customer[i*2+1]);
	}
	MMPF_OS_Sleep_MS(10);
//	ISP_IF_AE_SetBaseShutter(336, 280);

	// Set preview resolution as default
	//SNR_IMX291_SetSensorResolution_Customer(ISP_SENSOR_RESOL_1920x1080);
}

MMP_USHORT SNR_IMX291_Reg_1920x1080_Customer[] = {

	0x3000, 0x01,
	0x3002, 0x00,
	0x3005, 0x01,
	0x3007, 0x00,
	0x3009, 0x12,
	0x300a, 0xf0,
	0x300f, 0x00,
	0x3010, 0x21,
	0x3012, 0x64,
	0x3016, 0x09,
	0x3018, 0x70,	//0x65,	//0x70
	0x3019, 0x04,
	0x301c, 0x30,  // Horizontal clock count 0x301D~0x301C: 0x1130 (30 FPS), 0x11C8 (29 FPS)
	0x301d, 0x11,
	0x3046, 0x01,
	0x304b, 0x0a,
	0x305c, 0x18,
	0x305d, 0x03,
	0x305e, 0x20,
	0x305f, 0x01,
	0x3070, 0x02,
	0x3071, 0x11,
	0x309b, 0x10,
	0x309c, 0x22,
	0x30a2, 0x02,
	0x30a6, 0x20,
	0x30a8, 0x20,
	0x30aa, 0x20,
	0x30ac, 0x20,
	0x30b0, 0x43,
	0x3119, 0x9e,
	0x311c, 0x1e,
	0x311e, 0x08,
	0x3128, 0x05,
	0x3129, 0x00,
	0x313d, 0x83,
	0x3150, 0x03,
	0x315e, 0x1a,
	0x3164, 0x1a,
	0x317c, 0x00,
	0x317e, 0x00,
	0x31ec, 0x0e,
	0x32b8, 0x50,
	0x32b9, 0x10,
	0x32ba, 0x00,
	0x32bb, 0x04,
	0x32c8, 0x50,
	0x32c9, 0x10,
	0x32ca, 0x00,
	0x32cb, 0x04,
//	0x3304, 0x22,
	0x332c, 0xd3,
	0x332d, 0x10,
	0x332e, 0x0d,
	0x3358, 0x06,
	0x3359, 0xe1,
	0x335a, 0x11,
	0x3360, 0x1e,
	0x3361, 0x61,
	0x3362, 0x10,
	0x33b0, 0x50,
	0x33b2, 0x1a,
	0x33b3, 0x04,
	0x3405, 0x10,
	0x3407, 0x01,//03,  //1 for 2lane; 3 for 4lane
	0x3414, 0x0a,
	0x3418, 0x49,
	0x3419, 0x04,
	0x3441, 0x0c,
	0x3442, 0x0c,
	0x3443, 0x01,//03,  //1 for 2lane; 3 for 4lane
	0x3444, 0x20,
	0x3445, 0x25,
	0x3446, 0x57,
	0x3447, 0x00,
	0x3448, 0x37,
	0x3449, 0x00,
	0x344a, 0x1f,
	0x344b, 0x00,
	0x344c, 0x1f,
	0x344d, 0x00,
	0x344e, 0x1f,
	0x344f, 0x00,
	0x3450, 0x77,
	0x3451, 0x00,
	0x3452, 0x1f,
	0x3453, 0x00,
	0x3454, 0x17,
	0x3455, 0x00,
	0x3472, 0x9c,
	0x3473, 0x07,
	0x3480, 0x49,

	0x3000, 0x00,
	0x0100,	0x01
};

extern void MMPF_Sensor_DoMirrorFlip(void);
void SNR_IMX291_SetSensorResolution_Customer(ISP_SENSOR_RESOL res)
{
	// implement your change sensor resolution routine here =======================================

	MMP_ULONG i, VIFGrab_H_Start, VIFGrab_H_Length, VIFGrab_V_Start, VIFGrab_V_Length;


	switch (res) {

	case ISP_SENSOR_RESOL_1920x1080:
		RTNA_DBG_Str3("---------------ISP_SENSOR_RESOL_1920x1080-------------\r\n");
		// set register...........
		for (i = 0; i < ISP_ARRSIZE(SNR_IMX291_Reg_1920x1080_Customer)/2; i++) {
			MMPF_Sensor_SetSensorReg(SNR_IMX291_Reg_1920x1080_Customer[i*2], SNR_IMX291_Reg_1920x1080_Customer[i*2+1]);
            //delay_1us(100); 
		}

		// set vif grab range which is fetched to ISP (for lens shading and 3A window calculation)
		VIFGrab_H_Start     = 1 ;
		VIFGrab_H_Length    = 1928;//1288;
		VIFGrab_V_Start     = 16  ;
		VIFGrab_V_Length    = 1092;//730;

		{
			ISP_UINT32 base_x, base_y;
			ISP_UINT32 x_scale_n = 0x200;
			ISP_UINT32 y_scale_n = 0x200;
			ISP_IF_IQ_GetCaliBase(&base_x, &base_y);
			
			x_scale_n = y_scale_n = VIFGrab_H_Length * 0x200 / base_x;

			
			ISP_IF_IQ_SetCaliRatio(	x_scale_n,	//x_scale_n, 
									0x200,		//x_scale_m, 
									0,			//x_offset, 
									y_scale_n,	//y_scale_n, 
									0x200,		//y_scale_m, 
									0);			//y_offset
		}

		g_SNR_LineCntPerSec = 1141*30; //33600;//1120 * 30;    //???? 1sec H_s
//		g_SNR_LineCntPerSec = 70080; //33600;//1120 * 30;    //???? 1sec H_s
		//ISP_IF_AE_SetMaxSensorFPSx10(30 * 10);
		ISP_IF_AE_SetMaxSensorFPSx10(30 * 10);

		break;

	}

	ISP_IF_IQ_SetISPInputLength(VIFGrab_H_Length, VIFGrab_V_Length);
	ISP_IF_IQ_SetColorID(0);
	//MMPF_Sensor_SetColorID
    //MMPF_Sensor_DoMirrorFlip();
	
//	ISP_IF_AE_SetShutter(g_SNR_LineCntPerSec, g_SNR_LineCntPerSec);
	ISP_IF_AE_SetShutterBase(g_SNR_LineCntPerSec);

	// end of your implementation =================================================================

	// set VIF OPR (Actual sensor output size)
	{
		AITPS_VIF  pVIF = AITC_BASE_VIF;
        #if (CHIP == P_V2)
		volatile MMP_BYTE* pISP = (volatile MMP_BYTE*)AITC_BASE_ISP;
		#endif
		MMP_UBYTE   vid = 0;
		MMPF_VIF_GRAB_INFO  VifGrab;

		VifGrab.usStartX    = VIFGrab_H_Start;
		VifGrab.usGrabWidth = VIFGrab_H_Length;
		VifGrab.usStartY    = VIFGrab_V_Start;
		VifGrab.usGrabHeight = VIFGrab_V_Length;
		MMPF_VIF_SetGrabRange(vid, &VifGrab);

		pVIF->VIF_INT_LINE_NUM_0[vid] = VIFGrab_V_Length - 61;//121;

        #if (CHIP == P_V2)
		if (m_gsISPCoreID == 868) {
			// ISP pipeline selection
			//APICAL_XBYTE[APICAL_CTL_REG1] &= ~(0x80); // clear auto size
			pISP[0x0B] = 0x30;
			pISP[0x09] = 0x0C;

			// IP ColorID setting
			//pISP[0x09] &= ~(0xF0);
			//pISP[0x09] |= (ISP_IF_SNR_GetColorID() == 0 ? 3 : (ISP_IF_SNR_GetColorID() == 1 ? 2 : (ISP_IF_SNR_GetColorID() == 2 ? 1: 0))) * (0x50);
		}
		#endif
    ISP_IF_IQ_SetAll();
	// SetBeforePreview and SetBeforeCapture should be called after getting new configurations of the current resolution
	if (gSensorISPMode  == MMPF_SENSOR_SNAPSHOT) {
		// update AE before capture
		//ISP_IF_AE_UpdateBeforeCapture();

		// update AWB before capture
		//ISP_IF_AWB_UpdateBeforeCapture();

		// special settings for capture could be set here =============================================

		// end of special settings ====================================================================
	} else if (gSensorISPMode == MMPF_SENSOR_PREVIEW) {
		// update AE before preview
		//ISP_IF_AE_UpdateBeforePreview();

		//// update AWB before preview
		//ISP_IF_AWB_UpdateBeforePreview();
	}

	// set direction (color ID, orientation, etc.)
	ISP_IF_IQ_SetDirection(ISP_IQ_DIRECTION_ORIGINAL);

	// set exposure parameters

	{	
		//ISP_IF_IQ_SetAEGain(ISP_IF_AE_GetDGain(),ISP_IF_AE_GetGainBase());
		//gsSensorFunction->MMPF_Sensor_SetSensorGain(ISP_IF_AE_GetGain());
		//gsSensorFunction->MMPF_Sensor_SetSensorShutter(0, 0);
	}
	// update ISP window
	ISP_IF_IQ_UpdateInputSize();

	ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AE, 1);
    ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AWB, 1);
    ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AF, 1);	
	// check if modules are bypassed
	ISP_IF_IQ_CheckBypass();

	// force update iq to hardware
//	ISP_IF_IQ_UpdateOprtoHW(ISP_IQ_SWITCH_R4, 1);
	

	}
}

MMP_ERR  MMPF_MOTOR_WriteI2C(MMP_USHORT usAddr, MMP_USHORT usData)
{
	//MMPF_I2cm_WriteReg(&m_motor_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}

MMP_ERR  MMPF_MOTOR_ReadI2C(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	*usData = 0xFF;
    //MMPF_I2cm_ReadReg(&m_motor_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}


#if SUPPORT_IR_LED
MMP_ERR MMPF_ICR_Control(MMP_USHORT NightMode)
{
	//MMPF_PIO_SetData(MMPF_PIO_REG_GPIO1, 0);  // Set ENB to low

    if(NightMode){
    	MMPF_PIO_SetData(MMPF_PIO_REG_GPIO7, 1);  // Set FBC to high, Night mode
    	dbg_printf(3, "ICR Night mode\r\n");
    }
    else{
    	MMPF_PIO_SetData(MMPF_PIO_REG_GPIO7, 0);  // Set FBC to low, Normal mode
    	dbg_printf(3, "ICR Normal mode\r\n");
	}

	//MMPF_PIO_SetData(MMPF_PIO_REG_GPIO1, 1);  // Set ENB to high
}

#endif

#if (SENSOR_ID_IMX291== 0)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module0 = &SensorFunction_IMX291;
#endif

#if (SENSOR_ID_IMX291== 1)
MMPF_SENSOR_FUNCTION  *SensorFunction_Module1 = &SensorFunction_IMX291;
#endif

#endif  //BIND_SENSOR_IMX291
#endif
