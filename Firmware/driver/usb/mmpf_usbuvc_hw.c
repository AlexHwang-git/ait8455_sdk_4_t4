#include "includes_fw.h"
#include "lib_retina.h"
#include "config_fw.h"
#include "mmp_reg_usb.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbuvc_hw.h"

void USB_FLUSH_EP0_DATA(void)
{
    MMP_USHORT csr ;
    csr = UsbReadTxEpCSR(0);
    UsbWriteTxEpCSR(0,csr | EP0_FLUSHFIFO_BIT) ;
}
void USB_WAIT_EP0_READY(void)
{
    MMP_USHORT csr ;
    do {
        csr = UsbReadTxEpCSR(0);
        if(csr & TX_FIFO_NOTEMPTY_BIT) {
            //dbg_printf(0,"F,");
        }
    } while ( csr & TX_FIFO_NOTEMPTY_BIT ) ;
}

void USB_PUT_EP0_DATA(MMP_UBYTE req,MMP_USHORT data_len,MMP_UBYTE *data)
{
    USB_PUT_FIFO_DATA(req,0,data_len,data,1);
}

void USB_PUT_FIFO_DATA(MMP_UBYTE req,MMP_UBYTE ep,MMP_USHORT data_len ,MMP_UBYTE *data,MMP_UBYTE unit)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_USHORT i;
    if(ep==MMPF_USB_EP0) {
        USBInPhase = req;           
        USBEP0TxCnt = data_len;//unit;
        USB_WAIT_EP0_READY();
    }
    if(unit==1) {
        if(data) {
            for(i=0;i<data_len;i++) {
                pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = data[i]; 
            }
        } else {
            for(i=0;i<data_len;i++) {
                pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = 0 ;
            }
        
        }
    } 
    else if(unit==2) {
        MMP_USHORT *data16 = (MMP_USHORT *)data ;
        if(data) {
            for(i=0;i<data_len/2;i++) {
                pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = data16[i] & 0xFF;
                pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = (data16[i] >> 8 ) & 0xFF; 
            }
        } else {
            for(i=0;i<data_len/2;i++) {
                pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = 0 ;
                pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = 0 ;
            }
        
        }
    } 
    else {
        dbg_printf(3,"put fifo unit:NG\r\n");
    }
    
    if(ep==MMPF_USB_EP0) {
        usb_ep0_in();
    }
}

void USB_GET_FIFO_DATA(MMP_UBYTE ep,MMP_USHORT fifo_len,MMP_UBYTE *buf,MMP_USHORT buf_len ) 
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
 
    MMP_USHORT i , rd_len = 0,skip_len = 0 ;
 
    MMP_UBYTE skip ;
       
    if (fifo_len > buf_len ) {
        rd_len = buf_len ;
        skip_len = fifo_len - rd_len ;
    } else {
        rd_len = fifo_len ;
    }
    
    
    // Clean buf
    for(i=0;i<buf_len;i++ ) {
        buf[i] = 0 ;
    }
    
    // Read valid data into buf
    for(i=0;i < rd_len ; i++) {
        buf[i] = pUSB_CTL->USB_FIFO_EP[ep].FIFO_B;
    }
    
    // Skip 
    for(i=0;i < skip_len ; i++) {
        skip = pUSB_CTL->USB_FIFO_EP[ep].FIFO_B;
    }
    
}

#pragma arm section code = "usbfifo_copy", rwdata = "usbfifo_copy",  zidata = "usbfifo_copy"
void usb_vc_fifo_copy(MMP_UBYTE ep_id,MMP_ULONG src_addr,MMP_USHORT xsize)
{
    MMP_USHORT r,d,i;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    
    //volatile MMP_ULONG  *USB_REG_BASE_L = (volatile MMP_ULONG  *)0x8000a800;   
    //volatile MMP_UBYTE  *USB_REG_BASE_B = (volatile MMP_UBYTE  *)0x8000a800; 
    
    MMP_ULONG  *ptr32  = (MMP_ULONG  *)src_addr ; 
    MMP_UBYTE  *ptr8   = (MMP_UBYTE  *)src_addr ; 
    MMP_USHORT ep;
   // int i;
    if(ep_id==0) {
        ep = MMPF_USB_EP1 ;
    } 
    else if(ep_id==1){
        ep = MMPF_USB_EP2 ;
    }
    else {
        ep = MMPF_USB_EP0 ;
    }
    
    r = ( src_addr & 3 );
    
	if(r) {
		for(i = 0 ;i < 4 - r;i++) {
            //USB_REG_BASE_B[fifoaddr8] = *ptr8++ ;
            pUSB_CTL->USB_FIFO_EP[ep].FIFO_B =  *ptr8++ ; 
		}
		xsize-= (4-r);
	}
    ptr32 = (MMP_ULONG *)ptr8 ;
    
    d = xsize >> 2 ;
    r = xsize &  3 ;
    
    ptr8 = ptr8 + ( d << 2);
    
    for(i=0;i<d;i++) {
       // USB_REG_BASE_L[fifoaddr32] = ptr32[i] ;
        pUSB_CTL->USB_FIFO_EP[ep].FIFO_D =  ptr32[i] ; 
    }  
    
    for(i=0;i<r;i++) {
        pUSB_CTL->USB_FIFO_EP[ep].FIFO_B  = ptr8[i] ;
    }  

}



MMP_USHORT usb_vc_get_sof_ticks(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    return (pUSB_CTL->USB_FRAME_NUM & 0x7FF);
}