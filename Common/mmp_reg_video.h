//==============================================================================
//
//  File        : mmp_reg_video.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Will Tseng/Truman Yang
//  Revision    : 1.0
//
//==============================================================================

/** @file mmp_reg_video.h
@brief The header file for the AIT chip.

@author Will Tseng
@author Truman Yang
@version 2.0 Add 83x registers
@version 1.1 V1 added
@version 1.0 82x Original version
*/


#ifndef _MMP_REG_VID_H_
#define _MMP_REG_VID_H_

#include    "mmp_register.h"


/** @addtogroup MMPH_reg
@{
*/
#if (CHIP == P_V2)
typedef struct _AITS_MEMC {
    AIT_REG_B MEMC_INT_HOST_EN;           // 0x0000
        /*-DEFINE-----------------------------------*/
        #define MEMC_FRM_ED               0x01
        /*------------------------------------------*/
    AIT_REG_B MEMC_INT_HOST_SR;           // 0x0001
    AIT_REG_B MEMC_INT_CPU_EN;            // 0x0002
    AIT_REG_B MEMC_INT_CPU_SR;            // 0x0003
    AIT_REG_D                             _x0004[3];
    AIT_REG_B MEMC_ES_CTL;                // 0x0010
        /*-DEFINE-----------------------------------*/
        #define MEMC_ES_EN                0x01
        #define MEMC_ES_HALF_PIXL         0x04
        #define MEMC_ES_UNRES_EN          0x08       
        #define MEMC_ES_LINEBUF_EN        0x02
        #define MEMC_ES_ENC_LCL_BUF_EN    0x10
		#if (CHIP == P_V2)
		#define MEMC_ES_DVS_EN            0x20
		#endif        
        /*------------------------------------------*/
    AIT_REG_B MEMC_MC_CTL;                // 0x0011
        /*-DEFINE-----------------------------------*/
        #define MEMC_MC_EN                0x01
        #define MEMC_CCIR601_EN           0x04
        /*------------------------------------------*/
    AIT_REG_B MEMC_MB_H;                  // 0x0012
    AIT_REG_B MEMC_MB_V;                  // 0x0013
    AIT_REG_B MEMC_SEARCH_CTL;            // 0x0014
    AIT_REG_B MEMC_SEARCH_WIN_SIZE;       // 0x0015
    AIT_REG_W MEMC_MB_SEL_OFST;           // 0x0016
    AIT_REG_D MEMC_MV_ADDR;               // 0x0018
	AIT_REG_D							  _x001C;
    AIT_REG_D MEMC_REF_Y_LINEBUF[4];        // 0x0020
    AIT_REG_D MEMC_REF_U_LINEBUF[4];        // 0x0030
    AIT_REG_D MEMC_REF_V_LINEBUF[4];        // 0x0040
    AIT_REG_D MEMC_Y_BUF_LOWBD;           // 0x0050
    AIT_REG_D MEMC_Y_BUF_UPBD;            // 0x0054
    AIT_REG_D                             _x0058[2];
    AIT_REG_D MEMC_U_BUF_LOWBD;           // 0x0060
    AIT_REG_D MEMC_U_BUF_UPBD;            // 0x0064
    AIT_REG_D                             _x0068[2];
    AIT_REG_D MEMC_V_BUF_LOWBD;           // 0x0070
    AIT_REG_D MEMC_V_BUF_UPBD;            // 0x0074
    AIT_REG_D                             _x0078[2];
    AIT_REG_D MEMC_REF_Y_FRM_ADDR;        // 0x0080
    AIT_REG_D MEMC_REF_U_FRM_ADDR;        // 0x0084
    AIT_REG_D MEMC_REF_V_FRM_ADDR;        // 0x0088
    AIT_REG_D                             _x008C;
    AIT_REG_D MEMC_GEN_Y_FRM_ADDR;        // 0x0090
    AIT_REG_D MEMC_GEN_U_FRM_ADDR;        // 0x0094
    AIT_REG_D MEMC_GEN_V_FRM_ADDR;        // 0x0098
    AIT_REG_D                             _x009C;
    AIT_REG_B MEMC_EMM_Y_LINEBUF;         // 0x00A0
    AIT_REG_B MEMC_EMM_U_LINEBUF;         // 0x00A1
    AIT_REG_B MEMC_EMM_V_LINEBUF;         // 0x00A2
    AIT_REG_B                             _x00A3;
    // the similar function of MEMC_EMM_CUR_ADDR_Y
    AIT_REG_B MEMC_EMM_Y_CURBUF;          // 0x00A4
    AIT_REG_B MEMC_EMM_U_CURBUF;          // 0x00A5
    AIT_REG_B MEMC_EMM_V_CURBUF;          // 0x00A6
    AIT_REG_B                             _x00A7;
    AIT_REG_B MEMC_EMM_Y_REFGEN;          // 0x00A8
    AIT_REG_B MEMC_EMM_U_REFGEN;          // 0x00A9
    AIT_REG_B MEMC_EMM_V_REFGEN;          // 0x00AA
    AIT_REG_B                             _x00AB;
 
    #if (CHIP == P_V2)
    AIT_REG_D                             _x00AC;
    AIT_REG_B MEMC_PMV_FAST_SEARCH;       // 0x00B0 
    AIT_REG_B MEMC_PMV_SEARCH_LIMIT;      // 0x00B1
    AIT_REG_B MEMC_PMV_SAD_TH_LSB;        // 0x00B2
    AIT_REG_B MEMC_PMV_SAD_TH_MSB;        // 0x00B3
    AIT_REG_B MEMC_PMV_TO_SQUARE_CNT;     // 0x00B4
    AIT_REG_B                             _x00B5[8];
    AIT_REG_B MEMC_I_BLK_IN_P;            // 0x00BD
    //AIT_REG_B                             _x00BE[50];
    AIT_REG_B                             _x00BE[2];
    AIT_REG_D MEMC_BAK_Y_FRM_ADDR;        // 0x00C0
    AIT_REG_D MEMC_BAK_U_FRM_ADDR;        // 0x00C4
    AIT_REG_D MEMC_BAK_V_FRM_ADDR;        // 0x00C8
    #if (CHIP == P_V2)
    AIT_REG_D                             _x00CC;
    AIT_REG_B MEMC_DVS_FRM_EN;             // 0x00D0
        /*-DEFINE-----------------------------------*/
        #define MEMC_DVS_EN                0x01
        /*------------------------------------------*/
    AIT_REG_B                             _x00D1[31];
    #else
    AIT_REG_D                             _x00CC[9];
    #endif
	
#endif
    AIT_REG_B MEMC_TEST_MODE;             // 0x00F0
    AIT_REG_B                             _x00F1[15];
} AITS_MEMC, *AITPS_MEMC;

typedef struct _AITS_VID {
    AIT_REG_W VID_ENC_H;							// 0x0000
    AIT_REG_W VID_ENC_V;	              			// 0x0002
    AIT_REG_B VID_DCAC_PRED_TH;           			// 0x0004
    AIT_REG_B VID_DCAC_PRED_EN;          		 	// 0x0005
        /*-DEFINE-----------------------------------*/
        #define ACDC_DYN_PRED_EN        0x0003
        #define	ACDC_PRED_EN		    0x0002
        #define	DYNA_ACDC_PRED	        0x0001
        #define	ACDC_PRED_DIS	        0x0000
        /*------------------------------------------*/
    AIT_REG_B VID_ENC_CFG;							// 0x0006
        /*-DEFINE-----------------------------------*/
        #define	VID_HW_HDR_EN           0x0000
        /*------------------------------------------*/
    AIT_REG_B 							  _x0007;
    AIT_REG_W VID_VOP_TIME_RES;           			// 0x0008
    AIT_REG_W VID_FIXED_VOP_TIME_INCR;    			// 0x000A
    AIT_REG_B VID_ENC_FRM_CTL;            			// 0x000C
    	/*-DEFINE-----------------------------------*/
		#define VID_ENC_MODULO_TIME		0x0001
		#define VID_ENC_ROUNDING		0x0004
		#define VID_ENC_NOROUND			0x0000
		#define VID_ENC_TYPE_I			0x0002
		#define VID_ENC_TYPE_P			0x0000
		/*------------------------------------------*/
    AIT_REG_B                             _x000D[3];
    AIT_REG_W VID_DEC_DATA_PORT;          			// 0x0010
    AIT_REG_W VID_DEC_ADDR_PORT;          			// 0x0012
    AIT_REG_W VID_DEC_CTL0;               			// 0x0014
        /*-DEFINE-----------------------------------*/
        #define H_SCAN_PATTERN          0x0008
        #define V_SCAN_PATTERN          0x0004
        #define ZIG_ZAG_SCAN_PATTERN    0x0002
        #define ST_DEC_MB               0x0001
        #define ROUNDING_TYPE_SEL       0x0400
        #define MB_TYPE_INTRA           0x0200
        #define MB_TYPE_INTER           0x0000
        #define FRM_DEC_TYPE_I          0x0100
        #define FRM_DEC_TYPE_P          0x0000
        /*------------------------------------------*/
    AIT_REG_B VID_DEC_CTL2;               			// 0x0016
        /*-DEFINE-----------------------------------*/
        #define MEMC_VECTOR_SET         0x01
        /*------------------------------------------*/
    AIT_REG_B                             _x0017;
    AIT_REG_B 							  _x0018;
    AIT_REG_B VID_ENGINE_SR;              			// 0x0019
        /*-DEFINE-----------------------------------*/
        #define VID_CODEC_BUSY          0x01
        /*------------------------------------------*/
    AIT_REG_B                             _x001A[22];
    AIT_REG_D VID_INT_CPU_EN;             			// 0x0030
        /*-DEFINE-----------------------------------*/
        #define VID_FRM_ED				0x02
        #define VID_FRM_ST				0x01
        /*------------------------------------------*/
    AIT_REG_D VID_INT_CPU_SR;             			// 0x0034
    AIT_REG_D VID_INT_HOST_EN;            			// 0x0038
    AIT_REG_D VID_INT_HOST_SR;            			// 0x003C
    AIT_REG_W VID_MV_X_1ST;               			// 0x0040
    AIT_REG_W VID_MV_Y_1ST;               			// 0x0042
    AIT_REG_W VID_MV_X_2ND;               			// 0x0044
    AIT_REG_W VID_MV_Y_2ND;               			// 0x0046
    AIT_REG_W VID_MV_X_3RD;               			// 0x0048
    AIT_REG_W VID_MV_Y_3RD;               			// 0x004A
    AIT_REG_W VID_MV_X_4TH;               			// 0x004C
    AIT_REG_W VID_MV_Y_4TH;               			// 0x004E
    AIT_REG_W VID_MP4_ENC_CTL;            			// 0x0050
        /*-DEFINE-----------------------------------*/
        #define VID_MP4_ENC_EN          0x01
        /*------------------------------------------*/
    AIT_REG_W VID_MP4_DEC_CTL;            			// 0x0052
        /*-DEFINE-----------------------------------*/
        #define VID_MP4_DEC_EN          0x01
        /*------------------------------------------*/
    AIT_REG_W VID_H263_ENC_CTL;           			// 0x0054
        /*-DEFINE-----------------------------------*/
        #define VID_H263_ENC_EN         0x01
        /*------------------------------------------*/
    AIT_REG_W VID_H263_DEC_CTL;           			// 0x0056
        /*-DEFINE-----------------------------------*/
        #define VID_H263_DEC_EN         0x01
        #if (CHIP == P_V2)
        // ++ PYTHON V1
        #define VID_CUT_SEG_EN          0x02
        // -- PYTHON V1
        #endif
        /*------------------------------------------*/
    AIT_REG_B VID_I_FRM_QP;               // 0x0058
    AIT_REG_B VID_P_FRM_QP;               // 0x0059
    AIT_REG_B VID_H263_CFG;               // 0x005A
        /*-DEFINE-----------------------------------*/
        #define VID_H263_RES_128x96     0x01
        #define VID_H263_RES_176x144    0x02
        #define VID_H263_RES_352x288    0x03
        #define VID_H263_PROFILE_0_EN   0x08
        /*------------------------------------------*/
    AIT_REG_B                             _x005B;
    AIT_REG_B VID_DEC_CFG;                // 0x005C
        /*-DEFINE-----------------------------------*/
        #define VID_VLD_PRED_EN         0x01
		#define VID_VLD_H263_SHORT_HDR	0x02
		#if (CHIP == P_V2)
		// ++ PYTHON V1
		#define VID_MP4_QTYPE_EN        0x04
		#define VID_MP4_QMATRIX_EN      0x08
		// -- PYTHON V1
		#endif
        /*------------------------------------------*/
    AIT_REG_B                             _x005D;
    #if (CHIP == P_V2)
    // ++ PYTHON V1
    AIT_REG_W VID_LOAD_QMATRIX_V1;                  // 0x005E
    // -- PYTHON V1
    #endif
    AIT_REG_D VID_ENC_CUR_BUF_ADDR_Y0;    			// 0x0060
    AIT_REG_D VID_ENC_CUR_BUF_ADDR_U0;    			// 0x0064
    AIT_REG_D VID_ENC_CUR_BUF_ADDR_V0;    			// 0x0068
    AIT_REG_D                             _x006C;
    AIT_REG_D							  _x0070[3];
    AIT_REG_D VID_ENC_BSOUT_ST;     	 			// 0x007C
    AIT_REG_D VID_ENC_BSOUT_CUR;     	 			// 0x0080
    AIT_REG_D VID_ENC_BSOUT_ED;		      			// 0x0084
    AIT_REG_D VID_ENC_BSOUT_STS_ADDR;     			// 0x0088
    AIT_REG_D VID_ENC_TOTAL_STS_ADDR;     			// 0x008C
    AIT_REG_D VID_ENC_FRM_SIZE;           			// 0x0090
	AIT_REG_D							  _x0094[11];
    AIT_REG_D VID_ENC_Y_DC_ADDR;       				// 0x00C0
    AIT_REG_D VID_ENC_Y_AC_ADDR;       				// 0x00C4
    AIT_REG_D VID_ENC_C_DC_ADDR;    	   			// 0x00C8
    AIT_REG_D VID_ENC_C_AC_ADDR;	       			// 0x00CC
	AIT_REG_B							  _x00D0[4];
	AIT_REG_D							  _x00D4[7];
    AIT_REG_B VID_HDR_DATA_PORT;          			// 0x00F0
    AIT_REG_B                             _0x00F1[15];
} AITS_VID, *AITPS_VID;


//------------------------------
// VLD structure (0x8000 B400)
//------------------------------

typedef struct _AITS_VLD {
    AIT_REG_B VLD_CTL_0;                    // 0x0000
        /*-DEFINE-----------------------------------*/
        #if 1 //Retina
        #define VLD_ST_EN                0x01
        #else //Retina-V1
        #define VLD_STS                  0x02 // read-only
        #define VLD_ST_EN                0x01 // start next seg
        #endif
        /*------------------------------------------*/
    AIT_REG_B                               _x0001;
    AIT_REG_B VLD_CTL_1;                    // 0x0002
    AIT_REG_B                               _x0003;
    AIT_REG_B VLD_CFG_0;                    // 0x0004
        /*-DEFINE-----------------------------------*/
        #if 0
        #define VLD_VID_FMT_MPEG4        0x00
        #define VLD_VID_FMT_H263         0x01
        #else
        #define VLD_H263_SHORT_HDR       0x01
        #endif
        #define MB_DATA_PARTITIONED      0x08
        #define VLD_RVLC_EN              0x10
        #define VLD_VOP_LEVEL            0x00
        #define VLD_VP_LEVEL             0x40
        #define VLD_MB_LEVEL             0x80
        /*------------------------------------------*/
    AIT_REG_B VLD_CFG_1;                    // 0x0005
        /*-DEFINE-----------------------------------*/
        #if (CHIP == P_V2)
        #define VLD_MP4ASP_DEC_ENABLE    0x10
        #endif
        /*------------------------------------------*/
    AIT_REG_B VLD_VOP_TIME_INC_BIT_NUM;     // 0x0006
    AIT_REG_B VLD_MB_NUM_BIT;               // 0x0007
    AIT_REG_B VLD_H263_SRC_FMT;             // 0x0008
    AIT_REG_B                               _x0009[3];
    AIT_REG_W VLD_MDLO_TIME_BASE;           // 0x000C
    AIT_REG_W VLD_VOP_TIME_INCR;            // 0x000E
    AIT_REG_B VLD_VOP_HDR;                  // 0x0010
    AIT_REG_B VLD_H263_TEMPORAL_REF;        // 0x0011
    AIT_REG_B VLD_H263_PSUPP;               // 0x0012
    AIT_REG_B VLD_H263_GOB_LAYER_HDR;       // 0x0013
    AIT_REG_W VLD_MB_NUM;                   // 0x0014
    AIT_REG_W                               _x0016;
    AIT_REG_B VLD_VOP_QP;                   // 0x0018
    AIT_REG_B VLD_VOP_DC_TH;                // 0x0019
    AIT_REG_B VLD_VOP_FCODE;                // 0x001A
    AIT_REG_B                               _x001B;
    AIT_REG_D VLD_DATA_PARTITION_ADDR;      // 0x001C
    AIT_REG_D VLD_BLK_COEF_OUT_ADDR;        // 0x0020
    AIT_REG_D VLD_BS_ADDR;                  // 0x0024
    AIT_REG_B VLD_BS_ST_BIT;                // 0x0028
    AIT_REG_B                               _x0029[3];
    AIT_REG_B VLD_INT_CPU_EN;               // 0x002C
        /*-DEFINE-----------------------------------*/
        #define VLD_FRM_ED               0x01
        #define VLD_SEG_ED               0x02
        /*------------------------------------------*/
    AIT_REG_B VLD_INT_CPU_SR;               // 0x002D
    AIT_REG_B VLD_INT_HOST_EN;              // 0x002E
    AIT_REG_B VLD_INT_HOST_SR;              // 0x002F
    AIT_REG_B VLD_INT_STS;                  // 0x0030
        /*-DEFINE-----------------------------------*/
        #define VLD_STS_DONE             0x01
        #define VLD_STS_NOT_CODED        0x02
        #define VLD_STS_DEC_ERROR        0x04
        // 83x only ++
        #define VLD_STS_LEN_UNDERFLOW    0x08
        #define VLD_STS_LEN_OVERFLOW     0x10
        // 83x only --
        #if (CHIP == P_V2)
        #define VID_MB_NOT_FULL             0x20
        #endif
        /*------------------------------------------*/
    AIT_REG_B                               _x0031;
    AIT_REG_B VLD_VOP_HDR_DONE;             // 0x0032
    AIT_REG_B                               _x0033;
    AIT_REG_B VLD_SRAM_BUF_DELSEL;          // 0x0034
    AIT_REG_B                               _x0035[11];
    AIT_REG_B VLD_PRED_CFG;                 // 0x0040
        #define VLD_QP_OUT_ONLY          0x02
        #define VLD_MB_INFO_OUT          0x01
    AIT_REG_B                               _x0041[3];
    AIT_REG_B VLD_VOP_W;                    // 0x0044
    AIT_REG_B                               _x0045;
    AIT_REG_W VLD_VOP_MB_NUM;               // 0x0046
    AIT_REG_D VLD_MV_PRED_BUF_ADDR;         // 0x0048
    AIT_REG_D VLD_MV_OUT_ADDR;              // 0x004C
    AIT_REG_D VLD_BS_LOWBD;                 // 0x0050;
    AIT_REG_D VLD_BS_UPBD;                  // 0x0054
    AIT_REG_D VLD_BS_LEN;                   // 0x0058, 3 bytes data
    AIT_REG_D VLD_SEG_LEN;                  // 0x005C, 3 bytes data
    AIT_REG_B VLD_LAST_SEG;                 // 0x0060;
        /*-DEFINE-----------------------------------*/
        #define VLD_IS_LAST_SEG             0x01
        /*------------------------------------------*/
    AIT_REG_B                               _x0061[3];
    AIT_REG_B VLD_BS_BUF_OPT;               // 0x0064
        /*-DEFINE-----------------------------------*/
        #define VLD_REFETCH_SEG_ST_ADDR_EN  0x02 // Set 1 to refetch the start address. 0 to continue to get next 32 bytes
        #define VLD_RING_BUF_EN             0x01 // Use ring buffer. 1 for ring buffer. 0 for plain buffer
        /*------------------------------------------*/
    AIT_REG_B                               _x0065[3];
    AIT_REG_D VLD_CUR_BIT;                  // 0X0068, 20 bits data
	#if (CHIP == P_V2)
    AIT_REG_B                               _x006C[4];
    AIT_REG_W VLD_BSCALE_FW;                // 0x0070
    AIT_REG_W VLD_BSCALE_BW;                // 0x0072
	#endif
} AITS_VLD, *AITPS_VLD;


//-------------------------------
// PRED structure (0x8000 B800)
//-------------------------------

typedef struct _AITS_PRED {
    AIT_REG_B PRED_SW_ST;                // 0x0000
    AIT_REG_B                            _x0001[3];
    AIT_REG_B PRED_INT_CPU_EN;           // 0x0004
    AIT_REG_B PRED_INT_CPU_SR;           // 0x0005
    AIT_REG_B PRED_INT_HOST_EN;          // 0x0006
    AIT_REG_B PRED_INT_HOST_SR;          // 0x0007
    AIT_REG_B PRED_SW_RST;               // 0x0008
    AIT_REG_B                            _x0009[3];
    AIT_REG_B PRED_CFG;                  // 0x000C
    AIT_REG_B                            _x000D;
    AIT_REG_B PRED_VOP_W;                // 0x000E
    AIT_REG_B                            _x000F;
    AIT_REG_D PRED_H_BUF_ADDR;           // 0x0010
    AIT_REG_D PRED_V_BUF_ADDR;           // 0x0014
    AIT_REG_D                            _x0018[5];
    AIT_REG_B PRED_MB_CFG;               // 0x002C;
    AIT_REG_B                            _x002D[3];
    AIT_REG_W PRED_VP_NUM;               // 0x0030
    AIT_REG_W PRED_MB_NUM;               // 0x0032
    AIT_REG_D                            _x0034;
    AIT_REG_B PRED_BLK_NUM;              // 0x0038
    AIT_REG_B PRED_COEF_IDX;             // 0x0039
    AIT_REG_B                            _x003A[6];
} AITS_PRED, *AITPS_PRED;


//--------------------------------
// DBLK structure (0x8000 BC00)
//--------------------------------
typedef struct _AITS_DBLK {
    AIT_REG_B DBLK_CTL;                    // 0x0000
        /*-DEFINE-----------------------------------*/
        #define DBLK_SW_ST                 0x01
        /*------------------------------------------*/
    AIT_REG_B                              _x0001[3];
    AIT_REG_B DBLK_INT_HOST_EN;            // 0x0004
    AIT_REG_B DBLK_INT_HOST_SR;            // 0x0005
    AIT_REG_B DBLK_INT_CPU_EN;             // 0x0006
        /*-DEFINE-----------------------------------*/
        #define DBLK_FRM_ED_EN             0x01
        /*------------------------------------------*/
    AIT_REG_B DBLK_INT_CPU_SR;             // 0x0007
        /*-DEFINE-----------------------------------*/
        #define DBLK_FRM_ED_SR             0x01
        /*------------------------------------------*/
    AIT_REG_D                              _x0008[2];
    AIT_REG_D DBLK_Y_SRC_ADDR;             // 0x0010
    AIT_REG_D DBLK_U_SRC_ADDR;             // 0x0014
    AIT_REG_D DBLK_V_SRC_ADDR;             // 0x0018
    AIT_REG_D DBLK_MB_QP_ST_ADDR;          // 0x001C
    AIT_REG_D DBLK_Y_OUT_ADDR;             // 0x0020
    AIT_REG_D DBLK_U_OUT_ADDR;             // 0x0024
    AIT_REG_D DBLK_V_OUT_ADDR;             // 0x0028
    AIT_REG_D                              _x002C;
    AIT_REG_B DBLK_FRM_NUM_MB_W;           // 0x0030
    AIT_REG_B DBLK_FRM_NUM_MB_H;           // 0x0031
    AIT_REG_W                              _x0032;
    AIT_REG_B DBLK_PARAM;                  // 0x0034
        /*-DEFINE-----------------------------------*/
        #define DBLK_K3_EN                 0x04
        #define DBLK_QP_BY_MB              0x02
        #define DBLK_Y_ONLY                0x01
        /*------------------------------------------*/
    AIT_REG_B                               _x0035;
    AIT_REG_B DBLK_FRM_QP;                 // 0x0036
    AIT_REG_B                               _x0037;
    AIT_REG_B DBLK_TH;                     // 0x0038
    AIT_REG_B DBLK_TH_SCAL;                // 0x0039
    AIT_REG_B DBLK_TH_OFST;                // 0x003A
    AIT_REG_B                              _x003B;
    AIT_REG_B DBLK_TH_ADJNT;               // 0x003C
    AIT_REG_B DBLK_K1_K2;                  // 0x003D
    AIT_REG_B DBLK_K3;                     // 0x003E
    AIT_REG_B                              _x003F;
} AITS_DBLK, *AITPS_DBLK;

#if !defined(BUILD_FW)
// MEMC OPR
#define MEMC_INT_HOST_EN          (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_INT_HOST_EN    ))) // 0x0000
#define MEMC_INT_HOST_SR          (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_INT_HOST_SR    ))) // 0x0001
#define MEMC_INT_CPU_EN           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_INT_CPU_EN     ))) // 0x0002
#define MEMC_INT_CPU_SR           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_INT_CPU_SR     ))) // 0x0003
#define MEMC_ES_CTL               (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_ES_CTL         ))) // 0x0010
#define MEMC_MC_CTL               (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_MC_CTL         ))) // 0x0011
#define MEMC_MB_H                 (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_MB_H           ))) // 0x0012
#define MEMC_MB_V                 (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_MB_V           ))) // 0x0013
#define MEMC_SEARCH_CTL           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_SEARCH_CTL     ))) // 0x0014
#define MEMC_SEARCH_WIN_SIZE      (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_SEARCH_WIN_SIZE))) // 0x0015
#define MEMC_MB_SEL_OFST          (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_MB_SEL_OFST    ))) // 0x0016
#define MEMC_MV_ADDR              (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_MV_ADDR        ))) // 0x0018
#define MEMC_REF_Y_LINEBUF0       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_REF_Y_LINEBUF[0]))) // 0x0020
#define MEMC_REF_U_LINEBUF0       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_REF_U_LINEBUF[0]))) // 0x0030
#define MEMC_REF_V_LINEBUF0       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_REF_V_LINEBUF[0]))) // 0x0040
#define MEMC_Y_BUF_LOWBD          (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_Y_BUF_LOWBD    ))) // 0x0050
#define MEMC_Y_BUF_UPBD           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_Y_BUF_UPBD     ))) // 0x0054
#define MEMC_U_BUF_LOWBD          (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_U_BUF_LOWBD    ))) // 0x0060
#define MEMC_U_BUF_UPBD           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_U_BUF_UPBD     ))) // 0x0064
#define MEMC_V_BUF_LOWBD          (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_V_BUF_LOWBD    ))) // 0x0070
#define MEMC_V_BUF_UPBD           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_V_BUF_UPBD     ))) // 0x0074
#define MEMC_REF_Y_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_REF_Y_FRM_ADDR ))) // 0x0080
#define MEMC_REF_U_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_REF_U_FRM_ADDR ))) // 0x0084
#define MEMC_REF_V_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_REF_V_FRM_ADDR ))) // 0x0088
#define MEMC_GEN_Y_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_GEN_Y_FRM_ADDR ))) // 0x0090
#define MEMC_GEN_U_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_GEN_U_FRM_ADDR ))) // 0x0094
#define MEMC_GEN_V_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_GEN_V_FRM_ADDR ))) // 0x0098
#define MEMC_EMM_Y_LINEBUF        (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_Y_LINEBUF  ))) // 0x00A0
#define MEMC_EMM_U_LINEBUF        (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_U_LINEBUF  ))) // 0x00A1
#define MEMC_EMM_V_LINEBUF        (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_V_LINEBUF  ))) // 0x00A2
#define MEMC_EMM_Y_CURBUF         (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_Y_CURBUF   ))) // 0x00A4
#define MEMC_EMM_U_CURBUF         (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_U_CURBUF   ))) // 0x00A5
#define MEMC_EMM_V_CURBUF         (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_V_CURBUF   ))) // 0x00A6
#define MEMC_EMM_Y_REFGEN         (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_Y_REFGEN   ))) // 0x00A8
#define MEMC_EMM_U_REFGEN         (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_U_REFGEN   ))) // 0x00A9
#define MEMC_EMM_V_REFGEN         (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_EMM_V_REFGEN   ))) // 0x00AA
#define MEMC_PMV_FAST_SEARCH      (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_PMV_FAST_SEARCH))) // 0x00B0
#define MEMC_PMV_SEARCH_LIMIT     (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_PMV_SEARCH_LIMIT)))// 0x00B1
#define MEMC_PMV_SAD_TH_LSB       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_PMV_SAD_TH_LSB ))) // 0x00B2
#define MEMC_PMV_SAD_TH_MSB       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_PMV_SAD_TH_MSB ))) // 0x00B3
#define MEMC_PMV_TO_SQUARE_CNT    (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_PMV_TO_SQUARE_CNT)))// 0x00B4
#define MEMC_I_BLK_IN_P           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_I_BLK_IN_P     ))) // 0x00BD

#define MEMC_BAK_Y_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_BAK_Y_FRM_ADDR ))) // 0x00C0
#define MEMC_BAK_U_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_BAK_U_FRM_ADDR ))) // 0x00C4
#define MEMC_BAK_V_FRM_ADDR       (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_BAK_V_FRM_ADDR ))) // 0x00C8
#define MEMC_DVS_FRM_EN           (MEMC_BASE + (MMP_ULONG)(&(((AITPS_MEMC)0)->MEMC_DVS_FRM_EN     ))) // 0x00D0

// VID OPR
#define VID_ENC_H                 (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_H               ))) // 0x0000
#define VID_ENC_V                 (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_V               ))) // 0x0002
#define VID_DCAC_PRED_TH;         (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_DCAC_PRED_TH        ))) // 0x0004
#define VID_DCAC_PRED_EN          (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_DCAC_PRED_EN        ))) // 0x0005
#define VID_ENC_CFG               (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_CFG             ))) // 0x0006
#define VID_VOP_TIME_RES          (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_VOP_TIME_RES        ))) // 0x0008
#define VID_FIXED_VOP_TIME_INCR   (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_FIXED_VOP_TIME_INCR ))) // 0x000A
#define VID_ENC_FRM_CTL   		  (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_FRM_CTL 		))) // 0x000C
#define VID_CODEC_QP_SR           (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_CODEC_QP_SR         ))) // 0x0018
#define VID_ENGINE_SR             (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENGINE_SR           ))) // 0x0019
#define VID_INT_CPU_EN            (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_INT_CPU_EN          ))) // 0x0030
#define VID_INT_CPU_SR            (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_INT_CPU_SR          ))) // 0x0034
#define VID_INT_HOST_EN           (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_INT_HOST_EN         ))) // 0x0038
#define VID_INT_HOST_SR           (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_INT_HOST_SR         ))) // 0x003C
#define VID_I_FRM_QP              (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_I_FRM_QP            ))) // 0x0058
#define VID_P_FRM_QP              (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_P_FRM_QP            ))) // 0x0059
#define VID_H263_CFG              (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_H263_CFG            ))) // 0x005A
#define VID_ENC_CUR_BUF_ADDR_Y0   (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_CUR_BUF_ADDR_Y0 ))) // 0x0060
#define VID_ENC_CUR_BUF_ADDR_U0   (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_CUR_BUF_ADDR_U0 ))) // 0x0064
#define VID_ENC_CUR_BUF_ADDR_V0   (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_CUR_BUF_ADDR_V0 ))) // 0x0068
#define VID_ENC_CUR_BUF_ADDR_Y1   (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_CUR_BUF_ADDR_Y1 ))) // 0x0070
#define VID_ENC_CUR_BUF_ADDR_U1   (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_CUR_BUF_ADDR_U1 ))) // 0x0074
#define VID_ENC_CUR_BUF_ADDR_V1   (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_CUR_BUF_ADDR_V1 ))) // 0x0078
#define VID_ENC_BSOUT_ST     	  (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_BSOUT_ST   		))) // 0x0080
#define VID_ENC_BSOUT_ED	      (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_BSOUT_ED		))) // 0x0084
#define VID_ENC_Y_DC_ADDR      	  (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_Y_DC_ADDR 	    ))) // 0x00C0
#define VID_ENC_Y_AC_ADDR     	  (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_Y_AC_ADDR	    ))) // 0x00C4
#define VID_ENC_C_DC_ADDR 	      (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_C_DC_ADDR	    ))) // 0x00C8
#define VID_ENC_C_AC_ADDR         (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_C_AC_ADDR	    ))) // 0x00CC
#define VID_ENC_FRM_SEL_BASE      (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_FRM_SEL_BASE    )))
#define VID_ENC_FRM_SEL_BIT       (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_FRM_SEL_BIT     )))
#define VID_ENC_BSOUT_STS_ADDR    (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_BSOUT_STS_ADDR  )))
#define VID_ENC_TOTAL_STS_ADDR    (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_TOTAL_STS_ADDR  )))
#define VID_ENC_FRM_SIZE          (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_FRM_SIZE        )))
#define VID_ENC_TOTAL_FRM_SIZE    (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_TOTAL_FRM_SIZE  )))
#define VID_ENC_BSOUT_CUR     	  (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_ENC_BSOUT_CUR 		))) // 0x0080
#define VID_MP4_ENC_CTL           (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_MP4_ENC_CTL         ))) // 0x0050
#define VID_MP4_DEC_CTL           (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_MP4_DEC_CTL         ))) // 0x0052
#define VID_H263_ENC_CTL          (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_H263_ENC_CTL        ))) // 0x0054
#define VID_H263_DEC_CTL          (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_H263_DEC_CTL        ))) // 0x0056
#define VID_DEC_CFG               (VID_BASE + (MMP_ULONG)(&(((AITPS_VID)0)->VID_DEC_CFG             ))) // 0x005C

// VLD OPR
#define VLD_CTL_0		          (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_CTL_0               ))) // 0x0000
#define VLD_CFG_0                 (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_CFG_0               ))) // 0x0004
#define VLD_CFG_1                 (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_CFG_1               ))) // 0x0005
#define VLD_VOP_TIME_INC_BIT_NUM  (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_VOP_TIME_INC_BIT_NUM))) // 0x0006
#define VLD_MB_NUM_BIT            (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_MB_NUM_BIT          ))) // 0x0007
#define VLD_VOP_QP	              (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_VOP_QP              ))) // 0x0018
#define VLD_DATA_PARTITION_ADDR	  (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_DATA_PARTITION_ADDR ))) // 0x001C
#define VLD_INT_CPU_SR            (VID_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_INT_CPU_SR          ))) // 0x002D
#define VLD_BS_ADDR		          (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BS_ADDR             ))) // 0x0024
#define VLD_BS_ST_BIT		      (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BS_ST_BIT           ))) // 0x0028
#define VLD_INT_STS               (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_INT_STS             ))) // 0x0030
#define VLD_VOP_W                 (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_VOP_W               ))) // 0x0044
#define VLD_VOP_MB_NUM            (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_VOP_MB_NUM          ))) // 0x0046
#define VLD_MV_PRED_BUF_ADDR      (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_MV_PRED_BUF_ADDR    ))) // 0x0048
#define VLD_BS_LOWBD              (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BS_LOWBD            ))) // 0x0050
#define VLD_BS_UPBD               (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BS_UPBD             ))) // 0x0054
#define VLD_BS_LEN                (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BS_LEN              ))) // 0x0058
#define VLD_SEG_LEN               (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_SEG_LEN             ))) // 0x005C
#define VLD_LAST_SEG              (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_LAST_SEG            ))) // 0x0060
#define VLD_BS_BUF_OPT            (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BS_BUF_OPT          ))) // 0x0064
#define VLD_CUR_BIT               (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_CUR_BIT             ))) // 0x0068

#define VLD_BSCALE_FW             (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BSCALE_FW         ))) // 0x0070
#define VLD_BSCALE_BW             (VLD_BASE + (MMP_ULONG)(&(((AITPS_VLD)0)->VLD_BSCALE_BW         ))) // 0x0072

// PRED OPR
#define PRED_VOP_W	              (PRED_BASE + (MMP_ULONG)(&(((AITPS_PRED)0)->PRED_VOP_W            ))) // 0x000E
#define PRED_H_BUF_ADDR	          (PRED_BASE + (MMP_ULONG)(&(((AITPS_PRED)0)->PRED_H_BUF_ADDR       ))) // 0x0010
#define PRED_V_BUF_ADDR		      (PRED_BASE + (MMP_ULONG)(&(((AITPS_PRED)0)->PRED_V_BUF_ADDR       ))) // 0x0014


// RTNA-DBLK OPR
#define DBLK_CTL	              (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_CTL              ))) // 0x0000
#define DBLK_FRM_W	              (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_FRM_W            ))) // 0x0004
#define DBLK_FRM_H	              (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_FRM_H            ))) // 0x0006
#define DBLK_TH                   (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_TH               ))) // 0x0008
#define DBLK_QP                   (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_QP               ))) // 0x0009
#define DBLK_PARM                 (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_PARM             ))) // 0x000A
#define DBLK_Y_ONLY_OPT           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_Y_ONLY_OPT       ))) // 0x000B
#define DBLK_Y_ST_ADDR            (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_Y_ST_ADDR        ))) // 0x0010
#define DBLK_U_ST_ADDR            (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_U_ST_ADDR        ))) // 0x0014
#define DBLK_V_ST_ADDR            (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_V_ST_ADDR        ))) // 0x0018
#define DBLK_Y_SRC_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_Y_SRC_ADDR       ))) // 0x0010
#define DBLK_U_SRC_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_U_SRC_ADDR       ))) // 0x0014
#define DBLK_V_SRC_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_V_SRC_ADDR       ))) // 0x0018
#define DBLK_Y_BUF_LOWBD          (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_Y_BUF_LOWBD      ))) // 0x001C

#define DBLK_MB_QP_ST_ADDR        (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_MB_QP_ST_ADDR    ))) // 0x001C

#define DBLK_Y_BUF_UPBD           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_Y_BUF_UPBD       ))) // 0x0020
#define DBLK_U_BUF_LOWBD          (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_U_BUF_LOWBD      ))) // 0x0024
#define DBLK_U_BUF_UPBD           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_U_BUF_UPBD       ))) // 0x0028
#define DBLK_V_BUF_LOWBD          (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_V_BUF_LOWBD      ))) // 0x002C
#define DBLK_V_BUF_UPBD           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_V_BUF_UPBD       ))) // 0x0030

// DMND-DBLK OPR
#define DBLK_Y_SRC_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_Y_SRC_ADDR       ))) // 0x0010
#define DBLK_U_SRC_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_U_SRC_ADDR       ))) // 0x0014
#define DBLK_V_SRC_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_V_SRC_ADDR       ))) // 0x0018
#define DBLK_Y_OUT_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_Y_OUT_ADDR       ))) // 0x0020
#define DBLK_U_OUT_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_U_OUT_ADDR       ))) // 0x0024
#define DBLK_V_OUT_ADDR           (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_V_OUT_ADDR       ))) // 0x0028
#define DBLK_FRM_NUM_MB_W         (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_FRM_NUM_MB_W     ))) // 0x0030
#define DBLK_FRM_NUM_MB_H         (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_FRM_NUM_MB_H     ))) // 0x0031
#define DBLK_PARAM                (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_PARAM            ))) // 0x0034
#define DBLK_FRM_QP               (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_FRM_QP           ))) // 0x0036
#define DBLK_TH_SCAL              (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_TH_SCAL          ))) // 0x0039
#define DBLK_TH_OFST              (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_TH_OFST          ))) // 0x003A
#define DBLK_TH_ADJNT             (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_TH_ADJNT         ))) // 0x003C
#define DBLK_K1_K2                (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_K1_K2            ))) // 0x003D
#define DBLK_K3                   (DBLK_BASE + (MMP_ULONG)(&(((AITPS_DBLK)0)->DBLK_K3               ))) // 0x003E

#endif //#if !defined(BUILD_FW)

#endif //#if (CHIP == P_V2)
/// @}
#endif // _MMPH_REG_VID_H_
