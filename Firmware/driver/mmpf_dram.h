#ifndef _MMPF_DRAM_H
#define _MMPF_DRAM_H
#include "mmp_err.h"
#include "mmpf_typedef.h"
/// @ait_only
/** @addtogroup MMPF_DRAM
 *  @{
 */
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


//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPF_DRAM_TYPE
{
    MMPF_DRAM_TYPE_NONE = 0,	// no stack memory
    MMPF_DRAM_TYPE_1,			// first used
    MMPF_DRAM_TYPE_2,			// second used
    MMPF_DRAM_TYPE_3,			// third used
    MMPF_DRAM_TYPE_EXT,
    MMPF_DRAM_TYPE_AUTO
} MMPF_DRAM_TYPE;

typedef enum _MMPF_DRAMID_CHIPID_MAP
{
    MMPF_DRAMID_NONE = 0x0,
    #if (CHIP == VSN_V2)
    MMPF_DRAMID_WINBOND_SDR16 = 0x7,
    MMPF_DRAMID_WINBOND_SDR64 = 0x04,
    MMPF_DRAMID_WINBOND_DDR64 = 0x0A,
    #endif
    #if (CHIP == VSN_V3)
    MMPF_DRAMID_WINBOND_DDR256 = 0x01,
    #endif
    MMPF_DRAMID_MAX
} MMPF_DRAMID_CHIPID_MAP;
 
typedef enum _MMPF_DRAM_MODE
{
    MMPF_DRAM_MODE_SDR = 0,	// SD RAM
    MMPF_DRAM_MODE_DDR,		// DDR RAM
    MMPF_DRAM_MODE_DDR2,
    MMPF_DRAM_MODE_AUTO,
    MMPF_DRAM_MAX_MODE
} MMPF_DRAM_MODE;
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
MMP_ERR MMPF_DRAM_RangeScan(void);
MMP_ERR MMPF_DRAM_Initialize(MMP_ULONG *ulSize, MMP_ULONG ulClock, MMPF_DRAM_MODE drammode);
MMP_ERR MMPF_DRAM_SetSelfRefresh(MMP_BOOL bEnterSelfRefresh);
/// @}
#endif

/// @end_ait_only

