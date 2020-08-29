//==============================================================================
//
//  File        : mmpf_task_cfg.h
//  Description : Task configuration file for A-I-T MMPF source code
//  Author      : Philip Lin
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_TASK_CFG_H_
#define _MMPF_TASK_CFG_H_
#include "config_fw.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#define     DEBUG

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define TASK_SYS_STK_SIZE       (1024)//(3072)
#define TASK_DSC_STK_SIZE       (512)
#define LTASK_STK_SIZE              (128)
#define TASK_SENSOR_STK_SIZE    (384)

#define TASK_FDTC_STK_SIZE      (768)
#define TASK_STK_SIZE           (128)       // 128 x4 Bytes
#define PLAYER_STK_SIZE       	(1024) // Stack size in DW (106)
#define DECODER_STK_SIZE      	(4096) // Stack size in DW (106)
#define DEMUXER_STK_SIZE      	(4096) // Stack size in DW (298)
#define MP4VENC_STK_SIZE        (384)       // 384 x4 Bytes
#define VIDMGR_STK_SIZE         (1024)      // 512 x4 Bytes
#if (MP3HD_P_EN == 1)
#define AUDIO_PLAY_STK_SIZE     (5120)
#else
#define AUDIO_PLAY_STK_SIZE     (4096)
#endif
#define AUDIO_RECORD_STK_SIZE       (4096)
#define USB_EP1_STK_SIZE            (2400/*2300*/) // (3072)  // 3072 x4 Bytes , sean@2010_01_11 change to 2048 
#define USB_EP2_STK_SIZE            (1024)
#define FS_STK_SIZE                 (1024)  // alterman 1024 x4 Bytes ||// 512 x4 Bytes
#define TASK_PCAM_STK_SIZE          (2048)

//#define TASK_SYS_PRIO           (1)
#define TASK_PRIO_OFFSET    (2)
enum {
    TASK_1ST_PRIO           = 1,
    TASK_SYS_PRIO           = 1 * TASK_PRIO_OFFSET ,
    TASK_MP4VENC_PRIO       = 2 * TASK_PRIO_OFFSET,
    TASK_VIDMGR_PRIO        = 3 * TASK_PRIO_OFFSET,
    TASK_USB_EP2_PRIO       = 4 * TASK_PRIO_OFFSET,
    //Timer Task in OS use priority 5 !!!
    TASK_USB_EP1_PRIO       = 6 * TASK_PRIO_OFFSET,
    TASK_AUDIO_RECORD_PRIO  = 7 * TASK_PRIO_OFFSET,
    TASK_USB_PRIO1          = 8 * TASK_PRIO_OFFSET,
    TASK_AUDIO_PLAY_PRIO    = 9 * TASK_PRIO_OFFSET, 
    TASK_3GPPARSR_PRIO      = 10* TASK_PRIO_OFFSET, 
    TASK_VIDDEC_PRIO        = 11* TASK_PRIO_OFFSET, 
    TASK_VIDPLAY_PRIO       = 12* TASK_PRIO_OFFSET, 
    TASK_DSC_PRIO           = 13* TASK_PRIO_OFFSET, 
    TASK_SENSOR_PRIO        = 14* TASK_PRIO_OFFSET, 
    TASK_APP_PRIO           = 15* TASK_PRIO_OFFSET, 
    TASK_AUDIO_WRITEFILE_PRIO = 16* TASK_PRIO_OFFSET, 
    TASK_FS_PRIO            = 17* TASK_PRIO_OFFSET, 
    TASK_FDTC_PRIO          = 18* TASK_PRIO_OFFSET, 
    TASK_PCAM_PRIO          = 29* TASK_PRIO_OFFSET
} ;

typedef struct
{
	void*	context;
	int		event;
	long	lParam;
	void*	pParam;
} MediaEventMsg;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MMPF_TASK_CFG_H_ */
