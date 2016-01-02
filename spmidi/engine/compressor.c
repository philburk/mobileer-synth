/* $Id: compressor.c,v 1.19 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Dynamic Range Compressor
 * This compressor uses a unique formula to calculate the gain from the peak amplitude:
 *
 *  if( peakAmplitude < threshold ) peakAmplitude = threshold;
 *
 *  gain = (target + curve) / (peakAmplitude + curve)
 *
 * The effect of this curve value is to soften the gain versus peak function
 * that normally has a sharp corner at the threshold value.
 * The result is that softer passage are boosted, but are still
 * softer then loud passages resulting in a more natural sound.
 *
 * There are two peak followers.
 * The fastPeak tracks the peaks of the input samples and decays exponentially.
 * The slowPeak follows fastPeak except when fastPeak is rising and then slowPeak
 * rises exponentially based on attack time.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/compressor.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/include/spmidi_print.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

/* Set resolution for gain so that mixer bus resolution is preserved. */
#define COMPRESSOR_SHIFT   (12)

/* Default compressor parameters. */
#define THRESHOLD    ( 2)
#define CURVATURE    (20)
#define TARGET       (88)

#define PER_10000(n)  (((n) << COMPRESSOR_SHIFT) / 10000) /* DIVIDE - init */

/** Precalculated attack and decay scalers based on this equation.
 * attackScaler = 10000 * (2.0 ^ (blockSize / (sampleRate * timeInSeconds)))
 * decayScaler  = 10000 * (0.5 ^ (blockSize / (sampleRate * timeInSeconds)))
 *
 * Default values are based on 25 msec attack and 260 msec decay times.
 * Times are "half lifes", the time it takes to double or half the value.
 */
#define ATTACK_SCALE (8)

#if (ATTACK_SCALE == 2)
/* These will rise 2X in 25 msec. */
#define ATTACK_16000 PER_10000(10070)
#define ATTACK_22050 PER_10000(10101)
#define ATTACK_44100 PER_10000(10050)

#elif (ATTACK_SCALE == 4)
/* These will rise 4X in 25 msec. */
#define ATTACK_16000 PER_10000(10140)
#define ATTACK_22050 PER_10000(10203)
#define ATTACK_44100 PER_10000(10101)

#elif (ATTACK_SCALE == 8)
/* These will rise 8X in 25 msec. */
#define ATTACK_16000 PER_10000(10210)
#define ATTACK_22050 PER_10000(10306)
#define ATTACK_44100 PER_10000(10152)
#endif

#define DECAY_16000  PER_10000( 9993)
#define DECAY_22050  PER_10000( 9990)
#define DECAY_44100  PER_10000( 9995)

#define BLOCK_22050   (8)
#define BLOCK_16000   (4)
#define BLOCK_44100   (8)

/* Sufficient delay to give gain time to drop before big peak is heard. */
#define MSEC_DELAY   (22)
#define MSEC_PER_SECOND   (1000)

/********************************************************************/
void Compressor_Term( Compressor_t *compressor )
{
#if SPMIDI_SUPPORT_MALLOC
    if( compressor->delayLine != NULL )
    {
        SPMIDI_FreeMemory( compressor->delayLine );
        compressor->delayLine = NULL;
    }
#else
    (void) compressor;
#endif
}

/********************************************************************/
int Compressor_Init( Compressor_t *compressor, int sampleRate )
{
    int delayLineNumBytes = COMP_DELAY_LINE_SIZE * sizeof(FXP31);

#if SPMIDI_SUPPORT_MALLOC
    compressor->delayLine = SPMIDI_ALLOC_MEM( delayLineNumBytes, "compressor" );
    if( compressor->delayLine == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
#endif

    MemTools_Clear( compressor->delayLine, delayLineNumBytes );

    /* Sample rate dependant parameters. */
    if( sampleRate <= 20000 )
    {
        compressor->attack = ATTACK_16000;
        compressor->decay = DECAY_16000;
        compressor->blockSize = BLOCK_16000;
    }
    else if( sampleRate <= 30000 )
    {
        compressor->attack = ATTACK_22050;
        compressor->decay = DECAY_22050;
        compressor->blockSize = BLOCK_22050;
    }
    else
    {
        compressor->attack = ATTACK_44100;
        compressor->decay = DECAY_44100;
        compressor->blockSize = BLOCK_44100;
    }

    Compressor_SetParameter( compressor, SPMIDI_PARAM_COMPRESSOR_CURVE, CURVATURE );
    Compressor_SetParameter( compressor, SPMIDI_PARAM_COMPRESSOR_TARGET, TARGET );
    Compressor_SetParameter( compressor, SPMIDI_PARAM_COMPRESSOR_THRESHOLD, THRESHOLD );

    compressor->sampleCounter = 0;
    compressor->writeIndex = 0;

    /* Start peak follower at a reasonable value so beginning of song does not swell or drop. */
#define START_SHIFT  (3)
    compressor->fastPeak = ( compressor->threshold > (FXP31_MAX_VALUE >> START_SHIFT) )
                           ? (FXP31_MAX_VALUE >> START_SHIFT)
                           : (compressor->threshold << START_SHIFT);
    compressor->slowPeak = compressor->fastPeak;

    compressor->frameDelay = (MSEC_DELAY * sampleRate) / MSEC_PER_SECOND; /* DIVIDE - init */

    /* Hold compressor gain steady to reduce pumping effect. */
//#define SPMIDI_COMPRESSOR_HOLD_TIME_MSEC  (500)
//  compressor->holdCounter = 0;
//  compressor->holdCounterInitial = SPMIDI_COMPRESSOR_HOLD_TIME_MSEC * sampleRate / (1000 * compressor->blockSize);

    return SPMIDI_Error_None;
}

/********************************************************************/
void Compressor_CompressBuffer( Compressor_t *compressor, FXP31 *samples, int samplesPerFrame, FXP7 gain )
{
    FXP31  scaledSample;
    FXP31  absSample;
    FXP31  sample;
    long   numerator;
    int    is;
    int    numSamples = SS_FRAMES_PER_BUFFER * samplesPerFrame;
    int    blockMask = (compressor->blockSize * samplesPerFrame) - 1;
    int    sampleDelay = compressor->frameDelay * samplesPerFrame;
    int    readIndex;

    /* Make sure delay is not bigger than array size. */
    if( sampleDelay > (COMP_DELAY_LINE_SIZE - samplesPerFrame) )
    {
        sampleDelay = (COMP_DELAY_LINE_SIZE - samplesPerFrame);
    }

    /* Assumes always called with same samplesPerFrame. */
    readIndex = (compressor->writeIndex - sampleDelay) & COMP_DELAY_LINE_MASK;

    numerator = (((compressor->target + compressor->curvature) * gain) << (COMPRESSOR_SHIFT - 7));

    for( is=0; is<numSamples; is++ )
    {
        /* Occasionally update gain.
         * The fastPeak only decays in this section. So 
         * Input value of fastPeak is max of samples in previous sections.
         */
        if( (compressor->sampleCounter++ & blockMask) == 0 )
        {
            int    isRising;
            long   shiftedPeak;
            /* Calculate slow peak follower from fastPeak. */
            isRising = compressor->fastPeak > compressor->slowPeak;
            if( isRising )
            {
                /* slowPeak rises at a slower speed so that the attack is smooth. */
                compressor->slowPeak = (compressor->slowPeak >> COMPRESSOR_SHIFT) * compressor->attack;
            }
            else
            {
                /* Decay for fast and slow follower is the same. */
                compressor->slowPeak = compressor->fastPeak;
            }

            /* Use slowPeak for calculating gain. */
            shiftedPeak = compressor->slowPeak >> ((SS_MIXER_BUS_RESOLUTION - 1) - COMPRESSOR_SHIFT);

            compressor->gain = numerator / (shiftedPeak + compressor->curvature); /* DIVIDE - control rate */

            /* Exponentially decay the fast peak by scaling. */
            compressor->fastPeak = (compressor->fastPeak >> COMPRESSOR_SHIFT)  * compressor->decay;

            /* Don't let peak go below threshold. */
            if( compressor->fastPeak < compressor->threshold )
            {
                compressor->fastPeak = compressor->threshold;
            }
        }

        /* Get next sample from input. Advance pointer when we write to it at end of loop. */
        sample = samples[is];

        /* Write incoming samples to delay line. */
        compressor->delayLine[ compressor->writeIndex + is ] = sample;

        /* Absolute value of current sample. */
        absSample = (sample < 0) ? (0 - sample) : sample;

        /* Traditional peak follower that rises directly with signal. */
        if( absSample > compressor->fastPeak )
        {
            compressor->fastPeak = absSample;
        }

        /* Apply latest gain to delayed signal. */
        scaledSample = (compressor->delayLine[readIndex] >> COMPRESSOR_SHIFT) * compressor->gain;
        readIndex = (readIndex + 1) & COMP_DELAY_LINE_MASK;

        /* Clip to bus rails. */
#if SPMIDI_USE_SOFTCLIP
        /* Soft clip.. */
        scaledSample = SS_MixerSoftClip( scaledSample );
#else
        /* Hard clip. */
        if( scaledSample > SS_MIXER_BUS_MAX )
        {
            scaledSample = SS_MIXER_BUS_MAX;
        }
        else if( scaledSample < SS_MIXER_BUS_MIN )
        {
            scaledSample = SS_MIXER_BUS_MIN;
        }
#endif

        /* Write back to original array. */
#define DEBUG_COMPRESSOR  (0)
#if DEBUG_COMPRESSOR
#if !defined(PARANOID)
#error Writing compressor debug data to right channel.
#endif
        /* Write debug data to right channel. */
        if( (is & 1) == 0)
        {
            samples[is] = scaledSample;
        }
        /* else samples[is] = compressor->slowPeak >> 2; */
        else
        {
            //samples[is] = compressor->gain << 11;
            samples[is] = compressor->slowPeak;
            //samples[is] = compressor->fastPeak;
        }
#else
        samples[is] = scaledSample;
#endif

    }
    /* Be careful. This assumes that the writeIndex only wraps at the end of the FOR loop.
     * So the delay line size must be a multiple of the loop count.
     */
    compressor->writeIndex = (compressor->writeIndex + numSamples) & COMP_DELAY_LINE_MASK;
}


/********************************************************************/
FXP31 Compressor_EstimateBuffer( Compressor_t *compressor, FXP31 sample, FXP7 gain )
{
    FXP31  compressedSample;
    FXP31  shiftedPeak;

    shiftedPeak = sample >> ((SS_MIXER_BUS_RESOLUTION - 1) - COMPRESSOR_SHIFT);

    compressor->gain = (((compressor->target + compressor->curvature) * gain) << (COMPRESSOR_SHIFT - 7)) /
                       (shiftedPeak + compressor->curvature);

    compressedSample = (sample >> COMPRESSOR_SHIFT) * compressor->gain;

    return compressedSample;

}

/********************************************************************/
int Compressor_SetParameter( Compressor_t *compressor, SPMIDI_Parameter parameterIndex, int value )
{
    int result = 0;
    switch( parameterIndex )
    {

    case SPMIDI_PARAM_COMPRESSOR_CURVE:
        compressor->curvature = ((value << COMPRESSOR_SHIFT) / 100); /* DIVIDE - init */
        /* Avoid divide-by-zero error if shiftedPeak is also zero. */
        if( compressor->curvature < 16 )
        {
            compressor->curvature = 16;
        }
        break;

    case SPMIDI_PARAM_COMPRESSOR_TARGET:
        compressor->target = ((value << COMPRESSOR_SHIFT) / 100); /* DIVIDE - init */
        break;

    case SPMIDI_PARAM_COMPRESSOR_THRESHOLD:
        compressor->threshold = ((value << (SS_MIXER_BUS_RESOLUTION - 1)) / 100); /* DIVIDE - init */
        break;

    default:
        result = SPMIDI_Error_UnrecognizedParameter;
        break;
    }
    return result;
}

/********************************************************************/
int Compressor_GetParameter( Compressor_t *compressor, SPMIDI_Parameter parameterIndex, int *valuePtr )
{
    int result = 0;
    switch( parameterIndex )
    {

    case SPMIDI_PARAM_COMPRESSOR_CURVE:
        *valuePtr = (compressor->curvature * 100) >> COMPRESSOR_SHIFT;
        break;

    case SPMIDI_PARAM_COMPRESSOR_TARGET:
        *valuePtr = (compressor->target * 100) >> COMPRESSOR_SHIFT;
        break;

    case SPMIDI_PARAM_COMPRESSOR_THRESHOLD:
        *valuePtr = (compressor->threshold * 100) >> (SS_MIXER_BUS_RESOLUTION - 1);
        break;

    default:
        result = SPMIDI_Error_UnrecognizedParameter;
        break;
    }
    return result;
}
