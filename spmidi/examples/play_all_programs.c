/* $Id: play_all_programs.c,v 1.12 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play each MIDI Program
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define SAMPLE_RATE   (22050)
#define CHANNEL           (3)
#define ON_TIME         (400)
#define OFF_TIME        (800)
#define PITCH            (60)

/*******************************************************************/
int main(void);
int main(void)
{
	int err;
	int i;
	SPMIDI_Context *spmidiContext = NULL;

	char * FILENAME = NULL;
	//	char * FILENAME = "D:\\temp\\all_programs_me2000.wav";

	printf("SPMIDI Test: play all programs. SR = %d\n", SAMPLE_RATE );

	err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, FILENAME, SPMUTIL_OUTPUT_MONO );
	if( err < 0 )
		return 1;

	/* Turn off compressor so we hear unmodified instrument sound. */
	err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
	if( err < 0 )
		goto error;

	SPMIDI_SetMasterVolume( spmidiContext, SPMIDI_DEFAULT_MASTER_VOLUME * 8 );

	for( i=0; i<128; i++ )
	{
		printf("Program #%3d, %s\n", i, MIDI_GetProgramName( i ) );
		fflush(stdout);
		SPMUtil_ProgramChange( spmidiContext, CHANNEL, i );

		/* Note On */
		SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH, 64 );
		SPMUtil_PlayMilliseconds( spmidiContext, ON_TIME );

		/* Note Off */
		SPMUtil_NoteOff( spmidiContext, CHANNEL, PITCH, 0 );
		SPMUtil_PlayMilliseconds( spmidiContext, OFF_TIME );
	}

	SPMUtil_Stop(spmidiContext);

	printf("Test finished.\n");
	return 0;
error:
	printf("Test failed.\n");
	return 1;

}
