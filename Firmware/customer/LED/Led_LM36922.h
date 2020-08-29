
//#include "config_fw.h"

#if SUPPORT_LED_DRIVER

#if BIND_LED_LM36922  

#if SUPPORT_PWM_LED

#define   LM36922_Divider		(20000) // 6KHz
#define   LM36922_MAX_BRIGHT       (15700)//(17000) 
#define   LM36922_Min_BRIGHT       (3000) 	// step 1 

#define   LM36922_BRIGHT_STEP      (12)   //  internal step 1 ~ 36 steps
//#define	Ext_STEP	(12)				// customer define step
#define	Ext_CHA_INT_DIV	(2)

#else
#define LM36922_MAX_BRIGHT       (2047) // 2047 = 0x7ff (11-bit)

#define LM36922_BRIGHT_STEP      (12)   // 1 ~ 12 steps
#endif
#define LM36922_BRIGHT_PERSTEP   (LM36922_MAX_BRIGHT - LM36922_Min_BRIGHT)/(LM36922_BRIGHT_STEP - 1)  //step 1 is LM36922_Min_BRIGHT

#define LM36922_DEF_STEP         (0) // (1)

#define LM36922_DEF_BRIGHT       (LM36922_DEF_STEP*LM36922_BRIGHT_PERSTEP)


/*******************************************************************************/
/*               LED Driver LM36922 Register Map                               */
/*******************************************************************************/
#define LM36922_SW_RST                      (0x01)
    #define NORMAL_OPERATION                 (0x00)
    #define DEVICE_RESET                     (0x01)

#define LM36922_ENABLE                      (0x10)
    #define DEVICE_ENABLE                    (0x01)
    #define LED1_ENABLE                      (0x02)
    #define LED2_ENABLE                      (0x04)

#define LM36922_BRIGHTNESS_CTL              (0x11)
    // BL_ADJ Polarity
    #define BL_ADJ_POLARITY_HIGH              (0x01)
    // Ramp Rate
    #define RAMP_RATE_D250MS                  (0x02)
    #define RAMP_RATE_D500MS                  (0x04)
    #define RAMP_RATE_1MS                     (0x06)
    #define RAMP_RATE_2MS                     (0x08)
    #define RAMP_RATE_4MS                     (0x0A)
    #define RAMP_RATE_8MS                     (0x0C)
    #define RAMP_RATE_16MS                    (0x0E)
    // Ramp Enable
    #define RAMP_ENABLE                       (0x10)
    // Brightness Mode
    #define BRIGHTNESS_MODE_PWM_ONLY          (0x20)
    #define BRIGHTNESS_MODE_MULTI_RAMP        (0x40)
    #define BRIGHTNESS_MODE_RAMP_MULTI        (0x60)
    // Mapping Mode
    #define MAPPING_MODE_EXP                  (0x80)

#define LM36922_PWM_CTL                     (0x12)

#define LM36922_BOOST_CTL                   (0x13)

#define LM36922_AUTO_FREQ_HIGH_THRESHOLD    (0x15)

#define LM36922_AUTO_FREQ_LOW_THRESHOLD     (0x16)

#define LM36922_BACKLIGHT_ADJ_THRESHOLD     (0x17)

#define LM36922_BRIGHTNESS_REG_LSB          (0x18)

#define LM36922_BRIGHTNESS_REG_MSB          (0x19)

#define LM36922_FAULT_CTL                   (0x1E)

#define LM36922_FAULT_FLAGS                 (0x1F)

#endif

#endif

