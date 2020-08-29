/*
 * nvram.c
 */
#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_hif.h"
#include "mmpf_sif.h"
#include "mmp_reg_sif.h"
#include "stdface.h"
 
// The NVRAM is located at 400KBytes to 512KBytes in Serial Flash.
// All of input read/write address will be added NVRAM_ADDRESS.
#define	TO_ADDRESS(p)		((unsigned int)(p) + NVRAM_ADDRESS)
#define	IN_RANGE(p, s)		((TO_ADDRESS(p) < NVRAM_MAX_ADDRESS) &&	\
							 (TO_ADDRESS(p + s) < NVRAM_MAX_ADDRESS))
//
/*MMP_ERR MMPF_SIF_Init(void);
MMP_ERR MMPF_SIF_DisableWrite(void);
MMP_ERR MMPF_SIF_ReadData(MMP_ULONG destAddr, MMP_ULONG fbAddr, MMP_USHORT Count);
MMP_ERR	MMPF_SIF_WriteData(MMP_ULONG flashAddress, MMP_ULONG fbAddr, MMP_ULONG size);
void MMPF_MMU_FlushCacheAll(void);
*/

int vnopen()
{
	//MMPF_SIF_Init();
	return 0;
}

int vnclose()
{
	//MMPF_SIF_DisableWrite();
	return 0;
}

unsigned int nvread(void *nvaddr, char* data, unsigned int size)
{

	memcpy( data ,(char*) ((unsigned int)nvaddr + 0x100000), size);
/*	MMPF_MMU_FlushAndInvalidDCache();
	vnopen();
	if (IN_RANGE((char*)nvaddr, size)) {
		if (MMPF_SIF_ReadData(TO_ADDRESS(nvaddr), (MMP_ULONG)MEMPOOL_TO_PHY(data), (MMP_USHORT)size) != MMP_ERR_NONE) {
			printk("MMPF_SIF_ReadData error\r\n");
			return 0;
		}
		return size;
	}*/
	
	return 0;
}
unsigned int nvwrite(void *nvaddr, char* data, unsigned int size)
{

memcpy( (char*) ((unsigned int)nvaddr + 0x100000) , data , size);
/*	MMPF_MMU_FlushAndInvalidDCache();
	vnopen();
	if (IN_RANGE((char*)nvaddr, size)) {
		if (MMPF_SIF_WriteData(TO_ADDRESS(nvaddr), (MMP_ULONG)MEMPOOL_TO_PHY(data), (MMP_USHORT)size) != MMP_ERR_NONE) {
			printk("nvwrite ERROR\r\n");
			return 0;
		}
	}*/
	return 0;
}
