#ifndef MMPF_WAVDECAPI_H
#define MMPF_WAVDECAPI_H
#include "mmpf_audio_ctl.h"
#include "mmpf_audio_typedef.h"

//#ifndef WAV_DEC_API
//#define WAV_DEC_API

#define WAV_OUTFRAMESIZE    (1024)
#define WAV_BUFDEPTH        (2)
#define WAV_READ_SAMPLE     (256)

#define WAV_READBUF_SIZE    (1024*4)    
#define WAV_FILLUNIT        (WAV_READBUF_SIZE/2)

#define UNCOMPRESSED_PCM    (0x01)

typedef struct {    
    Word32u rd_ind;             // read pointer
    Word32u wr_ind;             // write pointer
    Word32  SampleLeft;         // remainder samples in buffer
    Word32u BufButton;          // 2*2304-1 or 2*1152   
} WavInDataObj;

typedef struct _WAVFileInfo {
    Word32u bitRate             /*AvgBytesPerSec*/;
    Word32u totaldataSize;      //total wave data to play.
    Word16u compressionCode;
    Word16u nChannels;
    Word16u sampleRate;
    Word16u newSampleRate;
    /*Word16u   blockAlign;*/
    Word16u bitsPerSample;
} WAVFileInfo;

extern Word32 WavInbufTask(void);

extern void MMPF_StartWAVDec(void);
extern void MMPF_StopWAVDec(void);
extern void MMPF_PauseWAVDec(void);
extern void MMPF_ResumeWAVDec(void);

extern void MMPF_DecodeWAVFrame(int decoder);
extern MMP_SHORT MMPF_ExtractWAVInfo(WAVFileInfo *wavinfo);

extern MMP_ULONG MMPF_GetWAVDecFramesInByte(void);

#endif

