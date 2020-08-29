//************************************************************************************
// Copyright (C) 2003-2004 Alpha Imaging Technology Corp.
// This file is confidential. All rights reserved.
// Description : AIT810 firmware source code
//************************************************************************************
#define exUSBSTD
#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"

#include "mmp_reg_gbl.h"
#include "mmpf_audio_ctl.h"

#include "mmpf_pll.h"

#include "mmps_3gprecd.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc.h"
#include "mmpd_system.h"
#include "pcam_usb.h"
#include "mmp_reg_usb.h"

/** @addtogroup MMPF_USB
@{
*/
#define WIN81_TABLET_PATCH  (1)

extern MMPF_OS_FLAGID   SYS_Flag_Hif;

//extern USB_REQUEST_PAYLOAD UsbRequestPayload;
//extern MMP_ULONG   glUsbCtlState;
//extern MMP_ULONG   glTestModeFeature;

#if (USB_EN)

#pragma O2


#if (SUPPORT_UVC_FUNC==1)
/*
extern MMP_UBYTE UsbRequestPayload_bmRequestType;
extern MMP_UBYTE UsbRequestPayload_bRequest;
extern MMP_USHORT UsbRequestPayload_wValue;
extern MMP_USHORT UsbRequestPayload_wIndex;
extern MMP_USHORT UsbRequestPayload_wLength;
*/
extern UsbCtrlReq_t gsUsbCtrlReq ;
extern MMP_UBYTE  gbUsbHighSpeed;

extern   MMP_USHORT USBEP0TxCnt;
extern   MMP_UBYTE  USBInPhase;


//extern   MMP_USHORT USBFrameNum;

//extern   MMP_UBYTE  USBConfig;
//extern   MMP_UBYTE  USBInterface;
//extern   MMP_UBYTE  USBRequestType;
//extern   MMP_UBYTE  USBDevice; 
extern   MMP_UBYTE  USBCmd;
extern   MMP_UBYTE  gbEP0NotEnd;
extern   MMP_UBYTE  USBOutPhase;  
extern   MMP_USHORT USBConfigDescLen;
//extern   MMP_UBYTE  InterfaceMode;

//extern   MMP_UBYTE  gbvcstart;

extern   MMP_USHORT USBConfig1Len;

extern   MMP_UBYTE  USBSN[13];

//extern   MMP_UBYTE  USBAlterSetting;

extern   MMP_UBYTE  USBDescBufAddr[0x800];

MMP_USHORT  EP0TXOffset;

//extern   MMP_UBYTE gbStopPreviewEvent ;
//extern	 MMP_UBYTE gbStillCaptureEvent;
//extern   MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
//extern  MMP_ULONG   glPccamResolution;



//*****************************************************************
// function : send_getstatus_data
// input    : none
// output   : none
//*****************************************************************
void send_getstatus_data(void)
{
    USBInPhase = GET_STATUS_CMD;
    USBEP0TxCnt = 0x02;  
    //    USBEP0TxAddr = USBEP0TxBufAddr;  
    usb_ep0_in();
}

//*****************************************************************
// function : std_setup_getstatus
// input    : none
// output   : none
//*****************************************************************
void std_setup_getstatus(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_UBYTE reqrecipient = gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqRecipient;

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    if(/*USBRequestType*/reqrecipient == REQ_DEVICE) {
        pUSB_CTL->USB_FIFO_EP[0x0].FIFO_W = gsUsbCtrlReq.bDevice ;//USBDevice;
        send_getstatus_data();
    } else if(/*USBRequestType*/reqrecipient == REQ_INTERFACE) {
        pUSB_CTL->USB_FIFO_EP[0x0].FIFO_W = gsUsbCtrlReq.wInterface;//USBInterface;
        send_getstatus_data();
    } else if(/*USBRequestType*/reqrecipient == REQ_ENDPOINT) {
        pUSB_CTL->USB_FIFO_EP[0x0].FIFO_W = gsUsbCtrlReq.bEndPoint;//USBEndpoint;
        send_getstatus_data();
    } else {
        usb_error();
    }
}

//*****************************************************************
// function : std_in_getstatus
// input    : none
// output   : none
//*****************************************************************
void std_in_getstatus(void)
{
    USBOutPhase = GET_STATUS_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_out_getstatus
// input    : none
// output   : none
//*****************************************************************
void std_out_getstatus(void)
{
}

//*****************************************************************
// function : send_clrfeature_data
// input    : none
// output   : none
//*****************************************************************
void send_clrfeature_data(void)
{
    USBInPhase = CLEAR_FEATURE_CMD;
    usb_ep0_null_in();
}

//*****************************************************************
// function : std_setup_clearfeature
// input    : none
// output   : none
//*****************************************************************
void std_setup_clearfeature(void)
{
    MMP_UBYTE tmp,reqrecipient ;
    reqrecipient = gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqRecipient;
    tmp = /*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex;
    //UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    if( /*USBRequestType*/ reqrecipient== REQ_DEVICE) {
        if((/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue & 0xFF) != 2) {
            usb_error();
        } else {
            /*USBDevice*/gsUsbCtrlReq.bDevice &= 0xFD;
            send_clrfeature_data();
        }
    } else if( /*USBRequestType*/ reqrecipient == REQ_INTERFACE) {
        //           if( ((UsbRequestPayload_wIndex&0xFF)!= 0) && ((UsbRequestPayload_wIndex&0xFF)!= 1) ) {
        usb_error();
        //         }
        //      USBInterface = 0;
        //        send_clrfeature_data();
    } else if( /*USBRequestType*/ reqrecipient == REQ_ENDPOINT) {
        /*USBEndpoint*/gsUsbCtrlReq.bEndPoint  &= 0xFE; 
        if(tmp == 0x81) {
            clear_ep1_sequence();         
            send_clrfeature_data();
#if USB_UVC_BULK_EP			
            if(gbvcstart) {
			   usb_uvc_stop_preview();
            }
#endif
        } else if(tmp == 0x82) {
            clear_ep2_sequence();
            send_clrfeature_data();
#if USB_UVC_BULK_EP			
            if(gbvcstart) {
			   usb_uvc_stop_preview();
            }
#endif
        } else if(tmp == 0x83) {
            clear_ep3_sequence();
            send_clrfeature_data();
        } else if(tmp == 0x84) { // interrupt ep for ch8 test
            clear_ep4_sequence();
            send_clrfeature_data();
        } else {
            usb_error();
        }
    } else {
        usb_error();
    }
}

//*****************************************************************
// function : std_in_clearfeature
// input    : none
// output   : none
//*****************************************************************
void std_in_clearfeature(void)
{
    USBOutPhase = CLEAR_FEATURE_CMD;
}

//*****************************************************************
// function : std_out_clearfeature
// input    : none
// output   : none
//*****************************************************************
void std_out_clearfeature(void)
{
}

//*****************************************************************
// function : send_setfeature_data
// input    : none
// output   : none
//*****************************************************************
void send_setfeature_data(void)
{
    USBInPhase = SET_FEATURE_CMD;
    usb_ep0_null_in();
}

//*****************************************************************
// function : std_setup_setfeature
// input    : none
// output   : none
//*****************************************************************
void std_setup_setfeature(void)
{
    MMP_UBYTE  reqrecipient = gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqRecipient;
    if(gsUsbCtrlReq.wValue == 0x02) {
        gsUsbCtrlReq.wTestMode = gsUsbCtrlReq.wIndex;
        if((gsUsbCtrlReq.wIndex & 0xFF) != 0) {
            usb_error();
            return;
        }
    } else if(gsUsbCtrlReq.wValue == 0x00) {
        ;
    } else {
        usb_error();            
        return;
    }

    if(  reqrecipient== REQ_DEVICE) {
        if((gsUsbCtrlReq.wValue & 0xFF) != 2) {
            usb_error();
            return;
        }
        gsUsbCtrlReq.bDevice |= 0x02;
        send_setfeature_data();
    } else if(reqrecipient == REQ_INTERFACE) {
        //           if( ((UsbRequestPayload_wIndex&0xFF)!= 0) && ((UsbRequestPayload_wIndex&0xFF)!= 1) ) {
        usb_error();
        return;
        //         }
        //           USBInterface = 0;
        //           send_setfeature_data();
    } else if( reqrecipient == REQ_ENDPOINT) {
        if((( gsUsbCtrlReq.wIndex & 0xFF) != 0x81) && 
           (( gsUsbCtrlReq.wIndex & 0xFF) != 0x82) && 
           (( gsUsbCtrlReq.wIndex & 0xFF) != 0x83) &&
           (( gsUsbCtrlReq.wIndex & 0xFF) != (0x80 | PCCAM_EX_EP_ADDR ))) {
            usb_error();
            return;
        }
        if( gsUsbCtrlReq.wIndex==(0x80|PCCAM_EX_EP_ADDR)) {
           StallTx(PCCAM_EX_EP_ADDR);  
        }   
        gsUsbCtrlReq.bEndPoint |= 0x01;
        send_setfeature_data();
    } else {
        usb_error();
        return;
    }
}

//*****************************************************************
// function : std_in_setfeature
// input    : none
// output   : none
//*****************************************************************
void std_in_setfeature(void)
{
    //    USBOutPhase = SET_FEATURE_CMD;
}

//*****************************************************************
// function : std_out_setfeature
// input    : none
// output   : none
//*****************************************************************
void std_out_setfeature(void)
{
}
//*****************************************************************
// function : std_setup_setaddress
// input    : none
// output   : none
//*****************************************************************
void std_setup_setaddress(void)
{
    gsUsbCtrlReq.bDevAddr = gsUsbCtrlReq.wValue; //UsbRequestPayload_wValue;  
    #if 0
    RTNA_DBG_Byte(0, gsUsbCtrlReq.bDevAddr );                            
    RTNA_DBG_Str(0, "--USBDevAddr\r\n");
    #endif
    
    //    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    USBInPhase = SET_ADDRESS_CMD;
    usb_ep0_null_in();
}

//*****************************************************************
// function : std_in_setaddress
// input    : none
// output   : none
//*****************************************************************
void std_in_setaddress(void)
{
   AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;

    pUSB_CTL->USB_FADDR = gsUsbCtrlReq.bDevAddr ;
}

//*****************************************************************
// function : std_out_setaddress
// input    : none
// output   : none
//*****************************************************************
void std_out_setaddress(void)
{
}
//*****************************************************************
// function : std_setup_setdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_setup_setdescriptor(void)
{
    //    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    USBInPhase = SET_DESCRIPTOR_CMD;
    usb_ep0_null_in();
}

//*****************************************************************
// function : std_in_setdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_in_setdescriptor(void)
{
    USBOutPhase = SET_DESCRIPTOR_CMD;
    // usb_ep0_null_out();  
}

//*****************************************************************
// function : std_out_setdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_out_setdescriptor(void)
{
}

//*****************************************************************
// function : std_setup_getconfiguration
// input    : none
// output   : none
//*****************************************************************
void std_setup_getconfiguration(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    #if WIN81_TABLET_PATCH==0
    pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B  =gsUsbCtrlReq.bConfig;// USBConfig;                           
    USBInPhase = GET_CONFIG_CMD;
    USBEP0TxCnt = 0x01;
    usb_ep0_in();
    #else
    USB_PUT_EP0_DATA(GET_CONFIG_CMD,1,(MMP_UBYTE *)&gsUsbCtrlReq.bConfig);
    #endif
    //    USBEP0TxAddr = USBEP0TxBufAddr;
}

//*****************************************************************
// function : std_in_getconfiguration
// input    : none
// output   : none
//*****************************************************************
void std_in_getconfiguration(void)
{
    USBOutPhase = GET_CONFIG_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_out_getconfiguration
// input    : none
// output   : none
//*****************************************************************
void std_out_getconfiguration(void)
{
}
//*****************************************************************
// function : std_setup_setconfiguration
// input    : none
// output   : none
//*****************************************************************
void std_setup_setconfiguration(void)
{
    //USBConfig = gsUsbCtrlReq.wValue ;//UsbRequestPayload_wValue;  
    gsUsbCtrlReq.bConfig = gsUsbCtrlReq.wValue ; 
    // UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    clear_ep1_sequence();
    clear_ep2_sequence();
    clear_ep3_sequence();
    
    gsUsbCtrlReq.wAlterSetting = 0x00;
    gsUsbCtrlReq.wInterface    = 0x00;
    gsUsbCtrlReq.bEndPoint     = 0x00;
    //USBInterface = 0; 
    //USBEndpoint = 0;  
    //USBAlterSetting = 0; 
    USBInPhase = SET_CONFIG_CMD; 
    usb_ep0_null_in();
}

//*****************************************************************
// function : std_in_setconfiguration
// input    : none
// output   : none
//*****************************************************************
void std_in_setconfiguration(void)
{
    //    USBOutPhase = SET_CONFIG_CMD;
}

//*****************************************************************
// function : std_out_setconfiguration
// input    : none
// output   : none
//*****************************************************************
void std_out_setconfiguration(void)
{
}
//*****************************************************************
// function : std_setup_getinterface
// input    : none
// output   : none
//*****************************************************************
void std_setup_getinterface(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    #if WIN81_TABLET_PATCH==0
    pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B  = gsUsbCtrlReq.wAlterSetting;//USBAlterSetting;                           
    USBInPhase = GET_INTERFACE_CMD;
    USBEP0TxCnt = 0x01;
    usb_ep0_in();
    #else
    USB_PUT_EP0_DATA(GET_INTERFACE_CMD,1,(MMP_UBYTE *)&gsUsbCtrlReq.wAlterSetting );
    #endif
    //    USBEP0TxAddr = USBEP0TxBufAddr;
}

//*****************************************************************
// function : std_in_getinterface
// input    : none
// output   : none
//*****************************************************************
void std_in_getinterface(void)
{
    USBOutPhase = GET_INTERFACE_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_out_getinterface
// input    : none
// output   : none
//*****************************************************************
void std_out_getinterface(void)
{
}
//*****************************************************************
// function : std_setup_setinterface
// input    : none
// output   : none
//*****************************************************************

void std_setup_setinterface(void)
{
extern MMP_USHORT  UVCTxFIFOSize[] ;
extern MMP_SHORT   gsVol ;
    //USBAlterSetting =  gsUsbCtrlReq.wValue; // UsbRequestPayload_wValue;  
    //USBInterface    =  gsUsbCtrlReq.wIndex;//UsbRequestPayload_wIndex;
    gsUsbCtrlReq.wAlterSetting = gsUsbCtrlReq.wValue;
    gsUsbCtrlReq.wInterface    = gsUsbCtrlReq.wIndex;
    
    //dbg_printf(3,"set if(indx=%x,val=%x)\r\n",UsbRequestPayload_wIndex,UsbRequestPayload_wValue);
    
    if(/*USBInterface*/ gsUsbCtrlReq.wInterface == 0) {
    
    } else if(/*USBInterface*/ gsUsbCtrlReq.wInterface == 1) {
        STREAM_SESSION *ss ;
        ss =  MMPF_Video_GetStreamSessionByEp(0);
        if((ss->tx_flag & SS_TX_COMMITED) && (gsUsbCtrlReq.wAlterSetting)) {
            ss->tx_flag &= ~SS_TX_COMMITED ;
            if(UVC_VCD()==bcdVCD15) {
                MMPF_Video_InitStreamSession(0,SS_TX_STREAMING | SS_TX_BY_DMA,UVCTxFIFOSize[0]);
            } else {
                MMPF_Video_InitStreamSession(0,SS_TX_STREAMING | SS_TX_BY_DMA,UVCTxFIFOSize[0]); 
            }
            
            usb_uvc_start_preview(ss->ep_id,0);
        } else if((/*gbvcstart*/ss->tx_flag & SS_TX_STREAMING) && ( gsUsbCtrlReq.wAlterSetting == 0x00)) {
            usb_uvc_stop_preview(ss->ep_id,0);
        }
    } else if(/*USBInterface*/ gsUsbCtrlReq.wInterface == 2) {
        STREAM_SESSION *ss ;
        ss =  MMPF_Video_GetStreamSessionByEp(1);
        if((ss->tx_flag & SS_TX_COMMITED) && ( gsUsbCtrlReq.wAlterSetting)) {
            ss->tx_flag &= ~SS_TX_COMMITED ;
            if(UVC_VCD()==bcdVCD15) {
                MMPF_Video_InitStreamSession(1,SS_TX_STREAMING | SS_TX_BY_DMA,UVCTxFIFOSize[1]);
            } else {
                MMPF_Video_InitStreamSession(1,SS_TX_STREAMING | SS_TX_BY_DMA,UVCTxFIFOSize[1]); 
            }
            
            usb_uvc_start_preview(ss->ep_id,0);
        } else if((/*gbvcstart*/ss->tx_flag & SS_TX_STREAMING) && ( gsUsbCtrlReq.wAlterSetting == 0x00)) {
            usb_uvc_stop_preview(ss->ep_id,0);
        }

    } else if(/*USBInterface*/ gsUsbCtrlReq.wInterface == 3) {
		extern int AlternateSamplingRate;
		switch( gsUsbCtrlReq.wAlterSetting){
			case 0:
					USB_AudioPreviewStop(PCAM_API/*PCAM_NONBLOCKING*/);
		            RTNA_DBG_Str(0, "  *[USB cmd]A_E_Q*\r\n");
		            break;
		    case 1: //16K sampling	
				    AlternateSamplingRate= 16000;	
				    PCAM_USB_SetGain_UAC2ADC(gsVol,AlternateSamplingRate);    		
				    usb_audio_set_max_pktsize(AlternateSamplingRate,0);
		            USB_AudioPreviewStart(PCAM_API/*PCAM_NONBLOCKING*/);
            		RTNA_DBG_Str(0, "  *[USB cmd]A_S,16Kz*\r\n");
		            break;
			case 2: //24K sampling
					AlternateSamplingRate= 24000;
					PCAM_USB_SetGain_UAC2ADC(gsVol,AlternateSamplingRate); 
					usb_audio_set_max_pktsize(AlternateSamplingRate,0);
		            USB_AudioPreviewStart(PCAM_API/*PCAM_NONBLOCKING*/);
            		RTNA_DBG_Str(0, "  *[USB cmd]A_S,24Khz*\r\n");
            		break;
            case 3: //32K sampling
            		AlternateSamplingRate= 32000;
            		PCAM_USB_SetGain_UAC2ADC(gsVol,AlternateSamplingRate); 
            		usb_audio_set_max_pktsize(AlternateSamplingRate,0);
		            USB_AudioPreviewStart(PCAM_API/*PCAM_NONBLOCKING*/);
            		RTNA_DBG_Str(0, "  *[USB cmd]A_S,32Khz*\r\n");
					break;
			case 4: //48K sampling
					AlternateSamplingRate= 48000;
					PCAM_USB_SetGain_UAC2ADC(gsVol,AlternateSamplingRate); 
					usb_audio_set_max_pktsize(AlternateSamplingRate,0);
		            USB_AudioPreviewStart(PCAM_API/*PCAM_NONBLOCKING*/);
            		RTNA_DBG_Str(0, "  *[USB cmd]A_S,48Kz*\r\n");
            		break;            
		}   
    }

    // UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    //USBEndpoint = 0;     
    gsUsbCtrlReq.bEndPoint = 0;
    USBInPhase = SET_INTERFACE_CMD;
    usb_ep0_null_in();
}

//*****************************************************************
// function : std_in_setinterface
// input    : none
// output   : none
//*****************************************************************
void std_in_setinterface(void)
{
    //USBOutPhase = SET_INTERFACE_CMD;
    // usb_ep0_null_out();  
}

//*****************************************************************
// function : std_out_setinterface
// input    : none
// output   : none
//*****************************************************************
void std_out_setinterface(void)
{
}

//*****************************************************************
// function : std_setup_synchframe
// input    : none
// output   : none
//*****************************************************************
void std_setup_synchframe(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    
    if(/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex==0)
    {
       UsbWriteEp0CSR(SET_EP0_SENDSTALL);       
    }
    else
    {
    #if WIN81_TABLET_PATCH==0
       pUSB_CTL->USB_FIFO_EP[0x0].FIFO_W  =gsUsbCtrlReq.wSyncFrame;//USBFrameNum;
       USBInPhase = SYNCH_FRAME_CMD;
       USBEP0TxCnt = 0x02;
       usb_ep0_in();
    #else
        USB_PUT_EP0_DATA(SYNCH_FRAME_CMD,2,(MMP_UBYTE *)&gsUsbCtrlReq.wSyncFrame) ;
    #endif   
       //    USBEP0TxAddr = USBEP0TxBufAddr;
    }
}

//*****************************************************************
// function : std_in_syncframe
// input    : none
// output   : none
//*****************************************************************
void std_in_syncframe(void)
{
    USBOutPhase = SYNCH_FRAME_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_out_syncframe
// input    : none
// output   : none
//*****************************************************************
void std_out_syncframe(void)
{
}
//*****************************************************************
// function : std_setup_getdevicedescriptor
// input    : none
// output   : none
//*****************************************************************
void std_setup_getdevicedescriptor(void)
{
    MMP_UBYTE tmp;
    MMP_USHORT i;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;


    tmp = gsUsbCtrlReq.wLength ; //UsbRequestPayload_wLength;
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    
    if(tmp < 0x12) {
        USBEP0TxCnt = tmp;
    } else {
        USBEP0TxCnt = DEVICE_DESCRIPTOR_DATA1[0];//0x12;
    }
    
    //USBEP0TxCnt = DEVICE_DESCRIPTOR_DATA1[0];
    #if WIN81_TABLET_PATCH==0
    for(i = 0;i < USBEP0TxCnt;i++) {
        pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = DEVICE_DESCRIPTOR_DATA1[i] ;
    }
    USBInPhase = GET_DEV_DESCR_CMD;
    usb_ep0_in();
    #else
    USB_PUT_EP0_DATA(GET_DEV_DESCR_CMD,USBEP0TxCnt,DEVICE_DESCRIPTOR_DATA1);
    #endif
}


//*****************************************************************
// function : std_in_getdevicedescriptor
// input    : none
// output   : none
//*****************************************************************
void std_in_getdevicedescriptor(void)
{
    USBOutPhase = GET_DEV_DESCR_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_in_getdevquadescriptor
// input    : none
// output   : none
//*****************************************************************
void std_in_getdevquadescriptor(void)
{
    USBOutPhase = GET_DEVQUA_DESCR_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_out_getdevicedescriptor
// input    : none
// output   : none
//*****************************************************************
void std_out_getdevicedescriptor(void)
{
}

//*****************************************************************
// function : std_out_getdevquadescriptor
// input    : none
// output   : none
//*****************************************************************
void std_out_getdevquadescriptor(void)
{
}
//*****************************************************************
// function : std_setup_getdevicequalifierdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_setup_getdevicequalifierdescriptor(void)
{
    MMP_UBYTE tmp;
    MMP_USHORT i;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;

    tmp = /*UsbRequestPayload_wLength*/gsUsbCtrlReq.wLength;
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    
    if(tmp < 0x0A) {
        USBEP0TxCnt = tmp;
    } else {
        USBEP0TxCnt = DEVICE_QUALIFIER_DESCRIPTOR_DATA1[0] ;//0x0A;
    }
    
    //USBEP0TxCnt = DEVICE_QUALIFIER_DESCRIPTOR_DATA1[0] ;
    #if WIN81_TABLET_PATCH==0
    for(i = 0;i < USBEP0TxCnt;i++) {
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = DEVICE_QUALIFIER_DESCRIPTOR_DATA1[i];
    }

    USBInPhase = GET_DEVQUA_DESCR_CMD;
    usb_ep0_in();
    #else
    USB_PUT_EP0_DATA(GET_DEVQUA_DESCR_CMD,USBEP0TxCnt,DEVICE_QUALIFIER_DESCRIPTOR_DATA1);
    #endif
}


//*****************************************************************
// function : std_in_getdevicequalifierdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_in_getdevicequalifierdescriptor(void)
{
    USBOutPhase = GET_DEVQUA_DESCR_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_out_getdevicequalifierdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_out_getdevicequalifierdescriptor(void)
{
}
//*****************************************************************
// function : std_setup_getdevicedescriptor
// input    : none
// output   : none
//*****************************************************************
void std_setup_getotherspeedconfigdescriptor(void)
{
    MMP_UBYTE  *desc ;
    MMP_USHORT tmp,length;
    MMP_USHORT i;
    MMP_UBYTE tmpspeed;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;    

    EP0TXOffset = 0;
    tmpspeed = gbUsbHighSpeed;

    if(tmpspeed) {
        gbUsbHighSpeed = 0;
    } else {
        gbUsbHighSpeed = 1;
    }
#if USING_EXT_USB_DESC==0
    desc = std_get_configuration_desc(gbUsbHighSpeed,&length) ;
    //usb_desc_init();
#endif
    if(tmpspeed) {
        gbUsbHighSpeed = 1;
    } else {
        gbUsbHighSpeed = 0;
    }
    tmp = gsUsbCtrlReq.wLength;//UsbRequestPayload_wLength;

    //length = (MMP_USHORT) (USBConfigDescLen);

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    gbEP0NotEnd = 0;

    if(tmp < 0x40) {
        if(length < tmp) {
            USBEP0TxCnt = length; 
            USBConfigDescLen = 0;
        } else {
            USBEP0TxCnt = tmp; 
            USBConfigDescLen = 0;
        }
        #if USING_EXT_USB_DESC==1
        OutputConfDesc(USBEP0TxCnt);
        #endif
    } else {
        if(length >= 0x40) {
            USBEP0TxCnt = 0x40; 
            if(length > tmp) {
                USBConfigDescLen = tmp;
            } else {
                USBConfigDescLen = length;
            }
            #if USING_EXT_USB_DESC==1
            OutputConfDesc(USBConfigDescLen);
            #endif
            
            USBConfigDescLen -= 0x40;

            gbEP0NotEnd = 1;
        } else {
            USBEP0TxCnt = length; 
            USBConfigDescLen = 0;
            #if USING_EXT_USB_DESC==1
	   		OutputConfDesc(USBEP0TxCnt);
            #endif
        }
    }

#if WIN81_TABLET_PATCH==0
    #if USING_EXT_USB_DESC==0   

        for(i = 0;i < USBEP0TxCnt;i++) {
            if(i == 1) // type
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x07; // other speed 
            else
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B  = desc[i];
        }
        EP0TXOffset += USBEP0TxCnt;
    #endif
        USBInPhase = GET_OTHERSP_DESCR_CMD;
        usb_ep0_in();
#else
    {
        #if USING_EXT_USB_DESC==0
            MMP_UBYTE other_speed ;
            other_speed = desc[1] ;
            desc[1] = 0x07 ;
            USB_PUT_EP0_DATA(GET_OTHERSP_DESCR_CMD,USBEP0TxCnt,desc[i] );
            desc[1] = other_speed;
            EP0TXOffset += USBEP0TxCnt;
            
        #else
            USBInPhase = GET_OTHERSP_DESCR_CMD;
            usb_ep0_in();
        #endif
    }

#endif    
}


//*****************************************************************
// function : std_in_getdevicedescriptor
// input    : none
// output   : none
//*****************************************************************
void std_in_getotherspeedconfigdescriptor(void)
{
    MMP_UBYTE  *desc ;
    MMP_USHORT i,length;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;    
    #if USING_EXT_USB_DESC==0
    desc = std_get_configuration_desc(0,&length) ;
    #endif
    if(USBConfigDescLen == 0) {
        //usb_desc_init();     
        USBOutPhase = GET_OTHERSP_DESCR_CMD;
        usb_ep0_null_out();
        // std_out_getotherspeedconfigdescriptor(); avoid lost out int
    } else {
        if(USBConfigDescLen < 0x40) {
            USBEP0TxCnt = USBConfigDescLen;
            USBConfigDescLen = 0;
            gbEP0NotEnd = 0;
        } else {
            USBConfigDescLen -= 0x40;
            USBEP0TxCnt = 0x40;
        }
        #if WIN81_TABLET_PATCH==0
        
        #if USING_EXT_USB_DESC==0
        for(i = 0;i < USBEP0TxCnt;i++) {
             pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = desc[ i + EP0TXOffset ] ;//USBDescBufAddr[CONFIG_DESCRIPTOR_DATA1_ADDR + i + EP0TXOffset];
        } 
        #endif
        
        EP0TXOffset += USBEP0TxCnt;        
        USBInPhase = GET_OTHERSP_DESCR_CMD;
        usb_ep0_in();
        
        #else
        {
        #if USING_EXT_USB_DESC==0
            MMP_UBYTE *ptr = desc + EP0TXOffset ;
            USB_PUT_EP0_DATA(GET_OTHERSP_DESCR_CMD,USBEP0TxCnt,ptr);
            EP0TXOffset += USBEP0TxCnt;  
        #else
            EP0TXOffset += USBEP0TxCnt;        
            USBInPhase = GET_OTHERSP_DESCR_CMD;
            usb_ep0_in();
        
        
        #endif
        }
        
        #endif
    }
}

//*****************************************************************
// function : std_out_getotherspeedconfigdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_out_getotherspeedconfigdescriptor(void)
{
#if USING_EXT_USB_DESC==1
   // usb_desc_init();
#endif    
    USBConfigDescLen = USBConfig1Len;
}
//*****************************************************************
// function : std_setup_getstringdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_setup_getstringdescriptor(void)
{
    MMP_USHORT tmp,length;
    MMP_UBYTE i,cmd,*desc=0;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;    

    tmp = gsUsbCtrlReq.wValue & 0xFF;//UsbRequestPayload_wValue & 0xFF;
    length = gsUsbCtrlReq.wLength;//UsbRequestPayload_wLength;  

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    if(tmp == 0) {
        if(length > LANGUAGE_ID_DATA[0]) {
            USBEP0TxCnt = LANGUAGE_ID_DATA[0];
        } else {
            USBEP0TxCnt = length;
        }
        
        #if WIN81_TABLET_PATCH==0
        for(i = 0;i < USBEP0TxCnt;i++) {
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = LANGUAGE_ID_DATA[i];// USBDescBufAddr[LANGUAGE_ID_DATA_ADDR + i];
        }
        USBInPhase = GET_STRING_DESCR_CMD;
        usb_ep0_in();
        #else
        desc = LANGUAGE_ID_DATA;
        cmd = GET_STRING_DESCR_CMD ;
        #endif
        
    } else if(tmp == 0x01) {
        tmp = length;
        if(tmp < MANUFACTURER_STRING_DATA[0]) {
            USBEP0TxCnt = tmp;
        } else {
            USBEP0TxCnt = MANUFACTURER_STRING_DATA[0];
        }
        #if WIN81_TABLET_PATCH==0
        for(i = 0;i < USBEP0TxCnt;i++) {
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = MANUFACTURER_STRING_DATA[i];//USBDescBufAddr[MANUFACTURER_STRING_DATA_ADDR + i];
        }
        USBInPhase = GET_STRING_DESCR_CMD;
        usb_ep0_in();
        #else
        desc = MANUFACTURER_STRING_DATA;
        cmd = GET_STRING_DESCR_CMD ;
        #endif
    } else if(tmp == 0x02) {
        tmp = length;
        if(tmp < PRODUCT_STRING_DATA[0]) {
            USBEP0TxCnt = tmp;
        } else {
            USBEP0TxCnt = PRODUCT_STRING_DATA[0];
        }
        #if WIN81_TABLET_PATCH==0
        for(i = 0;i < USBEP0TxCnt;i++) {
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = PRODUCT_STRING_DATA[i];//USBDescBufAddr[PRODUCT_STRING_DATA_ADDR + i];
        }
        USBInPhase = GET_STRING_DESCR_CMD;
        usb_ep0_in();
        #else
        desc = PRODUCT_STRING_DATA; 
        cmd = GET_STRING_DESCR_CMD ;
        #endif
    } else if(tmp == 0x03) {
        USBInPhase = GET_STRING_DESCR_CMD;
        usb_ep0_in();
    } else {
        usb_error();
    }
    #if WIN81_TABLET_PATCH
    if(desc) {
        USB_PUT_EP0_DATA(cmd,USBEP0TxCnt,desc);
    }
    #endif
}

//*****************************************************************
// function : std_in_getstringdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_in_getstringdescriptor(void)
{
    USBOutPhase = GET_STRING_DESCR_CMD;
    usb_ep0_null_out();
}

//*****************************************************************
// function : std_out_getstringdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_out_getstringdescriptor(void)
{
}


MMP_UBYTE *std_get_configuration_desc(MMP_BOOL highspeed,MMP_USHORT *desclen)
{
extern MMP_UBYTE *get_configuration_len(MMP_BOOL highspeed,MMP_USHORT *len);
    MMP_USHORT total_len,act_len = 0,i;
    MMP_UBYTE  *ptrbase = get_configuration_len(highspeed,&total_len);
    MMP_UBYTE  *ptr ;
    ptr = ptrbase ;
    do {
        act_len+= ptr[0] ;
        ptr += ptr[0] ;
    } while(act_len < total_len );
   //dbg_printf(3,"confdesc len( %d,%d )\r\n",total_len,act_len);
    *desclen = act_len ;
    ptrbase[2] = LOBYTE(act_len);
    ptrbase[3] = HIBYTE(act_len);
    return ptrbase ;
}

//*****************************************************************
// function : std_setup_getconfigurationdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_setup_getconfigurationdescriptor(void)
{
    MMP_UBYTE  *desc ;
    MMP_USHORT tmp,length,len;
    MMP_USHORT i;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;   

    EP0TXOffset = 0;

    tmp = gsUsbCtrlReq.wLength;//UsbRequestPayload_wLength;
#if USING_EXT_USB_DESC==1
    Init_Desc();
    length = GetSize_ConfDesc();
    //dbg_printf(3,"lgt.desccalclen : %d\r\n",length);
#else
    //length = (MMP_USHORT) (USBConfigDescLen);
    desc = std_get_configuration_desc(gbUsbHighSpeed,&length) ;
    //USBConfigDescLen = length ;
#endif
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    gbEP0NotEnd = 0;

    if(tmp <= 0x40) {
        if(length < tmp) {
            USBEP0TxCnt = length; 
            USBConfigDescLen = 0;
        } else {
            USBEP0TxCnt = tmp; 
            USBConfigDescLen = 0;
        }
        #if USING_EXT_USB_DESC==1
   		OutputConfDesc(USBEP0TxCnt);
        #endif
    } else {
        if(length >= 0x40) {
            USBEP0TxCnt = 0x40; 
            if(length > tmp) {
                USBConfigDescLen = tmp;
            } else {
                USBConfigDescLen = length;
            }
            #if USING_EXT_USB_DESC==1
	   		OutputConfDesc(USBConfigDescLen);
            #endif
            USBConfigDescLen -= 0x40;

            gbEP0NotEnd = 1;
        } else {
            USBEP0TxCnt = length; 
            USBConfigDescLen = 0;
            #if USING_EXT_USB_DESC==1
	   		OutputConfDesc(USBEP0TxCnt);
            #endif
        }
    }

#if WIN81_TABLET_PATCH==0    
#if USING_EXT_USB_DESC==0   
    for(i = 0;i < USBEP0TxCnt;i++) {
        pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = desc[i];
    }
    EP0TXOffset += USBEP0TxCnt;
#endif
    USBInPhase = GET_CONFIG_DESCR_CMD;
    usb_ep0_in();
    
#else


#if USING_EXT_USB_DESC==0 
    USB_PUT_EP0_DATA(GET_CONFIG_DESCR_CMD,USBEP0TxCnt,desc);
    EP0TXOffset += USBEP0TxCnt;
#else

    USBInPhase = GET_CONFIG_DESCR_CMD;
    usb_ep0_in();

#endif

#endif    
} 
//*****************************************************************
// function : std_in_getconfigurationdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_in_getconfigurationdescriptor(void)
{
    MMP_UBYTE  *desc ;
    MMP_USHORT  i,length;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    #if USING_EXT_USB_DESC==0
    desc = std_get_configuration_desc(gbUsbHighSpeed,&length) ;
    #endif
    if(USBConfigDescLen == 0) {
        USB_WAIT_EP0_READY();
        USBOutPhase = GET_CONFIG_DESCR_CMD;
        gbEP0NotEnd = 0;
        usb_ep0_null_out();
    } else {
    #if USING_EXT_USB_DESC==1
   		OutputConfDesc(USBConfigDescLen);
    #endif
        
		if(USBConfigDescLen <= 0x40) {
            USBEP0TxCnt = USBConfigDescLen;
            USBConfigDescLen = 0;
            gbEP0NotEnd = 0;
        } else {
            USBConfigDescLen -= 0x40;
            USBEP0TxCnt = 0x40;
        }
        
    #if WIN81_TABLET_PATCH==0        
    #if USING_EXT_USB_DESC==0    
        for(i = 0;i < USBEP0TxCnt;i++) {
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = desc[ i + EP0TXOffset ] ;//USBDescBufAddr[CONFIG_DESCRIPTOR_DATA1_ADDR + i + EP0TXOffset];
        } 
        EP0TXOffset += USBEP0TxCnt;        
    #endif
        USBInPhase = GET_CONFIG_DESCR_CMD;
        usb_ep0_in();
    #else
    
    #if USING_EXT_USB_DESC==0  
    {
        MMP_UBYTE *ptr = desc + EP0TXOffset ;
        USB_PUT_EP0_DATA(GET_CONFIG_DESCR_CMD,USBEP0TxCnt,ptr);
        EP0TXOffset += USBEP0TxCnt; 
    }    
    #else
        USBInPhase = GET_CONFIG_DESCR_CMD;
        usb_ep0_in();
    #endif
    
    #endif        
    }
}


//*****************************************************************
// function : std_out_getconfigurationdescriptor
// input    : none
// output   : none
//*****************************************************************
void std_out_getconfigurationdescriptor(void)
{
#if USING_EXT_USB_DESC==0
    //USBConfigDescLen = USBConfig1Len;
    //dbg_printf(3,"stdout:USBConfig1Len:%d\r\n",USBConfig1Len);
    std_get_configuration_desc(gbUsbHighSpeed,&USBConfigDescLen) ;
    //dbg_printf(3,"stdout:USBConfigDescLen:%d\r\n",USBConfigDescLen);
#endif    
}

//*****************************************************************
// function : std_request_setup 
// input    : none
// output   : none
//*****************************************************************
void std_request_setup(void)
{
    MMP_UBYTE tmp,tmp2;
    tmp = gsUsbCtrlReq.bRequest;//UsbRequestPayload_bRequest;
    tmp2 = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    //dbg_printf(3,"stdsetup:req : %d, sub :%d\r\n",tmp,tmp2);

    if(tmp == GET_STATUS) {
        std_setup_getstatus();
    } else if(tmp == SET_ADDRESS) {
        std_setup_setaddress();
    } else if(tmp == GET_DESCRIPTOR) {
        if(tmp2 == DEVICE_DESCR) {
            std_setup_getdevicedescriptor();
        } else if(tmp2 == CONFIG_DESCR) {
        #if USING_EXT_USB_DESC==1
			SetUSBSpeed(gbUsbHighSpeed);
        #endif	
            std_setup_getconfigurationdescriptor();
        } else if(tmp2 == STRING_DESCR) {
            std_setup_getstringdescriptor();
        } else if(tmp2 == INTERFACE_DESCR) {
            usb_error();
        } else if(tmp2 == ENDPOINT_DESCR) {
            usb_error();
        } else if(tmp2 == DEVICE_QUALIFIER_DESCR) {
            std_setup_getdevicequalifierdescriptor();
        } else if(tmp2 == OTHER_SPEED_CONFIG_DESC) {
        #if USING_EXT_USB_DESC==1
            SetUSBSpeed(!gbUsbHighSpeed | 0x80);
            std_setup_getconfigurationdescriptor();
        #else
            std_setup_getotherspeedconfigdescriptor();
        #endif    
        } else if(tmp2 == INTERFACE_POWER_DESC) {
            usb_error();
        } else {
            usb_error();
        }
    } else if(tmp == SET_DESCRIPTOR) {
        std_setup_setdescriptor();
    } else if(tmp == CLEAR_FEATURE) {
        std_setup_clearfeature();
    } else if(tmp == SET_FEATURE) {
        std_setup_setfeature();
    } else if(tmp == GET_INTERFACE) {
        std_setup_getinterface();
    } else if(tmp == SET_INTERFACE) {
        std_setup_setinterface();
    } else if(tmp == GET_CONFIGURATION) {
        std_setup_getconfiguration();
    } else if(tmp == SET_CONFIGURATION) {
        std_setup_setconfiguration();
    } else if(tmp == SYNCH_FRAME) {
        std_setup_synchframe();
    } else {
        usb_error();
    }
}

//*****************************************************************
// function : std_request_in
// input    : none
// output   : none
//*****************************************************************
void std_request_in(void)
{
    if(USBCmd == GET_DEV_DESCR_CMD) {
        std_in_getdevicedescriptor();
    } else if(USBCmd == GET_CONFIG_DESCR_CMD) {
        std_in_getconfigurationdescriptor();
    } else if(USBCmd == GET_STRING_DESCR_CMD) {
        std_in_getstringdescriptor();
    } else if(USBCmd == SET_DESCRIPTOR_CMD) {
        std_in_setdescriptor();
    } else if(USBCmd == GET_CONFIG_CMD) {
        std_in_getconfiguration();
    } else if(USBCmd == SET_CONFIG_CMD) {
        std_in_setconfiguration();
    } else if(USBCmd == GET_INTERFACE_CMD) {
        std_in_getinterface();
    } else if(USBCmd == SET_INTERFACE_CMD) {
        std_in_setinterface();
    } else if(USBCmd == GET_STATUS_CMD) {
        std_in_getstatus();
    } else if(USBCmd == CLEAR_FEATURE_CMD) {
        std_in_clearfeature();
    } else if(USBCmd == SET_FEATURE_CMD) {
        std_in_setfeature();
    } else if(USBCmd == SET_ADDRESS_CMD) {
        std_in_setaddress();
    } else if(USBCmd == SYNCH_FRAME_CMD) {
        std_in_syncframe();
    } else if(USBCmd == GET_DEVQUA_DESCR_CMD) {
        std_in_getdevquadescriptor();
    } else if(USBCmd == GET_OTHERSP_DESCR_CMD) {
        std_in_getotherspeedconfigdescriptor();
    } else {
        usb_error();
    }
}

//*****************************************************************
// function : std_request_out
// input    : none
// output   : none
//*****************************************************************
void std_request_out(void)
{
    if(USBCmd == GET_STATUS_CMD) {
        std_out_getstatus();
    } else if(USBCmd == CLEAR_FEATURE_CMD) {
        std_out_clearfeature();
    } else if(USBCmd == SET_FEATURE_CMD) {
        std_out_setfeature();
    } else if(USBCmd == SET_ADDRESS_CMD) {
        std_out_setaddress();
    } else if(USBCmd == GET_DEV_DESCR_CMD) {
        std_out_getdevicedescriptor();
    } else if(USBCmd == GET_CONFIG_DESCR_CMD) {
        std_out_getconfigurationdescriptor();
    } else if(USBCmd == GET_STRING_DESCR_CMD) {
        std_out_getstringdescriptor();
    } else if(USBCmd == SET_DESCRIPTOR_CMD) {
        std_out_setdescriptor();
    } else if(USBCmd == GET_CONFIG_CMD) {
        std_out_getconfiguration();
    } else if(USBCmd == SET_CONFIG_CMD) {
        std_out_setconfiguration();
    } else if(USBCmd == GET_INTERFACE_CMD) {
        std_out_getinterface();
    } else if(USBCmd == SET_INTERFACE_CMD) {
        std_out_setinterface();
    } else if(USBCmd == SYNCH_FRAME_CMD) {
        std_out_syncframe();
    } else if(USBCmd == GET_DEVQUA_DESCR_CMD) {
        std_out_getdevquadescriptor();
    } else if(USBCmd == GET_OTHERSP_DESCR_CMD) {
        std_out_getotherspeedconfigdescriptor();
    } else {
        usb_error();
    }
    //    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
}

#endif


#endif  //#if   defined(USB_FW)
#undef exUSBSTD

/// @}