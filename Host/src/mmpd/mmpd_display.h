//==============================================================================
//
//  File        : mmph_display.h
//  Description : INCLUDE File for the Host Display Control driver function, including LCD/TV/Win
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmpd_display.h
 *  @brief The header File for the Display control functions
 *  @author Penguin Torng
 *  @version 1.0
 */

#ifndef _MMPD_DISPLAY_H_
#define _MMPD_DISPLAY_H_
#include "config_fw.h"

#include "mmp_lib.h"
#include "mmpd_graphics.h"

/** @addtogroup MMPD_Display
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


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPD_DISPLAY_CONTROLLER
{
    MMPD_DISPLAY_PRM_CTL = 0,  ///< Primary display controller
    MMPD_DISPLAY_SCD_CTL = 1,  ///< Secondary display controller
    MMPD_DISPLAY_CTL_MAX = 2
} MMPD_DISPLAY_CONTROLLER;

typedef enum _MMPD_DISPLAY_OUTPUTPANEL
{
    MMPD_DISPLAY_NONE = 0,
    MMPD_DISPLAY_P_LCD,        ///< Output device is P-LCD
    MMPD_DISPLAY_S_LCD,        ///< Output device is S-LCD
    MMPD_DISPLAY_P_LCD_FLM,    ///< Output device is P-LCD and FLM enable
    MMPD_DISPLAY_RGB_LCD,      ///< Output device is RGBLCD
    MMPD_DISPLAY_TV,           ///< Output device is TV
    MMPD_DISPLAY_HDMI,
    MMPD_DISPLAY_CCIR
} MMPD_DISPLAY_OUTPUTPANEL;

typedef enum _MMPD_LCD_COLORDEPTH
{
    MMPD_LCD_COLORDEPTH_16 = 0,  ///< Panel color is 16bit
    MMPD_LCD_COLORDEPTH_18,      ///< Panel color is 18bit
    MMPD_LCD_COLORDEPTH_24       ///< Panel color is 24bit
} MMPD_LCD_COLORDEPTH;

typedef enum _MMPD_DISPLAY_LCD_CSSEL
{
    MMPD_DISPLAY_CSSEL_1 = 0,
    MMPD_DISPLAY_CSSEL_2
} MMPD_DISPLAY_LCD_CSSEL;

typedef struct _MMPD_DISPLAY_LCDATTRIBUTE
{
    MMP_USHORT          usWidth;
    MMP_USHORT          usHeight;
    MMPD_LCD_COLORDEPTH	colordepth;
    MMP_ULONG			ulBgColor;
} MMPD_DISPLAY_LCDATTRIBUTE;

typedef enum _MMPD_TV_TYPE {
    MMPD_TV_TYPE_NTSC = 0,
    MMPD_TV_TYPE_PAL
} MMPD_TV_TYPE;

typedef enum _MMPD_DISPLAY_HDMIOUTPUTMODE
{
    MMPD_DISPLAY_HDMIOUTPUT_USERDEF,
    MMPD_DISPLAY_HDMIOUTPUT_720X480P,
    MMPD_DISPLAY_HDMIOUTPUT_1280X720P
} MMPD_DISPLAY_HDMIOUTPUTMODE;

typedef struct _MMPD_DISPLAY_TVATTRIBUTE
{
    MMPD_TV_TYPE        tvtype;
    MMP_USHORT          usStartX;
    MMP_USHORT          usStartY;
    MMP_USHORT          usDisplayWidth;
    MMP_USHORT          usDisplayHeight;
    MMP_ULONG			ulDspyBgColor;
    MMP_UBYTE			ubTvBgYColor;
    MMP_UBYTE			ubTvBgUColor;
    MMP_UBYTE			ubTvBgVColor;
} MMPD_DISPLAY_TVATTRIBUTE;

typedef struct _MMPD_DISPLAY_HDMIATTRIBUTE {
    MMP_UBYTE  ubColorType;
    //MMP_USHORT usStartX;					
    //MMP_USHORT usStartY;					
    MMP_USHORT usDisplayWidth;				
    MMP_USHORT usDisplayHeight;				
    MMP_ULONG  ulDspyBgColor;
    MMP_UBYTE  ubOutputMode;			
} MMPD_DISPLAY_HDMIATTRIBUTE;

typedef enum _MMP_DISPLAY_WINCOLORDEPTH
{
    MMPD_DISPLAY_WINCOLORDEPTH_4 = 0,
    MMPD_DISPLAY_WINCOLORDEPTH_8,
    MMPD_DISPLAY_WINCOLORDEPTH_16,
    MMPD_DISPLAY_WINCOLORDEPTH_24,
    MMPD_DISPLAY_WINCOLORDEPTH_YUV420,
    MMPD_DISPLAY_WINCOLORDEPTH_YUV422,
    MMPD_DISPLAY_WINCOLORDEPTH_32,
    MMPD_DISPLAY_WINCOLORDEPTH_YUV420_INTERLEAVE
} MMP_DISPLAY_WINCOLORDEPTH;

typedef struct _MMPD_DISPLAY_WINATTRIBUTE {
    MMP_USHORT          usWidth;
    MMP_USHORT          usHeight;
    MMP_USHORT          usLineOffset;
    MMP_DISPLAY_WINCOLORDEPTH     colordepth;
    MMP_ULONG           ulBaseAddr;
    MMP_ULONG           ulBaseUAddr;
    MMP_ULONG           ulBaseVAddr;
} MMPD_DISPLAY_WINATTRIBUTE;

typedef struct _MMPD_DISPLAY_WBACKATTRIBUTE {
    MMP_USHORT          usWidth;
    MMP_USHORT          usHeight;
    MMP_USHORT          usStartX;
    MMP_USHORT          usStartY;
    MMP_ULONG           ulBaseAddr;
    MMP_DISPLAY_WINCOLORDEPTH     colordepth;
    MMP_BOOL            bWriteBackOnly;
} MMPD_DISPLAY_WBACKATTRIBUTE;

typedef enum _MMPD_DISPLAY_WINID
{
    MMPD_DISPLAY_WIN_MAIN = 0,
    MMPD_DISPLAY_WIN_PIP = 1,
    MMPD_DISPLAY_WIN_OVERLAY = 2,
    MMPD_DISPLAY_WIN_ICON = 3,
    MMPD_DISPLAY_WIN_SCD_WIN = 4,
    MMPD_DISPLAY_WIN_CCIR = 5,
    MMPD_DISPLAY_WIN_MAX = 6
} MMPD_DISPLAY_WINID;

typedef enum _MMPD_DISPLAY_ROTATE_TYPE
{
    MMPD_DISPLAY_ROTATE_NO_ROTATE = 0,
    MMPD_DISPLAY_ROTATE_RIGHT_90,
    MMPD_DISPLAY_ROTATE_RIGHT_180,
    MMPD_DISPLAY_ROTATE_RIGHT_270,
    MMPD_DISPLAY_ROTATE_MAX
} MMPD_DISPLAY_ROTATE_TYPE;

typedef enum _MMPD_DISPLAY_COLORMODE
{
    MMPD_DISPLAY_COLOR_RGB565 = 0,
    MMPD_DISPLAY_COLOR_RGB888,
    MMPD_DISPLAY_COLOR_YUV422,
    MMPD_DISPLAY_COLOR_YUV420,
    MMPD_DISPLAY_COLOR_YUV420_INTERLEAVE
} MMPD_DISPLAY_COLORMODE;

typedef enum _MMPD_DISPLAY_SEMITP_FUNC
{
    MMPD_DISPLAY_SEMITP_AVG = (0x00 << 2),
    MMPD_DISPLAY_SEMITP_AND = (0x01 << 2),
    MMPD_DISPLAY_SEMITP_OR =(0x02 << 2),
    MMPD_DISPLAY_SEMITP_INV = (0x03 << 2)
} MMPD_DISPLAY_SEMITP_FUNC;

typedef enum _MMPD_DISPLAY_TV_SYNCMODE
{
	MMPD_DISPLAY_TV_FRAMESYNC = 0,
	MMPD_DISPLAY_TV_FIELDSYNC
} MMPD_DISPLAY_TV_SYNCMODE;

typedef struct _MMPD_DISPLAY_DISPATTRIBUTE {
    MMP_USHORT          usStartX;
    MMP_USHORT          usStartY;
    MMP_USHORT          usDisplayWidth;
    MMP_USHORT          usDisplayHeight;
    MMP_USHORT          usDisplayOffsetX;
    MMP_USHORT          usDisplayOffsetY;
    MMP_BOOL            bMirror;
    MMPD_DISPLAY_ROTATE_TYPE    rotatetype;
} MMPD_DISPLAY_DISPATTRIBUTE;

typedef struct _MMPD_LCD_INDEXCOMMAND {
    MMP_UBYTE          ubIdxCmdBits;
    MMP_ULONG          ubIdxCmdData[8];
} MMPD_LCD_INDEXCOMMAND;

typedef enum _MMPD_DISPLAY_DUPLICATE {
    MMPD_DISPLAY_DUPLICATE_1X = 0,
    MMPD_DISPLAY_DUPLICATE_2X,
    MMPD_DISPLAY_DUPLICATE_4X
} MMPD_DISPLAY_DUPLICATE;

typedef struct _MMPD_DISPLAY_RECT {
    MMP_USHORT usLeft;
    MMP_USHORT usTop;
    MMP_USHORT usWidth;
    MMP_USHORT usHeight;
} MMPD_DISPLAY_RECT;

typedef enum _MMPD_DISPLAY_ALPHA_FMT
{
    MMPD_DISPLAY_ALPHA_RGBA = 0,			///< Specify LCD alpha blending RGBA
    MMPD_DISPLAY_ALPHA_ARGB 				///< Specify LCD alpha blending ARGB
} MMPD_DISPLAY_ALPHA_FMT;

typedef enum _MMPD_ICON_ALPHA_DEPTH
{
    MMPD_ICO_FORMAT_RGB565   = 0x00,
    MMPD_ICO_FORMAT_ARGB3454 = 0x04,
    MMPD_ICO_FORMAT_ARGB4444 = 0x08,
    MMPD_ICO_FORMAT_ARGB8888 = 0x0C
} MMPD_ICON_ALPHA_DEPTH;

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
#if PCAM_EN==0
// Output Function
MMP_ERR MMPD_Display_SetPLCDOutput(MMPD_DISPLAY_CONTROLLER controller, 
							MMPD_DISPLAY_LCD_CSSEL cssel, MMPD_DISPLAY_LCDATTRIBUTE *lcdattribute);
MMP_ERR MMPD_Display_SetRGBLCDOutput(MMPD_DISPLAY_CONTROLLER controller,
							MMPD_DISPLAY_LCDATTRIBUTE *lcdattribute);
MMP_ERR MMPD_Display_GetWidthHeight(MMPD_DISPLAY_CONTROLLER controller, MMP_USHORT *usWidth, MMP_USHORT *usHeight);
MMP_ERR MMPD_Display_SetDisplayRefresh(MMPD_DISPLAY_CONTROLLER controller);

// Window Function
MMP_ERR MMPD_Display_GetWinAttributes(MMPD_DISPLAY_WINID winID,
                                                  MMPD_DISPLAY_WINATTRIBUTE *winattribute);
MMP_ULONG MMPD_Display_SetDisplayBackground(MMP_ULONG ulBgcolor);

MMP_ERR MMPD_Display_SetWinAttributes(MMPD_DISPLAY_WINID winID,
                    MMPD_DISPLAY_WINATTRIBUTE *winattribute);
MMP_ERR MMPD_Display_ClearWindowBuf(MMPD_DISPLAY_WINATTRIBUTE *winattribute,
                                    MMPD_GRAPHICS_RECT *rect, MMP_ULONG ulClearColor);
MMP_ULONG MMPD_Display_SetWinTransparent(MMPD_DISPLAY_WINID winID,
                    MMP_BOOL bTranspActive, MMP_ULONG ulTranspColor);
MMP_ERR MMPD_Display_SetWinSemiTransparent(MMPD_DISPLAY_WINID winID, MMP_BOOL bSemiTranspActive, 
												MMPD_DISPLAY_SEMITP_FUNC semifunc, MMP_USHORT usSemiWeight);
MMP_ERR   MMPD_Display_GetWinSemiTransparent(MMPD_DISPLAY_WINID winID, MMP_USHORT* usSemiWeight);
MMP_ERR MMPD_Display_UpdateWinAddr(MMPD_DISPLAY_WINID winID, 
                            MMP_ULONG ulBaseAddr, MMP_ULONG ulBaseUAddr, MMP_ULONG ulBaseVAddr);
MMP_ERR MMPD_Display_SetWinToDisplay(MMPD_DISPLAY_WINID winID,
						MMPD_DISPLAY_DISPATTRIBUTE *dispattribute);

MMP_ERR MMPD_Display_SetWinPriority(MMPD_DISPLAY_WINID prio1,
                    MMPD_DISPLAY_WINID prio2, MMPD_DISPLAY_WINID prio3, MMPD_DISPLAY_WINID prio4);
MMP_ERR MMPD_Display_SetWinActive(MMPD_DISPLAY_WINID winID, MMP_BOOL bActive);
MMP_ERR MMPD_Display_GetWinActive(MMPD_DISPLAY_WINID winID, MMP_BOOL *bActive);

MMP_ERR MMPD_Display_SetAlphaBlending(MMPD_DISPLAY_WINID        winID, 
                                      MMP_BYTE                  bAlphaEn, 
									  MMPD_DISPLAY_ALPHA_FMT    alphaformat,
									  MMPD_ICON_ALPHA_DEPTH     icon_alpha_depth);
									  
MMP_ERR	MMPD_Display_SetAlphaWeight(MMP_UBYTE* ubAlphaWeight);
/// @ait_only
MMP_ERR MMPD_Display_SetTVOutput(MMPD_DISPLAY_CONTROLLER controller, 
							MMPD_DISPLAY_TVATTRIBUTE *tvattribute);
MMP_ERR MMPD_Display_SetHDMIOutput(MMPD_DISPLAY_CONTROLLER controller,
                            MMPD_DISPLAY_HDMIATTRIBUTE *hdmiattribute);
MMP_ERR MMPD_Display_SetCCIROutput(MMPD_DISPLAY_CONTROLLER controller,
                            MMPD_CCIR_CCIRATTRIBUTE ccirattribute);
MMP_ERR MMPD_Display_GetOutput(MMPD_DISPLAY_CONTROLLER controller,
                            MMPD_DISPLAY_OUTPUTPANEL *output);
MMP_ERR MMPD_Display_SetDisplayPause(MMPD_DISPLAY_CONTROLLER controller, MMP_BOOL bPause);
MMP_ERR MMPD_Display_SetTVRegisterSyncMode(MMPD_DISPLAY_TV_SYNCMODE mode);

// LCD Function
MMP_ERR MMPD_LCD_SendCommand(MMP_ULONG ulCommand);
MMP_ERR MMPD_LCD_SendIndex(MMP_ULONG ulIndex);
MMP_ERR MMPD_LCD_SendIndexCommand(MMP_ULONG ulIndex, MMP_ULONG ulCommand);
MMP_ERR MMPD_LCD_GetIndexCommand(MMP_ULONG ulIndex, MMP_ULONG *ulCommand);

// Window Function
MMP_ERR MMPD_Display_BindBufToWin(MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPD_DISPLAY_WINID winID);
MMP_ERR MMPD_Display_DrawWholeBufToWin(MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPD_DISPLAY_WINID winID, 
						MMP_USHORT usDstOffsetX, MMP_USHORT usDstOffsetY);
MMP_ERR MMPD_Display_SetWinDuplicate(MMPD_DISPLAY_WINID winID, 
                        MMPD_DISPLAY_DUPLICATE width, MMPD_DISPLAY_DUPLICATE height);
MMP_ERR MMPD_Display_SetWinScaling(MMPD_DISPLAY_WINID winID, MMP_BOOL bUserDefine, 
                                   MMP_BOOL bKeepAspectRatio, MMPF_SCALER_FIT_RANGE *fitrange,
                                   MMPF_SCALER_GRABCONTROL *grabctl);
MMP_ERR MMPD_Display_LoadWinPalette(MMPD_DISPLAY_WINID winID, 
                    MMP_UBYTE *ubPalette, MMP_USHORT usEntry);
// HDMI Function
MMP_ERR MMPD_HDMI_Initialize(MMPD_DISPLAY_HDMIATTRIBUTE *hdmiattribute);
// TV Function
MMP_ERR MMPD_TV_Initialize(MMP_BOOL bInit,MMPD_TV_TYPE output_panel);
MMP_ERR MMPD_TV_EnableDisplay(MMP_UBYTE enable);
MMP_ERR MMPD_TV_SetInterface(MMPD_DISPLAY_TVATTRIBUTE *tvattruibute);
MMP_ERR MMPD_TV_EncRegSet(MMP_ULONG addr, MMP_ULONG data);
MMP_ERR MMPD_TV_EncRegGet(MMP_ULONG addr, MMP_ULONG *data);
MMP_ERR MMPD_TV_SetBrightLevel(MMP_UBYTE ubLevel);
MMP_ERR MMPD_TV_SetContrast(MMP_USHORT usLevel);
MMP_ERR MMPD_TV_SetSaturation(MMP_UBYTE ubUlevel, MMP_UBYTE ubVlevel);
//==============================================================================
//                           RGB LCD Display function
//==============================================================================
MMP_ERR MMPD_RGBLCD_SetPartialPosition(MMPD_DISPLAY_RECT *rect);
//==============================================================================
//                           Second Display Function
//==============================================================================
MMP_ERR MMPD_Display_WritebackPrmDisplay(MMP_ULONG addr, MMP_USHORT width, MMP_USHORT height,
                                         MMP_USHORT offsetX, MMP_USHORT offsetY, 
                                         MMP_DISPLAY_WINCOLORDEPTH colordepth, MMP_BOOL typeRGB);
MMP_ERR MMPD_Display_EnableWriteback(void);
MMP_ERR MMPD_Display_DisableWriteback(void);

#endif // PCAM_EN==0
/// @end_ait_only
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

/// @}
#endif // _MMPD_LCD_H_

