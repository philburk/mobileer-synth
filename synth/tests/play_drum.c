/* $Id: play_drum.c,v 1.6 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Extremely simple test that just plays a drum hit.
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


#define SAMPLE_RATE         (8000)
//#define DRUM_PITCH          (38)  /* Acoustic Snare */
#define DRUM_PITCH          (70)  /* Maracas */
#define NUM_NOTES           (1)
#define DURATION            (1000)


/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;
    int i;
char *fileName = NULL;
//char *fileName = "rendered_drums.wav";
    printf("SPMIDI Test: play_drum on pitch %d = %s\n", DRUM_PITCH, MIDI_GetDrumName( DRUM_PITCH )  );

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    /* Turn off compressor so we hear unmodified instrument sound. */
    err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( err < 0 )
        goto error;

    SPMIDI_SetMasterVolume( spmidiContext, SPMIDI_DEFAULT_MASTER_VOLUME * 8 );

    /* Note On */
    for( i = 0; i<NUM_NOTES; i++ )
    {
        SPMUtil_NoteOn( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, DRUM_PITCH, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION );

        /* Note Off */
        SPMUtil_NoteOff( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, DRUM_PITCH, 0 );
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION );
    }

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
