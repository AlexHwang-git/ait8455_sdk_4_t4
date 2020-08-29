/// @ait_only
//==============================================================================
//
//  File        : mmpd_icon.c
//  Description : Retina Icon Module Control driver function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
*  @file mmpd_icon.c
*  @brief The Icon Module Control functions
*  @author Penguin Torng
*  @version 1.0
*/
#include "config_fw.h"

//#ifdef BUILD_CE
//#undef BUILD_FW
//#endif

#include "mmp_lib.h"
#include "mmpf_icon.h"
#include "mmpd_system.h"
#include "mmp_reg_icon.h"

/** @addtogroup MMPD_Icon
 *  @{
 */

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

//------------------------------------------------------------------------------
//  Function    : MMPF_ICON_SetAttributes
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function enables or disables the specified sticker with its sticker ID.

The function enables or disables the specified sticker with its sticker ID by sending host command
request to the firmware and waiting for the DSC command clear .

  @param[in] usStickerID the Sticker ID
  @param[in] bEnable enable or disable Sticker
  @return It reports the status of the operation.
*/
MMP_ERR MMPF_ICON_SetAttributes(MMPF_ICO_PIPEID pipeID, MMPF_ICO_PIPEATTRIBUTE *pipeattribute)
{
	AITPS_ICOB  pICOB = AITC_BASE_ICOB;

    if (pipeattribute->bDlineEn) {
        pICOB->ICO_DLINE_CFG[pipeID] = ICO_DLINE_SRC_SEL(pipeattribute->inputsel);
    }
    else {
        pICOB->ICO_DLINE_CFG[pipeID] = ICO_DLINE_BYPASS | ICO_DLINE_SRC_SEL(pipeattribute->inputsel);
    }

    return	MMP_ERR_NONE;
}

/// @}
/// @end_ait_only


//#ifdef BUILD_CE
//#define BUILD_FW
//#endif

