//==============================================================================
//
//  File        : mmph_hif.h
//  Description : INCLUDE File for the Host DSC Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPH_HIF_H_
#define _MMPH_HIF_H_

#include "mmp_lib.h"
#include "ait_bsp.h"
#ifdef BUILD_CE
#include "config_fw.h"
#endif
/** @addtogroup MMPH_reg
@{
*/

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

#define FUNC_SHFT           (0)
#define GRP_SHFT            (5)
#define SUB_SHFT            (8)

#define FUNC(_f)            (_f << FUNC_SHFT)
#define GRP(_g)             (_g << GRP_SHFT)
#define SUB(_s)             (_s << SUB_SHFT)

#define FUNC_MASK           FUNC(0x1F)
#define GRP_MASK            GRP(0x07)
#define SUB_MASK            SUB(0xFF)

#define GRP_SYS             GRP(0x00)
#define GRP_SENSOR          GRP(0x01)
#define GRP_DSC             GRP(0x02)
#define GRP_VID             GRP(0x03)
#define GRP_AUD             GRP(0x04)
#define GRP_STG             GRP(0x05)
#define GRP_USB             GRP(0x06)
#define GRP_USR             GRP(0x07)

#define HIF_CMD_SET_BYPASS_MODE         (GRP_SYS | FUNC(0x00))
#define     ENTER_BYPASS_MODE                       SUB(0x01)
#define     EXIT_BYPASS_MODE                        SUB(0x02)
#define HIF_CMD_VERSION_CONTROL         (GRP_SYS | FUNC(0x04))
#define     CUSTOMER_ID                             SUB(0x01)
#define     FW_ID                                   SUB(0x02)
#define     FW_RELEASE_TIME                         SUB(0x03)
#define     FW_RELEASE_DATE                         SUB(0x04)
#define HIF_CMD_SYSTEM_SET_PLL          (GRP_SYS | FUNC(0x06))
#define     PLL_SETTING                             SUB(0x00)
#define     PLL_POWER_UP                            SUB(0x01)
#define     PLL_HDMI_CHANGE                         SUB(0x02)
#define HIF_CMD_SET_PS_MODE             (GRP_SYS | FUNC(0x07))
#define     ENTER_PS_MODE                           SUB(0x01)
#define     EXIT_PS_MODE                            SUB(0x02)
#define HIF_CMD_CONFIG_TV               (GRP_SYS | FUNC(0x08))
#define     TV_COLORBAR                             SUB(0x01)
#define     TVENC_INIT                              SUB(0x02)
#define     TVENC_UNINIT                            SUB(0x03)
#define     TVENC_REG_SET                           SUB(0x04)
#define     TVENC_REG_GET                           SUB(0x05)
#define HIF_CMD_CONFIG_HINT             (GRP_SYS | FUNC(0x09))
#define     FLAG_ENABLE                             SUB(0x01)
#define     FLAG_DISABLE                            SUB(0x02)
#define     FLAG_CLEAR                              SUB(0x03)
#define	HIF_CMD_GET_FW_ADDR				(GRP_SYS | FUNC(0x0A))
#define     FW_END		                            SUB(0x01)
#define     AUDIO_START                             SUB(0x02)
#define HIF_CMD_ECHO             		(GRP_SYS | FUNC(0x0B))
#define     GET_INTERNAL_STATUS                     SUB(0x01)
#define HIF_CMD_SYSTEM_GET_GROUP_FREQ   (GRP_SYS | FUNC(0x0C))
#define HIF_CMD_SYSTEM_DRAM_MARCHCTEST  (GRP_SYS | FUNC(0x0E))

#define HIF_CMD_INIT_SENSOR             (GRP_SENSOR | FUNC(0x00))
#define HIF_CMD_SET_PREVIEW_MODE        (GRP_SENSOR | FUNC(0x01))
#define     ENABLE_PREVIEW                          SUB(0x80)
#define     DISABLE_PREVIEW                         SUB(0x00)
#define     COLOR_MODE_MASK                         SUB(0x07)
#define     RGB565_MODE                             SUB(0x00)
#define     RGB888_MODE                             SUB(0x01)
#define     YUV422_MODE                             SUB(0x02)
#define     YUV420_MODE                             SUB(0x03)
#define HIF_CMD_SENSOR_CONTROL          (GRP_SENSOR | FUNC(0x02))
#define     SET_REGISTER                            SUB(0x80)
#define     GET_REGISTER                            SUB(0x00)
#define HIF_CMD_SET_ZOOM_PARAM          (GRP_SENSOR | FUNC(0x03))
#define     ZOOM_PARAMTER                           SUB(0x01)
#define     SINGLE_ZOOM_RANGE                       SUB(0x02)
#define     ZOOM_DIRECTION                          SUB(0x03)
#define HIF_CMD_3A_FUNCTION             (GRP_SENSOR | FUNC(0x04))
#define     AUTO_FOCUS                              SUB(0x00)
#define     FOCUS_INFINITY                          SUB(0x01)
#define     LIGHT_FREQ                              SUB(0x02)
#define     AWB_TYPE                                SUB(0x03)
#define     AE_TYPE                                 SUB(0x04)
#define     NIGHT_MODE                              SUB(0x05)
#define     COLOR_CONTRAST                          SUB(0x06)
#define     COLOR_SATURATION                        SUB(0x07)
#define     COLOR_SHARPNESS                         SUB(0x08)
#define     SET_AF_WIN                              SUB(0x09)
#define     SET_AF_MODE                             SUB(0x10)
#define     GET_AF_STATUS                           SUB(0x11)
#define     SET_HW_BUFFER                           SUB(0x12)
#define     SET_LENS_POSITION                       SUB(0x13)
#define HIF_CMD_GET_EFFECT_NUM          (GRP_SENSOR | FUNC(0x05))
#define HIF_CMD_SET_PREVIEW_EFFECT      (GRP_SENSOR | FUNC(0x06))
#define HIF_CMD_SET_EXPOSURE_VALUE      (GRP_SENSOR | FUNC(0x07))
#define HIF_CMD_POWERDOWN_SENSOR        (GRP_SENSOR | FUNC(0x08))
#define HIF_CMD_SET_PREVIEW_BUF         (GRP_SENSOR | FUNC(0x09))
#define     BUFFER_ADDRESS                          SUB(0x00)
#define     BUFFER_COUNT                            SUB(0x01)
#define     BUFFER_WIDTH                            SUB(0x02)
#define     BUFFER_HEIGHT                           SUB(0x03)
#define HIF_CMD_SET_IBC_LINK_MODE       (GRP_SENSOR | FUNC(0x0A))
#define     LINK_NONE                               SUB(0x00)
#define     LINK_DISPLAY                            SUB(0x01)
#define     LINK_VIDEO                              SUB(0x02)
#define     LINK_DMA                                SUB(0x03)
#define     LINK_FDTC                               SUB(0x04)
#define     LINK_GRAPHIC                            SUB(0x05)
#define     UNLINK_GRAPHIC                          SUB(0x06)
#define HIF_CMD_SET_STABLE_STATE        (GRP_SENSOR | FUNC(0x0B))
#define HIF_CMD_SENSOR_PREVIEW_MODE     (GRP_SENSOR | FUNC(0x0C))
#define HIF_CMD_GET_ZOOM_PARAM          (GRP_SENSOR | FUNC(0x0D))
#define HIF_CMD_FDTC		           	(GRP_SENSOR | FUNC(0x0E))
#define 	SET_BUFFER								SUB(0x00)
#define     DO_FDTC                             	SUB(0x01)
#define     INIT_CONFIG                             SUB(0x02)
#define     SET_DISPLAY_INFO                        SUB(0x03)
#define HIF_CMD_SET_HISTEQ_STATE        (GRP_SENSOR | FUNC(0x0F))
#define     ZOOM_INFO                               SUB(0x01)
#define HIF_CMD_SET_SENSOR_MODE         (GRP_SENSOR | FUNC(0x10))
#define HIF_CMD_SET_ROTATE_BUF          (GRP_SENSOR | FUNC(0x11))
#define HIF_CMD_SET_SENSOR_FLIP         (GRP_SENSOR | FUNC(0x12))
#define HIF_CMD_GET_PREVIEW_BUF         (GRP_SENSOR | FUNC(0x13))
#define HIF_CMD_SET_AE_MODE             (GRP_SENSOR | FUNC(0x14))
#define HIF_CMD_SET_RAW_PREVIEW         (GRP_SENSOR | FUNC(0x15))
#define 	ENABLE_RAWSTORE							SUB(0x00)
#define 	RAWSTORE_ADDR							SUB(0x01)
#define     ENABLE_RAWPREVIEW                       SUB(0x02)
#define     ENABLE_DVSPREVIEW                       SUB(0x03)
#define     RAWSTORE_BAYER_ADDR                     SUB(0x04)
#define     RAWSTORE_LUMA_ADDR                      SUB(0x05)
#define     START_DVS_ME                            SUB(0x06)
#define     ENABLE_RAW_DOWNSAMPLE                   SUB(0x07)
#define     RESET_PREVIEW_ATTRIBUTE                 SUB(0x08)
#define     ENABLE_ZEROSHUTTERLAG_PATH              SUB(0x09)           //Ted ZSL
#define     SET_STORE_RANGE                         SUB(0x0A)
#define     SET_FETCH_RANGE                         SUB(0x0B)
#define HIF_CMD_SET_ZOOMOP              (GRP_SENSOR | FUNC(0x18))

#define HIF_CMD_SET_JPEG_QCTL           (GRP_DSC | FUNC(0x00))
#define     SET_QSCALE_ARRAY                        SUB(0x01)
#define     ENABLE_QSCALE_CTRL                      SUB(0x02)
#define HIF_CMD_TAKE_JPEG               (GRP_DSC | FUNC(0x01))
#define     INTERNAL_MEMORY                         SUB(0x00)
#define     STACK_MEMORY                            SUB(0x80)
#define     INTELLIGENT_ZOOM                        SUB(0x40)
#define HIF_CMD_SET_JPEG_RESOL          (GRP_DSC | FUNC(0x02))
#define HIF_CMD_SET_JPEG_MEDIA          (GRP_DSC | FUNC(0x03))
#define     STORAGE_PATH                            SUB(0x80)
#define     JPEG_FILE_NAME                          SUB(0x40)
#define HIF_CMD_DECODE_JPEG             (GRP_DSC | FUNC(0x04))
#define     GET_JPEG_INFO                           SUB(0x00)
#define     DECODE_JPEG                             SUB(0x01)
#define HIF_CMD_DSC_DECODE_PARAM        (GRP_DSC | FUNC(0x05))
#define     DATA_INPUT_BUF                          SUB(0x01)
#define HIF_CMD_TAKE_NEXT_JPEG          (GRP_DSC | FUNC(0x08))
#define HIF_CMD_TAKE_RAW_JPEG           (GRP_DSC | FUNC(0x09))
#define HIF_CMD_SET_IMAGE_PATH          (GRP_DSC | FUNC(0x0A))
#define     CAPTURE_PATH                            SUB(0x01)
#define HIF_CMD_PRAMA   		        (GRP_DSC | FUNC(0x0B))
#define     CALCULATE_MV                            SUB(0x01)
#define     PICTURE_MERGE                           SUB(0x02)
#define HIF_CMD_CARD_MODE_EXIF   		(GRP_DSC | FUNC(0x0C))
#define     EXIF_ENC                      			SUB(0x01)
#define     EXIF_DEC                      			SUB(0x02)
#define     GET_ENC_EXIF_OFFSET			            SUB(0x03)
#define     WRITE_EXIF							    SUB(0x04)
#define     EXIF_SET_FILL_TIME					    SUB(0x05)


#define HIF_CMD_JPEG_SAVE_CARD        	(GRP_DSC | FUNC(0x0D))
#define HIF_CMD_SET_ROTATE_CAPTURE      (GRP_DSC | FUNC(0x0E))
#define HIF_CMD_JPEG_STABLESHOT         (GRP_DSC | FUNC(0x0F)) //Ted 091029
#define     GET_STABLESHOT_INFO                     SUB(0x01) 
#define     SET_STABLESHOT_PARA            			SUB(0x02) 
#define HIF_CMD_CALIBRATE               (GRP_DSC | FUNC(0x11))
#define     START_CALIBRATE                         SUB(0x01)
#define     GET_CALIBRATE_INFO                      SUB(0x02)
#define     SET_CALIBRATE_INFO                      SUB(0x03)

#define HIF_CMD_PARSER                  (GRP_VID | FUNC(0x00))
#define     VIDPLAY_PARSE                           SUB(0x00)
#define     CLOSE_CONTENT                           SUB(0x01)
#define     VIDEO_DECODE_STORAGE_PATH               SUB(0x02)
#define     VIDEO_DECODE_FILE_NAME                  SUB(0x03)
#define     VIDEO_SET_HANDSHAKE_BUF                 SUB(0x04)
#define     VIDEO_SET_WORKINGBUF                    SUB(0x05)
#define     VIDEO_GET_WORKINGBUFSIZE                SUB(0x06)

#define HIF_CMD_VIDEO_DECODER           (GRP_VID | FUNC(0x02))
#define     INIT_MPEG4_DECODER                      SUB(0x00)
#define     INIT_DISPLAY_MEM                        SUB(0x01)
#define     DECODE_VO_HEADER                        SUB(0x02)
#define     DECODE_DATA                             SUB(0x03)
#define     DECODE_VIDEO_FRAME                      SUB(0x04)
#define     CLOSE_CODEC                             SUB(0x05)
#define HIF_CMD_VID_POSTPROCESSOR       (GRP_VID | FUNC(0x03))
#define HIF_CMD_VID_RENDERER            (GRP_VID | FUNC(0x04))
#define HIF_CMD_VIDPLAY_CLOCK           (GRP_VID | FUNC(0x05))
#define HIF_CMD_VIDPLAY_INFO            (GRP_VID | FUNC(0x06))
#define HIF_CMD_VIDPLAY_OTHER           (GRP_VID | FUNC(0x07))
#define HIF_CMD_VIDEO_DECODE_PARAM      (GRP_VID | FUNC(0x02))

#define HIF_CMD_MFD                     (GRP_VID | FUNC(0x08))
#define     MFD_MEM_SPACE                           SUB(0x00)
#define     MFD_CUR_MEM                             SUB(0x01)
#define     MFD_DECODE_HEADER                       SUB(0x02)
#define     MFD_DECODE_DATA                         SUB(0x03)
#define     MFD_GET_INFO                            SUB(0x04)
#define     MFD_GETPIC                              SUB(0x05)
#define     MFD_PP_INIT                             SUB(0x06)
#define     MFD_PP_CONFIG_DEINTERLACE_MEM           SUB(0x07)
#define     MFD_PP_CONFIG_SCALE_MEM                 SUB(0x08)
#define     MFD_PP_SET_CONFIG                       SUB(0x09)
#define     MFD_PP_SET_CONFIG_STANDALONE            SUB(0x0A)
#define     MFD_PP_GET_OUTPUT                       SUB(0x0B)
#define     MFD_PP_RELEASE                          SUB(0x0C)
#define     MFD_PP_CONFIG_PP_SCALE                  SUB(0x0D)

/*========================  begin of MDTV COMMAND =======================*/
#define HIF_CMD_MOBILE_TV               (GRP_VID | FUNC(0x08))
#define     VIDEO_PLAY_START                        SUB(0x01)
#define     VIDEO_PLAY_FILE_NAME                    SUB(0x02)
#define     VIDEO_PLAY_GET_RESOL                    SUB(0x03)
#define     VIDEO_PLAY_STOP                         SUB(0x04)
#define     VIDEO_PLAY_SET_AUDIO_VOLUME             SUB(0x06)
#define     VIDEO_PLAY_INIT                         SUB(0x07)
#define     GET_CHANNEL_NUM                         SUB(0x08)
#define     GET_CHANNEL_LISTS                       SUB(0x09)
#define     GET_PROGRAM_LISTS                       SUB(0x0A)
#define     GET_PROGRAM_NUM                         SUB(0x0B)
#define     GET_BUFFER_STATE                        SUB(0x0C)
#define     SET_MDTV_FREQ                           SUB(0x0D)
#define     CHK_ESG_DONE                            SUB(0x0E)
#define     START_UPDATE_ESG                        SUB(0x0F)
#define     STOP_UPDATE_ESG                         SUB(0x10)
#define     FLUSH_ESG_DB                            SUB(0x11)
#define     QUERY_ESG_DB_STATUS                     SUB(0x12)
#define     MDTV_INIT                               SUB(0x13)
#define     MDTV_CONFIG_MEM                         SUB(0x14)
#define     GET_MEM_START_ADDR                      SUB(0x15)
#define     MDTV_DBG_TIME                           SUB(0x16)
#define     MDTV_INIT_DISPLAY                       SUB(0x17)
#define     MDTV_INIT_DISPLAY_DONE                  SUB(0x18)
#define     MDTV_INIT_VIDEO_DONE                    SUB(0x19)
#define     GET_RECEIVER_STATUS                     SUB(0x1A)

#define HIF_CMD_MDTV_ESG                (GRP_VID | FUNC(0x12))
#define     MDTV_SET_MDTV_FREQ                      SUB(0x01)
#define     MDTV_GET_CHANNEL_NUM                    SUB(0x02)
#define     MDTV_GET_CHANNEL_LISTS                  SUB(0x03)
#define     MDTV_GET_PROGRAM_LISTS                  SUB(0x04)
#define     MDTV_GET_PROGRAM_NUM                    SUB(0x05)
#define     MDTV_CHK_ESG_DONE                       SUB(0x06)
#define     MDTV_START_UPDATE_ESG                   SUB(0x07)
#define     MDTV_STOP_UPDATE_ESG                    SUB(0x08)
#define     MDTV_FLUSH_ESG_DB                       SUB(0x09)
#define     MDTV_QUERY_ESG_DB_STATUS                SUB(0x0A)


#define HIF_CMD_H264_DEC                (GRP_VID | FUNC(0x13))
#define     H264_CONFIG_MEM_MAP                      SUB(0x01)
#define     H264_DEC_CODEC_INIT                      SUB(0x02)
#define     H264_DEC_DECODE_DATA                     SUB(0x03)
#define     H264_DEC_CODEC_RELEASE                   SUB(0x04)
#define     H264_CHECK_DECODE_DONE					 SUB(0x05)
#define     H264_SET_FRAME_REFRESH                   SUB(0x06)
#define     H264_GET_VIDEO_INFO                      SUB(0x07)
#define     H264_GET_VIDEO_FRAME                     SUB(0x08)
/*========================  end of MDTV COMMAND ============================*/

// This command is used to transfer data in memory mode
#define HIF_CMD_VIDEO_TRANSFER_DATA     (GRP_VID | FUNC(0x09))

// COMMON SUB: for value >= 0x40. It's used by any function
// COMMON SUB: first 2 bits 01b 0x40~0x7F(max 64). It's a command
#define     VIDPLAY_INIT                            SUB(0x40)
#define     VIDPLAY_START                           SUB(0x41)
#define     VIDPLAY_PAUSE                           SUB(0x42)
#define     VIDPLAY_RESUME                          SUB(0x43)
#define     VIDPLAY_STOP                            SUB(0x44)
#define     VIDPLAY_PREDECODE                       SUB(0x45)
#define     VIDPLAY_SEEK_BY_TIME                    SUB(0x46)
#define     VIDPLAY_SEEK_BY_FRAME                   SUB(0x47)
#define     VIDPLAY_SET_AUDIO_VOLUME                SUB(0x48)
#define     VIDPLAY_REDRAW                          SUB(0x49)
#define     VIDPLAY_DEBUG                           SUB(0x4F)
// host-parsed memory mode
#define     VIDPLAY_MEM_STOP                        SUB(0x50)
#define     VIDPLAY_CONTROL_AUDIO                   SUB(0x51)
#define     VIDPLAY_FLUSH                           SUB(0x52)
#define     VIDPLAY_CONFIG_MEM_DONE                 SUB(0x53)
#define     VIDPLAY_CLOSE                           SUB(0x54)
#define     VIDPARAM_VIDEO_DECODE_ENABLE            SUB(0x55)
#define     VIDPARAM_VIDEO_REFRESH_ENABLE           SUB(0x56)
#define     VIDPLAY_SET_PARSER_TYPE           		SUB(0x57)
#define     VIDPLAY_AUDIO_DECODE_ENABLE        		SUB(0x58)
#define     VIDPLAY_STEP                            SUB(0x59)
#define     VIDPLAY_SET_SEARCH_SPEED                SUB(0x60)
#define     VIDPLAY_REDRAW_LAST_REFRESH             SUB(0x61)

// COMMON SUB: first 2 bits 10b 0x80~0xBF.(max 64). It's a parameter to get.
#define     VIDPLAY_GET_PARAMETER                   SUB(0x80)
// COMMON SUB: first 2 bits 11b 0xC0~0xFF.(max 64). It's a parameter to get.
#define     VIDPLAY_SET_PARAMETER                   SUB(0xC0)
// parameters
#define     VIDPARAM_VIDEO_RESOL                    SUB(0x01)// width/height
#define     VIDPARAM_VIDEO_CROP                     SUB(0x02)// current frame
#define     VIDPARAM_TOTAL_TIME                     SUB(0x03)// file/audio/video total time, total size
#define     VIDPARAM_FORMAT                         SUB(0x04)// A/V format, A/V availability
#define     VIDPARAM_SAMPLING                       SUB(0x05)// audio sampling rate/channels, audio/video bitrate, FPS
#define     VIDPARAM_CURRENT_TIME                   SUB(0x06)
#define     VIDPARAM_CURRENT_STATE                  SUB(0x07)
#define     VIDPARAM_CURRENT_FRAME                  SUB(0x08)// in the player, it means the frame being rendered
#define     VIDPARAM_SPEED                          SUB(0x09)
#define     VIDPARAM_SUPPORTED_FORMAT               SUB(0x0A)// A/V supported format
#define     VIDPARAM_ACTIVE_PANEL                   SUB(0x0B)
#define     VIDPARAM_DEBLOCKTYPE                    SUB(0x0D)
#define     VIDPARAM_EMPTY_FRAME                    SUB(0x0E)// in the player, it means the frame being rendered
#define     VIDPARAM_DEPACK_SIZE                    SUB(0x0F)// depack buffer size

#define HIF_CMD_MERGER_PARAMETER        (GRP_VID | FUNC(0x10))
#define     AV_TIME_LIMIT                           SUB(0x02)
#define     AV_FILE_LIMIT                           SUB(0x03)
#define     AUDIO_ENCODE_CTL                        SUB(0x04)
#define     AV_SYNC_BUF_INFO                        SUB(0x05)
#define     AV_REPACK_BUF                           SUB(0x06)
#define     AUX_TABLE_ADDR                        	SUB(0x07)
#define     COMPRESS_BUF_ADDR                       SUB(0x08)
#define     GET_3GP_FILE_SIZE                       SUB(0x09)
#define     GET_3GP_DATA_RECODE                     SUB(0x0A)
#define     GET_3GP_FILE_STATUS                     SUB(0x0B)
#define     GET_MERGER_STATUS                       SUB(0x0C)
#define     ENCODE_STORAGE_PATH                     SUB(0x0D)
#define     ENCODE_FILE_NAME                        SUB(0x0E)
#define     AUDIO_AAC_MODE                          SUB(0x0F)
#define     AUDIO_AMR_MODE                          SUB(0x10)
#define     GET_RECORDING_TIME                      SUB(0x12)
#define     GET_RECORDING_FRAME                     SUB(0x13)
#define     VIDEO_BUFFER_THRESHOLD                  SUB(0x14)
#define		GOP_FRAME_TYPE                          SUB(0x16)
#define		ENCODE_SPEED_CTL						SUB(0x17)
/* ++swyang added for VIDEO_CONF_FW */
#define     AV_IS_ENCODE_FRAME_DONE                 SUB(0x20)
#define     AV_UPDATE_VIDEO_READ_PTR                SUB(0x21)
#define     AV_VOL_BUF_INFO                         SUB(0x22)
/* --swyang added for VIDEO_CONF_FW */
#define     ENCODE_DEBUG_MODE                       SUB(0xFF)
#define HIF_CMD_MERGER_OPERATION        (GRP_VID | FUNC(0x11))
#define HIF_CMD_MERGER_TAILSPEEDMODE    (GRP_VID | FUNC(0x12))
#define HIF_CMD_VIDRECD_PARAMETER       (GRP_VID | FUNC(0x18))
#define     ENCODE_RESOLUTION                       SUB(0x01)
#define     ENCODE_FORMAT                           SUB(0x02)
#define     ENCODE_QUALITY_CTL                      SUB(0x03)
#define     GET_VIDRECD_STATUS                      SUB(0x04)
#define		REG_STORE_BUF_INFO						SUB(0x05)
#define		ENCODE_FRAME_RATE						SUB(0x06)
#define		DVS_CONFIG       						SUB(0x07)
#define		DVS_BUF_CACHABLE                        SUB(0x08)
#define		DVS_ME_ENABLE                           SUB(0x09)
#define     ENCODE_TIMER_EN                         SUB(0x0A)
#define     SET_HEADER_BUF                          SUB(0x0B)
#define     GET_HEADER_INFO                         SUB(0x0C)
#define HIF_CMD_VIDRECD_OPERATION       (GRP_VID | FUNC(0x19))
#define     VIDEO_RECORD_START                      SUB(0x01)
#define     VIDEO_RECORD_PAUSE                      SUB(0x02)
#define     VIDEO_RECORD_RESUME                     SUB(0x03)
#define     VIDEO_RECORD_STOP                       SUB(0x04)

#define HIF_CMD_FS_ACCESS               (GRP_USR | FUNC(0x00))
#define         FS_FILE_OPEN                        SUB(0x00)
#define         FS_FILE_CLOSE                       SUB(0x01)
#define         FS_FILE_READ                        SUB(0x02)
#define         FS_FILE_WRITE                       SUB(0x03)
#define         FS_FILE_SEEK                        SUB(0x04)
#define         FS_FILE_TELL                        SUB(0x05)
#define         FS_FILE_COPY                        SUB(0x06)
#define         FS_FILE_TRUNCATE                    SUB(0x07)
#define         FS_FILE_GET_SIZE                    SUB(0x08)
#define         FS_FILE_REMOVE                      SUB(0x09)
#define         FS_FILE_DIR_GETATTRIBUTES           SUB(0x0A)
#define         FS_FILE_DIR_GETTIME                 SUB(0x0B)
#define         FS_FILE_DIR_MOVE                    SUB(0x0C)
#define         FS_FILE_DIR_RENAME                  SUB(0x0D)
#define         FS_FILE_DIR_SETATTRIBUTES           SUB(0x0E)
#define         FS_FILE_DIR_SETTIME                 SUB(0x0F)
#define         FS_DIR_CLOSE                        SUB(0x10)
#define         FS_DIR_GETATTRIBUTES                SUB(0x11)
#define         FS_DIR_GETNAME                      SUB(0x12)
#define         FS_DIR_GETSIZE                      SUB(0x13)
#define         FS_DIR_GETTIME                      SUB(0x14)
#define         FS_DIR_GETNUMFILES                  SUB(0x15)
#define         FS_DIR_CREATE                       SUB(0x16)
#define         FS_DIR_OPEN                         SUB(0x17)
#define         FS_DIR_READ                         SUB(0x18)
#define         FS_DIR_REWIND                       SUB(0x19)
#define         FS_DIR_REMOVE                       SUB(0x1A)
#define         FS_LOW_LEVEL_FORMAT                 SUB(0x1B)
#define         FS_HIGH_LEVLE_FORMAT                SUB(0x1C)
#define         FS_GET_FREE_SPACE                   SUB(0x1D)
#define         FS_GET_TOTAL_SPACE                  SUB(0x1E)
#define         FS_GET_NUM_VOLUMES                  SUB(0x1F)
#define         FS_GET_VOLUME_NAME                  SUB(0x20)
#define         FS_IS_VOLUME_MOUNTED                SUB(0x21)
#define         FS_CHECK_MEDIUM_PRESENT             SUB(0x22)
#define         FS_GET_VOLUME_INFO                  SUB(0x23)
#define         FS_GET_FILE_LIST                    SUB(0x24)
#define         FS_SET_CREATION_TIME                SUB(0x25)
#define         FS_DIR_GETNUMDIRS                   SUB(0x26)
#define         FS_INIT                             SUB(0x27)
#define         FS_MOUNT_VOLUME                     SUB(0x28)
#define         FS_UNMOUNT_VOLUME                   SUB(0x29)
#define         FS_FILE_DIR_GETINFO                 SUB(0x2A)
#define			FS_GET_MAX_DIR_COUNT				SUB(0x2B)
#define			FS_GET_MAX_FILE_COUNT				SUB(0x2C)
#define         FS_GET_FILE_LIST_DB                 SUB(0x2D)

#define HIF_CMD_FS_SETADDRESS           (GRP_USR | FUNC(0x01))
#define         FS_SET_FILENAME_ADDRESS             SUB(0x01)
#define         FS_SET_SD_TMP_ADDR                  SUB(0x02)
#define         FS_SET_SM_MAP_ADDR                  SUB(0x04)
#define         FS_SET_STORAGE_DMA_ADDR             SUB(0x05)
#define         FS_SET_FAT_GLOBALBUF_ADDR           SUB(0x06)
#define         FS_SET_DRM_IV_ADDR                  SUB(0x07)
#define HIF_CMD_FS_STORAGE              (GRP_USR | FUNC(0x02))
#define         FS_STORAGE_CONFIG                   SUB(0x02)
#define         FS_SM_ERASE                         SUB(0x03)
#define         FS_CHECK_CARDIN                     SUB(0x04)
#define         FS_IO_CTL                           SUB(0x05)

#if 1 //++Philip@20070504: defined for Brogent3D command handler
#define HIF_CMD_SET_DIRECTION           (GRP_USR | FUNC(0x03))
#define     DIR_UP                                 SUB(0x00)
#define     DIR_DOWN                               SUB(0x01)
#define     DIR_LEFT                               SUB(0x02)
#define     DIR_RIGHT                              SUB(0x03)
#define     DIR_ENTER                              SUB(0x04)
#define     DIR_EXIT                               SUB(0x05)
#define HIF_CMD_SET_INDEX               (GRP_USR | FUNC(0x04))

#define HIF_CMD_GET_HOST_CMD            (GRP_USR | FUNC(0x05))
#define HIF_CMD_GET_HOST_ARGS           (GRP_USR | FUNC(0x06))
#define     EX_ARG1                                 SUB(0x00)
#define     EX_ARG2                                 SUB(0x01)
#define HIF_CMD_GET_HOST_STRING         (GRP_USR | FUNC(0x07))
#define HIF_CMD_DEMO_FUNC               (GRP_USR | FUNC(0x08))
#define     EX_DEMO0                                SUB(0x00)
#define     EX_DEMO1                                SUB(0x01)
#define     EX_DEMO2                                SUB(0x02)
#endif //--Philip@20070504: defined for Brogent3D command handler
#define HIF_CMD_APP                     (GRP_USR | FUNC(0x09))
#define     SET_SDTC_BUF                            SUB(0x00)
#define     ENABLE_SDTC                             SUB(0x01)


#define HIF_CMD_MIDI_DEC_OP             (GRP_AUD | FUNC(0x00))
#define     DECODE_OP_PLAY_NOTE         			SUB(0x10)
#define     DECODE_OP_STOP_NOTE         			SUB(0x11)
#define     DECODE_OP_SET_PLAYTIME      			SUB(0x12)
#define     DECODE_OP_GET_PLAYTIME      			SUB(0x13)
#define HIF_CMD_AUDIO_DEC_OP            (GRP_AUD | FUNC(0x02))
#define     DECODE_OP_START             			SUB(0x01)
#define     DECODE_OP_PAUSE             			SUB(0x02)
#define     DECODE_OP_RESUME            			SUB(0x03)
#define     DECODE_OP_STOP              			SUB(0x04)
#define     DECODE_OP_GET_FILEINFO      			SUB(0x05)
#define     DECODE_OP_PREDECODE         			SUB(0x06)
#define     SET_MP3HD_ENABLE                        SUB(0x07)
#define HIF_CMD_SBC_ENCODE              (GRP_AUD | FUNC(0x03))
#define     SET_SBC_OUTPUT_BUF              		SUB(0x01)
#define     SET_SBC_ENCODE_ENABLE           		SUB(0x02)
#define     SET_SBC_CHANNEL_MODE            		SUB(0x03)
#define     SET_SBC_ALLOC_METHOD            		SUB(0x04)
#define     SET_SBC_SAMPLE_FREQ             		SUB(0x05)
#define     SET_SBC_NROF_BLOCKS             		SUB(0x06)
#define     SET_SBC_NROF_SUBBANDS           		SUB(0x07)
#define     SET_SBC_BITRATE                 		SUB(0x08)
#define     SET_SBC_HANDSHAKE_BUF           		SUB(0x09)
#define     GET_SBC_INFO                    		SUB(0x0A)

#define     SET_SBC_BITPOOL                         SUB(0x0C)
#define     GET_SBC_BITPOOL                         SUB(0x0D)
#define HIF_CMD_RA_DEC_OP               (GRP_AUD | FUNC(0x04))
#define     DECODE_OP_RA_SET_STARTTIME     	        SUB(0x11)
#define HIF_CMD_AUDIO_DECODE_PARAM      (GRP_AUD | FUNC(0x05))
#define     AUDIO_DECODE_BUF            			SUB(0x02)
#define     AUDIO_DECODE_FILE_NAME      			SUB(0x03)
#define     AUDIO_DECODE_PATH           			SUB(0x04)
#define     AUDIO_DECODE_GET_FILE_SIZE  			SUB(0x05)
#define     AUDIO_DECODE_SET_FILE_SIZE				SUB(0x06)
#define     AUDIO_DECODE_HANDSHAKE_BUF				SUB(0x07)
#define     AUDIO_DECODE_SPECTRUM_BUF				SUB(0x08)
#define     AUDIO_DECODE_FORMAT						SUB(0x09)
#define     AUDIO_DECODE_INT_THRESHOLD				SUB(0x0A)
#define     AUDIO_DECODE_DAC_PATH					SUB(0x0B)
#define     AUDIO_DECODE_I2S_FORMAT					SUB(0x0C)
#define     AUDIO_DECODE_SET_CALLBACK				SUB(0x0D)

#define HIF_CMD_AUDIO_EQ                (GRP_AUD | FUNC(0x06))
#define     AUDIO_EQ_NONE               			SUB(0x01)
#define     AUDIO_EQ_CLASSIC            			SUB(0x02)
#define     AUDIO_EQ_JAZZ               			SUB(0x03)
#define     AUDIO_EQ_POP                			SUB(0x04)
#define     AUDIO_EQ_ROCK               			SUB(0x05)
#define     AUDIO_EQ_BASS3              			SUB(0x06)
#define     AUDIO_EQ_BASS9              			SUB(0x07)
#define     AUDIO_EQ_LOUDNESS                       SUB(0x08)
#define     AUDIO_EQ_SPK                            SUB(0x09)
#define     AUDIO_HP_SURROUND           			SUB(0x10)
#define     AUDIO_SPK_SURROUND          			SUB(0x20)
#define     AUDIO_GRAPHIC_EQ                        SUB(0x30)
#define     AUDIO_ADJUST_EQ_BAND        			SUB(0x40)
#define		SET_OAEP_PARAMETER 					    SUB(0x80)

#define HIF_CMD_SET_VOLUME              (GRP_AUD | FUNC(0x07))
#define     AUDIO_PLAY_VOLUME                       SUB(0x01)
#define     AUDIO_RECORD_VOLUME            	        SUB(0x02)
#define HIF_CMD_WMA_DEC_OP              (GRP_AUD | FUNC(0x08))
#define     DECODE_OP_EXTRACT_TAG       			SUB(0x10)
#define     DECODE_OP_SET_STARTTIME     			SUB(0x11)
//#define     DECODE_OP_GET_VERSION	     			SUB(0x12)

#define HIF_CMD_AUDIO_PLAY_POS          (GRP_AUD | FUNC(0x09))
#define     SET_PLAY_POS                			SUB(0x01)
#define     GET_PLAY_POS                			SUB(0x02)
#define     SET_PLAY_TIME               			SUB(0x03)
#define     GET_PLAY_TIME               			SUB(0x04)

#define HIF_CMD_AB_REPEAT_MODE          (GRP_AUD | FUNC(0x0A))
#define     ABREPEAT_MODE_ENABLE        			SUB(0x01)
#define     ABREPEAT_MODE_DISABLE       			SUB(0x02)
#define     ABREPEAT_SET_POINTA         			SUB(0x03)
#define     ABREPEAT_SET_POINTB        			 SUB(0x04)
#define HIF_CMD_AUDIO_SPECTRUM          (GRP_AUD | FUNC(0x0B))
#define HIF_CMD_AUDIO_PLAY_STATUS       (GRP_AUD | FUNC(0x0C))
#define HIF_CMD_AUDIO_WRITE_STATUS      (GRP_AUD | FUNC(0x0D))
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define HIF_CMD_AUDIO_KARAOK		    (GRP_AUD | FUNC(0x0D))
#define 	KARAOK_VOCAL_REMOVE      				SUB(0x01)
#define 	KARAOK_MIC_VOLUME						SUB(0x02)    
#define 	KARAOK_SUPPORT    						SUB(0x03)    
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define HIF_CMD_MP3_CODE_OP			    (GRP_AUD | FUNC(0x0E))
#define		FLUSH_MP3_ZI_REGION						SUB(0x01)	
#define		INIT_MP3_ZI_REGION						SUB(0x02)	
#define		FLUSH_AUDIO_ZI_REGION					SUB(0x03)	
#define		INIT_AUDIO_ZI_REGION					SUB(0x04)	
#define		DUMMY_CODE_LINK1						SUB(0x05)	
#define		DUMMY_CODE_LINK2						SUB(0x06)	

#define HIF_CMD_AUDIO_GAPLESS           (GRP_AUD | FUNC(0x0F))
#define     GAPLESS_SET_MODE                        SUB(0x01)
#define     GET_ALLOWED_OP                          SUB(0x02)

#define HIF_CMD_AUDIO_ENC_OP          	(GRP_AUD | FUNC(0x11))
#define     ENCODE_OP_START             			SUB(0x01)
#define     ENCODE_OP_PAUSE             			SUB(0x02)
#define     ENCODE_OP_RESUME            			SUB(0x03)
#define     ENCODE_OP_STOP              			SUB(0x04)
#define     ENCODE_OP_GET_FILESIZE      			SUB(0x05)
#define     ENCODE_OP_PREENCODE         			SUB(0x06)

#define HIF_CMD_AUDIO_ENCODE_PARAM      (GRP_AUD | FUNC(0x12))
#define     AUDIO_ENCODE_MODE           			SUB(0x01)
#define     AUDIO_ENCODE_BUF            			SUB(0x02)
#define     AUDIO_ENCODE_FILE_LIMIT     			SUB(0x03)
#define     AUDIO_ENCODE_PATH           			SUB(0x04)
#define     AUDIO_ENCODE_FILE_NAME      			SUB(0x05)
#define     AUDIO_ENCODE_HANDSHAKE_BUF  			SUB(0x06)
#define     AUDIO_ENCODE_INT_THRESHOLD  			SUB(0x07)
#define     AUDIO_ENCODE_I2S_FORMAT     			SUB(0x08)
#define     AUDIO_ENCODE_FORMAT         			SUB(0x09)
#define     AUDIO_ENCODE_ADC_PATH					SUB(0x0A)
#define		AUDIO_ENCODE_LINEIN_CHANNEL				SUB(0x0B)

#define HIF_CMD_AUDIO_REC_STATUS		(GRP_AUD | FUNC(0x13))
#define HIF_CMD_AUDIO_AEC          		(GRP_AUD | FUNC(0x16))
#define HIF_CMD_AUDIO_DUPLEX_PATH   	(GRP_AUD | FUNC(0x17))

#define HIF_CMD_WAV_ENCODE              (GRP_AUD | FUNC(0x18))
#define     SET_WAV_OUTPUT_BUF              		SUB(0x01)
#define     SET_WAV_ENCODE_ENABLE                   SUB(0x02)
#define     GET_WAV_INFO                            SUB(0x03)

#define HIF_CMD_DRM                     (GRP_AUD | FUNC(0x19))
#define     SET_DRM_IV                        		SUB(0x01)
#define     SET_DRM_KEY                             SUB(0x02)
#define     SET_DRM_EN                              SUB(0x03)
#define     SET_DRM_HEADERLENGTH                    SUB(0x04)

#define HIF_CMD_AUDIO_BYPASS_PATH       (GRP_AUD | FUNC(0x1A))
#define HIF_CMD_AUDIO_DAC               (GRP_AUD | FUNC(0x1B))

#define HIF_CMD_AC3_CODE_OP             (GRP_AUD | FUNC(0x1C))
#define     SET_AC3_VHPAR                           SUB(0x01)

#define HIF_CMD_USB_SETMODE             (GRP_USB | FUNC(0x00))
#define HIF_CMD_USB_SET_MSDCBUF			(GRP_USB | FUNC(0x01))
#define HIF_CMD_USB_SET_PCSYNCBUF       (GRP_USB | FUNC(0x02))
#define     SET_PCSYNC_IN_BUF						SUB(0x01)
#define     SET_PCSYNC_OUT_BUF              		SUB(0x02)
#define     SET_PCSYNC_HANDSHAKE_BUF        		SUB(0x03)
#define HIF_CMD_USB_SET_CONTROLBUF		(GRP_USB | FUNC(0x03))
#define HIF_CMD_USB_SET_PCCAMBUF		(GRP_USB | FUNC(0x04))
#define     SET_PCCAM_COMPRESS_BUF					SUB(0x01)
#define     SET_PCCAM_LINE_BUF              		SUB(0x02)
#define HIF_CMD_USB_SET_MTPBUF          (GRP_USB | FUNC(0x05))
#define     SET_MTP_EP_BUF              			SUB(0x01)
#define HIF_CMD_DPS         			(GRP_USB | FUNC(0x06))
#define     DPS_START_JOB              				SUB(0x01)
#define     DPS_ABORT_JOB              				SUB(0x02)
#define     DPS_CONTINUE_JOB              			SUB(0x03)
#define     DPS_PRINTER_STATUS              		SUB(0x04)
#define     DPS_GETBUFADDR                  		SUB(0x05)
#define     DPS_GETFILEID                   		SUB(0x06)
#define HIF_CMD_PCCAM_CAPTURE           (GRP_USB | FUNC(0x07))
#define HIF_CMD_USB_SET_MEMDEV_BUF			(GRP_USB | FUNC(0x08))
#define HIF_CMD_MSDC_ACK                (GRP_USB | FUNC(0x09))
#define     HIF_WRITE_ACK                           SUB(0x01)
#define     HIF_READ_ACK                            SUB(0x02)

// Host<->Firmware Status, reserve b[15:0] for command executing status
#define CPU_BOOT_FINISH                             (0x00000001)
#define SENSOR_CMD_IN_EXEC                          (0x00000002)
#define DSC_CMD_IN_EXEC                             (0x00000004)
#define SYSTEM_CMD_IN_EXEC                          (0x00000008)
#define AUDIO_CMD_IN_EXEC                           (0x00000010)
#define MGR3GP_CMD_IN_EXEC                          (0x00000020)
#define PLAYVID_CMD_IN_EXEC                         (0x00000040)
#define FS_CMD_IN_EXEC                              (0x00000080)
#define RENDER_CMD_IN_EXEC                          (0x00000100)	// for Brogent FW
#define VIDRECD_CMD_IN_EXEC                         (0x00000100)
#define VIDDEC_CMD_IN_EXEC	                        (0x00000200)
#define USB_CMD_IN_EXEC                             (0x00000400)
#define ESG_CMD_IN_EXEC                             (0x00000800)

#define MGR3GP_CANCEL_FILE_SAVE                     (0x00010000)
#define MEDIA_FILE_OVERFLOW                         (0x00020000)
#define DSC_CAPTURE_BEGIN                           (0x00040000)
#define IMAGE_UNDER_ZOOM                            (0x00080000)
#define SD_CARD_NOT_EXIST                           (0x00100000)
//#define FDTC_CMD_IN_EXEC                            (0x0020000)
#define IMAGE_UNDER_PANTILT                         (0x00400000)
#define APP_CMD_IN_EXEC                             (0x08000000)

#ifdef BUILD_CE
#define SYS_FLAG_FS_CMD_DONE                        (0x10000000)
#define SYS_FLAG_AUD_CMD_DONE                       (0x20000000)
#define SYS_FLAG_DSC_CMD_DONE                       (0x40000000)
#define SYS_FLAG_SYS_CMD_DONE                       (0x80000000)
#define SYS_FLAG_CE_JOB_COMMAND                		(0x01000000)
#define SYS_FLAG_DPF_JPEG                      		(0x02000000)
#define SYS_FLAG_DPF_UI                      		(0x04000000)
#define SYS_FLAG_DPF_WAIT_INFO_RDY                  (0x08000000)
#define SYS_FLAG_DPF_USB                      		(0x00100000)
#define SYS_FLAG_USB_CMD_DONE                      	(0x00200000)
#define SYS_FLAG_VID_CMD_DONE                      	(0x00400000)
#define SYS_FLAG_SENSOR_CMD_DONE                    (0x00800000)

enum
{
	CMD_SHOW_JPEG_01=0,
	CMD_SHOW_PREVIEW_NO_STATUS_ICON,
	CMD_SHOW_PREVIEW_WITH_STATUS_ICON,
	CMD_SHOW_DISPLAY_NO_STATUS_ICON,
	CMD_SHOW_DISPLAY_WITH_STATUS_ICON,
	CMD_SHOW_HOME_SELECT_SHOOTING,
	CMD_SHOW_HOME_SELECT_VIEWIMAGE1,
	CMD_SHOW_HOME_SELECT_VIEWIMAGE2,
	CMD_SHOW_HOME_SELECT_PRINT,
	CMD_SHOW_HOME_SELECT_MANAGEMEMORY,
	CMD_SHOW_HOME_SELECT_SETTINGS,
	CMD_START_TP_CALIBRATION,
	CMD_TAKE_PHOTO,
	CMD_PHOTO_PLAYBACK,
	CMD_PHOTO_SHOOTING,
	CMD_VIDEO_RECORD,
	CMD_START_VIDEO_RECORD,
	CMD_STOP_VIDEO_RECORD,
	CMD_3GP_PLAYBACK_CHECK_STATUS,
	CMD_PHOTO_SLIDESHOW_CHANGE_PHOTO,
	MAX_CMD_NO
};

#endif

#define RTNA_HOSTIF_DATA_PORT       (RTNA_CS_BASE)
#define RTNA_HOSTIF_CMD_PORT        (RTNA_CS_BASE | RTNA_A0_SEL)
#define HIF_CmdP                    *(volatile MMP_UBYTE *)(RTNA_HOSTIF_CMD_PORT)
#define HIF_CmdPW                   *(volatile MMP_USHORT *)(RTNA_HOSTIF_CMD_PORT)
#define HIF_DataPW                  *(volatile MMP_USHORT *)(RTNA_HOSTIF_DATA_PORT)
#define HIF_DataPB(_a)              *(volatile MMP_UBYTE *)(RTNA_HOSTIF_DATA_PORT+(_a&0x1))

#define HIF_SET_ADDRH               0x00
#define HIF_SET_ADDRL               0x01
#define HIF_REG_WR_B                0x02
#define HIF_REG_WR_B_INC            0x03
#define HIF_REG_WR_W                0x04
#define HIF_REG_WR_W_INC            0x05
#define HIF_MEM_WR_B                0x06
#define HIF_MEM_WR_B_INC            0x07
#define HIF_MEM_WR_W                0x08
#define HIF_MEM_WR_W_INC            0x09
#define HIF_REG_WR_W_ADR0           0x0A
#define HIF_REG_WR_W_ADR1           0x0B
#define HIF_FIFO_WR                 0x0C
#define HIF_FIFO_RD                 0x0D
    #define HIF_FIFO_LEN_MASK               0xF0
    #define HIF_FIFO_LEN_SHFT               4
#define HIF_REG_RD_B                0x12
#define HIF_REG_RD_W                0x14
#define HIF_MEM_RD_B                0x16
#define HIF_MEM_RD_B_INC            0x17
#define HIF_MEM_RD_W                0x18
#define HIF_MEM_RD_W_INC            0x19
#define HIF_REG_RD_W_ADR0           0x1A
#define HIF_REG_RD_W_ADR1           0x1B
#define HIF_REG_CLR_IRQ             0xFF
#define HIF_MEM_BIG_FIFO_CNT        0x400

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPH_HIF_INTERFACE {
    MMPH_HIF_16_BIT_MODE = 1,
    MMPH_HIF_8_BIT_MODE,
    MMPH_HIF_BYPASS // Standalone, access register directly.
} MMPH_HIF_INTERFACE;


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
#ifdef BUILD_CE
MMP_UBYTE   Ind_RegGetB(MMP_ULONG ulAddr);
MMP_USHORT  Ind_RegGetW(MMP_ULONG ulAddr);
MMP_ULONG   Ind_RegGetL(MMP_ULONG ulAddr);
MMP_USHORT  Ind_RegFGetW0(void);
MMP_USHORT  Ind_RegFGetW1(void);
void        Ind_RegSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        Ind_RegSetW(MMP_ULONG ulAddr, MMP_USHORT usData);
void        Ind_RegSetL(MMP_ULONG ulAddr, MMP_ULONG ulData);
void        Ind_RegFSetW0(MMP_USHORT usData);
void        Ind_RegFSetW1(MMP_USHORT usData);
MMP_UBYTE   Ind_MemGetB(MMP_ULONG ulAddr);
MMP_USHORT  Ind_MemGetW(MMP_ULONG ulAddr);
MMP_ULONG   Ind_MemGetL(MMP_ULONG ulAddr);
void        Ind_MemSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        Ind_MemSetW(MMP_ULONG ulAddr, MMP_USHORT usData);
void        Ind_MemSetL(MMP_ULONG ulAddr, MMP_ULONG ulData);
void        Ind_FIFOOpenR(MMP_ULONG ulFifoaddr, MMP_USHORT usReadcount);
void        Ind_FIFOOpenW(MMP_ULONG ulFifoaddr, MMP_USHORT usWritecount);
MMP_USHORT  Ind_FIFOReadW(void);
void        Ind_FIFOWriteW(MMP_USHORT usData);
void        Ind_FIFOCReadW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usReadcount);
void        Ind_FIFOCWriteW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usWritecount);
void        Ind_ClearIrq(void);

#else

MMP_UBYTE   Ind_16_RegGetB(MMP_ULONG ulAddr);
MMP_UBYTE   Ind_8_RegGetB(MMP_ULONG ulAddr);
MMP_USHORT  Ind_16_RegGetW(MMP_ULONG ulAddr);
MMP_USHORT  Ind_8_RegGetW(MMP_ULONG ulAddr);
MMP_ULONG   Ind_16_RegGetL(MMP_ULONG ulAddr);
MMP_ULONG   Ind_8_RegGetL(MMP_ULONG ulAddr);

MMP_USHORT  Ind_16_RegFGetW0(void);
MMP_USHORT  Ind_8_RegFGetW0(void);
MMP_USHORT  Ind_16_RegFGetW1(void);
MMP_USHORT  Ind_8_RegFGetW1(void);

void        Ind_16_RegSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        Ind_8_RegSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        Ind_16_RegSetW(MMP_ULONG ulAddr, MMP_USHORT usData);
void        Ind_8_RegSetW(MMP_ULONG ulAddr, MMP_USHORT usData);
void        Ind_16_RegSetL(MMP_ULONG ulAddr, MMP_ULONG ulData);
void        Ind_8_RegSetL(MMP_ULONG ulAddr, MMP_ULONG ulData);

void        Ind_16_RegFSetW0(MMP_USHORT usData);
void        Ind_8_RegFSetW0(MMP_USHORT usData);
void        Ind_16_RegFSetW1(MMP_USHORT usData);
void        Ind_8_RegFSetW1(MMP_USHORT usData);

MMP_UBYTE   Ind_16_MemGetB(MMP_ULONG ulAddr);
MMP_UBYTE   Ind_8_MemGetB(MMP_ULONG ulAddr);
MMP_USHORT  Ind_16_MemGetW(MMP_ULONG ulAddr);
MMP_USHORT  Ind_8_MemGetW(MMP_ULONG ulAddr);
MMP_ULONG   Ind_16_MemGetL(MMP_ULONG ulAddr);
MMP_ULONG   Ind_8_MemGetL(MMP_ULONG ulAddr);
void        Ind_16_MemSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        Ind_8_MemSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        Ind_16_MemSetW(MMP_ULONG ulAddr, MMP_USHORT usData);
void        Ind_8_MemSetW(MMP_ULONG ulAddr, MMP_USHORT usData);
void        Ind_16_MemSetL(MMP_ULONG ulAddr, MMP_ULONG ulData);
void        Ind_8_MemSetL(MMP_ULONG ulAddr, MMP_ULONG ulData);

void        Ind_16_FIFOOpenR(MMP_ULONG ulFifoaddr, MMP_USHORT usReadcount);
void        Ind_8_FIFOOpenR(MMP_ULONG ulFifoaddr, MMP_USHORT usReadcount);
void        Ind_16_FIFOOpenW(MMP_ULONG ulFifoaddr, MMP_USHORT usWritecount);
void        Ind_8_FIFOOpenW(MMP_ULONG ulFifoaddr, MMP_USHORT usWritecount);
MMP_USHORT  Ind_16_FIFOReadW(void);
MMP_USHORT  Ind_8_FIFOReadW(void);
void        Ind_16_FIFOWriteW(MMP_USHORT usData);
void        Ind_8_FIFOWriteW(MMP_USHORT usData);
void        Ind_16_FIFOCReadW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usReadcount);
void        Ind_8_FIFOCReadW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usReadcount);
void        Ind_16_FIFOCWriteW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usWritecount);
void        Ind_8_FIFOCWriteW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usWritecount);
void        Ind_16_ClearIrq(void);
void        Ind_8_ClearIrq(void);
#endif

MMP_ERR MMPH_HIF_SetInterface(MMPH_HIF_INTERFACE mode);
MMP_ERR MMPH_HIF_GetInterface(MMPH_HIF_INTERFACE *mode);

#ifdef  AIT_REG_FUNC_DECLARE
MMP_UBYTE   (*MMPH_HIF_RegGetB)(MMP_ULONG ulAddr);
MMP_USHORT  (*MMPH_HIF_RegGetW)(MMP_ULONG ulAddr);
MMP_ULONG   (*MMPH_HIF_RegGetL)(MMP_ULONG ulAddr);
void        (*MMPH_HIF_RegSetB)(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        (*MMPH_HIF_RegSetW)(MMP_ULONG ulAddr, MMP_USHORT usData);
void        (*MMPH_HIF_RegSetL)(MMP_ULONG ulAddr, MMP_ULONG ulData);
MMP_UBYTE   (*MMPH_HIF_MemGetB)(MMP_ULONG ulAddr);
MMP_USHORT  (*MMPH_HIF_MemGetW)(MMP_ULONG ulAddr);
MMP_ULONG   (*MMPH_HIF_MemGetL)(MMP_ULONG ulAddr);
void        (*MMPH_HIF_MemSetB)(MMP_ULONG ulAddr, MMP_UBYTE ubData);
void        (*MMPH_HIF_MemSetW)(MMP_ULONG ulAddr, MMP_USHORT usData);
void        (*MMPH_HIF_MemSetL)(MMP_ULONG ulAddr, MMP_ULONG ulData);
void        (*MMPH_HIF_FIFOOpenR)(MMP_ULONG ulFifoaddr, MMP_USHORT usReadcount);
void        (*MMPH_HIF_FIFOOpenW)(MMP_ULONG ulFifoaddr, MMP_USHORT usWritecount);
MMP_USHORT  (*MMPH_HIF_FIFOReadW)(void);
void        (*MMPH_HIF_FIFOWriteW)(MMP_USHORT usData);
void        (*MMPH_HIF_FIFOCReadW)(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT i);
void        (*MMPH_HIF_FIFOCWriteW)(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT i);
MMP_USHORT  (*MMPH_HIF_RegFGetW0)(void);
MMP_USHORT  (*MMPH_HIF_RegFGetW1)(void);
void        (*MMPH_HIF_RegFSetW0)(MMP_USHORT usData);
void        (*MMPH_HIF_RegFSetW1)(MMP_USHORT usData);
void        (*MMPH_HIF_ClearIrq)(void);
#else
extern  MMP_UBYTE   (*MMPH_HIF_RegGetB)(MMP_ULONG ulAddr);
extern  MMP_USHORT  (*MMPH_HIF_RegGetW)(MMP_ULONG ulAddr);
extern  MMP_ULONG   (*MMPH_HIF_RegGetL)(MMP_ULONG ulAddr);
extern  void        (*MMPH_HIF_RegSetB)(MMP_ULONG ulAddr, MMP_UBYTE ubData);
extern  void        (*MMPH_HIF_RegSetW)(MMP_ULONG ulAddr, MMP_USHORT usData);
extern  void        (*MMPH_HIF_RegSetL)(MMP_ULONG ulAddr, MMP_ULONG ulData);
extern  MMP_UBYTE   (*MMPH_HIF_MemGetB)(MMP_ULONG ulAddr);
extern  MMP_USHORT  (*MMPH_HIF_MemGetW)(MMP_ULONG ulAddr);
extern  MMP_ULONG   (*MMPH_HIF_MemGetL)(MMP_ULONG ulAddr);
extern  void        (*MMPH_HIF_MemSetB)(MMP_ULONG ulAddr, MMP_UBYTE ubData);
extern  void        (*MMPH_HIF_MemSetW)(MMP_ULONG ulAddr, MMP_USHORT usData);
extern  void        (*MMPH_HIF_MemSetL)(MMP_ULONG ulAddr, MMP_ULONG ulData);
extern  void        (*MMPH_HIF_FIFOOpenR)(MMP_ULONG ulFifoaddr, MMP_USHORT usReadcount);
extern  void        (*MMPH_HIF_FIFOOpenW)(MMP_ULONG ulFifoaddr, MMP_USHORT usWritecount);
extern  MMP_USHORT  (*MMPH_HIF_FIFOReadW)(void);
extern  void        (*MMPH_HIF_FIFOWriteW)(MMP_USHORT usData);
extern  void        (*MMPH_HIF_FIFOCReadW)(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT i);
extern  void        (*MMPH_HIF_FIFOCWriteW)(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT i);
extern  MMP_USHORT  (*MMPH_HIF_RegFGetW0)(void);
extern  MMP_USHORT  (*MMPH_HIF_RegFGetW1)(void);
extern  void        (*MMPH_HIF_RegFSetW0)(MMP_USHORT usData);
extern  void        (*MMPH_HIF_RegFSetW1)(MMP_USHORT usData);
extern  void        (*MMPH_HIF_ClearIrq)(void);
#endif






MMP_ERR MMPH_HIF_CmdSend(MMP_USHORT usCommand);
void    MMPH_HIF_CmdSetParameterB(MMP_UBYTE ubParamnum, MMP_UBYTE ubParamdata);
void    MMPH_HIF_CmdSetParameterW(MMP_UBYTE ubParamnum, MMP_USHORT usParamdata);
void	MMPH_HIF_CmdSetParameterL(MMP_UBYTE ubParamnum, MMP_ULONG ulParamdata);
MMP_UBYTE   MMPH_HIF_CmdGetParameterB(MMP_UBYTE ubParamnum);
MMP_USHORT  MMPH_HIF_CmdGetParameterW(MMP_UBYTE ubParamnum);
MMP_ULONG   MMPH_HIF_CmdGetParameterL(MMP_UBYTE ubParamnum);
MMP_ULONG   MMPH_HIF_CmdGetStatusL(void);
void	MMPH_HIF_CmdSetStatusL(MMP_ULONG status);
void   MMPH_HIF_CmdClearStatusL(MMP_ULONG status);


void	MMPH_HIF_MemCopyHostToDev(MMP_ULONG ulDestaddr, MMP_UBYTE *usSrcaddr,
                MMP_ULONG ulLength);
void    MMPH_HIF_MemCopyDevToHost(MMP_UBYTE *ubDestaddr, MMP_ULONG ulSrcaddr,
                MMP_ULONG ulLength);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


/// @}
#endif // _MMPH_HIF_H_
