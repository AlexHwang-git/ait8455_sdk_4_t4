/*
 * fdfrcfg.h
 */
#define	SUPPORT_YUV_DUAL_STREAM

#define	FD_WIDTH			640//960
#define	FD_HEIGHT			480//720

#define	FD_SOURCE_WIDTH		640//1280
#define	FD_SOURCE_HEIGHT	480//720

extern int	glVideoFDSrcAddr;
extern int	glVideoFDBufMaxSize;
extern char	gbVideoCurFDOutputFrame;

extern int	gibc1_ready;
extern int	gfdfrimg;

#include "stdface.h"