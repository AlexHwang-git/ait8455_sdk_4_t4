//==============================================================================
//
//  File        : mmps_display.h
//  Description : INCLUDE File for the Host Display Control driver function, including LCD/TV/Win
//  Author      : Alan Wu
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmps_display.h
 *  @brief The header File for the Display control functions
 *  @author Alan Wu
 *  @version 1.0
 */

#ifndef _MMPS_DISPLAY_H_
#define _MMPS_DISPLAY_H_

#include	"mmp_lib.h"
#include	"mmpd_display.h"
#include	"mmpf_icon.h"

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
typedef enum _MMPS_DISPLAY_WINID
{
    MMPS_DISPLAY_WIN_MAIN = 0,		///< Display Main window, usually for OSD
    MMPS_DISPLAY_WIN_PIP = 1,		///< Display PIP window, usually for camera and video image
    MMPS_DISPLAY_WIN_OVERLAY = 2,	///< Display Overlay window, usually second layer of camera and video image.
    MMPS_DISPLAY_WIN_ICON = 3,		///< Display Icon window, usually for seperated OSD
    MMPS_DISPLAY_WIN_SCD_WIN = 4,	///< Display Second window, used in secondary controller
    MMPS_DISPLAY_WIN_MAX = 5		///< Display window total counts.
} MMPS_DISPLAY_WINID;

typedef enum _MMPS_DISPLAY_COLORMODE
{
    MMPS_DISPLAY_COLOR_RGB565 = 0,
    MMPS_DISPLAY_COLOR_RGB888,
    MMPS_DISPLAY_COLOR_YUV422,
    MMPS_DISPLAY_COLOR_YUV420
} MMPS_DISPLAY_COLORMODE;

typedef enum _MMPS_DISPLAY_HDMICOLORMODE
{
    MMPS_DISPLAY_HDMICOLOR_RGB888 = 0x00,
    MMPS_DISPLAY_HDMICOLOR_RGB666,
    MMPS_DISPLAY_HDMICOLOR_RGB565,
    MMPS_DISPLAY_HDMICOLOR_RGB444,
    MMPS_DISPLAY_HDMICOLOR_RGB333,
    MMPS_DISPLAY_HDMICOLOR_RGB332,
    MMPS_DISPLAY_HDMICOLOR_BGR888 = 0x08,
    MMPS_DISPLAY_HDMICOLOR_BGR666,
    MMPS_DISPLAY_HDMICOLOR_BGR565,
    MMPS_DISPLAY_HDMICOLOR_BGR444,
    MMPS_DISPLAY_HDMICOLOR_BGR333,
    MMPS_DISPLAY_HDMICOLOR_BGR332,
    MMPS_DISPLAY_HDMICOLOR_UY0VY1 = 0x20,
    MMPS_DISPLAY_HDMICOLOR_VY0UY1,
    MMPS_DISPLAY_HDMICOLOR_UY1VY0,
    MMPS_DISPLAY_HDMICOLOR_VY1UY0,
    MMPS_DISPLAY_HDMICOLOR_Y0UY1V,
    MMPS_DISPLAY_HDMICOLOR_Y0VY1U,
    MMPS_DISPLAY_HDMICOLOR_Y1UY0V,
    MMPS_DISPLAY_HDMICOLOR_Y1VY0U
} MMPS_DISPLAY_HDMICOLORMODE;

typedef enum _MMPS_DISPLAY_HDMIOUTPUTMODE
{
    MMPS_DISPLAY_HDMIOUTPUT_USERDEF,
    MMPS_DISPLAY_HDMIOUTPUT_720X480P,
    MMPS_DISPLAY_HDMIOUTPUT_1280X720P
} MMPS_DISPLAY_HDMIOUTPUTMODE;

typedef struct _MMPS_OSD_ATTRIBUTE
{
    //Window Attribute ++
    MMPD_DISPLAY_WINID  winid;                  ///< The window layer to output OSD
    MMP_USHORT          usWinWidth;             ///< The window width
    MMP_USHORT          usWinHeight;            ///< The window height
    MMP_DISPLAY_WINCOLORDEPTH  wincolordepth;   ///< The color depth of the window layer
    //Window Attribute --
    //Display Attribute ++
    MMP_USHORT          usDispWidth;            ///< The width of display area on display device
    MMP_USHORT          usDispHeight;           ///< The height of display area on display device
    MMP_USHORT          usDispStartX;           ///< The starting X on display device Origin
    MMP_USHORT          usDispStartY;           ///< The starting Y on display device Origin
    MMP_BOOL            bDispMirror;            ///< Mirror the image when output to display device
    MMPD_DISPLAY_ROTATE_TYPE  disprotatetype;   ///< Rotate the image when output to display device (By LCD controller)
    MMP_BOOL            bTpEnable;              ///< Enable the transparent color on the window layer
    MMP_ULONG           ulTpColor;              ///< The transparent color on the window layer
    MMP_BOOL            bSemiTpEnable;          ///< Enable the semi-transparent color on the window layer
    MMPD_DISPLAY_SEMITP_FUNC semifunc;          ///< The semi-transparent method
    MMP_UBYTE           ubSemiWeight;           ///< The weight for the semi-transparent
	MMP_UBYTE           ubAlphaEnable;			///< Enable the alpha blending color on the window layer
	MMPD_DISPLAY_ALPHA_FMT	alphaformat;	///< The alpha blending format
    //Display Attribute --
    //Buffer Copy Attribute ++
    MMPD_GRAPHICS_ROTATE_TYPE imagerotate;      //Image rotation for FIFO to frame buffer filling   ///< This flag is used for MMPS_Display_UpdateOsdImage(). If this flag is on, the image will be rotated when an OSD image is copied into the image buffer
    //Buffer Copy Attribute ++
    //Icons Attribute in ICON Window, Icon used for OSD on ICON WINDOW only ++
    MMP_USHORT          usIconNum;              ///< Total OSD number that is output from the ICON window-layer. 8 OSD maximum could be output thru ICON window-layer
    MMP_USHORT          usIconStartX[8];        ///< The starting X on display device Origin on ICON window-layer
    MMP_USHORT          usIconStartY[8];        ///< The starting Y on display device Origin on ICON window-layer
    MMP_USHORT          usIconWidth[8];         ///< The width of display area on display device on ICON window-layer
    MMP_USHORT          usIconHeight[8];        ///< The height of display area on display device on ICON window-layer
    MMPD_ICON_COLOR     iconcolorformat;        ///< The OSD color format that is output from ICON window-layer
    MMP_ULONG           ulIconTpColor;          ///< The transparent color on the ICON window-layer
    MMP_BOOL            bIconbTpEnable;         ///< Enable the transparent color on the ICON window-layer
    MMP_BOOL            bIconSemiTpEnable;      ///< Enable the semi-transparent color on the ICON window-layer
    MMP_UBYTE           ubIconIconWeight;       ///< The weight for the icon semi-transparent on the ICON window-layer
    MMP_UBYTE           ubIconDstWeight;        ///< The weight for the destination window-layer
    MMP_USHORT          usSrcStartX[8];         ///< Location in source, for grab. The starting X on image buffer of ICON window-layer
    MMP_USHORT          usSrcStartY[8];         ///< Location in source, for grab. The starting Y on image buffer of ICON window-layer
    //Icon Attributes in ICON Window, Icon used for OSD on ICON WINDOW only --
} MMPS_OSD_ATTRIBUTE;

typedef struct _MMPS_OSD_MEM
{
    MMP_ULONG ulWinBufferAddr[4];               ///< Store the each window address for OSD
    MMP_ULONG ulWinBufferUAddr[4];              ///< Store the each window U address for OSD
    MMP_ULONG ulWinBufferVAddr[4];              ///< Store the each window V address for OSD
    MMP_ULONG ulIconWinIconAddr[8];             ///< Store the each icon address for OSD
} MMPS_OSD_MEM;

typedef struct _MMPS_OSD_INFO
{
    MMP_USHORT         usTotalWinLayer; ///< Total window layer used to output OSD
    MMPS_OSD_ATTRIBUTE osd[4];          ///< Window attribute and display attribute of window layer that output OSD
    MMPS_OSD_MEM       osdMenAddr;      ///< The OSD memory map
    MMPD_DISPLAY_WINID prio1;           ///< The 1st window layer priority
    MMPD_DISPLAY_WINID prio2;           ///< The 2nd window layer priority
    MMPD_DISPLAY_WINID prio3;           ///< The 3rd window layer priority
    MMPD_DISPLAY_WINID prio4;           ///< The 4th window layer priority
	MMP_UBYTE			ubAlphaWeight[8];	///< The alpha blending weight for all layers
} MMPS_OSD_INFO;

typedef enum _MMPS_LCD_COLORDEPTH
{
    MMPS_LCD_COLORDEPTH_16 = 0,			///< Specify LCD colordepth 65536
    MMPS_LCD_COLORDEPTH_18,				///< Specify LCD colordepth 26K
    MMPS_LCD_COLORDEPTH_24				///< Specify LCD colordepth 16M
} MMPS_LCD_COLORDEPTH;

typedef enum _MMPS_DISPLAY_LCD_TYPE
{		
    MMPS_DISPLAY_P_LCD = 0,				///< LCD type, parallel interface, GRAM inside
    MMPS_DISPLAY_S_LCD,					///< LCD type, serial interface, GRAM inside
    MMPS_DISPLAY_RGB_LCD				///< LCD type, Hsync/Vsync interface, no GRAM inside
} MMPS_DISPLAY_LCD_TYPE;

typedef enum _MMPS_DISPLAY_LCD_CSSEL
{
    MMPS_DISPLAY_CSSEL_1 = 0,			///< LCD CS signal connected to PLCD_CS1
    MMPS_DISPLAY_CSSEL_2				///< LCD CS signal connected to PLCD_CS2
} MMPS_DISPLAY_LCD_CSSEL;

typedef enum _MMPS_DISPLAY_CONTROLLER
{
    MMPS_DISPLAY_PRM_CTL = 0,			///< Display main controller, has MAIN/PIP/OVERLAY/ICON layers.
    MMPS_DISPLAY_SCD_CTL = 1,			///< Display secondary controller, has secondary WIN only.
    MMPS_DISPLAY_CTL_MAX = 2			///< Display controller total count.
} MMPS_DISPLAY_CONTROLLER;

typedef MMP_BOOL MMPS_DISPLAY_INIT_FUNC (void);

typedef struct _MMPS_DISPLAY_LCDATTRIBUTE {
    MMPS_DISPLAY_LCD_TYPE		lcdtype;	///< LCD type
    MMPS_DISPLAY_LCD_CSSEL  cssel;			///< LCD CS control signal connection
    MMPS_DISPLAY_INIT_FUNC  *initialfx;		///< LCD related setting in AIT controller side
    MMPS_DISPLAY_INIT_FUNC  *reinitialfx;		///< LCD related setting in AIT controller side
    MMP_USHORT          usWidth;			///< LCD total width
    MMP_USHORT          usHeight;			///< LCD total height
    MMPS_LCD_COLORDEPTH colordepth;			///< LCD colordepth
    MMP_ULONG           ulBgColor;			///< LCD background color, when no window is enabled in that pixel
} MMPS_DISPLAY_LCDATTRIBUTE;

typedef struct _MMPS_DISPLAY_TVATTRIBUTE {
    MMP_USHORT usStartX;					///< TV display offset X, start from left of TV view region  
    MMP_USHORT usStartY;					///< TV display offset Y, start from top of TV view region  
    MMP_USHORT usDisplayWidth;				///< Display width 
    MMP_USHORT usDisplayHeight;				///< Display height
    MMP_ULONG  ulDspyBgColor;				///< Display background color, when no window is enabled in that pixel
    MMP_UBYTE  ubTvBgYColor;				///< TV background Y color, outside usDisplayWidth/usDisplayHeight 
    MMP_UBYTE  ubTvBgUColor;				///< TV background U color, outside usDisplayWidth/usDisplayHeight 
    MMP_UBYTE  ubTvBgVColor;				///< TV background V color, outside usDisplayWidth/usDisplayHeight 
} MMPS_DISPLAY_TVATTRIBUTE;

typedef struct _MMPS_DISPLAY_HDMIATTRIBUTE {
    MMPS_DISPLAY_HDMICOLORMODE colortype;
    //MMP_USHORT usStartX;					
    //MMP_USHORT usStartY;					
    MMP_USHORT usDisplayWidth;				
    MMP_USHORT usDisplayHeight;				
    MMP_ULONG  ulDspyBgColor;
    MMPS_DISPLAY_HDMIOUTPUTMODE OutputMode;				
} MMPS_DISPLAY_HDMIATTRIBUTE;

typedef struct _MMPS_DISPLAY_CCIRATTRIBUTE {
    MMP_UBYTE  ubColorType;
    MMP_UBYTE  ubOutputColor;
    MMP_USHORT usDisplayWidth;				
    MMP_USHORT usDisplayHeight;
    MMP_UBYTE  ubBusWidth;
    MMP_UBYTE  ubClockDiv;			
} MMPS_DISPLAY_CCIRATTRIBUTE;

typedef enum _MMPS_DISPLAY_OUTPUTPANEL {
	MMPS_DISPLAY_NONE = 0,					///< Choose no output device
	MMPS_DISPLAY_MAIN_LCD,					///< Choose MAIN LCD for output
	MMPS_DISPLAY_SUB_LCD,					///< Choose SUB LCD for output
	MMPS_DISPLAY_NTSC_TV,					///< Choose NTSC TV for output
	MMPS_DISPLAY_PAL_TV,					///< Choose PAL TV for output
	MMPS_DISPLAY_HDMI,                      ///< Choose HDMI for output
	MMPS_DISPLAY_CCIR                       ///< Choose CCIR for output
} MMPS_DISPLAY_OUTPUTPANEL;

typedef struct _MMPS_SYSTEM_DISPLAYMODES {
	MMPS_DISPLAY_LCDATTRIBUTE  mainlcd;		///< Main LCD attribute
	MMPS_DISPLAY_LCDATTRIBUTE  sublcd;		///< Sub LCD attribute
	MMPS_DISPLAY_TVATTRIBUTE   ntsctv;		///< NTSC TV attribute	
	MMPS_DISPLAY_TVATTRIBUTE   paltv;		///< NTSC PAL attribute	
	MMPS_DISPLAY_HDMIATTRIBUTE hdmi;        ///< HDMI attribute
	MMPS_DISPLAY_CCIRATTRIBUTE ccir;        ///< CCIR attribute	
	MMPS_DISPLAY_OUTPUTPANEL   OutputPanel;
	MMP_ULONG				   ulReservedOsdSize;	///< Reserved OSD size
	MMP_BOOL				   bDoubleBufferOSD;
} MMPS_SYSTEM_DISPLAYMODES;

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
MMPS_SYSTEM_DISPLAYMODES*  MMPS_Display_GetConfiguration(void);
MMP_ERR  MMPS_Display_SetOutputPanel(MMPS_DISPLAY_CONTROLLER controller, MMPS_DISPLAY_OUTPUTPANEL displaypanel);
MMP_ERR  MMPS_Display_GetOutputPanel(MMPS_DISPLAY_CONTROLLER controller, MMPS_DISPLAY_OUTPUTPANEL *displaypanel);
MMP_ERR  MMPS_Display_SetWinPriority(MMPD_DISPLAY_WINID prio1, MMPD_DISPLAY_WINID prio2, 
                                                MMPD_DISPLAY_WINID prio3, MMPD_DISPLAY_WINID prio4);
MMP_ERR  MMPS_Display_SetDisplayRefresh(MMPS_DISPLAY_CONTROLLER controller);


//Read the buffer data
MMP_ERR  MMPS_Display_ReadWinBuffer(MMPD_DISPLAY_WINID winID, MMP_USHORT *usMemPtr, 
            MMP_USHORT usWidth, MMP_USHORT usHeight, MMP_USHORT usStartx, MMP_USHORT usStarty);

//Write the buffer data
MMP_ERR  MMPS_Display_WriteWinBuffer(MMPD_DISPLAY_WINID winID,
            MMP_USHORT *usMemPtr, MMP_USHORT usWidth, MMP_USHORT usHeight,
            MMP_USHORT usStartx, MMP_SHORT usStarty);

//Clean specified Window with specified color
MMP_ERR  MMPS_Display_ClearWinBuffer(MMPD_DISPLAY_WINID winID, MMP_ULONG ulClearColor,
								MMP_USHORT usWidth, MMP_USHORT usHeight, 
								MMP_USHORT usStartx, MMP_SHORT usStarty);


MMP_ERR  MMPS_Display_DisableAllWindows(void);

//OSD function
MMP_ERR  MMPS_Display_GetOsdLocation(MMP_ULONG *ulDisplayOSDAddr, MMP_ULONG *ulDisplayOSDSize);
MMP_ERR  MMPS_Display_SetOsdAddress(MMP_ULONG ulOsdStartAddr, MMP_ULONG ulOsdSize);
MMP_ERR  MMPS_Display_ConfigOsd(MMPS_OSD_INFO *osdInfo);
MMP_ERR MMPS_Display_GetWinAttributes(MMP_USHORT usLayerIndex, MMP_USHORT *usWidth, MMP_USHORT *usHeight, MMP_ULONG *ulBuferAddr);
MMP_ERR  MMPS_Display_UpdateOsdImage(MMP_USHORT usLayerIndex, MMP_USHORT *usHostMemPtr,
            MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty, MMP_USHORT usHostLineOffset);
MMP_ERR  MMPS_Display_EnableOsd(MMP_SHORT sLayerIndex, MMP_BOOL bEnable);
//chris@0804
MMP_ERR   MMPS_Display_SetWinSemiTransparent(MMPS_DISPLAY_WINID winID, MMP_BOOL bSemiTranspActive, 
												MMPD_DISPLAY_SEMITP_FUNC semifunc, MMP_USHORT usSemiWeight);
MMP_ERR   MMPS_Display_GetWinSemiTransparent(MMPS_DISPLAY_WINID winID, MMP_USHORT* usSemiWeight);
MMPS_OSD_INFO*  MMPS_Display_GetOsdInfo(void);
#if (CHIP == P_V2) 
MMP_ERR MMPS_Display_SetIconWinRotationAddr(MMP_ULONG rotate_buf_addr);
MMP_ULONG MMPS_Display_GetIconWinRotationAddr(void);
#endif
MMP_ERR MMPS_Display_SetWinRotation(MMP_USHORT usLayerIndex, MMP_USHORT rotate_type);
MMP_ERR MMPS_Display_SetRotateMode(MMP_USHORT usLayerIndex, MMP_USHORT rotate_type);
MMP_ERR MMPS_Display_PartialUpdateOsdImage(MMP_USHORT usLayerIndex, void *usHostMemPtr,
                MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty, 
                MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight,
                MMP_USHORT srcColorDepth,
                MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, 
                MMP_USHORT usDstWidth, MMP_USHORT usDstHeight);
//Currently , not in use 
MMP_ERR  MMPS_Display_UpdateOsdImageRotation(MMP_USHORT usLayerIndex, MMP_USHORT *usHostMemPtr,
            MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty, MMP_USHORT usHostLineOffset,
            MMPD_GRAPHICS_ROTATE_TYPE rotate);

// TV setting: brightness, contrast, saturation
MMP_ERR  MMPS_TV_SetBrightLevel(MMP_UBYTE ubLevel);
MMP_ERR  MMPS_TV_SetContrast(MMP_USHORT usLevel);
MMP_ERR  MMPS_TV_SetSaturation(MMP_UBYTE ubUlevel, MMP_UBYTE ubVlevel);
MMP_ERR   MMPS_Display_ConvertBGRA8888ToARGB3454(MMP_USHORT srcWidth, MMP_USHORT srcHeight, MMP_USHORT *srcBuf, MMP_USHORT *dstBuf);
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================

#endif
