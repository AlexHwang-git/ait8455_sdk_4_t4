//==============================================================================
//
//  File        : mmps_dsc.h
//  Description : INCLUDE File for the Host Application for DSC.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmps_dsc.h
 *  @brief The header File for the Host DSC control functions
 *  @author Penguin Torng
 *  @version 1.0
 */

#ifndef _MMPS_DSC_H_
#define _MMPS_DSC_H_

#include 	"mmp_lib.h"
#include	"mmpd_dsc.h"
#include	"mmpf_fctl.h"
#include    "mmps_display.h" 

/** @addtogroup MMPS_DSC
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
#define DSC_MAX_RAWSTORE_BUFFER_NUM     (2)

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPS_DSC_CAPTURE_FLOW
{
    MMPS_DSC_CAPFLOW_1_API = 0,    ///< JPEG image format
    MMPS_DSC_CAPFLOW_2_API          ///< RAW data image format
} MMPS_DSC_CAPTURE_FLOW;



typedef enum _MMPS_DSC_IMAGE_FORMAT
{
    MMPS_DSC_IMAGE_JPEG = 0,    ///< JPEG image format
    MMPS_DSC_IMAGE_RAW          ///< RAW data image format
} MMPS_DSC_IMAGE_FORMAT;


typedef enum _MMPS_DSC_PAN_DIRECTION
{
    MMPS_DSC_PAN_LEFT = 0,   ///< Picture pan left
    MMPS_DSC_PAN_RIGHT,      ///< Picture pan left
    MMPS_DSC_PAN_UP,         ///< Picture pan up
    MMPS_DSC_PAN_DOWN        ///< Picture pan down
} MMPS_DSC_PAN_DIRECTION;

typedef enum _MMPS_DSC_ZOOM_DIRECTION
{
    MMPS_DSC_ZOOM_IN = 0,	///< Picture/Preview zoom in
    MMPS_DSC_ZOOM_OUT,		///< Picture/Preview zoom out
    MMPS_DSC_ZOOM_STOP		///< Preview zoom stop
} MMPS_DSC_ZOOM_DIRECTION;

typedef enum _MMPS_DSC_CAPTUREMODE
{
    MMPS_DSC_STILL_SHOTMODE = 0,        ///< Still shot mode
    MMPS_DSC_MULTI_SHOTMODE             ///< Multi-shot mode
} MMPS_DSC_CAPTUREMODE;

typedef enum _MMPS_DSC_ZOOM_TYPE
{
    MMPS_DSC_LINEAR_ZOOM = 0,			///< Preview linear zoom mode
    MMPS_DSC_STEP_ZOOM					///< Preview step zoom mode
} MMPS_DSC_ZOOM_TYPE;


typedef enum _MMPS_DSC_JPEGQUALITY
{
    MMPS_DSC_CAPTURE_HIGH_QUALITY = 0,      ///< Capture high quality Jpeg
    MMPS_DSC_CAPTURE_NORMAL_QUALITY,        ///< Capture normal quality Jpeg
    MMPS_DSC_CAPTURE_LOW_QUALITY            ///< Capture normal quality Jpeg
} MMPS_DSC_JPEGQUALITY;

typedef enum _MMPS_DSC_MEDIA_PATH {
    MMPS_DSC_MEDIA_PATH_MEM = 0x00,    ///< Media operation by memory mode
    MMPS_DSC_MEDIA_PATH_CARD           ///< Media operation by card mode
} MMPS_DSC_MEDIA_PATH;

typedef enum _MMPS_DSC_PREBUFKEEP
{
    MMPS_DSC_PREBUF_KEEP_NULL = 0,   ///< Capture doesn't keep preview buffer 
    MMPS_DSC_PREBUF_KEEP_ONE,        ///< Capture keep one preview buffer
    MMPS_DSC_PREBUF_KEEP_ALL         ///< Capture keep all preview buffers
} MMPS_DSC_PREBUF_KEEP;

typedef enum _MMPS_DSC_EXIF_INPUT ///Only use for Card Mode
{
    MMPS_DSC_EXIF_INPUT_CARD = 0,   ///< The input raw data of Thumbnail is Jpeg of card
    MMPS_DSC_EXIF_INPUT_DRAM,       ///< The input raw data of Thumbnail is Jpeg of DRAM (JPG_EXTM_BUF)
    MMPS_DSC_EXIF_INPUT_DISPLAY     ///< The input raw data of Thumbnail is raw of display buffer. 
} MMPS_DSC_EXIF_INPUT;

typedef enum _MMPS_DSC_EXIF_OUTPUT ///Only use for Card Mode
{
    MMPS_DSC_EXIF_OUTPUT_CARD = 0,   ///< Exif header and Thumbnail Jpeg ouput to SD file
    MMPS_DSC_EXIF_OUTPUT_DRAM       ///< Exif header and Thumbnail Jpeg ouput to DRAM (JPG_EXTM_BUF)
} MMPS_DSC_EXIF_OUTPUT;


typedef enum _MMPS_DSC_CAPTURE_ROTATE_TYPE
{
    MMPS_DSC_CAPTURE_ROTATE_NO_ROTATE = 0,
    MMPS_DSC_CAPTURE_ROTATE_RIGHT_90,
    MMPS_DSC_CAPTURE_ROTATE_RIGHT_180,
    MMPS_DSC_CAPTURE_ROTATE_RIGHT_270
} MMPS_DSC_CAPTURE_ROTATE_TYPE;

typedef struct _MMPS_DSC_JPEGINFO {
    MMP_BYTE    bJpegFileName[DSC_MAX_FILE_NAME_SIZE * 2];
    MMP_USHORT  usJpegFileNameLength;
    MMP_USHORT  *usJpegbuf;
    MMP_ULONG   ulJpegbufaddr;
    MMP_ULONG   ulJpegbufsize;
    MMP_BOOL    bValid; 
    MMP_USHORT  usWidth;
    MMP_USHORT  usHeight;
    MMPD_DSC_JPEGFORMAT format;
	MMP_BOOL 	bDecodeThumbnail;
} MMPS_DSC_JPEGINFO;

typedef struct _MMPS_DSC_CAPTURE_INFO {
	MMP_USHORT	*usJpegbufaddr;				///< Host Memory mode jpeg picture buffer address
	MMP_ULONG 	ulHostbuflimit;				///< Host Memory mode jpeg buffer size limit

	MMP_BYTE 	*ubFilename;				///< Card mode file name
	MMP_USHORT	usFilenamelen;				///< Card mode file name length
	
	MMP_BOOL	bExif;						///< Enable EXIF function or not
	MMP_BOOL	bThumbnail;					///< Enable thumbnail function or not
	MMP_ULONG	ulJpegfilesize;				///< Captured JPEG size

	MMP_BOOL	bFirstShot;					///< First shot or second shot
	MMPS_DSC_CAPTURE_FLOW flow;				///< 0: old path, 1:new path 
	MMP_BOOL rawcapture;                    ///< enable/disable raw capture
} MMPS_DSC_CAPTURE_INFO;

typedef struct _MMPS_DSC_MULTISHOT_INFO {
	MMP_ULONG   ulExtcompstart[DSC_MULTISHOT_MAX_NUM];      	///< Jpeg DRAM start address 
	MMP_ULONG	ulJpegfilesize[DSC_MULTISHOT_MAX_NUM];			///< Jpeg file size
	MMP_USHORT  usShotindex;				                	///< Multishot index

	//for Exif
	MMP_ULONG   ulExifthumbnailoffset[DSC_MULTISHOT_MAX_NUM];  	///< Exif thumbnail offset 
	MMP_ULONG	ulExifthumbnailsize[DSC_MULTISHOT_MAX_NUM];		///< Exif thumbnail size
	MMP_ULONG	ulJpegrangesize[DSC_MULTISHOT_MAX_NUM];			///< Jpeg range size
	
} MMPS_DSC_MULTISHOT_INFO;

typedef struct _MMPS_DSC_EXIF_CTL {
    MMP_USHORT  usThumbnailwidth;				///< DSC capture, thumbnail width
	MMP_USHORT  usThumbnailheight;				///< DSC capture, thumbnail height
	MMP_ULONG   ulMaxthumbnailsize; 			///< DSC capture, thumbnail maximum size

    //Card mode EXIF path control
	MMPS_DSC_EXIF_INPUT	ubThumbnailInputMode;	///< The input raw data mode of Thumbnail
	MMPS_DSC_EXIF_OUTPUT ubThumbnailOutputMode;	///< The output jpeg data mode of Thumbnail

	//Thumbnai quality control
	MMP_UBYTE   ubHqfactor;						///< Thumbnai capture, high quality factor
    MMP_UBYTE   ubLqfactor;						///< Thumbnai capture, low quality factor
    MMP_USHORT  ubIntialfactor;					///< Thumbnai capture, initial quality factor
    MMP_USHORT  usHqjpegsize;					///< Thumbnai capture, high quality factor, estimate file size
    MMP_USHORT  usLqjpegsize;					///< Thumbnai capture, low quality factor, estimate file size
} MMPS_DSC_EXIF_CTL;


typedef struct _MMPS_DSC_PLAYBACK_INFO
{
	MMP_USHORT  usJpegWidth;				///< Jpeg picture original image width
	MMP_USHORT	usJpegHeight;				///< Jpeg picture original image height
	MMP_USHORT  usDisplayWidth;				///< Jpeg picture display width
	MMP_USHORT	usDisplayHeight;			///< Jpeg picture display height
	MMP_USHORT	usScaleLimit;				///< Jpeg picture scale limit
	MMPS_DSC_JPEGINFO 			jpeginfo;	///< Jpeg file information
	MMPD_GRAPHICS_BUFATTRIBUTE	bufattribute;	///< Decoded raw data buffer attribute
	MMPF_SCALER_GRABCONTROL		grabctl;		///< Grab information between original image and display image
	MMPD_DISPLAY_ROTATE_TYPE	rotatetype;		///< Rotation attribute when display
} MMPS_DSC_PLAYBACK_INFO;

typedef struct _MMPS_DSC_ZOOM_INFO
{
    MMP_USHORT	usZoomMin;					///< Minimum zoom rate
    MMP_USHORT	usZoomMax;					///< Maximum zoom rate
    MMP_USHORT	usZoomNow;					///< Current zoom rate
    MMP_USHORT	usStepIndex;    			///< Current zoom step
    MMP_USHORT  usZoomSteps;                 ///< Zoom Steps
    MMPF_SCALER_PATH  scalerpath;           ///< Scaler path  
} MMPS_DSC_ZOOM_INFO;


typedef struct _MMPS_DSC_DISPMODEPARAMS {
	MMP_UBYTE	ubDSCpreviewmodenum;					///< DSC preview mode number
// 20060308 : Penguin		
    MMP_BOOL    bDSCpreviewrangeuserdef[DSC_MODE_MAX_NUM];	///< DSC preview mode by user define or auto-calculation
    //++ This region can be use-define or system auto-calculate
    MMP_USHORT  usDSCgrabstartx[DSC_MODE_MAX_NUM];			///< specify grab range if bDSCpreviewrangeuserdef == MMP_TRUE
    MMP_USHORT  usDSCgrabstarty[DSC_MODE_MAX_NUM];			///< specify grab range if bDSCpreviewrangeuserdef == MMP_TRUE
    MMP_USHORT  usDSCgrabendx[DSC_MODE_MAX_NUM];			///< specify grab range if bDSCpreviewrangeuserdef == MMP_TRUE
    MMP_USHORT  usDSCgrabendy[DSC_MODE_MAX_NUM];			///< specify grab range if bDSCpreviewrangeuserdef == MMP_TRUE
    MMP_USHORT  usDSCgrabscaleN[DSC_MODE_MAX_NUM];			///< specify grab parameter if bDSCpreviewrangeuserdef == MMP_TRUE
    MMP_USHORT  usDSCgrabscaleM[DSC_MODE_MAX_NUM];			///< specify grab parameter if bDSCpreviewrangeuserdef == MMP_TRUE
    //--- This region can be user-define or system auto-calculate
    MMPS_DISPLAY_COLORMODE  DSCcolormode[DSC_MODE_MAX_NUM];	///< specify preview color format
    MMPF_FCTL_PREVIEWMODE	Displaymode[DSC_MODE_MAX_NUM];	///< specify output display control type
    MMPD_DISPLAY_WINID      Displaywinid[DSC_MODE_MAX_NUM]; ///< specify output display win id
    MMP_USHORT  usDSCdispbuffercount[DSC_MODE_MAX_NUM];		///< specify output buffer counts for preview
    MMP_USHORT  usDSCbufstartx[DSC_MODE_MAX_NUM];			///< specify start X output buffer display attribute
    MMP_USHORT  usDSCbufstarty[DSC_MODE_MAX_NUM];			///< specify start Y output buffer display attribute
    MMP_USHORT  usDSCdispwidth[DSC_MODE_MAX_NUM];			///< specify output buffer display width
    MMP_USHORT  usDSCdispheight[DSC_MODE_MAX_NUM];			///< specify output buffer display height
    MMP_USHORT  usDSCdispstartx[DSC_MODE_MAX_NUM];			///< specify output buffer display offset X
    MMP_USHORT  usDSCdispstarty[DSC_MODE_MAX_NUM];			///< specify output buffer display offset Y
    MMP_BOOL    bDSCdispmirror[DSC_MODE_MAX_NUM];			///< specify output buffer display mirror attribute
    MMPD_DISPLAY_ROTATE_TYPE   DSCdirection[DSC_MODE_MAX_NUM];	///< specify output buffer display rotate attribute
    MMP_BOOL    bDSCdispscaleen[DSC_MODE_MAX_NUM];			///< specify output buffer display scale enable attribute
    MMP_USHORT  usDSCdispscalewidth[DSC_MODE_MAX_NUM];		///< specify output buffer display scale output width
    MMP_USHORT  usDSCdispscaleheight[DSC_MODE_MAX_NUM];		///< specify output buffer display scale output height

    //--- This region is for face detection
    MMP_USHORT  usFaceDetectInputWidth[DSC_MODE_MAX_NUM];   ///< the width of input preview rectangle for face detection.
    MMP_USHORT  usFaceDetectInputHeight[DSC_MODE_MAX_NUM];  ///< the height of input preview rectangle for face detection.

    //++ Use DMA rotate
    MMP_BOOL    bUseRotateDMA[DSC_MODE_MAX_NUM];			///< specify DMA rotate enable  
    MMP_USHORT  usRotateBufCnt[DSC_MODE_MAX_NUM];			///< specify DMA rotate buffer counter 
    //-- Use DMA rotate

    //++ DSC preview Zoom
    MMPS_DSC_ZOOM_TYPE	DSCzoomtype[DSC_MODE_MAX_NUM];		///< DSC preview zoom type
    MMP_UBYTE   ubDSCzoomstep[DSC_MODE_MAX_NUM];			///< DSC preview zoom step
    MMP_BOOL	bDSCzoomjump[DSC_MODE_MAX_NUM];				///< DSC preview zoom jump or continuous
    MMP_BOOL	bDSCRealTimeScaleUp2x[DSC_MODE_MAX_NUM];    ///< DSC preview zoom real time scale up 2x
    //-- DSC preview Zoom
} MMPS_DSC_DISPMODEPARAMS;

typedef struct _MMPS_DSC_JPEGRESOLPARAMS {
    MMP_UBYTE   ubDSCresolnum;								///< DSC capture resolution total number
    MMP_USHORT  usDSCcapturebase[DSC_RESOL_MAX_NUM];		///< DSC capture base for each resolution
    MMP_USHORT  usDSCresolwidth[DSC_RESOL_MAX_NUM];			///< DSC capture width for each resolution
    MMP_USHORT  usDSCresolheight[DSC_RESOL_MAX_NUM];		///< DSC capture height for each resolution

    MMP_BOOL    bDSCOSDkeepwhencapture[DSC_MODE_MAX_NUM][DSC_SHOT_MODE_MAX_NUM];	///< DSC keep OSD refresh when capture
	MMPS_DSC_PREBUF_KEEP  DSCPrekeepwhencapture[DSC_MODE_MAX_NUM][DSC_SHOT_MODE_MAX_NUM];	///< DSC keep preview buffer refresh when capture
	MMP_BOOL    bDSCpreviewimageon[DSC_MODE_MAX_NUM][DSC_SHOT_MODE_MAX_NUM];	///< DSC keep preview output when capture
    MMP_ULONG   ulDSCencoutbufsize[DSC_MODE_MAX_NUM][DSC_RESOL_MAX_NUM];		///< DSC capture, output bitstream size
    MMP_ULONG   ulDSCextencbufsize[DSC_MODE_MAX_NUM][DSC_RESOL_MAX_NUM];		///< DSC capture, output bitstream size in external buffer

    // ++ Quality Control
    MMP_UBYTE   ubDSChqfactor[DSC_RESOL_MAX_NUM];					///< DSC capture, high quality factor
    MMP_UBYTE   ubDSClqfactor[DSC_RESOL_MAX_NUM];					///< DSC capture, low quality factor
    MMP_USHORT  ubDSCintialfactor[DSC_RESOL_MAX_NUM];				///< DSC capture, initial quality factor
    MMP_USHORT  usDSChqjpegsize[DSC_RESOL_MAX_NUM];					///< DSC capture, high quality factor, estimate file size
    MMP_USHORT  usDSClqjpegsize[DSC_RESOL_MAX_NUM];					///< DSC capture, low quality factor, estimate file size
    MMP_USHORT  usDSCtargetsize[DSC_RESOL_MAX_NUM][DSC_QTABLE_MAX_NUM];    	///< DSC capture, target file size
    MMP_USHORT  usDSClimitsize[DSC_RESOL_MAX_NUM][DSC_QTABLE_MAX_NUM];      ///< DSC capture, limit file size 	
    MMP_UBYTE   ubDSCqtable[DSC_QTABLE_MAX_NUM][128];					///< DSC capture, quality table
    MMP_USHORT  usDSCResolZoomStep[DSC_RESOL_MAX_NUM];		        ///< DSC zoom steps depend on jpeg resolution
    MMP_UBYTE   ubDSCMaxZoomMultiplier[DSC_RESOL_MAX_NUM];			///< DSC preview max. zoom base
    MMP_UBYTE   ubDSCRawPathThreshold[DSC_RESOL_MAX_NUM];				///< DSC capture, if the zoom ratio is over threshold, it caputre from rawproc path
    // ++ Thumbnail
	MMPS_DSC_EXIF_CTL Exif;                                             ///< DSC capture Jpeg EXIF 
    // -- Thumbnail
} MMPS_DSC_JPEGRESOLPARAMS;

typedef struct _MMPS_DSC_PREVIEWDATA {
    MMPS_DSC_DISPMODEPARAMS   dispmodeparams;               ///< DSC preview related settings
    MMPS_DSC_JPEGRESOLPARAMS  jpegresolparams;              ///< DSC capture related settings
} MMPS_DSC_PREVIEWDATA;

typedef struct _MMPS_DSC_DECODEDATA {
    MMP_UBYTE   ubDSCdecodemodenum;							///< DSC playback mode total number
    MMP_ULONG   ulDSCDecmaxlinebufsize;						///< DSC supported maximum jpeg decode line buffer size
    MMP_BOOL    bDSCOSDkeepwhendecode;						///< DSC decode, keep OSD alive
    MMP_ULONG   ulDSCDeccompressbufsize;					///< DSC decode, reserve data input buffer size
    MMP_ULONG   ulDSCDecinbufsize;							///< DSC decode, reserve data temp buffer size
    MMPS_DISPLAY_COLORMODE  Decodecolormode[DSC_DECODE_MODE_MAX_NUM];	///< DSC decode, decode output color mode
	MMP_USHORT	usDecodewidth[DSC_DECODE_MODE_MAX_NUM];				///< DSC decode output width
	MMP_USHORT	usDecodeheight[DSC_DECODE_MODE_MAX_NUM];			///< DSC decode output height
	MMP_BOOL	bMirror[DSC_DECODE_MODE_MAX_NUM];					///< DSC decode display mirror attribute
	MMPD_DISPLAY_ROTATE_TYPE	rotatetype[DSC_DECODE_MODE_MAX_NUM];	///< DSC decode display rotation attribute
    MMP_BOOL    bDecodedispscaleen[DSC_DECODE_MODE_MAX_NUM];			///< DSC decode display scale enable attribute
    MMPF_SCALER_FIT_MODE	fitmode[DSC_DECODE_MODE_MAX_NUM];			///< DSC decode display scale auto-fit mode
    MMP_USHORT  usDecodedispscalewidth[DSC_DECODE_MODE_MAX_NUM];		///< DSC decode display scale output width
    MMP_USHORT  usDecodedispscaleheight[DSC_DECODE_MODE_MAX_NUM];		///< DSC decode display scale output height
    MMP_BOOL	bDoubleoutputbuffer[DSC_DECODE_MODE_MAX_NUM];			///< DSC decode, double buffer for store decoded raw buffer
    MMP_ULONG	ulDecodeOSDSize;										///< DSC decode OSD available size
    MMP_UBYTE	ubDecodezoomstep;								///< DSC decode zoom step
    MMP_UBYTE	ubDecodepanstep;								///< DSC decode zoom pan
} MMPS_DSC_DECODEDATA;

typedef struct _MMPS_DSC_CONFIG {
    MMP_BOOL			    bFixInternalOSD;					                ///< fix OSD location for all capture and decode mode
    MMP_USHORT              usDSCmodesensorpreviewmode[DSC_MODE_MAX_NUM];	    ///< choose sensor preview mode	
    MMP_USHORT              usDSCmodesensorpreviewphasecount[DSC_MODE_MAX_NUM];	///< choose sensor phase count
    MMP_USHORT              usDSCmodedispwidth[DSC_MODE_MAX_NUM];			    ///< DSC preview width
    MMP_USHORT              usDSCmodedispheight[DSC_MODE_MAX_NUM];			    ///< DSC preview height
    MMPS_DSC_PREVIEWDATA    previewdata;								        ///< DSC preview and capture setting
    MMPS_DSC_DECODEDATA	    decodedata;									        ///< DSC decode setting
    MMP_ULONG               ulMaxPanoramaShot;                                  ///< Max panorama shot number
    MMP_BOOL                bDrawPanoramaOverlapArea;                           ///< Draw panorama overlap area or not
    MMP_ULONG               ulPanoramaThumbWidth;                               ///< Thumbnail width for panorama
    MMP_ULONG               ulPanoramaThumbHeight;                              ///< Thumbnail height for panorama
    MMP_BOOL                bRawPreviewEnable;                                  ///< Enable/Disable raw preview mode
    MMP_BOOL                bRawCaptureEnable;                                  ///< Enable/Disable raw capture mode
    MMP_ULONG               ulRawStoreBufferCount;                              ///< the number of rawstore buffer
    MMP_ULONG               ulRawBufferOverlapOffset;                           ///< the raw buffer start address to overlap external compressed buffer
    MMP_BOOL                bSizeControlPredictModeEnable;                      ///< Enable/Disable size control predict mode enable
} MMPS_DSC_CONFIG;

typedef struct _MMPS_DSC_GRABCONTROL {
    MMP_USHORT  usScaleN;				///< DSC scaler grab control scale ratio N
    MMP_USHORT  usScaleM;				///< DSC scaler grab control scale ratio M
    MMP_USHORT  usStartX;				///< DSC scaler grab control start X
    MMP_USHORT  usStartY;				///< DSC scaler grab control start Y
    MMP_USHORT  usEndX;					///< DSC scaler grab control end X
    MMP_USHORT  usEndY;					///< DSC scaler grab control end Y
} MMPS_DSC_GRABCONTROL;

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
MMPS_DSC_CONFIG*  MMPS_DSC_GetConfiguration(void);
MMP_ERR MMPS_DSC_SetSystemMode(MMP_USHORT usDSCmode);
MMP_ERR MMPS_DSC_SetShotMode(MMPS_DSC_CAPTUREMODE capturemode);
MMP_ERR MMPS_DSC_SetMediaPath(MMPS_DSC_MEDIA_PATH mediapath);
MMP_ERR MMPS_DSC_SetPreviewMode(MMP_USHORT usJpegResol);
MMP_ERR MMPS_DSC_SetPreviewDisplay(MMP_BOOL bEnable, MMP_BOOL bCheckFrameEnd);
MMP_ERR MMPS_DSC_GetPreviewDisplay(MMP_BOOL *bEnable);

MMP_ERR MMPS_DSC_SelectCaptureBuffers(MMP_BOOL bExtbuffer,MMPS_DSC_CAPTURE_ROTATE_TYPE CaptureRotateType);
MMP_ERR MMPS_DSC_SetCaptureJpegQuality(MMP_BOOL bTargetCtrl, MMP_BOOL bLimitCtrl,
                                               MMP_USHORT usMaxCount, MMPS_DSC_JPEGQUALITY qualitylevel);

MMP_ERR MMPS_DSC_CaptureTakeJpeg(MMPS_DSC_CAPTURE_INFO *captureinfo);
MMP_ERR MMPS_DSC_GetJpegInfo(MMPS_DSC_JPEGINFO *jpeginfo);
MMP_ERR MMPS_DSC_JpegDram2Card(MMPS_DSC_CAPTURE_INFO *captureinfo);
MMP_ERR MMPS_DSC_JpegDram2Host(MMPS_DSC_CAPTURE_INFO *captureinfo);

MMP_ERR MMPS_DSC_DecodeJpeg(MMPS_DSC_JPEGINFO *jpeginfo, MMP_USHORT *usHostbufaddr,
	            MMP_USHORT usOutputWidth, MMP_USHORT usOutputHeight, MMPS_DISPLAY_COLORMODE OutputFormat, MMPS_DSC_GRABCONTROL *GrabCtl);
MMP_ERR MMPS_DSC_GetJpegBufAttribute(MMPD_GRAPHICS_BUFATTRIBUTE *jpegbufattribute);
#ifdef BUILD_CE
MMP_ERR MMPS_DSC_PlaybackJpeg(MMPS_DSC_JPEGINFO *jpeginfo, 
                MMP_USHORT usDisplayOffsetX, MMP_USHORT usDisplayOffsetY, MMP_USHORT *OutputPhotoBuffer);
#else
MMP_ERR MMPS_DSC_PlaybackJpeg(MMPS_DSC_JPEGINFO *jpeginfo, MMP_USHORT usStartX, MMP_USHORT usStartY);
#endif

MMP_ERR MMPS_DSC_CanPanPlayback(MMPS_DSC_PAN_DIRECTION pandir, MMP_BOOL *bPanenable);
MMP_ERR MMPS_DSC_CanZoomPlayback(MMPS_DSC_ZOOM_DIRECTION zoomdir, MMP_BOOL *bZoomenable);
MMP_ERR MMPS_DSC_PlaybackJpegPan(MMPS_DSC_PAN_DIRECTION pandir);
MMP_ERR MMPS_DSC_PlaybackJpegZoom(MMPS_DSC_ZOOM_DIRECTION zoomdir);


MMP_ERR MMPS_DSC_SetPreviewZoom(MMPS_DSC_ZOOM_DIRECTION zoomdir);
MMP_ERR MMPS_DSC_SetPlaybackMode(MMP_USHORT usDecodeMode);

MMP_ERR MMPS_DSC_SetSticker(MMPD_ICON_BUFATTRIBUTE *StickerBufAttribute1, MMPD_ICON_BUFATTRIBUTE *StickerBufAttribute2);
MMP_ERR MMPS_DSC_WriteStickerBufRgb565(MMP_USHORT usIconID, MMP_UBYTE *mem_ptr, MMP_USHORT width, MMP_USHORT height);
MMP_ERR MMPS_DSC_SetStickerdEnable(MMP_USHORT usIconID, MMP_BOOL bEnable);

MMP_ERR MMPS_DSC_SetPreviewZoom3(MMP_USHORT usZoomStep); // DIFFDIFF
MMP_ERR MMPS_DSC_SetExifFillTime(MMP_ULONG ulStr0, MMP_ULONG ulStr1, MMP_ULONG ulStr2, MMP_ULONG ulStr3, MMP_ULONG ulStr4);
MMP_ERR MMPS_DSC_EnablePanoramaCaptureMode(MMP_BOOL bEnable);
MMP_ERR MMPS_DSC_PanoramaInitConfig(MMP_USHORT usDSCmode,MMP_ULONG ulMaxShot,MMP_USHORT usJpegSrcResol);
MMP_ERR MMPS_DSC_EncodePanoramaToJpeg(MMPS_DSC_CAPTURE_INFO *captureinfo);
MMP_ERR MMPS_DSC_RawPreviewEnable(MMP_BOOL bEnable);
MMP_ERR MMPS_DSC_SetIntelligentZoomPara(MMP_USHORT usJpegResol, MMP_BOOL enable);
MMP_ERR MMPS_DSC_SwtichZeroShutterLag(MMP_BOOL bEnable);
MMP_ERR MMPS_DSC_CheckDefectPixel(MMP_ULONG *ulHostMemoryAddr, MMP_BYTE* filename);
MMP_ERR MMPS_DSC_SetDefectPixelInfo(MMP_UBYTE *ubHostMemoryAddr, MMP_BYTE* bfilename);

//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _INCLUDES_H_
