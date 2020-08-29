/// @ait_only
//==============================================================================
//
//  File        : mmpd_sensor.c
//  Description : Ritian Sensor Control driver function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

/**
*  @file mmpd_sensor.c
*  @brief The Sensor Control functions
*  @author Penguin Torng
*  @version 1.0
*/
#include "config_fw.h"

#ifdef BUILD_CE
#undef BUILD_FW
#endif

#include "mmp_lib.h"
#include "mmpd_sensor.h"
#include "mmpd_system.h"
#include "mmph_hif.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_gbl.h"
#include "mmpf_sensor.h"

/** @addtogroup MMPD_Sensor
 *  @{
 */


//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
/**@brief set stable capture enable or not.

Use @ref MMPS_Sensor_SetStableState to set it.
Use @ref MMPS_Sensor_GetStableState to get the value of them.
*/
//static MMP_BOOL m_bMMPStableState = MMP_FALSE;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_Initialize
//  Description :
//------------------------------------------------------------------------------
/** @brief The function initializes the sensor by sending host command request

The function initializes the sensor by sending host command request (HIF_CMD_INIT_SENSOR) to the
firmware and waiting for the DSC command clear by calling MMPD_System_WaitForCommandClear. It
returns the status about the initialization result.

@param[in] sensor the sensor module
@param[in] ubPreviewMode the preview mode
@param[in] ubPhaseCount the phase count
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_Initialize(MMP_USHORT usSensorID, MMP_UBYTE ubFrameRate,
                                          MMP_UBYTE ubPreviewMode, MMP_UBYTE ubPhaseCount)
{
    MMP_ERR	mmpstatus;

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI,  MMP_TRUE );
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ISP,  MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_I2CM, MMP_TRUE);
    mmpstatus = MMPF_Sensor_Initialize(usSensorID,ubFrameRate,ubPreviewMode); 
    if (mmpstatus)
        return MMP_SENSOR_ERR_INITIALIZE;
    else
        return MMP_ERR_NONE;
}



//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set sensor camera preview mode or video preview mode

The function initializes the sensor by sending host command request (HIF_CMD_SET_SENSOR_MODE) to the
firmware and waiting for the DSC command clear by calling MMPD_System_WaitForCommandClear. It
returns the status about the set mode result.

@param[in] sensormode the sensor mode
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetMode(MMPD_SENSOR_MODE sensormode)
{
#if PCAM_EN==0
    MMPH_HIF_CmdSetParameterW(0, sensormode);

    // HIF_CMD_INIT_SENSOR : sensor group command
    MMPH_HIF_CmdSend(HIF_CMD_SET_SENSOR_MODE);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
#else
extern MMP_USHORT gsSensorMode;
    gsSensorMode =   sensormode ;
    return MMP_ERR_NONE ;  
#endif    
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_PreviewMode
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set the sensor preview mode

@param[in] ubPreviewMode the preview mode
@param[in] ubPhaseCount the phase count
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_Sensor_PreviewMode(MMP_UBYTE ubPreviewMode, MMP_UBYTE ubPhaseCount)
{
    MMPH_HIF_CmdSetParameterW(0, ubPreviewMode);
    MMPH_HIF_CmdSetParameterW(2, ubPhaseCount);

    MMPH_HIF_CmdSend(HIF_CMD_SENSOR_PREVIEW_MODE);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}
#endif


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_PowerDown
//  Description :
//------------------------------------------------------------------------------
/** @brief The function power down the sensor by sending host command request

The function power down  the sensor by sending host command request (HIF_CMD_POWERDOWN_SENSOR) to the
firmware and waiting for the DSC command clear by calling MMPD_System_WaitForCommandClear. It
returns the status about the initialization result.

@param[in] usSensorID the sensor module
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_PowerDown(MMP_USHORT usSensorID,MMP_UBYTE poweroff_mode)
{
    MMP_ERR	mmpstatus;
    mmpstatus = MMPF_Sensor_PowerDown(usSensorID,poweroff_mode);
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_FALSE);
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ISP, MMP_FALSE);
	MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_I2CM, MMP_FALSE);
    return mmpstatus;
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetAwbType
//  Description :
//------------------------------------------------------------------------------
/** @brief config the AWB type

@param[in] usType the AWB type
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_Sensor_SetAwbType(MMP_USHORT usType)
{
    MMPH_HIF_CmdSetParameterW(0, usType);

    MMPH_HIF_CmdSend(AWB_TYPE | HIF_CMD_3A_FUNCTION);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetAeType
//  Description :
//------------------------------------------------------------------------------
/** @brief config the AE type

@param[in] usType the AE type
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetAeType(MMP_USHORT usType)
{
    MMPH_HIF_CmdSetParameterW(0, usType);

    MMPH_HIF_CmdSend(AE_TYPE | HIF_CMD_3A_FUNCTION);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetNightMode
//  Description :
//------------------------------------------------------------------------------
/** @brief config the night mode

@param[in] usMode the night mode
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetNightMode(MMP_USHORT usMode)
{
    MMPH_HIF_CmdSetParameterW(0, usMode);

    MMPH_HIF_CmdSend(NIGHT_MODE | HIF_CMD_3A_FUNCTION);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetColorContrast
//  Description :
//------------------------------------------------------------------------------
/** @brief config the color contrast level

@param[in] usLevel the color contrast level
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetColorContrast(MMP_USHORT usLevel)
{
    MMPH_HIF_CmdSetParameterW(0, usLevel);

    MMPH_HIF_CmdSend(COLOR_CONTRAST | HIF_CMD_3A_FUNCTION);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetColorSaturation
//  Description :
//------------------------------------------------------------------------------
/** @brief config the color saturation

@param[in] usChannel the clannel
@param[in] usLevel the level
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetColorSaturation(MMP_USHORT usChannel,MMP_USHORT usLevel)
{
    MMPH_HIF_CmdSetParameterW(0, usChannel);
    MMPH_HIF_CmdSetParameterW(2, usLevel);

    MMPH_HIF_CmdSend(COLOR_SATURATION | HIF_CMD_3A_FUNCTION);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetColorEdge
//  Description :
//------------------------------------------------------------------------------
/** @brief config the color edge

@param[in] usLevel the color edge level
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetColorEdge(MMP_USHORT usLevel)
{
    MMPH_HIF_CmdSetParameterW(0, usLevel);

    MMPH_HIF_CmdSend(COLOR_SHARPNESS | HIF_CMD_3A_FUNCTION);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetSensorReg
//  Description :
//------------------------------------------------------------------------------
/** @brief Write the sensor register

@param[in] usAddr the register
@param[in] usData the data
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetSensorReg(MMP_USHORT usAddr, MMP_USHORT usData)
{
    MMPH_HIF_CmdSetParameterW(0, usAddr);
    MMPH_HIF_CmdSetParameterW(2, usData);

    MMPH_HIF_CmdSend(SET_REGISTER | HIF_CMD_SENSOR_CONTROL);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_GetSensorReg
//  Description :
//------------------------------------------------------------------------------
/** @brief Read the sensor register

@param[in] usAddr the register
@param[in] usData the data
@retval MMPS_DSC_SUCCESS success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_GetSensorReg(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	MMP_ERR	mmpstatus;

    MMPH_HIF_CmdSetParameterW(0, usAddr);

    MMPH_HIF_CmdSend(GET_REGISTER | HIF_CMD_SENSOR_CONTROL);
    mmpstatus = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);;
	if (mmpstatus == MMP_ERR_NONE) {
	    *usData = MMPH_HIF_CmdGetParameterW(0);
	}	    
    return mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_DSC_SetExposureValue
//  Description :
//------------------------------------------------------------------------------
/** @brief Set the exposure value of DSC

The function configures the exposure value of DSC
@param[in] usIndex the index of the exposure value array
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetExposureValue(MMP_USHORT usExpVal)

{
    MMPH_HIF_CmdSetParameterW(0, usExpVal);

    MMPH_HIF_CmdSend(HIF_CMD_SET_EXPOSURE_VALUE);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}
    
//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetAEMode
//  Description :
//------------------------------------------------------------------------------
/** @brief Set the AE mode of DSC

The function configures the AE mode(ISO) of DSC
@param[in] ubAEMode the index of the AE mode
    0:AE auto, 1:AE manual, 2:ISO, 3:Shutter
@param[in] ubISOMode the index of the ISO value
    0:ISO auto, 1:ISO 50, 2:ISO 100, 3:ISO 200, 4: ISO400, 5: ISO 800, 6: ISO 1600, 7: ISO 3200.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetAEMode(MMP_UBYTE ubAEMode, MMP_UBYTE ubISOMode)

{
    MMPH_HIF_CmdSetParameterW(0, ubAEMode);
    MMPH_HIF_CmdSetParameterW(2, ubISOMode);

    MMPH_HIF_CmdSend(HIF_CMD_SET_AE_MODE);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetAFMode
//  Description :
//------------------------------------------------------------------------------
/** @brief Set the AE mode of DSC

The function configures the AF mode of DSC
@param[in] ubAFMode the index of the AF mode
    0:Auto, 1:Manual, 2:MACRO, 3: FULL, 4: Test
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetAFMode(MMP_UBYTE ubAFMode)

{
    MMPH_HIF_CmdSetParameterW(0, ubAFMode);

    MMPH_HIF_CmdSend(SET_AF_MODE | HIF_CMD_3A_FUNCTION);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetStableState
//  Description :
//------------------------------------------------------------------------------
/** @brief Set stable capture enable or not.

The function set stable capture enable or not of DSC
@param[in] bSensorStableEnable sensor stable capture enable or disable.
@param[in] bMMPStableEnable MMP stable capture enable or disable.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetStableState(MMP_BOOL bSensorStableEnable, MMP_BOOL bMMPStableEnable) 
{
    m_bMMPStableState = bMMPStableEnable; 
    MMPH_HIF_CmdSetParameterL(0, (MMP_ULONG)bSensorStableEnable);
	MMPH_HIF_CmdSetParameterL(4, (MMP_ULONG)bMMPStableEnable);
    MMPH_HIF_CmdSend(HIF_CMD_SET_STABLE_STATE);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_GetMMPStableState
//  Description :
//------------------------------------------------------------------------------
/** @brief Get stable capture enable or not.

The function get stable capture enable or not of DSC
@param[out] bMMPStableEnable stable capture enable or disable.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_GetMMPStableState(MMP_BOOL *bMMPStableEnable) 
{
	*bMMPStableEnable = m_bMMPStableState;
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetHistEQState
//  Description :
//------------------------------------------------------------------------------
/** @brief Set histogram equalizer state enable or not.

The function set histogram equalizer enable or not.
@param[in] bHistEQState histogram equalizer enable or disable.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetHistEQState(MMP_BOOL bHistEQState) 
{
    MMPH_HIF_CmdSetParameterW(0, (MMP_USHORT)bHistEQState);
    MMPH_HIF_CmdSend(HIF_CMD_SET_HISTEQ_STATE);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
}
#endif
#if FDTC_SUPPORT==1
//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_InitFaceDetectConfig
//  Description :
//------------------------------------------------------------------------------
/** @brief Initialize face detection configuration.

The function set face detection initail configuration
@param[in] fdtcattribute the attributes of the face detection.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_InitFaceDetectConfig(MMPD_SENSOR_FDTCATTRIBUTE *fdtcattribute)
{
    MMPH_HIF_CmdSetParameterW(0, fdtcattribute->usInputW);
    MMPH_HIF_CmdSetParameterW(2, fdtcattribute->usInputH);
	MMPH_HIF_CmdSetParameterW(4, fdtcattribute->ubInputBufCnt|(fdtcattribute->ubDetectFaceNum << 8));
	MMPH_HIF_CmdSend(INIT_CONFIG | HIF_CMD_FDTC);
    return MMPC_System_CheckMMPStatus(FDTC_CMD_IN_EXEC);
}
//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetFaceDetectBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief Set face detection buffer.

The function set face detection buffer for operation
@param[in] fdtcattribute the attributes of the face detection.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetFaceDetectBuf(MMPD_SENSOR_FDTCATTRIBUTE *fdtcattribute)
{
    MMPH_HIF_CmdSetParameterL(0, fdtcattribute->ulWorkBufAddr);
    MMPH_HIF_CmdSetParameterL(4, fdtcattribute->ulFeatureBufAddr);
	MMPH_HIF_CmdSetParameterL(8, fdtcattribute->ulInfoBufAddr);

	MMPH_HIF_CmdSend(SET_BUFFER | HIF_CMD_FDTC);
    return MMPC_System_CheckMMPStatus(FDTC_CMD_IN_EXEC);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetFaceDetectEnable
//  Description :
//------------------------------------------------------------------------------
/** @brief the function will calculated face detection.

The function set command to firmware to do face detection.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetFaceDetectEnable(MMP_BOOL bEnable) 
{
    MMPH_HIF_CmdSetParameterL(0, (MMP_ULONG)bEnable);
    MMPH_HIF_CmdSend(DO_FDTC | HIF_CMD_FDTC);
    return  MMPC_System_CheckMMPStatus(FDTC_CMD_IN_EXEC);
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetFaceDetectDisplayInfo
//  Description :
//------------------------------------------------------------------------------
/** @brief the function set necessary information to firmware for displaying
           face rectangles. This function can be called only when using firware 
           to draw face rectangles to preview buffers

The function set command to firmware to set necessary information for face rectangles
displaying.
@param[in] bDrawFaceByFw draw face rectangles by FW or not.
@param[in] pipeID the IBC pipe ID of preview path.
@param[in] usPreviewW the width of preview buffer.
@param[in] usPreviewH the height of preview buffer.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetFaceDetectDisplayInfo(MMP_BOOL bDrawFaceByFw, MMPD_IBC_PIPEID pipeID, 
                                             MMP_USHORT usPreviewW, MMP_USHORT usPreviewH)
{
    MMPH_HIF_CmdSetParameterW(0, (MMP_USHORT)bDrawFaceByFw);
    MMPH_HIF_CmdSetParameterW(2, (MMP_USHORT)pipeID);
    MMPH_HIF_CmdSetParameterW(4, usPreviewW);
    MMPH_HIF_CmdSetParameterW(6, usPreviewH);
    MMPH_HIF_CmdSend(SET_DISPLAY_INFO | HIF_CMD_FDTC);
    return  MMPC_System_CheckMMPStatus(FDTC_CMD_IN_EXEC);
}
//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetAFWin
//  Description :
//------------------------------------------------------------------------------
/** @brief The function set AF window.

The function set command to firmware to set AF window.
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetAFWin(MMPD_SENSOR_AFWIN AFWin)
{
	MMP_USHORT	i;
	MMP_ERR		mmpstatus;	
	
	for(i = 0; i < AFWin.usAFWinNum; i++) {		
		MMPH_HIF_CmdSetParameterW(0,  i);
		MMPH_HIF_CmdSetParameterW(2,  AFWin.usPreviewWidth);
		MMPH_HIF_CmdSetParameterW(4,  AFWin.usPreviewHeight);
		MMPH_HIF_CmdSetParameterW(6,  AFWin.usStartX[i]);
		MMPH_HIF_CmdSetParameterW(8,  AFWin.usStartY[i]);
		MMPH_HIF_CmdSetParameterW(10, AFWin.usWidth[i]);
		MMPH_HIF_CmdSetParameterW(12, AFWin.usHeight[i]);

	    MMPH_HIF_CmdSend(SET_AF_WIN | HIF_CMD_3A_FUNCTION);
	    mmpstatus = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);	
        if (mmpstatus) {
            return mmpstatus;
        }
	}
    return  mmpstatus;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_AutoFocus
//  Description :
//------------------------------------------------------------------------------
/** @brief The function request sensor to perform auto focus

The function request sensor to perform auto focus of lens
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_Sensor_AutoFocus(void)
{
	MMP_ERR err = MMP_ERR_NONE;
	MMP_UBYTE AFStatus = 1;
    MMPH_HIF_CmdSend(AUTO_FOCUS | HIF_CMD_3A_FUNCTION);
    err = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
    if (err != MMP_ERR_NONE)
    {
    	return err;
    }
#if 0  // speed up start preview, tomy@2010_01_20
    while((AFStatus == 0) ||(AFStatus == 1))
    {
    	MMPH_HIF_CmdSend(GET_AF_STATUS | HIF_CMD_3A_FUNCTION);
    	err = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
    	AFStatus = MMPH_HIF_CmdGetParameterB(0);
    }
   	if (AFStatus == 3) {
   		return MMP_SENSOR_ERR_AF_MISS;
    }
#endif    
    return err;
}
//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_InfinityFocus
//  Description :
//------------------------------------------------------------------------------
/** @brief The function ...

The function request sensor to perform infinity focus of lens
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_InfinityFocus(void)
{
    MMPH_HIF_CmdSend(FOCUS_INFINITY | HIF_CMD_3A_FUNCTION);
    return  MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);	

}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetLightFreq
//  Description :
//------------------------------------------------------------------------------
/** @brief The function config the light frequency mode in the ISP.

The function config the light frequency mode in the ISP.
@param[in] mode the light frequency mode
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetLightFreq(MMPD_SENSOR_DEBAND_MODE mode)
{
    MMPH_HIF_CmdSetParameterW(0, (MMP_USHORT)mode);
    MMPH_HIF_CmdSend(LIGHT_FREQ | HIF_CMD_3A_FUNCTION);
    return  MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);	
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_GetPreviewEffectNum
//  Description :
//------------------------------------------------------------------------------
/** @brief Retrive the image effect number

The function returns the number of effects the system can support.
@return It returns the number supported
*/
MMP_ERR MMPD_Sensor_GetPreviewEffectNum(MMP_UBYTE *ubEffectNum)
{
	MMP_ERR	mmpstatus;

    MMPH_HIF_CmdSend(HIF_CMD_GET_EFFECT_NUM);
    mmpstatus = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);	
    if (mmpstatus == MMP_ERR_NONE) {
	    *ubEffectNum = MMPH_HIF_CmdGetParameterW(0);
    }

    return  mmpstatus;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetPreviewEffect
//  Description :
//------------------------------------------------------------------------------
/** @brief Configure the preview image effect

The function sets the preview effect with the specified effect ID input. It saves the effect ID for internal
use after the setting.

@param[in] ubEffectID the effect ID in the effect array
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetPreviewEffect(MMP_UBYTE ubEffectID)
{
    MMPH_HIF_CmdSetParameterW(0, ubEffectID);
    MMPH_HIF_CmdSend(HIF_CMD_SET_PREVIEW_EFFECT);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);	
}


//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetPDCLK
//  Description :
//------------------------------------------------------------------------------
/** @brief Configure the preview image effect

The function configures the VIF clock divider for PDCLK (Sensor Clock)

@param[in] ubDivider the divider of VIF clock
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_SetPDCLK(MMP_UBYTE ubDivider)
{
    MMPH_HIF_RegSetB(VIF_SENSR_CLK_FREQ, ubDivider);

    return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_SetFlip
//  Description :
//------------------------------------------------------------------------------
/** @brief Configure the sensor flip

The function sets the sensor flip with the specified flip type input. It saves the flip type for internal
use after the setting.

@param[in] ubFlipType the flip type
@return It reports the status of the operation.
*/
#if PCAM_EN==0
MMP_ERR MMPD_Sensor_SetFlip(MMP_UBYTE ubFlipType)
{
    MMPH_HIF_CmdSetParameterW(0, ubFlipType);
    MMPH_HIF_CmdSend(HIF_CMD_SET_SENSOR_FLIP);
    return MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);	
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Sensor_GetPreviewBuffer
//  Description :
//------------------------------------------------------------------------------
/** @brief Get the address of preview buffer

The function return the address of preview buffer
@param[out] ulBaseAddr The address of preview buffer.
@param[out] ulBaseAddr The U buffer address(valid only for YUV420).
@param[out] ulBaseAddr The V buffer address(valid only for YUV420).
@retval MMP_ERR_NONE success
@return It reports the status of the operation.
*/
MMP_ERR MMPD_Sensor_GetPreviewBuffer(MMPD_IBC_PIPEID pipeId, MMP_ULONG *ulBaseAddr, MMP_ULONG *ulBaseUAddr, MMP_ULONG *ulBaseVAddr)
{
	MMP_ERR	mmpstatus;

    MMPH_HIF_CmdSetParameterW(0, pipeId);
    MMPH_HIF_CmdSend(HIF_CMD_GET_PREVIEW_BUF);
    mmpstatus = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);	
    if (mmpstatus == MMP_ERR_NONE) {
	    *ulBaseAddr = MMPH_HIF_CmdGetParameterL(0);
	    *ulBaseUAddr = MMPH_HIF_CmdGetParameterL(4);
    	*ulBaseVAddr = MMPH_HIF_CmdGetParameterL(8);
	}
    return  mmpstatus;
}
#endif
/// @}
/// @end_ait_only

#ifdef BUILD_CE
#define BUILD_FW
#endif

