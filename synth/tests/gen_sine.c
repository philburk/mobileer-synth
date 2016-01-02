/* $Id$ */
/**
 *
 * Generate a test sine wave on one or two channels.
 *
 * Author: Phil Burk
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "gen_sine.h"

static const short sSineTable[] =
    {
        0,   1607,   3211,   4807,   6392,   7961,   9511,  11038,
        12539,  14009,  15446,  16845,  18204,  19519,  20787,  22004,
        23169,  24278,  25329,  26318,  27244,  28105,  28897,  29621,
        30272,  30851,  31356,  31785,  32137,  32412,  32609,  32727,
        32766,  32727,  32609,  32412,  32137,  31785,  31356,  30851,
        30272,  29621,  28897,  28105,  27244,  26318,  25329,  24278,
        23169,  22004,  20787,  19519,  18204,  16845,  15446,  14009,
        12539,  11038,   9511,   7961,   6392,   4807,   3211,   1607,
        0,  -1607,  -3211,  -4807,  -6392,  -7961,  -9511, -11038,
        -12539, -14009, -15446, -16845, -18204, -19519, -20787, -22004,
        -23169, -24278, -25329, -26318, -27244, -28105, -28897, -29621,
        -30272, -30851, -31356, -31785, -32137, -32412, -32609, -32727,
        -32766, -32727, -32609, -32412, -32137, -31785, -31356, -30851,
        -30272, -29621, -28897, -28105, -27244, -26318, -25329, -24278,
        -23169, -22004, -20787, -19519, -18204, -16845, -15446, -14009,
        -12539, -11038,  -9511,  -7961,  -6392,  -4807,  -3211,  -1607
    };

#define NUM_SINE_VALUES (sizeof(sSineTable) / sizeof(short))

/* Advance through sine table at different prime deltas
 * for left and right channel. This wil make it obvious if there
 * are glitches or buffer sizing problems in the audio driver.
 */
static int sineIndices[] = { 0 , 0 };
static int sineDeltas[] = { 3 , 5 };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Fill a buffer with a mono or stereo sinewave signal.
 * This can be used for testing an audio device driver.
 *
 * @param samples pointer to buffer to receive sine wave samples
 * @param framesPerBuffer number of mono sample or stereo pairs
 * @param samplesPerFrame 1 for mono, 2 for stereo
 * @return zero or negative error code
 */
int FillBufferWithSines( short *samples, int framesPerBuffer, int samplesPerFrame )
{
    int frameIndex;
    int channelIndex;

    if( (samplesPerFrame > 2) || (samplesPerFrame < 1) )
    {
        return -1;
    }

    for( frameIndex=0; frameIndex<framesPerBuffer; frameIndex++ )
    {
        for( channelIndex=0; channelIndex<samplesPerFrame; channelIndex++ )
        {
            int n = sineIndices[ channelIndex ];
            *samples++ = sSineTable[ n ];
            n += sineDeltas[ channelIndex ];
            if( n >= NUM_SINE_VALUES )
            {
                n -= NUM_SINE_VALUES;
            }
            sineIndices[ channelIndex ] = n;
        }
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
