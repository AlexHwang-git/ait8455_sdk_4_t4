/*
 * stdface.h
 */
// Global Variables
#include "config_fw.h"

#include "OkaoDef.h"

#define	TO_CACHE(p)		(0x80000000 | (unsigned int)p)
//
//extern unsigned int		kSystemTick;	// System Tick

//#define	Y_BUFFER			0x81680000
#if (CHIP==VSN_V2)
#define FRAME_POOL          0x01600000
#define	MEMPOOL_START		0x01700000
#endif
#if (CHIP==VSN_V3)
#define FRAME_POOL          (0x01000000 + 30 * 1024 * 1024 )
#define	MEMPOOL_START		(0x01000000 + 31 * 1024 * 1024 )
#endif

#define MEMPOOL_SIZE        (1024*1024)
#define	MEMPOOL_END			(MEMPOOL_START + MEMPOOL_SIZE)

int printk(const char *fmt, ...);
void hexdump(void* buf, short len);
void* mxinit(void *pool, unsigned long poolsize);
void* mxalloc(unsigned long memsize);
void mxfree(void *memptr);

// Face Processor Routines
#define FACE_DEFAULT	0
#define	RUNTIME_DEFAULT	1	
int initFaceProcessor(int mode);
int setFaceProcessor(int width, int height);
int resetFaceProcessor(void);
int imageFaceProcessor(char *yfrm);
int stopFaceProcessor(void);
void getFaceProcSize(short *pfw, short *pfh);

int myFaceProcess(char* yframe,int w,int h);
int IsFaceProcessDone(void);
void SetFaceProcessDone(void);
int imageFace(short *yuv);

// Face Album
void cleanFaceAlbum(signed char user, signed char data);
int flashFaceAlbum(void);

// Face Information
void ShowFaceParam(void);
void ShowFdrSpeed(void);
void ShowMessage(const char *fmt, ...);

//
// For OSD
#define	WHITE_COLOR		(short)0x80ff
#define	BLACK_COLOR		(short)0xff00
#define	GREEN_COLOR		(short)0x00ff
#define	RED_COLOR		(short)0xffff

void osdScreenLine(int linebytes);
int osdbufprint(int x, int y, short fclr, short bclr, const char* fmt, ...);
int osdbufprintSmall(int x, int y, short fclr, short bclr, const char* fmt, ...);
void begin_osd(void);
void update_osd(void);
//void paint_osd(short *);
void reset_osd(void);

// Text
void osdprint(short *screen, int x, int y, short fclr, short bclr, const char* fmt, ...);
// Rectangle
int osdbufrect(POINT* p1, POINT* p2, POINT* p3, POINT* p4, short clr);
int GetFaceSizeMax(void);
int GetFaceSizeMin(void);

//
// Face command in EU1
// Control Selector For Face FD/FR
// Notes, add new CS, it has to modify Extension Unit Descriptor (bmControls)
#define	EU1_CS_FACE					(0x02)
#define	EU1_CS_FACE_LEN				(0x20)

#define	MAX_DATA_LEN				30
typedef struct _FACE_CMD {
	char	cmd;
	char	dlen;
	char	d[MAX_DATA_LEN];
} FACE_CMD;

#define	EUCMD_FACE				0x60
#define	EUCMD_FACE_IN			0x61
#define	EUCMD_ALBUM_SAVE		0x62
#define	EUCMD_ALBUM_CLEAN		0x63
#define	EUCMD_ALBUM_RECALL		0x64
#define	EUCMD_FACE_TRANS		0x65
#define	EUCMD_FACE_DT_MODE		0x66
#define	EUCMD_FACE_DT_DENSITY	0x67
#define	EUCMD_FACE_DT_FACERANGE	0x68
#define	EUCMD_FACE_DT_THRESHOLD	0x69
#define	EUCMD_FACE_ID_THRESHOLD	0x6A
#define	EUCMD_FACE_DT_ANGLE		0x6B
#define	EUCMD_FACE_RESET		0x6C
#define	EUCMD_SETTING_SAVE		0x6D
#define	EUCMD_FACE_EN    		0x6E
// Trans data type
#define	BRAIN_TYPE				0x01
#define	FACE_DTPARM_TYPE		0x02

int faceWorker(FACE_CMD *fc);
char* faceGetCurrent(int size);

// Graphic
void setPaper(short *yuv, short width);
void setPenColor(short clr);
// Bresenham return number of pixel to plot
void Bresenham(int x1, int y1, int x2, int y2);
// drawRectangle return hiword = number of pixel to plot for a side
//                      loword = number of pixel to plot for another side
void drawRectangle(POINT* p);

// Debug
int printk(const char *format, ...);
void hexdump(void* buf, short len);


#include "nvram.h"
// 