#include    "mmpf_usbvend.h"
#include    "config_fw.h"
#include	"mmpf_usbpcsync.h"
#include	"lib_retina.h"
#include 	"mmpf_usbextrn.h"
#include 	"mmp_reg_usb.h"
#include    "mmpf_usbuvc15h264.h"
#include    "mmpf_usbuvch264.h"

#include    "mmpf_usbpccam.h"

#if SUPPORT_PCSYNC

#define PCSYNC_USE_DMA_MODE 0
#define SIM_TEST    (0)

//for vendor control
extern MMP_UBYTE gbEP0NotEnd;
extern MMP_UBYTE gbUsbHighSpeed;
extern MMP_USHORT  EP0TXOffset;
/*
extern MMP_UBYTE UsbRequestPayload_bmRequestType;
extern MMP_UBYTE UsbRequestPayload_bRequest;
extern MMP_USHORT UsbRequestPayload_wValue;
extern MMP_USHORT UsbRequestPayload_wIndex;
extern MMP_USHORT UsbRequestPayload_wLength;
*/
extern UsbCtrlReq_t gsUsbCtrlReq ;

int gUsbControlDataCount = 0;
//extern char FacePosition[];

extern int GetFacePostion(void **face_pos);
extern void USB_SetDownloadFWSize(MMP_ULONG fw_size);
	
extern MMP_UBYTE  USBOutData;
//for pcsync bulk transfer 
PCSyncBulkEp gPCSyncBulkEpTx,gPCSyncBulkEpRx; 
MMP_UBYTE *gPcSyncBuf = (MMP_UBYTE*)0x1500000;
extern void GPIO_OutputControl(MMP_UBYTE num, MMP_UBYTE status);

MMP_ULONG  glAccEncBytes[3]  ;

void pcsync_read_frame_end(void);
void pcsync_write_frame_end(void);
void pcsync_init_bulk_transfer(PCSyncBulkEp *pcSyncEp,
								  MMP_UBYTE ep,
								  MMP_UBYTE *buf,
								  MMP_LONG length,
								  MMP_UBYTE direction,
								  BulkEndCB *cb)
{
	pcSyncEp->BufAddr = buf;
	pcSyncEp->Direction = direction;
	pcSyncEp->ReqBytes = length;
	pcSyncEp->Ep = ep;
	pcSyncEp->CallBack = cb ;
	pcsync_set_ep_direction(ep,direction);
}

void pcsync_set_ep_direction(MMP_UBYTE ep,MMP_UBYTE direction)
{
	//volatile MMP_USHORT *USB_REG_BASE_W = (volatile MMP_USHORT *) USB_CTL_BASE_ADDR;
	MMP_USHORT csr = UsbReadTxEpCSR(ep);
	
#if PCSYNC_USE_DMA_MODE	
	csr = csr | TX_DMAREQENAB;//enable ep1 dma	
#else
	csr = csr & ~TX_DMAREQENAB;//disable ep1 dma	
#endif

	if(direction == PCSYNC_DIRECTION_BULK_IN)//set tx mode
	{
		UsbWriteTxEpCSR(ep, csr | SET_TX_MODE);
	}
	else
	{
		UsbWriteTxEpCSR(ep,csr & ~SET_TX_MODE);
		//csr = UsbReadRxEpCSR(csr);
	}
}


void pcsync_bulk_ep_interrupt(PCSyncBulkEp *pcSyncEp)
{
	MMP_ULONG TransLen;
	
	//RTNA_DBG_Str(DBG_LEVEL, "PCSYNC :bulk ep interrupt\r\n");

	if(!pcSyncEp->ReqBytes)  {
		return;
	}
	TransLen = pcsync_ep_bulk_rw(pcSyncEp->BufAddr,pcSyncEp->ReqBytes,pcSyncEp->Direction,pcSyncEp->Ep);
	
	pcSyncEp->BufAddr += TransLen;
	pcSyncEp->ReqBytes -= TransLen;
	if(!pcSyncEp->ReqBytes) {
		if(pcSyncEp->CallBack) {
		    pcSyncEp->CallBack();
		}
	}
}

MMP_ULONG pcsync_ep_bulk_in_ringbuf(MMP_UBYTE *srcRingBuf, MMP_UBYTE *currentptr,MMP_ULONG tolbufsize, MMP_ULONG length,MMP_UBYTE Ep)
{
	volatile MMP_UBYTE *EpFifoAddrB;
	volatile MMP_ULONG *EpFifoAddrD;
	MMP_ULONG TransBytes;
	MMP_ULONG i,N;
	MMP_USHORT csr;
	MMP_ULONG *tmp=0;
	MMP_UBYTE *tmp1=0;
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	
		
	while(	length ){
		
		csr = UsbReadTxEpCSR(Ep);
		while( (csr & TX_TXPKTRDY_BIT)==1)
		{
				MMPF_OS_Sleep(10);
				csr = UsbReadTxEpCSR(Ep);
		}
		EpFifoAddrD = (volatile MMP_ULONG*)((&pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B)+Ep*4);
		EpFifoAddrB = (volatile MMP_UBYTE*)EpFifoAddrD;
	
		if(gbUsbHighSpeed == USB_HIGH_SPEED)
			TransBytes = length>PCSYNC_EP_MAX_PK_SIZE?PCSYNC_EP_MAX_PK_SIZE:length;
		else
			TransBytes = length>64?64:length;		
		
		N = TransBytes >> 2;
		tmp = (MMP_ULONG*)currentptr;
		for(i=0;i<N;++i)
		{
			*EpFifoAddrD = *tmp++;
			if((MMP_ULONG) tmp >= (MMP_ULONG)(srcRingBuf+tolbufsize)){	
				tmp = (MMP_ULONG*)srcRingBuf; 
			//	RTNA_DBG_PrintLong(0,(MMP_ULONG)tmp);
			}
		}

		i*=4;	
		tmp1 = (MMP_UBYTE *)tmp;
		
		for(;i<TransBytes;++i)
		{
			*EpFifoAddrB = *tmp1++;
			if( tmp1 >= (srcRingBuf+tolbufsize)){			
				tmp1 = (MMP_UBYTE*)srcRingBuf; 
				//RTNA_DBG_PrintLong(0,(MMP_ULONG)tmp1);
			}			
		}
		
		csr = UsbReadTxEpCSR(Ep);
		csr &= TXCSR_RW_MASK;
	    UsbWriteTxEpCSR(Ep, csr | SET_TX_TXPKTRDY);

		length -= TransBytes;
		currentptr+=TransBytes;
		
		if((MMP_ULONG)currentptr >= (MMP_ULONG)(srcRingBuf+tolbufsize))
			currentptr =  currentptr - tolbufsize;
	}
	return 0;
}
//BulkIn = 1 Bulk-in
//BulkIn = 0 Bulk-out
MMP_ULONG pcsync_ep_bulk_rw(MMP_UBYTE *srcBuf,MMP_ULONG length,MMP_UBYTE direction,MMP_UBYTE Ep)
{
	MMP_ULONG TransBytes;
	MMP_ULONG i,N,R;
	MMP_USHORT csr;
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	
	if(direction == PCSYNC_DIRECTION_BULK_IN) {	
		MMP_USHORT fifo_c ;
		
	    do {
		    csr = UsbReadTxEpCSR(Ep);
		   // dbg_printf(3,"T");
		} while (csr&TX_TXPKTRDY_BIT) ;
		
		fifo_c = UsbReadRxEpCOUNT(Ep) ;	
		if( fifo_c ) {
		    dbg_printf(3,"IN.fifo:%d\r\n",fifo_c);
		}	
        TransBytes = length > PCSYNC_EP_MAX_PK_SIZE?PCSYNC_EP_MAX_PK_SIZE:length;
		N = TransBytes >> 2;
		
//dbg_printf(3,"[%x,%x,%x,%x,%x]\r\n",srcBuf[0],srcBuf[1],srcBuf[2],srcBuf[3],srcBuf[4]);

		for(i=0;i<N;++i) {
			//*EpFifoAddrD = ((MMP_ULONG*)srcBuf)[i];
			pUSB_CTL->USB_FIFO_EP[Ep].FIFO_D = ((MMP_ULONG*)srcBuf)[i];
		}
		i*=4;
		for(;i<TransBytes;++i) {
			//*EpFifoAddrB = srcBuf[i];
			pUSB_CTL->USB_FIFO_EP[Ep].FIFO_B = srcBuf[i];	
		}
		
		csr = UsbReadTxEpCSR(Ep);
		csr &= TXCSR_RW_MASK;
	    UsbWriteTxEpCSR(Ep, csr | SET_TX_TXPKTRDY);
	}
	else {

		TransBytes = UsbReadRxEpCOUNT(Ep);
		N=TransBytes/4;
				
		for(i=0;i<N;++i) {
			//((MMP_ULONG*)srcBuf)[i] = *EpFifoAddrD;
			((MMP_ULONG*)srcBuf)[i] = pUSB_CTL->USB_FIFO_EP[Ep].FIFO_D ;
		}
		i*=4;	
        for(;i<TransBytes;++i) {
			//srcBuf[i] = *EpFifoAddrB;
			srcBuf[i] = pUSB_CTL->USB_FIFO_EP[Ep].FIFO_B ;
		}

		csr = UsbReadRxEpCSR(Ep);
    	csr = csr & RXCSR_RW_MASK;		
		UsbWriteRxEpCSR(Ep,csr|CLEAR_RX_RXPKTRDY);
		//UsbWriteRxEpCSR(Ep,csr & (~RX_RXPKTRDY_BIT) );
	}
	 //RTNA_DBG_Str(DBG_LEVEL, "EXIT  pcsync_ep_bulk_rw \r\n");
	return TransBytes;
}

//Ep : end point
//buf :	buf != 0 :source buffer
//	   	buf == 0 : push zero to fifo
MMP_ULONG pcsync_usb_fifo_push(MMP_BYTE Ep,MMP_UBYTE *buf,MMP_USHORT length)
{
	MMP_ULONG TransBytes;
	MMP_ULONG i;
	
	volatile MMP_ULONG *FIFO_ADDR_D;
	volatile MMP_UBYTE *FIFO_ADDR_B;
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
	
	FIFO_ADDR_D= (volatile MMP_ULONG *)((&pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B)+Ep*4);
	FIFO_ADDR_B=(volatile MMP_UBYTE *)FIFO_ADDR_D;

	if(gbUsbHighSpeed == USB_HIGH_SPEED && Ep>0)
		//TransBytes = length>256?256:length;
		TransBytes = length>PCSYNC_EP_MAX_PK_SIZE?PCSYNC_EP_MAX_PK_SIZE:length;
	else
		TransBytes = length>64?64:length;

	if(buf)
	{
		for(i=0;i<TransBytes;++i)
			*FIFO_ADDR_B = buf[i];
	}		
	else
	{
		for(i=0;i<TransBytes;++i)		
			*FIFO_ADDR_B = 0x00;
	}
	
	return TransBytes;
}


//*****************************************************************
// function : pcsync_vendor_request_setup
// input    : none
// output   : none
//*****************************************************************
//extern int faceCount;

//for PCSYNC_GET_FD_LIST command
//MMP_BYTE *gFacePosition = NULL;
#include "pcam_dfu.h"
//#include "cx2070x_download.h"
MMP_ULONG glPcSyncFrameSeq[2] ;
PCSYNC_VIDEO_FRAME gsFrameDesc_t[2] ;
MMP_BOOL gbPcSyncPreview = MMP_FALSE ;
MMP_BOOL gbWorkingStream[2] = { MMP_FALSE , MMP_FALSE } ;
MMP_USHORT gsEP0RxCnt,gsEP0RxLen ;
MMP_UBYTE gbCurStreamId ;

void pcsync_vendor_request_setup(void)
{
    MMP_UBYTE req;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    
	gbEP0NotEnd = 0;
	EP0TXOffset = 0;
	USBEP0TxCnt = 0;
	
    req = USBREQ_R(gsUsbCtrlReq);
	UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
	
    switch(req)
    {
	    case PCSYNC_CONFIG:
		{
		    MMP_USHORT sub_cmd = USBREQ_V(gsUsbCtrlReq);
		    MMP_USHORT cmd_len = USBREQ_L(gsUsbCtrlReq) ;
		    
			//dbg_printf(DBG_LEVEL,"vsetup.config : %d,%d\r\n",sub_cmd,cmd_len);
			USBOutPhase = VENDOR_CMD ;
			USBOutData = 1  ;
			pcsync_ep0_rx_init(cmd_len);
			break;
    	}		
	    case PCSYNC_START:
		{
			dbg_printf(DBG_LEVEL,"vsetup.preview\r\n");
		    pcsync_start_preview();
			break;
    	}		
    	case PCSYNC_WRITE_FRAME:	//Vender control in
    	{
		    MMP_USHORT sub_cmd = USBREQ_V(gsUsbCtrlReq);
		    MMP_USHORT cmd_len = USBREQ_L(gsUsbCtrlReq) ;
		    
			//dbg_printf(DBG_LEVEL,"vsetup.frame : %d,%d\r\n",sub_cmd,cmd_len);
			USBOutPhase = VENDOR_CMD ;
			USBOutData = 1  ;
			pcsync_ep0_rx_init(cmd_len);
			break;
    	}
    	case PCSYNC_READ_FRAME:
    	{

            PCSYNC_VIDEO_STATUS status = { 0,0 };
            MMP_ULONG  frameptr,framesize;
                       
            while( MMPF_Video_IsEmpty(PIPE_1) ) {
                MMPF_OS_Sleep(1);
               // dbg_printf(0,"@");
            }
            
            {
                MMP_USHORT sId;
                frameptr = pcsync_get_frame(&framesize,&sId) ;
                if(frameptr) {
                    MMP_ULONG cpu_sr ;
                    glPcSyncFrameSeq[sId]++ ;
                    status.length = framesize ;
                    //dbg_printf(3,"read.frame.status(%d,%d)\r\n",status.error,status.length);
                    //
                    // Tx is handled in IRQ, we need to lock 1st tx to prevent
                    // Next Tx IRQ is coming quickly.
                    //
                    IRQ_LOCK(
                        pcsync_init_bulk_transfer(&gPCSyncBulkEpTx,PCSYNC_EP_ADDR,(MMP_UBYTE *)frameptr,status.length,PCSYNC_DIRECTION_BULK_IN,pcsync_read_frame_end);
                        pcsync_bulk_ep_interrupt(&gPCSyncBulkEpTx);
                    )
                }
                else {
                    status.error = 1 ;
                    status.length = 0 ;
                   dbg_printf(3,"read.empty.frame2\r\n");
                }
                
            }
            USB_PUT_EP0_DATA(VENDOR_CMD,8,(MMP_UBYTE *)&status);
        	break ;
		}
		case PCSYNC_STOP:
    	{
			dbg_printf(DBG_LEVEL,"vsetup.stop\r\n");
			dbg_printf(DBG_LEVEL,"input frame(s0,s1) :( %d,%d)\r\n",gsFrameDesc_t[0].dwFrameSeq,gsFrameDesc_t[1].dwFrameSeq);
			dbg_printf(DBG_LEVEL,"output frame(s0,s1):(%d,%d)\r\n",glPcSyncFrameSeq[0],glPcSyncFrameSeq[1]);
			
			gbWorkingStream[0] = MMP_FALSE ;
			gbWorkingStream[1] = MMP_FALSE ;
		    pcsync_stop_preview();
			break;
    	}
				
		default:
			usb_error();
			break;
    }
}

void pcsync_ep0_rx_init(MMP_USHORT rx_len)
{
    gbEP0NotEnd = 0;
    gsEP0RxLen = rx_len;
    gsEP0RxCnt = 0;
    
}

MMP_BOOL pcsync_ep0_rx(MMP_UBYTE *ptr)
{
    MMP_USHORT i,rx_len ;
    ptr += gsEP0RxCnt ;
    if(gsEP0RxLen > 64) {
        rx_len = 64 ;
    }
    else {
        rx_len = gsEP0RxLen ;
    }
    
    USB_GET_FIFO_DATA(MMPF_USB_EP0,rx_len,ptr,rx_len ) ;
    #if 0
    for(i = 0; i <rx_len;i++) {
        RTNA_DBG_Byte(3,ptr[i]);
        if( i && !(i&15) ) {
            RTNA_DBG_Str(3,"\r\n");
        }
    }
    #endif
    gsEP0RxLen -= rx_len ;
    gsEP0RxCnt += rx_len ;
    if(!gsEP0RxLen) {
        return MMP_TRUE;
    }
    return MMP_FALSE ;
}
//*****************************************************************
// function : pcsync_vendor_request_data_out
// input    : none
// output   : none
//*****************************************************************

__align(4) PCSYNC_H264_CONFIG pcsync_h264_config[2];
UVC15_VIDEO_CONFIG uvc15_h264_config ;
MMP_BOOL gbConverTest,gbConverTest30FPS = MMP_TRUE ;
MMP_UBYTE gbSvcLayers ;

void pcsync_vendor_request_data_out(void)
{
    MMP_USHORT req = USBREQ_R(gsUsbCtrlReq);
    MMP_USHORT cmd = USBREQ_V(gsUsbCtrlReq);
    MMP_USHORT len = USBREQ_L(gsUsbCtrlReq);
    //dbg_printf(3,"out ( req,cmd,len)=(%x,%x,%d)\r\n",req,cmd,len);
    //dbg_printf(3,"outphase,outdata=(%x,%d)\r\n",req,USBOutData);
    
    switch(req) {
        case PCSYNC_CONFIG:
    
    	switch(cmd) {
            case CONFIG_UVC15_H264:
        		{
         		    MMP_BOOL last_pkt ,conver_test ;
                    MMP_UBYTE *config_ptr ;
         		    MMP_USHORT i,rx_len = 0 , sId = USBREQ_I(gsUsbCtrlReq) ;
        		    UVC15_VIDEO_CONFIG *config ;
         		    config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sId) ;
        		    config_ptr = (MMP_UBYTE *)&pcsync_h264_config[sId] ;
                    last_pkt = pcsync_ep0_rx(config_ptr);
         		    if(last_pkt) {
         		        dbg_printf(3,"config(s%d) len:%d,sizeof(config):%d\r\n",sId,len,sizeof(PCSYNC_H264_CONFIG));
        		        //pcsync_dump_h264_config(sId,&pcsync_h264_config[sId]) ;
        		        gbConverTest = pcsync_h264_to_uvc15_h264(&uvc15_h264_config,&pcsync_h264_config[sId]);
        		        
        		        if(sId==0) {
            		        if(uvc15_h264_config.dwFrameInterval==333333) {
            		            gbConverTest30FPS = MMP_TRUE ;
            		        }
            		        else {
            		            gbConverTest30FPS = MMP_FALSE ;
            		            //uvc15_h264_config.dwAverageBitRate[0] *= 2;
            		        }
            		        
            		        
            		        gbSvcLayers = pcsync_h264_config[sId].bSvcLayers;
        		        } 
        		        *config = uvc15_h264_config ;
        		        
        		        
         		        #if SIM_TEST
        		        {
        		        
        		            config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(1) ;
        		            pcsync_h264_config.wLayerOrViewID = 1 ;
        		            pcsync_h264_to_uvc15_h264(&uvc15_h264_config,&pcsync_h264_config[sId]);
        		            *config = uvc15_h264_config ;
        		            //config->wLayerOrViewID = SVC_LAYER_ID(0,0,0,1,0);;
        		            gbWorkingStream[1] = MMP_TRUE ;
        		        }
        		        #endif
       		        
                        if(sId < 2 ) {
                            gbWorkingStream[sId] = MMP_TRUE ;
                        }
        		        USBOutData = 0;
        		        USBOutPhase = 0;
        		        UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY|EP0_DATAEND_BIT);
                    }
                    else {
                        USBOutPhase = VENDOR_CMD;
                        USBOutData = 1;
                        UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                    }
        	    }
        	    break ;
     		default:
    			usb_error();
    			break;
        }
        break;
        
        case PCSYNC_WRITE_FRAME:
        {
            MMP_BOOL    last_pkt ;
            MMP_UBYTE   *curIBCPtr ,*desc_ptr ;
            
            MMP_USHORT sId = USBREQ_I(gsUsbCtrlReq) ;
            
            desc_ptr = (MMP_UBYTE *)&gsFrameDesc_t[sId] ;
            last_pkt = pcsync_ep0_rx(desc_ptr);
            
            if(last_pkt) {
                MMP_ULONG frameSeq;
                
                gbCurStreamId = sId ;
                USBOutData = 0;
                USBOutPhase = 0;
                UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY|EP0_DATAEND_BIT);
                curIBCPtr = pcsync_get_cur_ibc_buffer(sId) ;
                frameSeq = gsFrameDesc_t[sId].dwFrameSeq ;
                
     			pcsync_init_bulk_transfer(&gPCSyncBulkEpRx,PCSYNC_EP_ADDR,curIBCPtr,gsFrameDesc_t[sId].dwPayloadSize,PCSYNC_DIRECTION_BULK_OUT,pcsync_write_frame_end); //REMOVE FOR DMA TEST
                //dbg_printf(DBG_LEVEL,"frame wr:%d,sid:%d,size:%d\r\n",frameSeq,sId,gsFrameDesc_t[sId].dwPayloadSize);
            }
            else {
                dbg_printf(3,"Err Wr.Frame\r\n");
                USBOutPhase = VENDOR_CMD;
                USBOutData  = 1 ;
                UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
            }
            
        }
        break;
        case PCSYNC_READ_FRAME:
        {}
        break;
        default:
            usb_error();
            break ;
    	
	}
}

//*****************************************************************
// function : pcsync_vendor_request_data_in
// input    : none
// output   : none
//*****************************************************************
void pcsync_vendor_request_data_in(void)
{
   // dbg_printf(3,"v.datain\r\n");
}

MMP_ULONG glProcessTime ;
extern MMP_ULONG PTS ;


void pcsync_start_preview(void)
{
extern H264_FORMAT_TYPE gbCurH264Type;
    extern UVC_VSI_INFO    gCurVsiInfo[];
    extern MMP_USHORT      UVCTxFIFOSize[];
    extern MMP_UBYTE STREAM_EP_H264 ;
    UVC15_VIDEO_CONFIG *config ;
    STREAM_SESSION *ss ;
    MMP_BOOL sim_h264 = MMP_FALSE ;
    
    config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(0) ;
    sim_h264 = gbWorkingStream[0] & gbWorkingStream[1] ;
    UVCX_SetSimulcastH264(sim_h264);
    STREAM_EP_H264 = 0 ;
    gCurVsiInfo[STREAM_EP_H264].bInterface  = 1 ;
    if(UVC_VCD()==bcdVCD15) {
        STREAM_EP_H264 = 1;
        gCurVsiInfo[STREAM_EP_H264].bInterface = 2 ;
    }
    gCurVsiInfo[STREAM_EP_H264].bFormatIndex = FRAME_BASE_H264_INDEX + 1 ;
    gCurVsiInfo[STREAM_EP_H264].bFrameIndex = GetResIndexBySize(config->res.wWidth,config->res.wHeight) + 1;
    gbCurH264Type = FRAMEBASE_H264 ; 
    USB_SetH264Resolution(PCAM_API,gCurVsiInfo[STREAM_EP_H264].bFrameIndex-1 );
    ss = MMPF_Video_GetStreamSessionByEp(STREAM_EP_H264);
    MMPF_Video_InitStreamSession(STREAM_EP_H264,SS_TX_BY_DMA | SS_TX_STREAMING,UVCTxFIFOSize[STREAM_EP_H264]);
    gbPcSyncPreview = MMP_TRUE ;
    glPcSyncFrameSeq[0] = 0 ;
    glPcSyncFrameSeq[1] = 0 ;
    glAccEncBytes[0]=glAccEncBytes[1]=glAccEncBytes[2]= 0;
    
    glProcessTime = usb_vc_cur_sof();
    UVCX_SetH264EncodeBufferMode(FRAME_MODE);
    
    PTS = 0 ;

    usb_uvc_start_preview(ss->ep_id,0);
}

void pcsync_stop_preview(void)
{
    extern UVC_VSI_INFO    gCurVsiInfo[];
    extern MMP_USHORT      UVCTxFIFOSize[];
    STREAM_SESSION *ss ;
    ss = MMPF_Video_GetStreamSessionByEp(STREAM_EP_H264);
    MMPF_IBC_SetStoreEnable(MMPF_IBC_PIPE_1,MMP_TRUE);
    usb_uvc_stop_preview(ss->ep_id,0);
    gbPcSyncPreview = MMP_FALSE ;
    dbg_printf(3,"processed time : %d secs\r\n",(usb_vc_cur_sof() - glProcessTime) / 1000);
}

MMP_BOOL pcsync_is_preview(void)
{
    return gbPcSyncPreview ;
}

extern  MMP_UBYTE   gbIBCLinkEncId[MMPF_IBC_PIPE_MAX] ;
extern	MMP_ULONG   glPreviewBufAddr[MMPF_IBC_PIPE_MAX][4] ;
extern  MMP_UBYTE   gbCurIBCBuf[MMPF_IBC_PIPE_MAX] ;
extern  MMP_UBYTE	gbExposureDoneFrame[MMPF_IBC_PIPE_MAX];
extern  MMP_UBYTE   gbPreviewBufferCount[MMPF_IBC_PIPE_MAX] ;
extern	MMP_ULONG   glPreviewUBufAddr[MMPF_IBC_PIPE_MAX][4];
extern	MMP_ULONG   glPreviewVBufAddr[MMPF_IBC_PIPE_MAX][4];

MMP_UBYTE *pcsync_get_cur_ibc_buffer(MMP_UBYTE sId)
{

    MMP_UBYTE pipe = (sId==0)?1:0 ;
    return (MMP_UBYTE *)glPreviewBufAddr[pipe][gbCurIBCBuf[pipe]];
}

void pcsync_encode_frame(MMP_UBYTE sId)
{
    MMPF_VIDENC_BITRATE_CTL BrCtl;
    MMP_ULONG br ;
    
    MMP_UBYTE usIBCPipe = (sId==0)?1:0 ;
    if(!pcsync_is_preview()) {
        return ;
    }
    MMPF_VIDENC_GetParameter(sId, MMPF_VIDENC_ATTRIBUTE_BR, (void*)&BrCtl);
    br = glAccEncBytes[sId]*8 / (glPcSyncFrameSeq[sId]+1 );
    br = br * (gbConverTest30FPS?30:15) ;
    dbg_printf(3,"pcsync.encode.f %d,sid:%d,br:%d\r\n",glPcSyncFrameSeq[sId],sId,br);
  
    MMPF_VIDENC_TriggerFrameDone(gbIBCLinkEncId[usIBCPipe], //main enc trig ,orig = 0 
                                &(gbExposureDoneFrame[usIBCPipe]),
                                &(gbCurIBCBuf[usIBCPipe]),
                                gbPreviewBufferCount[usIBCPipe],
                                glPreviewBufAddr[usIBCPipe],
                                glPreviewUBufAddr[usIBCPipe],
                                glPreviewVBufAddr[usIBCPipe]);
    
}


MMP_ULONG pcsync_get_frame(MMP_ULONG *frame_size,MMP_USHORT *sId)
{
    PCSYNC_VIDEO_FRAME  vframe ;
    FRAME_PAYLOAD_HEADER *frame_ph ;
    MMP_ULONG frameptr = 0;
    MMP_USHORT sid = 0;
    *frame_size = 0 ;
    
    frame_ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_1);
    
    //frame_ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_NextRdPtr(PIPE_1);
    
    if(frame_ph->UVC_H264_PH.dwPayloadSize) {
        frameptr = (MMP_ULONG)frame_ph ;
        frameptr = frameptr + FRAME_PAYLOAD_HEADER_SZ -  sizeof(PCSYNC_VIDEO_FRAME)  ;
        
        //vframe.dwPayloadSize = frame_ph->UVC_H264_PH.dwPayloadSize ;
        sid = UVC_PH_SID(frame_ph->UVC_H264_PH.dwFrameRate) ;
        if(sid==0) {
            vframe.wStreamType = ST_H264 ;
            
        }
        else if (sid==1) {
            vframe.wStreamType = ST_H264_2 ;
        }
        *sId = sid ;
         vframe.wVersion = 0x0090 ;
        vframe.wHeaderLen = UVC_H264_PH_LEN ;
        vframe.wImageWidth = frame_ph->UVC_H264_PH.wImageWidth ;
        vframe.wImageHeight = frame_ph->UVC_H264_PH.wImageHeight ;
        vframe.dwFrameSeq = glPcSyncFrameSeq[sid] ;
        vframe.dwTimeStamp = frame_ph->UVC_H264_PH.dwTimeStamp ;
        vframe.wFrameRate = 1;
        vframe.dwPayloadSize = frame_ph->UVC_H264_PH.dwPayloadSize ;
        vframe.dwReserved = 0;//frame_ph->UVC_H264_PH.dwPayloadSize ;
        *(PCSYNC_VIDEO_FRAME *)frameptr =vframe;
        *frame_size = vframe.dwPayloadSize + sizeof(PCSYNC_VIDEO_FRAME) ;
        dbg_printf(3,"frame rd : %d,sid:%d,size:%d,w,h:%d,%d\r\n",vframe.dwFrameSeq,sid,vframe.dwPayloadSize,vframe.wImageWidth,vframe.wImageHeight);
        glAccEncBytes[*sId] += frame_ph->UVC_H264_PH.dwPayloadSize ;
    }
    #if SIM_TEST 
    {
        frame_ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_NextRdPtr(PIPE_1);  
        //frame_ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_1);  
        sid = UVC_PH_SID(frame_ph->UVC_H264_PH.dwFrameRate) ;  
        if(frame_ph->UVC_H264_PH.dwPayloadSize) {
            dbg_printf(3,"<nextframe :sid:%d,size:%d>\r\n",sid,frame_ph->UVC_H264_PH.dwPayloadSize);
            
        }
    }
    #endif
    return frameptr ;
}

void pcsync_read_frame_end(void)
{
    //dbg_printf(3,"tx.frame end : %d\r\n",glPcSyncFrameSeq);
    MMPF_Video_UpdateRdPtr(PIPE_1);
    #if SIM_TEST
    MMPF_Video_UpdateRdPtr(PIPE_1);
    #endif
}

void pcsync_move_data_end(void)
{
    pcsync_encode_frame(1) ;
}

extern MMP_USHORT SetH264BitRate(EU_RC rc,MMP_UBYTE sid,MMP_UBYTE tid,MMP_UBYTE cur_layers,MMP_ULONG *br);
void pcsync_write_frame_end(void)
{
/*
UC Config Mode 1 : 
RC 500  240p : 104 ~ 190 overflow 
RC 1000 240p : 186 ~ 236 overflow 

RC 1000 1080 : 449 ~ 498
RC 500  1080 : 555 ~ 578

RC 1000 360p : 599 ~ 1000


*/
    MMP_UBYTE layer = 0;
    UVC15_VIDEO_CONFIG *config ;
    MMP_ULONG ratio = 100,frame_n = 0;
    config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(gbCurStreamId) ;
    //dbg_printf(3,"rx.frame end : %d,id:%d\r\n",glPcSyncFrameSeq,gbCurStreamId);
    if(gbConverTest) {

        MMP_ULONG bitrate[MAX_NUM_TMP_LAYERS] = {0,0,0};
        if( gbConverTest30FPS ) {
            
            if (gsFrameDesc_t[gbCurStreamId].dwFrameSeq == 150) {
                ratio = 90 ;
                frame_n = 150 ;
            }
            else if (gsFrameDesc_t[gbCurStreamId].dwFrameSeq == 300) {
                ratio = 75 ;
                frame_n = 300 ;
            }
            else if (gsFrameDesc_t[gbCurStreamId].dwFrameSeq == 450) {
            #if 0
                ratio = 45 ; // > 240p
            #else
                ratio = 50;
            #endif    
                frame_n = 450 ;
            }
        }
        else {
            
            
                
        
            if (gsFrameDesc_t[gbCurStreamId].dwFrameSeq == 75) {
                ratio = 90 ;
                frame_n = 75 ;
            }
            else if (gsFrameDesc_t[gbCurStreamId].dwFrameSeq == 150) {
                ratio = 75 ;
                frame_n = 150 ;
            }
            else if (gsFrameDesc_t[gbCurStreamId].dwFrameSeq == 225) {
                ratio = 50;
                frame_n = 225 ;
            }
        }
        
        if(ratio < 100) {
            #if 1
            ratio = (ratio * 95) / 100 ; // ucconfig mode 1 360 p RC 1000 ms FAIL
            #else
            
            #endif
            for(layer=0;layer < gbSvcLayers ; layer++) {
                bitrate[layer] = ( config->dwAverageBitRate[layer] * ratio) /100;
            
            }
            dbg_printf(3,"Frame #%d(#%d layers), %d percent bitrate:%d,%d,%d\r\n",frame_n,gbSvcLayers,ratio,bitrate[0],bitrate[1],bitrate[2]);
            if(gbSvcLayers==1) {
                SetH264BitRate(RC_GVBR,gbCurStreamId,0,gbSvcLayers,bitrate);
            }
            else {
                SetH264BitRate(RC_VBR,gbCurStreamId,SVC_WILDCARD_MASK,gbSvcLayers,bitrate);
            }    
        }
    }
    # if 1
    {
        if(gbConverTest30FPS) {
            PTS += 33/*( (gsFrameDesc_t[gbCurStreamId].dwFrameSeq + 1) * 33) +*/+ (((gsFrameDesc_t[gbCurStreamId].dwFrameSeq + 1) % 3) ? 0 : 1 );
        }
        else {
            
            PTS += 66 /*((gsFrameDesc_t[gbCurStreamId].dwFrameSeq + 1) * 66) +*/+ (((gsFrameDesc_t[gbCurStreamId].dwFrameSeq + 1) % 6) ? 0 : 1);
        }
       // dbg_printf(3,"PTS[%d] : %d ms\r\n",gsFrameDesc_t[gbCurStreamId].dwFrameSeq,PTS);
    }
    #endif
    pcsync_encode_frame(gbCurStreamId) ;
    #if SIM_TEST
    {
        MMP_ULONG ptrIBC0,ptrIBC1 ;
        ptrIBC1 = (MMP_ULONG)pcsync_get_cur_ibc_buffer(0);
        ptrIBC0 = (MMP_ULONG)pcsync_get_cur_ibc_buffer(1);
        MMPF_DMA_MoveData1(ptrIBC1,ptrIBC0,(gsFrameDesc_t[gbCurStreamId].wImageWidth*gsFrameDesc_t[gbCurStreamId].wImageHeight * 3)/2,0,pcsync_move_data_end);
    }
    #endif
}

MMP_BOOL gbLyncCertSEIOn ;
MMP_BOOL pcsync_h264_to_uvc15_h264(UVC15_VIDEO_CONFIG *uvc15_h264,PCSYNC_H264_CONFIG *pcsync_h264)
{

    MMP_BOOL sei_on = MMP_FALSE ;
    MMP_BOOL convergence_test = MMP_FALSE ;
    MMP_UBYTE sid ;
    VIDEO_PROBE *commit_info = UVC15_GetH264CommitDesc();
    int i ;
    
    if(pcsync_h264->bSvcLayers & 0x80) {
        convergence_test = MMP_TRUE ;  
        pcsync_h264->bSvcLayers &= ~0x80 ;  
    }
    //if(pcsync_h264->bSvcLayers & 0x40) {
    //    sei_off = MMP_TRUE ;  
    //    pcsync_h264->bSvcLayers &= ~0x40 ;  
    //}
#if 0
    sei_on = MMP_TRUE ;
#endif  
    if( ( pcsync_h264->bRateControlMode!=RC_CQP ) /*&&(!convergence_test)*/) {
        sei_on = MMP_TRUE ;
    }  
    gbLyncCertSEIOn = sei_on ;
    
    if(pcsync_h264->wLayerOrViewID==0) {
        sid = 0 ;
    } else {
        sid = 1 ;
    }
    uvc15_h264->wLayerOrViewID  = SVC_LAYER_ID(0,0,SVC_WILDCARD_MASK,sid,0);
    
dbg_printf(0,"##s%d,svc layers :%d,convergence test:%d\r\n",pcsync_h264->wLayerOrViewID,pcsync_h264->bSvcLayers,convergence_test);

    if( (pcsync_h264->bSvcLayers )&&(pcsync_h264->bSvcLayers <= MAX_NUM_TMP_LAYERS)) {
        commit_info->bmLayoutPerStream[sid] = COMMIT_LAYOUT(pcsync_h264->bSvcLayers) ;
    }
dbg_printf(0,"##Stream Layout[%d]:%d\r\n",sid, commit_info->bmLayoutPerStream[sid]);
    
    uvc15_h264->res.wWidth  = pcsync_h264->res.wWidth ;
    uvc15_h264->res.wHeight = pcsync_h264->res.wHeight;
    
    uvc15_h264->toolSet.wProfile            = pcsync_h264->toolSet.wProfile ;
    uvc15_h264->toolSet.wConstrainedToolset = pcsync_h264->toolSet.wConstrainedToolset ;
    uvc15_h264->toolSet.bmSetting           = pcsync_h264->toolSet.bmSetting ;
    
    uvc15_h264->dwFrameInterval                 = pcsync_h264->dwFrameInterval ;
dbg_printf(0,"##s%d,Frame Inteval:%d\r\n",sid, uvc15_h264->dwFrameInterval);
    uvc15_h264->sliceMode.wSliceMode            = pcsync_h264->sliceMode.wSliceMode ;
    uvc15_h264->sliceMode.wSliceConfigSetting   = pcsync_h264->sliceMode.wSliceConfigSetting ;
#if 1
    uvc15_h264->sliceMode.wSliceConfigSetting     = 1 ;
#endif
    
    uvc15_h264->bRateControlMode                = pcsync_h264->bRateControlMode ;
    if(sid==0) {
        commit_info->bmRateControlModes &= ~0x000F ; 
        commit_info->bmRateControlModes |= COMMIT_RC(uvc15_h264->bRateControlMode,0,0,0);
    } 
    else {
        commit_info->bmRateControlModes &= ~0x00F0 ; 
        commit_info->bmRateControlModes |= COMMIT_RC(0,uvc15_h264->bRateControlMode,0,0);
    }
    
    uvc15_h264->syncRefFrameCtrl.bSyncFrameType         = pcsync_h264->syncRefFrameCtrl.bSyncFrameType  ;
    uvc15_h264->syncRefFrameCtrl.wSyncFrameInterval     = pcsync_h264->syncRefFrameCtrl.wSyncFrameInterval;
    uvc15_h264->syncRefFrameCtrl.bGradualDecoderRefresh = pcsync_h264->syncRefFrameCtrl.bGradualDecoderRefresh ;
    
    uvc15_h264->bLevelIDC                       = pcsync_h264->bLevelIDC ;
    
    uvc15_h264->bUpdate = 1 ;
    uvc15_h264->bUpdatetId = MAX_NUM_TMP_LAYERS - 1 ;
    
    uvc15_h264->dwPeakBitRate = pcsync_h264->dwPeakBitRate  ;
    
    for(i=0;i<MAX_NUM_TMP_LAYERS;i++) {
        if(pcsync_h264->bSvcLayers > 1 ){
        #if 1
        /*
            if (uvc15_h264->res.wWidth * uvc15_h264->res.wHeight <= 640*360) {
                uvc15_h264->dwAverageBitRate[i]  = (pcsync_h264->dwAverageBitRate[i] * 96 ) / 100; // Funny / Fucky
            }
            else {
               uvc15_h264->dwAverageBitRate[i]  = (pcsync_h264->dwAverageBitRate[i] * 95 ) / 100; // Funny / Fucky
            
            }*/
            if( (pcsync_h264->dwLeakyBucket[i]==1000 ) && (uvc15_h264->res.wWidth * uvc15_h264->res.wHeight == 1280*720)) {
                uvc15_h264->dwAverageBitRate[i]  = (pcsync_h264->dwAverageBitRate[i] * 935 ) / 1000; 
            }
            else {
                uvc15_h264->dwAverageBitRate[i]  = (pcsync_h264->dwAverageBitRate[i] * 95 ) / 100; 
            }
        #else
            uvc15_h264->dwAverageBitRate[i]  = pcsync_h264->dwAverageBitRate[i] ; 
        #endif    
        }
        else {
            uvc15_h264->dwAverageBitRate[i]  = pcsync_h264->dwAverageBitRate[i] ;
        }
        #if 0 // reduce the rounding error for SVC layers
        uvc15_h264->dwCPBsize[i]         = EU_LBSize2CPBSize( pcsync_h264->dwLeakyBucket[i],pcsync_h264->dwAverageBitRate[i]) ;
        #else
        uvc15_h264->dwCPBsize[i]         = pcsync_h264->dwLeakyBucket[i] ;
        #endif
        dbg_printf(0,"##CPBSize : %d,Br:%d\r\n",uvc15_h264->dwCPBsize[i],  uvc15_h264->dwAverageBitRate[i]);
        
        uvc15_h264->QPCtrl[i].wQpPrime_I = pcsync_h264->QPCtrl[i].wQpPrime_I;
        uvc15_h264->QPCtrl[i].wQpPrime_P = pcsync_h264->QPCtrl[i].wQpPrime_P;
        uvc15_h264->QPCtrl[i].wQpPrime_B = pcsync_h264->QPCtrl[i].wQpPrime_B;
        uvc15_h264->bPriority[i]         = pcsync_h264->bPriority[i] ;
        uvc15_h264->qpRange[i].minQP     = pcsync_h264->qpRange[i].minQP  ;
        uvc15_h264->qpRange[i].maxQP     = pcsync_h264->qpRange[i].maxQP  ;
        
        uvc15_h264->bUpdateLayerMap[i]   = 1 ;
        dbg_printf(0,"##Layer #%d : QP(I,P,B)=(%d,%d,%d)\r\n",i,uvc15_h264->QPCtrl[i].wQpPrime_I,uvc15_h264->QPCtrl[i].wQpPrime_P,uvc15_h264->QPCtrl[i].wQpPrime_B);

    }
    return convergence_test ;
}

void pcsync_dump_h264_config(MMP_USHORT streamid,PCSYNC_H264_CONFIG *config_in)
{
    PCSYNC_H264_CONFIG *config ;
    if(streamid > bMaxStreamId) {
        return  ;
    }   
    if(!config_in) {
    //    config = (UVC15_VIDEO_CONFIG *)&gsCurUVC15H264Config[streamid] ;
        return ;
    }
    else {
        config = config_in ;
    }
    dbg_printf(3,"-----stream %d config-----\r\n",streamid);
    dbg_printf(3,"wLayerOrViewID:%x\r\n",config->wLayerOrViewID);
    dbg_printf(3,"res : (%d,%d)\r\n",config->res.wWidth,config->res.wHeight);
    dbg_printf(3,"toolSet,(profile : %x,toolset:%x,bmSetting:%x\r\n",config->toolSet.wProfile,
                                                                   config->toolSet.wConstrainedToolset,
                                                                   config->toolSet.bmSetting );
    dbg_printf(3,"dwFrameInterval : %d\r\n",config->dwFrameInterval);
    dbg_printf(3,"sliceMode : wSliceMode:%d, wSliceConfigSetting :%d\r\n",config->sliceMode.wSliceMode,
                                                                         config->sliceMode.wSliceConfigSetting );
    dbg_printf(3,"bRateControlMode : %d\r\n",config->bRateControlMode );
    dbg_printf(3,"dwAverageBitRate : (%d,%d,%d)\r\n",config->dwAverageBitRate[0],
                                                     config->dwAverageBitRate[1],
                                                     config->dwAverageBitRate[2] );
                                                     
    dbg_printf(3,"dwLeakyBucket : (%d,%d,%d)\r\n",config->dwLeakyBucket[0],
                                                  config->dwLeakyBucket[1],
                                                  config->dwLeakyBucket[2] );
    dbg_printf(3,"dwPeakBitRate : %d\r\n",config->dwPeakBitRate) ;
    dbg_printf(3,"QPCtrl[0] : ( %d,%d,%d)\r\n",config->QPCtrl[0].wQpPrime_I,
                                                config->QPCtrl[0].wQpPrime_P,
                                                config->QPCtrl[0].wQpPrime_B) ;
                                                
    dbg_printf(3,"QPCtrl[1] : ( %d,%d,%d)\r\n",config->QPCtrl[1].wQpPrime_I,
                                                config->QPCtrl[1].wQpPrime_P,
                                                config->QPCtrl[1].wQpPrime_B) ;
                                                
    dbg_printf(3,"QPCtrl[2] : ( %d,%d,%d)\r\n",config->QPCtrl[2].wQpPrime_I,
                                                config->QPCtrl[2].wQpPrime_P,
                                                config->QPCtrl[2].wQpPrime_B) ;
                                                
    dbg_printf(3,"syncRefFrameCtrl :(type:%d,interval:%d,refresh:%d\r\n",config->syncRefFrameCtrl.bSyncFrameType,
                                                                        config->syncRefFrameCtrl.wSyncFrameInterval,
                                                                        config->syncRefFrameCtrl.bGradualDecoderRefresh ) ;
                                                                        
    dbg_printf(3,"bPriority : (%d,%d,%d)\r\n",config->bPriority[0],
                                              config->bPriority[1],
                                              config->bPriority[2] );
    dbg_printf(3,"qpRange[0] : (%d,%d)\r\n",config->qpRange[0].minQP,
                                            config->qpRange[0].maxQP);
    dbg_printf(3,"qpRange[1] : (%d,%d)\r\n",config->qpRange[1].minQP,
                                            config->qpRange[1].maxQP);
    dbg_printf(3,"qpRange[2] : (%d,%d)\r\n",config->qpRange[2].minQP,
                                            config->qpRange[2].maxQP);
                                            
    dbg_printf(3,"bLevelIDC : %d\r\n",config->bLevelIDC);
                                            
}


void pcsync_dump_frame_desc(PCSYNC_VIDEO_FRAME *desc)
{
    dbg_printf(3," wVersion      : %d\r\n",desc->wVersion);                   
    dbg_printf(3," wHeaderLen    : %d\r\n",desc->wHeaderLen);                 
    dbg_printf(3," wStreamType   : %d\r\n",desc->wStreamType); // 41,42,43,44 
    dbg_printf(3," wImageWidth   : %d\r\n",desc->wImageWidth);                
    dbg_printf(3," wImageHeight  : %d\r\n",desc->wImageHeight);                
    dbg_printf(3," wFrameRate    : %d\r\n",desc->wFrameRate);                 
    dbg_printf(3," dwFrameSeq    : %d\r\n",desc->dwFrameSeq);                 
    dbg_printf(3," dwTimeStamp   : %d\r\n",desc->dwTimeStamp);                
    dbg_printf(3," dwFlag        : %d\r\n",desc->dwFlag);                     
    dbg_printf(3," dwReserved    : %d\r\n",desc->dwReserved);                 
    dbg_printf(3," dwPayloadSize : %d\r\n",desc->dwPayloadSize);              
}

#endif //SUPPORT_PCSYNC
