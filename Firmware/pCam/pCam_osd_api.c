/**
 @file pCam_osd.c
 @brief OSD general control Function
 @author 
 @version 1.0
*/
/*===========================================================================
 * Include files
 *===========================================================================*/ 

#include "includes_fw.h"
#include "pCam_osd_api.h"
#include "pCam_msg.h"
#include "mmpf_dma.h"


#if SUPPORT_OSD_FUNC

/*===========================================================================
 * Macro define
 *===========================================================================*/ 


/*===========================================================================
 * Extern function
 *===========================================================================*/ 

extern void dbg_printf(unsigned long level, char *fmt, ...);


/*===========================================================================
 * Global varible
 *===========================================================================*/ 


/*===========================================================================
 * Static varible
 *===========================================================================*/ 

static MMP_USHORT m_usCurrentW = 1280, m_usCurrentH = 720;
static MMP_USHORT m_usOsdStartXOffset = 40, m_usOsdStartYoffset = 120;
static MMP_BOOL   gbEnableOsd = 0 ;

//
// 
//
//static MMP_UBYTE  gbYColor = 0xc0,gbUColor = 240 ,gbVColor = 110 ,gbAlpha = 100 ;
static MMP_UBYTE  gbYColor = 52,gbUColor = 242 ,gbVColor = 147 ,gbAlpha = 100 ;

static MMP_USHORT m_OffScrBufW,m_OffScrBufH ;
static MMP_ULONG  m_OffScrBufAddr,m_OffScrBufUAddr;
static void bilinear_scale(MMP_UBYTE ori_buf[] , MMP_USHORT ori_w, MMP_USHORT ori_h,
					MMP_UBYTE new_buf[] , MMP_USHORT new_w, MMP_USHORT new_h,
					MMP_USHORT scale_m_h, MMP_USHORT scale_n_h,
					MMP_USHORT scale_m_v, MMP_USHORT scale_n_v,
					MMP_USHORT clip_L, MMP_USHORT clip_H,MMP_USHORT off);

MMP_ULONG m_OffScrScalingYBufAddr[2],m_OffScrScalingUBufAddr[2];
MMP_USHORT m_OffScrScalingW[2],m_OffScrScalingH[2];

MMP_ULONG m_OffScrCopyYBufAddr,m_OffScrCopyUBufAddr;
MMP_USHORT m_OffScrCopyW,m_OffScrCopyH;

/*===========================================================================
 * Main body
 *===========================================================================*/ 

void pcam_Int2Str(MMP_ULONG value, MMP_BYTE *string)
{
    MMP_ULONG i,j;
    MMP_BYTE   temp[16];
    
    for(i=0;i<16;i++) {
        if ( (value / 10) != 0 ) {
            temp[i] = '0' + value % 10;
            
            value = value / 10;
        }
        else {
            temp[i] = '0' + value % 10;
            break;
        }
    }
    
    for(j=0;j<(i+1);j++) {
        string[j] = temp[i - j];
    }
    
    string[j] = 0;
}



void _____OSD_Function_________(void){return;} //dummy

MMP_BOOL pcam_Osd_IsEnable(void)
{
    return gbEnableOsd ;
}

MMP_USHORT pcam_Osd_ConfigCurrentStream(MMP_BOOL en,MMP_USHORT pwCurrentW, MMP_USHORT pwCurrentH)
{
#if 0
    gbEnableOsd = en ;
#else
    gbEnableOsd = 0;// force disable
#endif    
    if( (!pwCurrentW) || (!pwCurrentH) ) {
        return PCAM_ERROR_NONE ;
    }
	m_usCurrentW 	= pwCurrentW;
	m_usCurrentH 	= pwCurrentH;
	//dbg_printf(3,"--osd en : %d,(w,h) : (%d,%d)\r\n",en,pwCurrentW,pwCurrentH);
	return PCAM_ERROR_NONE;
}


MMP_USHORT pcam_Osd_GetCurrentStreamRes(MMP_USHORT *pwCurrentW, MMP_USHORT *pwCurrentH)
{
 	*pwCurrentW = m_usCurrentW 	;
	*pwCurrentH = m_usCurrentH 	;
	return PCAM_ERROR_NONE;
}

void  pcam_Osd_SetColor(MMP_UBYTE yColor,MMP_UBYTE uColor,MMP_UBYTE vColor)
{
    gbYColor = yColor;
    gbUColor = uColor;
    gbVColor = vColor;
    
    dbg_printf(3,"[Y,U,V] = [%d,%d,%d]\r\n",gbYColor,gbUColor,gbVColor);
   // not support
   // gbAlpha  = alpha ;
}

void  pcam_Osd_SetPos(MMP_USHORT xPos,MMP_USHORT yPos)
{
    m_usOsdStartXOffset = xPos ;
    m_usOsdStartYoffset = yPos ;
}

void  pcam_Osd_DrawStr(const char *pStr, MMP_SHORT nStrLen, MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr)
{
#if RES_OSD_H24_FONT
    extern unsigned char	FontTable16x24[][2*24];
#endif

	MMP_UBYTE				Value;
	MMP_USHORT				i, j;

    if(!gbEnableOsd) {
        return ;
    }

	for (i = 0; i < OSD_FONT_H; i++) {
		for (j = 0; j < nStrLen * OSD_FONT_W; j++) {
#if RES_OSD_H24_FONT
			Value = FontTable16x24[pStr[j/OSD_FONT_W]-' '][(i<<1)+(j%OSD_FONT_W)/8];
#else
			Value = 0;
#endif

			if (Value & (1 << (7 - (j & 7)))) {

				*(MMP_UBYTE *)(ulYBufAddr + (i + m_usOsdStartYoffset) * m_usCurrentW + j + m_usOsdStartXOffset) =  gbYColor;						
				if(( !(j&1) ) && ( !(i&1) ))
				{
					*(MMP_UBYTE *)(ulUVBufAddr + (i + m_usOsdStartYoffset)/2 * m_usCurrentW + j + m_usOsdStartXOffset) 	=  gbUColor;
					*(MMP_UBYTE *)(ulUVBufAddr + (i + m_usOsdStartYoffset)/2 * m_usCurrentW + j + m_usOsdStartXOffset + 1) =  gbVColor;
				}	
			}
		}
	}
}


void pcam_Osd_AllocBuffer(MMP_USHORT osd_w,MMP_USHORT osd_h)
{

    MMP_ULONG size,i ;
    m_OffScrBufW = osd_w ;
    m_OffScrBufH = osd_h;  
     
    size = ( m_OffScrBufW * m_OffScrBufH * 3 ) / 2 ;
    m_OffScrBufAddr         = (MMP_ULONG)MMPF_SYS_AllocFB("OSD_SRC",size ,32,0);
    m_OffScrBufUAddr        = m_OffScrBufAddr + m_OffScrBufW * m_OffScrBufH  ;
    MEMSET( (char *)m_OffScrBufAddr,0,size);
    
    size = (( m_OffScrBufW * 6 ) * ( m_OffScrBufH * 6) ) * 3 /2;
    m_OffScrScalingYBufAddr[0] = (MMP_ULONG)MMPF_SYS_AllocFB("OSD_DST1",size ,32,0);
    m_OffScrScalingYBufAddr[1] = (MMP_ULONG)MMPF_SYS_AllocFB("OSD_DST2",size ,32,0);
    //m_OffScrCopyYBufAddr = (MMP_ULONG)MMPF_SYS_AllocFB("OSD_COPY",size ,32,0);
    
    //MEMSET( (char *)m_OffScrScalingYBufAddr[0],0,size);
    //MEMSET( (char *)m_OffScrScalingYBufAddr[1],0,size);
    dbg_printf(3,"m_OffScrBufAddr : %x,w:%d,h:%d\r\n",m_OffScrBufAddr,m_OffScrBufW,m_OffScrBufH );
    pcam_Osd_ConfigCurrentStream(MMP_TRUE,m_OffScrBufW,m_OffScrBufH);
    pcam_Osd_SetPos(0,0);
    pcam_Osd_SetColor((MMP_UBYTE)52,(MMP_UBYTE)242,(MMP_UBYTE)147); // blue
    pcam_Osd_DrawColor(m_OffScrBufAddr,m_OffScrBufUAddr,m_OffScrBufW,m_OffScrBufH);
    
    pcam_Osd_ScalingBuf(0, 10,10 );
    pcam_Osd_ScalingBuf(1, 10,10 );
}


void pcam_Osd_DrawColor(MMP_ULONG y_addr,MMP_ULONG u_addr,MMP_USHORT w,MMP_USHORT h)
{
    int i;
    MMP_UBYTE *ptr_y = (MMP_UBYTE *)y_addr ;
    MMP_UBYTE *ptr_u = (MMP_UBYTE *)u_addr ;
    for ( i= 0 ; i < w * h ; i++) {
        ptr_y[i] = gbYColor ;
    }
    for ( i= 0 ; i < w * h >> 1 ; i+=2) {
        ptr_u[i] = gbUColor ;
        ptr_u[i+1] = gbVColor ;
    }
    
    
}

MMP_ULONG pcam_Osd_GetOffScrBuffer(void)
{
    return m_OffScrBufAddr ;
}

MMPF_DMA_M_LOFFS_DATA gDmaLinOff ;
MMP_ULONG gbOsdDMASrcAddr, gbOsdDMADstAddr,glOsdDMASize ;
void pcam_Osd_DMADone(void)
{
    if (MMPF_DMA_MoveData1(gbOsdDMASrcAddr, gbOsdDMADstAddr, glOsdDMASize , &gDmaLinOff,0)) {
        RTNA_DBG_Str(0, "#Error : Mgr DMA move fail\r\n");
    }
}

//void pcam_Osd_DrawBuf(MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr ,MMP_ULONG ulSrcYBufAddr,MMP_ULONG ulSrcUBufAddr ,MMP_USHORT usSrcW,MMP_USHORT usSrcH)
void pcam_Osd_DrawBuf(MMP_USHORT buf_id,MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr )
{
    MMPF_DMA_M_LOFFS_DATA dmalineoff ;
	MMP_USHORT i, j,usSrcW,usSrcH;
	MMP_ULONG  dstOffY = 0,dstOffU = 0 ;
	MMP_ULONG  srcOffY = 0,srcOffU = 0 ;
	MMP_ULONG  srcYAddr,srcUAddr;
	MMP_ULONG  __us ;
	
	
	if(!gbEnableOsd) {
	    return ;
	}
	
	usSrcW = m_OffScrScalingW[buf_id] ;
	usSrcH = m_OffScrScalingH[buf_id] ;
	srcYAddr = m_OffScrScalingYBufAddr[buf_id] ;
	//srcUAddr = m_OffScrScalingYBufAddr[buf_id] +  usSrcW * usSrcH;
	srcUAddr = m_OffScrScalingUBufAddr[buf_id] ;
	__us = usb_vc_cur_usof();
	
	// 1st move DMA for Y
	dstOffY = ulYBufAddr + m_usOsdStartYoffset * m_usCurrentW + m_usOsdStartXOffset ;
	dmalineoff.SrcWidth = usSrcW;
	dmalineoff.SrcOffset= usSrcW;
	dmalineoff.DstWidth = usSrcW;
	dmalineoff.DstOffset= m_usCurrentW;
    // 2nd move DMA for U.V
    glOsdDMASize = usSrcW * usSrcH >> 1 ;
    gbOsdDMADstAddr = ulUVBufAddr + (m_usOsdStartYoffset * m_usCurrentW >> 1 )+ m_usOsdStartXOffset ;
    gbOsdDMASrcAddr = srcUAddr ;
	gDmaLinOff.SrcWidth = usSrcW;
	gDmaLinOff.SrcOffset= usSrcW;
	gDmaLinOff.DstWidth = usSrcW;
	gDmaLinOff.DstOffset= m_usCurrentW;
    if (MMPF_DMA_MoveData1(srcYAddr, dstOffY, usSrcW * usSrcH, &dmalineoff,pcam_Osd_DMADone)) {
        RTNA_DBG_Str(0, "#Error : Mgr DMA move fail\r\n");
    }
	//dbg_printf(0,"draw-t : %d\r\n",usb_vc_cur_usof() - __us );
    
}

void pcam_Osd_CopySrcBuf(MMP_USHORT buf_id,OSD_OP_DIR dir,MMP_ULONG ulYBufAddr, MMP_ULONG ulUVBufAddr)
{
    MMPF_DMA_M_LOFFS_DATA dmalineoff ;
	MMP_USHORT i, j,usSrcW,usSrcH;
	MMP_ULONG  dstOffY = 0,dstOffU = 0 ;
	MMP_ULONG  srcOffY = 0,srcOffU = 0 ;
	MMP_ULONG  srcYAddr,srcUAddr;
	
	// src/dst width,height
	m_OffScrCopyW = m_OffScrScalingW[buf_id] ;
	m_OffScrCopyH = m_OffScrScalingH[buf_id] ;
	
	
	// 1st move DMA for Y
	if(dir==BACKUP_IMAGE) {
    	srcOffY = ulYBufAddr + m_usOsdStartYoffset * m_usCurrentW + m_usOsdStartXOffset ;
    	dstOffY = m_OffScrCopyYBufAddr ;
    	
    	dmalineoff.SrcWidth = m_OffScrCopyW;
    	dmalineoff.SrcOffset= m_usCurrentW;
    	dmalineoff.DstWidth = m_OffScrCopyW;
    	dmalineoff.DstOffset= m_OffScrCopyW;
	}
	else {
	    srcOffY = m_OffScrCopyYBufAddr ;
	    dstOffY = ulYBufAddr + m_usOsdStartYoffset * m_usCurrentW + m_usOsdStartXOffset ;

    	dmalineoff.SrcWidth = m_OffScrCopyW;
    	dmalineoff.SrcOffset= m_OffScrCopyW;
    	dmalineoff.DstWidth = m_OffScrCopyW;
    	dmalineoff.DstOffset= m_usCurrentW;
	
	}
	
    // 2nd move DMA for U.V
    m_OffScrCopyUBufAddr = m_OffScrCopyYBufAddr + m_OffScrCopyW * m_OffScrCopyH ;
    glOsdDMASize = m_OffScrCopyW * m_OffScrCopyH >> 1 ;
    
    if(dir==BACKUP_IMAGE) {
        gbOsdDMASrcAddr = ulUVBufAddr + (m_usOsdStartYoffset * m_usCurrentW >> 1 )+ m_usOsdStartXOffset ;
        gbOsdDMADstAddr = m_OffScrCopyUBufAddr  ;
    	gDmaLinOff.SrcWidth = m_OffScrCopyW;
    	gDmaLinOff.SrcOffset= m_usCurrentW;
    	gDmaLinOff.DstWidth = m_OffScrCopyW;
    	gDmaLinOff.DstOffset= m_OffScrCopyW;
    }  
    else {
        gbOsdDMADstAddr = ulUVBufAddr + (m_usOsdStartYoffset * m_usCurrentW >> 1 )+ m_usOsdStartXOffset ;
        gbOsdDMASrcAddr = m_OffScrCopyUBufAddr  ;
    	gDmaLinOff.SrcWidth = m_OffScrCopyW;
    	gDmaLinOff.SrcOffset= m_OffScrCopyW;
    	gDmaLinOff.DstWidth = m_OffScrCopyW;
    	gDmaLinOff.DstOffset= m_usCurrentW;
    }
                  
    if (MMPF_DMA_MoveData1(srcOffY, dstOffY , m_OffScrCopyW * m_OffScrCopyH, &dmalineoff,pcam_Osd_DMADone)) {
        RTNA_DBG_Str(0, "#Error : Mgr DMA move fail\r\n");
    }
	
}



#if 0
	for (i = 0; i < usSrcH; i++) {
	    dstOffY = ( i + m_usOsdStartYoffset       ) * m_usCurrentW + m_usOsdStartXOffset ;
	    dstOffU = ( (i + m_usOsdStartYoffset) >> 1) * m_usCurrentW + m_usOsdStartXOffset ;
		for (j = 0; j < usSrcW ; j++) {
		    //dstOffY = ( i + m_usOsdStartYoffset      ) * m_usCurrentW + j + m_usOsdStartXOffset ;
		    //dstOffU = ((i + m_usOsdStartYoffset) >> 1) * m_usCurrentW + j + m_usOsdStartXOffset ;
		    
		    srcOffY = (i * usSrcW)      + j ;
		    srcOffU = (i >> 1) * usSrcW + j ;
		    
		    src_y = *(MMP_UBYTE *)(srcYAddr  + srcOffY    ) ;
		    src_u = *(MMP_UBYTE *)(srcUAddr  + srcOffU    ) ;
		    src_v = *(MMP_UBYTE *)(srcUAddr  + srcOffU+1  ) ;
		    
		    dst_y = *(MMP_UBYTE *)(ulYBufAddr   + dstOffY    ) ;
		    dst_u = *(MMP_UBYTE *)(ulUVBufAddr  + dstOffU    ) ;
		    dst_v = *(MMP_UBYTE *)(ulUVBufAddr  + dstOffU+1  ) ;
		    
		    if(src_y) {
		    	*(MMP_UBYTE *)(ulYBufAddr  + dstOffY    )   =  src_y ;//( src_y * gbAlpha + dst_y * (100 - gbAlpha) ) / 100 ;
			}
			if(src_u) {
			    *(MMP_UBYTE *)(ulUVBufAddr + dstOffU    )   =  src_u ;//( src_u * gbAlpha + dst_u * (100 - gbAlpha) ) / 100 ;
			}
			if(src_v) {
			    *(MMP_UBYTE *)(ulUVBufAddr + dstOffU + 1)   =  src_v ;//( src_v * gbAlpha + dst_v * (100 - gbAlpha) ) / 100 ;
			}    
		    dstOffY ++;
		    dstOffU ++;
		    //srcOffY ++;
		    //srcOffU ++;
		}
	}
#endif

void pcam_Osd_ScalingBuf(MMP_USHORT buf_id,MMP_USHORT m,MMP_USHORT n)
{
    MMP_USHORT scale_m_h = m ;
    MMP_USHORT scale_m_v = m ;
    MMP_USHORT scale_n_h = n ;
    MMP_USHORT scale_n_v = n ;
    MMP_USHORT scal_w,scal_h ;
    
    MMP_UBYTE  *y_addr,*u_addr ;
    MMP_ULONG  __us ;
    
    int i;
    if(!gbEnableOsd) {
        return ;
    }
    gbEnableOsd = MMP_FALSE ;
    do {
        scal_w = (scale_n_h * m_OffScrBufW + scale_m_h - 1 - 0) / scale_m_h;
        if( (scal_w&1)==0) {
            break ;
        }
        scale_n_h++ ;
    } while (1);
    
    
    do {
        scal_h = (scale_n_v * m_OffScrBufH + scale_m_v - 1 - 0) / scale_m_v;
        if( (scal_h&1)==0) {
            break ;
        }
        scale_n_v++ ;
        
    } while (1);
    m_OffScrScalingW[buf_id] = scal_w ; //m_OffScrScalingW[buf_id] + ( m_OffScrScalingW[buf_id] & 1) ;
    m_OffScrScalingH[buf_id] = scal_h ; //m_OffScrScalingH[buf_id] + ( m_OffScrScalingH[buf_id] & 1) ;
    dbg_printf(3,"[%d]m_OffScrScalingW  :%d,m_OffScrScalingH:%d\r\n",buf_id,m_OffScrScalingW[buf_id],m_OffScrScalingH[buf_id] );
   
    
     __us = usb_vc_cur_usof() ;
    
    bilinear_scale( (MMP_UBYTE *)m_OffScrBufAddr,m_OffScrBufW,m_OffScrBufH,
                        (MMP_UBYTE *)m_OffScrScalingYBufAddr[buf_id],m_OffScrScalingW[buf_id],m_OffScrScalingH[buf_id],
                    scale_m_h,scale_n_h,
                    scale_m_v,scale_n_v,
                    0,255,0);
                    
    m_OffScrScalingUBufAddr[buf_id] =  (m_OffScrScalingYBufAddr[buf_id] +  m_OffScrScalingW[buf_id] * m_OffScrScalingH[buf_id] );

    scal_w  =  m_OffScrScalingW[buf_id]  >> 1 ;//(scale_n_h * (m_OffScrBufW>>1) + scale_m_h - 1 - 0) / scale_m_h;
    scal_h  =  m_OffScrScalingH[buf_id]  >> 1 ;//(scale_n_v * (m_OffScrBufH>>1) + scale_m_v - 1 - 0) / scale_m_v;
    
    
    dbg_printf(3,"[%d]u_w  :%d,u_h:%d\r\n",buf_id,scal_w,scal_h );
    
    bilinear_scale( (MMP_UBYTE *)m_OffScrBufUAddr,m_OffScrBufW>>1,m_OffScrBufH>>1,
                        (MMP_UBYTE *)m_OffScrScalingUBufAddr[buf_id],scal_w,scal_h,
                    scale_m_h,scale_n_h,
                    scale_m_v,scale_n_v,
                    0,255,1);

    bilinear_scale( (MMP_UBYTE *)(m_OffScrBufUAddr+1),m_OffScrBufW>>1,m_OffScrBufH>>1,
                        (MMP_UBYTE *)(m_OffScrScalingUBufAddr[buf_id]+1),scal_w,scal_h,
                    scale_m_h,scale_n_h,
                    scale_m_v,scale_n_v,
                    0,255,1);
    
//    for ( i = 0 ; i <  (m_OffScrScalingW[buf_id] * m_OffScrScalingH[buf_id] >> 1 ) ;i+=2) {
//        u_addr[i]   = gbUColor ;
//        u_addr[i+1] = gbVColor ;
//    }
#if 0
    gbEnableOsd = MMP_TRUE ;
#endif
    
    dbg_printf(3,"scal-t : %d\r\n",usb_vc_cur_usof() - __us );
}


#define SCALER_BILINEAR_SHIFT		8//16

MMP_ULONG big_div(MMP_ULONG a, MMP_ULONG b);
MMP_ULONG roundRS(MMP_ULONG indata, unsigned int n);

MMP_ULONG aaa0,aaa1,bbb0,bbb1 ;


void bilinear_scale(MMP_UBYTE ori_buf[] , MMP_USHORT ori_w, MMP_USHORT ori_h,
					MMP_UBYTE new_buf[] , MMP_USHORT new_w, MMP_USHORT new_h,
					MMP_USHORT scale_m_h, MMP_USHORT scale_n_h,
					MMP_USHORT scale_m_v, MMP_USHORT scale_n_v,
					MMP_USHORT clip_L, MMP_USHORT clip_H,MMP_USHORT off)
{

    MMP_USHORT new_y ,new_x ;
    
	for (new_y = 0; new_y < new_h; new_y++) {
		for ( new_x = 0; new_x < new_w; new_x++) {

			MMP_ULONG ori_x = big_div(new_x * scale_m_h << SCALER_BILINEAR_SHIFT, scale_n_h);
			MMP_ULONG ori_y = big_div(new_y * scale_m_v << SCALER_BILINEAR_SHIFT, scale_n_v);

			MMP_ULONG fx0 = ori_x >> SCALER_BILINEAR_SHIFT;
			MMP_ULONG fy0 = ori_y >> SCALER_BILINEAR_SHIFT;


			if(1) {// (fx0 >= 0 && fx0 < ori_w - 1 && fy0 >= 0 && fy0 < ori_h - 1) {
				MMP_ULONG fx1 = fx0 ;
				MMP_ULONG fy1 = fy0 ;
				MMP_ULONG alpha = ori_x - (fx0 << SCALER_BILINEAR_SHIFT);
				MMP_ULONG beta  = ori_y - (fy0 << SCALER_BILINEAR_SHIFT);


				MMP_ULONG col1_ofstx = (fx0 < ori_w - 1) ? 0 : -1; 
				MMP_ULONG row1_ofsty = (fy0 < ori_h - 1) ? 0 : -ori_w * 1;


				MMP_ULONG val;
				
				aaa0 = (fy1 * ori_w + fx1 + col1_ofstx + row1_ofsty ) << off;
				aaa1 = (fy1 * ori_w + fx0              + row1_ofsty ) << off;
				bbb0 = (fy0 * ori_w + fx1 + col1_ofstx              ) << off;
				bbb1 = (fy0 * ori_w + fx0                           ) << off;
				//dbg_printf(0,"%d[%d,%d,%d,%d]\r\n",off,aaa0,aaa1,bbb0,bbb1);
				
				#if 1
				val = roundRS(
					              alpha                              * beta                              * ori_buf[aaa0]
								+ ((1<<SCALER_BILINEAR_SHIFT)-alpha) * beta                              * ori_buf[aaa1]
								+ alpha                              * ((1<<SCALER_BILINEAR_SHIFT)-beta) * ori_buf[bbb0]
								+ ((1<<SCALER_BILINEAR_SHIFT)-alpha) * ((1<<SCALER_BILINEAR_SHIFT)-beta) * ori_buf[bbb1]
								, SCALER_BILINEAR_SHIFT * 2);
				#else
				val = roundRS(
					              alpha                              * beta                              * ori_buf[fy1 * ori_w + fx1 + col1_ofstx + row1_ofsty]
								+ ((1<<SCALER_BILINEAR_SHIFT)-alpha) * beta                              * ori_buf[fy1 * ori_w + fx0              + row1_ofsty]
								+ alpha                              * ((1<<SCALER_BILINEAR_SHIFT)-beta) * ori_buf[fy0 * ori_w + fx1 + col1_ofstx]
								+ ((1<<SCALER_BILINEAR_SHIFT)-alpha) * ((1<<SCALER_BILINEAR_SHIFT)-beta) * ori_buf[fy0 * ori_w + fx0]
								, SCALER_BILINEAR_SHIFT * 2);
				#endif				
				new_buf[(new_y * new_w + new_x) << off] = (MMP_UBYTE)((val > clip_H) ? clip_H : (val < clip_L) ? clip_L : val);
			} else {
				new_buf[new_y * new_w + new_x] = 0;
			}
		}
	}
}

MMP_ULONG big_div(MMP_ULONG a, MMP_ULONG b)
{
    while (b >= 256) {
        a /= 2;
        b /= 2;
    }
    return a / b;
}

MMP_ULONG roundRS(MMP_ULONG indata, unsigned int n)
{
    MMP_ULONG Out;
    if (indata >= 0) {
        indata = indata >> (n-1);
        if ((indata & 0x01) == 1)
            Out = indata + 1;
        else
            Out = indata;

        Out = Out >> 1;
    } else {
        indata = (-indata) >> (n-1);
        if ((indata & 0x01) == 1)
            Out = indata + 1;
        else
            Out = indata;

        Out = Out >> 1;
        Out = -Out;
    }
    return(Out);
}


void _____RTC_Function_________(void){return;} //dummy



#endif//SUPPORT_OSD_FUNC

