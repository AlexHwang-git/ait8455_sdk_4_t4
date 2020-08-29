
#ifndef _MMPF_BOOT_H_
#define _MMPF_BOOT_H_

#include    "includes_fw.h"

#define VALID_BOOTINFO_SIG 0x33323438

typedef struct _MMPF_BOOTINFO
{
    MMP_ULONG  vector[8] ;
    MMP_ULONG  sig ;  //0x20
    MMP_ULONG  abort_lr; 
    MMP_ULONG  abort_sp;
	
} MMPF_BOOTINFO ;

#define BC_USB_ON   0x00
#define BC_USB_OFF  0x80


typedef struct _MMPF_BOOTEXINFO
{
    MMP_UBYTE   loadfw ; // which firmware is load
    MMP_UBYTE   watchdog_reset;
} MMPF_BOOTEXINFO ;

#endif