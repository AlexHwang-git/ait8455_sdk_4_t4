#ifndef _MOTOR_CTL_H_
#define _MOTOR_CTL_H_

#include "config_fw.h"
//#include "mmpf_sensor.h"
//#include "mmp_reg_vif.h"
//#include "mmpf_pll.h"
#include "lib_retina.h"
#include "isp_if.h"

MMP_ERR  MMPF_MOTOR_ReadI2C(MMP_USHORT usAddr, MMP_USHORT *usData);
MMP_ERR  MMPF_MOTOR_WriteI2C(MMP_USHORT usAddr, MMP_USHORT usData);

#endif // _MOTOR_CTL_H_
