/**
  @file ait_bsp.c
  @brief It contains the BSP dependent configurations need to be ported to the customer platform.
  @author Philip

  @version
- 1.0 Original version
 */
#include "config_fw.h"
#include "mmp_lib.h"
#include "mmph_hif.h"
#include "ait_bsp.h"
//#include "ait_utility.h"
#if defined(BUILD_CE)
#include "ucos_ii.h"
#include "os_cpu.h"
#include "os_cfg.h"
#endif

MMP_ULONG RTNA_CPU_CLK_M = 0;
//==============================================================================
//                              FUNCTION PROTOTYPES
//==============================================================================
