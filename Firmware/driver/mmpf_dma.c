//==============================================================================
//
//  File        : mmpf_dma.c
//  Description : Firmware Graphic Control Function (DMA portion)
//  Author      : Alan Wu
//  Revision    : 1.0
//
//==============================================================================
#include "config_fw.h"
#include "includes_fw.h"
#include "mmpf_dma.h"
#include "mmp_reg_dma.h"
#include "lib_retina.h"

#define CPU_MOVE_SIZE   (16)
static MMPF_DMA_ROT_DATA gDmaRotateData[DMA_R_NUM];
static DmaCallBackFunc *CallBackFuncM0 = NULL;
static DmaCallBackFunc *CallBackFuncM1 = NULL;
static DmaCallBackFunc *CallBackFuncR0 = NULL;
static DmaCallBackFunc *CallBackFuncR1 = NULL;


static MMPF_OS_SEMID  gDMAMoveSemID;
static MMPF_OS_SEMID  gDMARotSemID;
static MMP_BOOL gbDmaFreeM0;
static MMP_BOOL gbDmaFreeM1;

static MMP_BOOL mbDmaM0SpecialCase;
static MMP_BOOL mbDmaM1SpecialCase;
static MMPF_DMA_M_2NDROUND M0InfoDataA;
static MMPF_DMA_M_2NDROUND M1InfoDataA;

static MMPF_DMA_M_2NDROUND* M0InfoDataPtr = &M0InfoDataA;
static MMPF_DMA_M_2NDROUND* M1InfoDataPtr = &M1InfoDataA;

static MMP_BOOL gbDmaFreeR0;
static MMP_BOOL gbDmaFreeR1;

/*static*/ MMP_UBYTE gbDmaEnCount = 0;
//static MMPF_DAM_ROT_DATA gDmaRotateData[DMA_R_NUM];

extern OS_EVENT		*os_sem_tbl[MMPF_OS_SEMID_MAX];
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
/*static MMP_ERR MMPF_DMA_Rotate(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,MMP_USHORT usSrcwidth, MMP_USHORT usSrcheight,
                        MMPF_GRAPHICS_COLORDEPTH colordepth, MMPF_DMA_R_TYPE rotatetype,
                        MMP_USHORT usSrcOffest, MMP_USHORT usDstOffset, MMPF_DMA_R_ID Dmarid, MMP_BOOL mirrorEnable, MMPF_DMA_R_MIRROR_TYPE mirrortype);

*/
//------------------------------------------------------------------------------
//  Function    : MMPF_DMA_Initialize
//  Description :
//  Note        : put in sys_task:main() or XX_Task() (ex: dsc_Task)
//------------------------------------------------------------------------------
MMP_ERR MMPF_DMA_Initialize(void)
{
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    //AITPS_GBL   pGBL = AITC_BASE_GBL;
	static MMP_BOOL bInitDriverDone = MMP_FALSE;
	
	if(bInitDriverDone == MMP_FALSE) {
		bInitDriverDone = MMP_TRUE;
	    RTNA_AIC_Open(pAIC, AIC_SRC_DMA, dma_isr_a,
	                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
	    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_DMA);
		
		gDMAMoveSemID = MMPF_OS_CreateSem(2);
		gDMARotSemID = MMPF_OS_CreateSem(2);
	}
	
    gbDmaFreeM0 = MMP_TRUE;
    gbDmaFreeM1 = MMP_TRUE;
    
	gbDmaFreeR0	= MMP_TRUE;
	gbDmaFreeR1 = MMP_TRUE;
	
    CallBackFuncM0 = NULL;
    CallBackFuncM1 = NULL;
    CallBackFuncR0  = NULL;

	CallBackFuncR1  = NULL;
		mbDmaM0SpecialCase = MMP_FALSE;
		mbDmaM1SpecialCase = MMP_FALSE;
	//temp solution to enable clock. clock enable operation must move to Host side
	//pGBL->GBL_CLK_EN |= GBL_CLK_DMA;

    return MMP_ERR_NONE;
}



//------------------------------------------------------------------------------
//  Function    : MMPF_DMA_ISR
//  Description :
//------------------------------------------------------------------------------
void MMPF_DMA_ISR(void)
{
	AITPS_DMA   pDMA = AITC_BASE_DMA;
    MMP_USHORT  intsrc,i;
    
    intsrc = pDMA->DMA_INT_CPU_SR & pDMA->DMA_INT_CPU_EN;
    pDMA->DMA_INT_CPU_SR = intsrc;

    if (intsrc & DMA_INT_M0) {

        if (CallBackFuncM0 != NULL)
            CallBackFuncM0();

        gbDmaFreeM0 = MMP_TRUE;
        
        if(mbDmaM0SpecialCase == MMP_TRUE) {
            MMP_UBYTE *ptr_src = (MMP_UBYTE *)(M0InfoDataPtr->ulSrcaddr - CPU_MOVE_SIZE );
            MMP_UBYTE *ptr_dst = (MMP_UBYTE *)(M0InfoDataPtr->ulDstaddr - CPU_MOVE_SIZE );
            for(i=0;i<CPU_MOVE_SIZE;i++) {
                ptr_dst[i] = ptr_src[i]  ;
            }
        	MMPF_DMA_MoveData0(M0InfoDataPtr->ulSrcaddr, M0InfoDataPtr->ulDstaddr,
        						M0InfoDataPtr->ulCount,
        						M0InfoDataPtr->ptrLineOffset, M0InfoDataPtr->CallBackFunc);
        }
    }

	#if (CHIP == VSN_V3)
	if (intsrc & DMA_INT_M1) {
	
	    gbDmaFreeM1 = MMP_TRUE;
	    
        if (CallBackFuncM1 != NULL) {
            CallBackFuncM1();
            
        } 

         if(mbDmaM1SpecialCase == MMP_TRUE) {
        	MMPF_DMA_MoveData0(M1InfoDataPtr->ulSrcaddr, M1InfoDataPtr->ulDstaddr,
        						M1InfoDataPtr->ulCount,
        						M1InfoDataPtr->ptrLineOffset, M1InfoDataPtr->CallBackFunc);
        }
    }
	#endif
    if (intsrc & DMA_INT_R0) {
        //Scale();
        if (CallBackFuncR0 != NULL)
            CallBackFuncR0();

        gbDmaFreeR0 = MMP_TRUE;
    }
}


MMP_ERR MMPF_DMA_MoveData(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr, 
                                  MMP_ULONG ulCount, void *CallBackFunc, MMP_BOOL bEnableLOFFS, MMPF_DMA_M_LOFFS_DATA* ptrLineOffset)
{

#if 0
    MMPF_DMA_M_ID Dmamid;  
    //MMP_USHORT  semCount = 0;
    AITPS_DMA   pDMA = AITC_BASE_DMA;
    //AITPS_GBL   pGBL = AITC_BASE_GBL;
    //MMP_UBYTE   ubSemRet = 0;
    //MMP_USHORT  usSemCount = 0x0;
 
	gbDmaFreeM0 = MMP_FALSE;
	Dmamid = MMPF_DMA_M_0;
			      

	if(bEnableLOFFS == MMP_TRUE) {
		pDMA->DMA_M_LOFFS[Dmamid].DMA_M_SRC_LOFFS_W = ptrLineOffset->SrcWidth;
		pDMA->DMA_M_LOFFS[Dmamid].DMA_M_SRC_LOFFS_OFFS = ptrLineOffset->SrcOffset;
		pDMA->DMA_M_LOFFS[Dmamid].DMA_M_DST_LOFFS_W = ptrLineOffset->DstWidth;
		pDMA->DMA_M_LOFFS[Dmamid].DMA_M_DST_LOFFS_OFFS = ptrLineOffset->DstOffset;
		if (Dmamid == MMPF_DMA_M_0) {
			pDMA->DMA_M0_LOFFS_EN = DMA_LOFFS_ENABLE;
		}
		else {
			RTNA_DBG_Str(3, "Error DMA LOFFS ID\r\n");
		}
	}

	pDMA->DMA_M[Dmamid].DMA_M_SRC_ADDR = ulSrcaddr;
	pDMA->DMA_M[Dmamid].DMA_M_DST_ADDR = ulDstaddr;
	pDMA->DMA_M[Dmamid].DMA_M_BYTE_CNT = ulCount;	

	pDMA->DMA_INT_CPU_SR = DMA_INT_M0<<Dmamid;
	pDMA->DMA_INT_CPU_EN = pDMA->DMA_INT_CPU_EN | (0x1<<Dmamid);
 	
	pDMA->DMA_EN |= DMA_M_ENABLE;
	    
	//while((pDMA->DMA_INT_CPU_SR & DMA_INT_M0) == 0x0);	
	//pDMA->DMA_INT_CPU_SR |= DMA_INT_M0; //Clean interrupt status
#endif
	return MMP_ERR_NONE;
}	

static MMP_ERR MMPF_DMA_Rotate(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,MMP_USHORT usSrcwidth, MMP_USHORT usSrcheight,
                        MMPF_GRAPHICS_COLORDEPTH colordepth, MMPF_DMA_R_TYPE rotatetype,
                        MMP_USHORT usSrcOffest, MMP_USHORT usDstOffset, MMPF_DMA_R_ID Dmarid, MMP_BOOL mirrorEnable, MMPF_DMA_R_MIRROR_TYPE mirrortype)
{
     
    AITPS_DMA pDMA = AITC_BASE_DMA;

    pDMA->DMA_R[Dmarid].DMA_R_SRC_ADDR = ulSrcaddr;
    pDMA->DMA_R[Dmarid].DMA_R_SRC_OFST = usSrcOffest;
    pDMA->DMA_R[Dmarid].DMA_R_DST_ADDR = ulDstaddr;
    pDMA->DMA_R[Dmarid].DMA_R_DST_OFST = usDstOffset;
    pDMA->DMA_R[Dmarid].DMA_R_PIX_W = usSrcwidth - 1;
    pDMA->DMA_R[Dmarid].DMA_R_PIX_H = usSrcheight - 1;   

    switch (colordepth) {
    case MMPF_GRAPHICS_COLORDEPTH_8:
    case MMPF_GRAPHICS_COLORDEPTH_YUV420:
        pDMA->DMA_R[Dmarid].DMA_R_CTL = DMA_R_BLK_16X16 | DMA_R_BPP_8;
        break;
    case MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE:
    	if(gDmaRotateData[Dmarid].BufferIndex == 0) {	//Y
    		pDMA->DMA_R[Dmarid].DMA_R_CTL = DMA_R_BLK_16X16 | DMA_R_BPP_8;
    	}
    	else if(gDmaRotateData[Dmarid].BufferIndex == 1) { //UV
    		pDMA->DMA_R[Dmarid].DMA_R_CTL = DMA_R_BLK_16X16 | DMA_R_BPP_16;
    	}
    	break;
    case MMPF_GRAPHICS_COLORDEPTH_16:
        pDMA->DMA_R[Dmarid].DMA_R_CTL = DMA_R_BLK_16X16 | DMA_R_BPP_16;
        break;        
    case MMPF_GRAPHICS_COLORDEPTH_24:
        pDMA->DMA_R[Dmarid].DMA_R_CTL = DMA_R_BLK_8X8 | DMA_R_BPP_24;
        break;
    default:
        return MMP_DMA_ERR_NOT_SUPPORT;
    }
    
    switch (rotatetype) {
    case MMPF_DMA_R_NO:
        pDMA->DMA_R[Dmarid].DMA_R_CTL = pDMA->DMA_R[Dmarid].DMA_R_CTL | DMA_R_NO;
        break;
    case MMPF_DMA_R_90:
        pDMA->DMA_R[Dmarid].DMA_R_CTL = pDMA->DMA_R[Dmarid].DMA_R_CTL | DMA_R_90;
        break;
    case MMPF_DMA_R_270:
        pDMA->DMA_R[Dmarid].DMA_R_CTL = pDMA->DMA_R[Dmarid].DMA_R_CTL | DMA_R_270;
        break;
    case MMPF_DMA_R_180:
        pDMA->DMA_R[Dmarid].DMA_R_CTL = pDMA->DMA_R[Dmarid].DMA_R_CTL | DMA_R_180;
        break;
    default:
        return MMP_DMA_ERR_NOT_SUPPORT;
    }

	
	
	if (mirrorEnable == MMP_TRUE) {	
		switch(mirrortype) {
			case MMPF_DMA_R_MIRROR_H:
				pDMA->DMA_R[Dmarid].DMA_R_MIRROR_EN = DMA_R_H_ENABLE;
				break;
			case MMPF_DMA_R_MIRROR_V:
				pDMA->DMA_R[Dmarid].DMA_R_MIRROR_EN = DMA_R_V_ENABLE;
				break;
		}
	}
	else {
		pDMA->DMA_R[Dmarid].DMA_R_MIRROR_EN = DMA_R_MIRROR_DISABLE;
	}
	
	if (Dmarid == MMPF_DMA_R_0) {
		pDMA->DMA_INT_CPU_EN = pDMA->DMA_INT_CPU_EN | DMA_INT_R0;
		#if (CHIP == VSN_V2) 
		pDMA->DMA_EN |= DMA_R_ENABLE;
		#endif
		#if (CHIP == VSN_V3)
		pDMA->DMA_EN |= DMA_R0_ENABLE;
		#endif
		
		
		
		//while((pDMA->DMA_INT_CPU_SR & DMA_INT_R0) == 0x0);
		//pDMA->DMA_INT_CPU_SR |= DMA_INT_R0; //Clean interrupt status
		//RTNA_DBG_Str(0, "DMA done !\r\n");
	}
	else {
		RTNA_DBG_Str(3, "Error DMA Ratate ID\r\n");
	}

	
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_DMA_RotateImageBuftoBuf(MMPF_GRAPHICS_BUFATTRIBUTE *srcbufattribute, 
                                 MMPF_GRAPHICS_RECT *srcrect, MMPF_GRAPHICS_BUFATTRIBUTE *dstbufattribute, 
                                 MMP_USHORT usDststartx, MMP_USHORT usDststarty, MMPF_GRAPHICS_ROTATE_TYPE rotatetype, 
                                 DmaCallBackFunc *CallBackFunc, MMP_BOOL mirrorEnable, MMPF_DMA_R_MIRROR_TYPE mirrortype)
{
    MMP_ULONG   src_add_offset;
    MMP_ULONG   dst_add_offset;
    MMP_ULONG   src_add_offset_u;
    MMP_ULONG   dst_add_offset_u;
    //MMP_USHORT  semCount = 0;
    //MMP_UBYTE   ubSemRet = 0;
	//MMP_USHORT  usSemCount = 0x0;
	//AITPS_GBL   pGBL = AITC_BASE_GBL;
    //#if (OS_CRITICAL_METHOD == 3)
    //OS_CPU_SR   cpu_sr = 0;
    //#endif

    MMPF_DMA_R_ID Dmarid = MMPF_DMA_R_0;

	if ((dstbufattribute->colordepth !=  MMPF_GRAPHICS_COLORDEPTH_8) &&
		(dstbufattribute->colordepth !=  MMPF_GRAPHICS_COLORDEPTH_16) &&
		(dstbufattribute->colordepth !=  MMPF_GRAPHICS_COLORDEPTH_24) &&
		(dstbufattribute->colordepth !=  MMPF_GRAPHICS_COLORDEPTH_YUV420)&&
		(dstbufattribute->colordepth !=  MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE)) {
		RTNA_DBG_Str(3,"MMPF_DMA_RotateImageBuftoBuf: failure parameter 4");
		return MMP_DMA_ERR_PARAMETER;
	}

	
	//Enable Clock 
	//pGBL->GBL_CLK_EN |= GBL_CLK_DMA;
	gbDmaFreeR0 = MMP_FALSE;
	Dmarid = MMPF_DMA_R_0;            
	CallBackFuncR0 = CallBackFunc;
	

    gDmaRotateData[Dmarid].ColorDepth = srcbufattribute->colordepth;
    gDmaRotateData[Dmarid].RotateType = (MMPF_DMA_R_TYPE)rotatetype;     
    gDmaRotateData[Dmarid].SrcWidth[0] = (srcrect->usWidth);
    gDmaRotateData[Dmarid].SrcHeight[0] = (srcrect->usHeight);  
    gDmaRotateData[Dmarid].SrcLineOffset[0] = srcbufattribute->usLineOffset; 
    gDmaRotateData[Dmarid].DstLineOffset[0] = dstbufattribute->usLineOffset; 
    switch (srcbufattribute->colordepth){
    case MMPF_GRAPHICS_COLORDEPTH_8:
		gDmaRotateData[Dmarid].BufferNum = 1;
		gDmaRotateData[Dmarid].BytePerPixel[0] = 1;
        break;
    case MMPF_GRAPHICS_COLORDEPTH_16:
		gDmaRotateData[Dmarid].BufferNum = 1;
	    gDmaRotateData[Dmarid].BytePerPixel[0] = 2;
        break;
    case MMPF_GRAPHICS_COLORDEPTH_24:
		gDmaRotateData[Dmarid].BufferNum = 1;
		gDmaRotateData[Dmarid].BytePerPixel[0] = 3;
        break;
    case MMPF_GRAPHICS_COLORDEPTH_YUV420:
		gDmaRotateData[Dmarid].BufferNum = 3;
		gDmaRotateData[Dmarid].BytePerPixel[0] = 1;
		
	    gDmaRotateData[Dmarid].SrcLineOffset[1] = srcbufattribute->usLineOffset/2;
	    gDmaRotateData[Dmarid].SrcLineOffset[2] = gDmaRotateData[Dmarid].SrcLineOffset[1];        
	    gDmaRotateData[Dmarid].DstLineOffset[1] = dstbufattribute->usLineOffset/2;
	    gDmaRotateData[Dmarid].DstLineOffset[2] = gDmaRotateData[Dmarid].DstLineOffset[1];
	    
        gDmaRotateData[Dmarid].SrcWidth[1] = (srcrect->usWidth/2);
        gDmaRotateData[Dmarid].SrcHeight[1] = (srcrect->usHeight/2);
        gDmaRotateData[Dmarid].SrcWidth[2] = (srcrect->usWidth/2);
        gDmaRotateData[Dmarid].SrcHeight[2] = (srcrect->usHeight/2);
       
    	src_add_offset_u = srcbufattribute->usLineOffset*srcrect->usTop/4 + srcrect->usLeft*gDmaRotateData[Dmarid].BytePerPixel[0]/2;
    	dst_add_offset_u = dstbufattribute->usLineOffset*usDststarty/4 + usDststartx*gDmaRotateData[Dmarid].BytePerPixel[0]/2;
	   
	    gDmaRotateData[Dmarid].SrcAddr[1] = srcbufattribute->ulBaseUAddr +  src_add_offset_u;
	    gDmaRotateData[Dmarid].SrcAddr[2] = srcbufattribute->ulBaseVAddr +  src_add_offset_u;  
	    gDmaRotateData[Dmarid].DstAddr[1] = dstbufattribute->ulBaseUAddr +  dst_add_offset_u;
	    gDmaRotateData[Dmarid].DstAddr[2] = dstbufattribute->ulBaseVAddr +  dst_add_offset_u;
        break;
    case MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE:
    	gDmaRotateData[Dmarid].BufferNum = 2;
    	gDmaRotateData[Dmarid].BytePerPixel[0] = 1;
    	gDmaRotateData[Dmarid].BytePerPixel[1] = 2;
    	
	    gDmaRotateData[Dmarid].SrcLineOffset[1] = srcbufattribute->usLineOffset;
	    gDmaRotateData[Dmarid].DstLineOffset[1] = dstbufattribute->usLineOffset;
    	gDmaRotateData[Dmarid].SrcWidth[1] = (srcrect->usWidth/2);
        gDmaRotateData[Dmarid].SrcHeight[1] = (srcrect->usHeight/2);
       
    	src_add_offset_u = srcbufattribute->usLineOffset*srcrect->usTop/2 + srcrect->usLeft*gDmaRotateData[Dmarid].BytePerPixel[1]/2;
    	dst_add_offset_u = dstbufattribute->usLineOffset*usDststarty/2 + usDststartx*gDmaRotateData[Dmarid].BytePerPixel[1]/2;
	    gDmaRotateData[Dmarid].SrcAddr[1] = srcbufattribute->ulBaseUAddr +  src_add_offset_u;
	    gDmaRotateData[Dmarid].DstAddr[1] = dstbufattribute->ulBaseUAddr +  dst_add_offset_u;
    	break;
    default:
        return MMP_DMA_ERR_NOT_SUPPORT; 
    }
	src_add_offset = srcbufattribute->usLineOffset*srcrect->usTop + srcrect->usLeft*gDmaRotateData[Dmarid].BytePerPixel[0];
    dst_add_offset = dstbufattribute->usLineOffset*usDststarty + usDststartx*gDmaRotateData[Dmarid].BytePerPixel[0];
    gDmaRotateData[Dmarid].SrcAddr[0] = srcbufattribute->ulBaseAddr  +  src_add_offset ;
    gDmaRotateData[Dmarid].DstAddr[0] = dstbufattribute->ulBaseAddr  +  dst_add_offset;
    gDmaRotateData[Dmarid].BufferIndex = 0;
    
    gDmaRotateData[Dmarid].MirrorEnable = mirrorEnable;
    gDmaRotateData[Dmarid].MirrorType = mirrortype;
  

    if(gDmaRotateData[Dmarid].BufferIndex < gDmaRotateData[Dmarid].BufferNum) {
   	    MMPF_DMA_Rotate(gDmaRotateData[Dmarid].SrcAddr[gDmaRotateData[Dmarid].BufferIndex], gDmaRotateData[Dmarid].DstAddr[gDmaRotateData[Dmarid].BufferIndex], 
               gDmaRotateData[Dmarid].SrcWidth[gDmaRotateData[Dmarid].BufferIndex], gDmaRotateData[Dmarid].SrcHeight[gDmaRotateData[Dmarid].BufferIndex], 
               gDmaRotateData[Dmarid].ColorDepth, gDmaRotateData[Dmarid].RotateType, 
               gDmaRotateData[Dmarid].SrcLineOffset[gDmaRotateData[Dmarid].BufferIndex], gDmaRotateData[Dmarid].DstLineOffset[gDmaRotateData[Dmarid].BufferIndex], Dmarid,
               gDmaRotateData[Dmarid].MirrorEnable, gDmaRotateData[Dmarid].MirrorType);
	}

    return MMP_ERR_NONE;
}




//------------------------------------------------------------------------------
//  Function    : MMPF_DMA_MoveData0
//  Description :
//  Note        : 1. The function doesn't use in ISR, because it called MMPF_OS_AcquireSem.
//                2. The execute time of DMARetFun must transient, because MMPF_DMA_ISR used the
//                   Callback function.
//------------------------------------------------------------------------------
MMP_ERR MMPF_DMA_MoveData0(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
                            MMP_ULONG ulCount, MMPF_DMA_M_LOFFS_DATA* ptrLineOffset,
                            DmaCallBackFunc *CallBackFunc)
{
	AITPS_DMA pDMA = AITC_BASE_DMA;
	#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr = 0;
	#endif
	MMP_ULONG ulTempAddress = 0x0;
	MMP_UBYTE ubStateCtl = 0x0; //State 1: Take care source address boundary issue.
								//State 2: Take care destination address boundary issue.
	MMP_ULONG ulTempCount = ulCount;
    if (gbDmaFreeM0 == MMP_FALSE) {
        RTNA_DBG_Str(0, "DMA_0 busy\r\n");
    }

    gbDmaEnCount++;
    gbDmaFreeM0 = MMP_FALSE;
    CallBackFuncM0 = CallBackFunc;
    mbDmaM0SpecialCase = MMP_FALSE;
   	
   	//Check if address is at dram 16MB boundary
    if((ulSrcaddr & 0xF000000)&&((ulSrcaddr + ulTempCount)&0xF000000) != (ulSrcaddr&0xF000000)) {
    	ubStateCtl = 0x1;
    	mbDmaM0SpecialCase = MMP_TRUE;
    }
    
    if((ulDstaddr & 0xF000000)&&((ulDstaddr + ulTempCount)&0xF000000) != (ulDstaddr&0xF000000)) {
    	if(ulDstaddr == 0x0) {
    		ubStateCtl = 0x2;
    	}
    	else {
    		if((ulSrcaddr & 0xF000000)&&((ulSrcaddr & 0xFFFFFF) >= (ulDstaddr & 0xFFFFFF))) {
    			ubStateCtl = 0x1;
    		}
    		else {
    			ubStateCtl = 0x2;
    		}
    	}
    	mbDmaM0SpecialCase = MMP_TRUE;
    }
    //dbg_printf(0,"dma(%x->%x) : %d\r\n",ulSrcaddr,ulDstaddr,mbDmaM0SpecialCase);
    
    if ((ptrLineOffset != NULL)&& (ubStateCtl != 0x0)) {
    	RTNA_DBG_Str(0, "DMA boundary detect error!!\r\n");
    	return MMP_DMA_ERR_PARAMETER;
    }
    
    //RTNA_DBG_PrintByte(0, ubStateCtl);
   
    
    if(ubStateCtl != 0x0) {
	    if(ubStateCtl == 0x1) {  //State 1: Take care source address boundary issue.
			ulTempAddress = (ulSrcaddr + ulTempCount);
			M0InfoDataPtr->ulSrcaddr = (ulTempAddress&0xF000000);
			M0InfoDataPtr->ulCount = ulTempAddress - (M0InfoDataPtr->ulSrcaddr);
			M0InfoDataPtr->ulDstaddr = (ulDstaddr + (ulTempCount - M0InfoDataPtr->ulCount));	
		}
		else if (ubStateCtl == 0x2) { //State 2: Take care destination address boundary issue.
			ulTempAddress = (ulDstaddr + ulTempCount);
			M0InfoDataPtr->ulDstaddr = (ulTempAddress&0xF000000);
			M0InfoDataPtr->ulCount = ulTempAddress - (M0InfoDataPtr->ulDstaddr);
			M0InfoDataPtr->ulSrcaddr = (ulSrcaddr + (ulTempCount - M0InfoDataPtr->ulCount));
		}
		M0InfoDataPtr->ptrLineOffset = ptrLineOffset;
		M0InfoDataPtr->CallBackFunc = CallBackFunc;
		ulCount = (ulTempCount - M0InfoDataPtr->ulCount);
		CallBackFuncM0 = NULL;
		
		ulCount = ulCount - CPU_MOVE_SIZE ;
	}
	
	
	#if (CHIP == VSN_V2)
    pDMA->DMA_M[MMPF_DMA_M_0].DMA_M_SRC_ADDR = ulSrcaddr;
    pDMA->DMA_M[MMPF_DMA_M_0].DMA_M_DST_ADDR = ulDstaddr;
    pDMA->DMA_M[MMPF_DMA_M_0].DMA_M_BYTE_CNT = ulCount;

    if (ptrLineOffset) {
        pDMA->DMA_M_LOFFS[MMPF_DMA_M_0].DMA_M_SRC_LOFFS_W = ptrLineOffset->SrcWidth;
        pDMA->DMA_M_LOFFS[MMPF_DMA_M_0].DMA_M_SRC_LOFFS_OFFS = ptrLineOffset->SrcOffset;
        pDMA->DMA_M_LOFFS[MMPF_DMA_M_0].DMA_M_DST_LOFFS_W = ptrLineOffset->DstWidth;
        pDMA->DMA_M_LOFFS[MMPF_DMA_M_0].DMA_M_DST_LOFFS_OFFS = ptrLineOffset->DstOffset;
        pDMA->DMA_M0_LOFFS_EN |= DMA_LOFFS_ENABLE;
    }
    else {
        pDMA->DMA_M0_LOFFS_EN &= ~DMA_LOFFS_ENABLE;
    }
	#endif
	
	#if (CHIP == VSN_V3)
	pDMA->DMA_M0.DMA_M_SRC_ADDR = ulSrcaddr;
    pDMA->DMA_M0.DMA_M_DST_ADDR = ulDstaddr;
    pDMA->DMA_M0.DMA_M_BYTE_CNT = ulCount;

    if (ptrLineOffset) {
        pDMA->DMA_M_LOFFS0.DMA_M_SRC_LOFFS_W = ptrLineOffset->SrcWidth;
        pDMA->DMA_M_LOFFS0.DMA_M_SRC_LOFFS_OFFS = ptrLineOffset->SrcOffset;
        pDMA->DMA_M_LOFFS0.DMA_M_DST_LOFFS_W = ptrLineOffset->DstWidth;
        pDMA->DMA_M_LOFFS0.DMA_M_DST_LOFFS_OFFS = ptrLineOffset->DstOffset;    
    }
    #endif
    
    pDMA->DMA_INT_CPU_SR = DMA_INT_M0;  //Clean interrup status
    
    OS_ENTER_CRITICAL();
    if (ptrLineOffset) {
    	#if (CHIP == VSN_V2)
    	pDMA->DMA_M0_LOFFS_EN |= DMA_LOFFS_ENABLE;
    	#endif
        #if (CHIP == VSN_V3)
    	pDMA->DMA_M_LOFFS_EN |= DMA_LOFFS_ENABLE_M0;
    	#endif
    }
    else {
    	#if (CHIP == VSN_V2)
    	pDMA->DMA_M0_LOFFS_EN &= ~DMA_LOFFS_ENABLE;
    	#endif
    	#if (CHIP == VSN_V3)
    	pDMA->DMA_M_LOFFS_EN &= ~DMA_LOFFS_ENABLE_M0;
    	#endif
    }
    pDMA->DMA_INT_CPU_EN |= DMA_INT_M0; //TBD, check if enabled at dma init
    #if (CHIP == VSN_V2)
    pDMA->DMA_EN = DMA_M_ENABLE;
    #endif
    #if (CHIP == VSN_V3)
    pDMA->DMA_EN = DMA_M0_ENABLE;
    #endif
    OS_EXIT_CRITICAL();
    
   

    return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_DMA_MoveData1
//  Description :
//  Note        : 1. The function doesn't use in ISR, because it called MMPF_OS_AcquireSem.
//                2. The execute time of DMARetFun must transient, because MMPF_DMA_ISR used the
//                   Callback function.
//------------------------------------------------------------------------------
MMP_ERR MMPF_DMA_MoveData1(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
								  MMP_ULONG ulCount, MMPF_DMA_M_LOFFS_DATA* ptrLineOffset,
								  DmaCallBackFunc *CallBackFunc)
{
	#if (CHIP == VSN_V2)
    return MMPF_DMA_MoveData0(ulSrcaddr, ulDstaddr, ulCount, NULL, CallBackFunc);
    #endif
    
    #if (CHIP == VSN_V3)
    AITPS_DMA pDMA = AITC_BASE_DMA;
	#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr = 0;
	MMP_ULONG ulTempAddress = 0x0;
	MMP_UBYTE ubStateCtl = 0x0; //State 1: Take care source address boundary issue.
								//State 2: Take care destination address boundary issue.
	MMP_ULONG ulTempCount = ulCount;
	#endif

    if (gbDmaFreeM1 == MMP_FALSE) {
        RTNA_DBG_Str(0, "DMA_1 busy\r\n");
    }

    gbDmaEnCount++;
    gbDmaFreeM1 = MMP_FALSE;
    CallBackFuncM1 = CallBackFunc;
    mbDmaM1SpecialCase = MMP_FALSE;
   	
   	//Check if address is at dram 16MB boundary
    if((ulSrcaddr & 0xF000000)&&((ulSrcaddr + ulTempCount)&0xF000000) != (ulSrcaddr&0xF000000)) {
    	ubStateCtl = 0x1;
    	mbDmaM1SpecialCase = MMP_TRUE;
    }
    
    if((ulDstaddr & 0xF000000)&&((ulDstaddr + ulTempCount)&0xF000000) != (ulDstaddr&0xF000000)) {
    	if(ulDstaddr == 0x0) {
    		ubStateCtl = 0x2;
    	}
    	else {
    		if((ulSrcaddr & 0xF000000)&&((ulSrcaddr & 0xFFFFFF) >= (ulDstaddr & 0xFFFFFF))) {
    			ubStateCtl = 0x1;
    		}
    		else {
    			ubStateCtl = 0x2;
    		}
    	}
    	mbDmaM1SpecialCase = MMP_TRUE;
    }
    
    if ((ptrLineOffset != NULL)&& (ubStateCtl != 0x0)) {
    	RTNA_DBG_Str(0, "DMA boundary detect error!!\r\n");
    	return MMP_DMA_ERR_PARAMETER;
    }
    if(mbDmaM1SpecialCase) {
        dbg_printf(3,"DMA 1 specialcase\r\n");
    }
    //RTNA_DBG_PrintByte(0, ubStateCtl);
   
    
    if(ubStateCtl != 0x0) {
	    if(ubStateCtl == 0x1) {  //State 1: Take care source address boundary issue.
			ulTempAddress = (ulSrcaddr + ulTempCount);
			M1InfoDataPtr->ulSrcaddr = (ulTempAddress&0xF000000);
			M1InfoDataPtr->ulCount = ulTempAddress - (M1InfoDataPtr->ulSrcaddr);
			M1InfoDataPtr->ulDstaddr = (ulDstaddr + (ulTempCount - M1InfoDataPtr->ulCount));	
		}
		else if (ubStateCtl == 0x2) { //State 2: Take care destination address boundary issue.
			ulTempAddress = (ulDstaddr + ulTempCount);
			M1InfoDataPtr->ulDstaddr = (ulTempAddress&0xF000000);
			M1InfoDataPtr->ulCount = ulTempAddress - (M1InfoDataPtr->ulDstaddr);
			M1InfoDataPtr->ulSrcaddr = (ulSrcaddr + (ulTempCount - M1InfoDataPtr->ulCount));
		}
		M1InfoDataPtr->ptrLineOffset = ptrLineOffset;
		M1InfoDataPtr->CallBackFunc = CallBackFunc;
		ulCount = (ulTempCount - M1InfoDataPtr->ulCount);
		CallBackFuncM1 = NULL;
	}
   
	pDMA->DMA_M1.DMA_M_SRC_ADDR = ulSrcaddr;
    pDMA->DMA_M1.DMA_M_DST_ADDR = ulDstaddr;
    pDMA->DMA_M1.DMA_M_BYTE_CNT = ulCount;

    if (ptrLineOffset) {
        pDMA->DMA_M_LOFFS1.DMA_M_SRC_LOFFS_W = ptrLineOffset->SrcWidth;
        pDMA->DMA_M_LOFFS1.DMA_M_SRC_LOFFS_OFFS = ptrLineOffset->SrcOffset;
        pDMA->DMA_M_LOFFS1.DMA_M_DST_LOFFS_W = ptrLineOffset->DstWidth;
        pDMA->DMA_M_LOFFS1.DMA_M_DST_LOFFS_OFFS = ptrLineOffset->DstOffset;    
    }
    
    pDMA->DMA_INT_CPU_SR = DMA_INT_M1;  //Clean interrup status
    
    OS_ENTER_CRITICAL();
    if (ptrLineOffset) {
    	 pDMA->DMA_M_LOFFS_EN |= DMA_LOFFS_ENABLE_M1;
    }
    else {
    	pDMA->DMA_M_LOFFS_EN &= ~DMA_LOFFS_ENABLE_M1;
    }
    pDMA->DMA_INT_CPU_EN |= DMA_INT_M1; //TBD, check if enabled at dma init
   
    pDMA->DMA_EN = DMA_M1_ENABLE;
  
    OS_EXIT_CRITICAL();
    
    return MMP_ERR_NONE;
    #endif
}

//------------------------------------------------------------------------------
//  Function    : MMPF_DMA_RotateData0
//  Description :
//  Note        : 1. The function doesn't use in ISR, because it called MMPF_OS_AcquireSem.
//                2. The execute time of DMARetFun must transient, because MMPF_DMA_ISR used the
//                   Callback function.
//------------------------------------------------------------------------------
MMP_ERR MMPF_DMA_RotateData0(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
								  MMP_ULONG ulOffset, MMP_ULONG ulWidth, MMP_ULONG ulHeight, 
								  MMP_ULONG ctl_type, MMP_ULONG mirror_type, DmaCallBackFunc *CallBackFunc)
{
	AITPS_DMA pDMA = AITC_BASE_DMA;
	#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr = 0;
	#endif

    if (gbDmaFreeR0 == MMP_FALSE) {
        RTNA_DBG_Str(0, "DMA_R busy\r\n");
    }

    if ((ulWidth > 0x2000) || (ulHeight > 0x2000)) {
        RTNA_DBG_Short(0, ulWidth);
        RTNA_DBG_Short(0, ulHeight);
        RTNA_DBG_Str(0, "DMA_R wrong width/height\r\n");
    }

    gbDmaEnCount++;
    gbDmaFreeR0 = MMP_FALSE;
    CallBackFuncR0 = CallBackFunc;

    pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_SRC_ADDR = ulSrcaddr;
    pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_DST_ADDR = ulDstaddr;
    pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_SRC_OFST = ulOffset;
	pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_DST_OFST = ulOffset;
	pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_PIX_W = ulWidth - 1;
	pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_PIX_H = ulHeight - 1;
	pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_CTL = ctl_type;
	pDMA->DMA_R[MMPF_DMA_R_0].DMA_R_MIRROR_EN = mirror_type;

    pDMA->DMA_INT_CPU_SR = DMA_INT_R0;
    OS_ENTER_CRITICAL();
    pDMA->DMA_INT_CPU_EN |= DMA_INT_R0; //TBD, check if enabled at dma init
    #if (CHIP == VSN_V2)
    pDMA->DMA_EN = DMA_R_ENABLE; 
    #endif
    #if (CHIP == VSN_V3)
    pDMA->DMA_EN = DMA_R0_ENABLE; 
    #endif
    OS_EXIT_CRITICAL();

	return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_DMA_MoveRotate1
//  Description :
//  Note        : 1. The function doesn't use in ISR, because it called MMPF_OS_AcquireSem.
//                2. The execute time of DMARetFun must transient, because MMPF_DMA_ISR used the
//                   Callback function.
//------------------------------------------------------------------------------
MMP_ERR MMPF_DMA_RotateData1(MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,
								  MMP_ULONG ulOffset, MMP_ULONG ulWidth, MMP_ULONG ulHeight,
								  MMP_ULONG ctl_type, MMP_ULONG mirror_type, DmaCallBackFunc *CallBackFunc)
{
    return MMP_ERR_NONE;
}
