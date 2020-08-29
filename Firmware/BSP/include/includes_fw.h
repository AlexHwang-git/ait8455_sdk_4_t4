//==============================================================================
//
//  File        : includes_fw.h
//  Description : Top level global definition and configuration.
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================



#ifndef _INCLUDES_FW_H_
#define _INCLUDES_FW_H_

#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>

#include    "ucos_ii.h"

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================
#include    "mmp_err.h"
#include    "config_fw.h"

// *** Use this flag to config sensor to use HW or SW I2C ***
#define USB_DMA_SINGLE_PACKET


#define SYS_Q1_SIZE             (10)

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#include    "mmpf_task_cfg.h"

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
#include    "mmpf_typedef.h"

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
/** @brief Memory set functions extactly like memset in standard c library.

The uFS uses FS_MEMSET marco to memset too. So we just use it here.
And this file has included string.h already.
There is a deprecated version function in sys.c.
@note Please try to use this function for faster memset and reduce the code size.
*/
#define MEMSET(s,c,n)       memset(s,c,n)
/// Set this variable to 0
#define MEMSET0(s)      memset(s,0,sizeof(*s))
#define MEMCPY(d, s, c) memcpy (d, s, c)

void                MMPF_DSC_TaskHandler(void *p_arg);

void                MMPF_MP4VENC_TaskHandler(void *p_arg);
void                MMPF_VIDMGR_TaskHandler(void *p_arg);
void                MMPF_VIDPLAY_TaskHandler(void *p_arg);
void                MMPF_3GPPARSR_TaskHandler(void *p_arg);
void                MMPF_VIDDEC_TaskHandler(void *p_arg);
void                MMPF_SENSOR_TaskHandler(void *p_arg);
void                MMPF_FS_TaskHandler(void *p_arg);
void                MMPF_AMRPLAY_TaskHandler(void *p_arg);
void                MMPF_AUDIOPLAY_TaskHandler(void *p_arg);
void                MMPF_AACPLAY_TaskHandler(void *p_arg);
void                MMPF_AUDIOSTREAMING_TaskHandler(void *p_arg);
void                MMPF_USB_EP1_TaskHandler(void *p_arg);
void                MMPF_MobileTV_TaskHandler(void *p_arg);
void                MMPF_VIDEO_DECODE_TaskHandler(void *p_arg);
void                MMPF_FDTC_TaskHandler(void *p_arg);
void                MMPF_AUDIORECORD_TaskHandler(void *p_arg);
void                MMPF_APP_TaskHandler(void *p_arg);
void                MMPF_PCAM_TaskHandler(void *p_arg);
void                MMPF_USB_EP2_TaskHandler(void *p_arg);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

#define MMPF_OS_Sleep_MS(_ms)   MMPF_OS_Sleep((MMP_USHORT)(OS_TICKS_PER_SEC * ((MMP_ULONG)_ms + 500L / OS_TICKS_PER_SEC) / 1000L));

#define ALIGN2(_a)      (((_a) + 1) >> 1 << 1)
#define FLOOR4(_a)      ((_a) >> 2 << 2)
#define ALIGN4(_a)      (((_a) + 3) >> 2 << 2)
#define ALIGN8(_a)      (((_a + 0x07) >> 3) << 3)
#define ALIGN16(_a)     (((_a) + 15) >> 4 << 4)
#define FLOOR32(_a)     ((_a) >> 5 << 5)
#define ALIGN32(_a)     (((_a) + 31) >> 5 << 5)
#define FLOOR512(_a)     ((_a) >> 8 << 8)
#define ALIGN512(_a)     (((_a) + 511) >> 10 << 10)

#include    "os_wrap.h"
#include    "bsp.h"
#include    "mmpf_hif.h"

#include    "mmpf_graphics.h"


#endif // _INCLUDES_FW_H_

