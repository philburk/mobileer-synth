/* $Id: qa_common.c,v 1.12 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_common.c
 * @brief Utility routines for QA.
 *
 * @author Robert Marsanyi, Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_play.h"
#include "qa_common.h"

/************************************************************/
void TestAssert( int c, const char *testname, const char *msg, int *failed )
{
    if( !c )
    {
        printf( "FAILED: %s: %s\n", testname, msg );
        *failed = -1;  /* set the return value for main() */
    }
    /*
    else
        {
            printf(" SUCCESS: %s: %s\n", testname, msg );
        }
    */
}

/************************************************************/
void InitMemoryCheck( MemoryCheck_t *memCheck, int *success )
{
    TestAssert( ( memCheck->numAllocs = SPMIDI_GetMemoryAllocationCount() ) >= 0, "InitMemoryCheck", "Memory allocation counter enabled.", success );
    TestAssert( ( memCheck->numBytes = SPMIDI_GetMemoryBytesAllocated() ) >= 0, "InitMemoryCheck", "Memory allocation counter enabled.", success );
}

/************************************************************/
void TestMemory( MemoryCheck_t *memCheck, const char *testname, int *success )
{
    TestAssert( memCheck->numAllocs == SPMIDI_GetMemoryAllocationCount(), testname, "All allocs are deallocated.", success );
    TestAssert( memCheck->numBytes == SPMIDI_GetMemoryBytesAllocated(), testname, "All bytes are deallocated.", success );
}

/************************************************************/
int ChooseRandom( int max )
{
    return( (int)( (float)rand() / (float)RAND_MAX * (float)max ) );
}

/*******************************************************************
 * Measure the frequency of a voice by starting it, letting it settle
 * and then doing a zero-crossing detection.
 */
double MeasureFrequency( SPMIDI_Context *spmidiContext, int note )
{
    int    i;
    int    numCrossings;
    int    firstCrossingIndex = -1;
    int    lastCrossingIndex = -1;
    int    lastValue;
    int    thisValue;
    double frequency = 0.0;
    short samples[ DETECT_PITCH_FRAMES ];

    SPMUtil_NoteOn( spmidiContext, 0, note, 64 );

    /* Throw some away until we are past attack. */
    SPMIDI_ReadFrames( spmidiContext, samples, DETECT_PITCH_FRAMES/4, SPMUTIL_OUTPUT_MONO, 8 );

    SPMIDI_ReadFrames( spmidiContext, samples, DETECT_PITCH_FRAMES, SPMUTIL_OUTPUT_MONO, 8 );

    SPMUtil_NoteOff( spmidiContext, 0, note, 64 );

    /* Look for zero crossings. */
    numCrossings = 0;
    lastValue = samples[0];
    for( i=1; i<DETECT_PITCH_FRAMES; i++ )
    {
        thisValue = samples[i];
        /* Got a crossing. */
        if( (lastValue < 0) && (thisValue >= 0 ) )
        {
            if( numCrossings == 0 )
            {
                firstCrossingIndex = i;
            }
            numCrossings += 1;
            lastCrossingIndex = i;
        }
        lastValue = thisValue;
    }

    /* Write to file for external analysis. */
#if 0
    {
        FILE *fid = fopen("recorded_pitch.raw", "wb");
        if( fid != NULL )
        {
            /* Write samples to a file. */
            fwrite( samples, sizeof(short), DETECT_PITCH_FRAMES, fid );
            fclose( fid );
        }
    }
#endif

    /* Calculate frequency based on zero crossings. */
    if( numCrossings > 2 )
    {
        frequency = (((double) QA_SAMPLE_RATE) * (numCrossings - 1)) /
                    (lastCrossingIndex - firstCrossingIndex);
    }

    return frequency;
}

/*******************************************************************
 * Measure the (almost) instantaneous amplitude of an already-sounding
 * note
 */
int MeasureAmplitude( SPMIDI_Context *spmidiContext, short *samples )
{
    int max = 0;
    int i;

    SPMIDI_ReadFrames( spmidiContext, samples, DETECT_AMP_FRAMES, SPMUTIL_OUTPUT_MONO, sizeof(short) * 8 );

    for( i = 0; i < DETECT_AMP_FRAMES; i++ )
    {
        int sample = samples[i];
        int magnitude = (sample < 0 ) ? -sample : sample;
        if( magnitude > max )
            max = magnitude;

    }
    return max;
}
