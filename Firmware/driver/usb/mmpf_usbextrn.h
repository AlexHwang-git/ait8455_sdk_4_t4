//************************************************************************
// usbint.c
//************************************************************************
#ifdef  exUSBINT

/** @addtogroup MMPF_USB
@{
*/

//MMP_ULONG   dwTotalSec;
//MMP_ULONG   USBDescBufAddr;
//MMP_ULONG   USBEP0TxBufAddr;
//MMP_ULONG   CBWRxBufAddr;
//MMP_ULONG   CSWTxBufAddr;

MMP_USHORT  USBEP0TxCnt;
//MMP_ULONG   USBEP0TxAddr;
//MMP_ULONG   USBEP0RxAddr;

MMP_USHORT  USBConfigDescLen;
MMP_UBYTE   USBInPhase;
MMP_UBYTE   USBOutPhase;
//MMP_UBYTE   USBDevAddr = 0;
//MMP_UBYTE   USBRequestType;
//MMP_UBYTE   USBVendCmd;
//MMP_UBYTE   USBConfig;
//MMP_UBYTE   USBInterface;
//MMP_UBYTE   USBEndpoint;
//MMP_UBYTE   USBAlterSetting;

//MMP_USHORT  USBFrameNum;
//MMP_UBYTE   USBDevice;

MMP_ULONG   USBBulkTxAddr;

MMP_UBYTE   USBCmd;
MMP_USHORT  USBConfig1Len;
MMP_UBYTE   USBConfig2Len;


void usb_ep0_setup_done(void);
void usb_ep0_in_done(void);
void usb_ep0_out_done(void);
void usb_ep1_done(void);
void usb_ep2_done(void);
void usb_ep3_done(void);
void usb_plug_det(void);
void usb_reset_event(void);
void usb_sof_event(void);
void usb_error(void);
void usb_ep0_error(void);
void usb_ep1_error(void);
void usb_ep2_error(void);
void usb_ep3_error(void);
void usb_ep4_error(void);
void clear_ep1_sequence(void);
void clear_ep2_sequence(void);
void clear_ep3_sequence(void);
void usb_ep2_ack(void);
void usb_ep2_nak(void);
void usb_bulk_tx_start(void);
void usb_bulk_txrx_start(void);
void class_request_setup(void);
void class_request_in(void);
void class_request_out(void);
void class_reset(void);
void class_in_reset(void);
void class_out_reset(void);
void class_get_max_lun(void);
void class_in_get_max_lun(void);
void class_out_get_max_lun(void);
void usb_ep0_null_out(void);
void usb_ep0_null_in(void);
void usb_ep0_xbyte_in(void);
void usb_ep0_in(void);
void usb_reset_module3(void);
#else
extern  MMP_USHORT  USBEP0TxCnt;
extern  MMP_USHORT  USBConfigDescLen;
extern  MMP_UBYTE   USBInPhase;
extern  MMP_UBYTE   USBOutPhase;
//extern  MMP_UBYTE   USBDevAddr;
//extern  MMP_UBYTE   USBRequestType;
//extern  MMP_UBYTE   USBVendCmd;
//extern  MMP_UBYTE   USBConfig;
//extern  MMP_UBYTE   USBInterface;
//extern  MMP_UBYTE   USBEndpoint;
//extern  MMP_UBYTE   USBAlterSetting;

//extern  MMP_USHORT  USBFrameNum;
//extern  MMP_UBYTE   USBDevice;

extern  MMP_ULONG   USBBulkTxAddr;

extern  MMP_UBYTE   USBCmd;
extern  MMP_USHORT  USBConfig1Len;
extern  MMP_UBYTE   USBConfig2Len;

extern  void usb_ep0_setup_get_req(void);
extern  void usb_isr(void);
extern  void usb_ep0_setup_done(void);
extern  void usb_ep0_in_done(void);
extern  void usb_ep0_out_done(void);
extern  void usb_ep1_done(void);
extern  void usb_ep2_done(void);
extern  void usb_ep3_done(void);
extern  void usb_plug_det(void);
extern  void usb_reset_event(void);
extern  void usb_sof_event(void);
extern  void usb_error(void);
extern  void usb_ep0_error(void);
extern  void usb_ep1_error(void);
extern  void usb_ep2_error(void);
extern  void usb_ep3_error(void);
extern  void usb_ep4_error(void);
extern  void clear_ep1_sequence(void);
extern  void clear_ep2_sequence(void);
extern  void clear_ep3_sequence(void);
extern  void clear_ep4_sequence(void);
extern  void clear_ep_sequence(MMP_UBYTE ep);
extern  void usb_ep2_ack(void);
extern  void usb_ep2_nak(void);
extern  void usb_bulk_tx_start(void);
extern  void usb_bulk_txrx_start(void);
extern  void class_request_setup(void);
extern  void class_request_in(void);
extern  void class_request_out(void);
extern  void class_reset(void);
extern  void class_in_reset(void);
extern  void class_out_reset(void);
extern  void class_get_max_lun(void);
extern  void class_in_get_max_lun(void);
extern  void class_out_get_max_lun(void);
extern  void usb_ep0_null_out(void);
extern  void usb_ep0_null_in(void);
extern  void usb_ep0_xbyte_in(void);
extern  void usb_ep0_in(void);
#endif



//************************************************************************
// usbstd.c
//************************************************************************
#ifdef  exUSBSTD
void std_request_setup(void);
void std_request_in(void);
void std_request_out(void);
void send_getstatus_data(void);
void std_setup_getstatus(void);
void std_in_getstatus(void);
void std_out_getstatus(void);
void send_clrfeature_data(void);
void std_setup_clearfeature(void);
void std_in_clearfeature(void);
void std_out_clearfeature(void);
void send_setfeature_data(void);
void std_setup_setfeature(void);
void std_in_setfeature(void);
void std_out_setfeature(void);
void std_setup_setaddress(void);
void std_in_setaddress(void);
void std_out_setaddress(void);
void std_setup_setdescriptor(void);
void std_in_setdescriptor(void);
void std_out_setdescriptor(void);
void std_setup_getconfiguration(void);
void std_in_getconfiguration(void);
void std_out_getconfiguration(void);
void std_setup_setconfiguration(void);
void std_in_setconfiguration(void);
void std_out_setconfiguration(void);
void std_setup_getinterface(void);
void std_in_getinterface(void);
void std_out_getinterface(void);
void std_setup_setinterface(void);
void std_in_setinterface(void);
void std_out_setinterface(void);
void std_setup_synchframe(void);
void std_in_syncframe(void);
void std_out_syncframe(void);
void std_setup_getdevicedescriptor(void);
void std_in_getdevicedescriptor(void);
void std_out_getdevicedescriptor(void);
void std_setup_getstringdescriptor(void);
void std_in_getstringdescriptor(void);
void std_out_getstringdescriptor(void);
void std_setup_getconfigurationdescriptor(void);
void std_in_getconfigurationdescriptor(void);
void std_out_getconfigurationdescriptor(void);
MMP_UBYTE *std_get_configuration_desc(MMP_BOOL highspeed,MMP_USHORT *desclen);
#else
extern  void std_request_setup(void);
extern  void std_request_in(void);
extern  void std_request_out(void);
extern  void send_getstatus_data(void);
extern  void std_setup_getstatus(void);
extern  void std_in_getstatus(void);
extern  void std_out_getstatus(void);
extern  void send_clrfeature_data(void);
extern  void std_setup_clearfeature(void);
extern  void std_in_clearfeature(void);
extern  void std_out_clearfeature(void);
extern  void send_setfeature_data(void);
extern  void std_setup_setfeature(void);
extern  void std_in_setfeature(void);
extern  void std_out_setfeature(void);
extern  void std_setup_setaddress(void);
extern  void std_in_setaddress(void);
extern  void std_out_setaddress(void);
extern  void std_setup_setdescriptor(void);
extern  void std_in_setdescriptor(void);
extern  void std_out_setdescriptor(void);
extern  void std_setup_getconfiguration(void);
extern  void std_in_getconfiguration(void);
extern  void std_out_getconfiguration(void);
extern  void std_setup_setconfiguration(void);
extern  void std_in_setconfiguration(void);
extern  void std_out_setconfiguration(void);
extern  void std_setup_getinterface(void);
extern  void std_in_getinterface(void);
extern  void std_out_getinterface(void);
extern  void std_setup_setinterface(void);
extern  void std_in_setinterface(void);
extern  void std_out_setinterface(void);
extern  void std_setup_synchframe(void);
extern  void std_in_syncframe(void);
extern  void std_out_syncframe(void);
extern  void std_setup_getdevicedescriptor(void);
extern  void std_in_getdevicedescriptor(void);
extern  void std_out_getdevicedescriptor(void);
extern  void std_setup_getstringdescriptor(void);
extern  void std_in_getstringdescriptor(void);
extern  void std_out_getstringdescriptor(void);
extern  void std_setup_getconfigurationdescriptor(void);
extern  void std_in_getconfigurationdescriptor(void);
extern  void std_out_getconfigurationdescriptor(void);
extern  MMP_UBYTE *std_get_configuration_len(MMP_BOOL highspeed,MMP_USHORT *desclen);

#endif

//************************************************************************
// usbvend.c
//************************************************************************
#ifdef  exUSBVEND
void vendor_request_setup(void);
void vendor_request_in(void);
void vendor_request_out(void);
void VendorOutProcess(void);
void usb_check(void);
void VendorSetupProcess(void);
void VendorInProcess(void);
void SetBuffAddr(void);
void SetRxBuffAddr(void);
void usb_get_buff(void);
void usb_receive_buff(void);
void vendor_set_buff_access_addr(void);
void vendor_get_buff_access_size(void);
void vendor_set_resolution(void);
void vendor_set_device_mode(void);
void vendor_update_flash_ATF_area(void);
#else
extern  void vendor_request_setup(void);
extern  void vendor_request_in(void);
extern  void vendor_request_out(void);
extern  void VendorOutProcess(void);
extern  void usb_check(void);
extern  void VendorSetupProcess(void);
extern  void VendorInProcess(void);
extern  void SetBuffAddr(void);
extern  void SetRxBuffAddr(void);
extern  void usb_get_buff(void);
extern  void usb_receive_buff(void);
extern  void vendor_set_buff_access_addr(void);
extern  void vendor_get_buff_access_size(void);
extern  void vendor_set_resolution(void);
extern  void vendor_set_device_mode(void);
extern  void vendor_update_flash_ATF_area(void);
#endif


#ifdef  exUSBDESCR

#else
#if (SUPPORT_UVC_FUNC==1)
    extern  MMP_UBYTE DEVICE_QUALIFIER_DESCRIPTOR_DATA0[0x0A];
    extern  MMP_UBYTE DEVICE_QUALIFIER_DESCRIPTOR_DATA1[0x0A];
    extern  MMP_UBYTE   HS_CONFIG_DESCRIPTOR_DATA1[];
    extern  MMP_UBYTE   DEVICE_DESCRIPTOR_DATA0[0x12];
    extern MMP_UBYTE    CONFIG_DESCRIPTOR_DATA1[];

    extern  MMP_UBYTE   HIDREPORT_DESCRIPTOR[0x1a];
    extern  MMP_UBYTE   DEVICE_DESCRIPTOR_DATA1[0x12];
    extern  MMP_UBYTE   DEVICE_QUALIFIER_DESCRIPTOR_DATA2[0x0A];
    extern  MMP_UBYTE   HS_CONFIG_DESCRIPTOR_DATA2[0x20];
    extern  MMP_UBYTE   DEVICE_DESCRIPTOR_DATA2[0x12];
    extern  MMP_UBYTE   CONFIG_DESCRIPTOR_DATA2[0x20];
    extern  MMP_UBYTE   DEVICE_QUALIFIER_DESCRIPTOR_DATA3[0x0A];
    extern  MMP_UBYTE   HS_CONFIG_DESCRIPTOR_DATA3[0x20];
    extern  MMP_UBYTE   DEVICE_DESCRIPTOR_DATA3[0x12];
    extern  MMP_UBYTE   CONFIG_DESCRIPTOR_DATA3[0x20];

    extern  MMP_UBYTE   CONFIG_DESCRIPTOR_DATA5[0x43];
    extern  MMP_UBYTE   DEVICE_DESCRIPTOR_DATA5[0x12];
    extern  MMP_UBYTE   HS_CONFIG_DESCRIPTOR_DATA5[0x43];
    extern  MMP_UBYTE   DEVICE_QUALIFIER_DESCRIPTOR_DATA5[0x0A];

    extern  MMP_UBYTE   LANGUAGE_ID_DATA[0x04];
    extern  MMP_UBYTE   MANUFACTURER_STRING_DATA[];
    extern  MMP_UBYTE   PRODUCT_STRING_DATA[];
    extern  MMP_UBYTE   SERIALNUMBER_STRING_DATA[];

#else

    extern  MMP_UBYTE   DEVICE_DESCRIPTOR_DATA1[0x12];
    extern  MMP_UBYTE   CONFIG_DESCRIPTOR_DATA1[0x20];
    extern  MMP_UBYTE   DEVICE_DESCRIPTOR_DATA2[0x12];
    extern  MMP_UBYTE   CONFIG_DESCRIPTOR_DATA2[0x20];
    extern  MMP_UBYTE  DEVICE_DESCRIPTOR_DATA3[0x12];
    extern  MMP_UBYTE  CONFIG_DESCRIPTOR_DATA3[0x20];
    extern  MMP_UBYTE   LANGUAGE_ID_DATA[0x04];
    extern  MMP_UBYTE   MANUFACTURER_STRING_DATA[0x1E];
    extern  MMP_UBYTE   PRODUCT_STRING_DATA[0x1E];
    
#endif
#endif


void EnableEx(MMP_UBYTE endpoint);

#ifdef  exUSBPUB

void A810L_InitUSB(void);

void USBDetectEvent(void);
void usb_suspend_event(void);
void usb_snap_cmd(void);
void vendor_set_reg(void);
void vendor_get_reg(void);
void usb_desc_init(void);
void usb_sw_enum(void);
void usb_ep_init(void);

#else

extern  void MMPF_InitUSB(void);

extern  void USBDetectEvent(void);
extern  void usb_suspend_event(void);
extern  void usb_snap_cmd(void);
extern  void vendor_set_reg(void);
extern  void vendor_get_reg(void);
extern  void usb_desc_init(void);
extern  void usb_sw_enum(void);

#endif

/// @}
