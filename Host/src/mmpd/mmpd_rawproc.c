#include "config_fw.h"
#ifdef BUILD_CE
#undef BUILD_FW
#endif

#include "mmp_err.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_rawproc.h"
#include "mmph_hif.h"
#include "mmpd_rawproc.h"
#include "mmpd_system.h"
#if PCAM_EN==1
#define RAWPROC_EN  (0)
#endif
//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_EnablePath
//  Description :
//------------------------------------------------------------------------------
/** @brief Enable/Disable raw path to store/fetch data

Enable/Disable raw path preview to store/fetch data

@param[in] bEnable  enable/disable
@return It reports the status of the configuration.
*/
MMP_ERR MMPD_RAWPROC_EnablePath(MMP_BOOL bEnable, MMP_BOOL bLumaEn ,MMP_BOOL bRaw2ISP)    //Ted ZSL
{
#if RAWPROC_EN==1
    MMP_ERR ret;
    if (bEnable) {
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_TRUE);
        MMPH_HIF_RegSetB(RAWPROC_SRC_SEL, MMPH_HIF_RegGetB(RAWPROC_SRC_SEL)|RAWPROC_SRC_VIF);
        MMPH_HIF_RegSetB(RAWPROC_MODE_SEL, MMPH_HIF_RegGetB(RAWPROC_MODE_SEL) & ~RAWPROC_10BIT_MODE);
        MMPH_HIF_RegSetB(RAWPROC_MODE_SEL, MMPH_HIF_RegGetB(RAWPROC_MODE_SEL)|RAWPROC_STORE_EN|RAWPROC_F_BURST_EN);
        
        // raw high speed mode
        if(bRaw2ISP)                    //Ted ZSL
            MMPH_HIF_RegSetB(VIF_RAW_OUT_EN, MMPH_HIF_RegGetB(VIF_RAW_OUT_EN)|VIF_2_RAW_EN|VIF_HIGH_SPEED_MODE|VIF_2_ISP_EN|VIF_V_DOWNSAMPLE_V1|VIF_H_DOWNSAMPLE_V1);
        else    
        MMPH_HIF_RegSetB(VIF_RAW_OUT_EN, MMPH_HIF_RegGetB(VIF_RAW_OUT_EN)|VIF_2_RAW_EN|VIF_HIGH_SPEED_MODE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_FALSE);
        
        if(MMP_TRUE == bRaw2ISP)
            MMPH_HIF_CmdSetParameterL(0, MMP_FALSE);
        else
            MMPH_HIF_CmdSetParameterL(0, MMP_TRUE);

        MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|ENABLE_RAWSTORE);
        ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
        if (ret != MMP_ERR_NONE) 
            return ret;
    }
    else {
        MMPH_HIF_CmdSetParameterL(0, bEnable);
        MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|ENABLE_RAWSTORE);
        ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);

        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_TRUE);
        MMPH_HIF_RegSetB(RAWPROC_MODE_SEL, MMPH_HIF_RegGetB(RAWPROC_MODE_SEL) & ~(RAWPROC_STORE_EN|RAWPROC_F_BURST_EN));
        if(bRaw2ISP)                    //Ted ZSL
            MMPH_HIF_RegSetB(VIF_RAW_OUT_EN, 0);
        else
        MMPH_HIF_RegSetB(VIF_RAW_OUT_EN, MMPH_HIF_RegGetB(VIF_RAW_OUT_EN) & ~(VIF_HIGH_SPEED_MODE));
        MMPH_HIF_RegSetB(RAWPROC_SRC_SEL, MMPH_HIF_RegGetB(RAWPROC_SRC_SEL)&~RAWPROC_SRC_VIF);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
        if (ret != MMP_ERR_NONE)
            return ret;
    }
    if (bLumaEn) {
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
        MMPH_HIF_RegSetB(RAWPROC_LUMA_MODE, MMPH_HIF_RegGetB(RAWPROC_LUMA_MODE) | RAWPROC_LUMA_EN);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
    }
    else {
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
        MMPH_HIF_RegSetB(RAWPROC_LUMA_MODE, MMPH_HIF_RegGetB(RAWPROC_LUMA_MODE) & ~RAWPROC_LUMA_EN);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
    }
#endif    
    return MMP_ERR_NONE;
}
        
//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_SetStoreBuf
//  Description :
//------------------------------------------------------------------------------
/** @brief Set raw store buffer address
Set raw store buffer address
@param[in] rawbuf  bayer & luma buffer address
@return It reports the status of the configuration.
*/
MMP_ERR MMPD_RAWPROC_SetStoreBuf(MMPD_RAWPROC_STOREBUF *rawbuf)
{
#if RAWPROC_EN==1
    MMP_ULONG i;
    MMP_ERR ret;

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);

    if (rawbuf->ulRawBufCnt > 0)
        MMPH_HIF_RegSetL(RAWPROC_S_ADDR, rawbuf->ulRawBufAddr[0]); // 1st store addr
    if (rawbuf->ulLumaBufCnt > 0)
        MMPH_HIF_RegSetL(RAWPROC_LUMA_ADDR, rawbuf->ulLumaBufAddr[0]);
        
    MMPH_HIF_CmdSetParameterL(0, rawbuf->ulRawBufCnt);
    for (i = 0; i < rawbuf->ulRawBufCnt; i++)
        MMPH_HIF_CmdSetParameterL((i+1)*4, rawbuf->ulRawBufAddr[i]);
    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW | RAWSTORE_BAYER_ADDR);
    ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
    if (ret != MMP_ERR_NONE) {
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
        return ret;
    }
        
    MMPH_HIF_CmdSetParameterL(0, rawbuf->ulLumaBufCnt);
    for (i = 0; i < rawbuf->ulLumaBufCnt; i++)
        MMPH_HIF_CmdSetParameterL((i+1)*4, rawbuf->ulLumaBufAddr[i]);
    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW | RAWSTORE_LUMA_ADDR);
    ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
    if (ret != MMP_ERR_NONE) {
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
        return ret;
    }

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
#endif    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_SetStoreAddr
//  Description :
//------------------------------------------------------------------------------
/** @brief Set raw store buffer address

Set raw store buffer address

@param[in] ulAddr1  the first buffer address
@param[in] ulAddr2  the second buffer address
@param[in] ulBufNum total buffer numbers

@return It reports the status of the configuration.
*/
MMP_ERR MMPD_RAWPROC_SetStoreAddr(MMP_ULONG ulAddr1, MMP_ULONG ulAddr2,MMP_ULONG ulBufNum)
{
    MMP_ERR ret = MMP_ERR_NONE;
#if RAWPROC_EN==1
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
    
    MMPH_HIF_RegSetL(RAWPROC_S_ADDR, ulAddr1); // 1st store addr
    
    MMPH_HIF_CmdSetParameterL(0, ulAddr1);
    MMPH_HIF_CmdSetParameterL(4, ulAddr2);
    MMPH_HIF_CmdSetParameterL(8, ulBufNum);
    
    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|RAWSTORE_ADDR);
    ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
    
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
    
#endif	
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_EnablePreview
//  Description :
//------------------------------------------------------------------------------
/** @brief Enable raw preview

Enable raw preview

@param[in] bEnable  Enable/Disable

@return It reports the status of the configuration.
*/
MMP_ERR MMPD_RAWPROC_EnablePreview(MMP_BOOL bEnable)
{
    MMP_ERR ret = MMP_ERR_NONE;
#if RAWPROC_EN==1
    MMPH_HIF_CmdSetParameterL(0, bEnable);
        
    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|ENABLE_RAWPREVIEW);
    ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
#endif    
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_EnableZeroShutterLagPath
//  Description :
//------------------------------------------------------------------------------
/** @brief Enable Zero Shutter lag path

Enable Zero Shutter lag path

@param[in] bEnable  Enable/Disable

@return It reports the status of the configuration.
*/
MMP_ERR MMPD_RAWPROC_EnableZeroShutterLagPath(MMP_BOOL bEnable)   //Ted ZSL
{
    MMP_ERR ret = MMP_ERR_NONE;
#if RAWPROC_EN==1
    MMPH_HIF_CmdSetParameterL(0, bEnable);
        
    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|ENABLE_ZEROSHUTTERLAG_PATH);
    ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
#endif    
    return ret;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_EnableDVSPreview
//  Description :
//------------------------------------------------------------------------------
/** @brief Enable VIS preview

Enable raw preview

@param[in] bEnable  Enable/Disable

@return It reports the status of the configuration.
*/
#if PCAM_EN==0
MMP_ERR MMPD_RAWPROC_EnableDVSPreview(MMP_BOOL bEnable)
{
    MMP_ERR ret = MMP_ERR_NONE;
    
    MMPH_HIF_CmdSetParameterL(0, bEnable);

    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|ENABLE_DVSPREVIEW);
    ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
    
    return ret;
}
#endif
//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_EnableStoreDownsample
//  Description :
//------------------------------------------------------------------------------
/** @brief Enable raw store downsample

Enable raw store downsample

@param[in] bEnable  Enable/Disable
@param[out] ulRawStoreWidth   raw store width
@param[out] ulRawStoreHeight  raw store height

@return It reports the status of the configuration.
*/
MMP_ERR MMPD_RAWPROC_EnableStoreDownsample(MMP_BOOL bEnable,MMP_USHORT *usRawStoreWidth,MMP_USHORT *usRawStoreHeight)
{
#if RAWPROC_EN==1
    
    //currently, Fix to 1/2 downsample
    MMP_ULONG ulDownRatio = 2;
    MMP_ULONG ulWidth,ulHeight;
    
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
    
    MMPH_HIF_CmdSetParameterL(0, bEnable);
        
    if ( bEnable ) {
        MMPH_HIF_CmdSetParameterL(4, ulDownRatio);
        
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_TRUE);
        ulWidth  = MMPH_HIF_RegGetW(VIF_GRAB_PIXL_ED) - MMPH_HIF_RegGetW(VIF_GRAB_PIXL_ST);
        ulHeight = MMPH_HIF_RegGetW(VIF_GRAB_LINE_ED) - MMPH_HIF_RegGetW(VIF_GRAB_LINE_ST);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_FALSE);
        
        *usRawStoreWidth  = ((ulWidth  + (ulDownRatio>>1) )/ ulDownRatio) - 8;
        *usRawStoreHeight = ((ulHeight + (ulDownRatio>>1) )/ ulDownRatio) - 8;
    
    }
    
    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|ENABLE_RAW_DOWNSAMPLE);
    MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);

    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
    
#endif    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_RAWPROC_ResetPreviewAttribute
//  Description :
//------------------------------------------------------------------------------
/** @brief reset raw preview attribute

reset raw preview attributes

@return It reports the status of the configuration.
*/
MMP_ERR MMPD_RAWPROC_ResetPreviewAttribute()
{
#if RAWPROC_EN==1
    
    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|RESET_PREVIEW_ATTRIBUTE);
    MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
    
#endif    
    return MMP_ERR_NONE;
}

MMP_ERR MMPD_RAWPROC_StartDVS(MMP_BOOL bStart)
{
    MMP_ERR ret = MMP_ERR_NONE;
#if RAWPROC_EN==1
    
    MMPH_HIF_CmdSetParameterB(0, bStart);

    MMPH_HIF_CmdSend(HIF_CMD_SET_RAW_PREVIEW|START_DVS_ME);
    ret = MMPC_System_CheckMMPStatus(SENSOR_CMD_IN_EXEC);
#endif    
    return ret;
}

MMP_ERR MMPD_RAWPROC_GetStoreRange(MMP_USHORT *usWidth, MMP_USHORT *usHeight)
{
#if RAWPROC_EN==1
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_TRUE);
    *usWidth = MMPH_HIF_RegGetW(VIF_GRAB_PIXL_ED) - MMPH_HIF_RegGetW(VIF_GRAB_PIXL_ST) + 1;
    *usHeight = MMPH_HIF_RegGetW(VIF_GRAB_LINE_ED) - MMPH_HIF_RegGetW(VIF_GRAB_LINE_ST) + 1;
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_FALSE);
#endif
    return MMP_ERR_NONE;
}

MMP_ERR MMPD_RAWPROC_SetFetchRange(MMP_USHORT usWidth, MMP_USHORT usHeight)
{
#if RAWPROC_EN==1
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
    MMPH_HIF_RegSetW(RAWPROC_F_H_BYTE, usWidth);
    MMPH_HIF_RegSetW(RAWPROC_F_V_BYTE, usHeight);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
#endif
    return MMP_ERR_NONE;
}

MMP_ERR MMPD_RAWPROC_SetStoreDownsample(MMP_UBYTE ubDn, MMP_UBYTE ubLumaDn)
{
#if RAWPROC_EN==1
    if (ubDn < 1)
        ubDn = 1;
    if (ubLumaDn < 2) // ubLumaDn should be even numbers, eg. 2, 4, 6...
        ubLumaDn = 2;
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
    MMPH_HIF_RegSetB(RAWPROC_S_SAMP_DN, ubDn-1);
    MMPH_HIF_RegSetB(RAWPROC_LUMA_SAMP_DN, ubLumaDn/2 - 1); // 1/2(n+1)
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
#endif
    return MMP_ERR_NONE;
}

#ifdef BUILD_CE
#define BUILD_FW
#endif

