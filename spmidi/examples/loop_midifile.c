/* $Id: loop_midifile.c,v 1.13 2007/10/10 00:24:04 philjmsl Exp $ */
/**
 * Loop a MIDI File whose name is passed on the command line.
 * Assume that the EndOfTrack events are positioned appropriately so that
 * the song plays correctly when restarted immediately at the end.
 * It is common to place a silent measure at the end of the song
 * for a pause in the ring.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/midifile_player.h"

#include "spmidi/examples/midifile_names.h"

#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define NUM_LOOPS           (3)

/* This callback is used when scanning to see if a song uses the vibrator. */
static void CheckVibratorCallback( void *vibratorUserData, int pitch, int velocity )
{
	int *isVibratorUsedPtr = (int *)vibratorUserData;
	(void) pitch;
	/* Indicate that vibrator is used. */
	if( velocity > 0 )
		*isVibratorUsedPtr = 1;
}

static void MyVibratorCallback( void *vibratorUserData, int pitch, int velocity )
{
	printf("VIBRATE!! data = %p, pitch = %d, velocity = %d\n",
	       vibratorUserData, pitch, velocity );
	fflush( stdout );
}

/****************************************************************/
int SPMCustom_EstimateMaxAmplitude( MIDIFilePlayer *player, int samplesPerFrame,
                                    int masterVolume, int *isVibratorUsedPtr )
{
	int result;
#define SAMPLES_PER_BUFFER  (SPMIDI_MAX_SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)

	int maxAmplitude = 0;
	int amp;
	SPMIDI_Context *spmidiContext;

	/* Start synthesis engine with default number of voices. */
	result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		return result;

	/* While scanning, call back if a vibrator message is found. */
	SPMIDI_SetVibratorCallback( spmidiContext, CheckVibratorCallback, isVibratorUsedPtr );

	SPMIDI_SetMasterVolume( spmidiContext, masterVolume );

	while( (result = MIDIFilePlayer_PlayFrames( player, spmidiContext, SPMIDI_MAX_FRAMES_PER_BUFFER )) == 0)
	{
		/* Partially synthesize result, estimate amplitude. */
		amp = SPMIDI_EstimateMaxAmplitude( spmidiContext, SPMIDI_MAX_FRAMES_PER_BUFFER, samplesPerFrame );
		if( amp > maxAmplitude )
		{
			maxAmplitude = amp;
		}
	}

	SPMIDI_DeleteContext(spmidiContext);

	return maxAmplitude;
}

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes )
{
	int i;
	int result;
	int msec;
	int seconds;
	int rem_msec;
	int timeout;
	MIDIFilePlayer *player;
	SPMIDI_Context *spmidiContext;
	int maxAmplitude;
	int testVolume = 0x040;
	int masterVolume;
	int isVibratorUsed = 0;

	SPMIDI_Initialize();

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
	if( result < 0 )
		goto error;

	/* Estimate maximum amplitude so that we can calculate
	 * optimal volume.
	 */
	maxAmplitude = SPMCustom_EstimateMaxAmplitude( player, SAMPLES_PER_FRAME,
	               testVolume, &isVibratorUsed );
	if( result < 0 )
		goto error;
	printf("At masterVolume %d, estimated maxAmplitude is %d\n", testVolume, maxAmplitude );
	MIDIFilePlayer_Rewind( player );

	printf("Vibrator is %sused in this song.\n", isVibratorUsed ? "" : "NOT " );

	msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
	seconds = msec / 1000;
	rem_msec = msec - (seconds * 1000);
	printf("Duration = %d.%03d seconds\n", seconds, rem_msec );

	/*
	 * Initialize SPMIDI Synthesizer.
	 * Output to audio or a file.
	 */
	result = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SAMPLES_PER_FRAME );
	if( result < 0 )
		goto error;

	SPMIDI_SetVibratorCallback( spmidiContext, MyVibratorCallback,
	                            (void *) 0x01234567 );

	/* Calculate masterVolume that will maximize the signal amplitude without clipping. */
	masterVolume = (testVolume * 32767) / maxAmplitude;
	printf("New masterVolume = %d\n", masterVolume );
	SPMIDI_SetMasterVolume( spmidiContext, masterVolume );

	fflush(stdout); /* Needed for Mac OS X */

	for( i=0; i<NUM_LOOPS; i++ )
	{
		/*
		 * Play until we hit the end of all tracks.
		 * Tell the MIDI player to move forward on all tracks
		 * by a time equivalent to a buffers worth of frames.
		 * Generate one buffers worth of data and write it to the output stream.
		 */
		while( SPMUtil_PlayFileBuffer( player, spmidiContext ) == 0 )
			;

		/* Rewind song. */
		SPMUtil_Reset( spmidiContext );
		MIDIFilePlayer_Rewind( player );
	}

	timeout = (SAMPLE_RATE * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		SPMUtil_PlayBuffers( spmidiContext, 1 );
	}

	SPMUtil_Stop(spmidiContext);

	MIDIFilePlayer_Delete( player );

error:
	SPMIDI_Terminate();
	return result;
}


/****************************************************************/
int main( int argc, char ** argv )
{
	void *data = NULL;
	int  fileSize;
	int   result = -1;
	char *inputFileName = DEFAULT_FILENAME;

	/* Parse command line. */
	if( argc > 1 )
	{
		inputFileName = argv[1];
	}

	/* Load MIDI File into a memory image. */
	data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
	if( data == NULL )
	{
		printf("ERROR reading MIDI File.\n" );
		goto error;
	}

	printf("File: %s\n", inputFileName );

	result = MIDIFile_Play( data, fileSize );
	if( result < 0 )
	{
		printf("Error playing MIDI File = %d = %s\n", result,
		       SPMUtil_GetErrorText( result ) );
		goto error;
	}

error:
	if( data != NULL )
		SPMUtil_FreeFileImage( data );
	return result;
}

