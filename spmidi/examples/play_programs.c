/* $Id: play_programs.c,v 1.9 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play an assortment of MIDI Programs
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"

#define SAMPLE_RATE         (44100)

#define CHANNEL (5)

void PlayProgram( SPMIDI_Context *spmidiContext, int programIndex )
{
	int pitch;

	SPMUtil_ProgramChange( spmidiContext, CHANNEL, programIndex );

	SPMUtil_PlayBuffers( spmidiContext, 500 );

	for( pitch=40; pitch<110; pitch+=12 )
	{
		/* Note On */
		SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, 64 );
		SPMUtil_PlayMilliseconds( spmidiContext, 300 );

		/* Note Off */
		SPMUtil_NoteOff( spmidiContext, CHANNEL, pitch, 0 );
		SPMUtil_PlayMilliseconds( spmidiContext, 300 );
	}
}

/*******************************************************************/
int main(void);
int main(void)
{
	int err;
	SPMIDI_Context *spmidiContext = NULL;

	PRTMSGNUMD("SPMIDI Test: play several instruments. SR = ", SAMPLE_RATE);

	err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
	if( err < 0 )
		goto error;

	/*
	*/
	PRTMSG("Piano\n");
	PlayProgram( spmidiContext, 0x00 );

	PRTMSG("Harpsichord\n");
	PlayProgram( spmidiContext, 0x06 );

	PRTMSG("Glockenspiel\n");
	PlayProgram( spmidiContext, 0x09 );

	PRTMSG("Marimba\n");
	PlayProgram( spmidiContext, 0x0C );

	PRTMSG("Drawbar Organ\n");
	PlayProgram( spmidiContext, 0x10 );

	PRTMSG("Violin\n");
	PlayProgram( spmidiContext, 0x28 );

	PRTMSG("Trumpet\n");
	PlayProgram( spmidiContext, 0x38 );

	PRTMSG("Clarinet\n");
	PlayProgram( spmidiContext, 0x47 );

	PRTMSG("Flute\n");
	PlayProgram( spmidiContext, 0x49 );

	PRTMSG("New Age Pad\n");
	PlayProgram( spmidiContext, 0x58 );
	/*
	*/

	SPMUtil_Stop(spmidiContext);

	PRTMSG("Test finished.\n");
	return err;
error:
	return err;
}
