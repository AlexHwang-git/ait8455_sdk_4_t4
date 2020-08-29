// Memory management
/*
 * mxmem.h
 * mx Memory management
 *
 */
#include "string.h"
#include "mxmem.h"

typedef	unsigned long	ulong;

#define	MX_OK			0
#define	MX_ERR01		1
#define	MX_ERR02		2
#define	MX_ERR03		3

#define MX_HEADER		((ulong)sizeof(ulong))
#define MX_SIZE(p)		(*(ulong*)p)
#define MX_NEXT(p)		(*((ulong*)p + 1))
#define MX_CELLEND(p)	(p + MX_HEADER + MX_SIZE(p))

static char     *endmxptr  ;
static void		*mxfreeptr = 0;
static void		*mxroot;
static ulong	mxsize;
static char		mxerr;
extern void dbg_printf(unsigned int level, char *fmt, ...);
#define	DEBUG_MSG 	dbg_printf

/*
 * mxinit
 * Initialize memory pool and size
 */
void* mxinit(void *pool, unsigned long poolsize)
{
	mxroot = mxfreeptr = pool;
	mxsize = poolsize - MX_HEADER;
//	memset((char*)mxfreeptr + MX_HEADER, 0x55, mxsize);
	MX_SIZE(mxfreeptr) = mxsize;
	MX_NEXT(mxfreeptr) = 0;
	mxerr = MX_OK;
	DEBUG_MSG(3,"MXINIT- MX memory start %x size %x\r\n", mxroot, mxsize);
	endmxptr = 0 ;
	return mxfreeptr;
}

/*
 * mxalloc0
 * Alloc memory and fill 0
 */
#ifdef _MXMEM_DBG_
void* mxalloc0(unsigned long memsize, int line, char* file)
#else
void* mxalloc0(unsigned long memsize)
#endif
{
	void*	ptr;
	
#ifdef _MXMEM_DBG_
	ptr = mxalloc(memsize, line, file);
#else
	ptr = mxalloc(memsize);
#endif
	memset(ptr, 0, memsize);
	return ptr;
}

/*
 * mxalloc
 * Alloc memory
 */
#ifdef _MXMEM_DBG_
void* mxalloc(unsigned long memsize, int line, char* file)
#else
void* mxalloc(unsigned long memsize)
#endif
{
	char	*ptr, *tmp, *pre;
    
    //dbg_printf(3,"mxalloc size:%d\r\n",memsize);
    
	if (memsize == 0)
		return NULL;
	memsize = ((memsize + 3) >> 2) << 2;
	mxerr = MX_OK;
	ptr = (char*)mxfreeptr;
	if (!ptr)
		return NULL;
	pre = NULL;
	do {
		if (MX_SIZE(ptr) > (memsize + 2 * MX_HEADER)) {
			tmp = ptr;
			ptr += memsize + MX_HEADER;
			if (pre) {
				MX_NEXT(pre) = (ulong)ptr;
			} else {
				mxfreeptr = ptr;
			}
			MX_SIZE(ptr) = MX_SIZE(tmp) - (memsize + MX_HEADER);
			MX_NEXT(ptr) = MX_NEXT(tmp);
			MX_SIZE(tmp) = memsize;
			if (MX_SIZE(tmp) != memsize)
				DEBUG_MSG(3,"@@@@ Alloc %x/%x %x@@@@\r\n", (tmp + MX_HEADER), memsize, tmp + MX_HEADER);
#ifdef _MXMEM_DBG_
			DEBUG_MSG("alloc %x/%x>%s %d", tmp + MX_HEADER, MX_SIZE(tmp), file, line);
#endif 

            if( tmp > endmxptr) {
                endmxptr = tmp ;
                //dbg_printf(3,"mxcurptr : %x, usedsize : %d KB\r\n",endmxptr,(endmxptr - (char *)mxroot) >> 10) ;
            }

			return tmp + MX_HEADER;
		} else if (MX_SIZE(ptr) >= (memsize + MX_HEADER)) {
			if (pre)
				MX_NEXT(pre) = MX_NEXT(ptr);
			else
				mxfreeptr = (void*)MX_NEXT(ptr);
//			 DEBUG_MSG("@@@@ Alloc %x %x @@@@", (ptr + MX_HEADER), memsize);
#if 1//def _MXMEM_DBG_
			DEBUG_MSG(3,"alloc %x/%x/%x>%s %d", ptr + MX_HEADER, MX_SIZE(ptr), memsize);
#endif 
            if( ptr > endmxptr) {
                endmxptr = ptr ;
                //dbg_printf(3,"mxcurptr : %x, usedsize : %d KB\r\n",endmxptr,(endmxptr - (char *)mxroot) >> 10) ;
            }
			return ptr + MX_HEADER;
		}
		pre = ptr;
		ptr = (char*)MX_NEXT(ptr);
	} while (ptr);
	return NULL;
}

/*
 * mxrealloc
 * Realloc memory
 */
#ifdef _MXMEM_DBG_
void* mxrealloc(void *memptr, unsigned long resize, int line, char* file)
#else
void* mxrealloc(void *memptr, unsigned long resize)
#endif
{
	char	*newptr;
	ulong	orgsize;
	
	resize = ((resize + 3) >> 2 ) << 2;
	mxerr = MX_OK;
#ifdef _MXMEM_DBG_
	newptr = (char*)mxalloc(resize, line, file);
#else
	newptr = (char*)mxalloc(resize);
#endif
	if (newptr) {
		if (memptr) {
			orgsize = *(ulong*)((char*)memptr - MX_HEADER);
			memcpy(newptr, memptr, (resize > orgsize)? orgsize : resize);
			MXFREE(memptr);
		}
		return newptr;
	}
	return NULL;
}

/*
 * mxfree
 * Free memory
 */
#ifdef _MXMEM_DBG_
void mxfree(void *memptr, int line, char* file)
#else
void mxfree(void *memptr)
#endif
{
	char	*ptr, *the, *pre;
	
	// DEBUG_MSG("@@@@ Free %x @@@@", memptr);
	if (!memptr) {
#ifdef _MXMEM_DBG_
		DEBUG_MSG(3,"free is NULL, %s %d", file, line);
#endif
		return;
	}
	mxerr = MX_OK;
	ptr = (char*)memptr - MX_HEADER;
#ifdef _MXMEM_DBG_
	DEBUG_MSG(3,"free %x %x>%s %d", memptr, MX_SIZE(ptr), file, line);
#endif
	if ((ulong)ptr < (ulong)mxroot) {
#ifdef _MXMEM_DBG_
		mxerror(4, ptr, line, file);
#else
		mxerror(4, ptr);
#endif
		return;
	}
	if (MX_SIZE(ptr)    == 0      ||
		MX_SIZE(ptr)    >  mxsize ||
		MX_CELLEND(ptr) >  ((char*)mxroot + mxsize + MX_HEADER)) {
#ifdef _MXMEM_DBG_
		mxerror(5, ptr, line, file);
#else
		mxerror(5, ptr);
#endif
		return;
	}
	//memset(memptr, 0xcc, MX_SIZE(ptr));
	the = mxfreeptr;
	pre = NULL;
	do {
		if (ptr >= the && ptr < (the + MX_HEADER + MX_SIZE(the))) {
#ifdef _MXMEM_DBG_
			mxerror(6, ptr, line, file);
#else
			mxerror(6, ptr);
#endif
			return;
		}
		if (ptr < the) {
			if (MX_CELLEND(ptr) == the) {
				MX_SIZE(ptr) += MX_SIZE(the) + MX_HEADER;
				MX_NEXT(ptr)  = MX_NEXT(the);
			} else  if (MX_CELLEND(ptr) < the) {
				MX_NEXT(ptr) = (ulong)the;
			} else {
#ifdef _MXMEM_DBG_
				mxerror(2, ptr, line, file);
#else
				mxerror(2, ptr);
#endif
				return;
			}
			if (pre  == NULL) {
				mxfreeptr = (void*)ptr;
				return;
			} else {
				MX_NEXT(pre) = (ulong)ptr;
				if (MX_CELLEND(pre) == (char*)MX_NEXT(pre)) {
					MX_SIZE(pre) += MX_SIZE(ptr) + MX_HEADER;
					MX_NEXT(pre)  = MX_NEXT(ptr);
				}
				return;
			}
		}
		pre = the;
	} while ((the = (char*)MX_NEXT(the)) != NULL);
	the = pre;
	if (ptr == MX_CELLEND(the)) {
		MX_SIZE(the) += MX_SIZE(ptr) + MX_HEADER;
	} else {
		MX_NEXT(ptr) = MX_NEXT(the);
		MX_NEXT(the) = (ulong)ptr;
	}
}

/*
 * for DEBUG use
 */
void mxlayout()
{
	char	*ptr;
	int		i;
	
	ptr = (char*)mxfreeptr;
	i = 0;
	while (ptr) {
//		DEBUG_MSG("No.%03d Free Address, size: %x, %x %x",
//					i, ptr, MX_SIZE(ptr), MX_NEXT(ptr));
		if (!(char*)MX_NEXT(ptr))
			break;
		i++;
		ptr = (char*)MX_NEXT(ptr);
	}
	DEBUG_MSG(3,"No.%03d Free Address, size: %x, %x %x",
				i, ptr, MX_SIZE(ptr), MX_NEXT(ptr));
}

#ifdef _MXMEM_DBG_
void mxerror(char errcode, void *ptr, int line, char *file)
#else
void mxerror(char errcode, void *ptr)
#endif
{
#ifdef _MXMEM_DBG_
	DEBUG_MSG(3,"MX Error %d %x %d at %s %d\r\n", errcode, (unsigned long)ptr, MX_SIZE(ptr), file, line);
#else
	DEBUG_MSG(3,"MX Error %d %x %d\r\n", errcode, (unsigned long)ptr, MX_SIZE(ptr));
#endif
	mxerr = errcode;
}

