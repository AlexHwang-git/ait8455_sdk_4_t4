#include "mmpf_i2cm.h"
#include "motor_ctl.h"
#include "hdm_ctl.h"
#include "mmpf_dram.h"

#if SUPPORT_AUTO_FOCUS

#define MOTOR_I2C_ADDR_DW9714   0x0C
//#define MOTOR_I2C_ADDR_DRV201   0x0E

#if (BIND_SENSOR_OV4688)||(BIND_SENSOR_OV4689)
#define MOTOR_I2C_ADDR          MOTOR_I2C_ADDR_DW9714
#endif

//#if (BIND_SENSOR_OV5647)
//#undef MOTOR_I2C_ADDR
//#define MOTOR_I2C_ADDR          MOTOR_I2C_ADDR_DRV201
//#endif

MMPF_I2CM_ATTRIBUTE m_motor_I2cmAttribute = {
SENSOR_I2CM_ID, MOTOR_I2C_ADDR, 8, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE,
MMP_FALSE, 0, 0, 1, MMPF_I2CM_SPEED_HW_250K, NULL, NULL
};

MMP_ERR  MMPF_MOTOR_WriteI2C(MMP_USHORT usAddr, MMP_USHORT usData)
{
    m_motor_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_0;

	MMPF_I2cm_WriteReg(&m_motor_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}

MMP_ERR  MMPF_MOTOR_ReadI2C(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	*usData = 0xFF;

    m_motor_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_0;

    MMPF_I2cm_ReadReg(&m_motor_I2cmAttribute, usAddr, usData);
	
	return  MMP_ERR_NONE;
}

#endif
