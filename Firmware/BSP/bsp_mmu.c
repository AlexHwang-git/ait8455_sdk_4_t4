
#include <lib_arm920t.h>

#include "includes_fw.h"

#include <bsp_mmu.h>

//*----------------------------------------------------------------------------
//*  AT91F_CleanDCache
//*  Clean and invalidate D Cache
//*----------------------------------------------------------------------------
void AT91F_CleanDCache()
{
	register char seg, index;
	for (seg = 0; seg < 8; ++seg) {
		for (index = 0; index < 64; ++index) {
			AT91F_ARM_CleanDCacheIDX((index << 26) | (seg << 5));
		}
	}
}

//*----------------------------------------------------------------------------
//*  AT91F_ResetICache
//*  Reset I Cache (Should be run from a non cachable area)
//*----------------------------------------------------------------------------
void AT91F_ResetICache()
{
	// Flush I TLB
	AT91F_ARM_InvalidateITLB();
	// Flush I cache
	AT91F_ARM_InvalidateICache();
}

//*----------------------------------------------------------------------------
//*  AT91F_ResetDCache
//*  Reset D Cache (Should be run from a non cachable area)
//*----------------------------------------------------------------------------
void AT91F_ResetDCache()
{
	// Achieve pending write operations
	AT91F_CleanDCache();
	// Flush write buffers
	AT91F_ARM_DrainWriteBuffer();
	// Flush D TLB
	AT91F_ARM_InvalidateDTLB();
	// Flush D cache
	AT91F_ARM_InvalidateDCache();
}

//*----------------------------------------------------------------------------
//*  AT91F_EnableMMU
//*  Enable MMU
//*----------------------------------------------------------------------------
void AT91F_EnableMMU()
{
	unsigned int ctl;

	ctl = AT91F_ARM_ReadControl();
	ctl |= (1 << 0);
	AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//*  AT91F_DisableMMU
//*  Disable MMU
//*----------------------------------------------------------------------------
void AT91F_DisableMMU()
{
	unsigned int ctl;

	ctl = AT91F_ARM_ReadControl();
	ctl &= ~(1 << 0);
	AT91F_ARM_WriteControl(ctl);
}


//*----------------------------------------------------------------------------
//*  AT91F_EnableICache
//*  Enable I Cache
//*----------------------------------------------------------------------------
void AT91F_EnableICache()
{
	unsigned int ctl;

	ctl = AT91F_ARM_ReadControl();
	ctl |= (1 << 12);
	AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//*  AT91F_DisableICache
//*  Disable I Cache
//*----------------------------------------------------------------------------
void AT91F_DisableICache()
{
	unsigned int ctl;

	ctl = AT91F_ARM_ReadControl();
	ctl &= ~(1 << 12);
	AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//*  AT91F_EnableDCache
//*  Enable D Cache
//*----------------------------------------------------------------------------
void AT91F_EnableDCache()
{
	unsigned int ctl;

	ctl = AT91F_ARM_ReadControl();
	ctl |= (1 << 2);
	AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//*  AT91F_DisableDCache
//*  Disable D Cache
//*----------------------------------------------------------------------------
void AT91F_DisableDCache()
{
	unsigned int ctl;

	ctl = AT91F_ARM_ReadControl();
	ctl &= ~(1 << 2);
	AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//*  AT91F_LockITLB
//*  Lock one I TLB entry after entries previously locked
//*----------------------------------------------------------------------------
void AT91F_LockITLB(unsigned int address)
{
	// Set the P bit
	AT91F_ARM_WriteITLBLockdown( 1); // base 0 victim 0 P 1
	// Prefetch the instruction assuming an ITLB miss occurs, the entry base is loaded
	AT91F_ARM_PrefetchICacheLine(address);
	// Update base and victim values
	AT91F_ARM_WriteITLBLockdown((1 << 26) | (1 << 20)); // base 1 victim 1 P 0
}

//*----------------------------------------------------------------------------
//*  AT91F_ARM_WriteITLBLockdown
//*  Write I TLB lockdown  (Should be run from a non cachable area)
//*----------------------------------------------------------------------------
void AT91F_LockICache(unsigned int startAddress, unsigned int size)
{
	unsigned int victim = 0;

	AT91F_ARM_InvalidateICache();
	AT91F_ARM_WriteICacheLockdown((victim++) << 26);
	while (size) {
		// Prefetch ICache line
		AT91F_ARM_PrefetchICacheLine(startAddress);
		startAddress += 32;
		// Test for segment 0, and if so increment victim pointer
		if ( !(startAddress & 0xE0) )
			AT91F_ARM_WriteICacheLockdown((victim++) << 26);
		size -= 32;

	}
	// If segment != 0 the increment victim pointer
	if ( (startAddress & 0xE0) )
		AT91F_ARM_WriteICacheLockdown(victim << 26);
}
