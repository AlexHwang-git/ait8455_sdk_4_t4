//==============================================================================
//
//  File        : mmpf_sensor.h
//  Description : INCLUDE File for the Sensor Driver Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_SENSOR_H_
#define _MMPF_SENSOR_H_

#include    "includes_fw.h"
#include "mmpf_scaler.h"

/** @addtogroup MMPF_Sensor
@{
*/
typedef enum {
	AIT_SCENE_MODE_AUTO				    = 0,
	AIT_SCENE_MODE_PORTRAIT			    = 1,
	AIT_SCENE_MODE_LANDSCAPE			= 2,
	AIT_SCENE_MODE_SPORTS				= 3,
	AIT_SCENE_MODE_SUNSET				= 4,
	AIT_SCENE_MODE_DUSK				    = 5,
	AIT_SCENE_MODE_DAWN				    = 6,
	AIT_SCENE_MODE_NIGHT_SHOT			= 7,
	AIT_SCENE_MODE_AGAINST_LIGHT		= 8,
	AIT_SCENE_MODE_TEXT				    = 9,
	AIT_SCENE_MODE_MANUAL				= 10,
	AIT_SCENE_MODE_INDOOR				= 11,
	AIT_SCENE_MODE_SNOW				    = 12,
	AIT_SCENE_MODE_FALL				    = 13,
	AIT_SCENE_MODE_WAVE				    = 14,
	AIT_SCENE_MODE_FIREWORKS			= 15,
	AIT_SCENE_MODE_SHOW_WIN			    = 16,
	AIT_SCENE_MODE_CANDLE				= 17,
	AIT_SCENE_MODE_NONE				    = 18		// camera mode
} AIT_SCENE_MODE;

/*****************************************************************************/
/* AF Mode Definition                                                        */
/*****************************************************************************/
typedef enum {
	AIT_AF_MODE_AUTO					= 0,
	AIT_AF_MODE_MANUAL				    = 1,
	AIT_AF_MODE_MACRO					= 2,
	AIT_AF_MODE_FULL					= 3,
	AIT_AF_MODE_MOTOR_TEST              = 4,
	AIT_AF_MODE_NULL					= 5
} AIT_AF_MODE;


/*****************************************************************************/
/* Color Tone                                                             */
/*****************************************************************************/
typedef enum {
	AIT_IMAGE_EFFECT_NORMAL			    = 0,
	AIT_IMAGE_EFFECT_GREY				= 1,
	AIT_IMAGE_EFFECT_SEPIA			    = 2,
	AIT_IMAGE_EFFECT_NEGATIVE			= 3,
	AIT_IMAGE_EFFECT_ANTIQUE			= 4,
	AIT_IMAGE_EFFECT_WATERCOLOR		    = 5,
	AIT_IMAGE_EFFECT_PORTRAIT			= 6,
	AIT_IMAGE_EFFECT_LANDSCAPE		    = 7,
	AIT_IMAGE_EFFECT_SUNSET			    = 8,
	AIT_IMAGE_EFFECT_DUSK				= 9,
	AIT_IMAGE_EFFECT_DAWN				= 10,
	AIT_IMAGE_EFFECT_RED				= 11,
	AIT_IMAGE_EFFECT_GREEN			    = 12,
	AIT_IMAGE_EFFECT_BLUE				= 13,
	AIT_IMAGE_EFFECT_YELLOW			    = 15,
	AIT_IMAGE_EFFECT_EMBOSS			    = 17,
	AIT_IMAGE_EFFECT_OIL				= 18,
	AIT_IMAGE_EFFECT_BW				    = 19,
	AIT_IMAGE_EFFECT_SKETCH			    = 20,
	AIT_IMAGE_EFFECT_CRAYONE			= 21,
	AIT_IMAGE_EFFECT_WHITEBOARD		    = 22,
	AIT_IMAGE_EFFECT_BLACKBOARD		    = 23,
	AIT_IMAGE_EFFECT_VIVID              = 24,
	AIT_IMAGE_EFFECT_END				= 25
} AIT_IMAGE_EFFECT;


/*****************************************************************************/
/* Exposure Level                 */
/*****************************************************************************/
typedef enum
{
    AIT_BRIGHTNESS_LEVEL_0 = 0,
    AIT_BRIGHTNESS_LEVEL_1,
    AIT_BRIGHTNESS_LEVEL_2,
    AIT_BRIGHTNESS_LEVEL_3,
    AIT_BRIGHTNESS_LEVEL_4,
    AIT_BRIGHTNESS_LEVEL_5,
    AIT_BRIGHTNESS_LEVEL_6,
    AIT_BRIGHTNESS_LEVEL_7,
    AIT_BRIGHTNESS_LEVEL_8,
    AIT_BRIGHTNESS_LEVEL_9
} AIT_BRIGHTNESS_LEVEL;



/*****************************************************************************/
/* AE ISO Definition                                                         */
/*****************************************************************************/
typedef enum {
	AIT_ISO_AUTO                = 0,
	AIT_ISO_50					= 1,
	AIT_ISO_100					= 2,
	AIT_ISO_200					= 3,
	AIT_ISO_400					= 4,
	AIT_ISO_800					= 5,
	AIT_ISO_1600                = 6,
	AIT_ISO_3200                = 7,
	AIT_ISO_END					= 8
} AIT_AE_ISO;


/*****************************************************************************/
/* AWB Mode Definition                                                       */
/*****************************************************************************/
typedef enum {
	AIT_AWB_MODE_BYPASS						= 0,
	AIT_AWB_MODE_AUTO						= 1,
	AIT_AWB_MODE_CLOUDY						= 2,
	AIT_AWB_MODE_DAYLIGHT					= 3,
	AIT_AWB_MODE_COOLWHITE					= 4,
	AIT_AWB_MODE_ALIGHT						= 5,
	AIT_AWB_MODE_FLUORSENT					= 6,
	AIT_AWB_MODE_EFFECT						= 7,
	AIT_AWB_MODE_DAWN						= 8,
	AIT_AWB_MODE_SUNSET						= 9
} AIT_AWB_MODE;

typedef enum {
    MMPF_SENSOR_RAWPATH_PREVIEW_ENABLE      = 0,
    MMPF_SENSOR_RAWPATH_STORE_DOWNSAMPLE,
    MMPF_SENSOR_RAWPATH_ZEROSHUTTERLAG_ENABLE       //Ted ZSL
} MMPF_SENSOR_PARAM;


typedef enum {
    MMPF_SENSOR_ROTATE_NO_ROTATE      = 0,
    MMPF_SENSOR_ROTATE_RIGHT_90,
    MMPF_SENSOR_ROTATE_RIGHT_180,
    MMPF_SENSOR_ROTATE_RIGHT_270,
    MMPF_SENSOR_ROTATE_H_MIRROR,
    MMPF_SENSOR_ROTATE_V_FLIP
} MMPF_SENSOR_ROTATE_TYPE;

typedef enum {
    MMPF_SENSOR_3A_RESET=0,
    MMPF_SENSOR_3A_SET
} MMPF_SENSOR_3A_STATE;

typedef enum {
	ISP_HDM_VIF_IF_PARALLEL				= 0,
	ISP_HDM_VIF_IF_MIPI_SINGLE_0		= 1,
	ISP_HDM_VIF_IF_MIPI_SINGLE_1		= 2,
	ISP_HDM_VIF_IF_MIPI_SINGLE_2		= 3,
	ISP_HDM_VIF_IF_MIPI_SINGLE_3		= 4,
	ISP_HDM_VIF_IF_MIPI_DUAL_01			= 5,
	ISP_HDM_VIF_IF_MIPI_DUAL_02			= 6,
	ISP_HDM_VIF_IF_MIPI_DUAL_03			= 7,
	ISP_HDM_VIF_IF_MIPI_DUAL_12			= 8,
	ISP_HDM_VIF_IF_MIPI_DUAL_13			= 9,
	ISP_HDM_VIF_IF_MIPI_DUAL_23			= 10,
	ISP_HDM_VIF_IF_MIPI_QUAD			= 11
} ISP_HDM_VIF_IF;

typedef enum {
	VENUS_SYSTEM_MODE_INIT				= 0,
	VENUS_SYSTEM_MODE_PREVIEW			= 1,
	VENUS_SYSTEM_MODE_SNAPSHOT			= 2,
	VENUS_SYSTEM_MODE_OTHERS			= 3
} VENUS_SYSTEM_MODE;
extern VENUS_SYSTEM_MODE gSystemMode;
#define VR_MAX(a,b)							(((a) > (b)) ? (a) : (b))
#define VR_MIN(a,b)							(((a) < (b)) ? (a) : (b))
#define VR_ARRSIZE(a)						( sizeof((a)) / sizeof((a[0])) )

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define SENSOR_DSC_MODE 	(1)
#define SENSOR_VIDEO_MODE 	(2)

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct _MMPF_SENSOR_FUNCTIION {
    MMP_ERR (*MMPF_Sensor_Initialize)(void);
    MMP_ERR (*MMPF_Sensor_InitializeVIF)(void);
    MMP_ERR (*MMPF_Sensor_InitializeISP)(void);
    MMP_ERR (*MMPF_Sensor_PowerDown)(void);
    MMP_ERR (*MMPF_Sensor_SetFrameRate)(MMP_UBYTE ubFrameRate);
    MMP_ERR (*MMPF_Sensor_ChangeMode)(MMP_USHORT usPreviewmode, MMP_USHORT usCapturemode);
    MMP_ERR (*MMPF_Sensor_ChangePreviewMode)(
                                MMP_USHORT usCurPreviewmode, MMP_USHORT usCurPhasecount,
                                MMP_USHORT usNewPreviewmode, MMP_USHORT usNewPhasecount);
    MMP_ERR (*MMPF_Sensor_SetPreviewMode)(MMP_USHORT usPreviewMode);
    MMP_ERR (*MMPF_Sensor_SetReg)(MMP_USHORT usAddr, MMP_USHORT usData);
    MMP_ERR (*MMPF_Sensor_GetReg)(MMP_USHORT usAddr, MMP_USHORT *usData);
    MMP_ERR (*MMPF_Sensor_DoAWBOperation)(void);
    MMP_ERR (*MMPF_Sensor_DoAEOperation_ST)(void);
    MMP_ERR (*MMPF_Sensor_DoAEOperation_END)(void);
    MMP_ERR (*MMPF_Sensor_DoAFOperation)(void);
    MMP_ERR (*MMPF_Sensor_DoAFOperation_FrameStart)(void);
    MMP_ERR (*MMPF_Sensor_DoIQOperation)(void);
    MMP_ERR (*MMPF_Sensor_SetImageEffect)(MMP_USHORT imgEffectID);
    MMP_ERR (*MMPF_SetLightFreq)(MMP_USHORT usMode);
    MMP_ERR (*MMPF_SetStableState)(MMP_BOOL bStableState);
	MMP_ERR (*MMPF_SetHistEQState)(MMP_BOOL bHistEQState);
    MMP_ERR (*MMPF_Sensor_SetAFPosition)	(MMP_UBYTE ubPos);	
	void (*SetAFWin)						(MMP_USHORT usIndex, MMP_USHORT usPreviewWidth, MMP_USHORT usPreviewHeight,
											MMP_USHORT  usStartX, MMP_USHORT  usStartY, MMP_USHORT  usWidth, MMP_USHORT  usHeight);
    void (*SetAFEnable)						(MMP_UBYTE enable); 		// Steven ADD
    MMP_ERR (*MMPF_Sensor_SetAEEnable)		(MMP_UBYTE ubEnable); 		// Steven ADD
    MMP_ERR (*MMPF_Sensor_SetAWBEnable)		(MMP_UBYTE enable); 		// Steven ADD
    MMP_ERR (*MMPF_Sensor_SetExposureValue)	(MMP_UBYTE ev); 			// Steven ADD
    void (*SetAEsmooth)						(MMP_UBYTE smooth);			// Steven ADD
    MMP_ERR (*MMPF_Sensor_SetImageScene)(MMP_USHORT imgSceneID); 	// Steven ADD

    MMP_UBYTE (*GetAFPosition)				(void);			// Steven ADD
    MMP_UBYTE (*GetAFEnable)				(void); 		// Steven ADD
    MMP_UBYTE (*GetExposureValue)			(void); 		// Steven ADD
    MMP_UBYTE (*GetAEsmooth)				(void);			// Steven ADD
    MMP_ERR	(*MMPF_Sensor_SetAWBType)(MMP_UBYTE ubType);
    MMP_ERR	(*MMPF_Sensor_SetContrast)(MMP_SHORT ubLevel);
    MMP_ERR	(*MMPF_Sensor_SetSaturation)(MMP_SHORT ubLevel);
    MMP_ERR	(*MMPF_Sensor_SetSharpness)(MMP_SHORT ubLevel);
    MMP_ERR	(*MMPF_Sensor_SetHue)(MMP_SHORT ubLevel);
    MMP_ERR	(*MMPF_Sensor_SetGamma)(MMP_SHORT ubLevel);
   // MMP_ERR	(*MMPF_Sensor_SetBacklight)(MMP_UBYTE ubLevel); // ooxx842
    MMP_ERR	(*MMPF_Sensor_SetAEMode)(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode);    
    MMP_ERR (*MMPF_Sensor_SetAFMode)(MMP_UBYTE ubAFMode);
	void (*MMPF_SetCaptureISPSetting)						(MMP_UBYTE usCaptureBegin);	
	void (*MMPF_Sensor_SetISPWindow)						(MMP_ULONG ulWidth, MMP_ULONG ulHeight);	
    MMP_UBYTE (*MMPF_Sensor_CheckPreviewAbility)                   (MMP_USHORT usPreviewmode);
    MMP_ERR (*MMPF_Sensor_Set3AStatus)                   (MMP_BOOL bEnable);
    void  (*MMPF_Sensor_SetColorID)(MMPF_SENSOR_ROTATE_TYPE RotateType);
    void  (*MMPF_Sensor_SetSensorGain)(MMP_ULONG gain);
    void  (*MMPF_Sensor_SetSensorShutter)(MMP_ULONG shutter, MMP_ULONG vsync);
} MMPF_SENSOR_FUNCTION;

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
extern	MMP_USHORT	gsSensorMode;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
MMP_ERR MMPF_Sensor_WaitFrame(MMP_USHORT usFrameCount);

MMP_ERR MMPF_Sensor_GetPreviewEffectNum(MMP_UBYTE *ubEffectNum);
MMP_ERR MMPF_Sensor_SetPreviewEffect(MMP_UBYTE ubEffectID);
MMP_ERR MMPF_Sensor_SetExposureValuel(MMP_USHORT usEV);

MMP_ERR MMPF_Sensor_ProcessCmd(void);

MMP_BOOL MMPF_Sensor_IsStableMode(void);
MMP_BOOL MMPF_Sensor_IsFDTCMode(void);

MMP_ERR MMPF_Sensor_CheckFrameStart(MMP_USHORT usFrameCount);
MMP_ERR MMPF_Sensor_CheckFrameEnd(MMP_USHORT usFrameCount);
MMP_ERR MMPF_Sensor_SetVIFInterrupt(MMP_UBYTE ubFlag, MMP_BOOL bEnable);
MMP_ERR MMPF_Sensor_SetISPInterrupt(MMP_UBYTE ubFlag, MMP_BOOL bEnable);
MMP_ERR MMPF_Sensor_3A_SetInterrupt(MMP_BOOL bEnable);
MMP_BOOL MMPF_Sensor_3A_GetInterrupt(void);
MMP_ERR MMPF_Sensor_GetParam(MMPF_SENSOR_PARAM param_type, void* param);
MMP_ERR MMPF_Sensor_SetParam(MMPF_SENSOR_PARAM param_type, void* param);
#if ISP_BUF_IN_SRAM==0
MMP_ULONG MMPF_Sensor_CalculateBufferSize (void);
MMP_ERR MMPF_Sensor_AllocateBuffer(MMP_ULONG ulStartAddr);
#else
MMP_ERR MMPF_Sensor_CalculateBufferSize (MMP_ULONG *ulBufSize, MMP_ULONG *ulDmaSize);
MMP_ERR MMPF_Sensor_AllocateBuffer(MMP_ULONG ulStartAddr, MMP_ULONG ulDmaBufAddr);
#endif

#if PCAM_EN==1
MMP_ERR MMPF_Sensor_Initialize(MMP_USHORT usSensorID, MMP_UBYTE ubFrameRate,MMP_UBYTE ubPreviewMode);
MMP_ERR MMPF_Sensor_SetPreviewEffect(MMP_UBYTE ubEffectID);
MMP_ERR MMPF_Sensor_AutoFocus(void);
MMP_ERR MMPF_Sensor_PreviewMode(MMP_BOOL ubEnable,MMP_USHORT usIBCPipe,MMP_BOOL bCheckFrameEnd);
MMP_ERR MMPF_Sensor_SetSensorReg(MMP_USHORT usAddr, MMP_USHORT usData);
MMP_ERR MMPF_Sensor_GetSensorReg(MMP_USHORT usAddr,MMP_USHORT usData) ;
MMP_ERR MMPF_Sensor_PowerDown(MMP_USHORT usSensorID,MMP_UBYTE off_mode);
MMP_ERR MMPF_Sensor_Set3AState(MMPF_SENSOR_3A_STATE state);
MMP_USHORT MMPF_Sensor_CheckSkipFrame(void);
MMP_ERR MMPF_Sensor_InitPad(void);
#endif
MMP_ERR MMPF_Sensor_UpdateAEWindow(MMP_UBYTE working_path,MMPF_SCALER_PATH select_path);
//MMP_ERR MMPF_Sensor_SetColorTransform(MMP_UBYTE range); //Steven 20111107 ADD

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================
typedef enum {
	ISP_SENSOR_RESOL_NULL			= 0x00,
	ISP_SENSOR_RESOL_320x240		= 0x02, // QVGA
	ISP_SENSOR_RESOL_512x384		= 0x03,
	ISP_SENSOR_RESOL_640x480		= 0x04, // VGA
	ISP_SENSOR_RESOL_800x600		= 0x06,
	ISP_SENSOR_RESOL_1024x768		= 0x08,
	ISP_SENSOR_RESOL_1280x1024		= 0x0A, // 1.3M
	ISP_SENSOR_RESOL_1280x960		= 0x0B,
	ISP_SENSOR_RESOL_1600x1200		= 0x0C, // 2M
	ISP_SENSOR_RESOL_2048x1536		= 0x0E, // 3M
	ISP_SENSOR_RESOL_2560x1920		= 0x10, // 5M
	ISP_SENSOR_RESOL_3264x2448		= 0x12, // 8M
	ISP_SENSOR_RESOL_1280x720		= 0x13, // HD
	ISP_SENSOR_RESOL_1920x1080		= 0x14,	// Full HD
	ISP_SENSOR_RESOL_400x300		= 0x15,
	ISP_SENSOR_RESOL_1600x900		= 0x16
} ISP_SENSOR_RESOL;
#endif // _MMPF_SENSOR_H_
/// @}
