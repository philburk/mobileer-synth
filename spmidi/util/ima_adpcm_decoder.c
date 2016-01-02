/* $Id: ima_adpcm_decoder.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * Low level IMA Intel/DVI ADPCM decoder and encoder.
 *
 * Place first encoded nibble at aaaa in low bits
 * Place second encoded nibble at bbbb in high bits
 * So encoded byte looks like bbbbaaaa.
 *
 * Note that this order matches the nibble order in IMA ADPCM WAV files,
 * but is the opposite of some raw IMA ADPCM implementations.
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/ima_adpcm.h"

//#include "stdio.h"
//#define PRINT(x) { printf x; }
#define PRINT(x)

int IMA_DecodeDelta( int stepSize, char encodedSample );

/*  DVI ADPCM step table */
const int gIMA_ADPCM_StepSizes[89] =
    {
        7,     8,     9,    10,    11,    12,    13,    14,    16,     17,    19,
        21,    23,    25,    28,    31,    34,    37,    41,    45,     50,    55,
        60,    66,    73,    80,    88,    97,   107,   118,   130,    143,   157,
        173,   190,   209,   230,   253,   279,   307,   337,   371,    408,   449,
        494,   544,   598,   658,   724,   796,   876,   963,  1060,   1166,  1282,
        1411,  1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,   3327,  3660,
        4026,  4428,  4871,  5358,  5894,  6484,  7132,  7845,  8630,   9493, 10442,
        11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,  27086, 29794,
        32767
    };

/* DVI table of stepsize index deltas */
const int gIMA_ADPCM_IndexDeltas[8]  =
    {
        -1, -1, -1, -1, 2, 4, 6, 8
    };

#define CLIP(x,lo,hi) \
    if ( x < lo ) \
    { \
        x = lo; \
    } \
    else if ( x > hi ) \
    { \
        x = hi; \
    }


/**************************************************************
 * Calculate the delta from an ADPCM Encoded Sample.
 * This is basically a low precision multiply.
 */

int IMA_DecodeDelta( int stepSize, char encodedSample )
{
    int delta = 0;

    if( encodedSample & 4)
    {
        delta = stepSize;
    }

    stepSize = stepSize >> 1;
    if( encodedSample & 2)
    {
        delta += stepSize;
    }

    stepSize = stepSize >> 1;
    if( encodedSample & 1)
    {
        delta += stepSize;
    }

    stepSize = stepSize >> 1;
    delta += stepSize;

    if (encodedSample & 8)
    {
        delta = -delta;
    }

    return( delta );
}

/************************************************************************************
 * Decode an array of packed IMA Intel/DVI ADPCM data into an array of shorts.
 * Set initialValue and initialStepIndex which may come from block headers
 * in an encoded stream.
 * @return final stepIndex;
 */
int IMA_DecodeArray( const unsigned char *encodedArray,
                     int   numSamples,         /* Two samples per byte of encodedArray. */
                     short initialValue,       /* Typically 0. */
                     int   initialStepIndex,   /* Value between 0 and 88, typically 0. */
                     short *decodedArray       /* Output array must be large enough for numSamples. */
                   )
{
    short outputSample;
    char  encodedSample;
    int   stepIndex;
    int   i;
    unsigned char inputByte = 0;

    outputSample = initialValue;
    stepIndex = initialStepIndex;

    for( i=0; i<numSamples; i++ )
    {
        if ( (i&1) == 0 )
        {
            inputByte = *encodedArray++;
            /* Use least significant nibble first. */
            encodedSample = (char) (0x000F & inputByte); 
        }
        else
        {
            /* Then use most significant nibble. */
            encodedSample = (char) (0x000F & (inputByte>>4));
        }

        outputSample = IMA_DecodeNibble( encodedSample, outputSample, &stepIndex );
        *decodedArray++ = outputSample;

    }
    return stepIndex;
}

/************************************************************************************
 * Decode an IMA Intel/DVI ADPCM nibble.
 * stepIndexPtr - pointer to step index whose value is used then updated.
 * Return decoded value.
 */
short IMA_DecodeNibble( unsigned char encodedSample,
                        short previousValue,
                        int   *stepIndexPtr   /* Value between 0 and 88, typically 0. */
                      )
{
    int   outputSample;
    int   stepIndex;
    int   stepSize;

    stepIndex = *stepIndexPtr;
    CLIP( stepIndex, 0, 88 );
    stepSize = gIMA_ADPCM_StepSizes[stepIndex];

    /* decode ADPCM code value to reproduce Dn and accumulates an estimated outputSample */
    outputSample = previousValue + IMA_DecodeDelta( stepSize, encodedSample);
    CLIP( outputSample, -32768, 32767 );

    /* stepsize adaptation */
    stepIndex += gIMA_ADPCM_IndexDeltas[ encodedSample & 7 ];
    CLIP( stepIndex, 0, 88 );
    *stepIndexPtr = stepIndex;

    return (short) outputSample;
}

