/*
 * part.c
 */
#include "mmp_err.h"
#include "mmpf_typedef.h"
//#include "loader.h"
#include "string.h"

int printk(const char *fmt, ...);
void hexdump(void* buf, short len);

static LOADER_HDR*	pld;

int LoadAgentFromPC(unsigned char *codeptr)
{
	char		*seg, *addr;
	int			size;

	seg  = GET_FIRST_SEG((char*)codeptr);
	addr = GET_SEG_ADDR(seg);
	size = GET_SEG_SIZE(seg);
	memcpy(addr, GET_SEG_DATA(seg), size);
	pld = (LOADER_HDR*)addr;
	if (pld->ldMagicID == MAGIC) {
		pld->ln_printk = printk;
		pld->ldEntry(0, 0);
		return 0;
	}
	return 1;
}

void AT91F_ResetICache(void);
void AT91F_ResetDCache(void);

#define	USB_CLOCK				0x80008C70
#define CLK_USB					0x00000100

void OffUsb()
{
	volatile unsigned char*	pusb		= (unsigned char*)USB_CLOCK;
    volatile unsigned char*	REG_BASE_B	= (unsigned char *)0x80000000;

    REG_BASE_B[0x6041] |= 0x80;       // Enter USB testmode
    REG_BASE_B[0x6040] &= ~(0x80);    // Pulldown SUSPENDM
    REG_BASE_B[0x6050] |= 0x40;       // opr_OSCOUTEN
    REG_BASE_B[0x6051] &= ~(0x08);    // opr_PLLALIV 
    REG_BASE_B[0x6041] |= 0x06;       // DP,DM Pulldown
    *pusb &= ~CLK_USB;
}

int LoadCodeToRAM(unsigned char *codeptr, unsigned int codecb)
{
	OffUsb();
	printk("Firmware @ %x size %x\r\n", codeptr, codecb);
	AT91F_ResetICache();
	AT91F_ResetDCache();
	pld->ldEntry(1, codeptr);	// Copy To RAM, Never return.
	return 0;
}

unsigned int MakeChkSum(void *ptr, unsigned int size)
{
	unsigned int	*data;
	unsigned int	chksum, len, i;
	
	data = (unsigned int*)ptr;
	len  = size >> 2;
	chksum = 0;
	for (i = 0; i < len; i++) {
		chksum += *data++;
	}
	return chksum;
}

#define	MBR_ID				0x3052424D
#define	MBR_VER				0x30003100
#define	MBR_RESERVE_SIZE	(128 - 5)
#define	FW_START			1
typedef struct {
	unsigned long	mid;
	unsigned long	mver;
	unsigned long	mchksum;
	unsigned long	mimgoff;
	unsigned long	mimglen;
	unsigned long	mres[MBR_RESERVE_SIZE];
} MBR;

static MBR	_mbr;

MMP_ERR MMPF_NAND_ReadPhysicalSector(MMP_ULONG ulAddr, MMP_ULONG ulPage);
MMP_ERR MMPF_NAND_WritePhysicalSector(MMP_ULONG ulAddr, MMP_ULONG ulPage);
/* 1 block = 256 sectors */
MMP_ERR MMPF_NAND_EraseBlock(MMP_ULONG ulStartPage);

MMP_ERR MMPF_NAND_WriteSector(MMP_ULONG dmastartaddr, MMP_ULONG startsect, MMP_USHORT sectcount);
MMP_ERR MMPF_NAND_ReadSector(MMP_ULONG dmastartaddr, MMP_ULONG startsect, MMP_USHORT sectcount);

int WriteToNAND(char* fwimg, unsigned int fwlen)
{
    unsigned int	chksum;
    unsigned int	sec;
    unsigned int	i;
    unsigned long*	fw;
    unsigned int	ve;

	chksum = MakeChkSum(fwimg, fwlen);
	printk("FirmwareImage %x size %x ChkSum %x\r\n",
			fwimg, fwlen, chksum);
	_mbr.mid  = MBR_ID;
	_mbr.mver = MBR_VER;
	_mbr.mchksum = chksum;
	_mbr.mimgoff = FW_START;
	_mbr.mimglen = fwlen;
	sec = (_mbr.mimglen + 511) >> 9;
	memset(_mbr.mres,0,  sizeof(unsigned long) * MBR_RESERVE_SIZE);
	for (i = 0; i < ((sec + 1/*MBR*/ + 255) >> 8); i++) {
		MMPF_NAND_EraseBlock(i << 8);
	}
	// Write MBR
	printk("Write MBR\r\n");
	MMPF_NAND_WritePhysicalSector((unsigned int)&_mbr, 0);
	// Read MBR
	fw = (unsigned long*)&_mbr;
	memset(fw, 0, sizeof(MBR));
	MMPF_NAND_ReadPhysicalSector((unsigned int)fw, 0);
	if (_mbr.mid != MBR_ID) {
		printk("WRITE MBR ERROR\r\n");
		hexdump(fw, 32);
		return 1;
	}
	// Write Firmware Image
	printk("Write Sectors %d\r\n", sec);
	for (i = 0; i < sec; i++) {
		// printk("Write Sector %d\r\n", i + FW_START);
		MMPF_NAND_WritePhysicalSector((unsigned int)fwimg + (i << 9), i + FW_START);
	}
	printk("Write Done!\r\n");
	ve = 0;
	// 
	for (i = 0; i < sec; i++) {
		int		j;
		
		MMPF_NAND_ReadPhysicalSector((unsigned int)fw, i + FW_START);
		for (j = 0; j < 128; j++) {
			ve += *(fw + j);
			fwlen -= 4;
			if (fwlen == 0) {
				printk("*** CHKSUM %x %x\r\n", ve, chksum);
				break;
			}
		}
	}
	if (ve != chksum)
		return 1;
	return 0;
}

extern MMP_ERR USB_DownloadFWInitialize(void);
extern MMP_ERR USB_Read840PCCAMFirmware(MMP_UBYTE *FWImagePtr, MMP_ULONG FWImageSize);
extern MMP_UBYTE *USBFirmwareBinPtr;
extern MMP_ULONG USBFirmwareBinSize;
extern MMP_UBYTE *FirmwareBinPtr;
extern MMP_ULONG FirmwareBinSize;

int LoadCodeFromNAND()
{
    unsigned int	chksum;
    unsigned int	sec;
    unsigned int	i, fwlen;
    unsigned long*	fw;
    MMP_ERR			status;
 
	#ifdef	_OKAO_
 	return 1;
	#endif
 	printk("LoadCodeFromNAND\r\n");
	MMPF_NAND_ReadPhysicalSector((unsigned int)&_mbr, 0);
	if (_mbr.mid  != MBR_ID || _mbr.mver != MBR_VER) {
		return 1;
	}
	USB_DownloadFWInitialize();
	fwlen = _mbr.mimglen;
	fw = (unsigned long*)USBFirmwareBinPtr;
	sec = (fwlen + 511) >> 9;
	for (i = 0; i < sec; i++) {
		MMPF_NAND_ReadPhysicalSector((unsigned int)fw + (i << 9),
									 i + FW_START);
	}
	chksum = MakeChkSum(fw, _mbr.mimglen);
	printk("CHKSUM %x %x\r\n", chksum, _mbr.mchksum);
	// If not equal, abort and run code in SF.
	if (chksum != _mbr.mchksum)
		return 1;
	USBFirmwareBinSize = _mbr.mimglen;
	//
    if (USB_Read840BootFirmware(USBFirmwareBinPtr, USBFirmwareBinSize)) {
        printk("USB_Read840BootFirmware fail\r\n");
        return 1;
    }
	LoadAgentFromPC(FirmwareBinPtr);
	status = USB_Read840PCCAMFirmware(USBFirmwareBinPtr, USBFirmwareBinSize);
	if(status) {
	    printk("USB_Read840PCCAMFirmware fail\r\n");
	    return 1;
	}
	// Put code in correct position and reboot, never return!
	LoadCodeToRAM(FirmwareBinPtr, FirmwareBinSize);
	return 0;
}

void FlashCodeToNAND()
{    	
    MMP_ERR			status;

	printk(">>> Flash Code to NAND <<<\r\n");
	WriteToNAND((char*)USBFirmwareBinPtr, USBFirmwareBinSize);
	// The FirmwareBinPtr is loader code, it is by USB_Read840BootFirmware(..)
	// when USB_Burning840FW2SFlash(..) called and check it bootcode or not.
	// If it is not bootcode then send a event to FDFR task to here.
	LoadAgentFromPC(FirmwareBinPtr);
	// Load firmware code to FirmwareBinPtr.
	status = USB_Read840PCCAMFirmware(USBFirmwareBinPtr, USBFirmwareBinSize);
	if(status) {
	    printk("USB_Read840PCCAMFirmware fail\r\n");
	    return;
	}
	LoadCodeToRAM(FirmwareBinPtr, FirmwareBinSize);
	return;
}