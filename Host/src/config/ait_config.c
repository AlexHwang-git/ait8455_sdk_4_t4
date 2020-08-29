/**
  @file ait_config.c
  @brief It contains the configurations need to be ported to the customer platform.
  @author Rogers

  @version
- 1.0 Original version
 */

#include "config_fw.h"
#include "mmp_lib.h"
#include "ait_config.h"
#include "mmps_system.h"
#include "mmps_audio.h"
#include "mmps_display.h"
#include "mmps_3gprecd.h"
#include "mmps_sensor.h"
#if PCAM_EN==1
#include "mmps_usb.h"
#endif

#include "mmpf_pio.h"

#if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)

#define QTABLE_NUM 3


#if PCAM_EN==1
/// Usb Memory Allocation Information
static MMPS_USBMODE_BUFFER_INFO m_UsbBufInfo;

/**@brief The system configuration

Use @ref MMPS_System_GetConfiguration to assign the field value of it.
You should read this functions for more information.
*/
static MMPS_SYSTEM_CONFIG	m_systemConfig;
/**@brief The system operation mode.*/

static MMPS_AUDIOMODE_CONFIG_INFO m_AudioBufInfo ;
#endif
//==============================================================================
//                              FUNCTION PROTOTYPES
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPC_Sensor_InitConfig
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMPC_Sensor_InitConfig(void)
{
    //5M/1.3M/8M Sensor
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorpreviewmodenum = 2;

#if BIND_SENSOR_OV9726
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 720;//800;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 720;//800;
#endif
#if BIND_SENSOR_OV2710 
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 1920;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 1088;//800;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1920;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 1088;//800;
#endif
#if (BIND_SENSOR_MT9T002)
    // sean@2011_01_18, fixed bug
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0]  = 1152 ;//2304 ;// 1920;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 648 ;//1088;//800;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1]  = 2304 ;//1920;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 1296 ;//1088;//800;
#endif

#if (BIND_SENSOR_OV4688) || (BIND_SENSOR_OV4689)
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0]  = 2688;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 1520;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1]  = 2688;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 1520;
#endif

#if BIND_SENSOR_OV9710
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 720;//800;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 720;//800;
#endif
#if BIND_SENSOR_S5K6A1GX
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 720;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 720;
#endif
#if BIND_SENSOR_S5K4B2FX
  #if SKYPE_FOV
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 1600;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 900;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1600;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 900;
  #else
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 1600;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 1200;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1600;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 1200;
  #endif
#endif
#if BIND_SENSOR_IMX045ES || BIND_SENSOR_OV5650 || BIND_SENSOR_MT9P111 || BIND_SENSOR_MT9P014
  #if SKYPE_FOV
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 2560;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 1920;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 720;
  #else
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 2560;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 1920;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 960;
  #endif
#endif    
#if BIND_SENSOR_IMX046TS
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[0] = 3264;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[0] = 2448;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewwidth[1] = 1600;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorpreviewheight[1] = 1200;
#endif

    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorcapturemodenum = 2;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorcapturewidth[0] = 2560;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorcaptureheight[0] = 1920;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorcapturewidth[1] = 1280;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].usSensorcaptureheight[1] = 960;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensordefaultpreviewmode = 1;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensordefaultphasecount = 0;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[0] = 0;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[1] = 1;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[2] = 2;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[3] = 3;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[4] = 4;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[5] = 5;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[6] = 6;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[7] = 7;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[8] = 8;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[9] = 9;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[10] = 10;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[11] = 11;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[12] = 12;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[13] = 13;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[14] = 14;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[15] = 15;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[16] = 16;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[17] = 17;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubSensorevdata[18] = 18;

	MMPS_Sensor_GetConfiguration()->sensorconfig[0].bRawStoreDownsample[0] = MMP_TRUE;
	MMPS_Sensor_GetConfiguration()->sensorconfig[0].bRawStoreDownsample[1] = MMP_FALSE;
	
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubFrameRate[0] = 0;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubFrameRate[1] = 1;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubFrameRate[2] = 2;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubFrameRate[3] = 3;
    MMPS_Sensor_GetConfiguration()->sensorconfig[0].ubFrameRate[4] = 4;

    //for Face Detection
    MMPS_Sensor_GetConfiguration()->faceDetectInputPath = MMPS_SENSOR_FDTC_2PIPE; //one pipe for preview, one pipe for FDTC
    MMPS_Sensor_GetConfiguration()->faceDetectDisplayMethod = MMPS_SENSOR_FDTC_FW_DRAW_FACE;
    MMPS_Sensor_GetConfiguration()->bFaceDetectEnable = MMP_TRUE;
    MMPS_Sensor_GetConfiguration()->usFaceDetectGrabScaleM = 80;
    MMPS_Sensor_GetConfiguration()->ubDetectFaceNum = 15; //Max: 15
    MMPS_Sensor_GetConfiguration()->ubFaceDetectInputBufCnt = 2;
    MMPS_Sensor_GetConfiguration()->ulFaceDetectWorkBufSize = 0x80000; //for 160x120, need cacheable
    MMPS_Sensor_GetConfiguration()->ulFaceDetectFeatureBufSize = 0x5000; //for 160x120, need cacheable
    MMPS_Sensor_GetConfiguration()->usFaceDetectInfoBufSize = 0x200; //512B
    return MMP_TRUE;
} /* MMPC_Sensor_InitConfig */



/** @brief Initialized the preset configurations for 3GP playing

@pre This function must be called after MMPC_Display_InitConfig because of the TV attribute dependence
@pre The DRAM is initlized so that the DRAM type and size are assigned.
@return Always returns true;
*/


#endif

//------------------------------------------------------------------------------
//  Function    : MMPC_Display_InitConfig
//  Description :
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//  Function    : MMPC_AUDIO_InitConfig
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMPC_AUDIO_InitConfig(void)
{
   // MMP_UBYTE i,j;

    MMPS_AUDIO_GetConfiguration()->AudioOutPath = MMPS_AUDIO_OUT_AFE_HP | MMPS_AUDIO_OUT_AFE_LINE;
	#if (PROJECT == DEMOKIT)
	MMPS_AUDIO_GetConfiguration()->AudioInPath = MMPS_AUDIO_IN_AFE_DIFF;
	#endif

   
    #if AUDIO_P_EN==1
    //MMPS_AUDIO_InitialInPath(MMPS_AUDIO_GetConfiguration()->AudioInPath);
    //MMPS_AUDIO_InitialOutPath(MMPS_AUDIO_GetConfiguration()->AudioOutPath);

    MMPS_AUDIO_GetConfiguration()->ubVolumeLevel = 31;
    for(i = 0; i < MMPS_AUDIO_GetConfiguration()->ubVolumeLevel; i++)
        for(j=0; j < 3; j++) {
            MMPS_AUDIO_GetConfiguration()->ubAudioOutVolHPTable[i][j] = ubAudOutVol_HP_table[i][j];
        }

    for(i = 0; i < MMPS_AUDIO_GetConfiguration()->ubVolumeLevel; i++)
        for(j=0; j < 2; j++) {
            MMPS_AUDIO_GetConfiguration()->ubAudioOutVolLineTable[i][j] = ubAudOutVol_Line_table[i][j];
        }


    MMPS_AUDIO_GetConfiguration()->ulNameBufSize = 0x200;
    MMPS_AUDIO_GetConfiguration()->ulParameterBufSize = 0x400;
    MMPS_AUDIO_GetConfiguration()->ulSdBufSize = 0x400;
    MMPS_AUDIO_GetConfiguration()->ulSmBufSize = 0x400;
    #endif
    
    
    #if AUDIO_P_EN==1

    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_PCM_P_MEM_MODE].ulBufStartAddr = 0x101C00;//0x110000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_PCM_P_MEM_MODE].ulPlayBufSize = 0x8000;


    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_WAV_P_MEM_MODE].ulBufStartAddr = 0x2A000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_WAV_P_MEM_MODE].ulPlayBufSize = 0x4000;

    #endif
    #if AUDIO_R_EN==1
    
    #if AMR_R_EN==1
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AMR_R_CARD_MODE].ulBufStartAddr = 0x101C00;//0x110000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AMR_R_CARD_MODE].ulRecBufSize = 0x4000;

    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AMR_R_MEM_MODE].ulBufStartAddr = 0x101C00;//0x110000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AMR_R_MEM_MODE].ulRecBufSize = 0x4000;
    #endif
    #if AAC_R_EN==1
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AAC_R_CARD_MODE].ulBufStartAddr = 0x101C00;//0x110000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AAC_R_CARD_MODE].ulRecBufSize = 0x4000;

    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AAC_R_MEM_MODE].ulBufStartAddr = 0x101C00;//0x110000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_AAC_R_MEM_MODE].ulRecBufSize = 0x4000;
    #endif
    #if MP3_R_EN==1
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_MP3_R_CARD_MODE].ulBufStartAddr = 0x101C00;//0x110000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_MP3_R_CARD_MODE].ulRecBufSize = 0x4000;

    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_MP3_R_MEM_MODE].ulBufStartAddr = 0x101C00;//0x110000;
    MMPS_AUDIO_GetConfiguration()->AudioMode[MMPS_AUDIO_MP3_R_MEM_MODE].ulRecBufSize = 0x4000;
    
    #endif
    
    #endif
    return MMP_TRUE;
} /* MMPC_AUDIO_InitConfig */

//------------------------------------------------------------------------------
//  Function    : MMPC_System_InitConfig
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMPC_System_InitConfig(void)
{
extern MMP_ULONG Image$$SRAM_MMU$$Base;
extern MMP_ULONG Image$$SRAM_JPG$$Base;
//extern MMP_ULONG Image$$SRAM_CODE$$Base;
extern MMP_ULONG Image$$SRAM_H264$$Base;
extern MMP_ULONG Image$$SRAM_END$$Base;
#if ISP_BUF_IN_SRAM==1
extern MMP_ULONG Image$$SRAM_IQ_DMA$$Base;
#endif
extern MMP_ULONG MMU_TRANSLATION_TABLE_ADDR,MMU_COARSEPAGE_TABLE_ADDR;

    MMPS_SYSTEM_CONFIG *pConfig = MMPS_System_GetConfiguration();
    MMPS_SYSTEM_SRAM_MAP *psrammap;
    
    pConfig->stackMemoryType = MMPS_DRAM_TYPE_AUTO;
    pConfig->ulStackMemoryStart = 0x1000000;
    pConfig->hostBusModeSelect = MMPS_HIF_16_BIT_MODE;
    pConfig->firmwareType = MMPS_FWTYPE_ALL;
    pConfig->ulMaxReserveBufferSize = 96*1024;
    pConfig->ulMaxReserveChannel = 2;
    pConfig->ulCpuFreq = 144;
    pConfig->ulG012Freq = 144;
    pConfig->ulG6Freq = 144;
    pConfig->stackMemoryMode = MMPS_DRAM_MODE_DDR;
    #if (CHIP == VSN_V2)
    pConfig->ulStackMemorySize = 8*1024*1024;
    #endif 
    #if (CHIP == VSN_V3)
    #if OMRON_FDTC_SUPPORT
    pConfig->ulStackMemorySize = 30*1024*1024;
    #else
    pConfig->ulStackMemorySize = 32*1024*1024;
    
    #endif
    #endif
    psrammap = &pConfig->sramMap ;
    psrammap->ulMMUTblAddr = (MMP_ULONG)&Image$$SRAM_MMU$$Base ;
    psrammap->ulMMUTblSize = 16*1024;
    psrammap->ulJPGLineBufAddr = (MMP_ULONG)&Image$$SRAM_JPG$$Base ;
    #if (CHIP==VSN_V2)
    psrammap->ulJPGLineBufSize = 0x7800; // TBD
    #endif
    #if (CHIP==VSN_V3)
    #if SUPPORT_GRA_ZOOM
    psrammap->ulJPGLineBufSize = 0x7800; // single line buffer mode : 1920 * 16
    #else
    psrammap->ulJPGLineBufSize = 0xF000; // double line buffer mode : 1920 * 32
    #endif
    
    #endif
    //psrammap->ulCodeAddr = (MMP_ULONG)&Image$$SRAM_CODE$$Base ;
    #if (CHIP==VSN_V2)
    psrammap->ulCodeSize = 0x1000; 
    #endif
    #if (CHIP==VSN_V3)
    //psrammap->ulCodeSize = 0x8000; 
    #endif
    
    psrammap->ulH264Addr = (MMP_ULONG)&Image$$SRAM_H264$$Base ;

    #if (CHIP==VSN_V2)
    psrammap->ulH264Size = 130*1024; 
    #endif
    #if (CHIP==VSN_V3)
    psrammap->ulH264Size = (384 + 90)*1024; // real-time H264 +90KB
    #endif
    // Share with H264 sram buffer, in this case, H264 should be paused.
    psrammap->ulJPGLineBufAddrOver1080p = psrammap->ulH264Addr ;
    psrammap->ulJPGLineBufSizeOver1080p = psrammap->ulH264Size ;
    
    #if ISP_BUF_IN_SRAM==1
    psrammap->ulISPDmaOprAddr = (MMP_ULONG)&Image$$SRAM_IQ_DMA$$Base ;
    psrammap->ulISPDmaOprSize = 16*1024 ;
    #endif
    psrammap->ulSramEndAddr = (MMP_ULONG)&Image$$SRAM_END$$Base;
    
    MMU_TRANSLATION_TABLE_ADDR = psrammap->ulMMUTblAddr ;
    MMU_COARSEPAGE_TABLE_ADDR = MMU_TRANSLATION_TABLE_ADDR + 0x400;
    return MMP_TRUE;
} /* MMPC_System_InitConfig */


void MMPC_System_DumpSramConfig(void)
{
#if 0
    MMPS_SYSTEM_SRAM_MAP *psrammap;
    psrammap = MMPS_System_GetSramMap(); ;
    dbg_printf(3,"sram_mmu : %x, size : %x\r\n" ,psrammap->ulMMUTblAddr,psrammap->ulMMUTblSize );
    dbg_printf(3,"sram_jpg : %x, size : %x\r\n" ,psrammap->ulJPGLineBufAddr,psrammap->ulJPGLineBufSize );
   // dbg_printf(3,"sram_code : %x, size : %x\r\n",psrammap->ulCodeAddr,psrammap->ulCodeSize );
    dbg_printf(3,"sram_h264 : %x, size : %x\r\n",psrammap->ulH264Addr,psrammap->ulH264Size );
    dbg_printf(3,"sram_end : %x\r\n" ,psrammap->ulSramEndAddr);
#endif
}

//------------------------------------------------------------------------------
//  Function    : MMPC_GPIO_InitConfig
//  Description :
//------------------------------------------------------------------------------
MMP_BOOL MMPC_GPIO_InitConfig(void)
{
#if (SUPPORT_ROTARY_ENCODER)

    MMP_UBYTE ret;

    #if (LIGHT_POSITIVE!=255)
    MMPF_PIO_Initialize();
    MMPF_PIO_EnableGpioMode(LIGHT_POSITIVE, MMP_TRUE);
    MMPF_PIO_EnableOutputMode(LIGHT_POSITIVE, MMP_FALSE);    // Input Mode
    #if (SUPPORT_TIMER_CTL_ENCODER == 0)
    MMPF_PIO_EnableTrigMode(LIGHT_POSITIVE, MMPF_PIO_TRIGMODE_EDGE_H2L, MMP_TRUE);
    MMPF_PIO_EnableInterrupt(LIGHT_POSITIVE, MMP_TRUE, 0x0, NULL);
    #endif
    #endif

    #if (LIGHT_NEGATIVE!=255)
    MMPF_PIO_Initialize();
    MMPF_PIO_EnableGpioMode(LIGHT_NEGATIVE, MMP_TRUE);
    MMPF_PIO_EnableOutputMode(LIGHT_NEGATIVE, MMP_FALSE);    // Input Mode
    //MMPF_PIO_EnableTrigMode(LIGHT_NEGATIVE, MMPF_PIO_TRIGMODE_EDGE_L2H, MMP_TRUE);
    //MMPF_PIO_EnableInterrupt(LIGHT_NEGATIVE, MMP_TRUE, 0x0, NULL);
    #endif

#endif

    // Power_LED_Gen
    #if (LED_ENABLE == 0)
    MMPF_PIO_EnableGpioMode(PWR_LED, MMP_TRUE);
    MMPF_PIO_EnableOutputMode(PWR_LED, MMP_TRUE);
    MMPF_PIO_SetData(PWR_LED, MMP_TRUE);
    #endif
}

//------------------------------------------------------------------------------
//  Function    : MMPS_System_GetConfiguration
//  Description :
//------------------------------------------------------------------------------
/** @brief The function gets the current system configuration for the host application

The function gets the current system configuration for reference by the host application. The current
configuration can be accessed from output parameter pointer. The function calls
MMPD_System_GetConfiguration to get the current settings from Host Device Driver Interface.

@return It return the pointer of the system configuration data structure.
*/
MMPS_SYSTEM_CONFIG* MMPS_System_GetConfiguration(void)
{
    return &m_systemConfig;
}

//==============================================================================
//  Function    : MMPS_USB_GetConfiguration
//==============================================================================
/** 
    @brief  Get Usb Configuration Structure
    @return m_UsbBufInfo
*/
MMPS_USBMODE_BUFFER_INFO*  MMPS_USB_GetConfiguration(void)
{
    return &m_UsbBufInfo;
}

MMPS_AUDIOMODE_CONFIG_INFO*  MMPS_AUDIO_GetConfiguration(void)
{
    return &m_AudioBufInfo;
}

MMPS_SYSTEM_SRAM_MAP *MMPS_System_GetSramMap(void)
{
    return &m_systemConfig.sramMap ;
}
