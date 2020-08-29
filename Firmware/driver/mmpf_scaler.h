//==============================================================================
//
//  File        : mmpf_scaler.h
//  Description : INCLUDE File for the Firmware Scaler Control driver function, including LCD/TV/Win
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_SCALER_H_
#define _MMPF_SCALER_H_
#include    "includes_fw.h"

#define GRA_DBG_LEVEL	4
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
#if (CHIP == VSN_V3)
#define TOTAL_SCALER_PATH               3

#define	SCALER_PATH0_MAX_WIDTH	        (3328)

#define	SCALER_PATH1_MAX_WIDTH	        (1920)

#define	SCALER_PATH2_MAX_WIDTH	        (1280)

#define	LPF_MAX_WIDTH			        (SCALER_PATH0_MAX_WIDTH)

#endif

#if (CHIP == VSN_V2)
#define TOTAL_SCALER_PATH               3

#define	SCALER_PATH0_MAX_WIDTH	        (2592)

#define	SCALER_PATH1_MAX_WIDTH	        (1280)

#define	SCALER_PATH2_MAX_WIDTH	        (800)

#define	LPF_MAX_WIDTH			        (SCALER_PATH0_MAX_WIDTH)

#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPF_SCALER_SOURCE {
    MMPF_SCALER_SOURCE_ISP = 0,
    MMPF_SCALER_SOURCE_JPG,
    MMPF_SCALER_SOURCE_GRA
} MMPF_SCALER_SOURCE;

typedef enum _MMPF_SCALER_FIT_MODE
{
    MMPF_SCALER_FITMODE_OUT = 0,
    MMPF_SCALER_FITMODE_IN
} MMPF_SCALER_FIT_MODE;

typedef enum _MMPF_SCALER_PATH
{
    MMPF_SCALER_PATH_0 = 0,
    MMPF_SCALER_PATH_1,
    MMPF_SCALER_PATH_2,
    MMPF_SCALER_PATH_MAX
} MMPF_SCALER_PATH;

typedef enum _MMPF_SCALER_LPF_SR {
    MMPF_SCALER_USE_LPF = 0,
    MMPF_SCALER_SKIP_LPF
} MMPF_SCALER_LPF_SR;

typedef enum _MMPF_SCALER_COLRMTX_MODE {
    MMPF_SCALER_COLRMTX_BT601 = 0,
    MMPF_SCALER_COLRMTX_FULLRANGE,
    MMPF_SCALER_COLRMTX_MAX
} MMPF_SCALER_COLRMTX_MODE;

typedef struct _MMPF_SCALER_FIT_RANGE {
    MMPF_SCALER_FIT_MODE   fitmode;
    MMP_USHORT  usFitResol;
    MMP_USHORT  usInWidth;
    MMP_USHORT  usInHeight;
    MMP_USHORT  usOutWidth;
    MMP_USHORT  usOutHeight;
} MMPF_SCALER_FIT_RANGE;

typedef struct _MMPF_SCALER_GRABCONTROL {
    MMP_USHORT  usScaleN;
    MMP_USHORT  usScaleM;
    MMP_USHORT  usStartX;
    MMP_USHORT  usStartY;
    MMP_USHORT  usEndX;
    MMP_USHORT  usEndY;
#if USB_LYNC_TEST_MODE 
    MMP_USHORT  usScaleXN,usScaleXM;
    MMP_USHORT  usScaleYN,usScaleYM;
#endif
   
} MMPF_SCALER_GRABCONTROL;

typedef enum _MMPF_SCALER_ZOOMDIRECTION
{
    MMPF_SCALER_ZOOMIN = 0,
	MMPF_SCALER_ZOOMOUT,
	MMPF_SCALER_ZOOMSTOP
} MMPF_SCALER_ZOOMDIRECTION;

typedef struct _MMPF_SCALER_ZOOM_INFO {
    MMP_USHORT                 usStepX;
    MMP_USHORT                 usStepY;
    MMP_USHORT                 usInputWidth;
    MMP_USHORT                 usInputHeight;
    MMPF_SCALER_GRABCONTROL    grabCtl;          ///< For Preview Grab Control
    MMPF_SCALER_GRABCONTROL    grabCtlIn;          ///< For Preview Grab Control
    MMP_USHORT                 usRangeMinN;      ///< To constrain the N-Value for zoom in and zoom out
    MMP_USHORT                 usRangeMaxN;      ///< To constrain the N-Value for zoom in and zoom out
    MMP_USHORT                 usBaseN;          ///< The real zoom base minimum , not floating whih zoom in and zoom out
    MMPF_SCALER_ZOOMDIRECTION  direction;
    MMP_USHORT                 usZoomRate;
#if USB_LYNC_TEST_MODE
    MMP_USHORT                 usBaseXN,usBaseYN;
    MMP_USHORT                 usBaseXM,usBaseYM;
    MMP_USHORT                 usRangeMinXN,usRangeMaxXN;      ///< To constrain the N-Value for zoom in and zoom out
    MMP_USHORT                 usRangeMinYN,usRangeMaxYN;      ///< To constrain the N-Value for zoom in and zoom out
#endif    
    MMP_USHORT                 usSensorWidth;
    MMP_USHORT                 usSensorHeight;
} MMPF_SCALER_ZOOM_INFO;

typedef struct _MMPF_SCALER_PANTILT_INFO {
    MMP_BOOL bGrabSrc ;
    MMP_LONG ulTargetPan,ulTargetTilt;
    MMP_USHORT usPanEnd,usTiltEnd,usStep;
    MMPF_SCALER_GRABCONTROL grabCtl,grabCtlIn;          ///< For Preview Grab Control
} MMPF_SCALER_PANTILT_INFO;


typedef enum _MMPF_SCALER_OP
{
    MMPF_SCALER_OP_FINISH = 0,
    MMPF_SCALER_OP_GOING
} MMPF_SCALER_OP;

typedef enum _MMPF_SCALER_COLORMODE
{
    MMPF_SCALER_COLOR_RGB565 = 0,
    MMPF_SCALER_COLOR_RGB888,
    MMPF_SCALER_COLOR_YUV444,
    MMPF_SCALER_COLOR_YUV422
} MMPF_SCALER_COLORMODE;

#if SCALER_ZOOM_LEVEL==BEST_LEVEL
typedef struct _RATIO_TBL
{
	MMP_UBYTE  n,m ;
	MMP_USHORT ratio ;
} RATIO_TBL ;
#endif

#define SCAL_FLOW_NONE          0x00
#define SCAL_WAIT_ENC_END       0x01
#define SCAL_ENC_END            0x02
#define SCAL_READY_TO_SWITCH    0x03

typedef struct _MMPF_SCALER_SWITCH_PATH
{
    MMP_BOOL set ;
    MMP_BOOL forceswitch;
    MMPF_SCALER_PATH path ;
    MMPF_SCALER_SOURCE src;
    MMP_USHORT m,n;
    MMP_USHORT flowctl; 
} MMPF_SCALER_SWITCH_PATH ;
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
MMP_ERR MMPF_Scaler_SetEngine(MMP_BOOL bUserDefine, MMPF_SCALER_PATH pathsel,
                        MMPF_SCALER_FIT_RANGE *fitrange, MMPF_SCALER_GRABCONTROL *grabctl);

MMP_ERR MMPF_Scaler_SetLPF(MMPF_SCALER_PATH pathsel, MMPF_SCALER_FIT_RANGE *fitrange, MMPF_SCALER_GRABCONTROL *grabctl);
MMP_ERR MMPF_Scaler_SetOutputColor(MMPF_SCALER_PATH pathsel, MMP_BOOL bEnable, MMPF_SCALER_COLRMTX_MODE MatrixMode);
MMP_ERR MMPF_Scaler_SetOutputFormat(MMPF_SCALER_PATH pathsel, MMPF_SCALER_COLORMODE outcolor);
MMP_ERR MMPF_Scaler_SetZoomParams(MMPF_SCALER_PATH pathsel, MMPF_SCALER_GRABCONTROL *grabctl,
                        MMP_USHORT usInputWidth, MMP_USHORT usInputHeight, MMP_USHORT usStepX, MMP_USHORT usStepY);
MMP_ERR MMPF_Scaler_SetZoomRange(MMPF_SCALER_PATH pathsel, MMP_USHORT usZoomMin, MMP_USHORT usZoomMax);
MMP_ERR MMPF_Scaler_SetDigitalZoom(MMPF_SCALER_PATH pathsel, MMPF_SCALER_ZOOMDIRECTION zoomdir, 
                                              MMP_USHORT zoomrate, MMP_USHORT startOP);
MMP_ERR MMPF_Scaler_SetZoomOP(void);
MMPF_SCALER_ZOOM_INFO* MMPF_Scaler_GetCurInfo(MMPF_SCALER_PATH PathSel);
MMP_ERR MMPF_Scaler_GetZoomInfo(MMPF_SCALER_PATH pathsel, MMP_USHORT *usBaseN, MMP_USHORT *usCurrentN);
MMP_ERR MMPF_Scaler_GetZoomOutPos(MMPF_SCALER_PATH PathSel, MMPF_SCALER_GRABCONTROL *SetgrabCtl);
MMP_ERR MMPF_Scaler_GetScaleUpHBlanking(MMPF_SCALER_PATH pathsel, MMP_BYTE pixel_delay, MMP_ULONG *HBlanking);
MMP_ERR MMPF_Scaler_EnableDNSMP(MMPF_SCALER_PATH pathsel, MMP_BYTE bDownSample);
MMP_ERR MMPF_Scaler_SetDigitalZoomOP(MMPF_SCALER_PATH pathsel, MMPF_SCALER_ZOOMDIRECTION zoomdir, 
                                              MMP_USHORT zoomrate);


MMPF_SCALER_PANTILT_INFO* MMPF_Scaler_GetCurPanTiltInfo(MMPF_SCALER_PATH PathSel);
MMP_ERR MMPF_Scaler_SetPanTiltOP(void);
MMP_ERR MMPF_Scaler_SetDigitalPanTilt(MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_SetPanTiltParams(MMPF_SCALER_PATH pathsel,MMP_LONG targetPan,MMP_LONG targetTilt,MMP_USHORT steps,MMP_LONG *pan_off,MMP_LONG *tilt_off);
MMP_ERR MMPF_Scaler_SetZoomSinglePipe(MMPF_SCALER_PATH mainpipe);
MMP_ERR MMPF_Scaler_EnableZoom(MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_DisableZoom(MMPF_SCALER_PATH pathsel);

MMP_ERR MMPF_Scaler_GetBestFitScale(MMPF_SCALER_FIT_RANGE *fitrange,
                        MMPF_SCALER_GRABCONTROL *grabctl);
MMP_ERR MMPF_Scaler_SetEnable(MMPF_SCALER_PATH pathsel, MMP_BOOL bEnable);
MMP_ERR MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE source, MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_CheckZoomComplete(MMPF_SCALER_PATH pathsel, MMP_BOOL *bComplete);
MMP_ERR MMPF_Scaler_CheckPanTiltComplete(MMPF_SCALER_PATH pathsel, MMP_BOOL *bComplete);
#if SCALER_ZOOM_LEVEL==BEST_LEVEL
MMP_USHORT MMPF_Scaler_CreateScalerTable(void);
RATIO_TBL *MMPF_Scaler_GetBestM(MMPF_SCALER_PATH pipe,MMP_USHORT *item_at,MMP_USHORT in_w,MMP_USHORT in_h,MMP_USHORT out_w,MMP_USHORT out_h,MMP_USHORT user_ratio);
MMP_USHORT MMPF_Scaler_GetRatioStartItem(MMPF_SCALER_PATH pipe);
MMP_USHORT MMPF_Scaler_GetRatioCurItem(MMPF_SCALER_PATH pipe);
void MMPF_Scaler_SetRatioCurItem(MMPF_SCALER_PATH pipe,MMP_USHORT item);
MMP_ERR MMPF_Scaler_GetBestGrabRange(MMPF_SCALER_PATH pipe,MMP_USHORT item,MMPF_SCALER_FIT_RANGE *fitrange,MMPF_SCALER_GRABCONTROL *grabctl,MMPF_SCALER_GRABCONTROL *grabinctl,MMP_USHORT lync_ratio);
RATIO_TBL *MMPF_Scaler_GetBaseRatioTbl(MMPF_SCALER_PATH pipe);
RATIO_TBL *MMPF_Scaler_GetCurRatioTbl(MMPF_SCALER_PATH pipe);
RATIO_TBL *MMPF_Scaler_GetRatioTblByRatio(MMPF_SCALER_PATH pipe,MMP_USHORT new_ratio,MMP_BOOL move_ratio_item);
#endif
MMP_ERR MMPF_Scaler_SetPixelLineDelay(MMPF_SCALER_PATH pathsel, MMP_UBYTE ubPixelDelay, MMP_UBYTE ubLineDelay);
MMPF_SCALER_SOURCE MMPF_Scaler_GetPath(MMPF_SCALER_PATH pathsel);

MMP_BOOL MMPF_Scaler_CheckUpdatePath(void);
MMP_ERR MMPF_Scaler_SwitchPath(MMP_BOOL set,MMPF_SCALER_PATH pathsel,MMPF_SCALER_SOURCE src,MMP_BOOL wait_end);
MMPF_SCALER_SWITCH_PATH *MMPF_Scaler_GetPathInfo(MMPF_SCALER_PATH pathsel);
void MMPF_Scaler_ResetPathInfo(MMPF_SCALER_PATH pathsel);
void MMPF_Scaler_SetForceSwitchPath(MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_SetZoomSensorInput(MMPF_SCALER_PATH pathsel,MMP_USHORT w,MMP_USHORT h);
MMP_UBYTE MMPF_Scaler_GetLineDelay(MMPF_SCALER_PATH pathsel);
MMP_ERR MMPF_Scaler_AccessGrabArea(MMP_BOOL set,MMPF_SCALER_PATH pathsel,MMPF_SCALER_GRABCONTROL *grabin,MMPF_SCALER_GRABCONTROL *grabout);
MMP_BOOL MMPF_Scaler_IfGrabInput(MMPF_SCALER_PATH pathsel,MMP_USHORT w,MMP_USHORT n,MMP_USHORT m);
MMP_USHORT MMPF_Scaler_GetNbyM(MMP_USHORT m,MMP_USHORT in_w,MMP_USHORT in_h,MMP_USHORT out_w,MMP_USHORT out_h);
MMP_ERR MMPF_Scaler_GetCurRatio(MMPF_SCALER_PATH pathsel,MMP_USHORT *ratioN,MMP_USHORT *ratioM);
MMP_ERR MMPF_Scaler_SetEdge(MMPF_SCALER_PATH pathsel,MMP_UBYTE edge_core,MMP_UBYTE edge_val);
MMP_ERR MMPF_Scaler_GetEdge(MMPF_SCALER_PATH pathsel,MMP_UBYTE *edge_core,MMP_UBYTE *edge_val);
MMP_ERR MMPF_Scaler_EnableEdge(MMPF_SCALER_PATH pathsel,MMP_BOOL enable);
MMP_ERR MMPF_Scaler_SetRefInHeight(MMP_USHORT scaler_ref_h);
MMP_ERR MMPF_Scaler_SetRefOutHeight(MMPF_SCALER_PATH pathsel,MMP_USHORT scaler_real_h);
MMP_BOOL MMPF_Scaler_IsScalingUp(MMPF_SCALER_PATH pathsel,MMP_USHORT ratio);
MMP_ERR MMPF_Scaler_SetOutputRange(MMPF_SCALER_PATH pathsel,
                                    MMP_UBYTE y_min,MMP_UBYTE y_max,
                                    MMP_UBYTE u_min,MMP_UBYTE u_max,
                                    MMP_UBYTE v_min,MMP_UBYTE v_max);
MMP_ERR MMPF_Scaler_GetOutputRange(MMPF_SCALER_PATH pathsel,
                                    MMP_UBYTE *y_min,MMP_UBYTE *y_max,
                                    MMP_UBYTE *u_min,MMP_UBYTE *u_max,
                                    MMP_UBYTE *v_min,MMP_UBYTE *v_max);
                                    
MMP_UBYTE MMPF_Scaler_GetMatrixCtl(MMPF_SCALER_PATH pathsel);
void MMPF_Scaler_SetMatrixCtl(MMPF_SCALER_PATH pathsel,MMP_UBYTE ctl);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _MMPD_SCALER_H_
