#define exUSBVEND

#include "includes_fw.h"
#include "lib_retina.h"

//#include "mmpf_usbpcsyn.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"
#include "mmpf_usbpccam.h"
#include "mmp_reg_usb.h"

extern USB_DESCRIPTOR      UsbDescriptorInfo;
extern USB_REQUEST_PAYLOAD UsbRequestPayload;

#if (SUPPORT_PCCAM_FUNC==1)
extern MMP_ULONG glPCCamDmaEnable;
extern MMP_ULONG    glPccamResolution;
#endif /* (SUPPORT_PCCAM_FUNC==1) */

extern MMP_ULONG   glUsbCtlState;
extern MMP_ULONG   glUsbCtlTxCnt;
extern MMP_ULONG   glUsbOperationSpeed;

MMP_ULONG glPCSyncHandshackBufAddr;
MMP_ULONG glPCSyncBulkInSize;
MMP_ULONG glPCSyncBulkInBufOffset;
MMP_ULONG glPCSyncHostOutBufAddr, glPCSyncHostInBufAddr;
//extern   MMP_ULONG PCCam_PictureOK_Counter;
#if defined(USB_FW)||(USB_EN)
#pragma  O2


#define MS_VENDOR_CODE	0x55
//------------------------------------------------------------------------------
//  Function    : PCCAMSetDevMode
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void PCCAMSetDevMode(int mode)
{
#if (SUPPORT_PCCAM_FUNC==1)
    volatile MMP_USHORT *USB_REG_BASE_W = (volatile MMP_USHORT *) USB_REG_BASE_ADDR;
    MMP_USHORT  csr;

    switch(mode) {
    case 0:
        RTNA_DBG_Str(0, "start pccam\r\n");
        if(UsbRequestPayload.wIndex == 0x5) {
            glPccamResolution = PCCAM_320_240;
            RTNA_DBG_Str(0, "320*240\r\n");
        } else if(UsbRequestPayload.wIndex == 0x9) {
            glPccamResolution = PCCAM_640_480;
            RTNA_DBG_Str(0, "640*480\r\n");
        } else if(UsbRequestPayload.wIndex == 0xA) {
            glPccamResolution = PCCAM_1024_768;
            RTNA_DBG_Str(0, "1024*768\r\n");
        }
        PCCamInitED = 1;
        PCCamModeEnable = 1;
        break;
    case 1:
        RTNA_DBG_Str(0, "stop pccam\r\n");
        if(glPCCamDmaEnable == 1) {
            glPCCamDmaEnable = 0;
            UsbDmaStop();
            RTNA_DBG_Short(0, USB_REG_BASE_W[0x112 >> 1]);
            RTNA_DBG_Str(0, "\r\n");
            csr = UsbReadTxEpCSR(PCCAM_TX_EP_ADDR);
            csr = csr & TXCSR_RW_MASK;
            UsbWriteTxEpCSR(PCCAM_TX_EP_ADDR, csr | SET_TX_FLUSHFIFO);
            UsbWriteTxEpCSR(PCCAM_TX_EP_ADDR, csr | SET_TX_FLUSHFIFO);
        }
        PCCamModeEnable = 0;
        break;
    case 2:
        RTNA_DBG_Str(0, "retry pccam\r\n");
        if(glPCCamDmaEnable == 1) {
            glPCCamDmaEnable = 0;
            UsbDmaStop();
            csr = UsbReadTxEpCSR(PCCAM_TX_EP_ADDR);
            csr = csr & TXCSR_RW_MASK;
            UsbWriteTxEpCSR(PCCAM_TX_EP_ADDR, csr | SET_TX_FLUSHFIFO);
            UsbWriteTxEpCSR(PCCAM_TX_EP_ADDR, csr | SET_TX_FLUSHFIFO);
            glPCCamDmaEnable = 0;
        }
        break;
    };
#endif /* (SUPPORT_PCCAM_FUNC==1) */
}
//------------------------------------------------------------------------------
//  Function    : UsbProcessVendorCmd
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbProcessVendorCmd(void)
{
#if PCAM_EN==0
    MMP_UBYTE cmd,request;
    MMP_USHORT csr,index,length;

    cmd = UsbRequestPayload.wValue & 0xFF;
    request = UsbRequestPayload.bRequest;
    index = UsbRequestPayload.wIndex;
    length = UsbRequestPayload.wLength;

    if(glUsbApplicationMode == USB_MTP_MODE) {
        if((request == MS_VENDOR_CODE) && (index == 0x4)) {
            glUsbCtlTxCnt = length;

            if(UsbDescriptorInfo.pExtendedConfigDataSize < length)
                glUsbCtlTxCnt = UsbDescriptorInfo.pExtendedConfigDataSize;

            glUsbCtlState = USB_CONTROL_TX;
        } else {
            csr = UsbReadEp0CSR();
            UsbWriteEp0CSR(csr | SET_EP0_SENDSTALL);
            RTNA_DBG_Str(0, "Unsupport Vendor Request");
        }
    } else if(glUsbApplicationMode == USB_PCCAM_MODE) {
        switch(cmd) {
        case PCCAM_SETDEVMODE_VCMD:
            break;
        default:
            csr = UsbReadEp0CSR();
            UsbWriteEp0CSR(csr | SET_EP0_SENDSTALL);
            RTNA_DBG_Str(0, "Unsupport Vendor Request");
            break;
        }
    } else if(glUsbApplicationMode == USB_PCSYNC_MODE) {
        switch(request) {
        case PCSYNC_BULKINSIZE_VCMD:
        case PCSYNC_BULKOUTSIZE_VCMD:
        case PCSYNC_READREG_VCMD:
            glUsbCtlState = USB_CONTROL_TX;
            break;
        default:
            csr = UsbReadEp0CSR();
            UsbWriteEp0CSR(csr | SET_EP0_SENDSTALL);
            RTNA_DBG_Str(0, "Unsupport Vendor Request");
            break;
        }
    }
#endif    
}

//------------------------------------------------------------------------------
//  Function    : UsbVendorCmdNoDataPhase
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbVendorCmdNoDataPhase(void)
{
    //     volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *)USB_REG_BASE_ADDR;
    //    volatile MMP_USHORT *USB_REG_BASE_W = (volatile MMP_USHORT *)USB_REG_BASE_ADDR;
    MMP_UBYTE cmd;

    cmd = UsbRequestPayload.wValue & 0xFF;

    switch(cmd) {
    case PCCAM_SETDEVMODE_VCMD:
        PCCAMSetDevMode(UsbRequestPayload.wValue >> 8);
        break;

    default:
        RTNA_DBG_Str(0, "UsbVendorCmdNoDataPhase error\r\n");
        break;
    }
}
//*****************************************************************
// function : PCSYNCBulkIn
// input    : none
// output   : none
//*****************************************************************
void PCSYNCBulkIn(MMP_ULONG TxByte)
{
    int i;
    MMP_UBYTE *tx_buf;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_USHORT csr;

    tx_buf = (MMP_UBYTE *) glPCSyncHostOutBufAddr + glPCSyncBulkInBufOffset;

    if(glUsbOperationSpeed == USB_HIGH_SPEED) {
        if(TxByte > 0x200) {
            TxByte = 0x200;
        }
    } else {
        if(TxByte > 0x40) {
            TxByte = 0x40;
        }
    }

    glPCSyncBulkInSize -= TxByte;
    glPCSyncBulkInBufOffset += TxByte;

    for(i = 0;i < TxByte;i++) {
        pUSB_CTL->USB_FIFO_EP[0x1].FIFO_B = tx_buf[i];
    }
    csr = UsbReadTxEpCSR(PCSYNC_TX_EP_ADDR);
    csr = csr & TXCSR_RW_MASK;
    UsbWriteTxEpCSR(PCSYNC_TX_EP_ADDR, csr | SET_TX_TXPKTRDY);
}
//*****************************************************************
// function : mtp_bulk_out
// input    : none
// output   : none
//*****************************************************************
void PCSYNCBulkOut(MMP_ULONG RxByte)
{
    if(glUsbOperationSpeed == USB_HIGH_SPEED) {
        if(RxByte == 0) {
            RxByte = 0x200;
        }
        if((RxByte & 0x1FF) != 0) {
            RxByte = RxByte + 0x200;
        }
        UsbDmaRxConfig(PCSYNC_RX_EP_ADDR, glPCSyncHostInBufAddr, 0x1FF, 0x1FF, (RxByte >> 9) - 1);
    } else {
        if(RxByte == 0) {
            RxByte = 0x40;
        }
        if((RxByte & 0x3F) != 0) {
            RxByte = RxByte + 0x40;
        }
        UsbDmaRxConfig(PCSYNC_RX_EP_ADDR, glPCSyncHostInBufAddr, 0x3F, 0x3F, (RxByte >> 6) - 1);
    }
}
//*****************************************************************
// function : PCSYNCBulkOutOk
// input    : none
// output   : none
//*****************************************************************
void PCSYNCBulkOutOk(void)
{
    *(MMP_UBYTE *) (glPCSyncHandshackBufAddr + PCSYNC_INFLAG_OFFSET_B) = 0x2;
}
//*****************************************************************
// function : PCSYNCBulkInOk
// input    : none
// output   : none
//*****************************************************************
void PCSYNCBulkInOk(void)
{
    if(glPCSyncBulkInSize == 0) {
        *(MMP_UBYTE *) (glPCSyncHandshackBufAddr + PCSYNC_OUTFLAG_OFFSET_B) = 0x0;
    } else {
        PCSYNCBulkIn(glPCSyncBulkInSize);
    }
}
//------------------------------------------------------------------------------
//  Function    : UsbVendorCmdTx
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
#if PCAM_EN==0
void UsbVendorCmdTx(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_USHORT i,length,size,index;
    MMP_UBYTE request,inflag,outflag;

    request = UsbRequestPayload.bRequest;
    length = UsbRequestPayload.wLength;
    index = UsbRequestPayload.wIndex;

    if(glUsbApplicationMode == USB_MTP_MODE) {
        for(i = 0;i < glUsbCtlTxCnt;i++) {
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = UsbDescriptorInfo.pExtendedConfigData[i];
        }
        glUsbCtlTxCnt = 0;
    } else if(glUsbApplicationMode == USB_PCSYNC_MODE) {
        switch(request) {
        case PCSYNC_BULKINSIZE_VCMD:
            size = *(MMP_USHORT *) (glPCSyncHandshackBufAddr + PCSYNC_OUTSIZE_OFFSET_W);

           	pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = size;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = size >> 8;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0;

            if(size != 0) {
                *(MMP_USHORT *) (glPCSyncHandshackBufAddr + PCSYNC_OUTSIZE_OFFSET_W) = 0;
                glPCSyncBulkInSize = size;
                glPCSyncBulkInBufOffset = 0;
                PCSYNCBulkIn(size);
            }
            glUsbCtlTxCnt = 0;

            break;
        case PCSYNC_BULKOUTSIZE_VCMD:
            for(i = 0;i < (length&0xFF);i++) {
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0;
            }

            *(MMP_USHORT *) (glPCSyncHandshackBufAddr + PCSYNC_INSIZE_OFFSET_W) = index;

            PCSYNCBulkOut(index);

            glUsbCtlTxCnt = 0;
            break;
        case PCSYNC_READREG_VCMD:
            if(index == 0x654C) {
                inflag = *(MMP_UBYTE *) (glPCSyncHandshackBufAddr + PCSYNC_INFLAG_OFFSET_B);
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = inflag;
            } else if(index == 0x654D) {
                outflag = *(MMP_UBYTE *) (glPCSyncHandshackBufAddr + PCSYNC_OUTFLAG_OFFSET_B);
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = outflag;
            }
            glUsbCtlTxCnt = 0;
            break;
        }
    }

    if(glUsbCtlTxCnt == 0) {
        UsbWriteEp0CSR(SET_EP0_TXPKTRDY | EP0_DATAEND_BIT);
        glUsbCtlState = USB_CONTROL_IDLE;
    } else {
        UsbWriteEp0CSR(SET_EP0_TXPKTRDY);
    }
}
#endif

//------------------------------------------------------------------------------
//  Function    : UsbVendorCmdRx
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbVendorCmdRx(void)
{
}

#endif //#if    defined(USB_FW)
#undef exUSBVEND
