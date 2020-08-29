#include "config_fw.h"
#include "mmpf_typedef.h"
#include "mmpf_pll.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_gpio.h"

#include "lib_retina.h"

void USB_EnableTaskRunning(MMP_UBYTE en) ;
void MMPF_SaveDefClock(MMP_ULONG clk0,MMP_ULONG clk1) ;
void MMPF_RestoreDefClock(void) ;

static MMP_ULONG glDefSysClock0,glDefSysClock1 ;

void MMPF_SaveDefClock(MMP_ULONG clk0,MMP_ULONG clk1) 
{
    glDefSysClock0 = clk0 ;
    glDefSysClock1 = clk1 ;
}

void MMPF_RestoreDefClock(void)
{
#if 0 // JTAG_DBG_EN==0 //DCDCDCDC
extern MMP_UBYTE glAudioEnable ;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    MMP_ULONG clk0 = pGBL->GBL_CLK_EN ;
    MMP_ULONG clk1 = pGBL->GBL_CLK_EN_2 ;
    clk0 = glDefSysClock0 | ( ~clk0) ;
    clk1 = glDefSysClock1 | ( ~clk1) ;
    clk0 = ~clk0 ;
    clk1 = ~clk1 ;
    // sean@2011_05_05, don't turn off Audio Clk if audio is running
    if(glAudioEnable) {
        clk0 &= ~GBL_CLK_AUD ; 
        clk1 &= ~GBL_CLK_AUDIO_CODAC ;  
    }
    
    pGBL->GBL_CLK_DIS = clk0 ;
    pGBL->GBL_CLK_DIS_2 = clk1 ;
    
  //  dbg_printf(3,"#Clk.Def[0,1] = [%x,%x] \r\n",glDefSysClock0,glDefSysClock1);    
  //  dbg_printf(3,"#Clk.En [0,1]=[%x,%x]\r\n",pGBL->GBL_CLK_EN,pGBL->GBL_CLK_EN_2);
  //  dbg_printf(3,"#Clk.Dis[0,1]=[%x,%x]\r\n",clk0,clk1);
#endif
    
}



#pragma O0
extern MMP_UBYTE gbUSBSuspendFlag ;
#pragma

volatile AITPS_TC    pTC0_ITCM = AITC_BASE_TC0;

//
// Enable\Disable OS running by set Timer intr.
//
void USB_EnableTaskRunning(MMP_UBYTE en)
{
    if(en) {
        RTNA_TC_IRQ_En(pTC0_ITCM, TC_CPCS);
    } else {
        RTNA_TC_IRQ_Dis(pTC0_ITCM, TC_CPCS);
    }
}

