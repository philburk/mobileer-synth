/* $Id: qa_song_leak_1.c,v 1.8 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * @file qa_song_leak_1.c
 * @brief Play a song and check for memory leaks.
 * @author Phil Burk, Robert Marsanyi Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 * Play the song more than once to make sure the leaks are
 * not just one-time allocations by SPMIDI.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // for memset
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/song_player.h"
#include "spmidi/examples/midifile_names.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)


static int sNumErrors = 0;
static int sNumSuccesses = 0;

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static void PlayAudioBuffer(SPMIDI_Context *spmidiContext)
{
    /* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
    short samples[SAMPLES_PER_BUFFER];

    /* Generate a buffer full of audio data as 16 bit samples. */
    SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
                       SAMPLES_PER_FRAME, 16 );
}


/****************************************************************/
/**
 * Play a song one audio buffer at a time.
 */
int SongPlayer_Play( SongPlayer *songPlayer, SPMIDI_Context *spmidiContext, int numLoops )
{
    int result;
    int timeout;
    int go = 1;

    /* Start the songplayer */
    result = SongPlayer_Start( songPlayer );
    if( result < 0 )
        goto error;

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
            if( --numLoops <= 0 )
            {
                go = 0;
            }
            else
            {
                /* Rewind song. */
                SPMUtil_Reset( spmidiContext );
                result = SongPlayer_Rewind( songPlayer );
                if( result < 0 )
                    goto error;
            }
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
    printf("play_song [-nNUMREPS] fileName\n");
    fflush(stdout);
}

static int ReportMemoryUsage( void )
{
    int numAllocations;
    int numBytes;

    PRTMSG("---------------- memory\n" );
    numAllocations = SPMIDI_GetMemoryAllocationCount();
    PRTMSGNUMD("    num allocations = ", numAllocations );

    numBytes = SPMIDI_GetMemoryBytesAllocated();
    PRTMSGNUMD("    num bytes = ", numBytes );

    return numAllocations;
}

#if 1
/*******************************************************************/
/**
 * Load a song, play it multiple times (tearing down the song player
 * between iterations), check that all memory is returned.
 * @return 0 if test is successful, non-0 otherwise.
 */
int main(int argc, char **argv);
int main(int argc, char **argv)
{
    int             result = 0;
    SPMIDI_Context *spmidiContext = NULL;
    SongPlayer     *songPlayerContext = NULL;
    unsigned char  *fileStart;
    int             fileSize;
    int             numLoops = 2;
    int             i;
    int             numAllocated;
    int             previousAllocated;

    char *fileName = DEFAULT_FILENAME;

    /* Parse command line. */
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];
        if( s[0] == '-' )
        {
            switch( s[1] )
            {
            case 'n':
                numLoops = atoi( &s[2] );
                break;
            case 'h':
            case '?':
                usage();
                return 0;
            }
        }
        else
        {
            fileName = argv[i];
        }
    }

    PRTMSG( "Test for memory leak when playing: " );
    PRTMSG( fileName );
    PRTMSG( "\n" );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( fileName, &( fileSize ) );
    if( fileStart == NULL )
    {
        printf("ERROR: file %s not found.\n", fileName );
        return 1;
    }

    SPMIDI_Initialize();

    previousAllocated = numAllocated = ReportMemoryUsage();

    for( i=0; i<numLoops; i++ )
    {
        /* Start synthesis engine with default number of voices. */
        result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
        if( result < 0 )
            goto error;

        /* Create a player for the song */
        result = SongPlayer_Create( &songPlayerContext, spmidiContext, fileStart, fileSize );
        if( result < 0 )
            goto error;

        /* Play the file */
        result = SongPlayer_Play( songPlayerContext, spmidiContext, numLoops );
        if( result < 0 )
            goto error;
        ReportMemoryUsage();

        PRTMSG("File playback complete.\n");

error:
        SongPlayer_Delete( songPlayerContext );

        /* Clean everything up */
        SPMIDI_DeleteContext(spmidiContext);

        previousAllocated = numAllocated;
        numAllocated = ReportMemoryUsage();
    }

    if( numAllocated != previousAllocated )
    {
        PRTMSGNUMD("ERROR - memory leak, numAllocations    = ", numAllocated );
        PRTMSGNUMD("                     previousAllocated = ", previousAllocated );
        sNumErrors++;
    }

    SPMIDI_Terminate();

    SPMUtil_FreeFileImage( fileStart );

    printf("\nqa_song_leak: numSuccesses = %d, numErrors = %d <<<<<<<<<<<<<<<<<<<<<-!!\n",
           sNumSuccesses, sNumErrors );

    if( result < 0 )
    {
        PRTMSG("Error playing file = ");
        PRTNUMD( result );
        PRTMSG("\n");
    }

    return (result < 0);
}

#endif
