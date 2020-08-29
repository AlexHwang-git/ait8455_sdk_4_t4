//==============================================================================
//
//  File        : mmpf_uart.h
//  Description : INCLUDE File for the Firmware UART Control Driver
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMPF_UART_H_
#define _MMPF_UART_H_

//#include    "includes.h"


//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define UART_RXINT_MODE_EN	(1)
#if (CHIP == P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
#define UART_DMA_MODE_EN	(0)
#endif

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPF_UART_ID
{
    MMPF_UART_ID_0 = 0,
    MMPF_UART_ID_1,
    MMPF_UART_ID_2,
    MMPF_UART_MAX_COUNT
} MMPF_UART_ID;

typedef enum _MMPF_UART_PADSET
{
    MMPF_UART_PADSET_0 = 0,
    MMPF_UART_PADSET_1,
    MMPF_UART_PADSET_2,
    MMPF_UART_PADSET_3,
    #if (CHIP == P_V2)   
    MMPF_UART_PADSET_4,
    MMPF_UART_PADSET_5,
    MMPF_UART_PADSET_6,
    #endif
    MMPF_UART_PADSED_MAX
} MMPF_UART_PADSET;

#if (UART_DMA_MODE_EN == 1)
typedef void UartCallBackFunc(void);

typedef enum _MMPF_UART_DMAMODE
{
    MMPF_UART_RXDMA_RING = 0,
    MMPF_UART_TXDMA,
    MMPF_UART_RXDMA,
    MMPF_UART_DMA_MAX
} MMPF_UART_DMAMODE;

typedef enum _MMPF_UART_DMA_INT_MODE
{
    MMPF_UART_TXDMA_FINISH_IENABLE = 0,
    MMPF_UART_RXDMA_THR_IEABLE,
    MMPF_UART_RXDMA_WRITEMEM_IENALBE,
    MMPF_UART_RXDMA_DROPDATA_IENALBE,
    MMPF_UART_DMA_INT_MAX
} MMPF_UART_DMA_INT_MODE;
#endif
typedef struct _MMPF_UART_ATTRIBUTE
{
    MMPF_UART_PADSET    padset;
    MMP_ULONG           ulMasterclk;
    MMP_ULONG           ulBaudrate;
    #if (DUAL_UART_DBG_EN == 1)
    MMP_BOOL            ubTxEnable;
    MMP_BOOL            ubRxEnable;
    MMP_USHORT          usReserved;
    #endif
} MMPF_UART_ATTRIBUTE;


//==============================================================================
//
//                              VARIABLES
//
//==============================================================================


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
MMP_ERR    MMPF_Uart_Open(MMPF_UART_ID uartId, MMPF_UART_ATTRIBUTE *uartattribute);
MMP_ERR    MMPF_Uart_Write(MMPF_UART_ID uartId, const char *str, MMP_ULONG ulLength);
MMP_ERR    MMPF_Uart_Close(MMPF_UART_ID uartId);
MMP_ERR    MMPF_Uart_GetDebugString(MMPF_UART_ID uartId, MMP_BYTE *bDebugString, MMP_ULONG *ulDebugStringLength);
MMP_ERR    MMPF_Uart_Init(void);
#if (UART_DMA_MODE_EN == 1)
MMP_ERR MMPF_Uart_SwitchToDmaMode(MMPF_UART_ID uartId, MMP_BOOL bEnable);
MMP_ERR MMPF_Uart_SetTxDmaMode(MMPF_UART_ID uartId, MMPF_UART_DMAMODE uartDmaMode, MMP_ULONG uartTxStartAddr, MMP_USHORT uartTxTotalByte);
MMP_ERR MMPF_Uart_SetRxDmaMode(MMPF_UART_ID uartId, MMPF_UART_DMAMODE uartDmaMode, MMP_ULONG uartRxStartAddr, MMP_ULONG uartRxEndAddr, MMP_ULONG uartRxLowBoundAddr);
MMP_ERR MMPF_Uart_SetDmaInterruptMode (MMPF_UART_ID uartId, MMPF_UART_DMA_INT_MODE intMode, MMP_BOOL bEnable, UartCallBackFunc* callBackFunc, MMP_USHORT uartRxThreshold);
MMP_ERR MMPF_Uart_EnableDmaMode(MMPF_UART_ID uartId, MMPF_UART_DMAMODE uartDmaMode, MMP_BOOL bEnable);
MMP_ERR MMPF_Uart_RxDmaStart(MMPF_UART_ID uartId);
#endif
//==============================================================================
//
//                              MACRO FUNCTIONS
//
//==============================================================================


#endif // _INCLUDES_H_
