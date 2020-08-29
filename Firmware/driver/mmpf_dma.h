//==============================================================================
//
//  File        : mmpf_dma.h
//  Description : INCLUDE File for the Firmware Graphic Driver (DMA portion).
//  Author      : Alan Wu
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_DMA_H_
#define _MMPF_DMA_H_

//#include    "includes_fw.h"


//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================
#define DMA_OFFSET 0x10
#define DMA_SEM_WAITTIME 10000
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
typedef enum _MMPF_DMA_M_ID
{
	#if (CHIP == P_V2)||(CHIP == VSN_V3)
    MMPF_DMA_M_0 = 0,
    MMPF_DMA_M_1
    #endif
    #if (CHIP == VSN_V2)
    MMPF_DMA_M_0 = 0
    #endif
} MMPF_DMA_M_ID;

typedef enum _MMPF_DMA_R_ID
{
    #if (CHIP == P_V2)
    MMPF_DMA_R_0 = 0,
    MMPF_DMA_R_1
    #endif
    #if (CHIP == VSN_V2)||(CHIP == VSN_V3)
    MMPF_DMA_R_0 = 0
    #endif
} MMPF_DMA_R_ID;


typedef enum _MMPF_DMA_R_BPP
{
    MMPF_DMA_R_BPP8 = 0,
    MMPF_DMA_R_BPP16,
    MMPF_DMA_R_BPP24
} MMPF_DMA_R_BPP;

typedef enum _MMPF_DMA_R_BLOCK
{
    MMPF_DMA_R_BLOCK8 = 0,
    MMPF_DMA_R_BLOCK16
} MMPF_DMA_R_BLOCK;

typedef enum _MMPF_DMA_R_TYPE
{
    MMPF_DMA_R_NO = 0,
    MMPF_DMA_R_90,
    MMPF_DMA_R_180,
    MMPF_DMA_R_270
} MMPF_DMA_R_TYPE;

typedef enum _MMPF_DMA_R_MIRROR_TYPE
{
    MMPF_DMA_R_MIRROR_H = 0,
    MMPF_DMA_R_MIRROR_V,
    MMPF_DMA_R_NO_MIRROR
} MMPF_DMA_R_MIRROR_TYPE;

typedef struct _MMPF_DMA_ROT_DATA{
	MMP_SHORT BufferNum;
	MMP_SHORT BufferIndex;
    MMP_SHORT SrcWidth[3];
    MMP_SHORT SrcHeight[3];
    MMP_ULONG SrcAddr[3];
    MMP_ULONG DstAddr[3];
    MMP_USHORT SrcLineOffset[3];
    MMP_USHORT DstLineOffset[3];
    MMP_SHORT BytePerPixel[3];
    MMPF_DMA_R_TYPE RotateType;
    MMPF_GRAPHICS_COLORDEPTH ColorDepth;
    MMP_BOOL MirrorEnable;
    MMPF_DMA_R_MIRROR_TYPE MirrorType;
} MMPF_DMA_ROT_DATA;

typedef struct _MMPF_DMA_M_LOFFS_DATA{
	MMP_ULONG SrcWidth;
	MMP_ULONG SrcOffset;
	MMP_ULONG DstWidth;
	MMP_ULONG DstOffset;
} MMPF_DMA_M_LOFFS_DATA;


typedef void DmaCallBackFunc(void);

typedef struct _MMPF_DMA_M_2NDROUND{
	MMP_ULONG ulSrcaddr;
	MMP_ULONG ulDstaddr;
	MMP_ULONG ulCount;
	MMPF_DMA_M_LOFFS_DATA* ptrLineOffset;
	DmaCallBackFunc *CallBackFunc;
}MMPF_DMA_M_2NDROUND;
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
MMP_ERR MMPF_DMA_Initialize(void);
MMP_ERR MMPF_DMA_RotateImageBuftoBuf(MMPF_GRAPHICS_BUFATTRIBUTE *srcbufattribute, 
                                  MMPF_GRAPHICS_RECT *srcrect, MMPF_GRAPHICS_BUFATTRIBUTE *dstbufattribute, 
                                  MMP_USHORT usDststartx, MMP_USHORT usDststarty, MMPF_GRAPHICS_ROTATE_TYPE rotatetype, 
                                  DmaCallBackFunc *CallBackFunc, MMP_BOOL mirrorEnable, MMPF_DMA_R_MIRROR_TYPE mirrortype);                             

MMP_ERR MMPF_DMA_CpuRotImageBufToBuf(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,MMP_USHORT usSrcwidth, MMP_USHORT usSrcheight,
                                  MMPF_GRAPHICS_COLORDEPTH colordepth, MMPF_GRAPHICS_ROTATE_TYPE rotatetype,
                                  MMP_USHORT usSrcOffest, MMP_USHORT usDstOffset);

MMP_ERR MMPF_DMA_MoveData0(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
                            MMP_ULONG ulCount, MMPF_DMA_M_LOFFS_DATA* ptrLineOffset,
                            DmaCallBackFunc *CallBackFunc);
MMP_ERR MMPF_DMA_MoveData1(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
								  MMP_ULONG ulCount, MMPF_DMA_M_LOFFS_DATA* ptrLineOffset,
								  DmaCallBackFunc *CallBackFunc);

MMP_ERR MMPF_DMA_RotateData0(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
								  MMP_ULONG ulOffset, MMP_ULONG ulWidth, MMP_ULONG ulHeight, 
								  MMP_ULONG ctl_type, MMP_ULONG mirror_type, DmaCallBackFunc *CallBackFunc);

MMP_ERR MMPF_DMA_RotateData1(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
								  MMP_ULONG ulOffset, MMP_ULONG ulWidth, MMP_ULONG ulHeight, 
								  MMP_ULONG ctl_type, MMP_ULONG mirror_type, DmaCallBackFunc *CallBackFunc);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _INCLUDES_H_
