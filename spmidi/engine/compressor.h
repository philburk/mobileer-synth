#ifndef _COMPRESSOR_H
#define _COMPRESSOR_H

/* $Id: compressor.h,v 1.12 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Dynamic Range Compressor
 * Boost the volume of quiet portions.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"

/* Set compressor buffer size based on Maximum Sample Rate.
 * We use smaller pre-delay if the max SR is low.
 */
#if (SPMIDI_MAX_SAMPLE_RATE <= 11025)
    #define COMP_DELAY_LINE_SIZE    (256 * SPMIDI_MAX_SAMPLES_PER_FRAME)
#elif (SPMIDI_MAX_SAMPLE_RATE <= 22050)
    #define COMP_DELAY_LINE_SIZE    (512 * SPMIDI_MAX_SAMPLES_PER_FRAME)
#else
    #define COMP_DELAY_LINE_SIZE    (1024 * SPMIDI_MAX_SAMPLES_PER_FRAME)
#endif

#define COMP_DELAY_LINE_MASK    (COMP_DELAY_LINE_SIZE - 1)

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct Compressor_s
    {
        FXP31      threshold;
        /** Rises immediately with peaks, decays slowly. */
        FXP31      fastPeak;
        /** Rises slowly with peaks. */
        FXP31      slowPeak;
        spmSInt32  attack;
        spmSInt32  decay;
        spmSInt32  curvature;
        spmSInt32  target;
        spmSInt32  gain;
        spmSInt    blockSize;
        spmSInt    sampleCounter;
        /* Used to countdown when holding the compressor gain steady. */
        //spmSInt    holdCounter;
        /* Value to reset holdCounter. */
        //spmSInt    holdCounterInitial;
        /* Delay line allows attack to rise before peak heard. */
        spmSInt    writeIndex;
        spmSInt    frameDelay;
#if SPMIDI_SUPPORT_MALLOC

        FXP31     *delayLine;
#else

        FXP31      delayLine[COMP_DELAY_LINE_SIZE];
#endif

    }
    Compressor_t;

    void Compressor_Term( Compressor_t *compressor );

    int Compressor_Init( Compressor_t *compressor, int sampleRate );

    /** Compress complete buffer of size SS_FRAMES_PER_BUFFER. */
    void Compressor_CompressBuffer( Compressor_t *compressor, FXP31 *samples,
                                    int samplesPerFrame, FXP7 gain );

    FXP31 Compressor_EstimateBuffer( Compressor_t *compressor, FXP31 sample, FXP7 gain );

    int Compressor_SetParameter( Compressor_t *compressor, SPMIDI_Parameter parameterIndex, int value );

    /**
     * Get indexed parameter value for compressor.
     */
    int Compressor_GetParameter( Compressor_t *compressor, SPMIDI_Parameter parameterIndex, int *valuePtr );


#ifdef __cplusplus
}
#endif

#endif /* _COMPRESSOR_H */

