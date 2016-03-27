/* $Id: play_drums.c,v 1.8 2007/10/02 16:15:32 philjmsl Exp $ */
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

#define SAMPLE_RATE         (44100)

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
	for( i=(GMIDI_FIRST_DRUM - 5); i<=(GMIDI_LAST_DRUM + 5); i++ )
	{
		printf("Pitch = #%d, Drum = %s\n", i, MIDI_GetDrumName(i) );

		/* Note On */
		SPMUtil_NoteOn( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, 64 );
		SPMUtil_PlayMilliseconds( spmidiContext, 300 );

		/* Note Off */
		SPMUtil_NoteOff( spmidiContext, MIDI_RHYTHM_CHANNEL_INDEX, i, 0 );
		SPMUtil_PlayMilliseconds( spmidiContext, 300 );
	}

	SPMUtil_Stop(spmidiContext);

	printf("Test finished.\n");
	return err;
}
