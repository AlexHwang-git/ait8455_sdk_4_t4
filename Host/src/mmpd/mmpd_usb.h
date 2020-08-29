/// @ait_only
//==============================================================================
//
//  File        : mmpd_usb.h
//  Description : USB Control Driver Function
//  Author      : Hnas Liu
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPD_USB_H_
#define _MMPD_USB_H_

/** @addtogroup MMPD_USB
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
#define MMPD_USB_PCSYNC_HANDSHAKE_BUF_SIZE           (16)
#define MMPD_USB_PCSYNC_OUTFLAG_OFFSET_B             (0)
#define MMPD_USB_PCSYNC_INFLAG_OFFSET_B              (1)
#define MMPD_USB_PCSYNC_INSIZE_OFFSET_W              (2)
#define MMPD_USB_PCSYNC_OUTSIZE_OFFSET_W             (4)

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

extern MMP_ERR	MMPD_USB_SetMode(MMP_UBYTE ubMode);
extern MMP_ERR	MMPD_USB_SetMSDCBuf(MMP_ULONG ulBufAddr,MMP_ULONG ulBufSize);
extern MMP_ERR	MMPD_USB_SetCtlBuf(MMP_ULONG ulBufAddr,MMP_ULONG ulBufSize);
extern MMP_ERR	MMPD_USB_SetPCSYNC_InBuf(MMP_ULONG ulBufAddr,MMP_ULONG ulBufSize);
extern MMP_ERR	MMPD_USB_SetPCSYNC_OutBuf(MMP_ULONG ulBufAddr,MMP_ULONG ulBufSize);
extern MMP_ERR	MMPD_USB_SetPCSYNC_HandShakeBuf(MMP_ULONG ulBufAddr,MMP_ULONG ulBufSize);
extern MMP_ERR	MMPD_USB_GetPCSYNCInFlag(MMP_ULONG *ulFlag);
extern MMP_ERR	MMPD_USB_SetPCSYNCInFlag(MMP_ULONG ulFlag);
extern MMP_ERR	MMPD_USB_GetPCSYNCOutFlag(MMP_ULONG *ulFlag);
extern MMP_ERR	MMPD_USB_SetPCSYNCOutFlag(MMP_ULONG ulFlag);
extern MMP_ERR	MMPD_USB_GetPCSYNCInSize(MMP_ULONG *ulSize);
extern MMP_ERR	MMPD_USB_ReadPCSYNCInData(MMP_UBYTE *ubMemStart,MMP_ULONG ulSize);
extern MMP_ERR	MMPD_USB_WritePCSYNCOutData(MMP_UBYTE *ubMemStart,MMP_ULONG ulSize);
extern MMP_ERR	MMPD_USB_SetPCSYNCOutSize(MMP_ULONG ulSize);
extern MMP_ERR	MMPD_USB_SetPCamCompressBuf(MMP_ULONG ulBufAddr,MMP_ULONG ulBufSize);
extern MMP_ERR	MMPD_USB_SetPCCamLineBuf(MMP_ULONG ulBufAddr,MMP_ULONG ulBufSize);
//extern MMP_ERR	MMPD_USB_SetMTP_EPBuf(MMP_ULONG ulRxBufAddr,MMP_ULONG ulTxBufAddr,MMP_ULONG ulBufSize); //Andy--
extern MMP_ERR	MMPD_USB_SetMTPEPBuf(MMP_ULONG ulRxBufAddr,MMP_ULONG ulTxBufAddr,MMP_ULONG ulBufSize,MMP_ULONG ulMBufAddr,MMP_ULONG ulMBufSize); //Andy++
extern MMP_ERR  MMPD_USB_GetDpsBufAddr(MMP_UBYTE ubBufType, MMP_ULONG *ulAddr);
extern MMP_ERR	MMPD_DPS_START_JOB(void);
extern MMP_ERR	MMPD_DPS_AbortPrint(void);
extern MMP_ERR	MMPD_DPS_ContinuePrint(void);
extern MMP_ERR	MMPD_DPS_GetPrinterStatus(MMP_USHORT *usStatus );
extern MMP_ERR  MMPD_USB_CapturePCCam(void);

/// @}
#endif // _MMPD_USB_H_

/// @end_ait_only

