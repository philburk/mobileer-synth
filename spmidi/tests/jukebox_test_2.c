/* $Id: jukebox_test_2.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play a few notes on JukeBox.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_jukebox.h"
#include "spmidi/include/spmidi_print.h"

#define SAMPLE_RATE        (22050)
#define SAMPLES_PER_FRAME  (1)
#define MAX_FRAMES         (1024)

short sAudioBuffer[MAX_FRAMES];
/* Host independant writable audio device. */
SPMIDI_AudioDevice  sHostAudioDevice;

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Error err;
    int i;

    PRTMSG("jukebox_test_2 - play first song in playlist.\n");

    /* Initialize audio device. */
    err = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
    if( err < 0 )
        goto error1;

    err = JukeBox_Initialize( SAMPLE_RATE );
    if( err < 0 )
        goto error2;

    err = JukeBox_QueueSong( 0, 1 );

    for( i=0; i<10000; i++ )
    {
        int numFrames;

        numFrames = JukeBox_SynthesizeAudioTick( sAudioBuffer, MAX_FRAMES, SAMPLES_PER_FRAME );
        if( numFrames < 0 )
        {
            err = numFrames;
            goto error3;
        }

    /* Write audio samples to the audio device. */
        err = SPMUtil_WriteAudioBuffer( sHostAudioDevice, sAudioBuffer, numFrames );
        if( err < 0 )
            goto error3;
    }

error3:
    JukeBox_Terminate();

error2:
    /* Close audio hardware. */
    SPMUtil_StopAudio( sHostAudioDevice );

error1:
    printf("Test finished.\n");
    return err;
}
