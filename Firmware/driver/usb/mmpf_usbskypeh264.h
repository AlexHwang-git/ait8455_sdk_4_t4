#ifndef _MMPF_USBSKYPE_H264_H
#define _MMPF_USBSKYPE_H264_H
#include "config_fw.h"
#include "mmpf_typedef.h"
#if USB_UVC_SKYPE==1

#define set_double(var,val) var[0] = ((val) >> 8 ) & 0xff ; \
                            var[1] = ((val)      ) & 0xff

#define set_triple(var,val) var[0] = ((val) >> 16) & 0xff ; \
                            var[1] = ((val) >> 8 ) & 0xff ; \
                            var[2] = ((val)      ) & 0xff 

#define set_quad(var,val)   var[0] = ((val) >> 24) & 0xff ; \
                            var[1] = ((val) >> 16) & 0xff ; \
                            var[2] = ((val) >> 8 ) & 0xff ; \
                            var[3] = ((val)      ) & 0xff 

#define get_double(var) ((*var << 8) + *(var + 1))
#define get_triple(var) ((*var << 16) +     \
                         (*(var + 1)<<8) + *(var+2))

#define get_quad(var)   ( (*var << 24) + \
                        (*(var + 1) << 16) + \
                        (*(var + 2) <<  8) + *(var + 3))

// 3.3 H.264 UVC Extension Unit definition
#define XU_SKYPE_VERSION    (0x01)
#define XU_SKYPE_LASTERROR	(0x02)
#define XU_SKYPE_FWDAYS     (0x03)
#define XU_SKYPE_STREAMID   (0x04)
#define XU_SKYPE_ENDPOINT   (0x05)
#define XU_SKYPE_SF_PROBE   (0x08)
#define XU_SKYPE_SF_COMMIT  (0x09)
#define XU_SKYPE_BITRATE    (0x18)
#define XU_SKYPE_FRAMERATE  (0x19)
#define XU_SKYPE_KEYFRAME   (0x1a)

#define SKYPE_VERSION 0x14 
#define SKYPE_MAGIC   0x534B5950

typedef enum {
	NoError = 0,
	NotAllowed,
	InvalidArgument,
	NotSupported	   
} SkypeH264Error ;

typedef enum {
	SingleEndPoint = 0,
	DualEndPointMainFirst,
	DualEndPointPreviewFirst
} SkypeH264EndPointType ;

typedef enum {
	SkypeYUY2 = 0,
	SkypeNV12 ,
	SkypeMJPEG,
	SkypeH264
} SkypeH264StreamType;

typedef enum {
	SkypeMainStream = 0 ,
	SkypePreviewStream ,
	SkypeStreamEnd
} SkypeH264StreamID;
	
typedef __packed struct _SKYPE_VIDEO_CONFIG {
	MMP_UBYTE   bStreamType ;
	MMP_USHORT	wWidth ;
	MMP_USHORT	wHeight ;
	MMP_ULONG   dwFrameInterval;
	MMP_ULONG   dwBitrate ;
} SKYPE_VIDEO_CONFIG ;
#define 	SKYPE_VIDEO_CONFIG_LEN sizeof(SKYPE_VIDEO_CONFIG)
#define     SKYPE_HEADER_LEN    (20)
void SKYPE_SetLastError( SkypeH264Error err)  ;
MMP_USHORT SKYPE_SetH264BitRate(MMP_ULONG bitrate);
MMP_USHORT SKYPE_SetH264FrameRate(MMP_ULONG frameinterval);
MMP_USHORT SKYPE_SetH264KeyFrame(MMP_UBYTE keyframe);

#endif

#endif



