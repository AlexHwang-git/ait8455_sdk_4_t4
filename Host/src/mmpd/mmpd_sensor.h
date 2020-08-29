/// @ait_only
//==============================================================================
//
//  File        : mmpd_sensor.h
//  Description : INCLUDE File for the Host DSC Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpd_sensor.h
 *  @brief The header File for the Host DSC control functions
 *  @author Penguin Torng
 *  @version 1.0
 */

#ifndef _MMPD_SENSOR_H_
#define _MMPD_SENSOR_H_

#include    "mmp_lib.h"
#include    "mmpf_ibc.h"
/** @addtogroup MMPD_Sensor
 *  @{
 */

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
#define SENSOR_AFWIN_MAX_NUM 	1
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPD_SENDOR_MODE {
	SENSOR_DSC_PRW_MODE = 1,
	SENSOR_VIDEO_PRW_MODE = 2
} MMPD_SENSOR_MODE;

typedef enum _MMPD_SENSOR_DEBAND_MODE {
	SENSOR_DSC_DEBAND_60HZ = 0,
	SENSOR_DSC_DEBAND_50HZ,
	SENSOR_DSC_VIDEO_DEBAND_60HZ,
	SENSOR_DSC_VIDEO_DEBAND_50HZ
} MMPD_SENSOR_DEBAND_MODE;

typedef struct _MMPD_SENSOR_AFWIN {
	MMP_USHORT 	usAFWinNum;
	MMP_USHORT  usPreviewWidth;
    MMP_USHORT  usPreviewHeight;
	MMP_USHORT  usStartX[SENSOR_AFWIN_MAX_NUM];
    MMP_USHORT  usStartY[SENSOR_AFWIN_MAX_NUM];
	MMP_USHORT  usWidth[SENSOR_AFWIN_MAX_NUM];
    MMP_USHORT  usHeight[SENSOR_AFWIN_MAX_NUM];
} MMPD_SENSOR_AFWIN;

typedef struct _MMPD_SENSOR_FDTCATTRIBUTE {
    MMP_UBYTE   *ulHostInfoBufAddr;         ///< the address of host buffer for saving face detection results
    MMP_ULONG   ulHostInfoBufSize;          ///< the size of host buffer for saving face detection results
	MMP_USHORT  usInputW;                   ///< the width of input frame to do face detection
	MMP_USHORT  usInputH;                   ///< the height of input frame to do face detection
    MMP_ULONG   ulInputBufAddr[2];          ///< the address of slots to keep whole frames
    MMP_ULONG   ulWorkBufAddr;              ///< the address of face detection working buffer
    MMP_ULONG   ulFeatureBufAddr;           ///< the address of face detection feature buffer
    MMP_ULONG   ulInfoBufAddr;              ///< the address of buffer for keeping face detection results
    MMP_USHORT  usGrabScaleM;               ///< the grab M factor of face detection scale path
    MMP_UBYTE   ubInputBufCnt;              ///< the total count of slots to keep whole frames
    MMP_UBYTE   ubDetectFaceNum;            ///< the maximum number of face for detection
    MMP_ULONG   ulOSDbufferAddr;            ///< the buffer address for face rectangles OSD
    MMP_ULONG   ulOSDbufferSize;            ///< the buffer size for face rectangles OSD
    MMP_ULONG   winID;                      ///< the window to display face rectangles
} MMPD_SENSOR_FDTCATTRIBUTE;

typedef enum {
    MMPS_SENSOR_SET_EFFECT              = 0x00000001,
    MMPS_SENSOR_SET_AWB                 = 0x00000002,
    MMPS_SENSOR_SET_AE_ENABLE           = 0x00000004,
    MMPS_SENSOR_SET_NIGHTMODE           = 0x00000008,
    MMPS_SENSOR_SET_CONTRAST            = 0x00000010,
    MMPS_SENSOR_SET_SATURATION          = 0x00000020,
    MMPS_SENSOR_SET_SATURATION_LEVEL    = 0x00000040,
    MMPS_SENSOR_SET_EDGE                = 0x00000080,
    MMPS_SENSOR_SET_EXPOSURE            = 0x00000100,
    MMPS_SENSOR_SET_AE_MODE             = 0x00000200,
    MMPS_SENSOR_SET_ISO                 = 0x00000400,
    MMPS_SENSOR_SET_AF_MODE             = 0x00000800,
    MMPS_SENSOR_SET_REG                 = 0x00001000,
    MMPS_SENSOR_SET_REG_DATA            = 0x00002000,
    MMPS_SENSOR_SET_LIGHT_FREQ          = 0x00004000,
    MMPS_SENSOR_SET_STABLE_ENABLE       = 0x00008000,
    MMPS_SENSOR_SET_MMP_STABLE_ENABLE   = 0x00010000,
    MMPS_SENSOR_SET_AF_WINDOW           = 0x00020000,
    MMPS_SENSOR_SET_MAX                 = 18
} MMPS_SENSOR_EFFECT;

typedef enum {
    MMPS_SENSOR_SETPARAM_EFFECT = 0,
    MMPS_SENSOR_SETPARAM_AWB,
    MMPS_SENSOR_SETPARAM_AE_ENABLE,
    MMPS_SENSOR_SETPARAM_NIGHTMODE,
    MMPS_SENSOR_SETPARAM_CONTRAST,
    MMPS_SENSOR_SETPARAM_SATURATION,
    MMPS_SENSOR_SETPARAM_SATURATION_LEVEL,
    MMPS_SENSOR_SETPARAM_EDGE,
    MMPS_SENSOR_SETPARAM_EXPOSURE,
    MMPS_SENSOR_SETPARAM_AE_MODE,
    MMPS_SENSOR_SETPARAM_ISO,
    MMPS_SENSOR_SETPARAM_AF_MODE,
    MMPS_SENSOR_SETPARAM_REG,
    MMPS_SENSOR_SETPARAM_REG_DATA,
    MMPS_SENSOR_SETPARAM_LIGHT_FREQ,
    MMPS_SENSOR_SETPARAM_STABLE_ENABLE,
    MMPS_SENSOR_SETPARAM_MMP_STABLE_ENABLE,
    MMPS_SENSOR_SETPARAM_AF_WINDOW,
    MMPS_SENSOR_SETPARAM_MAX
} MMPS_SENSOR_EFFECT_PARAM;

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
MMP_ERR	MMPD_Sensor_Initialize(MMP_USHORT usSensorID, MMP_UBYTE ubFrameRate,
                                    MMP_UBYTE ubPreviewMode, MMP_UBYTE ubPhaseCount);
MMP_ERR MMPD_Sensor_AllocateBuffer(MMP_ULONG ulStartAddr, MMP_ULONG *ulSize);
MMP_ERR	MMPD_Sensor_SetMode(MMPD_SENSOR_MODE sensormode);
MMP_ERR	MMPD_Sensor_PowerDown(MMP_USHORT usSensorID,MMP_UBYTE off_mode);
MMP_ERR	MMPD_Sensor_SetAwbType(MMP_USHORT usType);
MMP_ERR	MMPD_Sensor_SetAeType(MMP_USHORT usType);
MMP_ERR	MMPD_Sensor_SetNightMode(MMP_USHORT usMode);
MMP_ERR	MMPD_Sensor_SetColorContrast(MMP_USHORT usLevel);
MMP_ERR	MMPD_Sensor_SetColorSaturation(MMP_USHORT usChannel,MMP_USHORT usLevel);
MMP_ERR	MMPD_Sensor_SetColorEdge(MMP_USHORT usLevel);
MMP_ERR	MMPD_Sensor_SetSensorReg(MMP_USHORT usAddr, MMP_USHORT usData);
MMP_ERR	MMPD_Sensor_GetSensorReg(MMP_USHORT usAddr, MMP_USHORT *usData);
MMP_ERR	MMPD_Sensor_SetExposureValue(MMP_USHORT usExpVal);
MMP_ERR	MMPD_Sensor_SetHistEQState(MMP_BOOL bHistEQState);

MMP_ERR MMPD_Sensor_SetStableState(MMP_BOOL bSensorStableEnable, MMP_BOOL bMMPStableEnable);
MMP_ERR MMPD_Sensor_GetMMPStableState(MMP_BOOL *bMMPStableEnable);
MMP_ERR MMPD_Sensor_SetHistEQState(MMP_BOOL bHistEQState) ;

MMP_ERR MMPD_Sensor_InitFaceDetectConfig(MMPD_SENSOR_FDTCATTRIBUTE *fdtcattribute);
MMP_ERR MMPD_Sensor_SetFaceDetectBuf(MMPD_SENSOR_FDTCATTRIBUTE *fdtcattribute);
MMP_ERR MMPD_Sensor_SetFaceDetectEnable(MMP_BOOL bEnable);

MMP_ERR  MMPD_Sensor_SetAFWin(MMPD_SENSOR_AFWIN AFWin);
MMP_ERR MMPD_Sensor_AutoFocus(void);
MMP_ERR MMPD_Sensor_InfinityFocus(void);
MMP_ERR MMPD_Sensor_SetLightFreq(MMPD_SENSOR_DEBAND_MODE mode);
MMP_ERR MMPD_Sensor_GetPreviewEffectNum(MMP_UBYTE *ubEffectNum);
MMP_ERR MMPD_Sensor_SetPreviewEffect(MMP_UBYTE ubEffectID);
MMP_ERR MMPD_Sensor_SetPDCLK(MMP_UBYTE ubDivider);
MMP_ERR MMPD_Sensor_PreviewMode(MMP_UBYTE ubPreviewMode, MMP_UBYTE ubPhaseCount);
MMP_ERR MMPD_Sensor_SetFlip(MMP_UBYTE ubFlipType);
MMP_ERR MMPD_Sensor_SetAEMode(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode);
MMP_ERR MMPD_Sensor_SetAFMode(MMP_UBYTE ubAFMode);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _INCLUDES_H_
/// @end_ait_only

