/* $Id: analyze_ringtone.c,v 1.3 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Analyse voice profile and memory usage of a ringtone.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/include/song_player.h"

#include "spmidi/examples/midifile_names.h"

#define PRINT(x)   { printf x; }

#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)
#define PERIOD_MSEC         (100)
#define NUMFRAMES           (SPMIDI_GetFramesPerBuffer())
#define MAX_SILENCE         (2)

int sChannelMaxCounts[MIDI_NUM_CHANNELS] = { 0 };
int sMaxVoices = 0;
int sMaxVoicesInPeriod = 0;
int sNumMeasurements = 0;
int sSumVoiceMeasurements = 0;

/****************************************************************
 * Measure voice counts per channel and keep stats.
 */
int MeasureVoiceCounts( SPMIDI_Context *spmidiContext )
{
    int i;
    int sum = 0;
    int count;

    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        count = SPMIDI_GetChannelActiveNoteCount( spmidiContext, i );
        if( count > sChannelMaxCounts[i] )
        {
            sChannelMaxCounts[i] = count;
        }
        sum += count;
    }

    if( sum > sMaxVoices ) sMaxVoices = sum;
    if( sum > sMaxVoicesInPeriod ) sMaxVoicesInPeriod = sum;

    sNumMeasurements += 1;
    sSumVoiceMeasurements += sum;
    return sum;
}

/****************************************************************/
int MeasureMaxAmplitude( SPMIDI_Context *spmidiContext )
{
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
    short samples[SAMPLES_PER_BUFFER];
    int i;
    int maxAmplitude = 0;
    int minAmplitude = 0;
    int samplesPerBuffer = NUMFRAMES * SAMPLES_PER_FRAME;

    /* Synthesize samples and fill buffer. */
    SPMIDI_ReadFrames( spmidiContext, samples, NUMFRAMES, SAMPLES_PER_FRAME, 16 );
    for( i=0; i<samplesPerBuffer; i++ )
    {
        int sample = samples[i];
        if( sample > maxAmplitude )
        {
            maxAmplitude = sample;
        }
        else if( sample < minAmplitude )
        {
            minAmplitude = sample;
        }
    }

    if( -minAmplitude > maxAmplitude )
        maxAmplitude = -minAmplitude;

    return maxAmplitude;
}

/****************************************************************
 * Estimate a MIDIFIle amplitude and print voice counts in CSV format.
 */
int MIDIFile_Estimate( unsigned char *image, int numBytes, int showProfile )
{
    int result;
    int maxAmplitude = 0;
    SongPlayer     *songPlayer = NULL;
    SPMIDI_Context *spmidiContext;
    int nextFrame;
    int framesPerPeriod;
    int frames;
    int amp;
    int count;
    int i;
    int lastFrameWithVoices = 0;


    result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
        goto error1;

    /* Turn off compressor for more meaningful amplitude study. */
    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( result < 0 )
        goto error2;

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = SongPlayer_Create( &songPlayer, spmidiContext, image, numBytes );
    if( result < 0 )
        goto error2;

    /* Start the songplayer */
    result = SongPlayer_Start( songPlayer );
    if( result < 0 )
    {
        goto error3;
    }

    framesPerPeriod = PERIOD_MSEC * SAMPLE_RATE / 1000;
    framesPerPeriod = ((framesPerPeriod + NUMFRAMES - 1) / NUMFRAMES) * NUMFRAMES;
    nextFrame = framesPerPeriod;

    if( showProfile )
    {

        PRINT(("-------- the following may be used as a CSV data for a spreadsheet -----------\n"));
        PRINT(("frames,msec,voices\n"));
    }
    while( (result = SongPlayer_PlayFrames( songPlayer, NUMFRAMES )) == 0 )
    {
        /* Synthesize and measure amplitude. */
        amp = MeasureMaxAmplitude( spmidiContext );
        if( amp > maxAmplitude )
        {
            maxAmplitude = amp;
        }

        count = MeasureVoiceCounts( spmidiContext );
        frames = MIDIFilePlayer_GetFrameTime( SongPlayer_GetMIDIFilePlayer( songPlayer ) );

        if( count > 0 )
        {
            lastFrameWithVoices = frames;
        }
        else
        {

            int silent = (frames - lastFrameWithVoices) / SAMPLE_RATE;
            if( silent >= MAX_SILENCE )
            {
                PRINT(( "Stopping because silent for %d seconds!\n", MAX_SILENCE ));
                break;
            }
        }
        if( frames >= nextFrame )
        {
            int time = (frames * 1000) / SAMPLE_RATE;
            nextFrame += framesPerPeriod;
            if( showProfile )
            {
                PRINT(("%d,%d,%d\n", frames, time, sMaxVoicesInPeriod ));
            }
            sMaxVoicesInPeriod = 0;
        }
    }
    if( showProfile )
    {
        PRINT(("------------ end CSV data -----------\n"));
    }

    PRINT(("Maximum amplitude without compressor = %d\n", maxAmplitude ));
    PRINT(("Peak Voices Per Channel\n" ));
    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        if( sChannelMaxCounts[i] > 0 )
        {
            PRINT(("%d : %d\n", i+1, sChannelMaxCounts[i] ));
        }
    }

    PRINT(("Peak number of voices = %d\n", sMaxVoices ));
    PRINT(("Average number of voices = %5.2f\n", ((double)sSumVoiceMeasurements / sNumMeasurements) ));
    {
        int numAllocs = SPMIDI_GetMemoryAllocationCount();
        int numBytes = SPMIDI_GetMemoryBytesAllocated();
        PRINT(("Number of memory allocations = %d\n", numAllocs ));
        PRINT(("Number of bytes allocated = %d\n", numBytes ));

        PRINT(("Total bytes allocated (depending on allocation overhead) = %d\n",
            ((numAllocs*16) + numBytes) ));
    }

    result = SongPlayer_Stop( songPlayer );

error3:
    SongPlayer_Delete( songPlayer );

error2:
    SPMIDI_DeleteContext(spmidiContext);

error1:
    return result;
}


/****************************************************************/
int analyse_ringtone( const char *inputFileName, int showProfile )
{
    void *data = NULL;
    int  fileSize;
    int   result = -1;


    /* Load MIDI File into a memory image. */
    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        PRINT(("ERROR reading MIDI File.\n" ));
        goto error;
    }
    result = MIDIFile_Estimate( data, fileSize, showProfile );

    if( result < 0 )
    {
        PRINT(("Error playing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) ));
        goto error;
    }

error:
    if( data != NULL )
        SPMUtil_FreeFileImage( data );

    return result;
}

/****************************************************************/
void usage( void )
{
    PRINT(("analyse_ringtone {filename} ...\n"));
    PRINT(("(C) 2006 Mobileer, Inc\n"));
    PRINT(("CONFIDENTIAL and PROPRIETARY\n"));
}

/****************************************************************/
int main( int argc, char ** argv )
{
    char *inputFileName = DEFAULT_FILENAME;
    int result;
    int showProfile = 1;
    (void) argc;
    (void) argv;

    usage();

    if( argc > 1 ) inputFileName = argv[1];

    SPMIDI_Initialize();

    PRINT(("Analyzing file: %s\n", inputFileName ));
    result = analyse_ringtone( inputFileName, showProfile );

    SPMIDI_Terminate();
    return result;
}

