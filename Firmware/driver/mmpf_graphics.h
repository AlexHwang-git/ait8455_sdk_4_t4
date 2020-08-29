//==============================================================================
//
//  File        : mmpf_graphic2.h
//  Description : INCLUDE File for the Firmware Graphic Driver.
//  Author      : Rogers Chen
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_GRAPHICS_H_
#define _MMPF_GRAPHICS_H_


#include "mmp_err.h"
#if defined(ALL_FW)
#include "mmpd_graphics.h"
#endif
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
#define  GRAPHICS_SEM_TIMEOUT	0x100
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPF_GRAPHICS_COLORDEPTH
{
    MMPF_GRAPHICS_COLORDEPTH_8 = 1,
    MMPF_GRAPHICS_COLORDEPTH_16 = 2,
    MMPF_GRAPHICS_COLORDEPTH_24 = 3,
    MMPF_GRAPHICS_COLORDEPTH_YUV422 = 4,
	MMPF_GRAPHICS_COLORDEPTH_YUV420 = 5,
	MMPF_GRAPHICS_COLORDEPTH_32                 = 6,
	MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE  = 7,
    MMPF_GRAPHICS_COLORDEPTH_YUY2 = 8
} MMPF_GRAPHICS_COLORDEPTH;

typedef enum _MMPF_GRAPHICS_SCAL_SRC
{
  MMPF_GRAPHICS_SCAL_FIFO = 0,
  MMPF_GRAPHICS_SCAL_FB
} MMPF_GRAPHICS_SCAL_SRC;

typedef enum _MMPF_GRAPHICS_ROTATE_TYPE
{
    MMPF_GRAPHICS_ROTATE_NO_ROTATE = 0,
    MMPF_GRAPHICS_ROTATE_RIGHT_90,
    MMPF_GRAPHICS_ROTATE_RIGHT_180,
    MMPF_GRAPHICS_ROTATE_RIGHT_270
} MMPF_GRAPHICS_ROTATE_TYPE;

typedef struct _MMPF_GRAPHICS_BUFATTRIBUTE {
    MMP_USHORT          usWidth;
    MMP_USHORT          usHeight;
    MMP_USHORT          usLineOffset;
    MMPF_GRAPHICS_COLORDEPTH     colordepth;
	MMP_ULONG           ulBaseAddr;
    MMP_ULONG           ulBaseUAddr;
    MMP_ULONG           ulBaseVAddr;
} MMPF_GRAPHICS_BUFATTRIBUTE;

typedef struct _MMPF_GRAPHICS_RECT {
    MMP_USHORT usLeft;
    MMP_USHORT usTop;
    MMP_USHORT usWidth;
    MMP_USHORT usHeight;
} MMPF_GRAPHICS_RECT;


typedef void GraphicRotDMACallBackFunc(void);
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
void MMPF_Graphics_Init(void);
MMP_ERR MMPF_Graphics_Scale_P0(MMPF_GRAPHICS_BUFATTRIBUTE src,
                            MMPF_GRAPHICS_BUFATTRIBUTE dst,
                            GraphicRotDMACallBackFunc *GRACallBack,
                            MMP_UBYTE  ubPixelDelay,
                            MMP_USHORT usWaitMs);

MMP_ERR MMPF_Graphics_Scale(MMPF_GRAPHICS_BUFATTRIBUTE src,
                            MMPF_GRAPHICS_BUFATTRIBUTE dst,
                            GraphicRotDMACallBackFunc *GRACallBack,
                            MMP_UBYTE  ubPixelDelay,
                            MMP_USHORT usWaitMs);
                            
MMP_ERR MMPF_Graphics_SetScaleAttribute(MMPF_GRAPHICS_BUFATTRIBUTE *bufattribute,
                        MMP_ULONG ulScaleBufAddr, MMPF_GRAPHICS_COLORDEPTH incolormode,                       
						MMPF_GRAPHICS_RECT *rect, MMP_USHORT usUpscale, 
						MMPF_GRAPHICS_SCAL_SRC srcsel);
                            
MMP_ERR MMPF_Graphics_SetCrop(MMP_ULONG ulStartX, MMP_ULONG ulStartY,
                              MMP_ULONG ulCropWidth, MMP_ULONG ulCropHeight,
                              MMP_ULONG ulYAddr,MMP_ULONG ulUAddr,MMP_ULONG ulVAddr) ;
#if (CHIP == P_V2)
MMP_ULONG MMPF_Graphic_SetKeyColor(MMP_BOOL bSetFgColor, MMP_ULONG ulColor);
MMP_ERR   MMPF_Graphic_DrawRectToBuf(MMPF_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPF_GRAPHICS_RECT *rect, MMP_BYTE bThickness);
MMP_ERR MMPF_Graphics_CopyImageBuftoBuf(MMP_ULONG SrcAddr,MMP_ULONG dstAddr,  
						MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty,
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode);
#endif
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _INCLUDES_H_
