#ifndef _MMPD_DRAM_H
#define _MMPD_DRAM_H
/// @ait_only
/** @addtogroup MMPD_System
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
typedef enum _MMPD_DRAM_TYPE
{
    MMPD_DRAM_TYPE_NONE = 0,	// no stack memory
    MMPD_DRAM_TYPE_1,			// first used
    MMPD_DRAM_TYPE_2,			// second used
    MMPD_DRAM_TYPE_3,			// third used
    MMPD_DRAM_TYPE_EXT,
    MMPD_DRAM_TYPE_AUTO
} MMPD_DRAM_TYPE;
 
typedef enum _MMPD_DRAM_MODE
{
    MMPD_DRAM_MODE_SDRAM = 0,	// SD RAM
    MMPD_DRAM_MODE_DDR,		// DDR RAM
    MMPD_DRAM_MODE_DDR2
} MMPD_DRAM_MODE;
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

MMP_ERR MMPD_DRAM_Initialize(MMPD_DRAM_TYPE dramtype, MMP_ULONG *ulSize, MMP_ULONG ulClock, MMPD_DRAM_MODE drammode);
MMP_ERR MMPD_DRAM_SendCommand(MMP_USHORT usCmd);
MMP_ERR MMPD_DRAM_SetPowerDown(MMP_BOOL bEnterPowerDown);
MMP_ERR MMPD_DRAM_SetSelfRefresh(void);
MMP_ERR MMPD_DRAM_ConfigPad(void);
/// @}
#endif

/// @end_ait_only

