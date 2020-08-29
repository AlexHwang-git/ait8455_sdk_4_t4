#define exUSBMSDC
#include "includes_fw.h"
#include "lib_retina.h"
#include    "config_fw.h"

#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"
#include "mmpf_usbuac.h"
#include "mmpf_usbuvc.h"
#include "mmp_err.h"
#include "pcam_api.h"
#include "pcam_usb.h"
#include "mmp_reg_usb.h"


#if (SUPPORT_UVC_FUNC==1)
/*
extern MMP_UBYTE UsbRequestPayload_bmRequestType;
extern MMP_UBYTE UsbRequestPayload_bRequest;
extern MMP_USHORT UsbRequestPayload_wValue;
extern MMP_USHORT UsbRequestPayload_wIndex;
extern MMP_USHORT UsbRequestPayload_wLength;
*/
extern UsbCtrlReq_t gsUsbCtrlReq;

extern MMP_UBYTE  USBOutData;

MMP_UBYTE gbACITID = 0x01;
MMP_UBYTE gbACOTID = 0x03;
MMP_UBYTE gbACFUID = 0x05;
MMP_UBYTE gbACASEP = (0x80 | PCCAM_AU_EP_ADDR); // same with descriptor

MMP_UBYTE  gbMute;
MMP_SHORT  gsVol;
MMP_ULONG  glSamplingFreq;

#ifndef VOL_DEF
#define VOL_DEF VOL_MAX
#endif

#define VOL_MUTE 0x8000

#define SAM_MAX 48000
#define SAM_MIN 8000
#define SAM_RES 8000
#define SAM_DEF AUDIN_SAMPLERATE

#define MUTE_MAX 1
#define MUTE_MIN 0
#define MUTE_RES 1
#define MUTE_DEF 0


MMP_UBYTE gbACERRCode;

MMP_ULONG au_offset=0;

MMP_BOOL uac_is_tx_busy(void)
{
    MMP_USHORT csr ;
    csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR);
    return (csr & TX_TXPKTRDY_BIT)?1:0 ;
}

void uac_process_audio_data(void)
{
    MMP_USHORT csr;
    csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR);
    csr = csr & TXCSR_RW_MASK;
    UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr | SET_TX_TXPKTRDY | SET_TX_ISO); // ISO 0x4000
    au_offset++;
}


void uac_init(void)
{
    gbMute = MUTE_DEF ;
    gsVol = VOL_DEF;
    glSamplingFreq = AUDIN_SAMPLERATE ;
#if (MIC_SOURCE==MIC_IN_PATH_AFE)    
//    PCAM_USB_SetGain_UAC2ADC(gsVol,glSamplingFreq);
#endif
}

VC_CMD_CFG ASE_FREQ_CFG =
{
    (CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD),
    0, // no GET_INFO
    3,0,
    SAM_RES,SAM_DEF,SAM_MIN,SAM_MAX,SAM_DEF 
} ;


void usb_ac_ase_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs;
    //volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   

    cs = ( /*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    switch(cs) {
    case ASE_SAMPLING_FREQ_CONTROL:
        usb_vc_cmd_cfg(req,&ASE_FREQ_CFG,glSamplingFreq);
        break;
    case ASE_CONTROL_UNDEFINED:
    case ASE_PITCH_CONTROL:
    default:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbACERRCode = CONTROL_INVALID_CONTROL;
        break;
    }
}



void usb_ac_ase_cs_out(void)
{
    MMP_UBYTE cs,len;
    MMP_ULONG tmp3;
   AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;   

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    len = /*UsbRequestPayload_wLength*/gsUsbCtrlReq.wLength ;

    switch(cs) {
    case ASE_SAMPLING_FREQ_CONTROL:
        // under coding
        USBOutData = 0;           
        tmp3 = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
        tmp3 |= pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B << 8;
        tmp3 |= pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B << 16;
        if(tmp3 > SAM_MAX || tmp3 < SAM_MIN) {
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbACERRCode = CONTROL_OUT_OF_RANGE;                    
            break;
        } else {
            //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
            glSamplingFreq = tmp3;
            USB_AudioSetSampleRate(PCAM_API,glSamplingFreq) ;
            // to do here         
        }
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;
    case ASE_CONTROL_UNDEFINED:
    case ASE_PITCH_CONTROL:
    default:
        // impossible flow       
        break;
    }
}

VC_CMD_CFG FU_MUTE_CFG =
{
    (CAP_SET_CUR_CMD | CAP_GET_CUR_CMD ),
    0, // no GET_INFO
    1,0,
    MUTE_RES,MUTE_DEF,MUTE_MIN,MUTE_MAX,MUTE_DEF 
} ;

VC_CMD_CFG FU_VOL_CFG =
{
   (CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD),
    0, // no GET_INFO
    2,0,
    VOL_RES,VOL_DEF,VOL_MIN,VOL_MAX,VOL_DEF 
} ;


void usb_ac_fu_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs;
    cs = (gsUsbCtrlReq.wValue >> 8);
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    switch(cs) {
    case FU_MUTE_CONTROL:
        usb_vc_cmd_cfg(req,&FU_MUTE_CFG,gbMute);
        break;
     
    case FU_VOLUME_CONTROL:
        usb_vc_cmd_cfg(req,&FU_VOL_CFG,gsVol);
        break;
    case FU_CONTROL_UNDEFINED:
    case FU_BASS_CONTROL:
    case FU_MID_CONTROL:
    case FU_TREBLE_CONTROL:
    case FU_GRAPHIC_EQUALIZER_CONTROL:
    case FU_AUTOMATIC_GAIN_CONTROL:
    case FU_DELAY_CONTROL:
    case FU_BASS_BOOST_CONTROL:
    case FU_LOUDNESS_CONTROL:
    default:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbACERRCode = CONTROL_INVALID_CONTROL;
        break;
    }
}

void usb_ac_fu_cs_out(void)
{
    MMP_UBYTE cs,tmp,len,channel;
    MMP_SHORT tmp2;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;  


    cs = (gsUsbCtrlReq.wValue >> 8);
    len = gsUsbCtrlReq.wLength;
    // ooxx
    channel = gsUsbCtrlReq.wValue & 0xFF ;
    switch(cs) {
    case FU_MUTE_CONTROL:
        // under coding
        USBOutData = 0;           
        tmp = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
        if(tmp > 1) {
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbACERRCode = CONTROL_OUT_OF_RANGE;                    
            RTNA_DBG_Str(0, " ** FU_MUTE_CONTROL stall\r\n"); 
            break;
        } else {
            gbMute = tmp;
          //  RTNA_DBG_Str(0, " ** FU_MUTE_CTL:"); 
          //  RTNA_DBG_Byte(0, gbMute);     
         //   RTNA_DBG_Str(0, " **\r\n"); 
            // to do here   
            USB_AudioSetMute(PCAM_OVERWR, gbMute);     
        }
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;
    case FU_VOLUME_CONTROL:
        // under coding
        USBOutData = 0;           
        tmp2 = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
        tmp2 |= pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B << 8;
      //  RTNA_DBG_Str(0, " ** VOL :"); 
      //  RTNA_DBG_Short(0, tmp2);     
     //   RTNA_DBG_Str(0, " **\r\n"); 
        if(tmp2==(short)VOL_MUTE) {
            gsVol = tmp2;
            //RTNA_DBG_Str3("Mute\r\n") ;
            USB_AudioSetVolume(PCAM_OVERWR/*PCAM_API*/, gsVol); // -> Use over write method
        } else {
        
            if(  ((short) tmp2) > (short)VOL_MAX || ((short) tmp2) < (short)VOL_MIN ) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbACERRCode = CONTROL_OUT_OF_RANGE;                    
                RTNA_DBG_Str(0, " ** FU_VOLUME_CONTROL stall\r\n"); 
                break;
            } else {
                gsVol = tmp2;
                //dbg_printf(0,"gsVol:%x\r\n",gsVol);
                // to do here
                USB_AudioSetVolume(PCAM_OVERWR, gsVol);  
            }
        }
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;
    case FU_CONTROL_UNDEFINED:
    case FU_BASS_CONTROL:
    case FU_MID_CONTROL:
    case FU_TREBLE_CONTROL:
    case FU_GRAPHIC_EQUALIZER_CONTROL:
    case FU_AUTOMATIC_GAIN_CONTROL:
    case FU_DELAY_CONTROL:
    case FU_BASS_BOOST_CONTROL:
    case FU_LOUDNESS_CONTROL:
    default:
        // impossible flow       
        break;
    }
}

void usb_ac_req_proc(MMP_UBYTE req)
{
    MMP_UBYTE id;

    id = (/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex >> 8);

    if(id == 0x00) {
        if((/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xFF) == gbACASEP) {
            usb_ac_ase_cs(req);
        } else {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbACERRCode = CONTROL_INVALID_UNIT;
        }
    } else if(id == gbACITID) {
    } else if(id == gbACOTID) {
    } else if(id == gbACFUID) {
        usb_ac_fu_cs(req);
    } else {
        UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
        gbACERRCode = CONTROL_INVALID_UNIT;
    }
}

void usb_ac_out_data(void)
{
    MMP_UBYTE id;

    id = (/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex >> 8);

    if(id == 0x00) {
        if((/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xFF) == gbACASEP) {
            usb_ac_ase_cs_out();
        } else {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbACERRCode = CONTROL_INVALID_UNIT;
        }
    } else if(id == gbACITID) {
    } else if(id == gbACOTID) {
    } else if(id == gbACFUID) {
        usb_ac_fu_cs_out();
    } else {
        UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
        gbACERRCode = CONTROL_INVALID_UNIT;
    }
}

#endif