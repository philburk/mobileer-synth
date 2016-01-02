/* $Id: convert_mid_smid.c,v 1.2 2007/10/02 16:24:51 philjmsl Exp $ */
/**
 *
 * Convert an SMF file to a SMID file.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/include/midistream_player.h"

#define FRAMES_PER_SECOND   (22050)
#define SAMPLES_PER_FRAME   (1)

//#define DEFAULT_FILENAME    "FurElise_rt.mid"
#define DEFAULT_FILENAME    "UpAndDown_rt.mid"
//#define DEFAULT_FILENAME    "TwoTrills.mid"

#define FRAMES_PER_TICK     (SPMIDI_GetFramesPerBuffer())

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
static short sSampleBuffer[SAMPLES_PER_BUFFER];

static FILE *sMidiStreamFile = NULL;
#define MAX_MIDI_BYTES (256)
static unsigned char sMIDIBuffer[ MAX_MIDI_BYTES ];

static int sNumMIDI = 0;
static int sLastTickWritten = 0;
static int sCurrentTick = 0;
static int sLastCommand = 0;

/****************************************************************
 * Write MIDIStream Header to file.
 */
int MIDIStream_WriteHeader( int framesPerTick, int framesPerSecond )
{
    int numWritten;
    unsigned char pad[MIDISTREAM_HEADER_SIZE];

    if( (framesPerTick > 0x0000FFFF) ||
        (framesPerSecond > 0x0000FFFF) )
    {
        return SPMIDI_Error_OutOfRange;
    }

    pad[0] = (unsigned char) 'S';
    pad[1] = (unsigned char) 'M';
    pad[2] = (unsigned char) 'I';
    pad[3] = (unsigned char) 'D';

    pad[4] = (unsigned char) (framesPerTick >> 8);
    pad[5] = (unsigned char) (framesPerTick & 0x00FF);

    pad[6] = (unsigned char) (framesPerSecond >> 8);
    pad[7] = (unsigned char) (framesPerSecond & 0x00FF);

    numWritten = fwrite( pad, 1, sizeof(pad), sMidiStreamFile );

    return (numWritten != sizeof(pad)) ? -1 : 0;
}

/****************************************************************
 * Write all the bytes stored in the buffer to the SPMIDI synth.
 */
void FlushMIDIBuffer( void *midiContext )
{
    int numWritten;
    unsigned char pad[2];

    int delay = sCurrentTick - sLastTickWritten;

    (void) midiContext;

    if( (delay >= 255) || (sNumMIDI > 0) )
    {
//      printf("delay = %d, sNumMIDI = %d\n", delay, sNumMIDI );

        pad[0] = (unsigned char) delay;
        pad[1] = (unsigned char) sNumMIDI;
        numWritten = fwrite( pad, 1, 2, sMidiStreamFile );

        if(sNumMIDI > 0)
        {
            numWritten = fwrite( sMIDIBuffer, 1, sNumMIDI, sMidiStreamFile );
        }
        sLastTickWritten = sCurrentTick;
        sNumMIDI = 0;
    }

}

/****************************************************************
 * Write byte to MIDI stream.
 */
void MyWriteCallback( void *midiContext, int byte )
{
    if( sNumMIDI >= MAX_MIDI_BYTES )
    {
        FlushMIDIBuffer( midiContext );
    }

    /* Is it a command byte? If so then maybe use running status. */
    if( byte >= 0x80 )
    {
        if( byte == sLastCommand ) 
        {
            /* Do not include command byte . Use running status. */
            return;
        }
        else
        {
            sLastCommand = byte;
        }
    }

    sMIDIBuffer[ sNumMIDI ] = (unsigned char) byte;
    sNumMIDI += 1;
}

/****************************************************************
 */
void MyResetCallback( void *midiContext )
{
    (void) midiContext;
}

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_ConvertToStream( unsigned char *image, int numBytes, const char *fileName )
{
    int result = 0;
    int go = 1;
    MIDIFilePlayer *player;
    int smidLength = 0;

    /* Reset variables for each song. */
    sNumMIDI = 0;
    sLastTickWritten = 0;
    sCurrentTick = 0;
    sLastCommand = 0;

    sMidiStreamFile = fopen( fileName, "wb" );
    if( sMidiStreamFile == NULL )
    {
        printf("ERROR: could not open %s for output\n", fileName );
        goto error;
    }

    MIDIStream_WriteHeader( FRAMES_PER_TICK, FRAMES_PER_SECOND );

#if SPMIDI_SUPPORT_MALLOC
    /* If we are using memory allocation,
     * then we need to allocate the memory allocator. */
    SPMIDI_HostInit();
#endif

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) FRAMES_PER_SECOND, image, numBytes );
    if( result < 0 )
        goto error;

    /*
     * Set function to be called when a  MIDI Event is
     * encountered while playing.
     */
    MIDIFilePlayer_SetSynthCallbacks( player, MyWriteCallback, MyResetCallback );

    fflush(stdout); /* Needed for Mac OS X */

    /*
     * Play until we hit the end of all tracks.
     * Tell the MIDI player to move forward on all tracks
     * by a time equivalent to a buffers worth of frames.
     */
    while( go )
    {
    
        result = MIDIFilePlayer_PlayFrames( player, NULL, FRAMES_PER_TICK  );
        if( result < 0 )
        {
            goto error;
        }
        else if( result > 0 )
        {
            go = 0;
        }
        
        FlushMIDIBuffer( NULL );
    
        sCurrentTick += 1;

    }

    MIDIFilePlayer_Delete( player );

error:
#if SPMIDI_SUPPORT_MALLOC
    SPMIDI_HostTerm();
#endif

    if( sMidiStreamFile != NULL )
    {
        smidLength = ftell( sMidiStreamFile );
        fclose( sMidiStreamFile );
        sMidiStreamFile = NULL;
    }

    printf(", %d, %d\n", numBytes, smidLength );

    return result;
}


/****************************************************************/
int convertSMF( const char *inputFileName )
{
    int   result = -1;
    int   len;
    int   outlen;
    char *outputFileName = NULL;
    unsigned char *image;
    int  fileSize;


    /* Load MIDI File into a memory image. */
    image = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( image == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    printf("%s", inputFileName );
    
    len = strlen( inputFileName );
    outlen = len + 1; // to allow room for extra letter, mid -> smid
    outputFileName = malloc( outlen + 1 ); // to allow room for NUL
    if( outputFileName == NULL )
        return -1;

    /* Build output file name. */
    strcpy( outputFileName, inputFileName );
    outputFileName[ outlen - 4 ] = 's';
    outputFileName[ outlen - 3 ] = 'm';
    outputFileName[ outlen - 2 ] = 'i';
    outputFileName[ outlen - 1 ] = 'd';
    outputFileName[ outlen - 0 ] = 0;

    result = MIDIFile_ConvertToStream( image, fileSize, outputFileName );
    if( result < 0 )
    {
        printf("Error playing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) );
        goto error;
    }

error:
    if( image != NULL )
        SPMUtil_FreeFileImage( image );

    if( outputFileName != NULL )
        free( outputFileName );

    return result;
}

/****************************************************************/
int main( int argc, char ** argv )
{
    int i;
    int result = 0;

    /* Parse command line options. */
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
    }

    
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];
        if( s[0] != '-' )
        {
            result = convertSMF( s );
            if( result < 0 )
            {
                goto error;
            }
        }
    }

    if( argc == 1 )
    {
        convertSMF( DEFAULT_FILENAME );
    }

    return 0;

error:
    return 1;


}

