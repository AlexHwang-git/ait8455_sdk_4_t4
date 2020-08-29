//==============================================================================
//
//  File        : mmpf_hif.c
//  Description : Firmware Host Interface Control Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"
/** @addtogroup MMPF_HIF
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
MMP_ULONG	glA8IntEnableFlag;
MMP_ULONG	glHostStatus = 0x0;
#ifndef HOSTCMD_STS
#define HOSTCMD_STS ((MMP_ULONG)&glHostStatus)
#endif


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
#if (PCAM_EN == 0)
//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_GetCmd
//  Description : Set the status of command, handshake between host and firmware
//------------------------------------------------------------------------------
MMP_USHORT	MMPF_HIF_GetCmd(void)
{
	//T.B.D for VSN_V2
	return 0;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_ClearCmd
//  Description : Set the status of command, handshake between host and firmware
//------------------------------------------------------------------------------
void	MMPF_HIF_ClearCmd(void)
{
	//T.B.D for VSN_V2
	return;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_SetCmdStatus
//  Description : Set the status of command, handshake between host and firmware
//------------------------------------------------------------------------------
void	MMPF_HIF_SetCmdStatus(MMP_ULONG status)
{
	glHostStatus |= status;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_ClearCmdStatus
//  Description : Clear the status of command, handshake between host and firmware
//------------------------------------------------------------------------------
void	MMPF_HIF_ClearCmdStatus(MMP_ULONG status)
{
	glHostStatus &= ~status;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_GetCmdStatus
//  Description : Get the status of command, handshake between host and firmware
//------------------------------------------------------------------------------
MMP_ULONG MMPF_HIF_GetCmdStatus(MMP_ULONG status)
{
	return (glHostStatus & status);
}

#if PCAM_EN==0
//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_CmdSetParameterB
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPF_HIF_CmdSetParameterB(MMP_UBYTE ubParamnum, MMP_UBYTE ubParamdata)
{
	//T.B.D for VSN_V2
    return;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_CmdSetParameterW
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPF_HIF_CmdSetParameterW(MMP_UBYTE ubParamnum, MMP_USHORT usParamdata)
{
	//T.B.D for VSN_V2
    return;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_CmdSetParameterL
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
void    MMPF_HIF_CmdSetParameterL(MMP_UBYTE ubParamnum, MMP_ULONG ulParamdata)
{
	//T.B.D for VSN_V2
    return;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_CmdGetParameterB
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
MMP_UBYTE    MMPF_HIF_CmdGetParameterB(MMP_UBYTE ubParamnum)
{
	//T.B.D for VSN_V2
	return 0;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_CmdGetParameterW
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
MMP_USHORT    MMPF_HIF_CmdGetParameterW(MMP_UBYTE ubParamnum)
{
	//T.B.D for VSN_V2
	return 0;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_CmdGetParameterL
//  Description : The parameter only has 24 bytes (12 words or 6 ulong int)
//------------------------------------------------------------------------------
MMP_ULONG    MMPF_HIF_CmdGetParameterL(MMP_UBYTE ubParamnum)
{
	//T.B.D for VSN_V2
	return 0;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Int_SetCpu2HostInt
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function set the cpu interrupt to the host.

The function set the cpu interrupt to the host.

  @return It reports the status of the operation.
*/
void	MMPF_HIF_SetCpu2HostInt(MMPF_HIF_INT_FLAG status) 
{
  	//T.B.D for VSN_V2
	return;
}

//------------------------------------------------------------------------------
//  Function    : MMPD_Int_SetCpu2HostIntFlag
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function set spcific cpu2host interrupt.

The function set spcific cpu2host interrupt.

  @param[in] bEnable enable or disable the cpu to host interrupt
  @param[in] flag Specify which (cpu to host) interrupt needs to be set
  @return It reports the status of the operation.
*/
void	MMPF_HIF_SetCpu2HostIntFlag(MMP_BOOL bEnable, MMPF_HIF_INT_FLAG flag) 
{
	//T.B.D for VSN_V2
	return;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_HIF_ClearCpu2HostIntFlag
//  Description : 
//------------------------------------------------------------------------------
/** @brief The function clear spcific cpu2host interrupt.

The function clear spcific cpu2host interrupt.

  @param[in] ulClearFlag Specify which (cpu to host) interrupt needs to be clear
  @return It reports the status of the operation.
*/
void	MMPF_HIF_ClearCpu2HostIntFlag(MMP_ULONG ulClearFlag) 
{
  	//T.B.D for VSN_V2
	return;
}
#endif



/** @} */ //end of MMPF_HIF