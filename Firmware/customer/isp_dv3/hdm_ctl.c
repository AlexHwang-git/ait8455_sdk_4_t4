#include "hdm_ctl.h"

#include "text_ctl.h"
#include "mmpf_sensor.h"
#include "mmpf_i2cm.h"





extern	MMP_ULONG	m_glISPBufferStartAddr;
extern  MMP_ULONG   m_glISPDmaBufferStartAddr ;
extern  ISP_UINT16	AF_Mar;
extern  ISP_UINT16	AF_Inf;
extern  ISP_UINT8	VCM_version;
MMP_ULONG	AEAcc_Buffer[256];
MMP_ULONG	AWBAcc_Buffer[256* 5];
MMP_ULONG	AFAcc_Buffer[60];
MMP_ULONG	HistAcc_Buffer[128];
MMP_ULONG   FlickerAcc_Buffer[32];

//#define	 AF_Inf_def	0x1E0

extern const ISP_UINT8 Sensor_IQ_CompressedText[];
extern const ISP_UINT8 Sensor_IQ_CompressedText1[];
extern const ISP_UINT8 Sensor_IQ_CompressedText2[];
extern ISP_UINT32 Sensor_IQ_TestText;
extern MMP_USHORT gsDev_ID;



 __align(4) ISP_UINT8 DstAddr[64*1024];
 __align(4) ISP_UINT8 TmpBufAddr[1024];


void ISP_HDM_IF_CALI_GetData(void)
{
	ISP_UINT8	*eeprom_buff;
	MMP_USHORT	print_cnt;
	  eeprom_buff = (ISP_UINT8 *)CALI_DATA_BUF_STA;


	//for(print_cnt = 0; print_cnt < 0x1528;print_cnt++)
	{	
		#if 1
		Read_Truly_EEPROM(0,eeprom_buff,(0x1528));
		#endif
		//Read_Truly_EEPROM(0,eeprom_buff,(0x10));
		//(*eeprom_buff) = print_cnt;
		//eeprom_buff++;
		
		
	}

	eeprom_buff = (ISP_UINT8 *)CALI_DATA_BUF_STA;
		
#if 0		//1
	//for(print_cnt = (1592 + 2816); print_cnt <100 + (1592 + 2816);print_cnt++)
	for(print_cnt = 28; print_cnt <100 + 28;print_cnt++)
	{
		dbg_printf(3, " I2C 0x%x  0x%x\r\n", print_cnt,  *(eeprom_buff+print_cnt));		
	}
#else
	for(print_cnt = ; print_cnt <= 0x1F;print_cnt++)
	{
		dbg_printf(3, " I2C 0x%x  0x%x\r\n", print_cnt,  *(eeprom_buff+print_cnt));		
	}
	for(print_cnt = 0xB00; print_cnt <= 0xB1F;print_cnt++)
	{
		dbg_printf(3, " I2C 0x%x  0x%x\r\n", print_cnt,  *(eeprom_buff+print_cnt));		
	}
	for(print_cnt = 0x1520; print_cnt <= 0x1528;print_cnt++)
	{
		dbg_printf(3, " I2C 0x%x  0x%x\r\n", print_cnt,  *(eeprom_buff+print_cnt));		
	}
#endif
#if 0
	*(eeprom_buff+0x20) = 0x0100;
	*(eeprom_buff+0x21) = 0x0100;
	*(eeprom_buff+0x22) = 0x0100;
	*(eeprom_buff+0x23) = 0x0100;
	/*
	*(eeprom_buff+0x24) = 0x00;
	*(eeprom_buff+0x25) = 0x01;
	*(eeprom_buff+0x26) = 0x00;
	*(eeprom_buff+0x27) = 0x01;*/
#endif
#if 0
    for(print_cnt = 0xB20; print_cnt < 0xB2A;print_cnt++)
	{
		dbg_printf(3, " SetIQ 0x00  0x%x\r\n",  *(eeprom_buff+print_cnt));		
	}
	//*(eeprom_buff + 0x28 ) = 0x80;
	//*(eeprom_buff + 0x29 ) = 0x07;
	//*(eeprom_buff +  0x2a) = 0x38;
	//*(eeprom_buff + 0x2b ) = 0x04;
	for(print_cnt = 0x638; print_cnt < (0x638+160);print_cnt++)
	{
		dbg_printf(3, " CS 0x00  0x%x\r\n",  *(eeprom_buff+print_cnt));		
	}
#endif

#if 0
	//R_gain = ISP_IF_CALI_GetAWBGainR();
	//dbg_printf(3, " RGain3  0x%x\r\n", R_gain);	
	ISP_IF_CALI_SetAWB_Bin(0, 5000, 1000, (ISP_UINT16 *)(eeprom_buff + 0x20));
	ISP_IF_CALI_SetAWB_Bin(1, 3000, 1000, (MMP_USHORT *)(eeprom_buff + 0xB20));
	//gain = ISP_IF_CALI_GetAWBGainR();
	//gain = ISP_IF_CALI_GetAWBGainG();
	//gain = ISP_IF_CALI_GetAWBGainB();
	//g_printf(3, " RGain3  0x%x  %x  %x\r\n", R_gain, G_gain, B_gain);
	ISP_IF_CALI_SetCS_Bin(0, 5000, 1000, (MMP_USHORT *)(eeprom_buff + 0x638));
	ISP_IF_CALI_SetCS_Bin(2, 3000, 1000, (MMP_USHORT *)(eeprom_buff + 0x1138));	
	ISP_IF_CALI_SetCS_Bin(3, 5000, 1000, (MMP_USHORT *)(eeprom_buff + 0x638));

	
	ISP_IF_CALI_SetLS_Bin(0, 5000, 1000, (MMP_USHORT *)(eeprom_buff + 0x28));
	ISP_IF_CALI_SetLS_Bin(2, 3000, 1000, (MMP_USHORT *)(eeprom_buff + 0xB28));
	ISP_IF_CALI_SetLS_Bin(3, 5000, 1000, (MMP_USHORT *)(eeprom_buff + 0x28));
#endif

/*	  //AlexH @ 20200828
	AF_Mar = *(eeprom_buff + 0x1522);
   	AF_Mar <<= 8;
   	AF_Mar |= *(eeprom_buff + 0x1521);
	//dbg_printf(3, "  AF_Mar %x  %x %x \r\n",  AF_Mar, *(eeprom_buff + 0x1522), *(eeprom_buff + 0x1521));
    AF_Inf = *(eeprom_buff + 0x1524);
    AF_Inf <<= 8;
    AF_Inf |= *(eeprom_buff + 0x1523);
	VCM_version = *(eeprom_buff + 0x1525);
*/
	AF_Mar = 0x330;
	AF_Inf = 0x2a0;
	VCM_version = 0;		//2;

	//dbg_printf(3, "  AF_Mar %x  %x %x \r\n",  AF_Inf, *(eeprom_buff + 0x1524), *(eeprom_buff + 0x1523));
	//if(AF_Inf <= AF_Inf_def)
	//	AF_Inf = AF_Inf_def;
	//MMPF_PIO_EnableGpioMode(CAM_PWR_EN, MMP_TRUE);
	//MMPF_PIO_EnableOutputMode(CAM_PWR_EN, MMP_TRUE);
	//MMPF_PIO_SetData(CAM_PWR_EN, MMP_FALSE);	
	//RTNA_WAIT_MS(10);	
}

ISP_UINT8 ISP_HDM_IF_IQ_IsApicalClkOff(void)
{
	return 1;
}

ISP_UINT32 ISP_HDM_IF_LIB_GetBufAddr(ISP_BUFFER_CLASS buf_class, ISP_UINT32 buf_size, ISP_BUFFER_TYPE buf_type)
{
	ISP_UINT32 addr = 0;

	switch (buf_class) {
	case ISP_BUFFER_CLASS_IQ_BIN_SRC:
		
		//addr = (ISP_UINT32)Sensor_IQ_CompressedText;
		addr = Sensor_IQ_TestText;
		break;
	case ISP_BUFFER_CLASS_IQ_BIN_DST:
		addr = (ISP_UINT32)DstAddr;
		break;
	case ISP_BUFFER_CLASS_IQ_BIN_TMP:
		addr = (ISP_UINT32)TmpBufAddr;
		break;
	case ISP_BUFFER_CLASS_MEMOPR_IQ:
		addr = IQ_BANK_ADDR;
		break;
	case ISP_BUFFER_CLASS_MEMOPR_LS:
		addr = LS_BANK_ADDR;
		break;
	case ISP_BUFFER_CLASS_MEMOPR_CS:
		addr = CS_BANK_ADDR;
		break;
	case ISP_BUFFER_CLASS_AE_HW:
		addr = (m_glISPBufferStartAddr + ISP_AWB_BUF_SIZE);
		break;
	case ISP_BUFFER_CLASS_AE_SW:
		addr = (ISP_UINT32)AEAcc_Buffer;
		break;
	case ISP_BUFFER_CLASS_AF_HW:
		addr = (m_glISPBufferStartAddr + ISP_AWB_BUF_SIZE + ISP_AE_BUF_SIZE);
		break;
	case ISP_BUFFER_CLASS_AF_SW:
		addr = (ISP_UINT32)AFAcc_Buffer;
		break;
	case ISP_BUFFER_CLASS_AWB_HW:
		addr = m_glISPBufferStartAddr;
		break;
	case ISP_BUFFER_CLASS_AWB_SW:
		addr = (ISP_UINT32)AWBAcc_Buffer;
		break;
	case ISP_BUFFER_CLASS_FLICKER_HW:
		addr = (m_glISPBufferStartAddr + ISP_AWB_BUF_SIZE + ISP_AE_BUF_SIZE + ISP_AF_BUF_SIZE + ISP_DFT_BUF_SIZE + ISP_HIST_BUF_SIZE);
		break;
	case ISP_BUFFER_CLASS_FLICKER_SW:
		addr = (ISP_UINT32)FlickerAcc_Buffer;
		break;
	case ISP_BUFFER_CLASS_HIST_HW:
		addr = (m_glISPBufferStartAddr + ISP_AWB_BUF_SIZE + ISP_AE_BUF_SIZE + ISP_AF_BUF_SIZE + ISP_DFT_BUF_SIZE);
		break;
	case ISP_BUFFER_CLASS_HIST_SW:
		addr = (ISP_UINT32)HistAcc_Buffer;
		break;
	default:
		break;
	}

	return addr;
}


ISP_UINT32 ISP_HDM_IF_LIB_RamAddrV2P(ISP_UINT32 addr)
{return addr;}
ISP_UINT32 ISP_HDM_IF_LIB_RamAddrP2V(ISP_UINT32 addr)
{return addr;}
ISP_UINT32 ISP_HDM_IF_LIB_OprAddrV2P(ISP_UINT32 addr)
{return addr;}
ISP_UINT32 ISP_HDM_IF_LIB_OprAddrP2V(ISP_UINT32 addr)
{return addr;}

#if DRAW_TEXT_FEATURE_EN
ISP_INT8 MultiShading;
ISP_INT16 CCM_PRT[9];

void VR_PrintStringOnPreview(void)
{
	if(1){
		
		if((*(MMP_UBYTE*)0x800070c4) & 0x01){

		//	_sprintf(gDrawTextBuf, (ISP_INT8*)"  Rid: %d, Bid= %d, RBpatcho = %x, Bratio= %x, R2ratio= %x, B2ratio= %x",(ISP_UINT32)ISP_IF_AWB_GetDbgData(11),  (ISP_UINT32)ISP_IF_AWB_GetDbgData(12),  (ISP_UINT32)ISP_IF_AWB_GetDbgData(13),(ISP_UINT32)ISP_IF_AWB_GetDbgData(14),(ISP_UINT32)ISP_IF_AWB_GetDbgData(6),(ISP_UINT32)ISP_IF_AWB_GetDbgData(7));
		//	VR_PrintString(gDrawTextBuf,  10, 0, 0x0000, 0x0000);
		//	_sprintf(gDrawTextBuf, (ISP_INT8*)"	 Lux=%x",(ISP_UINT32)ISP_IF_AE_GetLightCond());
		//	VR_PrintString(gDrawTextBuf,  30, 0, 0x0000, 0x0000);

		//	_sprintf(gDrawTextBuf, (ISP_INT8*)"	 pixel= %x, %x, %x, %x, %x, %x, %x, %x",(ISP_UINT32)ISP_IF_AWB_GetDbgData(24),(ISP_UINT32)ISP_IF_AWB_GetDbgData(25),(ISP_UINT32)ISP_IF_AWB_GetDbgData(26),(ISP_UINT32)ISP_IF_AWB_GetDbgData(27),(ISP_UINT32)ISP_IF_AWB_GetDbgData(28),(ISP_UINT32)ISP_IF_AWB_GetDbgData(29),(ISP_UINT32)ISP_IF_AWB_GetDbgData(30),(ISP_UINT32)ISP_IF_AWB_GetDbgData(31));
		//	VR_PrintString(gDrawTextBuf,  40, 0, 0x0000, 0x0000);
		//	_sprintf(gDrawTextBuf, (ISP_INT8*)"	 Wei= %x, %x, %x, %x, %x, %x, %x, %x",(ISP_UINT32)ISP_IF_AWB_GetDbgData(56),(ISP_UINT32)ISP_IF_AWB_GetDbgData(57),(ISP_UINT32)ISP_IF_AWB_GetDbgData(58),(ISP_UINT32)ISP_IF_AWB_GetDbgData(59),(ISP_UINT32)ISP_IF_AWB_GetDbgData(60),(ISP_UINT32)ISP_IF_AWB_GetDbgData(61),(ISP_UINT32)ISP_IF_AWB_GetDbgData(62),(ISP_UINT32)ISP_IF_AWB_GetDbgData(63));
		//	VR_PrintString(gDrawTextBuf,  80, 0, 0x0000, 0x0000);			
			_sprintf(gDrawTextBuf, (ISP_INT8*)"AWB : Mode=%x, GainR=%x, GainGr=%x, GainB=%x, CT = %x,%x", (ISP_UINT32)ISP_IF_AWB_GetMode(), (ISP_UINT32)ISP_IF_AWB_GetGainR(), (ISP_UINT32)ISP_IF_AWB_GetGainG(), (ISP_UINT32)ISP_IF_AWB_GetGainB(),(ISP_UINT32)ISP_IF_AWB_GetColorTemp(),(ISP_UINT32)ISP_IF_AWB_GetMode());
		VR_PrintString(gDrawTextBuf,  30, 0, 0x0000, 0x0000);
		}else{
	//		_sprintf(gDrawTextBuf, (ISP_INT8*)"  AE : Avglum=%x, Target=%x, Lux=%x, AE_Status:%x", (ISP_UINT32)ISP_IF_AE_GetCurrentLum(), (ISP_UINT32)ISP_IF_AE_GetTargetLum(), (ISP_UINT32)ISP_IF_AE_GetLightCond(), (ISP_UINT32)ISP_IF_AE_GetStatus());
	//		VR_PrintString(gDrawTextBuf,  20, 0, 0xFFFF, 0x0000);	
	//				_sprintf(gDrawTextBuf, (ISP_INT8*)"  UVM10 : 0x70f6=%x, 0x70f9=%x,", (ISP_UINT32)ISP_IF_IQ_GetOpr(0x70f6,1), (ISP_UINT32)(ISP_UINT32)ISP_IF_IQ_GetOpr(0x70f9,1));
	//		VR_PrintString(gDrawTextBuf,  50, 0, 0xFFFF, 0x0000);
			sprintf(gDrawTextBuf, (ISP_INT8*)"  Rid: %d, Bid= %d",(ISP_UINT32)ISP_IF_AWB_GetDbgData(11),  (ISP_UINT32)ISP_IF_AWB_GetDbgData(12));
			VR_PrintString(gDrawTextBuf,  10, 0, 0x0000, 0x0000);
			
			sprintf(gDrawTextBuf, (ISP_INT8*)"  Tid: %d",ISP_IF_IQ_GetID(2));
			VR_PrintString(gDrawTextBuf,  20, 0, 0x0000, 0x0000);
			sprintf(gDrawTextBuf, (ISP_INT8*)"AWB : Mode=%x, GainR=%x, GainGr=%x, GainB=%x, CT = %x,%x", (ISP_UINT32)ISP_IF_AWB_GetMode(), (ISP_UINT32)ISP_IF_AWB_GetGainR(), (ISP_UINT32)ISP_IF_AWB_GetGainG(), (ISP_UINT32)ISP_IF_AWB_GetGainB(),(ISP_UINT32)ISP_IF_AWB_GetColorTemp(),(ISP_UINT32)ISP_IF_AWB_GetMode());
		VR_PrintString(gDrawTextBuf,  30, 0, 0x0000, 0x0000);
			
						
		}


	 }
}
#endif

