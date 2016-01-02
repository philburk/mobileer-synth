/* $Id: qa_pitch.c,v 1.10 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_pitch_bend.c
 * @brief Measure pitch of synthesizer and pitch bend.
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 * 
 * This test uses floating point math to calculate frequencies
 * so it should be run on a host computer, eg. PC.
 * NOTE: Reverb must be turned off to get an accurate measure of pitch.
 *
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
//#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/qa/qa_tools.h"

#define SAMPLE_RATE         (44100)

#define CHANNEL             (3)
/* Pure sine. */
#define PROGRAM_SINE        (0)
/* Ringing state variable filter. */
#define PROGRAM_FILTER      (1)

#define NUM_FRAMES          (256 * 64)
#define BITS_PER_BYTE      (8)

static double sMaxError = 0.0;
static SPMIDI_Context *spmidiContext = NULL;

static const unsigned char SysExGMOff[] =
    {
        MIDI_SOX, 0x7E, 0x7F, 0x09, 0x02, MIDI_EOX
    };

static short samples[ NUM_FRAMES ];

static double PitchToFrequency( double pitch )
{
    return MIDI_FREQ_ZERO * pow( 2.0, (pitch / 12.0) );
}

static int sCurrentCoarseOffsets[MIDI_NUM_CHANNELS] = { 0 };


/*******************************************************************/
void SetTuningOffset( int channel, double semitoneOffset )
{
    int coarseOffset = (int)(semitoneOffset + 0.5);
    int coarseMSB = coarseOffset + 0x40;

    int fineOffset = (int)((semitoneOffset - coarseOffset) * 8192) + 0x2000;
    int fineLSB = fineOffset & 0x7F;
    int fineMSB = (fineOffset >> 7) & 0x7F;

    /* Point to coarse tuning RPN. */
    if( coarseOffset != sCurrentCoarseOffsets[ channel ] )
    {
        sCurrentCoarseOffsets[ channel ] = coarseOffset;

        SPMUtil_ControlChange( spmidiContext, channel, MIDI_CONTROL_RPN_MSB, 0 ); /* RPN # MSB */
        SPMUtil_ControlChange( spmidiContext, channel,
            MIDI_CONTROL_RPN_LSB, MIDI_RPN_COARSE_TUNING ); /* RPN # LSB */

        /* Set coarse tuning. */
        SPMUtil_ControlChange( spmidiContext, channel, MIDI_CONTROL_DATA_ENTRY, coarseMSB ); /* Data Entry MSB */
        /* This is actually ignored. */
        SPMUtil_ControlChange( spmidiContext, channel,
            MIDI_CONTROL_DATA_ENTRY + MIDI_CONTROL_LSB_OFFSET, 0 );
    }

    /* Point to fine tuning RPN. */
    SPMUtil_ControlChange( spmidiContext, channel, MIDI_CONTROL_RPN_MSB, 0 ); /* RPN # MSB */
    SPMUtil_ControlChange( spmidiContext, channel,
        MIDI_CONTROL_RPN_LSB, MIDI_RPN_FINE_TUNING ); /* RPN # LSB */

    /* Set fine tuning. */
    SPMUtil_ControlChange( spmidiContext, channel, MIDI_CONTROL_DATA_ENTRY, fineMSB ); /* Data Entry MSB */
    SPMUtil_ControlChange( spmidiContext, channel,
        MIDI_CONTROL_DATA_ENTRY + MIDI_CONTROL_LSB_OFFSET, fineLSB );

    /* For safety, reset RPN to NULL. */
    SPMUtil_ControlChange( spmidiContext, channel, MIDI_CONTROL_RPN_MSB, 127 );
    SPMUtil_ControlChange( spmidiContext, channel, MIDI_CONTROL_RPN_LSB, 127 );
}

/*******************************************************************/
static double MeasureFrequency( int pitch )
{
    int    i;
    int    numCrossings;
    int    firstCrossingIndex = -1;
    int    lastCrossingIndex = -1;
    int    lastValue;
    int    thisValue;
    double frequency = 0.0;

    SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, 64 );

    /* Throw some away until we are past attack. */
    SPMIDI_ReadFrames( spmidiContext, samples, NUM_FRAMES/4, SPMUTIL_OUTPUT_MONO, sizeof(short)*BITS_PER_BYTE );

    SPMIDI_ReadFrames( spmidiContext, samples, NUM_FRAMES, SPMUTIL_OUTPUT_MONO, sizeof(short)*BITS_PER_BYTE );

    SPMUtil_NoteOff( spmidiContext, CHANNEL, pitch, 64 );

    /* Look for zero crossings. */
    numCrossings = 0;
    lastValue = samples[0];
    for( i=1; i<NUM_FRAMES; i++ )
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
#if 1
    {
        FILE *fid = fopen("recorded_pitch.raw", "wb");
        if( fid != NULL )
        {
            /* Write samples to a file. */
            fwrite( samples, sizeof(short), NUM_FRAMES, fid );
            fclose( fid );
        }
    }
#endif

    /* Calculate frequency based on zero crossings. */
    if( numCrossings > 2 )
    {
        frequency = (((double) SAMPLE_RATE) * (numCrossings - 1)) /
                    (lastCrossingIndex - firstCrossingIndex);
    }

    return frequency;
}

/*******************************************************************/
static void CheckPitch( int fullBendRangeCents, double bendFraction,
                   int pitch, double tuningOffset, double maxDeviation )
{
    double error;
    double measuredFrequency;
    double bendRange = fullBendRangeCents * 0.01;
    int bendRangeSemis = fullBendRangeCents / 100;
    int bendRangeCents = fullBendRangeCents % 100;

    double targetPitch = pitch + (bendRange * bendFraction) + tuningOffset;
    double expectedFrequency = PitchToFrequency( targetPitch );

    int bend = MIDI_BEND_NONE + (int)((MIDI_BEND_NONE * bendFraction) + 0.5);
    if( bend > MIDI_BEND_MAX )
    {
        bend = MIDI_BEND_MAX;
    }

    printf("CheckPitch: ----------------------------------------\n");
    printf("CheckPitch: pitch = %d, bendRange = %f, bendFraction = %f, tuning = %f\n",
           pitch, bendRange, bendFraction, tuningOffset );
    printf("CheckPitch: targetPitch = %9.5f\n", targetPitch );

    SetTuningOffset( CHANNEL, tuningOffset );
    SPMUtil_SetBendRange( spmidiContext, CHANNEL, bendRangeSemis, bendRangeCents );
    SPMUtil_PitchBend( spmidiContext, CHANNEL, bend );
    measuredFrequency = MeasureFrequency( pitch );
    error = fabs( (measuredFrequency - expectedFrequency) / expectedFrequency );

    if( error > sMaxError )
    {
        sMaxError = error;
    }

    printf( "expectedFrequency = %g, measuredFrequency = %g\n",
            expectedFrequency, measuredFrequency );

    QA_Assert( (error < maxDeviation), " frequency out of range" );
}

/*******************************************************************/
static void CheckSeveral( double maxDeviation )
{
    sMaxError = 0.0;

    // bendRangeCents, bendFraction, pitch, tuningOffset, maxDeviation
    CheckPitch( 200,     0, 60,  0.0, maxDeviation  );
    CheckPitch( 200,     0, 61,  0.0, maxDeviation  );

    CheckPitch( 225,     0.5, 61,  0.0, maxDeviation  );
    CheckPitch( 250,     0.5, 61,  0.0, maxDeviation  );
    CheckPitch( 275,     0.5, 60,  0.0, maxDeviation  );
    CheckPitch( 300,     0.5, 60,  0.0, maxDeviation  );

    CheckPitch( 200,     0, 60,  0.932, maxDeviation  );
    CheckPitch( 200,     0, 60, 3.256, maxDeviation  );
    CheckPitch( 200,     0, 69,  0.0, maxDeviation  );
    CheckPitch( 200,     0, 69,  0.02, maxDeviation  );
    CheckPitch( 200,     0, 69,  0.4, maxDeviation  );
    CheckPitch( 200,     0, 69,  0.5, maxDeviation  );
    CheckPitch( 200,     0, 69,  0.6, maxDeviation  );
    CheckPitch( 200,     0, 69,  2.6, maxDeviation  );
    CheckPitch( 200,     0, 69, -0.4, maxDeviation  );
    CheckPitch( 200,     0, 69, -0.5, maxDeviation  );
    CheckPitch( 200,     0, 69, -0.6, maxDeviation  );
    CheckPitch( 200,     0, 69, -2.6, maxDeviation  );
    CheckPitch( 200,     0, 82,  0.0, maxDeviation  );
    CheckPitch( 200,     0, 75,  0.0, maxDeviation  );
    CheckPitch( 200,     0, 69,  0.0, maxDeviation  );
    CheckPitch( 200,     0, 53,  0.0, maxDeviation  );
    CheckPitch( 200,     0, 42,  0.0, maxDeviation  );
    CheckPitch( 200,   0.5, 53,  0.0, maxDeviation  );
    CheckPitch( 200,  -0.5, 61,  2.7, maxDeviation  );
    CheckPitch( 539, -0.37, 42,  0.0, maxDeviation  );
    CheckPitch( 781,  0.92, 39,  0.0, maxDeviation  );
    CheckPitch( 613, -0.17, 48, -1.577, maxDeviation  );
    CheckPitch( 327,  0.52, 33,  0.0, maxDeviation  );
    CheckPitch( 419,  0.33, 80,  0.0, maxDeviation  );
    /*
    */
    printf("MAX fractional deviation = %9.6f\n", sMaxError );

}
/*******************************************************************/
/**
 * Play several pitches with bends, check that the measured frequency
 * corresponds.
 * @return 0 if all tests succeed, non-0 if not
 */
int main(void);
int main(void)
{
    int err;
    int result = 0;

    printf("SPMIDI - test frequency of generated note. SR = %d\n", SAMPLE_RATE );

    QA_Init( "qa_pitch" );

    /* Start synthesis engine with default number of voices. */
    err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( err < 0 )
    {
        QA_CountError();
        goto error;
    }

    SPMIDI_Write( spmidiContext, SysExGMOff, sizeof( SysExGMOff ) );
    SPMUtil_BankSelect( spmidiContext, CHANNEL, SPMIDI_TEST_BANK );

    printf("\nCheck sine oscillator ================================\n\n");
    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM_SINE );
    CheckSeveral( 0.01 / 12 );

    /* Tuning accuracy for filter is not as accurate. */
    //    printf("\nCheck ringing filter =================================\n\n");
    //    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM_FILTER );
    //    CheckSeveral( 0.040 );

    SPMIDI_DeleteContext(spmidiContext);

    {
        double freq0 = PitchToFrequency( 60.00 );
        double freq1 = PitchToFrequency( 60.02 );
        double error = (freq1 - freq0) / freq0 ;
        printf("CheckPitch: 2 cents corresponds to a fractional deviation of = %9.6f\n", error );
    }

error:
    result = QA_Term( 30 );

    if( result )
    {
        printf("Pitch errors could be caused by REVERB complicating the waveform!\n");
    }

    printf("Test finished.\n");
    return result;

}
