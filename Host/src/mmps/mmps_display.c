//==============================================================================
//
//  File        : mmps_display.c
//  Description : Ritian Display Control host function, including LCD/TV/Win
//  Author      : Alan Wu
//  Revision    : 1.0
//
//==============================================================================

/**
*  @file mmps_display.c
*  @brief The Display control functions
*  @author Alan Wu
*  @version 1.0
*/
#include "config_fw.h"

#if LCD_DISPLAY_ON

#if (CHIP == P_V2)

#include "mmp_lib.h"
#include "mmps_display.h"
#include "mmps_system.h"
#include "mmpd_system.h"
#include "ait_utility.h"
/** @addtogroup MMPS_DISPLAY
@{
*/

/**@brief The system display mode

Use @ref MMPS_Display_SetOutputPanel to assign the field value of it.
*/
static  MMPS_SYSTEM_DISPLAYMODES    m_displayModes;
/**@brief The display type in each controller

Use @ref MMPS_Display_SetOutputPanel to set it.
And use @ref MMPS_Display_GetOutputPanel to get the current mode.
*/
static  MMPS_DISPLAY_OUTPUTPANEL    m_displayCurOutput[MMPS_DISPLAY_CTL_MAX] 
										= {MMPS_DISPLAY_NONE, MMPS_DISPLAY_NONE};

/**@brief The OSD attribute

Use @ref MMPS_Display_ConfigOsd to assign the field value of it.
And @ref MMPS_Display_UpdateOsdImage will refer to it when update/refresh OSD
*/
static  MMPS_OSD_INFO               m_osdInfo;
/**@brief The available OSD start address in AIT's chip

Use @ref MMPS_Display_SetOsdAddress to assign the value of it.
And @ref MMPS_Display_UpdateOsdImage will refer to it when update/refresh OSD
*/
static  MMP_ULONG                   m_ulOsdStartAddr;
/**@brief The OSD attribute

Use @ref MMPS_Display_SetOsdAddress to assign the value of it.
And @ref MMPS_Display_UpdateOsdImage will refer to it when update/refresh OSD
*/
static  MMP_ULONG                   m_ulOsdSize;
/**@brief The OSD double buffer index

Use @ref MMPS_Display_ConfigOsd to toggle the value of it.
*/
static	MMP_ULONG					m_ulCurOsdIndex = 0;

//------------------------------------------------------------------------------
//  Function    : MMPS_Display_GetConfiguration
//  Description :
//------------------------------------------------------------------------------
/** @brief The function gets the Display configuration for the host application

The function gets the current Display configuration for reference by the host application. 

@return It return the pointer of the Display configuration data structure.
*/
MMPS_SYSTEM_DISPLAYMODES*  MMPS_Display_GetConfiguration(void)
{
    return &m_displayModes;
}

/// @}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_SetOutputPanel
//  Description :
//------------------------------------------------------------------------------
/** @brief The function designates the output display device

The function designates the output device 
@param[in] controller  the display controller
@param[in] displaypanel the display type
@return It reports the status of the operation.
*/
MMP_ERR  MMPS_Display_SetOutputPanel(MMPS_DISPLAY_CONTROLLER controller, MMPS_DISPLAY_OUTPUTPANEL displaypanel)
{
	MMPD_DISPLAY_LCDATTRIBUTE 	lcdattribute;
	MMPD_DISPLAY_WINID 			winID;
	MMPD_DISPLAY_CONTROLLER 	displaycontrol = MMPD_DISPLAY_PRM_CTL;
	
	#if (TV_EN)
	MMPD_DISPLAY_TVATTRIBUTE 	tvattribute;
	#endif
	#if (CHIP == P_V2)
	MMPD_DISPLAY_HDMIATTRIBUTE hdmiattribute;
	#endif
	MMPD_CCIR_CCIRATTRIBUTE ccirattribute;

	switch (controller) {
	case MMPS_DISPLAY_PRM_CTL:	
		displaycontrol = MMPD_DISPLAY_PRM_CTL;
		break;
	case MMPS_DISPLAY_SCD_CTL:	
		displaycontrol = MMPD_DISPLAY_SCD_CTL;
		break;
	}

	if (m_displayCurOutput[controller] == displaypanel) {
		// To reset the LCD width/height and some important things. In bypass mode, customer maybe 
		// use the partial LCD refresh and change the LCD width/height by LCD command. So when AIT 
		// is active, we have to change the LCD width/height back.
		switch (displaypanel) {
		case MMPS_DISPLAY_MAIN_LCD:
			m_displayModes.mainlcd.reinitialfx();
			break;
		case MMPS_DISPLAY_SUB_LCD:
			m_displayModes.sublcd.reinitialfx();
			break;
		}

		return	MMP_ERR_NONE;
	}		

	if (controller == MMPS_DISPLAY_PRM_CTL) {
		for (winID = 0; winID < 4; winID++)
			MMPD_Display_SetWinActive(winID, MMP_FALSE);

		MMPD_Display_SetWinPriority(MMPD_DISPLAY_WIN_MAIN, MMPD_DISPLAY_WIN_PIP, 
								MMPD_DISPLAY_WIN_OVERLAY, MMPD_DISPLAY_WIN_ICON); 			
	}

	switch (displaypanel) {
	case MMPS_DISPLAY_NONE:
        #if (TV_EN)
		if ((m_displayCurOutput[controller] == MMPS_DISPLAY_NTSC_TV) ||
			(m_displayCurOutput[controller] == MMPS_DISPLAY_PAL_TV)) {
		    MMPD_TV_EnableDisplay(MMP_FALSE);
		    
		    if (m_displayCurOutput[controller] == MMPS_DISPLAY_NTSC_TV)
			    MMPD_TV_Initialize(MMP_FALSE,MMPD_TV_TYPE_NTSC);
			else
    			MMPD_TV_Initialize(MMP_FALSE,MMPD_TV_TYPE_PAL);
    			
            MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_TV, MMP_FALSE);
		}
        #endif
		break;
	case MMPS_DISPLAY_MAIN_LCD:
        #if (TV_EN)
		if ((m_displayCurOutput[controller] == MMPS_DISPLAY_NTSC_TV) ||
			(m_displayCurOutput[controller] == MMPS_DISPLAY_PAL_TV)) {
		    MMPD_TV_EnableDisplay(MMP_FALSE);
		    
			if (m_displayCurOutput[controller] == MMPS_DISPLAY_NTSC_TV)
			    MMPD_TV_Initialize(MMP_FALSE,MMPD_TV_TYPE_NTSC);
			else
    			MMPD_TV_Initialize(MMP_FALSE,MMPD_TV_TYPE_PAL);
    			
            MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_TV, MMP_FALSE);
		}
        #endif
		if ((m_displayModes.mainlcd.lcdtype == MMPS_DISPLAY_P_LCD) ||
			(m_displayModes.mainlcd.lcdtype == MMPS_DISPLAY_RGB_LCD)) {
			if (m_displayModes.mainlcd.lcdtype == MMPS_DISPLAY_P_LCD)			
			    m_displayModes.mainlcd.initialfx();
	
			lcdattribute.usWidth = m_displayModes.mainlcd.usWidth;
			lcdattribute.usHeight = m_displayModes.mainlcd.usHeight;
			switch(m_displayModes.mainlcd.colordepth) {
			case  MMPS_LCD_COLORDEPTH_16:
				lcdattribute.colordepth = MMPD_LCD_COLORDEPTH_16;
            	break;
			case  MMPS_LCD_COLORDEPTH_18:
				lcdattribute.colordepth = MMPD_LCD_COLORDEPTH_18;
        	    break;
			case  MMPS_LCD_COLORDEPTH_24:
				lcdattribute.colordepth = MMPD_LCD_COLORDEPTH_24;
				break;
			}
			lcdattribute.ulBgColor = m_displayModes.mainlcd.ulBgColor;
			if (m_displayModes.mainlcd.lcdtype == MMPS_DISPLAY_P_LCD) {
				if (m_displayModes.mainlcd.cssel == MMPS_DISPLAY_CSSEL_1)
					MMPD_Display_SetPLCDOutput(displaycontrol, 
												MMPD_DISPLAY_CSSEL_1, &lcdattribute);
				else
					MMPD_Display_SetPLCDOutput(displaycontrol,
												MMPD_DISPLAY_CSSEL_2, &lcdattribute);						
			}
			else if (m_displayModes.mainlcd.lcdtype == MMPS_DISPLAY_RGB_LCD) {
				MMPD_Display_SetRGBLCDOutput(displaycontrol, &lcdattribute);
			}												
		}

		break;
	case MMPS_DISPLAY_SUB_LCD:
        #if (TV_EN)
		if  ((m_displayCurOutput[controller] == MMPS_DISPLAY_NTSC_TV) ||
			(m_displayCurOutput[controller] == MMPS_DISPLAY_PAL_TV)) {
		    MMPD_TV_EnableDisplay(MMP_FALSE);
		    
			if (m_displayCurOutput[controller] == MMPS_DISPLAY_NTSC_TV)
			    MMPD_TV_Initialize(MMP_FALSE,MMPD_TV_TYPE_NTSC);
			else
    			MMPD_TV_Initialize(MMP_FALSE,MMPD_TV_TYPE_PAL);
    			
            MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_TV, MMP_FALSE);
		}            
        #endif

		if ((m_displayModes.sublcd.lcdtype == MMPS_DISPLAY_P_LCD) ||
			(m_displayModes.sublcd.lcdtype == MMPS_DISPLAY_RGB_LCD)) {
			if (m_displayModes.sublcd.lcdtype == MMPS_DISPLAY_P_LCD)			
			    m_displayModes.sublcd.initialfx();

			lcdattribute.usWidth = m_displayModes.sublcd.usWidth;
			lcdattribute.usHeight = m_displayModes.sublcd.usHeight;
			switch(m_displayModes.sublcd.colordepth) {
			case  MMPS_LCD_COLORDEPTH_16:
				lcdattribute.colordepth = MMPD_LCD_COLORDEPTH_16;
				break;
			case  MMPS_LCD_COLORDEPTH_18:
				lcdattribute.colordepth = MMPD_LCD_COLORDEPTH_18;
				break;
			case  MMPS_LCD_COLORDEPTH_24:
				lcdattribute.colordepth = MMPD_LCD_COLORDEPTH_24;
				break;
			}
			lcdattribute.ulBgColor = m_displayModes.sublcd.ulBgColor;
			if (m_displayModes.mainlcd.lcdtype == MMPS_DISPLAY_P_LCD) {
				if (m_displayModes.mainlcd.cssel == MMPS_DISPLAY_CSSEL_1)
					MMPD_Display_SetPLCDOutput(displaycontrol, 
												MMPD_DISPLAY_CSSEL_1, &lcdattribute);
				else
					MMPD_Display_SetPLCDOutput(displaycontrol,
												MMPD_DISPLAY_CSSEL_2, &lcdattribute);						
			}
			else if (m_displayModes.mainlcd.lcdtype == MMPS_DISPLAY_RGB_LCD) {
				MMPD_Display_SetRGBLCDOutput(displaycontrol, &lcdattribute);
			}												
		}
		break;

    #if (TV_EN)        
	case MMPS_DISPLAY_NTSC_TV:
		tvattribute.tvtype = MMPD_TV_TYPE_NTSC;
		tvattribute.usStartX = m_displayModes.ntsctv.usStartX;
		tvattribute.usStartY = m_displayModes.ntsctv.usStartY;
		tvattribute.usDisplayWidth = m_displayModes.ntsctv.usDisplayWidth;
		tvattribute.usDisplayHeight = m_displayModes.ntsctv.usDisplayHeight;
		tvattribute.ulDspyBgColor = m_displayModes.ntsctv.ulDspyBgColor;
		tvattribute.ubTvBgYColor = m_displayModes.ntsctv.ubTvBgYColor;
		tvattribute.ubTvBgUColor = m_displayModes.ntsctv.ubTvBgUColor;
		tvattribute.ubTvBgVColor = m_displayModes.ntsctv.ubTvBgVColor;
		MMPD_Display_SetTVOutput(displaycontrol, &tvattribute);
		break;
	case MMPS_DISPLAY_PAL_TV:
		tvattribute.tvtype = MMPD_TV_TYPE_PAL;
		tvattribute.usStartX = m_displayModes.paltv.usStartX;
		tvattribute.usStartY = m_displayModes.paltv.usStartY;
		tvattribute.usDisplayWidth = m_displayModes.paltv.usDisplayWidth;
		tvattribute.usDisplayHeight = m_displayModes.paltv.usDisplayHeight;
		tvattribute.ulDspyBgColor = m_displayModes.paltv.ulDspyBgColor;
		tvattribute.ubTvBgYColor = m_displayModes.paltv.ubTvBgYColor;
		tvattribute.ubTvBgUColor = m_displayModes.paltv.ubTvBgUColor;
		tvattribute.ubTvBgVColor = m_displayModes.paltv.ubTvBgVColor;
		MMPD_Display_SetTVOutput(displaycontrol, &tvattribute);
		break;
    #endif
	
    case MMPS_DISPLAY_HDMI:
    #if (CHIP == P_V2)
        hdmiattribute.ubColorType = m_displayModes.hdmi.colortype;
        //hdmiattribute.usStartX = m_displayModes.hdmi.usStartX;
        //hdmiattribute.usStartY = m_displayModes.hdmi.usStartY;
        hdmiattribute.usDisplayWidth = m_displayModes.hdmi.usDisplayWidth;
        hdmiattribute.usDisplayHeight = m_displayModes.hdmi.usDisplayHeight;
        hdmiattribute.ulDspyBgColor = m_displayModes.hdmi.ulDspyBgColor;
        hdmiattribute.ubOutputMode = m_displayModes.hdmi.OutputMode;
        MMPD_Display_SetHDMIOutput(displaycontrol, &hdmiattribute);
        break;
    #endif
    case MMPS_DISPLAY_CCIR:
        ccirattribute.ubColorType = m_displayModes.ccir.ubColorType;
        ccirattribute.ubOutputColor = m_displayModes.ccir.ubOutputColor;
        ccirattribute.usDisplayWidth = m_displayModes.ccir.usDisplayWidth;
        ccirattribute.usDisplayHeight = m_displayModes.ccir.usDisplayHeight;
        ccirattribute.ubBusWidth = m_displayModes.ccir.ubBusWidth;
        ccirattribute.ubClockDiv = m_displayModes.ccir.ubClockDiv;
        MMPD_Display_SetCCIROutput(displaycontrol, ccirattribute);
        break;
	}

	m_displayCurOutput[controller] = displaypanel;

    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_GetOutputPanel
//  Description :
//------------------------------------------------------------------------------
/** @brief The function retrives the current output display device

The function retrives the current output display device 
@param[in] controller  the display controller
@param[out] displaypanel  the display type
@return It reports the status of the operation.
*/
MMP_ERR  MMPS_Display_GetOutputPanel(MMPS_DISPLAY_CONTROLLER controller, MMPS_DISPLAY_OUTPUTPANEL *displaypanel)
{
	*displaypanel = m_displayCurOutput[controller];

    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_SetWinPriority
//  Description :
//------------------------------------------------------------------------------
/** @brief The function sets window display priorities by window ID

The function sets window display priorities by window IDs, the input parameters. The first parameter
gives the highest priority to that window and the fourth parameter gives the lowest priority to it. The
function can be used for dual panels.

@param[in] prio1 window ID for the first priority
@param[in] prio2 window ID for the second priority
@param[in] prio3 window ID for the third priority
@param[in] prio4 window ID for the fourth priority

@return It reports the status of the operation.
*/
MMP_ERR MMPS_Display_SetWinPriority(MMPD_DISPLAY_WINID prio1, MMPD_DISPLAY_WINID prio2, 
                                                MMPD_DISPLAY_WINID prio3, MMPD_DISPLAY_WINID prio4)
{
    MMPD_Display_SetWinPriority(prio1, prio2, prio3, prio4);
    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_SetDisplayRefresh
//  Description :
//------------------------------------------------------------------------------
/** @brief Refresh display data from the frame buffer to LCD

@return It reports the total effect number
*/
MMP_ERR MMPS_Display_SetDisplayRefresh(MMPS_DISPLAY_CONTROLLER controller)
{
	MMPD_DISPLAY_CONTROLLER 	displaycontrol = MMPD_DISPLAY_PRM_CTL;

    #if (!(defined(MBOOT_FW) || defined(UPDATER_FW)))
    MMPS_M_STATE state;
    
    MMPS_VIDPLAY_GetState(&state);
    
    if(state == MMPS_STATE_EXECUTING)
        return MMP_ERR_NONE;
    #endif


	switch (controller) {
	case MMPS_DISPLAY_PRM_CTL:	
		displaycontrol = MMPD_DISPLAY_PRM_CTL;
		break;
	case MMPS_DISPLAY_SCD_CTL:	
		displaycontrol = MMPD_DISPLAY_SCD_CTL;
		break;
	}

    MMPD_Display_SetDisplayRefresh(displaycontrol);
    return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_ReadWinBuffer
//  Description :
//------------------------------------------------------------------------------
/** @brief The function reads the display data of the specified Window

@param[in] winID the Wiondow ID
@param[in] usMemPtr the address of host buffer
@param[in] usWidth the width of clip
@param[in] usHeight the height of clip
@param[in] usStartx the X offset of window
@param[in] usStarty the Y offset of window
@return It reports the status of the operation.
*/
/*
MMP_ERR MMPS_Display_ReadWinBuffer(MMPD_DISPLAY_WINID winID,
            MMP_USHORT *usMemPtr, MMP_USHORT usWidth, MMP_USHORT usHeight,
            MMP_USHORT usStartx, MMP_USHORT usStarty)
{
    MMPD_DISPLAY_WINATTRIBUTE winattribute;
    MMPD_GRAPHICS_BUFATTRIBUTE bufattribute;
    MMPD_GRAPHICS_RECT rect;

    MMPD_Display_GetWinAttributes(winID, &winattribute);

    bufattribute.usWidth = winattribute.usWidth;;
    bufattribute.usHeight = winattribute.usHeight;;
    bufattribute.usLineOffset = winattribute.usLineOffset;;
    bufattribute.ulBaseAddr = winattribute.ulBaseAddr;
    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_8)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_8;

    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_16)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_16;

    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_24)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_24;

    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_YUV422)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_YUV422;

	rect.usLeft = usStartx;
	rect.usTop = usStarty;
	rect.usWidth = usWidth;
	rect.usHeight = usHeight;
    MMPD_Graphics_CopyImageBuftoFIFO(usMemPtr, &bufattribute, &rect);

    return  MMP_ERR_NONE;
}
*/

//------------------------------------------------------------------------------
//  Function    : MMPS_Display_WriteWinBuffer
//  Description :
//------------------------------------------------------------------------------
/** @brief The function writes the data into the specified Window

@param[in] winID the Wiondow ID
@param[in] usMemPtr the address of host buffer
@param[in] usWidth the width of clip
@param[in] usHeight the height of clip
@param[in] usStartx the X offset of window
@param[in] usStarty the Y offset of window
@return It reports the status of the operation.
*/
/*
MMP_ERR MMPS_Display_WriteWinBuffer(MMPD_DISPLAY_WINID winID,
            MMP_USHORT *usMemPtr, MMP_USHORT usWidth, MMP_USHORT usHeight,
            MMP_USHORT usStartx, MMP_SHORT usStarty)
{
    MMPD_DISPLAY_WINATTRIBUTE winattribute;
    MMPD_GRAPHICS_BUFATTRIBUTE bufattribute;
    MMPD_GRAPHICS_RECT rect;

    // The ICON window is not supported
    if (winID == MMPD_DISPLAY_WIN_ICON)
        return MMP_DISPLAY_ERR_NOT_IMPLEMENTED;

    //Get the window info
    MMPD_Display_GetWinAttributes(winID, &winattribute);

    bufattribute.usWidth = winattribute.usWidth;;
    bufattribute.usHeight = winattribute.usHeight;;
    bufattribute.usLineOffset = winattribute.usLineOffset;;
    bufattribute.ulBaseAddr = winattribute.ulBaseAddr;
    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_8)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_8;

    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_16)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_16;

    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_24)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_24;

    if (winattribute.colordepth == MMPD_DISPLAY_WINCOLORDEPTH_YUV422)
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_YUV422;

    rect.usLeft = 0;
    rect.usTop = 0;
    rect.usWidth = usWidth;
    rect.usHeight = usHeight;
    MMPD_Graphics_CopyImageFIFOtoBuf(usMemPtr, MMPD_GRAPHICS_COLORDEPTH_16, usWidth*2, &bufattribute,
                                    &rect, usStartx, usStarty, MMPD_GRAPHICS_ROP_SRCCOPY);


    return  MMP_ERR_NONE;
}
*/

//------------------------------------------------------------------------------
//  Function    : MMPS_Display_ClearWinBuffer
//  Description :
//------------------------------------------------------------------------------
/** @brief The function writes the data into the specified Window

@param[in] winID the Wiondow ID
@param[in] ulClearColor the clear color. If window color attribute is YUV422. then the low-word (16-bits LSB)
		ulClearColor will be translated as the same YCb and YCr.If window color attribute is YUV422, then
		ulClearColor represent 0(31-24), Cr(23-16) Cb(15-8) and Y(7-0).
@param[in] usWidth the width of clip
@param[in] usHeight the height of clip
@param[in] usStartx the X offset of window
@param[in] usStarty the Y offset of window
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Display_ClearWinBuffer(MMPD_DISPLAY_WINID winID, MMP_ULONG ulClearColor,
								MMP_USHORT usWidth, MMP_USHORT usHeight, 
								MMP_USHORT usStartx, MMP_SHORT usStarty)
{
    MMPD_DISPLAY_WINATTRIBUTE winattribute;
    MMPD_GRAPHICS_RECT	rect;
    MMP_ULONG loop;
    MMP_ULONG *pwin_src_long;
    MMP_USHORT *pwin_src_short;
    
    // The ICON window is not supported
    if (winID == MMPD_DISPLAY_WIN_ICON) {
        //return MMP_DISPLAY_ERR_NOT_IMPLEMENTED;
        MMPD_ICON_BUFATTRIBUTE bufattribute;
        MMPD_Icon_GetAttributes(0, &bufattribute);
        winattribute.usWidth = bufattribute.usWidth;
        winattribute.usHeight = bufattribute.usHeight;
        if(bufattribute.colorformat == MMPD_ICON_COLOR_INDEX8){
            winattribute.usLineOffset = bufattribute.usWidth * 1;
            winattribute.colordepth = MMPD_DISPLAY_WINCOLORDEPTH_8;
        } else if(bufattribute.colorformat == MMPD_ICON_COLOR_RGB565){
            winattribute.usLineOffset = bufattribute.usWidth * 2;
            winattribute.colordepth = MMPD_DISPLAY_WINCOLORDEPTH_16;
        }
        winattribute.ulBaseAddr = bufattribute.ulBaseAddr;
    } else {
        //Get the window info        
        MMPD_Display_GetWinAttributes(winID, &winattribute);
    }

	rect.usLeft = usStartx;
	rect.usTop = usStarty;
	rect.usWidth = usWidth;
	rect.usHeight = usHeight;

#if (CHIP == P_V2)
    switch(winattribute.colordepth){
        case MMPD_DISPLAY_WINCOLORDEPTH_32:
            pwin_src_long = (MMP_ULONG *)winattribute.ulBaseAddr;
            for(loop=0;loop<winattribute.usWidth*winattribute.usHeight;++loop){
                *(pwin_src_long + loop) = ulClearColor;
            }
			return MMP_ERR_NONE;
            break;
        default:
            break;
    }
#endif
    return	MMPD_Display_ClearWindowBuf(&winattribute, &rect, ulClearColor);
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_DisableAllWindows
//  Description :
//------------------------------------------------------------------------------
/** @brief This function disables all windows

@return It reports the status of the operation.
*/
MMP_ERR MMPS_Display_DisableAllWindows(void)
{
    MMPD_Display_SetWinActive(MMPD_DISPLAY_WIN_PIP, MMP_FALSE);
    MMPD_Display_SetWinActive(MMPD_DISPLAY_WIN_ICON, MMP_FALSE);
    MMPD_Display_SetWinActive(MMPD_DISPLAY_WIN_OVERLAY, MMP_FALSE);
    MMPD_Display_SetWinActive(MMPD_DISPLAY_WIN_MAIN, MMP_FALSE);

    return MMP_ERR_NONE;
}

/** @brief This function tell OSD module the available memory map

@return It reports the status of the operation.
*/
//------------------------------------------------------------------------------
//  Function    : MMPS_Display_SetOsdAddress
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_Display_SetOsdAddress(MMP_ULONG ulOsdStartAddr, MMP_ULONG ulOsdSize)
{
    m_ulOsdStartAddr = ulOsdStartAddr;
    m_ulOsdSize = ulOsdSize;
    
    return MMP_ERR_NONE;    
}

/** @brief This function give OSD default address depends on difference application mode.

@return It reports the status of the operation.
*/
//------------------------------------------------------------------------------
//  Function    : MMPS_Display_GetOsdLocation
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPS_Display_GetOsdLocation(MMP_ULONG *ulDisplayOSDAddr, MMP_ULONG *ulDisplayOSDSize)
{
	if (MMPS_Display_GetConfiguration()->ulReservedOsdSize == 0) {
		if (MMPS_Display_GetConfiguration()->bDoubleBufferOSD) {
		    *ulDisplayOSDAddr = MMPS_System_GetConfiguration()->ulStackMemoryStart + 
    	    					MMPS_System_GetConfiguration()->ulStackMemorySize - 
        						(MMPS_Display_GetConfiguration()->mainlcd.usWidth * MMPS_Display_GetConfiguration()->mainlcd.usHeight*2*2);
		}
		else {
		    *ulDisplayOSDAddr = MMPS_System_GetConfiguration()->ulStackMemoryStart + 
    	    					MMPS_System_GetConfiguration()->ulStackMemorySize - 
        						(MMPS_Display_GetConfiguration()->mainlcd.usWidth * MMPS_Display_GetConfiguration()->mainlcd.usHeight*2);
		}        						
	}        					
	else {
		if (MMPS_Display_GetConfiguration()->bDoubleBufferOSD) {
		    *ulDisplayOSDAddr = MMPS_System_GetConfiguration()->ulStackMemoryStart + 
    	    					MMPS_System_GetConfiguration()->ulStackMemorySize - MMPS_Display_GetConfiguration()->ulReservedOsdSize * 2;
		}
		else {
		    *ulDisplayOSDAddr = MMPS_System_GetConfiguration()->ulStackMemoryStart + 
    	    					MMPS_System_GetConfiguration()->ulStackMemorySize - MMPS_Display_GetConfiguration()->ulReservedOsdSize;
		}    	    					
	}
	if (ulDisplayOSDSize != NULL) {        					
		if (MMPS_Display_GetConfiguration()->ulReservedOsdSize == 0) {
			if (MMPS_Display_GetConfiguration()->bDoubleBufferOSD) {
			    *ulDisplayOSDSize = MMPS_Display_GetConfiguration()->mainlcd.usWidth * MMPS_Display_GetConfiguration()->mainlcd.usHeight*2*2;
		    }
		    else {
		    	*ulDisplayOSDSize = MMPS_Display_GetConfiguration()->mainlcd.usWidth * MMPS_Display_GetConfiguration()->mainlcd.usHeight*2;
		    }
		}
		else {
			if (MMPS_Display_GetConfiguration()->bDoubleBufferOSD) {
				*ulDisplayOSDSize = MMPS_Display_GetConfiguration()->ulReservedOsdSize*2;
			}
			else {
			    *ulDisplayOSDSize = MMPS_Display_GetConfiguration()->ulReservedOsdSize;
			}			    
		}		    
	}
	return MMP_ERR_NONE;    
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_ConfigOsd
//  Description :
//------------------------------------------------------------------------------
/**
The OSD (on screen display) could be output from 4-window layer, which are
MAIN, PIP, OVERLAY and ICON in MMP. This section gives an introduction
how to use the OSD APIs of MMP.

There are four steps to output OSD on the display device (LCD panel) by calling the following OSD API

1.  MMPS_Display_SetOsdAddress()
2.	MMPS_Display_ConfigOsd()
3.	MMPS_Display_UpdateOsdImage()
4.	MMPS_Display_EnableOsd()

This function allocates the frame buffer (in MMP) and configures the window
attribute and display attribute for each OSD. There is a formal parameter,
which is a structure for this function need to be configured properly before
using it. The structure, MMPS_OSD_INFO, contains all the OSD information. User
needs to configure all OSD at one step. The MAIN, PIP and OVERLAP window layer
could output only one OSD. The ICON window layer could output up to 8 OSD.
Therefore, totally 11 OSD could be output to display device. The OSDs outputs
are grouped by window-layer.

@param[in] osdInfo the OSD information
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPS_Display_ConfigOsd(MMPS_OSD_INFO *osdInfo)
{
    MMP_USHORT 	i, j;
    MMP_ULONG	osd_offset, osdaddr, osd_size;
    MMPD_DISPLAY_WINATTRIBUTE	winattribute;
    MMPD_DISPLAY_DISPATTRIBUTE	dispattribute;
    MMPD_ICON_BUFATTRIBUTE 		iconattribute;

    m_osdInfo = *osdInfo;
    osdaddr = ALIGN32(m_ulOsdStartAddr);
    if (MMPS_Display_GetConfiguration()->bDoubleBufferOSD) {
    	if (m_ulCurOsdIndex == 0) {
			osd_offset = 0;
			m_ulCurOsdIndex = 1;
    	}
    	else {
    		if (MMPS_Display_GetConfiguration()->ulReservedOsdSize == 0) {
    			osd_offset = ALIGN32(MMPS_Display_GetConfiguration()->mainlcd.usWidth * MMPS_Display_GetConfiguration()->mainlcd.usHeight * 2);
    		}
    		else {
				osd_offset = ALIGN32(MMPS_Display_GetConfiguration()->ulReservedOsdSize);    		
    		}
    		m_ulCurOsdIndex = 0;
		}    	
	}
	else {
	    osd_offset = 0;
	}

    //Calculate memory map
    for (i = 0; i < m_osdInfo.usTotalWinLayer; i++) {
        switch(m_osdInfo.osd[i].wincolordepth) {
        case MMPD_DISPLAY_WINCOLORDEPTH_8:
        case MMPD_DISPLAY_WINCOLORDEPTH_16:
        case MMPD_DISPLAY_WINCOLORDEPTH_24:
            osd_size = m_osdInfo.osd[i].usWinWidth * (MMP_USHORT)(m_osdInfo.osd[i].wincolordepth) * m_osdInfo.osd[i].usWinHeight;
            break;
        case MMPD_DISPLAY_WINCOLORDEPTH_YUV420:
            if (m_osdInfo.osd[i].winid != MMPD_DISPLAY_WIN_ICON) {
                osd_size = (m_osdInfo.osd[i].usWinWidth * m_osdInfo.osd[i].usWinHeight * 3) >> 1;
                break;
            }
        case MMPD_DISPLAY_WINCOLORDEPTH_YUV422:
            if (m_osdInfo.osd[i].winid != MMPD_DISPLAY_WIN_ICON) {
                osd_size = m_osdInfo.osd[i].usWinWidth * 2 * m_osdInfo.osd[i].usWinHeight;
                break;
            }
#if (CHIP == P_V2)
        case MMPD_DISPLAY_WINCOLORDEPTH_32:
            osd_size = m_osdInfo.osd[i].usWinWidth * (MMP_USHORT)(4) * m_osdInfo.osd[i].usWinHeight;
            break;
#endif
        default:
            PRINTF("MMPS_Display_ConfigOsd() --> Not implemented (color format) \r\n");
            return MMP_DISPLAY_ERR_NOT_IMPLEMENTED;
        }

        //Calculate Window buffer address and size
        switch (m_osdInfo.osd[i].winid) {
        case MMPD_DISPLAY_WIN_MAIN:
        case MMPD_DISPLAY_WIN_PIP:
        case MMPD_DISPLAY_WIN_OVERLAY:
            m_osdInfo.osdMenAddr.ulWinBufferAddr[i] = osdaddr + osd_offset;
            if (m_osdInfo.osd[i].wincolordepth == MMPD_DISPLAY_WINCOLORDEPTH_YUV420) {
                m_osdInfo.osdMenAddr.ulWinBufferUAddr[i] = m_osdInfo.osdMenAddr.ulWinBufferAddr[i] + 
                                            (m_osdInfo.osd[i].usWinWidth * m_osdInfo.osd[i].usWinHeight);
                m_osdInfo.osdMenAddr.ulWinBufferVAddr[i] = m_osdInfo.osdMenAddr.ulWinBufferUAddr[i] +
                                            ((m_osdInfo.osd[i].usWinWidth * m_osdInfo.osd[i].usWinHeight) >> 2);
            }
            //PRINTF("MMPS_Display_ConfigOsd() --> OSD Addr Start => ", osdaddr + osd_offset);
            //RTNA_DBG_Str(0, "MMPS_Display_ConfigOsd() --> OSD Addr Start => 0x");   RTNA_DBG_Long(0, osdaddr + osd_offset);   RTNA_DBG_Str(0, "\r\n");
            osdaddr += ALIGN32(osd_size);
            //PRINTF("  End => 0x%X\r\n", osdaddr + osd_offset);
            //RTNA_DBG_Str(0, "  End => ");   RTNA_DBG_Long(0, osdaddr + osd_offset);   RTNA_DBG_Str(0, "\r\n");

            if ((osdaddr - m_ulOsdStartAddr) > m_ulOsdSize) {
                PRINTF("MMPS_Display_ConfigOsd() --> OSD memory not enough (1) \r\n");
                //RTNA_DBG_Str(0, "MMPS_Display_ConfigOsd() --> OSD memory not enough (1) \r\n");
                return MMP_DISPLAY_ERR_INSUFFICIENT_OSDMEMORY;
            }
            break;
        case MMPD_DISPLAY_WIN_ICON:
            m_osdInfo.osdMenAddr.ulWinBufferAddr[i] = 0xFFFFFFFF; //Use this vale as flag
            for (j = 0; j < m_osdInfo.osd[i].usIconNum; j++) {
				m_osdInfo.osdMenAddr.ulIconWinIconAddr[j]= osdaddr + osd_offset;
                PRINTF("MMPS_Display_ConfigOsd() --> OSD(%d,%d) Addr Start => %x,", i, j, osdaddr + osd_offset);
                //RTNA_DBG_Str(0, "MMPS_Display_ConfigOsd() MMPD_DISPLAY_WIN_ICON--> OSD Addr Start => 0x");   RTNA_DBG_Long(0, osdaddr + osd_offset);   RTNA_DBG_Str(0, "\r\n");
                osdaddr += ALIGN32(osd_size);
                PRINTF("  End => %x\r\n", osdaddr + osd_offset);
                //RTNA_DBG_Str(0, "  End => ");   RTNA_DBG_Long(0, osdaddr + osd_offset);   RTNA_DBG_Str(0, "\r\n");

                if ((osdaddr-m_ulOsdStartAddr) > m_ulOsdSize) {
                    PRINTF("MMPS_Display_ConfigOsd() --> OSD memory not enough (2) \r\n");
                    return MMP_DISPLAY_ERR_INSUFFICIENT_OSDMEMORY;
                }
            }
#if (CHIP == P_V2) 
//Not used
#else
							//for Icon Rotated buffer
            MMPS_Display_SetIconWinRotationAddr(osdaddr + osd_offset);
            PRINTF("MMPS_Display_SetIconWinRotationAddr Start => %x,", osdaddr + osd_offset);
            //RTNA_DBG_Str(0, "MMPS_Display_SetIconWinRotationAddr --> OSD Addr Start => 0x");   RTNA_DBG_Long(0, osdaddr + osd_offset);   RTNA_DBG_Str(0, "\r\n");
            osdaddr += ALIGN32(osd_size);
            PRINTF("  End => %x\r\n", osdaddr + osd_offset);
            //RTNA_DBG_Str(0, "  End => ");   RTNA_DBG_Long(0, osdaddr + osd_offset);   RTNA_DBG_Str(0, "\r\n");

            if ((osdaddr-m_ulOsdStartAddr) > m_ulOsdSize) {
                PRINTF("MMPS_Display_ConfigOsd() --> OSD memory not enough (3) \r\n");
                return MMP_DISPLAY_ERR_INSUFFICIENT_OSDMEMORY;
            }
#endif            
            break;
        default :
            return MMP_DISPLAY_ERR_PARAMETER;
        }
    }

    for(i = 0; i < m_osdInfo.usTotalWinLayer; i++) {
        //Set Win Attributes
        winattribute.usWidth = m_osdInfo.osd[i].usWinWidth;
        winattribute.usHeight = m_osdInfo.osd[i].usWinHeight;
        switch(m_osdInfo.osd[i].wincolordepth) {
        case MMPD_DISPLAY_WINCOLORDEPTH_8:
        case MMPD_DISPLAY_WINCOLORDEPTH_16:
        case MMPD_DISPLAY_WINCOLORDEPTH_24:
            winattribute.usLineOffset = m_osdInfo.osd[i].usWinWidth * (MMP_USHORT)(m_osdInfo.osd[i].wincolordepth);
            break;
        case MMPD_DISPLAY_WINCOLORDEPTH_YUV420:
            winattribute.usLineOffset = m_osdInfo.osd[i].usWinWidth;
            break;
        case MMPD_DISPLAY_WINCOLORDEPTH_YUV422:
            winattribute.usLineOffset = m_osdInfo.osd[i].usWinWidth * 2;
            break;
#if (CHIP == P_V2)
        case MMPD_DISPLAY_WINCOLORDEPTH_32:
            winattribute.usLineOffset = m_osdInfo.osd[i].usWinWidth * (MMP_USHORT)(4);
            break;
#endif            
        }
        
        winattribute.colordepth = m_osdInfo.osd[i].wincolordepth;
        winattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulWinBufferAddr[i];
        if (m_osdInfo.osd[i].wincolordepth == MMPD_DISPLAY_WINCOLORDEPTH_YUV420) {
            winattribute.ulBaseUAddr = m_osdInfo.osdMenAddr.ulWinBufferUAddr[i];
            winattribute.ulBaseVAddr = m_osdInfo.osdMenAddr.ulWinBufferVAddr[i];
        }
        MMPD_Display_SetWinAttributes(m_osdInfo.osd[i].winid, &winattribute);

        //Set Display Attributes
        dispattribute.usStartX = 0; //No grab supported
        dispattribute.usStartY = 0; //No grab supported
        dispattribute.usDisplayWidth = m_osdInfo.osd[i].usDispWidth;
        dispattribute.usDisplayHeight = m_osdInfo.osd[i].usDispHeight;
        dispattribute.usDisplayOffsetX = m_osdInfo.osd[i].usDispStartX;
        dispattribute.usDisplayOffsetY= m_osdInfo.osd[i].usDispStartY;
        dispattribute.bMirror = m_osdInfo.osd[i].bDispMirror;
        dispattribute.rotatetype = m_osdInfo.osd[i].disprotatetype;
        MMPD_Display_SetWinToDisplay(m_osdInfo.osd[i].winid, &dispattribute);

		if (m_osdInfo.osd[i].winid != MMPD_DISPLAY_WIN_ICON) {
	        MMPD_Display_SetWinTransparent(m_osdInfo.osd[i].winid, m_osdInfo.osd[i].bTpEnable,
    	                    m_osdInfo.osd[i].ulTpColor);
		}
        MMPD_Display_SetWinSemiTransparent(m_osdInfo.osd[i].winid, m_osdInfo.osd[i].bSemiTpEnable,
	   	                    m_osdInfo.osd[i].semifunc, m_osdInfo.osd[i].ubSemiWeight);

		//MMPD_Display_SetAlphaWeight(&m_osdInfo.ubAlphaWeight[0]);

        
#if (CHIP == D_V1)||(CHIP == PYTHON)
            MMPD_Display_SetAlphaBlending(m_osdInfo.osd[i].winid, m_osdInfo.osd[i].ubAlphaEnable, m_osdInfo.osd[i].alphaformat,MMPD_ICO_FORMAT_ARGB3454);
#endif
#if (CHIP == P_V2)
            //TODO
#if USE_ARGB8888_OSD
            MMPD_Display_SetAlphaBlending(m_osdInfo.osd[i].winid, m_osdInfo.osd[i].ubAlphaEnable, m_osdInfo.osd[i].alphaformat,MMPD_ICO_FORMAT_ARGB8888);        
#else
            MMPD_Display_SetAlphaBlending(m_osdInfo.osd[i].winid, m_osdInfo.osd[i].ubAlphaEnable, m_osdInfo.osd[i].alphaformat,MMPD_ICO_FORMAT_RGB565);        
#endif
#endif
        //Config OSD on Icon Window ++
        if (m_osdInfo.osd[i].winid == MMPD_DISPLAY_WIN_ICON) {
            for (j = 0; j < m_osdInfo.osd[i].usIconNum; j++){
                iconattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulIconWinIconAddr[j];
                iconattribute.usStartX = m_osdInfo.osd[i].usIconStartX[j];
                iconattribute.usStartY = m_osdInfo.osd[i].usIconStartY[j];
                iconattribute.usWidth = m_osdInfo.osd[i].usIconWidth[j];
                iconattribute.usHeight = m_osdInfo.osd[i].usIconHeight[j];
                iconattribute.colorformat = m_osdInfo.osd[i].iconcolorformat;
                iconattribute.ulTpColor = m_osdInfo.osd[i].ulIconTpColor;
                iconattribute.bTpEnable = m_osdInfo.osd[i].bIconbTpEnable;
                iconattribute.bSemiTpEnable = m_osdInfo.osd[i].bIconSemiTpEnable;
                iconattribute.ubIconWeight = m_osdInfo.osd[i].ubIconIconWeight;
                iconattribute.ubDstWeight = m_osdInfo.osd[i].ubIconDstWeight;
                MMPD_Icon_SetAttributes(j, &iconattribute);
                #if 0
                //For testing... for icon ++
                MMPD_Sticker_SetAttributes(j, &iconattribute);
        		MMPD_Sticker_SetEnable(j, MMP_TRUE);
                //For testing... for icon --
                #endif
            }
        }//Config OSD on Icon Window --
        #endif
    }

    //Set Window Priority
    MMPD_Display_SetWinPriority(m_osdInfo.prio1, m_osdInfo.prio2, m_osdInfo.prio3, m_osdInfo.prio4);
    
    return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPS_Display_UpdateOsdImage
//  Description :
//------------------------------------------------------------------------------
/**
This function updates the image of an OSD to be displayed on display device.
This function updates only one OSD each time.

@param[in] usLayerIndex The window-layer index which is configured by the MMPS_Display_ConfigOsd()
@param[in] usHostMemPtr The address of image buffer on host
@param[in] usSrcStartx The X offset of image in the image buffer on host
@param[in] usSrcStarty The Y offset of image in the image buffer on host
@param[in] usHostLineOffset The line offset of image in the image buffer on host
@return It reports the status of the operation.
*/
/*
MMP_ERR MMPS_Display_UpdateOsdImage(MMP_USHORT usLayerIndex, MMP_USHORT *usHostMemPtr,
                MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty, MMP_USHORT usHostLineOffset)
{
    MMP_ERR				retstatus;
    MMPD_GRAPHICS_BUFATTRIBUTE bufattribute;
    MMPD_GRAPHICS_RECT srcrect;   
    MMP_USHORT 			i;
    
    switch(m_osdInfo.osd[usLayerIndex].wincolordepth){
    case MMPD_DISPLAY_WINCOLORDEPTH_8:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_8;
        break;
    case MMPD_DISPLAY_WINCOLORDEPTH_16:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_16;
        break;
    case MMPD_DISPLAY_WINCOLORDEPTH_24:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_24;
        break;
#if (CHIP == P_V2)
    case MMPD_DISPLAY_WINCOLORDEPTH_32:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_32;
        break;
#endif        
    default:
        return MMP_DISPLAY_ERR_NOT_IMPLEMENTED;
    }

    if (m_osdInfo.osd[usLayerIndex].winid == MMPD_DISPLAY_WIN_ICON) {
        for (i = 0; i < m_osdInfo.osd[usLayerIndex].usIconNum; i++){
            bufattribute.usWidth = m_osdInfo.osd[usLayerIndex].usIconWidth[i];
            bufattribute.usHeight = m_osdInfo.osd[usLayerIndex].usIconHeight[i];
#if (CHIP == P_V2)        
            if(MMPD_DISPLAY_WINCOLORDEPTH_32 == m_osdInfo.osd[usLayerIndex].wincolordepth){
                bufattribute.usLineOffset = bufattribute.usWidth * ((MMP_USHORT)4);
            }
            else{
            bufattribute.usLineOffset = bufattribute.usWidth *
                        ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
            }
#else
            bufattribute.usLineOffset = bufattribute.usWidth *
                        ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
#endif
            bufattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulIconWinIconAddr[i];

            srcrect.usLeft = m_osdInfo.osd[usLayerIndex].usSrcStartX[i];
            srcrect.usTop = m_osdInfo.osd[usLayerIndex].usSrcStartY[i];
            srcrect.usWidth = bufattribute.usWidth;
            srcrect.usHeight = bufattribute.usHeight;
            
            retstatus = MMPD_Graphics_RotateImageFIFOtoBuf(usHostMemPtr,
                            bufattribute.colordepth, usHostLineOffset,&bufattribute,
                            &srcrect, 0, 0, MMPD_GRAPHICS_ROP_SRCCOPY,
                            m_osdInfo.osd[usLayerIndex].imagerotate);

            if (MMP_ERR_NONE != retstatus) {
                PRINTF("MMPS_Display_UpdateOsdImage() --> Fail to copy image via FIFO (icon %d)\r\n", i);
                return retstatus;
            }
        }
    }
    else {
        bufattribute.usWidth = m_osdInfo.osd[usLayerIndex].usWinWidth;
        bufattribute.usHeight = m_osdInfo.osd[usLayerIndex].usWinHeight;
#if (CHIP == P_V2)          
            if(MMPD_DISPLAY_WINCOLORDEPTH_32 == m_osdInfo.osd[usLayerIndex].wincolordepth){
                bufattribute.usLineOffset = m_osdInfo.osd[usLayerIndex].usWinWidth * ((MMP_USHORT)4);
            }
            else{
        bufattribute.usLineOffset = m_osdInfo.osd[usLayerIndex].usWinWidth *
                    ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
            }
#else        
        bufattribute.usLineOffset = m_osdInfo.osd[usLayerIndex].usWinWidth *
                    ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
#endif
        bufattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulWinBufferAddr[usLayerIndex];

        srcrect.usLeft = usSrcStartx;
        srcrect.usTop = usSrcStarty;
        srcrect.usWidth = bufattribute.usWidth;
        srcrect.usHeight = bufattribute.usHeight;

        retstatus = MMPD_Graphics_RotateImageFIFOtoBuf(usHostMemPtr,
                        bufattribute.colordepth, usHostLineOffset, &bufattribute,
                        &srcrect, 0, 0, MMPD_GRAPHICS_ROP_SRCCOPY,
                        m_osdInfo.osd[usLayerIndex].imagerotate);

        if (MMP_ERR_NONE != retstatus) {
            PRINTF("MMPS_Display_UpdateOsdImage() --> Fail to copy image via FIFO\r\n");
            return retstatus;
        }

    }

    return MMP_ERR_NONE;
}
*/

//------------------------------------------------------------------------------
//  Function    : MMPS_Display_EnableOsd
//  Description :
//------------------------------------------------------------------------------
/**
This function enables the output of an OSD to the display device

@param[in] sLayerIndex The window-layer index which is configured by the MMPS_Display_ConfigOsd()
           @a -1 Use -1 to enable all OSD window
@param[in] bEnable Enable or disable the output of an OSD on the display device
@return It reports the status of the operation.
*/
MMP_ERR MMPS_Display_EnableOsd(MMP_SHORT sLayerIndex, MMP_BOOL bEnable)
{
    MMP_USHORT i;
    
    if (sLayerIndex == -1) {
        for (i = 0; i < m_osdInfo.usTotalWinLayer; i++) {
            MMPS_Display_EnableOsd(i, bEnable);
        }
        return MMP_ERR_NONE;
    }
    
    if (m_osdInfo.osd[sLayerIndex].winid == MMPD_DISPLAY_WIN_ICON){
        for (i=0; i<m_osdInfo.osd[sLayerIndex].usIconNum; i++){
            MMPD_Icon_SetEnable(i, bEnable);
        }
        for (i=m_osdInfo.osd[sLayerIndex].usIconNum; i<8; i++){
            MMPD_Icon_SetEnable(i, MMP_FALSE);
        }
    }
    MMPD_Display_SetWinActive(m_osdInfo.osd[sLayerIndex].winid, bEnable);

    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_GetOsdInfo
//  Description :
//------------------------------------------------------------------------------
/** @brief The function gets the OSD configuration for the host application

The function gets the OSD configuration for reference by the host application. 

@return It return the pointer of the Display configuration data structure.
*/
MMPS_OSD_INFO* MMPS_Display_GetOsdInfo(void)
{
    return &m_osdInfo;
}


/************ Currently , not in use , but workable **************/
#if 0  //The original one, No rotation
//------------------------------------------------------------------------------
//  Function    : MMPS_Display_UpdateOsdImage
//  Description :
//------------------------------------------------------------------------------
MMPS_DISPLAY_STATUS MMPS_Display_UpdateOsdImage(MMP_USHORT usLayerIndex, MMP_USHORT *usHostMemPtr,
                MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty, MMP_USHORT usHostLineOffset)
{
    MMPD_GRAPHICS_STATUS retstatus;
    MMPD_GRAPHICS_BUFATTRIBUTE bufattribute;
    MMP_USHORT i;
    
    switch(m_osdInfo.osd[usLayerIndex].wincolordepth){
    case MMPD_DISPLAY_WINCOLORDEPTH_8:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_8;
        break;
    case MMPD_DISPLAY_WINCOLORDEPTH_16:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_16;
        break;
    case MMPD_DISPLAY_WINCOLORDEPTH_24:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_24;
        break;
    default:
        return MMPS_DISPLAY_FAILURE_PARAMETER;
    }

    if (m_osdInfo.osd[usLayerIndex].winid == MMPD_DISPLAY_WIN_ICON){
        for (i=0; i < m_osdInfo.osd[usLayerIndex].usIconNum; i++){
            bufattribute.usWidth = m_osdInfo.osd[usLayerIndex].usIconWidth[i];
            bufattribute.usHeight = m_osdInfo.osd[usLayerIndex].usIconWidth[i];
            bufattribute.usLineOffset = bufattribute.usWidth *
                        ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
            bufattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulIconWinIconAddr[i];

            retstatus = MMPD_Graphics_CopyImageFIFOtoBuf(usHostMemPtr,
                            bufattribute.colordepth, usHostLineOffset,&bufattribute,
                            bufattribute.usWidth, bufattribute.usHeight,
                            m_osdInfo.osd[usLayerIndex].usIconStartX[i], m_osdInfo.osd[usLayerIndex].usIconStartY[i], 0, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            if (MMPD_DISPLAY_SUCCESS != retstatus) {
                PRINTF("MMPS_Display_UpdateOsdImage() --> Fail to copy image via FIFO (icon %d)\r\n", i);
                return MMPS_DISPLAY_FAILURE;
            }
        }
    }
    else{
        bufattribute.usWidth = m_osdInfo.osd[usLayerIndex].usWinWidth;
        bufattribute.usHeight = m_osdInfo.osd[usLayerIndex].usWinHeight;
        bufattribute.usLineOffset = m_osdInfo.osd[usLayerIndex].usWinWidth *
                    ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
        bufattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulWinBufferAddr[usLayerIndex];

        retstatus = MMPD_Graphics_CopyImageFIFOtoBuf(usHostMemPtr,
                        bufattribute.colordepth, usHostLineOffset,&bufattribute,
                        bufattribute.usWidth, bufattribute.usHeight,
                        usSrcStartx, usSrcStarty, 0, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

        if (MMPD_DISPLAY_SUCCESS != retstatus) {
            PRINTF("MMPS_Display_UpdateOsdImage() --> Fail to copy image via FIFO\r\n");
            return MMPS_DISPLAY_FAILURE;
        }

    }

    return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Display_UpdateOsdImageRotation
//  Description :
//------------------------------------------------------------------------------

MMP_ERR MMPS_Display_UpdateOsdImageRotation(MMP_USHORT usLayerIndex, MMP_USHORT *usHostMemPtr,
            MMP_USHORT usSrcStartx, MMP_USHORT usSrcStarty, MMP_USHORT usHostLineOffset,
            MMPD_GRAPHICS_ROTATE_TYPE rotate)
{
    MMPD_GRAPHICS_STATUS retstatus;
    MMPD_GRAPHICS_BUFATTRIBUTE bufattribute;
    MMPD_GRAPHICS_RECT srcrect;   
    MMP_USHORT i;
    
    switch(m_osdInfo.osd[usLayerIndex].wincolordepth){
    case MMPD_DISPLAY_WINCOLORDEPTH_8:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_8;
        break;
    case MMPD_DISPLAY_WINCOLORDEPTH_16:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_16;
        break;
    case MMPD_DISPLAY_WINCOLORDEPTH_24:
        bufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_24;
        break;
    default:
        return MMPS_DISPLAY_FAILURE_PARAMETER;
    }

    if (m_osdInfo.osd[usLayerIndex].winid == MMPD_DISPLAY_WIN_ICON){
        for (i=0; i<m_osdInfo.osd[usLayerIndex].usIconNum; i++){
            bufattribute.usWidth = m_osdInfo.osd[usLayerIndex].usIconWidth[i];
            bufattribute.usHeight = m_osdInfo.osd[usLayerIndex].usIconWidth[i];
            bufattribute.usLineOffset = bufattribute.usWidth *
                        ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
            bufattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulIconWinIconAddr[i];

            srcrect.usLeft = m_osdInfo.osd[usLayerIndex].usSrcStartX[i];
            srcrect.usTop = m_osdInfo.osd[usLayerIndex].usSrcStartY[i];
            srcrect.usWidth = bufattribute.usWidth;
            srcrect.usHeight = bufattribute.usHeight;
            
            retstatus = MMPD_Graphics_RotateImageFIFOtoBuf(usHostMemPtr,
                            bufattribute.colordepth, usHostLineOffset,&bufattribute,
                            &srcrect, 0, 0, MMPD_GRAPHICS_ROP_SRCCOPY, rotate);

            if (MMPD_DISPLAY_SUCCESS != retstatus) {
                PRINTF("MMPS_Display_UpdateOsdImage() --> Fail to copy image via FIFO (icon %d)\r\n", i);
                return MMPS_DISPLAY_FAILURE;
            }
        }
    }
    else{
        bufattribute.usWidth = m_osdInfo.osd[usLayerIndex].usWinWidth;
        bufattribute.usHeight = m_osdInfo.osd[usLayerIndex].usWinHeight;
        bufattribute.usLineOffset = m_osdInfo.osd[usLayerIndex].usWinWidth *
                    ((MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth);
        bufattribute.colordepth = (MMP_USHORT)m_osdInfo.osd[usLayerIndex].wincolordepth;
        bufattribute.ulBaseAddr = m_osdInfo.osdMenAddr.ulWinBufferAddr[usLayerIndex];

        srcrect.usLeft = usSrcStartx;
        srcrect.usTop = usSrcStarty;
        srcrect.usWidth = bufattribute.usWidth;
        srcrect.usHeight = bufattribute.usHeight;
        
        retstatus = MMPD_Graphics_RotateImageFIFOtoBuf(usHostMemPtr,
                        bufattribute.colordepth, usHostLineOffset,&bufattribute,
                        &srcrect, 0, 0, MMPD_GRAPHICS_ROP_SRCCOPY, rotate);

        if (MMPD_DISPLAY_SUCCESS != retstatus) {
            PRINTF("MMPS_Display_UpdateOsdImage() --> Fail to copy image via FIFO\r\n");
            return MMPS_DISPLAY_FAILURE;
        }

    }

    return MMP_ERR_NONE;
}
#endif


//------------------------------------------------------------------------------
//  Function    : MMPS_TV_SetBrightLevel
//  Description :
//------------------------------------------------------------------------------
/**
@brief Brightness adjustment on Luma in CVBS/SVideo.
@param[in] ubLevel Brightness level.
@retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_TV_SetBrightLevel(MMP_UBYTE ubLevel)
{
    return MMPD_TV_SetBrightLevel(ubLevel);
}


//------------------------------------------------------------------------------
//  Function    : MMPS_TV_SetContrast
//  Description :
//------------------------------------------------------------------------------
/**
@brief Luma scale of contrast in main channel.
@param[in] usLevel Contrast level.
@retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_TV_SetContrast(MMP_USHORT usLevel)
{
    return MMPD_TV_SetContrast(usLevel);
}


//------------------------------------------------------------------------------
//  Function    : MMPS_TV_SetSaturation
//  Description :
//------------------------------------------------------------------------------
/**
@brief Chroma scale of saturation in main and second channels.
@param[in] ubUlevel Saturation U scale level.
@param[in] ubVlevel Saturation V scale level.
@retval MMP_ERR_NONE Success.
*/
MMP_ERR MMPS_TV_SetSaturation(MMP_UBYTE ubUlevel, MMP_UBYTE ubVlevel)
{
    return MMPD_TV_SetSaturation(ubUlevel, ubVlevel);
}

#endif //#if (CHIP == P_V2)
#endif
