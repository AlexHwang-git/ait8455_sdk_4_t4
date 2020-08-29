#include "mmpf_usbuvch264.h"

#include "mmpf_usbuvc15h264.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbvend.h"
#include "pcam_msg.h"
#include "mmpf_mp4venc.h"
#include "mmpf_h264enc.h"
#include "lib_retina.h"
#include "mmps_3gprecd.h"

#define CBR_EN      (H264_CBR_PADDING_EN)

MMP_USHORT GetH264QPRange(MMP_UBYTE sid,MMP_UBYTE tid,EU_QP_RANGE *qpI,EU_QP_RANGE *qpP,EU_QP_RANGE *qpB);

static MMP_USHORT SetH264EntropyMode(MMP_UBYTE sid,H264_ENTROPYMODE entropymode ,MMP_USHORT profile);
static MMP_USHORT SetH264ProfileToolSet(MMP_UBYTE sid,EU_PROF_TOOLSET *toolset,MMP_UBYTE levelidc);
static MMP_USHORT SetH264FrameRate(MMP_UBYTE sId,MMP_ULONG frameInterval);
static MMP_USHORT SetH264RateControlMode(MMP_UBYTE sid, EU_RC rc);
/*static*/ MMP_USHORT SetH264BitRate(EU_RC rc,MMP_UBYTE sid,MMP_UBYTE tid,MMP_UBYTE cur_layers,MMP_ULONG *br);
static MMP_USHORT SetH264CPBSize(EU_RC rc,MMP_UBYTE sid,MMP_UBYTE tid,MMP_UBYTE cur_layers,MMP_ULONG *cpbsize,MMP_ULONG *br);
static MMP_USHORT SetH264QP(MMP_UBYTE sid, MMP_UBYTE tid,EU_QP *qp);
MMP_USHORT SetH264QPRange(MMP_UBYTE sid,MMP_UBYTE tid,EU_QP_RANGE *qpI,EU_QP_RANGE *qpP,EU_QP_RANGE *qpB);
static MMP_USHORT SetH264SyncRefFrame(MMP_UBYTE sid,EU_SYNC_REF *syncframe);
static MMP_USHORT SetH264SEIMessage(MMP_UBYTE sid,MMP_UBYTE *bitmap) ;
static MMP_USHORT SetH264SliceMode(MMP_UBYTE sid,EU_SLICEMODE *sliceMode);

extern MMP_USHORT  UVCTxFIFOSize[] ;

#if (ENCODING_UNIT == 1)
//---------------------------------------------------//
//-				Encoding Unit parameters			-//
//---------------------------------------------------//

	//-------------------------------------------//
	//-		CS1:	Select Layer Control		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_SELECT_LAYER_CONTROL)            
	MMP_USHORT		gw_EU_SelectLayer = SVC_CUR_ID ;
	VC_CMD_CFG  	EU_SELECT_LAYER_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_LEN_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    2,1,
	    0x0001,(MMP_ULONG)SVC_DEF_ID,(MMP_ULONG)SVC_MIN_ID,(MMP_ULONG)SVC_MAX_ID,(MMP_ULONG)SVC_CUR_ID
	} ;
#endif  

	//-------------------------------------------//
	//-		CS2: Profile & Toolset Control		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_PROFILE_TOOLSET_CONTROL)          
	
	EU_PROF_TOOLSET		gs_EU_ProfToolsetCur = 
	{
	    CONSTRAINED_HIGH_P,
	    1,    /// UC Constrained Hight Toolset 
	    0x20, // No picture reordering
	    
	} ;
	
	VC_CMD_CFG  		EU_PROFILE_TOOLSET_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD |  CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    sizeof(EU_PROF_TOOLSET), 1,
	    1, (MMP_ULONG)&gs_EU_ProfToolsetCur, (MMP_ULONG)&gs_EU_ProfToolsetCur, (MMP_ULONG)&gs_EU_ProfToolsetCur, (MMP_ULONG)&gs_EU_ProfToolsetCur
	} ;
#endif            

	//-------------------------------------------//
	//-		CS3: Video Resolution Control		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_VIDEO_RESOLUTION_CONTROL)      
	MMP_ULONG		gs_EU_VideoRes=EU_RES_CUR;
	VC_CMD_CFG  	EU_VIDEO_RESOLUTION_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD  | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    sizeof(EU_VDO_RES), 1,
	    0x0001, (MMP_ULONG)EU_RES_DEF, (MMP_ULONG)EU_RES_MIN, (MMP_ULONG)EU_RES_MAX, (MMP_ULONG)EU_RES_CUR
	} ;
#endif 

	//-------------------------------------------//
	//-		CS4: min. frame interval Control	-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_MIN_FRAME_INTERVAL_CONTROL)         
	MMP_ULONG		gl_EU_minFrmIntval =  EU_FRAME_INTERVAL(30);
	VC_CMD_CFG  	EU_FRAME_INTERVAL_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD ),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    4, 1,
	    1, EU_FRAME_INTERVAL(30) , EU_FRAME_INTERVAL(30), EU_FRAME_INTERVAL(5), EU_FRAME_INTERVAL(30)
	} ;

#endif            


	//-------------------------------------------//
	//-		CS5: 	Slice Mode Control			-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_SLICE_MODE_CONTROL)         
	EU_SLICEMODE	gs_EU_SliceMode = EU_SLICE_MODE_DEF ;
	VC_CMD_CFG  	EU_SLICE_MODE_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD  | CAP_GET_CUR_CMD| CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD ),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    sizeof(EU_SLICEMODE), 1,
	    0x0001, (MMP_ULONG)EU_SLICE_MODE_DEF, (MMP_ULONG)EU_SLICE_MODE_MIN, (MMP_ULONG)EU_SLICE_MODE_MAX, (MMP_ULONG)EU_SLICE_MODE_DEF
	} ;
#endif            

	//-------------------------------------------//
	//-		CS6: Rate control Mode 				-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_RATE_CONTROL_MODE_CONTROL)         
	MMP_UBYTE		gb_EU_RateCtrl = RC_VBR;
	VC_CMD_CFG  	EU_RATE_CTRL_MODE_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD ),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    1, 1,
	    1, RC_VBR , RC_VBR, RC_GVBR, RC_VBR // no CBR mode, CBR mode in UVC1.5 should need exactly bitrate
	} ;

#endif

	//-------------------------------------------//
	//-		CS7: Average Bit Rate Control		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_AVERAGE_BITRATE_CONTROL)         
	MMP_ULONG		gl_EU_AvgBitRate = EU_BR_DEF ;
	VC_CMD_CFG  	EU_AVG_BIT_RATE_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_RES_CMD ),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    4, 1,
	    EU_BR_RES, EU_BR_DEF, EU_BR_MIN, EU_BR_MAX, EU_BR_DEF
	} ;
#endif            

	//-------------------------------------------//
	//-		CS8:  	CPB Size Control			-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_CPB_SIZE_CONTROL)    
	#define CPBSIZE_MIN			1000     // TBD
	#define CPBSIZE_MAX			12000    // TBD
	#define CPBSIZE_DEF			1000     // TBD
	#define CPBSIZE_RES         1000
	
	MMP_ULONG		gl_EU_CPBSize=CPBSIZE_DEF;
	VC_CMD_CFG  	EU_CPB_SIZE_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_RES_CMD ),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    4, 1,
	    CPBSIZE_RES, CPBSIZE_DEF, CPBSIZE_MIN, CPBSIZE_MAX, CPBSIZE_DEF	// 
	} ;
#endif

		//-------------------------------------------//
		//-		CS9: 	Peak bit rate Control		-//
		//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_PEAK_BIT_RATE_CONTROL)         
	
	MMP_ULONG		gl_EU_PeakBitRate = EU_PBR_DEF ;
	VC_CMD_CFG  	EU_PEAK_BIT_RATE_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    4, 1,
	    1, EU_PBR_DEF, EU_PBR_MIN, EU_PBR_MAX, EU_PBR_DEF
	} ;

#endif            

	//-------------------------------------------//
	//-		CS10: Quantization Parameter CTRL	-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_QUANTIZATION_PARAMS_CONTROL)         
	EU_QP			gs_EU_QuantParam  ; // TBD
	VC_CMD_CFG  	EU_QUANTIZATION_PARAMS_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_DEF_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_RES_CMD ),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    sizeof(EU_QP), 1,
	    0x0001, (MMP_ULONG)&gs_EU_QuantParam, (MMP_ULONG)&gs_EU_QuantParam, (MMP_ULONG)&gs_EU_QuantParam, (MMP_ULONG)&gs_EU_QuantParam
	} ;

#endif    
	//-------------------------------------------//
	//-		CS11: Sync & LTR frame CTRL     	-//
	//-------------------------------------------//

#if EU_CS_VAL & (1 << EU_SYNC_REF_FRAME_CONTROL)         
	MMP_ULONG		gs_EU_Sync_RefFrm = EU_SYNC_REF_FRAME(SYNC_IDR_WITH_SPSPPS,45*1000,0) ;
	VC_CMD_CFG  	EU_SYNC_MODE_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD  | CAP_GET_CUR_CMD  | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    sizeof(EU_SYNC_REF), 1,
	    0x0001, (MMP_ULONG)EU_SYNC_REF_FRAME(SYNC_IDR_WITH_SPSPPS,45*1000,0), (MMP_ULONG)EU_SYNC_REF_FRAME(SYNC_IDR_WITH_SPSPPS,45*1000,0), (MMP_ULONG)EU_SYNC_REF_FRAME(SYNC_IDR_WITH_SPSPPS,45*1000,0), (MMP_ULONG)EU_SYNC_REF_FRAME(SYNC_IDR_WITH_SPSPPS,45*1000,0)
	} ;
	
#endif	//EU_SYNC_REF_FRAME_CONTROL
        

#if EU_CS_VAL & (1 << EU_PRIORITY_ID_CONTROL)         
	//-------------------------------------------//
	//-		CS12: Priority ID           		-//
	//-------------------------------------------//
	MMP_UBYTE		gb_EU_PriorityId = EU_PRIO_ID_CUR;
	VC_CMD_CFG  	EU_PRIORITY_ID_CTRL_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD |CAP_GET_INFO_CMD | CAP_GET_LEN_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    1, 1,
	    1, EU_PRIO_ID_DEF , EU_PRIO_ID_MIN, EU_PRIO_ID_MAX , EU_PRIO_ID_CUR
	} ;

#endif

	//-------------------------------------------//
	//-		CS13: Start/Stop Layer Control		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_START_OR_STOP_LAYER_CONTROL)         
	MMP_UBYTE		gb_EU_StartLayerCtrl;
	VC_CMD_CFG  	EU_START_STOP_LAYER_CTRL_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD |  CAP_GET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    1, 1,
	    1, 0 , 0, 1, 0
	} ;
#endif

	//-------------------------------------------//
	//-		CS0F: Start/Stop Layer Control		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_LEVEL_IDC_LIMIT_CONTROL)
	MMP_UBYTE		gb_EU_LevelIDCCtrl = EU_LEVELIDC_CUR ;
	VC_CMD_CFG  	EU_LEVEL_IDC_LIMIT_CTRL_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD |  CAP_GET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_DEF_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    1, 1,
	    1, EU_LEVELIDC_DEF , EU_LEVELIDC_MIN, EU_LEVELIDC_MAX , EU_LEVELIDC_CUR
	} ;
#endif


	//-------------------------------------------//
	//-		CS11: QP Range Control      		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_QP_RANGE_CONTROL)
    MMP_USHORT     gs_EU_QpRangeCtrl = EU_QP_RANGE_M(14,42);
	VC_CMD_CFG  	EU_QP_RANGE_CTRL_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD |  CAP_GET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_DEF_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    sizeof(EU_QP_RANGE), 1,
	    1, EU_QP_RANGE_M(14,42) ,  EU_QP_RANGE_M(14,42),  EU_QP_RANGE_M(14,42) ,  EU_QP_RANGE_M(14,42)
	} ;
    
#endif


	//-------------------------------------------//
	//-		CS11: SEI payload type control		-//
	//-------------------------------------------//
#if EU_CS_VAL & (1 << EU_SEI_PAYLOADTYPE_CONTROL)
    MMP_UBYTE       gb_EU_SEIPayloadTypeCtrl[8] = {0x3,0,0,0,0,0,0,0} ;
	VC_CMD_CFG  	EU_SEI_PAYLOAD_TYPE_CTRL_CFG = // (Enabled) 
	{
	    ( CAP_SET_CUR_CMD |  CAP_GET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_LEN_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD  | CAP_GET_DEF_CMD),
	    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
	    sizeof(gb_EU_SEIPayloadTypeCtrl), 1,
	    1, (MMP_ULONG)gb_EU_SEIPayloadTypeCtrl ,(MMP_ULONG)gb_EU_SEIPayloadTypeCtrl,(MMP_ULONG)gb_EU_SEIPayloadTypeCtrl ,(MMP_ULONG)gb_EU_SEIPayloadTypeCtrl
	} ;
#endif



//MMP_UBYTE			gb_EU_PriorityID;
//EU_BUF_CTRL			gs_EU_BufCtrl;
//EU_PIC_CTRL			gs_EU_PicCtrl;


#endif	//#if (ENCODING_UNIT == 1)

//
// An example for H264 format descriptor (  )
//

H264_FORMAT_DESC gsH264FormatDescEx = 
{
    52,     // Length
    0x24,   // bDescriptorType
    0x13,   // bDescriptorSubType
    0,      // bFormatIndex
    0,      // bNumFrameDescriptors
    0,      // bDefaultFrameIndex
    2,      // bMaxCodecConfigDelay
    (1<<UVC15_MULTI_SLICE_NUMs) | (1<<UVC15_MULTI_SLICE_ROWs ),//bmSupportedSliceModes
    (1<<SYNC_IDR_WITH_SPSPPS) | ( 1<<SYNC_I_WITH_SPSPPS),        //bmSupportedSyncFrameTypes
    (1<<SCALING_BY_DESC),       //bResolutionScaling
    1,      //bSimulcastSupport
    #if CBR_EN
    (1<<RC_CBR) | (1<<RC_VBR) | (1 << RC_CQP) | (1 <<RC_GVBR ),        //bmSupportedRateControlModes
    #else
    (1<<RC_VBR) | (1 << RC_CQP) | (1 <<RC_GVBR ),        //bmSupportedRateControlModes
    #endif
    (1920 * 1088 * 30) / 256000 ,// wMaxMBperSecOneResolutionNoScalability
    ((1280 * 720 + 640 *360 ) * 30 ) / 256000, //wMaxMBperSecTwoResolutionsNoScalability
    0,      // wMaxMBperSecThreeResolutionsNoScalability        
    0,      // wMaxMBperSecFourResolutionsNoScalability

    (1920 * 1088 * 30) / 256000 ,// wMaxMBperSecOneResolutionTemporalScalability
    ((1280 * 720 + 640 *360 ) * 30 ) / 256000, //wMaxMBperSecTwoResolutionsTemporalScalability
    0,      // wMaxMBperSecThreeResolutionsTemporalScalability        
    0,      // wMaxMBperSecFourResolutionsTemporalScalability
    0,0,0,0,
    0,0,0,0,
    0,0,0,0      
} ;


//
// An example for H264 frame descriptor ( High Profile 720p )
//
H264_FRAME_DESC gsH264FrameDescEx[MAX_NUM_ENC_SET] =
{

    {
        1,      						// wSARwidth
        1,      						// wSARheight
        CONSTRAINED_BASELINE_P, 		            // wProfile 
        40,     						// bLevelIDC
        0,								// wConstrainedToolset;
    	// bmCapabilities 
       (1 << SEPERATE_QP_LC) | (1 << NO_PIC_REORDERING), // High profile using cabac only
    	// bmSupportedUsage
        (1 << RT_UCCONFIG_MODE0) | (1 << RT_UCCONFIG_MODE1) | (1 <<STORAGE_MODE_IP),
        MAX_NUM_TMP_LAYERS-1, // support SVC 3 temperal layer (maximum layers - 1)
        0, // MVC
    },
    
    {
        1,      						// wSARwidth
        1,      						// wSARheight
        CONSTRAINED_BASELINE_P, 		            // wProfile 
        40,     						// bLevelIDC
        0,								// wConstrainedToolset;
    	// bmCapabilities 
       (1 << SEPERATE_QP_LC) | (1 << NO_PIC_REORDERING), // High profile using cabac only
        
    	// bmSupportedUsage
        (1 << RT_UCCONFIG_MODE0) | (1 << RT_UCCONFIG_MODE1) | (1 <<STORAGE_MODE_IP),
        
        MAX_NUM_TMP_LAYERS-1, // support SVC 3 temperal layer (maximum layers - 1)
        0, // MVC
    }
     
} ;


// An example for Encoding unit descriptor
// Hardcoding for test only
EU_DESC   gsEUDescCfgEx = 
{
// No reference in Test
    0,
    0,
    0x07,
    0,
    0,
    0,
// Refer in Test    
    2,
    EU_CS_VAL    >> 1,  // initial EU control
    EU_CS_VAL_RT >> 1// runtime EU control
} ; 

// an example for commit descriptor
VIDEO_PROBE gsH264CommitDescEx = 
{ 
     0,0,
     EU_FRAME_INTERVAL(30) & 0xFFFF,
     EU_FRAME_INTERVAL(30) >> 16,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     
     // UCConfig Mode 1
     (RT_UCCONFIG_MODE1 + 1 ),
    0, // bBitDepthLuma
    (1 << SEPERATE_QP_LC) | (1 << NO_PIC_REORDERING), // bmSetting
    0, //bMaxNumberOfRefFramesPlus1
    COMMIT_RC(RC_GVBR,RC_GVBR,RC_NONE,RC_NONE) ,//bmRateControlModes
    COMMIT_LAYOUT(3), //bmLayoutPerStream[0] stream 0 is 2 layer test mode
        
    COMMIT_LAYOUT(3), //bmLayoutPerStream[1] stream 1 is 2 layer test mode
    COMMIT_LAYOUT(0),
    COMMIT_LAYOUT(0),
} ;

#define H264_S1_BITRATE (4000*1000)
#define H264_S2_BITRATE (1500*1000)

UVC15_VIDEO_CONFIG gsCurUVC15H264Config[MAX_NUM_ENC_SET] =
{
    {
        SVC_LAYER_ID(0,0,0,0,0), // layer or view id
        {1280,720}, // res
        { CONSTRAINED_BASELINE_P,0,0}, // tool set , EU_PROF_TOOLSET.wConstrainedToolSet = 0
        333333, // frame interval
        #if (CHIP==VSN_V2)||(CHIP==VSN_V3)
        {UVC15_MULTI_SLICE_NUMs ,1 }, //  slice mode
       // {0,500}, // Macroblock mode
        #endif
        {RC_GVBR}, // ratecontrol mode
        
        {H264_S1_BITRATE,H264_S1_BITRATE,H264_S1_BITRATE}, // average bitrate
        
        //(500 *1000*1000)/ 16000,         // CPB size  = wLeakyBucketSize * dwAverageBitRate / 1000*16
        {EU_LBSize2CPBSize(500,H264_S1_BITRATE),EU_LBSize2CPBSize(500,H264_S1_BITRATE),EU_LBSize2CPBSize(500,H264_S1_BITRATE)},
        H264_S1_BITRATE*2, //peak bitrate
        { // QPCtrl
            EU_QP_ALL(35,0,0),
            EU_QP_ALL(35,0,0),
            EU_QP_ALL(35,0,0),
        },
        {SYNC_IDR_WITH_SPSPPS, 1000,0}, // sync frame
        {0,1,2}, // priority id
        1, // bUpdate
        MAX_NUM_TMP_LAYERS-1, // start / stop layer id   
        {1,1,1}, // LayerMap for bUpdate
        // QP default range.
        {
            { 0,0 },
            { 0,0 },
            { 0,0 },
        }        
    },
    {
        SVC_LAYER_ID(0,0,0,1,0), // layer or view id
        
        {640,360}, // res
        { CONSTRAINED_BASELINE_P,0,0}, // tool set , EU_PROF_TOOLSET.wConstrainedToolSet = 0
        333333, // frame interval
        #if (CHIP==VSN_V2)||(CHIP==VSN_V3)
        {UVC15_MULTI_SLICE_NUMs,1 }, // Macroblocks mode
        #endif
        {RC_GVBR}, // ratecontrol mode
        {H264_S2_BITRATE,H264_S2_BITRATE,H264_S2_BITRATE}, // average bitrate
        //500 *500*1000)/ 16000,         // CPB size  = wLeakyBucketSize * dwAverageBitRate / 1000*16
        {EU_LBSize2CPBSize(500,H264_S2_BITRATE),EU_LBSize2CPBSize(500,H264_S2_BITRATE),EU_LBSize2CPBSize(500,H264_S2_BITRATE)},
        H264_S2_BITRATE*2, //peak bitrate
        { 
            EU_QP_ALL(35,0,0),
            EU_QP_ALL(35,0,0),
            EU_QP_ALL(35,0,0),
        },
        {SYNC_IDR_WITH_SPSPPS, 10*1000,0}, // sync frame
        {0,1,2}, // priority id
        1, // bUpdate
        MAX_NUM_TMP_LAYERS-1, // start / stop layer id    
        {1,1,1}, // LayerMap for bUpdate
        // QP default range.
        {
            { 0,0 },
            { 0,0 },
            { 0,0 },
        }        
    }
} ;



MMP_ULONG  glH264StartCtrlBitmap = 0 ;
MMP_USHORT gsCurLayerOrViewId = SVC_LAYER_ID(0,0,SVC_WILDCARD_MASK,SIM_WILDCARD_MASK,0);


static MMP_UBYTE *gbCurH264FormatDesc ;
static MMP_UBYTE *gbCurH264FrameDesc[MAX_NUM_ENC_SET]  ;
static MMP_UBYTE *gbCurH264CommitDesc ;
static MMP_UBYTE *gbCurH264EUDesc     ;
/*
The following 3 APIs called by USB layer for H264 starting information
*/
void UVC15_CommitH264FormatDesc(MMP_UBYTE *formatdesc)
{
    if(formatdesc) {
        gbCurH264FormatDesc = formatdesc ;
    } else {
        // testcode
       gbCurH264FormatDesc = (MMP_UBYTE *)&gsH264FormatDescEx ;
    }

}

H264_FORMAT_DESC *UVC15_GetH264FormatDesc(void)
{
    return (H264_FORMAT_DESC *)gbCurH264FormatDesc ;
}


/*
The following 3 APIs called by USB layer for H264 starting information
*/
// 
// Only be called once after commit.
// For simulcast, need give different stream id
//
void UVC15_CommitH264FrameDesc(MMP_UBYTE sId,MMP_UBYTE *framedesc)
{

    UVC15_VIDEO_CONFIG *config ;
    config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
    
    if(framedesc) {
        gbCurH264FrameDesc[sId] = framedesc ;
    } else {
        // testcode
        gbCurH264FrameDesc[sId] = (MMP_UBYTE *)&gsH264FrameDescEx[sId] ;
    }
    
    // Assign to config, make startup mode more clean,
    config->toolSet.wProfile = UVC15_GetH264FrameDesc(sId)->wProfile ;
    config->toolSet.wConstrainedToolset = 0 ;
    config->bLevelIDC = UVC15_GetH264FrameDesc(sId)->bLevelIDC ;
}

H264_FRAME_DESC *UVC15_GetH264FrameDesc(MMP_UBYTE sId)
{
    return (H264_FRAME_DESC *)gbCurH264FrameDesc[sId];
}

// 
// Only be called once after commit.
//
void UVC15_CommitH264CommitDesc(MMP_UBYTE *commitdesc)
{
    UVC15_VIDEO_CONFIG *config ;
    MMP_USHORT i;
    
    if(commitdesc) {
        gbCurH264CommitDesc= (MMP_UBYTE *)commitdesc  ;  
    } else {
        // testcode
        gbCurH264CommitDesc = (MMP_UBYTE *)&gsH264CommitDescEx ;
    }
    // Assign to config, make startup mode more clean,
    for(i=0;i<MAX_NUM_ENC_SET;i++) {
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
        config->toolSet.bmSetting = UVC15_GetH264CommitDesc()->bmSetting ;
    }
}

VIDEO_PROBE *UVC15_GetH264CommitDesc(void)
{
    if(gbCurH264CommitDesc) {
        return (VIDEO_PROBE *)gbCurH264CommitDesc;
    }
    return (VIDEO_PROBE *)&gsH264CommitDescEx ;
}


void UVC15_CommitH264EUDesc(MMP_UBYTE *eudesc)
{
    if(eudesc) {
        gbCurH264EUDesc = (MMP_UBYTE *)eudesc ;  
    } else {
        // testcode
        gbCurH264EUDesc = (MMP_UBYTE *)&gsEUDescCfgEx  ;
    }
}

EU_DESC *UVC15_GetH264EUDesc(void)
{
    return (EU_DESC *)gbCurH264EUDesc;
}


MMP_USHORT UVC15_SetH264StartBitmap(MMP_ULONG bitmap)
{
    glH264StartCtrlBitmap = bitmap ;
    return PCAM_ERROR_NONE;
}


MMP_ULONG UVC15_GetH264StartBitmap(void)
{
    return glH264StartCtrlBitmap ;
}

//
// H264 Start Mode don;t support sId wildcard mask.
// Call twice if simulcast
//
MMP_USHORT UVC15_SetH264StartMode(UVC15_VIDEO_CONFIG *config,MMP_ULONG setbitmap)
{
#if LYNC_CERT_TEST_EN
extern MMP_BOOL gbLyncCertSEIOn ;
#endif

extern MMP_USHORT UVCX_SetH264ColorRangeByStream(void);
    //MMPF_VIDENC_MAX_FPS_CTL FpsCtl = {30*VIDEO_FPS_SCALE, VIDEO_FPS_SCALE};
   // MMPF_VIDENC_SLICE_CTL   SliceCtl;
    MMPF_VIDENC_QP_CTL      qc;
    MMP_UBYTE dId , qId, tId , sId ;
    H264_FORMAT_DESC *format_info ;
    H264_FRAME_DESC *frame_info ;
    //EU_DESC   *eu_info ;
    VIDEO_PROBE			*commit_info ;

    MMP_UBYTE rc ;
    
    MMP_UBYTE max_svc_layers  = 0,cur_svc_layers = 0,i,wildcard = 0;
    dbg_printf(3,"UVC15 : layerid:%x,flag:%x,(w,h):(%d,%d)\r\n",config->wLayerOrViewID,setbitmap,config->res.wWidth,config->res.wHeight);
     
    
    dId = SVC_DEPENDENCY_ID(config->wLayerOrViewID)  ;
    qId = SVC_QUALITY_ID( config->wLayerOrViewID )   ;
    tId = SVC_TEMPORAL_ID( config->wLayerOrViewID )  ;
    sId = SVC_STREAM_ID( config->wLayerOrViewID )    ;

    if(sId >= MAX_NUM_ENC_SET) {
        dbg_printf(3,"Invalid sId : %d \r\n",sId);
        return PCAM_USB_INIT_ERR ;
    }
    
    frame_info = UVC15_GetH264FrameDesc(sId); ;
    //eu_info = (EU_DESC *)config->eu_info ;
    commit_info = UVC15_GetH264CommitDesc() ;
    
    if(setbitmap==0) { 
        setbitmap = (MMP_ULONG)(1<<EU_START_INIT) ;
    }
    
    
    /*
    Determine layer wildcard mask.
    */
    max_svc_layers = (frame_info->bmSVCCapabilities & 0x7 ) + 1 ;
    cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId])  ;
    
    #if H264_SIMULCAST_EN
    if(cur_svc_layers==0) {
        cur_svc_layers = 1 ;
        dbg_printf(0,"EU:commit layout is wrong\r\n");
    }
    #endif
    
    // Only support up to 3 temperal layers
    dbg_printf(0,"[Max,Cur] SVC [%d,%d] layers\r\n",max_svc_layers,cur_svc_layers);
    
    if(max_svc_layers > MAX_NUM_TMP_LAYERS) {
        return PCAM_USB_INIT_ERR ;
    }
    if(cur_svc_layers > max_svc_layers) {
        return PCAM_USB_INIT_ERR ;
    }
    
    // Force to wildcard mask at initial time
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT)) {
        tId = SVC_WILDCARD_MASK ;
        config->wLayerOrViewID = SVC_LAYER_ID(dId , qId, tId , sId ,0);
        dbg_printf(3,"Init Layer ID:%x\r\n",config->wLayerOrViewID);
    } 
  
  
    // Set Profiletoolset
    
    #if 0 // sean@2013_07_23. old code
    
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT)) {
        EU_PROF_TOOLSET toolset ;
        toolset.wProfile = frame_info->wProfile ;
        toolset.wConstrainedToolset = 0;
        toolset.bmSetting = commit_info->bmSetting & 0x03;
        SetH264ProfileToolSet(sId,&toolset,frame_info->bLevelIDC);  
    }
    
    if(setbitmap & (MMP_ULONG)(1<< EU_PROFILE_TOOLSET_CONTROL)) {
        dbg_printf(0,"EU.Toolset.bmSetting:%x\r\n",config->toolSet.bmSetting );
        SetH264ProfileToolSet(sId,&config->toolSet,frame_info->bLevelIDC);  
    }   
    #else // new
    if(setbitmap & (MMP_ULONG)(1<< EU_START_INIT)) {
        dbg_printf(0,"EU.Toolset.bmSetting:%x\r\n",config->toolSet.bmSetting );
        SetH264ProfileToolSet(sId,&config->toolSet,config->bLevelIDC);  
    }   
    #endif
    // Set Frame Interval, TBD : need to modify for layer control
    if(setbitmap & (MMP_ULONG)(1<<EU_START_INIT)) {
		MMP_ULONG		dwFrameInterval;
		#if LYNC_CERT_TEST_EN
		dwFrameInterval = config->dwFrameInterval ;
		#else
        dwFrameInterval = commit_info->wIntervalLo | (commit_info->wIntervalHi << 16);
        #endif
        SetH264FrameRate(sId,dwFrameInterval);
    }
    
    
    // Set slice mode : Don't support EU in win8
    // And not in commit or frame descriptor 
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT)) {
        dbg_printf(3,"[EU_Init]:slicemode:%d,slice n:%d\r\n",config->sliceMode.wSliceMode,config->sliceMode.wSliceConfigSetting);
        SetH264SliceMode(sId, &config->sliceMode);
           
    }
    // Set default rate control mdoe
    if( setbitmap & (MMP_ULONG)(1 << EU_START_INIT) ) {
        rc = STR_RC(sId,commit_info->bmRateControlModes);
        dbg_printf(3,"[Commit.RateCtrl] : %d\r\n",rc);
        SetH264RateControlMode(sId,rc);
    }

    
    // Set average bitrate 
    if (setbitmap &  (MMP_ULONG)(1<< EU_START_INIT) ) {
        MMP_UBYTE i;
        MMP_UBYTE rc ;
        rc = STR_RC(sId,commit_info->bmRateControlModes);
        if(rc!=RC_CQP) {
            //dbg_printf(3,"AverBitRate:%d\r\n",config->dwAverageBitRate[0]);
            SetH264BitRate(rc,sId,tId,cur_svc_layers,config->dwAverageBitRate);
        }    
    
    }
    
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT) ) {
        
         MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_LAYERS, (void*)cur_svc_layers );
    }
    
    
    
    // set default leakybucket size (500 ms)
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT)) {
        MMP_UBYTE rc ;
        MMPF_VIDENC_LEAKYBUCKET_CTL lb;
        lb.ubLayerBitMap = 0; 
        rc = STR_RC(sId,commit_info->bmRateControlModes);
        //cur_str_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]) ;
        //dbg_printf(3,"[LeakyBucket]:%d,rc:%d\r\n",EU_DEF_LBS,rc );
       
        if(rc!=RC_CQP){
            if(tId == SVC_WILDCARD_MASK) {
                for(i=0;i < cur_svc_layers ;i++ ) {
                    lb.ubLayerBitMap |= (1 << i) ;
                    #if LYNC_CERT_TEST_EN
                    lb.ulLeakyBucket[i] = config->dwCPBsize[i] ;
                    #else
                    lb.ulLeakyBucket[i] = EU_CPBSize2LBSize( config->dwCPBsize[i] , config->dwAverageBitRate[i]); // EU_DEF_LBS;
                    #endif
                    dbg_printf(3,"LBsize[%d] : %d\r\n",i,lb.ulLeakyBucket[i]);
                }
            } else {
                lb.ubLayerBitMap |= (1 << tId) ;
                #if LYNC_CERT_TEST_EN
                lb.ulLeakyBucket[tId] = config->dwCPBsize[tId] ;
                #else
                lb.ulLeakyBucket[tId] = EU_CPBSize2LBSize( config->dwCPBsize[tId] , config->dwAverageBitRate[tId]);// EU_DEF_LBS;
                #endif
                dbg_printf(3,"LBsize[%d] : %d\r\n",tId,lb.ulLeakyBucket[tId]);
            }
        }
        
        MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_LB_SIZE, (void*)&lb);
    }

    
    // Set bUsage mode from commit
    if (setbitmap & (MMP_ULONG)(1 <<EU_START_INIT ) ) { 
        MMP_ULONG usage = commit_info->bUsage - 1 ;
        if (  ( 1 << usage )  & frame_info->bmSupportedUsages ) {
            dbg_printf(3,"[bUsage]:%x\r\n",usage+1) ;
            if( (usage==STORAGE_MODE_IP) || (usage==BROADCAST_MODE) )  {
                MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_RC_SKIPPABLE, (void*)MMP_FALSE);
            } else {
                MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_RC_SKIPPABLE, (void*)MMP_TRUE);
            } 
        }
         
    }
    
    if (setbitmap & (MMP_ULONG)(1 <<EU_START_INIT ) ) {
        // Set initial QP
        // set init QP !
        rc = STR_RC(sId,commit_info->bmRateControlModes); //sean@2013_08_23 add
        if(tId==SVC_WILDCARD_MASK) {
            qc.ubTID = TEMPORAL_ID_MASK;
        }
        else {
            qc.ubTID = tId ;// 1 << tId ;//sean@2013_08_23 , fixed bug
        }
        //sean@2013_08_23 , fixed bug RC using commit desc
        //if( (config->bRateControlMode==RC_CQP) && (commit_info->bmSetting & SEPERATE_QP_LC ) ) { // Constant QP
        if( (rc==RC_CQP)/* && (commit_info->bmSetting & (1<<SEPERATE_QP_LC) )*/ ) { // Constant QP
            MMP_UBYTE i ;
            qc.ubTypeBitMap =  (1 <<I_FRAME) | (1<<P_FRAME) ; //sean@2013_08_23 , fixed bug
            if(tId==SVC_WILDCARD_MASK) {
                for(i=0;i<cur_svc_layers;i++) {
                    qc.ubTID = i;// 1 << i; // layer id//sean@2013_08_23 , fixed bug
                    qc.ubQP[0] = EU_QP_Y(config->QPCtrl[i].wQpPrime_I);
                    qc.ubQP[1] = EU_QP_Y(config->QPCtrl[i].wQpPrime_P);
                    qc.ubQP[2] = EU_QP_Y(config->QPCtrl[i].wQpPrime_B);
                    qc.CbrQpIdxOffset[0] = EU_QP_U(config->QPCtrl[i].wQpPrime_I);
                    qc.CbrQpIdxOffset[1] = EU_QP_U(config->QPCtrl[i].wQpPrime_P);
                    qc.CbrQpIdxOffset[2] = EU_QP_U(config->QPCtrl[i].wQpPrime_B);
                    qc.CrQpIdxOffset[0] = EU_QP_V(config->QPCtrl[i].wQpPrime_I);
                    qc.CrQpIdxOffset[1] = EU_QP_V(config->QPCtrl[i].wQpPrime_P);
                    qc.CrQpIdxOffset[2] = EU_QP_V(config->QPCtrl[i].wQpPrime_B);
                    dbg_printf(0,"[CQP.Y][%d] = [%d,%d,%d]\r\n",i,qc.ubQP[0],qc.ubQP[1],qc.ubQP[2]); 
                    MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_FRM_QP, (void*)&qc);
                }
            } 
            else {
                qc.ubQP[0] = EU_QP_Y(config->QPCtrl[tId].wQpPrime_I);
                qc.ubQP[1] = EU_QP_Y(config->QPCtrl[tId].wQpPrime_P);
                qc.ubQP[2] = EU_QP_Y(config->QPCtrl[tId].wQpPrime_B);
                qc.CbrQpIdxOffset[0] = EU_QP_U(config->QPCtrl[tId].wQpPrime_I);
                qc.CbrQpIdxOffset[1] = EU_QP_U(config->QPCtrl[tId].wQpPrime_P);
                qc.CbrQpIdxOffset[2] = EU_QP_U(config->QPCtrl[tId].wQpPrime_B);
                qc.CrQpIdxOffset[0] = EU_QP_V(config->QPCtrl[tId].wQpPrime_I);
                qc.CrQpIdxOffset[1] = EU_QP_V(config->QPCtrl[tId].wQpPrime_P);
                qc.CrQpIdxOffset[2] = EU_QP_V(config->QPCtrl[tId].wQpPrime_B);
                dbg_printf(0,"[CQP.Y] = [%d,%d,%d]\r\n",qc.ubQP[0],qc.ubQP[1],qc.ubQP[2]); 
                MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_FRM_QP, (void*)&qc);
            }
        } else {
            qc.ubQP[0] = qc.ubQP[1] = qc.ubQP[2] = 34;// 35; // Init IDR is 34 for Lync Cert.
            qc.CbrQpIdxOffset[0] = qc.CbrQpIdxOffset[1] = qc.CbrQpIdxOffset[2] = 0;
            qc.CrQpIdxOffset[0] = qc.CrQpIdxOffset[1] = qc.CrQpIdxOffset[2] = 0;
            qc.ubTypeBitMap = (1 <<I_FRAME) | (1<<P_FRAME) ; /*| (1 << 2)*/; //I/P/no B frames init qp
            dbg_printf(3,"[Def Qp.Y] = [%d,%d,%d]\r\n",qc.ubQP[0],qc.ubQP[1],qc.ubQP[2]); 
            MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_FRM_QP, (void*)&qc);
        }
    }
     
    // Default is no I frame
    if (setbitmap & (MMP_ULONG)(1 <<EU_START_INIT ) ) {
        SetH264SyncRefFrame(sId,&config->syncRefFrameCtrl);
        //dbg_printf(3,"[Def.I-period] : %d,fps_res : %d,fps.inc : %d\r\n",GopCtl.usGopSize,FpsCtl.ulMaxFpsResolution,FpsCtl.ulMaxFpsIncreament);
    }

    
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT) ) {
        // Add SEI timestamp
        #if LYNC_CERT_TEST_EN
        //gbLyncCertSEIOn = 0 ;
        dbg_printf(0,"SEI : %d\r\n",gbLyncCertSEIOn);
        MMPF_H264ENC_SetSeiEnable(sId, gbLyncCertSEIOn);
        #else
        MMPF_H264ENC_SetSeiEnable(sId, MMP_TRUE);
        #endif
        //
        // This patch is for set H264 VUI full_range_flag
        //
        MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_VIDEO_FULL_RANGE, (void*)UVCX_SetH264ColorRangeByStream() );
    }
    
    // POC type 2(Lync)
    MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_POC_TYPE, (void*)MMPF_H264ENC_POC_TYPE_2);
    
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT) ) {
        // Start / stop layer
        UVC15_SetH264StartStopLayer(config->bUpdate);
    }
    
    // Assign default priority ID
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT) ) {
    
        #if 1
        if(UVCX_IsSimulcastH264()) {
            if(cur_svc_layers==1) {
            #if 1//LYNC_CERT_PATCH==0 // For simulcast MM mode
                MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_PREFIX_NALU_EN, (void*)MMP_TRUE); 
            #endif    
            }
        }
        #endif

        
    }
    if(setbitmap & (MMP_ULONG)(1 << EU_START_INIT) ) {
        MMPF_H264ENC_ENTROPY_MODE entropy ;
        EU_QP_RANGE qpI ,qpP,qpB ;
        MMPF_VIDENC_GetParameter(sId, MMPF_VIDENC_ATTRIBUTE_ENTROPY_MODE,(void *)&entropy);
        //
        // For H264 rt-mode, constrain the min QP for CABAC to 27
        // Or preview may get stuck when 
        //
        
        if((entropy==MMPF_H264ENC_ENTROPY_CABAC) && (UVCX_GetH264EncodeBufferMode()==REALTIME_MODE) ) {
            MMP_USHORT  max_mb = 8160 ;
            MMP_USHORT  cur_mb =  ( ALIGN16(config->res.wWidth) * ALIGN16(config->res.wHeight) >> 8 );
            MMP_UBYTE   new_qp_delta = ( 8160 / cur_mb ) * 2;
            if(new_qp_delta!=2) {
                if(new_qp_delta < H264E_CABAC_MIN_MB_I_QP) {
                    qpI.minQP = H264E_CABAC_MIN_MB_I_QP - new_qp_delta;
                }
                else {
                    qpI.minQP = 0;
                }
                if(new_qp_delta < H264E_CABAC_MIN_MB_P_QP) {
                    qpP.minQP = H264E_CABAC_MIN_MB_P_QP - new_qp_delta;
                }
                else {
                    qpP.minQP = 0;
                }
                if(qpI.minQP < H264E_MIN_MB_QP) {
                    qpI.minQP = H264E_MIN_MB_QP ;
                }
                if(qpP.minQP < H264E_MIN_MB_QP) {
                    qpP.minQP = H264E_MIN_MB_QP ;
                }
            } else {
                qpI.minQP = H264E_CABAC_MIN_MB_I_QP ;
                qpP.minQP = H264E_CABAC_MIN_MB_P_QP ;
            }
        
            qpI.maxQP = H264E_MAX_MB_QP ;
            qpP.maxQP = H264E_MAX_MB_QP ;
            qpB = qpP ;
            dbg_printf(0,"--cabac, new I,P min qp = %d,%d\r\n",qpI.minQP,qpP.minQP);
        } else {
            dbg_printf(0,"--%s\r\n",(entropy==MMPF_H264ENC_ENTROPY_CABAC)?"cabac":"cavlc");
            qpI.minQP = H264E_MIN_MB_QP ;
            qpI.maxQP = H264E_MAX_MB_QP ;
            qpP = qpB = qpI ;
        }
        if(tId==SVC_WILDCARD_MASK) {
        
            if( (config->qpRange[0].minQP < qpI.minQP ) || (config->qpRange[0].maxQP > qpI.maxQP) ) {
                SetH264QPRange(sId,0,&qpI,&qpP,&qpB);
            }
            else {
                SetH264QPRange(sId,0,&config->qpRange[0],&config->qpRange[0],&config->qpRange[0]);
            }
            
            if( (config->qpRange[1].minQP < qpI.minQP ) || (config->qpRange[1].maxQP > qpI.maxQP) ) {
                SetH264QPRange(sId,1,&qpI,&qpP,&qpB);
            }
            else {
                SetH264QPRange(sId,1,&config->qpRange[1],&config->qpRange[1],&config->qpRange[1]);
            }
            
            if( (config->qpRange[2].minQP < qpI.minQP ) || (config->qpRange[2].maxQP > qpI.maxQP) ) {
                SetH264QPRange(sId,2,&qpI,&qpP,&qpB);
            }
            else {
                SetH264QPRange(sId,2,&config->qpRange[2],&config->qpRange[2],&config->qpRange[2]);
            }
        } 
        else {
            if( (config->qpRange[tId].minQP < qpI.minQP ) || (config->qpRange[tId].maxQP > qpI.maxQP) ) {
                SetH264QPRange(sId,tId,&qpI,&qpP,&qpB);
            }
            else {
                SetH264QPRange(sId,tId,&config->qpRange[tId],&config->qpRange[tId],&config->qpRange[tId]);
            }
        }
    }
    //UVC15_SetH264StartBitmap(0);
    dbg_printf(3,"UVC15 : config end\r\n");
    
    return PCAM_ERROR_NONE ;

}


UVC15_VIDEO_CONFIG *UVC15_GetH264StartConfig(MMP_USHORT streamid)
{
    if(streamid > bMaxStreamId) {
        return (UVC15_VIDEO_CONFIG *)0 ;
    }   
    return (UVC15_VIDEO_CONFIG *)&gsCurUVC15H264Config[streamid] ;
}


// Has bug in simulcast H264 checking....
// CS1:	Select Layer Control
MMP_USHORT UVC15_SetH264LayerId(MMP_USHORT curLayerId)
{
    MMP_BOOL            eu_en ;
    MMP_UBYTE           i,cur_svc_layers,dId , qId, tId , sId ;
    //VIDEO_PROBE    *commit_info ;
    UVC15_VIDEO_CONFIG  *config ;
    
    dId = SVC_DEPENDENCY_ID( curLayerId) ;
    qId = SVC_QUALITY_ID( curLayerId )   ;
    tId = SVC_TEMPORAL_ID( curLayerId )  ;
    sId = SVC_STREAM_ID( curLayerId )    ;
    if(dId!=0) {
        return PCAM_USB_INIT_ERR ;
    }
    if(qId!=0) {
        return PCAM_USB_INIT_ERR ;
    }
    
    //if(tId > 1) {
    //    return PCAM_USB_INIT_ERR ;
    //}
    
    // Not a wildcard 
    if( ( sId & SIM_WILDCARD_MASK ) != SIM_WILDCARD_MASK ) {
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        sId = sId & SIM_WILDCARD_MASK ;
    } else {
    //is a wildcard mask
        sId = SIM_WILDCARD_MASK ;
    }
    /*
    if(sId==SIM_WILDCARD_MASK) {
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(0) ;
        dbg_printf(3,"wildcard sid in select layer?\r\n");
    } else {
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
    } 
    */  
    //commit_info = (VIDEO_PROBE *)UVC15_GetH264CommitDesc() ;
    // Ignore check
    //cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]) ;
    
    // Check if wildcard mask set in temporal layers
    if( (tId & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
        // Ignore check
        //if( tId >= cur_svc_layers ) {
        //    dbg_printf(3,"Invalid tid : %d >= %d\r\n",tId,cur_svc_layers);
        //    return PCAM_USB_INIT_ERR ;
        //}
        
        tId = tId & SVC_WILDCARD_MASK ;
    } else {
        tId = SVC_WILDCARD_MASK ;
    }
    gsCurLayerOrViewId = SVC_LAYER_ID(dId , qId, tId , sId , 0 );
    
    // Bug : can't assign when simulcast
    /*
    for(i=0;i< MAX_NUM_ENC_SET ;i++) {
        gsCurUVC15H264Config[i].wLayerOrViewID = gsCurLayerOrViewId ;
    }
    */
    dbg_printf(3,"[EU.SelectLayer] : (dId,qId,tId,sId)-(%d,%d,%d,%d)\r\n",dId,qId,tId,sId); 
    return PCAM_ERROR_NONE ;
}

//
//CS2: Profile & Toolset Control
// Any change to profile toolset will restart preview again.
//
MMP_USHORT UVC15_SetH264ProfileToolSet(EU_PROF_TOOLSET *toolset)
{
    MMP_BOOL ispreview;
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams = 0,i ;
    H264_FRAME_DESC *frame_info;
    MMP_UBYTE ep = usb_get_cur_image_ep(PIPE_1);
    
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;
    
    if(!toolset) {
        return PCAM_USB_INIT_ERR ;
    }
    
    if(sId==SIM_WILDCARD_MASK) {
        
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
        
    } else {
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
    }
    // Check profile rule
    if( (toolset->wProfile==BASELINE_P) || 
        (toolset->wProfile==SCALABLE_BASELINE_P) || 
        (toolset->wProfile==MULTIVIEW_HIGH_P) || 
        (toolset->wProfile==STEREO_HIGH_P) ) {
        return PCAM_USB_INIT_ERR ;
    }
    
    if(toolset->wProfile==SCALABALE_HIGH_P) {
        if(toolset->wConstrainedToolset!=1) {
            return PCAM_USB_INIT_ERR ;
        }
    }
    
    //if(toolset->bmSetting!=0x20) {
    //    return PCAM_USB_INIT_ERR ;
    //}
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&ispreview,ep); 
    if(ispreview) {
        usb_uvc_stop_preview(ep,1) ;
    }
    
    if(sId==SIM_WILDCARD_MASK) {
        for(i=0;i<streams;i++) {
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config) {
                config->toolSet = *toolset ;
                dbg_printf(3,"sid :%d,profile : %x,bmsetting :%x\r\n",i,config->toolSet.wProfile,config->toolSet.bmSetting);
                
                #if 0 // sean@2013_07_23, no need
                frame_info = (H264_FRAME_DESC *)UVC15_GetH264FrameDesc(i) ;
                if(!ispreview) {
                    ret = SetH264ProfileToolSet(i,&config->toolSet,frame_info->bLevelIDC) ;
                } else {
                    UVC15_SetH264StartBitmap(1<<EU_PROFILE_TOOLSET_CONTROL);
                }
                #endif  
            }
        }
    } else {
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        config->toolSet =  *toolset ;
        
        dbg_printf(3,"sid :%d,profile : %x,bmsetting :%x\r\n",sId,config->toolSet.wProfile,config->toolSet.bmSetting);
        
        #if 0 // sean@2013_07_23, no need
        frame_info = (H264_FRAME_DESC *)UVC15_GetH264FrameDesc(sId);
        if!ispreview) {
            ret = SetH264ProfileToolSet(sId,&config->toolSet,frame_info->bLevelIDC) ;
        } else {
            UVC15_SetH264StartBitmap(1<<EU_PROFILE_TOOLSET_CONTROL);
        }
        #endif
    }
    
    if(ispreview) {
        MMPF_Video_InitStreamSession(ep,SS_TX_STREAMING | SS_TX_BY_DMA ,UVCTxFIFOSize[ep]);
        usb_uvc_start_preview(ep,1);
    }
    return ret ; 
}


// TODO : Need to update set h264 resolution method
// from index mode to (w,h)->big change

// CS3: Video Resolution Control
MMP_USHORT UVC15_SetH264Resolution(EU_VDO_RES *res)
{
extern UVC_VSI_INFO gCurVsiInfo[] ;
//extern MMP_USHORT  UVCTxFIFOSize[] ;
    MMP_ULONG t1,t2 ,dbg_l ;
    MMP_BOOL ispreview,eu_en;
    UVC15_VIDEO_CONFIG *config ,*config_s0;
    MMP_UBYTE dId , qId, tId , sId ;
    MMP_UBYTE ep = usb_get_cur_image_ep(PIPE_1); 
    t1 = usb_vc_cur_sof();
    dbg_l = RTNA_DBG_Level(3);
    
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )     ;
    
    if(sId==SIM_WILDCARD_MASK) {
        dbg_printf(3,"Not supported wildcard\n");
        return PCAM_USB_INIT_ERR ;
        
    } else {
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
    }
    
    
    // spec. : should be multiple of 2   
    if(res->wWidth & 1) {
        return PCAM_USB_INIT_ERR ;
    }
    if(res->wHeight & 1) {
        return PCAM_USB_INIT_ERR ;
    }
    
    
    config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
    if(!config) {
        return PCAM_USB_INIT_ERR ;
    }

     
    config_s0 = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(0) ;
    // stream 1 resolution should smaller than stream 0
    if(UVCX_IsSimulcastH264()) {
        if(sId==1) {
            if( (res->wWidth > config_s0->res.wWidth) || (res->wHeight > config_s0->res.wHeight) ) {
                return PCAM_USB_INIT_ERR ;
            }
        }
    }
    config->res = *res ;
    
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&ispreview,ep); 
    
   
    if(ispreview) {
    // TODO : Restart preview with new resolution.
        //t1 = usb_vc_cur_sof(); 
        usb_uvc_stop_preview(ep,1) ;
        //t2 = usb_vc_cur_sof();
        
        //gCurVsiInfo[ep].bFrameIndex = GetResIndexBySize(res->wWidth,res->wHeight) + 1 ;
        
        //
        // Must get 1st H264 res here, 2nd H264 is updated into config->res
        //
        gCurVsiInfo[ep].bFrameIndex = GetResIndexBySize(config_s0->res.wWidth,config_s0->res.wHeight) + 1 ;
        
        USB_SetH264Resolution(PCAM_API,gCurVsiInfo[ep].bFrameIndex-1);	 
        MMPF_Video_InitStreamSession(ep,SS_TX_STREAMING | SS_TX_BY_DMA ,UVCTxFIFOSize[ep]);
        usb_uvc_start_preview(ep,1);
        dbg_printf(3,"ResIndex change to : %d\r\n",gCurVsiInfo[ep].bFrameIndex-1);  
    }
    t2 = usb_vc_cur_sof();
    RTNA_DBG_Level(dbg_l);
    dbg_printf(3,"[DynRes] : %d(%d,%d) ms\r\n",t2 - t1 ,t2,t1);
    return PCAM_ERROR_NONE ;
}


//CS4: min. frame interval Control
MMP_USHORT UVC15_SetH264FrameRate(MMP_ULONG frameInterval)
{
//extern MMP_UBYTE gbCurFrameRate[] ;
extern MMP_USHORT gsCurFrameRatex10[] ;
    MMP_BOOL eu_en ;
    
    MMP_USHORT fpsx10,ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams = 0,i;
//    MMP_UBYTE curFPS ;
    MMP_USHORT curFPSx10;
    MMP_UBYTE ep = usb_get_cur_image_ep(PIPE_1);
    
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )     ;
    
    //curFPS = gbCurFrameRate[ep] ;        
    curFPSx10 = gsCurFrameRatex10[ep] ;
    if(sId==SIM_WILDCARD_MASK) {
         streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;    
         for(i=0;i < streams ; i++) {

            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(!config) {
                return PCAM_USB_INIT_ERR ;
            }
            
            //fps = EU_FRAME_FPS(frameInterval) ;
            fpsx10 = EU_FRAME_FPSx10(frameInterval);
            
            if(fpsx10 <= curFPSx10) {
                config->dwFrameInterval = frameInterval ;
                //ret = UVC15_SetH264StartMode( config,(1<<EU_MIN_FRAME_INTERVAL_CONTROL) ) ;
                ret = SetH264FrameRate(i,config->dwFrameInterval);
            }
        }
    } else {
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
 
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        //fps = EU_FRAME_FPS(frameInterval) ;
        fpsx10 = EU_FRAME_FPSx10(frameInterval);
        // spec : can not less than the frame interval in probe/commit 
        // Can not over the fps in probe / commit
        if(fpsx10 <= curFPSx10) {
            config->dwFrameInterval = frameInterval ;
            ret = SetH264FrameRate(sId,config->dwFrameInterval);
            //return  UVC15_SetH264StartMode( config,(1<<EU_MIN_FRAME_INTERVAL_CONTROL) ) ;
        }
    }
    return ret ;
}


// CS5: 	Slice Mode Control	
MMP_USHORT UVC15_SetH264SliceMode(EU_SLICEMODE *sliceMode)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId,streams = 0,i;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )     ;
    
    if(sId==SIM_WILDCARD_MASK) {
         streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ; 
         for(i=0;i<streams;i++) {
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config && (sliceMode->wSliceMode==2) ) {
                config->sliceMode = *sliceMode ;
                //ret = UVC15_SetH264StartMode( config, (1 << EU_SLICE_MODE_CONTROL));
                ret = SetH264SliceMode(i,&config->sliceMode);
            } else {
                ret = PCAM_USB_INIT_ERR ;
            }
         }    
    
    } else {
    
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        if(sliceMode->wSliceMode != 2 ) {
            return PCAM_USB_INIT_ERR ;
        }   
        config->sliceMode = *sliceMode ;
        
        //return UVC15_SetH264StartMode( config, (1 << EU_SLICE_MODE_CONTROL));
        ret =  SetH264SliceMode(sId,&config->sliceMode);
    }
    return ret ;
}



//CS6: Rate control Mode
MMP_USHORT UVC15_SetH264RateControlMode( EU_RC rc)
{
// TODO : make a correct mapping....
    MMP_BOOL eu_en;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams = 0,i;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ; 
        for(i=0;i<streams;i++) {
            if(  (rc != RC_CBR) && (rc != RC_VBR ) && (rc != RC_CQP) && (rc != RC_GVBR) ) {
                return PCAM_USB_INIT_ERR ;
            }
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config) {
                config->bRateControlMode = rc ;
                //ret = UVC15_SetH264StartMode( config, (1 << EU_RATE_CONTROL_MODE_CONTROL)); 
                ret = SetH264RateControlMode(i,config->bRateControlMode);
            }
        }
    } else {
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        if((rc != RC_CBR) && (rc != RC_VBR ) && (rc != RC_CQP) && (rc != RC_GVBR) ) {
            return PCAM_USB_INIT_ERR ;
        }
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        config->bRateControlMode = rc ;
        //return UVC15_SetH264StartMode( config, (1 << EU_RATE_CONTROL_MODE_CONTROL));
        ret = SetH264RateControlMode(sId,config->bRateControlMode);
    }
    return ret ;
}


//CS7: Average Bit Rate Control
MMP_USHORT UVC15_SetH264BitRate(MMP_ULONG br)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    VIDEO_PROBE   *commit_info;
    
    MMP_UBYTE cur_svc_layers,dId , qId, tId , sId ,streams = 0,i;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )     ;
    commit_info = (VIDEO_PROBE *)UVC15_GetH264CommitDesc();
    
    cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]);
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ; 
        for(i=0;i<streams;i++) {

            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(!config) {
                return PCAM_USB_INIT_ERR ;
            }
            if(config->bRateControlMode!=RC_CQP) {
            
                if( (tId & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
                    config->dwAverageBitRate[tId] = br ;
                } 
                else {
                    for(i=0;i<cur_svc_layers;i++) {
                        config->dwAverageBitRate[i] = br;    
                    }
                }   
                //ret = UVC15_SetH264StartMode( config, (1 << EU_AVERAGE_BITRATE_CONTROL));
                ret = SetH264BitRate(config->bRateControlMode,i,tId,cur_svc_layers,config->dwAverageBitRate) ;
            }
        }
    
    } else {
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
       
        // Wrong State in CQP
        if(config->bRateControlMode!=RC_CQP) {
            if( (tId & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
                config->dwAverageBitRate[tId] = br ;
            } else {
                for(i=0;i<cur_svc_layers;i++) {
                    config->dwAverageBitRate[i] = br;    
                }
            }
            //return UVC15_SetH264StartMode( config, (1 << EU_AVERAGE_BITRATE_CONTROL));
            ret = SetH264BitRate(config->bRateControlMode,sId,tId,cur_svc_layers,config->dwAverageBitRate) ;
        }
    }
    return ret ;
}


// CS8:  	CPB Size Control
// TODO : wait for implement or not support
MMP_USHORT UVC15_SetH264CPBSize(MMP_ULONG cpbsize)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    VIDEO_PROBE   *commit_info;
    
    MMP_UBYTE cur_svc_layers,dId , qId, tId , sId,streams=0,i ;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;
    commit_info = (VIDEO_PROBE *)UVC15_GetH264CommitDesc();
    cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]) ;
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
        for(i=0;i<streams;i++) {
        
            
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config && (config->bRateControlMode!=RC_CQP)) {
            
                if( (tId & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
                    config->dwCPBsize[tId] = cpbsize ; 
                } else {
                    for(i=0;i<cur_svc_layers;i++) {
                        config->dwCPBsize[i] = cpbsize ;
                    }
                }
                //ret = UVC15_SetH264StartMode( config, (1 << EU_CPB_SIZE_CONTROL));   
                ret = SetH264CPBSize(config->bRateControlMode,i,tId,cur_svc_layers,config->dwCPBsize,config->dwAverageBitRate) ; 
            }
        
        }
    
    } else {
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        
        
        // Wrong State in CQP
        if(config->bRateControlMode==RC_CQP) {
            return PCAM_USB_INIT_ERR ;   
        }

        
        if( (tId & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
            config->dwCPBsize[tId] = cpbsize ;
        } 
        else {
            for(i=0;i<cur_svc_layers;i++) {
                config->dwCPBsize[i] = cpbsize ;
            }
        }
        //return UVC15_SetH264StartMode( config, (1 << EU_CPB_SIZE_CONTROL));
        ret = SetH264CPBSize(config->bRateControlMode,sId,tId,cur_svc_layers,config->dwCPBsize,config->dwAverageBitRate) ; 
    }
    return ret;
}



// CS9: 	Peak bit rate Control	
MMP_USHORT UVC15_SetH264PeakBitRate(MMP_ULONG pbr)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    VIDEO_PROBE   *commit_info;
    
    MMP_UBYTE cur_svc_layers,dId , qId, tId , sId,streams,i ;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )     ;
    
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
        for(i=0;i<streams;i++) {
            
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(!config) {
                return PCAM_USB_INIT_ERR ;
            }
            // Wrong State in CQP
             
            if(config && (config->bRateControlMode!=RC_CQP)) {
                config->dwPeakBitRate = pbr ;
                //ret = UVC15_SetH264StartMode( config, (1 << EU_PEAK_BIT_RATE_CONTROL));
            }
        }
    } else {
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        // Wrong State in CQP
        if(config->bRateControlMode==RC_CQP) {
            return PCAM_USB_INIT_ERR ;   
        }   

        commit_info = (VIDEO_PROBE *)UVC15_GetH264CommitDesc() ;
        cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]) ;
        
        if( (tId & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
            if( tId >= cur_svc_layers ) {
                return PCAM_USB_INIT_ERR ;
            }
        } 
         
        config->dwPeakBitRate = pbr ;
        //return UVC15_SetH264StartMode( config, (1 << EU_PEAK_BIT_RATE_CONTROL));
    
    }
    return ret ;
}




// CS10: Quantization Parameter CTRL
MMP_USHORT UVC15_SetH264QP(EU_QP *qp)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    VIDEO_PROBE   *commit_info;
    
    MMP_UBYTE cur_svc_layers,dId , qId, tId , sId ,streams = 0 , i,j;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )     ;
    commit_info = (VIDEO_PROBE *)UVC15_GetH264CommitDesc() ;
   
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
        for(i=0;i<streams;i++) {
            cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[i]);
            
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(!config) {
                return PCAM_USB_INIT_ERR ;
            }
            // CQP only
            if(config && (config->bRateControlMode==RC_CQP)) {
                if(tId==SVC_WILDCARD_MASK) {
                    for(j=0;j<cur_svc_layers;j++) {
                        config->QPCtrl[j] = *qp ;
                        ret = SetH264QP(i,j,&config->QPCtrl[j]);
                    }
                } 
                else {
                    config->QPCtrl[tId] = *qp ;
                    ret = SetH264QP(i,tId,&config->QPCtrl[tId]);
                }
            }
        }
    } else {
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        // Wrong State in CQP
        if(config->bRateControlMode!=RC_CQP) {
            return PCAM_USB_INIT_ERR ;   
        }    
        

        cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]);
        
        if( (tId & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
            if( tId >= cur_svc_layers ) {
                return PCAM_USB_INIT_ERR ;
            }
        }
         
        if(tId==SVC_WILDCARD_MASK) {
            for(j=0;j<cur_svc_layers;j++) {
                config->QPCtrl[j] = *qp ;
                ret = SetH264QP(sId,j,&config->QPCtrl[j]);
            }
        } 
        else {
            config->QPCtrl[tId] = *qp ;
            ret = SetH264QP(sId,tId,&config->QPCtrl[tId]);
        }
        
    }
    return ret ;
}



//CS11: Sync Ref frame control
MMP_USHORT UVC15_SetH264SyncRefFrame(EU_SYNC_REF *sync)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams=0,i;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )     ;
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
        for(i=0;i<streams;i++) {
        
            
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config) {
                config->syncRefFrameCtrl = *sync ;
                //ret = UVC15_SetH264StartMode( config, (1 << EU_SYNC_REF_FRAME_CONTROL)); 
                ret = SetH264SyncRefFrame(i,&config->syncRefFrameCtrl);  
            }
        }
    
    } else {
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        config->syncRefFrameCtrl = *sync ;
        //return UVC15_SetH264StartMode( config, (1 << EU_SYNC_REF_FRAME_CONTROL));   
        ret = SetH264SyncRefFrame(sId,&config->syncRefFrameCtrl);
    }  
    return ret ;
}


MMP_USHORT UVC15_SetH264QPRange(EU_QP_RANGE *qprange)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    VIDEO_PROBE  *commit_info ;
    
    MMP_UBYTE cur_svc_layers,dId , qId, tId , sId ,streams=0,i,j;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;
    commit_info = (VIDEO_PROBE *)UVC15_GetH264CommitDesc();
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
        for(i=0;i<streams;i++) {
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config) {
                if(tId==SVC_WILDCARD_MASK) {
                    cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[i]) ;
                    for(j = 0 ; j < cur_svc_layers ;j++) {
                        config->qpRange[j]= *qprange ;
                        ret = SetH264QPRange(i,j,&config->qpRange[j],&config->qpRange[j],&config->qpRange[j]) ;
                    }
                }
                else {
                    config->qpRange[tId] = *qprange ;
                    ret = SetH264QPRange(i,tId,&config->qpRange[tId],&config->qpRange[tId],&config->qpRange[tId]) ;
                }
            }
        }        
    } else {
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]) ;
        if(tId==SVC_WILDCARD_MASK) {
            for(j = 0 ; j < cur_svc_layers ;j++) {
                config->qpRange[j]= *qprange ;
                ret = SetH264QPRange(sId,j,&config->qpRange[j],&config->qpRange[j],&config->qpRange[j]) ;
            }
        }
        else {
            config->qpRange[tId] = *qprange ;
            ret = SetH264QPRange(sId,tId,&config->qpRange[tId],&config->qpRange[tId],&config->qpRange[tId]) ;
        }
    } 
    return ret ;
}


MMP_USHORT UVC15_SetH264SEIMessage(MMP_UBYTE *bitmap) 
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams,i,s;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
        for(s=0;s<streams;s++) {
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(s) ;
            if(!config) {
                return PCAM_USB_INIT_ERR ;
            }
            
            for(i=0;i< 8;i++) {
                config->bSEIBitmap[i] = bitmap[i] ;
            }
            ret = SetH264SEIMessage(s,config->bSEIBitmap) ;
            //ret = UVC15_SetH264StartMode( config, (1 << EU_SEI_PAYLOADTYPE_CONTROL));
        }
    } else {    
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        
        for(i=0;i< 8;i++) {
            config->bSEIBitmap[i] = bitmap[i] ;
        }
        ret = SetH264SEIMessage(sId,config->bSEIBitmap) ;
        //return UVC15_SetH264StartMode( config, (1 << EU_SEI_PAYLOADTYPE_CONTROL));   
    }
    return ret ;
}


MMP_USHORT UVC15_SetH264LevelIDC(MMP_UBYTE levelidc)
{
    MMP_BOOL eu_en ;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams,i,s;
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;
    
    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ?MAX_NUM_ENC_SET : 1 ;
        for(s=0;s<streams;s++) {
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(s) ;
            if(!config) {
                return PCAM_USB_INIT_ERR ;
            }
            config->bLevelIDC = levelidc;
            MMPF_VIDENC_SetParameter(s, MMPF_VIDENC_ATTRIBUTE_LEVEL, (void*)config->bLevelIDC);
        }
    } else {    
    
        if(sId > bMaxStreamId) {
            return PCAM_USB_INIT_ERR ;
        }
        
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(!config) {
            return PCAM_USB_INIT_ERR ;
        }
        config->bLevelIDC = levelidc;
        MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_LEVEL, (void*)config->bLevelIDC);   
    }
    return ret ;
}

//
// Set Entropy mode
//
MMP_USHORT UVC15_SetH264EntropyMode(H264_ENTROPYMODE entropymode ,MMP_USHORT profile)
{
    MMP_UBYTE sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;
    return SetH264EntropyMode(sId,entropymode,profile) ;
}


MMP_USHORT UVC15_SetH264StartStopLayer(MMP_UBYTE start)
{
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams=0 ,i;
    MMP_UBYTE cur_svc_layers ;
    VIDEO_PROBE  *commit_info ;
     
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;

    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ; 
        for(i=0;i<streams;i++) {
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config) {
            
                MMP_UBYTE layer;
                cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[i]);
                if(config->bUpdate!=start) {
                    if(tId==SVC_WILDCARD_MASK) {
                        config->bUpdatetId = cur_svc_layers - 1;
                    } 
                    else {
                        config->bUpdatetId = tId;
                    }
                    config->bUpdate = start ;
                }
                else {
                    // Start : New tId should over than old tId
                    // Otherwise, keep same.
                    if(start) {
                        if(tId > config->bUpdatetId) {
                            config->bUpdatetId = tId ;
                        }
                    }
                    else {
                        if(tId < config->bUpdatetId) {
                            config->bUpdatetId = tId ;
                        }
                    }
                }
                
                // Save to bUpdateLayerMap
                if(config->bUpdate) {// start
                    for(layer=0;layer < cur_svc_layers;layer++) {
                        if(config->bUpdatetId <=layer) {
                            config->bUpdateLayerMap[i] = 1 ;// the layer need to start
                        }
                        else {
                            config->bUpdateLayerMap[i] = 0;
                        }
                    }       
                } else {
                    for(layer=0;layer < cur_svc_layers;layer++) {
                        if(config->bUpdatetId >= layer) {
                            config->bUpdateLayerMap[i] = 1 ;// the layer need to stop
                        }
                        else {
                            config->bUpdateLayerMap[i] = 0;
                        }
                    }       
                }
                dbg_printf(3,"[EU/StartStop]-layermap : [%d,%d]-[%d,%d,%d]\r\n",
                            config->bUpdate,config->bUpdatetId,
                            config->bUpdateLayerMap[0],config->bUpdateLayerMap[1],config->bUpdateLayerMap[2] );
                
                
            }
        }
    } else {
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(config) {
            MMP_UBYTE layer;
            cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId]);
            if(config->bUpdate!=start) {
                if(tId==SVC_WILDCARD_MASK) {
                    config->bUpdatetId = cur_svc_layers - 1;
                } 
                else {
                    config->bUpdatetId = tId;
                }
                
                config->bUpdate = start ;
            }
            else {
                // Start : New tId should over than old tId
                // Otherwise, keep same.
                if(start) {
                    if(tId > config->bUpdatetId) {
                        config->bUpdatetId = tId ;
                    }
                }
                else {
                    if(tId < config->bUpdatetId) {
                        config->bUpdatetId = tId ;
                    }
                }
            }
            // Save to bUpdateLayerMap
            if(config->bUpdate) {// start
                for(layer=0;layer < cur_svc_layers;layer++) {
                    if(config->bUpdatetId <=layer) {
                        config->bUpdateLayerMap[i] = 1 ;// the layer need to start
                    }
                    else {
                        config->bUpdateLayerMap[i] = 0;
                    }
                }       
            } else {
                for(layer=0;layer < cur_svc_layers;layer++) {
                    if(config->bUpdatetId >= layer) {
                        config->bUpdateLayerMap[i] = 1 ;// the layer need to stop
                    }
                    else {
                        config->bUpdateLayerMap[i] = 0;
                    }
                }       
            }
            dbg_printf(3,"[EU/StartStop]-layermap : [%d,%d]-[%d,%d,%d]\r\n",
                        config->bUpdate,config->bUpdatetId,
                        config->bUpdateLayerMap[0],config->bUpdateLayerMap[1],config->bUpdateLayerMap[2] );
            
        }
    }
    
    return ret ;
}

MMP_USHORT UVC15_GetH264StartStopLayer(MMP_BOOL start[],MMP_UBYTE *stream_bitmap,MMP_UBYTE layer_dependents[])
{
    MMP_UBYTE streams=0 ,i;
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    
    //dbg_printf(3,"sId : %d, tId : %d\r\n",sId,tId);
    *stream_bitmap = 0 ;
    start[0]=start[1] = 0;
    layer_dependents[0] = layer_dependents[1] = 0;
    
    streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
    for(i=0;i<streams;i++) {
        *stream_bitmap |= (1 << i );
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
        if(config) {
            start[i] = config->bUpdate ;
            layer_dependents[i] = config->bUpdatetId ;
        } 
    }
    //dbg_printf(3,"*start/stop layer[%d,%d],streambitmap:%x,layers[%d,%d]\r\n",start[0],start[1],*stream_bitmap,layer_dependents[0],layer_dependents[1]);
    return ret;
}

MMP_USHORT UVC15_SetH264PriorityID(MMP_UBYTE pid)
{
    MMP_USHORT ret = PCAM_ERROR_NONE ;
    UVC15_VIDEO_CONFIG *config ;
    MMP_UBYTE dId , qId, tId , sId ,streams=0 ,cur_svc_layers = 0,i , j;
    VIDEO_PROBE  *commit_info ;
    commit_info = (VIDEO_PROBE *)UVC15_GetH264CommitDesc();
    
    dId = SVC_DEPENDENCY_ID( gsCurLayerOrViewId) ;
    qId = SVC_QUALITY_ID( gsCurLayerOrViewId )   ;
    tId = SVC_TEMPORAL_ID( gsCurLayerOrViewId )  ;
    sId = SVC_STREAM_ID( gsCurLayerOrViewId )    ;


    if(sId==SIM_WILDCARD_MASK) {
        streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ; 
        for(i=0;i<streams;i++) {
            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(i) ;
            if(config) {
                if(tId==SVC_WILDCARD_MASK) {
                    cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[i])  ;
                    for(j=0;j<cur_svc_layers;j++) {
                        config->bPriority[j] = pid ;
                        MMPF_H264ENC_SetPriority(i,j,pid);
                    }
                }
                else {
                    config->bPriority[tId] = pid ;
                    MMPF_H264ENC_SetPriority(i,tId,pid);
                }
            }
        }
    } else {
        config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        if(config) {
            if(tId==SVC_WILDCARD_MASK) {
                cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[sId])  ;
                for(j=0;j<cur_svc_layers;j++) {
                    config->bPriority[j] = pid ;
                    MMPF_H264ENC_SetPriority(sId, j ,pid);
                }
            }
            else {
                config->bPriority[tId] = pid ;
                MMPF_H264ENC_SetPriority(sId, tId ,pid);
            }
        }
    }
    return ret ;
}

MMP_USHORT UVC15_SetMultiplexedH264PriorityId(void)
{
    MMP_USHORT cur_svc_layers;
    MMP_ULONG size[bMaxStreamId+1] ;
    MMP_USHORT i,j,sid ;
    UVC15_VIDEO_CONFIG *cfg ;
    VIDEO_PROBE *commit_info = UVC15_GetH264CommitDesc() ;
    // sean@2013_11_27 fixed priority id / temporal id error when svc on
    MMP_USHORT streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
    
    for(sid=0;sid < streams ;sid++) {
        cfg = UVC15_GetH264StartConfig(sid);
        size[sid] = cfg->res.wWidth * cfg->res.wHeight ;
    }
    
    
    if(streams==1) {
        cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[0]);
        for(i=0;i<cur_svc_layers;i++) {
            MMPF_H264ENC_SetPriority(0, i , i );    
        }
    }
    else if(streams==2) {
        if(size[0] > size[1]) {
            cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[1]);
            for(i=0;i<cur_svc_layers;i++) {
                MMPF_H264ENC_SetPriority(1, i , i );    
            }
            cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[0]);
            for(j=0; j < cur_svc_layers;j++) {
                MMPF_H264ENC_SetPriority(0, j , i );
                i++ ;   
            }
        }
        else {
            cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[0]);
            for(i=0;i<cur_svc_layers;i++) {
                MMPF_H264ENC_SetPriority(0, i , i );    
            }
            cur_svc_layers = COMMIT_LAYOUT(commit_info->bmLayoutPerStream[1]);
            for(j=0; j < cur_svc_layers;j++) {
                MMPF_H264ENC_SetPriority(1, j , i ); 
                i++ ;   
            }
        }
    }        
    else {
        dbg_printf(0,"Err : Up to 2 streams now\r\n");
    }   
 
}        
//
// Set Entropy mode
//
static MMP_USHORT SetH264EntropyMode(MMP_UBYTE sId,H264_ENTROPYMODE entropymode ,MMP_USHORT profile)
{
    MMP_BOOL ispreview;
    MMPF_H264ENC_ENTROPY_MODE entroy ;
    MMP_UBYTE ep=usb_get_cur_image_ep(PIPE_1);
    
    if(entropymode==CAVLC_MODE) {
        entroy = MMPF_H264ENC_ENTROPY_CAVLC ;
    }
    else if (entropymode==CABAC_MODE) {
        entroy = MMPF_H264ENC_ENTROPY_CABAC ;
    }
    else {
        //
        // Device select
        // constrained high original is cabac
        // To follow Lync, change to cavlc
        if( (profile==CONSTRAINED_BASELINE_P)||(profile==BASELINE_P) || (profile==CONSTRAINED_HIGH_P) ) {
            entroy = MMPF_H264ENC_ENTROPY_CAVLC ;
        } else {
            entroy = MMPF_H264ENC_ENTROPY_CABAC ;

        }
    }
    
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&ispreview,ep); 
    if(ispreview) {
        usb_uvc_stop_preview(ep,1) ;
    }
    
    dbg_printf(3,"AIT sid:%d set entropy mode:%d\r\n",sId,entroy);
    
    MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_ENTROPY_MODE,(void *)entroy);
    if(ispreview) {
        MMPF_Video_InitStreamSession(ep,SS_TX_STREAMING | SS_TX_BY_DMA ,UVCTxFIFOSize[ep]);
        usb_uvc_start_preview(ep,1);
    }
    return PCAM_ERROR_NONE ; 
}


static MMP_USHORT SetH264ProfileToolSet(MMP_UBYTE sid,EU_PROF_TOOLSET *toolset,MMP_UBYTE levelidc)
{
    MMP_USHORT profile ,profile_flag;
    MMP_UBYTE  entropy ;
    
    profile = toolset->wProfile ;
    entropy = toolset->bmSetting & 0x03 ;
    dbg_printf(3,"#Profile : %x\r\n",profile); 
    if(profile==CONSTRAINED_BASELINE_P)  {
        profile_flag = CONSTRAINED_BASELINE_FLAG1;
    }       
    else if(profile==BASELINE_P) {
        profile_flag = BASELINE_P;
    } 
    else if(profile==HIGH_P) {
        profile_flag = HIGH_P;
    } 
    else if(profile==MAIN_P) {
        profile_flag = MAIN_P;
    }
    else if(profile==CONSTRAINED_HIGH_P) {
        profile_flag = CONSTRAINED_HIGH_P_FLAG1 ; //0x6430
    //    profile_flag = CONSTRAINED_HIGH_P ; // 0x640C
    }
    else {
        dbg_printf(0,"#Invalid H264 Profile:%d\r\n",profile);
        return PCAM_USB_INIT_ERR ;
    }
    profile_flag = ( (profile_flag>>8) & 0x00FF) | ( (profile_flag & 0x00FF) << 8 );
    dbg_printf(3,"<#Profile flag : %x,entropy :%d ,levelidc : %d>\r\n",profile_flag,entropy,levelidc);
    MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_PROFILE, (void*)profile_flag);
    MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_LEVEL, (void*)levelidc);
    //return UVC15_SetH264EntropyMode( (H264_ENTROPYMODE)entropy,profile) ;
    return SetH264EntropyMode(sid, (H264_ENTROPYMODE)entropy,profile) ;
}

static MMP_USHORT SetH264FrameRate(MMP_UBYTE sId,MMP_ULONG frameInterval)
{
    MMPF_VIDENC_MAX_FPS_CTL fpsctl = {30*VIDEO_FPS_SCALE, VIDEO_FPS_SCALE};
    fpsctl.ulMaxFpsIncreament = VIDEO_FPS_SCALE;
    fpsctl.ulMaxFpsResolution = VIDEO_FPS_SCALE * EU_FRAME_FPS(frameInterval);
    MMPF_VIDENC_SetParameter(sId, MMPF_VIDENC_ATTRIBUTE_MAX_FPS, (void*)&fpsctl);
    dbg_printf(0,"[FrameRate]H264(streamid:%d) FPS:%d/%d,interval:%d\r\n",sId,fpsctl.ulMaxFpsResolution, fpsctl.ulMaxFpsIncreament,frameInterval);
    return PCAM_ERROR_NONE ;
}

static MMP_USHORT SetH264RateControlMode(MMP_UBYTE sid, EU_RC rc)
{
    MMPF_VIDENC_RC_MODE_CTL RcModeCtl;
    switch(rc) {
    default:
#if CBR_EN
    case RC_CBR:
#endif    
    case RC_VBR:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
        RcModeCtl.bLayerGlobalRc = MMP_FALSE;
        break;
    case RC_CQP:
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CQP;
        RcModeCtl.bLayerGlobalRc = MMP_FALSE;
        break;
    case RC_GVBR: // Global VBR ???
        RcModeCtl.RcMode = MMPF_VIDENC_RC_MODE_CBR;
        RcModeCtl.bLayerGlobalRc = MMP_TRUE;
        break;
        
    } 
    #if CBR_EN
    if(rc==RC_CBR) {
        MMPF_VIDENC_SetParameter(sid,MMPF_VIDENC_ATTRIBUTE_CBR_PADDING,(void *)MMP_TRUE) ;
        // zero bit-stuffing buffer.
        usb_vs_zero_cbr_padding_buffer();
    }   
    else 
    #endif
    {
        MMPF_VIDENC_SetParameter(sid,MMPF_VIDENC_ATTRIBUTE_CBR_PADDING,(void *)MMP_FALSE);
    }
    MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_RC_MODE, (void*)&RcModeCtl);
    return PCAM_ERROR_NONE ;
}

/*static*/ MMP_USHORT SetH264BitRate(EU_RC rc,MMP_UBYTE sid,MMP_UBYTE tid,MMP_UBYTE cur_layers,MMP_ULONG *br)
{
    MMPF_VIDENC_BITRATE_CTL BrCtl;
    MMP_UBYTE i ;
    BrCtl.ubLayerBitMap = 0;
    if(rc==RC_CQP) {
        return PCAM_ERROR_NONE ;
    }    
    switch(rc) {
    case RC_GVBR:
        BrCtl.ubLayerBitMap |= 1;
        BrCtl.ulBitrate[0]  = br[0]; 
        dbg_printf(3,"[BitRate]GVBR( %x,%d)\r\n",BrCtl.ubLayerBitMap,BrCtl.ulBitrate[0]); 
        MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_BR, (void*)&BrCtl);
        break ;
    #if CBR_EN
    case RC_CBR:
    #endif    
    case RC_VBR:
        if( (tid & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
             BrCtl.ubLayerBitMap |= (1 << tid ) ;
             BrCtl.ulBitrate[tid] = br[tid];
        } else {
            for(i=0;i<cur_layers;i++) {
                BrCtl.ubLayerBitMap |= (1 << i ) ;
                BrCtl.ulBitrate[i]  = br[i] ;
            }
        }
        dbg_printf(3,"[BitRate]VBR: %x,%d,%d,%d)\r\n",BrCtl.ubLayerBitMap,BrCtl.ulBitrate[0],BrCtl.ulBitrate[1],BrCtl.ulBitrate[2]); 
        
        MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_BR, (void*)&BrCtl);
        break;   
    }
    return PCAM_ERROR_NONE ;
}

static MMP_USHORT SetH264CPBSize(EU_RC rc,MMP_UBYTE sid,MMP_UBYTE tid,MMP_UBYTE cur_layers,MMP_ULONG *cpbsize,MMP_ULONG *br)
{
    MMP_USHORT wLeakyBucketSize,i ;
    MMPF_VIDENC_LEAKYBUCKET_CTL lb;
    if(rc==RC_CQP) {
        return PCAM_ERROR_NONE ;
    }    
    lb.ubLayerBitMap = 0; 
    lb.ulLeakyBucket[0] = 0;
    lb.ulLeakyBucket[1] = 0;  
    lb.ulLeakyBucket[2] = 0; 
    switch(rc)
    {
    case RC_GVBR:
        wLeakyBucketSize = EU_CPBSize2LBSize(cpbsize[0] ,br[0] ) ;
        for(i=0;i < cur_layers ;i++ ) {
            lb.ubLayerBitMap |= (1 << i) ;
            lb.ulLeakyBucket[i] = wLeakyBucketSize;
        }
        MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_LB_SIZE, (void*)&lb);
        break;
    #if CBR_EN
    case RC_CBR:
    #endif    
    case RC_VBR:
        if( (tid & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
            for(i=0;i < cur_layers ;i++ ) {
                wLeakyBucketSize = EU_CPBSize2LBSize(cpbsize[i] ,br[i] ) ;
                lb.ubLayerBitMap |= (1 << i) ;
                lb.ulLeakyBucket[i] = wLeakyBucketSize;
            }
        }
        else {
            wLeakyBucketSize = EU_CPBSize2LBSize(cpbsize[tid] ,br[tid] ) ;
            lb.ubLayerBitMap |= (1 << tid) ;
            lb.ulLeakyBucket[tid] = wLeakyBucketSize;    
        }
        MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_LB_SIZE, (void*)&lb);
        break;        
    }
    return PCAM_ERROR_NONE ;
    
}

static MMP_USHORT SetH264QP(MMP_UBYTE sid, MMP_UBYTE tid,EU_QP *qp)
{
    MMPF_VIDENC_QP_CTL      qc;
    if( (tid & SVC_WILDCARD_MASK) != SVC_WILDCARD_MASK) {
        qc.ubTID = tid ;//1 << tid ; //sean@2013_08_23 fix bug
    } 
    else {
        qc.ubTID = TEMPORAL_ID_MASK;
    }
    qc.ubQP[0] =EU_QP_Y(qp->wQpPrime_I );
    qc.ubQP[1] =EU_QP_Y(qp->wQpPrime_P );
    qc.ubQP[2] =EU_QP_Y(qp->wQpPrime_B );
    qc.CbrQpIdxOffset[0] = EU_QP_U(qp->wQpPrime_I);
    qc.CbrQpIdxOffset[1] = EU_QP_U(qp->wQpPrime_P);
    qc.CbrQpIdxOffset[2] = EU_QP_U(qp->wQpPrime_B);
    qc.CrQpIdxOffset[0]  = EU_QP_V(qp->wQpPrime_I);
    qc.CrQpIdxOffset[1]  = EU_QP_V(qp->wQpPrime_P);
    qc.CrQpIdxOffset[2]  = EU_QP_V(qp->wQpPrime_B);

    qc.ubTypeBitMap = (1 << I_FRAME ) | (1 << P_FRAME ) /*| (1 << 2)*/; //I/P/no B frames init qp
    MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_FRM_QP, (void*)&qc);
    return PCAM_ERROR_NONE ;
}


MMP_USHORT SetH264QPRange(MMP_UBYTE sid,MMP_UBYTE tid,EU_QP_RANGE *qpI,EU_QP_RANGE *qpP,EU_QP_RANGE *qpB)
{
    if(qpI) {
        MMPF_VIDENC_SetQPBound(sid, tid, MMPF_3GPMGR_FRAME_TYPE_I, qpI->minQP, qpI->maxQP);
    }
    if(qpP) {    
        MMPF_VIDENC_SetQPBound(sid, tid, MMPF_3GPMGR_FRAME_TYPE_P, qpP->minQP, qpP->maxQP);    
    }
    if(qpB) {
        MMPF_VIDENC_SetQPBound(sid, tid, MMPF_3GPMGR_FRAME_TYPE_B, qpB->minQP, qpB->maxQP);    
    }
    return PCAM_ERROR_NONE ;
}

MMP_USHORT GetH264QPRange(MMP_UBYTE sid,MMP_UBYTE tid,EU_QP_RANGE *qpI,EU_QP_RANGE *qpP,EU_QP_RANGE *qpB)
{
    MMP_LONG    minIQP ,maxIQP ;
    MMP_LONG    minPQP ,maxPQP ;
    MMP_LONG    minBQP ,maxBQP ;
    
    
    MMPF_VIDENC_GetQPBound(sid, tid, MMPF_3GPMGR_FRAME_TYPE_I, &minIQP, &maxIQP); 
    qpI->minQP = (MMP_UBYTE)minIQP ;
    qpI->maxQP = (MMP_UBYTE)maxIQP ;
    MMPF_VIDENC_GetQPBound(sid, tid, MMPF_3GPMGR_FRAME_TYPE_P, &minPQP, &maxPQP);    
    qpP->minQP = (MMP_UBYTE)minIQP ;
    qpP->maxQP = (MMP_UBYTE)maxIQP ;
    MMPF_VIDENC_GetQPBound(sid, tid, MMPF_3GPMGR_FRAME_TYPE_B, &minBQP, &maxBQP); 
    qpB->minQP = (MMP_UBYTE)minIQP ;
    qpB->maxQP = (MMP_UBYTE)maxIQP ;
    
    dbg_printf(0,"#[%d]--QP I (%d->%d)\r\n",tid,qpI->minQP,qpI->maxQP); 
   // dbg_printf(0,"#[%d]QP P (%d->%d)\r\n",tid,qpP->minQP,qpP->maxQP);
   // dbg_printf(0,"#[%d]QP B (%d->%d)\r\n",tid,qpB->minQP,qpB->maxQP); 
    return PCAM_ERROR_NONE ;
}


static MMP_USHORT SetH264SyncRefFrame(MMP_UBYTE sid,EU_SYNC_REF *syncframe)
{
    MMP_BOOL update_gop = 0 ;
    MMPF_VIDENC_MAX_FPS_CTL fpsctl ;
    MMPF_VIDENC_GOP_CTL     curgopctl,newgopctl ;
    
    MMPF_VIDENC_GetParameter(sid, MMPF_VIDENC_ATTRIBUTE_MAX_FPS, (void*)&fpsctl);
    MMPF_VIDENC_GetParameter(sid, MMPF_VIDENC_ATTRIBUTE_GOP_CTL, (void*)&curgopctl);
    

    if(syncframe->wSyncFrameInterval!=0) {
        MMP_ULONG period_ms = syncframe->wSyncFrameInterval ;
        MMP_ULONG64 bit64 = (MMP_ULONG64)(((MMP_ULONG64)fpsctl.ulMaxFpsResolution*period_ms)
                            /(1000*fpsctl.ulMaxFpsIncreament));
        MMP_USHORT p_frame_c = (MMP_USHORT)bit64;
  
        newgopctl.usGopSize = p_frame_c /*+1*/;
        newgopctl.usMaxContBFrameNum = 0;
        if(syncframe->bSyncFrameType==SYNC_IDR_WITH_SPSPPS) {
            newgopctl.SyncFrameType = MMPF_VIDENC_SYNCFRAME_IDR;
        }
        if(syncframe->bSyncFrameType==SYNC_I_WITH_SPSPPS) {
            newgopctl.SyncFrameType = MMPF_VIDENC_SYNCFRAME_I ;
        }
        
        //MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_GOP_CTL, (void*)&newgopctl);

    } else {
        newgopctl.usGopSize = 0; //no regular I
        newgopctl.usMaxContBFrameNum = 0;
        if(syncframe->bSyncFrameType==SYNC_IDR_WITH_SPSPPS) {
            newgopctl.SyncFrameType = MMPF_VIDENC_SYNCFRAME_IDR;
        }
        if(syncframe->bSyncFrameType==SYNC_I_WITH_SPSPPS) {
            newgopctl.SyncFrameType = MMPF_VIDENC_SYNCFRAME_I ;
        }
        //MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_GOP_CTL, (void*)&newgopctl);
    
    }
    
    if( (curgopctl.usGopSize != newgopctl.usGopSize) || (curgopctl.SyncFrameType!=newgopctl.SyncFrameType) ) {
        dbg_printf(3,"[SyncRefFrame]New Gop Size, I-Type #:%d,%d\r\n",newgopctl.usGopSize,newgopctl.SyncFrameType);
        update_gop = 1; 
        MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_GOP_CTL, (void*)&newgopctl);
    }
    
    
        
    switch(syncframe->bSyncFrameType) {
        case SYNC_IDR_BY_DEVICE: // Reset ?
        break;
        case SYNC_IDR_WITH_SPSPPS:
        if(!update_gop) {
            MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)MMPF_VIDENC_PICCTL_IDR_RESYNC);
        }    
        break ;
        case SYNC_I_WITH_SPSPPS:
        if(!update_gop) {
            MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)MMPF_VIDENC_PICCTL_I);
        }
        break ;
    }
    return PCAM_ERROR_NONE;
}

static MMP_USHORT SetH264SEIMessage(MMP_UBYTE sid,MMP_UBYTE *bitmap)
{
    if(bitmap[0] & (MMPF_H264ENC_SEI_TYPE_BUF_PERIOD | MMPF_H264ENC_SEI_TYPE_PIC_TIMING)) {
        MMPF_H264ENC_SetSeiEnable(sid, MMP_TRUE);
    } else {
        MMPF_H264ENC_SetSeiEnable(sid, MMP_FALSE);
    }
    return PCAM_ERROR_NONE ;
}

static MMP_USHORT SetH264SliceMode(MMP_UBYTE sid,EU_SLICEMODE *sliceMode)
{
    MMPF_VIDENC_SLICE_CTL   SliceCtl;
    UVC15_VIDEO_CONFIG *config ;
    
    dbg_printf(3,"[EU_Slice]:slicemode:%d,slice n:%d\r\n",sliceMode->wSliceMode,sliceMode->wSliceConfigSetting);
    config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sid) ;
    if(sliceMode->wSliceMode==UVC15_MULTI_SLICE_NUMs) {
        MMP_BOOL slice_nr_ng = MMP_TRUE;
        if(sliceMode->wSliceConfigSetting==1) {
            SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_FRM;
            SliceCtl.ulSliceSize = 0;
            MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
        } else {
            MMP_USHORT slice_row, mb_height;
            mb_height = (config->res.wHeight + 15)>>4;
            slice_row = (mb_height+sliceMode->wSliceConfigSetting-1)/sliceMode->wSliceConfigSetting;
            if(slice_row) {
                MMP_USHORT slice_nr;
                slice_nr = mb_height/slice_row + ((mb_height%slice_row)?1:0);
                if (slice_nr == sliceMode->wSliceConfigSetting) {
                #if (CHIP==P_V2)
                    SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_ROW;
                    SliceCtl.ulSliceSize = slice_row;
                #endif
                #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
                {
                    MMP_USHORT mb_w = (config->res.wWidth + 15) >> 4 ;
                    SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_MB ;
                    SliceCtl.ulSliceSize = mb_w * slice_row ;
                }
                #endif    
                    MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
                    slice_nr_ng = MMP_FALSE;
                }
            }
            
            if (slice_nr_ng == MMP_TRUE) {
                dbg_printf(0,"#Invalid H264 Slice Mode, Unit:%d,%d\r\n",
                    sliceMode->wSliceMode, sliceMode->wSliceConfigSetting);
                return PCAM_USB_INIT_ERR ;
            }
            
        }
    }
    else if( sliceMode->wSliceMode==UVC15_MULTI_SLICE_ROWs) {
        MMP_USHORT slice_row = sliceMode->wSliceConfigSetting ;
        if(slice_row) {
        #if (CHIP==P_V2)
            SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_ROW ;
            SliceCtl.ulSliceSize = slice_row;   
        #endif
        #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
            MMP_USHORT mb_w = (config->res.wWidth + 15) >> 4 ;
            SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_MB ;
            SliceCtl.ulSliceSize = mb_w * slice_row ;
        #endif
            
            MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
        }
    
    }
    // Max macroblocks mode in each slice mode
    #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
    else if (sliceMode->wSliceMode==UVC15_MULTI_SLICE_MBs) {
        MMP_USHORT max_mbs = sliceMode->wSliceConfigSetting ;
        if(max_mbs) {
            SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_MB ;
            SliceCtl.ulSliceSize = max_mbs;   
            MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
        
        }
    }
    // MAx Bytes mode in each slice
    else if (sliceMode->wSliceMode==UVC15_MULTI_SLICE_BYTEs) {
        MMP_USHORT max_bytes = sliceMode->wSliceConfigSetting ;
        if(max_bytes) {
            SliceCtl.SliceMode = MMPF_VIDENC_SLICE_MODE_BYTE ;
            SliceCtl.ulSliceSize = max_bytes;   
            MMPF_VIDENC_SetParameter(sid, MMPF_VIDENC_ATTRIBUTE_SLICE_CTL, (void*)&SliceCtl);
        
        }
    }
    #endif
    else {
        return PCAM_USB_INIT_ERR;
    } 
    return PCAM_ERROR_NONE;
}

static MMP_USHORT SetH264LevelIDC(MMP_UBYTE sid,MMP_UBYTE levelidc)
{
        
}


