#ifndef COLOR_H
#define COLOR_H

#include "config_fw.h"

// PU Range : 
#define SAT_MIN (  0)
#define SAT_MAX (255)
#define SAT_DEF (128)

#define CON_MIN (  0)
#define CON_MAX (255)
#define CON_DEF (128)

#define BRI_MIN (  0)
#define BRI_MAX (255)
#define BRI_DEF (128)

#define HUE_MIN (  0)
#define HUE_MAX (255)
#define HUE_DEF (128)

#define GAM_MIN (  0)
#define GAM_MAX (255)
#define GAM_DEF (128)

#define BLC_MIN (  0)
#define BLC_MAX (  1)
#define BLC_DEF (  0) //sean@2011_01_24, Backlight def is 0

#define SHA_MIN (  0)
#define SHA_MAX (255)
#define SHA_DEF (128)

//#define EPV_MIN (0x0026)
//#define EPV_MAX (0x00D6)
//#define EPV_DEF (0x005A)

#define WBT_MIN (2000)
#define WBT_MAX (7500)
#define WBT_DEF ( 4000)

#define AWB_DEF (0x01)
#define AWB_MIN (0x00)
#define AWB_MAX (0x01)

#define BAND_MIN (  0)
#define BAND_MAX (  2)
#define BAND_DEF BAND_60HZ

#define GAIN_DEF (0)
#define GAIN_MIN (0  )
#define GAIN_MAX (255)

#if SCALER_ZOOM_LEVEL!=BEST_LEVEL
// Set Logical zoom range from 0 ~ 10
#define ZOM_MIN (0) 
#define ZOM_MAX (10)
#define ZOM_DEF (0)
#else
// Set Logical zoom range from 0 ~ 10
#define ZOM_MIN (100) 
#define ZOM_MAX (140)//(400)
#define ZOM_DEF (100)
#define ZOM_RES	(10)
#endif



#define PANTILT_RES (3600)

#define PAN_MIN (-10 * PANTILT_RES)
#define PAN_MAX ( 10 * PANTILT_RES)
#define PAN_DEF ( 0  )

#define TILT_MIN (-10 * PANTILT_RES)
#define TILT_MAX ( 10 * PANTILT_RES)
#define TILT_DEF (  0 * PANTILT_RES)


#define AAF_MIN (0) //absolute focus
#define AAF_MAX (255) //(0x00FF)
#define AAF_DEF (0/*128*/) //(0x0080)
#define AAF_RES (1)//12)

#define AUTOAF_MIN (0x0000) //auto focus
#define AUTOAF_MAX (0x0001)
#define AUTOAF_DEF (0x0001)

// Bits - set, not range
#define AEMODE_MANUAL   (0x01) // Manual
#define AEMODE_AUTO     (0x02)
#define AEMODE_SHUTTER  (0x04)
#define AEMODE_APERTURE (0x08)

#define AEMODE_DEF      (AEMODE_APERTURE)

#define AEPRIORITY_MIN      (0)
#define AEPRIORITY_MAX      (1)
#define AEPRIORITY_DEF      (1)//(0)
#define AEPRIORITY_AUTO     AEPRIORITY_MAX
#define AEPRIORITY_FIX      AEPRIORITY_MIN

#if 0
#define EXPOSURE_MIN        (3)
#define EXPOSURE_MAX        (2047)
#define EXPOSURE_DEF        (3) // TBD 
#endif

#define EXPOSURE_MIN        (3)	//-11
#define EXPOSURE_MAX        (2047)//(4095)//(16383)//(2047)	//-2
#define EXPOSURE_DEF        (127) // TBD //-6

#define SIMPLE_FOCUS_MIN        (0)
#define SIMPLE_FOCUS_MAX        (3)
#define SIMPLE_FOCUS_DEF        (0)    

#endif