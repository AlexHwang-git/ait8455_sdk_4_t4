/*
 *  Copyright (c) 2006 Alpha Imaging Technology Corp. All Rights Reserved
 *
 */
#include "avc_bs.h"

static __inline AVC_BSWAP(uint32_t a)
{
    return ((a) = (a << 24)  | (((a) & 0xff00) << 8) | (((a) >> 8) & 0xff00) | (a >> 24));
}

/*
 * FIXME!! 
 * Rewrite these functions which are copied from open source, they
 * are used to build the decoder quickly!!
 */

void avc_bs_init(BS *dec, void *bs, int32_t bs_len)
{
    uint32_t bitstream = (uint32_t)bs;
    uint32_t bytepos,tmp;

    bytepos = ((sizeof(uint32_t)-1) & (uint32_t)bs);
    bitstream -= bytepos;
    dec->start     = dec->tail     = (uint32_t*)bitstream;

    tmp = *(dec->start);
    
    dec->bufA = AVC_BSWAP(tmp);

    tmp = *(dec->start+1);
    
    dec->bufB = AVC_BSWAP(tmp);

    dec->bitpos    = dec->initpos    = 8*bytepos;
    dec->buf_length = bs_len;

    dec->bits_count = 0;
    dec->eof        = 0;
}

#if 0

uint32_t avc_bs_pos(BS *dec)
{
    return ((uint32_t)dec->tail - (uint32_t)dec->start) * 8 + dec->bitpos - dec->initpos;
}


uint32_t avc_bs_eof(BS *dec)
{
    uint32_t pos = ((uint32_t)dec->tail - (uint32_t)dec->start) * 8 + dec->bitpos - dec->initpos;
    return (pos >= dec->buf_length ? 1 : 0);
}
#endif

/*
 * TOTO
 * optimize
 */
uint32_t avc_bs_show(BS *dec, int32_t nbits)
{
    int32_t n;

    n =  (dec->bitpos + nbits) - 32;

    //if ( ( dec->bits_count + nbits - dec->bitpos) > dec->buf_length ) {
    //    //printf("eof!\n");
    //    //return 0xffffffff;
    //}

    if ( n > 0 ) {
        return  ((dec->bufA & (0xffffffff >> dec->bitpos) ) << n ) | (dec->bufB >> (32 - n));
    }
    else {
        return  (dec->bufA & (0xffffffff >> dec->bitpos) ) >> (32 - dec->bitpos - nbits);
    }
}

/* TODO optimize */
void avc_bs_skip(BS *dec, int32_t nbits)
{
    uint32_t tmp;

    if ( ( dec->bits_count + nbits ) >= dec->buf_length ) {
        dec->eof = 1;
        //printf("eof!\n");
        return;
        //return 0xffffffff;
    }

    dec->bitpos += nbits;

    dec->bits_count += nbits;

    if ( dec->bitpos >= 32 ) {
        dec->bufA = dec->bufB;
        tmp = *((uint32_t *) dec->tail + 2);

        

		dec->bufB = AVC_BSWAP(tmp);
        dec->tail++;
        dec->bitpos -= 32;
    }
}

uint32_t avc_bs_read(BS *dec, int32_t nbits )
{
    uint32_t ret;
    
    if ( ( dec->bits_count + nbits ) > dec->buf_length ) {
        dec->eof = 1;
        //printf("eof!\n");
        return 0xffffffff;
    }

    if(nbits > 0) 
    {
        ret = avc_bs_show(dec,nbits);
        avc_bs_skip(dec,nbits);

        return ret;
    }
    else {
        return 0;
    }
}

uint32_t avc_bs_read1(BS *dec)
{
    uint32_t ret;

    if ( ( dec->bits_count + 1 ) > dec->buf_length ) {
        dec->eof = 1;
        //printf("eof!\n");
        return 0xffffffff;
    }

    ret = avc_bs_show(dec,1);
    avc_bs_skip(dec,1);

    return ret;
}

#if 0
void avc_bs_align(BS *dec)
{
    int32_t n = (32 - dec->bitpos) % 8;
    if ( n != 0 ) {
        avc_bs_skip(dec,n);
    }
}
#endif

int avc_bs_read_ue(BS *dec)
{
    int32_t i = 0;
    uint32_t    ret;

    //while( bs_read1( dec ) == 0 && i < 32 )
    while( avc_bs_read1( dec ) == 0 && i < 16 )
    {
        if ( dec->eof ) return 0xffffffff;

        i++;
    }

    dec->golomb_zeros = i;
    if ( i >= 16 ) {
        ret = avc_bs_read( dec, i );

        dec->eof = 1;
        return 0xffffffff;
    }

    ret = avc_bs_read( dec, i );
    
    return( ( 1 << i) - 1 + ret );
}

int avc_bs_read_se(BS *dec)
{
    int32_t val = avc_bs_read_ue(dec);

    //if ( val == 0xffffffff)
    //    return 0xffffffff;

    return val&0x01 ? ((val+1) >> 1) : -(val >> 1);
}

#if 0
int avc_bs_read_te(BS *dec, int32_t x)
{
    if( x == 1 )
    {
        return 1 - avc_bs_read1(dec);
    }
    else if( x > 1 )
    {
        return avc_bs_read_ue(dec);
    }
    return 0;
}
#endif
