/* $Id: ima_adpcm_encoder.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/*
 * Low level IMA Intel/DVI ADPCM encoder.
 * The encoder requires the decoder.
 *
 * Place first encoded nibble at aaaa in low bits
 * Place second encoded nibble at bbbb in high bits
 * in byte as aaaabbbb.
 *
 * Note that this order matches the nibble order in IMA ADPCM WAV files.
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/ima_adpcm.h"

extern const int gIMA_ADPCM_StepSizes[];
extern const int gIMA_ADPCM_IndexDeltas[];

#define CLIP(x,lo,hi) \
    if ( x < lo ) \
    { \
        x = lo; \
    } \
    else if ( x > hi ) \
    { \
        x = hi; \
    }

int IMA_DecodeDelta( int stepSize, int delta );
static char IMA_EncodeDelta( int stepSize, int delta );

/************************************************************************************/
void IMA_EncodeArray(
    const short *decodedArray,         /* Input array of uncompresed samples. */
    int   numSamples,
    short *previousSamplePtr,    /* Updated with estimated new value. */
    int   *stepIndexPtr,         /* Value between 0 and 88, typically 0. */
    unsigned char *encodedArray  /* Two samples per byte of encodedArray. */
)
{
    short inputSample;
    char  encodedNibble;
    int   i;
    unsigned char outputByte;

    for( i=0; i<numSamples; i++ )
    {
        inputSample = *decodedArray++;

        encodedNibble = IMA_EncodeNibble( inputSample, previousSamplePtr, stepIndexPtr );

        if ( (i&1) == 0 )
        {
            /* Write least significant nibble first! */
            outputByte = (unsigned char) (0x000F & encodedNibble);
        }
        else
        {
            /* Write most significant nibble second! */
            outputByte |= (unsigned char) (encodedNibble << 4);
            *encodedArray++ = outputByte;
        }
    }
}

/************************************************************************************/
unsigned char IMA_EncodeNibble(
    short inputSample,
    short *previousSamplePtr,  /* Updated with estimated new value. */
    int   *stepIndexPtr   /* Value between 0 and 88, typically 0. */
)
{
    int  previousSample;
    int  stepSize;
    int  delta;
    int  stepIndex;
    unsigned char  encodedSample;

    stepIndex = *stepIndexPtr;
    previousSample = *previousSamplePtr;
    stepSize = gIMA_ADPCM_StepSizes[ stepIndex ];

    /* calculate delta */
    delta = inputSample - previousSample;
    CLIP( delta, -32768L, 32767L );

    /* encode delta relative to the current stepsize */
    encodedSample = IMA_EncodeDelta( stepSize, delta );

    /* decode ADPCM code value to reproduce delta and generate an estimated InputSample */
    previousSample += IMA_DecodeDelta( stepSize, encodedSample);
    CLIP( previousSample, -32768L, 32767L );
    *previousSamplePtr = (short) previousSample;

    /* adapt stepsize */
    stepIndex += gIMA_ADPCM_IndexDeltas[ encodedSample & 7 ];
    CLIP( stepIndex, 0, 88 );
    *stepIndexPtr = stepIndex;

    return encodedSample;
}

/**
 * Encode the differential value and output an ADPCM Encoded Sample
 */

static char IMA_EncodeDelta( int stepSize, int delta )
{
    char encodedSample = 0;

    /* Set sign bit. */
    if ( delta < 0 )
    {
        encodedSample = 8;
        delta = -delta;
    }

    if ( delta >= stepSize )
    {
        encodedSample |= 4;
        delta -= stepSize;
    }
    
    stepSize = stepSize >> 1;
    if ( delta >= stepSize )
    {
        encodedSample |= 2;
        delta -= stepSize;
    }
    
    stepSize = stepSize >> 1;
    if ( delta >= stepSize )
    {
        encodedSample |= 1;
    }

    return ( encodedSample );
}

