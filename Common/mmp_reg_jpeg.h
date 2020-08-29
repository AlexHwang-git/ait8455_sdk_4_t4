//==============================================================================
//
//  File        : mmp_reg_jpeg.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_JPEG_H_
#define _MMP_REG_JPEG_H_

#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/

typedef struct _AITS_JPG {
    AIT_REG_W   JPG_ENC_W;                                              // 0x00
    AIT_REG_W   JPG_ENC_H;												// 0x02
    AIT_REG_W   JPG_CTL;												// 0x04
        /*-DEFINE-----------------------------------------------------*/
        #define JPG_ENC_FMT_YUV422          0x0000
        #define JPG_ENC_FMT_MASK            0x0001
        #if (CHIP == P_V2)
        #define JPG_DEC_EN                  0x0002
        #define JPG_DEC_EN_WITH_PAUSE       0x0000
        #define JPG_DEC_EN_NO_PAUSE         0x0004
        #endif
        #define JPG_ENC_EN                  0x0008
		#define JPG_ENC_MJPG_EN             0x0010
        #define JPG_ENC_MARKER_EN           0x0020
		#define JPG_ENC_MJPG_HEADER_EN      0x0040	
        #define JPG_ENC_TO_FB_DIS           0x0100
        #if (CHIP == VSN_V3)
        #define JPG_ENC_CBR_DC_Q_DIR        0x0400
        #endif
        /*------------------------------------------------------------*/

    #if (CHIP == P_V2) || (CHIP == VSN_V3)
    AIT_REG_W   JPG_QLTY_CTL_FACTOR_1;                                  // 0x06
    AIT_REG_W   JPG_QLTY_CTL_FACTOR_2;                                  // 0x08
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_B   JPG_QLTY_CTL_FACTOR_1;                                  // 0x06
    AIT_REG_B   JPG_QLTY_CTL_FACTOR_SR_1;                               // 0x07
    AIT_REG_B   JPG_QLTY_CTL_FACTOR_2;                                  // 0x08
    AIT_REG_B   JPG_QLTY_CTL_FACTOR_SR_2;                               // 0x09
    #endif

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B                           _x0A[0x6];
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   JPG_DEC_CTL;											// 0x0A
        /*-DEFINE-----------------------------------------------------*/
        #define JPG_DEC_SCALUP_MASK         0x001F
        #define JPG_DEC_DNSAMP_2_EN         0x0040
        #define JPG_DEC_DB_EN               0x0080
        #define JPG_DEC_SCALUP(_a)          (_a & JPG_DEC_SCALUP_MASK)
        /*------------------------------------------------------------*/
    AIT_REG_B   JPG_DEC_FMT;
    AIT_REG_W   JPG_DEC_H;												// 0x0C
    AIT_REG_W   JPG_DEC_V;												// 0x0E
    #endif

    AIT_REG_D   JPG_BUF_TMP_ST;                                         // 0x10
    AIT_REG_D   JPG_BUF_CMP_ST;                                         // 0x14
    AIT_REG_D   JPG_BUF_CMP_ED;                                         // 0x18
    AIT_REG_D   JPG_BUF_CMP_SIZE;                                       // 0x1C

    #if (CHIP == VSN_V2)
    AIT_REG_W                           _x20;
    #endif
    #if (CHIP == P_V2) || (CHIP == VSN_V3)
    AIT_REG_W   JPG_FIFO_DATA;                                          // 0x20
    #endif

    AIT_REG_W   JPG_FIFO_CSR;                                           // 0x22
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == VSN_V3)
        #define JPG_FIFO_MASK               0x001F
        #define JPG_FIFO_FULL               0x0040
        #define JPG_FIFO_EN                 0x0080
        #define JPG_FIFO_ENC_FIN            0x0100
        #define JPG_FIFO_4_CPU              0x0400
        #define JPG_EXTM_EN                 0x0800
        #endif
        #if (CHIP ==VSN_V2)
        #define JPG_FIFO_EN                 0x0080
        #define JPG_FIFO_ENC_FIN            0x0100
        #define JPG_EXTM_EN                 0x0800
        #endif
        #if (CHIP == P_V2)
        #define JPG_FIFO_MASK               0x001F
        #define JPG_FIFO_MASK_V1            0x003F
        #define JPG_FIFO_EMPTY              0x0020
        #define JPG_FIFO_FULL               0x0040
        #define JPG_FIFO_EN                 0x0080
        #define JPG_FIFO_DIS                0x0000
        #define JPG_FIFO_ENC_FIN            0x0100
        #define JPG_FIFO_DEC_FIN            0x0200
        #define JPG_FIFO_4_HOST             0x0000
        #define JPG_FIFO_4_CPU              0x0400
        #define JPG_EXTM_EN                 0x0800
        #define JPG_DEC_FIFO_EN_V1          0x1000

        #define JPG_FIFO_DEPTH              16
        #define JPG_FIFO_DEPTH_V1           32
        #endif
        /*------------------------------------------------------------*/
    AIT_REG_D   JPG_DEC_FIFO_SIZE;                                      // 0x24
        /*-DEFINE-----------------------------------------------------*/
        #define JPEG_FIFO_SHIFT_UNIT         3                      // define size shift unit 
        #define JPEG_FIFO_SHIFT_UNIT_V1      5                      // define size shift unit 
        /*------------------------------------------------------------*/
    AIT_REG_D   JPG_EXTM_ST;											// 0x28
    AIT_REG_D   JPG_EXTM_ED;											// 0x2C

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B                           _x30[0x4];
    #endif
    #if (CHIP == P_V2)
    AIT_REG_W   JPG_H263_ENC_CTL;                                       // 0x30
    AIT_REG_W                           _x32;
    #endif

    AIT_REG_B   JPG_FIFO_SR;											// 0x34
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == P_V2) || (CHIP == VSN_V3)
        #define JPG_STORE_RAW_MASK          0x03
        #define JPG_STORE_ICO_YUV           0x02
        #define JPG_STORE_SCAL0_MASK        0x18
        #define JPG_STORE_SCAL1_MASK        0x60
        #endif
        #if (CHIP == VSN_V2)
        #define JPG_STORE_VIF_RAW           0x01                      // exclusive with encode_en
        #define JPG_STORE_ICO_YUV           0x02                      // directly store data to compress buffer
        #define JPG_STORE_SCAL0_YUV         0x04
        #define JPG_STORE_SCAL0_RGB565      0x08
        #define JPG_STORE_SCAL0_RGB888      0x10
        #define JPG_STORE_SCAL1_YUV         0x20
        #define JPG_STORE_SCAL1_RGB565      0x40
        #define JPG_STORE_SCAL1_RGB888      0x80
        #endif
        /*------------------------------------------------------------*/
    AIT_REG_B   JPG_IMG_FIFO_RST;											// 0x35
        /*-DEFINE-----------------------------------------------------*/     
        #define JPG_RST_FIFO                0x01
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x36[0x5];

    AIT_REG_W   JPG_INT_HOST_EN;                                        // 0x40
    AIT_REG_W   JPG_INT_HOST_SR;									 	// 0x42
    AIT_REG_W   JPG_INT_CPU_EN;											// 0x44
    AIT_REG_W   JPG_INT_CPU_SR;											// 0x46
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == VSN_V2)
        #define JPG_INT_ENC_DONE            0x01
        #define JPG_INT_HUFFMAN_BUF_OVERFLOW 0x02
        #define JPG_INT_STABILIZE_DONE      0x04
        #define JPG_INT_FIFO_OVERFLOW       0x08
        #define JPG_INT_FIFO_FINISH         0x10
        #endif
        #if (CHIP == P_V2) || (CHIP == VSN_V3)
        #define JPG_INT_ENC_DONE            0x0001
        #define JPG_INT_FIFO_OVERFLOW       0x0002
        #define JPG_INT_FIFO_FULL           0x0004
        #define JPG_INT_FIFO_EMPTY          0x0008
        #define JPG_INT_FIFO_FINISH         0x0010
        #if (CHIP == P_V2)
        #define JPG_INT_DEC_FIFO_FULL       0x0020
        #define JPG_INT_DEC_FIFO_EMPTY      0x0040
        #define JPG_INT_DEC_FIFO_TH         0x0080
        #define JPG_INT_DEC_DONE            0x0100
        #define JPG_INT_DEC_MARKER_DONE     0x0200
        #define JPG_INT_DEC_MARKER_ERR      0x0400
        #define JPG_INT_DEC_HUFFMAN_ERR     0x0800
        #define JPG_INT_DEC_FIFO_ERR        0x1000
        #define JPG_INT_DEC_NONBASELINE     0x2000
        #endif
        #define JPG_INT_HUFFMAN_BUF_OVERFLOW 0x4000
        #define JPG_INT_LINE_BUF_OVERLAP    0x8000
        #endif // (CHIP == P_V2) || (CHIP == VSN_V3)
        /*------------------------------------------------------------*/
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B                           _x48[0x8];
    #endif
    #if (CHIP == P_V2)
    AIT_REG_W   JPG_MARKER_CSR;                                         // 0x48
    AIT_REG_W                           _x4A;
    AIT_REG_B   JPG_DEC_SAMP_RAT_Y;                                     // 0x4C
    AIT_REG_B   JPG_DEC_SAMP_RAT_U;                                     // 0x4D
    AIT_REG_B   JPG_DEC_SAMP_RAT_V;                                     // 0x4E
        /*-DEFINE-----------------------------------------------------*/
        #define JPG_DEC_SAMP_RAT_V_MASK     0x0F
        #define JPG_DEC_SAMP_RAT_H_MASK     0xF0
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x4F;
    #endif

    AIT_REG_D   JPG_ENC_CNT;                                            // 0x50
    AIT_REG_D   JPG_ENC_CNT_SR8;										// 0x54
	AIT_REG_D   JPG_ENC_EXTMEM_CNT;                                     // 0x58
    AIT_REG_D                           _x5C;

    AIT_REG_B   JPG_BUF_TMP_2_EN;										// 0x60
        /*-DEFINE-----------------------------------------------------*/     
        #define JPG_SPLIT_TMP_BUF           0x01
    	#define JPG_SINGLE_LINE_BUF			0x02
        /*------------------------------------------------------------*/    
	AIT_REG_B                           _x61;
    AIT_REG_W                           _x62;
    AIT_REG_D   JPG_BUF_TMP_2_ST;										// 0x64	


    AIT_REG_W   JPG_ENC_INSERT_RST;								        // 0x68
    AIT_REG_W                           _x6A;
    AIT_REG_B   JPG_JPG_FIFO_RST;									    // 0x6C

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B                           _x6D;
    AIT_REG_B                           _x6E[0x32];
    AIT_REG_B   JPG_STABILIZER_CTL;									    // 0xA0
        /*-DEFINE-----------------------------------------------------*/     
        #define JPG_STAB_EN                 0x01
        #define JPG_STAB_BUF0_WIN           0x02
        /*------------------------------------------------------------*/    
    AIT_REG_B   JPG_STAB_FRM_NUM;									    // 0xA1
    AIT_REG_W                           _xA2;
    AIT_REG_D   JPG_STAB_EXT_BUF_ADR;	    						    // 0xA4

    #if (CHIP == VSN_V3)
    AIT_REG_D   JPG_STAB_EXT_BUF_ADR_END;    						    // 0xA8
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_D                           _xA8;
    #endif

    AIT_REG_D   JPG_EDGE_INFO;	    						            // 0xAC
    #endif // (CHIP == VSN_V2) || (CHIP == VSN_V3)

    #if (CHIP == P_V2)
    AIT_REG_B   JPG_DEC_FIFO_RST;										// 0x6D
        /*-DEFINE-----------------------------------------------------*/     
        //#define JPG_RST_FIFO              0x01
        /*------------------------------------------------------------*/    
    AIT_REG_B   JPG_DEC_SCAL_FIFO_RST;								    // 0x6E
        /*-DEFINE-----------------------------------------------------*/     
        #define JPG_RST_FIFO_JDS_2_SCAL     0x01
        #define JPG_RST_FIFO_SCAL_2_JDS     0x02
        /*------------------------------------------------------------*/    
    AIT_REG_B                           _x6F;

    AIT_REG_B   JPG_SCAL_CTL;                                          // 0x70
        /*-DEFINE-----------------------------------------------------*/
        #define JPG_SCAL_FIFO_EN            0x01
        #define JPG_SCAL_MP3_PLAY           0x02
        #define JPG_SCAL_FIFO_RGB565        0x04
        #define JPG_SCAL_FIFO_YUV422        0x00
        #define JPG_DEC_FIFO_CPU            0x08
        #define JPG_DEC_WAIT_SCALE_BUSY     0x10
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x71;
    AIT_REG_W                           _x72;
    AIT_REG_B   JPG_SOUT_DLYCNT;                                        // 0x74
    AIT_REG_B                           _x75;
    AIT_REG_W                           _x76;
    AIT_REG_W   JPG_DLYCNT;                                             // 0x78
    AIT_REG_W                           _x7A;
    AIT_REG_B   JPG_DEC_FIFO_RD_TH;                                     // 0x7C
    AIT_REG_B                           _x7D;
    AIT_REG_W                           _x7E;
    AIT_REG_W   JPG_DEC_FIFO_DATA;                                      // 0x80
    AIT_REG_W                           _x82;
    AIT_REG_B   JPG_DEC_FIFO_SR;                                        // 0x84
        /*-DEFINE-----------------------------------------------------*/
        #define JPG_DEC_FIFO_UNRD_MASK      0x1F
        #define JPG_DEC_FIFO_TH             0x20
        #define JPG_DEC_FIFO_FULL           0x40
        #define JPG_DEC_FIFO_EPY            0x80
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x85;
    AIT_REG_W                           _x86;
    AIT_REG_D                           _x88[2];
    AIT_REG_D   JPG_DEC_CMP_DATA_CNT;                                   // 0x90
    AIT_REG_D                           _x94;        
    AIT_REG_D   JPG_DEC_TRANS_ST;                                       // 0x98
    AIT_REG_D                           _x9C[5];
    AIT_REG_D   JPG_DEC_BUF_CMP_ST_V1;                                  //PYTHON_V1                         // 0xB0
    AIT_REG_D   JPG_DEC_BUF_CMP_ED_V1;                                  //PYTHON_V1                        // 0xB4
    #endif
} AITS_JPG, *AITPS_JPG;


#if !defined(BUILD_FW)
// JPEG OPR
#define	JPG_ENC_W					(JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_ENC_W		       )))
#define	JPG_ENC_H					(JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_ENC_H		       )))
#define	JPG_CTL  					(JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_CTL 		       )))
#if (CHIP == P_V2)
#define JPG_DEC_CTL                 (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_CTL          )))
#define JPG_DEC_H                   (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_H            )))
#define JPG_DEC_V                   (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_V            )))
#endif
#define JPG_BUF_TMP_ST              (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_BUF_TMP_ST        )))
#define JPG_BUF_CMP_ST              (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_BUF_CMP_ST        )))
#define JPG_BUF_CMP_ED              (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_BUF_CMP_ED        )))
#define JPG_BUF_CMP_SIZE            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_BUF_CMP_SIZE      )))
#if (CHIP == P_V2)
#define JPG_FIFO_DATA               (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_FIFO_DATA        )))
#endif
#define JPG_FIFO_CSR                (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_FIFO_CSR          )))
#define JPG_DEC_FIFO_SIZE           (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_FIFO_SIZE    )))
#define JPG_EXTM_ST     	        (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_EXTM_ST           )))
#define JPG_EXTM_ED		            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_EXTM_ED           )))
#define JPG_INT_HOST_SR             (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_INT_HOST_SR      )))
#define JPG_INT_CPU_SR              (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_INT_CPU_SR       )))
#define JPG_ENC_CNT                 (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_ENC_CNT          )))
#define JPG_ENC_CNT_SR8             (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_ENC_CNT_SR8      )))

#define JPG_FIFO_SR                 (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_FIFO_SR           )))
#define JPG_IMG_FIFO_RST            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_IMG_FIFO_RST      )))
#define JPG_INT_HOST_EN             (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_INT_HOST_EN       )))
#define JPG_INT_CPU_EN              (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_INT_CPU_EN        )))
#if (CHIP == P_V2)
#define JPG_MARKER_CSR              (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_MARKER_CSR        )))
#define JPG_DEC_SAMP_RAT_Y          (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_SAMP_RAT_Y    )))
#define JPG_DEC_SAMP_RAT_U          (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_SAMP_RAT_U    )))
#define JPG_DEC_SAMP_RAT_V          (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_SAMP_RAT_V    )))
#endif
#define JPG_ENC_EXTMEM_CNT          (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_ENC_EXTMEM_CNT    )))
#define JPG_BUF_TMP_2_EN            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_BUF_TMP_2_EN      )))
#define JPG_BUF_TMP_2_ST            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_BUF_TMP_2_ST      )))
#define JPG_ENC_INSERT_RST          (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_ENC_INSERT_RST    )))
#define JPG_JPG_FIFO_RST            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_JPG_FIFO_RST      )))
#if (CHIP == P_V2)
#define JPG_DEC_FIFO_RST            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_FIFO_RST      )))
#define JPG_DEC_SCAL_FIFO_RST       (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_SCAL_FIFO_RST )))
#define JPG_SCAL_CTL                (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_SCAL_CTL          )))
#define JPG_SOUT_DLYCNT             (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_SOUT_DLYCNT       )))
#define JPG_DLYCNT                  (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DLYCNT            )))
#define JPG_DEC_FIFO_RD_TH          (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_FIFO_RD_TH    )))
#define JPG_DEC_FIFO_DATA           (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_FIFO_DATA     )))
#define JPG_DEC_FIFO_SR             (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_FIFO_SR       )))
#define JPG_DEC_CMP_DATA_CNT        (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_CMP_DATA_CNT  )))
#define JPG_DEC_TRANS_ST            (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_TRANS_ST      )))
#define JPG_DEC_BUF_CMP_ST_V1       (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_BUF_CMP_ST_V1        )))   //PYTHON_V1
#define JPG_DEC_BUF_CMP_ED_V1       (JPG_BASE +(MMP_ULONG)(&(((AITPS_JPG )0)->JPG_DEC_BUF_CMP_ED_V1        )))
#endif
#define JPG_ENC_QTBL                (JPG_TBL_Q)
#endif

/// @}

#endif // _MMP_REG_JPEG_H_
