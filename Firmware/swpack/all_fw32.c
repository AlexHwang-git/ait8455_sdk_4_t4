//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : audio_p_fw.c32
//* Object              : Config System Software Environment by Firmware Type
//* Creation            : Philip Lin at 2008/7/2 17:53
//*
//*----------------------------------------------------------------------------
#include "includes_fw.h"
#include "mmpf_task_cfg.h"
extern OS_STK           DSC_Task_Stk[];
extern OS_STK           SENSOR_Task_Stk[];
extern OS_STK           AUDIO_Record_Task_Stk[];
extern OS_STK           AUDIO_Play_Task_Stk[];
extern OS_STK           AUDIO_WriteFile_Task_Stk[];
extern OS_STK           FDTC_Task_Stk[];
extern OS_STK           USB_Task_EP1_Stk[];
extern OS_STK           VIDPLAY_Task_Stk[];
extern OS_STK           PARSR3GP_Task_Stk[];
extern OS_STK           VIDDEC_Task_Stk[];
extern OS_STK           MP4VENC_Task_Stk[];
extern OS_STK           VIDMGR_Task_Stk[];
extern OS_STK           FS_Task_Stk[];
#if (APP_EN)
extern OS_STK           APP_Task_Stk[];
#endif
extern OS_STK           PCAM_Task_Stk[];
//#if WATCHDOG_RESET_EN==1
//extern OS_STK           LTASK_Task_Stk[];
//#endif
#if USB_UVC15==1
extern OS_STK           USB_Task_EP2_Stk[];
#endif

extern unsigned int Image$$ALL_DRAM$$ZI$$Limit;
extern unsigned char pCoarsePageUsage;

/** @addtogroup BSP
@{
*/

//*----------------------------------------------------------------------------
//* \fn    MMPF_SWPACK_MmuInit
//* \brief Application specific MMU configurations for SRAM and DRAM
//*----------------------------------------------------------------------------
extern unsigned int *pCurCoarsePageTableAddr;
void    MMPF_SWPACK_MmuTableInit(unsigned int *pTranslationTable,unsigned int *pCoarsePageTable)
{
    MMP_ULONG   i, endtab, j, ulMBs;
#if PCAM_EN==0
	ulMBs = ((MMP_ULONG)&Image$$ALL_DRAM$$ZI$$Limit) +  MUON_RSVD_HEAP_SIZE - 0x1000000;
#else
	ulMBs = ((MMP_ULONG)&Image$$ALL_DRAM$$ZI$$Limit)  - 0x1000000;

#endif	
	ulMBs = ulMBs/0x100000;
	for(i = 0; i < ulMBs; i++){
		pTranslationTable[0x10+i] =
	        ((0x10+i) << 20) |      // Physical Address
	        (1 << 10) |      // Access in supervisor mode
	        (15 << 5) |      // Domain
	        (1 << 4) |
	        0xE;             // Set as 1 Mbyte section
	}

	pTranslationTable[0x10+i] =
    ((unsigned int)pCoarsePageTable&0xFFFFFC00) |  // Physical Address
    (15 << 5) |      // Domain
	0x11;

#if PCAM_EN==0
	endtab = (((MMP_ULONG)&Image$$ALL_DRAM$$ZI$$Limit + MUON_RSVD_HEAP_SIZE + 0xFFF - (0x1000000+i*0x100000)) >> 12);
#else
	endtab = (((MMP_ULONG)&Image$$ALL_DRAM$$ZI$$Limit + 0xFFF - (0x1000000+i*0x100000)) >> 12);
#endif		
	//DRAM
    for (j = 0; j < 256; j++)
	   	pCoarsePageTable[j] =
    	(0x1000000 + i*0x100000)|
        (j << 12)|
	    0x332;

    for (j = 0; j < endtab; j++)
	    pCoarsePageTable[j] =
    	(0x1000000 + i*0x100000)|
        (j << 12)|
	    0x33E; 
    pCoarsePageUsage = 1;
    pCurCoarsePageTableAddr = pCoarsePageTable;

}

//*----------------------------------------------------------------------------
//* \fn    MMPF_SWPACK_TaskInit
//* \brief Application specific MMU configurations for SRAM and DRAM
//*----------------------------------------------------------------------------
void    MMPF_SWPACK_TaskInit()
{
extern MMP_ULONG MMU_TRANSLATION_TABLE_ADDR,MMU_COARSEPAGE_TABLE_ADDR ;
    MMPF_TASK_CFG   task_cfg;
    MMP_UBYTE ret;

    #if !(defined(MBOOT_FW)||defined(UPDATER_FW))
    MMPF_InitMMU((MMP_ULONG *)MMU_TRANSLATION_TABLE_ADDR,(MMP_ULONG *)MMU_COARSEPAGE_TABLE_ADDR);
    #endif

    #if PCAM_EN==0
    #if !defined(UPDATER_FW)
	task_cfg.ubPriority = TASK_HIGH_PRIO;
    task_cfg.pbos = (MMP_ULONG)&SYS_High_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&SYS_High_Stk[TASK_HIGH_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_HIGH_TaskHandler, &task_cfg, (void *)0);
    #endif
    #endif

    #if	1 
    task_cfg.ubPriority = TASK_VIDMGR_PRIO;
    task_cfg.pbos = (MMP_ULONG)&VIDMGR_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&VIDMGR_Task_Stk[VIDMGR_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_VIDMGR_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_VIDMGR", &ret);
    #endif    
    
    #if SUPPORT_UAC // (AUDIO_R_EN)||(VAMR_R_EN)||(VAAC_R_EN)
    task_cfg.ubPriority = TASK_AUDIO_RECORD_PRIO;
    task_cfg.pbos = (INT32U)&AUDIO_Record_Task_Stk[0];
    task_cfg.ptos = (INT32U)&AUDIO_Record_Task_Stk[AUDIO_RECORD_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_AUDIORECORD_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_AUDIORECORD", &ret);
    #endif

    #if (USB_UVC15==1) &&(SINGLE_STREAMING_TASK==0)
    task_cfg.ubPriority = TASK_USB_EP2_PRIO;
    task_cfg.pbos = (INT32U)&USB_Task_EP2_Stk[0];
    task_cfg.ptos = (INT32U)&USB_Task_EP2_Stk[USB_EP2_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_USB_EP2_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_USBEP2", &ret);
    #endif


    #if (AUDIO_P_EN)||(VAMR_P_EN)||(VAAC_P_EN)
    task_cfg.ubPriority = TASK_AUDIO_PLAY_PRIO;
    task_cfg.pbos = (INT32U)&AUDIO_Play_Task_Stk[0];
    task_cfg.ptos = (INT32U)&AUDIO_Play_Task_Stk[AUDIO_PLAY_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_AUDIOPLAY_TaskHandler, &task_cfg, (void *)0);
    #endif

    #if (VIDEO_P_EN)
    task_cfg.ubPriority = TASK_3GPPARSR_PRIO;
    task_cfg.pbos = (MMP_ULONG)&PARSR3GP_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&PARSR3GP_Task_Stk[DEMUXER_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_3GPPARSR_TaskHandler, &task_cfg, (void *)0);

    task_cfg.ubPriority = TASK_VIDDEC_PRIO;
    task_cfg.pbos = (MMP_ULONG)&VIDDEC_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&VIDDEC_Task_Stk[DECODER_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_VIDDEC_TaskHandler, &task_cfg, (void *)0);

    task_cfg.ubPriority = TASK_VIDPLAY_PRIO;
    task_cfg.pbos = (MMP_ULONG)&VIDPLAY_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&VIDPLAY_Task_Stk[PLAYER_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_VIDPLAY_TaskHandler, &task_cfg, (void *)0);
    #endif 

    #if (DSC_R_EN)||(DSC_P_EN)
    task_cfg.ubPriority = TASK_DSC_PRIO;
    task_cfg.pbos = (MMP_ULONG)&DSC_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&DSC_Task_Stk[TASK_DSC_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_DSC_TaskHandler, &task_cfg, (void *)0);
    #endif

    #if (SENSOR_EN)
    task_cfg.ubPriority = TASK_SENSOR_PRIO;
    task_cfg.pbos = (MMP_ULONG)&SENSOR_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&SENSOR_Task_Stk[TASK_SENSOR_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_SENSOR_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_SENSOR", &ret);
    
    #endif

    #if (VIDEO_R_EN)//&&(PCAM_EN==0)
    task_cfg.ubPriority = TASK_MP4VENC_PRIO;
    task_cfg.pbos = (MMP_ULONG)&MP4VENC_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&MP4VENC_Task_Stk[MP4VENC_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_MP4VENC_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_MP4VENC", &ret);
    #elif defined(ALL_FW) 
    MMPF_VIDENC_TaskInit();
    #endif
    
    #if 1//(USB_EN)
    task_cfg.ubPriority = TASK_USB_EP1_PRIO;
    task_cfg.pbos = (MMP_ULONG)&USB_Task_EP1_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&USB_Task_EP1_Stk[USB_EP1_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_USB_EP1_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_USBEP1", &ret);
    #endif


    #if ((AUDIO_P_EN)&&(AUDIO_STREAMING_EN == 1))||(AUDIO_R_EN)
    task_cfg.ubPriority = TASK_AUDIO_STREAM_PRIO;
    task_cfg.pbos = (INT32U)&AUDIO_WriteFile_Task_Stk[0];
    task_cfg.ptos = (INT32U)&AUDIO_WriteFile_Task_Stk[AUDIO_WRITEFILE_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_AUDIOSTREAMING_TaskHandler, &task_cfg, (void *)0);
    #endif

    #if (APP_EN)
    task_cfg.ubPriority = TASK_APP_PRIO;
    task_cfg.pbos = (INT32U)&APP_Task_Stk[0];
    task_cfg.ptos = (INT32U)&APP_Task_Stk[APP_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_APP_TaskHandler, &task_cfg, (void *)0);
    #endif
    
    #if (FS_EN)
    task_cfg.ubPriority = TASK_FS_PRIO;
    task_cfg.pbos = (INT32U)&FS_Task_Stk[0];
    task_cfg.ptos = (INT32U)&FS_Task_Stk[FS_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_FS_TaskHandler, &task_cfg, (void *)0);
    #endif
    
    #if PCAM_EN
    task_cfg.ubPriority = TASK_PCAM_PRIO;
    task_cfg.pbos = (MMP_ULONG)&PCAM_Task_Stk[0];
    task_cfg.ptos = (MMP_ULONG)&PCAM_Task_Stk[TASK_PCAM_STK_SIZE-1];
    MMPF_OS_CreateTask(MMPF_PCAM_TaskHandler, &task_cfg, (void *)"PCAM");
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_PCAM", &ret);
    #endif
    
    #if WATCHDOG_RESET_EN
    /*
    task_cfg.ubPriority = (OS_LOWEST_PRIO - 2);
    task_cfg.pbos = (INT32U)&LTASK_Task_Stk[0];
    task_cfg.ptos = (INT32U)&LTASK_Task_Stk[LTASK_STK_SIZE - 1];
    MMPF_OS_CreateTask(MMPF_LTASK_TaskHandler, &task_cfg, (void *)0);
    OSTaskNameSet(task_cfg.ubPriority, (INT8U *)"AIT_DOG", &ret);
    */
    #endif
    
}
/** @} */ /* End of BSP */
