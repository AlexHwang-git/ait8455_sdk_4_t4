#ifndef _MMPF_USBDESCR_H_
#define _MMPF_USBDESCR_H_

#include "mmp_lib.h"

typedef struct _USB_CONFIG_DESC_CFG
{
    MMP_UBYTE *cfg_desc ;
    MMP_UBYTE *iad_desc ;
    MMP_UBYTE *if_desc  ;
    MMP_UBYTE *vcif_desc;
    MMP_UBYTE *vcot_desc;
    MMP_UBYTE *vcxu1_desc ;
    MMP_UBYTE *vcxu2_desc ;
    MMP_UBYTE *vcxu3_desc ;
    MMP_UBYTE *vcit_dsec;
    MMP_UBYTE *vcpu_desc;
} USB_CONFIG_DESC_CFG ;

#endif
