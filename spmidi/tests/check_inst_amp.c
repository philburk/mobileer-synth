
/**
 * check_inst_amp.c
 * by Darren Gibbs
 *
 * Play all instruments and drums, measure peak amplitude and report 
 * as out of range instruments.
 *
 * Plays 5 notes an octave apart centered around Middle C (60)
 * for 300 msec then lets the sound die away,
 * measure peak absolute value of amplitude,
 * gather statistics,
 * take average of all instruments,
 * report all out of range instruments that are over 2* or
 * under 1/2 of average amplitude.
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

#define SAMPLE_RATE         (22050)
#define FORMAT_CSV          (0)

#define CHANNEL             (3)

#define DEFAULT_VELOCITY    (64)

#define MSEC_PER_NOTE       (200)
#define NUM_NOTES           (5)

/* Figure out how many samples are in a 300ms event at the current sample rate -- MONO ONLY --. */
#define ONE_NOTE_OF_FRAMES   ((SAMPLE_RATE * MSEC_PER_NOTE) / 1000)

/* Round up to nearest multiple of SPMIDI_MAX_FRAMES_PER_BUFFER */
#define NUM_FRAMES_PER_NOTE   (((ONE_NOTE_OF_FRAMES / SPMIDI_MAX_FRAMES_PER_BUFFER) + 1) * SPMIDI_MAX_FRAMES_PER_BUFFER)
#define NUM_FRAMES_TO_SKIP   NUM_FRAMES_PER_NOTE

/* Buffer to accumulate enough note events for each patch plus some extra. */
static short samples[ NUM_FRAMES_PER_NOTE * (NUM_NOTES + 1) ];

/* Place to read release samples into.  i.e. throw away release portion of inst envelope */
static short trash[ NUM_FRAMES_TO_SKIP ];

/* Peak sample values for each instrument program. */
static int inst_rms[128];
static int inst_peak[128];

/* Peak sample values for each drum. */
static int drum_rms[128];
static int drum_peak[128];


/*******************************************************************/
/* Read through a buffer and find absolute peak sample value. */
static int FindPeak(short *samps, long count)
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
/* Read through a buffer and measure RMS value. */
static double FindRMS(short *samps, long count)
{
    long    i;
    double  sum;
    double  meanSquare;
    long    numUsed = 0;
    long    numZerosInARow = 0;

    sum = 0.0;
    /* Skip leading zeros. */
    for( i = 0; i < count; i++ )
    {
        if( samps[i] != 0 )
            break;
    }

    for( ; i < count; i++ )
    {
        double sample = (double) samps[i];
        // sum of square
        sum += sample * sample;
        numUsed += 1;
        /* Don't penalize short notes by including zero. */
        if( sample == 0 )
        {
            numZerosInARow += 1;
            if( numZerosInARow > 1000 )
                break;
        }
        else
        {
            numZerosInARow = 0;
        }
    }
    meanSquare = sum / numUsed;

    return sqrt( meanSquare );
}


/*******************************************************************/
void ReportLine( int index, const char *name, int rms, int peak )
{
    double percentPeak = ((100.0 * rms)/peak);

#if FORMAT_CSV
    /* Output in CVS format for spreadsheets. */
    printf("%3d,%s,%d,%d,%7.4\n",
#else
    printf("#%3d - %28s, rms = %5d, peak = %5d, r/p %7.4f\n",
#endif
           index, name, rms, peak, percentPeak );

}

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err = 0;
    int i, j, pitch;
    int instCount = 0;
    int drumCount = 0;
    int averageRMS;
    int averagePeak;
    int tooLowRMS, tooHighRMS;
    int tooLowPeak, tooHighPeak;
    long x;
    long framesRead;
    int result;

    printf("SPMIDI - measure amplitude of instruments, report any out of range. SR = %d\n", SAMPLE_RATE );
    printf("SPMIDI max frames per buffer = %d\n", SPMIDI_MAX_FRAMES_PER_BUFFER );
    printf("%d ms worth of sample frames = %d\n", MSEC_PER_NOTE, (int)ONE_NOTE_OF_FRAMES );
    printf("%d ms rounded up to multiple of SPMIDI_MAX_FRAMES_PER_BUFFER = %d\n", MSEC_PER_NOTE, (int)NUM_FRAMES_PER_NOTE );


    /* Loop through all programs and store peaks for each program. */
    for( i=0; i<128; i++ )
    {
        int totalRead = 0;


        /* Play several notes an octave apart centered around Middle C (60) */
        pitch = 36; /* start two octaves below middle C */
        memset(samples, 0, sizeof(samples) ); /* clear out buffer */

        for( j=0; j<NUM_NOTES; j++ )
        {
            err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
            if( err < 0 )
                return 1;

            /* Turn off compressor so we hear unmodified instrument sound. */
            result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
            if( result < 0 )
                goto error;

            SPMIDI_SetMasterVolume( spmidiContext, 128 * 8 );

            SPMUtil_ProgramChange( spmidiContext, CHANNEL, i );

            SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, DEFAULT_VELOCITY );

            /* read note on portion */
            framesRead = SPMIDI_ReadFrames( spmidiContext, &samples[totalRead], NUM_FRAMES_PER_NOTE,
                                            SPMUTIL_OUTPUT_MONO, sizeof(short)*8 );
            if( framesRead < 0 )
            {
                err = framesRead;
                printf("Error in SPMIDI_ReadFrames() = %d = %s\n", err,
                       SPMUtil_GetErrorText( err ) );
                goto error;
            }

            SPMUtil_Stop(spmidiContext);

            /* up an octave */
            pitch += 12;

            /* advance to next 300ms chunk of sample buffer */
            totalRead += framesRead;
        }

        /* Search through the five note events for max amplitude and store it in array. */
        inst_rms[instCount] = (int) FindRMS(samples, totalRead);
        inst_peak[instCount] = (int) FindPeak(samples, totalRead);

        ReportLine( i, MIDI_GetProgramName(i), inst_rms[instCount], inst_peak[instCount] );

        fflush(stdout);

        instCount += 1;

    }

    printf("---------------------------------------------------\n");

    /* Loop through all drum sounds and find peaks. */
    for( i=GMIDI_FIRST_DRUM; i<=GMIDI_LAST_DRUM; i++ )
    {
        err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
        if( err < 0 )
            return 1;

        /* Turn off compressor so we hear unmodified instrument sound. */
        result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
        if( result < 0 )
            goto error;

        SPMIDI_SetMasterVolume( spmidiContext, 128 * 8 );

        SPMUtil_NoteOn( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, DEFAULT_VELOCITY );

        /* read 300 ms worth of frames */
        framesRead = SPMIDI_ReadFrames( spmidiContext, samples, NUM_FRAMES_PER_NOTE, SPMUTIL_OUTPUT_MONO, sizeof(short)*8 );

        SPMUtil_NoteOff( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, 0 );

        SPMUtil_Stop(spmidiContext);

        /* Search note event for max amplitude and store it in array. */
        drum_rms[drumCount] = (int) FindRMS(samples, framesRead);
        drum_peak[drumCount] = (int) FindPeak(samples, framesRead);

        ReportLine( i, MIDI_GetDrumName(i), drum_rms[drumCount], drum_peak[drumCount] );
        fflush(stdout);

        drumCount += 1;
    }


    /* Find average of all RMSs. */
    x = 0;
    for (i=0; i<instCount; i++ )
    {
        x += inst_rms[i];
    }

    for (i=0; i<drumCount; i++ )
    {
        x += drum_rms[i];
    }

    averageRMS = (x / (drumCount + instCount));
    printf("Average RMS: %d, \n", averageRMS );
    fflush(stdout);

    tooLowRMS = averageRMS / 2;
    tooHighRMS = averageRMS + tooLowRMS;


    /* Find average of all peaks. */
    x = 0;
    for (i=0; i<instCount; i++ )
    {
        x += inst_peak[i];
    }

    for (i=0; i<drumCount; i++ )
    {
        x += drum_peak[i];
    }

    averagePeak = (x / (drumCount + instCount));
    printf("Average Peak: %d, \n", averagePeak );
    fflush(stdout);

    tooLowPeak = averagePeak / 2;
    tooHighPeak = averagePeak + tooLowPeak;

    /* report out of bounds instrument Programs. */
    for (i=0; i<instCount; i++ )
    {
        if (inst_rms[i] < tooLowRMS)
        {
            printf("#%3d (%32s) RMS too low: %4d, %4d\n", i, MIDI_GetProgramName(i), inst_rms[i], inst_peak[i]);
            fflush(stdout);
        }
        if (inst_peak[i] < tooLowPeak)
        {
            printf("#%3d (%32s) PEAK too low: %4d, %4d\n", i, MIDI_GetProgramName(i), inst_rms[i], inst_peak[i]);
            fflush(stdout);
        }
    }
    for (i=0; i<instCount; i++ )
    {
        if (inst_rms[i] > tooHighRMS)
        {
            printf("#%3d (%32s) RMS too high: %4d, %4d\n", i, MIDI_GetProgramName(i), inst_rms[i], inst_peak[i]);
            fflush(stdout);
        }
        if (inst_peak[i] > tooHighPeak)
        {
            printf("#%3d (%32s) PEAK too high: %4d, %4d\n", i, MIDI_GetProgramName(i), inst_rms[i], inst_peak[i]);
            fflush(stdout);
        }
    }

    /* report out of bounds Drums */
    for (i=0; i<drumCount; i++ )
    {
        if (drum_rms[i] < tooLowRMS)
        {
            printf("Drum Pitch #%3d (%32s) RMS too low:  %4d, %4d\n", (i + GMIDI_FIRST_DRUM),
                   MIDI_GetDrumName( i + GMIDI_FIRST_DRUM ), drum_rms[i], drum_peak[i]);
            fflush(stdout);
        }
        if (drum_peak[i] < tooLowPeak)
        {
            printf("Drum Pitch #%3d (%32s) PEAK too low:  %4d, %4d\n", (i + GMIDI_FIRST_DRUM),
                   MIDI_GetDrumName( i + GMIDI_FIRST_DRUM ), drum_rms[i], drum_peak[i]);
            fflush(stdout);
        }
    }
    for (i=0; i<drumCount; i++ )
    {
        if (drum_rms[i] > tooHighRMS)
        {
            printf("Drum Pitch #%3d (%32s) RMS too high: %4d, %4d\n", (i + GMIDI_FIRST_DRUM),
                   MIDI_GetDrumName( i + GMIDI_FIRST_DRUM ), drum_rms[i], drum_peak[i]);
            fflush(stdout);
        }
        if (drum_peak[i] > tooHighPeak)
        {
            printf("Drum Pitch #%3d (%32s) PEAK too high: %4d, %4d\n", (i + GMIDI_FIRST_DRUM),
                   MIDI_GetDrumName( i + GMIDI_FIRST_DRUM ), drum_rms[i], drum_peak[i]);
            fflush(stdout);
        }
    }

    printf("Test finished.\n");
error:
    return err;
}


