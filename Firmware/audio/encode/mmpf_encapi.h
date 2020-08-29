#ifndef MMPF_ENCAPI_API
#define MMPF_ENCAPI_API
#include "mmpf_timer.h"

#define AUDIO_RESYNC_EN     (0)
#define UPSAMPLE_32K_48K    (0) // enable 32K to 48K upsample buffer control 
#define AFE_SAMPLES_TIME    (2)
#define ENC_SAMPLES_TIME    (1) // 4 ms
#define PRC_SAMPLES_TIME    (4) // How many samples for audio lib to process each time
#define ADV_DUMPY_SAMPLES_TIME  (2) // is (PRC_SAMPLES_TIME /AFE_SAMPLES_TIME)
 

#if ENC_SAMPLES_TIME==1
#define ENC_INBUF_FRAME_NUM  (16)//(8) How many ring buffer slot
#else
#define ENC_INBUF_FRAME_NUM  (4)//(8) How many ring buffer slot
#endif

#define ENC_MAX_SAMPLES  ( 48 * AUDIN_CHANNEL *  ENC_SAMPLES_TIME )// 48000 / 1000 * 2 ch * ENC_SAMPLES_TIME ms = 384.
// (192)   // 48000 / 1000 * 2 ch * 2 ms = 192.
#define ENC_INBUF_SIZE  (ENC_MAX_SAMPLES*ENC_INBUF_FRAME_NUM)  

#define PRC_INBUF_SIZE  (ENC_MAX_SAMPLES * PRC_SAMPLES_TIME)

typedef enum _AUDIO_SIZE_READ_TYPE
{ 
    SIZE_CAN_WR=0,
    SIZE_CAN_PR,
    SIZE_CAN_RD
} AUDIO_SIZE_READ_TYPE ;

#if AUDIO_RESYNC_EN==1
typedef struct _AudioResyncData_t
{
    MMP_ULONG  glPassedTicks ;
    MMP_ULONG  glResyncTicks ;
    MMP_USHORT gsSkipTimerTicks ;
    MMP_UBYTE  gbResyncDir ;
    MMP_UBYTE  gbResyncEn  ;
} AudioResyncData_t ;

void MMPF_Audio_Resync_Init(MMP_USHORT skip_ticks) ;
MMP_BOOL MMPF_Audio_Resync_Start(MMP_SHORT *samples);
void MMPF_Audio_Resync_En(MMP_UBYTE dir );
void MMPF_Audio_Resync_GetTicks(void);
#endif


void MMPF_Audio_EncodeFrame(void);
void MMPF_Audio_InitEncoder(MMP_USHORT sample_rate,MMP_SHORT samples_per_frame);
MMP_BOOL MMPF_Audio_IsEmpty(void);
MMP_BOOL MMPF_Audio_IsFull(void);
void MMPF_Audio_Move2USBFifo(MMP_SHORT samples);
void MMPF_Audio_UpdateWrPtr(MMP_ULONG size);
void MMPF_Audio_UpdateRdPtr(MMP_ULONG size);
void MMPF_Audio_UpdatePrPtr(MMP_ULONG size);
void MMPF_Audio_UpdatePtrByType(AUDIO_SIZE_READ_TYPE type,MMP_ULONG size);
MMP_ULONG MMPF_Audio_GetPrPtr(void);
MMP_ULONG MMPF_Audio_GetWrPtr(void);
MMP_ULONG MMPF_Audio_GetRdPtr(void);
int MMPF_Audio_GetSizeByType(AUDIO_SIZE_READ_TYPE type);
void MMPF_Audio_ExitEncoder(void);
MMP_BOOL MMPF_Audio_IsSkipPostProcessing(void);
void MMPF_Audio_SkipPostProcessing(MMP_BOOL flag);
MMP_ERR MMPF_Audio_Tx(MMP_SHORT samples);
void MMPF_Audio_TimerOpen(MMP_ULONG us,MMPF_TIMER_ID id);
void MMPF_Audio_TimerClose(MMPF_TIMER_ID id);
void MMPF_Audio_TimerISR(void);
MMP_USHORT MMPF_Audio_GetRealSampleRate(MMP_USHORT in_sr);
#if ADC_FINE_TUNE_POP_NOISE==FIXED_PGA_GAIN
void MMPF_Audio_SetTargetADCGain(MMP_BYTE adc_gain,MMP_USHORT ticks);
#endif

#endif
