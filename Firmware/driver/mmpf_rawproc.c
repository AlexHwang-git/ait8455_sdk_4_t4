//==============================================================================
//
//  File        : mmpf_rawproc.c
//  Description : JPEG DSC function
//  Author      : Ted Huang
//  Revision    : 1.0
//
//==============================================================================
#include "includes_fw.h"
#include "config_fw.h"

#include "lib_retina.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_rawproc.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_video.h"
#include "mmpf_rawproc.h"
#include "mmp_err.h"
//#include "mmpf_dvs.h"
#include "mmpf_scaler.h"
#if (VIDEO_R_EN)
#include "mmpf_mp4venc.h"
#endif

#include "mmpf_sensor.h"

#if PCAM_EN==1
#define RAWPROC_DOWNSIZE    (!CAPTURE_BAYER_RAW_ENABLE)
#endif
#if 1 //RAWPROC_DOWNSIZE==0
/** @addtogroup MMPF_RAWPROC
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
static MMP_ULONG    m_ulRawStoreAddr[3];
static MMP_ULONG    m_ulRawStoreBufferNum   = 0;
static MMP_ULONG    m_ulLumaStoreAddr[3];
static MMP_ULONG    m_ulLumaStoreBufferNum  = 0;
static MMP_BOOL     m_bFirstRawFrame        = MMP_TRUE;
static MMP_ULONG    m_ulRawStoreIndex       = 0;
static MMP_ULONG    m_ulZeroShutterLagBuffer   = 0;        //Ted ZSL
static MMP_BOOL     m_bStoringBuffer   = 0;                //Ted ZSL
static MMP_ULONG    m_ulLumaStoreIndex  = 0;
static MMP_BOOL     m_bEnableRawDownSample      = MMP_FALSE;
static MMP_ULONG    m_ulRawStoreDownSampleRatio = 1;
static MMP_ULONG    m_ulRawPathZoomLevel        = 0;
static MMP_ULONG    m_ulRawPathGrabStartX[4];
static MMP_ULONG    m_ulRawPathGrabStartY[4];
static MMP_ULONG    m_ulRawPathGrabOffsetX[4];
static MMP_ULONG    m_ulRawPathGrabOffsetY[4];
static MMP_ULONG    m_ulSensorRawStartX;
static MMP_ULONG    m_ulSensorRawStartY;
static MMP_ULONG    m_ulSensorRawEndX;
static MMP_ULONG    m_ulSensorRawEndY;
static MMP_ULONG    m_ulSensorColorID;
static MMP_ULONG    m_ulRawResetZoomOP = MMP_FALSE;
static MMP_BOOL     m_bMainPipeRawPreGrab      = MMP_FALSE;

MMP_BOOL gbDVSStartSig = MMP_FALSE;

extern	MMP_UBYTE	m_gbSystemCoreID;

#if (DSC_R_EN)||(VIDEO_R_EN)
extern MMPF_SENSOR_FUNCTION *gsSensorFunction;
#endif

static MMPF_RAWPROC_STORE_MODULE m_RawStoreModule;
MMPF_RAWPROC_BUFATTRIBUTE        m_BayerStoreAttribute;
MMPF_RAWPROC_BUFATTRIBUTE        m_LumaStoreAttribute;
//==============================================================================
//
//                          MODULE VARIABLES
//
//==============================================================================
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetFetchPixelDelay
//----------------------------------------------------------------------------
/** @brief set pixel delay

The function set pixel delay for RAWPROC

@param[in] PixelDelay  PixelDelay

@return MMP_ERR_NONE
*/

MMP_ERR MMPF_RAWPROC_SetFetchPixelDelay(MMP_UBYTE PixelDelay)
{
    AITPS_RAWPROC  pRaw   = AITC_BASE_RAWPROC;

    if(PixelDelay > 7){     //rawproc max. pixel delay, line delay
        PixelDelay = 7;
    }

    pRaw->RAWPROC_F_TIME_CTL    = PixelDelay;
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetFetchLineDelay
//----------------------------------------------------------------------------
/** @brief set line delay

The function set line delay for RAWPROC

@param[in] PixelDelay  PixelDelay

@return MMP_ERR_NONE
*/

MMP_ERR MMPF_RAWPROC_SetFetchLineDelay(MMP_UBYTE LineDelay)
{
    AITPS_RAWPROC  pRaw   = AITC_BASE_RAWPROC;

    pRaw->RAWPROC_F_LINE_TIME   = LineDelay;
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetFetchTiming
//----------------------------------------------------------------------------
/** @brief set pixel delay/line delay

The function set pixel delay/line delay for RAWPROC

@return MMP_ERR_NONE
*/

MMP_ERR MMPF_RAWPROC_SetFetchTiming(MMP_UBYTE PixelDelay, MMP_USHORT LineDelay)                  
{
    AITPS_RAWPROC  pRAW   = AITC_BASE_RAWPROC;
 
    RTNA_DBG_Str3("Raw SetFetchAttribute\r\n");

    if(PixelDelay > 7){     //rawproc max. pixel delay, line delay
        PixelDelay = 7;
    }

    pRAW->RAWPROC_F_TIME_CTL    = PixelDelay;
    pRAW->RAWPROC_F_LINE_TIME   = LineDelay;
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_Reset
//----------------------------------------------------------------------------
/** @brief Reset the RAWPROC module

The function reset the RAWPROC module

@return MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_Reset(void)
{
    AITPS_GBL   pGBL = AITC_BASE_GBL;

    pGBL->GBL_RST_CTL01 |= GBL_RAW_RST;
    RTNA_WAIT_CYCLE(100);
    pGBL->GBL_RST_CTL01 &= (~GBL_RAW_RST);
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_EnableRawPath
//----------------------------------------------------------------------------
/** @brief Enable/Disable RAWPROC path

The function enable or disable the RAWPROC path

@param[in] bEnable  Enable/Disable
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_EnableRawPath(MMP_BOOL bEnable)
{

    AITPS_RAWPROC   pRAW = AITC_BASE_RAWPROC;
    AITPS_VIF       pVIF = AITC_BASE_VIF;
	MMP_UBYTE       vid = VIF_CTL;
#if (CHIP == P_V2)
    if (bEnable) {
        *(volatile MMP_BYTE *)0x8000040A |= (1<<7);             //currently, it didn't have ISP datasheet
    }
    else {
        *(volatile MMP_BYTE *)0x8000040A &= ~(1<<7);             //currently, it didn't have ISP datasheet
    }
#endif    
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    if (bEnable) {
        *(volatile MMP_BYTE *)0x8000700A |= (1<<7);             
    }
    else {
        *(volatile MMP_BYTE *)0x8000700A &= ~(1<<7);             
    }
#endif

    #if RAW_PROC_10_BIT_EN
    pRAW->RAWPROC_MODE_SEL          = RAWPROC_STORE_EN | RAWPROC_F_BURST_EN | RAWPROC_10BIT_MODE ;  
    #else    
    pRAW->RAWPROC_MODE_SEL          = RAWPROC_STORE_EN | RAWPROC_F_BURST_EN ;  
    #endif  
   // pRAW->RAWPROC_F_BUSY_MODE_CTL = RAWPROC_BUSY_MODE_EN;
    pVIF->VIF_RAW_OUT_EN[vid]   |=   VIF_2_RAW_EN;
	
    m_bFirstRawFrame    = MMP_TRUE;
    m_ulRawStoreIndex   = 0;
    m_ulLumaStoreIndex  = 0;
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_ConfigRawPathMemory
//----------------------------------------------------------------------------
/** @brief Configure the RAWPROC frame stored address

The function config the frame address

@param[in] ulAddr1  the first frame address
@param[in] ulAddr2  the sencode frame address
@param[in] ulBufNum the number of frame buffers
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_ConfigRawPathMemory(MMP_ULONG ulAddr1,MMP_ULONG ulAddr2,MMP_ULONG ulBufNum)
{
    m_ulRawStoreBufferNum       = ulBufNum;
    m_ulRawStoreAddr[0]         = ulAddr1;
    
    if(m_ulRawStoreBufferNum > 1) {
        m_ulRawStoreAddr[1]     = ulAddr2;
    }
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_StoreSensorData
//----------------------------------------------------------------------------
/** @brief Store the sensor input frame data

The function store the sensor input frame data
@param[in]  bCapture    store for preview or store for capture
@param[in]  bWaitFinish Wait for store finished

@return MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_StoreSensorData(MMP_BOOL bCapture,MMP_BOOL bWaitFinish)
{
    AITPS_RAWPROC   pRAW = AITC_BASE_RAWPROC;
    AITPS_VIF       pVIF = AITC_BASE_VIF;
	MMP_UBYTE       vid = VIF_CTL;

    if (!bCapture) {
        #if (CHIP == P_V2)
    	if (m_ulLumaStoreBufferNum)
        	pRAW->RAWPROC_LUMA_ADDR = m_ulLumaStoreAddr[(m_ulLumaStoreIndex+1)%m_ulLumaStoreBufferNum];
        #endif
        pRAW->RAWPROC_S_ADDR        = m_ulRawStoreAddr[(m_ulRawStoreIndex+1)%m_ulRawStoreBufferNum];
        pVIF->VIF_RAW_OUT_EN[vid]   |=   VIF_2_RAW_EN;
		m_ulRawStoreIndex = (m_ulRawStoreIndex + 1) % m_ulRawStoreBufferNum;
    }
    else {
        #if RAW_PROC_10_BIT_EN
        pRAW->RAWPROC_MODE_SEL          = RAWPROC_STORE_EN | RAWPROC_F_BURST_EN | RAWPROC_10BIT_MODE ;  
        #else    
        pRAW->RAWPROC_MODE_SEL          = RAWPROC_STORE_EN | RAWPROC_F_BURST_EN ;  
        #endif        
        pVIF->VIF_RAW_OUT_EN[vid]       |= (VIF_2_ISP_EN | VIF_2_RAW_EN);
    }
    if(bWaitFinish) {
        while (pVIF->VIF_RAW_OUT_EN[vid] & VIF_2_RAW_EN);
        m_bStoringBuffer = 0;   
    }

    m_bStoringBuffer = 1;        
    return MMP_ERR_NONE;
}


//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_FetchSensorData
//----------------------------------------------------------------------------

/** @brief Fetch the sensor input frame data

The function fetch the sensor input frame data

@param[in]  bCapture    store for preview or store for capture
@param[in]  bWaitFinish Wait for store finished
@param[in]  bRotateDMA   Wait for store finished
@param[in]  ulRotateAddr Rotate buffer addr

@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_FetchSensorData(MMP_BOOL bCapture,MMP_BOOL bWaitFinish,MMPF_RAW_FETCH_ROTATE_TYPE RotateType,MMP_ULONG ulRotateAddr)
{
    AITPS_VIF       pVIF = AITC_BASE_VIF;
    AITPS_RAWPROC   pRAW = AITC_BASE_RAWPROC;
    MMP_USHORT      fetch_w, fetch_h;
    MMP_UBYTE       bTempISPColorID;
    MMP_UBYTE       vid = VIF_CTL;
        
    if ((m_bFirstRawFrame) && (!bCapture)){
        if (m_ulLumaStoreBufferNum)
        	m_ulLumaStoreIndex   = (m_ulLumaStoreIndex + 1) % m_ulLumaStoreBufferNum;
        m_ulRawStoreIndex   = (m_ulRawStoreIndex + 1) % m_ulRawStoreBufferNum;
        m_bFirstRawFrame    = MMP_FALSE;
    }
    else {
        if (pRAW->RAWPROC_MODE_SEL & RAWPROC_FETCH_EN) {
            RTNA_DBG_Str3("Raw fetch busy !\r\n");
            return MMP_ERR_NONE;
        }

        fetch_w                     = (pVIF->VIF_GRAB[vid].PIXL_ED - pVIF->VIF_GRAB[vid].PIXL_ST + 1)/m_ulRawStoreDownSampleRatio;
    	fetch_h                     = (pVIF->VIF_GRAB[vid].LINE_ED - pVIF->VIF_GRAB[vid].LINE_ST + 1)/m_ulRawStoreDownSampleRatio;
        
        if ( !bCapture ) {
            pRAW->RAWPROC_F_ADDR    = m_ulRawStoreAddr[m_ulRawStoreIndex];
        }
        
        if ( ( MMPF_RAW_FETCH_ROTATE_RIGHT_90  == RotateType ) ||
             ( MMPF_RAW_FETCH_ROTATE_RIGHT_270 == RotateType )
             )
        {
            pRAW->RAWPROC_F_ADDR        = ulRotateAddr;
            
            pRAW->RAWPROC_F_H_BYTE      = fetch_h;
            pRAW->RAWPROC_F_V_BYTE      = fetch_w;
        	pRAW->RAWPROC_F_ST_OFST     = 0;
            pRAW->RAWPROC_F_PIX_OFST    = 1;
            pRAW->RAWPROC_F_LINE_OFST   = fetch_h;
        }
        else {
            if ( MMPF_RAW_FETCH_ROTATE_RIGHT_180  == RotateType ) {
                pRAW->RAWPROC_F_ADDR        = ulRotateAddr;
            }
            
        pRAW->RAWPROC_F_H_BYTE      = fetch_w;
        pRAW->RAWPROC_F_V_BYTE      = fetch_h;
    	pRAW->RAWPROC_F_ST_OFST     = 0;
        pRAW->RAWPROC_F_PIX_OFST    = 1;
        pRAW->RAWPROC_F_LINE_OFST   = fetch_w;
        }

        #if (CHIP == P_V2)
        bTempISPColorID = *(MMP_BYTE *)0x80000400;

		*(MMP_BYTE *)0x80000400     = ((*(MMP_BYTE *)0x80000400) ) ^ ((pVIF->VIF_SENSR_CTL[vid]&VIF_COLORID_FORMAT_MASK)<<2);
        // Set ISP src to rawproc, frame sync.
        *((MMP_UBYTE*)0x8000040A)   = *((MMP_UBYTE*)0x8000040A) | (1 << 7);
        #endif
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        bTempISPColorID = (pVIF->VIF_SENSR_CTL[vid]&VIF_COLORID_FORMAT_MASK) >> 2;
		
		//*(MMP_BYTE *)0x80007000     = ((*(MMP_BYTE *)0x80007000) ) ^ ((pVIF->VIF_SENSR_CTL[vid]&VIF_COLORID_FORMAT_MASK)<<2);
        *(volatile MMP_BYTE *)0x8000700A  = (*(volatile MMP_BYTE *)0x8000700A) &~ 0x30;
        *((MMP_UBYTE*)0x8000700A) |= *((MMP_UBYTE*)0x8000700A) | (1 << 7);
        #endif

        if( !bCapture ) {
            pRAW->RAWPROC_MODE_SEL |= (RAWPROC_FETCH_EN|RAWPROC_F_BURST_EN);
        }
        else {
            pRAW->RAWPROC_MODE_SEL |= (RAWPROC_FETCH_EN|RAWPROC_F_BURST_EN);
        }
        
        // test flow
        if( !bCapture ) {
            //m_ulRawStoreIndex = (m_ulRawStoreIndex + 1) % m_ulRawStoreBufferNum;
        }
        if (bWaitFinish) {
            while((pRAW->RAWPROC_MODE_SEL)&RAWPROC_FETCH_EN);
        }

        #if (CHIP == P_V2)
		*(volatile MMP_BYTE *)0x80000400 = bTempISPColorID;
		#endif    
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        /*resume ISP color ID*/
       *(volatile MMP_BYTE *)0x8000700A |= (bTempISPColorID << 5);

        ISP_IF_IQ_SetOpr(0x700A, 1, *(volatile MMP_BYTE *)0x8000700A);
		#endif        
    }
    
    return MMP_ERR_NONE;
}


//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SwitchZeroShutterLagStoreBuffer
//----------------------------------------------------------------------------

/** @brief Fetch the sensor input frame data

The function fetch the sensor input frame data
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SwitchZeroShutterLagStoreBuffer(void)
{
    m_ulZeroShutterLagBuffer = m_ulRawStoreIndex;
    m_ulRawStoreIndex = (m_ulRawStoreIndex + 1) % m_ulRawStoreBufferNum;

    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetZeroShutterLagFetchBuffer
//----------------------------------------------------------------------------

/** @brief Fetch the sensor input frame data

The function fetch the sensor input frame data
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetZeroShutterLagFetchBuffer(void)
{
    AITPS_RAWPROC   pRAW    = AITC_BASE_RAWPROC;

    pRAW->RAWPROC_F_ADDR    = m_ulRawStoreAddr[m_ulZeroShutterLagBuffer];
    
    VAR_L(3,pRAW->RAWPROC_F_ADDR);
    VAR_L(3,m_ulZeroShutterLagBuffer);
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_PollingRawStoreDone
//----------------------------------------------------------------------------

/** @brief Fetch the sensor input frame data

The function fetch the sensor input frame data
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_CheckRawStoreDone(void)
{
    AITPS_VIF       pVIF = AITC_BASE_VIF;
    MMP_UBYTE       vid = VIF_CTL;

    if(m_bStoringBuffer){
        while (pVIF->VIF_RAW_OUT_EN[vid] & VIF_2_RAW_EN);
        m_bStoringBuffer = 0;
    }
    return MMP_ERR_NONE;
}


//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_FetchGrabData
//----------------------------------------------------------------------------

/** @brief Fetch a grabed range of the sensor input frame data

@param[in]  grab_x  horizontal grab start location
@param[in]  grab_y  vertical grab start location

@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_FetchGrabData(MMP_SHORT grab_x, MMP_SHORT grab_y)
{
    AITPS_VIF pVIF = AITC_BASE_VIF;
    AITPS_RAWPROC pRAW = AITC_BASE_RAWPROC;
    MMP_USHORT  line_ofst;
    MMP_UBYTE   vid = VIF_CTL;

    if (pRAW->RAWPROC_MODE_SEL & RAWPROC_FETCH_EN) {
        RTNA_DBG_Str3("Raw fetch busy !\r\n");
        return MMP_SYSTEM_ERR_HW;
    }
    grab_x &= ~1; // Avoid color id fault, need check
    grab_y &= ~1;
    grab_x += 1;
    grab_y += 1;
    #if 0
    DBG_S(3, "Grab: ");
    RTNA_DBG_Short(3, grab_x);
    RTNA_DBG_Short(3, grab_y);
    DBG_S(3, "\r\n");
    #endif

	line_ofst = pVIF->VIF_GRAB[vid].PIXL_ED - pVIF->VIF_GRAB[vid].PIXL_ST + 1;

    pRAW->RAWPROC_F_ADDR        = m_ulRawStoreAddr[m_ulRawStoreIndex];
	pRAW->RAWPROC_F_ST_OFST     = grab_y*line_ofst + grab_x;
    pRAW->RAWPROC_F_PIX_OFST    = 1;
    pRAW->RAWPROC_F_LINE_OFST   = line_ofst;

    // Set ISP src to rawproc, frame sync.
#if (CHIP == P_V2)
    *((AIT_REG_B*)0x8000040A) = *((AIT_REG_B*)0x8000040A) | (1 << 7);
#endif
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    *(volatile MMP_BYTE *)0x8000700A |= (1<<7);             
#endif
    pRAW->RAWPROC_MODE_SEL    |= RAWPROC_FETCH_EN;
    //VAR_L(3, pRAW->RAWPROC_F_ADDR);

    m_ulRawStoreIndex = (m_ulRawStoreIndex + 1) % m_ulRawStoreBufferNum;
    if (m_ulLumaStoreBufferNum)
        m_ulLumaStoreIndex = (m_ulLumaStoreIndex + 1) % m_ulLumaStoreBufferNum;

    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetCaptureStoreAddr
//----------------------------------------------------------------------------

/** @brief Set store/fetch address for jpeg capture

The function set store/fetch address for jpeg capture

@return MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetCaptureStoreAddr()
{
    AITPS_RAWPROC   pRAW    = AITC_BASE_RAWPROC;

    
    pRAW->RAWPROC_S_ADDR    = m_ulRawStoreAddr[0];
    pRAW->RAWPROC_F_ADDR    = m_ulRawStoreAddr[0];
    
    VAR_L(3,m_ulRawStoreAddr[0]);
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_GetCaptureStoreAddr
//----------------------------------------------------------------------------

/** @brief Get store address for jpeg capture

The function get store address for jpeg capture

@return MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_GetCaptureStoreAddr(MMP_ULONG *ulAddr, MMP_BOOL bZeroShutterLag)
{
    if ( !bZeroShutterLag ) {
        *ulAddr = m_ulRawStoreAddr[0];
    }
    else {
        *ulAddr = m_ulRawStoreAddr[m_ulZeroShutterLagBuffer];
    }
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetRawBuffer
//----------------------------------------------------------------------------

/** @brief Set one raw store bayer buffer by index
@param[in]  ubBufIdx  the i-th bayer buffer
@param[in]  ulAddr  buffer address

@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetRawBuffer(MMP_UBYTE ubBufIdx, MMP_ULONG ulAddr)
{
    m_ulRawStoreAddr[ubBufIdx] = ulAddr;

    if ((ubBufIdx+1) > m_ulRawStoreBufferNum)
        m_ulRawStoreBufferNum = ubBufIdx + 1; // update buffer count

    VAR_L(3, ubBufIdx);
    VAR_L(3, m_ulRawStoreAddr[ubBufIdx]);
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetRawBuffer
//----------------------------------------------------------------------------

/** @brief Set one raw store bayer buffer by index
@param[in]  ubBufIdx  the i-th bayer buffer
@param[in]  ulAddr  buffer address

@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetLumaBuffer(MMP_UBYTE ubBufIdx, MMP_ULONG ulAddr)
{
    m_ulLumaStoreAddr[ubBufIdx] = ulAddr;
    if ((ubBufIdx+1) > m_ulLumaStoreBufferNum)
        m_ulLumaStoreBufferNum = ubBufIdx + 1; // update buffer count

    VAR_L(3, ubBufIdx);
    VAR_L(3, m_ulLumaStoreAddr[ubBufIdx]);
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_GetLumaBuffer
//----------------------------------------------------------------------------

/** @brief Get current luma store buffer address

@return             MMP_ERR_NONE
*/
MMP_ULONG MMPF_RAWPROC_GetLumaBuffer(void)
{
    return m_ulLumaStoreAddr[m_ulLumaStoreIndex];
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_EnableDownsample
//----------------------------------------------------------------------------
/** @brief Enable/Disable raw store downsample

The function enable or disable raw store downsample

@param[in] bEnable  Enable/Disable
@param[in] ulRatio  downsample ratio
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_EnableDownsample(MMP_BOOL bEnable,MMP_ULONG ulRatio)
{
    AITPS_RAWPROC   pRAW    = AITC_BASE_RAWPROC;
    
    m_bEnableRawDownSample = bEnable;
    
    if (bEnable) {
        m_ulRawStoreDownSampleRatio = ulRatio;
    }
    else {
        m_ulRawStoreDownSampleRatio = 1;
    }

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    pRAW->RAWPROC_S_DNSAMP_H = (pRAW->RAWPROC_S_DNSAMP_V = (m_ulRawStoreDownSampleRatio - 1));
    #endif
    #if (CHIP == P_V2)
    pRAW->RAWPROC_S_SAMP_DN = (m_ulRawStoreDownSampleRatio - 1);
    #endif

    return MMP_ERR_NONE;
}



//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_ResetPreviewAttribue
//----------------------------------------------------------------------------
/** @brief reset variables for preview path

The function reset variables for raw preview path

@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_ResetPreviewAttribue()
{
    MMP_ULONG   i;
    
    m_ulRawStoreDownSampleRatio = 1;
    m_ulRawResetZoomOP          = MMP_FALSE;
    m_bFirstRawFrame    = MMP_TRUE;
    m_bEnableRawDownSample      = MMP_FALSE;

    m_ulRawPathZoomLevel        = 0;
    
    for (i=0;i<4;i++) {
        m_ulRawPathGrabStartX[i]  = 0;
        m_ulRawPathGrabStartY[i]  = 0;
        m_ulRawPathGrabOffsetX[i] = 0;
        m_ulRawPathGrabOffsetY[i] = 0;
    }

    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_GetVIFSetting
//----------------------------------------------------------------------------
/** @brief backup original VIF setting 

The function backup original VIF setting

@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_GetVIFSetting()
{
    AITPS_VIF       pVIF = AITC_BASE_VIF;
    static  MMP_UBYTE bColorIDValid = 0;
    MMP_UBYTE       vid = VIF_CTL;
    
    m_ulSensorRawStartX = pVIF->VIF_GRAB[vid].PIXL_ST;
    m_ulSensorRawEndX   = pVIF->VIF_GRAB[vid].PIXL_ED;
    m_ulSensorRawStartY = pVIF->VIF_GRAB[vid].LINE_ST;
    m_ulSensorRawEndY   = pVIF->VIF_GRAB[vid].LINE_ED;
    
    if(bColorIDValid == 0){
    m_ulSensorColorID   = pVIF->VIF_SENSR_CTL[vid];
	bColorIDValid = 1;
    }
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_ResetZoomOP
//----------------------------------------------------------------------------
/** @brief restore original VIF/RAWPROC setting 

The function restore original VIF/RAWPROC setting 

@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_ResetZoomOP()
{
    AITPS_VIF       pVIF    = AITC_BASE_VIF;
    AITPS_RAWPROC   pRAW    = AITC_BASE_RAWPROC;
    MMP_UBYTE       vid = VIF_CTL;
    
    pVIF->VIF_GRAB[vid].PIXL_ST = m_ulSensorRawStartX;
    pVIF->VIF_GRAB[vid].PIXL_ED = m_ulSensorRawEndX;
    pVIF->VIF_GRAB[vid].LINE_ST = m_ulSensorRawStartY;
    pVIF->VIF_GRAB[vid].LINE_ED = m_ulSensorRawEndY;
    
    pVIF->VIF_SENSR_CTL[vid] = m_ulSensorColorID;

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    pRAW->RAWPROC_S_DNSAMP_H = (pRAW->RAWPROC_S_DNSAMP_V = 0);
    #endif
    #if (CHIP == P_V2)
    pRAW->RAWPROC_S_SAMP_DN     = 0;
    #endif
    
    m_ulRawStoreDownSampleRatio = 1;
    m_ulRawResetZoomOP          = MMP_TRUE;
    
    pRAW->RAWPROC_F_TIME_CTL    = 0;
    pRAW->RAWPROC_F_LINE_TIME   = 2;
    
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetZoomOffset
//----------------------------------------------------------------------------
/** @brief Set grab offset for raw path zoom-in/zoom-out

The function set grab offset for raw path zoom-in/zoom-out

@param[in] direction  zoom-in/zoom-out
@param[in] ulOffset   grab ofset
@return MMP_ERR_NONE
*/

MMP_ERR MMPF_RAWPROC_SetZoomOffset(MMPF_SCALER_ZOOMDIRECTION  direction,MMP_ULONG ulOffset)
{
    if (direction == MMPF_SCALER_ZOOMIN) {
            m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel] += ulOffset;
            m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel] += ulOffset;
    }
    else if (direction == MMPF_SCALER_ZOOMOUT) {
            m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel] -= ulOffset;
            m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel] -= ulOffset;
    }

    return MMP_ERR_NONE;
}


//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_CalcRawFetchTiming
//----------------------------------------------------------------------------
/** @brief Calculate raw fetch timing

The function calculate raw fetch timing

@param[in]  gScalerPath: the timing calculation is depended on this path
@param[in]  mode: timingmode
@return MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_CalcRawFetchTiming(MMPF_SCALER_PATH gScalerPath, MMPF_RAWPROC_FETCH_TIMING_MODE mode )
{
    MMP_USHORT	            usTempLineDelay, usTempPixDelay;

    RTNA_DBG_Str0("set rawproc line delay and pixel delay\r\n");
    // calculate pixel delay and line delay according to Scaler limilation
    //if( mode == MMPF_RAWPROC_DEFAULT)
    {
        usTempPixDelay = 0;
        while(1){       
            MMPF_Scaler_GetScaleUpHBlanking(gScalerPath, usTempPixDelay,  (MMP_ULONG *)&usTempLineDelay);

            usTempLineDelay = (usTempLineDelay+15 ) >> 4;      // rawproc line delya * 16
            if(usTempLineDelay <= 0xFFF)
                break;
          
            usTempPixDelay ++;
            if(usTempPixDelay >7){   //
                usTempPixDelay = 7;
                usTempLineDelay = 0xFFF;
                break;
            }
        }
    }

    #if (CHIP == P_V2) || (CHIP == VSN_V2)
    if (usTempLineDelay < 8) {
        usTempLineDelay = 8;
    }
    #endif
    #if (CHIP == VSN_V3)
    #endif

    MMPF_RAWPROC_SetFetchTiming((MMP_UBYTE)usTempPixDelay, usTempLineDelay);

    return MMP_ERR_NONE;

}


//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetSubPipeZoomOP
//----------------------------------------------------------------------------
/** @brief Set zoom OP for raw path zoom-in/zoom-out

The function set zoom OP for raw path zoom-in/zoom-out

@param[in]  bRawDownSample  raw store downsample or not
@param[in]  CurZoomPathSel  zoom path
@param[out] fitrange        fit range
@param[out] grabCtl         grab range
@param[out] grabCtl         maingrabCtl grab range
@return MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetSubPipeZoomOP(MMP_BOOL                bRawDownSample,
                               MMPF_SCALER_PATH        CurZoomPathSel, 
                               MMPF_SCALER_FIT_RANGE   *fitrange,
                               MMPF_SCALER_GRABCONTROL *grabCtl,
                               MMPF_SCALER_GRABCONTROL *maingrabCtl)
{
    MMP_ULONG   max_scale_width;
    MMP_ULONG   out_width, out_height;
    MMP_ULONG   x_scale, y_scale;

    if ( bRawDownSample ) {
        RTNA_DBG_Str (3,"MMPF_RAWPROC_AdjustSubPipeZoomOP Not support two pipes \r\n");
    }
    else {
        if (CurZoomPathSel == MMPF_SCALER_PATH_0) {
		    max_scale_width = SCALER_PATH0_MAX_WIDTH;
	    }
	    else {
		    max_scale_width = SCALER_PATH1_MAX_WIDTH;
	    }

        if (m_bMainPipeRawPreGrab){
            /*Get sub pipe out size*/
            out_width = grabCtl->usEndX - grabCtl->usStartX;
            out_height = grabCtl->usEndY - grabCtl->usStartY;
            
            /*Follow Main pipe scaler ratio*/
            x_scale = (out_width * grabCtl->usScaleM + fitrange->usInWidth - 1) / fitrange->usInWidth;
            y_scale = (out_height * grabCtl->usScaleM + fitrange->usInHeight - 1) / fitrange->usInHeight;

            if (x_scale > y_scale)
                grabCtl->usScaleN = x_scale;
            else
                grabCtl->usScaleN = y_scale;

            /*Follow Main pipe input resolution*/
            //fitrange->usInWidth  = mainfitrange->usInWidth;
            //fitrange->usInHeight = mainfitrange->usInWidth;

            /*Calculate new grab offset*/
            grabCtl->usStartX = 1;
            grabCtl->usEndX = grabCtl->usStartX + out_width;
            grabCtl->usStartY = 1;
            grabCtl->usEndY = grabCtl->usStartY + out_height;

            /*
            VAR_L(3,grabCtl->usScaleN);
            VAR_L(3,grabCtl->usScaleM);
            VAR_L(3,grabCtl->usEndX);
            VAR_L(3,grabCtl->usEndY);
            */
        }
    }
 
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetMainPipeZoomOP
//----------------------------------------------------------------------------
/** @brief Set zoom OP for raw path zoom-in/zoom-out

The function set zoom OP for raw path zoom-in/zoom-out

@param[in]  bRawDownSample  raw store downsample or not
@param[in]  ScalerZoomInfo  zoom info
@param[in]  CurZoomPathSel  zoom path
@param[in]  N_offset        offset
@param[out] fitrange        fit range
@param[out] grabCtl         grab range
@return MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetMainPipeZoomOP(MMP_BOOL                bRawDownSample,
                               MMPF_SCALER_ZOOM_INFO   *ScalerZoomInfo,
                               MMPF_SCALER_PATH        CurZoomPathSel, 
                               MMP_ULONG               N_offset,
                               MMPF_SCALER_FIT_RANGE   *fitrange,
                               MMPF_SCALER_GRABCONTROL *grabCtl)
{
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    MMP_ULONG   ulTargetN;
    MMP_ULONG   ulVIFStartX, ulVIFEndX;
    MMP_ULONG   ulVIFStartY, ulVIFEndY;
    MMP_ULONG   ulGrabWidth, ulGrabHeight;
    MMP_ULONG   ulScaleM;
    MMP_ULONG   ulOffsetX, ulOffsetY;
	MMP_ULONG   ulTargetWidth, ulTargetHeight;
    MMP_ULONG   max_scale_width;
    MMP_ULONG   unscale_width,unscale_height;
    
    if ( bRawDownSample ) {
        ulTargetN       = ScalerZoomInfo[CurZoomPathSel].grabCtl.usScaleN;
    	ulScaleM        = ScalerZoomInfo[CurZoomPathSel].grabCtl.usScaleM;
    	
    	if (((ulTargetN*2) < ScalerZoomInfo[CurZoomPathSel].grabCtl.usScaleM))
    	{
    	    //DBG_S3("1\r\n");
    	    MMPF_RAWPROC_EnableDownsample(MMP_TRUE,2);
    	    
    	    if ( (m_ulRawPathZoomLevel == 1) || m_ulRawResetZoomOP){
    	        
    	        
    	        ulTargetWidth                                       = (ScalerZoomInfo[CurZoomPathSel].usInputWidth);
        	    ulTargetHeight                                      = (ScalerZoomInfo[CurZoomPathSel].usInputHeight);
        	    
        	    ulVIFStartX                                         = m_ulSensorRawStartX;//3;
                ulVIFEndX                                           = m_ulSensorRawEndX - 8;//ulVIFStartX + ulTargetWidth - 1;
                ulVIFStartY                                         = m_ulSensorRawStartY;//21;
                ulVIFEndY                                           = m_ulSensorRawEndY - 8;//ulVIFStartY + ulTargetHeight - 1;
                
                pVIF->VIF_GRAB[vid].PIXL_ST                         = ulVIFStartX;
                pVIF->VIF_GRAB[vid].PIXL_ED                         = ulVIFEndX + 8;
                pVIF->VIF_GRAB[vid].LINE_ST                         = ulVIFStartY;
                pVIF->VIF_GRAB[vid].LINE_ED                         = ulVIFEndY + 8;
                
    	        m_ulRawPathZoomLevel                                = 0;
    	        
    	        if (m_ulRawResetZoomOP)
    	            m_ulRawResetZoomOP = MMP_FALSE;
    	    }
    	    
    	    grabCtl->usScaleN    = ScalerZoomInfo[CurZoomPathSel].grabCtl.usScaleN * 2;
    	    fitrange->usInWidth  = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 2);
            fitrange->usInHeight = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 2);
    	}
    	else if (((ulTargetN) < ScalerZoomInfo[CurZoomPathSel].grabCtl.usScaleM))
    	{
    	    //DBG_S3("2\r\n");
    	    
    	    MMPF_RAWPROC_EnableDownsample(MMP_TRUE,1);
    	    
    	    if ( (m_ulRawPathZoomLevel == 0) || m_ulRawResetZoomOP ){
        	    
        	    
        	    ulTargetWidth                                       = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 2);
        	    ulTargetHeight                                      = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 2);
        	    
        	    ulOffsetX                                           = ScalerZoomInfo[CurZoomPathSel].grabCtl.usStartX;
        	    ulOffsetY                                           = ScalerZoomInfo[CurZoomPathSel].grabCtl.usStartY;
                
                ulOffsetX                                           = 2 * ((ulOffsetX*ulScaleM + (ulTargetN*2-1))/(ulTargetN*2));
                ulOffsetY                                           = 2 * ((ulOffsetY*ulScaleM + (ulTargetN*2-1))/(ulTargetN*2));
                
        	    ulVIFStartX                                         = 3  + ulOffsetX;
                ulVIFEndX                                           = ulVIFStartX + ulTargetWidth;
                ulVIFStartY                                         = 21 + ulOffsetY;
                ulVIFEndY                                           = ulVIFStartY + ulTargetHeight;
                
                
                pVIF->VIF_GRAB[vid].PIXL_ST                         = ulVIFStartX;
                pVIF->VIF_GRAB[vid].PIXL_ED                         = ulVIFEndX + 8;
                pVIF->VIF_GRAB[vid].LINE_ST                         = ulVIFStartY;
                pVIF->VIF_GRAB[vid].LINE_ED                         = ulVIFEndY + 8;
        	    
        	    m_ulRawPathZoomLevel                                = 1;
        	    m_ulRawPathGrabStartX[m_ulRawPathZoomLevel]         = ulVIFStartX;
        	    m_ulRawPathGrabStartY[m_ulRawPathZoomLevel]         = ulVIFStartY;
        	    m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]        = 0;
                m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]        = 0;
                
                if (m_ulRawResetZoomOP)
    	            m_ulRawResetZoomOP = MMP_FALSE;
    	    }
    	    else {
    	        if(m_ulRawPathZoomLevel == 2) {
    	            ulTargetWidth                                   = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 2);
            	    ulTargetHeight                                  = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 2);
            	    
            	    m_ulRawPathZoomLevel                            = 1;
            	    
            	    ulVIFStartX                                     = m_ulRawPathGrabStartX[m_ulRawPathZoomLevel];
                    ulVIFEndX                                       = ulVIFStartX + ulTargetWidth;
                    ulVIFStartY                                     = m_ulRawPathGrabStartY[m_ulRawPathZoomLevel];
                    ulVIFEndY                                       = ulVIFStartY + ulTargetHeight;
                    
                    pVIF->VIF_GRAB[vid].PIXL_ST                     = ulVIFStartX;
                    pVIF->VIF_GRAB[vid].PIXL_ED                     = ulVIFEndX + 8;
                    pVIF->VIF_GRAB[vid].LINE_ST                     = ulVIFStartY;
                    pVIF->VIF_GRAB[vid].LINE_ED                     = ulVIFEndY + 8;
                    
                    
                    m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]    -= ((ScalerZoomInfo[CurZoomPathSel].usStepX/1) * N_offset);
                    m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]    -= ((ScalerZoomInfo[CurZoomPathSel].usStepY/1) * N_offset);
                    //MMPF_RAWPROC_SetFetchPixelDelay(0);
                    //MMPF_RAWPROC_SetFetchLineDelay(2);
    	        }
    	    }
    	    
    	    ulGrabWidth         = grabCtl->usEndX - grabCtl->usStartX;
            ulGrabHeight        = grabCtl->usEndY - grabCtl->usStartY;
            grabCtl->usStartX    = 1 + (m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]+1)/2;//m_ulGrabOffsetX1;
    	    grabCtl->usEndX      = grabCtl->usStartX + ulGrabWidth;
    	    grabCtl->usStartY    = 1 + (m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]+1)/2;//m_ulGrabOffsetY1;
    	    grabCtl->usEndY      = grabCtl->usStartY + ulGrabHeight;
    	    fitrange->usInWidth  = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 2);
            fitrange->usInHeight = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 2);
            
            /*
            VAR_L(3,grabCtl->usStartX);
            VAR_L(3,grabCtl->usEndX);
            VAR_L(3,grabCtl->usStartY);
            VAR_L(3,grabCtl->usEndY);
            VAR_L(3,fitrange->usInWidth);
            VAR_L(3,fitrange->usInHeight);
            */
    	}
    	//4X ~ 8X
    	else if ( (ulTargetN >=  ScalerZoomInfo[CurZoomPathSel].grabCtl.usScaleM  ) &&
    	          (ulTargetN <  (ScalerZoomInfo[CurZoomPathSel].grabCtl.usScaleM*2)) )
    	{   
    	    //DBG_S3("3\r\n");
    	    if ( (m_ulRawPathZoomLevel == 1) || (m_ulRawResetZoomOP) ) {
                ulTargetWidth                                       = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 4);
                ulTargetHeight                                      = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 4);

                ulOffsetX = (m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]+1)/2;
                ulOffsetY = (m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]+1)/2;

                ulOffsetX = 2 * ((ulOffsetX*ulScaleM + (ulTargetN*2-1))/(ulTargetN*2));
                ulOffsetY = 2 * ((ulOffsetY*ulScaleM + (ulTargetN*2-1))/(ulTargetN*2));

                ulVIFStartX                                         = m_ulRawPathGrabStartX[m_ulRawPathZoomLevel] + 
                                                                      ulOffsetX;
                ulVIFEndX                                           = ulVIFStartX + ulTargetWidth;
                ulVIFStartY                                         = m_ulRawPathGrabStartY[m_ulRawPathZoomLevel] + 
                                                                      ulOffsetY;
                ulVIFEndY                                           = ulVIFStartY + ulTargetHeight;

                pVIF->VIF_GRAB[vid].PIXL_ST                         = ulVIFStartX;
                pVIF->VIF_GRAB[vid].PIXL_ED                         = ulVIFEndX + 8;
                pVIF->VIF_GRAB[vid].LINE_ST                         = ulVIFStartY;
                pVIF->VIF_GRAB[vid].LINE_ED                         = ulVIFEndY + 8;

                m_ulRawPathZoomLevel                                = 2;
                m_ulRawPathGrabStartX[m_ulRawPathZoomLevel]         = ulVIFStartX;
                m_ulRawPathGrabStartY[m_ulRawPathZoomLevel]         = ulVIFStartY;
                m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]        = 0;
                m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]        = 0;
    	    }
    	    else {
    	        if(m_ulRawPathZoomLevel == 3) {
                    ulTargetWidth                                   = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 4);
                    ulTargetHeight                                  = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 4);

                    m_ulRawPathZoomLevel                            = 2;

                    ulVIFStartX                                     = m_ulRawPathGrabStartX[m_ulRawPathZoomLevel];
                    ulVIFEndX                                       = ulVIFStartX + ulTargetWidth;
                    ulVIFStartY                                     = m_ulRawPathGrabStartY[m_ulRawPathZoomLevel];
                    ulVIFEndY                                       = ulVIFStartY + ulTargetHeight;

                    pVIF->VIF_GRAB[vid].PIXL_ST                     = ulVIFStartX;
                    pVIF->VIF_GRAB[vid].PIXL_ED                     = ulVIFEndX + 8;
                    pVIF->VIF_GRAB[vid].LINE_ST                     = ulVIFStartY;
                    pVIF->VIF_GRAB[vid].LINE_ED                     = ulVIFEndY + 8;


                    m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]    -= ((ScalerZoomInfo[CurZoomPathSel].usStepX/1) * N_offset);
                    m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]    -= ((ScalerZoomInfo[CurZoomPathSel].usStepY/1) * N_offset);
                    
    	        }
    	    }
    	    
    	    ulGrabWidth         = grabCtl->usEndX - grabCtl->usStartX;
            ulGrabHeight        = grabCtl->usEndY - grabCtl->usStartY;
            
            grabCtl->usStartX    = 1 + (m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel] + 2)/4;//m_ulGrabOffsetX2;
    	    grabCtl->usEndX      = grabCtl->usStartX + ulGrabWidth;
    	    grabCtl->usStartY    = 1 + (m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel] + 2)/4;//m_ulGrabOffsetY2;
    	    grabCtl->usEndY      = grabCtl->usStartY + ulGrabHeight;
    	    fitrange->usInWidth  = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 4);
            fitrange->usInHeight = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 4);
            
            
            
            
            
    	}
    	else {
            if ( (m_ulRawPathZoomLevel == 2) || (m_ulRawResetZoomOP) ) {
                ulTargetWidth                                       = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 8);
                ulTargetHeight                                      = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 8);

                ulOffsetX = (m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]+2)/4;
                ulOffsetY = (m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]+2)/4;

                ulOffsetX = 2 * ((ulOffsetX*ulScaleM + (ulTargetN*2-1))/(ulTargetN*2));
                ulOffsetY = 2 * ((ulOffsetY*ulScaleM + (ulTargetN*2-1))/(ulTargetN*2));


                ulVIFStartX                                         = m_ulRawPathGrabStartX[m_ulRawPathZoomLevel] + 
                                                                      ulOffsetX;
                ulVIFEndX                                           = ulVIFStartX + ulTargetWidth;
                ulVIFStartY                                         = m_ulRawPathGrabStartY[m_ulRawPathZoomLevel] + 
                                                                      ulOffsetY;
                ulVIFEndY                                           = ulVIFStartY + ulTargetHeight;

                pVIF->VIF_GRAB[vid].PIXL_ST                         = ulVIFStartX;
                pVIF->VIF_GRAB[vid].PIXL_ED                         = ulVIFEndX + 8;
                pVIF->VIF_GRAB[vid].LINE_ST                         = ulVIFStartY;
                pVIF->VIF_GRAB[vid].LINE_ED                         = ulVIFEndY + 8;


                m_ulRawPathZoomLevel                                = 3;
                m_ulRawPathGrabStartX[m_ulRawPathZoomLevel]         = ulVIFStartX;
                m_ulRawPathGrabStartY[m_ulRawPathZoomLevel]         = ulVIFStartY;
                m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]        = 0;
                m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]        = 0;
    	    }
    	    
    	    ulGrabWidth         = grabCtl->usEndX - grabCtl->usStartX;
            ulGrabHeight        = grabCtl->usEndY - grabCtl->usStartY;
            grabCtl->usStartX    = 1 + (m_ulRawPathGrabOffsetX[m_ulRawPathZoomLevel]+4)/8;//m_ulGrabOffsetX2;
    	    grabCtl->usEndX      = grabCtl->usStartX + ulGrabWidth;
    	    grabCtl->usStartY    = 1 + (m_ulRawPathGrabOffsetY[m_ulRawPathZoomLevel]+4)/8;//m_ulGrabOffsetY2;
    	    grabCtl->usEndY      = grabCtl->usStartY + ulGrabHeight;
    	    fitrange->usInWidth  = (ScalerZoomInfo[CurZoomPathSel].usInputWidth  / 8);
            fitrange->usInHeight = (ScalerZoomInfo[CurZoomPathSel].usInputHeight / 8);
        }
    }
    else {
        if (CurZoomPathSel == MMPF_SCALER_PATH_0) {
		    max_scale_width = SCALER_PATH0_MAX_WIDTH;
	    }
	    else {
		    max_scale_width = SCALER_PATH1_MAX_WIDTH;
	    }
	
        if ((fitrange->usInWidth * grabCtl->usScaleN / grabCtl->usScaleM > max_scale_width)||(grabCtl->usScaleN > grabCtl->usScaleM)){
            if((grabCtl->usScaleN > grabCtl->usScaleM)){    //scale up function: UP((length_h-1)*Nh/Mh) * UP((length_v-1)*Nv/Mv)
                unscale_width  = 2 + ((grabCtl->usEndX - grabCtl->usStartX ) * grabCtl->usScaleM ) / grabCtl->usScaleN;
                unscale_height = 2 + ((grabCtl->usEndY - grabCtl->usStartY ) * grabCtl->usScaleM ) / grabCtl->usScaleN;
            }
            else {  //scale down function: DOWN((length_h)*Nh/Mh) * DOWN((length)*Nv/Mv)
                unscale_width  = ((grabCtl->usEndX - grabCtl->usStartX + 1) * grabCtl->usScaleM + (grabCtl->usScaleN - 1)) / grabCtl->usScaleN;
                unscale_height = ((grabCtl->usEndY - grabCtl->usStartY + 1) * grabCtl->usScaleM + (grabCtl->usScaleN - 1)) / grabCtl->usScaleN;
            }
            
            ulVIFStartX = (grabCtl->usStartX * grabCtl->usScaleM + grabCtl->usScaleN - 1) / grabCtl->usScaleN;
            ulVIFEndX   = ulVIFStartX + unscale_width;
		    ulVIFStartY = (grabCtl->usStartY * grabCtl->usScaleM + grabCtl->usScaleN - 1) / grabCtl->usScaleN;
		    ulVIFEndY   = ulVIFStartY + unscale_height;
		    
		    
		    pVIF->VIF_GRAB[vid].PIXL_ST                         = ulVIFStartX;
            pVIF->VIF_GRAB[vid].PIXL_ED                         = ulVIFEndX + 8;
            pVIF->VIF_GRAB[vid].LINE_ST                         = ulVIFStartY;
            pVIF->VIF_GRAB[vid].LINE_ED                         = ulVIFEndY + 8;
            
            fitrange->usInWidth  = unscale_width;
            fitrange->usInHeight = unscale_height;
            
            unscale_width  = grabCtl->usEndX - grabCtl->usStartX + 1;
            unscale_height = grabCtl->usEndY - grabCtl->usStartY + 1;
            
		    grabCtl->usStartX = 1;
		    grabCtl->usEndX   = grabCtl->usStartX + unscale_width - 1;
		    grabCtl->usStartY = 1;
		    grabCtl->usEndY   = grabCtl->usStartY + unscale_height - 1;
		    
		    
		    
            m_bMainPipeRawPreGrab = MMP_TRUE;
        }
        else {
            pVIF->VIF_GRAB[vid].PIXL_ST                         = m_ulSensorRawStartX;//1;
            pVIF->VIF_GRAB[vid].PIXL_ED                         = m_ulSensorRawEndX;//(ScalerZoomInfo[CurZoomPathSel].usInputWidth) + 8;
            pVIF->VIF_GRAB[vid].LINE_ST                         = m_ulSensorRawStartY;//1;
            pVIF->VIF_GRAB[vid].LINE_ED                         = m_ulSensorRawEndY;//(ScalerZoomInfo[CurZoomPathSel].usInputHeight) + 8;
            
            m_bMainPipeRawPreGrab = MMP_FALSE;
            //pVIF->VIF_SENSR_CTL = m_ulSensorColorID;
        }
    }
   
    if ( (pVIF->VIF_GRAB[vid].PIXL_ST & 1) != (m_ulSensorRawStartX & 1) )
    {
        if (m_ulSensorColorID & VIF_PIXL_ID_POLAR)
            pVIF->VIF_SENSR_CTL[vid] &= ~(VIF_PIXL_ID_POLAR);
        else if ( !(m_ulSensorColorID & VIF_PIXL_ID_POLAR) )
            pVIF->VIF_SENSR_CTL[vid] |= (VIF_PIXL_ID_POLAR);
    }
    else if ( (pVIF->VIF_GRAB[vid].PIXL_ST & 1) == (m_ulSensorRawStartX & 1) )
    {
        if (m_ulSensorColorID & VIF_PIXL_ID_POLAR)
            pVIF->VIF_SENSR_CTL[vid] |= (VIF_PIXL_ID_POLAR);
        else if ( !(m_ulSensorColorID & VIF_PIXL_ID_POLAR) )
            pVIF->VIF_SENSR_CTL[vid] &= ~(VIF_PIXL_ID_POLAR);
    }
    
    if ( (pVIF->VIF_GRAB[vid].LINE_ST & 1) != (m_ulSensorRawStartY & 1) )
    {
        if (m_ulSensorColorID & VIF_LINE_ID_POLAR)
            pVIF->VIF_SENSR_CTL[vid] &= ~(VIF_LINE_ID_POLAR);
        else if ( !(m_ulSensorColorID & VIF_LINE_ID_POLAR) )
            pVIF->VIF_SENSR_CTL[vid] |= (VIF_LINE_ID_POLAR);
    }
    else if ( (pVIF->VIF_GRAB[vid].LINE_ST & 1) == (m_ulSensorRawStartY & 1) )
    {
        if (m_ulSensorColorID & VIF_LINE_ID_POLAR)
            pVIF->VIF_SENSR_CTL[vid] |= (VIF_LINE_ID_POLAR);
        else if ( !(m_ulSensorColorID & VIF_LINE_ID_POLAR) )
            pVIF->VIF_SENSR_CTL[vid] &= ~(VIF_LINE_ID_POLAR);
    }

            
    return MMP_ERR_NONE;
}


//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetStoreRange
//----------------------------------------------------------------------------

/** @brief Change vif grab end to change store range

The function change vif grab end to change store range
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetStoreRange(MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    AITPS_VIF   pVIF    = AITC_BASE_VIF;
    MMP_UBYTE   vid     = VIF_CTL;

	pVIF->VIF_GRAB[vid].PIXL_ED = pVIF->VIF_GRAB[vid].PIXL_ST + usWidth - 1;
	pVIF->VIF_GRAB[vid].LINE_ED = pVIF->VIF_GRAB[vid].LINE_ST + usHeight - 1;

    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetStoreRange
//----------------------------------------------------------------------------

/** @brief Change vif grab end to change store range

The function change vif grab end to change store range
@return             MMP_ERR_NONE
*/

MMP_ERR MMPF_RAWPROC_SetFetchRange(MMP_USHORT usWidth, MMP_USHORT usHeight)
{
    AITPS_RAWPROC pRAW = AITC_BASE_RAWPROC;

    pRAW->RAWPROC_F_H_BYTE = usWidth;
    pRAW->RAWPROC_F_V_BYTE = usHeight;

#if 1 //update isp 3a acc param
	#if (DSC_R_EN)||(VIDEO_R_EN)
    gsSensorFunction->MMPF_Sensor_SetISPWindow(usWidth, usHeight);
    #endif
#else
    gSensorConfig.VIFGrab_H_Length = usWidth;
    gSensorConfig.VIFGrab_V_Length = usHeight;
    VR_AE_SetAccParams();
    VR_AWB_SetAccParams();
    VR_AF_SetAccParams();
#endif
    return MMP_ERR_NONE;
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_SetAttributes
//----------------------------------------------------------------------------

/** @brief Set raw store attribute

The function set raw store attribute and get some parameter from VIF.
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_SetAttributes(MMPF_RAWPROC_STORE_MODULE  bayerall, MMPF_RAWPROC_BUFATTRIBUTE *attribute) 
{
    AITPS_VIF   pVIF    = AITC_BASE_VIF;
    MMP_UBYTE   vid     = VIF_CTL;

	*attribute->usWidth = pVIF->VIF_GRAB[vid].PIXL_ED - pVIF->VIF_GRAB[vid].PIXL_ST + 1;
	*attribute->usHeight = pVIF->VIF_GRAB[vid].LINE_ED - pVIF->VIF_GRAB[vid].LINE_ST + 1;
	*attribute->usLineOffset = *attribute->usWidth / attribute->ubDownsampleRatio;
	if (bayerall == MMPF_RAW_STORE_BAYER) {
		m_BayerStoreAttribute.colordepth = attribute->colordepth;
		m_BayerStoreAttribute.ubDownsampleRatio = attribute->ubDownsampleRatio;
		m_BayerStoreAttribute.ulBaseAddr = attribute->ulBaseAddr;
	}
	else if (bayerall == MMPF_RAW_STORE_LUMA) {
		m_LumaStoreAttribute.colordepth = attribute->colordepth;
		m_LumaStoreAttribute.ubDownsampleRatio = attribute->ubDownsampleRatio;
		m_LumaStoreAttribute.ulBaseAddr = attribute->ulBaseAddr;	
	}
	m_RawStoreModule |= bayerall;

	return MMP_ERR_NONE;
}
//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_StoreData
//----------------------------------------------------------------------------

/** @brief Store the sensor data by raw store attribute

The function store the sensor data.
@param[in]  ubFeatures  High speed enable or not.
@param[in]  bWaitFinish  Wait store data finish or not.
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_StoreData(MMP_BOOL bWaitFinish) 
{
	AITPS_RAWPROC pRAW = AITC_BASE_RAWPROC;
	AITPS_VIF   pVIF    = AITC_BASE_VIF;
	MMP_UBYTE   vid     = VIF_CTL;

	pRAW->RAWPROC_MODE_SEL     = 0;
	#if (CHIP == P_V2)
	pRAW->RAWPROC_LUMA_MODE    = 0;
	#endif
	if (m_RawStoreModule & MMPF_RAW_STORE_BAYER) {
		pRAW->RAWPROC_S_ADDR       = m_BayerStoreAttribute.ulBaseAddr;

        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        pRAW->RAWPROC_S_DNSAMP_H = (pRAW->RAWPROC_S_DNSAMP_V = (m_BayerStoreAttribute.ubDownsampleRatio - 1));
        #endif
        #if (CHIP == P_V2)
		pRAW->RAWPROC_S_SAMP_DN    = m_BayerStoreAttribute.ubDownsampleRatio - 1;
		#endif

		pRAW->RAWPROC_MODE_SEL     = RAWPROC_STORE_EN; 
	}
	#if (CHIP == P_V2)
	if (m_RawStoreModule & MMPF_RAW_STORE_LUMA) {
		pRAW->RAWPROC_LUMA_ADDR    = m_LumaStoreAttribute.ulBaseAddr;			
		pRAW->RAWPROC_LUMA_SAMP_DN = m_LumaStoreAttribute.ubDownsampleRatio - 1;
		pRAW->RAWPROC_LUMA_MODE    = RAWPROC_LUMA_EN;
	}
	#endif

	pVIF->VIF_RAW_OUT_EN[vid]   |= VIF_2_RAW_EN;
    if(bWaitFinish) {
        while (pVIF->VIF_RAW_OUT_EN[vid] & VIF_2_RAW_EN);
    }
	return MMP_ERR_NONE;
}


MMP_ERR MMPF_RAWPROC_CalStoredData(MMP_ULONG RawStoreAddr) {

	unsigned char *Addr = (unsigned char*)RawStoreAddr;
	int i=0;
	int width = m_ulSensorRawEndX - m_ulSensorRawStartX;
	//int height = m_ulSensorRawEndY - m_ulSensorRawStartY;
	
	for(i=0;i<width;i++){
	 	RTNA_DBG_Byte(0, Addr[i]);
		RTNA_DBG_Str(0, ",");
	} 
	return 0;   
}


/** @}*/ //end of MMPF_RAWPROC

#else
static MMP_ULONG    m_ulLumaStoreAddr[3];
static MMP_ULONG    m_ulLumaStoreBufferNum  = 0;
//static MMP_BOOL     m_bFirstRawFrame        = MMP_TRUE;
//static MMP_ULONG    m_ulRawStoreIndex       = 0;
static MMP_ULONG    m_ulLumaStoreIndex  = 0;
static MMP_BOOL     m_bStoringBuffer   = 0;                //Ted ZSL
//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_GetLumaBuffer
//----------------------------------------------------------------------------

/** @brief Get current luma store buffer address

@return             MMP_ERR_NONE
*/
MMP_ULONG MMPF_RAWPROC_GetLumaBuffer(void)
{
    return m_ulLumaStoreAddr[m_ulLumaStoreIndex];
}

//----------------------------------------------------------------------------
// Function Name       : MMPF_RAWPROC_PollingRawStoreDone
//----------------------------------------------------------------------------

/** @brief Fetch the sensor input frame data

The function fetch the sensor input frame data
@return             MMP_ERR_NONE
*/
MMP_ERR MMPF_RAWPROC_CheckRawStoreDone(void)
{
    AITPS_VIF   pVIF    = AITC_BASE_VIF;
    MMP_UBYTE   vid     = VIF_CTL;

    if(m_bStoringBuffer){
        while (pVIF->VIF_RAW_OUT_EN[vid] & VIF_2_RAW_EN);
        m_bStoringBuffer = 0;
    }
    return MMP_ERR_NONE;
}


#endif

