
/**
 * check_inst_amp.c
 * by Darren Gibbs
 *
 * Play all instruments and drums, measure peak amplitude and compare
 * with result from SPMIDI_EstimateMaxAmplitude().
 *
 * Plays 5 notes an octave apart centered around Middle C (60)
 * for 300 msec then lets the sound die away,
 * measure peak absolute value of amplitude,
 * gather statistics,
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define SAMPLE_RATE         (44100)

#define CHANNEL             (3)

#define DEFAULT_VELOCITY    (64)

/* Create a big sample buffer. */
#define BUFFER_SIZE         (((SAMPLE_RATE / SPMIDI_MAX_FRAMES_PER_BUFFER) + 1) * SPMIDI_MAX_FRAMES_PER_BUFFER)

/* Buffer to accumulate 5 note events for each patch */
static short samples[ BUFFER_SIZE ];

/* Peak sample values for each instrument program. */
static int inst_peaks[128];

/* Estimated max values for each instrument program. */
static int inst_estimates[128];


/*******************************************************************/
/* Read through a buffer and find absolute peak sample value. */
static int FindPeakValue(short *samps, long count)
{
    long    i;
    long   thisValue;
    long   max;

    max = abs(samps[0]);
    for( i = 1; i < count; i++ )
    {
        thisValue = abs(samps[i]);

        if( thisValue > max )
        {
            max = thisValue;
        }
    }

    return max;
}


/*******************************************************************/
int main(void);
int main(void)
{
    int  err;
    int  i;
    long framesRead;
    int  numTooLow, numTooHigh;
    SPMIDI_Context *spmidiContext = NULL;

    printf("SPMIDI - measure amplitude of instruments, report any out of range. SR = %d\n", SAMPLE_RATE );
    printf("SPMIDI max frames per buffer = %d\n", SPMIDI_MAX_FRAMES_PER_BUFFER );

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        return 1;

    /* Loop through all programs and store peaks for each program. */
    for( i=0; i<128; i++ )
    {
        SPMUtil_ProgramChange( spmidiContext, CHANNEL, i );

        /* Play Middle C (60) */
        memset(samples, 0, BUFFER_SIZE); /* clear out buffer */

        SPMUtil_NoteOn( spmidiContext, CHANNEL, 60, DEFAULT_VELOCITY );

        /* read ~300ms worth of frames */
        framesRead = SPMIDI_ReadFrames( spmidiContext, samples, BUFFER_SIZE, SPMUTIL_OUTPUT_MONO, sizeof(short)*8 );
        if( framesRead < 0 )
        {
            err = framesRead;
            printf("Error in SPMIDI_ReadFrames() = %d = %s\n", err,
                   SPMUtil_GetErrorText( err ) );
            goto error;
        }

        /* Search through the five note events for max amplitude and store it in array. */
        inst_peaks[i] = FindPeakValue(samples, framesRead);

        SPMUtil_NoteOff( spmidiContext, CHANNEL, 60, 0 );

        printf("Program #%3d's peak was %4d   \"%s\"\n", i,
               inst_peaks[i], MIDI_GetProgramName(i) );
        fflush(stdout);

        /* Make sure note finishes. */
        SPMIDI_ReadFrames( spmidiContext, samples, BUFFER_SIZE, SPMUTIL_OUTPUT_MONO, sizeof(short)*8 );
        SPMIDI_ReadFrames( spmidiContext, samples, BUFFER_SIZE, SPMUTIL_OUTPUT_MONO, sizeof(short)*8 );


    }

    /* Loop through all programs and store Estimates for each program. */
    for( i=0; i<128; i++ )
    {
        int maxAmplitude;

        SPMUtil_ProgramChange( spmidiContext, CHANNEL, i );

        /* Play Middle C (60) */
        memset(samples, 0, BUFFER_SIZE); /* clear out buffer */
        SPMUtil_NoteOn( spmidiContext, CHANNEL, 60, DEFAULT_VELOCITY );

        /* read ~300ms worth of frames */
        maxAmplitude = SPMIDI_EstimateMaxAmplitude( spmidiContext, BUFFER_SIZE, SPMUTIL_OUTPUT_MONO );
        if( maxAmplitude < 0 )
        {
            err = maxAmplitude;
            printf("Error in SPMIDI_EstimateMaxAmplitude() = %d = %s\n", err,
                   SPMUtil_GetErrorText( err ) );
            goto error;
        }
        inst_estimates[i] = maxAmplitude;

        SPMUtil_NoteOff( spmidiContext, CHANNEL, 60, 0 );

        printf("Program #%3d's est max was %4d   \"%s\"\n", i,
               inst_estimates[i], MIDI_GetProgramName(i) );
        fflush(stdout);

        /* read and discard ~100 ms worth of frames. Don't need release of envelope. */
        SPMIDI_EstimateMaxAmplitude( spmidiContext, BUFFER_SIZE, SPMUTIL_OUTPUT_MONO );
        SPMIDI_EstimateMaxAmplitude( spmidiContext, BUFFER_SIZE, SPMUTIL_OUTPUT_MONO );
    }

    SPMUtil_Stop(spmidiContext);


    numTooLow = 0;
    numTooHigh = 0;
    /* report out of bounds instrument Programs. */
    for (i=0; i<128; i++ )
    {
        if (inst_estimates[i] < ((inst_peaks[i] * 3) / 4))
        {
            printf("----> Instrument Program #%3d (%20s) low:   %4d < %4d LOW\n",
                   i, MIDI_GetProgramName(i), inst_estimates[i], inst_peaks[i] );
            fflush(stdout);
            numTooLow++;
        }
        if (inst_estimates[i] > ((inst_peaks[i] * 5) / 4))
        {
            printf("----> Instrument Program #%3d (%20s) high:  %4d > %4d\n",
                   i, MIDI_GetProgramName(i), inst_estimates[i], inst_peaks[i]);
            fflush(stdout);
            numTooHigh++;
        }
    }

    printf("%d too low, %d too high!\n", numTooLow, numTooHigh );
    printf("Test finished now.\n");
error:
    return err;
}


