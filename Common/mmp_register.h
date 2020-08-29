//==============================================================================
//
//  File        : mmp_register.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REGISTER_H_
#define _MMP_REGISTER_H_

///@ait_only
#if defined(BUILD_FW)
#include "config_fw.h"
#else
#include "ait_config.h"
#endif


/** @addtogroup MMPH_reg
@{
*/

typedef volatile unsigned char  AIT_REG_B;
typedef volatile unsigned short AIT_REG_W;
typedef volatile unsigned int   AIT_REG_D;


// ********************************
//   Register Naming Abbreviation
// ********************************

// AUD          : audio
// ADDR         : address (new)
// ADJST        : adjust
// ACC          : accumulate
// AF           : auto focus
// AWB          : auto white balance
// AE           : auto exporsure
// AZOOM        : auto zoom
// BLD          : blend
// BS           : bit stream
// BUF          : buffer
// CFG          : configuration
// CLK          : clock
// CMD          : command
// CNT          : count, counter
// COLR         : color
// COMPEN       : compensation
// COMP         : compress
// CONV         : convert
// CSR          : control and status register
// CTL          : control (new)
// CUR          : current (new)
// DBL          : double
// DBLK         : deblocking
// DEC          : decode (new)
// DLINE        : delay line
// DNSAMP       : down sample
// DNLD         : download
// DEFT         : defect
// Dn           : divided by n
// DSI          : display serial interface
// DSPY         : display
// EN,DIS       : enable, disable
// END          : finish, the end
// FRM          : frame
// FMT          : format
// FOUT         : FIFO out
// GAMA         : gamma
// GBL          : global
// PANL         : panel
// PAL          : palette
// PRED         : prediction (new)
// ENC,DEC      : encode, decode
// FINISH       : finish, consider using DONE if possible(new)
// FREQ         : frequency
// SHFT         : shift
// HSTGRM       : histogram
// H,V          : horizontal, vertical
// INT          : interrupt
// IN,OUT       : input, output
// IDX          : index
// IMG          : image
// LCL          : local
// LS           : lenshading
// LOWBD        : lower bound (new)
// LPF          : low-pass filter
// LRP          : linear interpolation
// MTX          : matrix
// MDL          : module
// NO           : number id
// NUM          : count number
// NR           : noise reduction
// OPT          : option (new)
// OVLY         : overlay
// OFST         : offset
// PWR          : power
// PAR          : parity
// PARM         : paramter
// PHL          : peripheral
// PRIO         : priority
// PREVW        : preview
// POLAR        : polarity
// PST,NEG      : positive, negative
// PIXL         : pixel
// QLTY         : quality
// RES          : resolution (new)
// RC,WC        : read cycle, cycle
// RW,RD,WR     : read/write, read, write
// RX           : receive
// RST          : reset
// ROTE         : rotate
// SEL          : select (new)
// SRC          : source id
// STG          : storage
// ST,ED        : start and end point
// SENSR        : sensor
// SERL,PARL    : serial, parellel
// SR           : status register
// SCAL         : scaling
// SIN,SOUT     : scaling in, scaling out
// SPC          : space
// TBL          : table
// TH           : threshold (new)
// TX           : transfer
// U            : Cb (new)
// UPBND        : upper bound (new)
// UPD          : update
// V            : Cr (new), verticial
// VAL          : value (new)
// VID          : video
// WT           : weight
// W,H          : width , height
// WC           : Write cycle, RC
// WD           : watchdog
// WR           : write
// X,Y          : coordinate X, Y
//

// ADDR         : address
// BD           : bound
// BS           : bit stream
// BUF          : buffer
// CUR          : current
// CTL          : control
// FINISH       : finish, consider using DONE if possible
// HDR          : header
// LOWBD        : lower bound
// OPT          : option
// RES          : resolution
// SEL          : select
// TH           : threshold
// UPBD         : upper bound
// VAL          : value
// ===== naming that is not defined before =====
// BITSTREAM some of them uses BS

// ****************************************************************************
//                              Retina Control Register
// ****************************************************************************

// *********************************************************************
//                              Retina Peripheral Base Address
// *********************************************************************

#define AITC_BASE_OPR               ((AIT_REG_B*)   0x00000000)

#define AITC_BASE_H264DEC_CTL	    ((AITPS_H264DEC_CTL)        0x80000000)
#define AITC_BASE_H264DEC_REF	    ((AITPS_H264DEC_REF)        0x80000010)
#define AITC_BASE_H264DEC_REC	    ((AITPS_H264DEC_REC)        0x80000060)
#define AITC_BASE_H264DEC_DBLK	    ((AITPS_H264DEC_DBLK)       0x80000070)
#define AITC_BASE_H264DEC_VLD	    ((AITPS_H264DEC_VLD)        0x80000080)
#define AITC_BASE_H264DEC_PARSE_EP3 ((AITPS_H264DEC_PARSE_EP3)  0x800000F0)
#define AITC_BASE_H264DEC_MB_DATA0  ((AITPS_H264DEC_MB_DATA)    0x80000100)
#define AITC_BASE_H264DEC_MB_DATA1  ((AITPS_H264DEC_MB_DATA)    0x80000180)
#define AITC_BASE_H264ENC           ((AITPS_H264ENC)            0x80000400)

#define AITC_BASE_PWM               ((AITPS_PWM)    0x80000800)
#define AITC_BASE_USBCTL            ((AITPS_USB_CTL)0x80001000)
#define AITC_BASE_USBDMA            ((AITPS_USB_DMA)0x80001400)
#define AITC_BASE_LS                ((AIT_REG_B*)   0x80001800)

#if (CHIP == VSN_V3)
#define AITC_BASE_GBL               ((AITPS_GBL )   0x80005D00)         // GBL      Base Address
#endif
#define AITC_BASE_VIF               ((AITPS_VIF )   0x80006000)         // VIF      Base Address
#define AITC_BASE_MIPI              ((AITPS_MIPI)   0x80006110)         // MIPI     Base Address
#define AITC_BASE_JPG               ((AITPS_JPG )   0x80006200)         // JPG      Base Address
#define AITC_BASE_SD                ((AITPS_SD)     0x80006300)         // SD       Base Address
#define AITC_BASE_SCAL              ((AITPS_SCAL)   0x80006400)         // SCAL     Base Address
#define AITC_BASE_IBC               ((AITPS_IBC )   0x80006500)         // IBC      Base Address
#define AITC_BASE_GPIO              ((AITPS_GPIO)   0x80006600)         // GPIO     Base Address
#define AITC_BASE_SIF				((AITPS_SIF)    0x80006700)
#if (CHIP == VSN_V2)
#define AITC_BASE_GBL               ((AITPS_GBL )   0x80006900)         // GBL      Base Address
#endif
#define AITC_BASE_UARTB             ((AITPS_UARTB)  0x80006A00)         // US       Base Address
#define AITC_BASE_UARTB_1           ((AITPS_UARTB)  0x80005C00)         // US       Base Address
#define AITC_BASE_RAWPROC           ((AITPS_RAWPROC)0x80006B00)         // RAWPROC  Base Address
#define AITC_BASE_ICOB              ((AITPS_ICOB)   0x80006C00)         // ICOs     Base Address
#define AITC_BASE_SPIB              ((AITPS_SPIB)   0x80006D00)         // SPIs     Base Address
#define AITC_BASE_DRAM              ((AITPS_DRAM)   0x80006E00)
//#define AITC_BASE_DSI               ((AITPS_DSI)    0x80006F00)         // DSI      Base Address
#define AITC_BASE_ISP               ((AITPS_ISP )   0x80007000)         // ISP      Base Address
#define AITC_BASE_ISP1              ((AITPS_ISP1 )  0x80007100)         // ISP      Base Address
#define AITC_BASE_ISP2              ((AITPS_ISP2 )  0x80007200)         // ISP      Base Address
#define AITC_BASE_DFT               ((AIT_REG_B*)   0x80007400)
#define AITC_BASE_DMA               ((AITPS_DMA)    0x80007600)         // DMA      Base Address
#define AITC_BASE_MCI               ((AITPS_MCI)    0x80007700)
#define AITC_BASE_AUD               ((AITPS_AUD)    0x80007800)         // AUDIO    BASE Address
#define AITC_BASE_GRA               ((AITPS_GRA )   0x80007900)         // GRA      Base Address
#define AITC_BASE_PAD               ((AITPS_PAD)    0x80009C00)         // PAD      Base Address
#define AITC_BASE_I2CM              ((AITPS_I2CM)   0x80007A00)
#define AITC_BASE_TBL_Q             ((AIT_REG_B*)   0x80007C00)
#if (CHIP == VSN_V3)
#define AITC_BASE_AFE				((AITPS_AFE)	0x80007F00)			// AUDIO AFE BASE Address
#endif

////////////////////////////////////
// Register definition
//
// OPR base address
#define GBL_BASE                    ((MMP_ULONG)AITC_BASE_GBL  - (MMP_ULONG)AITC_BASE_OPR)
#define VIF_BASE                    ((MMP_ULONG)AITC_BASE_VIF  - (MMP_ULONG)AITC_BASE_OPR)
#define JPG_BASE                    ((MMP_ULONG)AITC_BASE_JPG  - (MMP_ULONG)AITC_BASE_OPR)
#define GRA_BASE                    ((MMP_ULONG)AITC_BASE_GRA  - (MMP_ULONG)AITC_BASE_OPR)
#define ICO_BASE                    ((MMP_ULONG)AITC_BASE_ICOB - (MMP_ULONG)AITC_BASE_OPR)
#define DRAM_BASE                   ((MMP_ULONG)AITC_BASE_DRAM - (MMP_ULONG)AITC_BASE_OPR)
#define SIF_BASE                    ((MMP_ULONG)AITC_BASE_SIF  - (MMP_ULONG)AITC_BASE_OPR)
#define SPI_BASE                    ((MMP_ULONG)AITC_BASE_SPIB - (MMP_ULONG)AITC_BASE_OPR)
#define IBC_BASE                    ((MMP_ULONG)AITC_BASE_IBC  - (MMP_ULONG)AITC_BASE_OPR)
#define GPIO_BASE                   ((MMP_ULONG)AITC_BASE_GPIO - (MMP_ULONG)AITC_BASE_OPR)
#define SCAL_BASE                   ((MMP_ULONG)AITC_BASE_SCAL - (MMP_ULONG)AITC_BASE_OPR)
#define I2CM_BASE                   ((MMP_ULONG)AITC_BASE_I2CM  - (MMP_ULONG)AITC_BASE_OPR)
#define RAWPROC_BASE                ((MMP_ULONG)AITC_BASE_RAWPROC - (MMP_ULONG)AITC_BASE_OPR)
#define PWM_BASE                    ((MMP_ULONG)AITC_BASE_PWM - (MMP_ULONG)AITC_BASE_OPR)
#define UARTB_BASE                  ((MMP_ULONG)AITC_BASE_UARTB - (MMP_ULONG)AITC_BASE_OPR)
#define DMA_BASE                    ((MMP_ULONG)AITC_BASE_DMA  - (MMP_ULONG)AITC_BASE_OPR)
#define H264DEC_BASE    		    ((MMP_ULONG)AITC_BASE_H264DEC_CTL  - (MMP_ULONG)AITC_BASE_OPR)
#define H264REF_BASE    		    ((MMP_ULONG)AITC_BASE_H264DEC_REF  - (MMP_ULONG)AITC_BASE_OPR)
#define H264REC_BASE    		    ((MMP_ULONG)AITC_BASE_H264DEC_REC  - (MMP_ULONG)AITC_BASE_OPR)
#define H264DBLK_BASE    		    ((MMP_ULONG)AITC_BASE_H264DEC_DBLK  - (MMP_ULONG)AITC_BASE_OPR)
#define H264VLD_BASE                ((MMP_ULONG)AITC_BASE_H264DEC_VLD  - (MMP_ULONG)AITC_BASE_OPR)
#define JPG_TBL_Q                   ((MMP_ULONG)AITC_BASE_TBL_Q  - (MMP_ULONG)AITC_BASE_OPR)
#define SD_BASE                     ((MMP_ULONG)AITC_BASE_SD - (MMP_ULONG)AITC_BASE_OPR)
//#define DSI_BASE                    ((MMP_ULONG)AITC_BASE_DSI - (MMP_ULONG)AITC_BASE_OPR)
#define H264ENC_BASE                ((MMP_ULONG)AITC_BASE_H264ENC - (MMP_ULONG)AITC_BASE_OPR)
#define AUD_BASE                    ((MMP_ULONG)AITC_BASE_AUD - (MMP_ULONG)AITC_BASE_OPR)
#define AFE_BASE                    ((MMP_ULONG)AITC_BASE_AFE - (MMP_ULONG)AITC_BASE_OPR)


/// @}

#endif // _MMPH_HIF_H_
///@end_ait_only
