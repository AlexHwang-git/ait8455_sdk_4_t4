#ifndef _PCAM_DFU_H
#define _PCAM_DFU_H

#include "mmpf_typedef.h"

#define	SFLASH_WIP	0x01
#define	SFLASH_WEL	0x02
#define	SFLASH_SECTOR_SIZE	0x1000
#define	SFLASH_PAGE_SIZE	0x100


typedef struct
{
	MMP_BOOL bDefault;//:1,// = 1
	MMP_BOOL bRemoveHeader;// = 1
	MMP_BOOL bKeyRom;// = 1
	MMP_BOOL bPassword;// = 1
	MMP_BOOL bPartialUpdate;// = 1
	MMP_BOOL bHidden;// = 1
	MMP_BOOL bUpdateSize;// = 1
	MMP_BOOL bExtendedData;// = 1
	MMP_BOOL bEncryption;// = 1
	MMP_BOOL bChecksum;// = 1
	MMP_BOOL bBinaryComparasion;// = 1
} ROM_FILE_DESC_FLAG;

typedef struct
{
	MMP_ULONG dwNextROMOffset;	// 	;4
	MMP_ULONG dwROMAddr;			//	;4
	ROM_FILE_DESC_FLAG *dwFlags;			//	;4
	MMP_ULONG dwROMDataSize;
	MMP_UBYTE bROMNameSize;		//	;1
	MMP_UBYTE *strROMName;
	MMP_ULONG dwExtSize;
	MMP_UBYTE *bExtData;
	MMP_ULONG dwBinaryUpdateSize;
	MMP_UBYTE bPasswordSize;		//	; 4
	MMP_UBYTE *strPassword;		// = "1234"
	MMP_UBYTE *bDataPtr;
} ROM_FILE_DESC;


MMP_ERR USB_Read840BootFirmware(MMP_UBYTE *FWImagePtr, MMP_ULONG FWImageSize);
MMP_ERR USB_Read840PCCAMFirmware(MMP_UBYTE *FWImagePtr, MMP_ULONG FWImageSize);
MMP_ERR USB_DownloadFWInitialize(void);
MMP_ERR USB_DownloadFWData(MMP_UBYTE *data, MMP_USHORT len);
MMP_ERR USB_Burning840FW2SFlash(void);
MMP_ERR	Burning840PCCAMFirmwareImage(MMP_ULONG FlashAddr);
MMP_ERR BurningBootCode(void);
MMP_ERR Burning845xFWImage(void);
void MoveData2SIF(MMP_ULONG ulSifAddr, MMP_ULONG ulSourceAddr, MMP_ULONG ulDataSize);
MMP_ERR GetSIFCRCValue(MMP_USHORT* crc);
MMP_USHORT USB_GetStorageLayout(MMP_UBYTE *fwptr,MMP_ULONG fwsize);
MMP_USHORT USB_GetStorageLayoutFromFlash(void);
MMP_ERR	SaveVideoPro(void);
MMP_ERR	GetVideoPro(void);
#endif
