//==============================================================================
//
//  File        : mmpf_ico.h
//  Description : INCLUDE File for the Firmware ICON/ICO engine control  
//  Author      : Jerry Lai
//  Revision    : 1.0
//
//==============================================================================


#ifndef _MMPF_ICON_H_
#define _MMPF_ICON_H_

#include    "includes_fw.h"
#include "mmpf_scaler.h"

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
#define ICON_OFFSET 0x20
#define MAX_ICON_NUM (10)

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================


typedef enum _MMPD_ICON_COLOR
{
    MMPD_ICON_COLOR_INDEX8 = 0,
    MMPD_ICON_COLOR_RGB565,
    MMPD_ICON_COLOR_ARGB888
} MMPD_ICON_COLOR;

typedef struct _MMPD_ICON_BUFATTRIBUTE
{
    MMP_ULONG           ulBaseAddr;
    MMP_USHORT          usStartX;
    MMP_USHORT          usStartY;
    MMP_USHORT          usWidth;
    MMP_USHORT          usHeight;
    MMPD_ICON_COLOR     colorformat;
    MMP_ULONG           ulTpColor;
    MMP_BOOL            bTpEnable;
    MMP_BOOL            bSemiTpEnable;
    MMP_UBYTE           ubIconWeight;
    MMP_UBYTE           ubDstWeight;
} MMPD_ICON_BUFATTRIBUTE;


typedef struct _MMPF_ICO_PIPEATTRIBUTE
{
    MMPF_SCALER_PATH    inputsel;
    MMP_BOOL            bDlineEn;
} MMPF_ICO_PIPEATTRIBUTE;

typedef enum _MMPF_ICO_PIPEID
{
    MMPF_ICO_PIPE_0 = 0,
    MMPF_ICO_PIPE_1,
    MMPF_ICO_PIPE_2,
    MMPF_ICO_PIPE_MAX
} MMPF_ICO_PIPEID;


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR MMPF_ICON_SetAttributes(MMPF_ICO_PIPEID pipeID, 
                        MMPF_ICO_PIPEATTRIBUTE *pipeattribute);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

#endif //_MMPF_ICON_H_