#pragma O2
/*
 WAV play kernel API
*/
#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_audio_ctl.h"
#include "mmpf_i2s_ctl.h"
//#include "mmpf_stream_ctl.h"
#include "mmpf_audio.h"
#include "mmpf_wavdecapi.h"
#if AUDIO_EFFECT_EN==1
#include "eq.h"
#include "MaxLoud.h"
#include "equalizer.h"
#endif
#include "mmpf_audio_typedef.h"
#if (LGE_OAEP_EN == 1)
#include "mmpf_oaep.h"
#endif
#if (SRC_SUPPORT == 1)
#include "mmpf_srcapi.h"
#endif

#if (WAV_P_EN)

WAVFileInfo         glWAVInfo;
extern short        *pOutBuf;
static Word32       eofReached;
static MMP_ULONG    readPtr;
extern short        fSmoothing;

static WavInDataObj wavInData;
static OutDataObj   wavOutData;

static WavInDataObj *pInData;
extern OutDataObj   *pOutData;

extern int          glbitrate;
static MMP_ULONG    glFirstFramePos = 0;
extern MMP_ULONG    glsamplerate;

extern MMP_UBYTE    gbEQType;
extern MMP_UBYTE    gbUseAitDAC;
//define in audio_ctl.c
extern MMP_ULONG	glAudioPlayBufStart;			//start addr of buf	
extern MMP_ULONG	glAudioPlayBufSize;				//buf size (should be 2 ^ N)
extern MMP_ULONG	glAudioPlayReadPtr;				//index		
extern MMP_ULONG	glAudioPlayReadPtrHigh;			//read loop counter
extern MMP_ULONG    glAudioPlayWritePtr;
extern MMP_ULONG    glAudioPlayWritePtrHigh;
extern MMP_ULONG	glAudioTotalSize;
extern MMP_USHORT   gsSmoothDir;
extern MMP_USHORT   gsVolUp;
extern MMP_USHORT   gsVolUpLimit;
extern short        gsVolDown;

extern MMP_SHORT    VOL_TABLE[40];
extern MMP_ULONG    glAudioStartPos;
extern MMP_ULONG    glDecFrameInByte;
extern MMP_USHORT   gsAudioStatus;
extern MMP_USHORT	gsAudioVolume, gsAudioTargetVolume;
extern MMP_BOOL		gbAudioVolumeSet;

extern MMP_ULONG    glAudioPlayWaitBufFlush;
extern MMP_USHORT   gsSurrounding;
#if AUDIO_EFFECT_EN==1
extern MMP_LONG     *gpEqFilter;
extern EQStruct     sEQ, sEQ2;
extern MMP_BOOL     gbGraphicEQEnable;
#endif
#if (LGE_OAEP_EN == 1)
extern MMP_BOOL     gbOAEPEnable;
#endif
extern MMP_BOOL     gbAudioDecodeDone;

#if (SRC_SUPPORT == 1)
static MMP_BOOL     m_bWavSRCEnable;
extern short        gsSRCOutbuf[SRC_OUTBUF_SIZE];
extern int          glSRCWorkingBuf[SRC_SIZEOF_WORKING_BUF>>2];
extern Resample_Para_t	SRC_Info;
#endif

int InitWAVDecoder(int decoder);
extern MMP_USHORT   MMPF_SetAudioPlayReadPtr(void);

extern Surround_HP(short *,short *, int);
extern Surround_SPK(short *,short *, int);

#if (WAV_P_EN)
Word16              *pWavOutBuf;
Word16              *pWavInBuf;
extern  MMP_ULONG   glAudioWorkingBufferEnd;
#if (SRC_SUPPORT == 1)
static short        *m_sSRCInBuf;
#endif
#endif

extern MMPF_OS_FLAGID       SYS_Flag_Hif;
extern MMPF_AUDIO_DATAPATH  audioplaypath;

static int FillReadBuffer(MMP_UBYTE *readBuf, MMP_UBYTE *readPtr, int bufSize, int bytesLeft, MMP_ULONG inaddr)
{
	int         nRead = 0;
	int         i = 0, preind;
	MMP_UBYTE   *ptab = (MMP_UBYTE *)(glAudioPlayBufStart);

#if (AUDIO_STREAMING_EN == 1)    	
	MMP_ULONG streamReadPtr, streamWritePtr, streamReadPtrHigh, streamWritePtrHigh;
	MMPF_Audio_GetStreamingPtr(&streamReadPtr, &streamReadPtrHigh, &streamWritePtr, &streamWritePtrHigh);
	preind = streamReadPtr;

    while (i < bytesLeft) {
		*readBuf++ = *readPtr++;
        i++;
	}

	nRead = WAV_FILLUNIT;
	for (i = 0; i < WAV_FILLUNIT; i++) {
		readBuf[i] = ptab[streamReadPtr++];
		streamReadPtr = streamReadPtr & (glAudioPlayBufSize-1);
	}
	//allen 0318 add
	if(nRead < (bufSize - bytesLeft))
		for(i=0; i<bufSize-bytesLeft-nRead; i++)
			readBuf[bytesLeft+nRead+i] = 0;
	
	if (streamReadPtr < preind)
		streamReadPtrHigh++;
	MMPF_Audio_SetStreamingReadPtr(streamReadPtr, streamReadPtrHigh);
#else
	preind = glAudioPlayReadPtr;

    while (i < bytesLeft) {
		*readBuf++ = *readPtr++;
        i++;
	}

	nRead = WAV_FILLUNIT;
	for (i = 0; i < WAV_FILLUNIT; i++) {
		readBuf[i] = ptab[glAudioPlayReadPtr++];
		glAudioPlayReadPtr = glAudioPlayReadPtr & (glAudioPlayBufSize-1);
	}
	//allen 0318 add
	if(nRead < (bufSize - bytesLeft))
		for(i=0; i<bufSize-bytesLeft-nRead; i++)
			readBuf[bytesLeft+nRead+i] = 0;

	if (glAudioPlayReadPtr < preind)
		glAudioPlayReadPtrHigh++;
#endif	
	MMPF_SetAudioPlayReadPtr();
/*
	if (gbAudioPlayPath) {						//card mode only
		#if (ENABLE_AUDIO_STREAMING == 1) 
		MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_AUDIOREAD_FILE, MMPF_OS_FLAG_SET);
		#else
        MMPF_TransferAudioDataFromCard();
		#endif
	}
	else
	{
    	MMPF_FillBufInterruptToHost();
	}
*/	
	return nRead;
}



Word32 WavInbufTask(void)
{
	int nRead;
	
	if(pInData->SampleLeft < WAV_FILLUNIT && !eofReached) {		//2*2048
		nRead = FillReadBuffer((MMP_UBYTE *)pWavInBuf, (MMP_UBYTE *)readPtr, WAV_READBUF_SIZE, pInData->SampleLeft, 0);
		pInData->SampleLeft += nRead;
		readPtr = (MMP_ULONG)pWavInBuf;
		if (nRead == 0)
			eofReached = 1;
	}

	return eofReached;
}

int WavUnderFlowProtect(void)
{
    if(pInData->SampleLeft < WAV_FILLUNIT && !eofReached) {		
        MMPF_UpdateAudioPlayWritePtr();

		if((glAudioPlayWritePtr==glAudioPlayReadPtr)&&(glAudioPlayWritePtrHigh==glAudioPlayReadPtrHigh)){
	        return 1;
	    }else if((glAudioPlayWritePtr>glAudioPlayReadPtr)&&((glAudioPlayWritePtr-glAudioPlayReadPtr)<WAV_FILLUNIT)){
            return 1;
        }else if((glAudioPlayWritePtr<glAudioPlayReadPtr)&&((glAudioPlayWritePtr+(glAudioPlayBufSize-glAudioPlayReadPtr))<WAV_FILLUNIT)){
            return 1; 
	    }
    }
	return 0;
}


Word32 WavStart(short *outBuf, int *samples)
{
    int     outOfData = 0;
    int     i;
    int     datalength;
    int     err;
    #if (SRC_SUPPORT == 1)
    int     srcOutSize;
    #endif
    short   *pcmOutBuf = outBuf;

    //Memory-mode under flow protect,hans add
    if (audioplaypath == MMPF_AUDIO_DATAPATH_MEM) {
        if((glDecFrameInByte + WAV_READBUF_SIZE) < glAudioTotalSize){
            if(WavUnderFlowProtect()) {
                *samples = WAV_READ_SAMPLE;
                for(i = 0; i < WAV_READ_SAMPLE; i++){
                    outBuf[i] = 0;
                }
                RTNA_DBG_Str(0,"mem mode underflow\r\n");
                return 0;
            }
        }
    }

    err = WavInbufTask();

    if (err == 1) {
        outOfData = 1 ;
        return outOfData;
    }

    datalength = WAV_READ_SAMPLE;
    *samples = WAV_READ_SAMPLE;

    #if (SRC_SUPPORT == 1)
    if (m_bWavSRCEnable) {
        #if (SBC_SUPPORT == 1)
        if (!gbSBCEncodeEnable) {
        #endif
            pcmOutBuf = &m_sSRCInBuf[0];
        #if (SBC_SUPPORT == 1)
        }
        #endif
    }
    #endif
    if (glWAVInfo.bitsPerSample == 8) {
        for(i = 0; i < datalength; i++) {
            pcmOutBuf[i] = (short)((*(MMP_UBYTE *)(readPtr + i) - 0x80) << 8);
        }
    }
    else {
        for(i = 0; i < datalength; i++) {
            pcmOutBuf[i] = *((short *)(readPtr + (i << 1)));
        }
    }
    #if (SRC_SUPPORT == 1)
    if (m_bWavSRCEnable) {
        #if (SBC_SUPPORT == 1)
        if (!gbSBCEncodeEnable) {
        #endif
            srcOutSize = SRC_FrameResample(pcmOutBuf, outBuf, WAV_READ_SAMPLE, SRC_OUTBUF_SIZE, glWAVInfo.nChannels);

            if (srcOutSize > (pOutData->BufButton - pOutData->SampleLeft)) {
                RTNA_DBG_Str0("No enough free space of output buffer\r\n");
                *samples = 0;
                return 0;
            }
            *samples = srcOutSize * glWAVInfo.nChannels;
        #if (SBC_SUPPORT == 1)
        }
        #endif
    }
    #endif

    if (glWAVInfo.bitsPerSample == 16)
        datalength = datalength << 1;

    readPtr += datalength;
    glDecFrameInByte += datalength;
    pInData->SampleLeft -= datalength;

    if (glDecFrameInByte >= glAudioTotalSize) {
        outOfData = 1;
        RTNA_DBG_PrintLong(0, pOutData->SampleLeft);
    }

	return outOfData;
}

void MMPF_StartWAVDec(void)
{
    MMP_LONG lframeLength = WAV_READ_SAMPLE;

    #if (SRC_SUPPORT == 1)
    MMP_LONG lWorkBufSize;
    #endif
    fSmoothing = 5;

    InitWAVDecoder(WAVE);

    #if (SRC_SUPPORT == 1)
    #if (SBC_SUPPORT == 1)
    if(!gbSBCEncodeEnable) {
    #endif
        if (glWAVInfo.sampleRate != glWAVInfo.newSampleRate) {
            m_bWavSRCEnable = MMP_TRUE;
            lWorkBufSize = SRC_SetWorkingBuf(glSRCWorkingBuf);
    		if (lWorkBufSize > sizeof(glSRCWorkingBuf)){
    			RTNA_DBG_Str(0, "Error SRC_SetWorkingBuf\r\n");
    			return;
    		}
    		SRC_Config(&SRC_Info, glWAVInfo.sampleRate, glWAVInfo.newSampleRate);
            lframeLength = (WAV_READ_SAMPLE * glWAVInfo.newSampleRate)/glWAVInfo.sampleRate;
        }
    #if (SBC_SUPPORT == 1)
    }
    #endif
    #endif //(SRC_SUPPORT == 1)

    #if (LGE_OAEP_EN == 1)
    if (glWAVInfo.nChannels == 1) 
	    MMPF_OAEP_Iniitalize(lframeLength, glWAVInfo.newSampleRate);
	else
		MMPF_OAEP_Iniitalize(lframeLength >> 1, glWAVInfo.newSampleRate);
    #endif

    #if (SBC_SUPPORT == 1)
    if (!gbSBCEncodeEnable)
    {
    #endif
        if (gbUseAitDAC != AUDIO_OUT_I2S) {
            MMPF_InitAudioSetting(AFE_OUT, glWAVInfo.newSampleRate); //initial iis registers
            MMPF_Audio_InitializePlayFIFO(AFE_OUT, WAV_I2S_FIFO_WRITE_THRESHOLD);
            MMPF_Audio_SetMux(AFE_OUT, 1);
        }
        else {
            MMPF_InitAudioSetting(I2S_OUT, glWAVInfo.newSampleRate); //initial iis registers
            MMPF_Audio_InitializePlayFIFO(I2S_OUT, WAV_I2S_FIFO_WRITE_THRESHOLD);
            MMPF_Audio_SetMux(I2S_OUT, 1);
        }
    #if (SBC_SUPPORT == 1)
    }
    #endif

    RTNA_DBG_PrintLong(0, glsamplerate);

    //alterman@0324: Some file total size is not match to data size plus header size
    glAudioTotalSize = glFirstFramePos + glWAVInfo.totaldataSize;
    if (glAudioStartPos > glAudioTotalSize) {
        glAudioTotalSize = 0;
    }
    else {
        glAudioTotalSize -= glAudioStartPos;
    }

    gsSmoothDir = AUDIO_SMOOTH_UP;
    gsVolUp = 0;
    gsVolUpLimit = VOL_TABLE[gsAudioVolume];
}

void MMPF_StopWAVDec(void)
{
    gsSmoothDir = AUDIO_SMOOTH_DOWN;
    gsVolDown = VOL_TABLE[gsAudioVolume];
}

void MMPF_PauseWAVDec(void)
{
    gsSmoothDir = AUDIO_SMOOTH_DOWN;
    gsVolDown = VOL_TABLE[gsAudioVolume];
}

void MMPF_ResumeWAVDec(void)
{
	if (gbUseAitDAC != AUDIO_OUT_I2S) {
        MMPF_InitAudioSetting(AFE_OUT, glsamplerate);	
		MMPF_Audio_InitializePlayFIFO(AFE_OUT, WAV_I2S_FIFO_WRITE_THRESHOLD);		//allen 0407 add
		MMPF_Audio_SetMux(AFE_OUT, 1);
	}
	else{
        MMPF_InitAudioSetting(I2S_OUT, glsamplerate);	
		MMPF_Audio_InitializePlayFIFO(I2S_OUT, WAV_I2S_FIFO_WRITE_THRESHOLD);		//allen 0407 add
		MMPF_Audio_SetMux(I2S_OUT, 1);
    }		
    gsSmoothDir = AUDIO_SMOOTH_UP;
	gsVolUp = 0;
	gsVolUpLimit = VOL_TABLE[gsAudioVolume];	
}

MMP_ULONG MMPF_GetWAVDecFramesInByte(void)
{
	if((glDecFrameInByte + glAudioStartPos) >= glFirstFramePos)	{
	    //VAR_L(3,glDecFrameInByte + glAudioStartPos - glFirstFramePos);
		return glDecFrameInByte + glAudioStartPos - glFirstFramePos;
    }
	else {
	    //VAR_L(3,glDecFrameInByte + glAudioStartPos - glFirstFramePos);
		return 0;
    }
}
static void SmoothFilter(short *pBuf, MMP_USHORT len, MMP_USHORT chs)
{
	MMP_USHORT		i;	
	
	if(chs == 2) {
		for(i = 0; i < len; i += 2) {
			pBuf[i] = 0;
			pBuf[i+1] = 0;
		}
	}
    else {
		for(i = 0; i < len; i++) {
			pBuf[i] = 0;
		}
	}
}

int InitWAVDecoder(int decoder)
{
    #if (WAV_P_EN)
    pWavInBuf = (Word16 *)(&(glAudioDecWorkingBuf[0]));
    pWavOutBuf = (Word16 *)(&(glAudioDecWorkingBuf[WAV_OUTFRAMESIZE * WAV_BUFDEPTH]));
    #if (SRC_SUPPORT == 1)
    m_sSRCInBuf = (Word16 *)(&(glAudioDecWorkingBuf[(WAV_OUTFRAMESIZE * WAV_BUFDEPTH) << 1]));
    glAudioWorkingBufferEnd = ((WAV_OUTFRAMESIZE * WAV_BUFDEPTH) << 3) + (WAV_READ_SAMPLE << 1);
    #else
    glAudioWorkingBufferEnd = WAV_OUTFRAMESIZE * WAV_BUFDEPTH * 4;
    #endif

    #if (LGE_OAEP_EN == 1)
    RTNA_DBG_Str(0, "Working buffer need : ");
    RTNA_DBG_Long(0, glAudioWorkingBufferEnd + 0x888 + 0x6720);
    RTNA_DBG_Str(0, "\r\n");
    #else
    RTNA_DBG_Str(0, "Working buffer need : ");
    RTNA_DBG_Long(0, glAudioWorkingBufferEnd);
    RTNA_DBG_Str(0, "\r\n");
    #endif
    #endif

    #if (WAV_P_EN)
    RTNA_DBG_Str(0, "Working buffer available : ");
    #if (AUDIO_DEC_ENC_SHARE_WB == 1)
    #if (AUDIO_ENC_SIZEOF_TOTAL_WORKING_BUF > AUDIO_DEC_SIZEOF_TOTAL_WORKING_BUF)
    RTNA_DBG_Long(0, AUDIO_ENC_SIZEOF_TOTAL_WORKING_BUF);
    #else
    RTNA_DBG_Long(0, AUDIO_DEC_SIZEOF_TOTAL_WORKING_BUF);
    #endif
    #else
    RTNA_DBG_Long(0, AUDIO_DEC_SIZEOF_TOTAL_WORKING_BUF);
    #endif
    RTNA_DBG_Str(0, "\r\n");
    #endif

    // WAV decoder initialization
    if (decoder == WAVE)
    {
        MMP_ULONG	i;

        RTNA_DBG_Str(0,"\r\n====== WAV firmware ver.0.1 ======\r\n");
        RTNA_DBG_Str(0,"====== release: 98/02/01 ======\r\n");

        pOutBuf  = pWavOutBuf;
        readPtr  = (MMP_ULONG)pWavInBuf;
        pOutData = &wavOutData;
        pInData  = &wavInData;

        pInData->rd_ind         = 0;
        pInData->wr_ind         = 0;
        pInData->SampleLeft     = 0;
        pInData->BufButton      = WAV_OUTFRAMESIZE * WAV_BUFDEPTH * 2;

        pOutData->rd_ind        = 0;
        pOutData->wr_ind        = 0;
        #if (SRC_SUPPORT == 1)
        pOutData->SampleLeft    = WAV_OUTFRAMESIZE * WAV_BUFDEPTH * 2;
        pOutData->BufButton     = WAV_OUTFRAMESIZE * WAV_BUFDEPTH * 2;
        #else
        pOutData->SampleLeft    = WAV_OUTFRAMESIZE * WAV_BUFDEPTH;
        pOutData->BufButton     = WAV_OUTFRAMESIZE * WAV_BUFDEPTH;
        #endif

        glAudioPlayWaitBufFlush = 0;
        glDecFrameInByte        = 0;
        eofReached              = 0;

        for(i = 0; i < pOutData->BufButton; i++){
            pOutBuf[i] = 0;
        }

        if (audioplaypath == MMPF_AUDIO_DATAPATH_MEM) {
            MMPF_UpdateAudioPlayReadPtr();	// rftorng modify for 512 sync read file system in the host side
        }    			
        else {
            glAudioPlayReadPtr = 0;	
            glAudioPlayReadPtrHigh = 0;	
        }

        #if (SRC_SUPPORT == 1)
        m_bWavSRCEnable = MMP_FALSE;
        #endif

        return 0;
	}

    return -1;
}

void MMPF_DecodeWAVFrame(int decoder)
{
	// WAV start

	if (decoder == WAVE)
	{
		MMP_ULONG 	    ind;
		int   	        outOfData = 0, i, j;
		extern	short	fSmoothing;
		short           *p_src, *p_des;
		int             outputSamps = 0;
        short           *outBuf;

        gbAudioDecodeDone = MMP_FALSE;

		ind = pOutData->wr_ind;
        outBuf = &pOutBuf[ind];

        #if (SRC_SUPPORT == 1)
        if (m_bWavSRCEnable) {
            #if (SBC_SUPPORT == 1)
            if (!gbSBCEncodeEnable) {
            #endif
                outBuf = gsSRCOutbuf;
            #if (SBC_SUPPORT == 1)
            }
            #endif
        }
        #endif

		if (glAudioPlayWaitBufFlush){
            if (pOutData->SampleLeft <= WAV_I2S_FIFO_WRITE_THRESHOLD) {
                MMPF_PostPlayAudioOp();
        	    RTNA_DBG_Str(0,"WAVDEC_STOP\r\n");
            }
            else {
    	        #if (SBC_SUPPORT == 1)
    	        if (gbSBCEncodeEnable) {
    	            MMPF_PostPlayAudioOp();
    	        }
    	        #endif
    	        RTNA_DBG_Str0("MMPF_DecodeWAVFrame\r\n");
    	        RTNA_DBG_PrintLong(0, pOutData->SampleLeft);
    	    }
    	    gbAudioDecodeDone = MMP_TRUE;
			return;
	    }
		outOfData = WavStart(outBuf, &outputSamps);
        if (!outOfData && (outputSamps == 0)) {
            DBG_S0("outputSamps == 0\r\n");
            return;
        }

        if (glWAVInfo.nChannels == 1) {
		    p_des = outBuf + (outputSamps << 1) - 1;
		    p_src = outBuf + outputSamps - 1; 
		    for(i = 0; i < outputSamps; i++) {
	            *p_des--= *p_src;
	            *p_des--= *p_src--;
	        } 
	        outputSamps = outputSamps << 1;
	    }

	    #if (SBC_SUPPORT == 1)
        if (gbSBCEncodeEnable) {
            sEQ.enable = 0;
	    }
        #endif

        #if AUDIO_EFFECT_EN==1
		// EQ
        // 10-band EQ
        if (gbGraphicEQEnable && gpEqFilter) {
		    MMPF_AUDIO_Graphic_EQ(outBuf, outBuf, gpEqFilter, outputSamps >> 1);
        }
        
        #endif
        
        
        #if (LGE_OAEP_EN == 1)
        else if (gbOAEPEnable == MMP_FALSE) {
        #endif
        #if AUDIO_EFFECT_EN==1
            // Pre-defined EQ
            if (sEQ.enable == 1) {
                if (gbEQType == LOUDNESS) {
                    MaxLoudness(outBuf, outBuf, outputSamps >> 1, 2, 0x7FFF);
                }
                else {
                    EQFilter2s(&sEQ, outBuf, outputSamps >> 1);
                    EQFilter2s(&sEQ2, outBuf + 1, outputSamps >> 1);
                }
            }
            else {
                if(gsSurrounding == 1) {
                    Surround_HP(outBuf, outBuf, outputSamps >> 1);
                }
                else if(gsSurrounding == 2) {
                    Surround_SPK(outBuf, outBuf, outputSamps >> 1);
                }
            }
        #endif
                    
        #if (LGE_OAEP_EN == 1)
        }
        #endif

        if (gbAudioVolumeSet == MMP_TRUE) {
            for (i = 0; i < outputSamps; i+=128) {
                if (outputSamps - i >= 128) {	// more than 128 LR samples for smooth operation
					if (gsAudioTargetVolume > gsAudioVolume) {	// volume up
						MMP_SHORT startvolume, endvolume;	
						MMP_SHORT diff;
                        #if AUDIO_EFFECT_EN==1
						if (sEQ.enable != 1 && gsSurrounding == 2) 
						{
							startvolume = (int)(1.2f * VOL_TABLE[gsAudioVolume]);
							endvolume = (int)(1.2f * VOL_TABLE[gsAudioVolume+1]);
							if (startvolume > 32767L)
			                    startvolume = 32767L;
							if (endvolume > 32767L)
			                    endvolume = 32767L;
						}
						else 
						#endif
						{
							startvolume = VOL_TABLE[gsAudioVolume];
							endvolume = VOL_TABLE[gsAudioVolume + 1];
						}
						diff = (endvolume - startvolume)/64;

						for (j = i; j < (i + 128); j+=2) {
                            outBuf[j] = (short)(((int)outBuf[j] * startvolume) >> 15);
							outBuf[j+1] = (short)(((int)outBuf[+1] * startvolume) >> 15);
							startvolume += diff;
						}
						gsAudioVolume++;
					}
					else if (gsAudioTargetVolume < gsAudioVolume) {	// volume down
						MMP_SHORT startvolume, endvolume;	
						MMP_SHORT diff;
                        #if AUDIO_EFFECT_EN==1
						if (sEQ.enable != 1 && gsSurrounding == 2) {
							startvolume = (int)(1.2f * VOL_TABLE[gsAudioVolume]);
							endvolume = (int)(1.2f * VOL_TABLE[gsAudioVolume - 1]);
							if (startvolume > 32767L)
			                    startvolume = 32767L;
							if (endvolume > 32767L)
			                    endvolume = 32767L;
						}			
						else 
						#endif
						
						{
							startvolume = VOL_TABLE[gsAudioVolume];
							endvolume = VOL_TABLE[gsAudioVolume - 1];
						}								
						diff = (startvolume - endvolume)/64;

						for (j = i; j < (i + 128); j+=2) {
                            outBuf[j] = (short)(((int)outBuf[j] * startvolume) >> 15);
							outBuf[j+1] = (short)(((int)outBuf[j+1] * startvolume) >> 15);
							startvolume -= diff;
						}
						gsAudioVolume--;
					}
					else {
						for (j = i; j < (i + 128); j++) {
                            outBuf[j] = (short)(((int)outBuf[j] * VOL_TABLE[gsAudioVolume]) >> 15);
						}
					}
                }
                else {
					for (j = i; j < outputSamps; j++) {
                        outBuf[j] = (short)(((int)outBuf[j] * VOL_TABLE[gsAudioVolume]) >> 15);
					}
				}
            }
			if (gsAudioVolume == gsAudioTargetVolume) {
				gbAudioVolumeSet = MMP_FALSE;
			}
        }
        else {
			for (i = 0; i < outputSamps; i++) {
                outBuf[i] = (short)(((int)outBuf[i] * VOL_TABLE[gsAudioVolume]) >> 15);
			}
        }

        //smoothing
        if (glAudioTotalSize > 0x8000) {//hans add, no smooth for small file
            if(fSmoothing != 0) {
                SmoothFilter(outBuf, outputSamps, 2);
		    }
	    }
		fSmoothing --;
        if (fSmoothing < 0)
            fSmoothing = 0;

        #if (SBC_SUPPORT == 1)
        if (gbSBCEncodeEnable && (outOfData == 0)) {
            MMPF_SBCEncodeAudioFrame(outBuf, outputSamps);
	    }
        #endif

		if(outOfData == 1) {
			if(audioplaypath == MMPF_AUDIO_DATAPATH_CARD) {
			    MMP_ULONG size;
			    MMPF_GetAudioPlayFileSize(&size);
			    glDecFrameInByte = size;
				glAudioStartPos = 0;
				glFirstFramePos = 0;
			}
			glAudioPlayWaitBufFlush = 1;

		    RTNA_DBG_Str(0,"EOF\r\n");
		}
		else {
			#if (LGE_OAEP_EN == 1)
            MMPF_OAEP_Process(outBuf);
			#endif

            #if (SRC_SUPPORT == 1)
            if (m_bWavSRCEnable) {
                #if (SBC_SUPPORT == 1)
                if (!gbSBCEncodeEnable) {
                #endif
                    p_des = &pOutBuf[ind];
                    if (outputSamps <= (pOutData->BufButton - pOutData->wr_ind)) {
                        for(i = 0; i < outputSamps; i++)
                            *p_des++ = *outBuf++;
                    }
                    else {
                        for(i = 0; i < (pOutData->BufButton - pOutData->wr_ind); i++)
                            *p_des++ = *outBuf++;
                        p_des = pOutBuf;
                        for(; i < outputSamps; i++)
                            *p_des++ = *outBuf++;
                    }
                #if (SBC_SUPPORT == 1)
                }
                #endif
            }
            #endif

            ind += outputSamps;

			if (ind >= pOutData->BufButton)
                ind -= pOutData->BufButton;
    		pOutData->wr_ind = ind;
    		pOutData->SampleLeft += outputSamps;
        }
    }
    gbAudioDecodeDone = MMP_TRUE;

    return;
}
//============================================================================
// MMPF_ExtractWAVInfo()
//========================================================================
#define     STR_RIFF        0x46464952
#define     STR_WAVE        0x45564157
#define     STR_fmt         0x20746d66
#define     STR_data        0x61746164

#define     GetWaveDataShort(ptr)   (*((short *)(ptr)))
#define     GetWaveDataInt(ptr)     ((GetWaveDataShort(ptr) & 0xffff) | (GetWaveDataShort(ptr+2) << 16))


MMP_SHORT MMPF_ExtractWAVInfo(WAVFileInfo *wavinfo)
{
    MMP_ULONG   chunkdatasize;
	MMP_LONG    nChunks = 0;
	MMP_LONG    offset = 0;
	MMP_LONG    err;
	MMP_USHORT  blockAlign;
    
    if (InitWAVDecoder(WAVE) != 0)
        return -1;

    glFirstFramePos = 0;

    err = WavInbufTask();
	if (err == 1) {
		RTNA_DBG_Str0("WAV: out of data\r\n");
        return -1;
	}

	if (readPtr & 0x03) {
		RTNA_DBG_Str0("readPtr is not 4-byte alignment\r\n");
        return -1;
	}

	// Check RIFF structure.
	if(GetWaveDataInt(readPtr) != STR_RIFF) {
		RTNA_DBG_Str0("Not RIFF structure\r\n");
        return -1;
	}
	readPtr += 4;
	offset += 4;

	// Skip file Size field.
	readPtr += 4;
	offset += 4;

	// Check RIFF structure.
	if(GetWaveDataInt(readPtr) != STR_WAVE) {
		RTNA_DBG_Str0("Not WAVE format\r\n");
        return -1;
	}
	readPtr += 4;
	offset += 4;

	// Check fmt and data chunk.
	while((GetWaveDataInt(readPtr) != STR_data) && (nChunks < 16)) {
		if (GetWaveDataInt(readPtr) == STR_fmt) {
			wavinfo->compressionCode = GetWaveDataShort(readPtr + 8);
			wavinfo->nChannels = GetWaveDataShort(readPtr + 10);
			wavinfo->sampleRate = GetWaveDataInt(readPtr + 12);
			wavinfo->bitRate = GetWaveDataInt(readPtr + 16);
			wavinfo->bitRate = wavinfo->bitRate << 3;

			blockAlign = GetWaveDataShort(readPtr + 20);
			wavinfo->bitsPerSample = GetWaveDataShort(readPtr + 22);
		}
		chunkdatasize = GetWaveDataInt(readPtr + 4);
		RTNA_DBG_PrintLong(0, chunkdatasize);

		readPtr = readPtr + 8 + chunkdatasize;
		offset = offset + 8 + chunkdatasize;
		// Make sure memory which readPtr points to is accessable
		if ((offset + 8) > WAV_FILLUNIT) {
		    return -1;
		}
		nChunks++;
	}

	if(nChunks >= 16) {
		RTNA_DBG_Str0("nChunks >= 16\r\n");
		return -1;
	}
    else if (wavinfo->compressionCode != UNCOMPRESSED_PCM) {
        wavinfo->bitRate = 0;   // means not support wave format
        RTNA_DBG_Str0("Unsupport wave compressed code\r\n");
        return -1;
    }

	wavinfo->totaldataSize = GetWaveDataInt(readPtr + 4);
	RTNA_DBG_PrintLong(0, wavinfo->totaldataSize);
	readPtr += 8;
	offset += 8;
	// readPtr points to wave raw data now.
	glFirstFramePos = offset;

    wavinfo->newSampleRate = wavinfo->sampleRate;

    #if (SRC_SUPPORT == 1)
    switch(glWAVInfo.sampleRate) {
    case SRATE_8000:
    case SRATE_11025:
    case SRATE_12000:
    case SRATE_16000:
    case SRATE_22050:
    case SRATE_24000:
    case SRATE_32000:
    case SRATE_44100:
    case SRATE_48000:
        wavinfo->newSampleRate = glWAVInfo.sampleRate;
        break;
    default:
        if (glWAVInfo.sampleRate < SRATE_8000)
            wavinfo->newSampleRate = SRATE_8000;
        else if (glWAVInfo.sampleRate < SRATE_16000)
            wavinfo->newSampleRate = SRATE_16000;
        else if (glWAVInfo.sampleRate < SRATE_32000)
            wavinfo->newSampleRate = SRATE_32000;
        else if (glWAVInfo.sampleRate < SRATE_48000)
            wavinfo->newSampleRate = SRATE_48000;
        else
            wavinfo->newSampleRate = glWAVInfo.sampleRate;
        break;
    }
    #endif //(SRC_SUPPORT == 1)

#if 1
    RTNA_DBG_PrintShort(0, wavinfo->compressionCode);
    RTNA_DBG_PrintShort(0, wavinfo->nChannels);
	RTNA_DBG_PrintLong(0, wavinfo->sampleRate);
    RTNA_DBG_PrintLong(0, wavinfo->newSampleRate);
	RTNA_DBG_PrintLong(0, wavinfo->bitRate);
	RTNA_DBG_PrintShort(0, blockAlign);
	RTNA_DBG_PrintShort(0, wavinfo->bitsPerSample);
#endif

    #if (SBC_SUPPORT == 1)
    m_CodecSampleRate = wavinfo->sampleRate;
    #endif

   	return 0;
}

#endif

