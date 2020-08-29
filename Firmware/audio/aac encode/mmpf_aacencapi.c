/*
   AAC encoder API
 */
#include "includes_fw.h"
#if (AUDIO_AAC_R_EN)

#include "lib_retina.h"
#include "mmpf_audio_ctl.h"
#include "mmpf_aacencapi.h"
#include "mmpf_audio.h"
#include "mmpf_audio_typedef.h"
/* for WAV audio file support */

//#include "au_channel.h"
//#include "aacenc.h"
//#include "qc_data.h"
//#include "psy_main.h"
//#include "bitenc.h"

#include "aacenc_include.h"

#include "mmpf_mts.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc.h"

/* ETSI operators types and WMOPS measurement */
//#include "typedef.h"
/*
MMP_ULONG	glAACEncParameter[56][2] = {	//Wilson@100729
				{16000, 16000},	{16000, 24000},	{16000, 32000},	{16000, 64000},
				{16000, 96000}, {16000, 128000}, {16000, 160000}, {16000, 192000},
				///////////////////////////////////////////////////////////////////////
				{22050, 16000},	{22050, 24000}, {22050, 32000},	{22050, 64000},
				{22050, 96000},	{22050, 128000}, {22050, 160000}, {22050, 192000}, {22050, 256000},
				///////////////////////////////////////////////////////////////////////
				{24000, 16000},	{24000, 24000}, {24000, 32000},	{24000, 64000},
				{24000, 96000},	{24000, 128000}, {24000, 160000}, {24000, 192000}, {24000, 256000},
				///////////////////////////////////////////////////////////////////////
				{32000, 16000},	{32000, 24000}, {32000, 32000},	{32000, 64000},
				{32000, 96000},	{32000, 128000}, {32000, 160000}, {32000, 192000}, {32000, 256000}, {32000, 320000},
				///////////////////////////////////////////////////////////////////////
				{44100, 16000},	{44100, 24000}, {44100, 32000},	{44100, 64000},
				{44100, 96000},	{44100, 128000}, {44100, 160000}, {44100, 192000}, {44100, 256000}, {44100, 320000},
				///////////////////////////////////////////////////////////////////////
				{48000, 16000},	{48000, 24000}, {48000, 32000},	{48000, 64000},
				{48000, 96000},	{48000, 128000}, {48000, 160000}, {48000, 192000}, {48000, 256000}, {48000, 320000}
			};
*/
/* Profiling */
enum {
  SBR_ENC = 0,
  RESAMPLER,
  AAC_ENC
};
UWord32 M2tsCounter = 1;

#define NULL                0

#define MAX_PAYLOAD_SIZE    128

#define BYTE_NUMBIT 8       /* bits in byte (char) */

typedef struct
{
  long numBit;          /* number of bits in buffer */
  long size;            /* buffer size in bytes */
  long currentBit;      /* current bit position in bit stream */
  long numByte;         /* number of bytes read/written (only file) */
  unsigned char *data;      /* data bits */
} BitStream;


#ifndef min
#define min(a, b)  ((a < b) ? (a) : (b))
#endif

#define AACENC_SIZEOF_WORKING_BUF	0x8000	//32K bytes

#if (AUDIO_AAC_R_EN)
int *aacencWorkingBuf=0;
short *gsAACEncInVoiceBuf=0;
unsigned char *outputBuffer=0;
MMP_ULONG	glAudioWorkingBufferEnd;

#endif

short                   inSamples;
AACENC_CONFIG           config;
AuChanInfo              inputInfo;
int                     frmCnt = 0;
int                     sampleRateAAC;
Flag                    bEncodeMono = 0;
short                   numAncDataBytes=0;
unsigned char           ancDataBytes[MAX_PAYLOAD_SIZE];

int                     nOutBytes;
unsigned char           *bitstreamOut;// = (unsigned char*)outputBuffer;       /////////

#if (VAAC_R_EN)
extern MMP_ULONG MMPF_VIDMGR_GetAudioCompBufReadPtr(void);
#endif
extern unsigned int     glAudioRecBufStart;
extern unsigned int     glAudioRecBufSize;
extern unsigned int     glAudioRecWritePtr;
extern unsigned int     glAudioRecWritePtrHigh;
extern unsigned int     glAudioRecReadPtr;
extern unsigned int     glAudioRecReadPtrHigh;
extern MMPF_AUDIO_DATAPATH    audiorecpath;

extern  unsigned int    glAudioEncodeLength;
unsigned int            glAACDataEncoded;

AUDIO_ENC_INBUF_HANDLE AudioAACEncInBufHandle;
extern MMP_USHORT gsAudioEncodeFormat;
unsigned int glAacRecWritePtr;
//unsigned short AACFrameSizeQ[AACENC_FRAMESIZE_Q_NUM];
//unsigned short AACFrameSizeQIndex;

#if (NOISE_REDUCTION_EN == 1)
#include "noise_reduction.h"
extern	NRState *den;
extern	int	nr_total_ram;
#endif

/* Function prototypes */
BitStream *OpenBitStream(int size, unsigned char *buffer);
int CloseBitStream(BitStream *bitStream);


static int WriteADTSHeader(int sampleRate,
                           int channels,
                           BitStream *bitStream,
                           int writeFlag,
                           int numBytes);

int PutBit(BitStream *bitStream,
           unsigned long data,
           int numBit);

int GetSRIndex(unsigned int sampleRate);

int WriteByte(BitStream *bitStream,
              unsigned long data,
              int numBit);

MMP_ULONG            glAACEncSampleRate;
MMP_ULONG            glAACEncBitRate;
//----------------------------------------
//MMP_SHORT   *gsAACEncInVoiceBuf=0;
int AACvoiceinbuf(short* bufin, int sample){
	int i=0;
	short *dst = (short*)(&gsAACEncInVoiceBuf[AudioAACEncInBufHandle.wr_index]);
	for(i = 0 ; i< sample;i++)
		dst[i] = bufin[i];
	
	//memcpy((void*) (gsAACEncInVoiceBuf + AudioAACEncInBufHandle.wr_index),(void*)bufin , sample);

	AudioAACEncInBufHandle.wr_index += sample ;
	AudioAACEncInBufHandle.total_wr += 1;
	if (AudioAACEncInBufHandle.wr_index >= (AACENC_INBUF_SIZE>>1))
		AudioAACEncInBufHandle.wr_index -= (AACENC_INBUF_SIZE>>1);
	
	return 0;
}
MMP_BOOL AAC_Audio_IsSlotFull(void)
{
	//Gason@ use circular method.
	if(AudioAACEncInBufHandle.total_wr >=  AudioAACEncInBufHandle.total_rd){
	    if ((AudioAACEncInBufHandle.total_wr - AudioAACEncInBufHandle.total_rd) >= 32 ) {
	        return 1 ;
	    }		
	}else{
		if(AudioAACEncInBufHandle.total_wr <  AudioAACEncInBufHandle.total_rd){
			if((0xFFFFFFFF - AudioAACEncInBufHandle.total_rd + AudioAACEncInBufHandle.total_wr) >= 32)
				return 1;

		}
			
	}
    return 0 ;
}
MMPF_MTS_TIME_INFO AACTimeInfo;
int encodeAACfileInit(void)
{

	short error;
	short nChannelsAAC;

	int retsize;

	#if (NOISE_REDUCTION_EN == 1)
	nr_total_ram = 0;              //2007-01-15 add
   	den = nr_state_init(NR_NOISE_FLOOR);
   	#endif

	AudioAACEncInBufHandle.rd_index = 0;
	AudioAACEncInBufHandle.total_rd = 0;
	AudioAACEncInBufHandle.wr_index = 0;
	AudioAACEncInBufHandle.total_wr = 0;
	glAudioEncWorkingBuf =(int	*) 0x1740000;
	AACTimeInfo.ubTimeMsb32 = 0;
	AACTimeInfo.ulTime = 54000000;
	#if 1
	AACTimeInfo.ulTimeDelta = 5625; // = (90K * sample_per_frame)/sample_rate
	#elif 0
	AACTimeInfo.ulTimeDelta = 51200;
	#endif
	AACTimeInfo.ulTimeDecimalDelta = 0;
	

	#if (AUDIO_AAC_R_EN)
	aacencWorkingBuf = glAudioEncWorkingBuf;
	gsAACEncInVoiceBuf = (short *)((MMP_ULONG)glAudioEncWorkingBuf + (AACENC_SIZEOF_WORKING_BUF));
	outputBuffer = (unsigned char *)( (MMP_ULONG)gsAACEncInVoiceBuf + (AACENC_INBUF_SIZE));
	glAudioWorkingBufferEnd = AACENC_SIZEOF_WORKING_BUF + AACENC_INBUF_SIZE
								+ (6144/8)*MAX_CHANNELS;		
	/*
	aacencWorkingBuf = glAudioEncWorkingBuf;
	gsAACEncInVoiceBuf = (short *)(&glAudioEncWorkingBuf[AACENC_SIZEOF_WORKING_BUF>>2]);
	outputBuffer = (unsigned char *)(&(glAudioEncWorkingBuf[(AACENC_SIZEOF_WORKING_BUF>>2) ]));
	glAudioWorkingBufferEnd = AACENC_SIZEOF_WORKING_BUF +  768*MAX_CHANNELS;					
	*/
	#endif
	memset(ancDataBytes,0,MAX_PAYLOAD_SIZE);
	memset(gsAACEncInVoiceBuf,0,AACENC_INBUF_SIZE);
/*	RTNA_DBG_PrintLong(0,(MMP_ULONG)aacencWorkingBuf);
	RTNA_DBG_PrintLong(0,(MMP_ULONG)gsAACEncInVoiceBuf);
	RTNA_DBG_PrintLong(0,(MMP_ULONG)outputBuffer);
	RTNA_DBG_PrintLong(0,(MMP_ULONG)glAudioWorkingBufferEnd);
*/
//20080801 add for audio all in one. ---
  retsize = AACEnc_SetWorkingBuf(aacencWorkingBuf);

  if(retsize > AACENC_SIZEOF_WORKING_BUF){
	  RTNA_DBG_Str(0,"Error set aacenc working buffer\r\n");
      return -1;
  }


	bitstreamOut = outputBuffer;



 /* default bitrate = 128kbps */

  /*
    open audio input file
  */

  inputInfo.bitsPerSample  = 16 ;       /* only relevant if valid == 1 */
  inputInfo.sampleRate = glAACEncSampleRate;
  inputInfo.nChannels      = 2 ;        /* only relevant if valid == 1 */
  inputInfo.nSamples       = 0 ;        /* only relevant if valid == 1 */
  inputInfo.isLittleEndian = 1;

  inputInfo.fpScaleFactor  = 1 ;      /* must be set */
  inputInfo.valid          = 1 ;        /* must be set */
  inputInfo.useWaveExt     = 0;


  if ( (!bEncodeMono) && (inputInfo.nChannels!=2) ) {
    RTNA_DBG_Str(0,"\r\nNeed stereo input for stereo coding mode !");
    return -1;
  }

  sampleRateAAC = inputInfo.sampleRate;

  /* open encoder */
  /* make reasonable default settings */
  AACEnc_AacInitDefaultConfig(&config);

  nChannelsAAC = inputInfo.nChannels;
  sampleRateAAC = inputInfo.sampleRate;
  config.bitRate = glAACEncBitRate;
  config.nChannelsIn = nChannelsAAC;
  config.nChannelsOut = nChannelsAAC;

  config.bandWidth=(int)((inputInfo.sampleRate>>1)*0.8);	//wilson: modify@100802
  /*
  if(inputInfo.sampleRate == 22050) {
    config.bandWidth = 8820;
  }else if(inputInfo.sampleRate == 32000) {
    config.bandWidth = 12800;
  }else if(inputInfo.sampleRate == 44100) {
    config.bandWidth = 17640;
  }
  */

//  config.bandWidth=7000;

  config.sampleRate = sampleRateAAC;

  error = AACEnc_AacEncOpen(&config);

  if ( error ) {
    RTNA_DBG_Str(0,"AacEncOpen failed \r\n" );
    return 1;
  }

//Hans Modify
//  for ( i = 0; i < (int)sizeof(inputBuffer)/2; i++ ) {
//    inputBuffer[i] = 0;
//  }


  /*
    Be verbose
  */
#if 0
  fprintf(stdout,"input file %s: \nsr = %d, nc = %d\n\n",
          audioFileName, (bDoUpsample) ? inputInfo.sampleRate/2 : inputInfo.sampleRate, inputInfo.nChannels);
  fprintf(stdout,"output file %s: \nbr = %ld sr-OUT = %ld  nc-OUT = %d\n\n",
          aacFileName, bitrate, (!bDingleRate) ? sampleRateAAC*2 : sampleRateAAC, nChannelsAAC);
  fflush(stdout);
#endif

/*
    RTNA_DBG_Str(0,"\r\nInput sample rate : ");
    RTNA_DBG_Long(0,(int)inputInfo.sampleRate);
    RTNA_DBG_Str(0,"\r\nInput channels : ");
    RTNA_DBG_Long(0,(int)inputInfo.nChannels);

    RTNA_DBG_Str(0,"\r\nOutput sample rate : ");
    RTNA_DBG_Long(0,(int)sampleRateAAC);
    RTNA_DBG_Str(0,"\r\nOutput channels : ");
    RTNA_DBG_Long(0,(int)nChannelsAAC);
    RTNA_DBG_Str(0,"\r\n");
    */
  /*
    set up input samples block size feed
  */
  inSamples = AACENC_BLOCKSIZE * inputInfo.nChannels ;

    return 0;
}

int AacOverFlowProtect(void)
{
	switch (gsAudioEncodeFormat) {
	/*#if (VAAC_R_EN)
	//case VIDEO_AAC_REC_MODE:
		//glAudioRecReadPtr = MMPF_VIDMGR_GetAudioCompBufReadPtr()&0xFFFF; //
	//	break;
	#endif		*/
//	#if (AAC_R_EN)
	case AAC_REC_MODE:
    	// MMPF_UpdateAudioRecReadPtr();
    	break;
	}

    if((glAudioRecWritePtr>glAudioRecReadPtr)&&((glAudioRecReadPtr+(glAudioRecBufSize-glAudioRecWritePtr))<0x600)){
        return 1;
    }else if((glAudioRecWritePtr<glAudioRecReadPtr)&&((glAudioRecReadPtr-glAudioRecWritePtr)<0x600)){
        return 1;
    }
    return 0;
}

//M2tsCounter = 1;
extern unsigned char aac_pes_data[];
extern unsigned char aac_ts_header[];
#if (MTS_MUXER_EN == 1)
UWord32 M2TSPackager(unsigned char *adts,UWord32 Targetbufstart, char *Audiobufstart, int audiolen){

	UWord32 targetbufaddr=(UWord32) Targetbufstart;
	UWord16 padding=0;
	UWord16 currAudioffset=0, i = 0 ;
	UWord16 need_PES_ADTS=1; // first package need PES & ADTS
    UWord16 packge_len[2][2] = { {184, 0} , {163, 1} };
	//RTNA_DBG_PrintLong(0, Targetbufstart);		
	//RTNA_DBG_PrintLong(0, audiolen);		
	while(currAudioffset < audiolen ){
        if ((audiolen-currAudioffset) < packge_len[need_PES_ADTS][0]) {
            padding = packge_len[need_PES_ADTS][0] - (audiolen-currAudioffset);
        }
        else {
            padding = 0;
        }
		MMPF_MTS_UpdateTsHeader (
        (MMP_UBYTE*)&aac_ts_header,
        packge_len[need_PES_ADTS][1],
        M2tsCounter++,
        padding);
		memcpy( (void*) targetbufaddr , (void*)aac_ts_header , (4+padding));
	    targetbufaddr+= (4+padding);

		if(need_PES_ADTS){
			// =================================================================================
		    // PES part
	    	// =================================================================================	
	    	MMPF_MTS_IncreamentPesTimestamp(&AACTimeInfo);
			if(M2tsCounter!=2)
		    MMPF_MTS_UpdatePesTimestamp(
	        &(aac_pes_data[PES_PTS_PAYLOAD_OFFSET]), //PES_PTS_PAYLOAD_OFFSET¬OPTSªºoffset
	        AACTimeInfo.ubTimeMsb32,          //msb 33th bit 
	        AACTimeInfo.ulTime);  //bit[31:0]
			MMPF_MTS_UpdatePesAudioLens(&(aac_pes_data[PES_PTS_AUDIO_LENS_PAYLOAD_OFFSET]), audiolen+7);
			
			memcpy( (void*) targetbufaddr , (void*)aac_pes_data , 14);			
			targetbufaddr+=14 ; 
			memcpy( (void*) targetbufaddr , (void*) adts , 7 );
			targetbufaddr+= 7 ; 
		}

		i =  ( (audiolen-currAudioffset) > packge_len[need_PES_ADTS][0]) ? packge_len[need_PES_ADTS][0] : (audiolen-currAudioffset) ; 
		
		memcpy( (void*) targetbufaddr , (void*)(Audiobufstart + currAudioffset),i);
		targetbufaddr+=i;
		currAudioffset += i;		
	//	RTNA_DBG_PrintShort(0, currAudioffset);

		if(need_PES_ADTS)
			need_PES_ADTS = 0;
	}


	//RTNA_DBG_PrintLong(0, targetbufaddr);		
	return targetbufaddr;
}
#endif //#if (MTS_MUXER_EN == 1)
extern volatile MMP_ULONG STC;
int encodeAACfile(void)
{
   // int i;

    /* encode one frame */
    /* a stereo encoder always needs two channel input */

    #if (NOISE_REDUCTION_EN == 1)
	for (i = 0; i < 8; i++) {
		noise_reduction(den, (Word16 *)&gsAACEncInVoiceBuf[AudioAACEncInBufHandle.rd_index + 256*i], 2);
	}
	#endif

    AACEnc_AacEncEncode(
                  (Word16 *)&gsAACEncInVoiceBuf[AudioAACEncInBufHandle.rd_index],
                  (const UWord8 *)ancDataBytes,
                  (Word16 *)&numAncDataBytes,
                  (UWord8 *)bitstreamOut,
                  (Word32 *)&nOutBytes );
	
    AudioAACEncInBufHandle.rd_index+= inSamples;
    AudioAACEncInBufHandle.total_rd += 32;
    if (AudioAACEncInBufHandle.rd_index>= (AACENC_INBUF_SIZE>>1))
        AudioAACEncInBufHandle.rd_index -= (AACENC_INBUF_SIZE>>1);

    /*
      Write one frame of encoded audio to file
    */
//RTNA_DBG_PrintLong(0,(MMP_ULONG)numOutBytes);
    if (nOutBytes) {
      unsigned char *ptr;
	  unsigned int number;
      /* write bitstream to aac file */
      BitStream *bitStream;
      unsigned char   ADTSBuf[20];

      bitStream = OpenBitStream(20, &ADTSBuf[0]);

      /* Write ADTS header (56 bits)*/   
      WriteADTSHeader(inputInfo.sampleRate, inputInfo.nChannels, bitStream, 1, nOutBytes);    
      /* Write encoded AAC data */   		
      #if (MTS_MUXER_EN == 1)
      ptr = (unsigned char *) M2TSPackager((unsigned char *)&ADTSBuf, (MMP_ULONG)MMPF_Video_CurWrPtr(2)+32 ,(char*) bitstreamOut, nOutBytes);
      #else
      RTNA_DBG_Str(0, "#Err : MTS muxer is disabled\r\n");
      ptr = (MMPF_Video_CurWrPtr(2)+32);
      MEMCPY((void*)ptr, (void*)ADTSBuf, 7); ptr += 7;
      MEMCPY((void*)ptr, (void*)bitstreamOut, nOutBytes); ptr += nOutBytes;
      #endif
	  number =(MMP_ULONG) ptr - (MMP_ULONG)MMPF_Video_CurWrPtr(2) - 32;
       glAACDataEncoded += number;        
	   usb_uvc_fill_payload_header(MMPF_Video_CurWrPtr(2),ALIGN32(number),frmCnt,0,STC,0,0,46/*ST_M2TS*/, 0, number);
	   MMPF_Video_UpdateWrPtr(2);
	 //  RTNA_DBG_PrintShort(0, frmCnt);
	 //  RTNA_DBG_Str(0,"AAC ED\r\n");
    frmCnt++;
    }



    return 0;

}


/* size in bytes! */
BitStream *OpenBitStream(int size, unsigned char *buffer)
{
    static BitStream BitBuf;
    BitStream *bitStream;

//    bitStream = malloc(sizeof(BitStream));
    bitStream = (BitStream *)&BitBuf.numBit;
    bitStream->size = size;

    bitStream->numBit = 0;
    bitStream->currentBit = 0;

    bitStream->data = buffer;

    return bitStream;
}

static int WriteADTSHeader(int sampleRate,
                           int channels,
                           BitStream *bitStream,
                           int writeFlag,
                           int numBytes)
{
    int bits = 56;
    int sampleRateIdx;
 //   int i;
  //  unsigned char *ptr;

    if (writeFlag) {
        /* Fixed ADTS header */
        PutBit(bitStream, 0xFFFF, 12); /* 12 bit Syncword */
//        PutBit(bitStream, hEncoder->config.mpegVersion, 1); /* ID == 0 for MPEG4 AAC, 1 for MPEG2 AAC */
        PutBit(bitStream, 0, 1); /* ID == 0 for MPEG4 AAC, 1 for MPEG2 AAC */
        PutBit(bitStream, 0, 2); /* layer == 0 */
        PutBit(bitStream, 1, 1); /* protection absent */
//        PutBit(bitStream, hEncoder->config.aacObjectType - 1, 2); /* profile */
        PutBit(bitStream, 2 - 1, 2); /* profile */
        sampleRateIdx = GetSRIndex(sampleRate);
        PutBit(bitStream, sampleRateIdx, 4); /* sampling rate */
        PutBit(bitStream, 0, 1); /* private bit */
        PutBit(bitStream, channels, 3); /* ch. config (must be > 0) */
                                                     /* simply using numChannels only works for
                                                        6 channels or less, else a channel
                                                        configuration should be written */
        PutBit(bitStream, 0, 1); /* original/copy */
        PutBit(bitStream, 0, 1); /* home */

#if 0 // Removed in corrigendum 14496-3:2002
        if (hEncoder->config.mpegVersion == 0)
            PutBit(bitStream, 0, 2); /* emphasis */
#endif

        /* Variable ADTS header */
        PutBit(bitStream, 0, 1); /* copyr. id. bit */
        PutBit(bitStream, 0, 1); /* copyr. id. start */
//        PutBit(bitStream, hEncoder->usedBytes, 13);
        PutBit(bitStream, numBytes + 56/8, 13);
        PutBit(bitStream, 0x7FF, 11); /* buffer fullness (0x7FF for VBR) */
        PutBit(bitStream, 0, 2); /* raw data blocks (0+1=1) */

    }

    /*
     * MPEG2 says byte_aligment() here, but ADTS always is multiple of 8 bits
     * MPEG4 has no byte_alignment() here
     */
    /*
    if (hEncoder->config.mpegVersion == 1)
        bits += ByteAlign(bitStream, writeFlag);
    */

//    fwrite(bitStream->data, 1, 56/8, outfile);
 /*       for (i = 0; i < 7; i++) {
            ptr = (unsigned char *)(glAudioRecBufStart + ((glAudioRecWritePtr + i) & (glAudioRecBufSize - 1)));
            *ptr = bitStream->data[i];

        }

         glAudioRecWritePtr += 7;
         if (glAudioRecWritePtr >= glAudioRecBufSize) {
            glAudioRecWritePtr -= glAudioRecBufSize;
            glAudioRecWritePtrHigh++;
         }
*/

        glAACDataEncoded += 7;
    bits += numBytes*8;
    return bits;
}

int PutBit(BitStream *bitStream,
           unsigned long data,
           int numBit)
{
    int num,maxNum,curNum;
    unsigned long bits;

    if (numBit == 0)
        return 0;

    /* write bits in packets according to buffer byte boundaries */
    num = 0;
    maxNum = BYTE_NUMBIT - bitStream->currentBit % BYTE_NUMBIT;
    while (num < numBit) {
        curNum = min(numBit-num,maxNum);
        bits = data>>(numBit-num-curNum);
        if (WriteByte(bitStream, bits, curNum)) {
            return 1;
        }
        num += curNum;
        maxNum = BYTE_NUMBIT;
    }

    return 0;
}


int WriteByte(BitStream *bitStream,
              unsigned long data,
              int numBit)
{
    long numUsed,idx;

    idx = (bitStream->currentBit / BYTE_NUMBIT) % bitStream->size;
    numUsed = bitStream->currentBit % BYTE_NUMBIT;

    if (numUsed == 0)
        bitStream->data[idx] = 0;

    bitStream->data[idx] |= (data & ((1<<numBit)-1)) << (BYTE_NUMBIT-numUsed-numBit);
    bitStream->currentBit += numBit;
    bitStream->numBit = bitStream->currentBit;

    return 0;
}

/* Returns the sample rate index */
int GetSRIndex(unsigned int sampleRate)
{
    if (92017 <= sampleRate) return 0;
    if (75132 <= sampleRate) return 1;
    if (55426 <= sampleRate) return 2;
    if (46009 <= sampleRate) return 3;
    if (37566 <= sampleRate) return 4;
    if (27713 <= sampleRate) return 5;
    if (23004 <= sampleRate) return 6;
    if (18783 <= sampleRate) return 7;
    if (13856 <= sampleRate) return 8;
    if (11502 <= sampleRate) return 9;
    if (9391 <= sampleRate) return 10;

    return 11;
}


void    MMPF_InitAACEncoder(void)
{
    glAACDataEncoded = 0;

  /*  AudioAACEncInBufHandle.rd_index=0;
    AudioAACEncInBufHandle.wr_index=0;
    AudioAACEncInBufHandle.total_rd=0;
    AudioAACEncInBufHandle.total_wr=0;
    */
    glAacRecWritePtr = 0;
   // AACFrameSizeQIndex = 0;
    glAudioRecWritePtrHigh = 0;
    glAudioRecWritePtr = 0;
    frmCnt = 0;
	M2tsCounter = 1; // start from 1
    encodeAACfileInit();

}
void    MMPF_EncodeAACFrame(void)
{
    int     result;

    /// Encode all AAC frame when threshold is reached
    while ((AudioAACEncInBufHandle.total_wr-AudioAACEncInBufHandle.total_rd)>=16){
        result = encodeAACfile();
    }
}

unsigned int MMPF_GetAACRecWritePtr(void){
    return glAacRecWritePtr;
}

unsigned int MMPF_GetAACRecFrameCnt(void){
    return frmCnt;
}

void MMPF_SetAACRecParameter(MMP_ULONG bitrate, MMP_ULONG sampleRate){
    glAACEncBitRate = bitrate;
    glAACEncSampleRate = sampleRate;
    return;
}
void MMPF_GetAACRecParameter(MMP_ULONG *bitrate,MMP_ULONG *sampleRate){
    *bitrate = config.bitRate;
    *sampleRate = inputInfo.sampleRate;
    return;
}
void MMPF_SetAACEncMode(unsigned short mode){
#if 0	//dbg msg
	RTNA_DBG_PrintLong(0, glAACEncParameter[mode][1]);
	RTNA_DBG_PrintLong(0, glAACEncParameter[mode][0]);

    MMPF_SetAACRecParameter(glAACEncParameter[mode][1],glAACEncParameter[mode][0]);
#endif
    return;
}

#if 0
//------------------------------------------------------------------------------
//  Function    : MMPF_PostsetAACRecordData
//  Parameter   :
//                filename - File name
//  Return Value :
//                none
//  Description : Postset the AAC Recorded Data 
//------------------------------------------------------------------------------
void MMPF_PostsetAACRecordData(MMP_BYTE *glFileName)
{
	MMP_ERR		status;
	MMP_ULONG   ulFileID;
	MMP_ULONG	ulAudioTailCutSize = 0;
	
	if(glRecordTailCutTime == 0) {
		return;
	}
	
	MMPF_FS_FOpen((MMP_BYTE *)glFileName, "a+", &ulFileID);
	ulAudioTailCutSize = (MMP_ULONG64)(glRecordTailCutTime * (glAACEncBitRate/8000));	
	
	status = MMPF_FS_Truncate(ulFileID, (glAACDataEncoded - ulAudioTailCutSize));
   	if (status) {
		RTNA_DBG_Str0("FileTruncate failed.\r\n");
	}
	
	//To do: Adjust container header
	
	//FClose ??
}
#endif
#endif
