#ifndef _GENERAL_H_
#define _GENERAL_H_

/* 8-bit */
#define CHAR            signed char 
#define UCHAR           unsigned char
#define PCHAR           signed char *
#define PUCHAR          unsigned char *
#define BYTE            unsigned char
#define U8              unsigned char
#define S8              signed char

/* 16-bit */
#define SHORT           signed short
#define USHORT          unsigned short
#define PSHORT          signed short *
#define PUSHORT         unsigned short *
#define WORD            unsigned short
#define U16             unsigned short
#define S16             signed short

/* 32-bit */
#define LONG            signed long
#define ULONG           unsigned long
#define PLONG           signed long *
#define PULONG          unsigned long *
#define DOUBLEWORD      unsigned long
#define U32             unsigned long
#define S32             signed long

/* boolean */
#define FALSE           0
#define TRUE            1

#define M_HiByteOfWord(x)				(unsigned char) ((x) >> 8)
#define M_LoByteOfWord(x)				(unsigned char) (x)
#define M_ByteSwapOfWord(x)				(((x) >> 8) | ((x) << 8))
#define M_HiByteOfDword(x)				(unsigned char) ((x) >> 24)
#define M_MidHiByteOfDword(x)			(unsigned char) ((x) >> 16)
#define M_MidLoByteOfDword(x)			(unsigned char) ((x) >> 8)
#define M_LoByteOfDword(x)				(unsigned char) (x)
#define M_ByteSwapOfDword(x)			(((unsigned long)(x) << 24) | (((unsigned long)(x) & 0x0000ff00) << 8) | (((unsigned long)(x) & 0x00ff0000) >> 8) | ((unsigned long)(x) >> 24))
#define M_UCHARToULONG(x,y,u,v)			(unsigned long) (((unsigned long)x)<<24)|(((unsigned long)y)<<16)|(((unsigned long)u)<<8)|((unsigned long)(v))
#define M_UCHARToUSHORT(x,y)			(unsigned short)  ((((unsigned short)x)<<8)|((unsigned short)y))

#define Read_NVRAM(structure, value,field, size, offset) 			\
{																	\
	USHORT bAddress = offset + offsetof(struct structure, field);	\
	NVRAM_RD(bAddress,value,size);									\
}							  										\

#define Write_NVRAM(structure, value, field,size, offset) 			\
{																	\
	USHORT bAddress = offset + offsetof(struct structure, field);	\
	NVRAM_WR(bAddress,value,size);									\
}							  										\


#endif		//#define _GENERAL_H_
