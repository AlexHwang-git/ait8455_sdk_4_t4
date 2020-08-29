#ifndef _LED_CTL_H_
#define _LED_CTL_H_

#if BIND_LED_LM36922
#include "Led_LM36922.h"
#endif

/*******************************************************************/
/*                      LED Driver                                 */
/*******************************************************************/
MMP_ERR  MMPF_LED_WriteI2C(MMP_USHORT usAddr, MMP_USHORT usData);
MMP_ERR  MMPF_LED_ReadI2C(MMP_USHORT usAddr, MMP_USHORT *usData);

void LED_Driver_Init(void);
void LED_Driver_Enable(MMP_UBYTE enable);
void LED_Driver_SetBrightnessCode(MMP_USHORT code);
MMP_USHORT LED_Driver_GetBrightnessCode(void);
MMP_UBYTE LED_Driver_GetBrightnessStep(void);
MMP_UBYTE LED_Driver_SetBrightnessStep(MMP_UBYTE val);
void Set_Rotational_Encode_timer(void);

#endif // _LED_CTL_H_
