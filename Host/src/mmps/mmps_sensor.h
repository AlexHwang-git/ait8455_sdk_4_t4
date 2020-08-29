//==============================================================================
//
//  File        : mmps_sensor.h
//  Description : INCLUDE File for the Host Application for Sensor.
//  Author      : Alan Wu
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmps_sensor.h
 *  @brief The header File for the Host Sensor control functions
 *  @author Alan Wu
 *  @version 1.0
 */

#ifndef _MMPS_SENSOR_H_
#define _MMPS_SENSOR_H_

#include "mmp_lib.h"
#include "ait_config.h"
#include "mmps_display.h"
#include "mmpd_sensor.h"

/** @addtogroup MMPS_DSC
@{
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

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPS_SENSOR_FDTC_PATH {
    MMPS_SENSOR_FDTC_2PIPE = 0x00,
    MMPS_SENSOR_FDTC_LOOPBACK
} MMPS_SENSOR_FDTC_PATH;

typedef enum _MMPS_SENSOR_FDTC_FACE_DRAW {
    MMPS_SENSOR_FDTC_HOST_DRAW_FACE = 0x0,                                    ///< draw face detection by host
    MMPS_SENSOR_FDTC_FW_DRAW_FACE                                       ///< draw face detection by AIT firmware
} MMPS_SENSOR_FDTC_FACE_DRAW;

typedef struct _MMPS_SENSOR_CONFIG {
    MMP_UBYTE               ubSensorpreviewmodenum;                     ///< sensor total preview mode number.
    MMP_UBYTE               ubSensordefaultpreviewmode;                 ///< sensor default preview mode.
    MMP_UBYTE               ubSensorcapturemodenum;                     ///< sensor total capture mode number.
    MMP_UBYTE               ubSensordefaultphasecount;                  ///< sensor default phase count.
    MMP_USHORT              usSensorpreviewwidth[SENSOR_MODE_MAX_NUM];  ///< sensor preview width for each mode.
    MMP_USHORT              usSensorpreviewheight[SENSOR_MODE_MAX_NUM]; ///< sensor preview height for each mode.
    MMP_USHORT              usSensorcapturewidth[SENSOR_MODE_MAX_NUM];  ///< sensor capture width for each mode.
    MMP_USHORT              usSensorcaptureheight[SENSOR_MODE_MAX_NUM]; ///< sensor capture height for each mode.
    MMP_UBYTE               ubFrameRate[SENSOR_FRAMERATE_MAX_NUM];      ///< sensor total frame rate mode number.
    MMP_UBYTE               ubSensorevdata[SENSOR_EV_MAX_NUM];          ///< sensor exposure level.
    MMP_BOOL                bRawStoreDownsample[SENSOR_EV_MAX_NUM];     ///< enable raw store downsample or not
} MMPS_SENSOR_CONFIG;

typedef struct _MMPS_SENSOR_MODES {
    MMP_UBYTE               ubSensormodenum;                            ///< sensor total count
    MMPS_SENSOR_CONFIG      sensorconfig[SENSOR_COUNT_MAX_NUM];         ///< sensor configuration for each sensor
    // ++ Face Detection
    MMP_BOOL                bFaceDetectEnable;                          ///< need to support face detection function or not.
    MMP_USHORT              usFaceDetectGrabScaleM;                     ///< the grab M factor for face detection scale path.
    MMP_ULONG               ulFaceDetectWorkBufSize;                    ///< the maximum size of working buffer for face detection.
    MMP_ULONG               ulFaceDetectFeatureBufSize;                 ///< the maximum size of feature buffer for face detection.
    MMP_USHORT              usFaceDetectInfoBufSize;                    ///< the maximum size of info. buffer for detection result.
    MMP_UBYTE               ubDetectFaceNum;                            ///< maximum number of face to detect.
    MMP_UBYTE               ubFaceDetectInputBufCnt;                    ///< the number of FD frame buffers.
    MMPS_SENSOR_FDTC_PATH   faceDetectInputPath;                        ///< face detection input path selection
    MMPS_SENSOR_FDTC_FACE_DRAW faceDetectDisplayMethod;                 ///< the method to draw face rectangles
    // -- Face Detection
} MMPS_SENSOR_MODES;

typedef struct _MMPS_SENSOR_FDTC_RESULT {
    MMP_UBYTE           ubFacePosX;
    MMP_UBYTE           ubFacePosY;
    MMP_UBYTE           ubFaceWidthHeight;
    MMP_UBYTE           ubReserve;
} MMPS_SENSOR_FDTC_RESULT;

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
MMPS_SENSOR_MODES*  MMPS_Sensor_GetConfiguration(void);
MMP_ERR  MMPS_Sensor_Initialize(MMP_USHORT usSensorID, MMP_UBYTE ubPreviewMode, MMPD_SENSOR_MODE sensormode);
MMP_ERR  MMPS_Sensor_ChangePreviewMode(MMP_UBYTE ubPreviewMode, MMP_UBYTE ubPhaseCount);
MMP_ERR  MMPS_Sensor_GetCurResolution(MMP_USHORT *usWidth, MMP_USHORT *usHeight);
MMP_ERR  MMPS_Sensor_GetCurConfiguration(MMPS_SENSOR_CONFIG *config);
MMP_ERR  MMPS_Sensor_PowerDown(MMP_USHORT usSensor);

MMP_ERR  MMPS_Sensor_GetPreviewEffectNum(MMP_UBYTE *ubEffectNum);
MMP_ERR  MMPS_Sensor_SetPreviewEffect(MMP_UBYTE ubEffectID);

MMP_ERR  MMPS_Sensor_SetAwbType(MMP_USHORT type);
MMP_ERR  MMPS_Sensor_SetAeType(MMP_USHORT type);
MMP_ERR  MMPS_Sensor_SetNightMode(MMP_USHORT mode);
MMP_ERR  MMPS_Sensor_SetColorContrast(MMP_USHORT level);
MMP_ERR  MMPS_Sensor_SetColorSaturation(MMP_USHORT channel,MMP_USHORT level);
MMP_ERR  MMPS_Sensor_SetColorEdge(MMP_USHORT level);
MMP_ERR  MMPS_Sensor_SetExposureValue(MMP_UBYTE ubIndex);
MMP_ERR  MMPS_Sensor_SetSensorReg(MMP_USHORT usAddr, MMP_USHORT usData);
MMP_ERR  MMPS_Sensor_GetSensorReg(MMP_USHORT usAddr, MMP_USHORT *usData);
MMP_ERR  MMPS_Sensor_SetFrameRate(MMP_UBYTE ubIndex);
MMP_ERR  MMPS_Sensor_SetLightFreq(MMPD_SENSOR_DEBAND_MODE mode);

MMP_ERR  MMPS_Sensor_SetStableState(MMP_BOOL bSensorStableEnable, MMP_BOOL bMMPStableEnable);
MMP_ERR  MMPS_Sensor_GetMMPStableState(MMP_BOOL *bMMPStableEnable);

MMP_ERR  MMPS_Sensor_AllocateFaceDetectBuffer(MMP_ULONG *ulStartAddr, MMP_USHORT usInputWidth, 
                                             MMP_USHORT usInputHeight, MMP_ULONG ulResevOsdSize);
MMP_ERR  MMPS_Sensor_InitializeFaceDetect(MMP_UBYTE *ulFaceInfoBufAddr, MMP_ULONG ulFaceInfoBufSize);
MMP_ERR  MMPS_Sensor_SetupFaceDetectPath(void);
MMP_ERR  MMPS_Sensor_SetFaceDetectEnable(MMP_BOOL bEnable);
MMP_BOOL MMPS_Sensor_GetFaceDetectEnable(void);
MMP_ERR  MMPS_Sensor_GetFaceDetectResult(MMP_UBYTE *bFaceNum, MMPS_SENSOR_FDTC_RESULT **FdtcResult);
MMP_ERR  MMPS_Sensor_ConfigFaceDetectDisplay(MMPS_OSD_ATTRIBUTE *osdAttr);
MMP_ERR  MMPS_Sensor_DisplayFaceRectangles(MMP_ULONG ulTpColor, MMP_ULONG ulRectColor);

MMP_ERR  MMPS_Sensor_SetAFWin(MMPD_SENSOR_AFWIN AFWin);
MMP_ERR  MMPS_Sensor_AutoFocus(void);

MMP_ERR  MMPS_Sensor_SetFlip(MMP_UBYTE ubFlipType);
MMP_ERR  MMPS_Sensor_SetAEMode(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode);
MMP_ERR  MMPS_Sensor_SetAFMode(MMP_UBYTE ubAFMode);
MMP_ERR  MMPS_Sensor_GetMaxResolution(MMP_ULONG *usWidth, MMP_ULONG *usHeight);
MMP_ERR  MMPS_Sensor_GetRawStoreDownsample(MMP_BOOL *bDownsample);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _INCLUDES_H_
