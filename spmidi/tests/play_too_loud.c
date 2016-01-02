/* $Id: play_too_loud.c,v 1.7 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Play a MIDI File whose name is passed on the command line.
 * Use the SPMIDI Synthesizer.
 * Amplify the song to a level at which it distorts.
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
#define SAMPLES_PER_FRAME   (2)
#define PERCENT_GAIN        (80)
#define USE_ESTIMATE        (0)

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, const char *fileName )
{
    SPMIDI_Context *spmidiContext = NULL;
    int result;
    int timeout;
    int maxAmplitude;
    int testVolume = 0x020;
    int unityVolume;
    int masterVolume;
    MIDIFilePlayer *player;

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
        goto error;

    /* Get maximum amplitude so that we can calculate
     * optimal volume.
     */

#if USE_ESTIMATE

    maxAmplitude = SPMUtil_EstimateMaxAmplitude( player, SAMPLES_PER_FRAME,
                   testVolume, (int) SAMPLE_RATE );
    printf("Use SPMUtil_EstimateMaxAmplitude\n");
#else

    maxAmplitude = SPMUtil_GetMaxAmplitude( player, SAMPLES_PER_FRAME,
                                            testVolume, (int) SAMPLE_RATE );
    printf("Use SPMUtil_GetMaxAmplitude\n");
#endif

    if( maxAmplitude < 0 )
        goto error;
    printf("At testVolume %d, maxAmplitude is %d\n", testVolume, maxAmplitude );
    MIDIFilePlayer_Rewind( player );

    /*
     * Initialize SPMIDI Synthesizer.
     * Output to audio or a file.
     */
    result = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;

    /* Calculate masterVolume that may clip. */
    unityVolume = (testVolume * 32767) / maxAmplitude;
    printf("unityVolume = %d\n", unityVolume );
    masterVolume = (PERCENT_GAIN * unityVolume) / 100;
    printf("masterVolume = %d\n", masterVolume );
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
    //    char *outputFileName = NULL;
    char *outputFileName = "FurElise_SteelDrum_80.wav";
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

