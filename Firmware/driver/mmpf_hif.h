//==============================================================================
//
//  File        : mmpf_hif.h
//  Description : INCLUDE File for the Firmware Host Interface Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_HIF_H_
#define _MMPF_HIF_H_
///@ait_only
/** @addtogroup MMPF_HIF
@{
*/


#define FUNC_SHFT           (0)
#define GRP_SHFT            (5)
#define SUB_SHFT            (8)

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

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


#define HIF_CMD_SET_BYPASS_MODE         (GRP_SYS | FUNC(0x00))
#define     ENTER_BYPASS_MODE                       SUB(0x01)
#define     EXIT_BYPASS_MODE                        SUB(0x02)
#define HIF_CMD_SET_CAMERA_MODE         (GRP_SYS | FUNC(0x01))
#define     CAMERA_MODE                             SUB(0x01)
#define     VIDEOPLAY_MODE                          SUB(0x02)
#define     VIDEORECORD_MODE                        SUB(0x03)
#define HIF_CMD_GET_INTERNAL_STATUS     (GRP_SYS | FUNC(0x02))
#define     CAMERA_STATUS                           SUB(0x01)
#define     VIDEO_STATUS                            SUB(0x02)
#define     AUDIO_STATUS                            SUB(0x03)
#define     VIDEO_STOP_FRAME                        SUB(0x04)
#define     AUDIO_STOP_POS                          SUB(0x05)
#define     MIDI_STOP_TIME                          SUB(0x06)
#define     ECHO_COMMAND                            SUB(0x07)
#define HIF_CMD_SET_INTERNAL_STATUS     (GRP_SYS | FUNC(0x03))
#define HIF_CMD_VERSION_CONTROL         (GRP_SYS | FUNC(0x04))
#define     CUSTOMER_ID                             SUB(0x01)
#define     FW_ID                                   SUB(0x02)
#define     FW_RELEASE_TIME                         SUB(0x03)
#define     FW_RELEASE_DATE                         SUB(0x04)
#define HIF_CMD_SET_DAC_MCLK            (GRP_SYS | FUNC(0x05))
#define     DAC_MCLK_ON                             SUB(0x80)
#define     DAC_MCLK_OFF                            SUB(0x00)
#define HIF_CMD_SYSTEM_SET_PLL          (GRP_SYS | FUNC(0x06))
#define     PLL_SETTING                             SUB(0x00)
#define     PLL_POWER_UP                            SUB(0x01)
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
#define		LINK_NONE								SUB(0x00)
#define		LINK_DISPLAY							SUB(0x01)
#define		LINK_VIDEO								SUB(0x02)
#define     LINK_DMA                                SUB(0x03)
#define     LINK_FDTC                               SUB(0x04)
#define     LINK_GRAPHIC                            SUB(0x05)
#define     UNLINK_GRAPHIC                          SUB(0x06)
#define HIF_CMD_SET_STABLE_STATE        (GRP_SENSOR | FUNC(0x0B))
#define HIF_CMD_SENSOR_PREVIEW_MODE     (GRP_SENSOR | FUNC(0x0C))
#define HIF_CMD_GET_ZOOM_PARAM          (GRP_SENSOR | FUNC(0x0D))
#define HIF_CMD_FDTC		        	(GRP_SENSOR | FUNC(0x0E))
#define 	SET_BUFFER								SUB(0x00)
#define     DO_FDTC                                 SUB(0x01)
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

#define HIF_CMD_VIDEO_DECODER           (GRP_VID | FUNC(0x02))
#define     INIT_MPEG4_DECODER                      SUB(0x00)
#define     INIT_DISPLAY_MEM                        SUB(0x01)
#define     DECODE_VO_HEADER                        SUB(0x02)
#define     DECODE_DATA                             SUB(0x03)
#define HIF_CMD_VID_POSTPROCESSOR       (GRP_VID | FUNC(0x03))
#define HIF_CMD_VID_RENDERER            (GRP_VID | FUNC(0x04))
#define HIF_CMD_VIDPLAY_CLOCK           (GRP_VID | FUNC(0x05))
#define HIF_CMD_VIDPLAY_INFO            (GRP_VID | FUNC(0x06))
#define HIF_CMD_VIDPLAY_OTHER           (GRP_VID | FUNC(0x07))

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
#define     H264_CHECK_DECODE_DONE                  SUB(0x05)
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

// COMMON SUB: first 2 bits 10b 0x80~0xBF.(max 64). It's a parameter to get.
#define     VIDPLAY_GET_PARAMETER                   SUB(0x80)
// COMMON SUB: first 2 bits 11b 0xC0~0xFF.(max 64). It's a parameter to get.
#define     VIDPLAY_SET_PARAMETER                   SUB(0xC0)
// parameters
#define     VIDPARAM_VIDEO_RESOL                    SUB(0x01)// width/height
#define     VIDPARAM_VIDEO_CROP                     SUB(0x02)// 4 croppings
#define     VIDPARAM_TOTAL_TIME                     SUB(0x03)// file/audio/video total time, total size
#define     VIDPARAM_FORMAT                         SUB(0x04)// A/V format, A/V availability, A/V format supported
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

#define HIF_CMD_MERGER_PARAMETER      	(GRP_VID | FUNC(0x10))
#define     AV_TIME_LIMIT                           SUB(0x02)
#define     AV_FILE_LIMIT                           SUB(0x03)
#define     AUDIO_ENCODE_CTL                        SUB(0x04)
#define     AV_SYNC_BUF_INFO                        SUB(0x05)
#define     AV_REPACK_BUF                           SUB(0x06)
#define     AUX_TABLE_ADDR                          SUB(0x07)
#define     COMPRESS_BUF_ADDR                       SUB(0x08)
#define     GET_3GP_FILE_SIZE                       SUB(0x09)
#define     GET_3GP_DATA_RECODE                     SUB(0x0A)
#define     GET_3GP_FILE_STATUS                     SUB(0x0B)
#define     GET_MERGER_STATUS                  		SUB(0x0C)
#define     ENCODE_STORAGE_PATH                     SUB(0x0D)
#define     ENCODE_FILE_NAME                        SUB(0x0E)
#define     AUDIO_AAC_MODE                          SUB(0x0F)
#define     AUDIO_AMR_MODE                          SUB(0x10)
#define     GET_RECORDING_TIME                      SUB(0x12)
#define     GET_RECORDING_FRAME                     SUB(0x13)
#define     VIDEO_BUFFER_THRESHOLD                  SUB(0x14)
#define		TYPE_P_FRAME_COUNT						SUB(0x16)
#define		ENCODE_SPEED_CTL						SUB(0x17)
#define HIF_CMD_MERGER_OPERATION      	(GRP_VID | FUNC(0x11))
#define HIF_CMD_MERGER_TAILSPEEDMODE    (GRP_VID | FUNC(0x12))

#define HIF_CMD_VIDRECD_PARAMETER       (GRP_VID | FUNC(0x18))
#define     ENCODE_RESOLUTION                       SUB(0x01)
#define     ENCODE_FORMAT                           SUB(0x02)
#define     ENCODE_QUALITY_CTL                      SUB(0x03)
#define     GET_VIDRECD_STATUS                  	SUB(0x04)
#define		REG_STORE_BUF_INFO						SUB(0x05)
#define		ENCODE_FRAME_RATE						SUB(0x06)
#define		CONFIG_DVS       						SUB(0x07)
#define HIF_CMD_VIDRECD_OPERATION       (GRP_VID | FUNC(0x19))
#define     VIDEO_RECORD_START                      SUB(0x01)
#define     VIDEO_RECORD_PAUSE                      SUB(0x02)
#define     VIDEO_RECORD_RESUME                     SUB(0x03)
#define     VIDEO_RECORD_STOP                       SUB(0x04)


#define HIF_CMD_MIDI_DEC_OP             (GRP_AUD | FUNC(0x00))
#define     DECODE_OP_PLAY_NOTE         			SUB(0x10)
#define     DECODE_OP_STOP_NOTE         			SUB(0x11)
#define     DECODE_OP_SET_PLAYTIME      			SUB(0x12)
#define     DECODE_OP_GET_PLAYTIME      			SUB(0x13)
#define HIF_CMD_AUDIO_EDIT_PARAM        (GRP_AUD | FUNC(0x01))
#define     AUDIO_EDIT_FILE_NAME            		SUB(0x01)
#define     AUDIO_EDIT_BUF		            		SUB(0x02)
#define     AUDIO_EDIT_POINT	            		SUB(0x03)
#define HIF_CMD_AUDIO_DEC_OP            (GRP_AUD | FUNC(0x02))
#define     DECODE_OP_START             			SUB(0x01)
#define     DECODE_OP_PAUSE             			SUB(0x02)
#define     DECODE_OP_RESUME            			SUB(0x03)
#define     DECODE_OP_STOP              			SUB(0x04)
#define     DECODE_OP_GET_FILEINFO      			SUB(0x05)
#define     DECODE_OP_PREDECODE         			SUB(0x06)
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
#define HIF_CMD_RA_DEC_OP             (GRP_AUD | FUNC(0x04))
#define     DECODE_OP_RA_SET_STARTTIME     	   SUB(0x11)
#define     DECODE_OP_GET_CURRENTPLAYTIME      SUB(0x12) 

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
#define		SET_OAEP_PARAMETER					    SUB(0x80)

#define HIF_CMD_SET_VOLUME              (GRP_AUD | FUNC(0x07))
#define     AUDIO_PLAY_VOLUME               		SUB(0x01)
#define     AUDIO_RECORD_VOLUME            			SUB(0x02)
#define HIF_CMD_WMA_DEC_OP              (GRP_AUD | FUNC(0x08))
#define     DECODE_OP_EXTRACT_TAG       			SUB(0x10)
#define     DECODE_OP_SET_STARTTIME     			SUB(0x11)
#define     DECODE_OP_GET_CURRENTPLAYTIME      		SUB(0x12) 
#define		DECODE_OP_GET_VERSION					SUB(0x13)

#define HIF_CMD_AUDIO_PLAY_POS          (GRP_AUD | FUNC(0x09))
#define     SET_PLAY_POS                			SUB(0x01)
#define     GET_PLAY_POS                			SUB(0x02)
#define     SET_PLAY_TIME               			SUB(0x03)
#define     GET_PLAY_TIME               			SUB(0x04)

#define HIF_CMD_AB_REPEAT_MODE          (GRP_AUD | FUNC(0x0A))
#define     ABREPEAT_MODE_ENABLE        			SUB(0x01)
#define     ABREPEAT_MODE_DISABLE       			SUB(0x02)
#define     ABREPEAT_SET_POINTA         			SUB(0x03)
#define     ABREPEAT_SET_POINTB        				SUB(0x04)
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
#define     AUDIO_ENCODE_ADC_PATH         			SUB(0x0A)
#define		AUDIO_ENCODE_LINEIN_CHANNEL				SUB(0x0B)

#define HIF_CMD_AUDIO_REC_STATUS		(GRP_AUD | FUNC(0x13))
#define HIF_CMD_AUDIO_AEC          		(GRP_AUD | FUNC(0x16))
#define HIF_CMD_AUDIO_DUPLEX_PATH   	(GRP_AUD | FUNC(0x17))

#define HIF_CMD_WAV_ENCODE              (GRP_AUD | FUNC(0x18))
#define     SET_WAV_ENCODE_ENABLE                   SUB(0x02)
#define     GET_WAV_INFO                            SUB(0x03)

#define HIF_CMD_DRM              (GRP_AUD | FUNC(0x19))
#define     SET_DRM_IV                        		SUB(0x01)
#define     SET_DRM_KEY                             SUB(0x02)
#define     SET_DRM_EN                              SUB(0x03)
#define     SET_DRM_HEADERLENGTH                    SUB(0x04)

#define HIF_CMD_AUDIO_BYPASS_PATH       (GRP_AUD | FUNC(0x1A))
#define HIF_CMD_AUDIO_DAC               (GRP_AUD | FUNC(0x1B))

#define HIF_CMD_USB_SETMODE             (GRP_USB | FUNC(0x00))
#define HIF_CMD_USB_SET_MSDCBUF         (GRP_USB | FUNC(0x01))
#define HIF_CMD_USB_SET_PCSYNCBUF       (GRP_USB | FUNC(0x02))
#define     SET_PCSYNC_IN_BUF                       SUB(0x01)
#define     SET_PCSYNC_OUT_BUF                      SUB(0x02)
#define     SET_PCSYNC_HANDSHAKE_BUF                SUB(0x03)
#define HIF_CMD_USB_SET_CONTROLBUF      (GRP_USB | FUNC(0x03))
#define HIF_CMD_USB_SET_PCCAMBUF        (GRP_USB | FUNC(0x04))
#define     SET_PCCAM_COMPRESS_BUF                  SUB(0x01)
#define     SET_PCCAM_LINE_BUF                      SUB(0x02)
#define HIF_CMD_USB_SET_MTPBUF          (GRP_USB | FUNC(0x05))
#define     SET_MTP_EP_BUF                          SUB(0x01)
#define HIF_CMD_DPS                     (GRP_USB | FUNC(0x06))
#define     DPS_START_JOB                           SUB(0x01)
#define     DPS_ABORT_JOB                           SUB(0x02)
#define     DPS_CONTINUE_JOB                        SUB(0x03)
#define     DPS_PRINTER_STATUS                      SUB(0x04)
#define     DPS_GETBUFADDR                          SUB(0x05)
#define     DPS_GETFILEID                           SUB(0x06)
#define HIF_CMD_PCCAM_CAPTURE           (GRP_USB | FUNC(0x07))
#define HIF_CMD_USB_SET_MEMDEV_BUF         (GRP_USB | FUNC(0x08))
#define HIF_CMD_MSDC_ACK                (GRP_USB | FUNC(0x09))
#define     HIF_WRITE_ACK                           SUB(0x01)
#define     HIF_READ_ACK                            SUB(0x02)

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
#define			FS_GET_FILE_LIST_DB					SUB(0x2D)
#define HIF_CMD_FS_SETADDRESS           (GRP_USR | FUNC(0x01))
#define         FS_SET_FILENAME_ADDRESS             SUB(0x01)
#define         FS_SET_SD_TMP_ADDR                  SUB(0x02)
#define         FS_SET_SM_MAP_ADDR                  SUB(0x04)
#define         FS_SET_FAT_GLOBALBUF_ADDR           SUB(0x06)
#define         FS_SET_DRM_IV_ADDR                  SUB(0x07)
#define HIF_CMD_FS_STORAGE              (GRP_USR | FUNC(0x02))
#define         FS_STORAGE_CONFIG                   SUB(0x02)
#define         FS_SM_ERASE                         SUB(0x03)
#define         FS_CHECK_CARDIN                     SUB(0x04)
#define         FS_IO_CTL                           SUB(0x05)

#if defined(BROGENT_FW)
#define HIF_CMD_SET_DIRECTION           (GRP_USR | FUNC(0x03))
#define     DIR_UP                                  SUB(0x00)
#define     DIR_DOWN                                SUB(0x01)
#define     DIR_LEFT                                SUB(0x02)
#define     DIR_RIGHT                               SUB(0x03)
#define     DIR_ENTER                               SUB(0x04)
#define     DIR_EXIT                                SUB(0x05)
#define HIF_CMD_SET_INDEX               (GRP_USR | FUNC(0x04))

#define HIF_CMD_GET_HOST_CMD            (GRP_USR | FUNC(0x05))
#define HIF_CMD_GET_HOST_ARGS           (GRP_USR | FUNC(0x06))
#define     TEST_ARG1                               SUB(0x00)
#define     TEST_ARG2                               SUB(0x01)
#define HIF_CMD_GET_HOST_STRING         (GRP_USR | FUNC(0x07))
#define HIF_CMD_DEMO_FUNC               (GRP_USR | FUNC(0x08))
#define     EX_DEMO0                                SUB(0x00)
#define     EX_DEMO1                                SUB(0x01)
#define     EX_DEMO2                                SUB(0x02)
#endif /* BROGENT_FW */

#if (APP_EN)
#define HIF_CMD_APP                     (GRP_USR | FUNC(0x09))
#define     SET_SDTC_BUF                            SUB(0x00)
#define     ENABLE_SDTC                             SUB(0x01)
#endif

// Host<->Firmware Status, reserve b[15:0] for command executing status
#define CPU_BOOT_FINISH                             (0x00000001)
#define SENSOR_CMD_IN_EXEC                          (0x00000002)
#define DSC_CMD_IN_EXEC                             (0x00000004)
#define SYSTEM_CMD_IN_EXEC                          (0x00000008)
#define AUDIO_CMD_IN_EXEC                           (0x00000010)
#define MGR3GP_CMD_IN_EXEC                          (0x00000020)
#define PLAYVID_CMD_IN_EXEC                         (0x00000040)
#define FS_CMD_IN_EXEC                              (0x00000080)
#define VIDRECD_CMD_IN_EXEC                         (0x00000100)
#define RENDER_CMD_IN_EXEC                          (0x00000100)	// for Brogent FW
#define VIDDEC_CMD_IN_EXEC	                        (0x00000200)
#define USB_CMD_IN_EXEC                             (0x00000400)
#define ESG_CMD_IN_EXEC                             (0x00000800)

#define MGR3GP_CANCEL_FILE_SAVE                     (0x00010000)
#define MEDIA_FILE_OVERFLOW                         (0x00020000)
#define DSC_CAPTURE_BEGIN                           (0x00040000)
#ifndef BUILD_CE
#define IMAGE_UNDER_ZOOM                            (0x01000000)
#define SD_CARD_NOT_EXIST                           (0x02000000)
#define FDTC_CMD_IN_EXEC                            (0x04000000)
#else
#define IMAGE_UNDER_ZOOM                            (0x00080000)
#define SD_CARD_NOT_EXIST                           (0x00100000)
#define FDTC_CMD_IN_EXEC                            (0x00200000)
#endif
#define IMAGE_UNDER_PANTILT                         (0x00400000)
#define APP_CMD_IN_EXEC                             (0x08000000)


// Event flag for HIF_CMD
#define SYS_FLAG_SYS                                (0x00000001)
#define SYS_FLAG_DSC                                (0x00000002)
#define SYS_FLAG_FS                                 (0x00000004)
#define SYS_FLAG_SENSOR                             (0x00000008)
#define SYS_FLAG_VIDPLAY                            (0x00000010)
#define SYS_FLAG_PSR3GP                             (0x00000020)
#define SYS_FLAG_VIDRECD                            (0x00000040)
#define SYS_FLAG_MGR3GP                             (0x00000080)
#define CMD_FLAG_MGR3GP                             (0x00000100)
#define SYS_FLAG_AUDIOPLAY                          (0x00000200)
#define SYS_FLAG_AUDIOREC                           (0x00000400)
#define SYS_FLAG_VID_DEC                            (0x00000800)
#define SYS_FLAG_USB                                (0x00001000)
#define SYS_FLAG_DEMUX                              (0x00002000)
#define SYS_FLAG_ROTDMA                             (0x00004000)
#define SYS_FLAG_AUDIOWRITE_FILE                    (0x00008000)
#define CMD_FLAG_VIDRECD							(0x00010000)
#define	SYS_FLAG_DEBUG								(0x00020000)
#define	SYS_FLAG_USB_EP1							(0x00040000)
#define SYS_FLAG_AUDIOREAD_FILE                     (0x00080000)
#define SYS_FLAG_USB_EP2                            (0x00100000)
#define SYS_FLAG_MEMDEV_ACK                         (0x00200000)
#define SYS_FLAG_USB_START_PREVIEW                  (0x00400000)
#define SYS_FLAG_USB_STOP_PREVIEW                   (0x00800000)
#define SYS_FLAG_ENC_FRM_0                          (0x01000000)
#define SYS_FLAG_ENC_FRM_1                          (0x04000000)
#define SYS_FLAG_SENSOR_LINE_NO                     (SYS_FLAG_DEBUG)//(0x00400000)
#define SYS_FLAG_FDTC_DRAWRECT                      (0x00800000)
#define SYS_FLAG_DETECT_SENSOR                      (0x00800000)
#define SYS_FLAG_ISP_AE_START						(0x02000000)
#define SYS_FLAG_MGR_0                              (0x08000000)
#define SYS_FLAG_LTASK								(0x10000000)
#define SYS_FLAG_ISP_AE_STOP						(0x20000000)

#define	SNAP_BUTTON_PRESSED		0x00000001
#define	FLIP_BUTTON_PRESSED		0x00000002
#define	IRHID_BUTTON_PRESSED	0x00000004


// For Rotary Encoder
#define ROTARY_SWITCH_CW        (0x00000001) // ClockWise
#define ROTARY_SWITCH_CCW       (0x00000002) // Counter ClockWise


typedef enum _MMPF_HIF_INT_FLAG
{
    MMPF_HIF_INT_VIDEOR_END     = 0x00000001,
    MMPF_HIF_INT_VIDEOP_END     = 0x00000002,
    MMPF_HIF_INT_AUDIOR_END     = 0x00000004,
    MMPF_HIF_INT_AUDIOP_END     = 0x00000008,
    MMPF_HIF_INT_AUDIOR_GETDATA = 0x00000010,
    MMPF_HIF_INT_AUDIOP_FILLBUF = 0x00000020,
    MMPF_HIF_INT_VIDEOR_MOV     = 0x00000040,
    MMPF_HIF_INT_VIDEOP_MOV     = 0x00000080,
    MMPF_HIF_INT_FDTC_END       = 0x00000100,
    MMPF_HIF_INT_SDCARD_REMOVE  = 0x00000200,
    MMPF_HIF_INT_MDTV_ESG_DONE  = 0x00000400,
    MMPF_HIF_INT_MDTV_BUF_OK    = 0x00000800,
    MMPF_HIF_INT_SBC_EMPTY_BUF  = 0x00001000,
    MMPF_HIF_INT_WAV_EMPTY_BUF  = 0x00002000,
    MMPF_HIF_INT_USB_SUSPEND    = 0x00004000,
    MMPF_HIF_INT_USB_HOST_DEV   = 0x00008000,
    MMPF_HIF_INT_SMILE_SHOT     = 0x00010000
} MMPF_HIF_INT_FLAG;


typedef enum _MMPF_SYSTEM_APMODE
{
    MMPF_SYSTEM_APMODE_NULL = 0, /**<Null parameter. It's used to reset the FirmwareHandler state
                                     machine to download the firmware again.*/
    MMPF_SYSTEM_APMODE_BOOT = 1, ///<for DIAMOND change PLL
    MMPF_SYSTEM_APMODE_USB, ///< USB storage
    MMPF_SYSTEM_APMODE_DSC_MP3, ///< DSC with MP3 playback. DSC_MP3_P_FW
    MMPF_SYSTEM_APMODE_MAX /**< It could be used to debug different versions of system mode definition.*/
} MMPF_SYSTEM_APMODE;


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================


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
MMP_USHORT    MMPF_HIF_GetCmd(void);
void    MMPF_HIF_ClearCmd(void);
void    MMPF_HIF_SetCmdStatus(MMP_ULONG status);
void    MMPF_HIF_ClearCmdStatus(MMP_ULONG status);
MMP_ULONG   MMPF_HIF_GetCmdStatus(MMP_ULONG status);

void    MMPF_HIF_CmdSetParameterB(MMP_UBYTE ubParamnum, MMP_UBYTE ubParamdata);
void    MMPF_HIF_CmdSetParameterW(MMP_UBYTE ubParamnum, MMP_USHORT usParamdata);
void    MMPF_HIF_CmdSetParameterL(MMP_UBYTE ubParamnum, MMP_ULONG ulParamdata);
MMP_UBYTE    MMPF_HIF_CmdGetParameterB(MMP_UBYTE ubParamnum);
MMP_USHORT   MMPF_HIF_CmdGetParameterW(MMP_UBYTE ubParamnum);
MMP_ULONG    MMPF_HIF_CmdGetParameterL(MMP_UBYTE ubParamnum);

void    MMPF_HIF_SetCpu2HostIntFlag(MMP_BOOL bEnable, MMPF_HIF_INT_FLAG flag);
void    MMPF_HIF_SetCpu2HostInt(MMPF_HIF_INT_FLAG status);
void    MMPF_HIF_ClearCpu2HostIntFlag(MMP_ULONG ulClearFlag);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
/// @end_ait_only

#endif // _MMPF_HIF_H_
