///@ait_only
/** @file ait_config.h

All configuration compiler options go here.
The purpose is to prepare many versions of the file to different use cases,
such as different customers and different platforms. When compiling, simply replace this file and then compile the whole project.
This file could be used both in host and firmware projects.

@brief Configuration compiler options
@author Truman Yang
@since 10-Jul-06
@version
- 2.0 Changed name ait_config.h from config.h to reduce integration naming issue
- 1.0 Original version
*/
#ifndef AIT_CONFIG_H
#define AIT_CONFIG_H

#include "mmp_lib.h"
#include "ait_bsp.h"
#include "mmps_system.h"

	#define DEMOKIT  (1)
	#define PROJECT     (DEMOKIT)
	
/** @name Sensor & panel setting
@{ */
    /// Using this flag to switch I2C setting between sensor and external audio DAC.
    /// When firmware initial sensor with software I2C, set as 0. Otherwise, set as 1.
    /// Default: 1.
    #define SENSOR_USE_HW_I2C  1

    #define SENSOR_COUNT_MAX_NUM        (2)         ///< Maximum sensor support count
    #define SENSOR_MODE_MAX_NUM         (3)         ///< Maximum sensor mode count
    #define SENSOR_FRAMERATE_MAX_NUM    (6)         ///< Maximum sensor frame rate count
	#define SENSOR_EV_MAX_NUM           (19)        /// maximum exposure value level

    /// Select sensor model for multi-sensor configuration.
    #define MAIN_SENSOR                 0
    #define SUB_SENSOR                  1

/** @} */ // end of sensor and panel

/** @name DSC
@{ */
    #define DSC_MODE_MAX_NUM            (4)         ///< maximum preview mode	
    #define DSC_DECODE_MODE_MAX_NUM     (5)         ///< maximum decode mode
    #if (PROJECT == DEMOKIT)
    #define DSC_RESOL_MAX_NUM           (7)         ///< maximum capture resolution number
    #endif
	#define DSC_SHOT_MODE_MAX_NUM       (2)         ///< maximum shot mode
    #define DSC_MULTISHOT_MAX_NUM       (9)         ///< maximum multishot number
	#define DSC_QTABLE_MAX_NUM          (3)         ///< maximum quality control mode
    #define DSC_PANORAMA_MAX_SHOT       (3)

    #define DSC_NORMAL_PREVIEW_MODE     0
    #define DSC_FULL_PREVIEW_MODE       1
    #define DSC_TV_PREVIEW_MODE         2

    #define DSC_NORMAL_DECODE_MODE      0
    #define DSC_FULL_DECODE_MODE        1
    #define DSC_TV_NTSC_DECODE_MODE     2
    #define DSC_TV_PAL_DECODE_MODE      3
    #define DSC_PANORAMA_DECODE_MODE    4
/** @} */ // end of DSC

/** @name Video Recorder
@{ */
    /**@brief Define moving segment of AV and FT data.*/
    #define VIDRECD_MOV_AV_MASK     (~(MMP_ULONG)0x0FFF)
    #define VIDRECD_MOV_FT_MASK     (~(MMP_ULONG)0x01FF)
    #define VIDRECD_INT_AV_MASK     ((MMP_ULONG)(8*1024))

    #define VIDRECD_MODE_MAX_NUM       (7)          ///< Video system mode

    #define VIDRECD_QLEVEL_MAX_NUM     (3)          ///< Video quality from LOW to HIGH
    #define VIDRECD_QUALITY_HIGH        0           ///< Video bit rate quality, high level
    #define VIDRECD_QUALITY_MID         1           ///< Video bit rate quality, middle level
    #define VIDRECD_QUALITY_LOW         2           ///< Video bit rate quality, low level

    #define VIDRECD_FPS_MAX_NUM        (6)          ///< Video FPS from 7.5 to 30
    #define VIDRECD_FRAMERATE_30FPS     0           ///< Video frame rate, 30 fps
    #define VIDRECD_FRAMERATE_25FPS     1           ///< Video frame rate, 24 fps
    #define VIDRECD_FRAMERATE_20FPS     2           ///< Video frame rate, 20 fps
    #define VIDRECD_FRAMERATE_15FPS     3           ///< Video frame rate, 15 fps
    #define VIDRECD_FRAMERATE_10FPS     4           ///< Video frame rate, 12 fps
    #define VIDRECD_FRAMERATE_5FPS      5           ///< Video frame rate, 7.5 fps

/** @} */ // end of video Recorder


/** @name Video Player
@{ */



    /// Enable Speed up 3GP parser mode
	#define PSR3GP_SPEED_UP_EN  (1)

/** @} */ // end of video player

/** @name MMP_USB
@{ */
    /// Connect host storage device to AIT USB
    #define SUPPORT_HOST_STORAGE_DEVICE (0) // 0: Disable host storage, 1: Enable host storage
/** @} */ /* end of USB */

/** @name MMP_SYSTEM
@{ */
    /// Enable AutoClock Gating
    #ifndef BUILD_CE
    #define AUTO_CLK_GATING_EN	(1)
    #else
    #define AUTO_CLK_GATING_EN	(0)
    #endif
/** @} */ /* end of SYSTEM */

    
/** @} */ /* end of AUDIO */



    MMP_BOOL MMPC_System_InitConfig(void);
    MMP_BOOL MMPC_Sensor_InitConfig(void);
    MMP_BOOL MMPC_AUDIO_InitConfig(void);
    void MMPC_System_DumpSramConfig(void );
    MMP_BOOL MMPC_GPIO_InitConfig(void);

#endif /* AIT_CONFIG_H */

///@end_ait_only
