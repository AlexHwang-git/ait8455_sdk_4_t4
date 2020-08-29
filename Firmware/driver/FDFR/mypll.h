/*
 * mypll.h
 */

#define	PLL_MAGIC_ID	0xa5a5feed
typedef MMP_ERR (*SETUP_PLL)(void);
typedef struct _PLL_DATA {
	long			magic;
	SETUP_PLL		setupll;
    MMPF_PLL_NO		CPUSrc;
	MMPF_PLL_NO		GroupSrc[7];
	MMPF_PLL_SRC	PLLSrc[5]; 
	MMPF_PLL_FREQ	PLLFreq[5];	
	MMP_ULONG		CPUFreq;
} PLL_DATA;

#define	PLL_DATA_ADDR	(PLL_DATA*)(0x1400)