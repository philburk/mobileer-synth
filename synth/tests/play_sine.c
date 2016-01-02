/* $Id$ */
/**
 *
 * Play a sine wave to test the Audio interface layer.
 *
 * This file is to assist integration on target systems.
 *
 * Author: Phil Burk
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"

#include "gen_sine.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (64)


/* Host independant writable audio device. */
SPMIDI_AudioDevice  sHostAudioDevice;

short sAudioBuffer[ FRAMES_PER_BUFFER * SAMPLES_PER_FRAME ];


/****************************************************************/
int main( void )
{
    int result;
    int i;

    printf("Play sine waves.\n");
    printf("(C) 2005 Mobileer Inc\n");

    /* Initialize audio hardware. */
    result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;

    /* Loop enough times to hear the sine waves. */
    for( i=0; i<10000; i++ )
    {
        FillBufferWithSines( sAudioBuffer, FRAMES_PER_BUFFER, SAMPLES_PER_FRAME );

        /* Write audio samples to the audio device. */
        SPMUtil_WriteAudioBuffer( sHostAudioDevice, sAudioBuffer, FRAMES_PER_BUFFER );
    }

    /* Close audio hardware. */
    SPMUtil_StopAudio( sHostAudioDevice );

error:
    return result;
}

