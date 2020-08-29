/// @ait_only
//==============================================================================
//
//  File        : mmpf_mts.h
//  Description : Header mpeg transport stream
//  Author      : 
//  Revision    : 1.0
//
//==============================================================================
#include "mmpf_mts.h"

#if (MTS_MUXER_EN == 1)
#if 0 //titany
MMP_UBYTE   MtsPmtVidEsInfoDesc[] = {0x28,0x04,0x64,0xE0,0x28,0xBF};
MMP_UBYTE   MtsPmtProgramInfo[] = {0x88,0x04,0x0F,0xFF,0x84,0xFC,0x05,0x04,0x48,0x44,0x4D,0x56};
#else //tsMuxer
MMP_UBYTE   MtsPmtVidEsInfoDesc[] = {0x28,0x04,0x64,0x00,0x28,0xBF};
MMP_UBYTE   MtsPmtProgramInfo[] = {0x05,0x04,0x48,0x44,0x4D,0x56,0x88,0x04,0x0F,0xFF,0xFC,0xFC};
#endif
MMP_UBYTE   MtsPmtPcmEsInfoDesc[] = {0x05,0x08,0x48,0x44,0x4D,0x56,0x00,0x80,0x31,0x40};

//SIT
MMP_UBYTE   MtsSitInfo[] = {0x00,0x7F,0xF0,0x19,0xFF,0xFF,0xC1,0x00,0x00,0xF0,0x0A,0x63,0x08,0xC1,0x5A,0xAE,
                             0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x01,0x80,0x00,0x34,0x1E,0xE7,0x4E};

MMP_UBYTE   MtsPesVidHeader[] = {0x00,0x00,0x01,0xE0,0x00,0x00,0x84,0xC0,0x0A,0x31,0x0C,0xDF,0xF3,0x01,0x11,0x0C,
                                0xDF,0xD5,0xAD};
MMP_UBYTE   MtsPesAudHeader[] = {0x00,0x00,0x01,0xC0,0x02,0x0F,0x84,0x80,0x05,0x21,0x0C,0xDF,0xF3,0x01};

/** @addtogroup MMPF_MTS
 *  @{
*/

void MMPF_CalcCrc32(MMP_UBYTE *pSrc, MMP_ULONG n, MMP_UBYTE *pData)
{
    MMP_UBYTE   txcrc[32];
    MMP_UBYTE   nxtxcrc[32];
    MMP_UBYTE   txdin, crcshin;
    MMP_ULONG   txd;
    MMP_UBYTE   *txdout = (MMP_UBYTE*)&txd;

    MMP_ULONG   bcnt;   // bit cnt
    MMP_ULONG   i;
    MMP_ULONG   offset;

    MEMSET(txcrc, 1, 32);
    txdout[3] = (txcrc[31]<<7) + (txcrc[30]<<6) + (txcrc[29]<<5) + (txcrc[28]<<4)
        + (txcrc[27]<<3) + (txcrc[26]<<2) + (txcrc[25]<<1) + (txcrc[24]<<0);
    txdout[2] = (txcrc[23]<<7) + (txcrc[22]<<6) + (txcrc[21]<<5) + (txcrc[20]<<4)
        + (txcrc[19]<<3) + (txcrc[18]<<2) + (txcrc[17]<<1) + (txcrc[16]<<0);
    txdout[1] = (txcrc[15]<<7) + (txcrc[14]<<6) + (txcrc[13]<<5) + (txcrc[12]<<4)
        + (txcrc[11]<<3) + (txcrc[10]<<2) + (txcrc[ 9]<<1) + (txcrc[ 8]<<0);
    txdout[0] = (txcrc[ 7]<<7) + (txcrc[ 6]<<6) + (txcrc[ 5]<<5) + (txcrc[ 4]<<4)
        + (txcrc[ 3]<<3) + (txcrc[ 2]<<2) + (txcrc[ 1]<<1) + (txcrc[ 0]<<0);

    for (offset = 0; offset < n; offset++) {
        txdin = pSrc[offset];

        for (bcnt=0;bcnt<8;bcnt++) {
            crcshin = txcrc[31] ^ ((txdin >> (7-bcnt)) & 0x01);

            nxtxcrc[31] = txcrc[30];
            nxtxcrc[30] = txcrc[29];
            nxtxcrc[29] = txcrc[28];
            nxtxcrc[28] = txcrc[27];
            nxtxcrc[27] = txcrc[26];
            nxtxcrc[26] = txcrc[25] ^ crcshin;
            nxtxcrc[25] = txcrc[24];
            nxtxcrc[24] = txcrc[23];
            nxtxcrc[23] = txcrc[22] ^ crcshin;
            nxtxcrc[22] = txcrc[21] ^ crcshin;
            nxtxcrc[21] = txcrc[20];
            nxtxcrc[20] = txcrc[19];
            nxtxcrc[19] = txcrc[18];
            nxtxcrc[18] = txcrc[17];
            nxtxcrc[17] = txcrc[16];
            nxtxcrc[16] = txcrc[15] ^ crcshin;
            nxtxcrc[15] = txcrc[14];
            nxtxcrc[14] = txcrc[13];
            nxtxcrc[13] = txcrc[12];
            nxtxcrc[12] = txcrc[11] ^ crcshin;
            nxtxcrc[11] = txcrc[10] ^ crcshin;
            nxtxcrc[10] = txcrc[ 9] ^ crcshin;
            nxtxcrc[ 9] = txcrc[ 8];
            nxtxcrc[ 8] = txcrc[ 7] ^ crcshin;
            nxtxcrc[ 7] = txcrc[ 6] ^ crcshin;
            nxtxcrc[ 6] = txcrc[ 5];
            nxtxcrc[ 5] = txcrc[ 4] ^ crcshin;
            nxtxcrc[ 4] = txcrc[ 3] ^ crcshin;
            nxtxcrc[ 3] = txcrc[ 2];
            nxtxcrc[ 2] = txcrc[ 1] ^ crcshin;
            nxtxcrc[ 1] = txcrc[ 0] ^ crcshin;
            nxtxcrc[ 0] = crcshin;

            for(i=0;i<32;i++)
                txcrc[i] = nxtxcrc[i];
        }

        //printf("out "); for(j=0;j<32;j++) printf("%d ", txcrc[31-j]);

        txdout[3] = (txcrc[31]<<7) + (txcrc[30]<<6) + (txcrc[29]<<5) + (txcrc[28]<<4)
            + (txcrc[27]<<3) + (txcrc[26]<<2) + (txcrc[25]<<1) + (txcrc[24]<<0);
        txdout[2] = (txcrc[23]<<7) + (txcrc[22]<<6) + (txcrc[21]<<5) + (txcrc[20]<<4)
            + (txcrc[19]<<3) + (txcrc[18]<<2) + (txcrc[17]<<1) + (txcrc[16]<<0);
        txdout[1] = (txcrc[15]<<7) + (txcrc[14]<<6) + (txcrc[13]<<5) + (txcrc[12]<<4)
            + (txcrc[11]<<3) + (txcrc[10]<<2) + (txcrc[ 9]<<1) + (txcrc[ 8]<<0);
        txdout[0] = (txcrc[ 7]<<7) + (txcrc[ 6]<<6) + (txcrc[ 5]<<5) + (txcrc[ 4]<<4)
            + (txcrc[ 3]<<3) + (txcrc[ 2]<<2) + (txcrc[ 1]<<1) + (txcrc[ 0]<<0);
    }

    *pData++ = txdout[3];
    *pData++ = txdout[2];
    *pData++ = txdout[1];
    *pData++ = txdout[0];
}

#if 0
void __TS_SECTION__(void) {}
#endif

MMP_ULONG MMPF_MTS_GenerateTsHeader (MMP_UBYTE *pData, MMPF_MTS_TS *pTS)
{
    *pData++ = MTS_TS_SYNC_BYTE;
    *pData++ = ((pTS->usPID>>8)&0x1F) | (pTS->bPayloadUnitStart? 0x40: 0);
    *pData++ = (MMP_UBYTE) pTS->usPID;

    if (pTS->bPcrFlag) {
        *pData++ = (pTS->ubContinuity & 0x0F) | 0x20;
        *pData++ = 0xB7; //length
        *pData++ = 0x10; //PCR flag
        *pData++ = 0x01; //init PCR_base, PCR_ext
        *pData++ = 0x9B;
        *pData++ = 0xA4;
        *pData++ = 0xDC;
        *pData++ = 0x7E;
        *pData++ = 0x00; //end PCR_base, PCR_ext
        MEMSET(pData, 0xFF, 0xB7);

        return MTS_TS_PACKET_SIZE;
    }
    else {
        *pData++ = (pTS->ubContinuity & 0x0F) | (pTS->bAdaptationCtl? 0x30: 0x10);

        if (pTS->bHeaderSection) {
            *pData++ = 0;
            return 5;
        }
        else {
            return 4;
        }
    }
}

void MMPF_MTS_UpdateTsHeader (MMP_UBYTE *pData, MMP_BOOL bPayloadUnitStart, MMP_UBYTE ubContinuityCnt, MMP_UBYTE ubStuffingBytes)
{
    if (bPayloadUnitStart) {
        pData[1] |= 0x40; //payload_unit_start_indicator
    }
    else {
        pData[1] &= ~0x40;
    }

    if (ubStuffingBytes) {
        pData[3] = (pData[3]&0xD0) | 0x20 | (ubContinuityCnt&0x0F);

        pData[4] = ubStuffingBytes - 1; //adaptation_field_length
        if (ubStuffingBytes - 1) {
            pData[5] = 0; //header 8 bit
            if (ubStuffingBytes - 2) {
                MEMSET(pData+6, 0xFF, ubStuffingBytes-2); //stuffing bytes
            }
        }
    }
    else {
        pData[3] = (pData[3]&0xD0) | (ubContinuityCnt&0x0F);
    }
}

void MMPF_MTS_IncreamentTsPcr (MMPF_MTS_TIME_INFO *time_info)
{
    time_info->ulTime           += time_info->ulTimeDelta;
    time_info->ulTimeDecimal    += time_info->ulTimeDecimalDelta;

    if (time_info->ulTimeDecimal >= 300) {
        time_info->ulTimeDecimal -= 300;
        time_info->ulTime++;
    }
    if (time_info->ulTime <= time_info->ulTimeDelta) { //overflow
        time_info->ubTimeMsb32 ^= 0x01; //carry out
    }
}

void MMPF_MTS_UpdateTsPcr(MMP_UBYTE *PcrPacket, MMPF_MTS_TIME_INFO *PcrInfo)
{
    MMP_ULONG temp = ((PcrInfo->ubTimeMsb32 << 31) & 0x80000000) | 
                                ((PcrInfo->ulTime >> 1) & 0x7FFFFFFF);

    *PcrPacket++ = (temp>>24) & 0xFF;
    *PcrPacket++ = (temp>>16) & 0xFF;
    *PcrPacket++ = (temp>> 8) & 0xFF;
    *PcrPacket++ = (temp    ) & 0xFF;

    *PcrPacket++ = ((PcrInfo->ulTime << 7) & 0x80) | 0x7E | ((PcrInfo->ulTimeDecimal >> 8) & 0x01);
    *PcrPacket++ = PcrInfo->ulTimeDecimal & 0xFF;
}

#if 0
void __PES_SECTION__(void) {}
#endif
//******************************************************************************
// time_stamp_code()
//******************************************************************************
// 0) time_stamp_32   : time stamp, bit [32];
//    time_stamp_31_00: time stamp, bit [31:0];
//Sample API usage
//MMPF_MTS_UpdatePesTimestamp(ulTimestampMsb32, ulTimestamp, &(MtsPesPayload[PES_PTS_PAYLOAD_OFFSET]));
//MMPF_MTS_UpdatePesTimestamp(ulTimestampMsb32, ulTimestamp, &(MtsPesPayload[PES_PTS_PAYLOAD_OFFSET]));
void MMPF_MTS_UpdatePesTimestamp(MMP_UBYTE *pPesData, MMP_ULONG time_stamp_32, MMP_ULONG time_stamp_31_00)
{
    MMP_ULONG   time_code_31_00;

    time_code_31_00  =( ((time_stamp_31_00 & 0x00007FFF)<<1)
                       +((time_stamp_31_00 & 0x3FFF8000)<<2)) | 0x00010001;//marker bits '1'

    *(pPesData +4) = ((time_code_31_00 & 0x000000FF)    );
    *(pPesData +3) = ((time_code_31_00 & 0x0000FF00)>> 8);
    *(pPesData +2) = ((time_code_31_00 & 0x00FF0000)>>16);
    *(pPesData +1) = ((time_code_31_00 & 0xFF000000)>>24);

    *(pPesData +0) = (((time_stamp_31_00 & 0xC0000000)>>29) +
                      ((time_stamp_32                )<< 3)) | ((*pPesData) & 0xF1);
}
void MMPF_MTS_UpdatePesAudioLens(MMP_UBYTE *pPesData, MMP_USHORT length)
{
    length += 8; //8 byte for header part

    *(pPesData +1) = ((length & 0x00FF)    );
    *(pPesData ) = ((length & 0xFF00)>> 8);
}

void MMPF_MTS_UpdatePesStreamId(MMP_UBYTE *pPesData, MMP_UBYTE ubStreamId)
{
    *pPesData = ubStreamId;
}

void MMPF_MTS_IncreamentPesTimestamp (MMPF_MTS_TIME_INFO *time_info)
{
    time_info->ulTime           += time_info->ulTimeDelta;
    time_info->ulTimeDecimal    += time_info->ulTimeDecimalDelta;

    if (time_info->ulTimeDecimal >= 100000) {
        time_info->ulTimeDecimal -= 100000;
        time_info->ulTime++;
    }
    if (time_info->ulTime <= time_info->ulTimeDelta) { //overflow
        time_info->ubTimeMsb32 ^= 0x01; //carry out
    }
}

MMP_ULONG MMPF_MTS_GeneratePesVidHeader(MMP_UBYTE *pPesVidHdr)
{
    MEMCPY(pPesVidHdr, MtsPesVidHeader, sizeof(MtsPesVidHeader));

    return sizeof(MtsPesVidHeader);
}

MMP_ULONG MMPF_MTS_GeneratePesAudHeader(MMP_UBYTE *pPesAudHdr)
{
    MEMCPY(pPesAudHdr, MtsPesAudHeader, sizeof(MtsPesAudHeader));

    return sizeof(MtsPesAudHeader);
}

#if 0
void __SIT_SECTION__(void) {}
#endif

void MMPF_MTS_GenerateSit(MMP_UBYTE *pData, MMP_ULONG *ulLength)
{
    *ulLength = sizeof(MtsSitInfo);

    MEMCPY(pData, MtsSitInfo, *ulLength);
}

#if 0
void __PMT_SECTION__(void) {}
#endif

// Sample API usage
//MMPF_MTS_InitPmtConfig (&Psi, MTS_PID_PCR_0, MtsPmtProgramInfo, sizeof(MtsPmtProgramInfo));
void MMPF_MTS_InitPmtConfig (
        MMPF_MTS_PSI    *pPsi,
        MMP_USHORT      usPcrPid,
        MMP_UBYTE       *pProgrameInfo,
        MMP_USHORT      usProgramInfoLen)
{
    pPsi->TableID           = MMPF_MTS_PSI_TABLE_PMT;
    pPsi->ubVersionNum      = 0;
    pPsi->ubSectionNum      = 0;
    pPsi->ubLastSectionNum  = 0;
    pPsi->ubVersionNum      = 0;
    pPsi->usIdNum           = 1;

    pPsi->Tab.Pmt.usPCRPid      = usPcrPid;
    pPsi->Tab.Pmt.pProgramInfo  = pProgrameInfo;
    pPsi->Tab.Pmt.usProgramInfoLen = usProgramInfoLen;

    pPsi->Tab.Pmt.ubNumEs = 0;

}

void MMPF_MTS_GetPmtVidDesc(MMP_UBYTE **pDesc, MMP_ULONG *ulDescLen)
{
    *pDesc      = MtsPmtVidEsInfoDesc;
    *ulDescLen  = sizeof(MtsPmtVidEsInfoDesc);
}

void MMPF_MTS_GetPmtAudDesc(MMP_UBYTE **pDesc, MMP_ULONG *ulDescLen, MMPF_MTS_STREAM_TYPE type)
{
    if (type == MMPF_MTS_STREAM_TYPE_AAC) {
        *pDesc = NULL;
        *ulDescLen = 0;
    }
    else if (type == MMPF_MTS_STREAM_TYPE_PCM) {
        *pDesc = MtsPmtPcmEsInfoDesc;
        *ulDescLen = sizeof(MtsPmtPcmEsInfoDesc);
    }
    else {
        *pDesc = NULL;
        *ulDescLen = 0;
    }
}

void MMPF_MTS_GetPmtProgramDesc(MMP_UBYTE **pDesc, MMP_ULONG *ulDescLen)
{
    *pDesc      = MtsPmtProgramInfo;
    *ulDescLen  = sizeof(MtsPmtProgramInfo);
}

void MMPF_MTS_UpdatePmtVidDesc (MMP_UBYTE *pDesc, MMP_USHORT usProfile, MMP_USHORT usLevel)
{
    pDesc[2] = (MMP_UBYTE) usProfile;
    pDesc[4] = (MMP_UBYTE) usLevel;
}

// Sapmle API usage
// MMPF_MTS_AddEsToPmt(&Psi, MMPF_MTS_STREAM_TYPE_H264, MTS_PID_ES_VIDEO_0, MtsPmtVidEsInfoDesc, sizeof(MtsPmtVidEsInfoDesc));
// MMPF_MTS_AddEsToPmt(&Psi, MMPF_MTS_STREAM_TYPE_AAC, MTS_PID_ES_AUDIO_0, NULL, 0);
void MMPF_MTS_AddEsToPmt (
        MMPF_MTS_PSI    *pPsi,
        MMPF_MTS_STREAM_TYPE EsType,
        MMP_USHORT      usEsPid,
        MMP_UBYTE       *pEsInfoDesc,
        MMP_USHORT      usEsInfoLen)
{
    pPsi->Tab.Pmt.ubStreamType[pPsi->Tab.Pmt.ubNumEs]   = (MMP_UBYTE)EsType;
    pPsi->Tab.Pmt.usEsPid[pPsi->Tab.Pmt.ubNumEs]        = usEsPid;
    pPsi->Tab.Pmt.pEsInfo[pPsi->Tab.Pmt.ubNumEs]        = pEsInfoDesc;
    pPsi->Tab.Pmt.usEsInfoLen[pPsi->Tab.Pmt.ubNumEs]    = usEsInfoLen;

    pPsi->Tab.Pmt.ubNumEs++;
}

#if 0
void __PAT_SECTION__(void) {}
#endif

//MMPF_MTS_InitPatConfig(&Psi, MTS_PID_PMT, 1);
void MMPF_MTS_InitPatConfig (MMPF_MTS_PSI *pPsi, MMP_USHORT usPmtPid[], MMP_USHORT usNumPmt)
{
    MMP_UBYTE program;

    pPsi->TableID           = MMPF_MTS_PSI_TABLE_PAT;
    pPsi->ubVersionNum      = 0;
    pPsi->ubSectionNum      = 0;
    pPsi->ubLastSectionNum  = 0;
    pPsi->ubVersionNum      = 0;
    pPsi->usIdNum           = 1;

    pPsi->Tab.Pat.ubNumProgram = usNumPmt + 1; //reserve 1 from NIT

    pPsi->Tab.Pat.usPmtPid[0] = MTS_PID_NIT;
    pPsi->Tab.Pat.usProgramNum[0] = 0;
    for (program = 1; program < pPsi->Tab.Pat.ubNumProgram; program++) {
        pPsi->Tab.Pat.usPmtPid[program] = usPmtPid[program-1];
        pPsi->Tab.Pat.usProgramNum[program] = program;
    }
}

#if 0
void __COMMON_SECTION__(void) {}
#endif

MMP_ERR MMPF_MTS_GeneratePsiTable (MMP_UBYTE *pData, MMP_ULONG *ulDataLen, MMPF_MTS_PSI *pPsi)
{
    MMP_USHORT  i, usSectionLen = 0;
    MMP_UBYTE   *pStart = pData;

    *pData++ =  (MMP_UBYTE)pPsi->TableID;
    *pData++ =  0xB0; //section_syntax_indicator'1'|'0'|reserved'11'|section_len['00'|9:8]
    *pData++ =  0x00; //section_len fill later
    *pData++ =  pPsi->usIdNum >> 8; // big-endian
    *pData++ =  pPsi->usIdNum & 0xFF;
    *pData++ =  0xC0 | //reserved
                ((pPsi->ubVersionNum&0x1F) << 1) |
                0x01; // cur_next_indicator
    *pData++ = pPsi->ubSectionNum;
    *pData++ = pPsi->ubLastSectionNum;
    usSectionLen = 5; //count from usIdNum

    switch (pPsi->TableID) {
    case MMPF_MTS_PSI_TABLE_PAT:
        for (i = 0; i < pPsi->Tab.Pat.ubNumProgram; i++) {
            *pData++ = (pPsi->Tab.Pat.usProgramNum[i] >> 8) & 0xFF;
            *pData++ = pPsi->Tab.Pat.usProgramNum[i] & 0xFF;
            *pData++ = 0xE0 | ((pPsi->Tab.Pat.usPmtPid[i] >> 8) & 0x1F);
            *pData++ = pPsi->Tab.Pat.usPmtPid[i] & 0xFF;
            usSectionLen += 4;
        }
        break;
    case MMPF_MTS_PSI_TABLE_PMT:
        *pData++ = 0xE0 | ((pPsi->Tab.Pmt.usPCRPid >> 8) & 0x1F);//reserved'111'|PCR_PID[12:8]
        *pData++ = pPsi->Tab.Pmt.usPCRPid & 0xFF; //PCR_PID[7:0]
        *pData++ = 0xF0 | ((pPsi->Tab.Pmt.usProgramInfoLen >> 8) & 0x03); //reserved'1111'|ProgInfoLen['00'|9:8]
        *pData++ = pPsi->Tab.Pmt.usProgramInfoLen & 0xFF; //ProgInfoLen[7:0]
        usSectionLen += 4;
        if (pPsi->Tab.Pmt.usProgramInfoLen) {
            MEMCPY(pData, pPsi->Tab.Pmt.pProgramInfo, pPsi->Tab.Pmt.usProgramInfoLen); //descriptor
            pData += pPsi->Tab.Pmt.usProgramInfoLen;
            usSectionLen += pPsi->Tab.Pmt.usProgramInfoLen;
        }
        for (i = 0; i < pPsi->Tab.Pmt.ubNumEs; i++) {
            *pData++ = pPsi->Tab.Pmt.ubStreamType[i]; //stream_type
            *pData++ = 0xE0 | ((pPsi->Tab.Pmt.usEsPid[i] >> 8) & 0x1F); //ES_PID[12:8]
            *pData++ = pPsi->Tab.Pmt.usEsPid[i] & 0xFF; //ES_PID[7:0]
            *pData++ = 0xF0 | ((pPsi->Tab.Pmt.usEsInfoLen[i] >> 8) & 0x03); //ES_Info_len[00|9:8]
            *pData++ = pPsi->Tab.Pmt.usEsInfoLen[i] & 0xFF; //ES_Info_len[00|7:0]
            usSectionLen += 5;
            if (pPsi->Tab.Pmt.usEsInfoLen[i]) {
                MEMCPY(pData, pPsi->Tab.Pmt.pEsInfo[i], pPsi->Tab.Pmt.usEsInfoLen[i]); //descriptor
                pData += pPsi->Tab.Pmt.usEsInfoLen[i];
                usSectionLen += pPsi->Tab.Pmt.usEsInfoLen[i];
            }
        }
        break;
    default:
        //unspport
        break;
    }

    usSectionLen += 4;
    pStart[1] |= ((usSectionLen >> 8) & 0x03);
    pStart[2] |= (MMP_UBYTE) usSectionLen;

    MMPF_CalcCrc32 (pStart, (usSectionLen+3)-4, pData);

    *ulDataLen = usSectionLen + 3;

    return MMP_ERR_NONE;
}
#endif

/// @}
/// @end_ait_only

