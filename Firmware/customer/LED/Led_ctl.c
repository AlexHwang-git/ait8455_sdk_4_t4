#include "mmpf_i2cm.h"
#include "Led_ctl.h"
#include "mmpf_dram.h"
#include "mmpf_timer.h"

#if SUPPORT_LED_DRIVER

#if BIND_LED_LM36922
#define LED_I2C_ADDR_LM36922   (0x36)
#define LED_I2C_ADDR           (LED_I2C_ADDR_LM36922)
#endif

MMPF_I2CM_ATTRIBUTE m_led_I2cmAttribute = {
            SENSOR_I2CM_ID, LED_I2C_ADDR, 8, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE,
            MMP_FALSE, 0, 0, 0, MMPF_I2CM_SPEED_HW_250K, NULL, NULL};  

#if (CHIP == VSN_V2)
extern volatile MMP_UBYTE   gbDramID;
#endif

//*----------------------------------------------------------------------------
// function : MMPF_LED_WriteI2C
// input    :
// output   : none
// descript : Write LED Driver register
//*----------------------------------------------------------------------------
MMP_ERR  MMPF_LED_WriteI2C(MMP_USHORT usAddr, MMP_USHORT usData)
{
#if (CHIP == VSN_V2)	
	if((gbDramID == MMPF_DRAMID_WINBOND_SDR16) || (gbDramID == MMPF_DRAMID_WINBOND_SDR16_1_8V)){
        m_led_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_0;  // for AIT8433 70 pin package
    }
    else{
        m_led_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_1;  // for AIT8451 105 pin package
    }
#endif

#if (CHIP == VSN_V3)	
		 m_led_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_0; 
#endif

	MMPF_I2cm_WriteReg(&m_led_I2cmAttribute, usAddr, usData);
	return  MMP_ERR_NONE;
}

//*----------------------------------------------------------------------------
// function : MMPF_LED_ReadI2C
// input    :
// output   : none
// descript : Read LED Driver register
//*----------------------------------------------------------------------------
MMP_ERR  MMPF_LED_ReadI2C(MMP_USHORT usAddr, MMP_USHORT *usData)
{
	*usData = 0xFF;
#if (CHIP == VSN_V2)	
	if((gbDramID == MMPF_DRAMID_WINBOND_SDR16) || (gbDramID == MMPF_DRAMID_WINBOND_SDR16_1_8V)){
        m_led_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_0;  // for AIT8433 70 pin package
    }
    else{
        m_led_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_1;  // for AIT8451 105 pin package
    }
#endif

#if (CHIP == VSN_V3)
	 m_led_I2cmAttribute.uI2cmID = MMPF_I2CM_ID_0;
#endif

    MMPF_I2cm_ReadReg(&m_led_I2cmAttribute, usAddr, usData);
	
	return  MMP_ERR_NONE;
}


//*----------------------------------------------------------------------------
// function : LED_Driver_Init
// input    :
// output   : 
// descript : Initialize LED Driver registers
//*----------------------------------------------------------------------------
void LED_Driver_Init(void)
{
#if BIND_LED_LM36922
	LED_LM36922_Init();
#endif
}

//*----------------------------------------------------------------------------
// function : LED_Driver_Enable
// input    :
// output   : 
// descript : Enable or disable LED Driver
//*----------------------------------------------------------------------------
void LED_Driver_Enable(MMP_UBYTE enable)
{
#if BIND_LED_LM36922
    LED_LM36922_Enable(enable);
#endif
}

//*----------------------------------------------------------------------------
// function : LED_Driver_SetBrightnessCode
// input    :
// output   : 
// descript : Set LED Brightness Code
//*----------------------------------------------------------------------------
void LED_Driver_SetBrightnessCode(MMP_USHORT code)
{
#if BIND_LED_LM36922
    LED_LM36922_SetBrightnessCode(code);
#endif
}

//*----------------------------------------------------------------------------
// function : LED_Driver_GetBrightnessCode
// input    :
// output   : 
// descript : Get LED Brightness Code
//*----------------------------------------------------------------------------
MMP_USHORT LED_Driver_GetBrightnessCode(void)
{
	
#if BIND_LED_LM36922
    return LED_LM36922_GetBrightnessCode();
#endif
}

//*----------------------------------------------------------------------------
// function : LED_Driver_GetBrightnessStep
// input    :
// output   : 
// descript : Get LED Brightness Step
//*----------------------------------------------------------------------------
MMP_UBYTE LED_Driver_GetBrightnessStep(void)
{
#if BIND_LED_LM36922
    LED_LM36922_GetBrightnessStep();
#endif
}

//*----------------------------------------------------------------------------
// function : LED_Driver_SetBrightnessStep
// input    :
// output   : 
// descript : Set LED Brightness Step
//*----------------------------------------------------------------------------
MMP_UBYTE LED_Driver_SetBrightnessStep(MMP_UBYTE val)
{
#if BIND_LED_LM36922
	MMP_USHORT code;
	if(val == 0)
	  	code = 0;
	 else
              code = ((val -1)*LM36922_BRIGHT_PERSTEP) + LM36922_Min_BRIGHT;
	 LED_LM36922_SetBrightnessCode(code);
       dbg_printf(3, "Current step: %d (%d)\r\n",val,code);
					   
#endif
}

void Set_Rotational_Encode_timer(void)
{
	Rotationa_Encode_timer_open(100,MMPF_TIMER_1);
}
#endif
