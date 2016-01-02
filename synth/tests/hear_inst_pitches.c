/* $Id: hear_inst_pitches.c,v 1.6 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play all pitches.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"

#define SAMPLE_RATE     (16000)

#define FIRST_PROGRAM   (0)
#define NUM_PROGRAMS    (127)

#define PITCH           (84)
#define ON_MSEC         (300)
#define OFF_MSEC        (100)


/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;
    int currentProgram;
    int channelA;
    int channelB;
    int result;

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    currentProgram = FIRST_PROGRAM;
    channelA = 0;
    channelB = 1;

    SPMIDI_SetMasterVolume( spmidiContext, SPMIDI_DEFAULT_MASTER_VOLUME * 4 );

    /* Turn off compressor so we hear unmodified instrument sound. */
    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( result < 0 )
        goto error;

    SPMUtil_ProgramChange( spmidiContext, channelA, currentProgram );
    while( currentProgram < (FIRST_PROGRAM + NUM_PROGRAMS) )
    {

        /* Note On */
        SPMUtil_NoteOn( spmidiContext, channelA, PITCH, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, ON_MSEC );

        /* Note Off */
        SPMUtil_NoteOff( spmidiContext, channelA, PITCH, 0 );
        SPMUtil_PlayMilliseconds( spmidiContext, OFF_MSEC );

        printf("Program = 0x%02X (%d) %s\n", currentProgram,
               currentProgram+1, MIDI_GetProgramName(currentProgram) );
        fflush(stdout);

        /* Note On */
        SPMUtil_NoteOn( spmidiContext, channelA, PITCH, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, ON_MSEC );

        /* Note Off */
        SPMUtil_NoteOff( spmidiContext, channelA, PITCH, 0 );
        SPMUtil_PlayMilliseconds( spmidiContext, OFF_MSEC );

        currentProgram += 1;
        SPMUtil_ProgramChange( spmidiContext, channelB, currentProgram );

        /* Two Notes On */
        SPMUtil_NoteOn( spmidiContext, channelA, PITCH, 64 );
        SPMUtil_NoteOn( spmidiContext, channelB, PITCH, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, ON_MSEC );

        /* Two Notes Off */
        SPMUtil_NoteOff( spmidiContext, channelA, PITCH, 0 );
        SPMUtil_NoteOff( spmidiContext, channelB, PITCH, 0 );
        SPMUtil_PlayMilliseconds( spmidiContext, OFF_MSEC );

        channelA = channelA ^ 1;
        channelB = channelB ^ 1;
    }

    SPMUtil_Stop(spmidiContext);

    PRTMSG("Test finished.\n");
    return err;
error:
    return err;
}
