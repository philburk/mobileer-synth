/* $Id: test_play_midifile.c,v 1.3 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Play a MIDI file on an audio device that uses blocking writes.
 *
 * This file is to assist integration on target systems.
 * It demonstrates the basic techniques and is intended as
 * the starting point for a platform specific implementation.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/examples/midifile_names.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)

/* 1 for mono, 2 for stereo */
#define SAMPLES_PER_FRAME   (2)

/* How many times do we play the song? */
#define NUM_REPITITIONS   (1)

#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)


/* Host independant writable audio device. */
SPMIDI_AudioDevice  sHostAudioDevice;

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
short sAudioBuffer[SAMPLES_PER_BUFFER];
    
/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static int PlayAudioBuffer(SPMIDI_Context *spmidiContext)
{
    int result;
    
    /* Generate a buffer full of audio data as 16 bit samples. */
    result = SPMIDI_ReadFrames( spmidiContext, sAudioBuffer, FRAMES_PER_BUFFER,
                       SAMPLES_PER_FRAME, 16 );
    if( result < 0 ) return result;
                       
    /* Write audio samples to the audio device. */
    return SPMUtil_WriteAudioBuffer( sHostAudioDevice, sAudioBuffer, FRAMES_PER_BUFFER );
}

/****************************************************************/
/**
 * Play the song several times in a loop.
 */
static int MIDIFile_Loop( SPMIDI_Context *spmidiContext, MIDIFilePlayer *player, int numLoops )
{
    int i;
    int timeout;
    int result;
    int go;
    int frames;

    frames = MIDIFilePlayer_GetDurationInFrames( player );
    printf("Duration in frames = %d\n", frames );
    printf("Estimated buffersPlayed = %d\n", frames / FRAMES_PER_BUFFER );

    
    for( i=0; i<numLoops; i++ )
    {
        int buffersPlayed = 0;

        /* Reset MIDI programs, controllers, etc. */
        SPMUtil_Reset( spmidiContext );
        
        /* Rewind song to beginning. */
        MIDIFilePlayer_Rewind( player );
        
        /*
         * Play the song once.
         * Process one buffer's worth of MIDI data each time through the loop.
         */
        go = 1;
        while( go )
        {
            /* Play a small bit of the MIDI file. */
            result = MIDIFilePlayer_PlayFrames( player, spmidiContext, FRAMES_PER_BUFFER );
            if( result > 0 )
            {
                /* Song has finished. */
                go = 0;
            }
            
            if( result < 0 )
            {
                /* Error has occured. */
                return result;
            }
            else
            {
                /* Still in the middle of the song so synthesize a buffer full of audio. */
                result = PlayAudioBuffer(spmidiContext);
                if( result < 0 ) return result;
                buffersPlayed += 1;
            }
        }
        printf("Actual buffersPlayed = %d\n", buffersPlayed );
    }
    
        
    /*
     * Continue playing until all of the notes have finished sounding,
     * or for one second, whichever is shorter.
     */
    timeout = SAMPLE_RATE / FRAMES_PER_BUFFER;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        result = PlayAudioBuffer(spmidiContext);
        if( result < 0 ) return result;
    }
    
    return 0;
}

/****************************************************************/
/**
 * Play a MIDI file one audio buffer at a time.
 */
static int MIDIFile_Play( const unsigned char *image, int numBytes )
{
    int result;
    MIDIFilePlayer *player;
    SPMIDI_Context *spmidiContext = NULL;

    SPMIDI_Initialize();

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
        goto error1;

    /* Create an SP-MIDI synthesis engine. */
    result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
        goto error2;

    /* Initialize audio device. */
    result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error3;

    /* Play the song several times. */
    result = MIDIFile_Loop( spmidiContext, player, NUM_REPITITIONS );
    if( result < 0 )
        goto error2;
    
    /* Close audio hardware. */
    SPMUtil_StopAudio( sHostAudioDevice );

    /* Terminate SP-MIDI synthesizer. */
error3:
    SPMIDI_DeleteContext(spmidiContext);

    /* Free any data structures allocated for playing the MIDI file. */
error2:
    MIDIFilePlayer_Delete( player );

error1:
    SPMIDI_Terminate();
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

    /*
     * Play the midifile contained in the midiFileImage char array.
     */
    result = MIDIFile_Play( data, fileSize );
    if( result < 0 )
    {
        PRTMSG("Error playing MIDI File = ");
        PRTNUMD( result );
        PRTMSG( SPMUtil_GetErrorText( result ) );
        PRTMSG("\n");
    }

    PRTMSG("MIDI File playback complete.\n");

error:
    return (result < 0);
}

