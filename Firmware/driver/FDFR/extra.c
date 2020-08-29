/*
 * extra.c
 */
#include "stdface.h"
#include "os_cpu.h"
#include "ucos_ii.h"

static	unsigned long top = 0;
void *OkaoExtraMalloc(unsigned int sSize)
{
	void	*p;

	p = mxalloc((unsigned long)sSize);
	if (p)
		if (top < (unsigned long)p + sSize) top = (unsigned long)p + sSize;
		
	/*RTNA_DBG_Str(0,"FDTCMalloc:");
	RTNA_DBG_Long(0,(unsigned  int)p);
	RTNA_DBG_Str(0,",size:");
	RTNA_DBG_Long(0,sSize);
	RTNA_DBG_Str(0,"\r\n");*/
	return p;
}

void OkaoExtraFree(void *pvBlock)
{
	mxfree(pvBlock);
}

unsigned long gettopheap()
{
	return top;
}

void resettopheap()
{
	top = 0;
}

unsigned int	mBegTime;
unsigned int OkaoExtraInitTime()
{
	mBegTime = 100;//OSTimeGet();
	return mBegTime;
}

unsigned int OkaoExtraGetTime(unsigned int tm)
{
	return 1000 - tm;
}