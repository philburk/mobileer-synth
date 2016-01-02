/* $Id: test_drums_8000.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play each Rhythm Instrument
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define SAMPLE_RATE         (11025)
#define DURATION            (700)

extern int gUseSineIfTooHigh;
extern int gReducePhaseModWithPitch;

#define FIRST_DRUM   (GMIDI_FIRST_DRUM - 2)
#define LAST_DRUM    (GMIDI_LAST_DRUM + 2)

/*******************************************************************/
int main(void);
int main(void)
{
    int err;
    int i;
    SPMIDI_Context *spmidiContext = NULL;

    printf("SPMIDI Test: play all drums, include pitches out of range.\n");

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_STEREO );
    if( err < 0 )
        return 1;

    /* Hit each available drums.
     * Go outside range to make sure code can handle it. */
    for( i=FIRST_DRUM; i<=LAST_DRUM; i++ )
    {
        printf("Pitch = #%d, Drum = %s\n", i, MIDI_GetDrumName(i) );

        gUseSineIfTooHigh = 0;
        gReducePhaseModWithPitch = 0;
        SPMUtil_NoteOn( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION );

        
        gUseSineIfTooHigh = 1;
        gReducePhaseModWithPitch = 0;
        SPMUtil_NoteOn( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION );

        
        gUseSineIfTooHigh = 0;
        gReducePhaseModWithPitch = 1;
        SPMUtil_NoteOn( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION );

        
        gUseSineIfTooHigh = 1;
        gReducePhaseModWithPitch = 1;
        SPMUtil_NoteOn( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION );
        
        SPMUtil_PlayMilliseconds( spmidiContext, 500 );

    }

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
}
