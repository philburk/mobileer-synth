/* $Id: test_click.c,v 1.6 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test for clicks that can occur when stealing notes.
 * The worst case is when playing the same note repeatedly
 * on an instrument with a slow attack.
 * The click is eliminated in Mobileer code by using a
 * cross fade into the new note.
 * .
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_play.h"

/* #define PROGRAM             (0x5D) /* slow attack pad */
#define PROGRAM             (0x05) /* fast attack piano */

#define SAMPLE_RATE         (44100)
#define BASE_PITCH          (64)
#define NUM_NOTES           (16)
#define CHANNEL             (3)


/************************************************************/
/* Calculate pseudo-random 32 bit number based on linear congruential method. */
static unsigned long GenerateRandomNumber( void )
{
    /* Change this seed for different random sequences. */
    static unsigned long randSeed = 22222;
    randSeed = (randSeed * 196314165) + 907633515;
    return randSeed;
}

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;
    int i;
    unsigned long msec;

    printf("SPMIDI Test: play_note on program %d\n", PROGRAM );

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    /*    SPMIDI_Write( SysExGMOff, sizeof( SysExGMOff ) ); */

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

    /* Note On */
    for( i=0; i<NUM_NOTES; i++ )
    {
        int pitch = BASE_PITCH;
        printf("Pitch = %d\n", pitch );
        SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, 64 );
        msec = (GenerateRandomNumber() >> (32-9)) + 30;
        SPMUtil_PlayMilliseconds( spmidiContext, msec );

        /* Note Off */
        SPMUtil_NoteOff( spmidiContext, CHANNEL, pitch, 0 );
    }

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
