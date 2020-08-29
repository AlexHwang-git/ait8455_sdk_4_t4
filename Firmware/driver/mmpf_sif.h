
#ifndef _MMPF_SIF_H_
#define _MMPF_SIF_H_

#include    "includes_fw.h"



typedef enum _MMPF_SIF_CMD
{
    MMPF_SIF_STATUS_WRITE = 1,
    MMPF_SIF_PAGE_PROGRAM,
    MMPF_SIF_DATA_READ,
    MMPF_SIF_WRITE_DISABLE, 
    MMPF_SIF_STATUS_READ,
    MMPF_SIF_WRITE_ENABLE
} MMPF_SIF_DIR;

MMP_ERR MMPF_SIF_Init(void);
MMP_ERR MMPF_SIF_ReadID(MMP_BYTE chipIDCMD, MMP_ULONG fbAddr);
MMP_ERR MMPF_SIF_ReadData(MMP_ULONG destAddr, MMP_ULONG fbAddr, MMP_ULONG Count);
MMP_ERR MMPF_MX_SIF_WriteData(MMP_ULONG destAddr, MMP_ULONG fbAddr, MMP_USHORT Count);
MMP_ERR MMPF_SIF_EraseSector(MMP_ULONG destAddr);
MMP_ERR	MMPF_SIF_WriteData(MMP_ULONG flashAddress, MMP_ULONG fbAddr,MMP_ULONG size);
MMP_ERR MMPF_SIF_EnableWrite(void);
#endif