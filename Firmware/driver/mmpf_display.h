//==============================================================================
//
//  File        : mmpf_display.h
//  Description : INCLUDE File for the Firmware Display Control driver function, including LCD/TV/Win
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_DISPLAY_H_
#define _MMPF_DISPLAY_H_

//#include    "includes_fw.h"
#include "mmpf_typedef.h"
#include "mmp_err.h"
#include "mmpf_scaler.h"
#include "mmpf_ibc.h"
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

typedef void JpgCallBackFunc(void);

typedef struct _MMPF_PIPE_CROP_INFO {
    MMP_BOOL    bPipeEn;
    MMP_BOOL    bCropEn;
    MMP_ULONG   ulSrcWidth;
    MMP_ULONG   ulSrcHeight;
    MMP_ULONG   ulSrcX;
    MMP_ULONG   ulSrcY;
    MMP_ULONG   ulTargetWidth;
    MMP_ULONG   ulTargetHeight;
} MMPF_PIPE_CROP_INFO;

typedef enum _MMPF_DISPLAY_WINID
{
    MMPF_DISPLAY_WIN_MAIN = 0,
    MMPF_DISPLAY_WIN_PIP,
    MMPF_DISPLAY_WIN_OVERLAY,
    MMPF_DISPLAY_WIN_ICON,
    MMPF_DISPLAY_WIN_SCD_WIN,
    MMPF_DISPLAY_WIN_CCIR
} MMPF_DISPLAY_WINID;

typedef enum _MMPF_DISPLAY_CONTROLLER
{
    MMPF_DISPLAY_PRM_CONTROL = 0,
    MMPF_DISPLAY_SCD_CONTROL
} MMPF_DISPLAY_CONTROLLER;

typedef enum _MMPF_TV_TYPE {
    MMPF_TV_TYPE_NTSC = 0,
    MMPF_TV_TYPE_PAL
} MMPF_TV_TYPE;

typedef enum _MMPF_DISPLAY_PREVIEW_MODE
{
    MMPF_DISPLAY_P_LCD = 0,
    MMPF_DISPLAY_FLM,
    MMPF_DISPLAY_RGB_LCD,
    MMPF_DISPLAY_TV,
    MMPF_DISPLAY_HDMI,
    MMPF_DISPLAY_CCIR
} MMPF_DISPLAY_PREVIEW_MODE;

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
MMP_ERR MMPF_Display_SetRefresh(MMPF_DISPLAY_CONTROLLER controller);
MMP_ERR MMPF_Display_StartPreview(MMP_USHORT usIBCPipe, MMP_BOOL bCheckFrameEnd);
MMP_ERR MMPF_Display_StopPreview(MMP_USHORT usIBCPipe);

MMP_ERR MMPF_Display_FrameDoneTrigger(MMP_USHORT usIBCPipe);
MMP_ERR MMPF_Display_StartSoftwareRefresh(MMP_USHORT usIBCPipe);
#if PCAM_EN==1
MMP_ERR MMPF_SetDataReadyLine(MMP_UBYTE pipe); 
MMP_ERR MMPF_Display_WaitFrame(MMP_USHORT frame_n,MMP_UBYTE pipe);
MMP_ERR MMPF_NotifyDataReady(MMP_UBYTE yuv_pipe);
MMP_ERR MMPF_Display_SkipFrame(MMP_SHORT frame_n);
MMP_ERR MMPF_Display_TriggerGRA(MMP_UBYTE yuv_pipe, MMP_ULONG buf_addr, MMP_USHORT usCropWidth, MMP_USHORT usCropHeight);
MMP_ERR MMPF_Display_DoGRA(MMP_UBYTE eid/*yuv_pipe*/, MMP_ULONG buf_addr, MMP_USHORT usCropWidth, MMP_USHORT usCropHeight,MMP_BOOL bSkipGraLoop);
MMP_BOOL MMPF_Display_ConfigGRA(MMP_UBYTE eid/*yuv_pipe*/, MMP_ULONG buf_addr, MMP_USHORT usCropWidth, MMP_USHORT usCropHeight);
MMP_ERR MMPF_Display_SkipGraLoop(MMP_BOOL skip);
#endif
void MMPF_Display_LoobackFrame(MMPF_IBC_PIPEID SrcPipe, MMPF_IBC_PIPEID DstPipe,
                               MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight, 
                               MMP_USHORT usWidth, MMP_USHORT usHeight,
                               MMP_ULONG  ulSrcYAddr,
                               MMP_ULONG  ulSrcUAddr,
                               MMP_ULONG  ulSrcVAddr,
                               MMPF_GRAPHICS_RECT *Rect,
                               GraphicRotDMACallBackFunc *GraEndCallback);

void MMPF_Display_PauseSimulcast(void);
MMP_UBYTE MMPF_Display_StateSimulcast(void);
void MMPF_Display_ResumeSimulcast(void);
void MMPF_Display_SyncH264Ratio(void);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _MMPD_LCD_H_
