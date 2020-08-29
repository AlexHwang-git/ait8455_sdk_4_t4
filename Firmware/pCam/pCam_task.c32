#include "mmp_lib.h"
#include "os_wrap.h"
//#include "mmps_usb.h"
#include "pcam_msg.h"

//extern void *MMPF_SYS_AllocFB(char *tag,MMP_ULONG size, MMP_USHORT align);
extern void MMPF_PCAM_Task(void) ;
static void MMPF_PCAM_Init(void) ;
static void MMPF_Init_DefaultClock(void);

MMPF_OS_FLAGID PCAM_Flag;
//MMPF_OS_SEMID  PCAM_Sem = 0xFF ;
OS_STK PCAM_Task_Stk[TASK_PCAM_STK_SIZE];

void MMPF_PCAM_Init(void)
{
 //   MMP_ULONG cur_buf = 0 ,cur_3abuf_size ;
   // MMP_ERR err ;
    // Create Flags
    PCAM_Flag = MMPF_OS_CreateEventFlagGrp(0);
    // Create Sem
    //PCAM_Sem = MMPF_OS_CreateSem(0);
    // Create Msg Block
    pcam_init_msg_queue(); 
    // Init Frame buffer Allocation
    //MMPF_SYS_InitFB();//Gason
    
}

void MMPF_PCAM_TaskHandler(void *p_arg)
{
    MMPF_PCAM_Init();
#if OS_TASK_STAT_EN > 0
    OSStatInit() ;
#endif
    MMPF_PCAM_Task();
}

