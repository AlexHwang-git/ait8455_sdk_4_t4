/*
 * nvram.h
 */
#ifndef	_NVRAM_H_
#define	_NVRAM_H_

#define	NVRAM_ADDRESS		(400 * 1024)
#define	NVRAM_MAX_ADDRESS	(512 * 1024)
#define NVRAM_SIZE			(NVRAM_MAX_ADDRESS - NVRAM_ADDRESS)

// For NVRam I/O
unsigned int nvread(void *nvaddr, char* data, unsigned int size);
unsigned int nvwrite(void *nvaddr, char* data, unsigned int size);

#endif	// _NVRAM_H_