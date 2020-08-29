#include "config_fw.h"
#include "mmpf_usbskypeh264.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbvend.h"
#include "pcam_msg.h"
#include "mmpf_mp4venc.h"
#include "mmpf_h264enc.h"
#include "mmps_3gprecd.h"
#include "lib_retina.h"
#if USB_UVC_SKYPE
SKYPE_VIDEO_CONFIG gsSkypeProbeCfgCur[2] =
{
    {
    SkypeH264,
    640,480,
    330000,
    500*1024
    },
    {
    SkypeYUY2,
    320,240,
    330000,
    320*240*2*30
    }
} ;

SKYPE_VIDEO_CONFIG gsSkypeProbeCfgMin[2] = 
{
    {
    SkypeH264,
    160,90,
    330000,
    25*1024
    },
    {
    SkypeYUY2,
    160,90,
    330000,
    160*90*2*30
    }
} ;

SKYPE_VIDEO_CONFIG gsSkypeProbeCfgMax[2] = 
{
    {
    SkypeH264,
    1920,1080,
    330000,
    5*1024*1024
    },
    {
    SkypeYUY2,
    320,240,
    330000,
    320*240*2*30,
    }
} ;
SkypeH264Error gbSkypeLastError = NoError ;

MMP_ULONG glSkypeFrameInterval = 0x51615 ;
MMP_USHORT gsSkypeFwDays ;
SkypeH264StreamID gbSkypeStreamID = SkypeMainStream ;
SkypeH264EndPointType gbSkypeEndpoint = DualEndPointPreviewFirst ;
MMP_ULONG glSkypeBitrate   ;
MMP_UBYTE gbSkypeKeyFrame  = 1 ;

VC_CMD_CFG SKYPE_VERSION_CFG = {
    ( CAP_GET_CUR_CMD | CAP_GET_INFO_CMD ), 
    ( INFO_GET_SUPPORT ),
    1,1,
    1,SKYPE_VERSION,0,0,SKYPE_VERSION
} ;


VC_CMD_CFG SKYPE_FWDAYS_CFG = {
    ( CAP_GET_CUR_CMD | CAP_GET_INFO_CMD ), 
    ( INFO_GET_SUPPORT ),
    2,1,
    1,0,0,0,0
} ;

VC_CMD_CFG SKYPE_LASTERROR_CFG = {
    ( CAP_GET_CUR_CMD | CAP_GET_INFO_CMD ), 
    ( INFO_GET_SUPPORT ),
    1,1,
    1,NoError,0,0,NoError
} ;

VC_CMD_CFG SKYPE_STREAMID_CFG = {
    ( CAP_GET_MIN_CMD | CAP_GET_MAX_CMD|CAP_SET_CUR_CMD  ), 
    (INFO_GET_SUPPORT),
    1,1,
    1,SkypeMainStream,SkypeMainStream,SkypePreviewStream,SkypeMainStream 
} ;

VC_CMD_CFG SKYPE_ENDPOINT_CFG = {
    ( CAP_GET_CUR_CMD | CAP_GET_INFO_CMD ), 
    ( INFO_GET_SUPPORT ),
    1,1,
    1,SingleEndPoint,SingleEndPoint,SingleEndPoint,SingleEndPoint
} ;

VC_CMD_CFG SKYPE_VIDEO_CFG = {
    ( CAP_GET_CUR_CMD | CAP_GET_MAX_CMD | CAP_GET_MIN_CMD | CAP_SET_CUR_CMD ), 
    ( INFO_GET_SUPPORT ),
    13,1,
    1,0,(MMP_ULONG)&gsSkypeProbeCfgMin,(MMP_ULONG)&gsSkypeProbeCfgMax,(MMP_ULONG)&gsSkypeProbeCfgCur
} ;

VC_CMD_CFG SKYPE_BITRATE_CFG = {
    (CAP_SET_CUR_CMD),
    ( INFO_GET_SUPPORT ),
    4,1,
    1,0,0,0,0     
} ;

VC_CMD_CFG SKYPE_FRAMERATE_CFG = {
    (CAP_SET_CUR_CMD),
    ( INFO_GET_SUPPORT ),
    4,1,
    1,0x51615,0,0,0x51615
} ;

VC_CMD_CFG SKYPE_KEYFRAME_CFG = {
    (CAP_SET_CUR_CMD),
    ( INFO_GET_SUPPORT ),
    1,1,
    1,1,0,0,1     
};

void SKYPE_SetLastError( SkypeH264Error err) 
{
    gbSkypeLastError =  err ;
}

MMP_USHORT SKYPE_SetH264BitRate(MMP_ULONG bitrate)
{
    MMPF_VIDENC_BITRATE_CTL BrCtl;
    glSkypeBitrate = bitrate ;
    BrCtl.ubLayerBitMap = 1;
    BrCtl.ulBitrate[0]  = bitrate ;

    MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_BR, (void*)&BrCtl);

    return PCAM_ERROR_NONE ;    
    
}

MMP_USHORT SKYPE_SetH264FrameRate(MMP_ULONG frameinterval)
{
    MMP_USHORT fps ;
    MMPF_VIDENC_MAX_FPS_CTL FpsCtl;

    glSkypeFrameInterval = frameinterval ;
    if(glSkypeFrameInterval) {
        fps = 10000000 / glSkypeFrameInterval ;
    }
    if(fps < 5) fps = 5 ;
    else if(fps > 30) fps = 30; 
    
    FpsCtl.ulMaxFpsResolution = fps*100;
    FpsCtl.ulMaxFpsIncreament = 100;

    MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_MAX_FPS, (void*)&FpsCtl);
    
    return PCAM_ERROR_NONE ;    
    
}

MMP_USHORT SKYPE_SetH264KeyFrame(MMP_UBYTE keyframe)
{
    MMPF_VIDENC_PICCTL PicCtl = MMPF_VIDENC_PICCTL_IDR;
    if(keyframe!=1) {
        return PCAM_USB_FIRMWARE_ERR;
    } 
    gbSkypeKeyFrame = keyframe ;

    MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)PicCtl);

    return PCAM_ERROR_NONE ;
}

#endif
