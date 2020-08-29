//==============================================================================
//
//  File        : lib_retina.h
//  Description : Retina function library
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================
#include "general.h"
#include "includes_fw.h"
#include "reg_retina.h"
#include "lib_retina.h"
#include "config_fw.h"

#include "mmpf_uart.h"
#include "mmp_reg_vif.h"
#include "stdarg.h"

void dbg_printf(unsigned long level, char *fmt, ...);
/** @addtogroup BSP
@{
*/
extern MMP_ULONG RTNA_CPU_CLK_M ; // move to ait_bsp.c
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

// Common API

MMP_ULONG   dbg_level   = DBG_LEVEL;


//==============================================================================
//
//                              DBG Functions
//
//==============================================================================
void delay_1us(MMP_ULONG us)
{
    RTNA_WAIT_US(us);
}

void delay_1ms(MMP_ULONG ms)
{
    RTNA_WAIT_MS(ms);
}

MMP_ULONG RTNA_DBG_Level(MMP_ULONG new_level)
{
    MMP_ULONG old_level = dbg_level ;
    dbg_level = new_level ;
    return old_level ;
}
//------------------------------------------------------------------------------
//  Function    : RTNA_DBG_Open
//  Description : Initialize and enable RTNA debug output
//------------------------------------------------------------------------------
#if UART_DBG_EN==1
#if (DUAL_UART_DBG_EN == 1)
void RTNA_DBG_Open(MMP_ULONG fclk, MMP_ULONG baud, MMP_USHORT uart_set, MMP_USHORT uartpad, MMP_BOOL bTxEnable, MMP_BOOL bRxEnable)
{
    MMPF_UART_ATTRIBUTE uartattribute;
    
    uartattribute.padset = uartpad;
    uartattribute.ulMasterclk = fclk * 1000000;
    uartattribute.ulBaudrate = baud;
    uartattribute.ubTxEnable = bTxEnable;
    uartattribute.ubRxEnable = bRxEnable;
	MMPF_Uart_Open(uart_set, &uartattribute);
}

void dbg_uartout(char *fmt, ...)
{
	#if defined(ALL_FW)&&(UART_DBG_EN == 0x1)
	extern int _vsprintf(char *str, const char *format, va_list ap);
	S32			n;
	S8			buf[80];
	va_list		args;
	
	va_start(args, fmt);
	n = _vsprintf((char*)buf, fmt, args);
	va_end(args);

	MMPF_Uart_Write(1, (char *)buf, _strlen((char*)&buf));
	#endif
}
#else
void RTNA_DBG_Open(MMP_ULONG fclk, MMP_ULONG baud)
{
    MMPF_UART_ATTRIBUTE uartattribute;
    
    uartattribute.padset = DEBUG_UART_PIN;
    uartattribute.ulMasterclk = fclk * 1000000;
    uartattribute.ulBaudrate = baud;

	MMPF_Uart_Open(DEBUG_UART_NUM, &uartattribute);
}
#endif
//-------------------------------------------------------------------------------------------------------------------//
//-	Function 	: dbg_printf(U32 dbg_level, S8 *fmt, ...)															-//
//-	Description	: New message												-//
//-	Parameters	: None																								-//
//- Return		: The start address of NVRAM area.																	-//
//-------------------------------------------------------------------------------------------------------------------//
void dbg_printf(unsigned long level, char *fmt, ...)
{
	S32			n;
	S8			buf[128];
	va_list		args;
	
    if (level > dbg_level)
        return;


	va_start(args, fmt);
	n = _vsprintf(buf, fmt, args);
	va_end(args);
    if(_strlen(buf) >= ( sizeof(buf) - 1 ) ) {
        dbg_printf(3,"[Crash] : dbg_printf *out of buf*\r\n");
        while(1);
    }

	MMPF_Uart_Write(DEBUG_UART_NUM, (char *)buf, _strlen(buf));
}



//------------------------------------------------------------------------------
//  Function    : RTNA_DBG_Str
//  Description : Debug output a string
//------------------------------------------------------------------------------

void RTNA_DBG_Str(MMP_ULONG level, char *str)
{
#if 1

	dbg_printf(level, (char *)"%s", str);
#else
    int		size;
    char	*pchar;
    
    if (level > dbg_level) {
        return;
    }

    size = 0;
    pchar = str;
    while(*pchar++ != 0) {
        size++;
    }

	MMPF_Uart_Write(DEBUG_UART_NUM, str, size);
#endif
}

#if 0
//------------------------------------------------------------------------------
//  Function    : RTNA_DBG_Hex
//  Description : Debug output a hex format number
//------------------------------------------------------------------------------

void RTNA_DBG_Hex(MMP_ULONG level, MMP_ULONG val, int bit)
{
    char str[16];
    int i;

//#if OS_CRITICAL_METHOD == 3
//    OS_CPU_SR   cpu_sr = 0;
//#endif

    if (level > dbg_level) {
        return;
    }

//    OS_ENTER_CRITICAL();

    str[0] = '0';
    str[1] = 'x';

    for (i = (bit>>2); i > 0; i--) {
        if ((val & 0xF) >= 10) {
            str[i+1] = (val & 0xF) - 10 + 'A';
        }
        else {
            str[i+1] = (val & 0xF) + '0';
        }
        val >>= 4;
    }

    str[2+(bit>>2)] = 0;

    RTNA_DBG_Str(level, str);

//    OS_EXIT_CRITICAL();
}
#endif

/** @brief Print the number via UART

@post The UART is initialized in current CHIP frequency.
@param[in] val The value to be printeded.
@param[in] digits The number of digits to be printed in hexadecimal or decimal. If the digits is > 0,
                  use hexadecimal output, otherwise it's a decimal output.
@remarks When outputting a decimal, the val is always be regarded as a signed value.
         When outputting a hexadecimal, the val is always be regarded as an unsigned value.
@note Using signed digits would cause confusing sometimes but it could save extra 4 bytes each calling.
*/// 108 bytes, extra 38 bytes to RTNA_DBG_Hex
void MMPF_DBG_Int(MMP_ULONG val, MMP_SHORT digits)
{
#if 0
	dbg_printf(0, (S8 *)"%x", val);
#else
    char str[12];
    MMP_USHORT i;
    MMP_SHORT base;
    MMP_SHORT stopAt;

    str[0] = ' ';
    str[1] = 'x';
    if (digits < 0) {
        base = 10;
        digits = -digits;
        stopAt = 1;
        if (val > 0x80000000) {
            val = -val;
            str[0] = '-';
        }
    }
    else {
        base = 16;
        stopAt = 2;
    }

    for (i = digits + stopAt - 1; i >= stopAt; i--) {
        MMP_USHORT num;
        num = val % base;
        if (num >= 10) {
            str[i] = num + ('A' - 10);
        }
        else {
            str[i] = num + '0';
        }
        val /= base;
    }
    str[stopAt + digits] = '\0';

    RTNA_DBG_Str(0, str);
#endif
}

//------------------------------------------------------------------------------
//  Function    : RTNA_DBG_Long
//  Description : Debug output a long number
//------------------------------------------------------------------------------

void RTNA_DBG_Long(MMP_ULONG level, MMP_ULONG val)
{
    if (level <= dbg_level) {
        MMPF_DBG_Int(val, 8);
    }
}


//------------------------------------------------------------------------------
//  Function    : RTNA_DBG_Short
//  Description : Debug output a short number
//------------------------------------------------------------------------------

void RTNA_DBG_Short(MMP_ULONG level, MMP_USHORT val)
{
    if (level <= dbg_level) {
        MMPF_DBG_Int(val, 4);
    }
}


//------------------------------------------------------------------------------
//  Function    : RTNA_DBG_Byte
//  Description : Debug output a byte number
//------------------------------------------------------------------------------

void RTNA_DBG_Byte(MMP_ULONG level, MMP_UBYTE val)
{
    if (level <= dbg_level) {
        MMPF_DBG_Int(val, 2);
    }
}
#endif

//==============================================================================
//
//                              MISC. Functions
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : RTNA_Wait_Count
//  Description : RTNA delay loop function
//------------------------------------------------------------------------------

//[jerry] it may not a ciritical function, call it by function.
// It's about 4 cycle one loop in ARM mode
#pragma O0
void RTNA_Wait_Count(MMP_ULONG count)
{
    while (count--);
}
#pragma
void RTNA_WAIT_MS(MMP_ULONG ms)
{
    RTNA_Wait_Count(ms * RTNA_CPU_CLK_M / WHILE_CYCLE * 1000) ;
}

void RTNA_WAIT_US(MMP_ULONG us)
{
    RTNA_Wait_Count(us * RTNA_CPU_CLK_M / WHILE_CYCLE);
}

void RTNA_WAIT_CYCLE(MMP_ULONG cycle)
{
    RTNA_Wait_Count(cycle / WHILE_CYCLE);
}
//------------------------------------------------------------------------------
//  Function    : RTNA hardware initialization
//  Description : Initialize RTNA register status.
//------------------------------------------------------------------------------

void RTNA_Init(void)
{
    AITPS_GBL   pGBL  = AITC_BASE_GBL;
	AITPS_VIF   pVIF  = AITC_BASE_VIF;
	MMP_BOOL	bClockEnable;
	MMP_UBYTE   vid = VIF_CTL;

	bClockEnable = (pGBL->GBL_CLK_DIS0 & GBL_CLK_VI_DIS) ? MMP_FALSE : MMP_TRUE;
	if (bClockEnable) 
		pVIF->VIF_OPR_UPD[vid] = VIF_OPR_UPD_EN;
	else {
	    pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_VI_DIS);
		pVIF->VIF_OPR_UPD[vid] = VIF_OPR_UPD_EN;
		pGBL->GBL_CLK_DIS0 |= GBL_CLK_VI_DIS ;
	}	
}




//==============================================================================
//
//                    Complement Missing ISR Functions
//
//==============================================================================

/// @brief keep integrity between lib_retina.c and irq.s



#if defined(ALL_FW)
#define COMPLEMENT_MISSING_ISR
#if (!DSC_R_EN)&&(!PCAM_EN)
void MMPF_JPG_ISR(void) {}
#endif
#if (!USB_EN)
void MMPF_USB_ISR(void) {}
#endif
#if (!VIDEO_R_EN)
void MMPF_VIDEO_ISR(void) {}
void MMPF_MEMC_ISR(void) {}
void MMPF_H264ENC_ISR(void) {}
#endif
#if (!SENSOR_EN)
void MMPF_VIF_ISR(void) {}
void MMPF_ISP_ISR(void) {}
#endif
#if (!VIDEO_P_EN)
void MMPF_DISPLAY_ISR(void) {}
void MMPF_MP4VDEC_FrameEndISR(void) {}
void MMPF_MP4VDEC_VldISR(void) {}
#endif
#endif

#if (defined(UPDATER_FW)||defined(MBOOT_FW))
#define COMPLEMENT_MISSING_ISR
void MMPF_DISPLAY_ISR(void) {}
#endif

#if	defined(BROGENT_FW)
#define COMPLEMENT_MISSING_ISR
void MMPF_DISPLAY_ISR(void){}
#endif /* BROGENT_FW */

#if (!USING_SM_IF)
MMP_ERR MMPF_NAND_ReadSector(MMP_ULONG dmastartaddr, MMP_ULONG startsect, MMP_USHORT sectcount){return 1;}
MMP_ERR MMPF_NAND_WriteSector(MMP_ULONG dmastartaddr, MMP_ULONG startsect, MMP_USHORT sectcount){return 1;}
MMP_ERR MMPF_NAND_GetSize(MMP_ULONG *pSize){*pSize = 0;return 1;}
MMP_ERR MMPF_NAND_InitialInterface(void){return 1;}
MMP_ERR MMPF_NAND_Reset(void){return 1;}
MMP_ERR MMPF_NAND_LowLevelFormat(void){return 1;}
MMP_ERR MMPF_NAND_FinishWrite(void){return 1;}
void MMPF_NAND_ISR(void){}
#endif /* (!USING_SM_IF) */

#ifndef COMPLEMENT_MISSING_ISR
/**  Keep this invalid variable so that compiler can automatically detect the problem
  *  while new package forget to define missing ISR.
  */
Please_Read_Me;
/**
 * If you encountered an compiler error, you have to complement the missing ISR here.
 *
 * Each package has to add its own section here, or else compiler would occur error.
 * @todo Each section has to put it's missing ISR with empty function here.
 *       You can reference the existing code and compiler messages for what ISR you're missing
 *       after #define COMPLEMENT_MISSING_ISR.
 */
#endif
// Truman Logic --

/** @}*/ //BSP
