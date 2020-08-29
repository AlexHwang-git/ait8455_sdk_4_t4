//#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include "config_fw.h"
#include "agc.h"

#if SUPPORT_AGC

unsigned int AIT_AGC_Version = 537924144;	/* 2010-12-30 */

extern void RTNA_DBG_Str(unsigned long level, char *str);
extern void RTNA_DBG_Long(unsigned long level, unsigned long val);

void AIT_AGC_Initial(AIT_AGC_st *agc, float level)
{
	if(agc == NULL)
		return;

	agc->sample_max = 1;
	agc->counter = 0;
	agc->igain = 65536;
	agc->target = (int)(SHORT_MAX * level * 65536);
	agc->sample_rate_idx = 32;
	agc->attack_time = 5;
	agc->decay_time = 5;
	agc->hold_time = 5;
	agc->hold_time_count = agc->hold_time * agc->sample_rate_idx;
	agc->igain_attack = 65536;
	agc->igain_decay = 65536;
	agc->gain_old = 0;
	//agc->silence_count = 0;
	//agc->decay_limit = agc->target >> 12;
	agc->decay_limit = 1 ;
	agc->sample_max_L_1 = 0;
	agc->attack_flag = 0;
	agc->decay_flag = 0;
	agc->noise_floor_decay = 2072;//321;	/* ~ -40dB */
	agc->noise_floor_attack = 3300; /* ~ -20dB */


	//RTNA_DBG_Str(0, "===== AGC Version : ");
	//RTNA_DBG_Long(0, AIT_AGC_Version);
	//RTNA_DBG_Str(0, " ===== \r\n");

	return;
}

void AIT_AGC_Codec(AIT_AGC_st *agc, short *buffer, int len, unsigned int FrameCount)
{
	int i;
	long gain_new;
	int sample;

	for(i=0; i<len; i++)
	{
// get the abs of buffer[i] 

		sample = buffer[i];
		sample = (sample<0 ? -(sample) : sample);
/* update the max */
		if(sample > (int)agc->sample_max)
		{
			agc->sample_max = (unsigned int)sample;
		}
		
		if( (i<4) && (agc->sample_max < agc->sample_max_L_1) )
			goto output;

        if(agc->sample_max == 0 )
	        agc->sample_max = agc->sample_max_L_1;
	        
		agc->counter ++;
/* Will we get an overflow with the current gain factor? */		
        if (((sample * agc->igain) >> 16) > agc->target)
	    {
			/* Yes: Calculate new gain. */
		    agc->igain = ((agc->target / agc->sample_max) * 62259 ) >> 16;
			buffer[i] = (short) ((buffer[i] * agc->igain) >> 16 ) ;
	    }

		gain_new = ((agc->target / agc->sample_max) * 62259 ) >> 16;
		if(agc->gain_old >= gain_new)
		{
			gain_new = agc->gain_old;
		}

		agc->decay_limit = (agc->target / agc->sample_max) ;
/* Calculate new gain factor 10x per second */

		if(agc->sample_max >= agc->noise_floor_attack)
		{
			agc->attack_flag = 1;
			if(agc->decay_flag == 1)
			{
				agc->decay_flag = 0;
				agc->igain = 65535;
				goto output;
			}
			agc->igain_decay = 65536;

			//gain_new =  ( gain_new * 13926 ) >> 14 ;	//for Sine Wave //13926 = 2^14 * 0.85

			if(agc->counter >= agc->attack_time)
			{
				agc->igain_attack += (gain_new - agc->igain_attack) / agc->sample_rate_idx;		
				//agc->igain_attack += ((gain_new - agc->igain_attack) * 819 ) >> 14;		
				agc->counter = 0;
				if(agc->igain_attack <= agc->igain)
					agc->igain = agc->igain_attack;
			}
		} 
		else
		{
			agc->decay_flag = 1;
			if(agc->sample_max > agc->noise_floor_decay)	/* speaking? */
			{
				if(agc->hold_time_count == 0)
				{
					agc->igain_attack = 65536;
					if(agc->counter >= agc->decay_time)
					{
						agc->igain_decay += (agc->igain_decay - gain_new) / agc->sample_rate_idx;		
						if( agc->igain_decay >= agc->decay_limit )
							agc->igain_decay = agc->decay_limit;

						agc->counter = 0;
					}
					agc->igain = agc->igain_decay;
				}
				else{
					agc->hold_time_count --;
					if(agc->hold_time_count<=0)
						agc->hold_time_count = 0;
				}
			}
			else
				agc->igain = 65535;
				//agc->igain = 0;
		}

//if(i % 64 == 0)
//{
//printf("i:%d, agc->decay:%d, agc->igain : %d, agc->sample_max:%d\r\n", i, agc->igain_decay, agc->igain, agc->sample_max);
//printf("i:%d, agc->decay_limit:%d, diff:%d, agc->igain:%d\r\n", i, agc->decay_limit, (agc->target / agc->sample_max ), agc->igain);
//printf("i:%d, agc->target:%d, agc->sample_max:%d\r\n", i, agc->target, agc->sample_max);
//}
output:
		buffer[i] = (short)((buffer[i] * agc->igain) >> 16);
	}
	agc->sample_max_L_1 = agc->sample_max;
	agc->sample_max = 0;
	agc->gain_old = gain_new;
}


void AIT_AGC_Delete(AIT_AGC_st *agc)
{
	//free(agc);
}

#endif