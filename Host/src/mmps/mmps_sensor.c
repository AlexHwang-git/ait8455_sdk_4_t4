//==============================================================================
//
//  File        : mmps_sensor.c
//  Description : Ritian Sensor Control driver function
//  Author      : Alan Wu
//  Revision    : 1.0
//
//==============================================================================

/**
 *  @file mmps_sensor.c
 *  @brief The Sensor control functions
 *  @author Alan Wu
 *  @version 1.0
 */
#include "config_fw.h"

#include "mmp_lib.h"
//#include "ait_utility.h"
#include "mmph_hif.h"
#include "mmps_sensor.h"
#include "mmps_dsc.h"
#include "mmpd_system.h"


/** @addtogroup MMPS_SENSOR
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

/**@brief The sensor configuration

Use @ref MMPS_Sensor_GetConfiguration to assign the field value of it.
You should read this functions for more information.
*/
static	MMPS_SENSOR_MODES	    m_sensorModes;
/**@brief The current working sensor ID.
         
Use @ref MMPS_Sensor_Initialize to set it.
*/
static	MMP_USHORT				m_usCurSensorId = SENSOR_COUNT_MAX_NUM;
/**@brief The current used sensor preview width and height.
         
Use @ref MMPS_Sensor_Initialize to set it.
Use @ref MMPS_Sensor_GetCurResolution to get the value of them.
*/
static	MMP_USHORT				m_usCurSensorWidth, m_usCurSensorHeight;
/**@brief The current working sensor frame rate.

Use @ref MMPS_Sensor_SetFrameRate to set it. Set before @ref MMPS_Sensor_Initialize is called. It will take effect
only when @ref MMPS_Sensor_Initialize is called.
*/
static	MMP_USHORT				m_usCurSensorFramerate = 0; // 0 is for default
/**@brief The current sensor mode.

Use @ref MMPS_Sensor_Initialize to set it.
*/
static  MMPD_SENSOR_MODE        m_CurSensorMode;
/**@brief The face detection attribute.

Use @ref MMPS_Sensor_InitializeFaceDetect to set it.
*/
//static MMPD_SENSOR_FDTCATTRIBUTE    m_FdtcAttribute;
/**@brief The face detection mode.

Use @ref MMPS_Sensor_GetFaceDetectEnable to get it,
         and use MMPS_Sensor_SetFaceDetectEnable to set it.
*/
//static MMP_BOOL                 m_CurFdtcMode = MMP_FALSE;
/**@brief The current preview mode.

Use @ref MMPS_Sensor_Initialize to set it.
*/
static MMP_UBYTE                    m_ubCurPreviewMode;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================


//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetConfiguration
//  Description :
//------------------------------------------------------------------------------
/** @brief Retrives sensor configuration

The function retrives all sensor configuration, including sensor output width/heigh in preview mode or in capture mode.
It also report the framerate selection and exposure value level.

@return It returns all sensor configuration
*/
MMPS_SENSOR_MODES*  MMPS_Sensor_GetConfiguration(void)
{
    return &m_sensorModes;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_Initialize
//  Description :
//------------------------------------------------------------------------------
/** @brief Initialize Sensor

The function initializes Sensor. Before the function is called, the exact firmware code needs
to be downloaded into the MMP memory

@param[in] usSensorID the Sensor ID (0, 1, .....)
@param[in] ubPreviewMode Sensor preview mode, choose the input resolution of the sensor
@param[in] sensormode Sensor preview mode, choose the camera or video preview mode

@return It reports the status of the initialization result.
*/
#include "lib_retina.h"
MMP_ERR MMPS_Sensor_Initialize( MMP_USHORT usSensorID,
                                MMP_UBYTE ubPreviewMode,
                                MMPD_SENSOR_MODE sensormode)
{
    m_usCurSensorId = usSensorID;
    m_CurSensorMode = sensormode;
    m_ubCurPreviewMode  = ubPreviewMode;

    if (ubPreviewMode >= m_sensorModes.sensorconfig[usSensorID].ubSensorpreviewmodenum) {
	    m_usCurSensorWidth = m_sensorModes.sensorconfig[usSensorID].usSensorpreviewwidth[m_sensorModes.sensorconfig[usSensorID].ubSensordefaultpreviewmode];
	    m_usCurSensorHeight = m_sensorModes.sensorconfig[usSensorID].usSensorpreviewheight[m_sensorModes.sensorconfig[usSensorID].ubSensordefaultpreviewmode];

		MMPD_Sensor_SetMode(sensormode);
		return	MMPD_Sensor_Initialize(usSensorID, 
	                                   m_sensorModes.sensorconfig[usSensorID].ubFrameRate[m_usCurSensorFramerate],
									   m_sensorModes.sensorconfig[usSensorID].ubSensordefaultpreviewmode,
									   m_sensorModes.sensorconfig[usSensorID].ubSensordefaultphasecount); 
	}
	else {
	    m_usCurSensorWidth = m_sensorModes.sensorconfig[usSensorID].usSensorpreviewwidth[ubPreviewMode];
	    m_usCurSensorHeight = m_sensorModes.sensorconfig[usSensorID].usSensorpreviewheight[ubPreviewMode];
		MMPD_Sensor_SetMode(sensormode);
		return	MMPD_Sensor_Initialize(usSensorID, 
	                                   m_sensorModes.sensorconfig[usSensorID].ubFrameRate[m_usCurSensorFramerate],
									   ubPreviewMode,
									   m_sensorModes.sensorconfig[usSensorID].ubSensordefaultphasecount); 
	}

}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_ChangePreviewMode
//  Description :
//------------------------------------------------------------------------------
/** @brief Change the sensor preview mode

The function can be called after sensor is initialized to change the sensor preview mode.

@param[in] ubPreviewMode Sensor preview mode, choose the input resolution of the sensor
@param[in] ubPhaseCount Sensor phase count

@return It reports the status of the change mode status
*/
#if PCAM_EN==0
MMP_ERR MMPS_Sensor_ChangePreviewMode(MMP_UBYTE ubPreviewMode, MMP_UBYTE ubPhaseCount)
{
    m_ubCurPreviewMode  = ubPreviewMode;
    
    m_usCurSensorWidth = m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewwidth[ubPreviewMode];
    m_usCurSensorHeight = m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewheight[ubPreviewMode];

    return	MMPD_Sensor_PreviewMode(ubPreviewMode, ubPhaseCount);

}
#endif
//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetCurResolution
//  Description :
//------------------------------------------------------------------------------
/** @brief The function retrives current sensor output resolution

The function retrives current sensor output resolution
@param[out] usWidth  the width
@param[out] usHeight  the height
@retval MMP_ERR_NONE success
*/
MMP_ERR MMPS_Sensor_GetCurResolution(MMP_USHORT *usWidth, MMP_USHORT *usHeight)
{
    *usWidth = m_usCurSensorWidth;
    *usHeight = m_usCurSensorHeight;

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetMaxResolution
//  Description :
//------------------------------------------------------------------------------
/** @brief The function retrives current sensor output resolution

The function retrives sensor max output resolution
@param[out] usWidth  the width
@param[out] usHeight  the height
@retval MMP_ERR_NONE success
*/
MMP_ERR MMPS_Sensor_GetMaxResolution(MMP_ULONG *usWidth, MMP_ULONG *usHeight)
{
    MMP_ULONG i;
    
    *usWidth  = m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewwidth[0];
    *usHeight = m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewheight[0];

    for(i=1;i<SENSOR_MODE_MAX_NUM;i++) {
        if ( *usWidth  < m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewwidth[i] )
            *usWidth  = m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewwidth[i];
        if ( *usHeight  < m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewheight[i] )
            *usHeight  = m_sensorModes.sensorconfig[m_usCurSensorId].usSensorpreviewheight[i];
    }
    
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetRawStoreDownsample
//  Description :
//------------------------------------------------------------------------------
/** @brief The function retrives current sensor output resolution

The function retrives raw store downsample or not
@param[out] bDownsample  downsample or not
@retval MMP_ERR_NONE success
*/
MMP_ERR MMPS_Sensor_GetRawStoreDownsample(MMP_BOOL *bDownsample)
{
    *bDownsample = m_sensorModes.sensorconfig[m_usCurSensorId].bRawStoreDownsample[m_ubCurPreviewMode];

    return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_GetCurConfiguration
//  Description :
//------------------------------------------------------------------------------
/** @brief Retrives current sensor configuration

The function retrives current sensor configuration
@param[out] config the current sensor configuration
@retval MMP_ERR_NONE success
*/
MMP_ERR MMPS_Sensor_GetCurConfiguration(MMPS_SENSOR_CONFIG *config)
{
	if (m_usCurSensorId == SENSOR_COUNT_MAX_NUM)
		return	MMP_SENSOR_ERR_INITIALIZE_NONE;

	*config = m_sensorModes.sensorconfig[m_usCurSensorId];

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPS_Sensor_SetFrameRate
//  Description :
//------------------------------------------------------------------------------
/** @brief Set the sensor frame rate

@param[in] ubIndex the index of frame rate table
@retval MMP_ERR_NONE success
@return It reports the status of the operation.
*/

MMP_ERR MMPS_Sensor_SetFrameRate(MMP_UBYTE ubIndex)
{
    m_usCurSensorFramerate = ubIndex;

    return MMP_ERR_NONE;
}

/// @}
