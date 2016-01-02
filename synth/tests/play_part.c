/* $Id: play_part.c,v 1.6 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Play part of a MIDI File whose name is passed on the command line.
 * Test whether control values and program changes are correct
 * even if we start in the middle of a song.
 * Use the SPMIDI Synthesizer.
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

#define START_FRAME        (200000)
#define END_FRAME          (500000)
#define SAMPLE_RATE         (44100)
#define SAMPLES_PER_FRAME   (2)


/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, const char *fileName )
{
    int result;
    int timeout;
#if OPTIMIZE_VOLUME

    int maxAmplitude;
    int testVolume = 0x020;
    int masterVolume;
#endif

    MIDIFilePlayer *player;
    SPMIDI_Context *spmidiContext = NULL;
    long lastFrameTime = 0;

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
        goto error;

    /*
     * Initialize SPMIDI Synthesizer.
     * Output to audio or a file.
     */
    result = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;


    MIDIFilePlayer_GoToFrame( player, spmidiContext, START_FRAME );
    lastFrameTime = START_FRAME;

    /*
     * Play until we hit the end of all tracks.
     * Tell the MIDI player to move forward on all tracks
     * by a time equivalent to a buffers worth of frames.
     * Generate one buffers worth of data and write it to the output stream.
     */
    while( SPMUtil_PlayFileBuffer( player, spmidiContext ) == 0 )
    {
        long frameTime = MIDIFilePlayer_GetFrameTime( player );
        if( (frameTime - lastFrameTime) >= 10000)
        {
            printf("frames = %8d\n", frameTime );
            fflush( stdout );
            lastFrameTime = frameTime;
        }
        if( frameTime > END_FRAME )
            break;
    }

    timeout = (SAMPLE_RATE * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
    SPMUtil_Reset(spmidiContext);
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
    char *outputFileName = NULL;
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

    result = MIDIFile_Play( data, fileSize, outputFileName );
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

