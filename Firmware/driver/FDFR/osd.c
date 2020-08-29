/*
 * osd.c
 */
#include "string.h"
#include "stdface.h"
#if 0
static void ShowTick(short *screen);
//static void ShowExtInfo(short *screen);
static short scrline = 640;

#define	MSG_SZLEN		80
typedef struct _MSG {
	int		cnt;
	char	sz[MSG_SZLEN];
} MSG;

#define	INFO_COUNTER	40
static MSG		msg;

void osdScreenLine(int linebytes)
{
	scrline = linebytes;
}

typedef void	(*PRINTSCREEN)(UINT16*, UINT32, UINT32, UINT16, UINT16, UINT8, char*);
typedef void	(*DRAWRECTANGLE)(POINT*);

void PrintStringYuv422 (UINT16 *dest_buf,	//image buffer
						UINT32 line_width,	//pixel per line
						UINT32 offset,		//string offset in text line
						UINT16 front_color,
						UINT16 back_color,
						UINT8 txt_interval,	//interval between text in pixel
						char* text);
void PrintStringYuv422Small(UINT16 *dest_buf,	//image buffer
							UINT32 line_width,	//pixel per line
							UINT32 offset,		//string offset in text line
							UINT16 front_color,
							UINT16 back_color,
							UINT8 txt_interval,	//interval between text in pixel
							char* text);

typedef struct _OSD_BUF {
	int				x;
	int				y;	
	int				fclr;
	int				bclr;
	PRINTSCREEN		font;
	char			str[80];
} OSD_BUF;

typedef struct _RECT_BUF {
	POINT			pt[4];
	int				clr;
	DRAWRECTANGLE	rectangle;
} RECT_BUF;

#define	OSDBUF_SIZE		30
typedef struct _OSD {
	int			no;
	int			av;
	OSD_BUF		buf[OSDBUF_SIZE];
	OSD_BUF		wrk[OSDBUF_SIZE];
	int			rtno;
	int			rtav;
	RECT_BUF	rtbuf[OSDBUF_SIZE];
	RECT_BUF	rtwrk[OSDBUF_SIZE];
} OSD;

OSD	_osd;

						
int print(char **out, int *varg);
extern short	*gTextPreviewBufAddr;

void begin_osd()
{
	_osd.av  = 0;
	_osd.rtav  = 0;
}

void reset_osd()
{
	_osd.no = 0;
	_osd.rtno = 0;
}

int osdbufprint(int x, int y, short fclr, short bclr, const char* fmt, ...)
{
	register int *varg = (int *)(&fmt);
	char	*p;
	int		i;

	if (_osd.av >= OSDBUF_SIZE)
		return OSDBUF_SIZE;
	i = _osd.av;
	p = _osd.buf[i].str;
	print((char**)&p, varg);
	_osd.buf[i].x = x;
	_osd.buf[i].y = y;
	_osd.buf[i].fclr = fclr;
	_osd.buf[i].bclr = bclr;
	_osd.buf[i].font = PrintStringYuv422;
	_osd.av++;
	return _osd.av;
}

int osdbufprintSmall(int x, int y, short fclr, short bclr, const char* fmt, ...)
{
	register int *varg = (int *)(&fmt);
	char	*p;
	int		i;

	if (_osd.av >= OSDBUF_SIZE)
		return OSDBUF_SIZE;
	i = _osd.av;
	p = _osd.buf[i].str;
	print((char**)&p, varg);
	_osd.buf[i].x = x;
	_osd.buf[i].y = y;
	_osd.buf[i].fclr = fclr;
	_osd.buf[i].bclr = bclr;
	_osd.buf[i].font = PrintStringYuv422;
	_osd.av++;
	return _osd.av;
}

int osdbufrect(POINT* p1, POINT* p2, POINT* p3, POINT* p4, short clr)
{
	int		i;
	
	if (_osd.rtav >= OSDBUF_SIZE)
		return OSDBUF_SIZE;
	i = _osd.rtav;
	_osd.rtbuf[i].pt[0] = *p1;
	_osd.rtbuf[i].pt[1] = *p2;
	_osd.rtbuf[i].pt[2] = *p3;
	_osd.rtbuf[i].pt[3] = *p4;
	_osd.rtbuf[i].clr   = clr;
	_osd.rtbuf[i].rectangle = drawRectangle;
	_osd.rtav++;
	return _osd.rtav;
}

void update_osd()
{
	memcpy(_osd.wrk, _osd.buf, sizeof(OSD_BUF) * _osd.av);
	memcpy(_osd.rtwrk, _osd.rtbuf, sizeof(RECT_BUF) * _osd.rtav);
	_osd.rtno = _osd.rtav;
	_osd.no   = _osd.av;
}

int MMPF_PLL_GetCPUFreq(unsigned int *ulCPUFreq);
void MMPF_MMU_FlushDCache(void);
void MMPF_MMU_FlushCacheAll(void);

void paint_osd(short *screen)
{
#ifdef	_OKAO_
	int		i;
	//
	// It is better to convert screen to cache area.
	// Otherwise, there are blinking osd on preview!
	// Why! Mmmmu, have to dig out.
	//screen = (short*)TO_CACHE(screen);
	ShowTick(screen);
	ShowExtInfo(screen);
	setPaper(screen, scrline);
	// Rectangle
	for (i = 0; i < _osd.rtno; i++) {
		setPenColor(_osd.rtwrk[i].clr);
		_osd.rtwrk[i].rectangle(_osd.rtwrk[i].pt);
	}
	// Text
	for (i = 0; i < _osd.no; i++) {
		_osd.wrk[i].font((unsigned short*)screen,
						 scrline,
						 _osd.wrk[i].x + _osd.wrk[i].y * scrline,
						 _osd.wrk[i].fclr, _osd.wrk[i].bclr,
						 0,
						 _osd.wrk[i].str);
	}
	// TODO: It doesn't work if call MMPF_MMU_FlushDCache!
	//       doesn't work means osd will be blinking after a while.
	//       Need to review cache program
	MMPF_MMU_FlushCacheAll();
//	MMPF_MMU_FlushDCache();
#else
	{
		unsigned int	f;
		
		MMPF_PLL_GetCPUFreq(&f);
		osdprint(screen, 2, 2, WHITE_COLOR, BLACK_COLOR, "%dMHz", f/1000);
		ShowTick(screen);
	}
#endif
}

int print(char **out, int *varg);
static char	osd_str[80];
void osdprint(short *screen, int x, int y, short fclr, short bclr, const char* fmt, ...)
{
	register int *varg = (int *)(&fmt);
	char	*p;

	p = osd_str;
	print((char**)&p, varg);
	PrintStringYuv422((UINT16*)screen,
					  (UINT32)scrline,
					  (UINT32)y * scrline + x,
					  fclr, bclr, 0,
					  osd_str);
}
int glCpuUsage=0;
void ShowTick(short *screen)
{
	static int		frms = 0;
	static unsigned int	ltime = 0;
	static int		fps;
	unsigned int	period;
	
	frms++;
	if (frms == 30) {
		period = kSystemTick - ltime;
		ltime  = kSystemTick;
		fps    = 1000 * frms * 10 / period;
		frms = 0;
	}
	osdprint(screen, 0, 20, WHITE_COLOR, BLACK_COLOR, "%d.%d %d,CPU:%d",
			fps / 10, fps - ((fps / 10) * 10), kSystemTick / 1000, glCpuUsage);
}

void ShowMessage(const char *fmt, ...)
{
	register int *varg = (int *)(&fmt);
	char	*p;

	msg.cnt = INFO_COUNTER;
	p = msg.sz;
	print((char**)&p, varg);
}

//
#define	EXT_INFO_X	0
#define	EXT_INFO_Y	36
//
void ShowExtInfo(short *screen)
{
	if (msg.cnt == 0)
		return;
	msg.cnt--;
	osdprint(screen, EXT_INFO_X, EXT_INFO_Y, RED_COLOR, BLACK_COLOR, msg.sz);
}
#endif
