/* $Id: test_clipping.c,v 1.5 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Play a MIDI File whose name is passed on the command line.
 * Play it at a level that will cause it to clip.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/midifile_player.h"

#include "spmidi/examples/midifile_names.h"

#define SAMPLE_RATE         (44100)
#define SAMPLES_PER_FRAME   (1)

#define OUTPUT_GAIN         (2.0)
//#define OUTPUT_FILENAME     "D:\\temp\\TangoHorn_Soft20X.wav"
#define OUTPUT_FILENAME     NULL

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, const char *outputFileName, double targetAmplitude )
{
    SPMIDI_Context *spmidiContext = NULL;
    int result;
    int timeout;
    int maxAmplitude;
    int testVolume = 0x020;
    int masterVolume;
    MIDIFilePlayer *player;

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
        goto error;

    /* Measure maximum amplitude so that we can calculate
     * optimal volume.
     */
    SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    SPMIDI_SetMasterVolume( spmidiContext, testVolume );

    printf("Measuring max amplitude.\n");
    maxAmplitude = SPMUtil_MeasureMaxAmplitude( player, spmidiContext, SAMPLES_PER_FRAME );
    if( maxAmplitude < 0 )
        goto error;
    printf("At masterVolume %d, maxAmplitude is %d\n", testVolume, maxAmplitude );
    SPMIDI_DeleteContext(spmidiContext);
    MIDIFilePlayer_Rewind( player );

    /*
     * Initialize SPMIDI Synthesizer.
     * Output to audio or a file.
     */
    result = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, outputFileName, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;

    /* Calculate masterVolume that will clip. */
    masterVolume = (int) ((targetAmplitude * testVolume * 32767) / maxAmplitude);
    printf("New masterVolume = %d\n", masterVolume );
    SPMIDI_SetMasterVolume( spmidiContext, masterVolume );

    fflush(stdout); /* Needed for Mac OS X */

    /*
     * Play until we hit the end of all tracks.
     * Tell the MIDI player to move forward on all tracks
     * by a time equivalent to a buffers worth of frames.
     * Generate one buffers worth of data and write it to the output stream.
     */
    while( SPMUtil_PlayFileBuffer( player, spmidiContext ) == 0 )
        ;

    timeout = (SAMPLE_RATE * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        SPMUtil_PlayBuffers( spmidiContext, 1 );
    }

    SPMUtil_Stop(spmidiContext);

    MIDIFilePlayer_Delete( player );

error:
    return result;
}


/****************************************************************/
int main( int argc, char ** argv )
{
    int   i;
    void *data = NULL;
    int  fileSize;
    int   result = -1;
    char *outputFileName = OUTPUT_FILENAME;
    char *inputFileName = DEFAULT_FILENAME;

    /* Parse command line. */
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];
        if( s[0] == '-' )
        {
            switch( s[1] )
            {
            case 'w':
                outputFileName = &s[2];
                break;
            }
        }
        else
        {
            inputFileName = argv[i];
        }
    }

    /* Load MIDI File into a memory image. */
    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    printf("File: %s\n", inputFileName );

    result = MIDIFile_Play( data, fileSize, outputFileName, OUTPUT_GAIN );
    if( result < 0 )
    {
        printf("Error playing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) );
        goto error;
    }

error:
    if( data != NULL )
        SPMUtil_FreeFileImage( data );
    return result;
}

