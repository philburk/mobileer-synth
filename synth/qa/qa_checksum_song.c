/* $Id: qa_checksum_song.c,v 1.4 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Generate checksum of a song a file using the SongPlayer API.
 * This can automatically detect and play either SMF or XMF files.
 *
 * In order for song_player support to be compiled,
 * The compiler flag SPMIDI_ME3000 must be defined as (1).
 *
 * Author: Phil Burk, Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/song_player.h"
#include "spmidi/qa/qa_tools.h"

#include "spmidi/examples/midifile_names.h"
#ifndef DEFAULT_FILENAME
#define DEFAULT_FILENAME "../data/FurryLisa_rt.mid"
#endif

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)

#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)

static spmUInt32 sCheckSum = 0;

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static void PlayAudioBuffer(SPMIDI_Context *spmidiContext)
{
    int i;
    
    /* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
    short samples[SAMPLES_PER_BUFFER];

    /* Generate a buffer full of audio data as 16 bit samples. */
    SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
                       SAMPLES_PER_FRAME, 16 );

    /* Generate CheckSum for audio data. */
    for( i=0; i<SAMPLES_PER_BUFFER; i++ )
    {
        int shifter = i & 7;
        sCheckSum += (samples[i] << shifter);
    }
    
}


/****************************************************************/
/**
 * Play a song one audio buffer at a time.
 */
int SongPlayer_Play( SongPlayer *songPlayer, SPMIDI_Context *spmidiContext )
{
    int result;
    int timeout;
    int go = 1;

    /* Start the songplayer */
    result = SongPlayer_Start( songPlayer );
    if( result < 0 )
    {
        goto error;
    }
    sCheckSum = 0;
    
    /*
     * Process one buffer worth of MIDI data each time through the loop.
     */
    while ( go )
    {
        if( SongPlayer_PlayFrames( songPlayer, FRAMES_PER_BUFFER ) == 0 )
        {
            PlayAudioBuffer(spmidiContext);
        }
        else
        {
            go = 0;
        }   
    }

    /*
    * Continue playing until all of the notes have finished sounding,
    * or for one second, whichever is shorter.
    */
    timeout = SPMIDI_GetSampleRate( spmidiContext ) / FRAMES_PER_BUFFER;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        PlayAudioBuffer(spmidiContext);
    }

    /* Stop playing */
    result = SongPlayer_Stop( songPlayer );

error:
    return result;
}


/****************************************************************/
static void usage( void )
{
    printf("qa_checksum_song [-rSampleRate] [-cExpectedSum] fileName\n");
    fflush(stdout);
}

#if 1
/*******************************************************************/
int main(int argc, char **argv);
int main(int argc, char **argv)
{
    int             result;
    SPMIDI_Context *spmidiContext = NULL;
    SongPlayer     *songPlayerContext = NULL;
    unsigned char  *fileStart;
    int             fileSize;
    int             i;
    int             sampleRate = 22050;
    spmUInt32       expectedSum = 0;

    char *fileName = DEFAULT_FILENAME;

    /* Parse command line. */
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];
        if( s[0] == '-' )
        {
            switch( s[1] )
            {
            case 'c':
                expectedSum = strtoul( &s[2], NULL, 10 );
                break;

            case 'r':
                sampleRate = atoi( &s[2] );
                break;

            default:
                usage();
                break;
            }
        }
        else
        {
            fileName = argv[i];
        }
    }
    
    QA_Init( fileName );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( fileName, &( fileSize ) );
    if( fileStart == NULL )
    {
        printf("ERROR: file %s not found.\n", fileName );
        return 1;
    }

    SPMIDI_Initialize();

    /* Start synthesis engine with default number of voices. */
    result = SPMIDI_CreateContext(  &spmidiContext, sampleRate );
    if( result < 0 )
    {
        goto error;
    }

    /* Create a player for the song */
    result = SongPlayer_Create( &songPlayerContext, spmidiContext, fileStart, fileSize );
    if( result < 0 )
    {
        goto error;
    }

    /* Play the file */
    result = SongPlayer_Play( songPlayerContext, spmidiContext );
    if( result < 0 )
    {
        goto error;
    }

error:
    /* Clean everything up */
    if( songPlayerContext != NULL )
        SongPlayer_Delete( songPlayerContext );
        
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext(spmidiContext);

    SPMIDI_Terminate();

    SPMUtil_FreeFileImage( fileStart );

    printf("Measured checkSum = %lu\n", sCheckSum );

    QA_Assert( (sCheckSum == expectedSum), "checksum matching" );

    QA_Assert( (result == 0), "checksum error return code" );

    return QA_Term( 2 );
}

#endif
