#ifndef MMPF_AACENCAPI_API
#define MMPF_AACENCAPI_API

#define AACENC_INBUF_FRAME_NUM  4
#define AACENC_INBUF_SIZE  (1024*2*AACENC_INBUF_FRAME_NUM)
#define AACENC_FRAMESIZE_Q_NUM  10
/*
typedef struct {
    short rd_index;
    short wr_index;
    unsigned int total_rd;
    unsigned int total_wr;
}AAC_ENC_INBUF_HANDLE;
*/
extern void MMPF_EncodeAACFrame(void);
extern void MMPF_InitAACEncoder(void);
extern void MMPF_SetAACRecParameter(unsigned int bitrate,unsigned int sampleRate);
extern void MMPF_GetAACRecParameter(unsigned int *bitrate,unsigned int *sampleRate);
extern void MMPF_SetAACEncMode(unsigned short mode);
extern unsigned int MMPF_GetAACRecFrameCnt(void);
extern void MMPF_PostsetAACRecordData(MMP_BYTE *glFileName);
MMP_BOOL AAC_Audio_IsSlotFull(void);
int AACvoiceinbuf(short* bufin, int sample);
#endif
