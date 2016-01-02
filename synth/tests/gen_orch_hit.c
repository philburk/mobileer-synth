/* $Id: gen_orch_hit.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Generate an orchestra hit
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

static int sPrograms[] =
    {
        0, /* Acoustic Grand. */
        35, /* Fretless Bass. */
        40, /* Violin. */
        56, /* Trumpet */
        26, /* Acoustic Steel Guitar */
        65, /* Alto Sax */
        72, /* Clarinet */
    };

static int sBends[] =
    {
        0, /* Acoustic Grand. */
        20, /* Fretless Bass. */
        -10, /* Violin. */
        5, /* Trumpet */
        -18, /* Acoustic Steel Guitar */
        8, /* Alto Sax */
        -8, /* Clarinet */
        9, /* extra for modulation */
        -19,
        9,
        -6,
        16,
        -4,
        12
    };

#define NUM_PROGRAMS        (sizeof(sPrograms)/sizeof(int))
#define NUM_BENDS           (sizeof(sPrograms)/sizeof(int))
#define NUM_NOTES           (15)
#define SAMPLE_RATE         (22050)
#define LOWEST_PITCH        (60 - 24)
#define PITCH_INCR          (12)
#define HIGHEST_PITCH       (60 + 24)
#define ON_DUR              (200)
#define OFF_DUR             (1400)

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;
    int i,j;
    //char *fileName = NULL;
    char *fileName = "mblr_orch_hits.wav";

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    /* Turn off compressor so we hear unmodified instrument sound. */
    err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( err < 0 )
        goto error;

    SPMIDI_SetMasterVolume( spmidiContext, SPMIDI_DEFAULT_MASTER_VOLUME * 2 );

    /* Setup channels. */
    for( j = 0; j<NUM_NOTES; j++ )
    {
        int channel = (j<9) ? j : (j+1); /* Skip drum channel */
        int programIndex = j % NUM_PROGRAMS;
        int bendIndex = j % NUM_BENDS;
        SPMUtil_ProgramChange( spmidiContext, channel, sPrograms[programIndex] );
        SPMUtil_PitchBend( spmidiContext, channel, MIDI_BEND_NONE + (70*sBends[bendIndex]) );
    }

    /* Note On */
    for( i = LOWEST_PITCH; i<=HIGHEST_PITCH; i+=PITCH_INCR )
    {
        for( j = 0; j<NUM_NOTES; j++ )
        {

            SPMUtil_NoteOn( spmidiContext, j, i, 64 );
        }
        SPMUtil_PlayMilliseconds( spmidiContext, ON_DUR );

        for( j = 0; j<NUM_NOTES; j++ )
        {

            SPMUtil_NoteOff( spmidiContext, j, i, 64 );
        }
        SPMUtil_PlayMilliseconds( spmidiContext, OFF_DUR );
    }

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
