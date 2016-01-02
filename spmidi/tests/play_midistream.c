/* $Id: play_midistream.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play a MIDI Stream File whose name is passed on the command line.
 * Use the ME2000 Synthesizer.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midistream_player.h"

#define FRAMES_PER_SECOND   (22050)
#define SAMPLES_PER_FRAME   (2)
#define NUM_REPETITIONS     (2)
#define FRAMES_PER_TICK     (SPMIDI_GetFramesPerBuffer())

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
static short sSampleBuffer[SAMPLES_PER_BUFFER];

#define DEFAULT_FILENAME "FurElise_rt.smid"

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIStream_Play( unsigned char *image, int numBytes, int numRepetitions )
{
    int result;
    int timeout;
    SPMIDI_Context *spmidiContext = NULL;
    MIDIStreamPlayer smidPlayer;

    SPMIDI_Initialize();

    result = MIDIStreamPlayer_Setup( &smidPlayer, FRAMES_PER_TICK, FRAMES_PER_SECOND,
        image, numBytes );
    if( result < 0 )
    {
        goto error;
    }
    
    /* Create synthesis engine for generating audio. */
    result = SPMIDI_CreateContext( &spmidiContext, FRAMES_PER_SECOND );
    if( result < 0 )
    {
        goto error;
    }

    /* Start an audio device so we can hear the result. */
    result = SPMUtil_StartVirtualAudio( FRAMES_PER_SECOND, NULL, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;

    while( numRepetitions > 0 )
    {
        /*
         * Play through the stream.
         * Generate one buffers worth of data and write it to the output stream.
         */
        while( MIDIStreamPlayer_PlayTick( &smidPlayer, spmidiContext )  == 0 )
        {
            /* Synthesize samples and fill buffer. */
            SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, FRAMES_PER_TICK, SAMPLES_PER_FRAME, 16 );

            /* Play the buffer to the audio device. */
            SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, FRAMES_PER_TICK );
        }

        /* Rewind and replay the song if requested. */
        if( numRepetitions > 0 )
        {
            /* Reposition cursor in SMID image. */
            MIDIStreamPlayer_Rewind( &smidPlayer );

            /* Reset MIDI Controllers to initial state at beginning of song. */
            SPMUtil_Reset( spmidiContext );
            numRepetitions -= 1;
        }
    }

    /* Wait for sound to die out. */
    timeout = 5 * FRAMES_PER_SECOND / FRAMES_PER_TICK;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, FRAMES_PER_TICK, SAMPLES_PER_FRAME, 16 );

        SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, FRAMES_PER_TICK );
    }

    SPMUtil_StopVirtualAudio();

    SPMIDI_DeleteContext(spmidiContext);

error:
    SPMIDI_Terminate();
    return result;
}


/****************************************************************/
int main( int argc, char ** argv )
{
    int   i;
    int   result = -1;
    void *data = NULL;
    int   fileSize;
    char *inputFileName = DEFAULT_FILENAME;

    /* Parse command line. */
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];
        if( s[0] == '-' )
        {
            switch( s[1] )
            {
            default:
                break;
            }
        }
        else
        {
            inputFileName = argv[i];
        }
    }

    /* Load MIDI Stream File into a memory image. */
    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    printf("File: %s\n", inputFileName );

    result = MIDIStream_Play( data, fileSize, NUM_REPETITIONS );
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

