#ifndef USBUAC_H
#define USBUAC_H

// for AC
#define  SET_CUR_CMD             0x01
#define  GET_CUR_CMD             0x81
#define  SET_MIN_CMD             0x02              
#define  GET_MIN_CMD             0x82              
#define  SET_MAX_CMD             0x03
#define  GET_MAX_CMD             0x83
#define  SET_RES_CMD             0x04
#define  GET_RES_CMD             0x84
#define  SET_MEM_CMD             0x05
#define  GET_MEM_CMD             0x85
#define  GET_STA_CMD             0xFF


#define  CAP_SET_CUR_CMD            (1 << 0)
#define  CAP_GET_CUR_CMD            (1 << 1)
#define  CAP_GET_MIN_CMD            (1 << 2)
#define  CAP_GET_MAX_CMD            (1 << 3)
#define  CAP_GET_RES_CMD            (1 << 4)
#define  CAP_GET_LEN_CMD            (1 << 5)
#define  CAP_GET_MEM_CMD            (1 << 5)
#define  CAP_GET_INFO_CMD           (1 << 6)
#define  CAP_GET_DEF_CMD            (1 << 7)
#define  CAP_SET_MIN_CMD            (1 << 8)
#define  CAP_SET_MAX_CMD            (1 << 9)
#define  CAP_SET_RES_CMD            (1 << 10)
#define  CAP_SET_MEM_CMD            (1 << 11)
#define  CAP_GET_STA_CMD            (1 << 11)


// audiostreaming endpoint control selectors
#define  ASE_CONTROL_UNDEFINED               0x00
#define  ASE_SAMPLING_FREQ_CONTROL           0x01
#define  ASE_PITCH_CONTROL                   0x02

// feature unit control selectors
#define  FU_CONTROL_UNDEFINED                      0x00
#define  FU_MUTE_CONTROL                           0x01
#define  FU_VOLUME_CONTROL                         0x02
#define  FU_BASS_CONTROL                           0x03
#define  FU_MID_CONTROL                            0x04
#define  FU_TREBLE_CONTROL                         0x05
#define  FU_GRAPHIC_EQUALIZER_CONTROL              0x06
#define  FU_AUTOMATIC_GAIN_CONTROL                 0x07
#define  FU_DELAY_CONTROL                          0x08
#define  FU_BASS_BOOST_CONTROL                     0x09
#define  FU_LOUDNESS_CONTROL                       0x0A

#if (MIC_SOURCE==MIC_IN_PATH_I2S)

#define FIXED_PREAMP (0x0000) //+0dB
#define VOL_MAX (0x7800/*WM8737 limit*/ + FIXED_PREAMP) // 30dB
#define VOL_MIN (0/*WM8737 limit*/ + FIXED_PREAMP) // -60dB
#define VOL_RES (128)   // 0.5 db gap.
#else    
#define VOL_MAX (40*256 ) // +30 dB
#define VOL_MIN (20*256)  // +20 dB
#define VOL_DEF (30*256)  // 25 db
#define VOL_RES (128)     // 0.5dB gap //(128*4)   // 2 db gap. ( ait analog gain limitation)
#endif

void uac_init(void);
void usb_ac_req_proc(MMP_UBYTE req);
void usb_ac_out_data(void);
void uac_process_audio_data(void);
MMP_BOOL uac_is_tx_busy(void);

#endif