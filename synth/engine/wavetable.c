/* $Id: wavetable.c,v 1.21 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 * WaveTable oscillator. Interpolate between adjacent samples
 * to allow variable pitch playback.
 *
 * Copyright 2004 Mobileer, Phil Burk, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_print.h"

#include "spmidi/engine/oscillator.h"

#define NOMINAL_PHASE_INCREMENT   (0x01000000)

#if SPMIDI_ME2000
/*********************************************************************/
/**
 * Play a portion of a wavetable using linear resampling.
 * Phase and PhaseIncrement are 8.24 fixed-point fractions
 * so that we can upsample up to 8 octaves.
 * Fill the output block with SS_FRAMES_PER_BLOCK interpolated values.
 * This routine is time critical and has been optimized for RISC CPUs.
 * Uses:
 *   osc->shared.wave.sampleIndex
 *   osc->shared.wave.waveTable->samples[index]
 *   osc->shared.wave.endAt
 *   osc->phaseInc
 *   osc->shared.wave.next
 *   osc->shared.wave.previous
 *   osc->phase
 *   osc->shared.wave.waveTable->loopEnd
 *   osc->shared.wave.waveTable->loopBegin
 *
 * Modifies:
 *   osc->shared.wave.sampleIndex
 *   osc->shared.wave.previous
 *   osc->shared.wave.next
 *   osc->phase = phase;
 *
 * @param osc wavetable oscillator state structure
 * @param output array to receive calculated values
 */
void Osc_WaveTableS16( Oscillator_t *osc, const WaveTable_t *waveTable, FXP31 *output )
{
    spmSInt     outputSamplesLeft;
    spmSample  *samplePtr;
    /* Index of next sample to be played, or -1 if done. */
    spmSInt32   index;
    /* Samples remaining until end of loop or end wavetable. */
    spmSInt32   samplesLeft;
    /* Position between adjacent sample values, generally 0.0 and 1.0 */
    FXP24       phase;
    /* Controls pitch. */
    FXP24       phaseInc;
    /* Next sample value. */
    FXP15       next;
    /* Previous sample value shifted left by 16 to optimize main loop. */ 
    FXP31       previousShifted;
    /* Difference between adjacent sample values for interpolation. */
    FXP15       delta;

    /* Generate one full block. */
    outputSamplesLeft = SS_FRAMES_PER_BLOCK;
    index = osc->shared.wave.sampleIndex;

    if( index >= 0 ) /* Is the wavetable still playing? */
    {
        /* Cache structure variable in local variables for speed. */
        samplePtr = &((spmSample *)waveTable->samples)[index];
        samplesLeft = osc->shared.wave.endAt - index;
        phaseInc = osc->phaseInc;

        /* These will be written back at the end of the subroutine. */
        next = osc->shared.wave.next;
        {
            spmSInt32 previous = osc->shared.wave.previous;
            previousShifted = previous << 16;
            /* Delta used for interpolation between previous and next. */
            delta = next - previous;
        }
        phase = osc->phase;

        /* Fill output buffer with interpolated samples. */
        /* On ARM, this is faster than a normal for loop.*/
        while( --outputSamplesLeft >= 0 )
        {
            /* Interpolate between previous and next.
             * Basically: out = previous + (phase * (next - previous))
             * Multiplying the FXP24 phase shifted by 9 by a FXP15 delta
             * results in a FXP30 result, shift by 1 to FXP31.
             */
            *output++ = previousShifted + (((phase >> 9) * delta) << 1);

            /* Advance phase. */
            phase += phaseInc;

            /* Has the phase gone past the next sample. */
            while( phase >= NOMINAL_PHASE_INCREMENT )
            {
                /* Move forward sample by sample until phase is back in range.
                 * Adjust phase by amount equivalent to one sample. */
                phase -= NOMINAL_PHASE_INCREMENT;

                /* Get next sample from wavetable. */
                previousShifted = next << 16;
                next = *samplePtr++;

                /* Are we past the end point? */
                if( --samplesLeft <= 0 )
                {
                    /* Are we in a loop? */
                    if( osc->shared.wave.endAt == waveTable->loopEnd )
                    {
                        /* Reset to beginning of loop. */
                        index = waveTable->loopBegin;
                        samplePtr = &((spmSample *)waveTable->samples)[index];
                        samplesLeft = osc->shared.wave.endAt - index;
                    }
                    else
                        /* If not then we must be finished? */
                    {
                        phaseInc = 0;
                        /* Mark oscillator as finished. */
                        osc->shared.wave.sampleIndex = -1;
                        /* Use goto as an efficient way to break out of the nested loops. */
                        goto clearRestOfBuffer;
                    }
                }

                /* Move this down here to get it away from the LOAD of "next" variable.
                 * This may prevent a stall on a CPU with a smart instruction scheduler. */
                delta = next - (previousShifted >> 16); /* Update delta with new samples. */
            }
        }

        osc->shared.wave.sampleIndex = osc->shared.wave.endAt - samplesLeft;
        osc->shared.wave.previous = (short) (previousShifted >> 16);
        osc->shared.wave.next = (short) next;
        osc->phase = phase;
        return;
    }

clearRestOfBuffer:
    /* Clear remainder of buffer. */
    while( --outputSamplesLeft >= 0 )
    {
        *output++ = 0;
    }
}


/* Unsigned A-Law char to index array, for a-law WAVE data. */
static const spmSInt16 sAlawTable[256] =
{
    -5504,   -5248,   -6016,   -5760,   -4480,   -4224,   -4992,
    -4736,   -7552,   -7296,   -8064,   -7808,   -6528,   -6272,
    -7040,   -6784,   -2752,   -2624,   -3008,   -2880,   -2240,
    -2112,   -2496,   -2368,   -3776,   -3648,   -4032,   -3904,
    -3264,   -3136,   -3520,   -3392,  -22016,  -20992,  -24064,
    -23040,  -17920,  -16896,  -19968,  -18944,  -30208,  -29184,
    -32256,  -31232,  -26112,  -25088,  -28160,  -27136,  -11008,
    -10496,  -12032,  -11520,   -8960,   -8448,   -9984,   -9472,
    -15104,  -14592,  -16128,  -15616,  -13056,  -12544,  -14080,
    -13568,    -344,    -328,    -376,    -360,    -280,    -264,
    -312,    -296,    -472,    -456,    -504,    -488,    -408,
    -392,    -440,    -424,     -88,     -72,    -120,    -104,
    -24,      -8,     -56,     -40,    -216,    -200,    -248,
    -232,    -152,    -136,    -184,    -168,   -1376,   -1312,
    -1504,   -1440,   -1120,   -1056,   -1248,   -1184,   -1888,
    -1824,   -2016,   -1952,   -1632,   -1568,   -1760,   -1696,
    -688,    -656,    -752,    -720,    -560,    -528,    -624,
    -592,    -944,    -912,   -1008,    -976,    -816,    -784,
    -880,    -848,    5504,    5248,    6016,    5760,    4480,
    4224,    4992,    4736,    7552,    7296,    8064,    7808,
    6528,    6272,    7040,    6784,    2752,    2624,    3008,
    2880,    2240,    2112,    2496,    2368,    3776,    3648,
    4032,    3904,    3264,    3136,    3520,    3392,   22016,
    20992,   24064,   23040,   17920,   16896,   19968,   18944,
    30208,   29184,   32256,   31232,   26112,   25088,   28160,
    27136,   11008,   10496,   12032,   11520,    8960,    8448,
    9984,    9472,   15104,   14592,   16128,   15616,   13056,
    12544,   14080,   13568,     344,     328,     376,     360,
    280,     264,     312,     296,     472,     456,     504,
    488,     408,     392,     440,     424,      88,      72,
    120,     104,      24,       8,      56,      40,     216,
    200,     248,     232,     152,     136,     184,     168,
    1376,    1312,    1504,    1440,    1120,    1056,    1248,
    1184,    1888,    1824,    2016,    1952,    1632,    1568,
    1760,    1696,     688,     656,     752,     720,     560,
    528,     624,     592,     944,     912,    1008,     976,
    816,     784,     880,     848
};

/*********************************************************************/
/**
 * Same as Osc_WaveTableU8 except it plays ALaw wave data.
 */
void Osc_WaveTableALaw( Oscillator_t *osc, const WaveTable_t *waveTable, FXP31 *output )
{
    spmSInt     outputSamplesLeft;
    /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
    unsigned char  *samplePtr;
    /*******************************************************************/
    /* Index of next sample to be played, or -1 if done. */
    spmSInt32   index;
    /* Samples remaining until end of loop or end wavetable. */
    spmSInt32   samplesLeft;
    /* Position between adjacent sample values, generally 0.0 and 1.0 */
    FXP24       phase;
    /* Controls pitch. */
    FXP24       phaseInc;
    /* Next sample value. */
    FXP15       next;
    /* Previous sample value shifted left by 16 to optimize main loop. */ 
    FXP31       previousShifted;
    /* Difference between adjacent sample values for interpolation. */
    FXP15       delta;

    /* Generate one full block. */
    outputSamplesLeft = SS_FRAMES_PER_BLOCK;
    index = osc->shared.wave.sampleIndex;

    if( index >= 0 ) /* Is the wavetable still playing? */
    {
        /* Cache structure variable in local variables for speed. */
        /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
        /* Point to current position in wavetable. */
        samplePtr = &(((unsigned char  *)(waveTable->samples))[index]);
        /*******************************************************************/
        samplesLeft = osc->shared.wave.endAt - index;
        phaseInc = osc->phaseInc;

        /* These will be written back at the end of the subroutine. */
        next = osc->shared.wave.next;
        {
            spmSInt32 previous = osc->shared.wave.previous;
            previousShifted = previous << 16;
            /* Delta used for interpolation between previous and next. */
            delta = next - previous;
        }
        phase = osc->phase;

        /* Fill output buffer with interpolated samples. */
        /* On ARM, this is faster than a normal for loop.*/
        while( --outputSamplesLeft >= 0 )
        {
            /* Interpolate between previous and next.
             * Basically: out = previous + (phase * (next - previous))
             * Multiplying the FXP24 phase shifted by 9 by a FXP15 delta
             * results in a FXP30 result, shift by 1 to FXP31.
             */
            *output++ = previousShifted + (((phase >> 9) * delta) << 1);

            /* Advance phase. */
            phase += phaseInc;

            /* Has the phase gone past the next sample. */
            while( phase >= NOMINAL_PHASE_INCREMENT )
            {
                /* Move forward sample by sample until phase is back in range.
                 * Adjust phase by amount equivalent to one sample. */
                phase -= NOMINAL_PHASE_INCREMENT;

                /* Get next sample from wavetable. */
                previousShifted = next << 16;
                /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
                /* Convert from ALaw to signed 16 bit. */
                next = (FXP15)( sAlawTable[(int)*samplePtr++] );
                /**********************************************************************/

                /* Are we past the end point? */
                if( --samplesLeft <= 0 )
                {
                    /* Are we in a loop? */
                    if( osc->shared.wave.endAt == waveTable->loopEnd )
                    {
                        /* Reset to beginning of loop. */
                        index = waveTable->loopBegin;
                        /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
                        /* Point to current position in wavetable. */
                        samplePtr = &(((unsigned char  *)(waveTable->samples))[index]);
                        /*******************************************************************/
                        samplesLeft = osc->shared.wave.endAt - index;
                    }
                    else
                        /* If not then we must be finished? */
                    {
                        phaseInc = 0;
                        /* Mark oscillator as finished. */
                        osc->shared.wave.sampleIndex = -1;
                        /* Use goto as an efficient way to break out of the nested loops. */
                        goto clearRestOfBuffer;
                    }
                }

                /* Move this down here to get it away from the LOAD of "next" variable.
                 * This may prevent a stall on a CPU with a smart instruction scheduler. */
                delta = next - (previousShifted >> 16); /* Update delta with new samples. */
            }
        }

        osc->shared.wave.sampleIndex = osc->shared.wave.endAt - samplesLeft;
        osc->shared.wave.previous = (short) (previousShifted >> 16);
        osc->shared.wave.next = (short) next;
        osc->phase = phase;
        return;
    }

clearRestOfBuffer:
    /* Clear remainder of buffer. */
    while( --outputSamplesLeft >= 0 )
    {
        *output++ = 0;
    }
}

/*********************************************************************/
/**
 * If there is no loop, or
 * if there is a loop with nothing after it then
 * then the endAt was already at numSamples.
 * If there is a loop with a release portion
 * then the oscillator will finish the loop and
 * continue to the end of the sample.
 * @param osc wavetable oscillator state structure
 */
void Osc_WaveTable_Release( Oscillator_t *osc, const WaveTable_t *waveTable )
{
    /* Tell oscillator to play to the end of the sample. */
    osc->shared.wave.endAt = waveTable->numSamples;
}


#if SPMIDI_ME3000
/*********************************************************************/
/**
 * Play a portion of a wavetable using linear resampling.
 * Phase and PhaseIncrement are 8.24 fixed-point fractions
 * so that we can upsample up to 8 octaves.
 * Fill the output block with interpolated values.
 * This routine is time critical and has been optimized for RISC CPUs.
 *
 * @param osc wavetable oscillator state structure
 * @param output array to receive calculated values
 */
void Osc_WaveTableU8( Oscillator_t *osc, const WaveTable_t *waveTable, FXP31 *output )
{
    spmSInt     outputSamplesLeft;
    /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
    unsigned char  *samplePtr;
    /*******************************************************************/
    /* Index of next sample to be played, or -1 if done. */
    spmSInt32   index;
    /* Samples remaining until end of loop or end wavetable. */
    spmSInt32   samplesLeft;
    /* Position between adjacent sample values, generally 0.0 and 1.0 */
    FXP24       phase;
    /* Controls pitch. */
    FXP24       phaseInc;
    /* Next sample value. */
    FXP15       next;
    /* Previous sample value shifted left by 16 to optimize main loop. */ 
    FXP31       previousShifted;
    /* Difference between adjacent sample values for interpolation. */
    FXP15       delta;

    /* Generate one full block. */
    outputSamplesLeft = SS_FRAMES_PER_BLOCK;
    index = osc->shared.wave.sampleIndex;

    if( index >= 0 ) /* Is the wavetable still playing? */
    {
        /* Cache structure variable in local variables for speed. */
        /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
        /* Point to current position in wavetable. */
        samplePtr = &(((unsigned char  *)(waveTable->samples))[index]);
        /*******************************************************************/
        samplesLeft = osc->shared.wave.endAt - index;
        phaseInc = osc->phaseInc;

        /* These will be written back at the end of the subroutine. */
        next = osc->shared.wave.next;
        {
            spmSInt32 previous = osc->shared.wave.previous;
            previousShifted = previous << 16;
            /* Delta used for interpolation between previous and next. */
            delta = next - previous;
        }
        phase = osc->phase;

        /* Fill output buffer with interpolated samples. */
        /* On ARM, this is faster than a normal for loop.*/
        while( --outputSamplesLeft >= 0 )
        {
            /* Interpolate between previous and next.
             * Basically: out = previous + (phase * (next - previous))
             * Multiplying the FXP24 phase shifted by 9 by a FXP15 delta
             * results in a FXP30 result, shift by 1 to FXP31.
             */
            *output++ = previousShifted + (((phase >> 9) * delta) << 1);

            /* Advance phase. */
            phase += phaseInc;

            /* Has the phase gone past the next sample. */
            while( phase >= NOMINAL_PHASE_INCREMENT )
            {
                /* Move forward sample by sample until phase is back in range.
                 * Adjust phase by amount equivalent to one sample. */
                phase -= NOMINAL_PHASE_INCREMENT;

                /* Get next sample from wavetable. */
                previousShifted = next << 16;
                /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
                /* Convert from unsigned 8 bit to signed 16 bit. */
                next = (((int)(*samplePtr++)) - 128) << 8;
                /**********************************************************************/

                /* Are we past the end point? */
                if( --samplesLeft <= 0 )
                {
                    /* Are we in a loop? */
                    if( osc->shared.wave.endAt == waveTable->loopEnd )
                    {
                        /* Reset to beginning of loop. */
                        index = waveTable->loopBegin;
                        /************* THIS LINE IS DIFFERENT THEN THE S16 VERSION ************/
                        /* Point to current position in wavetable. */
                        samplePtr = &(((unsigned char  *)(waveTable->samples))[index]);
                        /*******************************************************************/
                        samplesLeft = osc->shared.wave.endAt - index;
                    }
                    else
                        /* If not then we must be finished? */
                    {
                        phaseInc = 0;
                        /* Mark oscillator as finished. */
                        osc->shared.wave.sampleIndex = -1;
                        /* Use goto as an efficient way to break out of the nested loops. */
                        goto clearRestOfBuffer;
                    }
                }

                /* Move this down here to get it away from the LOAD of "next" variable.
                 * This may prevent a stall on a CPU with a smart instruction scheduler. */
                delta = next - (previousShifted >> 16); /* Update delta with new samples. */
            }
        }

        osc->shared.wave.sampleIndex = osc->shared.wave.endAt - samplesLeft;
        osc->shared.wave.previous = (short) (previousShifted >> 16);
        osc->shared.wave.next = (short) next;
        osc->phase = phase;
        return;
    }

clearRestOfBuffer:
    /* Clear remainder of buffer. */
    while( --outputSamplesLeft >= 0 )
    {
        *output++ = 0;
    }
}

#endif  /* SPMIDI_ME3000 */

#endif  /* SPMIDI_ME2000 */
