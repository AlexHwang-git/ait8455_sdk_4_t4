//==============================================================================
//
//  File        : reg_pcm.h
//  Description : Retina jpeg control block regsiter definition header file
//  Author      : Hans Liu
//  Revision    : 1.0
//
//==============================================================================


#ifndef _REG_PCM_H_
#define _REG_PCM_H_


#include	"reg_retina.h"


//==============================================================================
//
//                              Retina Control Register Definitions
//
//==============================================================================
//-----------------------------
// PCM0 structure (0x8000 6C00)
// PCM1 structure (0x8000 6D00)
//-----------------------------

typedef struct _AITS_PCM{
   AIT_REG_B   PCM_CTL;
   AIT_REG_B   PCM_FORMAT;
   AIT_REG_B   PCM_AUDIO_GAIN;
   AIT_REG_B   PCM_SETTING;
   
   AIT_REG_W   PCM_CLK_DIVISOR;
   AIT_REG_W   PCM_SYNC_DIVISOR;
   
   AIT_REG_B   PCM_CLK_ACTIVE_CYCLE;
   AIT_REG_B   PCM_CLK_REACTIVE_CYCLE;
   AIT_REG_B   PCM_SOFTWARE_RESET;
   
   AIT_REG_B                    _xB[5];
   
   AIT_REG_W   PCM_HOST_INT_ENA;
   AIT_REG_W                    _x12[1];
   AIT_REG_W   PCM_HOST_INT_STS;
   AIT_REG_W                    _x16[1];
   AIT_REG_W   PCM_CPU_INT_ENA;
   AIT_REG_W                    _x1A[1];
   AIT_REG_W   PCM_CPU_INT_STS;
   AIT_REG_W                    _x1E[1];
   AIT_REG_W   PCM_TX_FIFO_PORT;
   AIT_REG_W                    _x22[1];
   AIT_REG_W   PCM_RX_FIFO_PORT;
   
   AIT_REG_B                    _x26[2];
   
   AIT_REG_B   PCM_TX_FIFO_THRESHOLD;
   AIT_REG_B   PCM_TX_FIFO_LEFT;
   AIT_REG_B   PCM_RX_FIFO_THRESHOLD;
   AIT_REG_B   PCM_RX_FIFO_LEFT;   
} AITS_PCM, *AITPS_PCM;




#endif	// _REG_PCM_H_
