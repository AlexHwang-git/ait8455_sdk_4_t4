/** @file ait_bsp.h

All BSP dependent configuration in this file.

@brief BSP dependent configuration compiler options
@author Philip Lin
@since 10-Jul-08
@version
- 1.0 Original version
*/
#ifndef AIT_BSP_H
#define AIT_BSP_H

#include "mmp_lib.h"

/** @name System
@{ */

    /** @brief The starting free address in the host

    It's usually used in loading files to the host memory and transfer to the MMP device.
    */
	#define HOST_RAM_ADDRESS 0x3000000

    /*-----------macro declaration-------------------------------------*/
    /** @brief Define the address of the CS ping
    */
    #define RTNA_CS_BASE 0x02000000
    /** @brief Define the address of the A0 ping
    */
    #define RTNA_A0_SEL 0x00080000

    /** @brief Host CPU clock in mHz. The demo kit uses 66mHz ARM7.
        The customer should modify this value.*/
    #define HOST_CLK_M  144

    /** @brief The clock cycles for waiting in the ARM7.
        It's used in busy waiting to wait a certain period of time.*/
    #define HOST_WHILE_CYCLE   4  //1000

	/** @brief The chip ID which the current software supports
	*/
   // #define	P_V2		(1)
   // #define VSN_V2      (2)
   // #ifndef CHIP
   // #define	CHIP		(VSN_V2)
   // #endif

	#define	VER_A	(0)
	#define	VER_B	(1)
	#define	VER_C	(2)
	#define CHIP_VERSION (VER_A)	


	/** @} */ // end of system




#endif /* AIT_BSP_H */
