/** @addtogroup MMPF_USB
@{
*/

#define exUSBPUB
#include "includes_fw.h"
#include "lib_retina.h"
#include "config_fw.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"

#include "mmp_lib.h" // for u_short/u_char/...
#include "mmpf_usbuac.h"
#include "mmpf_usbuvc.h"
#include "mmpf_spi.h"
#include "mmpf_usbvend.h"
#include "mmp_reg_usb.h"

MMP_UBYTE USBDescBufAddr[0x800];
extern  MMP_UBYTE gbUsbHighSpeed;
extern  MMP_UBYTE  USBOutData;
extern  UsbCtrlReq_t gsUsbCtrlReq ;
MMP_USHORT   UVCTxFIFOSize[2];


#if defined(USB_FW)||(USB_EN)

#pragma O0


#if (SUPPORT_UVC_FUNC==1)
//*********************************************************************
// function : usb_desc_init
// input    : none
// output   : none
//*********************************************************************

void usb_audio_set_max_pktsize(MMP_ULONG sr,MMP_BOOL sync_mode)
{
    MMP_USHORT async = 1 ;
    MMP_USHORT pktsize = sr / 1000 ;
    if(sync_mode) {
        async = 0 ;
    }
    pktsize = ( pktsize  + async) * AUDIN_CHANNEL * 2 ; // to bytes, +1 is for async mode
    dbg_printf(3,"%d Hz, MaxPkt size:%d\r\n",sr,pktsize);
    UsbWriteTxMaxP(PCCAM_AU_EP_ADDR, pktsize);
}


void usb_ep_init(void)
{
#define FORCE_FS_USB 0
    MMP_USHORT csr,iso_mode=SET_TX_ISO;      
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA; 
    
#if FORCE_FS_USB==0
    pUSB_CTL->USB_POWER |= 0x20;  // high speed enable
#else
    pUSB_CTL->USB_POWER &= ~0x20; // full speed enable
#endif

#if USB_UVC_BULK_EP
    iso_mode = 0 ;
#endif
    if(pUSB_CTL->USB_POWER & HS_MODE_BIT) {
        gbUsbHighSpeed = 1; 
        if(VIDEO_EP_END()==1) {        
#if SUPPORT_UAC
            UsbWriteTxMaxP(PCCAM_AU_EP_ADDR, AU_EP_MAX_PK_SIZE_HS);
#endif
        } else {
            // EP2 for H264 stream
            UsbWriteTxMaxP(PCCAM_TX_EP2_ADDR, FS_TX_MAX_PK_SIZE);
        }
        UsbWriteTxMaxP(PCCAM_TX_EP1_ADDR, HS_TX_MAX_PK_SIZE);
#if USB_UVC_BULK_EP
        UVCTxFIFOSize[0] = HS_TX_MAX_PK_SIZE ;
#else        
        UVCTxFIFOSize[0] = HS_TX_MAX_PK_SIZE * TX_PER_FRAME ;
#endif        

        UVCTxFIFOSize[1] = FS_TX_MAX_PK_SIZE * 1; // same as full speed max pkt size
        UsbWriteTxMaxP(PCCAM_EX_EP_ADDR, EX_EP_MAX_PK_SIZE);
        // One video endpoint
        if(VIDEO_EP_END()==1) {  
    #if SUPPORT_UAC
            csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR);
            csr = csr & TXCSR_RW_MASK;
            UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr | SET_TX_MODE | SET_TX_ISO); // ISO 0x4000
    #endif
        } else {
        // two video endpoint
            csr = UsbReadTxEpCSR(PCCAM_TX_EP2_ADDR);
            csr = csr & TXCSR_RW_MASK;
            UsbWriteTxEpCSR(PCCAM_TX_EP2_ADDR, csr | SET_TX_MODE | iso_mode ); // ISO 0x4000
        }

        csr = UsbReadTxEpCSR(PCCAM_TX_EP1_ADDR);
        csr = csr & TXCSR_RW_MASK;
        UsbWriteTxEpCSR(PCCAM_TX_EP1_ADDR, csr | SET_TX_MODE | iso_mode); // ISO 0x4000
        
        csr = UsbReadTxEpCSR(PCCAM_EX_EP_ADDR);
        csr = csr & TXCSR_RW_MASK;
        UsbWriteTxEpCSR(PCCAM_EX_EP_ADDR, csr | SET_TX_MODE);
        
    #if SUPPORT_PCSYNC
        csr = UsbReadTxEpCSR(PCSYNC_EP_ADDR);
        csr = csr & TXCSR_RW_MASK;
        UsbWriteTxEpCSR(PCSYNC_EP_ADDR, csr | SET_TX_MODE);

        UsbWriteRxMaxP(PCSYNC_EP_ADDR, PCSYNC_EP_MAX_PK_SIZE);
        UsbWriteTxMaxP(PCSYNC_EP_ADDR, PCSYNC_EP_MAX_PK_SIZE);
        
		csr = UsbReadRxEpCSR(PCSYNC_EP_ADDR);
    	csr = csr & RXCSR_RW_MASK;
		csr = csr & ~RX_SET_ISO;
    	UsbWriteRxEpCSR(PCSYNC_EP_ADDR,csr|CLEAR_RX_RXPKTRDY);
    
    #endif    
        
    } else {
        gbUsbHighSpeed = 0;
        if(VIDEO_EP_END()==1) {            
    #if SUPPORT_UAC
            UsbWriteTxMaxP(PCCAM_AU_EP_ADDR, AU_EP_MAX_PK_SIZE_FS);
    #endif
        } else {
            UsbWriteTxMaxP(PCCAM_TX_EP2_ADDR, FS_TX_MAX_PK_SIZE);
        }
        UsbWriteTxMaxP(PCCAM_TX_EP1_ADDR, FS_TX_MAX_PK_SIZE);
        UVCTxFIFOSize[0] = FS_TX_MAX_PK_SIZE    ;
        UVCTxFIFOSize[1] = FS_TX_MAX_PK_SIZE    ;
        UsbWriteTxMaxP(PCCAM_EX_EP_ADDR, EX_EP_MAX_PK_SIZE);
        // One video endpoint
        if(VIDEO_EP_END()==1) {
#if SUPPORT_UAC
            csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR );
            csr = csr & TXCSR_RW_MASK;
            UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr  | SET_TX_MODE | SET_TX_ISO); // ISO 0x4000
#endif
        } else {
        // 2 video endpoint
            csr = UsbReadTxEpCSR(PCCAM_TX_EP2_ADDR);
            csr = csr & TXCSR_RW_MASK;
            UsbWriteTxEpCSR(PCCAM_TX_EP2_ADDR, csr | SET_TX_MODE | iso_mode); // ISO 0x4000
        }
        csr = UsbReadTxEpCSR(PCCAM_TX_EP1_ADDR);
        csr = csr & TXCSR_RW_MASK;
        UsbWriteTxEpCSR(PCCAM_TX_EP1_ADDR, csr | SET_TX_MODE | iso_mode); // ISO 0x4000
        csr = UsbReadTxEpCSR(PCCAM_EX_EP_ADDR);
        csr = csr & TXCSR_RW_MASK;
        UsbWriteTxEpCSR(PCCAM_EX_EP_ADDR, csr | SET_TX_MODE);
    }
    // disable non-used endpoint
    pUSB_DMA->USB_EP_DISABLE = PCCAM_EP_DISABLE_MAP ;
    
    
    //dbg_printf(3,"usb speed : %s\r\n",gbUsbHighSpeed?"high":"full" );
    
}

//*********************************************************************
// function : A810L_InitUSB
// input    : none
// output   : none
//*********************************************************************
void A810L_InitUSB(void)
{
    u_short csr;      
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_USHORT read_data;

    // varible initial
    USBOutData = 0;
    USBInPhase = 0;
    USBOutPhase = 0;
    USBConfig1Len = 0;//CONFIG_DESCRIPTOR_LEN;

    usb_ep_init();
    pUSB_CTL->USB_TX_INT_EN = EP0_INT_BIT | EP1_TX_INT_BIT | EP2_TX_INT_BIT;
    pUSB_CTL->USB_RX_INT_EN = 0;
    pUSB_CTL->USB_INT_EVENT_EN = RESET_INT_BIT /*| SOF_INT_BIT*/ | SUSPEND_INT_BIT | RESUME_INT_BIT ;
    
#if SUPPORT_PCSYNC
    pUSB_CTL->USB_TX_INT_EN |= EP5_TX_INT_BIT ;
    pUSB_CTL->USB_RX_INT_EN |= EP5_RX_INT_BIT ;
#endif
    

    //USBAlterSetting = 0x00;
    gsUsbCtrlReq.wAlterSetting = 0x00;
    gsUsbCtrlReq.wInterface    = 0x00;
    //USBConfig = 0x00;
   // USBInterface = 0x00;
   // USBEndpoint = 0x00;
   // USBFrameNum = 0x00;
   // USBDevice = 0x00;
    gsUsbCtrlReq.bConfig   = 0x00;
    gsUsbCtrlReq.bDevice   = 0x00;
    gsUsbCtrlReq.bEndPoint = 0x00;
    gsUsbCtrlReq.wSyncFrame = 0 ;
    uvc_init(1,0);
#if SUPPORT_UAC==1    
    uac_init();
#endif   
   // USBConfigDescLen = USBConfig1Len;

#if (JTAG == 1) // V06 : force usb enum again if using ICE to debug
     usb_sw_enum();
#endif
    
}

//*********************************************************************
// function : usb_sw_enum
// input    : none
// output   : none
//*********************************************************************
#if (JTAG == 1) 
void usb_sw_enum(void)
{
    MMP_UBYTE tmp;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;


    pUSB_CTL->USB_POWER &= ~0x41; // 0x40

    //for(tmp = 0;tmp < 100;tmp++)
    //    ;
    RTNA_WAIT_MS(10);
    
    pUSB_CTL->USB_POWER |= 0x41; // |= 0x40
}
#endif


#if (USB_PHY_TEST_MODE_EN == 1)
	MMP_UBYTE spi_read[]=
	{
		// read 20, 0a, 2001                        
		0x01,	// CS_N = 1  
		0x00,                                                 
		0x02,	// device addr bit0                   
		0x00,	                                     
		0x02,   //	bit1
		0x04,                                        
		0x06,	// bit2                               
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                                                               
		0x04,
		0x06,
		        // reg_addr = 0a                                       
		0x00,	                                     
		0x02,	// reg_addr bit0                      
		0x00,	                                     
		0x02,	// bit 1                              
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02, 
		0x00,
		0x02,                                       

		0x00,	                                     
		0x02,	// data0  bit0   
		//
		//read A013[0]
		0x00,	                                     
		0x02,	// bit 1                              
		//read A013[0]
		0x00,                                        
		0x02,         
		//read A013[0]                               
		0x00,                                        
		0x02,                                        
		//read A013[0]                               
		0x04,                                        
		0x06,                                        
		//read A013[0]                               
		0x04,                                        
		0x06,                                        
		//read A013[0]                               
		0x00,                                        
		0x02,
		//read A013[0]                               
		0x04,
		0x06,                                        
		//read A013[0]                               

		0x04,	                                     
		0x06,	// data1  bit0                        
		//read A013[0]                               
		0x04,	                                     
		0x06,	// bit 1                              
		//read A013[0]                               
		0x04,                                        
		0x06,                                        
		//read A013[0]                               
		0x04,                                        
		0x06,                                        
		//read A013[0]                               
		0x04,                                        
		0x06,                                        
		//read A013[0]                               
		0x04,                                        
		0x06,
		//read A013[0]                               
		0x04,
		0x06,                                        
		//read A013[0]                               
		0x04,                                        
		0x06,                                        
		//read A013[0]                               

		0x00,
		0x00,                                        
		0x01,	// CS_N =1                            

	};


	MMP_UBYTE spi_write[]=
	{
		// set 20, 0A, 2001                          
		0x01,	// CS_N = 1   
		0x00,                        
		0x02,	// device addr bit0                   
		0x00,	                                     
		0x02,   //	bit1                         
		0x04,                                        
		0x06,	// bit2                                
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		                                           
		0x00,	                                     
		0x02,	// reg_addr bit0                      
		0x00,	                                     
		0x02,	// bit 1                              
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02, 
		0x00,
		0x02,                                       


		0x00,	                                     
		0x02,	// data0  bit0   
		0x00,	                                     
		0x02,	// bit 1                              
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,
		0x00,                                        
		0x02,                                       
		0x00,                                        
		0x02,
		0x00,                                        
		0x02,                                       

		0x00,	                                     
		0x02,	// data0  bit0   
		0x00,	                                     
		0x02,	// bit 1                              
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,                                        
		0x00,                                        
		0x02,
		0x00,
		0x02,                                        


		0x00,
		0x00,                                        
		0x01,	// CS_N =1                            

	};


void SPI_Read(MMP_UBYTE addr)
{
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    MMP_ULONG i;
    MMP_UBYTE bit;
    MMP_UBYTE spi_array[sizeof(spi_read)];


    RTNA_DBG_Byte(0, addr);
    RTNA_DBG_Str(0, " output = ");  

    for (i=0; i<sizeof(spi_read); i++)
         spi_array[i] = spi_read[i];


    for(i=0; i<8; i++)
    {
        if(addr&(1<<i)){
            bit = 17 + (7-i)*2 ;
            spi_array[bit] = 0x04;
            spi_array[bit+1] = 0x06;
        }                
    }

    
    
    for (i=0; i<sizeof(spi_read); i++) {
        
        pUSB_DMA->USB_PHY_SPI_CTL1 = spi_array[i];
        //A810H_WriteI2CRegW(0x8c10, spi_2[i] << 5);
        if((i >= 34) && (i%2 == 0))
        {
            if(pUSB_DMA->USB_PHY_SPI_CTL2)
                RTNA_DBG_Str(0, "1");
            else
                RTNA_DBG_Str(0, "0");
        }
    }
    RTNA_DBG_Str(0, "\r\n");  
}


void SPI_Write(MMP_UBYTE addr, MMP_USHORT data)
{
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    MMP_ULONG i;
    MMP_UBYTE bit;
    MMP_UBYTE spi_array[sizeof(spi_write)];
   // dbg_printf(0,"SPI Write : %x,%x\r\n",addr,data);
    for (i=0; i<sizeof(spi_write); i++)
         spi_array[i] = spi_write[i];


    for(i=0; i<8; i++)
    {
        if(addr&(1<<i)){
            bit = 17 + (7-i)*2 ;
            spi_array[bit] = 0x04;
            spi_array[bit+1] = 0x06;
        }                
    }

    for(i=0; i<16; i++)
    {
        if(data&(1<<i)){
            bit = 33 + (15-i)*2 ;
            spi_array[bit] = 0x04;
            spi_array[bit+1] = 0x06;
        }                
    }    
    
    for (i=0; i<sizeof(spi_write); i++) {
        
        pUSB_DMA->USB_PHY_SPI_CTL1 = spi_array[i];
        //A810H_WriteI2CRegW(0x8c10, spi_2[i] << 5);
    }
    //RTNA_DBG_Str(0, "\r\n");  
}

#endif //(USB_PHY_TEST_MODE_EN == 1)




#endif


#endif  //#if   defined(USB_FW)
#undef  exUSBPUB
/// @}
