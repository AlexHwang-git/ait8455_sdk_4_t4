/// @ait_only
//==============================================================================
//
//  File        : mmpd_dsc.h
//  Description : INCLUDE File for the Host DSC Driver.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpd_dsc.h
 *  @brief The header File for the Host DSC control functions
 *  @author Penguin Torng
 *  @version 1.0
 */

#ifndef _MMPD_DSC_H_
#define _MMPD_DSC_H_

#include "mmp_lib.h"
#include "ait_config.h"
#include "mmpd_display.h"

/** @addtogroup MMPD_DSC
 *  @{
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
#define DSC_MAX_FILE_NAME_SIZE  256
#define DSC_MAX_OUTPUT_RANGE  0xFFFFFFFF

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPD_DSC_COLORPATH {
    MMPD_DSC_PATH_YUV = 0,
    MMPD_DSC_PATH_RGB
} MMPD_DSC_COLORPATH;

typedef struct _MMPD_DSC_ISPSCALECONFIG {
    MMPD_DSC_COLORPATH  colorpath;
    MMP_USHORT  usStartX;
    MMP_USHORT  usStartY;
    MMP_USHORT  usEndX;
    MMP_USHORT  usEndY;
    MMP_USHORT  usScaleN;
    MMP_USHORT  usScaleM;
} MMPD_DSC_ISPSCALECONFIG;

typedef struct _MMPD_DSC_CAPTUREBUFFER {
    MMP_ULONG  ulCompressStart;
    MMP_ULONG  ulCompressEnd;
    MMP_ULONG  ulLineStart;
    MMP_ULONG  ulExtCompStart;
    MMP_ULONG  ulExtCompEnd;
} MMPD_DSC_CAPTUREBUFFER;

typedef struct _MMPD_DSC_DECODEBUFFER {
    MMP_ULONG  ulDecompressStart;
    MMP_ULONG  ulDecompressEnd;
    MMP_ULONG  ulStorageTmpStart;
    MMP_ULONG  ulStorageTmpSize;
    MMP_ULONG  ulLineBufStart;
	MMP_ULONG  ulLineBufSize;
} MMPD_DSC_DECODEBUFFER;

typedef enum _MMPD_DSC_JPEGFORMAT
{
    MMPD_DSC_JPEG_FMT420 = 0,
    MMPD_DSC_JPEG_FMT422,  
    MMPD_DSC_JPEG_FMT422_V,
    MMPD_DSC_JPEG_FMT444,
    MMPD_DSC_JPEG_FMT411,
    MMPD_DSC_JPEG_FMT411_V,
    MMPD_DSC_JPEG_FMT_NOT_3_COLOR,
    MMPD_DSC_JPEG_FMT_NOT_BASELINE,
    MMPD_DSC_JPEG_FMT_FORMAT_ERROR

} MMPD_DSC_JPEGFORMAT;

typedef struct _MMPD_DSC_JPEGINFO {
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
} MMPD_DSC_JPEGINFO;

typedef enum _MMPD_DSC_CAPTURE_ROTATE_TYPE
{
    MMPD_DSC_CAPTURE_ROTATE_NO_ROTATE = 0,
    MMPD_DSC_CAPTURE_ROTATE_RIGHT_90,
    MMPD_DSC_CAPTURE_ROTATE_RIGHT_180,
    MMPD_DSC_CAPTURE_ROTATE_RIGHT_270
} MMPD_DSC_CAPTURE_ROTATE_TYPE;

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

MMP_ERR MMPD_DSC_SetCaptureBuffers(MMP_BOOL bExtbuffer, MMPD_DSC_CAPTUREBUFFER *capturebuf);
MMP_ERR MMPD_DSC_SetRotateCapture(MMPD_DSC_CAPTURE_ROTATE_TYPE RotateType,MMP_ULONG ulFrameAddr);
MMP_ERR MMPD_DSC_SetJpegResol(MMP_USHORT usCaptureBase, MMP_USHORT usJpegWidth, MMP_USHORT usJpegHeight);
MMP_ERR MMPD_DSC_TakeJpegPicture(MMP_USHORT usPreBufKeep, MMP_BOOL bPreviewOn, MMP_USHORT usCapFlow , MMP_UBYTE usRawThreshold, MMP_BOOL bPanorama, MMP_BOOL bIntelligentZoom);
MMP_ERR MMPD_DSC_TakeNextJpegPicture(MMP_BOOL bPreviewOn);

MMP_ERR MMPD_DSC_JpegDram2Card(MMP_ULONG ulWritesize, MMP_BOOL bFirstwrite, MMP_BOOL bLastwrite);	
MMP_ERR MMPD_DSC_JpegDram2Host(MMP_UBYTE* ubJpeghostbufaddr, MMP_ULONG ulJpegextmemoffset, MMP_ULONG ulretsize);
MMP_ERR MMPD_DSC_GetJpegPicture(MMP_USHORT *usJpegbufaddr, MMP_ULONG ulHostbuflimit);
MMP_ERR MMPD_DSC_GetJpegSize(MMP_ULONG *ulJpegfilesize);

MMP_ERR MMPD_DSC_SetDecodeBuffers(MMPD_DSC_DECODEBUFFER *decodebuf, MMP_BOOL biscardmode);
MMP_ERR MMPD_DSC_GetJpegInfo(MMPD_DSC_JPEGINFO *jpeginfo);

MMP_ERR	MMPD_DSC_SetFileNameAddr(MMP_ULONG ulFilenameAddr);
MMP_ERR MMPD_DSC_SetFileName(MMP_BYTE ubFilename[], MMP_USHORT usLength);

MMP_ERR MMPD_DSC_SetCardMode(MMP_BOOL bEnable);
MMP_ERR MMPD_RAWPROC_EnableZeroShutterLagPath(MMP_BOOL bEnable);

// EXIF and Thumbnail
MMP_ERR MMPD_DSC_SetCardModeExifEnc(MMP_BOOL bEncodeExif, MMP_BOOL bEncodeThumbnail, 
		MMP_USHORT usThumbnailwidth, MMP_USHORT usThumbnailheight, 
		MMP_ULONG ulMaxthumbnailsize, MMP_USHORT ubThumbnailInputMode,
	    MMP_USHORT ubThumbnailOutputMode, MMP_BOOL bFWCO);
MMP_ERR MMPD_DSC_WriteCardModeExif(MMP_ULONG ulThumbnailBufAddr, MMP_ULONG ulThumbnailJpegSize);
MMP_ERR MMPD_DSC_SetCardModeExifDec(MMP_BOOL bDecodeThumbnail);
MMP_ERR MMPD_DSC_GetExifOffset(MMP_ULONG* ulThumbnailOffset, MMP_ULONG* ulPrimaryOffset);
MMP_ERR MMPD_DSC_SetExifFillTime(MMP_ULONG ulStr0, MMP_ULONG ulStr1, MMP_ULONG ulStr2, MMP_ULONG ulStr3, MMP_ULONG ulStr4);
	
MMP_ERR MMPD_DSC_ChangeJpegExtCompAddr(MMP_ULONG ulExtCompStartAddr, MMP_ULONG ulExtCompEndAddr);
MMP_ERR MMPD_DSC_GetDisplayBufAddr(MMP_ULONG *ulDisplayBufAddr);

MMP_ERR MMPD_DSC_SetJpegQTable(MMP_UBYTE *ubQtable);
MMP_ERR MMPD_DSC_SetJpegQualityControl(MMP_BOOL bTargetCtrl,
                    MMP_BOOL bLimitCtrl, MMP_USHORT usTargetSize,
                    MMP_USHORT usLimitSize, MMP_USHORT usMaxCount, MMP_BOOL bSizePredictMode );
MMP_ERR	MMPD_DSC_SetJpegQualityArray(MMP_USHORT usHqFactor, MMP_USHORT usHqSize,
                    MMP_USHORT usLqFactor, MMP_USHORT usLqSize,
                    MMP_USHORT usInitFactor);
MMP_ERR MMPD_DSC_SetCapturePath(MMP_USHORT usUseLPF, MMP_USHORT usScalerPath,
                    MMP_USHORT usIBCPipe, MMP_USHORT usICOPipe);

// Panorama function
MMP_ERR MMPD_DSC_CalculatePramaMV(MMP_ULONG ulPictureAddr1, MMP_ULONG ulPictureAddr2,
								MMP_SHORT *sMvX, MMP_SHORT *sMvY);
MMP_ERR MMPD_DSC_WeightingPanoramaOverlapArea(MMP_ULONG ulPictureAddr1, MMP_ULONG ulPictureAddr2,
								MMP_SHORT sMvX,MMP_ULONG ulOverlayHeight);
MMP_ERR MMPD_DSC_InitialPanoramaConfig( MMP_SHORT usDSCmode,
                                        MMP_ULONG ulCaptureWidth,
                                        MMP_ULONG ulCaptureHeight,
                                        MMP_ULONG ulMaxShot,
                                        MMP_ULONG *ulPanoramaWidth,
                                        MMP_ULONG *ulPanoramaHeight);
MMP_ERR MMPD_DSC_SetPanoramaMem(MMP_ULONG ulMemStartAddr);
MMP_ERR MMPD_DSC_EncodePanoramaToJpeg(MMP_ULONG ulTargetWidth,MMP_ULONG ulTargetHeight,MMP_ULONG *ulSize);
MMP_ERR MMPD_DSC_MovePanoramaRaw(MMP_BOOL bDrawPanoramaOverlapArea,
                                 MMP_ULONG ulDisplayWidth, 
                                 MMP_ULONG ulDisplayHeight,
                                 MMP_ULONG ulDisplayOffsetX,
                                 MMP_ULONG ulDisplayOffsetY,
                                 MMP_BOOL  bRotateOverlapArea);
MMP_ERR MMPD_DSC_MergePanorama(void);   
MMP_ERR MMPD_DSC_TransformBuf(MMP_ULONG                 *ulSrcAddr,
                              MMP_ULONG                  ulSrcWidth,
                              MMP_ULONG                  ulSrcHeight,
                              MMPD_GRAPHICS_COLORDEPTH   SrcColorFormat,
                              MMP_ULONG                  *ulDestAddr,
                              MMP_ULONG                  ulDestWidth,
                              MMP_ULONG                  ulDestHeight,
                              MMPD_GRAPHICS_RECT         rect,
                              MMPD_GRAPHICS_COLORDEPTH   DestColorFormat) ;
MMP_ERR MMPD_DSC_Calibrate(MMP_ULONG ulRawStoreAddr, MMP_ULONG ulCalibrateAddr, MMP_BOOL ubLumaModeEn, MMP_UBYTE ubDownRatio, MMP_ULONG ulThreshold);
MMP_ERR MMPD_DSC_GetCalibrateInfo(MMP_USHORT *usDefectNum);
MMP_ERR MMPD_DSC_SetCalibrateInfo(MMP_UBYTE *ubHostAddress, MMP_BYTE *filename, MMP_ULONG ulCalibrateAddr, MMP_BOOL bCompensationEn);
MMP_ERR MMPD_DSC_SetStabShotParameter(MMP_ULONG ulMainAddr, MMP_ULONG ulSubAddr, MMP_USHORT usShotNum);
                                                                    								
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _INCLUDES_H_
/// @end_ait_only

