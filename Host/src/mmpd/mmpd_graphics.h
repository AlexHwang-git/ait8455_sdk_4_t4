//==============================================================================
//
//  File        : mmpd_graphics.h
//  Description : INCLUDE File for the Host Graphics Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpd_graphics.h
 *  @brief The header File for the Host Graphic control functions
 *  @author Penguin Torng
 *  @version 1.0
 */

#ifndef _MMPD_GRAPHICS_H_
#define _MMPD_GRAPHICS_H_

#include    "mmp_lib.h"

/** @addtogroup MMPD_Graphics
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

#define	 GRAPHICS_FIFO_RD_THRESHOLD	8
#define	 GRAPHICS_FIFO_WR_THRESHOLD	8
#define	 GRAPHICS_FIFO_RD_WR_THRESHOLD	0x0808

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPD_GRAPHICS_COLORDEPTH
{
    MMPD_GRAPHICS_COLORDEPTH_8                  = 1,
    MMPD_GRAPHICS_COLORDEPTH_16                 = 2,
    MMPD_GRAPHICS_COLORDEPTH_24                 = 3,
    MMPD_GRAPHICS_COLORDEPTH_YUV422             = 4,
	MMPD_GRAPHICS_COLORDEPTH_YUV420             = 5,
	MMPD_GRAPHICS_COLORDEPTH_32                 = 6,
	MMPD_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE  = 7
} MMPD_GRAPHICS_COLORDEPTH;

typedef struct _MMPD_GRAPHICS_BUFATTRIBUTE {
    MMP_USHORT          usWidth;
    MMP_USHORT          usHeight;
    MMP_USHORT          usLineOffset;
    MMPD_GRAPHICS_COLORDEPTH     colordepth;
	MMP_ULONG           ulBaseAddr;
    MMP_ULONG           ulBaseUAddr;
    MMP_ULONG           ulBaseVAddr;
} MMPD_GRAPHICS_BUFATTRIBUTE;

typedef enum _MMPD_GRAPHICS_PATMODE {
    MMPD_GRAPHICS_PATMODE_SNGL = 0,
    MMPD_GRAPHICS_PATMODE_4_SQ,
    MMPD_GRAPHICS_PATMODE_LR,
    MMPD_GRAPHICS_PATMODE_UD
} MMPD_GRAPHICS_PATMODE;

typedef struct _MMPD_GRAPHICS_COLORPAT {
    MMPD_GRAPHICS_PATMODE        patmode;
    MMP_ULONG       ulYColor[4];
    MMP_ULONG       ulUColor[4];
    MMP_ULONG       ulVColor[4];
} MMPD_GRAPHICS_COLORPAT;

typedef enum _MMPD_GRAPHICS_ROTATE_TYPE
{
    MMPD_GRAPHICS_ROTATE_NO_ROTATE = 0,
    MMPD_GRAPHICS_ROTATE_RIGHT_90,
    MMPD_GRAPHICS_ROTATE_RIGHT_180,
    MMPD_GRAPHICS_ROTATE_RIGHT_270
} MMPD_GRAPHICS_ROTATE_TYPE;

typedef enum _MMPD_GRAPHICS_KEYCOLOR
{
    MMPD_GRAPHICS_FG_COLOR = 0,
    MMPD_GRAPHICS_BG_COLOR
} MMPD_GRAPHICS_KEYCOLOR;

typedef struct _MMPD_GRAPHICS_RECT {
    MMP_USHORT usLeft;
    MMP_USHORT usTop;
    MMP_USHORT usWidth;
    MMP_USHORT usHeight;
} MMPD_GRAPHICS_RECT;

typedef enum _MMPD_GRAPHICS_RECTFILLTYPE {
    MMPD_GRAPHICS_SOLID_FILL = 0,
    MMPD_GRAPHICS_LINE_FILL,
    MMPD_GRAPHICS_FRAME_FILL
} MMPD_GRAPHICS_RECTFILLTYPE;

typedef enum _MMPD_GRAPHICS_SCAL_SRC
{
    MMPD_GRAPHICS_SCAL_FIFO = 0,
    MMPD_GRAPHICS_SCAL_FB
} MMPD_GRAPHICS_SCAL_SRC;

typedef enum _MMPD_GRAPHICS_ROP
{
    MMPD_GRAPHICS_ROP_BLACKNESS = 0,
    MMPD_GRAPHICS_ROP_NOTSRCERASE = 0x01,	// ~(S+D)
    MMPD_GRAPHICS_ROP_NOTSRCCOPY = 0x03,	// ~S
    MMPD_GRAPHICS_ROP_SRCERASE = 0x04,		// S.~D
    MMPD_GRAPHICS_ROP_DSTINVERT = 0x05,		// ~D
    MMPD_GRAPHICS_ROP_SRCINVERT = 0x06,		// S^D
    MMPD_GRAPHICS_ROP_SRCAND = 0x08,		// S.D
    MMPD_GRAPHICS_ROP_MERGEPAINT = 0x0B,	// ~S+D
    MMPD_GRAPHICS_ROP_SRCCOPY = 0x0C,		// S
    MMPD_GRAPHICS_ROP_SRCPAINT = 0x0E,		// S+D
    MMPD_GRAPHICS_ROP_WHITENESS = 0x0F
} MMPD_GRAPHICS_ROP;

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
MMP_ERR MMPD_Graphics_CopyImageBuftoBuf(MMPD_GRAPHICS_BUFATTRIBUTE *srcbufattribute, 
                        MMPD_GRAPHICS_RECT *srcrect, MMPD_GRAPHICS_BUFATTRIBUTE *dstbufattribute, 
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode);

MMP_ERR MMPD_Graphics_RotateImageBuftoBuf(MMPD_GRAPHICS_BUFATTRIBUTE *srcbufattribute,
						MMPD_GRAPHICS_RECT *srcrect, MMPD_GRAPHICS_BUFATTRIBUTE *dstbufattribute, 
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode,
						MMPD_GRAPHICS_ROTATE_TYPE rotate);
/// @ait_only
MMP_ERR MMPD_Graphics_CopyImageBuftoFIFO(MMP_USHORT *usHostbufaddr,
						MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPD_GRAPHICS_RECT *srcrect);

MMP_ERR MMPD_Graphics_CopyImageFIFOtoBuf(MMP_USHORT *usHostbufaddr,
						MMPD_GRAPHICS_COLORDEPTH colordepth, MMP_USHORT usHostLineOffset,
						MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                        MMPD_GRAPHICS_RECT *srcrect, MMP_USHORT usDstStartx, 
                        MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode);

MMP_UBYTE	MMPD_Graphics_SetTransparent(MMP_UBYTE transpActive);
MMP_ULONG   MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_KEYCOLOR keycolor, MMP_ULONG ulColor);
MMP_ERR     MMPD_Graphics_DrawRectToBuf(MMPD_GRAPHICS_BUFATTRIBUTE *dstbufattribute,
						MMPD_GRAPHICS_RECT *rect, MMPD_GRAPHICS_RECTFILLTYPE filltype, 
						MMP_USHORT usFrameLength, MMPD_GRAPHICS_ROP ropcode);


MMP_ERR MMPD_Graphics_ExpandMonoFIFOtoBuf(MMP_USHORT *usMonobufaddr,
                        MMP_USHORT usBmpwidth, MMP_USHORT usBmpheight, MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
						MMP_USHORT usStartx, MMP_USHORT usStarty);
MMP_ERR MMPD_Graphics_ExpandMonoMemtoBuf(MMP_ULONG ulBmpaddr,
				        MMP_USHORT usBmpwidth, MMP_USHORT usBmpheight, MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
			MMP_USHORT usStartx, MMP_USHORT usStarty, MMP_BOOL bDataContinue);
MMP_ERR MMPD_Graphics_DrawTexttoBuf(MMP_ULONG ulBmpaddr,
            MMP_USHORT usTextwidth, MMP_USHORT usTextheight,
						MMP_BYTE *ptext, MMP_USHORT usTextcount, MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
			MMP_USHORT usStartx, MMP_USHORT usStarty);


MMP_ERR MMPD_Graphics_RotateImageFIFOtoBuf(MMP_USHORT *usHostbufaddr,
						MMPD_GRAPHICS_COLORDEPTH colordepth, MMP_USHORT usHostLineOffset,
						MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
						MMPD_GRAPHICS_RECT *srcrect,
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode,
						MMPD_GRAPHICS_ROTATE_TYPE rotate);
						
MMP_ERR MMPD_Graphics_PaintDebugImage(MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                        MMPD_GRAPHICS_COLORPAT  *colorpat);

MMP_ERR MMPD_Graphics_SetScaleAttribute(MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                        MMP_ULONG ulScaleBufAddr, MMPD_GRAPHICS_COLORDEPTH incolormode,                       
						MMPD_GRAPHICS_RECT *rect, MMP_USHORT usUpscale, 
						MMPD_GRAPHICS_SCAL_SRC srcsel);
/// @end_ait_only
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _MMPD_GRAPHICS_H_

