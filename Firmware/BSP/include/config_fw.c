#include "mmpf_typedef.h"
#include "mmpf_system.h"

/// @deprecated Customer and project ID
MMP_CUSTOMER           gbCustomer  = {0xFF, 0x01, 0x01};
/// @brief Human-maintained release version
//MMP_RELEASE_VERSION    gbFwVersion = {0x01, 0x01, 0x729};
//MMP_RELEASE_VERSION    gbFwVersion = {0x01, 0x02, 0x0825};		//AlexH @ 2020/08/25
MMP_RELEASE_VERSION    gbFwVersion = {0x02, 0x00, 0x00, 0x0828};	//For version format in specification, AlexH @ 2020/08/25

/// @USB product stream
MMP_UBYTE USB_PRODUCT_STR[14] = {'A','I','T',' ','8','4','x',' ',' ',' ',' ',' ',' ',' '};/// @DSC EXIF
 