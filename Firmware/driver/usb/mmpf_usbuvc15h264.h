#ifndef _MMPF_USBUVC15_H264_H
#define _MMPF_USBUVC15_H264_H
#include "config_fw.h"
#include "mmpf_typedef.h"
#include "mmpf_mp4venc.h"
#include "mmpf_usbuvc.h"

#if 1//(ENCODING_UNIT == 1)
#define		EU_CONTROL_UNDEFINED    				0x00
#define		EU_SELECT_LAYER_CONTROL					0x01
#define		EU_PROFILE_TOOLSET_CONTROL          	0x02
#define		EU_VIDEO_RESOLUTION_CONTROL         	0x03
#define		EU_MIN_FRAME_INTERVAL_CONTROL			0x04
#define		EU_SLICE_MODE_CONTROL					0x05
#define		EU_RATE_CONTROL_MODE_CONTROL        	0x06
#define		EU_AVERAGE_BITRATE_CONTROL          	0x07
#define		EU_CPB_SIZE_CONTROL                 	0x08
#define		EU_PEAK_BIT_RATE_CONTROL            	0x09
#define		EU_QUANTIZATION_PARAMS_CONTROL			0x0A
#define		EU_SYNC_REF_FRAME_CONTROL           	0x0B

#define		EU_LTR_BUFFER_SIZE_CONTROL				0x0C		// Not support in AIT 8423
#define		EU_LTR_PICTURE_CONTROL					0x0D		// Not support in AIT 8423
#define		EU_LTR_VALIDATION_CONTROL				0x0E		// Not support in AIT 8423
#define		EU_LEVEL_IDC_LIMIT_CONTROL				0x0F

#define		EU_SEI_PAYLOADTYPE_CONTROL            	0x10
#define		EU_QP_RANGE_CONTROL                     0x11
#define		EU_PRIORITY_ID_CONTROL              	0x12
#define		EU_START_OR_STOP_LAYER_CONTROL      	0x13
#define     EU_START_INIT                           0x1F


#define SVC_DEPENDENCY_ID( wLayerID) (  wLayerID & 0x7 ) 
#define SVC_QUALITY_ID( wLayerID )   ( ( wLayerID >> 3 ) & 0xf ) 
#define SVC_TEMPORAL_ID( wLayerID )  ( ( wLayerID >> 7)  & 0x7 ) 
#define SVC_STREAM_ID( wLayerID )    ( ( wLayerID >> 10 )& 0x7 ) 
#define SVC_VI_INDEX( wLayerID )     ( ( wLayerID >> 13 )& 0x7 ) // reserved, set to 0

#define SVC_WILDCARD_MASK 0x07
#define SIM_WILDCARD_MASK 0x07


#define SVC_LAYER_ID( dId , qId, tId , sId, vsiIdx )   (  ((vsiIdx & 0x7) << 13) | ( ( sId & 0x7) << 10 )| ( (tId & 0x07) << 7) | ( (qId & 0xf) << 3  ) | (dId & 0x07)) 

#define SVC_DEF_ID  SVC_LAYER_ID(0,0,0,0,0)
#define SVC_MIN_ID  SVC_LAYER_ID(0,0,0,0,0) 
#define SVC_MAX_ID  SVC_LAYER_ID(0,0,0,0,0) 
#define SVC_CUR_ID  SVC_LAYER_ID(0,0,0,0,0) 

#define COMMIT_RC(s0_rc,s1_rc,s2_rc,s3_rc) ( ( s0_rc & 0xf ) | ((s1_rc & 0xf) << 4) | ((s2_rc & 0xf) << 8) | ((s3_rc & 0xf) << 4) )
#define COMMIT_LAYOUT(svc_str) (svc_str & 0x7)

#define STR_RC(sid,commit_rc)   (( commit_rc >> (4 * sid) ) & 0xf)

#define EU_S2L(lw,hw) ( ( lw & 0xFFFF) | (hw & 0xFFFF) << 16 ) 
#define EU_RES_CUR  EU_S2L( 640,480)
#define EU_RES_MIN  EU_S2L( 120, 90)
#define EU_RES_MAX  EU_S2L(1920,1080)
#define EU_RES_DEF  EU_RES_CUR
  
#define EU_FRAME_INTERVAL( fps )    ( 10000000 / fps     )  
#define EU_FRAME_FPS( intval )      ( 10000000 / intval  ) 
#define EU_FRAME_FPSx10( intval )   ( (10000000*10) / intval  ) 
  
#define EU_SLICE_MODE_MIN EU_S2L(2,1)  
#define EU_SLICE_MODE_MAX EU_S2L(2,5)  
#define EU_SLICE_MODE_DEF EU_S2L(2,4)  

#define EU_BR_MIN   (64000)
#define EU_BR_MAX   (12000000)
#define EU_BR_DEF   (2000000)
#define EU_BR_CUR   (EU_BR_DEF)
#define EU_BR_RES   (8000)  // 8 K bps
// TODO : Just put a value right now 
#define EU_PBR_MIN   (64000 *12 / 10)
#define EU_PBR_MAX   (12000000 * 12 /10)
#define EU_PBR_DEF   (2000000 * 12/10)
#define EU_PBR_CUR   (EU_PBR_DEF)

#define EU_SYNC_REF_FRAME(type,intval,refresh) ( (type & 0xFF) | ((intval&0xFFFF) << 8) || (( refresh&0xFF) << 24) )

#define EU_DEF_LBS  (500)
#define EU_CPBSize2LBSize(cpbsize,br)   (16000*cpbsize) / br
#define EU_LBSize2CPBSize(lbsize,br)    (lbsize*br) / 16000 

#define EU_QP_ALL(yqp,uqp,vqp) ( (yqp & 0xFF) | ((uqp & 0xf) << 8) | ( (vqp & 0xf) << 12) )

#define EU_QP_Y(qp)    ( qp & 0xFF )
#define EU_QP_U(qp)    ( ( qp >>  8 ) & 0xF )
#define EU_QP_V(qp)    ( ( qp >> 12 ) & 0xF )


#define EU_LEVELIDC_MIN (1)
#define EU_LEVELIDC_MAX (51)
#define EU_LEVELIDC_DEF (40)
#define EU_LEVELIDC_CUR EU_LEVELIDC_DEF

#define EU_QP_RANGE_M(min,max)    (( min & 0xFF ) | (( max & 0xFF) << 8 ) )

#define EU_PRIO_ID_MIN  (0 )
#define EU_PRIO_ID_MAX  (63)
#define EU_PRIO_ID_DEF  (0)
#define EU_PRIO_ID_CUR  EU_PRIO_ID_DEF

#define EU_CS_VAL_ALL   ( ( 1 << EU_SELECT_LAYER_CONTROL			) | \
                        ( 1 << EU_PROFILE_TOOLSET_CONTROL		) | \
                        ( 1 << EU_VIDEO_RESOLUTION_CONTROL		) | \
                        ( 1 << EU_MIN_FRAME_INTERVAL_CONTROL	) | \
                        ( 1 << EU_SLICE_MODE_CONTROL			) | \
                        ( 1 << EU_RATE_CONTROL_MODE_CONTROL		) | \
                        ( 1 << EU_AVERAGE_BITRATE_CONTROL		) | \
                        ( 1 << EU_CPB_SIZE_CONTROL				) | \
                        ( 1 << EU_PEAK_BIT_RATE_CONTROL			) | \
                        ( 1 << EU_QUANTIZATION_PARAMS_CONTROL	) | \
                        ( 1 << EU_SYNC_REF_FRAME_CONTROL 		) | \
                        ( 1 << EU_SEI_PAYLOADTYPE_CONTROL		) | \
                        ( 1 << EU_QP_RANGE_CONTROL) | \
                        ( 1 << EU_PRIORITY_ID_CONTROL 			) | \
                        ( 1 << EU_START_OR_STOP_LAYER_CONTROL 	) | \
                        ( 1 << EU_LEVEL_IDC_LIMIT_CONTROL ) )


#define EU_CS_VAL_WIN8  ( ( 1 << EU_SELECT_LAYER_CONTROL			) | \
                        ( 1 << EU_VIDEO_RESOLUTION_CONTROL		) | \
                        ( 1 << EU_MIN_FRAME_INTERVAL_CONTROL	) | \
                        ( 1 << EU_RATE_CONTROL_MODE_CONTROL		) | \
                        ( 1 << EU_AVERAGE_BITRATE_CONTROL		) | \
                        ( 1 << EU_CPB_SIZE_CONTROL				) | \
                        ( 1 << EU_QUANTIZATION_PARAMS_CONTROL	) | \
                        ( 1 << EU_SYNC_REF_FRAME_CONTROL 		) | \
                        ( 1 << EU_LEVEL_IDC_LIMIT_CONTROL       ) | \
                        ( 1 << EU_PRIORITY_ID_CONTROL           ) | \
                        ( 1 << EU_START_OR_STOP_LAYER_CONTROL 	) )
                        
	
#define EU_CS_VAL       (EU_CS_VAL_ALL /*>> 1*/ )// bmControls in EU desciptor	
#define EU_CS_VAL_RT    (EU_CS_VAL_WIN8 /*>> 1*/ )// bmControlsRunTime descriptor

typedef enum 
{
    RT_UCCONFIG_MODE0=0,
    RT_UCCONFIG_MODE1,
    RT_UCCONFIG_MODE2Q,
    RT_UCCONFIG_MODE2S,
    RT_UCCONFIG_MODE3,
    BROADCAST_MODE=8,
    STORAGE_MODE_IP=16,
    STORAGE_MODE_IPB=17,
    STORAGE_MODE_I = 18
} H264_USAGE ;
	
typedef enum
{
    CAVLC_ONLY = 0 ,
    CABAC_ONLY,
    CONST_FR,
    SEPERATE_QP_LC,
    SEPERATE_QP_CBR,
    NO_PIC_REORDERING,
    LTR_FRAME        
} H264_CAPABILITIES ;

typedef enum
{
    DEVICE_CHOOSE= 0x00,
    CAVLC_MODE   = 0x01,
    CABAC_MODE   = 0x02
} H264_ENTROPYMODE ;
	
typedef enum
{
    SYNC_IDR_BY_DEVICE = 0,
    SYNC_IDR_WITH_SPSPPS,
    SYNC_IDR_WITH_LTR_SPSPPS ,
    SYNC_I_WITH_SPSPPS,
    SYNC_I_WITH_LTR_SPSPPS,
    SYNC_P_WITH_LTR_SPSPPS,
    SYNC_WITH_GDR   
} SYNC_FRAME_TYPE ;
	
typedef enum
{
    RC_NONE=0,
    RC_VBR,
    RC_CBR,
    RC_CQP,
    RC_GVBR,
    RC_VBRN,
    RC_GVBRN        
} EU_RC ;
	
typedef enum
{
    UVC15_MULTI_SLICE_MBs = 0,  // Max number of MBs per slice 
    UVC15_MULTI_SLICE_BYTEs,     // Target compressed size per slice mode
    UVC15_MULTI_SLICE_NUMs,     // # of slices per frame mode
    UVC15_MULTI_SLICE_ROWs      // # of Macroblock rows per slice mode
} H264_SLICEMODE ;


typedef enum 
{
   NO_SCALING = 0,
   SCALING_15_20,
   SCALING_10_15_20,
   SCALING_BY_DESC,
   SCALING_ALL 
} H264_SCALINGMODE;
				    
typedef __packed struct
{
	MMP_USHORT	wWidth;
	MMP_USHORT	wHeight;
} EU_VDO_RES;

typedef __packed struct
{
	MMP_USHORT	wProfile;
	MMP_USHORT	wConstrainedToolset;
	MMP_USHORT	bmSetting;
}EU_PROF_TOOLSET;

typedef __packed struct
{
	MMP_USHORT	wSliceMode;
	MMP_USHORT	wSliceConfigSetting;
}EU_SLICEMODE;

typedef __packed struct
{
	MMP_USHORT	wQpPrime_I;
	MMP_USHORT	wQpPrime_P;
	MMP_USHORT	wQpPrime_B;
} EU_QP;

typedef __packed struct
{
	MMP_UBYTE	bSyncFrameType;
	MMP_USHORT	wSyncFrameInterval;
	MMP_UBYTE	bGradualDecoderRefresh;
}EU_SYNC_REF;

typedef __packed struct
{
	MMP_UBYTE	bLTRHostControl;
	MMP_UBYTE	bLTREncoderControl;
}EU_BUF_CTRL;

typedef __packed struct
{
	MMP_UBYTE	bPutAtPositionInLTRBuffer;
	MMP_UBYTE	bValidLTRs;
}EU_PIC_CTRL;

//
// TBD : NG, no I P B frame information
//
typedef __packed struct
{
    MMP_UBYTE minQP ;
    MMP_UBYTE maxQP ;
} EU_QP_RANGE ;


typedef __packed struct
{
    MMP_USHORT stream0;
    MMP_USHORT stream1;
    MMP_USHORT stream2;
    MMP_USHORT stream3;
        
} STREAM_LAYOUT ;

// Format descriptor (USB_Video_Payload_H264_1.5e.docx)
typedef __packed struct
{
    MMP_UBYTE bLength ;
    MMP_UBYTE bDescriptorType ;
    MMP_UBYTE bDescriptorSubType ;
    MMP_UBYTE bFormatIndex ;
    MMP_UBYTE bNumFrameDescriptors;
    MMP_UBYTE bDefaultFrameIndex ;
    
    
    MMP_UBYTE bMaxCodecConfigDelay ;
    MMP_UBYTE bmSupportedSliceModes;
    MMP_UBYTE bmSupportedSyncFrameTypes;
    MMP_UBYTE bResolutionScaling ;
    MMP_UBYTE Reserved1;//bSimulcastSupport ;
    MMP_UBYTE bmSupportedRateControlModes ;
    
    MMP_USHORT wMaxMBperSecOneResolutionNoScalability ;
    MMP_USHORT wMaxMBperSecTwoResolutionsNoScalability ;
    MMP_USHORT wMaxMBperSecThreeResolutionsNoScalability ;
    MMP_USHORT wMaxMBperSecFourResolutionsNoScalability ;
    
    MMP_USHORT wMaxMBperSecOneResolutionTemporalScalability ;
    MMP_USHORT wMaxMBperSecTwoResolutionsTemporalScalablility ;
    MMP_USHORT wMaxMBperSecThreeResolutionsTemporalScalability ;
    MMP_USHORT wMaxMBperSecFourResolutionsTemporalScalability ;
    
    
    MMP_USHORT wMaxMBperSecOneResolutionTemporalQualityScalability ;
    MMP_USHORT wMaxMBperSecTwoResolutionsTemporalQualityScalability;
    MMP_USHORT wMaxMBperSecThreeResolutionsTemporalQualityScalablity ;
    MMP_USHORT wMaxMBperSecFourResolutionsTemporalQualityScalability ;
    
    MMP_USHORT wMaxMBperSecOneResolutionsTemporalSpatialScalability ;
    MMP_USHORT wMaxMBperSecTwoResolutionsTemporalSpatialScalability ;
    MMP_USHORT wMaxMBperSecThreeResolutionsTemporalSpatialScalability;
    MMP_USHORT wMaxMBperSecFourResolutionsTemporalSpatialScalability ;
    
    MMP_USHORT wMaxMBperSecOneResolutionFullScalability ;
    MMP_USHORT wMaxMBperSecTwoResolutionsFullScalability ;
    MMP_USHORT wMaxMBperSecThreeResolutionsFullScalability ;
    MMP_USHORT wMaxMBperSecFourResolutionsFullScalability ;
    
} H264_FORMAT_DESC ;

typedef __packed struct 
{
	MMP_USHORT		wSARwidth;
	MMP_USHORT		wSARheight;
	MMP_USHORT		wProfile;
	MMP_UBYTE		bLevelIDC;
	MMP_USHORT		wConstrainedToolset;
	MMP_USHORT		bmCapabilities;
	MMP_ULONG		bmSupportedUsages;		
	MMP_ULONG		bmSVCCapabilities;
	MMP_ULONG		bmMVCCapabilities;
} H264_FRAME_DESC;

#if 0
typedef __packed struct
{
    MMP_UBYTE doncare[34] ;
    MMP_UBYTE bUsage ;
    MMP_UBYTE bBitDepthLuma ;               //[Not supported in Window 8]
    MMP_UBYTE bmSetting;                    //[Not supported in Window 8]
    MMP_UBYTE bMaxNumberOfRefFramesPlus1;   //[Not supported in Window 8]
    MMP_USHORT bmRateControlModes;
    MMP_USHORT bmLayoutPerStream[4] ;
} H264_COMMIT_DESC;
#endif

typedef __packed struct
{
    MMP_UBYTE bLength ;
    MMP_UBYTE bDescriptorType ;
    MMP_UBYTE bDescriptorSubType;
    MMP_UBYTE bUnitID ;
    MMP_UBYTE bSourceID ;
    MMP_UBYTE iH264Encoding ;
    MMP_UBYTE bControlSize ;
    MMP_USHORT bmControls ;
    MMP_USHORT bmControlsRuntime ;
} EU_DESC ;


typedef  struct
{
    // H264 video format descriptor
    //MMP_UBYTE       *format_info ;
    // H264 video frame descriptor
    //MMP_UBYTE       *frame_info ; // point to frame descriptor structure for current frame index
    // H264 probe & commit
    //MMP_UBYTE       *commit_info ;// point to commit descriptor structure for current commit
    // H264 encoding unit structure
    //MMP_UBYTE       *eu_info ;  // point to encoding unit descriptor structure
    // The Encoding Unit control sets
    MMP_USHORT      wLayerOrViewID ;
    EU_VDO_RES      res ;
    EU_PROF_TOOLSET toolSet;
    MMP_ULONG       dwFrameInterval ;
    EU_SLICEMODE    sliceMode ;
    EU_RC           bRateControlMode;
    MMP_ULONG       dwAverageBitRate[MAX_NUM_TMP_LAYERS]; // can controlled by layer
    MMP_ULONG       dwCPBsize[MAX_NUM_TMP_LAYERS] ;
    MMP_ULONG       dwPeakBitRate ;
    EU_QP           QPCtrl[MAX_NUM_TMP_LAYERS];
    EU_SYNC_REF     syncRefFrameCtrl;
    MMP_UBYTE       bPriority[MAX_NUM_TMP_LAYERS] ;
    MMP_UBYTE       bUpdate ;   // 0 -> stop , 1 -> start
    MMP_UBYTE       bUpdatetId; // Which layer should start/stop
    MMP_UBYTE       bUpdateLayerMap[MAX_NUM_TMP_LAYERS];
    // TBD      
    EU_QP_RANGE     qpRange[MAX_NUM_TMP_LAYERS];
    MMP_UBYTE       bSEIBitmap[8] ;
    MMP_UBYTE       bLevelIDC ;
} UVC15_VIDEO_CONFIG ;

#define SUPPORTED_USAGES				((1 << RT_UCCONFIG_MODE0) | (1 << RT_UCCONFIG_MODE1) )
#define BM_CAPABILITIES					((1 << 0) | (1<< 5))		// no picture recording			
#define BM_SVCCAPABILITIES				1
#define BM_MVCCAPABILITIES				0


void UVC15_CommitH264FormatDesc(MMP_UBYTE *formatdesc);
void UVC15_CommitH264FrameDesc(MMP_UBYTE sId,MMP_UBYTE *framedesc);
void UVC15_CommitH264EUDesc(MMP_UBYTE *eudesc);
void UVC15_CommitH264CommitDesc(MMP_UBYTE *commitdesc);
H264_FORMAT_DESC *UVC15_GetH264FormatDesc(void);
H264_FRAME_DESC *UVC15_GetH264FrameDesc(MMP_UBYTE sId);
VIDEO_PROBE *UVC15_GetH264CommitDesc(void);
EU_DESC *UVC15_GetH264EUDesc(void) ;



UVC15_VIDEO_CONFIG *UVC15_GetH264StartConfig(MMP_USHORT streamid);
MMP_USHORT UVC15_SetH264StartMode(UVC15_VIDEO_CONFIG *config,MMP_ULONG setbitmap);
// CS1:	Select Layer Control
MMP_USHORT UVC15_SetH264LayerId(MMP_USHORT curLayerId) ;
// CS2: Profile & Toolset Control
MMP_USHORT UVC15_SetH264ProfileToolSet(EU_PROF_TOOLSET *toolset) ;
// CS3: Video Resolution Control
MMP_USHORT UVC15_SetH264Resolution(EU_VDO_RES *res);
// CS4: min. frame interval Control
MMP_USHORT UVC15_SetH264FrameRate(MMP_ULONG frameInterval) ;
// CS5: 	Slice Mode Control	
MMP_USHORT UVC15_SetH264SliceMode(EU_SLICEMODE *sliceMode);
// CS6: Rate control Mode
MMP_USHORT UVC15_SetH264RateControlMode( EU_RC rc);
// CS7: Average Bit Rate Control
MMP_USHORT UVC15_SetH264BitRate( MMP_ULONG br);
// CS8:  	CPB Size Control
MMP_USHORT UVC15_SetH264CPBSize(MMP_ULONG cpbsize) ;
// CS9: 	Peak bit rate Control	
MMP_USHORT UVC15_SetH264PeakBitRate(MMP_ULONG pbr) ;
// CS10: Quantization Parameter CTRL
MMP_USHORT UVC15_SetH264QP(EU_QP *qp);
//CS11: Sync Ref frame control
MMP_USHORT UVC15_SetH264SyncRefFrame(EU_SYNC_REF *sync);
MMP_USHORT UVC15_SetH264QPRange(EU_QP_RANGE *qprange) ;
MMP_USHORT UVC15_SetH264SEIMessage(MMP_UBYTE *bitmap) ;
MMP_USHORT UVC15_SetH264LevelIDC(MMP_UBYTE levelidc);
MMP_USHORT UVC15_SetH264EntropyMode(H264_ENTROPYMODE entropymode ,MMP_USHORT profile);
MMP_USHORT UVC15_SetH264StartBitmap(MMP_ULONG bitmap) ;
MMP_ULONG UVC15_GetH264StartBitmap(void);
MMP_USHORT UVC15_SetH264StartStopLayer(MMP_UBYTE start);
MMP_USHORT UVC15_GetH264StartStopLayer(MMP_BOOL start[],MMP_UBYTE *stream_bitmap,MMP_UBYTE layer_dependents[]) ;
MMP_USHORT UVC15_SetH264PriorityID(MMP_UBYTE pid);
MMP_USHORT UVC15_SetMultiplexedH264PriorityId(void) ;
#endif		// #(ENCODING_UNIT == 1)
#endif



