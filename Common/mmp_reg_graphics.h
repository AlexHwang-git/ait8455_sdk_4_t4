//==============================================================================
//
//  File        : mmp_reg_graphics.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_GRAPHICS_H_
#define _MMP_REG_GRAPHICS_H_

#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/
//==============================================================================
//
//                              Retina Control Register Definitions
//
//==============================================================================

#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
// *******************************
//   GRA structure (0x8000 7900)
// *******************************
typedef struct _AITS_GRA {
    AIT_REG_B   GRA_SCAL_EN;                                           // 0x00
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_ST                 0x01
        #define GRA_YUV420_SCAL_ST          0x02
        #define GRA_YUV420_INTERLEAVE       0x04
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_SCAL_FLOW_CTL;                                      // 0x01
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_DLY_CTL            0x00
        #define GRA_SCAL_BUSY_CTL           0x01
        #define GRA_SCAL_Y_ONLY             0x02
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_SCAL_FMT;                                           // 0x02
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_FIFO_RGB565        0x02
        #define GRA_SCAL_FIFO_YUV422        0x03
        #define GRA_SCAL_MEM_YUV422         0x40
        #define GRA_SCAL_MEM_RGB565         0x00
        #define GRA_SCAL_MEM_RGB888         0x01
        #define GRA_SCAL_SRC_FMT_MASK       0x43
        #define GRA_SCAL_FIFO_MODE          0x02
        #define GRA_SCAL_Y_LOW              0x04
        #define GRA_SCAL_CR_FIRST           0x08
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_SCAL_UP_FACT;                                       // 0x03
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCALUP_MASK             0x1F
        #define GRA_SCALUP(_a)              (_a & GRA_SCALUP_MASK)
        /*------------------------------------------------------------*/
    AIT_REG_W   GRA_SCAL_FIFO_RST;                                      // 0x04
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_FIFO                0x0001
        #define GRA_SCAL_SCAL_FIFO           0x0100
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x106;
    AIT_REG_W   GRA_SCAL_INT_CPU_EN;                                     // 0x08
		/*-DEFINE-----------------------------------------------------*/
        #define GRA_FIFO_EMPTY              0x0001
		#define GRA_FIFO_FULL               0x0002
		#define GRA_FIFO_WR_TH              0x0004
		#define GRA_FIFO_RD_TH              0x0008
		#define GRA_YUV420_SCAL_DONE        0x0010		
		#define GRA_SCAL_DONE               0x0020
		/*------------------------------------------------------------*/
    AIT_REG_W   GRA_SCAL_INT_CPU_SR;
    AIT_REG_W   GRA_SCAL_INT_HOST_EN;
    AIT_REG_W   GRA_SCAL_INT_HOST_SR;

    AIT_REG_D   GRA_SCAL_ADDR_Y_ST;                                     // 0x10
    AIT_REG_D   GRA_SCAL_ADDR_U_ST;                                     // 0x14
    AIT_REG_D   GRA_SCAL_ADDR_V_ST;                                     // 0x18

    AIT_REG_W   GRA_SCAL_W;                                             // 0x1C
    AIT_REG_W   GRA_SCAL_H;                                             // 0x1E
    AIT_REG_B   GRA_SCAL_PIXL_DLY;                                      // 0x20
    AIT_REG_B                           _x21;
    AIT_REG_W   GRA_SCAL_LINE_DLY;                                      // 0x22
    AIT_REG_W   GRA_SCAL_FIFO_DATA;                                     // 0x24
    AIT_REG_W                           _x26;
    AIT_REG_B   GRA_SCAL_FIFO_WR_TH;                                    // 0x28
    AIT_REG_B   GRA_SCAL_FIFO_RD_TH;                                    // 0x29
    AIT_REG_W                           _x2A;
    AIT_REG_B   GRA_SCAL_FIFO_SR;                                       // 0x2C
	AIT_REG_B   	                    _x2D; 
    AIT_REG_B   GRA_SCAL_FIFO_WR_SPC;                                   // 0x2E
    AIT_REG_B   GRA_SCAL_FIFO_RD_SPC;									// 0x2F
		/*-DEFINE-----------------------------------------------------*/
        #define GRA_FIFO_DEPTH              0x10
		/*------------------------------------------------------------*/
    AIT_REG_D   GRA_SCAL_ADDR_Y_OFST;                                   // 0x30
    AIT_REG_D   GRA_SCAL_ADDR_U_OFST;                                   // 0x34
    AIT_REG_D   GRA_SCAL_ADDR_V_OFST;                                   // 0x38
	AIT_REG_D                           _x3C;
    AIT_REG_B   GRA_SCAL_FIFO_PTR;                                      // 0x40
} AITS_GRA, *AITPS_GRA;
#endif
#if (CHIP == P_V2)
// *******************************
//   GRA structure (0x8000 7800)
// *******************************
typedef struct _AITS_GRA {
    AIT_REG_W   GRA_BLT_EN;                                                 // 0x00
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_BLT_ST                  0x0001
        /*------------------------------------------------------------*/
    AIT_REG_W   GRA_BLT_FIFO_RST;
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_BLT_FIFO                0x0001
        /*------------------------------------------------------------*/
    AIT_REG_W   GRA_PAL_SWAP;
    AIT_REG_W                           _x06;
    AIT_REG_W   GRA_BLT_INT_CPU_EN;
		/*-DEFINE-----------------------------------------------------*/
        #define GRA_FIFO_EMPTY              0x0001
		#define GRA_FIFO_FULL               0x0002
		#define GRA_FIFO_WR_TH              0x0004
		#define GRA_FIFO_RD_TH              0x0008
		#define GRA_BLT_DONE                0x0010
		#define GRA_SCALING_DONE            0x0020
		#define GRA_DMA_DONE                0x0040
		#define GRA_FAST_MOVE_DONE          0x0080
		#define GRA_YUV420_SCALING_DONE     0x0100
		/*------------------------------------------------------------*/
    AIT_REG_W   GRA_BLT_INT_CPU_SR;
    AIT_REG_W   GRA_BLT_INT_HOST_EN;
    AIT_REG_W   GRA_BLT_INT_HOST_SR;

    AIT_REG_D   GRA_BLT_SRC_ADDR;                                           // 0x10
    AIT_REG_D   GRA_BLT_SRC_PITCH;
    AIT_REG_D   GRA_BLT_DST_ADDR;
    AIT_REG_D   GRA_BLT_DST_PITCH;

    AIT_REG_D   GRA_BLT_PAT_ADDR;                                       	// 0x20
    AIT_REG_D   GRA_BLT_PAT_PITCH;
    AIT_REG_D                           _x28[2];

    AIT_REG_W   GRA_BLT_FIFO_DATA;                                          // 0x30
    AIT_REG_W                           _x32;
    AIT_REG_W   GRA_BLT_FIFO_THR;                                        	// 0x34
	AIT_REG_W                           _x36;
    AIT_REG_B   GRA_BLT_FIFO_SR;
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_FIFO_DEPTH         		0x10
        #define GRA_FIFO_EPY                0x01
        #define GRA_FIFO_FUL                0x02
        #define GRA_FIFO_WR_U_TH            0x04
        #define GRA_FIFO_RD_O_TH            0x08
        /*------------------------------------------------------------*/
	AIT_REG_B   	                    _x39;
    AIT_REG_B   GRA_BLT_FIFO_WR_SPC;
		/*-DEFINE-----------------------------------------------------*/
		#define GRA_FIFO_MASK               0x1F
		/*------------------------------------------------------------*/
    AIT_REG_B   GRA_BLT_FIFO_RD_SPC;
    AIT_REG_W                           _x3D[2];

    AIT_REG_W   GRA_BLT_W;                                              // 0x40
    AIT_REG_W   GRA_BLT_H;
    AIT_REG_D   GRA_BLT_FG;
    AIT_REG_D   GRA_BLT_BG;
    AIT_REG_B   GRA_BLT_PAT_W;
    AIT_REG_B   GRA_BLT_PAT_H;
    AIT_REG_W                           _x4E;
    AIT_REG_B   GRA_BLT_FMT;                                            // 0x50
        /*-DEFINE-----------------------------------------------------*/
        #define	GRA_BIT_DAT_CON			0x80
        #define	GRA_BIT_DAT_NOT_CON		0x00
        #define	GRA_BIT_DAT_M_2_L		0x40
        #define	GRA_BIT_DAT_L_2_M		0x00
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_BLT_PAT_ST;
    AIT_REG_B   GRA_BLT_ROP;
        /*-DEFINE-----------------------------------------------------*/
        #define	GRA_ROP_MASK			0x0f
        #define	GRA_ROP_BLACKNESS		0x00
        #define	GRA_ROP_NOTSRCERASE		0x01	// ~(S+D)
        #define GRA_ROP_NOTSRCCOPY		0x03	// ~S
        #define	GRA_ROP_SRCERASE		0x04	// S.~D
        #define	GRA_ROP_DSTINVERT		0x05	// ~D
        #define GRA_ROP_SRCINVERT		0x06	// S^D
        #define GRA_ROP_SRCAND			0x08	// S.D
        #define GRA_ROP_MERGEPAINT		0x0B	// ~S+D
        #define	GRA_ROP_SRCCOPY			0x0C	// S
        #define	GRA_ROP_SRCPAINT		0x0E	// S+D
        #define	GRA_ROP_WHITENESS		0x0F
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_BLT_ROP_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define	GRA_OP_DO_ROP			0x00
        #define	GRA_OP_DO_TRANSP		0x01
        #define	GRA_COLOR_EXPANSION		0x02
        #define	GRA_MEM_2_HOST			0x00
        #define	GRA_HOST_2_MEM			0x04
        #define	GRA_MEM_2_MEM			0x08
        #define	GRA_PAT_2_DISPLAY		0x0C
        #define	GRA_SOLID_FILL			0x10
        #define	GRA_DRAW_PIXL			0x14
        #define	GRA_LEFT_TOP_RIGHT		0x00
        #define	GRA_RIGHT_TOP_LEFT		0x20
        #define	GRA_LEFT_BOT_RIGHT		0x40
        #define	GRA_RIGHT_BOT_LEFT		0x60
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_BLT_BLD_CTL;
    AIT_REG_B   GRA_BLT_BLD_WT;
    AIT_REG_W                           _x56;
    AIT_REG_W   GRA_LINE_DRAW_CTL;
    AIT_REG_W                           _x5A;
    AIT_REG_W   GRA_ROTE_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_ROTE_MODE	            0x01
        #define GRA_LINEOST_POS             0x02
        #define GRA_LINEOST_NEG             0x00
        /*------------------------------------------------------------*/
    AIT_REG_W   GRA_PIXEL_OFFSET;
    AIT_REG_W   GRA_PROBE_SEL;
    AIT_REG_W   GRA_FIFO_ERR;
    AIT_REG_W   GRA_LEFT_PIXL_CNT;
    AIT_REG_W   GRA_LINE_CNT;											//0x066
    AIT_REG_W   						_x68[76];
    AIT_REG_B   GRA_SCAL_EN;                                           // 0x100
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_ST                 0x01
        #define GRA_YUV420_SCAL_ST          0x02
        #define GRA_YUV420_INTERLEAVE       0x04
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_SCAL_FLOW_CTL;                                      // 0x101
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_BUSY_CTL           0x01
        #define GRA_SCAL_DLY_CTL            0x00
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_SCAL_FMT;                                           // 0x102
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_FIFO_RGB565        0x02
        #define GRA_SCAL_FIFO_YUV422        0x03
        #define GRA_SCAL_MEM_YUV422         0x40
        #define GRA_SCAL_MEM_RGB565         0x00
        #define GRA_SCAL_MEM_RGB888         0x01
        #define GRA_SCAL_FIFO_MODE          0x02
        #define GRA_SCAL_Y_LOW              0x04
        #define GRA_SCAL_CR_FIRST           0x08
        #define GRA_SCAL_FOUT_RGB565        0x10
        #define GRA_SCAL_FOUT_YUV422        0x00
        #define GRA_SCAL_MP3_PLAY           0x20
        /*------------------------------------------------------------*/
    AIT_REG_B   GRA_SCAL_UP_FACT;                                       // 0x103
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCALUP_MASK             0x1F
        #define GRA_SCALUP(_a)              (_a & GRA_SCALUP_MASK)
        /*------------------------------------------------------------*/
    AIT_REG_W   GRA_SCAL_FIFO_RST;                                      // 0x104
        /*-DEFINE-----------------------------------------------------*/
        #define GRA_SCAL_FIFO                0x0001
        #define GRA_SCAL_SCAL_FIFO           0x0100
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x106;
    AIT_REG_W   GRA_SCAL_INT_CPU_EN;                                     // 0x108
		/*-DEFINE-----------------------------------------------------*/
        #define GRA_FIFO_EMPTY              0x0001
		#define GRA_FIFO_FULL               0x0002
		#define GRA_FIFO_WR_TH              0x0004
		#define GRA_FIFO_RD_TH              0x0008
		#define GRA_YUV420_SCAL_DONE        0x0010		
		#define GRA_SCAL_DONE               0x0020
		/*------------------------------------------------------------*/
    AIT_REG_W   GRA_SCAL_INT_CPU_SR;
    AIT_REG_W   GRA_SCAL_INT_HOST_EN;
    AIT_REG_W   GRA_SCAL_INT_HOST_SR;

    AIT_REG_D   GRA_SCAL_ADDR_Y_ST;                                     // 0x110
    AIT_REG_D   GRA_SCAL_ADDR_U_ST;                                     // 0x114
    AIT_REG_D   GRA_SCAL_ADDR_V_ST;                                     // 0x118

    AIT_REG_W   GRA_SCAL_W;                                             // 0x11C
    AIT_REG_W   GRA_SCAL_H;                                             // 0x11E
    AIT_REG_B   GRA_SCAL_PIXL_DLY;                                      // 0x120
    AIT_REG_B                           _x121;
    AIT_REG_W   GRA_SCAL_LINE_DLY;                                      // 0x122
    AIT_REG_W   GRA_SCAL_FIFO_DATA;                                     // 0x124
    AIT_REG_W                           _x126;
    AIT_REG_W   GRA_SCAL_FIFO_THR;                                      // 0x128
    AIT_REG_W                           _x12A;
    AIT_REG_B   GRA_SCAL_FIFO_SR;                                       // 0x12C
	AIT_REG_B   	                    _x12D; 
    AIT_REG_B   GRA_SCAL_FIFO_WR_SPC;                                   // 0x12E
    AIT_REG_B   GRA_SCAL_FIFO_RD_SPC;									// 0x12F
} AITS_GRA, *AITPS_GRA;
#endif

#if	!defined(BUILD_FW)
// GRA OPR
#define GRA_BLT_EN                  (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_EN           )))
#define GRA_BLT_FIFO_RST    		(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FIFO_RST     )))
#define GRA_BLT_INT_HOST_EN 		(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_INT_HOST_EN  )))
#define GRA_BLT_INT_HOST_SR 		(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_INT_HOST_SR  )))
#define GRA_BLT_SRC_ADDR            (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_SRC_ADDR     )))
#define GRA_BLT_SRC_PITCH           (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_SRC_PITCH    )))
#define GRA_BLT_DST_ADDR            (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_DST_ADDR     )))
#define GRA_BLT_DST_PITCH           (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_DST_PITCH    )))
#define GRA_BLT_W                   (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_W            )))
#define GRA_BLT_H                   (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_H            )))
#define GRA_BLT_FG                  (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FG           )))
#define GRA_BLT_BG                  (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_BG           )))
#define GRA_BLT_FMT                 (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FMT          )))
#define GRA_BLT_ROP                 (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_ROP          )))
#define GRA_BLT_ROP_CTL             (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_ROP_CTL      )))
#define GRA_BLT_BLD_CTL             (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_BLD_CTL      )))

#define GRA_ROTE_CTL                (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_ROTE_CTL         )))
#define GRA_PIXEL_OFFSET            (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_PIXEL_OFFSET     )))

#define GRA_SCAL_FMT                (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FMT         )))
#define GRA_SCAL_PIXL_DLY           (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_PIXL_DLY    )))
#define GRA_SCAL_LINE_DLY           (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_LINE_DLY    )))
#define GRA_SCAL_UP_FACT            (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_UP_FACT     )))
#define GRA_SCAL_W                  (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_W           )))
#define GRA_SCAL_H                  (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_H           )))
#define GRA_BLT_FIFO_DATA           (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FIFO_DATA    )))
#define GRA_BLT_FIFO_THR            (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FIFO_THR     )))
#define GRA_BLT_FIFO_SR             (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FIFO_SR      )))
#define GRA_BLT_FIFO_WR_SPC         (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FIFO_WR_SPC  )))
#define GRA_BLT_FIFO_RD_SPC         (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_BLT_FIFO_RD_SPC  )))
#define GRA_JPG_ADDR                (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_JPG_ADDR         )))

#define	GRA_DMA_SRC_ADDR			(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_DMA_SRC_ADDR     )))
#define	GRA_DMA_SRC_OFST			(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_DMA_SRC_OFST     )))
#define	GRA_DMA_DST_ADDR			(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_DMA_DST_ADDR     )))
#define	GRA_DMA_DST_OFST			(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_DMA_DST_OFST     )))
#define	GRA_DMA_PIXL_W				(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_DMA_PIXL_W	   )))
#define	GRA_DMA_PIXL_H				(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_DMA_PIXL_H       )))
#define	GRA_DMA_CTL					(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_DMA_CTL          )))

#define GRA_SCAL_EN                 (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_EN          )))
#define GRA_SCAL_FIFO_RST	    	(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FIFO_RST    )))
#define GRA_SCAL_FLOW_CTL	    	(GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FLOW_CTL    )))
#define GRA_SCAL_ADDR_Y_ST   	    (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_ADDR_Y_ST   )))
#define GRA_SCAL_ADDR_U_ST   	    (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_ADDR_U_ST   )))
#define GRA_SCAL_ADDR_V_ST   	    (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_ADDR_V_ST   )))
#define GRA_SCAL_OFST_ST            GRA_SCAL_ADDR_U_ST

#define GRA_SCAL_FIFO_DATA          (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FIFO_DATA   )))
#define GRA_SCAL_FIFO_THR           (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FIFO_THR    )))
#define GRA_SCAL_FIFO_SR            (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FIFO_SR     )))
#define GRA_SCAL_FIFO_WR_SPC        (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FIFO_WR_SPC )))
#define GRA_SCAL_FIFO_RD_SPC        (GRA_BASE +(MMP_ULONG)(&(((AITPS_GRA )0)->GRA_SCAL_FIFO_RD_SPC )))
#endif
/// @}

#endif // _MMP_REG_SCALER_H_
