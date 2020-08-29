//==============================================================================
//
//  File        : mmp_lib.h
//  Description : Top level global definition and configuration.
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_LIB_H_
#define _MMP_LIB_H_

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#ifndef NULL
    #define NULL (0)
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef char            MMP_BYTE;
typedef unsigned char   MMP_UBYTE;

typedef short           MMP_SHORT;
typedef unsigned short  MMP_USHORT;

typedef int             MMP_LONG;
typedef unsigned int    MMP_ULONG;
typedef long long  MMP_LONG64;
typedef unsigned long long MMP_ULONG64;

typedef unsigned char   MMP_BOOL;

typedef unsigned int        u_int   ;
typedef unsigned short      u_short ;
typedef unsigned char       u_char  ;

#define MMP_TRUE    (1)
#define MMP_FALSE   (0)

#define ALIGN2(_a)      (((_a) + 1) >> 1 << 1)
#define FLOOR4(_a)      ((_a) >> 2 << 2)
#define ALIGN4(_a)      (((_a) + 3) >> 2 << 2)
#define ALIGN8(_a)      (((_a + 0x07) >> 3) << 3)
#define ALIGN16(_a)     (((_a) + 15) >> 4 << 4)
#define FLOOR32(_a)     ((_a) >> 5 << 5)
#define ALIGN32(_a)     (((_a) + 31) >> 5 << 5)
#define FLOOR512(_a)     ((_a) >> 8 << 8)
#define ALIGN512(_a)     (((_a) + 511) >> 10 << 10)
#define ALIGN256(_a)     (((_a) + 255) >> 8 << 8)
#define ALIGN4096(_a)     (((_a) + 4095) >> 12 << 12)

#define CEIL(_N, _M)    (((_N%_M)>0)?((_N/_M)+1):(_N/_M))  

#define VR_MAX(a,b)		(((a) > (b)) ? (a) : (b))
#define VR_MIN(a,b)		(((a) < (b)) ? (a) : (b))
#define VR_ARRSIZE(a)	( sizeof((a)) / sizeof((a[0])) )
  
// Error Codes Define
#include "mmp_err.h"


//==============================================================================
//
//                              VARIABLES
//
//==============================================================================


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================



//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================
// Truman 06-Nov-06
/** @brief Debug break point

Usage: Simply add this macro as a statesment.\n
If the debug environment can't sync with IDE, you could use this to set an break
point for quick debugging.\n
You could run to the break point and break the debugger.
Then set the real break_point or trace the variables.
*/
#define BREAK_POINT {\
MMP_BYTE c[2];\
GETS(&c[0]);\
}
#endif // _MMP_LIB_H_
