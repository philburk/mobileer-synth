/* $Id: reverb.c,v 1.12 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Reverberation based on a single multi-tap delay line.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_print.h"
#include "reverb.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#ifndef FALSE
#define FALSE (0)
#define TRUE  (1)
#endif

#define MOVE_TAPS  (0)

static const short tapBasePositions[] =
    {
//        1492, 2042, 2297, 2689, 3022, 3481, 3577, 3822
        411, 926, 1580, 2293, 3022, 3481, 3577, 3822
    };
#define NUM_TAPS (sizeof(tapBasePositions)/sizeof(short))

#if MOVE_TAPS
/* This should not be const because the displacements are moved. */
static unsigned char tapDisplacements[NUM_TAPS] = { 128, 128, 128, 128, 128, 128, 128, 128 };
#endif

#define OUTPUT_SHIFT     (11)
#define FEEDBACK_SCALER  (430)
#define FEEDBACK_SHIFT   (12)

#if MOVE_TAPS
static unsigned long randSeed = 22222;
/* Calculate pseudo-random 32 bit number based on linear congruential method. */
#define NEXT_RANDOM  (randSeed = (randSeed * 196314165) + 907633515)

/*********************************************************************************
 * Randomly move the taps around to reduce metallic ringing.
 * Unsigned displacement is moved towards a random unsigned byte value.
 * This keeps the displacements centered around 128.
 */
static void Reverb_MoveTaps( void )
{
    int i;
    unsigned long rand = NEXT_RANDOM >> 8;
    for( i=0; i<sizeof(tapDisplacements); i++ )
    {
        unsigned char disp = tapDisplacements[i];
        unsigned char nextByte = (unsigned char) rand;
        rand = rand >> 1;
        if( nextByte < disp )
            disp -= 1;
        else
            disp += 1;
        tapDisplacements[i] = disp;
    }
}
#endif

/*********************************************************************************/
void Reverb_Next( Reverb_t *reverb, FXP31 *input, FXP31 *output,
                 int numSamples, int isStereoInput )
{
    int is, it;
    int writeIndex = reverb->writeIndex;
    int filterOutput = reverb->filterOutput;

    for( is=0; is<numSamples; is++ )
    {
        int feedback;
        /* Mix multiple taps from delay line. */
        int mixedTaps = 0;
        for( it=0; it<NUM_TAPS; it++ )
        {
#if MOVE_TAPS
            int tapPos = (tapBasePositions[it] + tapDisplacements[it]);
#else
            int tapPos = tapBasePositions[it];
#endif
            int idx = (writeIndex - tapPos) & REVERB_INDEX_MASK;
            int delayedValue = reverb->delayLine[ idx ];
            mixedTaps += delayedValue;
        }
        /* Shifting previous output by 2 will give us an IIR filter feedback gain of 0.25 */
        filterOutput = mixedTaps + (filterOutput >> 2);
        output[ is ] = filterOutput << OUTPUT_SHIFT;

        /* Write mix of input and feedback into delay line. */
        feedback = (filterOutput * FEEDBACK_SCALER) >> FEEDBACK_SHIFT;

        /* Write input to delay line. */
        {
            int inputSample;
            if( isStereoInput )
            {
                /* Add adjacent input samples if input is interleaved stereo. */
                FXP31 *inputPtr = &input[ is << 1 ];
                inputSample = *inputPtr++;
                inputSample += *inputPtr;
            }
            else
            {
                /* Grab mono sample. */
                inputSample = input[ is ];
            }
            /* Subtract feedback to prevent it from blowing up at zero frequency. */
            reverb->delayLine[ writeIndex ] = (short) ((inputSample>>16) - feedback);
        }

        /* Advance write index and wrap at end. */
        writeIndex = (writeIndex + 1) & REVERB_INDEX_MASK;
    }
    reverb->writeIndex = writeIndex;
    reverb->filterOutput = filterOutput;

    /* Occasionally move taps to avoid metallic ringing sound. */
#if MOVE_TAPS

    reverb->framesSinceTapsMoved += numSamples;
    if( reverb->framesSinceTapsMoved >= 256 )
    {
        Reverb_MoveTaps();
        reverb->framesSinceTapsMoved = 0;
    }
#endif
}

#if 0
#include "spmidi/include/write_wav.h"

static Reverb_t REVERB = { 0 };

/* Render filter to file for external viewing with wave editor. */
int main( void )
{
    int i;
    short sample;
    WAV_Writer *writer;
    int result;
    FXP31 input[SS_FRAMES_PER_BLOCK];
    FXP31 output[SS_FRAMES_PER_BLOCK];

    printf("Test Reverb\n");


    /* Open file. */
    result = Audio_WAV_CreateWriter( &writer, "rendered_midi.wav" );
    if( result < 0 )
    {
        printf("Can't open output file rendered_svf.raw\n" );
        return 1;
    }

    result =  Audio_WAV_OpenWriter( writer, 44100, 2 );
    if( result < 0 )
        return result;

    /* clear input */
    for( i=0; i<SS_FRAMES_PER_BLOCK; i++ )
        input[i] = 0;

    for( i=0; i<1000; i++ )
    {
        int is;

        /* Occasional impulse. */
        /* if( (i & 7) == 0 ) input[5] = FXP31_MAX_VALUE/100; */
        if( i == 10 )
        {
            input[5] = FXP31_MAX_VALUE/2;
        }
        else
        {
            input[5] = 0;
        }

        Reverb_Next( &REVERB, input, output, SS_FRAMES_PER_BLOCK );

        /* Write input and output as a stereo pair. */
        for( is=0; is<SS_FRAMES_PER_BLOCK; is++ )
        {
            sample = (short) (input[is] >> (31-15));
            Audio_WAV_WriteShorts( writer, &sample, 1 );

            sample = (short) (output[is] >> (31-15));
            Audio_WAV_WriteShorts( writer, &sample, 1 );
        }
    }

    Audio_WAV_CloseWriter( writer );
    Audio_WAV_DeleteWriter( writer );

    printf("Test complete.\n");

    return 0;
}
#endif
