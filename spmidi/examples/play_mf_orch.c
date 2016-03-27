/* $Id: play_mf_orch.c,v 1.1 2007/10/10 00:24:04 philjmsl Exp $ */
/**
 *
 * Play a MIDI File whose name is passed on the command line.
 * Load a custom orchestra.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>

#include "spmidi/include/streamio.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/examples/midifile_names.h"
#include "spmidi/include/program_list.h"

#define SAMPLE_RATE         (44100)
#define SAMPLES_PER_FRAME   (2)

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
static short sSampleBuffer[SAMPLES_PER_BUFFER];

/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyTextCallback( int trackIndex, int metaEventType,
                     const char *addr, int numChars, void *userData )
{
	int i;
	(void) userData; /* Prevent compiler warnings about unused data. */
	printf("MetaEvent type %d on track %d: ", metaEventType, trackIndex );
	for( i=0; i<numChars; i++ )
		printf("%c", addr[i] );
	printf("\n");
}

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, const char *fileName )
{
	int result;
	int msec;
	int seconds;
	int rem_msec;
	int timeout;
	int go = 1;
	MIDIFilePlayer *player;
	SPMIDI_Context *spmidiContext = NULL;

	SPMIDI_Orchestra *orchestra = NULL;
//	char *orchestraFileName = NULL;
	char *orchestraFileName = "D:\\mobileer_work\\A_Orchestra\\exports\\exported.mbis";

	SPMIDI_Initialize();

	if( orchestraFileName != NULL )
	{
		StreamIO *sio = NULL;
		SPMIDI_ProgramList *programList =  NULL;
		
		result = SPMIDI_CreateProgramList( &programList );
		if( result < 0 ) goto error;
		
		// Scan the MIDIFile to see what instruments we should load.
		result = MIDIFile_ScanForPrograms( programList, image, numBytes );
		if( result < 0 ) goto error;

		// Load file orchestra from disk.
		sio = Stream_OpenFile( orchestraFileName, "rb" );
		if( sio == NULL )
		{
			goto error;
		}

		result = SPMIDI_LoadOrchestra( sio, programList, &orchestra );
		if( result < 0 )
		{
			printf( "SPMIDI_LoadOrchestra returned %d\n", result );
			goto error;
		}
		
		Stream_Close( sio );

		SPMIDI_DeleteProgramList( programList );
		//printf("Bytes allocated = %d\n", SPMIDI_GetMemoryBytesAllocated() );
	}

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
	if( result < 0 )
		goto error;

	msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
	seconds = msec / 1000;
	rem_msec = msec - (seconds * 1000);
	printf("Duration = %d.%03d seconds\n", seconds, rem_msec );

	/*
	 * Set function to be called when a text MetaEvent is
	 * encountered while playing. Includes Lyric events.
	 */
	MIDIFilePlayer_SetTextCallback( player, MyTextCallback, NULL );

	/*
	 * Initialize SPMIDI Synthesizer.
	 * Output to audio or a file.
	 */
	
	/* Start synthesis engine with default number of voices. */
	result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		goto error;

	result = SPMUtil_StartVirtualAudio( SAMPLE_RATE, fileName, SAMPLES_PER_FRAME );
	if( result < 0 )
		goto error;

	fflush(stdout); /* Needed for Mac OS X */

	/*
	 * Play until we hit the end of all tracks.
	 * Tell the MIDI player to move forward on all tracks
	 * by a time equivalent to a buffers worth of frames.
	 * Generate one buffers worth of data and write it to the output stream.
	 */
	while( go )
	{
	
		int mfperr = MIDIFilePlayer_PlayFrames( player, spmidiContext, SPMIDI_GetFramesPerBuffer()  );
		if( mfperr < 0 )
		{
			result = mfperr;
			goto error;
		}
		else if( mfperr > 0 )
		{
			go = 0;
		}

			/* Synthesize samples and fill buffer. */
		SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_GetFramesPerBuffer(), SAMPLES_PER_FRAME, 16 );

		SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, SPMIDI_GetFramesPerBuffer() );
	}

	/* Wait for sound to die out. */
	timeout = (SAMPLE_RATE * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_GetFramesPerBuffer(), SAMPLES_PER_FRAME, 16 );

		SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, SPMIDI_GetFramesPerBuffer() );
	}

	SPMUtil_StopVirtualAudio();

	SPMIDI_DeleteContext(spmidiContext);

	MIDIFilePlayer_Delete( player );

	SPMIDI_DeleteOrchestra( orchestra );

error:
	SPMIDI_Terminate();
	return result;
}


/****************************************************************/
int main( int argc, char ** argv )
{
	int   i;
	void *data = NULL;
	int  fileSize;
	int   result = 0;
	char *outputFileName = NULL;
	char *inputFileName = DEFAULT_FILENAME;

	/* Parse command line. */
	for( i=1; i<argc; i++ )
	{
		char *s = argv[i];
		if( s[0] == '-' )
		{
			switch( s[1] )
			{
			case 'w':
				outputFileName = &s[2];
				break;
			}
		}
		else
		{
			inputFileName = argv[i];
		}
	}

	/* Load MIDI File into a memory image. */
	data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
	if( data == NULL )
	{
		printf("ERROR reading MIDI File.\n" );
		goto error;
	}

	printf("File: %s\n", inputFileName );

	result = MIDIFile_Play( data, fileSize, outputFileName );
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

