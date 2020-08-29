#include "avc_api.h"
#include <string.h>
#include "avc_sps_pps.h"

/*swyang, temp code*/
MMP_UBYTE g_SPS[MAX_SPS_NUM][32];
MMP_UBYTE g_PPS[MAX_PPS_NUM][32];
MMP_ULONG g_SPS_len[MAX_SPS_NUM];
MMP_ULONG g_PPS_len[MAX_PPS_NUM];
MMP_UBYTE g_sps_num;
MMP_UBYTE g_pps_num;

MMP_LONG decode_H264_init_header(AVC_DATA_BUF *buf)
{
//    int32_t     ret = 0;
    int32_t     size = 0;
    
    size = decode_avc_init_header(buf);
    
    return size;
}


MMP_ULONG	nalu_to_rbsp(MMP_UBYTE *buf, MMP_LONG len)
{
	MMP_LONG prefix_len;
    MMP_ULONG count = 0;
    MMP_UBYTE *src,*src2;
	MMP_UBYTE *src_end;
	
//	MMP_ULONG i;
    
    src_end = buf + len;
    
	if((buf[0] == 0x00) & (buf[1] == 0x00) & (buf[2] == 0x01))
		prefix_len = 3;
	else if((buf[0] == 0x00) & (buf[1] == 0x00) & (buf[2] == 0x00) & (buf[3] == 0x01))
		prefix_len = 4;
	else	
		prefix_len = 0;
	
	src = &(buf[prefix_len]);
	
	count++;
	
	src++;
	
	//NALU to RBSP
	while(src < src_end)
	{
		if( (src < (src_end - 3)) && src[0] == 0x00 && src[1] == 0x00  && src[2] == 0x03 )
		{
			//! double check this code section
			src2 = &src[2];
			while(src2 < src_end)
			{
				*src2 = *(src2+1);
				src2++;
			}
			src += 2;
			count += 2;
			src_end -= 1;
			
			continue;
		}
		src++;
		count++;
	}
	
	if(prefix_len != 0)
		memcpy(buf,buf + prefix_len,count);
	
    return count;
}
