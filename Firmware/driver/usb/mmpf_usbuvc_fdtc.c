#include "config_fw.h"
#include "mmp_lib.h"
#include "pcam_api.h"
#include "mmpf_usbuvc_fdtc.h"

#if OMRON_FDTC_SUPPORT
MMP_BYTE gbEnableDoFDTC,gbFDTC_Done;
MMP_ULONG glFDTC_CurrentBuffer;

void usb_uvc_init_fdtc(MMP_BOOL en)
{
    gbFDTC_Done = 1;
    //gbEnableDoFDTC = 1;
    if(1/*gbEnableDoFDTC*/){
    	initFaceProcessor(0);
    }
    usb_uvc_enable_fdtc(en);
}

void usb_uvc_enable_fdtc(MMP_BOOL en)
{
    gbEnableDoFDTC = en ;
}

MMP_BOOL usb_uvc_is_fdtc_on(void)
{
    //dbg_printf(3,"<FDTC : gbEnableDoFDTC :%d\r\n",gbEnableDoFDTC);
    return gbEnableDoFDTC ;
}

MMP_BOOL usb_uvc_is_fdtc_idle(void)
{
    if(gbEnableDoFDTC) {
        if(!gbFDTC_Done) {
            return 0;
        }
    }
    return 1 ;
}
void usb_uvc_close_fdtc(void)
{
    //resetFDTC();    
    //resetFaceProcessor();
    //resetFDTCMemory();
    if(gbEnableDoFDTC)	{
        resetFDTCMemory();
        gbEnableDoFDTC = 0 ;
    }    
}

MMP_BOOL usb_uvc_prepareframe_fdtc(MMP_ULONG y_frame_addr,MMP_ULONG size)
{
    MMP_BOOL do_fdtc = 0;
	if(gbEnableDoFDTC && (gbFDTC_Done == 1)){
		gbFDTC_Done=0;	
		MMPF_MMU_FlushCache((MMP_ULONG)glFDTC_CurrentBuffer, size);
		MMPF_DMA_MoveData1(y_frame_addr, (MMP_ULONG)glFDTC_CurrentBuffer, size, NULL,NULL);
		USB_VideoFDTC_Start(PCAM_NONBLOCKING);
		do_fdtc = 1 ;
	}
	
	return do_fdtc ;

}
#endif
