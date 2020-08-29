//==============================================================================
//
//  File        : mmpf_uart.c
//  Description : Firmware UART Control Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "mmpf_uart.h"

#include "mmp_reg_uart.h"
#include "lib_retina.h"
//#include "config_fw.h"

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================


#if (UART_RXINT_MODE_EN == 1)
#define RX_ENTER_SIGNAL  	13
#define RX_SENSITIVE  		100
#define RX_QUEUE_SIZE		128
static	MMP_BYTE	m_bDebugString[MMPF_UART_MAX_COUNT][RX_QUEUE_SIZE];
static	MMP_ULONG	m_bDebugStringLen[MMPF_UART_MAX_COUNT];
MMPF_OS_SEMID    	m_semUartDebug[MMPF_UART_MAX_COUNT];
#endif

#if (UART_DMA_MODE_EN == 1)
MMPF_OS_SEMID    	m_semUartDmaMode;
static UartCallBackFunc *gUart_CallBackFunc[MMPF_UART_DMA_INT_MAX];
#endif

static MMP_SHORT	m_bUartDmaInUsedId = MMPF_UART_MAX_COUNT;
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================


#if (UART_DMA_MODE_EN == 1)
//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_SwitchToDmaMode
//  Description :
//  Note        : This is the 1st step of UART using DMA mode
//------------------------------------------------------------------------------
/** @brief This function set the UART device from normal mode to DMA mode.

This function set the UART device from normal mode to DMA mode.
@param[in] uartId indicate which UART device, please refer the data structure of MMPF_UART_ID
@param[in] bEnable stands for enable switch to DMA mode or back from DMA mode.
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Uart_SwitchToDmaMode(MMPF_UART_ID uartId, MMP_BOOL bEnable)
{
	AITPS_UARTB	pUART  = AITC_BASE_UARTB;
	if (bEnable == MMP_TRUE) {
		#if (UART_RXINT_MODE_EN == 1)
    	pUART->US[uartId].US_IER &= (~US_RX_FIFO_OVER_THRES);  //close uart RX normal interrupt
    	#endif
    	
    	#if (UART_DMA_MODE_EN == 1)
		MMPF_OS_AcquireSem(m_semUartDmaMode, 0x0);
		m_bUartDmaInUsedId = uartId;
		#endif
	}
	else {
		if (uartId == m_bUartDmaInUsedId) {
			//Clean DMA mode settings
			pUART->US[uartId].US_CR &= US_DMA_CLEAN;
			
			#if (UART_DMA_MODE_EN == 1)
			MMPF_OS_ReleaseSem(m_semUartDmaMode);
			m_bUartDmaInUsedId = MMPF_UART_MAX_COUNT;
			#endif
			
			#if (UART_RXINT_MODE_EN == 1)
    		pUART->US[uartId].US_IER = US_RX_FIFO_OVER_THRES;  //Resume uart RX normal interrupt
    		#endif
		}
	}
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_SetTxDmaMode
//  Description :
//  Note        : This is the 2nd step of UART using DMA TX mode
//------------------------------------------------------------------------------
/** @brief This function set the parameters using by UART DMA TX mode.

This function set the parameters using by UART DMA Tx mode.
@param[in] uartId indicate which UART device, please refer the data structure of MMPF_UART_ID
@param[in] MMPF_UART_DMAMODE indicate which DMA mode to be used, please refer the data structure MMPF_UART_DMAMODE.
@param[in] uartTxStartAddr indicate the Tx DMA start address.
@param[in] uartTxTotalByte indicate number of bytes would be sent (start from start address).
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Uart_SetTxDmaMode(MMPF_UART_ID uartId, MMPF_UART_DMAMODE uartDmaMode, MMP_ULONG uartTxStartAddr, MMP_USHORT uartTxTotalByte)
{
	AITPS_UARTB	pUART  = AITC_BASE_UARTB;
	
	if( m_bUartDmaInUsedId != uartId) {
		RTNA_DBG_Str(0, "uart Device have not got the DMA resource !!\r\n");
		return MMP_ERR_NONE;
	}
	
	if (uartDmaMode == MMPF_UART_TXDMA) {
		pUART->US[uartId].US_TXDMA_START_ADDR = uartTxStartAddr;
		pUART->US[uartId].US_TXDMA_TOTAL_BYTE = uartTxTotalByte;
	}
	else {
		RTNA_DBG_Str(0, "MMPF_Uart_SetRxDmaMode(): Error TX DMA mode \r\n");
		return MMP_ERR_NONE;
	}
	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_SetRxDmaMode
//  Description :
//  Note        : This is the 2nd step of UART using DMA RX mode
//------------------------------------------------------------------------------
/** @brief This function set the parameters using by UART DMA RX mode.

This function set the parameters using by UART DMA Tx mode.
@param[in] uartId indicate which UART device, please refer the data structure of MMPF_UART_ID
@param[in] MMPF_UART_DMAMODE indicate which DMA mode to be used, please refer the data structure MMPF_UART_DMAMODE.
@param[in] uartRxStartAddr indicate the RX DMA start address.
@param[in] uartRxEndAddr indicate the RX DMA End address.
@param[in] uartRxLowBoundAddr indicate the RX lower bound address (Using by RX DMA Ring Mode). 
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Uart_SetRxDmaMode(MMPF_UART_ID uartId, MMPF_UART_DMAMODE uartDmaMode, MMP_ULONG uartRxStartAddr, MMP_ULONG uartRxEndAddr, MMP_ULONG uartRxLowBoundAddr)
{
	AITPS_UARTB	pUART  = AITC_BASE_UARTB;
	
	if( m_bUartDmaInUsedId != uartId) {
		RTNA_DBG_Str(0, "uart Device have not got the DMA resource !!\r\n");
		return MMP_ERR_NONE;
	}
	
	if ((uartDmaMode == MMPF_UART_RXDMA) || (uartDmaMode == MMPF_UART_RXDMA_RING)) {
		pUART->US[uartId].US_RXDMA_START_ADDR = uartRxStartAddr;
		pUART->US[uartId].US_RXDMA_END_ADDR = uartRxEndAddr;
		if(uartDmaMode == MMPF_UART_RXDMA_RING) {
			pUART->US[uartId].US_RXDMA_LB_ADDR = uartRxLowBoundAddr;
		}	
	}
	else {
		RTNA_DBG_Str(0, "MMPF_Uart_SetRxDmaMode(): Error RX DMA mode \r\n");
		return MMP_ERR_NONE;
	}
	
	return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_SetDmaInterruptMode
//  Description :
//  Note        : This is the step of UART using DMA interrupt mode settings. (This step can be done betwee step2 and step3)
//------------------------------------------------------------------------------
/** @brief This function sets the UART DMA interrupt mode.

This function sets the UART DMA interrupt mode.
@param[in] uartId indicate which UART device, please refer the data structure of MMPF_UART_ID
@param[in] MMPF_UART_DMA_INT_MODE indicate which DMA interrupt mode to be used, please refer the data structure MMPF_UART_DMA_INT_MODE.
@param[in] bEnable stands for "enable the related interrupt mode or not".
@param[in] callBackFunc is used as interrupt handler.
@param[in] uartRxThreshold is used by RX DMA mode, when dma count reaches the Threshold and the related interrupt occurs. 
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Uart_SetDmaInterruptMode (MMPF_UART_ID uartId, MMPF_UART_DMA_INT_MODE intMode, MMP_BOOL bEnable, UartCallBackFunc* callBackFunc, MMP_USHORT uartRxThreshold)
{
	AITPS_UARTB	pUART  = AITC_BASE_UARTB;
	if (bEnable == MMP_TRUE) {
		switch (intMode) {
			case MMPF_UART_TXDMA_FINISH_IENABLE:
				pUART->US[uartId].US_IER |= US_TXDMA_FINISH_IENABLE;
				break;
			case MMPF_UART_RXDMA_THR_IEABLE:
				pUART->US[uartId].US_RXDMA_TOTAL_THR = uartRxThreshold;
				pUART->US[uartId].US_IER |= US_RXDMA_THR_IEABLE;
				break;
			case MMPF_UART_RXDMA_WRITEMEM_IENALBE:
				pUART->US[uartId].US_IER |= US_RXDMA_WRITEMEM_IENALBE;
				break;
			case MMPF_UART_RXDMA_DROPDATA_IENALBE:
				pUART->US[uartId].US_IER |= US_RXDMA_DROPDATA_IENALBE;
				break;
			default:
				RTNA_DBG_Str(0, "Error Dma Interrupt Mode ! \r\n");
				return MMP_ERR_NONE;
				break;
		}
		
		if (callBackFunc != NULL) {	
			gUart_CallBackFunc[intMode] = callBackFunc;
		}
	}
	else {
		switch (intMode) {
			case MMPF_UART_TXDMA_FINISH_IENABLE:
				pUART->US[uartId].US_IER &= (~US_TXDMA_FINISH_IENABLE);
				break;
			case MMPF_UART_RXDMA_THR_IEABLE:
				pUART->US[uartId].US_RXDMA_TOTAL_THR = 0;
				pUART->US[uartId].US_IER &= (~US_RXDMA_THR_IEABLE);
				break;
			case MMPF_UART_RXDMA_WRITEMEM_IENALBE:
				pUART->US[uartId].US_IER &= (~US_RXDMA_WRITEMEM_IENALBE);
				break;
			case MMPF_UART_RXDMA_DROPDATA_IENALBE:
				pUART->US[uartId].US_IER &= (~US_RXDMA_DROPDATA_IENALBE);
				break;
			default:
				RTNA_DBG_Str(0, "Error Dma Interrupt Mode ! \r\n");
				return MMP_ERR_NONE;
				break;
		}	
		gUart_CallBackFunc[intMode] = NULL;
	}
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_EnableDmaMode
//  Description :
//  Note        : This function is the 3rd step of UART DMA settings.
//------------------------------------------------------------------------------
/** @brief This function is used to enable or disable UART DMA mode.

This function is used to enable or disable UART DMA mode.
@param[in] uartId indicate which UART device, please refer the data structure of MMPF_UART_ID
@param[in] MMPF_UART_DMAMODE indicate which DMA mode to be used, please refer the data structure MMPF_UART_DMAMODE.
@param[in] bEnable stands for "enable the related mode or not".
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Uart_EnableDmaMode(MMPF_UART_ID uartId, MMPF_UART_DMAMODE uartDmaMode, MMP_BOOL bEnable)
{
	AITPS_UARTB	pUART  = AITC_BASE_UARTB;
	
	if (bEnable == MMP_TRUE) {
		switch (uartDmaMode) {
			case MMPF_UART_RXDMA_RING:
				pUART->US[uartId].US_CR |= US_RXDMA_RING_ENABLE;
				break;
    		case MMPF_UART_TXDMA:
    			pUART->US[uartId].US_CR |= US_TXDMA_ENABLE;
    			break;
    		case MMPF_UART_RXDMA:
    			pUART->US[uartId].US_CR |= US_RXDMA_ENABLE;
    			break;
    		default:
    			RTNA_DBG_Str(0, "Error! wrong DMA mode enable !\r\n");
    			break;
		}
	
	}
	else {
		switch (uartDmaMode) {
			case MMPF_UART_RXDMA_RING:
				pUART->US[uartId].US_CR &= (~US_RXDMA_RING_ENABLE);
				break;
    		case MMPF_UART_TXDMA:
    			pUART->US[uartId].US_CR &= (~US_TXDMA_ENABLE);
    			break;
    		case MMPF_UART_RXDMA:
    			pUART->US[uartId].US_CR &= (~US_RXDMA_ENABLE);
    			break;
    		default:
    			RTNA_DBG_Str(0, "Error! wrong DMA mode disable !\r\n");
    			break;
		}
	}
	
	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_EnableDmaMode
//  Description :
//  Note        : This operation need to be done after settings done or DMA RX restart.
//------------------------------------------------------------------------------
/** @brief This function to enable UART DMA RX operations.

This function to enable UART DMA RX operations.
@param[in] uartId indicate which UART device, please refer the data structure of MMPF_UART_ID
@return It reports the status of the operation.
*/
MMP_ERR MMPF_Uart_RxDmaStart(MMPF_UART_ID uartId)
{
	AITPS_UARTB	pUART  = AITC_BASE_UARTB;
	pUART->US[uartId].US_CR |= US_RXDMA_START_FLAG;
	return MMP_ERR_NONE;
}
#endif


//------------------------------------------------------------------------------
//  Function    : MMPF_UART_ISR
//  Description :
//  Note        :
//------------------------------------------------------------------------------
void MMPF_UART_ISR(void)
{
	#if(UART_DBG_EN == 0x1)
	AITPS_UARTB	pUART  = AITC_BASE_UARTB;
    MMP_ULONG 	len,	i, j = 0;
    
	if (m_bUartDmaInUsedId != MMPF_UART_MAX_COUNT) {  //UART DMA mode
		#if (UART_DMA_MODE_EN == 1)
		if (pUART->US[j].US_ISR & US_TXDMA_FINISH_IENABLE) {
		 	pUART->US[j].US_ISR &= US_TXDMA_FINISH_IENABLE;  //clean interrupt status
		 	if (gUart_CallBackFunc[MMPF_UART_TXDMA_FINISH_IENABLE] != NULL) {
		 		(*gUart_CallBackFunc[MMPF_UART_TXDMA_FINISH_IENABLE]) ();
		 	}
		 	
		}
		else if (pUART->US[j].US_ISR & US_RXDMA_THR_IEABLE) {
		 	pUART->US[j].US_ISR &= US_RXDMA_THR_IEABLE;	//clean interrupt status
		 	if (gUart_CallBackFunc[MMPF_UART_RXDMA_THR_IEABLE] != NULL) {
		 		(*gUart_CallBackFunc[MMPF_UART_RXDMA_THR_IEABLE]) ();
		 	}
		}
		else if (pUART->US[j].US_ISR & US_RXDMA_WRITEMEM_IENALBE) {
		 	pUART->US[j].US_ISR &= US_RXDMA_WRITEMEM_IENALBE;	//clean interrupt status
		 	if (gUart_CallBackFunc[MMPF_UART_RXDMA_WRITEMEM_IENALBE] != NULL) {
		 		(*gUart_CallBackFunc[MMPF_UART_RXDMA_WRITEMEM_IENALBE]) ();
		 	}
		}
		else if (pUART->US[j].US_ISR & US_RXDMA_DROPDATA_IENALBE) {
		 	pUART->US[j].US_ISR &= US_RXDMA_DROPDATA_IENALBE;	//clean interrupt status
		 	if (gUart_CallBackFunc[MMPF_UART_RXDMA_DROPDATA_IENALBE] != NULL) {
		 		(*gUart_CallBackFunc[MMPF_UART_RXDMA_DROPDATA_IENALBE]) ();
		 	}
		}
		else {
		}
		#endif
	}
	else {
		#if (UART_RXINT_MODE_EN == 1)
		for (j = 0; j < MMPF_UART_MAX_COUNT; j++) {
	    	#if DUAL_UART_DBG_EN == 0
			if (pUART->US[j].US_ISR & US_RX_FIFO_OVER_THRES) {
		    	#if (CHIP == VSN_V2)
		        len = pUART->US[j].US_FSR & US_RX_FIFO_UNRD_MASK;
				#endif
				#if (CHIP == VSN_V3)
				len = pUART->US[j].US_RX_FIFO_DATA_CNT;
				#endif
				
				//RTNA_DBG_PrintLong(0, len);
				//RTNA_DBG_PrintLong(0, j);
		        for(i = 0; i < len; i++) {
		        	m_bDebugString[j][(m_bDebugStringLen[j] + i)%RX_QUEUE_SIZE] = pUART->US[j].US_RXPR;	
					MMPF_Uart_Write(DEBUG_UART_NUM, &(m_bDebugString[j][m_bDebugStringLen[j]]), 1);
		        }
		        
		        m_bDebugStringLen[j] += len;
		        
		        if (m_bDebugStringLen[j] >= RX_QUEUE_SIZE) {
		        	RTNA_DBG_Str(0, "Error: UART RX overflow\r\n");
		        }
			    break;
		    }
			#else
		    if(j == 0)
		        pUART  = AITC_BASE_UARTB;
		    else if(j == 1)
		        pUART  = AITC_BASE_UARTB_1;
		    if (pUART->US[0].US_ISR & US_RX_FIFO_OVER_THRES) {
		    	#if (CHIP == VSN_V2)
		        len = pUART->US[0].US_FSR & US_RX_FIFO_UNRD_MASK;
				#endif
				#if (CHIP == VSN_V3)
				len = pUART->US[0].US_RX_FIFO_DATA_CNT;
				#endif
				
		        for(i = 0; i < len; i++) {
		        	m_bDebugString[0][(m_bDebugStringLen[0] + i)%RX_QUEUE_SIZE] = pUART->US[0].US_RXPR;	
					MMPF_Uart_Write(j, &(m_bDebugString[0][m_bDebugStringLen[0]]), 1);
		        }
		        
		        m_bDebugStringLen[0] += len;
		        
		        if (m_bDebugStringLen[0] >= RX_QUEUE_SIZE) {
		        	RTNA_DBG_Str(0, "Error: UART RX overflow\r\n");
		        }
			    break;
		    }
			#endif
	    }
	    #endif
    }
	#endif //#if(UART_DBG_EN == 0x1)   
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_GetDebugString
//  Description : This function will send back the string until the user enter the "Enter"
//  Note        :
//------------------------------------------------------------------------------
#if (UART_RXINT_MODE_EN == 1)
MMP_ERR MMPF_Uart_GetDebugString(MMPF_UART_ID uartId, MMP_BYTE *bDebugString, MMP_ULONG *ulDebugStringLength)
{
#if DUAL_UART_DBG_EN == 0
	MMP_LONG i, j;
	MMP_BOOL bEnterDetect = MMP_FALSE;
	AITPS_UARTB pUART  = AITC_BASE_UARTB;
	
	MMPF_Uart_Init();
	
	MMPF_OS_AcquireSem(m_semUartDebug[uartId], 0);
	m_bDebugStringLen[uartId] = 0;
	pUART->US[uartId].US_CR |= US_RXEN;

	while(bEnterDetect == MMP_FALSE) {
		for(i = m_bDebugStringLen[uartId]; i >= 0; i--) {
			if( m_bDebugString[uartId][i] == RX_ENTER_SIGNAL) {
				*ulDebugStringLength = i;  //Copy string except the "Enter" signal
				for (j = 0; j < i; j++) {
					bDebugString[j] = m_bDebugString[uartId][j];
				}
				bDebugString[i] = '\0';
				bEnterDetect = MMP_TRUE;
				MEMSET0(&m_bDebugString[uartId]);
				break;
			}
		}
		MMPF_OS_Sleep(RX_SENSITIVE);	
	}
	pUART->US[uartId].US_CR &= (~US_RXEN);
	MMPF_OS_ReleaseSem(m_semUartDebug[uartId]);
#endif
	return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_Init
//  Description : Initial the semaphore and call-back functions.
//  Note        :
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Uart_Init(void)
{
	#if(UART_DBG_EN == 0x1)
		#if (UART_RXINT_MODE_EN == 1)
		static MMP_BOOL bUartInitFlag = MMP_FALSE;  //To make sure the semaphore initial is called only once.
		AITPS_AIC   pAIC 	= AITC_BASE_AIC;
		MMP_USHORT 	i = 0;
		
		if (!bUartInitFlag) {
			for (i = 0; i < MMPF_UART_MAX_COUNT; i++) {
		    	m_semUartDebug[i] = MMPF_OS_CreateSem(1);
		    }

		    RTNA_AIC_Open(pAIC, AIC_SRC_UART,   uart_isr_a,
		                    AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE    | 3);
		    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_UART);
		    
		    #if (UART_DMA_MODE_EN == 1)
			m_semUartDmaMode =  MMPF_OS_CreateSem(1);
			for (i = 0; i < MMPF_UART_DMA_INT_MAX; i++) {
				gUart_CallBackFunc[i] = NULL;
			}
			
			#endif
			bUartInitFlag = MMP_TRUE;
			RTNA_DBG_Str0("UART enable interrupt !\r\n");
		}
		
		#endif
	#endif	
	return	MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_Init
//  Description : Initial the semaphore and call-back functions.
//  Note        :
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Uart_InitISR(void)
{
    AITPS_AIC   pAIC 	= AITC_BASE_AIC;
	
    RTNA_AIC_Open(pAIC, AIC_SRC_UART,   uart_isr_a,
		                    AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE    | 3);
    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_UART);

    MMPF_Uart_Write(MMPF_UART_ID_1,"<Uart-Rx>\r\n", _strlen("<Uart-Rx>\r\n") );

    
	return	MMP_ERR_NONE;
}

MMP_ERR    MMPF_Uart_DeInitISR(void)
{
		AITPS_AIC   pAIC 	= AITC_BASE_AIC;

    RTNA_AIC_Open(pAIC, AIC_SRC_UART,   uart_isr_a,
		                    AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE    | 3);
    RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_UART);
	return	MMP_ERR_NONE;
}

#if (DUAL_UART_DBG_EN == 0)
//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_Open
//  Description :
//  Note        :
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Uart_Open(MMPF_UART_ID uartId, MMPF_UART_ATTRIBUTE *uartattribute)
{
	#if(UART_DBG_EN == 0x1)
	 // Variable Declaration
    AITPS_UARTB pUART = AITC_BASE_UARTB;
	AITPS_GBL 	pGBL = AITC_BASE_GBL;
	
	
	//MMPF_Uart_Init();
    // PAD config for UART interface
    // Clean PAD config first.
    pGBL->GBL_IO_CTL1 &= ~(GBL_UART_TX_PAD0 | GBL_UART_TX_PAD1);
    pGBL->GBL_IO_CTL3 &= ~(GBL_UART_RX_PAD0 | GBL_UART_RX_PAD1 | GBL_UART_RX_PAD2 | GBL_UART_TX_PAD2);
    
    switch(uartattribute->padset) {
    	case MMPF_UART_PADSET_0:	// use AGPIO0 as uart TX
    		pGBL->GBL_IO_CTL1 |= GBL_UART_TX_PAD0;
    		#if (UART_RXINT_MODE_EN == 0x1)
    		pGBL->GBL_IO_CTL3 |= GBL_UART_RX_PAD0;
    		#endif
    		break;
    	case MMPF_UART_PADSET_1:	// use PSNR_D8 as uart TX
    		pGBL->GBL_IO_CTL1 |= GBL_UART_TX_PAD1;
    		#if (UART_RXINT_MODE_EN == 0x1)
    		pGBL->GBL_IO_CTL3 |= GBL_UART_RX_PAD1;
    		#endif
    		break;
    	case MMPF_UART_PADSET_2:	// use BGPIO14 as uart tx
    		pGBL->GBL_IO_CTL3 |= GBL_UART_TX_PAD2;
    		#if (UART_RXINT_MODE_EN == 0x1)
    		pGBL->GBL_IO_CTL3 |= GBL_UART_RX_PAD2;
    		#endif
    		break;
    	default:
    		RTNA_DBG_Str(0, "Un-supported uart PAD !!\r\n");
    		return MMP_UART_ERR_PARAMETER;
    		break;
    };	
  
    MMPF_Uart_Close(uartId);
	
  	// Define the baud rate divisor register
    pUART->US[uartId].US_BRGR = (((uartattribute->ulMasterclk << 1) / uartattribute->ulBaudrate) + 1) >> 1;
    // Define the USART mode 8-N-1

	#if (UART_RXINT_MODE_EN == 1)
	pUART->US[uartId].US_FTHR &= ~US_RX_FIFO_THRES_MASK;
	pUART->US[uartId].US_FTHR |= US_RX_FIFO_THRES(1);
	pUART->US[uartId].US_IER = US_RX_FIFO_OVER_THRES;
	#endif
	
	pUART->US[uartId].US_CR = US_ASYNC_MODE /*| US_RXEN*/ | US_TXEN; 
	  
	#endif //#if(UART_DBG_EN == 0x1)
	return	MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_Write
//  Description : Debug output a string
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Uart_Write(MMPF_UART_ID uartId, const char *str, MMP_ULONG ulLength)
{
	#if(UART_DBG_EN == 0x1)
	AITPS_UARTB pUART = AITC_BASE_UARTB;
    MMP_ULONG   i, txcnt, j = 0;
	
    while(ulLength) {
        #if (CHIP == VSN_V2)
        txcnt = (pUART->US[uartId].US_FSR & US_TX_FIFO_UNWR_MASK);
        #endif
        #if (CHIP == VSN_V3)
        txcnt = pUART->US[uartId].US_TX_FIFO_DATA_CNT;
        #endif
        
        if(txcnt) {
            if(txcnt > ulLength) {
                txcnt = ulLength;
            }
            for(i = 0;i < txcnt;i++,j++) {
                pUART->US[uartId].US_TXPR = *(str+j);
            }
            ulLength -= txcnt;
        }
    }
    #endif //#if(UART_DBG_EN == 0x1)
    return	MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_Close
//  Description :
//  Note        :
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Uart_Close(MMPF_UART_ID uartId)
{
	#if(UART_DBG_EN == 0x1)
	AITPS_UARTB   pUART  = AITC_BASE_UARTB;

    // Disable interrupts
    pUART->US[uartId].US_IER = 0;

    // Reset receiver and transmitter
    pUART->US[uartId].US_CR = US_RSTRX | US_RSTTX | US_RXDIS | US_TXDIS;
	#endif //#if(UART_DBG_EN == 0x1)
    return	MMP_ERR_NONE;
}
#else
MMP_ERR    MMPF_Uart_Open(MMPF_UART_ID uartId, MMPF_UART_ATTRIBUTE *uartattribute)
{
	#if(UART_DBG_EN == 0x1)
	 // Variable Declaration
    AITPS_UARTB pUART = AITC_BASE_UARTB;
	AITPS_GBL 	pGBL = AITC_BASE_GBL;
	
	if(uartId == 1)
	    pUART = AITC_BASE_UARTB_1;
	    
    // PAD config for UART interface
    // Clean PAD config first.
    if(uartId == 0)
    {
        pGBL->GBL_IO_CTL1 &= ~(GBL_UART_TX_PAD0 | GBL_UART_TX_PAD1);
        pGBL->GBL_IO_CTL3 &= ~(GBL_UART_RX_PAD0 | GBL_UART_RX_PAD1 | GBL_UART_RX_PAD2 | GBL_UART_TX_PAD2);
        
	    switch(uartattribute->padset) {
	    	case MMPF_UART_PADSET_0:	// use AGPIO0 as uart TX
	    		pGBL->GBL_IO_CTL1 |= GBL_UART_TX_PAD0;
	    		#if (UART_RXINT_MODE_EN == 0x1)
	    		pGBL->GBL_IO_CTL3 |= GBL_UART_RX_PAD0;
	    		#endif
	    		break;
	    	case MMPF_UART_PADSET_1:	// use PSNR_D8 as uart TX
	    		pGBL->GBL_IO_CTL1 |= GBL_UART_TX_PAD1;
	    		#if (UART_RXINT_MODE_EN == 0x1)
	    		pGBL->GBL_IO_CTL3 |= GBL_UART_RX_PAD1;
	    		#endif
	    		break;
	    	case MMPF_UART_PADSET_2:	// use BGPIO14 as uart tx
	    		pGBL->GBL_IO_CTL3 |= GBL_UART_TX_PAD2;
	    		#if (UART_RXINT_MODE_EN == 0x1)
	    		pGBL->GBL_IO_CTL3 |= GBL_UART_RX_PAD2;
	    		#endif
	    		break;
	    	default:
	    		RTNA_DBG_Str(0, "Un-supported uart PAD !!\r\n");
	    		return MMP_UART_ERR_PARAMETER;
	    		break;
	    };	
	    
		MMPF_Uart_Close(uartId);
    }
    else
    {
        pGBL->GBL_PROBE_CTL &= ~0x20 ;
        pGBL->GBL_LCD_BYPASS_CTL1 &= ~(GBL_UART1_TX_PAD0 | GBL_UART1_RX_PAD0 | GBL_UART1_TX_PAD1 | GBL_UART1_RX_PAD1);  
	    switch(uartattribute->padset) {
	    	case MMPF_UART_PADSET_0:	// use AGPIO0 as uart TX
	    		pGBL->GBL_LCD_BYPASS_CTL1 |= GBL_UART1_TX_PAD0;
	    		#if (UART_RXINT_MODE_EN == 0x1)
	    		pGBL->GBL_LCD_BYPASS_CTL1 |= GBL_UART1_RX_PAD0;
	    		#endif
	    		break;
	    	case MMPF_UART_PADSET_1:	// use PSNR_D8 as uart TX
	    		pGBL->GBL_LCD_BYPASS_CTL1 |= GBL_UART1_TX_PAD1;
	    		#if (UART_RXINT_MODE_EN == 0x1)
	    		pGBL->GBL_LCD_BYPASS_CTL1 |= GBL_UART1_RX_PAD1;
	    		#endif
	    		break;
	    	default:
	    		RTNA_DBG_Str(0, "Un-supported uart PAD !!\r\n");
	    		return MMP_UART_ERR_PARAMETER;
	    		break;
	    };	
	  
	    
		MMPF_Uart_Close(uartId);
    }
	#endif //#if(UART_DBG_EN == 0x1)
	
			
  	// Define the baud rate divisor register
    pUART->US[0].US_BRGR = (((uartattribute->ulMasterclk << 1) / uartattribute->ulBaudrate) + 1) >> 1;
    // Define the USART mode 8-N-1

	#if (UART_RXINT_MODE_EN == 1)
	pUART->US[0].US_FTHR &= ~US_RX_FIFO_THRES_MASK;
	pUART->US[0].US_FTHR |= US_RX_FIFO_THRES(1);
	pUART->US[0].US_IER = US_RX_FIFO_OVER_THRES;
	#endif
	
	pUART->US[0].US_CR = US_ASYNC_MODE | (uartattribute->ubRxEnable ? US_RXEN : 0) | (uartattribute->ubTxEnable ? US_TXEN : 0); 
	return	MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_Write
//  Description : Debug output a string
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Uart_Write(MMPF_UART_ID uartId, const char *str, MMP_ULONG ulLength)
{
	#if(UART_DBG_EN == 0x1)
	AITPS_UARTB pUART = AITC_BASE_UARTB;
    MMP_ULONG   i, txcnt, j = 0;
    
	if(uartId == 1)
	    pUART = AITC_BASE_UARTB_1;
	
    while(ulLength) {
        #if (CHIP == VSN_V2)
        txcnt = (pUART->US[0].US_FSR & US_TX_FIFO_UNWR_MASK);
        #endif
        #if (CHIP == VSN_V3)
        txcnt = pUART->US[0].US_TX_FIFO_DATA_CNT;
        #endif
        
        if(txcnt) {
            if(txcnt > ulLength) {
                txcnt = ulLength;
            }
            for(i = 0;i < txcnt;i++,j++) {
                pUART->US[0].US_TXPR = *(str+j);
            }
            ulLength -= txcnt;
        }
    }
    #endif //#if(UART_DBG_EN == 0x1)
    return	MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Uart_Close
//  Description :
//  Note        :
//------------------------------------------------------------------------------
MMP_ERR    MMPF_Uart_Close(MMPF_UART_ID uartId)
{
	#if(UART_DBG_EN == 0x1)
	AITPS_UARTB   pUART  = AITC_BASE_UARTB;
    if(uartId == 1)
	    pUART = AITC_BASE_UARTB_1;
	
    // Disable interrupts
    pUART->US[0].US_IER = 0;

    // Reset receiver and transmitter
    pUART->US[0].US_CR = US_RSTRX | US_RSTTX | US_RXDIS | US_TXDIS;
	#endif //#if(UART_DBG_EN == 0x1)
    return	MMP_ERR_NONE;
}
#endif