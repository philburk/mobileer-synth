/* $Id: test_reset.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Extremely simple test that just plays a few notes.
 * This is mostly for stepping through synth code.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_audio.h"
#include "portaudio.h"


#define PROGRAM_PIANO             (0x00) /* Acoustic Grand. */
#define PROGRAM_GLOCK             (0x09) /* Glockenspiel. */
#define PROGRAM_ORGAN             (0x12) /* RockOrgan. */
#define SAMPLES_PER_FRAME         (2)

#define SAMPLE_RATE         (11025)
#define CHANNEL             (1)

/* Host independant writable audio device. */
SPMIDI_AudioDevice  sHostAudioDevice;

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
short sAudioBuffer[SAMPLES_PER_BUFFER];

static int PlayAudioBuffers(SPMIDI_Context *spmidiContext, int numBuffers )
{
    int result = 0;
    int i;
    for( i=0; i<numBuffers; i++ )
    {
        /* Generate a buffer full of audio data as 16 bit samples. */
        int numFrames = SPMIDI_ReadFrames( spmidiContext, sAudioBuffer, SPMIDI_GetFramesPerBuffer(),
                           SAMPLES_PER_FRAME, 16 );
        if( numFrames < 0 ) return numFrames;
                           
        /* Write audio samples to the audio device. */
        result = SPMUtil_WriteAudioBuffer( sHostAudioDevice, sAudioBuffer, numFrames );
        if( result < 0 ) return result;
    }

    return result;
}

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;

    /* Create an SP-MIDI synthesis engine. */
    err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( err < 0 )
        goto error;

    /* Initialize audio device. */
    err = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
    if( err < 0 )
        goto error;

    /* Turn off compressor so we hear unmodified instrument sound. */
    err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( err < 0 )
        goto error;

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM_ORGAN );

        
    SPMUtil_NoteOn( spmidiContext, CHANNEL, 60, 64 );
    SPMUtil_NoteOn( spmidiContext, CHANNEL, 64, 64 );
    SPMUtil_NoteOn( spmidiContext, CHANNEL, 67, 64 );

    PlayAudioBuffers( spmidiContext, 20 );

    Pa_Sleep( 2000 );


    SPMIDI_DeleteContext(spmidiContext);
    SPMIDI_Terminate();

    printf("Test finished.\n");
    return err;

error:
    return err;
}
