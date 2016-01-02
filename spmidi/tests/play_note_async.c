/* $Id: play_note_async.c,v 1.7 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Extremely simple test that just plays one note asynchronously.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "portaudio.h"

#define SAMPLE_RATE         (44100)

#define PITCH               (60)
#define CHANNEL             (3)
#define PROGRAM             (0x15)

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;

    printf("SPMIDI Test: play_note on program %d in background thread.\n", PROGRAM );

    err = SPMUtil_StartAsync( &spmidiContext, SAMPLE_RATE );
    if( err < 0 )
        goto error;

    SPMUtil_WriteByteAsync( MIDI_PROGRAM_CHANGE + CHANNEL );
    SPMUtil_WriteByteAsync(  PROGRAM );

    /* Note On */
    SPMUtil_WriteByteAsync( MIDI_NOTE_ON + CHANNEL );
    SPMUtil_WriteByteAsync( PITCH );
    SPMUtil_WriteByteAsync( 64 );

    Pa_Sleep( 2000 );

    /* Note Off */
    SPMUtil_WriteByteAsync( MIDI_NOTE_OFF + CHANNEL );
    SPMUtil_WriteByteAsync( PITCH );
    SPMUtil_WriteByteAsync( 0 );

    Pa_Sleep( 2000 );

    SPMUtil_StopAsync(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
