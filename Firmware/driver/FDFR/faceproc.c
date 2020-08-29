/*
 * faceproc.c
 */
#include "string.h"
#include "stdface.h"
#include "OkaoAPI.h"
#include "OkaoDtAPI.h"
#include "OkaoPtAPI.h"
#include "OkaoFrAPI.h"
#include "isqrt.h"

#define printk //
#define hexdump // 
#define	IMGBUF_TASKMODE	0
#define	IMGBUF_UVCMODE	1

#define	FACEPROC_STOP	0
#define	FACEPROC_START	1

//#define	TRANS_X(x)	(((x - 160) << 1) / 3)
//#define	TRANS_Y(y)	((y << 1) / 3)
#define	TRANS_X(x)	((x << 1) / 3)
#define	TRANS_Y(y)	((y << 1) / 3)
#define	TRANS_POINT(p)	p.x = TRANS_X(p.x); p.y = TRANS_Y(p.y)

static int				bRollIn;
static int				bReset = 1;
static int				bFaceStart;
#ifdef	_OKAO_
static int		nMaxFace = 10;
static int		nMaxSwap = 0;
static unsigned int				_svw, _svh;
#endif

void usb_get_cur_image_size(UINT8 pipe, UINT32 *pw, UINT32 *ph);
int						gfdfrimg = 0;	// 1: for showing FDFR image
//
// Face Processor Parameters
// Notes: Don't Change order of elements, append new one from end.
//        Remember to update tool on PC.
typedef struct _FACEDT_PARM {
	INT32	fmode;		// OKAO_GetDtMode
	INT32	rangmin;	// OKAO_GetDtFaceSizeRange
	INT32	rangmax;
	INT32	nonstp;
	INT32	trkstp;		// OKAO_GetDtStep
	UINT32	nonang[POSE_TYPE_COUNT];
	UINT32	trkang;		// OKAO_GetDtAngle
	INT32	nonth;
	INT32	trkth;		// OKAO_GetDtThreshold
	INT32	ideth;		/* Identify Threshold */
} FACEDT_PARM;

#define	MAX_NUM_CELL	10
#define	BRAIN_MARK		0xADDAF001
#define	FP_TAG			'F' + ('P' << 8) + ('T' << 16) + ('G' << 24)
#define	FP_SIZ			sizeof(FACEDT_PARM) + 2 * sizeof(long)

typedef	struct _MYBRAIN {
	long			fptag;
	long			fpsiz;
	FACEDT_PARM		fpdat;
	long			mark;
	long			numcell;
	long			albumsize;
	long			map[MAX_NUM_CELL];
	char			cell[MAX_NUM_CELL][MAX_DATA_LEN];
} MYBRAIN;
MYBRAIN	myBrain = {BRAIN_MARK, MAX_NUM_CELL};
//
#define	IDENTIFY_THRESHOLD	500

typedef struct _FACEHDL {
	int				bImgBufMode;
	char			*fwinp;
	short			fwinW, fwinH;
	unsigned long	fwinS;
	HDETECTION		hfd;
	HDTRESULT		hfr;
	HPOINTER		hpt;	/* Facial Parts Detection handle */
	HPTRESULT		hpr;	/* Facial Parts Detection Result handle */
	HFEATURE		hff;	/* Face Recognition Data handle */
	HALBUM			hal;	/* Album handle */
	RECT rcNonTrackingArea;
	RECT rcTrackingArea;
	FACEDT_PARM		dtparm;
} FACEHDL;
static FACEHDL	theFace;

int imageFaceGetDTParm(HDETECTION hfd, FACEDT_PARM* fdp);

//
typedef	struct _FACEONSCR {
	int				dtid;
	int				hscore;
	int				nameid;
	int				focus;
} FACEONSCR;
static FACEONSCR	fos[MAX_NUM_CELL];
int faceFocus(FACEINFO *pfi, int rid, int rsc);
FACEONSCR* faceIsFocus(FACEINFO *pfi);

//
static char	freshman[MAX_DATA_LEN];
void* initBrain(unsigned char** album, unsigned int* size);
int IsInBrain(char* name, int *no);
int allocBrainCell(char* name, int* user, int* data);
int RollIn(char* name);
void dumpAlbum(void);
void resettopheap(void);
unsigned long gettopheap(void);

//

//static int		nMaxAlbumUser   = 5;
//static int		nMaxDataPerUser = 5;
static int		ltime = 0;
//
#define	MINFACE_SIZE	40
#define	MAXFACE_SIZE	400
char FacePosition[80+1];
static unsigned int kMemPoolAddress;
static unsigned int kMemPoolEnd;
extern void MMPF_MMU_ConfigWorkingBuffer(unsigned int startAddr, unsigned int endAddr, char cacheable);
extern void RTNA_DBG_Str(unsigned int level, char *str);
extern void RTNA_DBG_Short(unsigned int level,  unsigned short val);

void initFDTCMemory(){
	//dbg_printf(0, "initFDTCMemory\r\n");
	kMemPoolAddress = (unsigned int)MEMPOOL_START;
	kMemPoolEnd = MEMPOOL_END;
	mxinit((void*)kMemPoolAddress, MEMPOOL_SIZE);
	MMPF_MMU_ConfigWorkingBuffer(kMemPoolAddress, kMemPoolEnd, 1);
}
void resetFDTC(){
	//dbg_printf(0, "resetFDTC\r\n");

	bReset =1;
}
void resetFDTCMemory(){

	dbg_printf(0, "Close FDTC : resetFDTCMemory\r\n");
	resetFDTC();
	mxinit((void*)kMemPoolAddress, MEMPOOL_SIZE);
	//MMPF_MMU_ConfigWorkingBuffer(kMemPoolAddress,kMemPoolEnd,0);
}

int initFaceProcessor(int mode)
{
#ifdef	_OKAO_
	unsigned char major, minor;
	//dbg_printf(0, "initFaceProcessor\r\n");
	OKAO_GetVersion(&major, &minor);
	printk("OMRON Version %d.%d, Mode = %d\r\n", major, minor, mode);
	resettopheap();	
	//dbg_printf(3,"## &theFace.dtparm = %x ##\r\n",(unsigned long)&theFace.dtparm);
	if (mode == FACE_DEFAULT) {
		memset(&theFace.dtparm, 0, sizeof(FACEDT_PARM));
		theFace.bImgBufMode			= IMGBUF_TASKMODE;
		theFace.dtparm.fmode		= DT_MODE_DEFAULT;
		theFace.dtparm.rangmin      = MINFACE_SIZE;
		theFace.dtparm.rangmax      = MAXFACE_SIZE;
		theFace.dtparm.ideth		= IDENTIFY_THRESHOLD;
		theFace.rcNonTrackingArea.bottom = -1;
		theFace.rcNonTrackingArea.left   = -1;
		theFace.rcNonTrackingArea.right  = -1;
		theFace.rcNonTrackingArea.top    = -1;		

		theFace.rcTrackingArea.bottom = -1;
		theFace.rcTrackingArea.left   = -1;
		theFace.rcTrackingArea.right  = -1;
		theFace.rcTrackingArea.top    = -1;		
	}
#endif	
	return 0;
}

//
int setFaceProcessor(int width, int height)
{
#ifdef	_OKAO_
	unsigned char	*album;
	unsigned int	size;
	
	//dbg_printf(0, "setFaceProcessor\r\n");
	resetFaceProcessor();
	// To read album data and FDFR engine configurations
	// into myBrain
	initBrain(&album, &size);
	memcpy(&theFace.dtparm, &myBrain.fpdat, sizeof(FACEDT_PARM));
	//
	bFaceStart		= FACEPROC_START;
	theFace.fwinW   = width;
	theFace.fwinH   = height;
	theFace.fwinS   = theFace.fwinW * theFace.fwinH;
	theFace.hfd = OKAO_CreateDetection();
	if (!theFace.hfd) {
		printk("OKAO_CreateDetection ERROR!\r\n");
		return 1;
	}

	OKAO_SetDtMemorySize(theFace.hfd, 0); //
	theFace.hfr = OKAO_CreateDtResult(nMaxFace, nMaxSwap);

	if (!theFace.hfr) {
		printk("OKAO_CreateDtResult ERROR!\r\n");
		return 1;
	}
	if (OKAO_SetDtMode(theFace.hfd, theFace.dtparm.fmode) != OKAO_NORMAL) {
		printk("OKAO_SetDtMode ERROR!\r\n");
		return 1;
	}
	if (OKAO_SetDtFaceSizeRange(theFace.hfd,
								theFace.dtparm.rangmin,
								theFace.dtparm.rangmax) != OKAO_NORMAL) {
		printk("OKAO_SetDtFaceSizeRange ERROR!\r\n");
		return 1;
	}
	if (theFace.dtparm.nonstp != 0) {
		// Not program default, set it.
		if (OKAO_SetDtStep(theFace.hfd,
						   theFace.dtparm.nonstp,
						   theFace.dtparm.trkstp) != OKAO_NORMAL) {
			//ShowMessage("Set STEP ERROR!");
		}
	}
	if (theFace.dtparm.nonang[POSE_FRONT] != 0) {
		// Not program default, set it.
		if (OKAO_SetDtAngle(theFace.hfd,
							theFace.dtparm.nonang,
							theFace.dtparm.trkang) != OKAO_NORMAL) {
			//ShowMessage("Set ANGLE ERROR!");
		}
	}
	if (theFace.dtparm.nonth != 0) {
		// Not program default, set it.
		if (OKAO_SetDtThreshold(theFace.hfd,
								theFace.dtparm.nonth,
								theFace.dtparm.trkth) != OKAO_NORMAL) {
			//ShowMessage("Set Face Threshold Error!");
		}
	}
	#if 0

	theFace.hpt = OKAO_PT_CreateHandle();
	if (!theFace.hpt) {
		printk("OKAO_PT_CreateHandle ERROR!\r\n");
		return 1;
	}
	theFace.hpr = OKAO_PT_CreateResultHandle();
	if (!theFace.hpr) {
		printk("OKAO_PT_CreateResultHandle ERROR!\r\n");
		return 1;
	}

	theFace.hff = OKAO_FR_CreateFeatureHandle();
	if (!theFace.hff) {
		printk("OKAO_FR_CreateFeatureHandle ERROR!\r\n");
		return 1;
	}
	theFace.hal = NULL;
	if (album != NULL) {
		FR_ERROR	err;
		
		printk("Restore Album %d\r\n", size);
		if (size > 32) {
			theFace.hal = OKAO_FR_RestoreAlbum(album, size, &err);
			mxfree(album);
			if (!theFace.hal) {
				printk("OKAO_FR_RestoreAlbum ERROR!\r\n");
			}
		}
	}
	if (!theFace.hal) {
		printk("Create New Album\r\n");
		theFace.hal = OKAO_FR_CreateAlbumHandle(nMaxAlbumUser, nMaxDataPerUser);
		if (!theFace.hal) {
			printk("OKAO_FR_CreateAlbumHandle ERROR!\r\n");
			return 1;
		}
	}
	
	OKAO_SetDtDirectionMask(theFace.hfd, true);
//	OKAO_SetDtRectangleMask(theFace.hfd, 
	OKAO_SetDtTimeout(theFace.hfd, 0 ,0 );
	
	#endif
	OKAO_SetDtRectangleMask(theFace.hfd, theFace.rcNonTrackingArea, theFace.rcTrackingArea); 
	OKAO_SetDtDirectionMask(theFace.hfd, 1);
	OKAO_SetDtTimeout(theFace.hfd, 0 ,0 );
	
	imageFaceGetDTParm(theFace.hfd, &theFace.dtparm);
	
	// Clean fos
	memset(&fos, 0, sizeof(FACEONSCR) * MAX_NUM_CELL);
	// Will change to PIPE_3, 
	//usb_get_cur_image_size(PIPE_0,&_svw, &_svh);
	usb_get_cur_image_size(2, &_svw, &_svh);		//PIPE_0
	
#endif
	ltime = MMPF_BSP_GetTick();
	return 0;
}

void getFaceProcSize(short *pfw, short *pfh)
{
	*pfw = theFace.fwinW;
	*pfh = theFace.fwinH;
}

int stopFaceProcessor()
{
	bFaceStart = FACEPROC_STOP;
	return 0;
}

int resetFaceProcessor()
{
	//dbg_printf(0, "resetFaceProcessor\r\n");
#ifdef	_OKAO_
	if (theFace.hfd) {
		OKAO_DeleteDetection(theFace.hfd);
		theFace.hfd = NULL;
	}
#if 0	
	if (theFace.hfr) {
		OKAO_DeleteDtResult(theFace.hfr);
		theFace.hfr = NULL;
	}
	
	if (theFace.hpt) {
		OKAO_PT_DeleteHandle(theFace.hpt);
		theFace.hpt = NULL;
	}
	
	if (theFace.hpr) {
		OKAO_PT_DeleteResultHandle(theFace.hpr);
		theFace.hpr = NULL;
	}

	if (theFace.hff) {
		OKAO_FR_DeleteFeatureHandle(theFace.hff);
		theFace.hff = NULL;
	}
	if (theFace.hal) {
		OKAO_FR_DeleteAlbumHandle(theFace.hal);
		theFace.hal = NULL;
	}
#endif
	printk("resetFaceProcessor\r\n");
	theFace.fwinp = NULL;
	resettopheap();
	//reset_osd();
#endif
	return 0;
}

FACEINFO		fi[20];
//static int			theID[20];
//static int			theSC[20];
int			        faceCount=0;
int imageFace(short *yuv)
{
#ifdef	_OKAO_
	int		i;
	short	*src;
	char	*des;

	//dbg_printf(0, "imageFace\r\n");
	if (!theFace.fwinp || theFace.bImgBufMode == IMGBUF_TASKMODE)
		return 0;
	src = yuv;	
	des = theFace.fwinp;
	for (i = 0; i < theFace.fwinS; i++) {
		*des++ = (char)(*src++);
	}
#endif
	return 0;
}

#include "data_type.h"
#include "mmpf_graphics.h"
void Paint_FDTC_Rect(unsigned int yuvbuffer){
   MMPF_GRAPHICS_RECT rect= {50, 50,100,100} ;
   	MMPF_GRAPHICS_BUFATTRIBUTE bufattribute;
	//int			nFace;
	int i=0;

	
	bufattribute.colordepth = MMPF_GRAPHICS_COLORDEPTH_YUV422;
	bufattribute.usWidth = 640;
	bufattribute.usHeight = 480;
	bufattribute.usLineOffset = 640*2;
	bufattribute.ulBaseAddr = yuvbuffer;

	/*if(nFace > 0){
		RTNA_DBG_Str(0, "nFace:");
		RTNA_DBG_Short(0, nFace);
		RTNA_DBG_Str(0, "\r\n");		
	}*/

	for (i = 0; i < faceCount;  i++) {
		
		rect.usLeft = fi[i].ptLeftTop.x;
		rect.usTop= fi[i].ptLeftTop.y;
		rect.usWidth = fi[i].ptRightTop.x - fi[i].ptLeftTop.x;
		rect.usHeight = fi[i].ptLeftBottom.y - fi[i].ptLeftTop.y;
// patrick	
//		MMPF_Graphic_DrawRectToBuf(&bufattribute,&rect,1);
	}
	
}

//parameter:
//[out] void **face_pos :address of data buffer to recevice face position 
//return: data size :data size in byte
int GetFacePostion(void **face_pos){

	int i = 0, j=0;
/*	if(gbEnableDoFDTC==0)
	{
		FacePosition[0] = 0;	
		*face_pos = (void*)FacePosition;
		return 1;
	}
*/	
	FacePosition[0] = faceCount;
	{
		for (i = 1; i < (faceCount*8+1);  i+=8) {
			FacePosition[i] = fi[j].ptLeftTop.x & 0xFF;
			FacePosition[i+1] = (fi[j].ptLeftTop.x>>8) & 0xFF;

			FacePosition[i+2] = fi[j].ptLeftTop.y & 0xFF;
			FacePosition[i+3] = (fi[j].ptLeftTop.y>>8) & 0xFF;

			FacePosition[i+4] = fi[j].ptRightBottom.x & 0xFF;
			FacePosition[i+5] = (fi[j].ptRightBottom.x>>8) & 0xFF;

			FacePosition[i+6] = fi[j].ptRightBottom.y & 0xFF;
			FacePosition[i+7] = (fi[j].ptRightBottom.y>>8) & 0xFF;

		j++;	
		}
	}
	*face_pos = (void*)FacePosition;
	return faceCount * 8 + 1;

}
extern char gbFDTCDone;
int imageFaceProcessor(char *yfrm)
{
	int		ret = 0;
#ifdef	_OKAO_
	int			i, res;
	//int			cDataID, cUserID;
	//int			uina;
	//int			nRet;
	int			nFace;
	int			fsmin, fsmax;
	//FACEONSCR	*pfos;
	static int draw_counter=0;
	//
	//dbg_printf(0, "imageFaceProcessor\r\n");
	theFace.fwinp = (char*)(yfrm);
	res = OKAO_Detection(theFace.hfd, (RAWIMAGE*)theFace.fwinp,
						theFace.fwinW, theFace.fwinH,
						ACCURACY_NORMAL, theFace.hfr);
	if (res != OKAO_NORMAL) {
		//osdbufprint(0, 0, WHITE_COLOR, BLACK_COLOR, "face detection ERROR");
		ret = 1;
		goto ifp_END;
	}
	OKAO_GetDtFaceCount(theFace.hfr, &nFace);
	if (nFace == 0){
		draw_counter++;
		if(draw_counter==2)
			faceCount=0;
		
		goto ifp_END;
	}
	draw_counter=0;
	//if(nFace > 0){
		RTNA_DBG_Str(0, "-- nFace:");
		RTNA_DBG_Short(0, nFace);
		RTNA_DBG_Str(0, "\r\n");		
	//}
	/*
	osdbufprint(0, 0, WHITE_COLOR, BLACK_COLOR, "face %d", nFace);
	if (nFace == 0)
		goto ifp_END;
	//
	uina = 0;
	OKAO_FR_GetRegisteredUserNum(theFace.hal, &uina);*/
	// Extract Faces features	
	fsmin = 1000; fsmax = 0;
	faceCount = nFace;	
	for (i = 0; i < nFace;  i++) {
		//POINT	p1, p2, p3, p4;
 
		if (OKAO_GetDtFaceInfo(theFace.hfr, i, &fi[i]) != OKAO_NORMAL) {
			printk("OKAO_GetDtFaceInfo ERROR!\r\n");
			ret = 1;
			fi[i].nID = 0;
			goto ifp_END;
		}
	}

	#if 0
	for (i = 0; i < nFace;  i++) {
		POINT	p1, p2, p3, p4;
		
		if (OKAO_GetDtFaceInfo(theFace.hfr, i, &fi[i]) != OKAO_NORMAL) {
			printk("OKAO_GetDtFaceInfo ERROR!\r\n");
			ret = 1;
			goto ifp_END;
		}
		if (_svw == 640) {
			p1.x = TRANS_X(fi[i].ptLeftTop.x);
			p1.y = TRANS_Y(fi[i].ptLeftTop.y);
			p2.x = TRANS_X(fi[i].ptRightTop.x);
			p2.y = TRANS_Y(fi[i].ptRightTop.y);
			p3.x = TRANS_X(fi[i].ptRightBottom.x);
			p3.y = TRANS_Y(fi[i].ptRightBottom.y);
			p4.x = TRANS_X(fi[i].ptLeftBottom.x);
			p4.y = TRANS_Y(fi[i].ptLeftBottom.y);
		} else {
			p1.x = fi[i].ptLeftTop.x;
			p1.y = fi[i].ptLeftTop.y;
			p2.x = fi[i].ptRightTop.x;
			p2.y = fi[i].ptRightTop.y;
			p3.x = fi[i].ptRightBottom.x;
			p3.y = fi[i].ptRightBottom.y;
			p4.x = fi[i].ptLeftBottom.x;
			p4.y = fi[i].ptLeftBottom.y;
		}
		osdbufrect(&p1,
				   &p2,
				   &p3,
				   &p4,
				   WHITE_COLOR);
		// Get face size max and min
		fsize = fi[i].ptRightTop.x - fi[i].ptLeftTop.x;
		if (fi[i].ptRightTop.y != fi[i].ptLeftTop.y) {
			unsigned int	ys;
			
			ys = (fi[i].ptRightTop.y - fi[i].ptLeftTop.y) *
				 (fi[i].ptRightTop.y - fi[i].ptLeftTop.y);
			fsize = mcrowne_isqrt(ys + fsize * fsize);
		}
		if (fsmin > fsize) fsmin = fsize;
		if (fsmax < fsize) fsmax = fsize;
		//
		if (bRollIn == 0) {
			if ((pfos = faceIsFocus(&fi[i])) != NULL) {
				//osdbufprint(p4.x, p4.y,	GREEN_COLOR, BLACK_COLOR, "%s",
				//			myBrain.cell[pfos->nameid]);
				continue;
			}
		}
		if (OKAO_PT_SetPositionFromHandle(theFace.hpt, theFace.hfr, i) != OKAO_NORMAL) {
			printk("OKAO_PT_SetPositionFromHandle ERROR!\r\n");
			continue;
		}
		//
		if ((ret = OKAO_PT_DetectPoint(theFace.hpt,
								(RAWIMAGE*)theFace.fwinp,
								theFace.fwinW, theFace.fwinH,
								theFace.hpr)) != OKAO_NORMAL) {
			printk("OKAO_PT_DetectPoint ERROR %d!\r\n", ret);
			goto ifp_ERR;
		}
		if ((ret = OKAO_FR_ExtractFeatureFromPtHdl(theFace.hff, (RAWIMAGE*)theFace.fwinp,
											theFace.fwinW, theFace.fwinH,
											theFace.hpr)) != OKAO_NORMAL) {
			printk("OKAO_FR_ExtractFeatureFromPtHdl ERROR! %d\r\n", ret);
			goto ifp_ERR;
		}
		if (nFace == 1 && bRollIn == 1) {
			int		ret;
			bRollIn = 0;
			if ((ret = allocBrainCell(freshman, &cUserID, &cDataID)) != NULL) {
				/*if (ret == -1)
					ShowMessage("FULL,Cannot Add NEW user-%s!", freshman);
				else
					ShowMessage("FULL,Cannot Add NEW data-%s!", freshman);*/
				printk("Brain is full!\r\n");
				ret = 1;
				goto ifp_END;
			}
			if (OKAO_FR_RegisterData(theFace.hal, theFace.hff, cUserID, cDataID) != OKAO_NORMAL) {
				// Fail and clear
				/ShowMessage("Add User %s Failed!", freshman);
				myBrain.map[cUserID] &= ~(1 << cDataID);
				printk("OKAO_FR_RegisterData %d/%d ERROR!\r\n", cUserID, cDataID);
				ret = 1;
				goto ifp_END;
			}
			//ShowMessage("Register %s (%d/%d) Done", freshman, cUserID, cDataID + 1);
			printk("Register %d/%d\r\n", cUserID, cDataID);
		}
		if (uina == 0) {	// No User in Album
ifp_ERR:
			//osdbufprint(p4.x, p4.y, GREEN_COLOR, BLACK_COLOR, "#%d/%d", fi[i].nID, fi[i].nConfidence);
			continue;
		}
		if (OKAO_FR_Identify(theFace.hff, theFace.hal, nMaxAlbumUser, theID, theSC, &nRet) != OKAO_NORMAL) {
			//printk("OKAO_FR_Identify ERROR!\r\n");
		} else {
			//osdbufprint(p4.x, p4.y,	GREEN_COLOR, BLACK_COLOR, "%s?(%d)", myBrain.cell[theID[0]], theSC[0]);
			faceFocus(&fi[i], theID[0], theSC[0]);
		}
	}
	#endif
//	osdbufprint(384, 442, WHITE_COLOR, BLACK_COLOR, "Max/Min %d/%d", fsmax, fsmin);
	// FD/FR per sec
ifp_END:
#endif
//	osdbufprint(384, 426, WHITE_COLOR, BLACK_COLOR, "Mem %dK", ((gettopheap() - kMemPoolAddress) + 1024) >> 10);
	gbFDTCDone = 1;
	return ret;
}

int imageFaceGetDTParm(HDETECTION hfd, FACEDT_PARM* fdp)
{
#ifdef	_OKAO_
	//dbg_printf(0, "imageFaceGetDTParm\r\n");
	OKAO_GetDtMode(hfd, &fdp->fmode);
	OKAO_GetDtStep(hfd, &fdp->nonstp, &fdp->trkstp);
	OKAO_GetDtFaceSizeRange(hfd, &fdp->rangmin, &fdp->rangmax);
	OKAO_GetDtThreshold(hfd, &fdp->nonth, &fdp->trkth);
	OKAO_GetDtAngle(hfd, fdp->nonang, &fdp->trkang);
#endif
	return 0;
}

int faceFocus(FACEINFO *pfi, int rid, int rsc)
{
	if (rsc >= theFace.dtparm.ideth) {
		fos[rid].focus  = 1;
		fos[rid].dtid   = pfi->nID;
		fos[rid].nameid = rid;
		fos[rid].hscore = rsc;
	}
	return 0;
}

FACEONSCR* faceIsFocus(FACEINFO *pfi)
{
	int		i;
	
	for (i = 0; i < MAX_NUM_CELL; i++) {
		if (pfi->nID == fos[i].dtid) {
			return &fos[i];
		}
	}
	return NULL;
}

void ShowFdrSpeed()
{
	int			period;
    UINT32      size;
	period = MMPF_BSP_GetTick() - ltime;
	ltime  = MMPF_BSP_GetTick();
}

void ShowFaceParam()
{
	char		*info;
	int			x, y;
	FACEDT_PARM *fp;

	fp = &theFace.dtparm;
	switch (fp->fmode) {
	case DT_MODE_DEFAULT:
		info = "DT_MODE_DEFAULT";
		break;
	case DT_MODE_MOTION1:
		info = "Whole";
		break;
	case DT_MODE_MOTION2:	
		info = "3 Partition";
		break;
	case DT_MODE_MOTION3:
		info = "Gradual";
		break;
	}
	x  = 0;
	y  = 430;
	#if 0
	osdbufprintSmall(x, y,
				WHITE_COLOR, BLACK_COLOR,
				"Detection Mode: %s", info);
	y += 8;
	osdbufprintSmall(x, y,
				WHITE_COLOR, BLACK_COLOR,
				"Detection Threshold: %d/%d", fp->nonth, fp->trkth);
	y += 8;
	osdbufprintSmall(x, y,
				WHITE_COLOR, BLACK_COLOR,
				"Detection Density: %d/%d", fp->nonstp, fp->trkstp);
	y += 8;
	osdbufprintSmall(x, y,
				WHITE_COLOR, BLACK_COLOR,
				"Face Size: %d/%d", fp->rangmin, fp->rangmax);
	y += 8;
	osdbufprintSmall(x, y,
				WHITE_COLOR, BLACK_COLOR,
				"Identify Threshold: %d", fp->ideth);
	y += 8;
	osdbufprintSmall(x, y,
				WHITE_COLOR, BLACK_COLOR,
				"Face Angle: FRONT-%x HALF-%x PROFILE-%x, %x",
				fp->nonang[POSE_FRONT],
				fp->nonang[POSE_HALF_PROFILE],
				fp->nonang[POSE_PROFILE],
				fp->trkang);
	// Show Version
	x = 640 - 8 * 4;
	y = 480 - 8;
	osdbufprintSmall(x, y,
					WHITE_COLOR, BLACK_COLOR,
					"v2.5");
	#endif
}

static int	bFaceDone = 1;
extern unsigned long gTextPreviewBufAddr;

int myFaceProcess(char* yframe,int w,int h)
{
	//dbg_printf(0, "myFaceProcess\r\n");
	bFaceDone = 0;
	//begin_osd();
	
	if (bReset) {
		bReset = 0;
		//ShowMessage("Reset Face Engine...");
		//resetFaceProcessor();
		initFaceProcessor(RUNTIME_DEFAULT);
		setFaceProcessor(w, h);
	}
	if (bFaceStart == FACEPROC_START) {
		imageFaceProcessor(yframe);
	    ShowFdrSpeed();
		ShowFaceParam();
		//update_osd();
	} else {
		resetFaceProcessor();
	}
	return 0;
}

//
void* initBrain(unsigned char** album, unsigned int* size)
{
	//dbg_printf(0, "initBrain\r\n");
	bRollIn = 0;
	// Read Face Album
#if 1	
    memset((char *)&myBrain, 0x00, sizeof(MYBRAIN));
#else
	nvread((void*)0, (char*)&myBrain, sizeof(MYBRAIN));
#endif
	//memset(&myBrain,0, sizeof(MYBRAIN));
	hexdump(&myBrain.fptag, FP_SIZ);
	*album = NULL;
	if (myBrain.fptag != FP_TAG) {
		myBrain.fptag = FP_TAG;
		myBrain.fpsiz = FP_SIZ;
		myBrain.fpdat.fmode   = DT_MODE_DEFAULT;
		myBrain.fpdat.rangmin = MINFACE_SIZE;
		myBrain.fpdat.rangmax = MAXFACE_SIZE;
		myBrain.fpdat.nonstp  = 31;	// Face Engine default
		myBrain.fpdat.trkstp  = 31;	// Face Engine default
		myBrain.fpdat.nonang[POSE_FRONT] = 0;	// Face Engine default
		myBrain.fpdat.nonang[POSE_PROFILE] = 0;	// Face Engine default
		myBrain.fpdat.nonang[POSE_HALF_PROFILE] = 0;	// Face Engine default		
		myBrain.fpdat.trkang  = 0;	// Face Engine default
		myBrain.fpdat.nonth   = 0;	// Face Engine default
		myBrain.fpdat.trkth   = 0;	// Face Engine default
		myBrain.fpdat.ideth	  = IDENTIFY_THRESHOLD;
	}
	if (myBrain.mark == BRAIN_MARK && myBrain.numcell == MAX_NUM_CELL) {
		printk("Album size %d\r\n", myBrain.albumsize);
		if (myBrain.albumsize != 0 &&
			myBrain.albumsize < NVRAM_SIZE - sizeof(MYBRAIN)) {
			unsigned char	*ptr;
		
			ptr = (unsigned char*)mxalloc(myBrain.albumsize);
			//nvread((void*)sizeof(MYBRAIN), (char*)ptr, myBrain.albumsize);
			//memcpy( (char*)ptr,myBrain.albumsize);
			printk("Read Face album %d\r\n", myBrain.albumsize);
			*size  = myBrain.albumsize;
			*album = ptr;
			return ptr;
		}
	}
	myBrain.mark    = BRAIN_MARK;
	myBrain.numcell = MAX_NUM_CELL;
	myBrain.albumsize = 0;
	return NULL;
}
//
int IsInBrain(char* name, int *no)
{
	int		i;
	
	//dbg_printf(0, "IsInBrain\r\n");
	for (i = 0 ; i < myBrain.numcell; i++) {
		if (strcmp(name, myBrain.cell[i]) == 0) {
			*no = i;
			return 1;
		}
	}
	return 0;
}
int getFreeInBrain()
{
	int		i;
	
	for (i = 0; i < myBrain.numcell; i++) {
		if (myBrain.map[i] == 0)
			return i;
	}
	return -1;
}
//
int allocBrainCell(char* name, int* user, int* data)
{
	int		no, i;
	// Find user id
	if (!IsInBrain(name, &no)) {
		no = getFreeInBrain();
		if (no == -1) {
			// No Free Brain Cell for new user
			printk("No Free Brain Cell for new user\r\n");
			return no;
		}
	}
	if (myBrain.map[no] == 0xff) {
		// No free data for the user
		printk("No free data for the user %s\r\n", name);
		return -2;
	}
	// Find data id
	for (i = 0; i < 8; i++) {
		if (!(myBrain.map[no] & (0x01 << i)))
			break;
	}
	myBrain.map[no] |= (0x01 << i);
	if (!myBrain.cell[no][0]) {
		strcpy((char*)myBrain.cell[no], name);
	}
	// Got User/Data id, return success!
	*user = no;
	*data = i;
	printk("Got a free cell %d/%d for %s\r\n", *user, *data, (char*)myBrain.cell[no]);
	return 0;
}

int RollIn(char* name)
{
	bRollIn = 1;
	strcpy(freshman, name);
	return 0;
}

int flashFaceAlbum()
{
#if 0
#ifdef	_OKAO_
	unsigned char	*ptr;
	unsigned int	acb, total;
	
	if (OKAO_FR_GetSerializedAlbumSize(theFace.hal, &acb) != OKAO_NORMAL) {
		//ShowMessage("ERROR!");
		printk("OKAO_FR_GetSerializedAlbumSize ERROR!\r\n");
		return 1;
	}
	myBrain.albumsize = acb;
	total = sizeof(MYBRAIN) + acb;
	printk("Save Album %d\r\n", total);
	ptr = (unsigned char*)mxalloc(total);
	if (!ptr) {
		//ShowMessage("ERROR!");
		printk("flashFaceAlbum: The memory is insufficient!\r\n");
		return 1;
	}
	memcpy(ptr, &myBrain, sizeof(MYBRAIN));
	if (OKAO_FR_SerializeAlbum(theFace.hal, ptr + sizeof(MYBRAIN), acb) != OKAO_NORMAL) {
		//ShowMessage("ERROR!");
		printk("OKAO_FR_SerializeAlbum ERROR!\r\n");
		mxfree(ptr); 
		return 1;
	}
	nvwrite((void*)0, (char*)ptr, total);
	printk("Write Face Album %d to NV\r\n", total);
	mxfree(ptr);
	// Show Success on OSD
	//ShowMessage("Flash Done");
#endif
#endif
	return 0;
}

int flashSettings()
{
#ifdef	_OKAO_
	memcpy(&myBrain.fpdat, &theFace.dtparm, sizeof(FACEDT_PARM));
	myBrain.fptag = FP_TAG;
	myBrain.fpsiz = FP_SIZ;
	hexdump(&myBrain.fptag, FP_SIZ);
	//nvwrite((void*)0, (char*)&myBrain, sizeof(MYBRAIN) + myBrain.albumsize);
	// Show Success on OSD
	//ShowMessage("Flash Done");
#endif
	return 0;
}
/*
 * user = -1, to clean all in album
 * data = -1, to clean user in album
 */
void cleanFaceAlbum(signed char user, signed char data)
{
#if 0
#ifdef	_OKAO_
	if (user >= (signed char)nMaxAlbumUser || data >= (signed char)nMaxDataPerUser)
		return;
	if (user < 0) {
		int		i;
		
		if (OKAO_FR_ClearAlbum(theFace.hal) == OKAO_NORMAL) {
			//ShowMessage("Clean Album Done");
			for (i = 0; i < MAX_NUM_CELL; i++) {
				memset(myBrain.cell, 0, MAX_NUM_CELL * MAX_DATA_LEN); 
				myBrain.map[i] = 0;
			}
		} else {
			//ShowMessage("ERROR");
		}
		return;
	}
	if (data < 0) {
		if (OKAO_FR_ClearUser(theFace.hal, user) == OKAO_NORMAL) {
			// Show Success on OSD
			//ShowMessage("Delete User-%s Done", myBrain.cell[user]);
			memset(myBrain.cell[user], 0, MAX_DATA_LEN); 
			myBrain.map[user] = 0;
		} else {
			//ShowMessage("ERROR!");
		}
		return;
	}
	// TODO: Clear a specified data not finish yet!
	OKAO_FR_ClearData(theFace.hal, user, data);
#endif
#endif
}

void recallFaceAlbum()
{
#if 0
#ifdef	_OKAO_
	unsigned char	*album;
	unsigned int	size;
	
	if (initBrain(&album, &size) != NULL) {
		FR_ERROR	err;
		
		if (theFace.hal)
			OKAO_FR_DeleteAlbumHandle(theFace.hal);
			
		theFace.hal = OKAO_FR_RestoreAlbum(album, size, &err);
		mxfree(album);
		if (!theFace.hal) {
			//ShowMessage("ERROR!");
			printk("OKAO_FR_RestoreAlbum ERROR!\r\n");
		} else {
			//ShowMessage("Recall Album Done");
		}
	}
#endif
#endif
}

#define	MAX_TRANS_BUF	28
typedef struct _FACE_BUF {
	unsigned long	rest;
	char			d[MAX_TRANS_BUF];
} FACE_BUF;

typedef struct _TRANS_DATA {
	char			*src;
	unsigned int	total;
	unsigned int	curp;
	FACE_BUF		tbuf;
} TRANS_DATA;
static TRANS_DATA	td;

void transFaceData(int type)
{
	int		i, c;
	
	printk("Trans Type %x\r\n", type);
	switch (type) {
	case BRAIN_TYPE:
		td.src   = (char*)&myBrain.mark;
		td.total = sizeof(MYBRAIN) - FP_SIZ;
		td.curp  = 0;
		c = 0;
		for (i = 0; i < myBrain.numcell; i++)
			if (myBrain.map[i]) c++;
		//ShowMessage("Send %d users", c);
		break;
	case FACE_DTPARM_TYPE:
		imageFaceGetDTParm(theFace.hfd, &theFace.dtparm);
		td.src   = (char*)&theFace.dtparm;
		td.total = sizeof(FACEDT_PARM);
		td.curp  = 0;
		//ShowMessage("Send detection parameters");
		break;
	}
}

void dumpAlbum()
{
		/*//char	*ptr;
//	int		total;

	total = sizeof(MYBRAIN) + myBrain.albumsize;
	ptr = mxalloc(sizeof(MYBRAIN) + myBrain.albumsize);
	printk("Dump Face Album\r\n");
	nvread((void*)0, (char*)ptr, total);
	hexdump(ptr, total);
	mxfree(ptr);*/
}

#define	GET_ALBUM_USER(d)		((signed char)*(d))
#define	GET_ALBUM_DATA(d)		((signed char)*(d + 1))
#define	GET_FACEDATA_TYPE(d)	((int)*(d))
#define	GET_DETECTION_MODE(d)	(INT32)(*((char*)d))
#define	GET_NONTRACK_DENSITY(d)	(INT32)(*((short*)d))
#define	GET_TRACK_DENSITY(d)	(INT32)(*((short*)d + 1))
#define	GET_FACE_MINSIZE(d)		(INT32)(*((short*)d))
#define	GET_FACE_MAXSIZE(d)		(INT32)(*((short*)d + 1))
#define	GET_NON_THRESHOLD(d)	(INT32)(*((short*)d))
#define	GET_TRK_THRESHOLD(d)	(INT32)(*((short*)d + 1))
#define	GET_IDENTIFY_THRESHOLD(d)	(INT32)(*((short*)d))
#define	GET_FRONT_ANGLE(d)		(UINT32)((*(d))			  +	\
										 (*(d + 1) << 8)  +	\
										 (*(d + 2) << 16) + \
										 (*(d + 3) << 24));
#define	GET_H_PROFILE_ANGLE(d)	(UINT32)((*(d + 4))			  +	\
										 (*(d + 5) << 8)  +	\
										 (*(d + 6) << 16) + \
										 (*(d + 7) << 24));
#define	GET_PROFILE_ANGLE(d)	(UINT32)((*(d + 8))			  +	\
										 (*(d + 9) << 8)  +	\
										 (*(d +10) << 16) + \
										 (*(d +11) << 24));
#define	GET_EXT_ANGLE(d)		(UINT32)((*(d +12))			  +	\
										 (*(d +13) << 8)  +	\
										 (*(d +14) << 16) + \
										 (*(d +15) << 24));
// Receive Command from UVC Extension Unit No.6/CS 0x0A
extern MMP_BYTE gbEnableDoFDTC;
int faceWorker(FACE_CMD *fc)
{
	switch(fc->cmd) {
#ifdef	_OKAO_
    case EUCMD_FACE_EN:
		gbEnableDoFDTC = fc->d[0];
		if(gbEnableDoFDTC)
			RTNA_DBG_Str(0, "Enable FDTC\r\n");
		else
			RTNA_DBG_Str(0, "Disable FDTC\r\n");
		break;
	case EUCMD_FACE_IN:
		RollIn(fc->d);
		break;
	case EUCMD_ALBUM_SAVE:
		flashFaceAlbum();
		break;
	case EUCMD_SETTING_SAVE:
		flashSettings();
		break;
	case EUCMD_ALBUM_CLEAN:
		printk("Clean %d %d\r\n", GET_ALBUM_USER(fc->d), GET_ALBUM_DATA(fc->d));
		cleanFaceAlbum(GET_ALBUM_USER(fc->d), GET_ALBUM_DATA(fc->d));
		break;
	case EUCMD_ALBUM_RECALL:
		printk("EUCMD_ALBUM_RECALL\r\n");
		recallFaceAlbum();
		break;
	case EUCMD_FACE_TRANS:
		transFaceData(GET_FACEDATA_TYPE(fc->d));
		break;
	case EUCMD_FACE_DT_MODE:
		if (OKAO_SetDtMode(theFace.hfd, GET_DETECTION_MODE(fc->d)) == OKAO_NORMAL) {
			theFace.dtparm.fmode = GET_DETECTION_MODE(fc->d);
			printk("Set Detection mode to %d\r\n", (INT32)*(char*)fc->d);
		} else {
			printk("Set Detection mode: %d Error!\r\n", (INT32)*(char*)fc->d);
		}
		break;
	case EUCMD_FACE_DT_DENSITY:
		if (OKAO_SetDtStep(theFace.hfd, GET_NONTRACK_DENSITY(fc->d),
										GET_TRACK_DENSITY(fc->d)) == OKAO_NORMAL) {
			theFace.dtparm.nonstp = GET_NONTRACK_DENSITY(fc->d);
			theFace.dtparm.trkstp = GET_TRACK_DENSITY(fc->d);
		} else {
			printk("Set Search Density Error!\r\n");
		}
		break;
	case EUCMD_FACE_DT_FACERANGE:
		if (OKAO_SetDtFaceSizeRange(theFace.hfd, GET_FACE_MINSIZE(fc->d),
												 GET_FACE_MAXSIZE(fc->d)) == OKAO_NORMAL) {
			theFace.dtparm.rangmin = GET_FACE_MINSIZE(fc->d);
			theFace.dtparm.rangmax = GET_FACE_MAXSIZE(fc->d);
		} else {
			printk("Set Face Size Error!\r\n");
		}
		break;
	case EUCMD_FACE_DT_THRESHOLD:
		if (OKAO_SetDtThreshold(theFace.hfd, GET_NON_THRESHOLD(fc->d),
											 GET_TRK_THRESHOLD(fc->d)) == OKAO_NORMAL) {
			theFace.dtparm.nonth = GET_NON_THRESHOLD(fc->d);
			theFace.dtparm.trkth = GET_TRK_THRESHOLD(fc->d);
		} else {
			//ShowMessage("Set Face Threshold Error!");
		}
		break;
	case EUCMD_FACE_ID_THRESHOLD:
		theFace.dtparm.ideth = GET_IDENTIFY_THRESHOLD(fc->d);
		break;
	case EUCMD_FACE_DT_ANGLE: {
		UINT32	nontkAng[POSE_TYPE_COUNT];
		UINT32	tkAngExt;
		
		nontkAng[POSE_FRONT]		= GET_FRONT_ANGLE(fc->d);
		nontkAng[POSE_HALF_PROFILE] = GET_H_PROFILE_ANGLE(fc->d);
		nontkAng[POSE_PROFILE]		= GET_PROFILE_ANGLE(fc->d);
		tkAngExt                    = GET_EXT_ANGLE(fc->d);
		if (OKAO_SetDtAngle(theFace.hfd, nontkAng, tkAngExt) == OKAO_NORMAL) {
			theFace.dtparm.nonang[POSE_FRONT]        = nontkAng[POSE_FRONT];
			theFace.dtparm.nonang[POSE_HALF_PROFILE] = nontkAng[POSE_HALF_PROFILE];
			theFace.dtparm.nonang[POSE_PROFILE]      = nontkAng[POSE_PROFILE];
			theFace.dtparm.trkang                    = tkAngExt;
		} else {
			//ShowMessage("Set Detection Angle Error!");
		}
		break;
	}
#endif
	case EUCMD_FACE_RESET:
		bReset = 1;
		/* to show YUV420 for debugging*/
		// gfdfrimg ^= 1;
		break;
		
	default:
		return 1;	// Error!
	}
	return 0;
}

// for Get Cmd
char* faceGetCurrent(int size)
{
	int		c;
	if (size != sizeof(FACE_BUF)) {
		// This line to make sure transfer and buffer size are euqal
		printk("FATAL ERROR: TRANSER SIZE DOESN'T MATCH\r\n");
	}
	c = td.total - td.curp;
	if (c > MAX_TRANS_BUF)
		c = MAX_TRANS_BUF;
	memcpy(td.tbuf.d, (td.src + td.curp), c);
	td.tbuf.rest = td.total - td.curp;
	td.curp += c;
	return (char*)&td.tbuf;
}
