#ifndef AIT_AGC_H
#define AIT_AGC_H

#define		SHORT_MAX		32767

typedef struct AIT_AGC_struct
{
	unsigned int sample_max;
	unsigned int sample_max_L_1;
	unsigned int attack_flag;
	unsigned int decay_flag;
	unsigned int noise_floor_decay;
	unsigned int noise_floor_attack;
	int		target;
	long	igain;
	int		counter;
	int		sample_rate_idx;
	int		attack_time;
	int		decay_time;
	int		hold_time;
	long	igain_attack;
	long	igain_decay;
	long	gain_old;
	int		hold_time_count;
	int		silence_count;
	int		decay_limit;

} AIT_AGC_struct;

typedef struct AIT_AGC_struct	AIT_AGC_st;

void AIT_AGC_Initial(AIT_AGC_st *agc, float level);
void AIT_AGC_Codec(AIT_AGC_st *agc, short *buffer, int len, unsigned int FrameCount);
void AIT_AGC_Delete(AIT_AGC_st *agc);

/*
-------------------
sample rate index :
48 -> 48000
44 -> 44100
32 -> 32000
24 -> 24000
22 -> 22050
16 -> 16000
12 -> 12000
11 -> 11025
 8 ->  8000
-------------------


*/

#endif	//AIT_AGC_H