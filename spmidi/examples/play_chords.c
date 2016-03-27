/* $Id: play_chords.c,v 1.8 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play chords using SPMIDI
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdlib.h>
#include <stdio.h>
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

	printf("SPMIDI Test: play chords.\n");

	err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
	if( err < 0 )
		return 1;

	for( i=0; i<12; i++ )
	{
		/* Note On */
		int base = 60 + (2*i);
		SPMUtil_NoteOn( spmidiContext, 0, base, 64 );
		SPMUtil_NoteOn( spmidiContext, 0, base + 4, 64 );
		SPMUtil_NoteOn( spmidiContext, 0, base + 7, 64 );
		SPMUtil_PlayMilliseconds( spmidiContext, 150 );

		/* Note Off */
		SPMUtil_NoteOff( spmidiContext, 0, base, 0 );
		SPMUtil_NoteOff( spmidiContext, 0, base + 4, 0 );
		SPMUtil_NoteOff( spmidiContext, 0, base + 7, 0 );
		SPMUtil_PlayMilliseconds( spmidiContext, 150 );
	}

	SPMUtil_Stop(spmidiContext);

	printf("Test finished.\n");
	return err;
}
