//==============================================================================
//
//  File        : mmpf_rawproc.h
//  Description : INCLUDE File for the Firmware Rawproc Control driver function
//  Author      : Ted
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_RAWPROC_H_
#define _MMPF_RAWPROC_H_

#include    "includes_fw.h"
#include    "mmpf_scaler.h"

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
typedef enum
{
    MMPF_RAWPROC_DEFAULT,
    MMPF_RAWPROC_DISTRIBUTION
}MMPF_RAWPROC_FETCH_TIMING_MODE;

typedef enum _MMPF_RAW_FETCH_ROTATE_TYPE
{
    MMPF_RAW_FETCH_ROTATE_NO_ROTATE = 0,
    MMPF_RAW_FETCH_ROTATE_RIGHT_90,
    MMPF_RAW_FETCH_ROTATE_RIGHT_180,
    MMPF_RAW_FETCH_ROTATE_RIGHT_270
} MMPF_RAW_FETCH_ROTATE_TYPE;

typedef enum _MMPF_RAWPROC_STORE_MODULE {
	MMPF_RAW_STORE_BAYER = 1,
	MMPF_RAW_STORE_LUMA = 2
}MMPF_RAWPROC_STORE_MODULE;

typedef enum _MMPF_RAWPROC_COLORDEPTH {
	MMPF_RAW_COLOR_DEPTH8 = 0,
	MMPF_RAW_COLOR_DEPTH10
} MMPF_RAWPROC_COLORDEPTH;

typedef struct _MMPF_RAWPROC_BUFATTRIBUTE {
    MMP_USHORT          *usWidth;
    MMP_USHORT          *usHeight;
    MMP_USHORT          *usLineOffset;
    MMPF_RAWPROC_COLORDEPTH     colordepth; // << 8/10 bit mode
    MMP_UBYTE	   		ubDownsampleRatio;
	MMP_ULONG           ulBaseAddr;
} MMPF_RAWPROC_BUFATTRIBUTE;

typedef struct _MMPF_RAWPROC_FETCHATTRBUTE {
    MMP_USHORT usLeft;
    MMP_USHORT usTop;
    MMP_USHORT usWidth;
    MMP_USHORT usHeight;
    MMPF_RAW_FETCH_ROTATE_TYPE	rotateType;
} MMPF_RAWPROC_FETCHATTRBUTE;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

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
MMP_ERR MMPF_RAWPROC_Reset(void);
MMP_ERR MMPF_RAWPROC_EnableRawPath(MMP_BOOL bEnable);
MMP_ERR MMPF_RAWPROC_ConfigRawPathMemory(MMP_ULONG ulAddr1,MMP_ULONG ulAddr2,MMP_ULONG ulBufNum);
MMP_ERR MMPF_RAWPROC_SetFetchTiming(MMP_UBYTE PixelDelay, MMP_USHORT Linedelay);
MMP_ERR MMPF_RAWPROC_StoreSensorData(MMP_BOOL bCapture,MMP_BOOL bWaitFinish);
MMP_ERR     MMPF_RAWPROC_FetchSensorData(MMP_BOOL bCapture,MMP_BOOL bWaitFinish,MMPF_RAW_FETCH_ROTATE_TYPE RotateType,MMP_ULONG ulRotateAddr);
MMP_ERR     MMPF_RAWPROC_FetchGrabData(MMP_SHORT grab_x, MMP_SHORT grab_y);
MMP_ERR MMPF_RAWPROC_SetCaptureStoreAddr(void);
MMP_ERR     MMPF_RAWPROC_SetRawBuffer(MMP_UBYTE ubBufIdx, MMP_ULONG ulAddr);
MMP_ERR     MMPF_RAWPROC_SetLumaBuffer(MMP_UBYTE ubBufIdx, MMP_ULONG ulAddr);
MMP_ULONG   MMPF_RAWPROC_GetLumaBuffer(void);
MMP_ERR     MMPF_RAWPROC_EnableDownsample(MMP_BOOL bEnable,MMP_ULONG ulRatio);
MMP_ERR     MMPF_RAWPROC_SetFetchPixelDelay(MMP_UBYTE PixelDelay);
MMP_ERR     MMPF_RAWPROC_SetFetchLineDelay(MMP_UBYTE LineDelay);
MMP_ERR     MMPF_RAWPROC_SetZoomOffset(MMPF_SCALER_ZOOMDIRECTION  direction,MMP_ULONG ulOffset);
MMP_ERR     MMPF_RAWPROC_SetMainPipeZoomOP(MMP_BOOL                bRawDownSample,
                                   MMPF_SCALER_ZOOM_INFO   *ScalerZoomInfo,
                                   MMPF_SCALER_PATH        CurZoomPathSel, 
                                   MMP_ULONG               N_offset,
                                   MMPF_SCALER_FIT_RANGE   *fitrange,
                                   MMPF_SCALER_GRABCONTROL *grabCtl);
                                   
MMP_ERR MMPF_RAWPROC_SetSubPipeZoomOP(MMP_BOOL                bRawDownSample,
                               MMPF_SCALER_PATH        CurZoomPathSel, 
                               MMPF_SCALER_FIT_RANGE   *fitrange,
                               MMPF_SCALER_GRABCONTROL *grabCtl,
                               MMPF_SCALER_GRABCONTROL *maingrabCtl);
MMP_ERR     MMPF_RAWPROC_GetVIFSetting(void);
MMP_ERR     MMPF_RAWPROC_ResetZoomOP(void);
MMP_ERR     MMPF_RAWPROC_ResetPreviewAttribue(void);
MMP_ERR     MMPF_RAWPROC_CalcRawFetchTiming(MMPF_SCALER_PATH gScalerPath, MMPF_RAWPROC_FETCH_TIMING_MODE mode );
MMP_ERR     MMPF_RAWPROC_GetCaptureStoreAddr(MMP_ULONG *ulAddr, MMP_BOOL bZeroShutterLag);
MMP_ERR     MMPF_RAWPROC_CheckRawStoreDone(void);
MMP_ERR     MMPF_RAWPROC_SetZeroShutterLagFetchBuffer(void);
MMP_ERR     MMPF_RAWPROC_SwitchZeroShutterLagStoreBuffer(void);
MMP_ERR     MMPF_RAWPROC_SetStoreRange(MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR     MMPF_RAWPROC_SetFetchRange(MMP_USHORT usWidth, MMP_USHORT usHeight);
MMP_ERR 	MMPF_RAWPROC_SetAttributes(MMPF_RAWPROC_STORE_MODULE  bayerall, MMPF_RAWPROC_BUFATTRIBUTE *attribute) ;
MMP_ERR 	MMPF_RAWPROC_StoreData(MMP_BOOL bWaitFinish);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _MMPF_RAWPROC_H_

