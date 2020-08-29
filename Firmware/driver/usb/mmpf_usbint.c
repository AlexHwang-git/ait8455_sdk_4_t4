#define exUSBINT

#include    "config_fw.h"
#include    "includes_fw.h"
#if defined(USB_FW)||(USB_EN)
#include    "lib_retina.h"
#include    "mmpf_usbvend.h"
#include    "mmpf_usbextrn.h"
#include    "mmpf_usbpccam.h"
#include "mmps_3gprecd.h"
#include "mmpf_usbuac.h"
#include "mmpf_usbuvc_vs.h"
#include "mmpf_usbuvc.h"
#include "mmpf_msg.h"
#include "mmp_reg_usb.h"
#include "mmpf_sensor.h"
#if SUPPORT_PCSYNC
#include "mmpf_usbpcsync.h"
#endif
/** @addtogroup MMPF_USB
@{
*/



//MMP_ULONG   glUsbCtlState;
//MMP_ULONG   glTestModeFeature;
MMP_UBYTE   TestPacket[53] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,
0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,
0xFE,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x7F,0xBF,0xDF,0xEF,0xF7,0xFB,0xFD,0xFC,0x7E,0xBF,0xDF
,0xEF,0xF7,0xFB,0xFD,0x7E};

//USB_REQUEST_PAYLOAD UsbRequestPayload;


extern MMP_ULONG USBFirmwareBinSize;
extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;

#if (SUPPORT_UVC_FUNC==1)
static void UsbUVCTxIntHandler(MMP_UBYTE ep_id) ;
static void usb_class_req_proc(MMP_UBYTE req,MMP_BOOL no_ac);


/*
MMP_UBYTE UsbRequestPayload_bmRequestType;
MMP_UBYTE UsbRequestPayload_bRequest;
MMP_USHORT UsbRequestPayload_wValue;
MMP_USHORT UsbRequestPayload_wIndex;
MMP_USHORT UsbRequestPayload_wLength;
*/
UsbCtrlReq_t gsUsbCtrlReq ;


MMP_UBYTE  gbUsbHighSpeed;
MMP_UBYTE  USBOutData;

MMP_UBYTE gbEP0NotEnd;

//#ifdef UVC_TEST
extern volatile MMP_ULONG SOFNUM;
extern volatile MMP_ULONG uSOFNUM;
//volatile MMP_UBYTE gbdrawflag;
//extern volatile MMP_ULONG STC;
extern MMP_ULONG glCPUFreq;
extern MMPF_OS_FLAGID   SYS_Flag_Hif;
//extern volatile MMP_UBYTE usb_close_Tx_flag;
//extern MMP_UBYTE usb_preclose_Tx_flag;

volatile MMP_UBYTE gbUSBSuspendFlag = 0;
volatile MMP_UBYTE gbUSBSuspendEvent = 0;
#if USB_SUSPEND_TEST

extern void USB_SuspendProcess(void);
#endif

//#endif
#endif


#endif //#if (CHIP == PV2)

#pragma O0

#if (CHIP == P_V2) || (CHIP==VSN_V2)||(CHIP == VSN_V3)

void StallTx(u_char endpoint)
{
    MMP_USHORT csr;
    csr = UsbReadTxEpCSR(endpoint);
    csr = csr & TXCSR_RW_MASK;
    UsbWriteTxEpCSR(endpoint, csr | SET_TX_SENDSTALL);
}


void StallRx(u_char endpoint)
{
    MMP_USHORT csr;
    csr = UsbReadRxEpCSR(endpoint);
    csr = csr & RXCSR_RW_MASK;
    UsbWriteRxEpCSR(endpoint, csr | SET_RX_SENDSTALL);

}

//------------------------------------------------------------------------------
//  Function    : UsbWriteTxMaxP
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbWriteTxMaxP(MMP_ULONG endpoint, MMP_ULONG size)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    pUSB_CTL->USB_EP[endpoint].USB_EP_TX_MAXP = size;
}
//------------------------------------------------------------------------------
//  Function    : UsbWriteRxMaxP
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbWriteRxMaxP(MMP_ULONG endpoint, MMP_ULONG size)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    pUSB_CTL->USB_EP[endpoint].USB_EP_RX_MAXP = size;
	
}
//------------------------------------------------------------------------------
//  Function    : UsbReadEp0COUNT
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT UsbReadEp0COUNT(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_USHORT tmp = 0;
    tmp = (pUSB_CTL->USB_EP[0x0].USB_EP_COUNT & 0x7F);
    return tmp;
}
//------------------------------------------------------------------------------
//  Function    : UsbReadRxEpCOUNT
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT UsbReadRxEpCOUNT(MMP_ULONG endpoint)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_USHORT tmp = 0;
  //  pUSB_CTL->USB_INDEX_EP_SEL = endpoint;
    tmp = (pUSB_CTL->USB_EP[endpoint].USB_EP_COUNT & 0x1FFF);
    return tmp;
}
//------------------------------------------------------------------------------
//  Function    : UsbReadEp0CSR
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT UsbReadEp0CSR(void)
{
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	return pUSB_CTL->USB_EP[0x0].USB_EP_TX_CSR;
}
//------------------------------------------------------------------------------
//  Function    : UsbReadTxEpCSR
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT UsbReadTxEpCSR(MMP_ULONG endpoint)
{
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	return pUSB_CTL->USB_EP[endpoint].USB_EP_TX_CSR;
}
//------------------------------------------------------------------------------
//  Function    : UsbReadRxEpCSR
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT UsbReadRxEpCSR(MMP_ULONG endpoint)
{
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	return pUSB_CTL->USB_EP[endpoint].USB_EP_RX_CSR;
}
//------------------------------------------------------------------------------
//  Function    : UsbWriteEp0CSR
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbWriteEp0CSR(MMP_USHORT csr)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    pUSB_CTL->USB_EP[0x0].USB_EP_TX_CSR = csr;	
}
//------------------------------------------------------------------------------
//  Function    : UsbWriteTxEpCSR
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbWriteTxEpCSR(MMP_ULONG endpoint, MMP_USHORT csr)
{
   	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	pUSB_CTL->USB_EP[endpoint].USB_EP_TX_CSR = csr ;
 }
//------------------------------------------------------------------------------
//  Function    : UsbWriteRxEpCSR
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbWriteRxEpCSR(MMP_ULONG endpoint, MMP_USHORT csr)
{
 	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	pUSB_CTL->USB_EP[endpoint].USB_EP_RX_CSR = csr;	
}

#if PCAM_EN==0
//------------------------------------------------------------------------------
//  Function    : UsbDmaRxConfig
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbDmaRxConfig(MMP_ULONG endpoint, MMP_ULONG fb_addr, MMP_USHORT pkt_byte, MMP_USHORT last_pkt_byte,
                    MMP_ULONG pkt_sum)
{
  	RTNA_DBG_Str(0, "Call UsbDmaRxConfig....!!!\r\n");
   #if 0
   	AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    // ooxx842 add 0x02
    //pUSB_DMA->USB_DMA_CTL = 0x02;

    pUSB_DMA->USB_DMA_FB_ST_ADDR = fb_addr;

    pUSB_DMA->USB_DMA_PKT_BYTE = pkt_byte;
    pUSB_DMA->USB_DMA_PKT_BYTE_LAST = last_pkt_byte;
    pUSB_DMA->USB_DMA_PKT_SUM = pkt_sum;

    pUSB_DMA->USB_DMA_TAR_AND_VAL = 0xF820FFFF;
    pUSB_DMA->USB_DMA_TAR_OR_VAL = 0x01440000;

    pUSB_DMA->USB_DMA_INT_EN = 0x1;

    if(endpoint == 1) {
        pUSB_DMA->USB_DMA_FIFO_ADDR = 0xA824;
        pUSB_DMA->USB_DMA_CMD_ADDR = 0xA914;

        pUSB_DMA->USB_DMA_CTL = 0x25;
    }
    if(endpoint == 2) {
        pUSB_DMA->USB_DMA_FIFO_ADDR = 0xA828;
        pUSB_DMA->USB_DMA_CMD_ADDR = 0xA924;

        pUSB_DMA->USB_DMA_CTL = 0x45;
    }
   #endif
}
#endif
#if (CHIP==VSN_V2)
    static MMP_UBYTE epid_map_epaddr[] = { PCCAM_TX_EP1_ADDR , PCCAM_TX_EP1_ADDR} ;
#endif
#if (CHIP==VSN_V3)
    static MMP_UBYTE epid_map_epaddr[] = { PCCAM_TX_EP1_ADDR , PCCAM_TX_EP2_ADDR } ;
#endif

MMP_UBYTE usb_epaddr_link_epid(MMP_UBYTE ep_addr)
{
    MMP_UBYTE epid = 0;
    if( VIDEO_EP_END()>=2 ) {
        for(epid = 0 ; epid < sizeof(epid_map_epaddr) ;epid++ ) {
            if ( ep_addr == epid_map_epaddr[epid] ) {
                return epid ;
            } 
        }
    }
    
    //epid = 0;
    if(epid > 1) {
        dbg_printf(3,"@Bad epid : %d,%x\r\n",epid,ep_addr);
        while(1);
    } 
    return epid;
}

MMP_UBYTE usb_epid_link_epaddr(MMP_UBYTE ep_id)
{

    if(ep_id >= sizeof(epid_map_epaddr)) {
        return 0;
    }
    return epid_map_epaddr[ep_id] ;

}


MMP_UBYTE usb_epid_link_dmaid(MMP_UBYTE ep_id)
{
#if (CHIP==VSN_V2)
    static MMP_UBYTE ep_dma[] = { 0 , 0 } ;
#endif
#if (CHIP==VSN_V3)
    static MMP_UBYTE ep_dma[] = { 0 , 1 } ;
#endif
    if(ep_id >= sizeof(ep_dma)) {
        return 0;
    }
    return ep_dma[ep_id] ;
}

//    USB_DMA_DESC *pUsbDmaDesc;
MMP_USHORT MMPF_SYS_PostMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);

#pragma arm section code = "itcm", rwdata = "itcm",  zidata = "itcm"
void usb_dma_setlist(MMP_USHORT usSize, MMP_ULONG ulAddr, MMP_ULONG ulFbAddr, MMP_USHORT usPara)
{
    USB_DMA_DESC *pUsbDmaDesc;
    pUsbDmaDesc=(USB_DMA_DESC *)ulAddr;

    if(usPara&LIST_LAST)    
       pUsbDmaDesc->dwNextDescAddr=0;
    else
       pUsbDmaDesc->dwNextDescAddr=ulAddr + USBDMA_LIST_LEN;    
    
    pUsbDmaDesc->dwPLDAddr=ulFbAddr;
    pUsbDmaDesc->wPara=usPara;
    pUsbDmaDesc->wPLDSize=usSize-1; 
    
    #if 0
    RTNA_DBG_PrintLong(0, ulAddr);
    RTNA_DBG_PrintLong(0, pUsbDmaDesc->dwNextDescAddr);
    RTNA_DBG_PrintLong(0, pUsbDmaDesc->dwPLDAddr);
    RTNA_DBG_PrintShort(0, pUsbDmaDesc->wPara);
    RTNA_DBG_PrintShort(0, pUsbDmaDesc->wPLDSize);
    #endif
}
#pragma arm section code, rwdata,  zidata

MMP_UBYTE gbUsbDmaRun;


void usb_dma_en(MMP_ULONG ulAddr,MMP_UBYTE ubEnableInt, MMP_UBYTE ep_id,MMP_USHORT des_cnt)
{
#define NO_WAIT_IN_TOKEN    (0) // wait in - token 
	AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
	MMP_UBYTE dma_id = usb_epid_link_dmaid(ep_id);
	MMP_UBYTE ep_addr = usb_epid_link_epaddr(ep_id);
	
#if 1

    if(ep_id==0 ) {
        MMP_USHORT dcnt = pUSB_DMA->USB_DMA1_DESC_CNT ;
        MMP_USHORT pcnt = pUSB_DMA->USB_DMA1_PKT_BYTE_CNT ;
        
        if(dcnt) {
            dbg_printf(3,"<EP1d : %d>\r\n",dcnt);
        }
        if(pcnt) {
            dbg_printf(3,"<EP1p : %d>\r\n",pcnt );
        }
    }
    else {
        MMP_USHORT dcnt = pUSB_DMA->USB_DMA2_DESC_CNT ;
        MMP_USHORT pcnt = pUSB_DMA->USB_DMA2_PKT_BYTE_CNT ;
        if(dcnt) {
            dbg_printf(3,"<EP2d : %d>\r\n",dcnt );
        }
        if(pcnt) {
            dbg_printf(3,"<EP2p : %d>\r\n",pcnt);
        }
    
    }
#endif	
#if (CHIP == VSN_V2)
	pUSB_DMA->USB_DMA_FB_ST_ADDR = ulAddr;
	pUSB_DMA->USB_DMA_TAR_AND_VAL = 0xFF10FFFF;
	pUSB_DMA->USB_DMA_TAR_OR_VAL = 0x00A70000;
	pUSB_DMA->USB_DMA_CMD_ADDR = 0x0100|(ep_addr<<4); 
#endif
#if (CHIP == VSN_V3)
    if(dma_id==0) {
	    pUSB_DMA->USB_DMA1_FB_ST_ADDR = ulAddr;
	    pUSB_DMA->USB_DMA1_TAR_AND_VAL = 0xFF10FFFF;
	    pUSB_DMA->USB_DMA1_TAR_OR_VAL = 0x00A70000;
	    pUSB_DMA->USB_DMA1_CMD_ADDR = 0x0100|(ep_addr<<4);
	    
	    pUSB_DMA->USB_DMA1_DESC_CNT_NUM = 0;//des_cnt - 1 ;
	} else {
	    pUSB_DMA->USB_DMA2_FB_ST_ADDR = ulAddr;
	    pUSB_DMA->USB_DMA2_TAR_AND_VAL = 0xFF10FFFF;
	    pUSB_DMA->USB_DMA2_TAR_OR_VAL = 0x00A70000;
	    pUSB_DMA->USB_DMA2_CMD_ADDR = 0x0100|(ep_addr<<4);
	    
	    pUSB_DMA->USB_DMA2_DESC_CNT_NUM = 0;//des_cnt - 1 ;
	}
#endif

	if(ubEnableInt == MMP_TRUE) { 
	    if(dma_id==0) {
		    pUSB_DMA->USB_DMA_INT_EN |= (USB_INT_DMA1_DONE_EN  ) ;
		    /*
		    if(UVC_VCD()==bcdVCD15) {
		        pUSB_DMA->USB_DMA_INT_EN |= (USB_INT_DMA1_DESC_CNT_EN  ) ;
		    } 
		    else {
		        pUSB_DMA->USB_DMA_INT_EN &= ~(USB_INT_DMA1_DESC_CNT_EN  ) ;
		    }
		    */
		}
		#if (CHIP==VSN_V3)
		else {
		    pUSB_DMA->USB_DMA_INT_EN |= (USB_INT_DMA2_DONE_EN );
		    /*
		    if(UVC_VCD()==bcdVCD15) {
		        pUSB_DMA->USB_DMA_INT_EN |= (USB_INT_DMA2_DESC_CNT_EN  ) ;
		    } 
		    else {
		        pUSB_DMA->USB_DMA_INT_EN &= ~(USB_INT_DMA2_DESC_CNT_EN  ) ;
		    }
		    */
		}
		#endif
	}
	#if (CHIP == VSN_V2)
	pUSB_DMA->USB_DMA_CTL = 0x01;
	#endif
	#if (CHIP == VSN_V3)
	if(dma_id==0) {
	    //pUSB_DMA->USB_DMA1_CTL2 = NO_WAIT_IN_TOKEN;
	    pUSB_DMA->USB_DMA1_CTL1 = 0x01;
	} 
	else if(dma_id==1){
	    //pUSB_DMA->USB_DMA2_CTL2 = NO_WAIT_IN_TOKEN;
	    pUSB_DMA->USB_DMA2_CTL1 = 0x01;
	}
	else {
	    dbg_printf(3,"Bad DMA Id : %d\r\n",dma_id );
	}
	#endif
	
	if(UVC_VCD()==bcdVCD15) {
	    usb_dma_wait_start(ep_id);
	}
	//dbg_printf(3,"list cmd :%d,int en:%x\r\n",des_cnt,pUSB_DMA->USB_DMA_INT_EN);
}

void usb_dma_wait_start(MMP_UBYTE ep_id)
{
#define WAIT_DMA_START_TIMEOUT (2000)
    int cnt = 0 ;
    STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    ss->tx_flag &= ~SS_TX_1ST_PKT_START;
    if(ep_id==0) {
        while(pUSB_DMA->USB_DMA1_PKT_BYTE_CNT) {
            cnt++;//dbg_printf(3,"=");
            if(cnt >= WAIT_DMA_START_TIMEOUT ) {
                dbg_printf(3,"-DMA0 START NG-\r\n");
                break;
            }
        }
    }
    else {
        while(pUSB_DMA->USB_DMA2_PKT_BYTE_CNT) {
            cnt++;
            if(cnt >= WAIT_DMA_START_TIMEOUT ) {
                dbg_printf(3,"-DMA1 START NG-\r\n");
                break;
            }
        }
    }
    ss->tx_flag |= SS_TX_1ST_PKT_START;
}


MMP_BOOL usb_dma_busy(MMP_UBYTE ep_id)
{
    STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(ep_id);
	AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
	
	if(ss->tx_flag & SS_TX_1ST_PKT_DONE) {
	    dbg_printf(3,"-1STDONE-\r\n");
        if(ep_id==0) {
            if( (pUSB_DMA->USB_DMA1_DESC_CNT) || (pUSB_DMA->USB_DMA1_PKT_BYTE_CNT) ) {
                dbg_printf(3,"#");
                return TRUE; 
            }
        }
        else {
            if( (pUSB_DMA->USB_DMA2_DESC_CNT) || (pUSB_DMA->USB_DMA2_PKT_BYTE_CNT) ) {
                dbg_printf(3,"@");
                return TRUE; 
            }
        }
        //dbg_printf(3,"$");
    } 
    else if(ss->tx_flag & SS_TX_1ST_PKT_START) {
        //dbg_printf(3,"-1STSTART-\r\n");
        if(ep_id==0) {
            if( (pUSB_DMA->USB_DMA1_DESC_CNT) || (pUSB_DMA->USB_DMA1_PKT_BYTE_CNT) ) {
                dbg_printf(3,"#");
                return TRUE; 
            }
        }
        else {
            if( (pUSB_DMA->USB_DMA2_DESC_CNT) || (pUSB_DMA->USB_DMA2_PKT_BYTE_CNT) ) {
                dbg_printf(3,"@");
                return TRUE; 
            }
        }
   }
    return FALSE ;
}

#if USB_UVC_BULK_EP==1
//------------------------------------------------------------------------------
//  Function    : UsbDmaTxConfig
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbDmaTxConfig(MMP_ULONG endpoint, MMP_ULONG fb_addr, MMP_USHORT pkt_byte, MMP_USHORT last_pkt_byte,
                    MMP_ULONG pkt_sum, MMP_ULONG zero_end)
{
	MMP_UBYTE i = 0x0;
	MMP_UBYTE *ptr = (MMP_UBYTE*)fb_addr;
	MMP_USHORT DmaTxSize;

#if 1
	for(i = 0x0; i < pkt_sum; i++) {
		usb_dma_setlist((pkt_byte+1), USB_DESC_START_ADDR + i*USBDMA_LIST_LEN, fb_addr + i*(pkt_byte + 1),
                           LIST_TXPKTRDY|LIST_INDEX|endpoint);
    }
     
    usb_dma_setlist((last_pkt_byte + 1), USB_DESC_START_ADDR + i*USBDMA_LIST_LEN, fb_addr + i*(pkt_byte + 1),
                             LIST_TXPKTRDY|LIST_LAST|LIST_INDEX|endpoint);
#else

    DmaTxSize = (MMP_USHORT)((pkt_sum * (pkt_byte + 1)) + (last_pkt_byte + 1));
    usb_dma_setlist(DmaTxSize, USB_DESC_START_ADDR, fb_addr, LIST_TXPKTRDY|LIST_LAST|LIST_INDEX|endpoint);

#endif
                            
    usb_dma_en(USB_DESC_START_ADDR, USB_DMA_DONE, endpoint);


}
#endif

//------------------------------------------------------------------------------
//  Function    : UsbDmaStop
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbDmaStop(MMP_UBYTE ep_id)
{
	#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr = 0;
	#endif
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_USHORT csr;
    
    STREAM_SESSION *ss;
    MMP_UBYTE ep,dma_id;
    if(ep_id < VIDEO_EP_END()) {
        ss=MMPF_Video_GetStreamSessionByEp(ep_id);
        ep = usb_epid_link_epaddr(ep_id);
        dma_id = usb_epid_link_dmaid(ep_id);
        dbg_printf(3,"#<DMA stop,id : %d\r\n",dma_id);
        IRQ_LOCK(
            if(dma_id==0) {
                pUSB_DMA->USB_DMA1_CTL1 = 0x02;
            } else {
                pUSB_DMA->USB_DMA2_CTL1 = 0x02;
            }    
        )

    	if((pUSB_CTL->USB_EP[ep].USB_EP_TX_CSR & 0x2) != 0x0) {
    		RTNA_DBG_Str(0, "\r\nC1");
    		IRQ_LOCK(
        		csr = UsbReadTxEpCSR(ep);
        	    csr = csr & TXCSR_RW_MASK;
        	    UsbWriteTxEpCSR(ep, csr | SET_TX_FLUSHFIFO);
    	    )
    	}
    	RTNA_WAIT_US(100);
    	if((pUSB_CTL->USB_EP[ep].USB_EP_TX_CSR & 0x2) != 0x0) {
    		RTNA_DBG_Str(0, "-C2");
    		IRQ_LOCK(
        		csr = UsbReadTxEpCSR(ep);
        	    csr = csr & TXCSR_RW_MASK;
        	    UsbWriteTxEpCSR(ep, csr | SET_TX_FLUSHFIFO);
    	    )
    	}
    	RTNA_WAIT_US(100);
    	if((pUSB_CTL->USB_EP[ep].USB_EP_TX_CSR & 0x2) != 0x0) {
    		RTNA_DBG_Str(0, "-C3");
    		IRQ_LOCK(
        		csr = UsbReadTxEpCSR(ep);
        	    csr = csr & TXCSR_RW_MASK;
        	    UsbWriteTxEpCSR(ep, csr | SET_TX_FLUSHFIFO);
    	    )
    	}
    	
    	RTNA_WAIT_US(100);
    	if((pUSB_CTL->USB_EP[ep].USB_EP_TX_CSR & 0x2) != 0x0) {
    		RTNA_DBG_Str(0, "-C4");
    		IRQ_LOCK(
        		csr = UsbReadTxEpCSR(ep);
        	    csr = csr & TXCSR_RW_MASK;
        	    UsbWriteTxEpCSR(ep, csr | SET_TX_FLUSHFIFO);
    	    )
    	}

        IRQ_LOCK(
            
            if(dma_id==0) {
                pUSB_DMA->USB_DMA_INT_EN &= ~( USB_INT_DMA1_DONE_EN | USB_INT_DMA1_DESC_CNT_EN) ;
                pUSB_DMA->USB_DMA1_CTL1 = 0x00;
                pUSB_DMA->USB_DMA1_FB_ST_ADDR = 0;
                pUSB_DMA->USB_DMA1_DESC_CNT_NUM = 0;
                pUSB_DMA->USB_DMA1_DESC_CNT = 0;
            } else {
                pUSB_DMA->USB_DMA_INT_EN &= ~( USB_INT_DMA2_DONE_EN | USB_INT_DMA2_DESC_CNT_EN) ;
                pUSB_DMA->USB_DMA2_CTL1 = 0x00;
                pUSB_DMA->USB_DMA2_FB_ST_ADDR = 0;
                pUSB_DMA->USB_DMA2_DESC_CNT_NUM = 0;
                pUSB_DMA->USB_DMA2_DESC_CNT = 0;
            }    
        )
        //dbg_printf(0,"--EP[%d].CSR : %x\r\n",ep,pUSB_CTL->USB_EP[ep].USB_EP_TX_CSR);
        //dbg_printf(3,"-pUSB_DMA->USB_DMA1_DESC_CNT : %d\r\n",pUSB_DMA->USB_DMA1_DESC_CNT);
        //dbg_printf(3,"-pUSB_DMA->USB_DMA1_DESC_CNT_NUM : %d\r\n",pUSB_DMA->USB_DMA1_DESC_CNT_NUM);
    }
}

#pragma arm section code = "itcm", rwdata = "itcm",  zidata = "itcm"
void UsbISODmaListTxInit(MMP_UBYTE ep_id,MMP_ULONG fb_addr1,MMP_ULONG fb_addr2)
{
    volatile USB_DMA_DATA_INFO *data_info;
    data_info = (USB_DMA_DATA_INFO *)(fb_addr1 + USB_DMA_DATA_INFO_OFFSET);
    data_info->dmalist_status = DMALIST_INVALID ;
    data_info->checksum = 0;
    data_info = (USB_DMA_DATA_INFO *)(fb_addr2 + USB_DMA_DATA_INFO_OFFSET);
    data_info->dmalist_status = DMALIST_INVALID ;
    data_info->checksum = 0;
    #if H264_CBR_PADDING_EN==1
    data_info->dma_flag = 0 ;
    #endif    
}

/*
return 
1 : EOF
0 : Continue
*/
MMP_BOOL UsbISODmaListTxConfig(MMP_UBYTE ep_id,MMP_ULONG fb_addr,MMP_ULONG size,MMP_UBYTE flag)
{
    //MMP_UBYTE  *h = (MMP_UBYTE  *)h ;
    
    MMP_BOOL    _1st_pkt = 1 , eopkt = MMP_FALSE,last_blk = MMP_FALSE;
    MMP_LONG tx_size = size ;
    MMP_USHORT tx_header_len =0,list_last = 0, pkt_size, tx_cnt =0,pktrdy_cnt = 0 ;
    volatile USB_DMA_DATA_INFO *data_info = (USB_DMA_DATA_INFO *)(fb_addr + USB_DMA_DATA_INFO_OFFSET);
    MMP_ULONG data_addr = (MMP_ULONG)data_info->data_addr ;
    MMP_UBYTE endpoint = usb_epid_link_epaddr(ep_id);
    STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
    
    if(usb_cur_dma_addr(ep_id)==data_info->dmalist_addr) {
        dbg_printf(3,"* conflict * : %x\r\n",data_info->dmalist_addr);
    }
    
    if(ss->tx_flag & SS_TX_RT_MODE) {
        if(ss->framelength == (MMP_ULONG)ss->cursendptr)    {
            last_blk = MMP_TRUE ;
        }     
    } 
    
    if(tx_size==UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]) {
        usb_dma_setlist(UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id], data_info->dmalist_addr, (MMP_LONG)fb_addr,LIST_TXPKTRDY|LIST_LAST|endpoint);
        pktrdy_cnt = 1 ;
        #if EOF_IN_LAST_DATA_PKT
        if(ss->tx_flag & SS_TX_BY_ONE_SHOT) {
            data_info->dmalist_status = DMALIST_INVALID;
            //dbg_printf(3,"fb_addr : %x,size:%d\r\n",fb_addr,size);
            eopkt = MMP_TRUE ;
        }
        #endif
        //dbg_printf(3,"eof\r\n");
    } else {
    
        MMP_ULONG t1,t2;
        #if EOF_IN_LAST_DATA_PKT
        if( (ss->tx_flag & SS_TX_BY_ONE_SHOT) || last_blk ) {
            if(data_info->dmalist_status==DMALIST_BUSY) {
                eopkt = MMP_TRUE;
                if(data_info->checksum != (data_info->dmalist_addr + data_info->data_addr ) ) {
                    data_info->checksum = 0;
                } else {
                    UVC_VIDEO_PAYLOAD_HEADER *pUVCHeader1 = (UVC_VIDEO_PAYLOAD_HEADER *)(fb_addr ) ;
                    UVC_VIDEO_PAYLOAD_HEADER *pUVCHeader2 = (UVC_VIDEO_PAYLOAD_HEADER *)(fb_addr + UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]) ;
                    *pUVCHeader2 = *pUVCHeader1 ;
                    pUVCHeader2->bBFH |=  BFH_BIT( BFH_EOF ) | BFH_BIT( BFH_EOS );
                    goto _usb_dma_en ;
                }
            }
        }
        #endif
        
       // dbg_printf(3,"good : %d,%x,%x,%x\r\n",data_info->dmalist_status,fb_addr,data_info->dmalist_addr,data_info->data_addr);
        t1 = usb_vc_cur_usof();
        do {
            // 12 bytes UVC header
            tx_header_len = (_1st_pkt)?data_info->data_header_len : 0;
            usb_dma_setlist(UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]+tx_header_len, data_info->dmalist_addr+tx_cnt*USBDMA_LIST_LEN, (MMP_LONG)fb_addr,endpoint);
            tx_cnt++;
            
            
            #if 0
            // size : UVC 12 byte + Data Blk Size( may over packet size)
            if(_1st_pkt) {
                tx_size -= (UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id] +tx_header_len );
            }
            
            if(tx_size > (data_info->max_dsize - tx_header_len ) ) {
                pkt_size = data_info->max_dsize - tx_header_len; // add "-tx_header_len"
            } else {
                // Fixed bug if frame size < 3KB.
                pkt_size = (_1st_pkt)?tx_size : tx_size - tx_header_len ; // add "-tx_header_len"
                list_last = LIST_LAST ;
                //eopkt = MMP_TRUE ;
            }
            #else
            if(_1st_pkt) { 
                tx_size -= (UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id] +tx_header_len );
                if(tx_size > (data_info->max_dsize  - tx_header_len ) ) {
                    pkt_size = data_info->max_dsize - tx_header_len;
                }
                else {
                    pkt_size = tx_size ;
                    list_last = LIST_LAST ;
                }
            }
            else {
                if ( tx_size > data_info->max_dsize ) {
                    pkt_size = data_info->max_dsize ;
                }
                else {
                    pkt_size = tx_size ;
                    list_last = LIST_LAST ;
                }
            }
            #endif
            
            if(data_info->data_addr) {
                usb_dma_setlist(pkt_size, data_info->dmalist_addr+tx_cnt*USBDMA_LIST_LEN, (MMP_LONG)data_addr,LIST_TXPKTRDY|list_last|endpoint);    
                tx_cnt++;
                pktrdy_cnt++ ;
                #if H264_CBR_PADDING_EN==1
                if(data_info->dma_flag & DMA_CF_DUMMY ) {
                    //dbg_printf(3,"data_addr:%x,%d\r\n",data_addr,tx_size);
                }
                else {
                    data_addr += pkt_size ;
                }                
                #else
                data_addr += pkt_size ;
                #endif
                tx_size -= pkt_size ;
                
                #if EOF_IN_LAST_DATA_PKT
                if( (ss->tx_flag & SS_TX_BY_ONE_SHOT) || last_blk ) {
                    if( tx_size <= data_info->max_dsize) {
                        UVC_VIDEO_PAYLOAD_HEADER *pUVCHeader1 = (UVC_VIDEO_PAYLOAD_HEADER *)(fb_addr ) ;
                        UVC_VIDEO_PAYLOAD_HEADER *pUVCHeader2 = (UVC_VIDEO_PAYLOAD_HEADER *)(fb_addr + UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]) ;
                        *pUVCHeader2 = *pUVCHeader1 ;
                        pUVCHeader2->bBFH |=  BFH_BIT( BFH_EOF ) | BFH_BIT( BFH_EOS );
                        fb_addr += UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id] ;
                        eopkt = MMP_TRUE;
                        data_info->dmalist_status = DMALIST_BUSY;
                       // dbg_printf(3,"1st eofpk=true,tx_size:%d\r\n",tx_size);
                    }
                } 
                #endif
            } 
            else {
                while(1){
                    dbg_printf(3,"Bad Data Addr\r\n");
                }
            }
            _1st_pkt = 0;
            
        } while ( tx_size > 0 ) ;
        t2 = usb_vc_diff_usof(t1)  ;
        if(t2) {
            //dbg_printf(3,"[%d]t2-t1:%d\r\n" ,ep_id ,t2);
        }
        
        if(data_info->dmalist_status==DMALIST_BUSY) {
            data_info->checksum = data_info->dmalist_addr + data_info->data_addr ;
        }
    }
_usb_dma_en:   
    if(flag & USB_DMA_EN) {
    	#if OS_CRITICAL_METHOD == 3
        OS_CPU_SR   cpu_sr = 0;
    	#endif
        STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
        MMP_BOOL  abort ;
        
        abort = MMPF_USB_WaitDMA( ep_id );
        //if(ep_id==0)
        //dbg_printf(3,"(%d)ep_id : %d,dmalist:%x\r\n",abort,ep_id,data_info->dmalist_addr);
        if(abort) {
            dbg_printf(3,"***DMA/EP id : %d aborted\r\n",ep_id );
            return MMP_FALSE ;
        }
        //dbg_printf(3,"vs.s:%d\r\n",uSOFNUM );
        IRQ_LOCK(
            ss->tx_flag &= ~SS_TX_1ST_PKT_DONE ;
            usb_dma_en(data_info->dmalist_addr, USB_DMA_DONE, ep_id,pktrdy_cnt);
        )
         
    }
    
    return eopkt;
}
#pragma arm section code, rwdata,  zidata

//------------------------------------------------------------------------------
//  Function    : UsbRestIntHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbRestIntHandler(void)
{
    gsUsbCtrlReq.bDevAddr = 0 ; // clean address
    SOFNUM = 0;
    uSOFNUM = 0;
    clear_ep1_sequence();
    clear_ep2_sequence();
    clear_ep3_sequence();
    // sean@2011_01_03 gbEP0NotEnd=0 in here from Hans suggestion
    gbEP0NotEnd = 0;
    // varible initial
    USBOutData = 0;
    USBInPhase = 0;
    USBOutPhase = 0;
    //#ifdef UVC_MJPEG
    //    USBConfig1Len = 0x0190;
    //#else
    USBConfig1Len = 0;//CONFIG_DESCRIPTOR_LEN;

    usb_ep_init();
    

    gsUsbCtrlReq.wAlterSetting = 0x00;
    gsUsbCtrlReq.wInterface    = 0x00;
    gsUsbCtrlReq.bConfig       = 0x00 ;
    gsUsbCtrlReq.bDevice = 0x00;
    gsUsbCtrlReq.bEndPoint = 0x00;
    gsUsbCtrlReq.wSyncFrame = 0 ;
    uvc_init(1,0);
#if SUPPORT_UAC==1    
    uac_init();
#endif    
    //USBConfigDescLen = USBConfig1Len;
}

#if (SUPPORT_UVC_FUNC==1)

//------------------------------------------------------------------------------
//  Function    : EnableEx
//  Parameter   : None
//  Return Value : None
//  Description : FIFO mode 
//------------------------------------------------------------------------------  
void EnableEx(MMP_UBYTE endpoint)
{
    MMP_USHORT csr;

    csr = UsbReadTxEpCSR(endpoint);
    csr = csr & TXCSR_RW_MASK;
    UsbWriteTxEpCSR(endpoint, csr | SET_TX_TXPKTRDY);
}

//*********************************************************************
// function : usb_error
// input    : none
// output   : none
//*********************************************************************
void usb_error(void)
{
    #if 1
    dbg_printf(3,"[%x]Req.dir : %x,type:%x,rec:%x\r\n",
            gsUsbCtrlReq.bmRequestType.bmReqType,
            gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqDir,
            gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqType,
            gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqRecipient);
    dbg_printf(3,"req : %x,val :%d,indx :%x,len :%x\r\n",
            gsUsbCtrlReq.bRequest,
            gsUsbCtrlReq.wValue,
            gsUsbCtrlReq.wIndex,
            gsUsbCtrlReq.wLength);
    #endif        


    RTNA_DBG_Str(0, "usb_error\r\n");
    UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
}

void class_set_current_value(void)
{
    usb_class_req_proc(SET_CUR_CMD,0);
}    
    
void class_get_current_value(void)
{
    usb_class_req_proc(GET_CUR_CMD,0);
}
        
void class_get_min_value(void)
{
    usb_class_req_proc(GET_MIN_CMD,0);
}
        
void class_get_max_value(void)
{
    usb_class_req_proc(GET_MAX_CMD,0);
} 
       
void class_get_resolution(void)
{
    usb_class_req_proc(GET_RES_CMD,0);
} 
       
void class_get_length(void)
{
    usb_class_req_proc(GET_LEN_CMD,0);
}    
   
void class_get_info(void)
{
    usb_class_req_proc(GET_INFO_CMD,1);
}        

void class_get_default_value(void)
{
    usb_class_req_proc(GET_DEF_CMD,1);
}        

#if 0
// UVC1.5 new cmd.
void class_set_current_all(void)
{
    usb_class_req_proc(SET_CUR_ALL,1);

}

void class_get_current_all(void)
{
    usb_class_req_proc(GET_CUR_ALL,1);
}

void class_get_min_all(void)
{
    usb_class_req_proc(GET_MIN_ALL,1);
}

void class_get_max_all(void)
{
    usb_class_req_proc(GET_MAX_ALL,1);
}

void class_get_res_all(void)
{
    usb_class_req_proc(GET_RES_ALL,1);
}

void class_get_def_all(void)
{
    usb_class_req_proc(GET_DEF_ALL,1);
}
#endif

static void usb_class_req_proc(MMP_UBYTE req,MMP_BOOL no_ac)
{
    if((/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xff)  <= VIDEO_EP_END()) {
        usb_vc_req_proc(req);
    } else {
        if(no_ac==0) {
            usb_ac_req_proc(req);
        }
    }

}

void class_in_set_current_value(void)
{
    if((/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xff)  <= VIDEO_EP_END())
        usb_vc_update_automode();
}

void class_in_get_current_value(void)
{
    USBOutPhase = GET_CUR_CMD;
    usb_ep0_null_out();
}
void class_in_get_min_value(void)
{
    USBOutPhase = GET_MIN_CMD;
    usb_ep0_null_out();
}
void class_in_get_max_value(void)
{
    USBOutPhase = GET_MAX_CMD;
    usb_ep0_null_out();
}
void class_in_get_resolution(void)
{
    USBOutPhase = GET_RES_CMD;
    usb_ep0_null_out();
}
void class_in_get_length(void)
{
    USBOutPhase = GET_LEN_CMD;
    usb_ep0_null_out();
}
void class_in_get_info(void)
{
    USBOutPhase = GET_INFO_CMD;
    usb_ep0_null_out();
}
void class_in_get_default_value(void)
{
    USBOutPhase = GET_DEF_CMD;
    usb_ep0_null_out();
}
void class_out_set_current_value(void)
{
    if((/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xff)  <= VIDEO_EP_END()) {
        usb_vc_out_data();
    }    
    else {
        usb_ac_out_data();
    }    
}
void class_out_get_current_value(void)
{
}
void class_out_get_min_value(void)
{
}
void class_out_get_max_value(void)
{
}
void class_out_get_resolution(void)
{
}
void class_out_get_length(void)
{
}
void class_out_get_info(void)
{
}
void class_out_get_default_value(void)
{
}


//*****************************************************************
// function : class_request_setup
// input    : none
// output   : none
//*****************************************************************
void class_request_setup(void)
{
    MMP_UBYTE tmp; 
    tmp = /*UsbRequestPayload_bRequest*/gsUsbCtrlReq.bRequest;
    if(tmp == SET_CUR_CMD) {
        class_set_current_value();
    } else if(tmp == GET_CUR_CMD) {
        class_get_current_value();
    } else if(tmp == GET_MIN_CMD) {
        class_get_min_value();
    } else if(tmp == GET_MAX_CMD) {
        class_get_max_value();
    } else if(tmp == GET_RES_CMD) {
        class_get_resolution();
    } else if(tmp == GET_LEN_CMD) {
        class_get_length();
    } else if(tmp == GET_INFO_CMD) {
        class_get_info();
    } else if(tmp == GET_DEF_CMD) {
        class_get_default_value();
    } else {
        usb_error();
    }
}

//*****************************************************************
// function : class_request_in
// input    : none
// output   : none
//*****************************************************************
void class_request_in(void)
{
    if(USBCmd == SET_CUR_CMD) {
        class_in_set_current_value();
    } else if(USBCmd == GET_CUR_CMD) {
        class_in_get_current_value();
    } else if(USBCmd == GET_MIN_CMD) {
        class_in_get_min_value();
    } else if(USBCmd == GET_MAX_CMD) {
        class_in_get_max_value();
    } else if(USBCmd == GET_RES_CMD) {
        class_in_get_resolution();
    } else if(USBCmd == GET_LEN_CMD) {
        class_in_get_length();
    } else if(USBCmd == GET_INFO_CMD) {
        class_in_get_info();
    } else if(USBCmd == GET_DEF_CMD) {
        class_in_get_default_value();
    } else {
        usb_error();
    }
}

//*****************************************************************
// function : class_request_out
// input    : none
// output   : none
//*****************************************************************
void class_request_out(void)
{
    if(USBCmd == SET_CUR_CMD) {
        class_out_set_current_value();
    } else if(USBCmd == GET_CUR_CMD) {
        class_out_get_current_value();
    } else if(USBCmd == GET_MIN_CMD) {
        class_out_get_min_value();
    } else if(USBCmd == GET_MAX_CMD) {
        class_out_get_max_value();
    } else if(USBCmd == GET_RES_CMD) {
        class_out_get_resolution();
    } else if(USBCmd == GET_LEN_CMD) {
        class_out_get_length();
    } else if(USBCmd == GET_INFO_CMD) {
        class_out_get_info();
    } else if(USBCmd == GET_DEF_CMD) {
        class_out_get_default_value();
    } else {
        usb_error();
    }
    //    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
}



void usb_ep0_setup_get_req(UsbCtrlReq_t *req)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
     MMP_UBYTE reqtype ;
    req->bmRequestType.bmReqType = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
    req->bRequest = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
    
    req->wValue   = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_W;
    req->wIndex   = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_W;
    req->wLength  = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_W;
    
    #if 0
    reqtype = gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqType;
    if(reqtype==STD_REQ) {
    dbg_printf(3,"[%x]Req.dir : %x,type:%x,rec:%x\r\n",
            req->bmRequestType.bmReqType,
            req->bmRequestType.bmReqTypeBits.bReqDir,
            req->bmRequestType.bmReqTypeBits.bReqType,
            req->bmRequestType.bmReqTypeBits.bReqRecipient);
    dbg_printf(3,"req : %x,val :%d,indx :%x,len :%x\r\n",
            req->bRequest,
            req->wValue,
            req->wIndex,
            req->wLength);
    }
    #endif        
}

//*********************************************************************
// function : usb_ep0_setup_done
// input    : none
// output   : none
//*********************************************************************
void usb_ep0_setup_done(void)
{
    //MMP_UBYTE tmp;
    MMP_UBYTE reqtype ;
    usb_ep0_setup_get_req( &gsUsbCtrlReq );
    #if 1
    reqtype = gsUsbCtrlReq.bmRequestType.bmReqTypeBits.bReqType; 
    if ( reqtype == STD_REQ ) {
        std_request_setup();
    }
    else if( reqtype == CLASS_REQ) {
        class_request_setup();
    }
#if SUPPORT_PCSYNC
    else if( reqtype == VENDOR_REQ) {
        pcsync_vendor_request_setup();     
    }
#endif     
    else {
        usb_error();
    }
    #else
    USBCmd = /*UsbRequestPayload_bmRequestType*/gsUsbCtrlReq.bmRequestType;
    tmp = USBCmd;
    tmp &= 0x1f;
    USBRequestType = tmp;
    USBCmd &= 0x60;
    if(USBCmd == 0) {
        std_request_setup();
    } else if(USBCmd == 0x20) {
        class_request_setup();
    } else {
        usb_error();
    }
    #endif
} 

//*********************************************************************
// function : usb_ep0_in_done
// input    : none
// output   : none
//*********************************************************************
void usb_ep0_in_done(void)
{
    MMP_UBYTE tmp;

    USBCmd = USBInPhase;
    USBInPhase = 0;
    if(USBCmd) {
        tmp = USBCmd;
        tmp &= 0xf0;
        if(tmp == STD_CMD || tmp == 0x70) {
            std_request_in();
        } else if(tmp == CLASS_CMD || tmp == 0x20 || tmp == 0x80 || tmp == 0x00) {
            class_request_in();
        } 
        #if SUPPORT_PCSYNC
        else if ( tmp == VENDOR_CMD) {
            pcsync_vendor_request_data_in();    
        }
        #endif
        else {
            usb_error();
        }
    } else {
        usb_error();
    }
}

//*********************************************************************
// function : usb_ep0_out_done
// input    : none
// output   : none
//*********************************************************************
void usb_ep0_out_done(void)
{
    MMP_UBYTE tmp;

    USBCmd = USBOutPhase;
    USBOutPhase = 0;
    if(USBCmd) {
        tmp = USBCmd;
        tmp &= 0xf0;
        if(tmp == STD_CMD || tmp == 0x70) {
            std_request_out();
        } else if(tmp == CLASS_CMD || tmp == 0x20 || tmp == 0x80 || tmp == 0x00) {
            class_request_out();
        } 
        #if SUPPORT_PCSYNC
        else if ( tmp == VENDOR_CMD) {
            pcsync_vendor_request_data_out();
        }
        #endif
        else {
            usb_error();
        }
    } else {
        usb_error();
    }
}

void clear_ep_sequence(MMP_UBYTE ep)
{
    MMP_USHORT csr;

    csr = UsbReadTxEpCSR(ep);

    if((csr & 0x00FF) != 0) {
        UsbWriteTxEpCSR(ep, csr & 0xFF00);      
        csr = UsbReadTxEpCSR(ep);
    } 

    csr = csr & TXCSR_RW_MASK;
    UsbWriteTxEpCSR(ep, (csr & (~TX_SENDSTALL_BIT)) | SET_TX_CLRDATATOG);
}

//*****************************************************************
// function : clear_ep1_sequence
// input    : none
// output   : none
//*****************************************************************
void clear_ep1_sequence(void)
{
    clear_ep_sequence(1);    
}

//*****************************************************************
// function : clear_ep2_sequence
// input    : none
// output   : none
//*****************************************************************
void clear_ep2_sequence(void)
{
    clear_ep_sequence(2);    
}

//*****************************************************************
// function : clear_ep3_sequence
// input    : none
// output   : none
//*****************************************************************
void clear_ep3_sequence(void)
{
    clear_ep_sequence(3);    
}

//*****************************************************************
// function : clear_ep4_sequence
// input    : none
// output   : none
//*****************************************************************
void clear_ep4_sequence(void)
{
    clear_ep_sequence(4);    
}


//*****************************************************************
// function : usb_ep0_null_out
// input    : none
// output   : none
//*****************************************************************
void usb_ep0_null_out(void)
{
}

//*****************************************************************
// function : usb_ep0_null_in
// input    : none
// output   : none
//*****************************************************************
void usb_ep0_null_in(void)
{
    USBEP0TxCnt = 0;
    if(gbEP0NotEnd) {
        usb_ep0_xbyte_in();
    } else {
#if 0    
        UsbWriteEp0CSR(SET_EP0_TXPKTRDY | EP0_DATAEND_BIT | SET_EP0_SERVICED_RXPKTRDY);   // avoid stall bug follow setup->outdata
#else
        UsbWriteEp0CSR(EP0_DATAEND_BIT | SET_EP0_SERVICED_RXPKTRDY);   // dont need SET_EP0_TXPKTRDY if zero-size data
#endif
    }
}

//*****************************************************************
// function : usb_ep0_xbyte_in
// input    : none
// output   : none
//*****************************************************************
void usb_ep0_xbyte_in(void)
{
    //    USBEP0TxAddr = USBEP0TxBufAddr;
    usb_ep0_in();
}

//*****************************************************************
// function : usb_ep0_in
// input    : none
// output   : none
//*****************************************************************
void usb_ep0_in(void)
{
    if(gbEP0NotEnd)
        UsbWriteEp0CSR(SET_EP0_TXPKTRDY);
    else {
      //  dbg_printf(0,"-EOS-\r\n");
        UsbWriteEp0CSR(SET_EP0_TXPKTRDY | EP0_DATAEND_BIT);
    }    
}



#endif
//------------------------------------------------------------------------------
//  Function    : UsbEp0IntHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbEp0IntHandler(void)
{
extern void MMPF_AdjustUSBSquelchLevel(MMP_USHORT squelch_level);
    MMP_USHORT csr,i;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    if(/*glTestModeFeature*/gsUsbCtrlReq.wTestMode != 0) {
        dbg_printf(0, "Start Test Mode:%x\r\n",gsUsbCtrlReq.wTestMode);
        if( ( (gsUsbCtrlReq.wTestMode>>8) == TEST_J) || ( (gsUsbCtrlReq.wTestMode>>8) == TEST_K) ) {
            SPI_Write(0x08,0x89f);
            SPI_Write(0x02,0x197F);
            //dbg_printf(3,"USB08 : %x\r\n",0x89f);
            //dbg_printf(3,"USB02 : %x\r\n",0x197F);
            
        } else {
            MMPF_AdjustUSBSquelchLevel(USB_SQUELCH_LEVEL);
        }
        
        switch(/*glTestModeFeature*/gsUsbCtrlReq.wTestMode >> 8) {
        case TEST_J:
            pUSB_CTL->USB_TESTMODE = 0x2;
            break;
        case TEST_K:
            pUSB_CTL->USB_TESTMODE = 0x4;
            break;
        case TEST_SE0_NAK:
            pUSB_CTL->USB_TESTMODE = 0x1;
            break;
        case TEST_PACKET:
            for(i = 0;i < 53;i++) {
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = TestPacket[i];
            }
            pUSB_CTL->USB_TESTMODE = 0x8;
            UsbWriteEp0CSR(SET_EP0_TXPKTRDY);
            break;
        case TEST_FORCE_ENABLE:
            break;
        }
        //glTestModeFeature = 0;
        gsUsbCtrlReq.wTestMode = 0;
        return;
    } else {
    }
    
    
#if (SUPPORT_UVC_FUNC==1)
    csr = UsbReadEp0CSR();
    if(csr & EP0_SENTSTALL_BIT) {
        UsbWriteEp0CSR(csr & ~EP0_SENTSTALL_BIT);
        if(csr & EP0_SETUPEND_BIT) {
            RTNA_DBG_Str(0, "as--cs\r\n");            
            UsbWriteEp0CSR(SET_EP0_SERVICED_SETUPEND);
        }
    }
    if(USBInPhase) {
        usb_ep0_in_done();                  
        if(USBEP0TxCnt == 0) {
            if(csr & EP0_SETUPEND_BIT) {
                RTNA_DBG_Str(0,"ai-cs\r\n");                            
                UsbWriteEp0CSR(SET_EP0_SERVICED_SETUPEND);
            }
        }
    } 
#if 0
	else if(USBOutData && (csr & EP0_RXPKTRDY_BIT)) 
#else // avoid change state is too fast
	if(USBOutData && (csr & EP0_RXPKTRDY_BIT)) 
#endif
    {
        usb_ep0_out_done();   
        if(csr & EP0_SETUPEND_BIT) {
            //RTNA_DBG_Str(0, "after outdata, clear setupend\r\n");                                        
            //                UsbWriteEp0CSR(SET_EP0_SERVICED_SETUPEND);// debug only
        }
    } 
    else if((csr & EP0_RXPKTRDY_BIT) /*&& (USBInPhase == 0) && (USBOutPhase == 0)*/) { // WIN8.1.Dell
        usb_ep0_setup_done();
    }                  

    if(USBOutPhase && USBOutData == 0) {
        usb_ep0_out_done();   

        if(csr & EP0_SETUPEND_BIT) {
            RTNA_DBG_Str(0, "outphase\r\n");                                        
            //                UsbWriteEp0CSR(SET_EP0_SERVICED_SETUPEND);// debug only
        }
    }
#endif        
}

//------------------------------------------------------------------------------
//  Function    : UsbEp1RxIntHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbEp1RxIntHandler(void)
{
#if (SUPPORT_UVC_FUNC==1)
#endif    
}

//------------------------------------------------------------------------------
//  Function    : UsbEp1TxIntHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbEp1TxIntHandler(void)
{
    UsbUVCTxIntHandler(0);
}
//------------------------------------------------------------------------------
//  Function    : UsbEp1TxIntHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbEp2TxIntHandler(void)
{
    // "==" -> ">=1"
    if(VIDEO_EP_END()>=2) { // For endpoint 2 video FIFO mode.
        UsbUVCTxIntHandler(1);
    }
}

static void UsbUVCTxIntHandler(MMP_UBYTE ep_id)
{
#if (SUPPORT_UVC_FUNC==1)
extern MMP_BOOL MMPF_USB_ReleaseDMA(MMP_UBYTE ep_id);
    STREAM_SESSION *ss =  MMPF_Video_GetStreamSessionByEp(ep_id); 
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    MMP_ULONG dmalst_addr = 0;
    MMP_UBYTE dma_id ;
    
    ss->tx_flag &= ~SS_TX_1ST_PKT_DONE ;
    if( ss->tx_flag & SS_TX_PRECLOSE_STREAMING )
    {
        MMP_BOOL bRawPathPreview ;
        STREAM_CFG *cur_stream ,cur_stream_tmp = *(STREAM_CFG *)usb_get_cur_image_pipe_by_epid(ep_id);

        cur_stream = &cur_stream_tmp ;
        cur_stream->pipe_en &=~PIPEEN(GRA_SRC_PIPE);
        
        ss->tx_flag |= SS_TX_CLOSE_STREAMING ;
        MMPF_Sensor_GetParam(MMPF_SENSOR_RAWPATH_PREVIEW_ENABLE  , &bRawPathPreview);
        // Remove preview screen discontinue horizontal line
        if(/*(ss->tx_flag & SS_TX_SYNC_STREAMING)|| */(bRawPathPreview)) {
            MMP_UBYTE pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_stream->pipe_cfg);
            MMP_UBYTE pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_stream->pipe_cfg);
            if( IS_PIPE0_EN(cur_stream->pipe_en ) ){
                if(pipe0_cfg==PIPE_CFG_YUY2) {
                    MMPF_Video_UpdateWrPtr(PIPE_0);
                }                
            }
            if( IS_PIPE1_EN(cur_stream->pipe_en ) ){
                if(pipe1_cfg==PIPE_CFG_YUY2) {
                    MMPF_Video_UpdateWrPtr(PIPE_1);
                }                
            }
        }
        // One shot mode
        if(ss->tx_flag & SS_TX_BY_ONE_SHOT) {
        #if YUY2_848_480_30FPS==FIRE_AT_FRAME_END
            if( ss->tx_flag & SS_TX_STREAMING ) {
                if( IS_PIPE0_EN(cur_stream->pipe_en ) ){
                    MMPF_USB_ReleaseDm(PIPE_0);
                }
                if( IS_PIPE1_EN(cur_stream->pipe_en ) ){
                    MMPF_USB_ReleaseDm(PIPE_1);
                }
            }
        #endif    
        }
       // dbg_printf(3,"vs.e:%d\r\n",uSOFNUM );
    }
    if(( ( ss->tx_flag & SS_TX_CLOSE_STREAMING)==0) && (ss->tx_flag & SS_TX_STREAMING )) {
        usb_vs_next_packet(ss->ep_id);
    }
    else if (ss->tx_flag & SS_TX_RT_MODE ) {
        usb_vs_next_packet(ss->ep_id);
    }
    dma_id = usb_epid_link_dmaid(ep_id);
    if(dma_id==0) {
        dmalst_addr = pUSB_DMA->USB_DMA1_FB_ST_ADDR;
       // pUSB_DMA->USB_DMA1_FB_ST_ADDR = 0 ;
    } else {
        dmalst_addr = pUSB_DMA->USB_DMA2_FB_ST_ADDR;
       // pUSB_DMA->USB_DMA2_FB_ST_ADDR = 0 ;
    }
    if(dmalst_addr) {
        MMP_BOOL timeout ;
        timeout = MMPF_USB_ReleaseDMA( ep_id ); 
        if(!timeout) {
            usb_vs_release_dmabuf(ep_id,(MMP_ULONG )( dmalst_addr  - USB_DMA_DATA_LIST_OFFSET ) ) ;
        }
    }
#endif    

}

MMP_ULONG usb_cur_dma_addr(MMP_UBYTE ep_id)
{
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    MMP_UBYTE dma_id ;
    dma_id = usb_epid_link_dmaid(ep_id);
    if(dma_id==0) {
        return pUSB_DMA->USB_DMA1_FB_ST_ADDR;
    } else {
        return pUSB_DMA->USB_DMA2_FB_ST_ADDR;
    }

}
//------------------------------------------------------------------------------
//  Function    : UsbEp3TxIntHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbEp3TxIntHandler(void)
{
#if (SUPPORT_UVC_FUNC==1)
#endif    
}

#if SUPPORT_PCSYNC
#include "mmpf_usbpcsync.h"
extern PCSyncBulkEp gPCSyncBulkEpTx,gPCSyncBulkEpRx ;
#endif

void UsbEp5TxIntHandler(void)
{
#if SUPPORT_PCSYNC
	pcsync_bulk_ep_interrupt(&gPCSyncBulkEpTx);		
#endif 
}

void UsbEp5RxIntHandler(void)
{

	#if SUPPORT_PCSYNC
	pcsync_bulk_ep_interrupt(&gPCSyncBulkEpRx);
	#endif
	
	  
}

//------------------------------------------------------------------------------
//  Function    : UsbDetectModeHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void UsbDetectModeHandler(void) 
{
}


MMP_USHORT MMPF_SYS_SendMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT ret,err ;
    msg_t *msg;

    msg = allocate_msg(src_id);
    if(!msg ){
        return MSG_ERROR_NONE ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    ret = send_msg(msg,&err,src_id);
    /*destroy message*/
    free_msg( (void *)msg, src_id);
    if(ret) {
        return ret ;
    }
    return err;
}

MMP_USHORT MMPF_SYS_PostMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT ret=MSG_ERROR_NONE ;
    msg_t *msg;
    msg = allocate_msg(src_id);
    if(!msg ){
        dbg_printf(0,"!msg !msg(%d,%d,%d)\r\n",src_id,msg_id,msg_sub_id);
        return MSG_ERROR_NONE ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    ret = post_msg(msg, src_id);
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_USB_ISR
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void MMPF_USB_ISR(void)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    MMP_USHORT  dma_int;
    MMP_USHORT  tx_int,rx_int;
    MMP_UBYTE   usb_int;
    dma_int = pUSB_DMA->USB_DMA_INT_SR & pUSB_DMA->USB_DMA_INT_EN;
#if 1 // clean here for test
    if(dma_int != 0x0) {
    	//if( (dma_int!=0x05) && (dma_int!=0x28))
    	//    dbg_printf(3,">: %x\r\n",dma_int );
    	pUSB_DMA->USB_DMA_INT_SR &= ~dma_int ;
    }
#endif    
    tx_int = pUSB_CTL->USB_TX_INT_SR & pUSB_CTL->USB_TX_INT_EN;
    rx_int = pUSB_CTL->USB_RX_INT_SR & pUSB_CTL->USB_RX_INT_EN;
    usb_int = pUSB_CTL->USB_INT_EVENT_SR & pUSB_CTL->USB_INT_EVENT_EN;

    if(tx_int != 0) {
        if(tx_int & EP0_INT_BIT) {
            //dbg_printf(3,"tx_int:%x,%x\r\n",tx_int,pUSB_CTL->USB_TX_INT_SR & pUSB_CTL->USB_TX_INT_EN);
			MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_EP0_TX_INT,0,(void *)NULL);
        }
        if(tx_int & EP1_TX_INT_BIT) {
            #if (CHIP==P_V2)
            if(( pUSB_DMA->USB_DMA_CTL_B&0x20)==0) 
            #endif
            {
                UsbEp1TxIntHandler();
            }
        } 
        if(tx_int & EP2_TX_INT_BIT) {
            #if (CHIP==P_V2)
            if(( pUSB_DMA->USB_DMA_CTL_B&0x40)==0) 
            #endif
            {
                UsbEp2TxIntHandler();
            }
        }
        
        if(tx_int & EP3_TX_INT_BIT) {
            // audio, no need
			//MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_EP3_TX_INT,0,(void *)NULL);
        } 
        
        #if SUPPORT_PCSYNC
        if(tx_int & EP5_TX_INT_BIT) {
            UsbEp5TxIntHandler();
            //MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_EP5_TX_INT,0,(void *)NULL);
        }
        #endif
    }
    if(rx_int != 0) {
        if(rx_int & EP1_RX_INT_BIT) {
			MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_EP1_RX_INT,0,(void *)NULL);
            //UsbEp1RxIntHandler(); //Gason
        }
        if(rx_int & EP2_RX_INT_BIT) {
			MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_EP2_RX_INT,0,(void *)NULL);
            //UsbEp1RxIntHandler(); //Gason
        }
        #if SUPPORT_PCSYNC
        if(rx_int & EP5_RX_INT_BIT) {
            //UsbEp5RxIntHandler();
            MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_EP5_RX_INT,0,(void *)NULL);
        }
        #endif 
    }
    if(usb_int != 0) {
        if(usb_int & RESET_INT_BIT) {
#if USB_SUSPEND_TEST
            if(gbUSBSuspendFlag == 1){
                RTNA_DBG_Str(0, "-at rst-\r\n");
                gbUSBSuspendFlag = 0;
            }    
#endif            
            UsbRestIntHandler();
        } 
        if(usb_int & SUSPEND_INT_BIT) {

            if(gsUsbCtrlReq.bDevAddr!=0)
            { 
            //   RTNA_DBG_Str(0, "-at suspend-\r\n");
            }
#if USB_SUSPEND_TEST
            if(/*(USBDevAddr != 0) && */(gbUSBSuspendFlag==0) ){
                usb_uvc_suspend();          
                gbUSBSuspendEvent = 1;
                MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_SUSPEND_INT,0,(void *)NULL);
            }
#endif 
        }
        if(usb_int & RESUME_INT_BIT) {
        
#if  USB_SUSPEND_TEST
            if(gbUSBSuspendFlag == 1){
                RTNA_DBG_Str(0, "-at resume-\r\n");
                gbUSBSuspendFlag = 0;
            }
#endif                    
            RTNA_DBG_Str(0, "-USB Resume-\r\n");
        } else if(usb_int & SOF_INT_BIT) {
            dbg_printf(3,"WFI on, don't support this int\r\n");
            while(1);
#if (SUPPORT_UVC_FUNC==1)        
            if(gbUsbHighSpeed == 0) {
                SOFNUM++;
                uSOFNUM++ ;
            } else {
            /*
                if(uSOFNUM == 7) {
                    uSOFNUM = 0; 
                    SOFNUM++;
                } else {
                    uSOFNUM++;
                }
            */
                uSOFNUM++ ;
                SOFNUM = uSOFNUM >> 3;
            }
            
#endif
        } 
    }
    
dma_done:    
    if(dma_int != 0x0) {
        STREAM_SESSION *ss ;
        MMP_UBYTE epaddr ,epid;
    	//pUSB_DMA->USB_DMA_INT_SR = 0x0;
    	//dbg_printf(3,"dma.int : %x\r\n",dma_int );
    	if(dma_int & USB_INT_DMA1_DESC_CNT_EN ) {
    	    ss = MMPF_Video_GetStreamSessionByEp(0);
    	    ss->tx_flag |= SS_TX_1ST_PKT_DONE ;
    	}
    	if(dma_int & USB_INT_DMA2_DESC_CNT_EN ) {
    	    ss = MMPF_Video_GetStreamSessionByEp(1);
    	    ss->tx_flag |= SS_TX_1ST_PKT_DONE ;
    	}
    	
        if (dma_int & USB_INT_DMA1_DONE_EN) {
        	if(pUSB_DMA->USB_DMA1_DESC_CNT) {
        	    dbg_printf(3,"+%d(%x)\r\n",pUSB_DMA->USB_DMA1_DESC_CNT,dma_int );
        	}
        	if(pUSB_DMA->USB_DMA1_PKT_BYTE_CNT) {
        	    dbg_printf(3,"*%d(%x)\r\n",pUSB_DMA->USB_DMA1_PKT_BYTE_CNT,dma_int );
        	}
    	
    	    //dbg_printf(3,"R");
    	#if (CHIP==VSN_V2)
    	    epaddr = ( pUSB_DMA->USB_DMA_CMD_ADDR & 0xFF) >> 4 ; 
    	#endif
    	#if (CHIP==VSN_V3)
    	    epaddr = ( pUSB_DMA->USB_DMA1_CMD_ADDR & 0xFF) >> 4 ; 
    	#endif
    	    epid = usb_epaddr_link_epid(epaddr);
    	    if(epid!=0) {
    	        dbg_printf(3,"dma.int0 bad\r\n");
    	    }
    	  //  dbg_printf(3,"dma_int : %x / %d\r\n",dma_int,epid);
    	    UsbUVCTxIntHandler(epid);
    	    
    	}
    	#if (CHIP==VSN_V3)
        if (dma_int & USB_INT_DMA2_DONE_EN) {

        	if(pUSB_DMA->USB_DMA2_DESC_CNT) {
        	    dbg_printf(3,"-%d(%x)\r\n",pUSB_DMA->USB_DMA2_DESC_CNT,dma_int );
        	}
        	if(pUSB_DMA->USB_DMA2_PKT_BYTE_CNT) {
        	    dbg_printf(3,"#%d(%x)\r\n",pUSB_DMA->USB_DMA2_PKT_BYTE_CNT,dma_int );
        	}

    	    //dbg_printf(3,"B");
    	    epaddr = ( pUSB_DMA->USB_DMA2_CMD_ADDR & 0xFF) >> 4 ; 
    	    epid = usb_epaddr_link_epid(epaddr);
    	    if(epid!=1) {
    	        dbg_printf(3,"dma.int1 bad\r\n");
    	    }
    	    
    	    UsbUVCTxIntHandler(epid);
    	   // dbg_printf(3,"dma_int : %x / %d\r\n",dma_int,epid);
    	}
    	#endif
    	
    	dma_int = pUSB_DMA->USB_DMA_INT_SR & pUSB_DMA->USB_DMA_INT_EN;
    	if(dma_int!=0) {
    	    pUSB_DMA->USB_DMA_INT_SR &= ~dma_int;
    	    //dbg_printf(3,"-> : %x\r\n",dma_int);
    	    goto dma_done ;
    	}
    }
    
}
#endif  //#if   defined(USB_FW)||(USB_EN)
#undef exUSBINT

/// @}
