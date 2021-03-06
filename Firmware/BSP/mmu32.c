//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : mmu.c
//* Object              : Common MMU and CACHE operations
//* Creation            : ODi   Aug 22nd 02
//*
//*----------------------------------------------------------------------------
#include "includes_fw.h"
#include "lib_arm920t.h"
#include "lib_retina.h"
#if (defined(ALL_FW)&&(PCAM_EN == 0))
#include "..\FS\FS_ConfDefaults.h"
#endif
MMP_ULONG MMU_TRANSLATION_TABLE_ADDR,MMU_COARSEPAGE_TABLE_ADDR ;

#define MaxCoarsePageTableSlotNum 5
unsigned char pCoarsePageUsage = 0;
unsigned int *pCurCoarsePageTableAddr;
void    MMPF_SWPACK_MmuTableInit(unsigned int *pTranslationTable,unsigned int *pCoarsePageTable);
/** @addtogroup BSP
@{
*/

//*----------------------------------------------------------------------------
//* \fn    AT91F_CleanDCache
//* \brief Clean and invalidate D Cache
//*----------------------------------------------------------------------------
void AT91F_CleanDCache()
{
    register char seg, index;

#if 0 /* ARM920T */
    /// ARM920T addressing is index at the first, then seg.
    for (seg = 0; seg < 8; ++seg) {
        for (index = 0; index < 64; ++index) {
            AT91F_ARM_CleanDCacheIDX((index << 26) | (seg << 5));
        }
    }

#else /* ARM926EJ-S */
    /// For ARM926EJ-S, seg is the index of "WAY", index is the index of "set".
    /// ("WAY is ""LINE", 4-WAY means 4 lines in each "set")

    /// ARM926EJ-S addressing is WAY at the first, then SET.
    for (seg = 0; seg < 4; ++seg) {
        for (index = 0; index < 128; ++index) {
            AT91F_ARM_CleanDCacheIDX((seg << 30) | (index << 5));
        }
    }
#endif
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ResetICache
//* \brief Reset I Cache (Should be run from a non cachable area)
//*----------------------------------------------------------------------------
void AT91F_ResetICache()
{
    // Flush I TLB
    AT91F_ARM_InvalidateITLB();
    // Flush I cache
    AT91F_ARM_InvalidateICache();
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ResetDCache
//* \brief Reset D Cache (Should be run from a non cachable area)
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
//* \fn    AT91F_EnableMMU
//* \brief Enable MMU
//*----------------------------------------------------------------------------
void AT91F_EnableMMU()
{
    unsigned int ctl;

    ctl = AT91F_ARM_ReadControl();
    ctl |= (1 << 0);
    AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DisableMMU
//* \brief Disable MMU
//*----------------------------------------------------------------------------
void AT91F_DisableMMU()
{
    unsigned int ctl;

    ctl = AT91F_ARM_ReadControl();
    ctl &= ~(1 << 0);
    AT91F_ARM_WriteControl(ctl);
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_EnableICache
//* \brief Enable I Cache
//*----------------------------------------------------------------------------
void AT91F_EnableICache()
{
    unsigned int ctl;

    ctl = AT91F_ARM_ReadControl();
    ctl |= (1 << 12);
    AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DisableICache
//* \brief Disable I Cache
//*----------------------------------------------------------------------------
void AT91F_DisableICache()
{
    unsigned int ctl;

    ctl = AT91F_ARM_ReadControl();
    ctl &= ~(1 << 12);
    AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_EnableDCache
//* \brief Enable D Cache
//*----------------------------------------------------------------------------
void AT91F_EnableDCache()
{
    unsigned int ctl;

    ctl = AT91F_ARM_ReadControl();
    ctl |= (1 << 2);
    AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DisableDCache
//* \brief Disable D Cache
//*----------------------------------------------------------------------------
void AT91F_DisableDCache()
{
    unsigned int ctl;

    ctl = AT91F_ARM_ReadControl();
    ctl &= ~(1 << 2);
    AT91F_ARM_WriteControl(ctl);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_LockITLB
//* \brief Lock one I TLB entry after entries previously locked
//*----------------------------------------------------------------------------
#if (defined(ALL_FW)&&(PCAM_EN == 0))
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
//* \fn    AT91F_ARM_WriteITLBLockdown
//* \brief Write I TLB lockdown  (Should be run from a non cachable area)
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
#endif
/** @brief Initialize the MMU setting.

@note Free addresses from pTranslationTable: 0x8~0x3C, 0x48~0x1FFC, 0x2004~0x3FF8
@param[in] pTranslationTable The starting address of the 1MB granularity cache table.
           This address should be 16K bytes alignment in 820. 16KB size.
@param[in] pCoarsePageTable The starting address of the 4K granularity cache table.
           It has to be 1K alignment. 1KB size.
*/
void MMPF_InitMMU(unsigned int *pTranslationTable,unsigned int *pCoarsePageTable)
{
#if ISP_BUF_IN_SRAM==1
    MMP_UBYTE sram_t = 0x02 ;	
#else
    MMP_UBYTE sram_t = 0x0E ;
#endif
    MMP_ULONG i;

    if (((MMP_ULONG)pTranslationTable & (0x4000-1)) > 0) {// 0x4000: 16KB alignment
        RTNA_DBG_Str2("Invalid MMU table address\r\n");
        return;
    }
    // Program the TTB
    AT91F_ARM_WriteTTB((unsigned int) pTranslationTable);
    // Program the domain access register
    AT91F_ARM_WriteDomain(0xC0000000); // domain 15: access are not checked

    // Reset table entries
    for (i = 0; i < 4096; i++)
        pTranslationTable[i] = 0;

    // Program level 1 page table entry, 0x0~0x3 bytes
    pTranslationTable[0x0] =
        (0x0 << 20) |  // Physical Address
        (1 << 10) |      // Access in supervisor mode
        (15 << 5) |      // Domain
        (1 << 4) |
        0x2;             // Set as 1 Mbyte section

    //Config mmu for SRAM
	pTranslationTable[0x1] =
        (0x100000) |      // Physical Address
        (1 << 10) |      // Access in supervisor mode
        (15 << 5) |      // Domain
        (1 << 4) |
        sram_t ;//0xE;             // Set as 1 Mbyte section

    //Config mmu for SDRAM
    pTranslationTable[0x10] =
        ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
        (15 << 5) |      // Domain
        0x11;

    for (i = 0; i < 256; i++)
        pCoarsePageTable[i] =
        (0x1000000)|
        (i << 12) |
        0x332;

    //Config mmu for SDRAM
    #if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
    //Config mmu for SDRAM
    for (i = 0x11; i < 0x50; i++) {
        pTranslationTable[i] =
        (i << 20) |      // Physical Address
        (1 << 10) |      // Access in supervisor mode
        (15 << 5) |      // Domain
        (1 << 4) |
        0x2;             // Set as 1 Mbyte section
    }
    #endif

    //OPR
    pTranslationTable[0x800] = //0x2000~0x2003
        ((unsigned int)0x800 << 20) |  // Physical Address
        (1 << 10) |      // Access in supervisor mode
        (15 << 5) |      // Domain
        1 << 4 |
        0x2;             // Set as 1 Mbyte section

    //ARM's peripheral
    pTranslationTable[0xFFF] =
        ((unsigned int)0xFFF << 20) |  // Physical Address
        (1 << 10) |      // Access in supervisor mode
        (15 << 5) |      // Domain
        1 << 4 |
        0x2;             // Set as 1 Mbyte section



#if defined(ALL_FW)
    MMPF_SWPACK_MmuTableInit(pTranslationTable, pCoarsePageTable);
#endif

    // Enable the MMU
    AT91F_EnableMMU();
    AT91F_EnableICache();
    AT91F_EnableDCache();
}

/**
 *  Used to unload the MMU so that we can assign MMU to another address.
 *  Due to most of the functions are without parameters and returen value.
 *  So this function are designed so.
 */
// Note that these 2 MMPF_ functions are added in bsp.h
void MMPF_UninitMMU(void)
{
    AT91F_CleanDCache();
    AT91F_DisableDCache();
    AT91F_DisableICache();
    AT91F_DisableMMU(); 
    AT91F_ResetICache();
    AT91F_ResetDCache();
} 

/// @brief Flush the D cache and the write buffer to memory.
#pragma arm section rwdata = "player_isrs", code ="player_isrs"
void MMPF_MMU_FlushDCache(void)
{
    // Achieve pending write operations
    AT91F_CleanDCache();
    // Flush write buffers
    AT91F_ARM_DrainWriteBuffer();
}
#pragma arm section code, rwdata, rodata, zidata // return to default placement


void MMPF_MMU_FlushCache(MMP_ULONG ulRegion, MMP_ULONG ulSize)
{
    MMP_ULONG	i;
    MMP_ULONG	start_addr, end_addr;
    start_addr = FLOOR32(ulRegion);
    end_addr = ALIGN32(ulRegion + ulSize);

    for (i = start_addr; i < end_addr; i += 32) {
        AT91F_ARM_CleanInvalidateDCacheMVA(i);
    }		
    // Flush write buffers
    AT91F_ARM_DrainWriteBuffer();
}

void MMPF_MMU_FlushAndInvalidDCache(void)
{
    MMP_ULONG	i;
    MMP_ULONG	index;

    for (i = 0; i < 4; ++i) {
        for (index = 0; index < 128; ++index) {
            AT91F_ARM_CleanInvalidateDCacheIDX((i << 30) | (index << 5));
        }
    }

    // Flush write buffers
    AT91F_ARM_DrainWriteBuffer();
}


#if (((DSC_R_EN))&&(FDTC_SUPPORT == 1))||(VIDEO_R_EN)
/** @brief Update the MMU setting (cacheable/non-cacheable) of FDTC working buffer in DRAM.

@note the start address and end address should be 4KB alignment for 4K granularity cache table.
@param[in] startAddr The start address of FDTC working buffer in DRAM.
           This address should be 4K bytes alignment in 820. 4KB alignment size.
@param[in] endAddr The end address of FDTC working buffer in DRAM.
           This address should be 4K bytes alignment in 820. 4KB alignment size.
*/
void MMPF_MMU_ConfigFDTCWorkingBuffer(unsigned int startAddr, unsigned int endAddr, char cacheable)
{
    AITPS_AIC pAIC = AITC_BASE_AIC;
    MMP_ULONG ulInterruptMask = 0;
    MMP_ULONG i;
    MMP_ULONG ulSrcMbyteUnit, ulDstMbyteUnit;
    unsigned int *pTranslationTable, *pCoarsePageTable = 0;

    pTranslationTable = (unsigned int *)AT91F_ARM_ReadTTB();

    ulSrcMbyteUnit = (startAddr >> 20);
    ulDstMbyteUnit = (endAddr >> 20);

    //Disable all interrupts
    ulInterruptMask = pAIC->AIC_IMR;
    pAIC->AIC_IDCR = 0xFFFFFFFF;

    //DRAM
    if (cacheable) {
        //Find out the address of final coase page table
        for(i = ulDstMbyteUnit; i >= 0x10; i--) {
            if ((pTranslationTable[i] & 0x03) == 0x01) { //4 Kbyte page
                pCoarsePageTable = (unsigned int *)(pTranslationTable[i] & 0xFFFFFC00);
                break;
            }
        }

        if (i < 0x10) {
            RTNA_DBG_Str(0, "No coarse page table found\r\n");
            return;
        }

        if (ulSrcMbyteUnit == ulDstMbyteUnit) {
            if (ulSrcMbyteUnit != i) {   //1 Mbyte section originally
                pCoarsePageTable += 256; // calculate the address for next coarse page table

                pTranslationTable[ulSrcMbyteUnit] =
            	    ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
                	(15 << 5) |      // Domain
        	        0x11;
    	        
    	        //DRAM
            	for (i = 0; i < 256; i++)
        	        pCoarsePageTable[i] =
            	    (ulSrcMbyteUnit << 20)|
                	(i << 12)|
        	        0x332;
            }

            for (i = ((startAddr & 0xFFFFF) >> 12); i < ((endAddr & 0xFFFFF) >> 12); i++)
    	        pCoarsePageTable[i] =
        	    (ulSrcMbyteUnit << 20)|
            	(i << 12)|
    	        0x33E;
        }
        else {
            // Set start address entries as cacheable
            if ((pTranslationTable[ulSrcMbyteUnit] & 0x03) == 0x01) { //4 Kbyte section originally
                pCoarsePageTable = (unsigned int *)(pTranslationTable[ulSrcMbyteUnit] & 0xFFFFFC00);
            }
            else if ((pTranslationTable[ulSrcMbyteUnit] & 0x03) == 0x02) {  //1 Mbyte section originally
                pCoarsePageTable += 256;

                pTranslationTable[ulSrcMbyteUnit] =
            	    ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
                	(15 << 5) |      // Domain
        	        0x11;

    	        //DRAM
            	for (i = 0; i < ((startAddr & 0xFFFFF) >> 12); i++)
        	        pCoarsePageTable[i] =
            	    (ulSrcMbyteUnit << 20)|
                	(i << 12)|
        	        0x332;
            }
            else {
                RTNA_DBG_Str(0, "Unsupport!\r\n");
                return;
            }
            for (i = ((startAddr & 0xFFFFF) >> 12); i < 256; i++)
    	        pCoarsePageTable[i] =
        	    (ulSrcMbyteUnit << 20)|
            	(i << 12)|
    	        0x33E;

            // Set end address enties as cacheable
            if (ulDstMbyteUnit != i) { //1 Mbyte section originally
                pCoarsePageTable += 256;

                pTranslationTable[ulDstMbyteUnit] =
            	    ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
                	(15 << 5) |      // Domain
        	        0x11;
            }
            else {
                pCoarsePageTable = (unsigned int *)(pTranslationTable[ulDstMbyteUnit] & 0xFFFFFC00);
            }

            //DRAM
        	for (i = 0; i < ((endAddr & 0xFFFFF) >> 12); i++)
    	        pCoarsePageTable[i] =
        	    (ulDstMbyteUnit << 20)|
            	(i << 12)|
    	        0x33E;

            for (; i < 256; i++)
    	        pCoarsePageTable[i] =
        	    (ulDstMbyteUnit << 20)|
            	(i << 12)|
    	        0x332;
        }
    }
    else {
        MMPF_MMU_FlushCache(startAddr, endAddr-startAddr); //Flush cache before reset to non-cacheable
        if (ulSrcMbyteUnit == ulDstMbyteUnit) {
            pCoarsePageTable = (unsigned int *)(pTranslationTable[ulSrcMbyteUnit] & 0xFFFFFC00);
            for (i = ((startAddr & 0xFFFFF) >> 12); i < ((endAddr & 0xFFFFF) >> 12); i++)
    	        pCoarsePageTable[i] =
        	    (ulSrcMbyteUnit << 20)|
            	(i << 12)|
    	        0x332;
        }
        else {
            pCoarsePageTable = (unsigned int *)(pTranslationTable[ulSrcMbyteUnit] & 0xFFFFFC00);
            for (i = ((startAddr & 0xFFFFF) >> 12); i < 256; i++)
    	        pCoarsePageTable[i] =
        	    (ulSrcMbyteUnit << 20)|
            	(i << 12)|
    	        0x332;
    	    pCoarsePageTable = (unsigned int *)(pTranslationTable[ulDstMbyteUnit] & 0xFFFFFC00);
    	    for (i = 0; i < ((endAddr & 0xFFFFF) >> 12); i++)
    	        pCoarsePageTable[i] =
        	    (ulDstMbyteUnit << 20)|
            	(i << 12)|
                0x332;
        }
    }

    AT91F_ARM_InvalidateDTLB(); //Invalid data TLB
    pAIC->AIC_IECR = ulInterruptMask; //Turn on interrupt
}
#endif
/** @brief Update the MMU setting (cacheable/non-cacheable) of working buffer usage in DRAM.

@note the start address and end address should be 4KB alignment for 4K granularity cache table.
@param[in] startAddr The start address of working buffer in DRAM.
           This address should be 4K bytes alignment in 820. 4KB alignment size.
@param[in] endAddr The end address of working buffer in DRAM.
           This address should be 4K bytes alignment in 820. 4KB alignment size.
*/
void MMPF_MMU_ConfigWorkingBuffer(unsigned int startAddr, unsigned int endAddr, char cacheable)
{
   
    MMP_ULONG i;
    MMP_ULONG ulSrcMbyteUnit, ulDstMbyteUnit;
    unsigned int *pTranslationTable, *pCoarsePageTable = 0;
   	OS_CPU_SR   cpu_sr = 0;
    pTranslationTable = (unsigned int *)AT91F_ARM_ReadTTB();

    ulSrcMbyteUnit = (startAddr >> 20);
    ulDstMbyteUnit = (endAddr >> 20);

    OS_ENTER_CRITICAL();

    //DRAM
    if (cacheable) {
        //Find out the address of final coase page table
        if (ulSrcMbyteUnit == ulDstMbyteUnit) {
            if ((pTranslationTable[ulSrcMbyteUnit] & 0x03) == 0x01) { //4 Kbyte section originally
                pCoarsePageTable = (unsigned int *)(pTranslationTable[ulSrcMbyteUnit] & 0xFFFFFC00);
            }
            else {
				pCoarsePageTable = pCurCoarsePageTableAddr;
                pCoarsePageTable += 256; // calculate the address for next coarse page table
                pTranslationTable[ulSrcMbyteUnit] =
            	    ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
                	(15 << 5) |      // Domain
        	        0x11;
    	        
    	        //DRAM
            	for (i = 0; i < 256; i++)
        	        pCoarsePageTable[i] =
            	    (ulSrcMbyteUnit << 20)|
                	(i << 12)|
        	        0x332;		
        	    pCoarsePageUsage ++;
				pCurCoarsePageTableAddr = pCoarsePageTable;
			}
            for (i = ((startAddr & 0xFFFFF) >> 12); i < ((endAddr & 0xFFFFF) >> 12); i++)
    	        pCoarsePageTable[i] =
        	    (ulSrcMbyteUnit << 20)|
            	(i << 12)|
    	        0x33E;
        }
        else {
            // Set start address entries as cacheable
            if ((pTranslationTable[ulSrcMbyteUnit] & 0x03) == 0x01) { //4 Kbyte section originally
                pCoarsePageTable = (unsigned int *)(pTranslationTable[ulSrcMbyteUnit] & 0xFFFFFC00);
            }
            else if ((pTranslationTable[ulSrcMbyteUnit] & 0x03) == 0x02) {  //1 Mbyte section originally
				pCoarsePageTable = pCurCoarsePageTableAddr;
                pCoarsePageTable += 256; // calculate the address for next coarse page table

                pTranslationTable[ulSrcMbyteUnit] =
            	    ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
                	(15 << 5) |      // Domain
        	        0x11;


    	        //DRAM
            	for (i = 0; i < ((startAddr & 0xFFFFF) >> 12); i++)
        	        pCoarsePageTable[i] =
            	    (ulSrcMbyteUnit << 20)|
                	(i << 12)|
        	        0x332;
        	    pCoarsePageUsage ++;
				pCurCoarsePageTableAddr = pCoarsePageTable;
            }
            else {
                RTNA_DBG_Str(0, "Unsupport!\r\n");
                return;
            }
            for (i = ((startAddr & 0xFFFFF) >> 12); i < 256; i++)
    	        pCoarsePageTable[i] =
        	    (ulSrcMbyteUnit << 20)|
            	(i << 12)|
    	        0x33E;
			
            for (i = ulSrcMbyteUnit + 1; i < ulDstMbyteUnit; i++)
            {
            	// Change translation table to cache 
				if ((pTranslationTable[i] & 0x03) == 0x02) {
			        pTranslationTable[i] =
			        (i << 20) |      // Physical Address
			        (1 << 10) |      // Access in supervisor mode
			        (15 << 5) |      // Domain
			        (1 << 4) |
			        0xE;             // Set as 1 Mbyte section					
				}
            }
            // Set end address enties as cacheable
            if ((pTranslationTable[ulDstMbyteUnit] & 0x03) == 0x01) { //4 Kbyte section originally
                pCoarsePageTable = (unsigned int *)(pTranslationTable[ulDstMbyteUnit] & 0xFFFFFC00);
            }
            else if ((pTranslationTable[ulDstMbyteUnit] & 0x03) == 0x02) {  //1 Mbyte section originally
				pCoarsePageTable = pCurCoarsePageTableAddr;
                pCoarsePageTable += 256; // calculate the address for next coarse page table

                pTranslationTable[ulDstMbyteUnit] =
            	    ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
                	(15 << 5) |      // Domain
        	        0x11;
    	        //DRAM
            	for (i = 0; i < ((startAddr & 0xFFFFF) >> 12); i++)
        	        pCoarsePageTable[i] =
            	    (ulDstMbyteUnit << 20)|
                	(i << 12)|
        	        0x332;
        	    pCoarsePageUsage ++;
				pCurCoarsePageTableAddr = pCoarsePageTable;
            }
            else {
                RTNA_DBG_Str(0, "Unsupport!\r\n");
                return;
            }

            //DRAM
        	for (i = 0; i < ((endAddr & 0xFFFFF) >> 12); i++)
    	        pCoarsePageTable[i] =
        	    (ulDstMbyteUnit << 20)|
            	(i << 12)|
    	        0x33E;

            for (; i < 256; i++)
    	        pCoarsePageTable[i] =
        	    (ulDstMbyteUnit << 20)|
            	(i << 12)|
    	        0x332;
        }
    }
    else {
#if defined(ALL_FW)
    //Config mmu for SDRAM
    MMPF_MMU_FlushCache(startAddr, endAddr-startAddr); // Please update this line
    pCoarsePageTable = (MMP_ULONG *)MMU_COARSEPAGE_TABLE_ADDR;
    pTranslationTable[0x10] =
        ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
        (15 << 5) |      // Domain
        0x11;

    for (i = 0; i < 256; i++)
        pCoarsePageTable[i] =
        (0x1000000)|
        (i << 12) |
        0x332;

    //Config mmu for SDRAM
    #if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
    for (i = 0x11; i < 0x50; i++)
    #endif
    {
        pTranslationTable[i] =
        (i << 20) |      // Physical Address
        (1 << 10) |      // Access in supervisor mode
        (15 << 5) |      // Domain
        (1 << 4) |
        0x2;
    }
    MMPF_SWPACK_MmuTableInit((MMP_ULONG *)MMU_TRANSLATION_TABLE_ADDR,(MMP_ULONG *)MMU_COARSEPAGE_TABLE_ADDR);
	
#endif
    }
    MMPF_MMU_FlushAndInvalidDCache();
    AT91F_ARM_InvalidateDTLB(); //Invalid data TLB
    OS_EXIT_CRITICAL();
}
/** @} */
