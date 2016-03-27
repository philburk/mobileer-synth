/* $Id: play_scale.c,v 1.9 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play a scale using SPMIDI
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define SAMPLE_RATE         (44100)
#define CHANNEL             (0)

/*******************************************************************/
int main(void);
int main(void)
{
	int err;
	int i;
	SPMIDI_Context *spmidiContext = NULL;

	printf("SPMIDI Test: play scale. SR = %d\n", SAMPLE_RATE);

	/* Initialize SPMIDI and audio output. */
	err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_STEREO );
	if( err < 0 )
		goto error;

	SPMUtil_ProgramChange( spmidiContext, CHANNEL, 0x01 );

	for( i=0; i<12; i++ )
	{
		/* Note ON */
		SPMUtil_NoteOn( spmidiContext, CHANNEL, 60 + i, 64 );
		SPMUtil_PlayMilliseconds( spmidiContext, 100 );

		/* Note OFF */
		SPMUtil_NoteOff( spmidiContext, CHANNEL, 60 + i, 0 );
		SPMUtil_PlayMilliseconds( spmidiContext, 50 );
	}

	SPMUtil_PlayMilliseconds( spmidiContext, 200 );

	SPMUtil_Stop(spmidiContext);

	printf("Test finished.\n");
error:
	return err;
}
