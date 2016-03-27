/* $Id: play_scale_async.c,v 1.2 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play a scale using Mobileer ME2000 asynchronously.
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "portaudio.h"

#define SAMPLE_RATE         (44100)
#define CHANNEL             (2)
#define PROGRAM_PIANO       (0)

/*******************************************************************/
/**
 * Send a 3 byte MIDI command to a background MIDI player through a FIFO.
 */
int MIDIWrite3( int command, int data1, int data2 )
{
	SPMUtil_WriteByteAsync( command );
	SPMUtil_WriteByteAsync( data1 );
	return SPMUtil_WriteByteAsync( data2 );
}

/*******************************************************************/
/**
 * Send a 2 byte MIDI command to a background MIDI player through a FIFO.
 */
int MIDIWrite2( int command, int data1 )
{
	SPMUtil_WriteByteAsync( command );
	return SPMUtil_WriteByteAsync( data1 );
}

/*******************************************************************/
int main(void);
int main(void)
{
	int err;
	int i;
	SPMIDI_Context *spmidiContext = NULL;

	printf("SPMIDI Test: play scale. SR = %d\n", SAMPLE_RATE);

	/* Initialize SPMIDI and audio output. */
	err = SPMUtil_StartAsync( &spmidiContext, SAMPLE_RATE );
	if( err < 0 )
		goto error;

	MIDIWrite2( MIDI_PROGRAM_CHANGE + CHANNEL, PROGRAM_PIANO );

	for( i=0; i<12; i++ )
	{
		/* Note ON */
		MIDIWrite3( MIDI_NOTE_ON + CHANNEL, 60 + i, 64 );

		/* Sleep using PortAudio for 100 milliseconds. */
		Pa_Sleep( 100 );

		/* Note OFF */
		MIDIWrite3( MIDI_NOTE_OFF + CHANNEL, 60 + i, 0 );

		/* Sleep using PortAudio for 100 milliseconds. */
		Pa_Sleep( 100 );
	}

	/* Sleep for awhile to let notes die down. */
	Pa_Sleep( 1000 );

	SPMUtil_StopAsync(spmidiContext);

	printf("Test finished.\n");
error:
	return err;
}
