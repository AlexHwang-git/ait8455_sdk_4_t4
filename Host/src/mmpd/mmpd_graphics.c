/// @ait_only
//==============================================================================
//
//  File        : mmpd_graphics.c
//  Description : Ritian Graphics(2D) Engine Control driver function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
*  @file mmpd_graphics.c
*  @brief The Graphic control functions
*  @author Penguin Torng
*  @version 1.0
*/
#include "config_fw.h"

#ifdef BUILD_CE
#undef BUILD_FW
#endif

#include "mmp_lib.h"
#include "mmpd_graphics.h"
#include "mmpd_system.h"
#include "mmph_hif.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_graphics.h"
/** @addtogroup MMPD_Graphics
 *  @{
 */

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
static	MMP_UBYTE   m_bTransActive;
#if defined(SOFTWARE_2D_MODE)
MMP_ULONG   glFGKeyColor, glBGKeyColor;
#endif
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_SetTransparent
//  Description :
//------------------------------------------------------------------------------
/** @brief   The function configures window transparent with the transparency color.

  The function configures window transparent with the transparency color.
  @param[in] transpActive the transparency color
  @return It returns the previous transparency color configured
*/
#if PCAM_EN==0
MMP_UBYTE   MMPD_Graphics_SetTransparent(MMP_UBYTE transpActive)
{
    MMP_UBYTE   oldactive;

    oldactive = m_bTransActive;
    m_bTransActive = transpActive;

    return oldactive;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_CopyImageBuftoFIFO
//  Description :
//------------------------------------------------------------------------------
/** @brief The function copies image data from MMP memory to host memory via FIFO

  The function copies image data from MMP memory (frame buffer) to host memory via FIFO
  @param[out] usHostbufaddr the host momory address
  @param[in] bufattribute the graphic buffer
  @param[in] srcrect the rect of source image
  @return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_Graphics_CopyImageBuftoFIFO(MMP_USHORT *usHostbufaddr,
						MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPD_GRAPHICS_RECT *srcrect)
{
    MMP_ULONG   longtmp;
    MMP_USHORT  *pshorttmp;
    MMP_USHORT  cut_width, cut_height;
    MMP_USHORT  cut_width_short;
    MMP_USHORT  k;
    MMP_LONG    total_count;
	MMP_USHORT  fifocount;

	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_8) {
//		return MMP_GRA_ERR_NOT_IMPLEMENT;
	}

	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_24) {
		if ((bufattribute->usLineOffset & 0x1) || (bufattribute->usWidth & 0x1))
			return MMP_GRA_ERR_NOT_IMPLEMENT;
	}

	if (srcrect->usLeft >= bufattribute->usWidth || 
			srcrect->usTop >= bufattribute->usHeight ||
			!srcrect->usWidth || !srcrect->usHeight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if ((srcrect->usWidth + srcrect->usLeft) > bufattribute->usWidth)
		cut_width = bufattribute->usWidth - srcrect->usLeft;
	else	
		cut_width = srcrect->usWidth;

	if ((srcrect->usHeight + srcrect->usTop) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - srcrect->usTop;
	else	
		cut_height = srcrect->usHeight;

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

	// fast addressing mode only suuport for word-aligbed register access
	#if	(CHIP == P_V2)
	MMPH_HIF_RegSetL(GBL_FAST_IND_ADDR_0, GRA_BLT_FIFO_WR_SPC);
	#endif
	
    // set the graphics control parameter
    MMPH_HIF_RegSetW(GRA_BLT_FIFO_THR, GRAPHICS_FIFO_RD_WR_THRESHOLD);
    MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);
	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, ((0x1) << 2) | (0x1));
	}
	else {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, (((bufattribute->colordepth - 1)) << 2)
									| ((bufattribute->colordepth - 1)));
	}										
    MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_MEM_2_HOST | GRA_LEFT_TOP_RIGHT);

    // Deal Source Buffer
	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
    	longtmp = (bufattribute->ulBaseAddr)
	    			+ ((bufattribute->usLineOffset) * (srcrect->usTop))
		    		+ ((srcrect->usLeft) * 2);
    }
    else {
    	longtmp = (bufattribute->ulBaseAddr)
	    			+ ((bufattribute->usLineOffset) * (srcrect->usTop))
		    		+ ((srcrect->usLeft) * (bufattribute->colordepth));
    }		    		

    MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, longtmp);
	MMPH_HIF_RegSetL(GRA_BLT_SRC_PITCH, bufattribute->usLineOffset);

    MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, 0);

    pshorttmp = usHostbufaddr;

	switch (bufattribute->colordepth) {	// source format
    case MMPD_GRAPHICS_COLORDEPTH_8:
        if (!(cut_width & 0x0001))
            cut_width_short = cut_width >> 1;
        else
            cut_width_short = (cut_width+1) >> 1;
        break;
    case MMPD_GRAPHICS_COLORDEPTH_16:
	case MMPD_GRAPHICS_COLORDEPTH_YUV422:
        cut_width_short = cut_width;
        break;
    case MMPD_GRAPHICS_COLORDEPTH_24:
        if (!(cut_width & 0x0001))
            cut_width_short = (cut_width*3) >> 1;
        else
            cut_width_short = (cut_width*3+1) >> 1;
        break;
    }

    total_count = (cut_width_short * cut_height);
    k = 0;

    // need to reset the FIFO at first to ensure that there are no
    // data in the FIFO, or check the FIFO empty status
    MMPH_HIF_RegSetW(GRA_BLT_FIFO_RST, GRA_BLT_FIFO);
    if (MMPH_HIF_RegGetB(GRA_BLT_FIFO_RD_SPC) != 0) {
   		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
        return MMP_GRA_ERR_HW;
	}        

    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);

    if (total_count > GRAPHICS_FIFO_RD_THRESHOLD) {
        do {
			fifocount = MMPH_HIF_RegFGetW0() >> 8;
			
			if (fifocount) {
				total_count -= fifocount;

				MMPH_HIF_FIFOCReadW(GRA_BLT_FIFO_DATA, pshorttmp, fifocount);
				pshorttmp += fifocount;
			}
		} while (total_count > GRAPHICS_FIFO_RD_THRESHOLD);
	}

	while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

	while (total_count > 0) {
		fifocount = MMPH_HIF_RegFGetW0() >> 8;
			
		if (fifocount) {
			if (fifocount > total_count)
				fifocount = total_count;

			total_count -= fifocount;

			MMPH_HIF_FIFOCReadW(GRA_BLT_FIFO_DATA, pshorttmp, fifocount);
			pshorttmp += fifocount;
		}
	}

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_CopyImageFIFOtoBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief The function ...

  The function copy image data from host memory to MMP memory (frame buffer)
  @param[in] usHostbufaddr the momoey address
  @param[in] colordepth the color depth of the host memory
  @param[in] usHostLineOffset the line offset of image in host point point to
  @param[in] bufattribute the graphic buffer
  @param[in] srcrect the rect of source image
  @param[in] usDstStartx the x position of image in frame buffer
  @param[in] usDstStarty the y position of image in frame buffer
  @param[in] ropcode the Rop Code for graphics operation
  @return It reports the status of the operation.
*/
MMP_ERR MMPD_Graphics_CopyImageFIFOtoBuf(MMP_USHORT *usHostbufaddr,
						MMPD_GRAPHICS_COLORDEPTH colordepth, MMP_USHORT usHostLineOffset,
						MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPD_GRAPHICS_RECT *srcrect, 
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode)

{
	MMP_ULONG	longtmp = 0;
	MMP_USHORT	*pshorttmp = NULL;
	MMP_USHORT	fifocount = 0;
	MMP_USHORT	cut_width = 0, cut_height = 0;
	MMP_USHORT	cut_width_short = 0, width_short = 0;
	MMP_USHORT	j = 0;
	MMP_LONG	total_count = 0;

	if (colordepth == MMPD_GRAPHICS_COLORDEPTH_8) {
//		return MMP_GRA_ERR_NOT_IMPLEMENT;
	}
	
	if (colordepth == MMPD_GRAPHICS_COLORDEPTH_24) {
		if ((usHostLineOffset & 0x1) || (srcrect->usLeft & 0x1))
			return MMP_GRA_ERR_NOT_IMPLEMENT;
		if ((bufattribute->usLineOffset & 0x1) || (bufattribute->usWidth & 0x1))
			return MMP_GRA_ERR_NOT_IMPLEMENT;
	}

	if (usDstStartx >= bufattribute->usWidth ||
			usDstStarty >= bufattribute->usHeight ||
			!srcrect->usWidth || !srcrect->usHeight)  {
		return MMP_GRA_ERR_PARAMETER;
	}

	if (colordepth != (bufattribute->colordepth)) {
		return MMP_GRA_ERR_PARAMETER;
	}

	if ((srcrect->usWidth + usDstStartx) > bufattribute->usWidth)
		cut_width = bufattribute->usWidth - usDstStartx;
	else	
		cut_width = srcrect->usWidth;

	if ((srcrect->usHeight + usDstStarty) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - usDstStarty;
	else	
		cut_height = srcrect->usHeight;

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

	// set the graphics control parameter
	MMPH_HIF_RegSetW(GRA_BLT_FIFO_THR, GRAPHICS_FIFO_RD_WR_THRESHOLD);
	MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
	MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);
	MMPH_HIF_RegSetB(GRA_BLT_ROP, (MMP_UBYTE)ropcode & GRA_ROP_MASK);
	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, ((0x1) << 2) | 0x1);
	}
	else {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, ((bufattribute->colordepth - 1) << 2)
										|(colordepth - 1));
	}
	MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_OP_DO_ROP | GRA_HOST_2_MEM | GRA_LEFT_TOP_RIGHT);

	if (m_bTransActive)
		MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);

	// Deal Destination Buffer
	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
		longtmp = (bufattribute->ulBaseAddr)
					+ ((bufattribute->usLineOffset) * (usDstStarty))
					+ ((usDstStartx) * 2);
	}
	else {
		longtmp = (bufattribute->ulBaseAddr)
					+ ((bufattribute->usLineOffset) * (usDstStarty))
					+ ((usDstStartx) * (bufattribute->colordepth));
	}

	MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
	MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);

	MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, 0);

	switch (colordepth) {	// source format
		case MMPD_GRAPHICS_COLORDEPTH_8:
			if (!(cut_width & 0x0001))
				cut_width_short = cut_width >> 1;
			else
				cut_width_short = (cut_width+1) >> 1;

			width_short = usHostLineOffset >> 1;
			pshorttmp = usHostbufaddr + ((usHostLineOffset * srcrect->usTop) >> 1) + (srcrect->usLeft >> 1);
			break;
		case MMPD_GRAPHICS_COLORDEPTH_16:
		case MMPD_GRAPHICS_COLORDEPTH_YUV422:
			cut_width_short = cut_width;

			width_short = usHostLineOffset >> 1;
			pshorttmp = usHostbufaddr + ((usHostLineOffset * srcrect->usTop) >> 1) + (srcrect->usLeft);
			break;
		case MMPD_GRAPHICS_COLORDEPTH_24:
			if (!(cut_width & 0x0001))
				cut_width_short = (cut_width*3) >> 1;
			else
				cut_width_short = (cut_width*3+1) >> 1;

			width_short = usHostLineOffset >> 1;
			pshorttmp = usHostbufaddr + ((usHostLineOffset * srcrect->usTop + srcrect->usLeft * 3) >> 1);
			break;
    }

    total_count = (cut_width_short * cut_height);
    j = 0;

    // need to reset the FIFO at first to ensure that there are no
    // data in the FIFO, or check the FIFO empty status
    MMPH_HIF_RegSetW(GRA_BLT_FIFO_RST, GRA_BLT_FIFO);
    if (MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC) != GRA_FIFO_DEPTH) {
    	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
        return MMP_GRA_ERR_HW;
	}        

    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);

	if (width_short == cut_width_short) {
		do {
			fifocount = MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC);

			if (fifocount) {
				if (fifocount > total_count)
					fifocount = total_count;

				total_count -= fifocount;

				MMPH_HIF_FIFOCWriteW(GRA_BLT_FIFO_DATA, pshorttmp, fifocount);
				pshorttmp += fifocount;
			}
		} while (total_count > 0);
	}
	else {
		do {
			fifocount = MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC);

			if (fifocount) {
				if (fifocount > total_count)
					fifocount = total_count;

				total_count -= fifocount;

				MMPH_HIF_FIFOOpenW(GRA_BLT_FIFO_DATA, fifocount);
				for ( ; fifocount > 0; fifocount--) {
					if (j == (cut_width_short -1)) {
						MMPH_HIF_FIFOWriteW(*(pshorttmp++));
						pshorttmp += (width_short - cut_width_short);
						j = 0;
					}
					else {
						MMPH_HIF_FIFOWriteW(*(pshorttmp++));
						j++;
					}
				}
			}
		} while (total_count > 0);
	}

	// wait for the FIFO to be empty and graphic engine to stop
    while (MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
    return MMP_ERR_NONE;
}
//@end_ait_only

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_CopyImageBuftoBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief   The function copy image data from one frame buffer to another frame buffer

  The function copy image date from one buffer to another buffer
  @param[in] srcbufattribute the source graphic buffer
  @param[in] srcrect the rect of source image
  @param[in] dstbufattribute the destination graphic buffer
  @param[in] usDstStartx the x position of destination frame buffer
  @param[in] usDstStarty the y position of destination frame buffer
  @param[in] ropcode the Rop Code for graphics operation
  @return It reports the status of the operation.
*/
MMP_ERR MMPD_Graphics_CopyImageBuftoBuf(MMPD_GRAPHICS_BUFATTRIBUTE *srcbufattribute,
                        MMPD_GRAPHICS_RECT *srcrect, MMPD_GRAPHICS_BUFATTRIBUTE *dstbufattribute, 
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode)

{
    MMP_ULONG   longtmp;
    MMP_USHORT  cut_width, cut_height;
    MMP_USHORT  raster_dir;
    MMP_USHORT  colordepth;

	if (srcrect->usLeft >= srcbufattribute->usWidth || 
			srcrect->usTop >= srcbufattribute->usHeight ||
			usDstStartx >= dstbufattribute->usWidth || 
			usDstStarty >= dstbufattribute->usHeight ||
			!srcrect->usWidth || !srcrect->usHeight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if (srcbufattribute->colordepth != dstbufattribute->colordepth) {
        return MMP_GRA_ERR_PARAMETER;
	}			

    if(dstbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV420) {
        return MMP_GRA_ERR_PARAMETER;
	}	

    if(dstbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
        colordepth = 2;
	}	
    else {
        colordepth = dstbufattribute->colordepth;
    }     

	if ((srcrect->usWidth + srcrect->usLeft) > srcbufattribute->usWidth)
		cut_width = srcbufattribute->usWidth - srcrect->usLeft;
	else	
		cut_width = srcrect->usWidth;

	if ((srcrect->usHeight + srcrect->usTop) > srcbufattribute->usHeight)
		cut_height = srcbufattribute->usHeight - srcrect->usTop;
	else	
		cut_height = srcrect->usHeight;

	if ((cut_width + usDstStartx) > dstbufattribute->usWidth)
		cut_width = dstbufattribute->usWidth - usDstStartx;

	if ((cut_height + usDstStarty) > dstbufattribute->usHeight)
		cut_height = dstbufattribute->usHeight - usDstStarty;

    raster_dir = GRA_LEFT_TOP_RIGHT;

	if (srcbufattribute->ulBaseAddr == dstbufattribute->ulBaseAddr) {
		if (srcrect->usLeft == usDstStartx) {
			if (srcrect->usTop > usDstStarty) {
				raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
			}
			else {	// include (srcx == tgtx) and (srcy == tgty)
				raster_dir = GRA_LEFT_BOT_RIGHT;	// checked
			}	
		}	
		else if (srcrect->usTop == usDstStarty) {
			if (srcrect->usLeft > usDstStartx) {
				raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
			}	
			else {		
				raster_dir = GRA_RIGHT_BOT_LEFT;	// checked
			}
		}
		// Detect Overlap	
		else if (srcrect->usLeft > usDstStartx) {
			if ((usDstStartx + srcrect->usWidth) > srcrect->usLeft) {
				//(S:down-right d:upper-left)	
				if (srcrect->usTop > usDstStarty) {
					raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
				}
				//(S:upper-right d:down-left)
				else if (srcrect->usTop < usDstStarty) {
					raster_dir = GRA_LEFT_BOT_RIGHT;	// checked
				}		
			}	
		}
		else if (srcrect->usLeft < usDstStartx) {
			if ((srcrect->usLeft + srcrect->usWidth) > usDstStartx) {
				//(S:down-left d:upper-right)
				if (srcrect->usTop > usDstStarty) {
					raster_dir = GRA_LEFT_TOP_RIGHT;	// checked
				}
				//(S:upper-left d:down-right)
				else if (srcrect->usTop < usDstStarty) {	
					raster_dir = GRA_LEFT_BOT_RIGHT;	// checked
				}
			}	
		}	
	}

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

	switch (raster_dir) {
		case GRA_LEFT_TOP_RIGHT:
			MMPH_HIF_RegSetW(GRA_BLT_ROP_CTL, GRA_LEFT_TOP_RIGHT);

			// Deal Source Buffer
			longtmp = (srcbufattribute->ulBaseAddr)
					+ ((srcbufattribute->usLineOffset) * (srcrect->usTop)) 
					+ ((srcrect->usLeft) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, longtmp);

        // Deal Destination Buffer
			longtmp = (dstbufattribute->ulBaseAddr)
					+ ((dstbufattribute->usLineOffset) * (usDstStarty))
					+ ((usDstStartx) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
        break;

    case GRA_RIGHT_BOT_LEFT:
			MMPH_HIF_RegSetW(GRA_BLT_ROP_CTL, GRA_RIGHT_BOT_LEFT);

        // Deal Source Buffer
			longtmp = (srcbufattribute->ulBaseAddr)
				+ ((srcbufattribute->usLineOffset) * (srcrect->usTop + cut_height - 1)) 
				+ ((srcrect->usLeft + cut_width - 1) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, longtmp);

        // Deal Destination Buffer
			longtmp = (dstbufattribute->ulBaseAddr)
				+ ((dstbufattribute->usLineOffset) * (usDstStarty + cut_height - 1))
				+ ((usDstStartx + cut_width - 1) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
        break;

    case GRA_LEFT_BOT_RIGHT:
			MMPH_HIF_RegSetW(GRA_BLT_ROP_CTL, GRA_LEFT_BOT_RIGHT);

        // Deal Source Buffer
			longtmp = (srcbufattribute->ulBaseAddr)
				+ ((srcbufattribute->usLineOffset) * (srcrect->usTop + cut_height - 1)) 
				+ ((srcrect->usLeft) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, longtmp);

        // Deal Destination Buffer
			longtmp = (dstbufattribute->ulBaseAddr)
				+ ((dstbufattribute->usLineOffset) * (usDstStarty + cut_height - 1))
				+ ((usDstStartx) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
        break;

    case GRA_RIGHT_TOP_LEFT:
			MMPH_HIF_RegSetW(GRA_BLT_ROP_CTL, GRA_RIGHT_TOP_LEFT);

        // Deal Source Buffer
			longtmp = (srcbufattribute->ulBaseAddr)
				+ ((srcbufattribute->usLineOffset) * (srcrect->usTop)) 
				+ ((srcrect->usLeft + cut_width - 1) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, longtmp);

        // Deal Destination Buffer
			longtmp = (dstbufattribute->ulBaseAddr)
				+ ((dstbufattribute->usLineOffset) * (usDstStarty - 1))
				+ ((usDstStartx + cut_width - 1) * colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
        break;
    }

    MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);   
    MMPH_HIF_RegSetB(GRA_BLT_FMT, ((colordepth - 1) << 2)	// dst
									| (colordepth - 1));    
    MMPH_HIF_RegSetB(GRA_BLT_ROP, (MMP_UBYTE)ropcode & GRA_ROP_MASK);
    MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) |
		GRA_OP_DO_ROP | GRA_MEM_2_MEM);

    if (m_bTransActive)
        MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);

    // Deal Source Buffer
	MMPH_HIF_RegSetL(GRA_BLT_SRC_PITCH, srcbufattribute->usLineOffset);

    // Deal Destination Buffer
	MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, dstbufattribute->usLineOffset);


    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
    while (MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
    return MMP_ERR_NONE;
}
#endif


//@ait_only
//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_SetKeyColor
//  Description :
//------------------------------------------------------------------------------
/** @brief The function sets foreground or background color for Graphics

The function sets foreground or background color for Graphics drawing by programming Graphics controller
registers

  @param[in] keycolor the key color type (foreground or background)
  @param[in] ulColor the color to be set
  @return It returns the previous color
*/
#if PCAM_EN==0
MMP_ULONG   MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_KEYCOLOR keycolor, MMP_ULONG ulColor)
{
    MMP_ULONG   oldcolor;

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

    if (keycolor == MMPD_GRAPHICS_FG_COLOR) {
#if defined(SOFTWARE_2D_MODE)
        oldcolor = glFGKeyColor;
        glFGKeyColor = ulColor;
#else
        oldcolor = MMPH_HIF_RegGetL(GRA_BLT_FG);
        MMPH_HIF_RegSetL(GRA_BLT_FG, ulColor);
	#endif        
    } else if (keycolor == MMPD_GRAPHICS_BG_COLOR) {
#if defined(SOFTWARE_2D_MODE)
        oldcolor = glBGKeyColor;
        glBGKeyColor = ulColor;
#else
        oldcolor = MMPH_HIF_RegGetL(GRA_BLT_BG);
        MMPH_HIF_RegSetL(GRA_BLT_BG, ulColor);
	#endif
    }

 	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);

    return oldcolor;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_DrawRectToBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief The function draws a rectangle to the buffer with its buffer ID

The function draws a rectangle to the buffer with its buffer ID (usBufID). The second parameter gives
the rectangle information. The third parameter gives the type of rectangle fill. The fourth parameter
gives the ROP (Raster Operation) code for BitBlt operation. The system supports 16 raster operations.

  @param[in] bufattribute the graphic buffer
  @param[in] rect the rectangle information
  @param[in] filltype the rectangle fill type
  @param[in] ropcode the Rop Code for graphics operation
  @return It reports the status of the operation.
*/
MMP_ERR    MMPD_Graphics_DrawRectToBuf(MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
						MMPD_GRAPHICS_RECT *rect, MMPD_GRAPHICS_RECTFILLTYPE filltype, MMP_USHORT usFrameLength,
						MMPD_GRAPHICS_ROP ropcode)
{
    MMP_ULONG   longtmp;
    MMP_USHORT  cut_width, cut_height;
#if defined(SOFTWARE_2D_MODE)
	MMP_ULONG   x, y, fillval;
#endif
    if (rect->usLeft >= bufattribute->usWidth ||
			rect->usTop >= bufattribute->usHeight ||
        !rect->usWidth || !rect->usHeight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if ((rect->usWidth + rect->usLeft) > bufattribute->usWidth)
		cut_width = bufattribute->usWidth - rect->usLeft;
    else
        cut_width = rect->usWidth;

	if ((rect->usHeight + rect->usTop) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - rect->usTop;
    else
        cut_height = rect->usHeight;

    if (filltype == MMPD_GRAPHICS_SOLID_FILL) {
#if defined(SOFTWARE_2D_MODE)
        fillval = glFGKeyColor;
        switch (bufattribute->colordepth) {
        case MMPD_GRAPHICS_COLORDEPTH_8:        
            for (y = rect->usTop; y < (rect->usTop + rect->usHeight); y++) {
                for (x = rect->usLeft; x < (rect->usLeft + rect->usWidth); x++) {   
                    MMPH_HIF_MemSetB(bufattribute->ulBaseAddr + 
                                    (y * bufattribute->usLineOffset) + x, (MMP_UBYTE)fillval);
                }
            }    
            break;
        case MMPD_GRAPHICS_COLORDEPTH_16:        
        case MMPD_GRAPHICS_COLORDEPTH_YUV422:        
            for (y = rect->usTop; y < (rect->usTop + rect->usHeight); y++) {
                for (x = rect->usLeft; x < (rect->usLeft + rect->usWidth); x++) {   
                    MMPH_HIF_MemSetW(bufattribute->ulBaseAddr + 
                                    (y * bufattribute->usLineOffset) + x * 2, (MMP_USHORT)fillval);
                }
            }    
            break;
        case MMPD_GRAPHICS_COLORDEPTH_24:        
            for (y = rect->usTop; y < (rect->usTop + rect->usHeight); y++) {
                for (x = rect->usLeft; x < (rect->usLeft + rect->usWidth); x++) {   
                    MMPH_HIF_MemSetB(bufattribute->ulBaseAddr + 
                                    (y * bufattribute->usLineOffset) + x * 3, (MMP_UBYTE)fillval);
                    MMPH_HIF_MemSetB(bufattribute->ulBaseAddr + 
                                    (y * bufattribute->usLineOffset) + x * 3 + 1, (MMP_UBYTE)(fillval >> 8));
                    MMPH_HIF_MemSetB(bufattribute->ulBaseAddr + 
                                    (y * bufattribute->usLineOffset) + x * 3 + 2, (MMP_UBYTE)(fillval >> 16));
                }
            }    
            break;
        default:
            break;
        }
#else
		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);


        MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
        MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
			MMPH_HIF_RegSetB(GRA_BLT_FMT, 1 << 2);
		else
			MMPH_HIF_RegSetB(GRA_BLT_FMT, ((bufattribute->colordepth - 1) << 2));
        MMPH_HIF_RegSetB(GRA_BLT_ROP, (MMP_UBYTE)ropcode & GRA_ROP_MASK);
        MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_OP_DO_ROP | GRA_SOLID_FILL | GRA_LEFT_TOP_RIGHT);

        // Deal Destination Buffer
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
    		longtmp = (bufattribute->ulBaseAddr)
					+ ((bufattribute->usLineOffset) * (rect->usTop))
					+ ((rect->usLeft) * 2);
		else	
			longtmp = (bufattribute->ulBaseAddr)
					+ ((bufattribute->usLineOffset) * (rect->usTop))
					+ ((rect->usLeft) * bufattribute->colordepth);

        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
		MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);

        MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
        
       	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);

#endif
        return MMP_ERR_NONE;
	} 	
    else if (filltype == MMPD_GRAPHICS_LINE_FILL || filltype == MMPD_GRAPHICS_FRAME_FILL) {
   		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);
    
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
			MMPH_HIF_RegSetB(GRA_BLT_FMT, 1 << 2);
		else	
			MMPH_HIF_RegSetB(GRA_BLT_FMT, ((bufattribute->colordepth - 1) << 2));
    
        MMPH_HIF_RegSetB(GRA_BLT_ROP, (MMP_UBYTE)ropcode & GRA_ROP_MASK);
        MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_OP_DO_ROP | GRA_SOLID_FILL | GRA_LEFT_TOP_RIGHT);

		MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);

        // Up Line
        if (filltype == MMPD_GRAPHICS_FRAME_FILL) {
	        MMPH_HIF_RegSetW(GRA_BLT_W, usFrameLength);
    	    MMPH_HIF_RegSetW(GRA_BLT_H, 1);

			if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
    			longtmp = (bufattribute->ulBaseAddr)
						+ ((bufattribute->usLineOffset) * (rect->usTop))
						+ ((rect->usLeft) * 2);
			else	
				longtmp = (bufattribute->ulBaseAddr)
						+ ((bufattribute->usLineOffset) * (rect->usTop))
						+ ((rect->usLeft) * bufattribute->colordepth);

	        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

			if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
    			longtmp = (bufattribute->ulBaseAddr)
						+ ((bufattribute->usLineOffset) * (rect->usTop))
						+ ((rect->usLeft + cut_width - usFrameLength) * 2);
			else	
				longtmp = (bufattribute->ulBaseAddr)
						+ ((bufattribute->usLineOffset) * (rect->usTop))
						+ ((rect->usLeft + cut_width - usFrameLength) * bufattribute->colordepth);

	        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
		}
		else {        	
	        MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    	    MMPH_HIF_RegSetW(GRA_BLT_H, 1);

			if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
    			longtmp = (bufattribute->ulBaseAddr)
						+ ((bufattribute->usLineOffset) * (rect->usTop))
						+ ((rect->usLeft) * 2);
			else	
				longtmp = (bufattribute->ulBaseAddr)
						+ ((bufattribute->usLineOffset) * (rect->usTop))
						+ ((rect->usLeft) * bufattribute->colordepth);

	        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
		}

        // Down Line
        if (cut_height > 1) {
	        if (filltype == MMPD_GRAPHICS_FRAME_FILL) {
		        MMPH_HIF_RegSetW(GRA_BLT_W, usFrameLength);
    		    MMPH_HIF_RegSetW(GRA_BLT_H, 1);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
    				longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - 1))
							+ ((rect->usLeft) * 2);
				else	
					longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - 1))
							+ ((rect->usLeft) * bufattribute->colordepth);

		        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    		    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        		while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
    				longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - 1))
							+ ((rect->usLeft + cut_width - usFrameLength) * 2);
				else	
					longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - 1))
							+ ((rect->usLeft + cut_width - usFrameLength) * bufattribute->colordepth);

		        MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    		    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        		while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
    		}
        	else {
	            MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    	        MMPH_HIF_RegSetW(GRA_BLT_H, 1);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - 1))
							+ ((rect->usLeft) * 2);
				else 						
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + cut_height - 1))
							+ ((rect->usLeft) * bufattribute->colordepth);
			

            	MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

	            MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
    	        while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
        	}
		}        	

        // Left Line
        if (cut_height > 2) {
	        if (filltype == MMPD_GRAPHICS_FRAME_FILL) {
	            MMPH_HIF_RegSetW(GRA_BLT_W, 1);
    	        MMPH_HIF_RegSetW(GRA_BLT_H, usFrameLength - 1);
    	        //MMPH_HIF_RegSetW(GRA_BLT_H, cut_height - 1);
    	        

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft) * 2);
				else						
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft) * bufattribute->colordepth);

	            MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	        MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	    while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + rect->usHeight - usFrameLength))
							+ ((rect->usLeft) * 2);
				else						
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + rect->usHeight - usFrameLength))
							+ ((rect->usLeft) * bufattribute->colordepth);

	            MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	        MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	    while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
			}
		    else {    		
	            MMPH_HIF_RegSetW(GRA_BLT_W, 1);
    	        //MMPH_HIF_RegSetW(GRA_BLT_H, usFrameLength - 1);
    	        MMPH_HIF_RegSetW(GRA_BLT_H, cut_height - 1);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft) * 2);
				else						
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft) * bufattribute->colordepth);

	            MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	        MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	    while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
        	}
		}        	

        // Right Line
        if (cut_height > 2 && cut_width > 1) {
	        if (filltype == MMPD_GRAPHICS_FRAME_FILL) {
	            MMPH_HIF_RegSetW(GRA_BLT_W, 1);
    	        MMPH_HIF_RegSetW(GRA_BLT_H, usFrameLength - 1);
    	        //MMPH_HIF_RegSetW(GRA_BLT_H, cut_height - 1);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft + cut_width - 1) * 2);
				else
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft + cut_width - 1) * bufattribute->colordepth);
									

	            MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	        MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	    while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + rect->usHeight - usFrameLength))
							+ ((rect->usLeft + cut_width - 1) * 2);
				else
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + rect->usHeight - usFrameLength))
							+ ((rect->usLeft + cut_width - 1) * bufattribute->colordepth);
									

	            MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	        MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	    while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
			}
			else {
	            MMPH_HIF_RegSetW(GRA_BLT_W, 1);
    	        MMPH_HIF_RegSetW(GRA_BLT_H, cut_height - 2);

				if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft + cut_width - 1) * 2);
				else
				    longtmp = (bufattribute->ulBaseAddr)
							+ ((bufattribute->usLineOffset) * (rect->usTop + 1))
							+ ((rect->usLeft + cut_width - 1) * bufattribute->colordepth);
									

	            MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    	        MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);
        	    while(MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);
			}        	    
        }
        
       	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
    }
    else {
        return MMP_GRA_ERR_PARAMETER;
    }

    return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_ExpandMonoFIFOtoBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief The function (TBD)...

  The function (TBD)...
  @param[in] usMonobufaddr
  @param[in] usBmpwidth
  @param[in] usBmpheight
  @param[in] bufattribute the graphic buffer
  @param[in] usStartx
  @param[in] usStarty
  @return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_Graphics_ExpandMonoFIFOtoBuf(MMP_USHORT *usMonobufaddr,
                        MMP_USHORT usBmpwidth, MMP_USHORT usBmpheight, MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                                                       MMP_USHORT usStartx, MMP_USHORT usStarty)
{
    MMP_ULONG   longtmp;
    MMP_ULONG   cut_width, cut_height;
    MMP_ULONG   shorttmp;
	MMP_ULONG	fifocount, j;
    MMP_ULONG   cut_width_short, cut_width_byte;
    MMP_ULONG   width_byte;
    MMP_UBYTE   *pchartmp;
    MMP_LONG    total_count;

	if (usStartx >= bufattribute->usWidth ||
			usStarty >= bufattribute->usHeight ||
        !usBmpwidth || !usBmpheight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if ((usBmpwidth + usStartx) > bufattribute->usWidth)
		cut_width = bufattribute->usWidth - usStartx;
    else
        cut_width = usBmpwidth;

	if ((usBmpheight + usStarty) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - usStarty;
    else
        cut_height = usBmpheight;

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

    // set the graphics control parameter
    MMPH_HIF_RegSetW(GRA_BLT_FIFO_THR, GRAPHICS_FIFO_RD_WR_THRESHOLD);
    MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);
    MMPH_HIF_RegSetB(GRA_BLT_FMT, GRA_BIT_DAT_M_2_L
									|((bufattribute->colordepth - 1) << 2));

    // Expand All of the Data in FIFO Bitmap FIFO
    MMPH_HIF_RegSetB(GRA_BLT_ROP, 0x0f);
    MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_COLOR_EXPANSION | GRA_HOST_2_MEM | GRA_LEFT_TOP_RIGHT);

    if (m_bTransActive)
        MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);

    // Deal Destination Buffer
	longtmp = (bufattribute->ulBaseAddr)
				+ ((bufattribute->usLineOffset) * (usStarty))
				+ ((usStartx) * (bufattribute->colordepth));

    MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
	MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);

    pchartmp = (MMP_UBYTE *)usMonobufaddr;

    if (!(cut_width & 0x0007))
        cut_width_byte = cut_width >> 3;
    else
        cut_width_byte = (cut_width >> 3) + 1;

    if (!(cut_width_byte & 0x01))
        cut_width_short = cut_width_byte >> 1;
    else
        cut_width_short = (cut_width_byte >> 1) + 1;

    if (!(usBmpwidth & 0x0007))
        width_byte = usBmpwidth >> 3;
    else
        width_byte = (usBmpwidth >> 3) + 1;

    total_count = (cut_width_short * cut_height);
    j = 0;

    // need to reset the FIFO at first to ensure that there are no
    // data in the FIFO, or check the FIFO empty status
    MMPH_HIF_RegSetW(GRA_BLT_FIFO_RST, GRA_BLT_FIFO);
    if (MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC) != GRA_FIFO_DEPTH) {
   		MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
        return MMP_GRA_ERR_HW;
	}        

    MMPH_HIF_RegSetW(GRA_BLT_EN, GRA_BLT_ST);

    do {
        // wait for FIFO to be under threshold
        while (!(MMPH_HIF_RegGetB(GRA_BLT_FIFO_SR) & GRA_FIFO_WR_U_TH));
		fifocount = MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC);

		if (fifocount) {
			if (fifocount > total_count)
				fifocount = total_count;

			total_count -= fifocount;

			MMPH_HIF_FIFOOpenW(GRA_BLT_FIFO_DATA, fifocount);
			for ( ; fifocount > 0; fifocount--) {
				if (j == (cut_width_short -1)) {
					if (cut_width_byte & 0x01) {
	                    MMPH_HIF_FIFOWriteW((MMP_USHORT)(*(pchartmp++)));
    	            }
        	        else {
                	    shorttmp = *(pchartmp++);
            	        shorttmp |= (*(pchartmp++) << 8);
                    	MMPH_HIF_FIFOWriteW(shorttmp);
	                }
    	            j = 0;
        	        pchartmp += (width_byte - cut_width_byte);
            	}
	            else {
        	        shorttmp = *(pchartmp++);
    	            shorttmp |= (*(pchartmp++) << 8);
            	    MMPH_HIF_FIFOWriteW(shorttmp);
                	j += 1;
	            }
    	    }
		}
    } while (total_count > 0);

    // wait for the FIFO to be empty and graphic engine to stop
    while (MMPH_HIF_RegGetW(GRA_BLT_EN) & GRA_BLT_ST);
    
   	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_ExpandMonoMemtoBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief The function (TBD)...

  The function (TBD)...
  @param[in] ulBmpaddr
  @param[in] usBmpwidth
  @param[in] usBmpheight
  @param[in] bufattribute the graphic buffer
  @param[in] usStartx
  @param[in] usStarty
  @param[in] bDataContinue
  @return It reports the status of the operation.
*/
MMP_ERR    MMPD_Graphics_ExpandMonoMemtoBuf(MMP_ULONG ulBmpaddr,
			            MMP_USHORT usBmpwidth, MMP_USHORT usBmpheight, MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                                                         MMP_USHORT usStartx, MMP_USHORT usStarty, MMP_BOOL bDataContinue)
{
    MMP_ULONG   longtmp;
    MMP_USHORT  cut_width, cut_height;

	if (usStartx >= bufattribute->usWidth ||
			usStarty >= bufattribute->usHeight ||
        !usBmpwidth || !usBmpheight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if ((usBmpwidth + usStartx) > bufattribute->usWidth)
		cut_width = bufattribute->usWidth - usStartx;
    else
        cut_width = usBmpwidth;

	if ((usBmpheight + usStarty) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - usStarty;
    else
        cut_height = usBmpheight;

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

    // set the graphics control parameter
    MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);

    if (bDataContinue == MMP_FALSE) {
        MMPH_HIF_RegSetB(GRA_BLT_FMT, GRA_BIT_DAT_NOT_CON | GRA_BIT_DAT_M_2_L
									|((bufattribute->colordepth - 1) << 2));
    }
    else {
        MMPH_HIF_RegSetB(GRA_BLT_FMT, GRA_BIT_DAT_CON | GRA_BIT_DAT_M_2_L
									|((bufattribute->colordepth - 1) << 2));
    }

    MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_COLOR_EXPANSION | GRA_MEM_2_MEM | GRA_LEFT_TOP_RIGHT);

    if (m_bTransActive)
        MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);

    // Deal Source Buffer
    MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, ulBmpaddr);

    if (!(usBmpwidth & 0x0007))
        longtmp = usBmpwidth >> 3;
    else
        longtmp = (usBmpwidth >> 3) + 1;

    MMPH_HIF_RegSetL(GRA_BLT_SRC_PITCH, longtmp);

    // Deal Destination Buffer
	longtmp = (bufattribute->ulBaseAddr)
				+ ((bufattribute->usLineOffset) * (usStarty))
				+ ((usStartx) * (bufattribute->colordepth));


    MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
	MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);

    MMPH_HIF_RegSetW(GRA_BLT_EN, GRA_BLT_ST);
    while (MMPH_HIF_RegGetW(GRA_BLT_EN) & GRA_BLT_ST);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_DrawTexttoBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief   The function draws text into graphic buffer

  The function draws text into graphic buffer
  @param[in] ulBmpaddr the bmp buffer address of font table
  @param[in] usTextwidth the width of text
  @param[in] usTextheight the height of text
  @param[in] ptext pointer point to text string
  @param[in] usTextcount the text count
  @param[in] bufattribute the graphic buffer
  @param[in] usStartx the x position of image
  @param[in] usStarty the y position of image
  @return It reports the status of the operation.
*/
MMP_ERR    MMPD_Graphics_DrawTexttoBuf(MMP_ULONG ulBmpaddr,
                                                    MMP_USHORT usTextwidth, MMP_USHORT usTextheight,
						MMP_BYTE *ptext, MMP_USHORT usTextcount, MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                                                    MMP_USHORT usStartx, MMP_USHORT usStarty)
{
    MMP_ULONG   longtmp, dstaddr;
    MMP_USHORT  cut_width, cut_height;
    MMP_USHORT  text_size;
    MMP_USHORT  new_x;
    MMP_USHORT  i;

	if (usStartx >= bufattribute->usWidth ||
			usStarty >= bufattribute->usHeight ||
        !usTextwidth || !usTextheight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if ((usTextheight + usStarty) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - usStarty;
    else
        cut_height = usTextheight;

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

    MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);
    MMPH_HIF_RegSetB(GRA_BLT_FMT, GRA_BIT_DAT_NOT_CON | GRA_BIT_DAT_M_2_L
									|((bufattribute->colordepth - 1) << 2));

    MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_COLOR_EXPANSION | GRA_MEM_2_MEM | GRA_LEFT_TOP_RIGHT);

    if (m_bTransActive)
        MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);

    // Deal Destination Buffer Offset
    if (!(usTextwidth & 0x0007))
        longtmp = usTextwidth >> 3;
    else
        longtmp = (usTextwidth >> 3) + 1;

    MMPH_HIF_RegSetL(GRA_BLT_SRC_PITCH, longtmp);

    text_size = (MMP_USHORT)longtmp * (MMP_USHORT)usTextheight;

    // Deal Destination Buffer Offset
	MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);

	dstaddr = (bufattribute->ulBaseAddr)
				+ ((bufattribute->usLineOffset) * (usStarty))
				+ ((usStartx) * (bufattribute->colordepth));

    new_x = usStartx;

	for (i = 0; (i < usTextcount) && (new_x < bufattribute->usWidth); i++) {
		if ((usTextwidth + new_x) > bufattribute->usWidth)
			cut_width = bufattribute->usWidth - new_x;
        else
            cut_width = usTextwidth;

        MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);

        if (*ptext >= ' ' && *ptext <= '~') {

            // Deal Source Buffer Address
            longtmp =  ulBmpaddr + ((*ptext - ' ') * (MMP_ULONG)text_size);

            MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, longtmp);

            // Deal Destination Buffer Address
            MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, dstaddr);

			dstaddr += (usTextwidth * (bufattribute->colordepth));
            new_x += usTextwidth;

            MMPH_HIF_RegSetW(GRA_BLT_EN, GRA_BLT_ST);
            while (MMPH_HIF_RegGetW(GRA_BLT_EN) & GRA_BLT_ST);
        }
        ptext++;
    }
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
    return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_RotateImageFIFOtoBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief The function ...

  The function copy and rotate image data from host memory to MMP memory (frame buffer)
  @param[in] usHostbufaddr the momoey address
  @param[in] colordepth the color depth of the host memory
  @param[in] usHostLineOffset the line offset of image in host point point to
  @param[in] bufattribute the graphic buffer
  @param[in] srcrect the rect of source image
  @param[in] usDstStartx the x position of image in frame buffer
  @param[in] usDstStarty the y position of image in frame buffer
  @param[in] ropcode the Rop Code for graphics operation
  @param[in] rotate the rotate direction
  @return It reports the status of the operation.
*/
MMP_ERR MMPD_Graphics_RotateImageFIFOtoBuf(MMP_USHORT *usHostbufaddr,
						MMPD_GRAPHICS_COLORDEPTH colordepth, MMP_USHORT usHostLineOffset,
						MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute, MMPD_GRAPHICS_RECT *srcrect, 
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode,
						MMPD_GRAPHICS_ROTATE_TYPE rotate)
{
	MMP_ULONG	longtmp = 0;
	MMP_USHORT	*pshorttmp = NULL;
	MMP_USHORT	fifocount = 0;
	MMP_USHORT	cut_width = 0, cut_height = 0;
	MMP_USHORT	cut_width_short = 0, width_short = 0;
	MMP_USHORT	cut_height_short = 0;
	MMP_USHORT	j = 0;
	MMP_LONG	total_count = 0;

	if (colordepth == MMPD_GRAPHICS_COLORDEPTH_8) {
		return MMP_GRA_ERR_NOT_IMPLEMENT;
	}
	
	if (colordepth == MMPD_GRAPHICS_COLORDEPTH_24) {
		if ((usHostLineOffset & 0x1) || (srcrect->usLeft & 0x1))
			return MMP_GRA_ERR_NOT_IMPLEMENT;
		if ((bufattribute->usLineOffset & 0x1) || (bufattribute->usWidth & 0x1))
			return MMP_GRA_ERR_NOT_IMPLEMENT;
	}

	if (usDstStartx >= bufattribute->usWidth ||
			usDstStarty >= bufattribute->usHeight ||
			!srcrect->usWidth || !srcrect->usHeight)  {
		return MMP_GRA_ERR_PARAMETER;
	}

	if (colordepth != (bufattribute->colordepth)) {
		return MMP_GRA_ERR_PARAMETER;
	}


	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_16) {
		if (rotate == MMPD_GRAPHICS_ROTATE_NO_ROTATE) {
			if ((bufattribute->usWidth * 2) == bufattribute->usLineOffset) {
				if ((bufattribute->usWidth * 2) == usHostLineOffset) {
					if ((srcrect->usLeft == 0) && (srcrect->usTop == 0)) {
						MMPH_HIF_MemCopyHostToDev(bufattribute->ulBaseAddr, (MMP_UBYTE *)usHostbufaddr, 
												bufattribute->usWidth * bufattribute->usHeight * 2);					

			        	return MMP_ERR_NONE;
					}
				}
			}
		}
	}


	if ((srcrect->usWidth + usDstStartx) > bufattribute->usWidth)
		cut_width = bufattribute->usWidth - usDstStartx;
	else
		cut_width = srcrect->usWidth;

	if ((srcrect->usHeight + usDstStarty) > bufattribute->usHeight)
		cut_height = bufattribute->usHeight - usDstStarty;
	else
		cut_height = srcrect->usHeight;

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

	// set the graphics control parameter
	MMPH_HIF_RegSetW(GRA_BLT_FIFO_THR, GRAPHICS_FIFO_RD_WR_THRESHOLD);

	switch(rotate){
    case MMPD_GRAPHICS_ROTATE_NO_ROTATE :
    case MMPD_GRAPHICS_ROTATE_RIGHT_180:
    	MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    	MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);
        break;
    case MMPD_GRAPHICS_ROTATE_RIGHT_90:
    case MMPD_GRAPHICS_ROTATE_RIGHT_270:
        MMPH_HIF_RegSetW(GRA_BLT_W, cut_height);
    	MMPH_HIF_RegSetW(GRA_BLT_H, cut_width);
        break;
    default :
        return MMP_GRA_ERR_PARAMETER;
        break;
    }

	MMPH_HIF_RegSetB(GRA_BLT_ROP, (MMP_UBYTE)ropcode & GRA_ROP_MASK);
	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, ((0x1) << 2) | 0x1);
	}
	else {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, ((bufattribute->colordepth - 1) << 2)
										|(colordepth - 1));
	}
	MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_OP_DO_ROP | GRA_HOST_2_MEM | GRA_LEFT_TOP_RIGHT);

	if (m_bTransActive)
		MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);

	// Deal Destination Buffer
	switch(rotate) {
	case MMPD_GRAPHICS_ROTATE_NO_ROTATE :
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
			longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * 2);
		else 
	    	longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * (bufattribute->colordepth));
    					

		MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);
		MMPH_HIF_RegSetW(GRA_ROTE_CTL, 0);
		break;
	case MMPD_GRAPHICS_ROTATE_RIGHT_90:
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
            longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * 2)
    					+ ((srcrect->usWidth - 1) * 2) ;
            MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, 2);
		}
		else {
       		longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * (bufattribute->colordepth))
    					+ ((srcrect->usWidth - 1) * (bufattribute->colordepth)) ;
            MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->colordepth);
		}    					

		MMPH_HIF_RegSetW(GRA_ROTE_CTL, GRA_ROTE_MODE | GRA_LINEOST_NEG);
		MMPH_HIF_RegSetW(GRA_PIXEL_OFFSET, 0x8000 | (bufattribute->usLineOffset));
		break;
	case MMPD_GRAPHICS_ROTATE_RIGHT_180:
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
            longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty + srcrect->usHeight - 1))
    					+ ((usDstStartx + srcrect->usWidth-1) * 2);
		else
            longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty + srcrect->usHeight - 1))
    					+ ((usDstStartx + srcrect->usWidth-1) * (bufattribute->colordepth));


		MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->usLineOffset);

		MMPH_HIF_RegSetW(GRA_ROTE_CTL, GRA_ROTE_MODE | GRA_LINEOST_NEG);
		MMPH_HIF_RegSetW(GRA_PIXEL_OFFSET, 2);
		break;
	case MMPD_GRAPHICS_ROTATE_RIGHT_270:
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
            longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty + srcrect->usHeight - 1))
    					+ ((usDstStartx) * 2);
            MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, 2);
		}
		else {
            longtmp = (bufattribute->ulBaseAddr)
    					+ ((bufattribute->usLineOffset) * (usDstStarty + srcrect->usHeight - 1))
    					+ ((usDstStartx) * (bufattribute->colordepth));
		
			MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, bufattribute->colordepth);
		}    					

		MMPH_HIF_RegSetW(GRA_ROTE_CTL, GRA_ROTE_MODE | GRA_LINEOST_POS);
		MMPH_HIF_RegSetW(GRA_PIXEL_OFFSET, (bufattribute->usLineOffset));
		break;
	default :
		return MMP_GRA_ERR_PARAMETER;
		break;
	}
	MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);
	MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, 0);

	switch (colordepth) {	// source format
		case MMPD_GRAPHICS_COLORDEPTH_8:
			if (!(cut_width & 0x0001))
				cut_width_short = cut_width >> 1;
			else
				cut_width_short = (cut_width+1) >> 1;

			if (!(cut_height & 0x0001))
				cut_height_short = cut_height >> 1;
			else
				cut_height_short = (cut_height+1) >> 1;

			width_short = usHostLineOffset >> 1;
			pshorttmp = usHostbufaddr + ((usHostLineOffset * srcrect->usTop) >> 1) + (srcrect->usLeft >> 1);
			break;
		case MMPD_GRAPHICS_COLORDEPTH_16:
		case MMPD_GRAPHICS_COLORDEPTH_YUV422:
			cut_width_short = cut_width;
			cut_height_short = cut_height;

			width_short = usHostLineOffset >> 1;
			pshorttmp = usHostbufaddr + ((usHostLineOffset * srcrect->usTop) >> 1) + (srcrect->usLeft);
			break;
		case MMPD_GRAPHICS_COLORDEPTH_24:
			if (!(cut_width & 0x0001))
				cut_width_short = (cut_width*3) >> 1;
			else
				cut_width_short = (cut_width*3+1) >> 1;

			if (!(cut_height & 0x0001))
				cut_height_short = (cut_height*3) >> 1;
			else
				cut_height_short = (cut_height*3+1) >> 1;

			width_short = usHostLineOffset >> 1;
			pshorttmp = usHostbufaddr + ((usHostLineOffset * srcrect->usTop) >> 1) + ((srcrect->usLeft * 3) >> 1);
			break;
    }

	if (rotate == MMPD_GRAPHICS_ROTATE_NO_ROTATE || rotate == MMPD_GRAPHICS_ROTATE_RIGHT_180)
	    total_count = (cut_width_short * cut_height);
	else 	    
	    total_count = (cut_height_short * cut_width);
    j = 0;

    // need to reset the FIFO at first to ensure that there are no
    // data in the FIFO, or check the FIFO empty status
    MMPH_HIF_RegSetW(GRA_BLT_FIFO_RST, GRA_BLT_FIFO);
    if (MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC) != GRA_FIFO_DEPTH)
        return MMP_GRA_ERR_HW;

    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);

	if (((rotate == MMPD_GRAPHICS_ROTATE_NO_ROTATE || 
				rotate == MMPD_GRAPHICS_ROTATE_RIGHT_180) && width_short == cut_width_short) ||
		((rotate == MMPD_GRAPHICS_ROTATE_RIGHT_90 || 	
				rotate == MMPD_GRAPHICS_ROTATE_RIGHT_270) && width_short == cut_height)) {

	    do {
       		fifocount = MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC);

			if (fifocount) {
	        	if (fifocount > total_count)
   		        	fifocount = total_count;

				total_count -= fifocount;

       	        MMPH_HIF_FIFOCWriteW(GRA_BLT_FIFO_DATA, pshorttmp, fifocount);
           	    pshorttmp += fifocount;
			}
		} while (total_count > 0);
	}
	else {
		if (rotate == MMPD_GRAPHICS_ROTATE_NO_ROTATE ||
				rotate == MMPD_GRAPHICS_ROTATE_RIGHT_180) {
		    do {
       			fifocount = MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC);

				if (fifocount) {
		    	    if (fifocount > total_count)
   			    	    fifocount = total_count;

					total_count -= fifocount;

   		        	MMPH_HIF_FIFOOpenW(GRA_BLT_FIFO_DATA, fifocount);
    	    	    for ( ; fifocount > 0; fifocount--) {
	    	            if (j == (cut_width_short -1)) {
   		        	        MMPH_HIF_FIFOWriteW(*(pshorttmp++));
       			            pshorttmp += (width_short - cut_width_short);
           		    	    j = 0;
                    	}
	   	                else {
   		   	                MMPH_HIF_FIFOWriteW(*(pshorttmp++));
       		   	            j++;
           		   	    }
					}
				}
			} while (total_count > 0);
		}
		else if (rotate == MMPD_GRAPHICS_ROTATE_RIGHT_90 ||
					rotate == MMPD_GRAPHICS_ROTATE_RIGHT_270) {
		    do {
       			fifocount = MMPH_HIF_RegGetB(GRA_BLT_FIFO_WR_SPC);

				if (fifocount) {
		    	    if (fifocount > total_count)
   			    	    fifocount = total_count;

					total_count -= fifocount;

   		        	MMPH_HIF_FIFOOpenW(GRA_BLT_FIFO_DATA, fifocount);
    	    	    for ( ; fifocount > 0; fifocount--) {
	    	            if (j == (cut_height_short -1)) {
    	        	        MMPH_HIF_FIFOWriteW(*(pshorttmp++));
      			            pshorttmp += (width_short - cut_height_short);
           		    	    j = 0;
	                   	}
	   	                else {
    	   	                MMPH_HIF_FIFOWriteW(*(pshorttmp++));
       		   	            j++;
           		   	    }
					}
				}
			} while (total_count > 0);
		}
	}

    // wait for the FIFO to be empty and graphic engine to stop
    while (MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

    MMPH_HIF_RegSetW(GRA_ROTE_CTL, MMPH_HIF_RegGetW(GRA_ROTE_CTL) & ~GRA_ROTE_MODE);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);

    return MMP_ERR_NONE;
}


//@end_ait_only
//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_RotateImageBuftoBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief The function ...

  The function copy and rotate image data from memory to memory (frame buffer)
  
  @param[in] srcbufattribute the source graphic buffer
  @param[in] srcrect the rect of source image
  @param[in] dstbufattribute the destination graphic buffer
  @param[in] usDstStartx the x position of destination frame buffer
  @param[in] usDstStarty the y position of destination frame buffer
  @param[in] ropcode the Rop Code for graphics operation
  @param[in] rotate the rotate direction
  @return It reports the status of the operation.
*/
MMP_ERR MMPD_Graphics_RotateImageBuftoBuf(MMPD_GRAPHICS_BUFATTRIBUTE *srcbufattribute,
						MMPD_GRAPHICS_RECT *srcrect, MMPD_GRAPHICS_BUFATTRIBUTE *dstbufattribute, 
						MMP_USHORT usDstStartx, MMP_USHORT usDstStarty, MMPD_GRAPHICS_ROP ropcode,
						MMPD_GRAPHICS_ROTATE_TYPE rotate)

{
	MMP_ULONG	longtmp;
	MMP_USHORT	cut_width, cut_height;

	if (srcrect->usLeft >= srcbufattribute->usWidth || 
			srcrect->usTop >= srcbufattribute->usHeight ||
			usDstStartx >= dstbufattribute->usWidth || 
			usDstStarty >= dstbufattribute->usHeight ||
			!srcrect->usWidth || !srcrect->usHeight) {
        return MMP_GRA_ERR_PARAMETER;
    }

	if (srcbufattribute->colordepth != dstbufattribute->colordepth) {
        return MMP_GRA_ERR_PARAMETER;
	}			

    if(dstbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV420) {
        return MMP_GRA_ERR_PARAMETER;
	}	

	if ((srcrect->usWidth + srcrect->usLeft) > srcbufattribute->usWidth)
		cut_width = srcbufattribute->usWidth - srcrect->usLeft;
	else	
		cut_width = srcrect->usWidth;

	if ((srcrect->usHeight + srcrect->usTop) > srcbufattribute->usHeight)
		cut_height = srcbufattribute->usHeight - srcrect->usTop;
	else	
		cut_height = srcrect->usHeight;

	/*if ((cut_width + usDstStartx) > dstbufattribute->usWidth)
		cut_width = dstbufattribute->usWidth - usDstStartx;

	if ((cut_height + usDstStarty) > dstbufattribute->usHeight)
		cut_height = dstbufattribute->usHeight - usDstStarty;*/

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);

	switch(rotate) {
    case MMPD_GRAPHICS_ROTATE_NO_ROTATE :
    case MMPD_GRAPHICS_ROTATE_RIGHT_180:
        if ((cut_width + usDstStartx) > dstbufattribute->usWidth)
    		cut_width = dstbufattribute->usWidth - usDstStartx;

    	if ((cut_height + usDstStarty) > dstbufattribute->usHeight)
    		cut_height = dstbufattribute->usHeight - usDstStarty;
    	//MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    	//MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);
        break;
    case MMPD_GRAPHICS_ROTATE_RIGHT_90:
    case MMPD_GRAPHICS_ROTATE_RIGHT_270:
        if ((cut_height + usDstStartx) > dstbufattribute->usWidth)
    		cut_height = dstbufattribute->usWidth - usDstStartx;

    	if ((cut_width + usDstStarty) > dstbufattribute->usHeight)
    		cut_width = dstbufattribute->usHeight - usDstStarty;
        //MMPH_HIF_RegSetW(GRA_BLT_W, cut_height);
    	//MMPH_HIF_RegSetW(GRA_BLT_H, cut_width);
        break;
    default :
        return MMP_GRA_ERR_PARAMETER;
        break;
    }
    MMPH_HIF_RegSetW(GRA_BLT_W, cut_width);
    MMPH_HIF_RegSetW(GRA_BLT_H, cut_height);

	MMPH_HIF_RegSetB(GRA_BLT_ROP, (MMP_UBYTE)ropcode & GRA_ROP_MASK);
	if (srcbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, ((0x1) << 2) | 0x1);
	}
	else {
		MMPH_HIF_RegSetB(GRA_BLT_FMT, ((dstbufattribute->colordepth - 1) << 2)
										|(srcbufattribute->colordepth - 1));
	}
	MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, GRA_OP_DO_ROP | GRA_MEM_2_MEM | GRA_LEFT_TOP_RIGHT);

	if (m_bTransActive)
		MMPH_HIF_RegSetB(GRA_BLT_ROP_CTL, MMPH_HIF_RegGetB(GRA_BLT_ROP_CTL) | GRA_OP_DO_TRANSP);


    // Deal Source Buffer
	if (srcbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
    	longtmp = (srcbufattribute->ulBaseAddr)
	    			+ ((srcbufattribute->usLineOffset) * (srcrect->usTop))
		    		+ ((srcrect->usLeft) * 2);
    }
    else {
    	longtmp = (srcbufattribute->ulBaseAddr)
	    			+ ((srcbufattribute->usLineOffset) * (srcrect->usTop))
		    		+ ((srcrect->usLeft) * (srcbufattribute->colordepth));
    }		    		

    MMPH_HIF_RegSetL(GRA_BLT_SRC_ADDR, longtmp);
	MMPH_HIF_RegSetL(GRA_BLT_SRC_PITCH, srcbufattribute->usLineOffset);

	// Deal Destination Buffer
	switch(rotate){
	case MMPD_GRAPHICS_ROTATE_NO_ROTATE :
		if (dstbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
			longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * 2);
		else 
	    	longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * (dstbufattribute->colordepth));
    					

		MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, dstbufattribute->usLineOffset);
		MMPH_HIF_RegSetW(GRA_ROTE_CTL, 0);
		break;
	case MMPD_GRAPHICS_ROTATE_RIGHT_90:
		if (dstbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
            longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * 2)
    					+ ((dstbufattribute->usWidth - 1) * 2) ;
            MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, 2);
		}
		else {
       		longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty))
    					+ ((usDstStartx) * (dstbufattribute->colordepth))
    					+ ((dstbufattribute->usWidth - 1) * (dstbufattribute->colordepth)) ;
            MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, dstbufattribute->colordepth);
		}    					

		MMPH_HIF_RegSetW(GRA_ROTE_CTL, GRA_ROTE_MODE | GRA_LINEOST_NEG);
		MMPH_HIF_RegSetW(GRA_PIXEL_OFFSET, 0x8000 | (dstbufattribute->usLineOffset));
		break;
	case MMPD_GRAPHICS_ROTATE_RIGHT_180:
		if (dstbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422)
            longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty + dstbufattribute->usHeight - 1))
    					+ ((usDstStartx + dstbufattribute->usWidth-1) * 2);
		else
            longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty + dstbufattribute->usHeight - 1))
    					+ ((usDstStartx + dstbufattribute->usWidth-1) * (dstbufattribute->colordepth));


		MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, dstbufattribute->usLineOffset);

		MMPH_HIF_RegSetW(GRA_ROTE_CTL, GRA_ROTE_MODE | GRA_LINEOST_NEG);
		MMPH_HIF_RegSetW(GRA_PIXEL_OFFSET, 2);
		break;
	case MMPD_GRAPHICS_ROTATE_RIGHT_270:
		if (dstbufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
            longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty + dstbufattribute->usHeight - 1))
    					+ ((usDstStartx) * 2);
            MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, 2);
		}
		else {
            longtmp = (dstbufattribute->ulBaseAddr)
    					+ ((dstbufattribute->usLineOffset) * (usDstStarty + dstbufattribute->usHeight - 1))
    					+ ((usDstStartx) * (dstbufattribute->colordepth));
		
			MMPH_HIF_RegSetL(GRA_BLT_DST_PITCH, dstbufattribute->colordepth);
		}    					

		MMPH_HIF_RegSetW(GRA_ROTE_CTL, GRA_ROTE_MODE | GRA_LINEOST_POS);
		MMPH_HIF_RegSetW(GRA_PIXEL_OFFSET, (dstbufattribute->usLineOffset));
		break;
	default :
		return MMP_GRA_ERR_PARAMETER;
		break;
	}

	MMPH_HIF_RegSetL(GRA_BLT_DST_ADDR, longtmp);

    MMPH_HIF_RegSetB(GRA_BLT_EN, GRA_BLT_ST);


    // wait for the FIFO to be empty and graphic engine to stop
    while (MMPH_HIF_RegGetB(GRA_BLT_EN) & GRA_BLT_ST);

    MMPH_HIF_RegSetW(GRA_ROTE_CTL, MMPH_HIF_RegGetW(GRA_ROTE_CTL) & ~GRA_ROTE_MODE);

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_PaintDebugImage
//  Description : 
//------------------------------------------------------------------------------
/** @brief   The function generate some debug pattern for common use.

  The function generate some debug pattern for common use.
  @param[in] bufattribute the graphic buffer
  @param[in] colorpat the pattern for the debug image
  @return It reports the status of the operation.
*/
MMP_ERR MMPD_Graphics_PaintDebugImage(MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                        MMPD_GRAPHICS_COLORPAT  *colorpat)
{
    MMPD_GRAPHICS_RECT           rect;
    MMPD_GRAPHICS_BUFATTRIBUTE   newbufattribute;
    
    switch (bufattribute->colordepth) {
    case MMPD_GRAPHICS_COLORDEPTH_16:
    case MMPD_GRAPHICS_COLORDEPTH_24:
    case MMPD_GRAPHICS_COLORDEPTH_YUV422:
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_SNGL) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = bufattribute->usWidth;
            rect.usTop = 0;
            rect.usHeight = bufattribute->usHeight;
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_LR) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = bufattribute->usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = bufattribute->usHeight;
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[1]);  	
            rect.usLeft = bufattribute->usWidth >> 1;
            rect.usWidth = (bufattribute->usWidth - (bufattribute->usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = bufattribute->usHeight;
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_UD) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = bufattribute->usWidth;
            rect.usTop = 0;
            rect.usHeight = bufattribute->usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[1]);  	
            rect.usLeft = 0;
            rect.usWidth = bufattribute->usWidth;
            rect.usTop = bufattribute->usHeight >> 1;
            rect.usHeight = (bufattribute->usHeight - (bufattribute->usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_4_SQ) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = bufattribute->usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = bufattribute->usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[1]);  	
            rect.usLeft = bufattribute->usWidth >> 1;
            rect.usWidth = (bufattribute->usWidth - (bufattribute->usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = bufattribute->usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[2]);  	
            rect.usLeft = 0;
            rect.usWidth = bufattribute->usWidth >> 1;
            rect.usTop = bufattribute->usHeight >> 1;
            rect.usHeight = (bufattribute->usHeight - (bufattribute->usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[3]);  	
            rect.usLeft = bufattribute->usWidth >> 1;
            rect.usWidth = (bufattribute->usWidth - (bufattribute->usWidth >> 1));
            rect.usTop = bufattribute->usHeight >> 1;
            rect.usHeight = (bufattribute->usHeight - (bufattribute->usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(bufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }                        
        break;
    case MMPD_GRAPHICS_COLORDEPTH_YUV420:
        newbufattribute = *bufattribute;
        newbufattribute.colordepth = MMPD_GRAPHICS_COLORDEPTH_8;

        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_SNGL) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_LR) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[1]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_UD) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[1]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_4_SQ) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[1]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[2]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulYColor[3]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }

        newbufattribute.ulBaseAddr = newbufattribute.ulBaseUAddr;
        newbufattribute.usWidth = newbufattribute.usWidth >> 1;
        newbufattribute.usHeight = newbufattribute.usHeight >> 1;
        newbufattribute.usLineOffset = newbufattribute.usLineOffset >> 1;

        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_SNGL) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_LR) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[1]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_UD) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[1]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_4_SQ) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[1]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[2]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulUColor[3]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }

        newbufattribute.ulBaseAddr = newbufattribute.ulBaseVAddr;

        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_SNGL) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_LR) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[1]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_UD) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[1]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth;
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }
        if (colorpat->patmode == MMPD_GRAPHICS_PATMODE_4_SQ) {
            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[0]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[1]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = 0;
            rect.usHeight = newbufattribute.usHeight >> 1;
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[2]);  	
            rect.usLeft = 0;
            rect.usWidth = newbufattribute.usWidth >> 1;
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);

            MMPD_Graphics_SetKeyColor(MMPD_GRAPHICS_FG_COLOR, colorpat->ulVColor[3]);  	
            rect.usLeft = newbufattribute.usWidth >> 1;
            rect.usWidth = (newbufattribute.usWidth - (newbufattribute.usWidth >> 1));
            rect.usTop = newbufattribute.usHeight >> 1;
            rect.usHeight = (newbufattribute.usHeight - (newbufattribute.usHeight >> 1));
            MMPD_Graphics_DrawRectToBuf(&newbufattribute, &rect, 
                        MMPD_GRAPHICS_SOLID_FILL, 0, MMPD_GRAPHICS_ROP_SRCCOPY);
        }                        
        break;
    }

    return MMP_ERR_NONE;
}
#endif
//------------------------------------------------------------------------------
//  Function    : MMPD_Graphics_SetScaleAttribute
//  Description : 
//------------------------------------------------------------------------------
/** @brief   The function copy image date from one frame buffer to another frame buffer

  The function copy image date from one buffer to another buffer
  @param[in] bufattribute the source scale buffer
  @param[in] usSrcx the start x position of source image
  @param[in] usSrcy the start y position of source image
  @param[in] usWidth the width of image in frame buffer
  @param[in] usHeight the height of image in frame buffer
  @param[in] usUpscale the up scale ratio
  @param[in] srcsel input from graphic SCALE FIFO or directly from FB
  @return It reports the status of the operation.
*/
MMP_ERR MMPD_Graphics_SetScaleAttribute(MMPD_GRAPHICS_BUFATTRIBUTE *bufattribute,
                        MMP_ULONG ulScaleBufAddr, MMPD_GRAPHICS_COLORDEPTH incolormode,                       
						MMPD_GRAPHICS_RECT *rect, MMP_USHORT usUpscale, 
						MMPD_GRAPHICS_SCAL_SRC srcsel)
{
	MMP_ULONG	longtmp;

  	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV420) {
	  	if ((rect->usLeft != 0) || bufattribute->usLineOffset != rect->usWidth)
			return MMP_GRA_ERR_PARAMETER;				  		 
	}

//	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);     

    if (srcsel == MMPD_GRAPHICS_SCAL_FIFO) { 
        if (rect->usLeft != 0 || rect->usTop != 0)
            return MMP_GRA_ERR_PARAMETER;

	   	// need to reset the FIFO at first to ensure that there are no
    	// data in the FIFO, or check the FIFO empty status
	    MMPH_HIF_RegSetW(GRA_SCAL_FIFO_RST, GRA_SCAL_FIFO);
		if (MMPH_HIF_RegGetB(GRA_SCAL_FIFO_WR_SPC) != GRA_FIFO_DEPTH) {
			MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);	    
			return MMP_GRA_ERR_PARAMETER;
		}			
	}

    if (srcsel == MMPD_GRAPHICS_SCAL_FIFO) { 
        // setting for graphic engine
        MMPH_HIF_RegSetL(GRA_SCAL_ADDR_Y_ST, ulScaleBufAddr);
    }
    else if (srcsel == MMPD_GRAPHICS_SCAL_FB) { 
        #if (CHIP == P_V2) || (CHIP == VSN_V2) || (CHIP == VSN_V3)
       	if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV420 ||
       	    bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE)
       	#endif
        {
	        // setting for graphic engine address. Y address
   		    longtmp = (bufattribute->ulBaseAddr)
        			+ (bufattribute->usLineOffset * rect->usTop) + rect->usLeft;
	        MMPH_HIF_RegSetL(GRA_SCAL_ADDR_Y_ST, longtmp);

	        // setting for graphic engine address. U address
       		longtmp = (bufattribute->ulBaseUAddr)
        			+ ((bufattribute->usLineOffset >> 1) * (rect->usTop >> 1)) + (rect->usLeft >> 1);
	        MMPH_HIF_RegSetL(GRA_SCAL_ADDR_U_ST, longtmp);

        	// setting for graphic engine address. V address
	       	longtmp = (bufattribute->ulBaseVAddr)
    	    		+ ((bufattribute->usLineOffset >> 1) * (rect->usTop >> 1)) + (rect->usLeft >> 1);
    	    // setting for graphic engine
        	MMPH_HIF_RegSetL(GRA_SCAL_ADDR_V_ST, longtmp);

    	}
   	    else if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
    	    longtmp = (bufattribute->ulBaseAddr)
	    	    	+ ((bufattribute->usLineOffset) * (rect->usTop)) 
		    	    + ((rect->usLeft) * (2));
    	    // setting for graphic engine
        	MMPH_HIF_RegSetL(GRA_SCAL_ADDR_Y_ST, longtmp);
	        MMPH_HIF_RegSetL(GRA_SCAL_OFST_ST, bufattribute->usLineOffset);
       	}
        else {
	        longtmp = (bufattribute->ulBaseAddr)
    		    	+ ((bufattribute->usLineOffset) * (rect->usTop)) 
   	    			+ ((rect->usLeft) * (MMP_USHORT)(bufattribute->colordepth));
   	    			
	        // setting for graphic engine
        	MMPH_HIF_RegSetL(GRA_SCAL_ADDR_Y_ST, longtmp);
	        MMPH_HIF_RegSetL(GRA_SCAL_OFST_ST, bufattribute->usLineOffset);
        }
    }        

    MMPH_HIF_RegSetW(GRA_SCAL_W, rect->usWidth);
    MMPH_HIF_RegSetW(GRA_SCAL_H, rect->usHeight);

	MMPH_HIF_RegSetB(GRA_SCAL_UP_FACT, GRA_SCALUP(usUpscale));

	if (srcsel == MMPD_GRAPHICS_SCAL_FB) {
		if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_16) {
			MMPH_HIF_RegSetB(GRA_SCAL_FMT, GRA_SCAL_MEM_RGB565);
		}		
		else if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_24) {
			MMPH_HIF_RegSetB(GRA_SCAL_FMT, GRA_SCAL_MEM_RGB888);
		}		
		else if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
			MMPH_HIF_RegSetB(GRA_SCAL_FMT, GRA_SCAL_MEM_YUV422);
		}
		else if (bufattribute->colordepth == MMPD_GRAPHICS_COLORDEPTH_YUV420) {
			MMPH_HIF_RegSetB(GRA_SCAL_FMT, GRA_SCAL_MEM_RGB565);
		}
	}
	else if (srcsel == MMPD_GRAPHICS_SCAL_FIFO) {
    	if (incolormode == MMPD_GRAPHICS_COLORDEPTH_16) {
		    MMPH_HIF_RegSetB(GRA_SCAL_FMT, GRA_SCAL_FIFO_RGB565);
	    }	
    	else if (incolormode == MMPD_GRAPHICS_COLORDEPTH_YUV422) {
	    	MMPH_HIF_RegSetB(GRA_SCAL_FMT, GRA_SCAL_FIFO_YUV422);
    	}	
	}		

	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);	   
    return MMP_ERR_NONE;
}

/// @}
/// @end_ait_only


#ifdef BUILD_CE
#define BUILD_FW
#endif
