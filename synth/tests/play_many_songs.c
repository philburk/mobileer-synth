/* $Id: play_many_songs.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play several songs in a row.
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

#define PHIL_PREFIX   "E:\\nomad\\MIDISynth\\data\\"
#define ROBERT_PREFIX "C:\\business\\mobileer\\data\\"

#if defined(WIN32)
#define DATA_DIR   PHIL_PREFIX
#elif defined(MACOSX)
#define DATA_DIR  "../../../"
#else
#define DATA_DIR  PHIL_PREFIX
#endif

#define XMF_DIR        DATA_DIR"xmf\\"
#define RINGTONE_DIR   DATA_DIR"ringtones\\"

const char *songNames[] = 
{
    XMF_DIR"gibbs/AnyoneHome/AnyoneHome.mxmf",
    XMF_DIR"carumba/carumba.mxmf",
    RINGTONE_DIR"phil/songs/FurryLisa_rt.mid",
    XMF_DIR"charge_alaw/charge_alaw.mxmf",
    XMF_DIR"qa/LFOTest1.mxmf",
    RINGTONE_DIR"phil/rings/Tremolo1_Square.mid",
    XMF_DIR"TalkinReggae.mxmf",
    RINGTONE_DIR"jeanne/UrbanOre_rt.mid",
    XMF_DIR"wantya.mxmf"
};

#define NUM_SONGS (sizeof(songNames)/sizeof(char *))

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)

#define SHOW_SIZE(msg,size) {printf( msg "%d\n", (size) );}

/* Host independant writable audio device. */
static SPMIDI_AudioDevice  sHostAudioDevice;

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

    /* Write audio samples to the audio device. */
    SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, FRAMES_PER_BUFFER );
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

int PlaySongFile( const char *fileName )
{

    int             result;
    SPMIDI_Context *spmidiContext = NULL;
    SongPlayer     *songPlayerContext = NULL;
    unsigned char  *fileStart;
    int             fileSize;
    int             numLoops = 1;

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( fileName, &( fileSize ) );
    if( fileStart == NULL )
    {
        printf("ERROR: file %s not found.\n", fileName );
        return 1;
    }

    SHOW_SIZE( "file size = ", fileSize );

    SPMIDI_Initialize();
    SHOW_SIZE("after SPMIDI_Initialize() = ", SPMIDI_GetMemoryBytesAllocated() );

    /* Start synthesis engine with default number of voices. */
    result = SPMIDI_CreateContext(  &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
        goto error;
    SHOW_SIZE("after SPMIDI_CreateContext() = ", SPMIDI_GetMemoryBytesAllocated() );


    /* Create a player for the song */
    result = SongPlayer_Create( &songPlayerContext, spmidiContext, fileStart, fileSize );
    if( result < 0 )
        goto error;
    SHOW_SIZE("after SongPlayer_Create() = ", SPMIDI_GetMemoryBytesAllocated() );

    result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;


    /* Play the file */
    result = SongPlayer_Play( songPlayerContext, spmidiContext, numLoops );
    if( result < 0 )
        goto error;

    PRTMSG("File playback complete.\n");

error:
    /* Clean everything up */
    if( songPlayerContext != NULL )
        SongPlayer_Delete( songPlayerContext );
        
    if( sHostAudioDevice != NULL )
        SPMUtil_StopAudio( sHostAudioDevice );
        
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext(spmidiContext);

    SHOW_SIZE("Memory allocated before SPMIDI_Terminate() = ", SPMIDI_GetMemoryBytesAllocated() );

    SPMIDI_Terminate();

    SPMUtil_FreeFileImage( fileStart );

    if( result < 0 )
    {
        PRTMSG("Error playing file = ");
        PRTNUMD( result );
        PRTMSG( SPMUtil_GetErrorText( (SPMIDI_Error)result ) );
        PRTMSG("\n");
    }

    return result;
}


#if 1
/*******************************************************************/
int main(void);
int main(void)
{
    int             result = 0;
    int             i;

    while( result >= 0 )
    {
        for( i=0; i<NUM_SONGS; i++ )
        {
            result = PlaySongFile( songNames[i] );
            if( result < 0 ) goto error;
        }
    }

error:
    return 1;
}


#endif
