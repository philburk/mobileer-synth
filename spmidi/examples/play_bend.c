/* $Id: play_bend.c,v 1.8 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play a note with pitch bend using SPMIDI
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define SAMPLE_RATE     (44100)
#define CHANNEL         (0)
#define PROGRAM         (16) /* Drawbar organ */
//#define PROGRAM         (71) /* Clarinet */
//#define PROGRAM         (40) /* Violin */
//#define PROGRAM         (13) /* Marimba */

/********************************************************************/
void PlayNote( SPMIDI_Context *spmidiContext, int pitch )
{
	/* Note Off */
	SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, 64 );
	SPMUtil_PlayMilliseconds( spmidiContext, 200 );

	/* Note Off */
	SPMUtil_NoteOff( spmidiContext, CHANNEL, pitch, 0 );
	SPMUtil_PlayMilliseconds( spmidiContext, 100 );
}

/********************************************************************/
void TestRange( SPMIDI_Context *spmidiContext, int semitones )
{
	int i;
	int bend;

	printf("Play pairs of identical notes, first with pitch bend, then without. Range = %d\n", semitones );
	SPMUtil_PlayMilliseconds( spmidiContext, 500 );
	SPMUtil_PitchBend( spmidiContext, CHANNEL, MIDI_BEND_NONE );
	/* Play high note to mark start of sequence. */
	PlayNote( spmidiContext, 84 );

	SPMUtil_SetBendRange( spmidiContext, CHANNEL, semitones, 0 );
	for( i=0; i<((2*semitones) + 1); i++ )
	{
		bend = (MIDI_BEND_NONE * i) / semitones;
		if( bend > MIDI_BEND_MAX )
			bend = MIDI_BEND_MAX;
		printf("Bend = 0x%04X\n", bend );
		fflush(stdout);

		SPMUtil_PitchBend( spmidiContext, CHANNEL, bend );
		PlayNote( spmidiContext, 60 );

		SPMUtil_PitchBend( spmidiContext, CHANNEL, MIDI_BEND_NONE );
		PlayNote( spmidiContext, (60 - semitones) + i  );

		SPMUtil_PlayMilliseconds( spmidiContext, 200 );
	}
}

/*******************************************************************/
int main(void);
int main(void)
{
	int err;
	int bend;
	int i;
	SPMIDI_Context *spmidiContext = NULL;

	printf("SPMIDI Test Pitch Bend. SR = %d\n", SAMPLE_RATE);

	err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
	if( err < 0 )
		return 1;

	SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

	/* Test accuracy of bend. */
	TestRange( spmidiContext, 2 );
	TestRange( spmidiContext, 7 );
	TestRange( spmidiContext, 12 );

	printf("Bend one note through entire range to test smoothness of bend.\n");
	SPMUtil_PlayMilliseconds( spmidiContext, 500 );
	SPMUtil_PitchBend( spmidiContext, CHANNEL, MIDI_BEND_NONE );
	PlayNote( spmidiContext, 84 );

	SPMUtil_SetBendRange( spmidiContext, CHANNEL, 7, 0 );

	SPMUtil_PitchBend( spmidiContext, CHANNEL, 0 );
	SPMUtil_NoteOn( spmidiContext, CHANNEL, 60, 64 );

#define NUM_BENDS (4000)

	for( i=0; i<NUM_BENDS; i++ )
	{
		bend = (MIDI_BEND_MAX * i) / (NUM_BENDS - 1);

		SPMUtil_PitchBend( spmidiContext, CHANNEL, bend );

		SPMUtil_PlayMilliseconds( spmidiContext, 1 );
	}

	SPMUtil_NoteOff( spmidiContext, CHANNEL, 60, 64 );
	SPMUtil_PlayMilliseconds( spmidiContext, 200 );

	SPMUtil_Stop(spmidiContext);

	printf("Test finished.\n");
	return err;
}
