#include "includes_fw.h"
#include "Led_LM36922.h"

#include "mmpf_pwm.h"
#include "mmp_reg_gpio.h"
#include "mmpf_timer.h"
#include "mmpf_pio.h"
#include "pCam_msg.h"
#if SUPPORT_LED_DRIVER

#if BIND_LED_LM36922  
void LED_LM36922_Init(void);
void LED_LM36922_Enable(MMP_UBYTE enable);
void LED_LM36922_SetBrightnessCode(MMP_USHORT code);
MMP_USHORT LED_LM36922_GetBrightnessCode(void);
MMP_UBYTE LED_LM36922_GetBrightnessStep(void);
void Rotationa_Encode_timer_open(MMP_ULONG us,MMPF_TIMER_ID id);
void Rotationa_Encode_timer_close(MMPF_TIMER_ID id);
void Rotationa_Encode_timer_isr(void);
#if SUPPORT_PWM_LED
#define LM36922_USED_PWM	MMPF_PWM_ID_2	// PWM2 => PBGPIO13
MMPF_PWM_ATTRIBUTE pulseB = {LM36922_USED_PWM, MMPF_PWM_PULSE_ID_A, 0, 0, 0, 0, LM36922_Divider, 0x1};
#endif


void Rotationa_Encode_timer_open(MMP_ULONG us,MMPF_TIMER_ID id)
{
#if (SUPPORT_TIMER_CTL_ENCODER == 1)
    MMPF_Timer_OpenUs(id, us , Rotationa_Encode_timer_isr );
    MMPF_Timer_OpenInterrupt( id );
    MMPF_Timer_EnableInterrupt( id , MMP_TRUE);
#endif

}

void Rotationa_Encode_timer_close(MMPF_TIMER_ID id)
{
    MMPF_Timer_EnableInterrupt(id, MMP_FALSE);
    MMPF_Timer_Close(id);
}
MMP_UBYTE 	gRotation_debounce_val = 0; 
MMP_UBYTE 	gRotation_old_val = 0; 
MMP_UBYTE	gRotation_status = 0;		// 0: non-change   1: frist bit change( check C.W or C.C.W)   2: second bit change (don't care ) 
MMP_UBYTE	gRotation_cnt = 0;
MMP_UBYTE	Div_cnt = 0;
extern  MMP_ULONG   glSystemEvent;
void Rotationa_Encode_timer_isr(void)
{
	MMP_UBYTE bit0_val,bit1_val;
	MMPF_PIO_GetData(LIGHT_POSITIVE, &bit0_val);
	MMPF_PIO_GetData(LIGHT_NEGATIVE, &bit1_val);
	bit0_val &= 0x01;
	bit1_val &= 0x01;
	bit0_val |= bit1_val << 1;
	/*
	if( bit0_val != gRotation_old_val)
	{
		dbg_printf(0,"%d %d\r\n",bit0_val,gRotation_old_val);
		gRotation_old_val = bit0_val;
	}*/
	if( glSystemEvent != 0)	return;

	if( gRotation_debounce_val != bit0_val)
	{
		gRotation_debounce_val = bit0_val;
		gRotation_cnt = 0;
		return;
	}

	if(  gRotation_cnt <= 1 )		//debounce
	{
		gRotation_cnt ++;
		return;
	}
#if 0
	if( gRotation_cnt >= 2 )
	{	
		if( (gRotation_debounce_val == 0) || (gRotation_debounce_val == 3) )
		{
			gRotation_status = 1;
			gRotation_cnt = 0;
			gRotation_old_val = gRotation_debounce_val;
		}
		else if( gRotation_status == 1)
			gRotation_status = 2;		
	}

	if( gRotation_status == 2)			//check compare start
#else
		if( gRotation_old_val == gRotation_debounce_val)
			return;
		//gRotation_old_val = gRotation_debounce_val;
#endif
	{
		dbg_printf(0,"%d %d\r\n",gRotation_debounce_val,gRotation_old_val);
		if( (gRotation_debounce_val^gRotation_old_val) == 2 )
		{
			if(gRotation_status == 0)		gRotation_status = 1;
			else		gRotation_status = 2;
			if( gRotation_status == 1)
			{
				dbg_printf(0,"C.W.\r\n");
				if(Div_cnt < Ext_CHA_INT_DIV)
				{
					Div_cnt++;
					return;
				}
				Div_cnt = 0;
	         		glSystemEvent |= ROTARY_SWITCH_CW;
	     		       if(glSystemEvent != 0)
					MMPF_PCAM_PostMsg(0,TASK_PCAM_PRIO,PCAM_MSG_OTHERS,0,(void *)0);
			}
		}	
		else if ( (gRotation_debounce_val^gRotation_old_val) == 1 )
		{
			if(gRotation_status == 0)		gRotation_status = 1;
			else		gRotation_status = 2;
			if( gRotation_status == 1)
			{
		            dbg_printf(0,"C.C.W.\r\n");
				if(Div_cnt < Ext_CHA_INT_DIV)
				{
					Div_cnt++;
					return;
				}
				Div_cnt = 0;
		            glSystemEvent |= ROTARY_SWITCH_CCW;
		            if(glSystemEvent != 0)
					    MMPF_PCAM_PostMsg(0,TASK_PCAM_PRIO,PCAM_MSG_OTHERS,0,(void *)0);		
			}
		}
			if( gRotation_status == 2)
				gRotation_status = 0;
			gRotation_cnt = 0;		
			gRotation_old_val = gRotation_debounce_val;
	}


	#if 0
        glSystemEvent &= ~(ROTARY_SWITCH_CW|ROTARY_SWITCH_CCW);
        //dbg_printf(0,"%d\r\n",glSystemEvent);

        /*********************************************************************
         * Refer to datasheet config.1
         * (1) ClockWise Sequence   : A High 2 Low then B Low
         *     (1-1) A(Light+) High 2 Low is trigger on MMPC_GPIO_InitConfig
         *     (1-2) Get B(Light-) PIO status         
         *
         * (2) Counter C.W. Sequence: B High 2 Low then A Low       
         *********************************************************************/

        // Get B (Light-)
        // RTNA_WAIT_MS(4);
        MMPF_PIO_GetData(LIGHT_NEGATIVE, &bit_val);

        // then B low
        if(bit_val == 0)
        {
            dbg_printf(0,"C.W.\r\n");
            glSystemEvent |= ROTARY_SWITCH_CW;

            if(glSystemEvent != 0)
			    MMPF_PCAM_PostMsg(0,TASK_PCAM_PRIO,PCAM_MSG_OTHERS,0,(void *)0);
        }
        else
        {
            dbg_printf(0,"C.C.W.\r\n");
            glSystemEvent |= ROTARY_SWITCH_CCW;

            if(glSystemEvent != 0)
			    MMPF_PCAM_PostMsg(0,TASK_PCAM_PRIO,PCAM_MSG_OTHERS,0,(void *)0);
        }
		#endif

}

//*----------------------------------------------------------------------------
// function : LED_LM36922_Init
// input    :
// output   : none
// descript : Initialize LED Driver LM36922
//*----------------------------------------------------------------------------
void LED_LM36922_Init(void)
{
	#if SUPPORT_PWM_LED
	MMPF_PWM_Initialize();	
	MMPF_PWM_SetAttribe(&pulseB);	
	MMPF_PWM_ControlSet(LM36922_USED_PWM, (PWM_PULSE_A_FIRST|PWM_AUTO_CYC|PWM_PULSE_A_NEG|PWM_EN));	
	
	#else
	
	MMP_USHORT read_value;

    RTNA_DBG_Str(3, "\r\n[LED_LM36922_Init]\r\n");

    MMPF_LED_WriteI2C(LM36922_ENABLE,LED2_ENABLE|LED1_ENABLE/*|DEVICE_ENABLE*/);
    MMPF_LED_ReadI2C(LM36922_ENABLE, &read_value);
    RTNA_DBG_Str(3, "LM36922 Reg[0x10] = ");
    RTNA_DBG_Short(3, read_value);
    RTNA_DBG_Str(3, "\r\n");

    MMPF_LED_WriteI2C(LM36922_BRIGHTNESS_CTL,0x00|BL_ADJ_POLARITY_HIGH);
    MMPF_LED_ReadI2C(LM36922_BRIGHTNESS_CTL, &read_value);
    RTNA_DBG_Str(3, "LM36922 Reg[0x11] = ");
    RTNA_DBG_Short(3, read_value);
    RTNA_DBG_Str(3, "\r\n");

    MMPF_LED_WriteI2C(LM36922_BRIGHTNESS_REG_LSB,0x07); // R0x18
    MMPF_LED_ReadI2C(LM36922_BRIGHTNESS_REG_LSB, &read_value);
    RTNA_DBG_Str(3, "LM36922 Reg[0x18] = ");
    RTNA_DBG_Short(3, read_value);
    RTNA_DBG_Str(3, "\r\n");

    MMPF_LED_WriteI2C(LM36922_BRIGHTNESS_REG_MSB,0xff); // R0x19
    MMPF_LED_ReadI2C(LM36922_BRIGHTNESS_REG_MSB, &read_value);
    RTNA_DBG_Str(3, "LM36922 Reg[0x19] = ");
    RTNA_DBG_Short(3, read_value);
    RTNA_DBG_Str(3, "\r\n");

//    MMPF_LED_WriteI2C(LM36922_PWM_CTL,0x73);
//    MMPF_LED_ReadI2C(LM36922_PWM_CTL, &read_value);
//    RTNA_DBG_Str(3, "LM36922 Reg[0x12] = ");
//    RTNA_DBG_Short(3, read_value);
//    RTNA_DBG_Str(3, "\r\n");

//    MMPF_LED_WriteI2C(LM36922_BOOST_CTL,0x6F);
//    MMPF_LED_ReadI2C(LM36922_BOOST_CTL, &read_value);
//    RTNA_DBG_Str(3, "LM36922 Reg[0x13] = ");
//    RTNA_DBG_Short(3, read_value);
//    RTNA_DBG_Str(3, "\r\n");

    MMPF_LED_WriteI2C(LM36922_AUTO_FREQ_HIGH_THRESHOLD,0x00);
    MMPF_LED_ReadI2C(LM36922_AUTO_FREQ_HIGH_THRESHOLD, &read_value);
    RTNA_DBG_Str(3, "LM36922 Reg[0x15] = ");
    RTNA_DBG_Short(3, read_value);
    RTNA_DBG_Str(3, "\r\n");

    MMPF_LED_WriteI2C(LM36922_AUTO_FREQ_LOW_THRESHOLD,0x00);
    MMPF_LED_ReadI2C(LM36922_AUTO_FREQ_LOW_THRESHOLD, &read_value);
    RTNA_DBG_Str(3, "LM36922 Reg[0x16] = ");
    RTNA_DBG_Short(3, read_value);
    RTNA_DBG_Str(3, "\r\n");
    RTNA_DBG_Str(3, "\r\n");

    /***************************************/
    /*           TEST ONLY                 */
    /***************************************/
    // LED_LM36922_Enable(1);
    // LED_LM36922_SetBrightnessCode(1024);
    #endif
}

//*----------------------------------------------------------------------------
// function : LED_LM36922_Enable
// input    :
// output   : none
// descript : Enable or disable LED Driver LM36922
//*----------------------------------------------------------------------------
void LED_LM36922_Enable(MMP_UBYTE enable)
{
	MMP_USHORT temp;
	#if SUPPORT_PWM_LED
	if(enable == 0)
		LED_LM36922_SetBrightnessCode(LM36922_DEF_BRIGHT);
	#else
    RTNA_DBG_Str(3, "[LED_LM36922_Enable]\r\n");

    MMPF_LED_ReadI2C(LM36922_ENABLE, &temp);
    //dbg_printf(3,"LM36922 Reg[0x10] = x%x\r\n",temp);

    // Clear Rx10 bit0: Device Enable
    temp &= ~(DEVICE_ENABLE);
    //dbg_printf(3,"temp = x%x\r\n",temp);
    //dbg_printf(3,"temp|enable = x%x\r\n",temp|enable);

    MMPF_LED_WriteI2C(LM36922_ENABLE,temp|enable);
	#endif
}

//*----------------------------------------------------------------------------
// function : LED_LM36922_SetBrightnessCode
// input    :
// output   : none
// descript : Adjust LED Driver LM36922 Brightness
// note     : 0x01 ~ 0x7f(2047) 
//          : 0x00 No light
//*----------------------------------------------------------------------------
void LED_LM36922_SetBrightnessCode(MMP_USHORT code)
{
	#if SUPPORT_PWM_LED
	MMPF_PWM_ATTRIBUTE pulseTmp = {LM36922_USED_PWM, MMPF_PWM_PULSE_ID_A, 0, 0, 0, 0, LM36922_Divider, 0x1};
	#else
	   MMP_USHORT usMSB,usLSB;
	#endif

    // Error check
    if(code > LM36922_MAX_BRIGHT)
    {
        dbg_printf(3,"!!! Error Brightness Code!!!\r\n");
        code = LM36922_MAX_BRIGHT;
    }
	#if SUPPORT_PWM_LED
	pulseTmp.usClkDuty_T3 = code;	
	MMPF_PWM_SetAttribe(&pulseTmp);
	//dbg_printf(3,"-------set BrightCode = x%x  ---------- %d\r\n",code);
	#else

    usMSB = (code & 0x7F8)>>3; // 111 1111 1xxx
    usLSB = (code & 0x007);    // xxx xxxx x111
    //dbg_printf(3,"usMSB = x%x\r\n",usMSB);
    //dbg_printf(3,"usLSB = x%x\r\n",usLSB);
    //dbg_printf(3,"code : x%03x   %d\r\n\r\n",(usMSB<<3)|usLSB,(usMSB<<3)|usLSB);

    MMPF_LED_WriteI2C(LM36922_BRIGHTNESS_REG_LSB,usLSB);
    MMPF_LED_WriteI2C(LM36922_BRIGHTNESS_REG_MSB,usMSB);
	#endif
}

//*----------------------------------------------------------------------------
// function : LED_LM36922_GetBrightnessCode
// input    :
// output   : Brightness code
// descript : Get LED Driver LM36922 Brightness
//*----------------------------------------------------------------------------
MMP_USHORT LED_LM36922_GetBrightnessCode(void)
{
    MMP_USHORT usBrightCode;
	
	#if SUPPORT_PWM_LED
	MMPF_PWM_ATTRIBUTE pulseTmp = {LM36922_USED_PWM, MMPF_PWM_PULSE_ID_A, 0, 0, 0, 0, LM36922_Divider, 0x1};
	MMPF_PWM_GetAttribe(&pulseTmp);
	usBrightCode = pulseTmp.usClkDuty_T3;
	
	#else
    MMP_USHORT usMSB,usLSB;


    MMPF_LED_ReadI2C(LM36922_BRIGHTNESS_REG_LSB,&usLSB);
    MMPF_LED_ReadI2C(LM36922_BRIGHTNESS_REG_MSB,&usMSB);
    //dbg_printf(3,"usMSB = x%x\r\n",usMSB);
    //dbg_printf(3,"usLSB = x%x\r\n",usLSB);

    usBrightCode = (usMSB<<3)|usLSB;
   
	#endif
	 //dbg_printf(3,"usBrightCode = x%x   %d\r\n",usBrightCode,usBrightCode);
    return usBrightCode;
}
//*----------------------------------------------------------------------------
// function : LED_LM36922_GetBrightnessStep
// input    :
// output   : Brightness step (User define)
// descript : Get LED Driver LM36922 Brightness step
//*----------------------------------------------------------------------------
MMP_UBYTE LED_LM36922_GetBrightnessStep(void)
{
    MMP_UBYTE usBrightStep;
	#if SUPPORT_PWM_LED
	       if( LED_LM36922_GetBrightnessCode() == 0)
		   	usBrightStep = 0;
		else
			usBrightStep = (LED_LM36922_GetBrightnessCode() - LM36922_Min_BRIGHT) / (LM36922_BRIGHT_PERSTEP) + 1;
	#else
    		usBrightStep = LED_LM36922_GetBrightnessCode() / (LM36922_BRIGHT_PERSTEP);
	#endif

    //dbg_printf(3,"-----------------usBrightStep = %d---------------------------\r\n",usBrightStep);

    return usBrightStep;
}

#endif

#endif
