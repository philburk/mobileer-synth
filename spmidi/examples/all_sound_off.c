/* $Id: all_sound_off.c,v 1.2 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Force sound to stop abruptly using MIDI_CONTROL_ALLSOUNDOFF.
 *
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
#define SAMPLES_PER_FRAME  (2)
#define CHANNEL             (0)
#define NUM_NOTES           (6)

/** Choose an instrument that has a long release time. */
#define PROGRAM    (14) /* Tubular Bells */

static short sSampleBuffer[ SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER ];

/****************************************************************/
void PlayBuffers( SPMIDI_Context *spmidiContext, int numBuffers )
{
	int i;

	for( i=0; i<numBuffers; i++ )
	{
		/* Synthesize samples and fill buffer. */
		SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_GetFramesPerBuffer(), SAMPLES_PER_FRAME, 16 );

		SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, SPMIDI_GetFramesPerBuffer() );
	}
}

/************************************************************/
void BangNotes( SPMIDI_Context *spmidiContext )
{
	int i;
#define CALC_PITCH(ii) (50 + (ii*2))
	for( i=0; i<NUM_NOTES; i++ )
	{
		/* Note ON */
		SPMUtil_NoteOn( spmidiContext, CHANNEL, CALC_PITCH(i), 64 );
	}
	SPMUtil_PlayBuffers( spmidiContext, 1 );

	for( i=0; i<NUM_NOTES; i++ )
	{
		/* Note OFF */
		SPMUtil_NoteOff( spmidiContext, CHANNEL, CALC_PITCH(i), 0 );
	}
	SPMUtil_PlayBuffers( spmidiContext, 1 );
}

/************************************************************/
void PlayUntilSilent( SPMIDI_Context *spmidiContext )
{
	int count;
		
	/*
	 * Continue playing until all of the notes have finished sounding,
	 * or for one second, whichever is shorter.
	 */
	count = 0;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (count < 1000) )
	{
		SPMUtil_PlayBuffers( spmidiContext, 1 );
		count++;
	}

	printf("It took %d buffers for the sound to die down.\n", count);

	/* Leave a gap between tests so we can hear them more easily. */
	PlayBuffers( spmidiContext, 1000 );
}

/*******************************************************************/
int main(void);
int main(void)
{
	int err;
	SPMIDI_Context *spmidiContext = NULL;

	printf("SPMIDI Test: play notes and let them die slow or stop them abruptly.\n");

	/* Initialize SPMIDI and audio output. */
	err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_STEREO );
	if( err < 0 )
		goto error;

	SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

	printf("Bong!\n");
	BangNotes(spmidiContext);
	PlayUntilSilent( spmidiContext );

	printf("Bong and Reset.\n");
	BangNotes(spmidiContext);
	SPMUtil_Reset( spmidiContext );
	SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );
	PlayUntilSilent( spmidiContext );

	/* Use MIDI_CONTROL_ALLSOUNDOFF to abruptly stop the sound. */
	printf("Bong and ALLSOUNDOFF.\n");
	BangNotes(spmidiContext);
	SPMUtil_ControlChange( spmidiContext, CHANNEL, MIDI_CONTROL_ALLSOUNDOFF, 0 );
	PlayUntilSilent( spmidiContext );

	printf("Bong!\n");
	BangNotes(spmidiContext);
	PlayUntilSilent( spmidiContext );

	SPMUtil_Stop(spmidiContext);

	printf("Test finished.\n");
error:
	return err;
}
