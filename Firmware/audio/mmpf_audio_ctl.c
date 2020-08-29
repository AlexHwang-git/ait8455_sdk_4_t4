#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_audio_ctl.h"
#include "mmpf_audio.h"
#include "mmpf_i2s_ctl.h"
#include "config_fw.h"
#include "mmpf_pll.h"
#include "mmp_reg_audio.h"
#include "mmps_3gprecd.h"
#if PCAM_EN==1
#include "mmpf_encapi.h"
#endif
#include "mmp_reg_usb.h"
#include "mmpf_usbuac.h"

#if SUPPORT_UAC==1
//#include "maxloud.h"
/** @addtogroup MMPF_AUDIO
@{
*/

#if ADC_PERFORMANCE_TEST==1
void MMPF_Audio_TestPerformance(void);
#endif

extern MMP_UBYTE gbOrigADCDigitalGain   ;

static MMP_ULONG glAudioSamplerate = 0; //must default initialize

MMP_UBYTE    gbUseAitADC;
MMP_UBYTE    gbUseAitDAC;

MMPF_AUDIO_LINEIN_CHANNEL	m_audioLineInChannel;

MMP_UBYTE    gbAudioOutI2SFormat;
MMP_UBYTE    gbAudioInI2SFormat;

MMP_UBYTE    gbDACDigitalGain = DEFAULT_DAC_DIGITAL_GAIN;
MMP_UBYTE    gbDACAnalogGain = DEFAULT_DAC_ANALOG_GAIN;
MMP_UBYTE    gbADCDigitalGain = DEFAULT_ADC_DIGITAL_GAIN;
MMP_UBYTE    gbADCAnalogGain = DEFAULT_ADC_ANALOG_GAIN;
#if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
MMP_UBYTE    gbDACLineOutGain = DEFAULT_DAC_LINEOUT_GAIN; 
// Test case
// 50cm, need to have -27dB volume
MMP_UBYTE    gbADCBoost =  AFE_FIX_BOOST_GAIN ; // Change 1 or 0 to 20dB,30dB,40dB for Python_v2
#endif
MMP_BOOL     gbHasInitAitDAC = MMP_FALSE;
MMP_BOOL     gbAitADCEn = MMP_TRUE;

MMP_USHORT  gsAudioEncodeFormat;
MMP_USHORT  gsAudioRecEvent;
MMP_ULONG   glAudioEncodeLength;
AUDIO_ENC_INBUF_HANDLE AudioEncInBufHandle;

//extern short        numOutSamples;
extern MMPF_OS_FLAGID SYS_Flag_Hif;

extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_SHORT   gsAudioSamplesPerFrame,gsInAudioSamplesPerFrame ;




//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_AACRecordIntHandler
//  Parameter   : None
//  Return Value : None
//  Description : AMR record interrupt handler
//------------------------------------------------------------------------------

#include "mmpf_usbvend.h"
extern void uac_process_audio_data(void);
#pragma arm section code = "aud_enc", rwdata = "aud_enc",  zidata = "aud_enc"
MMP_ERR MMPF_Audio_DropDataHandler(MMP_USHORT usPath)
{
    #if (MIC_SOURCE==MIC_IN_PATH_AFE)
    AITPS_AFE	pAFE = AITC_BASE_AFE;
    #endif
    #if (MIC_SOURCE==MIC_IN_PATH_I2S)
    AITPS_AUD   pAUD    = AITC_BASE_AUD;
    #endif
    
    MMP_USHORT	i,th ;
    MMP_SHORT	tmp ;//  *ptempBuf;
    //th =  (usPath == I2S_PATH) ? pAUD->I2S_FIFO_RD_TH : pAUD->AFE_FIFO_RD_TH ;
    if (usPath == I2S_PATH) {
        #if (MIC_SOURCE==MIC_IN_PATH_I2S)
        th = pAUD->I2S_FIFO_RD_TH ;
        for(i = 0; i < (th>>1); i++) {
            tmp = pAUD->I2S_FIFO_DATA;
            tmp = pAUD->I2S_FIFO_DATA;
        }
        #endif    	
    } else {
        #if (MIC_SOURCE==MIC_IN_PATH_AFE)
        th = pAFE->AFE_FIFO_RD_TH ;
        for(i = 0; i < (th>>1); i++) {
            tmp = pAFE->AFE_FIFO_DATA;
            tmp = pAFE->AFE_FIFO_DATA;
        }	
        #endif        
    }
    return 0;
}

MMP_ERR MMPF_Audio_EncodeIntHandler(MMP_USHORT usPath)
{
#define OUT_FIXED_VAL   0//(20000)
    static int swap_fixed_val = 0 ;
    #if (MIC_SOURCE==MIC_IN_PATH_AFE)
    AITPS_AFE	pAFE = AITC_BASE_AFE;
    #endif
    #if (MIC_SOURCE==MIC_IN_PATH_I2S)
    AITPS_AUD   pAUD    = AITC_BASE_AUD;
    #endif
	MMP_SHORT	*pInbuf=NULL;
	MMP_USHORT	i,th ;
	MMP_SHORT	tmp ;//  *ptempBuf;
#if UPSAMPLE_32K_48K==1
    if(gsAudioSamplesPerFrame!=gsInAudioSamplesPerFrame) {
        pInbuf = (MMP_SHORT *)MMPF_Audio_GetWrPtr();
        pInbuf += AudioEncInBufHandle.afe_intr_i * gsInAudioSamplesPerFrame*AFE_SAMPLES_TIME ;
    } else {
        pInbuf = (MMP_SHORT *)MMPF_Audio_GetWrPtr();
    }
#else    
	pInbuf = (MMP_SHORT *)MMPF_Audio_GetWrPtr();
#endif	
	
	if (usPath == I2S_PATH) {
	#if (MIC_SOURCE==MIC_IN_PATH_I2S)
	    th = pAUD->I2S_FIFO_RD_TH ;
		for(i = 0; i < (th>>1); i++) {
			*pInbuf++ = pAUD->I2S_FIFO_DATA;
			*pInbuf++ = pAUD->I2S_FIFO_DATA;
		}
    #endif		
	}
	else {
	#if (MIC_SOURCE==MIC_IN_PATH_AFE)
	    th = pAFE->AFE_FIFO_RD_TH ;
		switch (m_audioLineInChannel) {
		case MMPF_AUDIO_LINEIN_DUAL:
			#if OUT_FIXED_VAL == 0
			for(i = 0; i < (th>>1); i++) {
			    *pInbuf++ = pAFE->AFE_FIFO_DATA ;
			    *pInbuf++ = pAFE->AFE_FIFO_DATA;
			}
			#endif
			#if OUT_FIXED_VAL > 0    
			for(i = 0; i < (th>>1); i++) {
			    if(swap_fixed_val&1) {
    			    *pInbuf++ = OUT_FIXED_VAL   ; tmp = pAFE->AFE_FIFO_DATA ;
    			    *pInbuf++ = 0-OUT_FIXED_VAL ; tmp = pAFE->AFE_FIFO_DATA;
			    } else {
    			    *pInbuf++ = 0-OUT_FIXED_VAL   ; tmp = pAFE->AFE_FIFO_DATA ;
    			    *pInbuf++ = OUT_FIXED_VAL ; tmp = pAFE->AFE_FIFO_DATA;
			    }
			       
			}	
			swap_fixed_val++; 
			#endif	    
			break;
		case MMPF_AUDIO_LINEIN_L:
			for(i = 0; i < (th>>1); i++) {
				tmp = pAFE->AFE_FIFO_DATA;
				*pInbuf++ = tmp;
				*pInbuf++ = tmp;
				tmp = pAFE->AFE_FIFO_DATA;
			}	
			break;
		case MMPF_AUDIO_LINEIN_R:
			for(i = 0; i < (th>>1); i++) {
				tmp = pAFE->AFE_FIFO_DATA;
				tmp = pAFE->AFE_FIFO_DATA;
				*pInbuf++ = tmp;
				*pInbuf++ = tmp;
			}	
			break;
		}
	#endif	
	}		
	return	MMP_ERR_NONE;
}
#pragma arm section code , rwdata,  zidata

//------------------------------------------------------------------------------
//  Function    : MMPF_AFE_ISR
//  Parameter   : None
//  Return Value : None
//  Description : AFE fifo isr
//------------------------------------------------------------------------------
#pragma arm section code = "afe_isr", rwdata = "afe_isr",  zidata = "afe_isr"
MMP_ULONG glAFETicks;

void    MMPF_AFE_ISR(void)
{
extern MMP_UBYTE   gbAud1stInToken ;

    AITPS_AUD   pAUD    = AITC_BASE_AUD;
#if (MIC_SOURCE==MIC_IN_PATH_AFE) ||  (MIC_SOURCE==MIC_IN_PATH_BOTH)
	#if (CHIP == VSN_V3)
	AITPS_AFE   pAFE    = AITC_BASE_AFE;
	#endif
#endif
    MMP_ULONG 	irq_src;
    
	#if (MIC_SOURCE==MIC_IN_PATH_AFE)    
		irq_src = (pAFE->AFE_FIFO_CPU_INT_EN & pAFE->AFE_FIFO_CSR);
	#elif (MIC_SOURCE==MIC_IN_PATH_I2S)
		irq_src = (pAUD->I2S_FIFO_CPU_INT_EN & pAUD->I2S_FIFO_SR);
	#elif (MIC_SOURCE==MIC_IN_PATH_BOTH)
		MMP_ULONG 	i2s_irq_src;
		irq_src = (pAFE->AFE_FIFO_CPU_INT_EN & pAFE->AFE_FIFO_CSR);
		i2s_irq_src = (pAUD->I2S_FIFO_CPU_INT_EN & pAUD->I2S_FIFO_SR);
	#endif	
	
	if (irq_src & AUD_INT_FIFO_REACH_UNRD_TH) {
	
    	// Processing currect frame
        if( gbAud1stInToken ) {
            #if (MIC_SOURCE==MIC_IN_PATH_AFE)   
            glAFETicks++; 
            MMPF_Audio_EncodeIntHandler(AFE_PATH);
            #elif (MIC_SOURCE==MIC_IN_PATH_I2S)
            MMPF_Audio_EncodeIntHandler(I2S_PATH);
            #endif
            
            
            #if UPSAMPLE_32K_48K==0
            MMPF_Audio_UpdateWrPtr(gsInAudioSamplesPerFrame*AFE_SAMPLES_TIME);
            #else
            if(gsInAudioSamplesPerFrame!=gsAudioSamplesPerFrame) {
                AudioEncInBufHandle.afe_intr_i++ ;
                if(AudioEncInBufHandle.afe_intr_i==ADV_DUMPY_SAMPLES_TIME) {
                    AudioEncInBufHandle.afe_intr_i = 0 ;  
                    // Advance 4 ms samples at once.
                    MMPF_Audio_UpdateWrPtr(gsAudioSamplesPerFrame*PRC_SAMPLES_TIME);
                }
            } else  {
                MMPF_Audio_UpdateWrPtr(gsInAudioSamplesPerFrame*AFE_SAMPLES_TIME);
            }
            #endif
            
            
            if( MMPF_Audio_GetSizeByType(SIZE_CAN_PR)  >= (gsAudioSamplesPerFrame*PRC_SAMPLES_TIME << 1)) {
                //gsAudioRecEvent |= EVENT_FIFO_OVER_THRESHOLD;
                //MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_AUDIOREC, MMPF_OS_FLAG_SET);             
				MMPF_Audio_UpdatePrPtr(gsAudioSamplesPerFrame*PRC_SAMPLES_TIME );
            }
        } else {
            #if (MIC_SOURCE==MIC_IN_PATH_AFE)    
            MMPF_Audio_DropDataHandler(AFE_PATH);
            #elif (MIC_SOURCE==MIC_IN_PATH_I2S)
            MMPF_Audio_DropDataHandler(I2S_PATH);
            #endif
        }
    }
}
#pragma arm section code, rwdata, zidata
//------------------------------------------------------------------------------
//  Function    : MMPF_InitAudioSetting
//  Parameter   :
//          path--specify audio path
//       samprate--sampleing rate
//  Return Value : AUDIO_NO_ERROR
//  Description : Init codec
//------------------------------------------------------------------------------
MMP_USHORT  MMPF_InitAudioSetting(MMP_USHORT path, MMP_ULONG samprate)
{
    switch (path) {
        case AFE_IN:
            MMPF_InitAitADC(samprate);
            break;
        #if (I2S_OUT_EN == 1)
        case I2S_OUT:
            MMPF_SetI2SFreq(samprate);
            if (gbAudioOutI2SFormat) {
                MMPF_SetI2SMode(I2S_I2S_MODE, LEFT_CHANNEL_LOW, I2S_OUTPUT_24_BITS);
                MMPF_PresetI2S(I2S_OUT,I2S_SLAVE_MODE,8);
            } else {
                MMPF_SetI2SMode(I2S_STD_MODE, LEFT_CHANNEL_LOW, I2S_OUTPUT_16_BITS);
                MMPF_PresetI2S(I2S_OUT,I2S_MASTER_MODE,16);
            }
            break;
        #endif
        case I2S_IN:
            MMPF_SetI2SFreq(samprate);
            if(gbAudioInI2SFormat){//0 -> master , 1 ->slave
                MMPF_SetI2SMode(I2S_I2S_MODE, LEFT_CHANNEL_LOW, I2S_OUTPUT_16_BITS);
                MMPF_PresetI2S(I2S_IN,I2S_SLAVE_MODE,0);
//                MMPF_PresetI2S(I2S_IN,I2S_SLAVE_MODE,1);  // tomy@2010_12_23, to support one bit clock cycle delay for CX20709
            }else{
                MMPF_SetI2SMode(I2S_STD_MODE, LEFT_CHANNEL_LOW, I2S_OUTPUT_16_BITS);
                MMPF_PresetI2S(I2S_IN,I2S_MASTER_MODE,0);
//                MMPF_PresetI2S(I2S_IN,I2S_MASTER_MODE,1);  // tomy@2010_12_23, to support one bit clock cycle delay for CX20709
            }
            break;
    }
    return  AUDIO_NO_ERROR;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_InitializePlayFIFO
//  Parameter   :
//          path-- select path
//          threshold -- fifo int threshold
//  Return Value : None
//  Description : Init Audio Output Fifo
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Audio_InitializePlayFIFO(MMP_USHORT usPath, MMP_USHORT usThreshold)
{
    AITPS_AUD   pAUD    = AITC_BASE_AUD;
    MMP_ULONG    i;

    switch (usPath) {
	case I2S_OUT:
    #if (I2S_OUT_EN == 1)
		pAUD->I2S_FIFO_RST = AUD_FIFO_RST_EN;
		pAUD->I2S_FIFO_RST = 0;

		pAUD->I2S_FIFO_WR_TH = usThreshold;
		for (i = 0; i< 256; i++)
			pAUD->I2S_FIFO_DATA = 0;
    #endif
	break;
    }

	return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetMux
//  Parameter   :
//          path--select path
//          bEnable--1:enable;0:disable
//  Return Value : MMP_ERR_NONE
//  Description : Set audio mux
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetMux(MMP_USHORT path, MMP_BOOL bEnable)
{
    AITPS_AUD   pAUD    = AITC_BASE_AUD;
    #if (CHIP == VSN_V3)
    AITPS_AFE   pAFE    = AITC_BASE_AFE;
    #endif

    switch (path) {
        #if (MIC_SOURCE==MIC_IN_PATH_AFE)  
        case AFE_IN:
			if (bEnable) {
				pAFE->AFE_FIFO_CPU_INT_EN = AFE_INT_FIFO_REACH_UNRD_TH;
				pAFE->AFE_MUX_MODE_CTL = AFE_MUX_AUTO_MODE;

				pAFE->AFE_ADC_CPU_INT_EN = AUD_ADC_INT_EN;
			}
			else {
				pAFE->AFE_FIFO_CPU_INT_EN = 0;
				pAFE->AFE_MUX_MODE_CTL = AFE_MUX_CPU_MODE;

				pAFE->AFE_ADC_CPU_INT_EN &= ~(AUD_ADC_INT_EN);
			}
            break;
        #endif    
        #if (I2S_OUT_EN == 1)
        case I2S_OUT:
            if (bEnable) {
                //Removed by Truman for surpress the pause message
                //RTNA_DBG_Str2("MMPF_Audio_SetMux I2S_out\r\n");
                pAUD->I2S_FIFO_CPU_INT_EN = AUD_INT_FIFO_REACH_UNWR_TH;
                pAUD->I2S_MUX_MODE_CTL = AUD_MUX_AUTO;
                pAUD->I2S_CPU_INT_EN = AUD_INT_EN;
            }
            else {
                pAUD->I2S_FIFO_CPU_INT_EN = 0;
                pAUD->I2S_MUX_MODE_CTL = AUD_MUX_CPU;
                pAUD->I2S_CPU_INT_EN = AUD_INT_DIS;
                //2005-05-23 : for video play "bobo" noise. some melody IC can't accept turn
                // on/off i2s clock too fast.
                //I2S_BASE_L[I2S_GENERAL_CTL_L] = I2S_DISABLE;
            }
            break;
        #endif
        #if (MIC_SOURCE==MIC_IN_PATH_I2S)  
        case I2S_IN:
            if (bEnable) {
                //RTNA_DBG_Str(0,"MMPF_Audio_SetMux I2S_IN\r\n");
                pAUD->I2S_FIFO_CPU_INT_EN = AUD_INT_FIFO_REACH_UNRD_TH | AUD_INT_FIFO_FULL;
                pAUD->I2S_MUX_MODE_CTL = AUD_MUX_AUTO;
                //pAUD->I2S_CPU_INT_EN = AUD_INT_EN;
                pAFE->AFE_MUX_MODE_CTL &= ~AUD_DATA_BIT_20;
            }
            else {
                pAUD->I2S_FIFO_CPU_INT_EN = 0;
                pAUD->I2S_MUX_MODE_CTL = AUD_MUX_CPU;
                pAUD->I2S_CPU_INT_EN = AUD_INT_DIS;
            }
            break;
        #endif    
    }
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_InitializeEncodeFIFO
//  Parameter   :
//          path-- select path
//          threshold -- fifo int threshold
//  Return Value : None
//  Description : Init audio input fifo
//------------------------------------------------------------------------------
MMP_ERR	MMPF_Audio_InitializeEncodeFIFO(MMP_USHORT usPath, MMP_USHORT usThreshold)
{
    AITPS_AUD   pAUD    = AITC_BASE_AUD;
    #if (CHIP == VSN_V3)
    	AITPS_AFE   pAFE    = AITC_BASE_AFE;
    #endif

    switch (usPath) {
    #if (MIC_SOURCE==MIC_IN_PATH_I2S)  
	case I2S_IN:
		pAUD->I2S_FIFO_RST = AUD_FIFO_RST_EN;
		pAUD->I2S_FIFO_RST = 0;
		pAUD->I2S_FIFO_RD_TH = usThreshold;
        break;
    #endif    
    #if (MIC_SOURCE==MIC_IN_PATH_AFE)   
	case AFE_IN:
		pAFE->AFE_FIFO_RST = AUD_FIFO_RST_EN;
		pAFE->AFE_FIFO_RST = 0;
		pAFE->AFE_FIFO_RD_TH = usThreshold;
        break;
    #endif    
    }
	return MMP_ERR_NONE;

}
//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetEncodeLength
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetEncodeLength(MMP_ULONG ulFileLimit)
{
	glAudioEncodeLength = ulFileLimit;

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetDACDigitalGain
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetDACDigitalGain(MMP_UBYTE gain)
{
    return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetDACAnalogGain
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetDACAnalogGain(MMP_UBYTE gain)
{
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetADCDigitalGain
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetADCDigitalGain(MMP_UBYTE gain)
{
#if (MIC_SOURCE==MIC_IN_PATH_AFE)
    AITPS_AFE   pAFE    = AITC_BASE_AFE;
    MMP_UBYTE  orig_gain = pAFE->AFE_ADC_LCH_DIGITAL_VOL;
    gbADCDigitalGain = gain;
    
    #if ADC_FINE_TUNE_POP_NOISE==FIXED_PGA_GAIN
    if(orig_gain > gain) {
        pAFE->AFE_ADC_DIGITAL_GAIN_MUTE_STEP = 0x02 | 0x01 ;    
    }
    else {
        pAFE->AFE_ADC_DIGITAL_GAIN_MUTE_STEP = 0x04 | 0x01 ;
    }
    #endif

    #if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN
    pAFE->AFE_ADC_DIGITAL_GAIN_MUTE_STEP = 0x04 | 0x01 ;
    #endif
    
    pAFE->AFE_ADC_LCH_DIGITAL_VOL = gbADCDigitalGain;
    pAFE->AFE_ADC_RCH_DIGITAL_VOL = gbADCDigitalGain;
    
    #if ADC_FINE_TUNE_POP_NOISE > 0
    pAFE->AFE_ADC_DIGITAL_GAIN_MUTE_STEP &= ~0x01 ;
    #endif
    
#endif
    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetADCAnalogGain
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetADCAnalogGain(MMP_UBYTE gain, MMP_UBYTE boostdb)
{
    #if (MIC_SOURCE==MIC_IN_PATH_AFE)
    AITPS_AFE   pAFE    = AITC_BASE_AFE;

    gbADCAnalogGain =  gain;

    pAFE->AFE_ADC_LCH_PGA_GAIN_CTL = gbADCAnalogGain & 0x1F;
    pAFE->AFE_ADC_RCH_PGA_GAIN_CTL = gbADCAnalogGain & 0x1F;

    gbADCBoost =  boostdb;
    	
    if (gbADCBoost==0)
    {
        pAFE->AFE_ADC_BOOST_CTL = MIC_NO_BOOST ;
    }
    else if (gbADCBoost==20)
    {
        pAFE->AFE_ADC_BOOST_CTL = MIC_LCH_BOOST(MIC_BOOST_20DB) | MIC_RCH_BOOST(MIC_BOOST_20DB);
    }
    else if (gbADCBoost==30)
    {
        pAFE->AFE_ADC_BOOST_CTL = MIC_LCH_BOOST(MIC_BOOST_30DB) | MIC_RCH_BOOST(MIC_BOOST_30DB);
    } 
    else {
        pAFE->AFE_ADC_BOOST_CTL = MIC_LCH_BOOST(MIC_BOOST_40DB) | MIC_RCH_BOOST(MIC_BOOST_40DB);
    }
    
    //dbg_printf(3,"Boost:%x,Again : %x\r\n",pAFE->AFE_ADC_BOOST_CTL,pAFE->AFE_ADC_LCH_PGA_GAIN_CTL);
    
    #endif
    return  MMP_ERR_NONE;
}

MMP_BYTE MMPF_Audio_GetADCAnalogGain(void)
{
    AITPS_AFE   pAFE = AITC_BASE_AFE;
    MMP_BYTE    gain = AFE_ADC2PGA(pAFE->AFE_ADC_LCH_PGA_GAIN_CTL) ;
    MMP_BYTE    boost ,boost_gain = 0;
    boost = (pAFE->AFE_ADC_BOOST_CTL & 0x03) ;
    if(boost==MIC_NO_BOOST) {
        boost_gain = 0;
    }
    else if (boost==MIC_BOOST_20DB) {
        boost_gain = 20 ;
    }
    else if(boost==MIC_BOOST_30DB) {
        boost_gain = 30 ;
    }
    else if(boost==MIC_BOOST_40DB) {
        boost_gain = 40 ;
    }
    dbg_printf(3,"cur boost :%d ,pga : %d db\r\n",boost_gain,gain);
    return (MMP_BYTE)(boost_gain + gain) ;
}

/** @} */ // end of MMPF_AUDIO
//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetEncodeFormat
//  Parameter   :
//              mode :the audio record mode
//  Return Value : None
//  Description : Set the audio record mode
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetEncodeFormat(MMP_USHORT mode)
{
    gsAudioEncodeFormat = mode;
    //RTNA_DBG_PrintLong(0, gsAudioEncodeFormat);

    return  MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_SetVoiceInPath
//  Parameter   : None
//  Return Value : None
//  Description : Set Voice In Path
//------------------------------------------------------------------------------
MMP_USHORT  MMPF_SetVoiceInPath(MMP_UBYTE path)
{
#if (MIC_SOURCE==MIC_IN_PATH_AFE)
	#if (CHIP == P_V2)
	AITPS_AUD   pAUD    = AITC_BASE_AUD;
	#elif (CHIP == VSN_V3)
	AITPS_AFE   pAUD    = AITC_BASE_AFE;
	#endif
	gbUseAitADC = path;

    #if (CHIP == P_V2)
	pAUD->AFE_ADC_INPUT_SEL &= ~(ADC_CTL_MASK);

	if (gbUseAitADC == AUDIO_IN_AFE_SING) {
        pAUD->AFE_ADC_INPUT_SEL |= ADC_SINGLE_IN;
    }
    if (gbUseAitADC == AUDIO_IN_AFE_DIFF) {
        pAUD->AFE_ADC_INPUT_SEL |= ADC_DIFF_IN;
    }
    if (gbUseAitADC == AUDIO_IN_AFE_DIFF2SING) {
        pAUD->AFE_ADC_INPUT_SEL |= ADC_DIFF2SINGLE_EN;
    }
	#elif (CHIP == VSN_V3)
	pAUD->AFE_ADC_INPUT_SEL &= ~(ADC_CTL_MASK);

	if (gbUseAitADC == AUDIO_IN_AFE_SING) {
        pAUD->AFE_ADC_INPUT_SEL |= ADC_AUX_IN;
    }
    if (gbUseAitADC == AUDIO_IN_AFE_DIFF) {
        pAUD->AFE_ADC_INPUT_SEL |= ADC_MIC_DIFF;
    }
    if (gbUseAitADC == AUDIO_IN_AFE_DIFF2SING) {
        pAUD->AFE_ADC_INPUT_SEL |= ADC_MIC_DIFF2SINGLE;
    }
	#endif
#endif	
    return  AUDIO_NO_ERROR;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetLineInChannel
//  Parameter   : None
//  Return Value : None
//  Description : Set Voice In Path
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetLineInChannel(MMPF_AUDIO_LINEIN_CHANNEL lineinchannel)
{
	m_audioLineInChannel = lineinchannel;

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_SetAudioOutPath
//  Parameter   : None
//  Return Value : None
//  Description : Set Voice In Path
//------------------------------------------------------------------------------
MMP_USHORT  MMPF_SetAudioOutPath(MMP_UBYTE path)
{
    return  AUDIO_NO_ERROR;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetI2SOutFormat
//  Parameter   : None
//  Return Value : None
//  Description : Set audio out i2s format
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetI2SOutFormat(MMP_UBYTE ubFormat)
{
    gbAudioOutI2SFormat = ubFormat;

    return  MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetI2SInFormat
//  Parameter   : None
//  Return Value : None
//  Description : Set audio in i2s format
//------------------------------------------------------------------------------
MMP_ERR  MMPF_Audio_SetI2SInFormat(MMP_UBYTE ubFormat)
{
    gbAudioInI2SFormat = ubFormat;

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_InitAitADC
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
#if ADC_PERFORMANCE_TEST

typedef struct _REG_VAL_CFG
{
    MMP_USHORT reg ;
    MMP_UBYTE  val ;
} REG_VAL_CFG ;

void MMPF_Audio_TestPerformance(void)
{
    volatile MMP_UBYTE* REG_BASE_B = (volatile MMP_UBYTE*)0x80000000;
    REG_VAL_CFG reg_val_cfg[] =
    {
        {0x7834 ,0x04 },//(32 bck)
        {0x7838 ,0x00 },//
        {0x7844 ,0x10 },//(left-justify)
        {0x7848 ,0x00 },//(slave mode)
        {0x7854 ,0x01 },//(I2S SDO data out enable)
        {0x7860 ,0x01 },//
        {0x7f54 ,0x02 },//(20 bit mode)
        {0x7878 ,0x08 },//(ADC to I2S enable)
        {0x7879 ,0x02 },//(Using I2S_MCLK as AFE clock)
        {0x7828 ,0x08 },//(I2S enable)
    } ; 
    MMP_USHORT i,items;
    items = sizeof(reg_val_cfg) / sizeof(REG_VAL_CFG) ;
    for(i=0;i<items;i++) {
        REG_BASE_B[reg_val_cfg[i].reg] = reg_val_cfg[i].val ;
        dbg_printf(3,"Reg : %x = %x \r\n",reg_val_cfg[i].reg,reg_val_cfg[i].val ); 
    }
}

#endif

MMP_USHORT  MMPF_InitAitADC(MMP_ULONG samplerate)
{
#if (MIC_SOURCE==MIC_IN_PATH_AFE)
    #if (CHIP == VSN_V3)
    AITPS_AFE   pAFE    = AITC_BASE_AFE;
    //RTNA_DBG_Str(0, "MMPF_InitAitADC\r\n");

    pAFE->FIX_AFE_ADC_OVERFLOW = 0x03; //wilson@120711: enable fix overflow
    
    pAFE->AFE_ADC_LCH_DIGITAL_VOL = gbADCDigitalGain ;
    pAFE->AFE_ADC_RCH_DIGITAL_VOL = gbADCDigitalGain ;
    pAFE->AFE_ADC_HPF_CTL =  ADC_HPF_EN; //disable hpf
    pAFE->AFE_ADC_LCH_PGA_GAIN_CTL = gbADCAnalogGain & 0x1F;
    pAFE->AFE_ADC_RCH_PGA_GAIN_CTL = gbADCAnalogGain & 0x1F;

    pAFE->AFE_GBL_BIAS_ADJ = GBL_BIAS_100;
    //pAFE->AFE_ADC_BIAS_ADJ = ANA_ADC_CONT_OP(0x10)|ANA_ADC_DISC_OP(0x10);
    pAFE->AFE_ADC_BIAS_ADJ = ANA_ADC_CONT_OP(0x01)|ANA_ADC_DISC_OP(0x01);
    
    pAFE->AFE_ADC_CTL_REG1 = AFE_ZERO_CROSS_DET;

    if (gbADCBoost==0)
    {
    	pAFE->AFE_ADC_BOOST_CTL = MIC_NO_BOOST ;
    }
    else if (gbADCBoost==20)
    {
    	pAFE->AFE_ADC_BOOST_CTL = MIC_LCH_BOOST(MIC_BOOST_20DB) | MIC_RCH_BOOST(MIC_BOOST_20DB);
    }
    else if (gbADCBoost==30)
    {
    	pAFE->AFE_ADC_BOOST_CTL = MIC_LCH_BOOST(MIC_BOOST_30DB) | MIC_RCH_BOOST(MIC_BOOST_30DB);
    } 
    else {
    	pAFE->AFE_ADC_BOOST_CTL = MIC_LCH_BOOST(MIC_BOOST_40DB) | MIC_RCH_BOOST(MIC_BOOST_40DB);
    }
#if 0 //Sync RD SVN
    pAFE->AFE_GBL_PWR_CTL |= PWR_UP_ANALOG;
    RTNA_WAIT_MS(ANA_PDN_DLY_MS); //1.03ms <= 50ms
    pAFE->AFE_GBL_PWR_CTL |= PWR_UP_VREF;
    pAFE->AFE_ADC_PWR_CTL |= ADC_PGA_RCH_POWER_EN | ADC_PGA_LCH_POWER_EN;
    RTNA_WAIT_MS(ADC_PGA_PDN_DLY_MS); //102.90ms <= 100ms
    pAFE->AFE_ADC_PWR_CTL |= ADC_SDM_RCH_POWER_EN | ADC_SDM_LCH_POWER_EN;
    pAFE->AFE_GBL_PWR_CTL |= PWR_UP_ADC_DIGITAL_FILTER;
#else
	pAFE->AFE_GBL_PWR_CTL |= PWR_UP_ANALOG;		
	//RTNA_WAIT_MS(ANA_PDN_DLY_MS); //1.03ms <= 50ms
	RTNA_WAIT_MS(20);
	pAFE->AFE_GBL_PWR_CTL |= PWR_UP_VREF;
	RTNA_WAIT_US(1);
	pAFE->AFE_ADC_PWR_CTL |= ADC_PGA_RCH_POWER_EN | ADC_PGA_LCH_POWER_EN;
	//RTNA_WAIT_MS(ADC_PGA_PDN_DLY_MS); //102.90ms <= 100ms
	pAFE->AFE_ADC_PWR_CTL |= ADC_SDM_RCH_POWER_EN | ADC_SDM_LCH_POWER_EN;
	pAFE->AFE_GBL_PWR_CTL |= PWR_UP_ADC_DIGITAL_FILTER;
	
	#if ADC_FINE_TUNE_POP_NOISE != DC_OFFSET_OFF 
	pAFE->AFE_GBL_PWR_CTL  |= BYPASS_OP ; 
	pAFE->AFE_ADC_CTL_REG3 &= ~0x01 ;
	pAFE->AFE_ADC_DIGITAL_GAIN_MUTE_STEP = 0x02  ; // digital zero crossing
	#else
	pAFE->AFE_GBL_PWR_CTL  |= BYPASS_OP ; 
	pAFE->AFE_ADC_CTL_REG3 |= 0x01 ;
	#endif
	
#endif

    pAFE->AFE_ADC_CTL_REG4 = ADC_MIC_BIAS_ON | ADC_MIC_BIAS_VOLTAGE075AVDD ;//Gason's patch

    if (gbUseAitADC == AUDIO_IN_AFE_SING)
    {
    	pAFE->AFE_ADC_INPUT_SEL = (pAFE->AFE_ADC_INPUT_SEL & ADC_CTL_MASK) | ADC_MIC_IN | ADC_MIC_DIFF2SINGLE;
    	RTNA_DBG_Str3("AFE SING\r\n")
    }
    else if (gbUseAitADC == AUDIO_IN_AFE_DIFF) {
    	pAFE->AFE_ADC_INPUT_SEL = (pAFE->AFE_ADC_INPUT_SEL & ADC_CTL_MASK) | ADC_MIC_IN | ADC_MIC_DIFF;
    	RTNA_DBG_Str3("AFE DIFF\r\n")
    }
    else if (gbUseAitADC == AUDIO_IN_AFE_DIFF2SING) {
    	pAFE->AFE_ADC_INPUT_SEL = (pAFE->AFE_ADC_INPUT_SEL & ADC_CTL_MASK) | ADC_MIC_IN | ADC_MIC_DIFF2SINGLE;
    	RTNA_DBG_Str3("AFE DIFF2SING\r\n")
    }

    pAFE->AFE_CLK_CTL = ADC_CLK_MODE_USB;
    pAFE->AFE_CLK_CTL |= ( /*ADC_CLK_INVERT |*/ AUD_CODEC_NORMAL_MODE); // disable invert clock for noise

    MMPF_SetAFEFreq(samplerate);  
    dbg_printf(3,"initadc : BAD(%x,%x,%x)\r\n",pAFE->AFE_ADC_BOOST_CTL,pAFE->AFE_ADC_LCH_PGA_GAIN_CTL,pAFE->AFE_ADC_LCH_DIGITAL_VOL);
     
    #endif
#endif
   
    return  AUDIO_NO_ERROR;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_InitAitCodec
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT  MMPF_InitAitCodec(MMP_ULONG samplerate)
{
    #if(CHIP == VSN_V3)
    AITPS_AFE   pAFE    = AITC_BASE_AFE;

    RTNA_DBG_Str(0, "MMPF_InitAitCodec()\r\n");
    pAFE->AFE_GBL_PWR_CTL |= (PWR_UP_ANALOG | PWR_UP_VREF | PWR_UP_ADC_DIGITAL_FILTER);
    pAFE->AFE_ADC_PWR_CTL |= (ADC_SDM_LCH_POWER_EN | ADC_SDM_RCH_POWER_EN | ADC_PGA_LCH_POWER_EN | ADC_PGA_RCH_POWER_EN);

    pAFE->AFE_ADC_INPUT_SEL = (pAFE->AFE_ADC_INPUT_SEL & ADC_CTL_MASK) | ADC_MIC_IN | ADC_MIC_DIFF2SINGLE;
    pAFE->AFE_ADC_LCH_DIGITAL_VOL = gbADCDigitalGain;
    pAFE->AFE_ADC_RCH_DIGITAL_VOL = gbADCDigitalGain;
    pAFE->AFE_ADC_LCH_PGA_GAIN_CTL = gbADCAnalogGain & 0x0F;
    pAFE->AFE_ADC_RCH_PGA_GAIN_CTL = gbADCAnalogGain & 0x0F;

    MMPF_SetAFEFreq(samplerate);
    #endif

    return  AUDIO_NO_ERROR;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Audio_SetPLL
//  Parameter   : 
//      ulSamplerate -- sampling rate
//  Return Value : None
//  Description : Dynamic change PLL for audio DAC.
//------------------------------------------------------------------------------
MMP_ERR MMPF_Audio_SetPLL(MMP_ULONG ulSamplerate)
{
    #if (MIC_SOURCE==MIC_IN_PATH_AFE)
    MMP_ERR         err;
    MMPF_PLL_MODE   pll_mode;
    if (glAudioSamplerate != ulSamplerate) {
        switch(ulSamplerate) {
        case 48000:
            pll_mode = MMPF_PLL_AUDIO_48K;
            break;
        case 44100:
            pll_mode = MMPF_PLL_AUDIO_44d1K;
            break;
        case 32000:
            pll_mode = MMPF_PLL_AUDIO_32K;
            break;
        case 22050:
            pll_mode = MMPF_PLL_AUDIO_22d05K;
            break;
        case 16000:
            pll_mode = MMPF_PLL_AUDIO_16K;
            break;
        case 12000:
            pll_mode = MMPF_PLL_AUDIO_12K;
            break;
        case 11025:
            pll_mode = MMPF_PLL_AUDIO_11d025K;
            break;
        case 8000:
            pll_mode = MMPF_PLL_AUDIO_8K;
            break;
        default:
            RTNA_DBG_Str0("Unsupported audio sample rate!\r\n");
            return MMP_AUDIO_ERR_PARAMETER;
            break;
        }
        err = MMPF_PLL_Setting(pll_mode, MMP_TRUE);
        if (err != MMP_ERR_NONE)
            return err;
        glAudioSamplerate = ulSamplerate;
    }
    #endif
    return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_SetAFEFreq
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------


MMP_USHORT  MMPF_SetAFEFreq(MMP_ULONG samplerate)
{
#if (MIC_SOURCE==MIC_IN_PATH_AFE)
    AITPS_AFE   pAFE    = AITC_BASE_AFE;
    switch(samplerate) {
        case 48000:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_48000HZ;
        break;	
        case 44100:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_44100HZ;
        break;	
        case 32000:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_32000HZ;
        break;	
        case 24000:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_24000HZ;
        break;	
        case 22050:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_22050HZ;
        break;	
        case 16000:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_16000HZ;
        break;	
        case 12000:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_12000HZ;
        break;	
        case 11025:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_11025HZ;
        break;
        case 8000:
        pAFE->AFE_ADC_SAMPLE_RATE_CTL = ADC_SRATE_MASK & SRATE_8000HZ;
        break;	
    }
#endif
    return AUDIO_NO_ERROR;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_CloseDac
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------		
MMP_USHORT MMPF_CloseDAC(void)
{
    return  AUDIO_NO_ERROR;
}

MMP_USHORT MMPF_CloseADC(void)
{
#if (MIC_SOURCE==MIC_IN_PATH_AFE)

    AITPS_AFE   pAFE    = AITC_BASE_AFE;

	//RTNA_DBG_Str(0, "MMPF_CloseADC\r\n");
    gbAitADCEn = MMP_FALSE;
    // turn off mic bias
    //pAFE->AFE_ADC_CTL_REG4 &= ~ADC_MIC_BIAS_ON ;
    pAFE->AFE_ADC_PWR_CTL &= ~(ADC_PGA_LCH_POWER_EN|ADC_PGA_RCH_POWER_EN|
    					ADC_SDM_LCH_POWER_EN|ADC_SDM_RCH_POWER_EN|PWR_UP_ADC_DIGITAL_FILTER);

#endif
    
    return  AUDIO_NO_ERROR;
}

MMP_USHORT MMPF_Audio_EnableAFEClock(MMP_BOOL bEnableClk, MMP_ULONG SampleRate)
{
#if (MIC_SOURCE==MIC_IN_PATH_AFE)
    AITPS_GBL pGBL = AITC_BASE_GBL;

    if (bEnableClk == MMP_TRUE) {
        pGBL->GBL_CLK_DIS2 &= (~ GBL_CLK_AUD_CODEC_DIS);	// (bit0) enable codec clock
        //under USB mode, g0 = 96M
        if (SampleRate > 24000)
        pGBL->GBL_ADC_CLK_DIV = 0x0773;
        else if (SampleRate > 12000)
        pGBL->GBL_ADC_CLK_DIV = 0x0FF7;
        else
        RTNA_DBG_Str(0, "WARNING: Need to lower audio source clock");
        //pGBL->GBL_ADC_CLK_DIV = 0x0FF7;
        //pGBL->GBL_ADC_CLK_DIV = 0x0333;						// audio codec clock divide by 4, ADC DF clock divide by 4, both get 96/4 = 24Mhz
        												// audio ADC HBF clock divide by 4, and get 96/4 = 24Mhz
    }
    else {
        pGBL->GBL_CLK_DIS2 |= GBL_CLK_AUD_CODEC_DIS;	// (bit0) enable codec clock
    }
#endif
	
	return AUDIO_NO_ERROR;
}

MMP_UBYTE MMPF_Audio_AddSamplingGain(MMP_ULONG sr)
{
    MMP_UBYTE gain = 0;
    switch (sr) {
    case  8000:
    case 16000:
    case 32000:
        gain = (MMP_UBYTE)(72 - 63 );
        break;
    case 12000:
    case 24000:
    case 48000:
        gain = (MMP_UBYTE)(68 - 63 );
        break ;    
    }
    
    return gain;
}
#else
void    MMPF_AFE_ISR(void) {}
#endif

/** @} */ // end of MMPF_AUDIO
