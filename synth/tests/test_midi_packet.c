/* $Id: test_midi_packet.c,v 1.3 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Play a MIDI File by buffering MIDI bytes in packets then playing the buffer.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/examples/midifile_names.h"

#define SAMPLE_RATE         (44100)
#define SAMPLES_PER_FRAME   (2)

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
static short sSampleBuffer[SAMPLES_PER_BUFFER];

#define MAX_MIDI_BYTES (512)
static unsigned char sMIDIBuffer[ MAX_MIDI_BYTES ];
static int sNumMIDI = 0;

/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyTextCallback( int trackIndex, int metaEventType,
                     const char *addr, int numChars, void *userData )
{
    int i;
    (void) userData; /* Prevent compiler warnings about unused data. */
    printf("MetaEvent type %d on track %d: ", metaEventType, trackIndex );
    for( i=0; i<numChars; i++ )
        printf("%c", addr[i] );
    printf("\n");
}


/****************************************************************
 * Write all the bytes stored in the buffer to the SPMIDI synth.
 */
void FlushMIDIBuffer( void *midiContext )
{
    SPMIDI_Write( midiContext, sMIDIBuffer, sNumMIDI );
    sNumMIDI = 0;
}

/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyWriteCallback( void *midiContext, int byte )
{

    if( sNumMIDI >= MAX_MIDI_BYTES )
    {
        FlushMIDIBuffer( midiContext );
    }

    sMIDIBuffer[ sNumMIDI ] = (unsigned char) byte;
    sNumMIDI += 1;
}

/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyResetCallback( void *midiContext )
{
    SPMUtil_Reset( midiContext );
}

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, const char *fileName )
{
    int result;
    int msec;
    int seconds;
    int rem_msec;
    int timeout;
    int go = 1;
    MIDIFilePlayer *player;
    SPMIDI_Context *spmidiContext = NULL;

    SPMIDI_Initialize();

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
        goto error;

    msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
    seconds = msec / 1000;
    rem_msec = msec - (seconds * 1000);
    printf("Duration = %d.%03d seconds\n", seconds, rem_msec );

    /*
     * Set function to be called when a text MetaEvent is
     * encountered while playing. Includes Lyric events.
     */
    MIDIFilePlayer_SetTextCallback( player, MyTextCallback, NULL );


    MIDIFilePlayer_SetSynthCallbacks( player, MyWriteCallback, MyResetCallback );

    /*
     * Initialize SPMIDI Synthesizer.
     * Output to audio or a file.
     */
    
    /* Start synthesis engine with default number of voices. */
    result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
        goto error;

    result = SPMUtil_StartVirtualAudio( SAMPLE_RATE, fileName, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;

    fflush(stdout); /* Needed for Mac OS X */

    /*
     * Play until we hit the end of all tracks.
     * Tell the MIDI player to move forward on all tracks
     * by a time equivalent to a buffers worth of frames.
     * Generate one buffers worth of data and write it to the output stream.
     */
    while( go )
    {
    
        result = MIDIFilePlayer_PlayFrames( player, spmidiContext, SPMIDI_GetFramesPerBuffer()  );
        if( result < 0 )
        {
            goto error;
        }
        else if( result > 0 )
        {
            go = 0;
        }
        
        FlushMIDIBuffer( spmidiContext );

            /* Synthesize samples and fill buffer. */
        SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_GetFramesPerBuffer(), SAMPLES_PER_FRAME, 16 );

        SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, SPMIDI_GetFramesPerBuffer() );
    }

    /* Wait for sound to die out. */
    timeout = (SAMPLE_RATE * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_GetFramesPerBuffer(), SAMPLES_PER_FRAME, 16 );

        SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, SPMIDI_GetFramesPerBuffer() );
    }

    SPMUtil_StopVirtualAudio();

    SPMIDI_DeleteContext(spmidiContext);

    MIDIFilePlayer_Delete( player );

error:
    SPMIDI_Terminate();
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

