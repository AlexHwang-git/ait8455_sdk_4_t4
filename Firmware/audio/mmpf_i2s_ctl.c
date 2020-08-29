#include    "includes_fw.h"
#include    "lib_retina.h"	
#include	"mmpf_audio_ctl.h"
#include	"mmpf_i2s_ctl.h"
#include    "config_fw.h"
#include    "mmp_reg_audio.h"
#if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
#include "mmpf_pll.h"
#endif
#if SUPPORT_UAC==1
//------------------------------------------------------------------------------
//  Function    : MMPF_SetI2SFreq
//  Parameter   :
//          freq--sampling rate
//  Return Value : None
//  Description : Set i2s mclk
//------------------------------------------------------------------------------

/** @addtogroup MMPF_I2S
@{
*/

void MMPF_SetI2SFreq(MMP_ULONG freq)
{
    AITPS_AUD   pAUD    = AITC_BASE_AUD; 
	MMP_USHORT /*mn_value,*/ ratioM, ratioN;
	MMP_ULONG audClk;
	MMP_UBYTE clkDiv/*, cuer_div*/;
    
    RTNA_DBG_Str(0, "SETTING I2S MCLK................................\r\n");

    {
    	extern MMP_ERR MMPF_PLL_GetGroupFreq(MMP_UBYTE ubGroupNum, MMP_ULONG *ulGroupFreq);
    	MMPF_PLL_GetGroupFreq(0, &audClk);
    }

    /* ********************************************************************* */
    // Wilson@120626 NOTE:
    // The MN value calculation is for general case,
    // you may need to modify MN value to meet the requirement for different CODEC
    // For example:
    // For DA7211, 16k, 32k, 48k sample rate, they are belong to same MCLK group;
    // the freq variable should set to 48k for these sample rates.
    /* ********************************************************************* */
    #if (AUDEXT_DAC == DA7211)
        freq = 48000;
    #endif
    #if (AUDEXT_DAC == WM8973)
    	freq = 0xB71B; //12M/256
    #endif
    #if (AUDEXT_DAC == WM8737)
    	freq = 48000;
    #endif
    

    audClk /= 2;
    RTNA_DBG_PrintLong(0, audClk);
    clkDiv = (MMP_UBYTE)((audClk * 1000)/(256 * freq));
    ratioM = (MMP_UBYTE)((audClk * 1000)/freq - clkDiv * 256);
    ratioN = 256 - ratioM;

    pAUD->I2S_MCLK_FIXMODE = 1;
    pAUD->I2S_CLK_DIV = clkDiv;
    pAUD->I2S_RATIO_N_M = ratioM << 8 | ratioN;

    RTNA_DBG_PrintByte(0, pAUD->I2S_CLK_DIV);
    RTNA_DBG_PrintLong(0, pAUD->I2S_RATIO_N_M);
    
  
	return;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_SetI2SMode
//  Parameter   :
//          i2s_mode--
//          lrck_mode--
//          output_bits--
//  Return Value : None
//  Description : Set i2s interface format
//------------------------------------------------------------------------------
void	MMPF_SetI2SMode(MMP_USHORT i2s_mode, MMP_USHORT lrck_mode, MMP_USHORT output_bits)
{
    AITPS_AUD   pAUD    = AITC_BASE_AUD;

    #if (I2S_OUT_EN == 1) //wilson: for VSN_V3
    pAUD->I2S_OUT_MODE_CTL = i2s_mode;
    #endif

    pAUD->I2S_LRCK_POL = lrck_mode;
    pAUD->I2S_BIT_CLT = output_bits;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_PresetI2S
//  Parameter   :
//          direction--specify i2s in or out
//          mode--specify master or slave mode
//  Return Value : None
//  Description : Set i2s data direction
//------------------------------------------------------------------------------
void	MMPF_PresetI2S(MMP_USHORT direction,MMP_USHORT mode,MMP_USHORT alignment)
{
    AITPS_AUD   pAUD    = AITC_BASE_AUD;
	#if defined(I2S_SWITCH_1DOT8_MODE) 					
    AITPS_GBL   pGBL = AITC_BASE_GBL;
#endif    

	pAUD->I2S_MCLK_CTL = I2S_256_FS;

	pAUD->I2S_MODE_CTL = (mode | I2S_MCLK_OUT_EN);

#if defined(I2S_SWITCH_1DOT8_MODE) 					
    pGBL->GBL_HOST_CTL |=(0x80); 
#endif

    switch (direction) {
    
    #if (I2S_OUT_EN == 1) //wilson: for VSN_V3
    case I2S_OUT:
        pAUD->I2S_BIT_ALIGN_OUT = (pAUD->I2S_BIT_ALIGN_OUT &0xFF00) | alignment;
        pAUD->I2S_DATA_OUT_EN = 1;
        pAUD->I2S_CTL = (I2S_SDO_OUT_EN | I2S_LRCK_OUT_EN | I2S_BCK_OUT_EN | I2S_HCK_CLK_EN);
        break;
	#endif
    
    case I2S_IN:
    #if PCAM_EN==0
        pAUD->I2S_BIT_ALIGN_IN = (pAUD->I2S_BIT_ALIGN_OUT &0xFF00) | alignment;
        pAUD->I2S_DATA_IN_SEL = I2S_SDI_IN;
        
        #if (I2S_OUT_EN == 1) //wilson: for VSN_V3
        pAUD->I2S_DATA_OUT_EN = 0;
		#endif
		
        pAUD->I2S_CTL = (I2S_SDO_OUT_EN | I2S_LRCK_OUT_EN | I2S_BCK_OUT_EN | I2S_HCK_CLK_EN);//enable sdo,lrck,bck,hck
    #else
    	RTNA_DBG_Str(0, "set i2s alignment, sdi in\r\n"); 
        pAUD->I2S_BIT_ALIGN_IN = alignment;
        pAUD->I2S_DATA_IN_SEL = I2S_SDI_IN;				
        
        #if (I2S_OUT_EN == 1) //wilson: for VSN_V3
        pAUD->I2S_DATA_OUT_EN = 0;
		#endif
		
        pAUD->I2S_CTL = (I2S_HCK_CLK_EN);//enable hck
    #endif
        break;
    }
}
//------------------------------------------------------------------------------
//  Function    : MMPF_CloseI2S
//  Parameter   : None
//  Return Value : None
//  Description : Disable i2s
//------------------------------------------------------------------------------
void	MMPF_CloseI2S(void)
{
	AITPS_AUD   pAUD    = AITC_BASE_AUD;
	
	pAUD->I2S_CTL = I2S_ALL_DIS;
}
/** @} */ // end of MMPF_I2S
#endif

