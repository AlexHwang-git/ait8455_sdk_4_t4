
#ifndef _MMPF_SF_H_
#define _MMPF_SF_H_

#include    "includes_fw.h"

typedef enum _MMPF_SIF_PAD
{
	MMPF_SIF_PAD_NONE = 0x0,
    MMPF_SIF_PAD_0 = 1,  //pad set PHDMI
    MMPF_SIF_PAD_1 = 2,	 //pad set PCGPIO
    MMPF_SIF_PAD_2 = 3,	 //pad set PBGPIO
    MMPF_SIF_PAD_MAX
} MMPF_SIF_PAD;


//------------------------------------------------------------------------------
// SF Command Index
//------------------------------------------------------------------------------
#define WRITE_STATUS			0x01
#define PAGE_PROGRAM			0x02
#define READ_DATA				0x03
#define WRITE_DISABLE			0x04
#define EXIT_OTP_MODE			0x04
#define READ_STATUS				0x05
#define WRITE_ENABLE			0x06
#define FAST_READ_DATA			0x0B
#define SECTOR_ERASE			0x20
#define ENTER_OTP_MODE			0x3A
#define BLOCK_ERASE				0x52
#define CHIP_ERASE				0x60
#define EBSY					0x70
#define DBSY					0x80
#define READ_DEVICE_ID			0x9F
#define RELEASE_DEEP_POWER_DOWN 0xAB
#define ADDR_AUTO_INC_WRITE		0xAD
#define DEEP_POWER_DOWN			0xB9
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SF Device ID
//------------------------------------------------------------------------------
#define EON_EN25F10				0x1C3111
#define EON_EN25F20				0x1C3112
#define EON_EN25F40				0x1C3113
#define EON_EN25F80				0x1C3114
#define EON_EN25Q16A			0x1C3015
#define EON_EN25Q32A			0x1C3016

#define WINBOND_25X10			0xEF3011
#define WINBOND_25X20			0xEF3012
#define WINBOND_25X40			0xEF3013
#define WINBOND_25X80			0xEF3014
#define WINBOND_25Q80			0xEF4014
#define WINBOND_25Q16			0xEF4015
#define WINBOND_25Q32			0xEF4016
#define WINBOND_25Q64			0xEF4017

#define MXIC_25L512				0xC22010
#define MXIC_25L1005			0xC22011
#define MXIC_25L2005			0xC22012
#define MXIC_25L4005			0xC22013
#define MXIC_25L8005			0xC22014

#define SST_25VF080B			0xBF258E
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SF Sector Size
//------------------------------------------------------------------------------
#define SerialFlashSectorSize	0x1000
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SF Status Register
//------------------------------------------------------------------------------
#define WRITE_IN_PROGRESS 		0x01
#define WRITE_ENABLE_LATCH 		0x02
//------------------------------------------------------------------------------

// SF Psd Set
//------------------------------------------------------------------------------
#define SET_PAD0	 		0x2000
#define SET_PAD1	 		0x2100
#define SET_PAD2	 		0x2200
#define RESET_BOOT_DEVICE_AND_SIF_PAD   0xE300
//------------------------------------------------------------------------------

MMP_ERR MMPF_SF_CheckReady(void);
MMP_ERR MMPF_SF_ReadStatus(MMP_UBYTE *ubStatus);
MMP_ERR MMPF_SF_WriteStatus(MMP_UBYTE ubData);
MMP_ERR MMPF_SF_EnableWrite(void);
MMP_ERR MMPF_SF_DisableWrite(void);
MMP_ERR MMPF_SF_SetTmpAddr(MMP_ULONG ulStartAddr);
MMP_ERR MMPF_SF_GetTmpAddr(MMP_ULONG *ulStartAddr);
MMP_ERR MMPF_SF_InitialInterface(void);
MMP_ERR MMPF_SF_Reset(void);
MMP_ERR MMPF_SF_GetSFInfo(MMP_ULONG *ulDevId, MMP_ULONG *ulSFTotalSize, MMP_ULONG *ulSFSectorSize);
MMP_ERR MMPF_SF_ReadDevId(MMP_ULONG *id);
MMP_ERR MMPF_SF_ReadData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount);
MMP_ERR MMPF_SF_FastReadData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount);
MMP_ERR MMPF_SF_WriteData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount);
MMP_ERR MMPF_SF_EraseSector(MMP_ULONG ulSFAddr);
MMP_ERR MMPF_SF_EraseBlock(MMP_ULONG ulSFAddr);
MMP_ERR MMPF_SF_EraseChip(void);
MMP_ERR MMPF_SF_AaiWriteData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount);
MMP_ERR MMPF_SF_EBSY(void);
MMP_ERR MMPF_SF_DBSY(void);
MMP_ERR MMPF_SF_ReleasePowerDown(void);
MMP_ERR MMPF_SF_PowerDown(void);
#if (CHIP == VSN_V3)
MMP_ERR MMPF_SF_EnableCrcCheck(MMP_BOOL bEnable);
MMP_ERR MMPF_SF_GetCrcValue(MMP_USHORT* usCrcValue);
#endif

#endif