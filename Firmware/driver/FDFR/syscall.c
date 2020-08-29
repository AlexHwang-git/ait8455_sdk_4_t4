/*
 * syscall.c
 */
typedef int (*SYS_PRINT)(const char *fmt, ...);

typedef	struct _SYSCALL_HDR {
		int			nSysC_CallID;
		int			nSysC_Version;
		SYS_PRINT	pSysC_printk;
		int			rSysC_Reserve[35];
} SYSCALL_HDR;

#define	SYSCALL_ID	('S') | ('Y' << 8) | ('S' << 16) | ('C' << 24)
#define	SYSCALL_VER	0x00000100

int printk(const char *format, ...);

#pragma arm section rodata="syscall"

SYSCALL_HDR sc_hdr = {
		SYSCALL_ID,
		SYSCALL_VER,
		printk,
		0
	};

int init_syscall()
{
	sc_hdr.nSysC_CallID  = SYSCALL_ID;
	sc_hdr.nSysC_Version = SYSCALL_VER;
	return 1;
}
