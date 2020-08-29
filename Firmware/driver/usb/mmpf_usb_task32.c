//==============================================================================
//
//  File        : mmpf_usb_task.c32
//  Description : Usb Task Entry
//  Author      : Hans Liu
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "reg_retina.h"
#include "lib_retina.h"
#include "mmpf_usbvend.h"
#include "os_wrap.h"
#include "mmpf_usbpccam.h"
#include "mmp_register.h"
#include "mmp_reg_usb.h"

/** @addtogroup MMPF_USB
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
OS_STK                  USB_Task_EP1_Stk[USB_EP1_STK_SIZE];
#if (USB_UVC15==1) &&(SINGLE_STREAMING_TASK==0)
OS_STK                  USB_Task_EP2_Stk[USB_EP2_STK_SIZE];
#endif
extern void             USB_Task_EP1(void);
extern void             USB_Task_EP2(void);
MMPF_OS_SEMID           gUSBDMASem[2] = {0xFF,0xFF} ;
MMPF_OS_SEMID           gUSBDMAListSem[2] = {0xFF,0xFF} ;
MMP_UBYTE gDMAListBufCnt[] = {EP1_DMA_LIST_BUF_CNT,EP2_DMA_LIST_BUF_CNT } ;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
MMP_USHORT MMPF_USB_DMAListSemCnt(MMP_UBYTE ep_id);
//------------------------------------------------------------------------------
//  Function    : USB_TaskHandler
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
void MMPF_USB_EP1_TaskHandler(void *p_arg)
{

    gUSBDMAListSem[0] = MMPF_OS_CreateSem(EP1_DMA_LIST_BUF_CNT);
    gUSBDMASem[0] = MMPF_OS_CreateSem(1);
#if SINGLE_STREAMING_TASK==1    
    gUSBDMASem[1] = MMPF_OS_CreateSem(1);
#endif    
    USB_Task_EP1();
}

#define USB_DMA_T  (0)//(4) //(0)// (100)

/*
return 0 : contine
return 1 : abort
*/
MMP_BOOL MMPF_USB_WaitDMAList(MMP_UBYTE ep_id) 
{
    MMP_UBYTE ret ;
    STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
wait:    
    ret = MMPF_OS_AcquireSem( gUSBDMAListSem[ep_id] ,ss->dma_timeout ); 
    
    if(ret) {
        //dbg_printf(3,"|LOUT(%d)|\r\n",ep_id);
        if( usb_dma_busy(ep_id) ) {
            //dbg_printf(3,"<LBZ>\r\n");
            if(!(ss->tx_flag & SS_TX_STREAMING)) {
                goto end ;
            } else {
                goto wait ;
            }
        }
    }
end:    
    if(!(ss->tx_flag & SS_TX_STREAMING)) {
        dbg_printf(3,"*dmalist forced stop\r\n");
        return MMP_TRUE;
    }
    return MMP_FALSE ;
}

void MMPF_USB_ReleaseDMAList(MMP_UBYTE ep_id)
{
    MMP_USHORT semcnt ;
    
    MMP_UBYTE ret;
    semcnt = MMPF_USB_DMAListSemCnt(ep_id);
    if(semcnt < gDMAListBufCnt[ep_id]) {
        ret = MMPF_OS_ReleaseSem( gUSBDMAListSem[ep_id] );  
    } 
    else {
       // dbg_printf(3,"*DMAList.sem(%d) cnt:%d\r\n",ep_id,semcnt); 
    }
}

void MMPF_USB_ResetDMAList(MMP_UBYTE ep_id) 
{
    if(ep_id < VIDEO_EP_END()) {
        MMPF_OS_SetSem(gUSBDMAListSem[ep_id],gDMAListBufCnt[ep_id]);
    }
}

MMP_USHORT MMPF_USB_DMAListSemCnt(MMP_UBYTE ep_id)
{
    MMP_USHORT semcnt;
    MMPF_OS_QuerySem(gUSBDMAListSem[ep_id],&semcnt);
    return semcnt ;
}

MMP_USHORT MMPF_USB_DMASemCnt(MMP_UBYTE ep_id)
{
    MMP_USHORT semcnt;
    MMPF_OS_QuerySem(gUSBDMASem[ep_id],&semcnt);
    return semcnt ;

}
/*
return 0 : contine
return 1 : abort
*/
MMP_BOOL MMPF_USB_WaitDMA(MMP_UBYTE ep_id) 
{
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    
    MMP_USHORT csr,ep ;
    STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
    MMP_UBYTE ret ;
    MMP_UBYTE dma_id = usb_epid_link_dmaid(ep_id);
wait:    
    ret = MMPF_OS_AcquireSem( gUSBDMASem[ep_id] ,ss->dma_timeout); 
    #if 1
    if(ret) {
        //dbg_printf(3,"|DOUT(%d)|\r\n",ep_id);
        if( usb_dma_busy(ep_id) ) {
            //dbg_printf(3,"|DBZ|\r\n");
            if(!(ss->tx_flag & SS_TX_STREAMING)) {
                goto end ;
            } else {
                goto wait ;
            }
        }
        #if 1 // TEST
        else {
            if(dma_id==0) {
                pUSB_DMA->USB_DMA_INT_SR |= USB_INT_DMA1_DONE_EN ;
            }
            else {
                pUSB_DMA->USB_DMA_INT_SR |= USB_INT_DMA2_DONE_EN ;
            }
            dbg_printf(3,"<T.DONE:%d>\r\n",dma_id);
            goto wait ;
        }
        #endif
    }
    #endif

end:    
    if(ep_id==0) {
        ep = PCCAM_TX_EP1_ADDR ;
    } else {
        ep = PCCAM_TX_EP2_ADDR ;
    }
    // Must wait CSR tx_pkt_ready & fifo not empty to be cleaned
    #if 1
    do {
        csr = UsbReadTxEpCSR(ep) & 0x3;
        if(!(ss->tx_flag & SS_TX_STREAMING)) {
            //dbg_printf(3,"*stream forced stop\r\n");
            return 1;
        }
        //dbg_printf(3,"@");
    } while(csr );
    #endif
    
    
    if(ret){
       // dbg_printf(3,"#waitdma.err:%d,%d\r\n",ret,ep_id);
    }
    //dbg_printf(3,"w");
    return 0;
   // dbg_printf(3,"wait.sem\r\n");
}

// return 
// TRUE : timeout out in wait sem
// FALSE : normal.
MMP_BOOL MMPF_USB_ReleaseDMA(MMP_UBYTE ep_id)
{
    MMP_USHORT ret,semcnt;
    
    MMPF_OS_QuerySem(gUSBDMASem[ep_id],&semcnt);
    if(semcnt!=0) {
        //dbg_printf(3,"*DMA.sem(%d) cnt:%d\r\n",ep_id,semcnt);
       // while(1);
       return MMP_TRUE ;
    } else {
        ret = MMPF_OS_ReleaseSem( gUSBDMASem[ep_id] );  
        return MMP_FALSE ;
    }
}

void MMPF_USB_ResetDMA(MMP_UBYTE ep_id) 
{
    MMP_UBYTE ret ;
    if(ep_id < VIDEO_EP_END()) {
        ret = MMPF_OS_SetSem(gUSBDMASem[ep_id],1);
    }
//    MMPF_OS_SetSem(gUSBDMASem[1],1);
}

#if (USB_UVC15==1) &&(SINGLE_STREAMING_TASK==0)
void MMPF_USB_EP2_TaskHandler(void *p_arg)
{
    gUSBDMAListSem[1] = MMPF_OS_CreateSem(EP2_DMA_LIST_BUF_CNT);
    gUSBDMASem[1] = MMPF_OS_CreateSem(1);
    USB_Task_EP2();
}
#endif


/// @}