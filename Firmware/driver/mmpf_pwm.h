#ifndef _MMPF_PWM_H_
#define _MMPF_PWM_H_



//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define PWM_SEM_TIMEOUT			0x0
typedef void PwmCallBackFunc(void);
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================
typedef enum _MMPF_PWM_ID {
	 MMPF_PWM_ID_0 = 0,
	 MMPF_PWM_ID_1,
	 MMPF_PWM_ID_2,
	 MMPF_PWM_ID_3,
	 MMPF_PWM_ID_MAX
}MMPF_PWM_ID;

typedef enum _MMPF_PWM_PULSE_ID {
	 MMPF_PWM_PULSE_ID_A = 0,
	 MMPF_PWM_PULSE_ID_B,
	 MMPF_PWM_PULSE_ID_MAX
}MMPF_PWM_PULSE_ID;

typedef enum _MMPF_PWM_INT {
	 MMPF_PWM_INT_PulseA = 0x0,
	 MMPF_PWM_INT_PulseB ,
	 MMPF_PWM_INT_OneRound,
	 MMPF_PWM_INT_MAX
}MMPF_PWM_INT;

typedef struct _MMPF_PWM_ATTRIBUTE {
	MMPF_PWM_ID uID;
	MMPF_PWM_PULSE_ID uPulseID;
	MMP_USHORT	usClkDuty_T0;
	MMP_USHORT	usClkDuty_T1;
	MMP_USHORT	usClkDuty_T2;
	MMP_USHORT	usClkDuty_T3;
	MMP_USHORT	usClockDiv;
	MMP_UBYTE	ubNumOfPulses;
}MMPF_PWM_ATTRIBUTE; 
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR MMPF_PWM_Initialize(void);
MMP_ERR MMPF_PWM_SetAttribe(MMPF_PWM_ATTRIBUTE* uPwmAttribute);
MMP_ERR MMPF_PWM_GetAttribe(MMPF_PWM_ATTRIBUTE* uPwmAttribute);
MMP_ERR MMPF_PWM_EnableInterrupt(MMPF_PWM_ID uID, MMP_BOOL bEnable, PwmCallBackFunc CallBackFunc, MMPF_PWM_INT IntItem);
MMP_ERR MMPF_PWM_ControlSet(MMPF_PWM_ID uID, MMP_UBYTE control);
#endif