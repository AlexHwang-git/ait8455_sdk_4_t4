//==============================================================================
//
//  File        : pCam_osd.h
//  Description : INCLUDE File for the OSD general function porting.
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================

#ifndef _PCAM_OSD_H_
#define _PCAM_OSD_H_

/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "config_fw.h"

#if SUPPORT_OSD_FUNC

/*===========================================================================
 *                              CONSTANTS
 *===========================================================================*/ 

#define STRLEN strlen
#define STRCPY strcpy
#define STRCMP strcmp
#define STRCAT strcat


/*===========================================================================
 * Structure define
 *===========================================================================*/


#define OSD_FONT_W  16
#define OSD_FONT_H  24
typedef enum {
    BACKUP_IMAGE = 0,
    RESTORE_IMAGE 
} OSD_OP_DIR ;

/*===========================================================================
 * Function prototype
 *===========================================================================*/

void pcam_Int2Str(MMP_ULONG value, MMP_BYTE *string);
MMP_BOOL    pcam_Osd_IsEnable(void);
MMP_USHORT  pcam_Osd_ConfigCurrentStream(MMP_BOOL en,MMP_USHORT pwCurrentW, MMP_USHORT pwCurrentH);
void        pcam_Osd_DrawStr(const char *Str, MMP_SHORT nStrLen, MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr);
MMP_USHORT  pcam_Osd_GetCurrentStreamRes(MMP_USHORT *pwCurrentW, MMP_USHORT *pwCurrentH) ;
void        pcam_Osd_SetPos(MMP_USHORT xPos,MMP_USHORT yPos);
void        pcam_Osd_SetColor(MMP_UBYTE yColor,MMP_UBYTE uColor,MMP_UBYTE vColor);
void        pcam_Osd_AllocBuffer(MMP_USHORT osdFontW,MMP_USHORT osdFontH) ;
MMP_ULONG pcam_Osd_GetOffScrBuffer(void);
//void pcam_Osd_DrawBuf(MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr ,MMP_ULONG ulSrcYBufAddr,MMP_ULONG ulSrcUBufAddr ,MMP_USHORT usSrcW,MMP_USHORT usSrcH);
void pcam_Osd_DrawBuf(MMP_USHORT buf_id,MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr ) ;
void pcam_Osd_ScalingBuf(MMP_USHORT buf_id,MMP_USHORT m,MMP_USHORT n);
void pcam_Osd_CopySrcBuf(MMP_USHORT buf_id,OSD_OP_DIR dir,MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr);
void pcam_Osd_DrawColor(MMP_ULONG y_addr,MMP_ULONG u_addr,MMP_USHORT w,MMP_USHORT h);
#endif//SUPPORT_OSD_FUNC

#endif

