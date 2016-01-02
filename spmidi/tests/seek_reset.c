/* $Id: seek_reset.c,v 1.4 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 * Test stuck notes when seeking in a MIDI file.
 * Play a MIDI file until long organ notes are playing.
 * Then seek to a place near the beginning.
 * The organ notes should stop.
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

#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes )
{
    int result;
    int timeout;
    MIDIFilePlayer *player;
    SPMIDI_Context *spmidiContext;

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
        goto error;

    /*
     * Initialize SPMIDI Synthesizer.
     * Output to audio or a file.
     */
    result = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;

    fflush(stdout); /* Needed for Mac OS X */

    while( MIDIFilePlayer_GetFrameTime( player ) < 138000 )
    {
        if( SPMUtil_PlayFileBuffer( player, spmidiContext ) != 0 )
            break;
    }

    /* Seek to near beginning of song. Organ notes should stop*/
    MIDIFilePlayer_GoToFrame( player, spmidiContext, 20000 );


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
    void *data = NULL;
    int  fileSize;
    int   result = -1;
    char *inputFileName = DEFAULT_FILENAME;

    /* Parse command line. */
    if( argc > 1 )
    {
        inputFileName = argv[1];
    }

    /* Load MIDI File into a memory image. */
    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    printf("File: %s\n", inputFileName );

    result = MIDIFile_Play( data, fileSize );
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

