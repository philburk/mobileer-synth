/* $Id: play_vibrator.c,v 1.8 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Test callback used to vibrate phone during a ringtone.
 * Alternate between audible rings and vibrations.
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
#define NUM_NOTES           (8)
#define AUDIBLE_CHANNEL     (5)
#define VIBRATOR_CHANNEL    (6)

static void MyVibratorCallback( void *vibratorUserData, int pitch, int velocity )
{
	printf("VIBRATE!! data = %p, pitch = %d, velocity = %d\n",
	       vibratorUserData, pitch, velocity );
}

/*******************************************************************/
int main(void);
int main(void)
{
	int err;
	int i;
	SPMIDI_Context *spmidiContext = NULL;

	printf("SPMIDI Test: play_vibrator\n" );

	err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
	if( err < 0 )
		goto error;

	/* Select audible ring. */
	SPMUtil_ProgramChange( spmidiContext, AUDIBLE_CHANNEL, SPMIDI_VIBRATOR_PROGRAM );

	/* Select vibrator. */
	SPMUtil_ControlChange( spmidiContext, VIBRATOR_CHANNEL, MIDI_CONTROL_BANK, SPMIDI_VIBRATOR_BANK >> 8 );
	SPMUtil_ControlChange( spmidiContext, VIBRATOR_CHANNEL,
	                       MIDI_CONTROL_BANK + MIDI_CONTROL_LSB_OFFSET, SPMIDI_VIBRATOR_BANK & 0x007F );
	SPMUtil_ProgramChange( spmidiContext, VIBRATOR_CHANNEL, SPMIDI_VIBRATOR_PROGRAM );


	SPMIDI_SetVibratorCallback( spmidiContext, MyVibratorCallback,
	                            (void *) 0x01234567 );


	/* Note On */
	for( i=0; i<NUM_NOTES; i++ )
	{
		/* Alternate between audible ring and vibrator. */
		int channel = ((i & 1) == 0) ? AUDIBLE_CHANNEL : VIBRATOR_CHANNEL;
		printf("Channel = %d\n", channel );
		fflush(stdout);

		SPMUtil_NoteOn( spmidiContext, channel, 60, 64 );
		SPMUtil_PlayMilliseconds( spmidiContext, 1200 );

		/* Note Off */
		SPMUtil_NoteOff( spmidiContext, channel, 60, 0 );
		SPMUtil_PlayMilliseconds( spmidiContext, 600 );
	}

	SPMUtil_Stop(spmidiContext);

	printf("Test finished.\n");
	return err;
error:
	return err;
}
