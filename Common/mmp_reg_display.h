//==============================================================================
//
//  File        : mmp_reg_display.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_DISPLAY_H_
#define _MMP_REG_DISPLAY_H_
///@ait_only
#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/

#if (CHIP == P_V2)
// *******************************
//   DSI structure (0x8000 6F00)
// *******************************

typedef struct _AITS_DSI {
    AIT_REG_B   DSI_INT_HOST_EN;                                        // 0x0000
    AIT_REG_B   DSI_INT_HOST_SR;
    AIT_REG_B   DSI_INT_CPU_EN;
    AIT_REG_B   DSI_INT_CPU_SR;
        /*-DEFINE-----------------------------------------------------*/
        #define CLANE_HSTX_READY            0x01
        #define DLANE_HSTX_DONE             0x02
        #define DLANE_TURN_ARUD_DONE        0x04
        #define DLANE_LPDT_BYTE_DONE        0x08
        #define DLANE_TX_STANDBY            0x10
        #define DLANE_RX_STANDBY            0x20
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x0004[6];
    AIT_REG_B   DSI_DSPY_BUF_TRIGGER;                                   // 0x0010
        /*-DEFINE-----------------------------------------------------*/
        #define LCD_DISP_BUF_EN             0x01
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x0011[15];
    AIT_REG_B   DSI_PACKET_ATTR;                                        // 0x0020
        /*-DEFINE-----------------------------------------------------*/
        #define LONG_PACKET_EN              0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   DSI_DATA_LANE_CNT;                                      // 0x0021
        /*-DEFINE-----------------------------------------------------*/
        #define ONE_DATA_LANE_EN            0x01
        #define TWO_DATA_LANE_EN            0x02
        #define DATA_LANE_CNT_MASK          0xFC
        /*------------------------------------------------------------*/
    AIT_REG_B   DSI_PACKET_CONFIG;                                      // 0x0022
        /*-DEFINE-----------------------------------------------------*/
        #define MAX_PACKETS_PER_HSTX        0x07
        #define DATA_TYPE_RGB565            0x00
        #define DATA_TYPE_RGB666            0x10
        #define DATA_TYPE_RGB666_LOOSE      0x20
        #define DATA_TYPE_RGB888            0x30
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x0023[1];
    AIT_REG_W   DSI_TRANSMIT_CNT;                                       // 0x0024
    AIT_REG_W   DSI_TRANSMIT_INTV;                                      // 0x0026
    AIT_REG_B   DSI_CLANE_HS_EN;                                        // 0x0028
    AIT_REG_B   DSI_DLANE_TRIGGER_MODE;                                 // 0x0029
        /*-DEFINE-----------------------------------------------------*/
        #define HS_TRANSMIT_MODE            0x01
        #define TURN_ARUD_MODE              0x02
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x002A[1];
    AIT_REG_B   DSI_LPDT_READ;                                          // 0x002C
    AIT_REG_B                           _x002D[19];
    AIT_REG_B   DSI_TLPX;                                               // 0x0040
    AIT_REG_B   DSI_TCLK_PREPARE;                                       // 0x0041
    AIT_REG_B   DSI_TCLK_ZERO;                                          // 0x0042
    AIT_REG_B   DSI_TCLK_PRE;                                           // 0x0043
    AIT_REG_B   DSI_TCLK_POST;                                          // 0x0044
    AIT_REG_B   DSI_TCLK_TRAIL;                                         // 0x0045
    AIT_REG_B   DSI_THS_PREPARE;                                        // 0x0046
    AIT_REG_B   DSI_THS_ZERO;                                           // 0x0047
    AIT_REG_B   DSI_THS_TRAIL;                                          // 0x0048
    AIT_REG_B   DSI_DPHY_CLK_SEL;                                       // 0x0049
        /*-DEFINE-----------------------------------------------------*/
        #define DPHY_1X_BYTE_CLK            0x00
        #define DPHY_2X_BYTE_CLK            0x01
        /*------------------------------------------------------------*/
} AITS_DSI, *AITPS_DSI;

// *******************************
//   Display structure (0x8000 7000)
// *******************************
typedef struct _AITS_DSPY {
    AIT_REG_W   DSPY_CTL_0;                                             // 0x0000
        /*-DEFINE-----------------------------------------------------*/
        #define LCD_PREVW_EN                0x0001
        #define LCD_FRAME_TX                0x0002
        #define LCD_IDX_RDY                 0x0004
        #define LCD_CMD_RDY                 0x0008
        #define LCD_TX_MASK                 0x000F
        #define LCD_FRAME_TX_SETADDR_EN     0x0010
        #define LCD_AUTO_TX_SETADDR_EN      0x0020
        #define LCD_PANL_READ_EN            0x0040
        #define LCD_BUSY_STS                0x0080
        #define SCD_DSPY_REG_READY          0x0100
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_CTL_2;                                             // 0x0002
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == P_V2)
        #define PRM_DSPY_DSI_SEL            0x4000
        #define SCD_DSPY_DSI_SEL            0x8000
        #endif
        #define PRM_DSPY_REG_READY          0x0001
        #define LCD_A0_DIS                  0x0002
        #define LCD_RD_DIS                  0x0004
        #define LCD_WR_DIS                  0x0008
        #define LCD_CS2_DIS                 0x0010
        #define LCD_CS1_DIS                 0x0020
        #define TV_FIEND_SYNC_EN            0x0040
        #define LCD_PREVW_3_BUF_EN          0x0080
        #define DSPY_TYPE_PL_LCD            0x0000
        #define DSPY_TYPE_SPI_LCD           0x01
        #define DSPY_TYPE_RGB_LCD           0x02
        #define DSPY_TYPE_TV                0x03
        #if (CHIP == P_V2)
        #define DSPY_TYPE_DSI               0x04
        #endif
        #define DSPY_PRM_SEL_SHIFT          8
        #define DSPY_PRM_SEL(_a)            (MMP_USHORT)(_a << DSPY_PRM_SEL_SHIFT)
        #define DSPY_PRM_SEL_MASK           0x0300
        #define DSPY_PRM_EN                 0x1000
        #define DSPY_SCD_SEL_SHIFT          10
        #define DSPY_SCD_SEL(_a)            (MMP_USHORT)(_a << DSPY_SCD_SEL_SHIFT)
        #define DSPY_SCD_SEL_MASK           0x0C00
        #define DSPY_SCD_EN                 0x2000
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_INT_HOST_EN;                                       // 0x0004
    AIT_REG_W   DSPY_INT_HOST_SR;
    AIT_REG_W   DSPY_INT_CPU_EN;
    AIT_REG_W   DSPY_INT_CPU_SR;
        /*-DEFINE-----------------------------------------------------*/
        #define PRM_FRME_TX_END             0x0001
        #define PRM_IDX_TX_END              0x0002
        #define PRM_CMD_TX_END              0x0004
        #define SCD_FRME_TX_END             0x0008
        #define SCD_IDX_TX_END              0x0010
        #define SCD_CMD_TX_END              0x0020
        #define VSYNC_ACTIVE                0x0040
        #define RGB_FRME_CNT_HIT            0x0080
        #define TV_LINE_INT1                0x0100
        #define TV_LINE_INT2                0x0200
        #define TV_2_DSPY_BUF_UF            0x0400
        #define TV_ODFIEL_START             0x0800
        #define TV_EVFIEL_START             0x1000
        #define TV_CPOUT_L_2_H              0x2000
        #define TV_CPOUT_H_2_L              0x4000
        /*------------------------------------------------------------*/
    AIT_REG_B   DSPY_LCD_FLM_CTL;                                       // 0x000C
    AIT_REG_B   DSPY_LCD_VSYNC_CTL;                                     
    AIT_REG_W   DSPY_FLM_VSYNC_CNT;                                     // 0x000E

    AIT_REG_D   DSPY_LCD_TX_0;                                          // 0x0010
    AIT_REG_D   DSPY_LCD_TX_1;
    AIT_REG_D   DSPY_LCD_TX_2;
    AIT_REG_D   DSPY_LCD_TX_3;

    AIT_REG_D   DSPY_LCD_TX_4;                                          // 0x0020
    AIT_REG_D   DSPY_LCD_TX_5;
    AIT_REG_D   DSPY_LCD_TX_6;
    AIT_REG_W   DSPY_LCD_AUTO_CFG;
        /*-DEFINE-----------------------------------------------------*/
        #define AUTO_TX_TYPE_IDX            0x0000
        #define AUTO_TX_TYPE_CMD            0x0001
        #define LCD_TX_TYPE_IDX(_n)         (AUTO_TX_TYPE_IDX << _n)
        #define LCD_TX_TYPE_CMD(_n)         (AUTO_TX_TYPE_CMD << _n)
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x002E;

    AIT_REG_W   DSPY_PLCD_CTL;                                          // 0x0030
        /*-DEFINE-----------------------------------------------------*/
        #define PLCD_BUS_8BPP             0x0000
        #define PLCD_BUS_16BPP            0x0001
        #define PLCD_BUS_18BPP            0x0002
        #define PLCD_BUS_12BPP            0x0003
        #define PLCD_BUS_MASK             0x0003
        #define PLCD_68_RC_HIGH           0x0004
        #define PLCD_CMD_BURST            0x0008
        #define PLCD_PH0                  0x0000
        #define PLCD_PH1                  0x0010
        #define PLCD_PO0                  0x0000
        #define PLCD_PO1                  0x0020
        #define PLCD_TYPE_80              0x0040
        #define PLCD_TYPE_68              0x0000
        #define LCD_PL1_SPI2              0x0100
        #define LCD_PL1_PL2               0x0200
        #define PLCD_RS_B4_CS             0x0400
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_PLCD_FMT;
        /*-DEFINE-----------------------------------------------------*/
        #define PLCD_RGB565_B16           0x0000
        #define PLCD_RGB444_B16           0x0001
        #define PLCD_RGB666_B16           0x0002
        #define PLCD_RGB888_B8            0x0003
        #define PLCD_RGB332_B8            0x0004
        #define PLCD_RGB444_B8            0x0005
        #define PLCD_RGB666_B8            0x0006
        #define PLCD_RGB565_B8            0x0007
        #define PLCD_RGB666_B18           0x0008
        #define PLCD_RGB666_B9_9          0x0009
        #define PLCD_RGB666_B2_16         0x000A
        #define PLCD_RGB666_B16_2         0x000B
        #define LCD_RGB24_B16_8             0x000C
        #define LCD_RGB24_B8_16             0x000D
        #define LCD_RGB666_B8_2_7           0x0010
        #define LCD_RBG444_B12_16           0x0020
        #define LCD_RBG444_B12_16_15_14     0x0040
        #define LCD_RGBB9_9_17              0x0080

        #define LCD_RGB666_B9               0x0100
        #define LCD_RGB666_B2_16            0x0200
        #define LCD_RGB666_B16_2            0x0300
        #define LCD_B8_RGB666_12_17         0x0400
        #define LCD_B16_1_8_10_17           0x0800
        #define LCD_B8_10_17                0x1000
        #define LCD_B8_1_8                  0x2000
        #define LCD_B8_RGB565_G_LSB_FIRST   0x4000
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_LCD_SR;
        /*-DEFINE-----------------------------------------------------*/
        #define PL_LCD_BUSY                 0x0001
        #define SPI_LCD_BUSY                0x0002
        #define TV_PLUG_STATUS              0x0008
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_TV_LINE;                                         	// 0x0036
    AIT_REG_W   DSPY_PLCD_RS_CS_CYC;
    AIT_REG_W   DSPY_PLCD_CS_RW_CYC;
    AIT_REG_W   DSPY_PLCD_RW_CYC;
    AIT_REG_W   DSPY_PLCD_IDX_CMD_NUM;

    AIT_REG_W   DSPY_W;                                                 // 0x0040
    AIT_REG_W                           _x0042;
    AIT_REG_W   DSPY_H;
    AIT_REG_W                           _x0046;
    AIT_REG_D   DSPY_PIXL_CNT;
    AIT_REG_W   DSPY_CTL_4;
        /*-DEFINE-----------------------------------------------------*/
        #define LCD_OUT_RGB                 0x0000
        #define LCD_OUT_BGR                 0x0002
        #define LCD_OUT_SEL_NONE            0x0000
        #define LCD_OUT_SEL_LCD1            0x0004
        #define LCD_OUT_SEL_LCD2            0x0008
        #define LCD_OUT_SEL_MASK            0x000C
        #define LCD_BG_COLR_565             0x0000
        #define LCD_BG_COLR_888             0x0020
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_WIN_PRIO;
        /*-DEFINE-----------------------------------------------------*/
        #define MAIN_WIN                0x0
        #define PIP_WIN                 0x1
        #define OVLY_WIN                0x2
        #define ICON_WIN                0x3
        #define WIN_1_SHFT              0
        #define WIN_2_SHFT              2
        #define WIN_3_SHFT              4
        #define WIN_4_SHFT              6
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_ICON_W;                                             // 0x0050 //
    AIT_REG_W                           _x0052;
    AIT_REG_W   DSPY_ICON_H;
    AIT_REG_W                           _x0056;
    AIT_REG_W   DSPY_ICON_X;
    AIT_REG_W                           _x005A;
    AIT_REG_W   DSPY_ICON_Y;
    AIT_REG_W                           _x005E;

    AIT_REG_W   DSPY_ICON_CTL;                                           // 0x0060
    AIT_REG_W   DSPY_BG_SEMITP_WT;
    AIT_REG_D   DSPY_BG_COLR;
    AIT_REG_D   DSPY_PLCD_READ;
    AIT_REG_B   DSPY_FIFO_CLR;
        /*-DEFINE-----------------------------------------------------*/
        #define MAIN_FIFO_CLR           0x01
        #define PIP_FIFO_CLR            0x02
        #define OVLY_FIFO_CLR           0x04
        #define DSPY_BUF_CLR            0x08
        #define SCAL_BUF_CLR            0x10
        #define OVLY_RGB_OUT_CLR        0x20
        #define SCD_DSPY_BUF_CLR        0x40
        /*------------------------------------------------------------*/
      
    AIT_REG_B   DSPY_WIN_FIFO_CX;
        /*-DEFINE-----------------------------------------------------*/
#if (CHIP == P_V2)
        #define PIP_DROP_LINE_FOR_TV   0x00
        #define PIP_DO_NOT_DROP_LINE   0x08
        #define D32BIT_ARGB_DISABLE    0x00
        #define D32BIT_ARGB_ENABLE     0x04
        #define FETCH_DATA_BY_PROG     0x00
        #define FETCH_DATA_BY_INTER    0x02
        #define FIFO_CX_RESET          0x00
#endif
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_MISC_PROBE_SEL;
    AIT_REG_W   TVENC[8*9];                                             // 0x0070-0x00FF

    AIT_REG_D   DSPY_MAIN_0_ADDR_ST;                                    // 0x0100
    AIT_REG_D   DSPY_MAIN_1_ADDR_ST;                                    // 0x0104
    AIT_REG_D   DSPY_MAIN_OFST_ST;										// 0x0108
    AIT_REG_D   DSPY_MAIN_2_ADDR_ST;                                    // 0x010C

    AIT_REG_W   DSPY_MAIN_OFST_PIXL;                                    // 0x0110
        /*-DEFINE-----------------------------------------------------*/
        #define WIN_OFST_NEG            0x8000
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x0112;
    AIT_REG_W   DSPY_MAIN_OFST_ROW;
    AIT_REG_W                           _x0116[5];
    AIT_REG_D   DSPY_MAIN_0_U_ADDR_ST;                                   // 0x0120
    AIT_REG_D   DSPY_MAIN_1_U_ADDR_ST;
    AIT_REG_D   DSPY_MAIN_OFST_UV_ST;
    AIT_REG_D   DSPY_MAIN_2_U_ADDR_ST;

    AIT_REG_W   DSPY_MAIN_OFST_UV_PIXL;                                  // 0x0130
    AIT_REG_W                           _x0132;
    AIT_REG_W   DSPY_MAIN_OFST_UV_ROW;
    AIT_REG_W                           _x0136[5];

    AIT_REG_D   DSPY_MAIN_0_V_ADDR_ST;                                   // 0x0140
    AIT_REG_D   DSPY_MAIN_1_V_ADDR_ST;                                   
    AIT_REG_D                           _x0148;
    AIT_REG_D   DSPY_MAIN_2_V_ADDR_ST;

    AIT_REG_W   DSPY_MAIN_W;                                             // 0x0150
    AIT_REG_W                           _x0152;
    AIT_REG_W   DSPY_MAIN_H;
    AIT_REG_W                           _x0156;
    AIT_REG_W   DSPY_MAIN_X;
    AIT_REG_W                           _x015A;
    AIT_REG_W   DSPY_MAIN_Y;
    AIT_REG_W                           _x015E;

    AIT_REG_W   DSPY_MAIN_CTL;                                           // 0x0160
        /*-DEFINE-----------------------------------------------------*/
        #define WIN_EN                  0x0001
        #define WIN_BURST_DIS           0x0002
        #define WIN_SRC_GRAB_EN         0x0004
        #define WIN_YUV_SCALUP_EN       0x0008
        #define WIN_DBL_BUF_EN          0x0010
        #define WIN_IN_RGB              0x0000
        #define WIN_IN_BGR              0x0040
        #define WIN_OUT_565_EX_888      0x0080
        #define WIN_YUV420_DITHER_18    0x0100
        #define WIN_420_RING_BUF_EN     0x0200
        #define WIN_YUV_LPF_EN          0x0400
        #define WIN_YUV420_DITHER_16    0x0800
        #define WIN_YUV420_DITHER_12    0x1000
        #define WIN_YUV420_INTERLEAVE_UV    0x8000
        #define WIN_YUV420_INTERLEAVE_VU    0xC000
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_MAIN_CTL_2;                                         // 0x0162
        /*-DEFINE-----------------------------------------------------*/
#if (CHIP == P_V2)
		#define WIN_ALPHA_RGBA_FMT	0x0080
		#define WIN_ALPHA_ARGB_FMT	0x0000
		#define WIN_ALPHA_FMT_MASK	0x0080		
		#define WIN_ALPHA_EN        0x0040
#endif
        #define WIN_V_1X            0x0001
        #define WIN_V_2X            0x0002
        #define WIN_V_4X            0x0004
        #define WIN_H_1X            0x0008
        #define WIN_H_2X            0x0010
        #define WIN_H_4X            0x0020
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_MAIN_FMT;
        /*-DEFINE-----------------------------------------------------*/
        #define WIN_4BPP                0x0001                      // MAIN,      OVLY
        #define WIN_8BPP                0x0002                      // MAIN,      OVLY
        #define WIN_16BPP               0x0004                      // MAIN, PIP, OVLY
        #define WIN_24BPP               0x0008                      //       PIP,
        #define WIN_YUV420              0x0010                      //       PIP, OVLY
        #define WIN_YUV422              0x0020                      //       PIP, OVLY
        #define WIN_32BPP               0x0040                      //       PIP, OVLY
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x0166;
    AIT_REG_D   DSPY_MAIN_PIXL_CNT;
    AIT_REG_W                           _x016C[2];

    AIT_REG_W   DSPY_MAIN_TP_CTL;                                        // 0x0170
        /*-DEFINE-----------------------------------------------------*/
        #define WIN_TP_EN                   0x0001
        #define WIN_SEMITP_EN               0x0002
        #define WIN_SEMITP_SHFT             2
        #define WIN_SEMITP_AVG              (0x00 << WIN_SEMITP_SHFT)
        #define WIN_SEMITP_AND              (0x01 << WIN_SEMITP_SHFT)
        #define WIN_SEMITP_OR               (0x02 << WIN_SEMITP_SHFT)
        #define WIN_SEMITP_INV              (0x03 << WIN_SEMITP_SHFT)
#if (CHIP == P_V2)
        #define WIN_SEMITP_MASK             0x001C
#else
        #define WIN_SEMITP_MASK             0x000C
#endif

#if (CHIP == P_V2)        
        //++P_V2
        #define WIN_ALPHA_LSB               0x0020
        #define WIN_ALPHA_MSB               0x0000
        
        #define WIN_GLOBAL_ALPHA_BLEND      0x0000
        #define WIN_GLOBAL_LOCAL_BLEND      0x0004
        #define WIN_ALPHA_BLEND_OP_AND      0x0008
        #define WIN_ALPHA_BLEND_OP_OR       0x000C
        
        #define WIN_ALPHA_BLEND_EN          0x0002
        #define WIN_ALPHA_BLEND_DIS         0x0000
        //++P_V2
#endif        
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_MAIN_SEMITP_WT;
    AIT_REG_D   DSPY_MAIN_TP_COLR;
    AIT_REG_W                           _x0178[4];

    AIT_REG_W                           _x0180[8];                      // 0x0180-0x018F
    AIT_REG_B   DSPY_MAIN_UV_GAIN_11;                                    // 0x0190
    AIT_REG_B   DSPY_MAIN_UV_GAIN_12;
    AIT_REG_B   DSPY_MAIN_UV_GAIN_21;
    AIT_REG_B   DSPY_MAIN_UV_GAIN_22;
    AIT_REG_W   DSPY_MAIN_RGB_GAIN;
    AIT_REG_W   DSPY_MAIN_RGB_OFST;
    AIT_REG_W                           _x0198[4];
    

    AIT_REG_W   DSPY_SCD_CTL;						                    // 0x01A0
        /*-DEFINE-----------------------------------------------------*/
		/* The same as DSPY_CTL_0
        #define LCD_FRAME_TX                0x0002
        #define LCD_IDX_RDY                 0x0004
        #define LCD_CMD_RDY                 0x0008
        #define LCD_TX_MASK                 0x000F
        #define LCD_FRAME_TX_SETADDR_EN     0x0010
        #define LCD_PANL_READ_EN            0x0040
        */
        #define	SCD_SRC_RGB888				0x0100
        #define	SCD_SRC_RGB565				0x0200
        #define	SCD_SRC_FMT_MASK			0x0300
        #define	SCD_565_2_888_STUFF_0		0x0000
        #define	SCD_565_2_888_STUFF_MSB		0x0400
        /*------------------------------------------------------------*/
    AIT_REG_B   DSPY_SCD_FLM_CTL;                                       // 0x01A4
    AIT_REG_B   DSPY_SCD_VSYNC_CTL;                                     // 0x01A5	
    AIT_REG_W   DSPY_SCD_FLM_VSYNC_CNT;                                 // 0x01A6
    AIT_REG_D   DSPY_SCD_BG_COLR;                                 		// 0x01A8
    AIT_REG_D   DSPY_SCD_PLCD_READ;										// 0x01AC

    AIT_REG_D   DSPY_SCD_LCD_TX_0;                                      // 0x01B0
    AIT_REG_D   DSPY_SCD_LCD_TX_1;										// 0x01B4
    AIT_REG_D   DSPY_SCD_LCD_TX_2;										// 0x01B8
    AIT_REG_W   DSPY_SCD_LCD_AUTO_CFG;									// 0x01BC
    AIT_REG_W                           _x01BE;
    AIT_REG_W   DSPY_SCD_W;                                             // 0x01C0
    AIT_REG_W                           _x01C2;
    AIT_REG_W   DSPY_SCD_H;												// 0x01C4
    AIT_REG_W                           _x01C6;
    AIT_REG_D   DSPY_SCD_PIXL_CNT;										// 0x01C8
    AIT_REG_D   DSPY_SCD_WIN_PIXL_CNT;									// 0x01CC
    AIT_REG_D   DSPY_SCD_WIN_ADDR_ST;                                 	// 0x01D0
    AIT_REG_D   DSPY_SCD_WIN_OFST_ST;
    AIT_REG_W   DSPY_SCD_WIN_W;                                         // 0x01D8
    AIT_REG_W   DSPY_SCD_WIN_H;
    AIT_REG_W   DSPY_SCD_WIN_X;
    AIT_REG_W   DSPY_SCD_WIN_Y;
    AIT_REG_D   DSPY_MAIN_TV_EVFIELD_ST;                                // 0x01E0
    AIT_REG_D                           _x01E4;
	AIT_REG_D	DSPY_WIN_ALPHA_WT_1;
	AIT_REG_D	DSPY_WIN_ALPHA_WT_2;
        /*-DEFINE-----------------------------------------------------*/
		#define ALPHA_W_1(l, w)  ((w | ((8-w) << 4)) << (l * 8))    
		#define ALPHA_W_2(l, w)  ((w | ((8-w) << 4)) << ((l - 4)* 8))    
        /*------------------------------------------------------------*/

    AIT_REG_W   DSPY_MAIN_FIFO_THR;                                     // 0x01F0
    AIT_REG_W                           _x01F2[3];
    AIT_REG_W   DSPY_MAIN_U_FIFO_THR;                                    // 0x01F8
    AIT_REG_W   DSPY_MAIN_V_FIFO_THR;
    AIT_REG_W                           _x01FC[2];


    AIT_REG_D   DSPY_PIP_0_ADDR_ST;                                      // 0x0200
    AIT_REG_D   DSPY_PIP_1_ADDR_ST;

    AIT_REG_D   DSPY_PIP_OFST_ST;
    AIT_REG_D   DSPY_PIP_2_ADDR_ST;

    AIT_REG_W   DSPY_PIP_OFST_PIXL;                                      // 0x0210
    AIT_REG_W                           _x0212;
    AIT_REG_W   DSPY_PIP_OFST_ROW;
    AIT_REG_W                           _x0216[5];

    AIT_REG_D   DSPY_PIP_0_U_ADDR_ST;                                    // 0x0220
    AIT_REG_D   DSPY_PIP_1_U_ADDR_ST;
    AIT_REG_D   DSPY_PIP_OFST_UV_ST;
    AIT_REG_D   DSPY_PIP_2_U_ADDR_ST;
    AIT_REG_W   DSPY_PIP_OFST_UV_PIXL;                                   // 0x0230
    AIT_REG_W                           _x0232;
    AIT_REG_W   DSPY_PIP_OFST_UV_ROW;
    AIT_REG_W                           _x0236[5];
 
    AIT_REG_D   DSPY_PIP_0_V_ADDR_ST;                                    // 0x0240
    AIT_REG_D   DSPY_PIP_1_V_ADDR_ST;
    AIT_REG_D                           _x0248;
    AIT_REG_D   DSPY_PIP_2_V_ADDR_ST;

    AIT_REG_W   DSPY_PIP_W;                                              // 0x0250
    AIT_REG_W                           _x0252;
    AIT_REG_W   DSPY_PIP_H;
    AIT_REG_W                           _x0256;
    AIT_REG_W   DSPY_PIP_X;
    AIT_REG_W                           _x025A;
    AIT_REG_W   DSPY_PIP_Y;
    AIT_REG_W                           _x025E;

    AIT_REG_W   DSPY_PIP_CTL;                                            // 0x0260
    AIT_REG_W   DSPY_PIP_CTL_2;                                          // 0x0262
    AIT_REG_W   DSPY_PIP_FMT;
    AIT_REG_W                           _x0266;
    AIT_REG_D   DSPY_PIP_PIXL_CNT;
    AIT_REG_D   DSPY_SOUT_GRAB_PIXL_CNT;

    AIT_REG_W   DSPY_PIP_TP_CTL;                                         // 0x0270
    AIT_REG_W   DSPY_PIP_SEMITP_WT;
    AIT_REG_D   DSPY_PIP_TP_COLR;
    AIT_REG_W                           _x0278[4];

    // For PIP only
    AIT_REG_W   DSPY_SCAL_CTL;                                           // 0x0280
        /*-DEFINE-----------------------------------------------------*/
        #define DSPY_SCAL_NM                 0x0000
        #define DSPY_SCAL_BYPASS             0x0001      // bypass N/M and LPF
        #define DSPY_LRP_DYN                 0x0000
        #define DSPY_LRP_FIX                 0x0002
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_SOUT_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define DSPY_SOUT_YUV_EN             0x0001
        #define DSPY_SOUT_RGB_EN             0x0002
        #define DSPY_SOUT_YUV_444            0x0000
        #define DSPY_SOUT_YUV_422            0x0008
        #define DSPY_SOUT_RGB_888            0x0000
        #define DSPY_SOUT_RGB_565            0x0010
        #define DSPY_SOUT_RGB_666            0x0020
        #define DSPY_SOUT_DITHER_EN   	     0x0040
        #define DSPY_SOUT_RGB            	 0x0000
        #define DSPY_SOUT_BGR           	 0x0080
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_SCAL_H_N;
    AIT_REG_W   DSPY_SCAL_H_M;
    AIT_REG_W   DSPY_SCAL_V_N;
    AIT_REG_W   DSPY_SCAL_V_M;
    AIT_REG_W   DSPY_SEDGE_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define DSPY_SEDGE_BYPASS             0x0001
        #define DSPY_SEDGE_YUV_AVG            0x0002
        /*------------------------------------------------------------*/
    AIT_REG_B   DSPY_SEDGE_GAIN_VAL; // 0x08
    AIT_REG_B   DSPY_SEDGE_CORE_VAL; // 0x04

    AIT_REG_B   DSPY_PIP_UV_GAIN_11;                                     // 0x0290
    AIT_REG_B   DSPY_PIP_UV_GAIN_12;
    AIT_REG_B   DSPY_PIP_UV_GAIN_21;
    AIT_REG_B   DSPY_PIP_UV_GAIN_22;
    AIT_REG_W   DSPY_PIP_RGB_GAIN;
    AIT_REG_W   DSPY_PIP_RGB_OFST;
    AIT_REG_W   DSPY_SCAL_H_WT;
    AIT_REG_W   DSPY_SCAL_V_WT;
        /*-DEFINE-----------------------------------------------------*/
        #define DSPY_SCAL_WT_AVG 	         0x0001        // default
        #define DSPY_SCAL_WT_MASK             0xFFFE
        #define DSPY_SCAL_WT_SHFT             1
        /*------------------------------------------------------------*/
    AIT_REG_W   DSPY_SCAL_WT_ONE_EN;
    AIT_REG_W                           _x029E;

    AIT_REG_W   DSPY_SIN_W;                                              // 0x02A0
    AIT_REG_W                           _x02A2;
    AIT_REG_W   DSPY_SIN_H;
    AIT_REG_W                           _x02A6;
    AIT_REG_W                           _x02A8[4];

    AIT_REG_W   DSPY_SOUT_GRAB_H_ST;                                     // 0x02B0
    AIT_REG_W                           _x02B2;
    AIT_REG_W   DSPY_SOUT_GRAB_H_ED;
    AIT_REG_W                           _x02B6;
    AIT_REG_W   DSPY_SOUT_GRAB_V_ST;
    AIT_REG_W                           _x02BA;
    AIT_REG_W   DSPY_SOUT_GRAB_V_ED;
    AIT_REG_W                           _x02BE;

    AIT_REG_D                           _x02C0[4];
    AIT_REG_W   DSPY_YUV420_ROW_CNT;                                    // 0x02D0
    AIT_REG_W                           _x02D2[7];

    AIT_REG_D   DSPY_PIP_TV_EVFIELD_ST;                                 // 0x02E0
    AIT_REG_D                           _x02E4[3];
    AIT_REG_W   DSPY_PIP_FIFO_THR;                                      // 0x02F0
    AIT_REG_W   DSPY_SCAL_BUF_FULL_THR;                                 // 0x02F2                    
    AIT_REG_W   DSPY_SCAL_IN_THR;                                       // 0x02F4        
    AIT_REG_W   DSPY_FIFO_THR;                                          // 0x02F6
    AIT_REG_B   DSPY_PIP_U_FIFO_THR;                                    // 0x02F8
    #if (CHIP == P_V2)
    // ++ PYTHON V1
    AIT_REG_B   DSPY_RGB_SYNC_MODE_V1;                                      // 0x02F9
        /*-DEFINE-----------------------------------------------------*/
        #define DSPY_RGB_SYNC_MODE_EN 	         0x01
        #define DSPY_RGB_SYNC_MODE_DIS           0x00
        /*------------------------------------------------------------*/
    // -- PYTHON V1
    #endif
    AIT_REG_B   DSPY_PIP_V_FIFO_THR;
    AIT_REG_B                           _x02FB;
    AIT_REG_W   DSPY_FIFO_SRAM_PARAM;
    AIT_REG_W   DSPY_PIP_FIFO_SRAM_PARAM;

    AIT_REG_D   DSPY_OVLY_0_ADDR_ST;                                     // 0x0300
    AIT_REG_D   DSPY_OVLY_1_ADDR_ST;                                     // 0x0300
    AIT_REG_D   DSPY_OVLY_OFST_ST;
    AIT_REG_D   DSPY_OVLY_2_ADDR_ST;                                     // 0x0300

    AIT_REG_W   DSPY_OVLY_OFST_PIXL;                                     // 0x0310
    AIT_REG_W                           _x0312;
    AIT_REG_W   DSPY_OVLY_OFST_ROW; 
    AIT_REG_W                           _x0316[5];

    AIT_REG_D   DSPY_OVLY_0_U_ADDR_ST;                                   // 0x0320
    AIT_REG_D   DSPY_OVLY_1_U_ADDR_ST;
    AIT_REG_D   DSPY_OVLY_OFST_UV_ST;
    AIT_REG_D   DSPY_OVLY_2_U_ADDR_ST;

    AIT_REG_W   DSPY_OVLY_OFST_UV_PIXL;                                  // 0x0330
    AIT_REG_W                           _x0332;
    AIT_REG_W   DSPY_OVLY_OFST_UV_ROW;
    AIT_REG_W                           _x0336[5];

    AIT_REG_D   DSPY_OVLY_0_V_ADDR_ST;                                   // 0x0340
    AIT_REG_D   DSPY_OVLY_1_V_ADDR_ST;                                   
    AIT_REG_D                           _x0348;
    AIT_REG_D   DSPY_OVLY_2_V_ADDR_ST;                                   

    AIT_REG_W   DSPY_OVLY_W;                                             // 0x0350
    AIT_REG_W                           _x0352;
    AIT_REG_W   DSPY_OVLY_H;
    AIT_REG_W                           _x0356;
    AIT_REG_W   DSPY_OVLY_X;
    AIT_REG_W                           _x035A;
    AIT_REG_W   DSPY_OVLY_Y;
    AIT_REG_W                           _x035E;

    AIT_REG_W   DSPY_OVLY_CTL;                                           // 0x0360
    AIT_REG_W   DSPY_OVLY_CTL_2;                                         // 0x0362
    AIT_REG_W   DSPY_OVLY_FMT;
    AIT_REG_W                           _x0366;
    AIT_REG_D   DSPY_OVLY_PIXL_CNT;
    AIT_REG_W                           _x036C[2];

    AIT_REG_W   DSPY_OVLY_TP_CTL;                                        // 0x0370
    AIT_REG_W   DSPY_OVLY_SEMITP_WT;
    AIT_REG_D   DSPY_OVLY_TP_COLR;
    AIT_REG_W                           _x0378[4];
    AIT_REG_W   DSPY_RGB_LINE_CPU_INT_1;                                 // 0x0380
    AIT_REG_W   DSPY_RGB_LINE_CPU_INT_2;                                 // 0x0382
    AIT_REG_W   DSPY_RGB_LINE_HOST_INT_1;                                // 0x0384
    AIT_REG_W   DSPY_RGB_LINE_HOST_INT_2;                                // 0x0386
    AIT_REG_B   DSPY_INT_HOST_EN_2;                                      // 0x0388
    AIT_REG_B   DSPY_INT_CPU_EN_2;                                       // 0x0389
    AIT_REG_B   DSPY_INT_HOST_SR_2;                                      // 0x038A
    AIT_REG_B   DSPY_INT_CPU_SR_2;                                       // 0x038B
        /*-DEFINE-----------------------------------------------------*/
        #define LCD_WRITE_BACK_DONE         0x08
        #define RGB_LINE_INT2               0x04
        #define RGB_LINE_INT1               0x02
        #define HSYNC_ACTIVE	            0x01
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x038C[2];                       // 0x0388
    AIT_REG_B   DSPY_OVLY_UV_GAIN_11;                                    // 0x0390
    AIT_REG_B   DSPY_OVLY_UV_GAIN_12;
    AIT_REG_B   DSPY_OVLY_UV_GAIN_21;
    AIT_REG_B   DSPY_OVLY_UV_GAIN_22;
    AIT_REG_W   DSPY_OVLY_RGB_GAIN;
    AIT_REG_W   DSPY_OVLY_RGB_OFST;
    AIT_REG_D                           _x0398[2];						// 0x0398-0x039F
	AIT_REG_B	DSPY_RGB_CTL;											// 0x03A0
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == P_V2)
        #define DDRCLK_POLAR_NORMAL     0x00
        #define DDRCLK_POLAR_INVERT     0x80
        #define DOTCLK_NORMAL_MODE      0x00
        #define DOTCLK_DDR_MODE         0x40
        #endif
        #define HSYNC_POLAR_LOW	       	0x00
        #define HSYNC_POLAR_HIGH        0x20
        #define VSYNC_POLAR_LOW         0x00
        #define VSYNC_POLAR_HIGH        0x10
        #define DOT_POLAR_PST           0x00
        #define DOT_POLAR_NEG           0x08
		#define	DEN_DATA_MODE			0x00
		#define	PRT_DATA_MODE			0x04
        #define PARTIAL_MODE_EN			0x02
        #define RGB_IF_EN		        0x01
        /*------------------------------------------------------------*/
	AIT_REG_B	DSPY_RGB_SPI_CTL;										// 0x03A1	
        /*-DEFINE-----------------------------------------------------*/
        #define SPICLK_POLAR_PST            0x00
        #define SPICLK_POLAR_NEG            0x08
        #define SPI_START_ID                0x01
        /*------------------------------------------------------------*/
	AIT_REG_B	DSPY_RGB_FMT;											// 0x03A2	
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == P_V2)
        #define YUV422_D8BIT_Y1VY0U         0x27
        #define YUV422_D8BIT_Y1UY0V         0x26
        #define YUV422_D8BIT_Y0VY1U         0x25
        #define YUV422_D8BIT_Y0UY1V         0x24
        #define YUV422_D8BIT_VY1UY0         0x23
        #define YUV422_D8BIT_UY1VY0         0x22
        #define YUV422_D8BIT_VY0UY1         0x21
        #define YUV422_D8BIT_UY0VY1         0x20
        #endif
        #define RGB_D24BIT_BGR332			0x0D
        #define RGB_D24BIT_BGR333			0x0C
        #define RGB_D24BIT_BGR444			0x0B
        #define RGB_D24BIT_BGR565			0x0A
        #define RGB_D24BIT_BGR666			0x09
        #define RGB_D24BIT_BGR888			0x08
        #define RGB_D24BIT_RGB332			0x05
        #define RGB_D24BIT_RGB333			0x04
        #define RGB_D24BIT_RGB444			0x03
        #define RGB_D24BIT_RGB565			0x02
        #define RGB_D24BIT_RGB666			0x01
        #define RGB_D24BIT_RGB888			0x00
        /*------------------------------------------------------------*/
    
    #if (CHIP == P_V2)
    AIT_REG_B   DSPY_RGB_SHARE_P_LCD_BUS;                                // 0x03A3
    /*-DEFINE-----------------------------------------------------*/
        #define P_LCD_ONLY                  0x00
        #define RGB_18BIT_SHARE_WITH_P_LCD  0x01
        #define RGB_24BIT_SHARE_WITH_P_LCD  0x02
        #define RGB_LCD_ONLY                0x03
    /*------------------------------------------------------------*/
    #endif
    
	AIT_REG_B	DSPY_RGB_DOT_CLK;										// 0x03A4	
	#if (CHIP == P_V2)
	// ++ PYTHON V1
    AIT_REG_B   DSPY_RGB_PORCH_HIGH_BIT_V1;                                // 0x03A5
    // -- PYTHON V1
    #endif
	AIT_REG_B	DSPY_RGB_V_BPORCH;										// 0x03A6	
	AIT_REG_B	DSPY_RGB_V_BLANK;										// 0x03A7	
	AIT_REG_B	DSPY_RGB_H_BPORCH;										// 0x03A8	
	AIT_REG_B	DSPY_RGB_H_BLANK;										// 0x03A9	
	AIT_REG_B	DSPY_RGB_HSYNC_W;										// 0x03AA	
	AIT_REG_B	DSPY_RGB_VSYNC_W;										// 0x03AB	
	AIT_REG_B	DSPY_RGB_V_2_H_DOT;										// 0x03AC	
    AIT_REG_B                           _x03AD;
	AIT_REG_B	DSPY_RGB_PRT_2_H_DOT;									// 0x03AE	
    AIT_REG_B                           _x03AF;
	AIT_REG_W	DSPY_RGB_PAR_ST_Y;  									// 0x03B0	
	AIT_REG_W	DSPY_RGB_PAR_ED_Y;	    							    // 0x03B2	
	AIT_REG_W	DSPY_RGB_PAR_ST_X;  									// 0x03B4	
	AIT_REG_W	DSPY_RGB_PAR_ED_X;									    // 0x03B6
    #if (CHIP == P_V2)

    AIT_REG_B	DSPY_RGB_RATIO_SPI_MCI;                                 // 0x03B8
    AIT_REG_B	                        _x03B9;                         // 0x03B9
    AIT_REG_B	DSPY_RGB_SPI_CS_SETUP_CYCLE;                            // 0x03BA
    AIT_REG_B	DSPY_RGB_SPI_CS_HOLD_CYCLE;                             // 0x03BB
    AIT_REG_B	DSPY_RGB_SPI_CS_HIGH_WIDTH;                             // 0x03BC
    AIT_REG_B	                        _x03BD;                         // 0x03BD
    AIT_REG_B	DSPY_RGB_SPI_CONTROL_REGISTER1;                         // 0x03BE
        #define RGB_SPI_DATA_ONLY_MODE                  0x01
        
    AIT_REG_B	DSPY_RGB_DELTA_MODE;                                    // 0x03BF
        #define RGB_DELTA_MODE_ENABLE                   0x01
        #define RGB_DUMMY_MODE_ENABLE                   0x02
        
        #define SPI_ODD_LINE_RGB                        0x00
        #define SPI_ODD_LINE_RBG                        0x04
        #define SPI_ODD_LINE_GRB                        0x08
        #define SPI_ODD_LINE_GBR                        0x0C
        #define SPI_ODD_LINE_BRG                        0x10
        #define SPI_ODD_LINE_BGR                        0x12
        
        #define SPI_EVEN_LINE_RGB                       0x00
        #define SPI_EVEN_LINE_RBG                       0x20
        #define SPI_EVEN_LINE_GRB                       0x40
        #define SPI_EVEN_LINE_GBR                       0x60
        #define SPI_EVEN_LINE_BRG                       0x80
        #define SPI_EVEN_LINE_BGR                       0xA0
        
    AIT_REG_B	DSPY_SPI_CONTROL_REGISTER1;                             // 0x03C0
        #define SPI_POLARITY_POSITIVE_EDGE              0x00
        #define SPI_POLARITY_NEGATIVE_EDGE              0x02
        #define SPI_RW_READ                             0x00
        #define SPI_RW_WRITE                            0x04
        #define SPI_RS_SEND_COMMAND                     0x00
        #define SPI_RS_SEND_INDEX                       0x08
        #define SPI_CS_ACTIVE_LOW                       0x00
        #define SPI_CS_ACTIVE_HIGH                      0x10
        #define SPI_PANEL_8BITS                         0x00
        #define SPI_PANEL_9BITS                         0x20
        #define SPI_PANEL_12BITS                        0x40
        #define SPI_PANEL_16BITS                        0x60
        #define SPI_PANEL_18BITS                        0x80
        #define SPI_PANEL_24BITS                        0xA0
        
    AIT_REG_B	                        _x03C1;                         // 0x03C1
    AIT_REG_B	DSPY_RATIO_SPI_MCI;									    // 0x03C2
    AIT_REG_B	                        _x03C3;                         // 0x03C3
    AIT_REG_B	DSPY_SPI_CS_SETUP_CYCLE;                                // 0x03C4
    AIT_REG_B	DSPY_SPI_CS_HOLD_CYCLE;                                 // 0x03C5
    AIT_REG_B	DSPY_SPI_CS_HIGH_WIDTH;                                 // 0x03C6
    AIT_REG_B	                        _x03C7;                         // 0x03C7
    #endif
    AIT_REG_B   DSPY_RGB_FRAM_CNT_CPU_INT;                              // 0x03C8
    AIT_REG_B                           _x03C9;
    AIT_REG_B   DSPY_RGB_FRAM_CNT_HOST_INT;                             // 0x03CA
    AIT_REG_B                           _x03CB;
    AIT_REG_W                           _x03CC[2];
    AIT_REG_B   DSPY_WBACK_CTL;                             			// 0x03D0
        /*-DEFINE-----------------------------------------------------*/
        #define WBACK_ONLY_MODE				0x10
        #define WBACK_FMT_BGR				0x08
        #define WBACK_FMT_RGB				0x00
        #define WBACK_FMT_888				0x04
        #define WBACK_FMT_565				0x02
        #define WBACK_EN					0x01
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x03D1[3];
    AIT_REG_D   DSPY_WBACK_ADDR;                             			// 0x03D4
    AIT_REG_W   DSPY_WBACK_W;                             				// 0x03D8
    AIT_REG_W   DSPY_WBACK_H;                             				// 0x03DA
    AIT_REG_W   DSPY_WBACK_X;                             				// 0x03DC
    AIT_REG_W   DSPY_WBACK_Y;                             				// 0x03DE
    AIT_REG_D   DSPY_OVLY_TV_EVFIELD_ST;                                // 0x03E0
    AIT_REG_D                           _x03E4[3];
    AIT_REG_W   DSPY_OVLY_FIFO_THR;                                     // 0x03F0
    AIT_REG_W                           _x03F2;
    AIT_REG_W                           _x03F4;
    AIT_REG_W                           _x03F6;
    AIT_REG_W   DSPY_OVLY_U_FIFO_THR;                                   // 0x03F8
    AIT_REG_W   DSPY_OVLY_V_FIFO_THR;                                   // 0x03FA
    AIT_REG_W                           _x03FC;
    AIT_REG_W   DSPY_OVLY_FIFO_SRAM_PARAM;
} AITS_DSPY, *AITPS_DSPY;


//-----------------------------
// TV structure (0x8000 7070)
//-----------------------------

typedef struct _AITS_TV {
    AIT_REG_D TVIF_TEST_1ST_Y2CrY1Cb;       // 0x0070
    AIT_REG_D TVIF_TEST_2ND_Y2CrY1Cb;       // 0x0074
    AIT_REG_B TVIF_DAC_IF_1ST_CTL;          // 0x0078
        /*-DEFINE-----------------------------------*/
        #define TV_DAC_POWER_DOWN_EN        0x08	
        #define TV_BGREF_POWER_DOWN_EN      0x04	
        #define TV_IQUARTER                 0x02	
        #define TV_OTYPE                    0x01	
        /*------------------------------------------*/
    AIT_REG_B TVIF_DAC_IF_2ND_CTL;          // 0x0079
        /*-DEFINE-----------------------------------*/
        #define TV_VPLUGREF                 0x10	
        #define TV_COMP_LEVEL               0x08	
        #define TV_HYS_ON                   0x04	
        #define TV_TEST_COMP                0x02	
        #define TV_PLUG_DECT                0x01	
        /*------------------------------------------*/
    AIT_REG_B TVIF_DAC_IF_3RD_CTL;          // 0x007A
        /*-DEFINE-----------------------------------*/
        #define TV_DAC_CLOCK_DATA_EXT       0x04
        /*------------------------------------------*/
    AIT_REG_B                               _x007B;
    AIT_REG_B TVIF_BACKGROUND_Y_COLOR;      // 0x007C
    AIT_REG_B TVIF_BACKGROUND_U_COLOR;      // 0x007D
    AIT_REG_B TVIF_BACKGROUND_V_COLOR;      // 0x007E
    #if (CHIP == P_V2)
    // ++ PYTHON V1
    AIT_REG_B TVIF_CLK_DELAY_V1;               // 0x007F
        /*-DEFINE-----------------------------------*/
        #define NO_DELAY                    0x00
        #define DELAY_1T                    0x01
        #define DELAY_2T                    0x02
        #define DELAY_3T                    0x03
        /*------------------------------------------*/
    // -- PYTHON V1
    #endif
    AIT_REG_B TVIF_IF_EN;                   // 0x0080
        /*-DEFINE-----------------------------------*/
        #define TV_ENC_TEST_MODE_EN         0x80
        #define TV_IF_DAC_CTL               0x40
        #define TV_TYPE_NTSC                0x00
        #define TV_TYPE_PAL                 0x20
        #define TV_UV_SEL_HALF_SUM          0x00
        #define TV_UV_SEL_U1V1              0x08
        #define TV_UV_SEL_U2V2              0x10
        #define TV_8MHZ_FPGA_TEST           0x04
        #define TV_DISPLAY_SPECIFIED_IMAGE  0x02
        #define TV_ENC_IF_EN                0x01
        /*------------------------------------------*/
    AIT_REG_B TVIF_ENDLINE_OFFSET_CTL;      // 0x0081
    AIT_REG_B TVIF_EARLY_PIXL;		        // 0x0082
    AIT_REG_B TVIF_1ST_PXL_RQST_TIMING;     // 0x0083
    AIT_REG_W TVIF_NTSC_ODFIELD_LINE;     	// 0x0084
    AIT_REG_W TVIF_NTSC_EVFIELD_LINE;       // 0x0086
    AIT_REG_W TVIF_PAL_1ST_FIELD_LINE;      // 0x0088
    AIT_REG_W TVIF_PAL_2ND_FIELD_LINE;      // 0x008A
    AIT_REG_W TVIF_NTSC_EVLINE_SUB1;        // 0x008C
    AIT_REG_W TVIF_PAL_EVLINE_SUB1;		    // 0x008E
    AIT_REG_W TVIF_INT1_CPU;                // 0x0090
    AIT_REG_W TVIF_INT2_CPU;                // 0x0092
    AIT_REG_W TVIF_INT1_HOST;               // 0x0094
    AIT_REG_W TVIF_INT2_HOST;               // 0x0096
    AIT_REG_W TVIF_IMAGE_WIDTH;             // 0x0098
    AIT_REG_W TVIF_IMAGE_HEIGHT;            // 0x009A
    AIT_REG_W TVIF_IMAGE_START_X;           // 0x009C
    AIT_REG_W TVIF_IMAGE_START_Y;           // 0x009E
    AIT_REG_D TVENC_SYNC_CTL;               // 0x00A0
        /*-DEFINE-----------------------------------*/
        #define TV_ENC_SYNC_SW_RST          0xC0000000
        #define TV_UV_SWAPPING_EN           0x00001000
        /*------------------------------------------*/
    AIT_REG_D TVENC_MODE_CTL;               // 0x00A4
        /*-DEFINE-----------------------------------*/
        #define TV_714MV_286MV_MODE         0x80000000
        #define TV_BLACKER_LEVEL_EN         0x40000000
        #define TV_COLOR_BAR_TYPE           0x10000000
        #define TV_FULL_WIDTH_OUTPUT_EN     0x08000000
        #define TV_SLEW_RATE_CTL_DIS        0x02000000
        #define TV_MIX_SUB_VIDEO_EN         0x01000000
        #define TV_SVIDEO_CVBS_EN           0x00040000
        #define TV_OUTPUT_CVBS_MODE         0x00020000
        #define TV_OUTPUT_SVIDEO_MODE       0x00010000
        #define TV_CHROMA_UPSAMPLE_EN       0x00004000
        #define TV_DELAY_INPUT_Y_HALF_PIX_1 0x00000000
        #define TV_DELAY_INPUT_Y_HALF_PIX_2 0x00000400
        #define TV_DELAY_INPUT_Y_HALF_PIX_3 0x00000800
        #define TV_DELAY_INPUT_Y_HALF_PIX_4 0x00000C00
        #define TV_DELAY_INPUT_Y_ONE_PIX_1  0x00000000
        #define TV_DELAY_INPUT_Y_ONE_PIX_2  0x00000100
        #define TV_DELAY_INPUT_Y_ONE_PIX_3  0x00000200
        #define TV_DELAY_INPUT_Y_ONE_PIX_4  0x00000300
        #define TV_LUMA_LPF_EN              0x00000080
        #define TV_UV_SWAPPING_SUB_VIDEO    0x00000040
        #define TV_CHROMA_LPF_EN            0x00000020
        #define TV_SETUP_751RE_EN           0x00000004
        #define TV_COLOR_BAR_EN             0x00000002
        #define TV_ENCODER_EN               0x00000001
        /*------------------------------------------*/
    AIT_REG_D TVENC_CLOSED_CAPTION;         // 0x00A8
        /*-DEFINE-----------------------------------*/
        #define TV_CLOSED_CAP_LINE_21_22    0x00010000
        #define TV_CLOSED_CAP_LINE_284_335  0x00020000
        /*------------------------------------------*/
    AIT_REG_D TVENC_Y_SCALE_CTL;            // 0x00AC
        /*-DEFINE-----------------------------------*/
        #define TV_SUB_VIDEO_DELAY_SEL_1T   0xC0000000
        #define TV_SUB_VIDEO_DELAY_SEL_2T   0x80000000
        #define TV_SUB_VIDEO_DELAY_SEL_3T   0x40000000
        #define TV_SUB_VIDEO_DELAY_SEL_4T   0x00000000
        #define TV_SUB_PIC_DELAY_SEL_1T     0x30000000
        #define TV_SUB_PIC_DELAY_SEL_2T     0x20000000
        #define TV_SUB_PIC_DELAY_SEL_3T     0x10000000
        #define TV_SUB_PIC_DELAY_SEL_4T     0x00000000
        #define TV_OSD_DELAY_SEL_1T         0x0C000000
        #define TV_OSD_DELAY_SEL_2T         0x08000000
        #define TV_OSD_DELAY_SEL_3T         0x04000000
        #define TV_OSD_DELAY_SEL_4T         0x00000000
        /*------------------------------------------*/
    AIT_REG_D TVENC_U_SCALE_CTL;            // 0x00B0
    AIT_REG_D TVENC_V_SCALE_CTL;            // 0x00B4
    AIT_REG_D TVENC_GAMMA_COEF_0;           // 0x00B8
        /*-DEFINE-----------------------------------*/
        #define TV_ACTIVE_VBI_EN            0x00400000
        /*------------------------------------------*/
    AIT_REG_D TVENC_GAMMA_COEF_1_2;         // 0x00BC
    AIT_REG_D TVENC_GAMMA_COEF_3_4;         // 0x00C0
    AIT_REG_D TVENC_GAMMA_COEF_5_6;         // 0x00C4
    AIT_REG_D TVENC_GAMMA_COEF_7_8;         // 0x00C8
    AIT_REG_D TVENC_DAC_CONFIG;             // 0x00CC
        /*-DEFINE-----------------------------------*/
        #define TV_VREF_OUTPUT_DIS          0x00008000
        #define TV_CLOCK_DAC_NEGATIVE_EDGE  0x00004000
        #define TV_TRIM_MODE_EN             0x00002000
        #define TV_PLUG_DETECT_EN           0x00001000
        #define TV_DAS_Y_OUTPUT_OFF         0x00000000
        #define TV_DAS_Y_AUTO_DETECT        0x00000040
        #define TV_DAS_Y_OUTPUT_ON          0x00000080
        #define TV_DAX_C_OUTPUT_OFF         0x00000000
        #define TV_DAX_C_AUTO_DETECT        0x00000010
        #define TV_DAX_C_OUTPUT_ON          0x00000020
        /*------------------------------------------*/
    AIT_REG_D TVENC_COLOR_BURST_CONFIG;     // 0x00D0
        /*-DEFINE-----------------------------------*/
        #define TV_PAL_MODE_BURST_SEL_STR   0x04000000
        #define TV_UV_EXTRA_GAIN_EN         0x02000000
        #define TV_PAL_MODE_BURST_SEL_END   0x01000000
        #define TV_FORCE_PAL60_NTSC443      0x00008000
        /*------------------------------------------*/
    AIT_REG_D                               _x00D4;
    AIT_REG_D TVENC_WSS_IF_MODE;            // 0x00D8
        /*-DEFINE-----------------------------------*/
        #define TV_WSS_IF_MODE_EN           0x00100000
        /*------------------------------------------*/
    AIT_REG_D TVENC_UV_SCALE_GAIN_4_5;      // 0x00DC
    AIT_REG_D TVENC_Y_LPF_COEF_00_03;       // 0x00E0
    AIT_REG_D TVENC_Y_LPF_COEF_04_07;       // 0x00E4
    AIT_REG_D TVENC_Y_LPF_COEF_08_0B;       // 0x00E8
    AIT_REG_D TVENC_Y_LPF_COEF_0C_0F;       // 0x00EC
    AIT_REG_D TVENC_Y_LPF_COEF_10_13;       // 0x00F0
    AIT_REG_D TVENC_C1_LPF_COEF_00_03;      // 0x00F4
    AIT_REG_D TVENC_C1_LPF_COEF_04;         // 0x00F8
    AIT_REG_D                               _x00FC;
} AITS_TV, *AITPS_TV;

#if !defined(BUILD_FW)
// DSI  OPR
#if (CHIP == P_V2)
#define DSI_INT_HOST_EN             (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_INT_HOST_EN      )))
#define DSI_INT_HOST_SR             (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_INT_HOST_SR      )))
#define DSI_INT_CPU_EN              (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_INT_CPU_EN       )))
#define DSI_INT_CPU_SR              (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_INT_CPU_SR       )))
#define DSI_DSPY_BUF_TRIGGER        (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_DSPY_BUF_TRIGGER )))
#define DSI_PACKET_ATTR             (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_PACKET_ATTR      )))
#define DSI_DATA_LANE_CNT           (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_DATA_LANE_CNT    )))
#define DSI_PACKET_CONFIG           (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_PACKET_CONFIG    )))
#define DSI_TRANSMIT_CNT            (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TRANSMIT_CNT     )))
#define DSI_TRANSMIT_INTV           (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TRANSMIT_INTV    )))
#define DSI_CLANE_HS_EN             (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_CLANE_HS_EN      )))
#define DSI_DLANE_TRIGGER_MODE      (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_DLANE_TRIGGER_MODE)))
#define DSI_LPDT_READ               (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_LPDT_READ        )))
#define DSI_TLPX                    (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TLPX             )))
#define DSI_TCLK_PREPARE            (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TCLK_PREPARE     )))
#define DSI_TCLK_ZERO               (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TCLK_ZERO        )))
#define DSI_TCLK_PRE                (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TCLK_PRE         )))
#define DSI_TCLK_POST               (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TCLK_POST        )))
#define DSI_TCLK_TRAIL              (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_TCLK_TRAIL       )))
#define DSI_THS_PREPARE             (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_THS_PREPARE      )))
#define DSI_THS_ZERO                (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_THS_ZERO         )))
#define DSI_THS_TRAIL               (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_THS_TRAIL        )))
#define DSI_DPHY_CLK_SEL            (DSI_BASE  +(MMP_ULONG)(&(((AITPS_DSI  )0)->DSI_DPHY_CLK_SEL     )))
#endif

// LCD  OPR
#if (CHIP == P_V2)
#define DSPY_WIN_ALPHA_WT_1			(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WIN_ALPHA_WT_1  )))
#define DSPY_WIN_ALPHA_WT_2			(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WIN_ALPHA_WT_2  )))
#endif
#define DSPY_CTL_0                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_CTL_0           )))
#define DSPY_CTL_2	                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_CTL_2           )))
#define DSPY_INT_CPU_EN             (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_CPU_EN      )))
#define DSPY_INT_CPU_SR             (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_CPU_SR      )))
#define DSPY_INT_HOST_EN            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_HOST_EN     )))
#define DSPY_INT_HOST_SR            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_HOST_SR     )))
#define DSPY_LCD_TX_0               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_TX_0        )))
#define DSPY_LCD_TX_1               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_TX_1        )))
#define DSPY_LCD_TX_2               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_TX_2        )))
#define DSPY_LCD_TX_3				(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_TX_3        )))
#define DSPY_LCD_TX_4               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_TX_4        )))
#define DSPY_LCD_TX_5               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_TX_5        )))
#define DSPY_LCD_TX_6               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_TX_6        )))
#define DSPY_LCD_AUTO_CFG           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_AUTO_CFG    )))
#define DSPY_PLCD_CTL               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PLCD_CTL        )))
#define DSPY_PLCD_FMT               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PLCD_FMT        )))
#define DSPY_LCD_SR                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_LCD_SR          )))
#define DSPY_PLCD_RS_CS_CYC         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PLCD_RS_CS_CYC  )))
#define DSPY_PLCD_CS_RW_CYC         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PLCD_CS_RW_CYC  )))
#define DSPY_PLCD_RW_CYC            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PLCD_RW_CYC     )))
#define DSPY_PLCD_IDX_CMD_NUM       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PLCD_IDX_CMD_NUM)))

#define DSPY_W                      (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_W               )))
#define DSPY_H                      (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_H               )))
#define DSPY_PIXL_CNT               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIXL_CNT        )))
#define DSPY_CTL_4                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_CTL_4           )))
#define DSPY_WIN_PRIO               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WIN_PRIO        )))

#define DSPY_ICON_CTL               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_ICON_CTL        )))
#define DSPY_ICON_W                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_ICON_W          )))
#define DSPY_ICON_H                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_ICON_H          )))
#define DSPY_ICON_X                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_ICON_X          )))
#define DSPY_ICON_Y                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_ICON_Y          )))

#define DSPY_BG_COLR                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_BG_COLR         )))
#define	DSPY_PLCD_READ              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PLCD_READ       )))
#define DSPY_FIFO_CLR               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_FIFO_CLR        )))

#define DSPY_WIN_FIFO_CX            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WIN_FIFO_CX     )))

#define DSPY_MAIN_0_ADDR_ST         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_0_ADDR_ST  )))
#define DSPY_MAIN_1_ADDR_ST         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_1_ADDR_ST  )))
#define DSPY_MAIN_2_ADDR_ST         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_2_ADDR_ST  )))
#define DSPY_MAIN_OFST_ST           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_OFST_ST    )))
#define DSPY_MAIN_OFST_PIXL         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_OFST_PIXL  )))
#define DSPY_MAIN_OFST_ROW          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_OFST_ROW   )))
#define DSPY_MAIN_0_U_ADDR_ST       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_0_U_ADDR_ST)))
#define DSPY_MAIN_1_U_ADDR_ST       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_1_U_ADDR_ST)))
#define DSPY_MAIN_2_U_ADDR_ST       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_2_U_ADDR_ST)))
#define DSPY_MAIN_0_V_ADDR_ST       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_0_V_ADDR_ST)))
#define DSPY_MAIN_1_V_ADDR_ST       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_1_V_ADDR_ST)))
#define DSPY_MAIN_2_V_ADDR_ST       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_2_V_ADDR_ST)))
#define DSPY_MAIN_OFST_UV_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_OFST_UV_ST  )))
#define DSPY_MAIN_OFST_UV_PIXL      (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_OFST_UV_PIXL)))
#define DSPY_MAIN_OFST_UV_ROW       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_OFST_UV_ROW )))
#define DSPY_MAIN_TV_EVFIELD_ST     (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_TV_EVFIELD_ST)))

#define DSPY_MAIN_W                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_W          )))
#define DSPY_MAIN_H                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_H          )))
#define DSPY_MAIN_X                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_X          )))
#define DSPY_MAIN_Y                 (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_Y          )))

#define DSPY_MAIN_CTL               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_CTL        )))
#define DSPY_MAIN_CTL_2             (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_CTL_2      )))
#define DSPY_MAIN_FMT               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_FMT        )))
#define DSPY_MAIN_PIXL_CNT          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_PIXL_CNT   )))
#define DSPY_MAIN_TP_CTL            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_TP_CTL     )))
#define DSPY_MAIN_TP_COLR           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_TP_COLR    )))
#define DSPY_MAIN_SEMITP_WT         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_SEMITP_WT  )))
#define DSPY_MAIN_FIFO_THR          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_FIFO_THR   )))

#define DSPY_PIP_0_ADDR_ST          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_0_ADDR_ST  )))
#define DSPY_PIP_1_ADDR_ST          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_1_ADDR_ST  )))
#define DSPY_PIP_2_ADDR_ST          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_2_ADDR_ST  )))
#define DSPY_PIP_OFST_ST            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_OFST_ST    )))
#define DSPY_PIP_OFST_PIXL          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_OFST_PIXL  )))
#define DSPY_PIP_OFST_ROW           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_OFST_ROW   )))
#define DSPY_PIP_0_U_ADDR_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_0_U_ADDR_ST)))
#define DSPY_PIP_1_U_ADDR_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_1_U_ADDR_ST)))
#define DSPY_PIP_2_U_ADDR_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_2_U_ADDR_ST)))
#define DSPY_PIP_0_V_ADDR_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_0_V_ADDR_ST)))
#define DSPY_PIP_1_V_ADDR_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_1_V_ADDR_ST)))
#define DSPY_PIP_2_V_ADDR_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_2_V_ADDR_ST)))
#define DSPY_PIP_OFST_UV_ST         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_OFST_UV_ST  )))
#define DSPY_PIP_OFST_UV_PIXL       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_OFST_UV_PIXL)))
#define DSPY_PIP_OFST_UV_ROW        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_OFST_UV_ROW )))

#define DSPY_PIP_W                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_W           )))
#define DSPY_PIP_H                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_H           )))
#define DSPY_PIP_X                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_X           )))
#define DSPY_PIP_Y                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_Y           )))
#define DSPY_PIP_CTL                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_CTL         )))
#define DSPY_PIP_CTL_2              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_CTL_2       )))
#define DSPY_PIP_FMT                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_FMT         )))
#define DSPY_PIP_PIXL_CNT           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_PIXL_CNT    )))
#define DSPY_PIP_SEMITP_WT          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_SEMITP_WT   )))
#define DSPY_PIP_TP_COLR            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_TP_COLR     )))
#define DSPY_ICON_SEMITP_WT         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_ICON_SEMITP_WT  )))
#define DSPY_SOUT_GRAB_PIXL_CNT     (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SOUT_GRAB_PIXL_CNT)))
#define DSPY_SCAL_CTL               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_CTL        )))
#define DSPY_SOUT_CTL               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SOUT_CTL        )))
#define DSPY_SCAL_H_N               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_H_N        )))
#define DSPY_SCAL_H_M               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_H_M        )))
#define DSPY_SCAL_V_N               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_V_N        )))
#define DSPY_SCAL_V_M               (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_V_M        )))
#define DSPY_SEDGE_CTL              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SEDGE_CTL       )))
#define DSPY_SCAL_H_WT   		    (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_H_WT  	   	)))
#define DSPY_SCAL_V_WT	            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_V_WT		)))
#define DSPY_SIN_W                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SIN_W           )))
#define DSPY_SIN_H                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SIN_H           )))
#define DSPY_SOUT_GRAB_H_ST         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SOUT_GRAB_H_ST  )))
#define DSPY_SOUT_GRAB_H_ED         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SOUT_GRAB_H_ED  )))
#define DSPY_SOUT_GRAB_V_ST         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SOUT_GRAB_V_ST  )))
#define DSPY_SOUT_GRAB_V_ED         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SOUT_GRAB_V_ED  )))
#define DSPY_PIP_TV_EVFIELD_ST      (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_TV_EVFIELD_ST)))
#define DSPY_PIP_FIFO_THR           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_FIFO_THR    )))
#define DSPY_SCAL_IN_THR            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCAL_IN_THR     )))
#define DSPY_PIP_U_FIFO_THR     	(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_U_FIFO_THR  )))

#if (CHIP == P_V2)
// ++ PYTHON V1
#define DSPY_RGB_SYNC_MODE_V1     	    (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_SYNC_MODE_V1   )))
// -- PYTHON V1
#endif

#define DSPY_PIP_V_FIFO_THR     	(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_PIP_V_FIFO_THR  )))
#define DSPY_MAIN_U_FIFO_THR     	(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_U_FIFO_THR  )))
#define DSPY_MAIN_V_FIFO_THR     	(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_MAIN_V_FIFO_THR  )))


// SCD OPR
#define DSPY_SCD_CTL                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_CTL          )))
#define DSPY_SCD_BG_COLR            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_BG_COLR      )))
#define DSPY_SCD_PLCD_READ          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_PLCD_READ    )))
#define DSPY_SCD_LCD_TX_0           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_LCD_TX_0     )))
#define DSPY_SCD_LCD_TX_1           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_LCD_TX_1     )))
#define DSPY_SCD_LCD_TX_2           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_LCD_TX_2     )))
#define DSPY_SCD_LCD_AUTO_CFG       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_LCD_AUTO_CFG )))
#define DSPY_SCD_W                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_W            )))
#define DSPY_SCD_H                  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_H            )))
#define DSPY_SCD_PIXL_CNT           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_PIXL_CNT     )))
#define DSPY_SCD_WIN_PIXL_CNT       (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_WIN_PIXL_CNT )))
#define DSPY_SCD_WIN_ADDR_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_WIN_ADDR_ST  )))
#define DSPY_SCD_WIN_OFST_ST        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_WIN_OFST_ST  )))
#define DSPY_SCD_WIN_W              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_WIN_W        )))
#define DSPY_SCD_WIN_H              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_WIN_H        )))
#define DSPY_SCD_WIN_X              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_WIN_X        )))
#define DSPY_SCD_WIN_Y              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SCD_WIN_Y        )))

// RGB OPR
#define DSPY_RGB_CTL                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_CTL          )))
#define DSPY_RGB_SPI_CTL            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_SPI_CTL      )))
#define DSPY_RGB_FMT                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_FMT          )))

#if (CHIP == P_V2)
#define DSPY_RGB_SHARE_P_LCD_BUS    (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_SHARE_P_LCD_BUS)))
#endif

#define DSPY_RGB_DOT_CLK            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_DOT_CLK      )))
#if (CHIP == P_V2)
// ++ PYTHON V1
#define DSPY_RGB_PORCH_HIGH_BIT_V1  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_PORCH_HIGH_BIT_V1)))
// -- PYTHON V1
#endif
#define DSPY_RGB_V_BPORCH           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_V_BPORCH     )))
#define DSPY_RGB_V_BLANK            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_V_BLANK      )))
#define DSPY_RGB_H_BPORCH           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_H_BPORCH     )))
#define DSPY_RGB_H_BLANK            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_H_BLANK      )))
#define DSPY_RGB_HSYNC_W            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_HSYNC_W      )))
#define DSPY_RGB_VSYNC_W            (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_VSYNC_W      )))
#define DSPY_RGB_V_2_H_DOT          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_V_2_H_DOT    )))
#define DSPY_RGB_PRT_2_H_DOT        (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_PRT_2_H_DOT  )))
#define DSPY_RGB_PAR_ST_X           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_PAR_ST_X     )))
#define DSPY_RGB_PAR_ST_Y           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_PAR_ST_Y     )))
#define DSPY_RGB_PAR_ED_X           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_PAR_ED_X     )))
#define DSPY_RGB_PAR_ED_Y           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_PAR_ED_Y     )))
#define DSPY_RGB_FRAM_CNT_CPU_INT   (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_FRAM_CNT_CPU_INT)))
#define DSPY_RGB_FRAM_CNT_HOST_INT  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_FRAM_CNT_HOST_INT)))

#define DSPY_RGB_LINE_CPU_INT_1		(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_LINE_CPU_INT_1)))
#define DSPY_RGB_LINE_CPU_INT_2		(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_LINE_CPU_INT_2)))
#define DSPY_RGB_LINE_HOST_INT_1	(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_LINE_HOST_INT_1)))
#define DSPY_RGB_LINE_HOST_INT_2	(DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_LINE_HOST_INT_2)))
#define DSPY_INT_HOST_EN_2          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_HOST_EN_2    )))
#define DSPY_INT_HOST_SR_2          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_HOST_SR_2    )))
#define DSPY_INT_CPU_EN_2           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_CPU_EN_2     )))
#define DSPY_INT_CPU_SR_2           (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_INT_CPU_SR_2     )))


// WBACK OPR
#define DSPY_WBACK_CTL              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WBACK_CTL        )))
#define DSPY_WBACK_ADDR             (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WBACK_ADDR       )))
#define DSPY_WBACK_W                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WBACK_W          )))
#define DSPY_WBACK_H                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WBACK_H          )))
#define DSPY_WBACK_X                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WBACK_X          )))
#define DSPY_WBACK_Y                (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_WBACK_Y          )))

#if (CHIP == P_V2)
//SPI
#define DSPY_RGB_RATIO_SPI_MCI          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_RATIO_SPI_MCI          )))
#define DSPY_RGB_SPI_CS_SETUP_CYCLE     (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_SPI_CS_SETUP_CYCLE     )))
#define DSPY_RGB_SPI_CS_HOLD_CYCLE      (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_SPI_CS_HOLD_CYCLE      )))
#define DSPY_RGB_SPI_CS_HIGH_WIDTH      (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_SPI_CS_HIGH_WIDTH      )))
#define DSPY_RGB_SPI_CONTROL_REGISTER1  (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_SPI_CONTROL_REGISTER1  )))
#define DSPY_RGB_DELTA_MODE             (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RGB_DELTA_MODE             )))
#define DSPY_SPI_CONTROL_REGISTER1      (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SPI_CONTROL_REGISTER1      )))
#define DSPY_RATIO_SPI_MCI              (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_RATIO_SPI_MCI              )))
#define DSPY_SPI_CS_SETUP_CYCLE         (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SPI_CS_SETUP_CYCLE         )))
#define DSPY_SPI_CS_HOLD_CYCLE          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SPI_CS_HOLD_CYCLE          )))
#define DSPY_SPI_CS_HIGH_WIDTH          (DSPY_BASE +(MMP_ULONG)(&(((AITPS_DSPY )0)->DSPY_SPI_CS_HIGH_WIDTH          )))
#endif

// PALETTE OPR
#if (CHIP == P_V2)
#define MAIN_PALETTE_R0             0x8000E400
#define MAIN_PALETTE_G0             0x8000E200
#define MAIN_PALETTE_B0             0x8000E000

#define OVERLAY_PALETTE_R0          0x8000F400
#define OVERLAY_PALETTE_G0          0x8000F200
#define OVERLAY_PALETTE_B0          0x8000F000

#define ICON_PALETTE_R0             0x8000DC00
#define ICON_PALETTE_G0             0x8000DA00
#define ICON_PALETTE_B0             0x8000D800
#endif
// TV OPR
#define TVIF_DAC_IF_1ST_CTL         (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_DAC_IF_1ST_CTL       ))) // 0x0078
#define TVIF_BACKGROUND_Y_COLOR     (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_BACKGROUND_Y_COLOR   ))) // 0x007C
#define TVIF_BACKGROUND_U_COLOR     (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_BACKGROUND_U_COLOR   ))) // 0x007D
#define TVIF_BACKGROUND_V_COLOR     (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_BACKGROUND_V_COLOR   ))) // 0x007E
#if (CHIP == P_V2)
// ++ PYTHON V1
#define TVIF_CLK_DELAY_V1           (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_CLK_DELAY_V1         ))) // 0x007F
// -- PYTHON V1
#endif
#define TVIF_IF_EN                  (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_IF_EN                ))) // 0x0080
#define TVIF_ENDLINE_OFFSET_CTL     (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_ENDLINE_OFFSET_CTL   ))) // 0x0081
#define TVIF_EARLY_PIXL		        (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_EARLY_PIXL           ))) // 0x0082
#define TVIF_NTSC_ODFIELD_LINE      (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_NTSC_ODFIELD_LINE    ))) // 0x0084
#define TVIF_NTSC_EVFIELD_LINE      (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_NTSC_EVFIELD_LINE    ))) // 0x0086
#define TVIF_PAL_ODFIELD_LINE       (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_PAL_ODFIELD_LINE     ))) // 0x0088
#define TVIF_PAL_EVFIELD_LINE       (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_PAL_EVFIELD_LINE     ))) // 0x008A
#define TVIF_NTSC_EVLINE_SUB1       (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_NTSC_EVLINE_SUB1     ))) // 0x008C
#define TVIF_PAL_EVLINE_SUB1        (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_PAL_EVLINE_SUB1      ))) // 0x008E
#define TVIF_IMAGE_WIDTH            (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_IMAGE_WIDTH          ))) // 0x0098
#define TVIF_IMAGE_HEIGHT           (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_IMAGE_HEIGHT         ))) // 0x009A
#define TVIF_IMAGE_START_X          (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_IMAGE_START_X        ))) // 0x009C
#define TVIF_IMAGE_START_Y          (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVIF_IMAGE_START_Y        ))) // 0x009E
#define TVENC_Y_SCALE_CTL           (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVENC_Y_SCALE_CTL         ))) // 0x00AC
#define TVENC_U_SCALE_CTL           (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVENC_U_SCALE_CTL         ))) // 0x00B0
#define TVENC_V_SCALE_CTL           (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVENC_V_SCALE_CTL         ))) // 0x00B4
#define TVENC_GAMMA_COEF_0          (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVENC_GAMMA_COEF_0        ))) // 0x00B8
#define TVENC_UV_SCALE_GAIN_4_5     (TV_BASE  +(MMP_ULONG)(&(((AITPS_TV  )0)->TVENC_UV_SCALE_GAIN_4_5   ))) // 0x00DC
#endif
/// @}

#endif //#if (CHIP == P_V2)

#endif // _MMPH_REG_DISPLAY_H_
///@end_ait_only
