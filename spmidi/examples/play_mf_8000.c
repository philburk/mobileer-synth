/* $Id: play_mf_8000.c,v 1.11 2007/10/10 00:24:04 philjmsl Exp $ */
/**
 *
 * Play a MIDI File at 8000 Hz by synthesizing at 16000 Hz
 * and downsampling the result.
 *
 * Author: Phil Burk
 * Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/examples/midifile_names.h"

#define SYNTHESIS_RATE      (16000)
#define OUTPUT_RATE         (SYNTHESIS_RATE/2)
#define SAMPLES_PER_FRAME   (1)


#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)
#define NUM_SYNTH (FRAMES_PER_BUFFER)
#define NUM_OUT (NUM_SYNTH/2)

short samplesSynth[NUM_SYNTH];
short samplesOut[NUM_OUT];

#define USE_AVERAGE_FILTER  (0)

/****************************************************************/
/******* Simple LowPass IIR Filter ****************************/
/****************************************************************/
typedef struct LowPassFilter_s
{
	int yn1;
}
LowPassFilter_s;

LowPassFilter_s sFilter = { 0 };

/* If we are converting from 16000 to 8000 Hz then we need
 * a lowpass cutoff at 4000 Hz.
 * We can use the recursive filter:
 *     y(n) = ax(n) - by(n-1)
 * Where a = 0.75 and b = -0.25 
 */
int Filter_LowPass( LowPassFilter_s *filter, int input )
{
	/* Equivalent to multiplying by 0.75 */
	int axn = input - (input >> 2);
	/* Equivalent to multiplying by -(-0.25) */
	int mbyn1 = filter->yn1 >> 2;
	int output = axn + mbyn1;
	filter->yn1 = output;
	return output;
}

/****************************************************************/
void PlayDownsampledBuffer( SPMIDI_Context *spmidiContext )
{
	int is;

	/* Synthesize samples and fill buffer. */
	SPMIDI_ReadFrames( spmidiContext, samplesSynth, NUM_SYNTH, SAMPLES_PER_FRAME, 16 );

	/* Downsample from synthesis rate to output rate. */
	for( is=0; is<NUM_OUT; is++ )
	{
#if USE_AVERAGE_FILTER
		/* Apply simple averaging filter before downsampling. */
		int s1 = (int) samplesSynth[ is * 2 ];
		int s2 = (int) samplesSynth[ (is * 2) + 1 ];
		short downSample = (short) ((s1 + s2) >> 1);
#else

		short downSample;
		Filter_LowPass( &sFilter, (int) samplesSynth[ is * 2 ] );
		// Only save every other sample to decimate by 2.
		downSample = (short) Filter_LowPass( &sFilter, (int) samplesSynth[ (is * 2) + 1 ] );
#endif

		samplesOut[is] = downSample;
	}

	/* Write samples to the audio device. */
	SPMUtil_WriteVirtualAudio( samplesOut, SAMPLES_PER_FRAME, NUM_OUT );
}

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, char *outputFileName, int drumVolume )
{
	int result;
	int msec;
	int seconds;
	int rem_msec;
	int timeout;
	MIDIFilePlayer *player;
	SPMIDI_Context *spmidiContext = NULL;

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, SYNTHESIS_RATE, image, numBytes );
	if( result < 0 )
		goto error1;

	msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
	seconds = msec / 1000;
	rem_msec = msec - (seconds * 1000);
	printf("Duration = %d.%03d seconds\n", seconds, rem_msec );

	/* Start synthesis engine with default number of voices. */
	result = SPMIDI_CreateContext( &spmidiContext, SYNTHESIS_RATE );
	if( result < 0 )
		goto error2;

	
	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, drumVolume );
	if( result < 0 )
	{
		printf("\nERROR: Illegal value for Rhythm Volume = %d\n", drumVolume );
	}

	result = SPMUtil_StartVirtualAudio( OUTPUT_RATE, outputFileName, SAMPLES_PER_FRAME );
	if( result < 0 )
		goto error3;


	fflush(stdout); /* Needed for Mac OS X */

	/*
	 * Play until we hit the end of all tracks.
	 * Tell the MIDI player to move forward on all tracks
	 * by a time equivalent to a buffers worth of frames.
	 * Generate one buffers worth of data and write it to the output stream.
	 */
	result = 0;
	while( result == 0 )
	{
		result = MIDIFilePlayer_PlayFrames( player, spmidiContext, FRAMES_PER_BUFFER );
		PlayDownsampledBuffer( spmidiContext );
	}

	timeout = (SYNTHESIS_RATE * 4) / FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		PlayDownsampledBuffer( spmidiContext );
	}


error3:
	SPMUtil_Stop(spmidiContext);

error2:
	MIDIFilePlayer_Delete( player );

error1:
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
	int   drumVolume = SPMIDI_DEFAULT_MASTER_VOLUME;
	char *inputFileName = DEFAULT_FILENAME;

//	char *outputFileName = "play_mf_8000_square.wav";
	char *outputFileName = NULL;

	printf("playmf_8000hz: August 25, 2004\n");
	printf("(C) 2004 Mobileer, Inc. All Rights Reserved\n");

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
			case 'v':
				drumVolume = atoi( &s[2] );
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

	printf("Input File: %s\n", inputFileName );
	if( outputFileName != NULL )
	{
		printf("Output File: %s\n", outputFileName );
	}

	result = MIDIFile_Play( data, fileSize, outputFileName, drumVolume );
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

