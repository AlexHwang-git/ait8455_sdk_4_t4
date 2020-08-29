#ifndef _USB_DESC_DEF_H
#define _USB_DESC_DEF_H

#include "general.h"

typedef struct {
	U8		*pDescriptor;
	U16		TotalSize;
}DESC_RTN;	

typedef struct _SENSOR_FRAME_SIZE {
	
	U16		targetHsize;      
	U16		targetVsize;      
} SENSOR_FRAME_SIZE;

typedef struct
{
	SENSOR_FRAME_SIZE		resolution;
	U8						Num_support_FPS;
	U8						Frame_used;
}VS_FRAME_INFO;


typedef struct
{
	U16			desc_idx;
	U16			FrmTbl_idx;
	U8			desc_buf[64];			

}DESC_OP;





#define	VC_INTFNum									0x00						// Interface ID number of Video Control
#define	VS_INTFNum									0x01						// Interface ID number of Video Stream
#define	AC_INTFNum									0x02						// Interface ID number of Audio Control
#define	AS_INTFNum									0x03						// Interface ID number of Audio Stream



#define AUDIO_UID_IT								1							// UnitID of Audio Input Terminal
#define AUDIO_UID_OT								3							// UnitID of Audio Output Terminal
#define AUDIO_UID_FU								5							// UnitID of Audio Feature Unit



#define VIDEO_UID_CT								1							// UnitID of Video Camera Terminal
#define VIDEO_UID_PU								3							// UnitID of Video Process Unit
#define VIDEO_UID_OT								4							// UnitID of Video Output Terminal

#define VIDEO_UID_XU_DEVINFO						8							// UnitID of the Device information XU
#define VIDEO_UID_XU_TEST							9							// UnitID of the Test XU
#define VIDEO_UID_XU_VIDEO							10							// UnitID of the Video XU
#define VIDEO_UID_XU_PERI							11							// UnitID of the Peripheral XU

#define VIDEO_UID_XU1								2							// UnitID of eXtension Unit 1
#define VIDEO_UID_XU2								6							// UnitID of eXtension Unit 2
#define VIDEO_UID_XU3								5							// UnitID of eXtension Unit 2




#endif	//#ifndef _USB_DESC_DEF_H
