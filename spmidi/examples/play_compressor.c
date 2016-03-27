/* $Id: play_compressor.c,v 1.7 2007/10/10 00:24:04 philjmsl Exp $ */
/**
 *
 * Play a MIDI File with various compressor settings.
 * Use the SPMIDI Synthesizer.
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

#define SAMPLE_RATE         (44100)
#define SAMPLES_PER_FRAME   (2)

#if 0
#define COMPRESSOR_ON       (0) /* Disable compression. */
#define CURVE               (20)
#define TARGET              (88)
#define THRESHOLD           ( 2)

#elif 0
#define COMPRESSOR_ON       (1)
#define CURVE               (20) /* Gentle compression. */
#define TARGET              (88)
#define THRESHOLD           ( 2)

#elif 0
#define COMPRESSOR_ON       (1)
#define CURVE               (10) /* More intense compression. */
#define TARGET              (90)
#define THRESHOLD           ( 2)

#elif 1
#define COMPRESSOR_ON       (1)
#define CURVE               (5) /* Extreme compression. */
#define TARGET              (90)
#define THRESHOLD           ( 2)
#endif

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, const char *fileName )
{
	int result;
	int timeout;
	MIDIFilePlayer *player;
	SPMIDI_Context *spmidiContext = NULL;

	SPMIDI_Initialize();

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
	if( result < 0 )
		goto error;

	/*
	 * Initialize SPMIDI Synthesizer.
	 * Output to audio or a file.
	 */
	result = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SAMPLES_PER_FRAME );
	if( result < 0 )
		goto error;

	/* Set parameters that control compressor. */
	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, COMPRESSOR_ON );
	if( result < 0 )
		goto error;
	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_CURVE, CURVE );
	if( result < 0 )
		goto error;
	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_TARGET, TARGET );
	if( result < 0 )
		goto error;
	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_THRESHOLD, THRESHOLD );
	if( result < 0 )
		goto error;

#if OPTIMIZE_VOLUME
	/* Calculate masterVolume that results in maximum amplitude without clipping. */
	masterVolume = (testVolume * 32767) / maxAmplitude;
	printf("New masterVolume = %d\n", masterVolume );
	SPMIDI_SetMasterVolume( masterVolume );
#endif

	fflush(stdout); /* Needed for Mac OS X */

	/*
	 * Play until we hit the end of all tracks.
	 * Tell the MIDI player to move forward on all tracks
	 * by a time equivalent to a buffers worth of frames.
	 * Generate one buffers worth of data and write it to the output stream.
	 */
	while( SPMUtil_PlayFileBuffer( player, spmidiContext ) == 0 )
		;

	timeout = (SAMPLE_RATE * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		SPMUtil_PlayBuffers( spmidiContext, 1 );
	}

	/* Let compressor and other delays settle. */
	SPMUtil_PlayBuffers( spmidiContext, 20 );

	SPMUtil_Stop(spmidiContext);

	MIDIFilePlayer_Delete( player );

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
	int   result = -1;
	char *outputFileName = NULL;
	//    char *outputFileName = "comp_3.wav";
	char *inputFileName = DEFAULT_FILENAME;

	printf("Play a MIDI file using specified compression settings.\n");

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

