// Memory management
/*
 * mxmem.h
 * mx Memory management
 */
#ifndef	MXMEM
#define	MXMEM

#ifdef _MXMEM_DBG_
#define	MXALLOC(s)		mxalloc(s, __LINE__, __FILE__)
#define MXREALLOC(p, s)	mxrealloc(p, s, __LINE__, __FILE__)
#define	MXFREE(p)		mxfree(p, __LINE__, __FILE__)
void* mxinit(void *pool, unsigned long poolsize);
void* mxalloc(unsigned long memsize, int lines, char* file);
void* mxalloc0(unsigned long memsize, int lines, char* file);
void* mxrealloc(void *memptr, unsigned long resize, int lines, char* file);
void* mxmoveHi(void **memptr, char bcopy);
void mxfree(void *memptr, int lines, char* file);
char mxstatus(void);
void mxlayout(void);
void mxerror(char errcode, void *ptr, int line, char *file);
#else
void* mxinit(void *pool, unsigned long poolsize);
#define	MXALLOC(s)		mxalloc(s)
#define MXREALLOC(p, s)	mxrealloc(p, s)
#define	MXFREE(p)		mxfree(p)
void* mxalloc(unsigned long memsize);
void* mxalloc0(unsigned long memsize);
void* mxrealloc(void *memptr, unsigned long resize);
void* mxmoveHi(void **memptr, char bcopy);
void mxfree(void *memptr);
char mxstatus(void);
void mxlayout(void);
void mxerror(char errcode, void *ptr);
#endif	// _MXMEM_DBG_

#endif	//MXMEM