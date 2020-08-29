//==============================================================================
//
//  File        : sensor_task.c32
//  Description : JPEG DSC function
//  Author      : Jerry Tsao
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#if (SENSOR_EN)
#include "reg_retina.h"
#include "lib_retina.h"


//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
/** @addtogroup MMPF_Sensor
@{
*/

OS_STK                  SENSOR_Task_Stk[TASK_SENSOR_STK_SIZE];



//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

extern void             SENSOR_Task(void);



//------------------------------------------------------------------------------
//  Function    : MMPF_SENSOR_TaskHandler
//  Description : Task entry function for SENSOR
//------------------------------------------------------------------------------

void MMPF_SENSOR_TaskHandler(void *p_arg)
{
    SENSOR_Task();
}
#endif

/// @}