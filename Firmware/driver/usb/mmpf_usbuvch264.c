#include "mmpf_usbuvch264.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbvend.h"
#include "pcam_msg.h"
#include "mmpf_mp4venc.h"
#include "mmpf_h264enc.h"
#include "lib_retina.h"
#include "mmps_3gprecd.h"
#include "mmpf_sensor.h"

#define FIX_SLICE_BITS_MODE_INACCURACY  (0)
#define SLICE_BITS_MODE_MIN_QP_ITEM     (0) // choose min-QP = 28

extern MMPF_SENSOR_FUNCTION *gsSensorFunction;

#define  MB_MAX_POWER   (244800)
MMP_USHORT UVCX_SetH264ColorRangeByStream(void) ;

//H264RateControlMode     gbUVCXCtlMode = CBR_H264 ;
UVCX_RateControlMode    gsUVCXCtlModeDef = { 0, CBR_H264        } ;
UVCX_RateControlMode    gsUVCXCtlModeMin = { 0, CBR_H264        } ;
UVCX_RateControlMode    gsUVCXCtlModeMax = { 0, CONSTANT_H264_QP} ;
UVCX_RateControlMode    gsUVCXCtlModeCur = { 0, CBR_H264        } ;

UVCX_TemporalScaleMode  gsUVCXTemporalScaleMode = { 0,ONE_LAYER_H264};
UVCX_SpatialScaleMode   gsUVCXSpatialScaleMode  = { 0,ONE_SPATIAL_LAYER};
UVCX_SNRScaleMode       gsUVCXSNRScaleMode  = {0,0,0};//RESERVED_LAYER ;
MMP_USHORT              gsUVCXVersion = 0x0100 ;
UVCX_FrameRateConfig    gsUVCXFrameRate ={ 0, 0x51615 };
UVCX_BitRateLayers      gsUVCXBitRate ;
UVCX_QPStepsLayers      gsUVCXQPStepSize ;
UVCX_PictureTypeControl gsUVCXPicTypeCtl ={ 0, PIC_IDR_FRAME } ;
UVCX_CropConfig         gsUVCXCropConfig ;
UVCX_AdvConfig          gsUVCXAdvConfig = {0,MB_MAX_POWER,40,0} ;
UVCX_LTRBufferSizeControl gsUVCXLtrBufferSizeCtl = { 0,0,0} ;
UVCX_LTRPictureControl    gsUVCXLtrPictCtl = { 0,0,0} ;
MMP_USHORT              gsUVCXEncoderResetId = 0 ;
MMP_BOOL                gbSimulcastH264 = 0 ;
// 3.3.3 UVCX_RATE_CONTROL_MODE
VC_CMD_CFG UVCX_RATE_CONTROL_MODE_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_RATECONTROLMODE_LEN,1,
    1,(MMP_ULONG)&gsUVCXCtlModeDef,(MMP_ULONG)&gsUVCXCtlModeMin,(MMP_ULONG)&gsUVCXCtlModeMax,(MMP_ULONG)&gsUVCXCtlModeCur
} ; 

// 3.3.4 UVCX_TEMPORAL_SCALE_MODE
VC_CMD_CFG UVCX_TEMPORAL_SCALE_MODE_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_TEMPORALSCALEMODE_LEN,1,
    1,(MMP_ULONG)&gsUVCXTemporalScaleMode,(MMP_ULONG)&gsUVCXTemporalScaleMode,(MMP_ULONG)&gsUVCXTemporalScaleMode,(MMP_ULONG)&gsUVCXTemporalScaleMode

} ;

// 3.3.5 UVCX_SPATIAL_SCALE_MODE
VC_CMD_CFG UVCX_SPATIAL_SCALE_MODE_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_SPATIALSCALEMODE_LEN,1,
    1,(MMP_ULONG)&gsUVCXSpatialScaleMode,(MMP_ULONG)&gsUVCXSpatialScaleMode,(MMP_ULONG)&gsUVCXSpatialScaleMode,(MMP_ULONG)&gsUVCXSpatialScaleMode

} ;

// 3.3.6 UVCX_SNR_SCALE_MODE
VC_CMD_CFG UVCX_SNR_SCALE_MODE_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_SNRSCALEMODE_LEN,1,
    1,(MMP_ULONG)&gsUVCXSNRScaleMode,(MMP_ULONG)&gsUVCXSNRScaleMode,(MMP_ULONG)&gsUVCXSNRScaleMode,(MMP_ULONG)&gsUVCXSNRScaleMode
} ;

// 3.3.7 UVCX_LTR_BUFFER_SIZE_CONTROL
VC_CMD_CFG UVCX_LTR_BUFFER_SIZE_CONTROL_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_LTRBUFFERSIZECONTROL_LEN,1,
    1,(MMP_ULONG)&gsUVCXLtrBufferSizeCtl,(MMP_ULONG)&gsUVCXLtrBufferSizeCtl,(MMP_ULONG)&gsUVCXLtrBufferSizeCtl,(MMP_ULONG)&gsUVCXLtrBufferSizeCtl
} ;

// 3.3.8 UVCX_LTR_PICTURE_CONTROL
VC_CMD_CFG UVCX_LTR_PICTURE_CONTROL_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_LTRPICTURECONTROL_LEN,1,
    1,(MMP_ULONG)&gsUVCXLtrPictCtl,(MMP_ULONG)&gsUVCXLtrPictCtl,(MMP_ULONG)&gsUVCXLtrPictCtl,(MMP_ULONG)&gsUVCXLtrPictCtl
} ;

// 3.3.9 UVCX_PICTURE_TYPE_CONTROL
VC_CMD_CFG UVCX_PICTURE_TYPE_CONTROL_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_PICTYPE_LEN,1,
    1,(MMP_ULONG)&gsUVCXPicTypeCtl,(MMP_ULONG)&gsUVCXPicTypeCtl,(MMP_ULONG)&gsUVCXPicTypeCtl,(MMP_ULONG)&gsUVCXPicTypeCtl
};

// 3.3.10 UVCX_VERSION
VC_CMD_CFG UVCX_VERSION_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    2,1,
    1,0x0100,0x0100,0x0100,0x0100
};

// 3.3.11 UVCX_ENCODER_RESET
VC_CMD_CFG UVCX_ENCODER_RESET_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    2,1,
    1,0,0,0,0
} ;

// 3.3.12 UVCX_FRAME_RATE_CONFIG
VC_CMD_CFG UVCX_FRAMERATE_CONFIG_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_FRAMERATE_LEN,1,
    1,(MMP_ULONG)&gsUVCXFrameRate,(MMP_ULONG)&gsUVCXFrameRate,(MMP_ULONG)&gsUVCXFrameRate,(MMP_ULONG)&gsUVCXFrameRate
};


// 3.3.13 UVCX_VIDEO_ADVANCE_CONFIG
VC_CMD_CFG UVCX_VIDEO_ADVANCE_CONFIG_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_ADVCONFIG_LEN,1,
    1,(MMP_ULONG)&gsUVCXAdvConfig,(MMP_ULONG)&gsUVCXAdvConfig,(MMP_ULONG)&gsUVCXAdvConfig,(MMP_ULONG)&gsUVCXAdvConfig
} ;

// 3.3.14 UVCX_BITRATE_LAYERS
VC_CMD_CFG UVCX_BITRATE_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_BITRATE_LEN,1,
    1,(MMP_ULONG)&gsUVCXBitRate,(MMP_ULONG)&gsUVCXBitRate,(MMP_ULONG)&gsUVCXBitRate,(MMP_ULONG)&gsUVCXBitRate
} ;

// 3.3.15 UVCX_QP_STEPS_LAYERS
VC_CMD_CFG UVCX_QP_STEPS_LAYERS_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD| CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_QPSTEP_LAYER_LEN,1,
    1,(MMP_ULONG)&gsUVCXQPStepSize,(MMP_ULONG)&gsUVCXQPStepSize,(MMP_ULONG)&gsUVCXQPStepSize,(MMP_ULONG)&gsUVCXQPStepSize
} ;




// <! V1.0
VC_CMD_CFG UVCX_CROP_CONFIG_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_LEN_CMD ), 
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    UVCX_CROP_CONFIG_LEN,1,
    1,(MMP_ULONG)&gsUVCXCropConfig,0,0,(MMP_ULONG)&gsUVCXCropConfig
};


SLICE_BITS_MODE_CFG gsSliceBitsModeCfg[] =
{
    { 10, 130/*140*//*500*/ },
    { 20, 125/*400*/ },
    { 22, 125/*350*/ },
    { 24, 125/*280*/ },
    { 26, 125/*250*/ }, //
    { 28, 125/*230 */},
    { 30, 125/*165*/ },
    { 32, 135 },
    { 34,  90 },
} ;



// This structure value is for test only
UVCX_VIDEO_CONFIG gsCurH264Config[MAX_NUM_ENC_SET] =
{
// Single H264 or Simulcast stream 0
    {
        (333333),                   // Framerate : Change frame rate test ok
        #if (STRICT_AVG_FRAME_SIZE == 1)
        1400000,              // Bitrate : Change bitrate test ok
        #else
        4*1000*1000,              // Bitrate : Change bitrate test ok
        #endif
        (CUR_HINT_BITS),            // bmHints
        1,                          // wConfigurationIndex : NA		 
        1280,                       // wWidth
        720,                        // wHeight 
        4,                      // wSliceUnits
        MULTI_SLICE_NUMs,//MULTI_SLICE_NUMs,           // wSliceMode
        CONSTRAINED_BASELINE_P,//HIGH_P ,    // wProfile
        10*1000,//10*1000,//100,              // wIFramePeriod
        100,                        // wEstimateVideoDelay;	
        100,                        // wEstimatedMaxConfigDelay
        REALTIME_H264,              // bUsageType
        CBR_H264,                   // bRateControlMode
        #if (STRICT_AVG_FRAME_SIZE == 1)
        ONE_LAYER_H264, 
        #else
        ONE_LAYER_H264,             // bTemporalScaleMode
        #endif
        ONE_SPATIAL_LAYER,          // bSpatialScaleMode
        RESERVED_LAYER,             // bSNRScaleMode
        MUX_AUX_STREAM(MUX_AUX_H264), // bStreamMuxOption
        ANNEXB_H264,                // bStreamFormat
        CAVLC_H264,                 // bEntropyCABAC;
        0,                          // bTimestamp
        0,                          // bNumOfReorderFrames
        0,                          // bPreviewFlipped
        0,                          // bView;
        0,                          // bReserved1
        0,                          // bReserved2
        0,                          // bStreamID
        0,                          // bSpatialLayerRatio
        1000                        // wLeakyBucketSize 
    },
// Simulcast stream 1    
    {
        (333333),                   // Framerate : Change frame rate test ok
        1 * 1024 * 1024,              // Bitrate : Change bitrate test ok
        (CUR_HINT_BITS),            // bmHints
        1,                          // wConfigurationIndex : NA		 
        640,                       // wWidth
        360,                        // wHeight 
        4,//0,                      // wSliceUnits
        MULTI_SLICE_NUMs,           // wSliceMode
        CONSTRAINED_BASELINE_P ,    // wProfile
        10*1000,//100,              // wIFramePeriod
        100,                        // wEstimateVideoDelay;	
        100,                        // wEstimatedMaxConfigDelay
        REALTIME_H264,              // bUsageType
        CBR_H264,                   // bRateControlMode
        ONE_LAYER_H264,             // bTemporalScaleMode
        ONE_SPATIAL_LAYER,          // bSpatialScaleMode
        RESERVED_LAYER,             // bSNRScaleMode
        MUX_AUX_STREAM(MUX_AUX_H264), // bStreamMuxOption
        ANNEXB_H264,                // bStreamFormat
        CAVLC_H264,                 // bEntropyCABAC;
        1,                          // bTimestamp
        0,                          // bNumOfReorderFrames
        0,                          // bPreviewFlipped
        0,                          // bView;
        0,                          // bReserved1
        0,                          // bReserved2
        1,                          // bStreamID
        0,                          // bSpatialLayerRatio
        500,                        // wLeakyBucketSize 
    }
} ;


void UVCX_ConfigH264BufferBySliceMode(UVCX_VIDEO_CONFIG *config)
{
    int i;
    STREAM_CFG *cur_stream = usb_get_cur_image_pipe_by_epid(STREAM_EP_H264) ;
    STREAM_CTL *cur_bctl = &cur_stream->pipe_b[PIPE_1] ;
    MMP_USHORT slice_bytes = config->wSliceUnits / 8 ;
    MMP_ULONG  buf_size = cur_bctl->slot_size * cur_bctl->slot_num  ;
    if(config->wSliceMode!=MULTI_SLICE_BITs) {
        return ;
    }
    if( !UVCX_IsSliceBasedTxStream(cur_stream) ) {
        return;
    }
    cur_bctl->slot_size = ( slice_bytes + FRAME_PAYLOAD_HEADER_SZ + 31) & (-32) ;
    cur_bctl->slot_num = buf_size / cur_bctl->slot_size ;
    dbg_printf(3,"--SliceBitsMode,slot#:%d,slot sz:%d\r\n",cur_bctl->slot_num,cur_bctl->slot_size );
    if(cur_bctl->buf_addr) {
        FRAME_PAYLOAD_HEADER *ph ;
        for(i=0;i<cur_bctl->slot_num;i++) {
            ph = (FRAME_PAYLOAD_HEADER *)(cur_bctl->buf_addr + i * cur_bctl->slot_size + cur_bctl->off_header );
            ph->UVC_H264_PH.dwPayloadSize = 0; 
        }
    }
}


MMP_USHORT UVCX_GetInaccurayBytesByMinQP(MMP_UBYTE minQP)
{
    MMP_USHORT i, c = sizeof(gsSliceBitsModeCfg) / sizeof(SLICE_BITS_MODE_CFG) ;
    for(i=0;i<c;i++) {
        if(minQP<=gsSliceBitsModeCfg[i].minQP) {
            return gsSliceBitsModeCfg[i].inaccuracyBytes ;
        }
    }
    return 0;
}

void UVCX_SetH264Profile(MMP_USHORT streamid,MMP_USHORT profile)
{
    gsCurH264Config[streamid].wProfile = profile ;
    if(profile==CONSTRAINED_BASELINE_P) {
        dbg_printf(3,"--constrained_baseline\r\n");
    }
    if(profile==HIGH_P) {
        dbg_printf(3,"--high\r\n");
    }
    if(profile==MAIN_P) {
        dbg_printf(3,"--main\r\n");
    }
}   

MMP_BOOL UVCX_IsSimulcastH264(void)
{

#if H264_SIMULCAST_EN
    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe_by_epid(STREAM_EP_H264);
    MMP_UBYTE pipe1_cfg ;
    pipe1_cfg =  CUR_PIPE_CFG(PIPE_1,cur_pipe->pipe_cfg); 
    
    if( ( PIPE_EN(cur_pipe->pipe_en) & PIPE1_EN) &&( pipe1_cfg==PIPE_CFG_H264) ) {
        if(cur_pipe->pipe_en&PIPE_EN_SIMULCAST) {
            return gbSimulcastH264; 
        }
    }
    return  0;
#else
    gbSimulcastH264 = 0 ;
    return gbSimulcastH264 ;
#endif
}

// Shoud be called after probe / commit and before start preview
void UVCX_SetSimulcastH264(MMP_BOOL en) 
{
    gbSimulcastH264 = en ;
    dbg_printf(3,"-gbSimulcastH264 : %d\r\n",gbSimulcastH264);
    /* no--need
    if(gbSimulcastH264) {
        UVCX_SetH264EncodeBufferMode(FRAME_MODE) ;
    } else {
        UVCX_SetH264EncodeBufferMode(REALTIME_MODE) ;
    }
    */
}


MMP_USHORT UVCX_SetH264ColorRangeByStream(void)
{
    MMP_ULONG full_range_on = 0 ;
    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(PIPE_1);
    MMP_UBYTE pipe0_cfg , pipe1_cfg , cr_id = 0  ;
    pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_pipe->pipe_cfg);//(cur_pipe->pipe_cfg & PIPE_CFG_MASK) ;
    pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_pipe->pipe_cfg);//( (cur_pipe->pipe_cfg >> 4) & PIPE_CFG_MASK) ;
    
    if( IS_PIPE01_EN(cur_pipe->pipe_en) /*(cur_pipe->pipe_en&PIPE_EN_MASK)!=(PIPE0_EN|PIPE1_EN) */) {
    // Single H264 , full_range_on = 0
        full_range_on = 0 ;   
    } else {
        // YUY2 + H264
        if( pipe0_cfg==PIPE_CFG_YUY2 ) {
            full_range_on = 0 ;
        }
        // MJPEG + H264
        else if( pipe0_cfg==PIPE_CFG_MJPEG) {
            full_range_on = 1 ;
        }
    }
        
    //MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_VIDEO_FULL_RANGE, (void*)full_range_on);
    return full_range_on ;
}

//
// After commit , need call this function to update 
// new config into gsCurH264Config
//
void UVCX_CommitH264Config(MMP_BOOL simulcast,void *config_commit)
{
    UVCX_VIDEO_CONFIG *streamcfg = (UVCX_VIDEO_CONFIG *)config_commit ;
    if(!streamcfg) {
        return ;
    }
    if(streamcfg->bStreamID > bMaxStreamId) {
        return  ;
    }
    UVCX_SetSimulcastH264(simulcast);   
    gsCurH264Config[streamcfg->bStreamID] = *(UVCX_VIDEO_CONFIG *)config_commit ;
}

UVCX_VIDEO_CONFIG *UVCX_GetH264StartConfig(MMP_USHORT streamid)
{
    if(streamid > bMaxStreamId) {
        return (UVCX_VIDEO_CONFIG *)0 ;
    }   
    return (UVCX_VIDEO_CONFIG *)&gsCurH264Config[streamid] ;
}


MMP_USHORT UVCX_SetH264Resolution(MMP_USHORT stream0_res_indx,MMP_USHORT stream1_res_indx,MMP_USHORT resnum )
{
    RES_TYPE_CFG *rescfg;
    MMPF_VIDENC_CROPPING crop_offset0 = { 0,0,0,0 };
    MMPF_VIDENC_CROPPING crop_offset1 = { 0,0,0,0 };
    MMP_USHORT resol_width[bMaxStreamId+1];
    MMP_USHORT resol_height[bMaxStreamId+1], dw, dh;

    if (resnum > (bMaxStreamId+1)) {
        return PCAM_USB_INIT_ERR ;
    }

// Set Crop info
// Sean : 2012_02_06, rewrite for UVC 15 EU_VIDEO_RESOLUTION_CONTROL
// The width & height could be set as multiple of 2
//    vidConfig = MMPS_3GPRECD_GetConfiguration();
    
    rescfg          = GetResCfg(stream0_res_indx) ;
    resol_width[0]  = ( rescfg->res_w + 15 ) & (-16) ;
    resol_height[0] = ( rescfg->res_h + 15 ) & (-16) ;

    dw = ( resol_width[0] - rescfg->res_w );
    dh = ( resol_height[0] - rescfg->res_h );
    if ((dw & 1) || (dh & 1)) {
        //not support odd number cropping
        return PCAM_USB_INIT_ERR ;
    }
#if 0
    crop_offset0.usRight    = (dw)? ((dw >> 1) + (dw & 2)): 0; //plus 1 if div2 is odd number
    crop_offset0.usLeft     = dw - crop_offset0.usLeft;

    crop_offset0.usBottom   = (dh)? ((dh >> 1) + (dh & 2)): 0;
    crop_offset0.usTop      = dh - crop_offset0.usBottom;
#else
    crop_offset0.usRight    = dw ;//plus 1 if div2 is odd number
    crop_offset0.usLeft     = 0;

    crop_offset0.usBottom   = dh;
    crop_offset0.usTop      = 0;

#endif
    dbg_printf(3,"#stream0 crop(%d,%d,%d,%d)\r\n",crop_offset0.usTop,crop_offset0.usBottom,crop_offset0.usLeft,crop_offset0.usRight);

    if(resnum==2/*UVCX_IsSimulcastH264()*/) {
        rescfg = GetResCfg(stream1_res_indx) ;
        resol_width[1] = ( rescfg->res_w + 15 ) & (-16) ;
        resol_height[1] = ( rescfg->res_h + 15 ) & (-16) ;

        dw = ( resol_width[1] - rescfg->res_w );
        dh = ( resol_height[1] - rescfg->res_h );
        if ((dw & 1) || (dh & 1)) {
            //not support odd number cropping
            return PCAM_USB_INIT_ERR ;
        }
#if 0        
        crop_offset1.usRight    = (dw)? ((dw >> 1) + (dw & 2)): 0; //plus 1 if div2 is odd number
        crop_offset1.usLeft     = dw - crop_offset1.usLeft;

        crop_offset1.usBottom   = (dh)? ((dh >> 1) + (dh & 2)): 0;
        crop_offset1.usTop      = dh - crop_offset1.usBottom;
#else
        crop_offset1.usRight    = dw; //plus 1 if div2 is odd number
        crop_offset1.usLeft     = 0;

        crop_offset1.usBottom   = dh;
        crop_offset1.usTop      = 0;

#endif

        dbg_printf(3,"#stream1 crop(%d,%d,%d,%d)\r\n",crop_offset1.usTop,crop_offset1.usBottom,crop_offset1.usLeft,crop_offset1.usRight);
    }

    MMPS_3GPRECD_SetResolution(resol_width, resol_height, resnum);

    MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_CROPPING, (void*)&crop_offset0);

    if (UVCX_IsSimulcastH264()) {
        MMPF_VIDENC_SetParameter(1, MMPF_VIDENC_ATTRIBUTE_CROPPING, (void*)&crop_offset1);
    }

    return PCAM_ERROR_NONE;
}

//
// Startup setting called by PCAM_USB_PreviewStart()
//
MMP_USHORT UVCX_SetH264StartMode(UVCX_VIDEO_CONFIG *config)
{
    MMPF_VIDENC_QP_CTL      qc;
    MMPF_VIDENC_BITRATE_CTL BrCtl;
    MMPF_VIDENC_SLICE_CTL   SliceCtl;
    MMPF_VIDENC_RC_MODE_CTL RcModeCtl;
    MMP_UBYTE               i, total_layers = ((config->bTemporalScaleMode == TWO_LAYER_H264)? 2: 1);
    MMP_USHORT              layerId ,profile_flag;
    
    UVCX_FrameRateConfig    fr ;
    
    dbg_printf(3,"#UVC H264 v1.0 Stream Id :%d\r\n",config->bStreamID);
    dbg_printf(3,"#w : %d, h : %d\r\n",config->wWidth,config->wHeight);
    if(!config) {
        return PCAM_USB_INIT_ERR ;
    }
    layerId =  LAYER_ID(config->bStreamID,0,0,config->bTemporalScaleMode) ; 
    fr.wLayerID = layerId ;
    
    fr.dwFrameInterval = config->dwFrameInterval ; 
    // 1. Convert H264 frame rate( from 100ns interval to FPS)
    UVCX_SetH264FrameRate( &fr );
    // 2. Set the initial bitrate
    //gbVideoEncQualityCustom = TRUE ;
    BrCtl.ubLayerBitMap = 0;
    for (i = 0; i < total_layers; i++) {
        BrCtl.ubLayerBitMap |= (1 << i);
        BrCtl.ulBitrate[i]  = config->dwBitRate / total_layers;
        dbg_printf(3,"[BitRate] : Layer : %d,Br:%d\r\n",i,BrCtl.ulBitrate[i]);
    }
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_BR, (void*)&BrCtl);

    // set init QP !
    qc.ubTID = TEMPORAL_ID_MASK;
    qc.ubQP[0] = 35;
    qc.ubQP[1] = 35;
    qc.ubQP[2] = 35;
    qc.CbrQpIdxOffset[0] = qc.CbrQpIdxOffset[1] = qc.CbrQpIdxOffset[2] = 0;
    qc.CrQpIdxOffset[0] = qc.CrQpIdxOffset[1] = qc.CrQpIdxOffset[2] = 0; //only valid in High Profile
    qc.ubTypeBitMap = 1 | (1 << 1) | (1 << 2); //I/P/B frames init qp
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_FRM_QP, &qc);

    // 3. Slice mode
    if ((config->wSliceUnits==0)||(config->wSliceMode == SINGLE_SLICE)) { // Single slice per frame
        SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_FRM;
        SliceCtl.ulSliceSize = 0;
        MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
    }
    else {
        MMP_BOOL slice_nr_ng = MMP_TRUE;
        // Slice Row mode
        if ((config->wSliceMode == MULTI_SLICE_NUMs)) {
            MMP_USHORT slice_row, mb_height;
            mb_height = (config->wHeight + 15)>>4;
            if (mb_height) {
                slice_row = (mb_height+config->wSliceUnits-1)/config->wSliceUnits;
                if (slice_row) {
                    MMP_USHORT slice_nr,mb_w;
                    slice_nr = mb_height/slice_row + ((mb_height%slice_row)?1:0);
                    mb_w = (config->wWidth + 15) >> 4 ;
                    SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_MB ;
                    SliceCtl.ulSliceSize = mb_w * slice_row ;
                    MMPF_VIDENC_SetParameter(config->bStreamID,MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
                    slice_nr_ng = MMP_FALSE;
                }
            }
        }
        else if ((config->wSliceMode == MULTI_SLICE_MBs)) {
            MMP_USHORT max_mbs = config->wSliceUnits ;
            if(max_mbs) {
                SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_MB ;
                SliceCtl.ulSliceSize = max_mbs;   
                MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
            }
        }
        else if ((config->wSliceMode == MULTI_SLICE_BITs)) {

            MMP_USHORT max_bytes = config->wSliceUnits / 8  ;
            
#if FIX_SLICE_BITS_MODE_INACCURACY==1
            UVCX_QPStepsLayers qp_step ;
            
            qp_step.wLayerID = 0 ;
            qp_step.bFrameType = (1<<1) | (1 << 0);
            qp_step.bMinQp = gsSliceBitsModeCfg[SLICE_BITS_MODE_MIN_QP_ITEM].minQP ;
            qp_step.bMaxQp = H264E_MAX_MB_QP ; // use default max QP
            UVCX_SetH264QPStepsLayers(&qp_step);
#else            
            if(max_bytes) {
                SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_BYTE ;
                SliceCtl.ulSliceSize = max_bytes;   
                MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
            
            }
#endif

            UVCX_ConfigH264BufferBySliceMode(config);
            slice_nr_ng = MMP_FALSE ;

        }
        if (slice_nr_ng == MMP_TRUE) {
            dbg_printf(0,"#Invalid H264 Slice Mode, Unit:%d,%d\r\n",
                config->wSliceMode, config->wSliceUnits);
            return PCAM_USB_INIT_ERR ;
        }
    }
    // 4. Profile
    if(config->wProfile==CONSTRAINED_BASELINE_P) {
        profile_flag = CONSTRAINED_BASELINE_FLAG1 ;
    } 
    else if(config->wProfile==BASELINE_P) {
        profile_flag = BASELINE_P ;
    } 
    else if(config->wProfile==HIGH_P) {
        profile_flag = HIGH_P ;
    } 
    else if(config->wProfile==MAIN_P) {
        profile_flag = MAIN_P ;
    }
    else {
        dbg_printf(0,"#Invalid H264 Profile:%d\r\n",config->wProfile);
        return PCAM_USB_INIT_ERR ;
    }
    
    profile_flag = ( (profile_flag>>8) & 0x00FF) | ( (profile_flag & 0x00FF) << 8 );
    dbg_printf(3,"#Profile flag : %x\r\n",profile_flag);
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_PROFILE, (void*)profile_flag);
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_LEVEL, (void*)gsUVCXAdvConfig.bLevel_idc);
    
    dbg_printf(3," H264 Profile:%x,entroyCABAC:%d\r\n",config->wProfile,config->bEntropyCABAC);
    
    if (config->bTemporalScaleMode == TWO_LAYER_H264) {
        if (config->wProfile==HIGH_P) {
            UVCX_TemporalScaleMode tempScaleMode ;
            tempScaleMode.wLayerID = layerId ;
            tempScaleMode.bTemporalScaleMode = TWO_LAYER_H264 ;
            UVCX_SetH264TemporalScaleMode(&tempScaleMode );
            //MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_TSVC, (void*)MMP_TRUE);
            MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_LAYERS, (void*)2);
        }
        else {
            dbg_printf(0,"#Invalid Only High Profile support 2 layer:%d\r\n", config->wProfile);
        }
    } else {
        MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_LAYERS, (void*)1);
    }
    // I Frame Period
    if(config->wIFramePeriod==0) {
        MMPF_VIDENC_GOP_CTL GopCtl;

        GopCtl.usGopSize = 0;
        GopCtl.usMaxContBFrameNum = 0;
        GopCtl.SyncFrameType = MMPF_VIDENC_SYNCFRAME_IDR;
        MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_GOP_CTL, (void*)&GopCtl);
    } else {
       // MMP_ULONG period_ms = config->wIFramePeriod * 100 ;
        MMP_ULONG period_ms = config->wIFramePeriod ; // V1.0 unit is milliseconds
        MMP_ULONG64 p_frame_c ;
        MMPF_VIDENC_MAX_FPS_CTL FpsCtl = {3000, 100};
        MMPF_VIDENC_GOP_CTL GopCtl;

        MMPF_VIDENC_GetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_MAX_FPS, &FpsCtl);

        p_frame_c = (MMP_ULONG64)((MMP_ULONG64)FpsCtl.ulMaxFpsResolution*period_ms)/(FpsCtl.ulMaxFpsIncreament*1000);

        dbg_printf(3,"#P-Frame #:%d\r\n", (MMP_USHORT)p_frame_c);

        GopCtl.usGopSize = (MMP_USHORT)p_frame_c;
        GopCtl.usMaxContBFrameNum = 0;
        GopCtl.SyncFrameType = MMPF_VIDENC_SYNCFRAME_IDR;
        MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_GOP_CTL, (void*)&GopCtl);
    }
    // TBD : Control Mode , CBR,VBR,Constant QP
    MMPF_VIDENC_SetParameter(config->bStreamID,MMPF_VIDENC_ATTRIBUTE_CBR_PADDING,(void *)MMP_FALSE) ;
    RcModeCtl.bLayerGlobalRc = MMP_FALSE;
    if(config->bRateControlMode == CBR_H264) {
       // dbg_printf(3,"#CBR\r\n");
       RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
 #if 0//H264_CBR_PADDING_EN==1
        MMPF_VIDENC_SetParameter(config->bStreamID,MMPF_VIDENC_ENABLE_CBR_PADDING,(void *)MMP_TRUE) ;
        usb_vs_zero_cbr_padding_buffer();

 #endif      
    } 
    else if(config->bRateControlMode == VBR_H264) {
       // dbg_printf(3,"#VBR\r\n");
       RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_VBR;
    } 
    else if(config->bRateControlMode == CONSTANT_H264_QP) {
       // dbg_printf(3,"#Constant QP\r\n");
       RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CQP;
    }
    else {
        dbg_printf(0,"#Invalid H264 Control Mode:%d\r\n",config->bRateControlMode );
    }
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_RC_MODE, (void*)&RcModeCtl);
    
    // TBD : Stream Format
    if(config->bStreamFormat==ANNEXB_H264) {
    } else {
        dbg_printf(0,"#Invalid H264 Bitstream Format\r\n");
    }
    // TBD : EntropyCABAC
    if(config->bEntropyCABAC) {
        
    } else {
    
    }
    // TBD :SEI timestamp
    if(config->bTimestamp) {
        MMPF_H264ENC_SetSeiEnable(config->bStreamID, MMP_TRUE);
    } else {
        MMPF_H264ENC_SetSeiEnable(config->bStreamID, MMP_FALSE);
    }
    // 
    if (config->wLeakyBucketSize) {
        MMPF_VIDENC_LEAKYBUCKET_CTL lb;
        lb.ubLayerBitMap = TEMPORAL_ID_MASK;
        lb.ulLeakyBucket[0] = config->wLeakyBucketSize;
        lb.ulLeakyBucket[1] = config->wLeakyBucketSize;
        MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_LB_SIZE, (void*)&lb);
    }
    #if 1 // Disalbe drop frame when storage / usage type
    if ( (config->bUsageType == STORAGE_H264)||(config->bUsageType == BROADCAST_H264)) {
        MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_RC_SKIPPABLE, (void*)MMP_FALSE);
    }
    #endif
    // Implement Flip by Sensor setting
    UVCX_SetH264FlipMode(config->bPreviewFlipped);

    #if 0 //test sample code for adjust max i/b weight, p is always 1000
    {
    MMPF_VIDENC_PIC_WEIGHT_CTL wctl = {
	    0,      // single layer 0
	    0x01,   // bit 0 means I frame weight
	    1864,   // weight value, resolution 1000
	    0       // reserve for B frame weight
	};
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_PIC_MAX_WEIGHT, (void*)&wctl);
    }
    #endif
    //
    // This patch is for set H264 VUI full_range_flag
    //
    //UVCX_SetH264ColorRangeByStream() ;
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_VIDEO_FULL_RANGE, (void*)UVCX_SetH264ColorRangeByStream() );


    // POC type 2(Lync)
    MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_POC_TYPE, (void*)MMPF_H264ENC_POC_TYPE_2 );

    return PCAM_ERROR_NONE ;    

}

// 3.3.3 UVCX_RATE_CONTROL_MODE
MMP_USHORT UVCX_SetH264ControlMode(UVCX_RateControlMode *ctlMode)
{
    MMP_USHORT streamid = 0 ;
    MMP_ULONG rc_method;
    MMPF_VIDENC_RC_MODE_CTL RcModeCtl;

    streamid = STREAM_ID(ctlMode->wLayerID );
    if(streamid > bMaxStreamId) {
        return PCAM_USB_INIT_ERR ;
    }   
    
    
    //gbUVCXCtlMode = ctlMode ;
    gsUVCXCtlModeCur = *ctlMode ;
    RcModeCtl.bLayerGlobalRc = MMP_FALSE;
    switch (gsUVCXCtlModeCur.bRateControlMode) {
    case CBR_H264:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
        break;
    case VBR_H264:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_VBR;
        break;
    case CONSTANT_H264_QP:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CQP;
        break;
    default:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
        break;
    }

    MMPF_VIDENC_SetParameter(streamid, MMPF_VIDENC_ATTRIBUTE_RC_MODE, (void*)&RcModeCtl);

    return PCAM_ERROR_NONE ;    
}

// 3.3.4 UVCX_TEMPORAL_SCALE_MODE
MMP_USHORT UVCX_SetH264TemporalScaleMode(UVCX_TemporalScaleMode *tempScaleMode)
{
    gsUVCXTemporalScaleMode = *tempScaleMode ;
    return PCAM_ERROR_NONE ;    
}

// 3.3.5 UVCX_SPATIAL_SCALE_MODE
MMP_USHORT UVCX_SetH264SpatialScaleMode(UVCX_SpatialScaleMode *spatialScaleMode)
{
    gsUVCXSpatialScaleMode = *spatialScaleMode ;
    return PCAM_ERROR_NONE ;
}

// 3.3.6 UVCX_SNR_SCALE_MODE
MMP_USHORT UVCX_SetH264SNRScaleMode(UVCX_SNRScaleMode *SNRScaleMode)
{
    gsUVCXSNRScaleMode = *SNRScaleMode ;
    return PCAM_ERROR_NONE ;
}

// 3.3.7 UVCX_LTR_BUFFER_SIZE
MMP_USHORT UVCX_SetH264LTRBufferSize(UVCX_LTRBufferSizeControl *ltrBufSize)
{
     return PCAM_ERROR_NONE ;
   
}

// 3.3.14 UVCX_BITRATE 
MMP_USHORT UVCX_SetH264BitRate(UVCX_BitRateLayers *bRc)
{

    MMP_USHORT streamid = 0 , temperalid = 0;
    streamid = STREAM_ID(bRc->wLayerID );
    if(streamid > bMaxStreamId) {
        return PCAM_USB_INIT_ERR ;
    }   
    
    dbg_printf(0,"SetBitRate streamid : %d\r\n",streamid);
    gsUVCXBitRate = *bRc ;

    //dbg_printf(3,"#H264 new bitrate :%d\r\n",gsUVCXBitRate.dwAverageBitrate);
    
    //if(gbUVCXCtlMode != CONSTANT_H264_QP) {
    if( gsUVCXCtlModeCur.bRateControlMode != CONSTANT_H264_QP) {
    // Following is come from currect bitrate control method.
        MMPF_VIDENC_BITRATE_CTL BrCtl;
        MMP_UBYTE i;
        BrCtl.ubLayerBitMap = 0;
        if (gsUVCXTemporalScaleMode.bTemporalScaleMode == ONE_LAYER_H264) {
            BrCtl.ubLayerBitMap |= 1;
            BrCtl.ulBitrate[0]  = bRc->dwAverageBitrate;
        }
        else {
        #if 0 // 
            for (i = 0; i < gsUVCXTemporalScaleMode.bTemporalScaleMode; i++) {
                if (bRc->wLayerID & (1<<(LAYER_ID_BIT_SHIFT_TEMPORAL+i))) {
                    BrCtl.ubLayerBitMap |= (1 << i);
                    BrCtl.ulBitrate[i]  = bRc->dwAverageBitrate;
                }
            }
        #else
            // V1.0
            temperalid = TEMPORAL_ID( bRc->wLayerID );
            BrCtl.ubLayerBitMap |= (1 << temperalid );
            BrCtl.ulBitrate[temperalid]  = bRc->dwAverageBitrate;
        #endif    
        }
        MMPF_VIDENC_SetParameter(streamid, MMPF_VIDENC_ATTRIBUTE_BR, (void*)&BrCtl);
       // MMPF_VIDENC_SetParameter(streamid, MMPF_VIDENC_ATTRIBUTE_BR, (void*)&BrCtl);
    }
    return PCAM_ERROR_NONE ;
}

//
// 3.3.8 UVCX_QPSTEP_SIZE
// minQP can't be any value if slice is byte mode
// there is predifine minQP-inAccuracy bytes structure 
// please check "gsSliceBitsModeCfg"
//

MMP_USHORT UVCX_SetH264QPStepsLayers(UVCX_QPStepsLayers *qStep)
{
#if FIX_SLICE_BITS_MODE_INACCURACY
    UVCX_VIDEO_CONFIG *config;
#endif    
    MMP_USHORT streamid = 0 ;
    streamid = STREAM_ID(qStep->wLayerID );
    if(streamid > bMaxStreamId) {
        return PCAM_USB_INIT_ERR ;
    }   
    gsUVCXQPStepSize = *qStep ;

    if (qStep->bMinQp > qStep->bMaxQp) {
        qStep->bMaxQp = qStep->bMinQp;
    }
    //RTNA_DBG_Long(3, qStep->bMinQp);
    //RTNA_DBG_Long(3, qStep->bMaxQp);
    if (qStep->bFrameType & 0x01) {
        //RTNA_DBG_Str(3, " i frame qp bound\r\n");
        MMPF_VIDENC_SetQPBound(streamid, 0, MMPF_3GPMGR_FRAME_TYPE_I, qStep->bMinQp, qStep->bMaxQp);
    }
    if (qStep->bFrameType & 0x02) {
        //RTNA_DBG_Str(3, " p frame qp bound\r\n");
        MMPF_VIDENC_SetQPBound(streamid, 0, MMPF_3GPMGR_FRAME_TYPE_P, qStep->bMinQp, qStep->bMaxQp);
    }
    if (qStep->bFrameType & 0x04) {
       // RTNA_DBG_Str(3, " b frame qp bound\r\n");
        MMPF_VIDENC_SetQPBound(streamid, 0, MMPF_3GPMGR_FRAME_TYPE_B, qStep->bMinQp, qStep->bMaxQp);
    }
#if FIX_SLICE_BITS_MODE_INACCURACY
    config = UVCX_GetH264StartConfig(streamid) ;
    if(config->wSliceMode == MULTI_SLICE_BITs) {
        MMPF_H264ENC_ENC_INFO *pEnc = MMPF_H264ENC_GetHandle(streamid);

        MMPF_VIDENC_SLICE_CTL   SliceCtl;
        MMP_USHORT max_bytes = config->wSliceUnits / 8  ;

        MMP_USHORT inacc_bytes;
        MMP_LONG minIQP,maxIQP;
        MMP_LONG minPQP,maxPQP;
        MMP_LONG curMinQP ;
        MMPF_VIDENC_GetQPBound(streamid, 0, MMPF_3GPMGR_FRAME_TYPE_I,&minIQP,&maxIQP);
        dbg_printf(3,"--QP_Layer I (%d,%d)\r\n",minIQP,maxIQP);
        MMPF_VIDENC_GetQPBound(streamid, 0, MMPF_3GPMGR_FRAME_TYPE_P,&minPQP,&maxPQP);
        dbg_printf(3,"--QP_Layer P (%d,%d)\r\n",minPQP,maxPQP);
        
        curMinQP = VR_MIN(minIQP,minPQP) ;
        curMinQP = VR_MIN(curMinQP,qStep->bMinQp);
        
        dbg_printf(3,"--QP=%d for inaccuray \r\n",curMinQP);
        inacc_bytes = UVCX_GetInaccurayBytesByMinQP(curMinQP);
        if(inacc_bytes==0) {
            dbg_printf(0,"Not Predef-QP in slice bits mode\r\n");
            return PCAM_USB_INIT_ERR ;
        }
        // Adjust new slice bits
        if(max_bytes) {
            SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_BYTE ;
            SliceCtl.ulSliceSize = max_bytes - inacc_bytes;  
            dbg_printf(3,"--Max Slice Byte : %d,Set to :%d\r\n",max_bytes,SliceCtl.ulSliceSize );
            MMPF_VIDENC_SetParameter(config->bStreamID, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
        }
    }
#endif

    return PCAM_ERROR_NONE ;
}

// 3.3.9 UVCX_PICUTRE_TYPE_CONTROL
MMP_USHORT UVCX_SetH264PictureTypeCtl(UVCX_PictureTypeControl *picType)
{
    MMP_USHORT streamid = 0 ;
    streamid = STREAM_ID(picType->wLayerID );
    
    if(streamid > bMaxStreamId) {
        return PCAM_USB_INIT_ERR ;
    }   
    gsUVCXPicTypeCtl = *picType ;

    switch (gsUVCXPicTypeCtl.wPicType) {
    case PIC_IDR_WITH_NEW_SPSPPS:
        MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)MMPF_VIDENC_PICCTL_IDR_RESYNC);
        break;
    case PIC_IDR_FRAME:
        MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)MMPF_VIDENC_PICCTL_IDR);
        break;
    case PIC_I_FRAME:
        MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)MMPF_VIDENC_PICCTL_I);
        break;
    default:
        MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)MMPF_VIDENC_PICCTL_NONE);
        break;
    }

    return PCAM_ERROR_NONE ;
}

// 3.3.11 UVCX_CROP_CONFIG
MMP_USHORT UVCX_SetH264CropConfig(UVCX_CropConfig *crop)
{
    MMPF_VIDENC_CROPPING    crop_offset;

    gsUVCXCropConfig = *crop;

    crop_offset.usTop   = crop->wCropTopOffset;
    crop_offset.usBottom = crop->wCropBottomOffset;
    crop_offset.usLeft  = crop->wCropLeftOffset;
    crop_offset.usRight = crop->wCropRightOffset;
    MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_CROPPING, (void*)&crop_offset);

    return PCAM_ERROR_NONE ;
}

// 3.3.12 UVCX_FRAMERATE_CONFIG
// Input is 100ns BASE
MMP_USHORT UVCX_SetH264FrameRate(UVCX_FrameRateConfig *frameInterval/*MMP_ULONG frameInterval*/)
{ 
    MMP_UBYTE  streamid = 0 ;
    MMP_USHORT fps = 0 ;
    MMPF_VIDENC_MAX_FPS_CTL FpsCtl = {3000, 100};
    
    streamid = STREAM_ID(frameInterval->wLayerID );
    
    if(streamid > bMaxStreamId) {
        return PCAM_USB_INIT_ERR ;
    }   
    gsUVCXFrameRate = *frameInterval ;
    if(gsUVCXFrameRate.dwFrameInterval) {
        fps = 10000000 / gsUVCXFrameRate.dwFrameInterval ;
    }
    if(fps < 5) fps = 5 ;
    else if(fps > 30) {
#if ENABLE_1152_648_MODE==0
        fps = 30; 
#endif    
    }
    
    FpsCtl.ulMaxFpsIncreament = VIDEO_FPS_SCALE;
    FpsCtl.ulMaxFpsResolution = VIDEO_FPS_SCALE * fps;
    dbg_printf(0,"H264(streamid:%d) FPS:%d/%d\r\n",streamid,FpsCtl.ulMaxFpsResolution,FpsCtl.ulMaxFpsIncreament);
    
    MMPF_VIDENC_SetParameter(streamid, MMPF_VIDENC_ATTRIBUTE_MAX_FPS, (void*)&FpsCtl);
    
    return PCAM_ERROR_NONE ;
}


MMP_USHORT UVCX_SetH264AdvConfig(UVCX_AdvConfig *advConfig)
{
    gsUVCXAdvConfig.bLevel_idc = advConfig->bLevel_idc ;    

    // static config
    //MMPD_H264ENC_SetLevel(0, advConfig->bLevel_idc);

    return PCAM_ERROR_NONE;
}


//
// USB_SetH264Resolution should be called before use this function
//
MMP_USHORT UVCX_GetH264EstimatedVideoDelay(MMP_USHORT *delay,MMP_USHORT *maxDelay)
{
#define DELAY_1080P      (30)
#define MAX_CONFIG_DELAY (250) // Don't know the meaning, just use value from spec. example

    MMP_USHORT d1=30,d2 = 33;
    PCAM_USB_INFO *pcamInfo = pcam_get_info();
    MMP_ULONG maxResSize = GetYUV420FrameSize(PCAM_USB_RESOL_1920x1080);
    MMP_ULONG curResSize = GetYUV420FrameSize(pcamInfo->pCamVideoRes);
    
    *delay = ( DELAY_1080P * curResSize) / maxResSize ;
    *maxDelay = MAX_CONFIG_DELAY ;
    dbg_printf(3,"#UVCX estimated (delay,maxdelay) : (%d,%d)\r\n");
    return PCAM_ERROR_NONE ;
    
}

//
// 0 : Normal (in spec.)
// 1 : Vertical Flip ,Mirror (in spec.)
// 2 : Horizontal Flip ( not in spec.)
// 3 : Both direction Flip (not in spec.)
MMP_USHORT UVCX_SetH264FlipMode(MMP_USHORT flip_id)
{
#if BIND_SENSOR_MT9T002==1    
    MMP_USHORT val ;//= ISP_HDM_IF_SNR_ReadI2C(0x3040);
    MMP_USHORT val1;//= val & 0x3FF ;
    gsSensorFunction->MMPF_Sensor_GetReg(0x3040, &val);
    
    val1 = val & 0x3FF ;
    if(flip_id==1) {
        val1 = ( val1 | 0x4000) ;
    }
    else if(flip_id==2) {
        val1 = ( val1 | 0x8000) ;
    }
    else if(flip_id==3) {
        val1 = ( val1 | 0xC000) ;
    }
    
    if(val!=val1) {
        gsSensorFunction->MMPF_Sensor_SetReg(0x3040,val1);
    }    
#endif    
    return PCAM_ERROR_NONE ;
}

void UVCX_EnableSliceBasedTx(MMP_BOOL en)
{

    MMPF_VIDENC_OUTPUT_SYNC_MODE sync_mode ;
#if SLOT_RING     
    sync_mode = (en)? MMPF_VIDENC_OUTPUT_SLICE_SYNC: MMPF_VIDENC_OUTPUT_FRAME_SYNC;
#else
    sync_mode = MMPF_VIDENC_OUTPUT_FRAME_SYNC;
#endif
    #if (STRICT_AVG_FRAME_SIZE == 1)
    sync_mode = MMPF_VIDENC_OUTPUT_FRAME_SYNC;
    #endif
    MMPF_VIDENC_SetOutputSyncMode(sync_mode);
    dbg_printf(3,"H264 slice-base Tx : %d\r\n",sync_mode);
}

MMP_BOOL UVCX_IsSliceBasedTx(PIPE_PATH pipe)
{
    STREAM_SESSION *ss ;
    STREAM_CFG  *cur_stream ;
    cur_stream = (STREAM_CFG *)usb_get_cur_image_pipe(pipe);
    return UVCX_IsSliceBasedTxStream(cur_stream);
}

MMP_BOOL UVCX_IsSliceBasedTxStream(STREAM_CFG *cur_stream_in)
{
    MMP_BOOL slicetx = 0 ;
    STREAM_CFG *cur_stream,cur_stream_tmp = *cur_stream_in;
    cur_stream = &cur_stream_tmp ;
    cur_stream->pipe_en &= ~PIPEEN(GRA_SRC_PIPE) ;
    if(cur_stream) {
        if( IS_PIPE01_EN(cur_stream->pipe_en) ) {
            return 0 ;
        }
        else if( IS_PIPE0_EN(cur_stream->pipe_en) ) {
            MMP_UBYTE pipe0_cfg ;
            pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_stream->pipe_cfg);//(cur_stream->pipe_cfg & PIPE_CFG_MASK) ;     
            if(pipe0_cfg==PIPE_CFG_H264) {
                slicetx = (MMPF_VIDENC_GetOutputSyncMode() == MMPF_VIDENC_OUTPUT_SLICE_SYNC);    
            }
        }
        else {
            MMP_UBYTE pipe1_cfg ;
            pipe1_cfg =CUR_PIPE_CFG(PIPE_1,cur_stream->pipe_cfg);// ( (cur_stream->pipe_cfg >> 4) & PIPE_CFG_MASK) ;
            if(pipe1_cfg==PIPE_CFG_H264) {
                slicetx = (MMPF_VIDENC_GetOutputSyncMode()== MMPF_VIDENC_OUTPUT_SLICE_SYNC);    
            }
        }
    }
    //dbg_printf(3,"#Slice Tx : %d\r\n",slicetx);
    return  slicetx ; // AABBTEST

}
//
// return Init QP before start preview
// return QP after RC after start preview
// wLayerId : UVC 1.1 H264 layer id
// typeBitmap : (1<<I_FRAME) ,(1<<P_FRAME) ,(1<<B_FRAME) 
// 
MMP_USHORT UVCX_GetH264CurQP(MMP_USHORT wLayerId,MMP_UBYTE typeBitMap, MMP_UBYTE *I_QP,MMP_UBYTE *P_QP,MMP_UBYTE *B_QP)
{
    MMPF_VIDENC_QP_CTL qp_ctl;
    MMP_USHORT streamid = 0 ,temperalid = 0 ;
    MMP_ERR err;
    streamid = STREAM_ID(wLayerId );
    if(streamid > bMaxStreamId) {
        return PCAM_USB_INIT_ERR ;
    }   
    temperalid = TEMPORAL_ID(wLayerId);
    
    qp_ctl.ubTID = temperalid;                   // layer id
    qp_ctl.ubTypeBitMap = typeBitMap ;         //  return I P B frame type
    *I_QP = *P_QP = *B_QP = 0 ;
    dbg_printf(3,"GetQP,sid=%d,tid=%d\r\n",streamid,temperalid);
    err = MMPF_VIDENC_GetParameter(streamid, MMPF_VIDENC_ATTRIBUTE_FRM_QP, &qp_ctl);
    if(err==MMP_ERR_NONE) {
        if(qp_ctl.ubTypeBitMap & (1 << I_FRAME)) {
            *I_QP = qp_ctl.ubQP[I_FRAME] ;
        }
        if(qp_ctl.ubTypeBitMap & (1 << P_FRAME)) {
            *P_QP = qp_ctl.ubQP[I_FRAME] ;
        }
        if(qp_ctl.ubTypeBitMap & (1 << B_FRAME)) {
            *B_QP = qp_ctl.ubQP[I_FRAME] ;
        }
    
    }
   //return ­È¦b : qp_ctl.ubQP[I_FRAME], qp_ctl.ubQP[P_FRAME], qp_ctl.ubQP[B_FRAME]
    return (MMP_USHORT)err;

}

#if H264_CBR_PADDING_EN==1
//
// Remap Rate Control Mode to the following :
// CBR -> AIT CBR with bit-stuffing
// VBR -> AIT CBR without bit-stuffing
// Default -> AIT CBR without bit-stuffing
//
MMP_USHORT UVCX_SetH264ControlModeFixCBR(UVCX_RateControlMode *ctlMode)
{
    MMP_USHORT streamid = 0 ;
    MMP_ULONG rc_method;
    MMPF_VIDENC_RC_MODE_CTL RcModeCtl;

    streamid = STREAM_ID(ctlMode->wLayerID );
    if(streamid > bMaxStreamId) {
        return PCAM_USB_INIT_ERR ;
    }   
    
    
    //gbUVCXCtlMode = ctlMode ;
    gsUVCXCtlModeCur = *ctlMode ;
    RcModeCtl.bLayerGlobalRc = MMP_FALSE;
    
    MMPF_VIDENC_SetParameter(streamid,MMPF_VIDENC_ATTRIBUTE_CBR_PADDING,(void *)MMP_FALSE) ;
    
    switch (gsUVCXCtlModeCur.bRateControlMode) {
    case CBR_H264:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
        MMPF_VIDENC_SetParameter(streamid,MMPF_VIDENC_ATTRIBUTE_CBR_PADDING,(void *)MMP_TRUE) ;
        usb_vs_zero_cbr_padding_buffer();
        break;
    case VBR_H264:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
        break;
    case CONSTANT_H264_QP:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CQP;
        break;
    default:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
        break;
    }

    MMPF_VIDENC_SetParameter(streamid, MMPF_VIDENC_ATTRIBUTE_RC_MODE, (void*)&RcModeCtl);

    return PCAM_ERROR_NONE ;    
}
#endif

ENCODE_BUF_MODE gbH264EncodeBufMode = FRAME_MODE ;
MMP_USHORT UVCX_SetH264EncodeBufferMode(ENCODE_BUF_MODE mode)
{
#if H264_SIMULCAST_EN
    gbH264EncodeBufMode = mode ;
#endif    
#if SUPPORT_PCSYNC
    if(pcsync_is_preview()) {
        gbH264EncodeBufMode = FRAME_MODE ;
    }
#endif
    dbg_printf(0,"gbH264EncodeBufMode : %d\r\n",gbH264EncodeBufMode);
    return PCAM_ERROR_NONE ;
}

ENCODE_BUF_MODE UVCX_GetH264EncodeBufferMode(void)
{
    return gbH264EncodeBufMode ;
}

MMP_USHORT UVCX_SwitchH264EncodeBufferMode(ENCODE_BUF_MODE mode)
{
#if H264_SIMULCAST_EN
extern MMP_UBYTE gbChangeH264BufMode ;
extern MMP_USHORT  UVCTxFIFOSize[] ;
    MMP_UBYTE ep =STREAM_EP_H264 ;/// usb_get_cur_image_ep(PIPE_1);
    MMP_BOOL ispreview; 
    dbg_printf(3,"<Switch H264 buffer mode:%d>\r\n",mode);
    
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&ispreview,ep); 
    UVCX_SetH264EncodeBufferMode(mode);
    if(ispreview) {
        gbChangeH264BufMode = 1 ;
        usb_uvc_stop_preview(ep,1) ;
        MMPF_Video_InitStreamSession(ep,SS_TX_STREAMING | SS_TX_BY_DMA ,UVCTxFIFOSize[ep]);
        usb_uvc_start_preview(ep,1);
        gbChangeH264BufMode = 0 ;
    }
#endif
    return PCAM_ERROR_NONE ;
}

MMP_USHORT UVCX_RestartPreview(MMP_UBYTE ep_id)
{
extern MMP_BOOL m_bVidRecdPreviewStatus[] ;
extern MMP_BOOL	gbGRAEn[]  ;
extern  MMP_USHORT	gsZoomRatio[2],gsLastZoomRatio[2];

    if(1) {
        dbg_printf(0,"<Restart ep:%d>\r\n",ep_id);
        MMPF_USB_PauseStreaming(ep_id,MMP_TRUE);
        
        if(UVCX_GetH264EncodeBufferMode()!=FRAME_MODE) {
            MMPF_IBC_SetStoreEnable(MMPF_IBC_PIPE_2,MMP_FALSE);
            MMPF_IBC_SetInterruptEnable(MMPF_IBC_PIPE_2,MMPF_IBC_EVENT_FRM_RDY,MMP_FALSE);
                
        }
        
        m_bVidRecdPreviewStatus[ep_id] = 0 ;
        gbGRAEn[ep_id] = MMP_FALSE ;
        gsZoomRatio[ep_id] = gsLastZoomRatio[ep_id] = ZOOM_RATIO_BASE;
        MMPF_Scaler_ResetPathInfo((ep_id==0)?MMPF_SCALER_PATH_0:MMPF_SCALER_PATH_1);
        
        gbRestartPreview = 1 ;
        usb_uvc_start_preview(ep_id,1);
        gbRestartPreview = 0 ;
        MMPF_USB_PauseStreaming(ep_id,MMP_FALSE);
        dbg_printf(0,"<Restart ep:%d end>\r\n",ep_id);
    }
    return PCAM_ERROR_NONE ;
}
