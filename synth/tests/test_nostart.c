/* $Id: test_nostart.c,v 1.5 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * What happens if we call SPMIDI routines before calling SPMIDI_CreateContext()?
 * It should return without an address exception.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"

#define PROGRAM             (0x44)

#define SAMPLE_RATE         (44100)
#define BASE_PITCH          (64)
#define PITCH_INCR          (1)
#define NUM_NOTES           (8)
#define CHANNEL             (3)


/*******************************************************************/
int main(void);
int main(void)
{
    int err = 0;
    int i;
    SPMIDI_Context *spmidiContext = NULL;

    printf("SPMIDI Test: play notes before calling SPMIDI_CreateContext() %d\n" );

    /*    SPMIDI_Write( SysExGMOff, sizeof( SysExGMOff ) ); */

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

    /* Note On */
    for( i=0; i<NUM_NOTES; i++ )
    {
        int pitch = BASE_PITCH + (i * PITCH_INCR);
        printf("Pitch = %d\n", pitch );
        SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, 64 );

        /* Note Off */
        SPMUtil_NoteOff( spmidiContext, CHANNEL, pitch, 0 );
    }

    printf("Test finished.\n");
    return err;
}
