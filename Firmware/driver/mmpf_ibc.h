//==============================================================================
//
//  File        : mmpf_ibc.h
//  Description : INCLUDE File for the Firmware IBC control  
//  Author      : Jerry Lai
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_IBC_H_
#define _MMPF_IBC_H_

#include    "includes_fw.h"
#include    "mmpf_icon.h"

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
#define IBCP_OFFSET 0x100


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================


typedef enum _MMPF_IBC_COLOR
{
    MMPF_IBC_COLOR_RGB565 = 0,
    MMPF_IBC_COLOR_YUV422 = 1,
    MMPF_IBC_COLOR_RGB888 = 2,
    MMPF_IBC_COLOR_RSRV = 3,
    MMPF_IBC_COLOR_I420 = 4,
    MMPF_IBC_COLOR_YUV420_LUMI_ONLY = 5,
    MMPF_IBC_COLOR_NV12 = 6,
    MMPF_IBC_COLOR_NV21 = 7,
    MMPF_IBC_COLOR_YUV422_YUYV = 8,
    MMPF_IBC_COLOR_YUV422_YVYU = 9,
    MMPF_IBC_COLOR_YUV422_UYVY = 10,
    MMPF_IBC_COLOR_YUV422_VYUY = 11
} MMPF_IBC_COLOR;

typedef enum _MMPF_IBC_PIPEID
{
    MMPF_IBC_PIPE_0 = 0,
    MMPF_IBC_PIPE_1,
    MMPF_IBC_PIPE_2,
    MMPF_IBC_PIPE_MAX
} MMPF_IBC_PIPEID;

typedef enum _MMPF_IBC_FX
{
    MMPF_IBC_FX_TOFB = 0,
    MMPF_IBC_FX_JPG, 
    MMPF_IBC_FX_RING_BUF,
    MMPF_IBC_FX_H264
} MMPF_IBC_FX;

typedef enum _MMPF_IBC_LINK_TYPE
{
	MMPF_IBC_LINK_NONE 		= 0x00,
	MMPF_IBC_LINK_DISPLAY	= 0x01,
	MMPF_IBC_LINK_VIDEO		= 0x02,
	MMPF_IBC_LINK_ROTATE	= 0x04,
	MMPF_IBC_LINK_FDTC      = 0x08,
	MMPF_IBC_LINK_GRAPHIC   = 0x10,
	MMPF_IBC_LINK_VID_FROM_GRA = 0x20,
	MMPF_IBC_LINK_CALLBACK = 0x40
} MMPF_IBC_LINK_TYPE;

typedef enum _MMPF_IBC_EVENT {
    MMPF_IBC_EVENT_FRM_ST = 0,
    MMPF_IBC_EVENT_FRM_RDY = 1,
    MMPF_IBC_EVENT_FRM_END = 2,
    MMPF_IBC_EVENT_FRM_PRERDY = 3,
    MMPF_IBC_EVENT_MAX = 4
} MMPF_IBC_EVENT;

typedef struct _MMPF_IBC_PIPEATTRIBUTE
{
    MMP_ULONG           ulBaseAddr;
    MMP_ULONG           ulBaseUAddr;
    MMP_ULONG           ulBaseVAddr;
    MMP_USHORT          usMirrorWidth;
    MMPF_IBC_COLOR      colorformat;
    MMPF_IBC_FX         function;
    MMP_ULONG           ulBaseEndAddr;  // only ring buf need config
    MMP_ULONG           ulLineOffset;
    MMP_BOOL            bMirrorEnable;
    MMPF_ICO_PIPEID     InputSource;
} MMPF_IBC_PIPEATTRIBUTE;

typedef void IbcCallBackFunc(void);
typedef void IbcLinkCallBackFunc(MMPF_IBC_PIPEID IbcPipe);
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
MMP_ERR MMPF_IBC_SetAttributes(MMPF_IBC_PIPEID pipeID, 
                            MMPF_IBC_PIPEATTRIBUTE *pipeattribute);
MMP_ERR MMPF_IBC_SetStoreEnable(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable);
MMP_BOOL MMPF_IBC_GetStoreEnable(MMPF_IBC_PIPEID pipeID);
MMP_ERR MMPF_IBC_SetMirrorEnable(MMPF_IBC_PIPEID pipeID, MMP_BOOL bEnable, MMP_USHORT usWidth);
MMP_ERR MMPF_IBC_SetInterruptEnable(MMPF_IBC_PIPEID pipeID, MMPF_IBC_EVENT event, MMP_BOOL bEnable);
MMP_ULONG MMPF_IBC_GetRingBufWrPtr(MMPF_IBC_PIPEID pipeID);
MMP_ERR MMPF_IBC_RegisterIntrCallBack(MMPF_IBC_PIPEID ibcpipe, MMPF_IBC_EVENT event, IbcCallBackFunc *pCallBack);
MMP_ERR MMPF_IBC_Reset(MMP_BOOL *pipe_store_en );
MMP_ERR MMPF_IBC_Restore(MMP_BOOL *pipe_store_en );
MMP_ERR MMPF_IBC_EncoderIBCSrc(MMPF_IBC_PIPEID pipeID, MMPF_IBC_FX function);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

#endif //_MMPF_IBC_H_