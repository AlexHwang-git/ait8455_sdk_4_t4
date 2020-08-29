/*
   AAC encoder API 
 */ 
#include "includes_fw.h"
#include "config_fw.h"
#if (PCAM_EN==1)&&(SUPPORT_UAC==1)
#include "lib_retina.h"
#include "mmpf_audio_ctl.h"
#include "mmpf_encapi.h"
#include "mmpf_audio.h"
#include "mmpf_audio_typedef.h"
#include "mmpf_usbvend.h"
#include "pcam_msg.h"
#include "mmp_reg_audio.h"
#include "mmp_reg_usb.h"

#define READYFRAME_HIGHBOUND_TH  12//(ENC_INBUF_FRAME_NUM - 4)
#define READYFRAME_LOWBOUND_TH   4 //2//(ENC_INBUF_FRAME_NUM - 1)

void MMPF_Audio_3rdPartyInit(void);
void MMPF_Audio_3rdPartyExit(void);
void MMPF_Audio_3rdPartyEncode(MMP_SHORT *outPtr,MMP_SHORT *inPtr,MMP_SHORT samples);

extern AUDIO_ENC_INBUF_HANDLE AudioEncInBufHandle;
extern MMP_USHORT   gsAudioRecEvent;
#pragma arm section  zidata = "audbuffer"
MMP_SHORT   gsAACEncInBuf[ENC_INBUF_SIZE];
#pragma arm section  zidata

MMP_SHORT   gsAudioSamplesPerFrame,gsInAudioSamplesPerFrame ;
MMP_USHORT  gsAudioSampleRate ;   
//----------------------------------------
MMP_ULONG gsAudTicks = 0 ;
MMP_UBYTE gbAud1stInToken = 0  ;
MMP_BOOL gbAudioTaskPrioStatus = 0;

MMP_USHORT ENC_CUR_SAMPLES,ENC_CUR_INBUF_SIZE ;

#if AUDIO_RESYNC_EN==1
AudioResyncData_t AudioResyncData ;
#endif

#if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN

#define ADC_ADJ_STEP    1 // 1db 
MMP_BYTE    gbTargetADCGain ;
MMP_USHORT  gsAdjustTimeTicks,gsAdjustTime ; 
void MMPF_Audio_SetTargetADCGain(MMP_BYTE adc_gain,MMP_USHORT ticks)
{
    MMP_ULONG cpu_sr ;
    IRQ_LOCK(
        gbTargetADCGain = AFE_ADC2PGA(adc_gain);
        gsAdjustTimeTicks= ticks ;
        gsAdjustTime = 0;
    )
    //dbg_printf(3,"TargetGain:%d\r\n",gbTargetADCGain);
}

void MMPF_Audio_AdjustADCGain(MMP_USHORT step)
{

    AITPS_AFE   pAFE    = AITC_BASE_AFE;
    MMP_UBYTE adc_gain,boost;
    gsAdjustTime++ ;

    if((gsAdjustTime==gsAdjustTimeTicks)&&(gsAdjustTimeTicks)) {
        MMP_BYTE cur_gain = (MMP_BYTE)(MMPF_Audio_GetADCAnalogGain() - AFE_FIX_BOOST_GAIN);
        if(cur_gain < gbTargetADCGain) {
            if(step) {
                cur_gain += step ;
            }
            else {
                cur_gain = gbTargetADCGain ;
            } 
        }
        else if(cur_gain > gbTargetADCGain) {
            if(step) {
                cur_gain -= step ;
            }
            else {
                cur_gain = gbTargetADCGain ;
            }
        }
        else {
            gsAdjustTime = 0;
            gsAdjustTimeTicks = 0;
            return ;
        }
        adc_gain = AFE_PGA2ADC(cur_gain);
        MMPF_Audio_SetADCAnalogGain(adc_gain,AFE_FIX_BOOST_GAIN);
        gsAdjustTime = 0;
        
    }
}
#endif

MMP_USHORT MMPF_Audio_GetRealSampleRate(MMP_USHORT in_sr)
{
#if UPSAMPLE_32K_48K==1
    if(in_sr==48000) {
        return 32000 ;
    }
#endif
    return in_sr ;
}

void MMPF_Audio_TimerOpen(MMP_ULONG us,MMPF_TIMER_ID id)
{
    dbg_printf(3,"#open aud timer\r\n");
    MMPF_Timer_OpenUs(id, us, MMPF_Audio_TimerISR); 
    MMPF_Timer_OpenInterrupt(id);
    MMPF_Timer_EnableInterrupt(id, MMP_TRUE);
}

void MMPF_Audio_TimerClose(MMPF_TIMER_ID id)
{
    MMPF_Timer_EnableInterrupt(id, MMP_FALSE);
    MMPF_Timer_Close(id);
    
}


#if CODE_OPT_VER >0
#pragma arm section code = "aud_timer", rwdata = "aud_timer",  zidata = "aud_timer"
#endif
extern MMP_ULONG glEncTicks,glAFETicks,glPrcTicks,glTimerTicks;
static int gbLogFirstSync = 0 ;    
static int glSkipTimerTicks ;
void MMPF_Audio_TimerISR(void)
{
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_AFE   pAFE = AITC_BASE_AFE;
    MMP_USHORT err ,csr;
    MMP_SHORT *ptr ; 
    int i ;
    //dbg_printf(3,"AudTx\r\n");
    gsAudTicks++;
    csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR);
    if(gbAud1stInToken==0) {
        if(csr &TX_UNDERRUN_BIT) {
            // fill zero to 1st 8 frames
            ptr = (MMP_SHORT *)MMPF_Audio_GetWrPtr();
            for(i=0;i<gsAudioSamplesPerFrame*PRC_SAMPLES_TIME*2;i++) {
                ptr[i] = 0 ;
            }
            for(i=0;i<PRC_SAMPLES_TIME*2;i++) {
                MMPF_Audio_UpdateWrPtr(gsAudioSamplesPerFrame);
            }    
            MMPF_Audio_EncodeFrame();
            MMPF_Audio_EncodeFrame();
            dbg_printf(3,"1stIn@Ticks:%d,:Free(%d),Ready(%d),Pr(%d)\r\n",gsAudTicks,MMPF_Audio_GetSizeByType(SIZE_CAN_WR),MMPF_Audio_GetSizeByType(SIZE_CAN_RD),MMPF_Audio_GetSizeByType(SIZE_CAN_PR));
           // dbg_printf(3,"istIn@TXCSR :%x\r\n",csr);
            MMPF_Audio_TimerClose(MMPF_TIMER_2);
            MMPF_Audio_TimerOpen(500,MMPF_TIMER_2);
            gbAud1stInToken = 1;
            gsAudTicks = 0;
#if AUDIO_RESYNC_EN
            MMPF_Audio_Resync_Init(PRC_SAMPLES_TIME);
            pAFE->AFE_FIFO_RST = 1; // flush fifo
#endif
#if (MIC_SOURCE == MIC_IN_PATH_AFE)
            RTNA_AIC_IRQ_En(pAIC, AIC_SRC_AFE_FIFO);
#elif (MIC_SOURCE == MIC_IN_PATH_I2S)
            RTNA_AIC_IRQ_En(pAIC, AIC_SRC_AUD_FIFO);
#endif
            glEncTicks=glAFETicks=glPrcTicks=glTimerTicks = 0;
#if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN
            gsAdjustTimeTicks= 0;
            gbTargetADCGain = MMPF_Audio_GetADCAnalogGain() - AFE_FIX_BOOST_GAIN ;
#endif            
        }
    } else {
        // print free slot size per seocnd
        if( (gsAudTicks % 2000)==0) {
            AITPS_GBL   pGBL = AITC_BASE_GBL;
           // dbg_printf(3," pGBL->GBL_RST_SEL:%x\r\n", pGBL->GBL_RST_SEL);
            //dbg_printf(3,"D:Free(%d),Ready(%d)\r\n",MMPF_Audio_GetSizeByType(SIZE_CAN_WR),MMPF_Audio_GetSizeByType(SIZE_CAN_RD) );
            //check_aud_ticks();
        }
        err = MMPF_Audio_Tx(gsAudioSamplesPerFrame);
#if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN        
        MMPF_Audio_AdjustADCGain(1);
#endif        
    }
}
#if CODE_OPT_VER >0
#pragma arm section code , rwdata ,  zidata 
#endif

void MMPF_Audio_InitEncoder(MMP_USHORT sample_rate,MMP_SHORT samples_per_frame)
{
    MMP_SHORT i,*ptr;
    MMP_UBYTE err;

    ENC_CUR_SAMPLES = samples_per_frame  ;
    ENC_CUR_INBUF_SIZE = ENC_CUR_SAMPLES*ENC_INBUF_FRAME_NUM ;

    #if 0 
    if(gbAudioTaskPrioStatus==1) {
        err = OSTaskChangePrio(TASK_USB_PRIO1,TASK_USB_PRIO);
        dbg_printf(3,"#Prio Usb:%d\r\n",err);
    }    
    #endif
    
    
    gbAudioTaskPrioStatus = 0 ;
    gsAudioRecEvent = 0 ;
    AudioEncInBufHandle.afe_intr_i = 0 ;
    AudioEncInBufHandle.rdPtr=0;
    AudioEncInBufHandle.wrPtr=0;
    AudioEncInBufHandle.prPtr=0;
    AudioEncInBufHandle.rdWrap=0;
    AudioEncInBufHandle.wrWrap=0;
    AudioEncInBufHandle.prWrap=0;
    AudioEncInBufHandle.bufSize = ENC_CUR_INBUF_SIZE *sizeof(MMP_SHORT);
   // AudioEncInBufHandle.freeSize=ENC_INBUF_SIZE *sizeof(MMP_SHORT);
    gsAudioSamplesPerFrame = samples_per_frame ;
    gsInAudioSamplesPerFrame =  ENC_SAMPLES_TIME*((MMPF_Audio_GetRealSampleRate(sample_rate)*AUDIN_CHANNEL)/1000) ;
    gsAudioSampleRate = sample_rate ;
    AudioEncInBufHandle.bufBase = (MMP_ULONG)gsAACEncInBuf;
    MMPF_Audio_3rdPartyInit() ;
    gsAudTicks = 0 ;
    gbAud1stInToken = 0 ;
    dbg_printf(3,"In/Out sampels : %d/%d\r\n",gsInAudioSamplesPerFrame,gsAudioSamplesPerFrame);
   // Open 2 ms timer first to detect 1st In Token
    #if(CHIP==P_V2)
    MMPF_Audio_TimerOpen(2000,MMPF_TIMER_3);
    #endif
    #if(CHIP==VSN_V2)||(CHIP==VSN_V3)
    MMPF_Audio_TimerOpen(2000,MMPF_TIMER_2);
    #endif
    
}


void    MMPF_Audio_ExitEncoder(void)
{
    gbAud1stInToken = 0 ;
    #if(CHIP==P_V2)
    MMPF_Audio_TimerClose(MMPF_TIMER_3);
    #endif
    #if(CHIP==VSN_V2)||(CHIP==VSN_V3)
    MMPF_Audio_TimerClose(MMPF_TIMER_2);
    #endif
    MMPF_Audio_3rdPartyExit();
}
 
MMP_ULONG glPrcTicks ;
void    MMPF_Audio_EncodeFrame(void)
{
    MMP_ULONG cpu_sr;
    int     result,size;
    int i , slots = 0,j ;
    MMP_SHORT *inPtr,*outPtr ;
    /* encode one frame */
    /* a stereo encoder always needs two channel input */
    // 3rd Party alogrithm put here
enc_entry:    
    IRQ_LOCK(
        size = MMPF_Audio_GetSizeByType(SIZE_CAN_PR) ;
    )
    if(size < 0) {
        dbg_printf(3,"#Enc.bad.PR : %d\r\n",size);
        
    }
    if(size >=  (gsAudioSamplesPerFrame*PRC_SAMPLES_TIME << 1) ) {
        MMP_SHORT *inPtr2 ;
        glPrcTicks++ ;
        inPtr = (MMP_SHORT *)MMPF_Audio_GetPrPtr() ; 
        outPtr = inPtr ;
        //if(size!=gsAudioSamplesPerFrame*PRC_SAMPLES_TIME<<1) {
        //    dbg_printf(3,"#PR size:%d\r\n",size);
        //}
        MMPF_Audio_3rdPartyEncode(outPtr,inPtr,/*gsAudioSamplesPerFrame*/gsInAudioSamplesPerFrame*PRC_SAMPLES_TIME );
        IRQ_LOCK( 
            MMPF_Audio_UpdatePrPtr(gsAudioSamplesPerFrame*PRC_SAMPLES_TIME );
        )
        // Go back to encode again if more samples
        goto enc_entry;
    }
}


void MMPF_Audio_UpdateWrPtr(MMP_ULONG size)
{
    MMP_ULONG wrPtr,sync = 0 ;
    // Drop this frame if buffer full
    if(MMPF_Audio_IsFull()) {
       //dbg_printf(3,"@F-(%d,%d,%d)\r\n",glAFETicks,glEncTicks,glPrcTicks);
       AudioEncInBufHandle.rdWrap = AudioEncInBufHandle.wrWrap ;
       AudioEncInBufHandle.rdPtr = AudioEncInBufHandle.wrPtr ;
       sync = 1; 
    }
	AudioEncInBufHandle.wrPtr += ( size << 1);
    if (AudioEncInBufHandle.wrPtr >=  AudioEncInBufHandle.bufSize ) {
    	AudioEncInBufHandle.wrPtr -=  AudioEncInBufHandle.bufSize  ;
    	AudioEncInBufHandle.wrWrap++;
    }
    if(sync) {
       AudioEncInBufHandle.prWrap = AudioEncInBufHandle.wrWrap ;
       AudioEncInBufHandle.prPtr = AudioEncInBufHandle.wrPtr ;
    
    }
    
}

void MMPF_Audio_UpdateRdPtr(MMP_ULONG size)
{

    MMP_ULONG rdPtr ;
    MMP_SHORT *ptr; 
    AudioEncInBufHandle.rdPtr+=( size << 1);
    if (AudioEncInBufHandle.rdPtr >= AudioEncInBufHandle.bufSize ) {
        AudioEncInBufHandle.rdPtr -= AudioEncInBufHandle.bufSize ;
    	AudioEncInBufHandle.rdWrap++;
    }
    
}


void MMPF_Audio_UpdatePrPtr(MMP_ULONG size)
{
    MMP_ULONG prPtr ;
    MMP_SHORT *ptr; 
    AudioEncInBufHandle.prPtr+=( size << 1);
    if (AudioEncInBufHandle.prPtr >= AudioEncInBufHandle.bufSize ) {
        AudioEncInBufHandle.prPtr -= AudioEncInBufHandle.bufSize ;
    	AudioEncInBufHandle.prWrap++;
    }
    //dbg_printf(3,"PR:%d,Wrap:%d\r\n",AudioEncInBufHandle.prPtr,AudioEncInBufHandle.prWrap);
}


void MMPF_Audio_UpdatePtrByType(AUDIO_SIZE_READ_TYPE type,MMP_ULONG size)
{
    MMP_ULONG ptr,wrap ;
    if(type==SIZE_CAN_WR) {
        ptr = AudioEncInBufHandle.wrPtr + (size << 1 );
        wrap = AudioEncInBufHandle.wrWrap ;
    } 
    else if(type==SIZE_CAN_PR) {
        ptr = AudioEncInBufHandle.prPtr + (size << 1 ); 
        wrap = AudioEncInBufHandle.prWrap ;
    }
    else if (type==SIZE_CAN_RD) {
        ptr = AudioEncInBufHandle.rdPtr + (size << 1 );
        wrap = AudioEncInBufHandle.rdWrap ;
    }
    if(ptr >= AudioEncInBufHandle.bufSize) {
        ptr-= AudioEncInBufHandle.bufSize ;
        wrap++ ;
    }
    if(type==SIZE_CAN_WR) {
        AudioEncInBufHandle.wrPtr = ptr; 
        AudioEncInBufHandle.wrWrap = wrap;
    }
    else if(type==SIZE_CAN_PR) {
        AudioEncInBufHandle.prPtr = ptr; 
        AudioEncInBufHandle.prWrap = wrap;
    }
    else if(type==SIZE_CAN_RD) {
        AudioEncInBufHandle.rdPtr = ptr; 
        AudioEncInBufHandle.rdWrap = wrap;
    }
}


MMP_ULONG MMPF_Audio_GetWrPtr(void)
{
	return AudioEncInBufHandle.bufBase + AudioEncInBufHandle.wrPtr;
}


MMP_ULONG MMPF_Audio_GetRdPtr(void)
{
	return AudioEncInBufHandle.bufBase + AudioEncInBufHandle.rdPtr;
}


MMP_ULONG MMPF_Audio_GetPrPtr(void)
{
	return AudioEncInBufHandle.bufBase + AudioEncInBufHandle.prPtr;
}


// How many fram can be send by USB
int MMPF_Audio_GetSizeByType(AUDIO_SIZE_READ_TYPE type)
{
    int wrap;
    int size,p1,p2;
    if(type == SIZE_CAN_WR) {
        wrap = AudioEncInBufHandle.wrWrap - AudioEncInBufHandle.rdWrap ; 
        p1 = AudioEncInBufHandle.wrPtr ;
        p2 = AudioEncInBufHandle.rdPtr ;
        if( (wrap < 0) || (wrap > 1) ) {
            //dbg_printf(3,"@W != R:%d,%d\r\n",AudioEncInBufHandle.wrWrap,AudioEncInBufHandle.rdWrap);
            return 0;
        }
    } 
    else if ( type == SIZE_CAN_PR ) {
        wrap = AudioEncInBufHandle.wrWrap - AudioEncInBufHandle.prWrap ; 
        p1 = AudioEncInBufHandle.wrPtr ;
        p2 = AudioEncInBufHandle.prPtr ;
        if( (wrap < 0) || (wrap > 1) ) {
           // dbg_printf(3,"@W != P:%d,%d\r\n",AudioEncInBufHandle.wrWrap,AudioEncInBufHandle.prWrap);
            return 0;
        }
    }
    else if ( type == SIZE_CAN_RD ) {
        wrap = AudioEncInBufHandle.prWrap - AudioEncInBufHandle.rdWrap ; 
        p1 = AudioEncInBufHandle.prPtr ;
        p2 = AudioEncInBufHandle.rdPtr ;
        if( (wrap < 0) || (wrap > 1) ) {
          //  dbg_printf(3,"@P != R:%d,%d\r\n",AudioEncInBufHandle.prWrap,AudioEncInBufHandle.rdWrap);
            return 0;
        }
    }
    
    if(wrap==0) {
        size = (int)(p1 - p2 ) ;
    } else {
        size = (int)(AudioEncInBufHandle.bufSize  + p1 - p2) ;
    }
    if(type == SIZE_CAN_WR) {
        size = AudioEncInBufHandle.bufSize - size ;
    }
    return size ;
    
}

#if CODE_OPT_VER >0
#pragma arm section code = "aud_tx", rwdata = "aud_tx",  zidata = "aud_tx"
#endif
MMP_ERR MMPF_Audio_Tx(MMP_SHORT samples)
{
extern MMP_ULONG usbframecount ;
static MMP_ULONG glPassedTicks = 0 ,glSyncTicks = 0 ,glSyncLoop = 0;              
    MMP_UBYTE err = OS_NO_ERR ;
    int readySize ;
#if AUDIO_RESYNC_EN    
    if(AudioResyncData.gsSkipTimerTicks > 0) {
        AudioResyncData.gsSkipTimerTicks-- ;
        return ;
    }
    glTimerTicks++;
#endif    
    readySize = MMPF_Audio_GetSizeByType(SIZE_CAN_RD);
    if(uac_is_tx_busy()) {
       // dbg_printf(3,"Tx0\r\n");
        return MMP_AUDIO_ERR_STREAM_OVERFLOW;
    }
    if( /*MMPF_Audio_IsEmpty()*/readySize < (samples+AUDIN_CHANNEL) << 1 ) {
        //dbg_printf(3,"@E : %d,(%d,%d,%d)-%d\r\n",readySize,glAFETicks,glEncTicks,glPrcTicks,usbframecount);
       // dbg_printf(3,"(%d,%d,%d,%d)\r\n",AudioEncInBufHandle.prWrap,AudioEncInBufHandle.rdWrap,AudioEncInBufHandle.prPtr,AudioEncInBufHandle.rdPtr);
        
        return MMP_AUDIO_ERR_STREAM_BUF_EMPTY;    
    }
#if AUDIO_RESYNC_EN    
    if( MMPF_Audio_Resync_Start(&samples) ) {
        dbg_printf(3,"resync :%d\r\n",readySize);
        goto tx_aud ;
    }
#endif    
    
    if(readySize >=  (READYFRAME_HIGHBOUND_TH*ENC_CUR_SAMPLES << 1) ) {
#if AUDIO_RESYNC_EN    
        if(AudioResyncData.gbResyncEn==0){
            dbg_printf(0,"+p: %d,%d\r\n",glTimerTicks,readySize);
        }
        MMPF_Audio_Resync_En(1);
#endif
        
        samples += AUDIN_CHANNEL ;
    }
    else  if(readySize <=READYFRAME_LOWBOUND_TH*ENC_CUR_SAMPLES << 1 ) {
#if AUDIO_RESYNC_EN    
        if(AudioResyncData.gbResyncEn==0){
            dbg_printf(0,"-p: %d,%d\r\n",glTimerTicks,readySize);
        }
        MMPF_Audio_Resync_En(2);
#endif        
       samples -= AUDIN_CHANNEL; 
    } 
#if AUDIO_RESYNC_EN    
    MMPF_Audio_Resync_GetTicks();
#endif
    
tx_aud:    
    MMPF_Audio_Move2USBFifo(samples);
    uac_process_audio_data();
    MMPF_Audio_UpdateRdPtr(samples);
    return MMP_ERR_NONE ;
    
}


void MMPF_Audio_Move2USBFifo(MMP_SHORT samples)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_SHORT i ,*ptr , s;
    ptr = (MMP_SHORT *)MMPF_Audio_GetRdPtr();
    
    s =  ( AudioEncInBufHandle.bufSize - ((MMP_ULONG)AudioEncInBufHandle.rdPtr))>>1 ;
    if(s >= samples) {
        for(i=0;i<samples;i++) {
            pUSB_CTL->USB_FIFO_EP[PCCAM_AU_EP_ADDR].FIFO_W = ptr[i] ;
        }
    } else {
        for(i=0;i< s ;i++) {
            pUSB_CTL->USB_FIFO_EP[PCCAM_AU_EP_ADDR].FIFO_W = ptr[i] ;
        }
        s = samples - s ;
        ptr = (MMP_SHORT *)AudioEncInBufHandle.bufBase;
        for(i=0;i< s ;i++) {
            pUSB_CTL->USB_FIFO_EP[PCCAM_AU_EP_ADDR].FIFO_W = ptr[i] ;
        }
    }  
}

#if CODE_OPT_VER >0
#pragma arm section code , rwdata,  zidata 
#endif

MMP_BOOL MMPF_Audio_IsEmpty(void)
{
    return MMPF_Audio_GetSizeByType(SIZE_CAN_RD)?0:1;
}

MMP_BOOL MMPF_Audio_IsFull(void)
{
    //return (AudioEncInBufHandle.freeSize <  (ENC_MAX_SAMPLES << 1) )?1:0;
    MMP_ULONG size ;
    size = MMPF_Audio_GetSizeByType(SIZE_CAN_WR) ;
    if(size < (ENC_CUR_SAMPLES << 1) ) {
        return 1 ;
    }
    return 0;
}


MMP_BOOL MMPF_Audio_IsSkipPostProcessing(void)
{
//    return gbSkipPostProcessing ;
    return 0;
}

void MMPF_Audio_SkipPostProcessing(MMP_BOOL flag)
{
}




void MMPF_Audio_3rdPartyInit(void)
{
  //  gbNotchFilterEn = 1 ;
}

void MMPF_Audio_3rdPartyExit(void)
{
}
 
void MMPF_Audio_3rdPartyEncode(MMP_SHORT *outPtr,MMP_SHORT *inPtr,MMP_SHORT samples)
{

}
#endif


#if AUDIO_RESYNC_EN
void MMPF_Audio_Resync_Init(MMP_USHORT skip_ticks)
{
    AudioResyncData.gsSkipTimerTicks = skip_ticks ;
    AudioResyncData.glPassedTicks = 0;
    AudioResyncData.gbResyncDir = 0 ;
    AudioResyncData.gbResyncEn = 0 ;
}

MMP_BOOL MMPF_Audio_Resync_Start(MMP_SHORT *samples)
{
    if(AudioResyncData.glPassedTicks) {
       AudioResyncData.glResyncTicks--;
       if(AudioResyncData.gbResyncDir==1) {
            if(AudioResyncData.glResyncTicks==0) {
                *samples += AUDIN_CHANNEL ;
                AudioResyncData.glResyncTicks = AudioResyncData.glPassedTicks ;
               // dbg_printf(3,"+ :%d\r\n",readySize);
                //goto tx_aud ;
                return MMP_TRUE ;
            }
        }
        if(AudioResyncData.gbResyncDir==2) {
            if(AudioResyncData.glResyncTicks==0) {
                *samples -= AUDIN_CHANNEL ;
                AudioResyncData.glResyncTicks = AudioResyncData.glPassedTicks ;
              //  dbg_printf(3,"- :%d\r\n",readySize);
                //goto tx_aud ;
                return MMP_TRUE ;
            }
        }
    }
    return MMP_FALSE ;

}

void MMPF_Audio_Resync_En(MMP_UBYTE dir)
{
    if(AudioResyncData.gbResyncEn==0) {
        AudioResyncData.gbResyncEn = 2 ;
    } 

}

void MMPF_Audio_Resync_GetTicks(void)
{
    if(AudioResyncData.gbResyncEn&&!AudioResyncData.glPassedTicks) {
        AudioResyncData.glPassedTicks = (glTimerTicks*AUDIN_CHANNEL ) / (4 * gsAudioSamplesPerFrame)  ;
        AudioResyncData.glResyncTicks = AudioResyncData.glPassedTicks ;
        AudioResyncData.gbResyncDir = AudioResyncData.gbResyncEn ;
        AudioResyncData.gbResyncEn = 0;
    }

}
#endif






