#define exUSBMSDC
#include "includes_fw.h"
#include "lib_retina.h"
#include "config_fw.h"
#include "mmp_register.h"
#include "mmp_reg_jpeg.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_dma.h"
#include "mmp_reg_h264enc.h"
#include "mmp_reg_usb.h"
#include "mmp_reg_graphics.h"
#include "mmp_reg_icon.h"
#include "mmp_reg_audio.h"


#if JTAG==1
#pragma O0
AITPS_VIF   z_VIF = AITC_BASE_VIF;
AITPS_JPG   z_JPG = AITC_BASE_JPG;
AITPS_IBC   z_IBC = AITC_BASE_IBC;
AITPS_SCAL  z_SCAL= AITC_BASE_SCAL;
AITPS_GRA   z_GRA = AITC_BASE_GRA ;
AITPS_GBL   z_GBL = AITC_BASE_GBL ;
AITPS_ICOB  z_ICOB = AITC_BASE_ICOB ;
AITPS_USB_DMA z_USBDMA = AITC_BASE_USBDMA ;
AITPS_USB_CTL z_USBCTL = AITC_BASE_USBCTL ;
AITPS_MCI   z_MCI = AITC_BASE_MCI ;
AITPS_AFE   z_AFE = AITC_BASE_AFE ;
#pragma
#endif



