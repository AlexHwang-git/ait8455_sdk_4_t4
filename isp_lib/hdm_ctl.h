#ifndef _HDM_CTL_H_
#define _HDM_CTL_H_


#include "isp_if.h"


void ISP_HDM_IF_CALI_GetData(void);
ISP_UINT8 ISP_HDM_IF_IQ_IsApicalClkOff(void);
ISP_UINT32 ISP_HDM_IF_LIB_GetBufAddr(ISP_BUFFER_CLASS buf_class, ISP_UINT32 buf_size, ISP_BUFFER_TYPE buf_type);

ISP_UINT32 ISP_HDM_IF_LIB_RamAddrV2P(ISP_UINT32 addr);
ISP_UINT32 ISP_HDM_IF_LIB_RamAddrP2V(ISP_UINT32 addr);
ISP_UINT32 ISP_HDM_IF_LIB_OprAddrV2P(ISP_UINT32 addr);
ISP_UINT32 ISP_HDM_IF_LIB_OprAddrP2V(ISP_UINT32 addr);

#endif // _HDM_CTL_H_

