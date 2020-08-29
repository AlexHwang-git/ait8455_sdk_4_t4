///@ait_only
/** @file config_fw.h

All customer-dependent compiler options go here.
The purpose is to prepare many versions of the file to different use cases,
such as different customers and different platforms. When compiling, simply replace this file and then compile the whole project.
This file could be used both in firmware projects.

This file should not be included in includes_fw.h.
Because the file is used to config and recompile the whole project,
it's better to be used only in the file which needs the config.
And it's highly recommended to use a value in the \#define instead of \#define only. That is
@code #define DOWNLOAD_METHOD 0 @endcode
is better than
@code #define USE_SD_DOWNLOAD_FW @endcode \n
Since the .c files might not include this file and get an unexpected result with any compiler warning.

@brief Configuration compiler options
@author Truman Yang
@since 10-aug-06
@version
- 1.0 Original version
*/


#ifndef _CONFIG_FW_H_
#define _CONFIG_FW_H_

    #define LYNC_CERT_PATCH         (1)
    #define LYNC_CERT_TEST_EN       (0)
    #define SUPPORT_OSD_FUNC        (1)
    #define RES_OSD_H24_FONT        (1)
    #define STREAMING_START_PIPE    (0)

    #define DUAL_UART_DBG_EN    (1)

    #define TRY_RT_CABAC        (0)
    


    #define SUPPORT_2FW_MODE    (1)    

    #define INIT_SCALE_PATH_EN     (0)
    
    // The code to test 848x480 YUY2 fps performance.
    #define FIRE_AT_FRAME_END   (1)
    #define FIRE_AT_FRAME_START (2)
    
    #define YUY2_848_480_30FPS  (FIRE_AT_FRAME_START)

    #define YUY2_TX_RT_MODE     (1)


    #if YUY2_848_480_30FPS > 0
    #define EOF_IN_LAST_DATA_PKT     (1) 
    #else
    #define EOF_IN_LAST_DATA_PKT     (0)
    #endif

    /** @brief Define if version B code is compiled.
    */
    #define	P_V2		(1)
    #define	VSN_V2		(2)
    #define VSN_V3      (3)
	// This chip name should be defined in target later.
	#ifndef CHIP
        #define	CHIP	(VSN_V3)
	#endif

    #define EXT_PMCLK_CKL	12000  
    
    #if LYNC_CERT_TEST_EN
    #define USING_EXT_USB_DESC       (0) // 0 for AIT, 1 for using customer desc lib.	
    #else
    #define USING_EXT_USB_DESC       (0) // 0 for AIT	
    #endif

    #define H264_POC_DUMP_EN         (0)
    #define CPU_EXCEPTION_LOG        (1) // Log CPU exceptio to flash


    // Enable watch dog reset if Divide by zero
    #define DIV_0_RESET_EN      (1)
    // Enable watch dog reset
    #define SYS_WD_ENABLE       (1)
    #define WATCHDOG_RESET_EN   (SYS_WD_ENABLE)
/*
level 0 : XTAL on / PMCLK on / DPLL on
level 1 : Can set, but flow is not ready
level 2 : XTAL on / PMCLK off / DPLL off
level 3 : All off, most saving power mode.
*/
    #define SUSPEND_LEVEL       (3)
    #define CODE_OPT_VER        (1)
/** @name System
@{ */
    // Python V1 : 160KB sram
    
/** @} */ // end of System
    
    #define USB_POWER_EN            (0) // D+ up moved to bootcode
    #define AIT_BOOTCODE_FLASH_ADDR     (0x00000000)
    #define AIT_BC1_FLASH_SIZE          (0x1000)    
    #define AIT_BOOTCODE_FLASH_SIZE     (0x8000)
    #define AIT_FW_FLASH_ADDR           (0x8000)
    #define SIF_BUFFER_START            0x01300000L  //0x110000	//Temp Buf, size:0x1000
    #define FW_DOWNLOAD_BUFFER          0x01400000L  
    #define CUSTOMER_TABLE_BUF_STA		0x01500000L
    #define CUSTOMER_TABLE_VP_BUF_STA	0x01600000L
    #define CUSTOMER_TABLE_VP_DUMP		0x01610000L
    #define SERIAL_NUMBER_WRITE_BUF_STA		0x01620000L
    #define SERIAL_NUMBER_READ_BUF_STA		0x01630000L
    #define CALI_DATA_BUF_STA	0x01630000L
    
    //#define CUSTOMER_INIT_FLASH_ADDR   (0x1e000)
    #define CUSTOMER_INIT_FLASH_ADDR   (0x7D000)
	#define CUSTOMER_INIT_FLASH_SIZE   (0x1000)
	#define CUSTOMER_INIT_FLASH_ADDR_VP	(0x7E000)
	#define SERIAL_NUMBER_FLASH_ADDR	(0x7F000)


    //#if(CHIP==VSN_V2)
    //#define SIF_BUFFER_START            0x010F0000L  //0x110000	//Temp Buf, size:0x1000
    //#define FW_DOWNLOAD_BUFFER          0x01100000L     
    //#endif
    //
    // 0x0D -> Rx Sensitivity
    // 0x0F -> The size of eye
    // REG[0x08] : bit 7 : 0 : fast slew rate, 1: slow slew rate
    
    // LGT : form factor board : Rx Level : 0x08
    // 0xAA7 : for USBIF
    // 0x8A7 : LGT
    #define USB_SQUELCH_LEVEL      (0x8A7)//(0x8A7)  // [7]: 0: fast slew rate, 1: slow slew rate for EMI issue
    
       #define G0_CLK      (192000)
        #define CPU_166     (0)
        #define CPU_216     (1)
        #define CPU_300     (2)
        #define CPU_192     (3)
        
    #define CPU_MHZ         (CPU_300)
// CPU_CFG_DEF begin
    #define G0_528_264MHZ   (0) // using CPU 528MHZ,G0 : 264MHZ
    #define G0_480_240MHZ   (1)
    #define G0_384_192MHZ   (2)
    #define G0_360_180MHZ   (3)
    #define G0_240_240MHZ   (4)

    #define G0_CPU_CLK      (G0_480_240MHZ) // for marina project
//  CPU_CFG_DEF end

    #define JTAG_DBG_EN         (0) // Enable Uart debug message
    #define UART_DBG_EN         (1)
    #if JTAG_DBG_EN==0
    #define HEARTBEAT_LED_EN    (0) // if GPIO-8(8423) connect a LED in AIT EV - board. (need to check ) 
    #define JTAG  (0)
    #else
    #define HEARTBEAT_LED_EN    (0)
    #define JTAG  (1)
    #endif


	#define	VER_A	(0)
	#define	VER_B	(1)
	#define	VER_C	(2)
	
	#define DEMOKIT  (1)
	#define PROJECT     (DEMOKIT)
	
	#define KYE       (1)
	#define BIS       (2)
	#define AZW       (3)
	#define SAL       (4)
	#define LGT       (5)
	#define DMX       (6)
	#define FCN       (7)
	#define BIS_5M    (8)
	#define SEM       (9)
	#define LON       (10)
	#define STK       (11)
    #define RAZ       (12)
	#define CUSTOMER    (RAZ)

    #define PYTHON_ECO    (0)
	
	#define I2S_OUT_EN 					(0) //wilson: for VSN_V3
    // definition for MIC IN path
    
	#define MIC_IN_PATH_AFE                	(1)
	#define MIC_IN_PATH_I2S                	(2)
	#define MIC_IN_PATH_BOTH				(3)
	#if (CHIP == VSN_V3)	
		#define MIC_SOURCE                   	(MIC_IN_PATH_AFE) //only vsn_v3 can set this define as MIC_IN_PATH_BOTH
	#else
		#define MIC_SOURCE						(MIC_IN_PATH_I2S)
	#endif
    
	//#define MicInPathAFE                (1)
	//#define MicInPathI2S                (2)
	//#define MicSource                   (0)//(MicInPathI2S) //V06 : defaut is I2S
	
	#if (MIC_SOURCE==MIC_IN_PATH_I2S)||(MIC_SOURCE==MIC_IN_PATH_BOTH)
	#define AUDIN_CHANNEL               2
	#define AUDIN_SAMPLERATE            16000 // V06 : change to 32000 for default,// 16000
	#else
	#define AUDIN_CHANNEL               2 // sean@2010_08_12 for stere test
	#define AUDIN_SAMPLERATE            16000
	#endif
	#define AS_INTERNAL_DELAY           2 // TBD
	#define NO_DAC                      0
	#define WM8737                      0x8737
	#define DA7211						0x7211
	#if (MIC_SOURCE==MIC_IN_PATH_I2S) || (MIC_SOURCE==MIC_IN_PATH_BOTH)
	#define AUDEXT_DAC                  WM8737
	#else
	#define AUDEXT_DAC                  NO_DAC
	#endif
	
	#define FW_VER_BCD                  (0x7913)  // for USB BCDdevice version        
	    
    #define GRA_ZOOM_BY_FOV             (1)
    #define GRA_ZOOM_BY_CROP            (2)
    #define SUPPORT_GRA_ZOOM            (GRA_ZOOM_BY_CROP)
    #define GRA_SRC_FIXED_SIZE          (0)
    #define GRA_SRC_FROM_PIPE           (2)
       #define GRA_SRC_PIPE             GRA_SRC_FROM_PIPE   
       #define GRA_DST_PIPE_1           (0) // ((GRA_SRC_PIPE==0)?1:0)   
       #define GRA_DST_PIPE_2           (1) //
    #define LOSSY_ZOOM_MIN_W            (640)
    #define LOSSY_ZOOM_MIN_H            (360)
    #define LOSSY_ZOOM_MAX_W            (1920)
    #define LOSSY_ZOOM_MAX_H            (1088)
    #define SUPPORT_CHANGE_AE_WINDOW    (0)
    #define SUPPORT_UAC                 (1)
    
    #define H264_LOW_BITRATE_TEST       (1)
    #define DEFAULT_VIDEO_FMT_H264      (0)  // 0: YUV/MJPEG, 1: H.264
    
    #define LCD_DISPLAY_ON              (0)
    
    #define LED_ON    1
    #define LED_OFF   0

    #define SUPPORT_HARDWARE_SNAP_BUTTON       (0)   // must be 1 for win7 suspend issue, tomy@2010_05_31

    #define SKYPE_FOV                   (0)    // for Skype video FOV specification
    
    #define CAPTURE_BAYER_RAW_ENABLE    (0)
    #define RAW_PROC_10_BIT_EN          (1)    // 0: 8 bits bayer raw, 1: 10 bits bayer raw
    // New definition for IQ table seperate in flash
    //#define IQ_TABLE_LOAD_FROM_FLASH    (0)
    #define IQ_TABLE_ADDR_IN_FLASH      (0x3C000)
    #define IQ_TABLE_MAX_SIZE           (0x4000)

    #define LED_READY      255 //8  // 8423 
    #define AF_EN          255
    #define SNAP_PIN       255
    #define TEST_PIN1      8  // if no used, change to 255  
    #define TEST_PIN2      255  
    #define TEST_PIN3      255  

	#define CAM_PWR_EN      45 //(SEN_PWR_EN CGPIO5)

	
	#define	ZOOM_RATIO_BASE	((MMP_USHORT)(-1))  
	
	#if (LED_READY!=255)
	#define LED_READY_PAD_CFG (PAD_E8_CURRENT | PAD_PULL_LOW)
	#endif
	#if (AF_EN!=255)
	#define AF_EN_PAD_CFG (PAD_E8_CURRENT | PAD_PULL_LOW)
	#endif
	#if (SNAP_PIN!=255)
	#define SNAP_PIN_PAD_CFG (PAD_E8_CURRENT | PAD_PULL_LOW)
	#endif
	#if (TEST_PIN1!=255)
	#define TEST_PIN1_PAD_CFG (PAD_E8_CURRENT | PAD_PULL_LOW)
	#endif
	#if (TEST_PIN2!=255)
	#define TEST_PIN1_PAD_CFG (PAD_E8_CURRENT | PAD_PULL_LOW)
	#endif
	#if (TEST_PIN3!=255)
	#define TEST_PIN1_PAD_CFG (PAD_E8_CURRENT | PAD_PULL_LOW)
	#endif
	
    /** @brief UART configuration

    Define which UART to use, and UART padset for output
    */
    #define DEBUG_UART_NUM (MMPF_UART_ID_0)
    // UART pin 0 : near EVB board reset button.
    // 0 : EV-board
    // 2 : REV001 board ( Scopio )
	#define DEBUG_UART_PIN (0)		// for JTAG mode using Pad Set3(PGPIO29/PGPIO30)
    /// Debug Level: see @ref debug_level for more detail.
    #define DBG_LEVEL (3)

/** @name System Clock
@{ */
	#if 1//	(CHIP == P_V2)
	#define EXT_CLK 12000       //for 12 MHz 
	#define USB_STC_CLK (30*1000*1000) // unit : Hz
	#define USB_STC_CLK_KHZ (30*1000) // unit : KHz
	#endif


/** @} */ // end of System Clock


/** @name MMU Table address
@{ */

    #if (CHIP == P_V2)
    #define SRAM_SIZE   (640*1024)
    #endif
    #if (CHIP == VSN_V2)
    #define SRAM_SIZE   (192*1024)
    #endif
    #if (CHIP == VSN_V3)
    #define SRAM_SIZE   (536*1024)
    #endif
    
    //#define MMU_TRANSLATION_TABLE_ADDR         (0x100000 + (SRAM_SIZE - MMU_OFFSET)        )
    //#define MMU_COARSEPAGE_TABLE_ADDR          (0x100000 + (SRAM_SIZE - MMU_OFFSET) + 0x400)   
    //#define SRAM_AREA_FOR_CRITICAL_REGION       (MMU_TRANSLATION_TABLE_ADDR - 0x4000)
/** @} */ // end of MMU Table address


/** @name USB Clock
@{ */
#if 1//(CHIP == P_V2)
	#define	USB_CLKINPUT_CRYSTAL	(0)
	#define	USB_CLKINPUT_PLL		(1)
	#define	USB_CLKFREQ_12MHZ		(0)
	#define	USB_CLKFREQ_30MHZ		(1)
	
	#define	USB_CLK_INPUT 	USB_CLKINPUT_CRYSTAL
	#define	USB_CLK_FREQ  	USB_CLKFREQ_12MHZ
	
    #define USB_PHY_TEST_MODE_EN (1)
	
#endif
/** @} */ // end of USB Clock

/** @name Sensor
@{ */

    #define TOTAL_SENSOR_NUMBER      1
//    #define SENSOR_ID_MT9D111        0
//  #define SENSOR_ID_OV7670           0 //Sample
//	#define SENSOR_ID_S5K4BAFB        0
//    #define SENSOR_ID_S5K5BAFX        0
///    #define SENSOR_ID_C2FKA244A      0
    #define SENSOR_ID_IMX045ES       0
    //#define SENSOR_ID_IMX046TS       0
//    #define SENSOR_ID_TCM9001MD      1

    #define BIND_SENSOR_MT9M011      0
    #define BIND_SENSOR_MT9M112      0
    #define BIND_SENSOR_MT9T013      0
    #define BIND_SENSOR_MT9T012      0
    #define BIND_SENSOR_MT9T111R3    0
    #define BIND_SENSOR_MT9D011      0
    #define BIND_SENSOR_MT9D111      0
    #define BIND_SENSOR_MT9P001      0
    #define BIND_SENSOR_MT9P012      0

	#define BIND_SENSOR_OV2650       0
	#define BIND_SENSOR_OV3642       0
    #define BIND_SENSOR_OV7660       0
    #define BIND_SENSOR_OV7670       0
    #define BIND_SENSOR_OV9650       0
    #define BIND_SENSOR_OV9653       0
    #define BIND_SENSOR_OV9655       0
    #define BIND_SENSOR_OV9660       0
    #define BIND_SENSOR_HV7131RP     0
	#define BIND_SENSOR_S5K3C1FX	 0
	#define BIND_SENSOR_S5K4BAFB	 0
	#define BIND_SENSOR_S5K5BAFX	 0
	#define BIND_SENSOR_C2FKA244A    0
    //MIPI
	#define BIND_SENSOR_IMX045ES     0
	#define	BIND_SENSOR_IMX046TS	 0	
	#define BIND_SENSOR_TCM9001MD    0
	#define	BIND_SENSOR_OV5650		 0
	#define	BIND_SENSOR_OV9726		 0	
	#define	BIND_SENSOR_S5K6A1GX	 0	
	#define	BIND_SENSOR_S5K4B2FX	 0	
	#define	BIND_SENSOR_MT9P111	     0	
	#define	BIND_SENSOR_OV2710		 0  
	#define	BIND_SENSOR_OV9710		 0	
    #define BIND_SENSOR_OV4688       1
    #define BIND_SENSOR_OV4689       0
    #define	BIND_SENSOR_MT9T002	     0
	
	#define BIND_SENSOR_VENUS 	 	 0

    #define SENSOR_IF_PARALLEL       (0)
    #define SENSOR_IF_MIPI_1_LANE    (1)
    #define SENSOR_IF_MIPI_2_LANE    (2)
    #define SENSOR_IF_MIPI_4_LANE    (3)

    #if (BIND_SENSOR_MT9T002) || (BIND_SENSOR_OV4688) || (BIND_SENSOR_OV4689)
    #define SENSOR_IF                (SENSOR_IF_MIPI_2_LANE)
    #endif

    #if BIND_SENSOR_OV2710==1
    #define SENSOR_IF                (SENSOR_IF_PARALLEL)
    #endif
    

    //#define MIPI_LANE_NUM               (2)  // test only for MT9T002 3M sensor
    
    #define CROP_1080P_FROM_2304_1296   (0)  // test only for MT9T002 3M sensor
    #define ENABLE_ROW_BINNING_2304_648   (0)  // test only for MT9T002 3M sensor
	

	#if	defined(SOFTWARE_I2CM)
	#if (BIND_SENSOR_S5K4BAFB)
	#define I2C_DELAY           70
	#else
	#define I2C_DELAY           30
	#endif	
	#endif


    #if (BIND_SENSOR_OV4688) || (BIND_SENSOR_OV4689)
    #define SUPPORT_AUTO_FOCUS          (1)
    #else
    #define SUPPORT_AUTO_FOCUS          (1)
    #endif

    #define SUPPORT_DIGITAL_ZOOM        (1) //(1)
    #define SUPPORT_DIGITAL_PAN         (1) //(1)

    // LED Driver
    #define SUPPORT_LED_DRIVER          (1)
	#define BIND_LED_LM36922            (1)
	#define SUPPORT_PWM_LED             (1)
    //#define LED_HWEN		            (255)

    // Rotary Encoder
    #define SUPPORT_ROTARY_ENCODER      (1)
    #define SUPPORT_TIMER_CTL_ENCODER	(1)	
    #define LIGHT_POSITIVE              (40) // CGPIO0
    #define LIGHT_NEGATIVE              (41) // CGPIO1

    // Power_LED_Gen
    #define PWR_LED                     (19) // BGPIO11
    #define LED_ENABLE	(1)

    //FOV LAYOUT option
    #if (BIND_SENSOR_MT9T002==1)
    #define SENSOR_16_9_W       (2304)
    #define SENSOR_16_9_H       (1296)
    #define SENSOR_IN_H         (1296)//(1296)
    #define SENSOR_16_9_W_60FPS (1152)
    #define SENSOR_16_9_H_60FPS (648 )
    #define SENSOR_IN_H_60FPS   (648 )
    #endif

    //FOV LAYOUT option
    #if (BIND_SENSOR_OV4688) || (BIND_SENSOR_OV4689)
    #define SENSOR_16_9_W       (2560)//(2688)
    #define SENSOR_16_9_H       (1440)//(1520)
    #define SENSOR_IN_H         (1440)//(1520)
    #define SENSOR_16_9_W_60FPS (1280) //(1276) //(1344) // 2688/2 = 1344
    #define SENSOR_16_9_H_60FPS (720)  //(716)  //(760)  // 1520/2 = 760
    //#define SENSOR_IN_H_60FPS   (720)  //(760)
    #endif

    /** @brief Choose which initialization method to be used
    @note There should be new implementation for each customer.
    */
	/// @brief Sensor reset GPIO pin
	#define MIPI_LANE_CTL (0) // 0: Single lane 0, 1: Single lane 1, 2: Dual Lane.
	#define SENSOR_SINGLE_LANE_0 0
	#define SENSOR_SINGLE_LANE_1 0
	#define SENSOR_DUAL_LANE 1	

	#define VIF_CTL_VIF0	0
	#define VIF_CTL_VIF1	1
	#define VIF_CTL			VIF_CTL_VIF0	

    /** @} */ // end of Sensor

	#define GPIO_SENSOR_PSEN      6    // 6 for AIT8433(70 pin package)

/** @name Video Player
@{ */

    /// Use software decoder for debugging or not. The SW library have to be added.
    #define SW_DECODER (0x10000)
    #define HW_MP4V (263)                  ///< Use hardware mpeg4 decoder
    #define SW_MP4V (HW_MP4V | SW_DECODER) ///< Use software mpeg4 decoder
    #define HW_H264 (264)                  ///< Use hardware h.264 decoder
    #define SW_H264 (HW_H264 | SW_DECODER) ///< Use software h.264 decoder

	/** @brief Support Rotate with DMA feature or not 0 for support. Set 1 to support.

	Set 0 to save code size. Some customer uses 823 and would never use this feature.
	@note If there is a chip ID config, replace with it.
	This could be removed later because we have CHIP_NAME now
	*/

/** @name Video Recorder
@{ */
	/** @brief Support landscape mode recording by rotate DMA.
	
	Instead of using LCD controller, frames for preview (in pipe 0) is rotated by rotate DMA.
	Presently it is only suitable for recording with large resolution (width>352),
	thus there will be distinct pipes for preview and encode.
	*/
	#define MGR_PROC_EN         (1)
	#define H264_SW_CODING_EN   (1)
	#define MTS_MUXER_EN        (0)
    #define VIDEO_FPS_SCALE     (100)
    #define LANDSCAPE_SUPPORT   (1)
    #define VID_TIME_SYNC_ST    (1) // Turn on this, sync h264 timestamp to v-snc PTS
    #define SLOT_RING           (1)
    #define FORCE_BASELINE_SUB_BLK_EN   (0) // may have level violation in small resolution(<720x576)
    #define H264E_RINGBUF       1
    #define SIMCAST_DROP        1
    #define SIMCAST_DROP_ID     255
    #define MIN_H264E_COMP_RATIO        (8)
    #define INIT_I_FRAME_NUM    (1)     // Number of consecutive i frames at start,
                                        // for UVC driver may loss 1st frame issue
    #define KITE_ONLY_VIDEO_PATCH       (0)
    #define INSERT_EOISOI      (0)    
    
    #define FLAG_EOF_IN_LAST_SLICE      1
    #if (FLAG_EOF_IN_LAST_SLICE == 1)
    // For H264 usage
        #define H264_UVC_PH_FLAG_IDR        (0x80000000)
        #define H264_UVC_PH_FLAG_EOF        (0x40000000)
        #define H264_UVC_PH_FLAG_SOF        (0x20000000)
        
        //#define H264_UVC_PH_LAYER_ID_MASK   (0x0000FFFF)
        #define H264_UVC_PH_LAYER_ID_MASK    (0x000000FF)
        #define H264_UVC_PH_STREAM_ID_MASK   (0x0000FF00)
        
        #define H264_UVC_PH_LAYER_ID(_a)    (_a & H264_UVC_PH_LAYER_ID_MASK )
        #define H264_UVC_PH_STREAM_ID(_a)    ((_a & H264_UVC_PH_STREAM_ID_MASK ) >> 8)
        
        
    // For Payload Header    
    // Bit[31:22] used as extension bit in UVC framerate payload item
        #define UVC_PH_EXT_BIT_MASK     (0xFFC00000)
        #define UVC_PH_LID_BIT_MASK     (0x0F000000)
        #define UVC_PH_SID_BIT_MASK     (0x00C00000)
        
        #define UVC_PH_EXT_BIT_SHIFT        (28)
        #define UVC_PH_LID_BIT_SHIFT        (24)
        #define UVC_PH_SID_BIT_SHIFT        (22)
        #define UVC_PH_FLAG_IDR             (0x80000000)
        #define UVC_PH_FLAG_EOF             (0x40000000)
        #define UVC_PH_FLAG_SOF             (0x20000000)
        
        #define UVC_PH_SID_BITS( sid )      ( ( sid & 0x3 ) << UVC_PH_SID_BIT_SHIFT )
        #define UVC_PH_LID_BITS( lid )      ( ( lid & 0xf ) << UVC_PH_LID_BIT_SHIFT )
        #define UVC_PH_SID( bits )          (( bits & UVC_PH_SID_BIT_MASK) >> UVC_PH_SID_BIT_SHIFT )
        #define UVC_PH_LID( bits )          (( bits & UVC_PH_LID_BIT_MASK) >> UVC_PH_LID_BIT_SHIFT )
    #endif
    #if (CHIP == VSN_V2)
    #define H264E_RINGBUF_SIZE  (400*1024)  // for H.264 720p
    #endif
    #if (CHIP == VSN_V3)
    #define H264E_RINGBUF_SIZE  (640*1024)  // for H.264 1080p
    #endif
    
    #define H264_CBR_PADDING_EN         (1)
    #define H264_CBR_PADDING_BS         (1024*3)
    
    #define H264_SIMULCAST_EN           (1)
    /** @} */ // end of Video Recorder



/** @name USB
@{ */
    #define SYS_SELF_SLEEP_ENABLE	(1) // 0: Disable System enter self-sleep (most power-saving) mode, 1: Enable

    #define SUPPORT_UVC_FUNC    (1) // 0: Disable UVC, 1: Enable UVC
#if (JTAG == 1)
    #define USB_SUSPEND_TEST    (0) // must be 0 for Linux  
#else
    #define USB_SUSPEND_TEST    (0) //(1) // must be 0 for Linux  
#endif
    #define USB_UVC_BULK_EP     (0) // must be 0 for Linux
    
    #if USB_UVC_BULK_EP==1
    #define UVC_DMA_SIZE   (0x2000)
    #endif
    
    #define USB_UVC_SKYPE       (0) // 0 : AIT H264 format, 1 : Skype format. (YUY2 + H264)
    #define USB_MJPEGH264_STREAM (0) // Force MJPEG include H264.
    // For AIT stream viewer
    // signal type 
    // 0 : yuy2+h264
    // 1 : mjpeg + h264
    #if USING_EXT_USB_DESC
    #define USB_FRAMEBASE_H264_DUAL_STREAM  (0) 
    #else
    #define USB_FRAMEBASE_H264_DUAL_STREAM  (1) 
    #endif
    #define USB_UVC_H264         (1) // for Logitech H264
    
    #define USB_UVC15            (1)

    #if USB_UVC15==1
    #define ENCODING_UNIT   (1)
    #else
    #define ENCODING_UNIT   (0)    
    #endif
    #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
    #define USB_LYNC_TEST_MODE     (1)
    #else
    #define USB_LYNC_TEST_MODE     (1)
    #endif
    #define USB_LYNC_CIF_RATIO      (93)
    #define USB_SPEEDUP_PREVIEW_TIME (1)
    
    #define USB_UAC_TO_QUEUE        (1)

      
    #if USB_MJPEGH264_STREAM==1 // AitH264CamTool for MJPEG + H264
    #define APP_MARKER 0xFFE3
    #else
    #define APP_MARKER 0xFFE4
    #endif
    
    #if (USB_MJPEGH264_STREAM==1) || (USB_FRAMEBASE_H264_DUAL_STREAM==1)
    #define ALIGN_PAYLOAD   0
    #else
    #define ALIGN_PAYLOAD   1
    #endif
    
#if SUPPORT_UAC

    #define USB_UAC_IF_CTL_HEADER_DESC_SIZE                  (0x09)
    #define USB_UAC_IF_CTL_IT_DESC_SIZE                      (0x0C)
    #define USB_UAC_IF_CTL_OT_DESC_SIZE                      (0x09)
    #define USB_UAC_IF_CTL_FU_DESC_SIZE                      (0x08)


    #define USB_UAC_IF_CTL_TOTAL_SIZE                        ( USB_UAC_IF_CTL_HEADER_DESC_SIZE+     \
                                                               USB_UAC_IF_CTL_OT_DESC_SIZE+         \
                                                               USB_UAC_IF_CTL_IT_DESC_SIZE+         \
                                                               USB_UAC_IF_CTL_FU_DESC_SIZE         ) 

    // IF3AL0  or IF4AL0 
    #define USB_IF_UAC_STREAM_DESC_SIZE                      (0x09)
    // IF3AL1  or IF4AL1 
    //#define USB_IF_UAC_STREAM_DESC_SIZE                    (0x09) // re-define

    #define USB_UAC_IF_STREAM_GENERAL_DESC_SIZE              (0x07)

    #define NUM_OF_UAC_SAMPLING_RATE                         (0x01)

    #define USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE           (0x08+0x03*NUM_OF_UAC_SAMPLING_RATE)

    #define USB_UAC_IF_STREAM_TOTAL_SIZE                     ( USB_UAC_IF_STREAM_GENERAL_DESC_SIZE+    \
                                                               USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE )
    
    #define USB_UAC_EP_DESC_SIZE                             (0x09)
 
    #define USB_UAC_EPG_DESC_SIZE                            (0x07)

#endif  // end of #if SUPPORT_UAC     

#define SUPPORT_PCSYNC LYNC_CERT_TEST_EN

/** @name DSC
@{ */
	#define USE_TASK_DO3A       (1)
	
	//#define OMRON_FDTC_F45_BUF_SIZE (1024*1024)  // F45 working buffer
	
	#define OMRON_FDTC_SUPPORT	(0) // 1: Enable face detection, 0:Disable face detection
/** @} */ // end of DSC    
     
    #if (CHIP == VSN_V3)&& (USB_UVC_BULK_EP==1)
    #error "Stop to support Bulk mode in 8455"
    #endif
    
    
    #define SINGLE_STREAMING_TASK   (0)
	/** @} */ /* end of USB */


/** @name AUDIO
@{ */
	#if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
	
	#define DC_OFFSET_OFF                     (0)
	#define FIXED_PGA_GAIN                    (1)
	#define FIXED_BOOST_GAIN                  (2)
	#define ADC_FINE_TUNE_POP_NOISE         (FIXED_BOOST_GAIN)
	
	#define ADC_FIX_PGA_GAIN                (5) // db
	
	#if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN
	#define AFE_FIX_BOOST_GAIN              (30)// db
	#else
	#define AFE_FIX_BOOST_GAIN              (40)// db
	#endif
	
	#define ADC_PERFORMANCE_TEST            (0)
	#define	DEFAULT_DAC_DIGITAL_GAIN 		0x3F //per channel
	#define	DEFAULT_DAC_ANALOG_GAIN  		0xAA
	#define DEFAULT_DAC_LINEOUT_GAIN        0xCC
	#define	DEFAULT_ADC_DIGITAL_GAIN 		0x3F //per channel
	
	#if ADC_FINE_TUNE_POP_NOISE==FIXED_PGA_GAIN
	#define DEFAULT_ADC_ANALOG_GAIN         0x10 // +5 db
	#else
	#define	DEFAULT_ADC_ANALOG_GAIN  		0x17 //+12 db
	#endif
	
	#endif

    #define VREF_CAPACITOR_UF   (1)        // ext capacitor for VREF
    #define ANA_PDN_DLY_MS      (1)
    #define ADC_PGA_PDN_DLY_MS  (10*VREF_CAPACITOR_UF)


	#define SRC_SUPPORT	        (0) // 0: Disable SRC encode, 1: Enable SRC encode
	#define WAV_ENC_SUPPORT     (0)
	#define GAPLESS_EN          (0)
	#define AUDIO_STREAMING_EN	(0)
	#define AUDIO_MIXER_EN		(0) // 0: Disable software mixer 1: Enable software mixer
	#define PCM_TO_DNSE_EN		(0) // 0: Disable software mixer 1: Enable software mixer
	#define PCM_ENC_SUPPORT     (0)
    #define GAPLESS_EN          (0) // 0: Disable gapless playback, 1: Enable gapless playback
    /** @} */ // end of AUDIO
/** @name H264
@{ */
/** @} */ // end of H264

    #define H264_SPS_PPS_INSERT_BEFORE_IDR_FRAME     (1)
    // H264 setting using the values after commit
    //
    #define USING_UVCX_VIDEO_CONFIG (1)
    
    //#define LAYER_ID_BIT_SHIFT_TEMPORAL     (16)
    
    #define H264_MTS_SUPPORT    (0)
    
    #define COARSE_LEVEL        (0)
    #define FINE_LEVEL          (1)
    #define BEST_LEVEL          (2)
    
    #define SCALER_ZOOM_LEVEL   BEST_LEVEL //: under developing
    #define CAL_MUL        (1)
    #define CAL_BASE       (1296 * CAL_MUL)
    
    
    #if 1//SCALER_ZOOM_LEVEL != BEST_LEVEL
    #define RATIO_M             (72)
    #endif
    
#if defined(ALL_FW)  // This part can be changed by difference project.
	#define FS_EN  0
	#define DSC_R_EN 0
	#define DSC_P_EN 0
	#define AAC_P_EN 0//1
	#define MP3_P_EN 0//1
	#define AAC_R_EN 0//1
	#define MP3_R_EN 0//1
	#define AMR_R_EN 0
	#define AMR_P_EN 0//1
	#define MIDI_EN 0//1
	#define WMA_EN 0
	#define AC3_P_EN 0//1
	#define VAMR_R_EN 0//1
	#define VAMR_P_EN 0//1
	#define VIDEO_R_EN 1
	#define VAAC_P_EN 0//1
	#define VAC3_P_EN 0//1	
	#define USB_EN 1
	#define OGG_EN 0//1
	#define WMAPRO10_EN 0//1
	#define RA_EN 0//1
	#define RV_EN 0
	#define VRA_P_EN 0
	#define WAV_P_EN 0//1
	#define VMP3_P_EN 0//1	
	#define MP3HD_P_EN 0
	#define WAV_R_EN 0
	#define VMP3_P_EN 0	
	#define VWAV_P_EN 0
	#define WMV_P_EN 0
	#define VWMA_P_EN 0
	#define FLAC_P_EN 0
	
	#define PCAM_EN ((USB_EN) && (SUPPORT_UVC_FUNC))
	
	#define AUDIO_EFFECT_EN     (0)
	
    #define SENSOR_EN  1

    #define DUMP_REG_EN (0)
    
#endif	


#endif
///@end_ait_only
///ISP setting=========================
#define ISP_BUF_IN_SRAM         (1)

#define IQ_OPR_DMA_ON			1

#if (CHIP == P_V2)
#define ISP_AWB_BUF_SIZE    (5*1024)
#define ISP_AE_BUF_SIZE     (1024)
#define ISP_AF_BUF_SIZE     (1024)
#define ISP_DFT_BUF_SIZE    (1024)
#define ISP_HIST_BUF_SIZE   (0)
#define ISP_FLICK_BUF_SIZE  (0)
#endif
#if (CHIP == VSN_V2)
#define ISP_AWB_BUF_SIZE    (5*1024)
#define ISP_AE_BUF_SIZE     (0)
#define ISP_AF_BUF_SIZE     (0)
#define ISP_DFT_BUF_SIZE    (0)
#define ISP_HIST_BUF_SIZE   (128*4)//(0)
#define ISP_FLICK_BUF_SIZE  (0)
#endif
#if (CHIP == VSN_V3)
#define ISP_AWB_BUF_SIZE    (5*1024)
#define ISP_AE_BUF_SIZE     (1024)
#define ISP_AF_BUF_SIZE     (1024)//(0)
#define ISP_DFT_BUF_SIZE    (0)
#define ISP_HIST_BUF_SIZE   (128*4)
#define ISP_FLICK_BUF_SIZE  (0)
#endif

#if (IQ_OPR_DMA_ON == 1)
    #define IQ_BANK_SIZE        (256*16)
    #define LS_BANK_SIZE        (256*(3+1)*2*2)
    #define CS_BANK_SIZE        (42*32*3*2*2)
#else
    #define IQ_BANK_SIZE        (0)
    #define LS_BANK_SIZE        (0)
    #define CS_BANK_SIZE        (0)
#endif

#if IQ_OPR_DMA_ON

#if ISP_BUF_IN_SRAM==1
#define IQ_BANK_ADDR		(((m_glISPDmaBufferStartAddr) >> 8 << 8))
#define LS_BANK_ADDR		(((m_glISPDmaBufferStartAddr) >> 8 << 8)+IQ_BANK_SIZE)
#define CS_BANK_ADDR		(((m_glISPDmaBufferStartAddr) >> 8 << 8)+IQ_BANK_SIZE+LS_BANK_SIZE)
#else
#define IQ_BANK_ADDR		(((m_glISPBufferStartAddr+ISP_AWB_BUF_SIZE+ISP_AE_BUF_SIZE+ISP_AF_BUF_SIZE+ISP_DFT_BUF_SIZE+ISP_HIST_BUF_SIZE+ISP_FLICK_BUF_SIZE+256) >> 8 << 8))
#define LS_BANK_ADDR		(((m_glISPBufferStartAddr+ISP_AWB_BUF_SIZE+ISP_AE_BUF_SIZE+ISP_AF_BUF_SIZE+ISP_DFT_BUF_SIZE+ISP_HIST_BUF_SIZE+ISP_FLICK_BUF_SIZE+256) >> 8 << 8)+IQ_BANK_SIZE)
#define CS_BANK_ADDR		(((m_glISPBufferStartAddr+ISP_AWB_BUF_SIZE+ISP_AE_BUF_SIZE+ISP_AF_BUF_SIZE+ISP_DFT_BUF_SIZE+ISP_HIST_BUF_SIZE+ISP_FLICK_BUF_SIZE+256) >> 8 << 8)+IQ_BANK_SIZE+LS_BANK_SIZE)
#endif


#else
#define IQ_BANK_ADDR		(0x80000000)
#define LS_BANK_ADDR		(0x80000000)
#define CS_BANK_ADDR		(0x80000000)
#endif

#define IQ_OPR_DMA_SIZE     (IQ_BANK_SIZE+LS_BANK_SIZE+CS_BANK_SIZE+256) //256 alignment
#if ISP_BUF_IN_SRAM==0
#define ISP_BUFFER_SIZE     (ISP_AE_BUF_SIZE+ISP_AF_BUF_SIZE+ISP_AWB_BUF_SIZE+ISP_DFT_BUF_SIZE+ISP_HIST_BUF_SIZE+ISP_FLICK_BUF_SIZE+IQ_OPR_DMA_SIZE)
#else
#define ISP_BUFFER_SIZE     (ISP_AE_BUF_SIZE+ISP_AF_BUF_SIZE+ISP_AWB_BUF_SIZE+ISP_DFT_BUF_SIZE+ISP_HIST_BUF_SIZE+ISP_FLICK_BUF_SIZE)
#endif
#define ISP_GNR_OFF     (0)

