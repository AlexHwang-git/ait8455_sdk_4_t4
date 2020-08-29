//==============================================================================
//
//  File        : mmph_hif.c
//  Description : Ritian Host Interface Hardware Control Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================
///@ait_only

#ifdef BUILD_CE
#undef BUILD_FW
#endif

#define AIT_REG_FUNC_DECLARE

#include "mmp_lib.h"
#include "mmph_hif.h"
#include "mmp_reg_gbl.h"
#include "ait_bsp.h"
#ifdef BUILD_CE
#include "os_wrap.h"
#include "mmp_reg_gbl.h"
#endif

#define USE_CUSTOMER_BURST_MODE (0)
#define REG_WAIT_MMP_COUNT  6

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
static	MMPH_HIF_INTERFACE  m_hostInterface;
static	MMP_ULONG			m_ulFIFOAddress;
static	MMP_USHORT			m_usFIFOReadCount, m_usFIFOWriteCount;

#if (USE_CUSTOMER_BURST_MODE == 1)
#define CopyCount 32
extern void write_burst_data1( unsigned long *src, unsigned long *dst , long) ;
extern void read_burst_data1( unsigned long *dst, unsigned long *src , long) ;
extern void GTL_MMICIF_device_request(void);
MMP_ULONG Burstflag = 0;
#endif

extern MMP_ULONG glHostStatus;
#ifndef HOSTCMD_STS
#define HOSTCMD_STS ((MMP_ULONG)&glHostStatus)
#endif
//==============================================================================
//
//                               FUNCTION PROTOTYPES
//
//==============================================================================
/** @addtogroup MMPH_HIF
@{
*/

#pragma O0
//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_SetInterface
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPH_HIF_SetInterface(MMPH_HIF_INTERFACE mode)
{

    m_hostInterface = mode;

    MMPH_HIF_RegGetB = Ind_RegGetB;
    MMPH_HIF_RegGetW = Ind_RegGetW;
    MMPH_HIF_RegGetL = Ind_RegGetL;
    MMPH_HIF_RegSetB = Ind_RegSetB;
    MMPH_HIF_RegSetW = Ind_RegSetW;
    MMPH_HIF_RegSetL = Ind_RegSetL;

	MMPH_HIF_RegFGetW0 = Ind_RegFGetW0;
	MMPH_HIF_RegFGetW1 = Ind_RegFGetW1;
	MMPH_HIF_RegFSetW0 = Ind_RegFSetW0;
	MMPH_HIF_RegFSetW1 = Ind_RegFSetW1;

    MMPH_HIF_MemGetB = Ind_MemGetB;
    MMPH_HIF_MemGetW = Ind_MemGetW;
	MMPH_HIF_MemGetL = Ind_MemGetL;
    MMPH_HIF_MemSetB = Ind_MemSetB;
    MMPH_HIF_MemSetW = Ind_MemSetW;
	MMPH_HIF_MemSetL = Ind_MemSetL;

    MMPH_HIF_FIFOOpenR = Ind_FIFOOpenR;
    MMPH_HIF_FIFOOpenW = Ind_FIFOOpenW;
    MMPH_HIF_FIFOReadW = Ind_FIFOReadW;
    MMPH_HIF_FIFOWriteW = Ind_FIFOWriteW;
    MMPH_HIF_FIFOCReadW = Ind_FIFOCReadW;
    MMPH_HIF_FIFOCWriteW = Ind_FIFOCWriteW;
    MMPH_HIF_ClearIrq = Ind_ClearIrq;

    return  MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_GetInterface
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPH_HIF_GetInterface(MMPH_HIF_INTERFACE *mode)
{
    if (m_hostInterface == 0)
        return  MMP_HIF_ERR_MODE_NOT_SET;

    *mode = m_hostInterface;

    return  MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegGetB
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE   Ind_RegGetB(MMP_ULONG ulAddr)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	return *(AIT_REG_B*)(pReg+ulAddr);
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegGetW
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT   Ind_RegGetW(MMP_ULONG ulAddr)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	return *(MMP_USHORT*)(pReg+ulAddr);
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegGetL
//  Description :
//------------------------------------------------------------------------------
MMP_ULONG   Ind_RegGetL(MMP_ULONG ulAddr)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	return *(AIT_REG_D*)(pReg+ulAddr);
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegSetB
//  Description :
//------------------------------------------------------------------------------
void    Ind_RegSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	*(AIT_REG_B*)(pReg+ulAddr) = ubData;
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegSetW
//  Description :
//------------------------------------------------------------------------------
void    Ind_RegSetW(MMP_ULONG ulAddr, MMP_USHORT usData)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	*(MMP_USHORT*)(pReg+ulAddr) = usData;
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegSetL
//  Description :
//------------------------------------------------------------------------------
void    Ind_RegSetL(MMP_ULONG ulAddr, MMP_ULONG ulData)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	*(AIT_REG_D*)(pReg+ulAddr) = ulData;
}

//------------------------------------------------------------------------------
//  Function    : Ind_MemGetB
//  Description :
//------------------------------------------------------------------------------
MMP_UBYTE   Ind_MemGetB(MMP_ULONG ulAddr)
{
	MMP_UBYTE *pMem = (MMP_UBYTE *)0;
	return *(MMP_UBYTE*)(pMem+ulAddr);
}

//------------------------------------------------------------------------------
//  Function    : Ind_MemGetW
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT   Ind_MemGetW(MMP_ULONG ulAddr)
{
	MMP_UBYTE *pMem = (MMP_UBYTE *)0;
	return *(MMP_USHORT*)(pMem+ulAddr);
}

//------------------------------------------------------------------------------
//  Function    : Ind_MemGetL
//  Description : 
//------------------------------------------------------------------------------
MMP_ULONG   Ind_MemGetL(MMP_ULONG ulAddr)
{
	MMP_UBYTE *pMem = (MMP_UBYTE *)0;
	return *(MMP_ULONG*)(pMem+ulAddr);
}

//------------------------------------------------------------------------------
//  Function    : Ind_MemSetB
//  Description :
//------------------------------------------------------------------------------
void    Ind_MemSetB(MMP_ULONG ulAddr, MMP_UBYTE ubData)
{
	MMP_UBYTE *pMem = (MMP_UBYTE *)0;
	*(MMP_UBYTE*)(pMem+ulAddr) = ubData;
}

//------------------------------------------------------------------------------
//  Function    : Ind_MemSetW
//  Description :
//------------------------------------------------------------------------------
void    Ind_MemSetW(MMP_ULONG ulAddr, MMP_USHORT usData)
{
	MMP_UBYTE *pMem = (MMP_UBYTE *)0;
	*(MMP_USHORT*)(pMem+ulAddr) = usData;
}

//------------------------------------------------------------------------------
//  Function    : Ind_MemSetL
//  Description : 
//------------------------------------------------------------------------------
void    Ind_MemSetL(MMP_ULONG ulAddr, MMP_ULONG ulData)
{
	MMP_UBYTE *pMem = (MMP_UBYTE *)0;
	*(MMP_ULONG*)(pMem+ulAddr) = ulData;
}

//------------------------------------------------------------------------------
//  Function    : Ind_FIFOOpenR
//  Description :
//------------------------------------------------------------------------------
void    Ind_FIFOOpenR(MMP_ULONG ulFifoaddr, MMP_USHORT usReadcount)
{
	m_ulFIFOAddress = ulFifoaddr;
	m_usFIFOReadCount = usReadcount;
	return;
}


//------------------------------------------------------------------------------
//  Function    : Ind_FIFOOpenW
//  Description :
//------------------------------------------------------------------------------
void    Ind_FIFOOpenW(MMP_ULONG ulFifoaddr, MMP_USHORT usWritecount)
{
	m_ulFIFOAddress = ulFifoaddr;
	m_usFIFOWriteCount = usWritecount;
	return;
}

//------------------------------------------------------------------------------
//  Function    : Ind_FIFOCReadW
//  Description : 
//------------------------------------------------------------------------------
void    Ind_FIFOCReadW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usReadcount)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	
	while (usReadcount--) {
	    *usData++ = *(MMP_USHORT*)(pReg+ulFifoaddr);						
	}
	
	return;
}

//------------------------------------------------------------------------------
//  Function    : Ind_FIFOCWriteW
//  Description : 
//------------------------------------------------------------------------------
void    Ind_FIFOCWriteW(MMP_ULONG ulFifoaddr, MMP_USHORT *usData, MMP_USHORT usWritecount)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	
	while (usWritecount--) {
	    *(MMP_USHORT*)(pReg+ulFifoaddr) = *usData++;		
	}

    return;    
}

//------------------------------------------------------------------------------
//  Function    : Ind_FIFOReadW
//  Description :
//------------------------------------------------------------------------------
MMP_USHORT    Ind_FIFOReadW(void)
{
    MMP_USHORT	value;
	AIT_REG_B* 	pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	
	value = *(MMP_USHORT*)(pReg + m_ulFIFOAddress);						
	
	return value;
}

//------------------------------------------------------------------------------
//  Function    : Ind_FIFOWriteW
//  Description :
//------------------------------------------------------------------------------
void    Ind_FIFOWriteW(MMP_USHORT usData)
{
	AIT_REG_B* pReg = (AIT_REG_B*)AITC_BASE_OPR; 
	*(MMP_USHORT*)(pReg + m_ulFIFOAddress) = usData;
	
    return;
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegFGetW0
//  Description : 
//------------------------------------------------------------------------------
MMP_USHORT   Ind_RegFGetW0(void)
{
    MMP_USHORT  value;
	value = Ind_RegGetW(0x80008c80);
    return  value;    
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegFGetW1
//  Description : 
//------------------------------------------------------------------------------
MMP_USHORT   Ind_RegFGetW1(void)
{
    MMP_USHORT  value;
	value = Ind_RegGetW(0x80008c84);
    return  value;    
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegFSetW0
//  Description : 
//------------------------------------------------------------------------------
void   Ind_RegFSetW0(MMP_USHORT usData)
{
	Ind_RegSetW(0x80008c80, usData);
}

//------------------------------------------------------------------------------
//  Function    : Ind_RegFSetW1
//  Description : 
//------------------------------------------------------------------------------
void   Ind_RegFSetW1(MMP_USHORT usData)
{
	Ind_RegSetW(0x80008c84, usData);
}

//------------------------------------------------------------------------------
//  Function    : Ind_ClearIrq
//  Description : 
//------------------------------------------------------------------------------
void    Ind_ClearIrq(void)
{
    return;    
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_MemCopyDevToHost
//  Description :
//------------------------------------------------------------------------------
void    MMPH_HIF_MemCopyDevToHost(MMP_UBYTE *ubDestptr, MMP_ULONG ulSrcaddr,
                MMP_ULONG ulLength)
{
	memcpy((MMP_UBYTE *)ubDestptr, (MMP_UBYTE *)ulSrcaddr, ulLength);

	return;
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_MemCopyHostToDev
//  Description :
//------------------------------------------------------------------------------
void	MMPH_HIF_MemCopyHostToDev(MMP_ULONG ulDestaddr, MMP_UBYTE *usSrcaddr,
                MMP_ULONG ulLength)
{
	memcpy((MMP_UBYTE *)ulDestaddr, usSrcaddr, ulLength);

	return;
}

extern MMPF_OS_FLAGID   SYS_Flag_Hif;
extern MMPF_OS_FLAGID 	DSC_UI_Flag;
extern MMPF_OS_SEMID    SYS_Sem_CommandSend;
#if PCAM_EN==0


void MMPF_HIF_HifISR(void);

MMP_ERR    MMPH_HIF_CmdSend(MMP_USHORT usCommand)
{
	MMPF_OS_FLAGS flags;
	MMP_USHORT    semCount;
	
	MMPF_OS_AcquireSem(SYS_Sem_CommandSend, 0);
	//RTNA_DBG_Str(3, "MMPH_HIF_CmdSend Start\r\n");
    MMPH_HIF_RegSetW(HOSTCMD_CMD, usCommand);
    
    //while (MMPH_HIF_RegGetW(HOSTCMD_CMD));
    //printf("Command Group=%x  %x\r\n", (usCommand & GRP_MASK), usCommand);
    
	switch (usCommand & GRP_MASK) {
	case GRP_USR:
    	MMPF_HIF_HifISR();
    	
        MMPF_OS_WaitFlags(DSC_UI_Flag, SYS_FLAG_FS_CMD_DONE,
			MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
		break;
		
	case GRP_AUD:
    	MMPF_HIF_HifISR();
        MMPF_OS_WaitFlags(DSC_UI_Flag, SYS_FLAG_AUD_CMD_DONE,
			MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
		break;
		
	case GRP_DSC:
    	MMPF_HIF_HifISR();
        MMPF_OS_WaitFlags(DSC_UI_Flag, SYS_FLAG_DSC_CMD_DONE,
			MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
		break;
		
	case GRP_SYS:
		MMPF_HIF_HifISR();
        MMPF_OS_WaitFlags(DSC_UI_Flag, SYS_FLAG_SYS_CMD_DONE,
			MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
		break;
		
	case GRP_USB:
		bWaitForUsbCommandDone=TRUE;
		MMPF_HIF_HifISR();
        MMPF_OS_WaitFlags(DSC_UI_Flag, SYS_FLAG_USB_CMD_DONE,
			MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
		bWaitForUsbCommandDone=FALSE;
		break;
		
	case GRP_VID:
		MMPF_HIF_HifISR();
        MMPF_OS_WaitFlags(DSC_UI_Flag, SYS_FLAG_VID_CMD_DONE,
			MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
		break;
		
	case GRP_SENSOR:
		MMPF_HIF_HifISR();
        MMPF_OS_WaitFlags(DSC_UI_Flag, SYS_FLAG_SENSOR_CMD_DONE,
			MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
		break;

	default:
		break;
	}
	
	
	MMPF_OS_ReleaseSem(SYS_Sem_CommandSend);
	
	// FH
	MMPF_OS_QuerySem(SYS_Sem_HifSetParameter, &semCount);
	
	
	if (semCount == 0) {
    	MMPF_OS_ReleaseSem(SYS_Sem_HifSetParameter);
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdSetParameterB
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPH_HIF_CmdSetParameterB(MMP_UBYTE ubParamnum, MMP_UBYTE ubParamdata)
{
    // FH
    if (ubParamnum == 0)
    {
        MMP_UBYTE status;
        status = MMPF_OS_AcquireSem(SYS_Sem_HifSetParameter, 100);
        if (status != 0)
        {
            status = 0;
        }
    }

    MMPH_HIF_RegSetB(HOSTCMD_PARM + ubParamnum, ubParamdata);

    return;
}


//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdSetParameterW
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPH_HIF_CmdSetParameterW(MMP_UBYTE ubParamnum, MMP_USHORT usParamdata)
{
    // FH
    if (ubParamnum == 0)
    {
        MMP_UBYTE status;
        status = MMPF_OS_AcquireSem(SYS_Sem_HifSetParameter, 100);
        if (status != 0)
        {
            status = 0;
        }
    }
    
    MMPH_HIF_RegSetW(HOSTCMD_PARM + ubParamnum, usParamdata);

    return;
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdSetParameterL
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPH_HIF_CmdSetParameterL(MMP_UBYTE ubParamnum, MMP_ULONG ulParamdata)
{
    // FH
    if (ubParamnum == 0)
    {
        MMP_UBYTE status;
        status = MMPF_OS_AcquireSem(SYS_Sem_HifSetParameter, 100);
        if (status != 0)
        {
            status = 0;
        }
    }
    
    MMPH_HIF_RegSetL(HOSTCMD_PARM + ubParamnum, ulParamdata);

    return;
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdGetParameterB
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
MMP_UBYTE    MMPH_HIF_CmdGetParameterB(MMP_UBYTE ubParamnum)
{
    return     MMPH_HIF_RegGetB(HOSTCMD_PARM + ubParamnum);
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdGetParameterW
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
MMP_USHORT    MMPH_HIF_CmdGetParameterW(MMP_UBYTE ubParamnum)
{
    return     MMPH_HIF_RegGetW(HOSTCMD_PARM + ubParamnum);
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdGetParameterL
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
MMP_ULONG    MMPH_HIF_CmdGetParameterL(MMP_UBYTE ubParamnum)
{
    return     MMPH_HIF_RegGetL(HOSTCMD_PARM + ubParamnum);
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdGetStatusL
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
MMP_ULONG    MMPH_HIF_CmdGetStatusL(void)
{
    return     MMPH_HIF_RegGetL(HOSTCMD_STS);
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdSetStatusL
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPH_HIF_CmdSetStatusL(MMP_ULONG status)
{
    MMPH_HIF_RegSetL(HOSTCMD_STS, MMPH_HIF_RegGetL(HOSTCMD_STS)|status);
    
    return;     
}

//------------------------------------------------------------------------------
//  Function    : MMPH_HIF_CmdClearStatusL
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPH_HIF_CmdClearStatusL(MMP_ULONG status)
{
    MMPH_HIF_RegSetL(HOSTCMD_STS, MMPH_HIF_RegGetL(HOSTCMD_STS)&(~status));
    
    return;     
}


/// @}
#pragma
///@end_ait_only

#ifdef BUILD_CE
#define BUILD_FW
#endif

