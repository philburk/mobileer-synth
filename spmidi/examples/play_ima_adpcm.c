/*
 * Play IMA ADPCM data from a WAV file image in memory.
 *
 * Author: Phil Burk
 * Copyright 1998 Phil Burk
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/ima_adpcm.h"
#include "spmidi/include/ima_adpcm_wav.h"
#include "spmidi/include/read_wav.h"
#include "spmidi/include/spmidi_load.h"

#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_print.h"

#define SAMPLE_RATE   (11025)
#define SAMPLES_PER_FRAME  (1)

#define SHORTS_PER_BUFFER  (512*1024)
static short decodedSamples[SHORTS_PER_BUFFER];

/**************************************************************/
int main( void )
{
	SPMIDI_AudioDevice device;
	char *inputFileName = "E:\\nomad\\MIDISynth\\test\\chewymonkeys_11025_ima.raw";
//	char *inputFileName = "E:\\nomad\\MIDISynth\\test\\short_11025_ima.raw";
	int   fileSize;
	void *data = NULL;
	unsigned char *encodedData;
	int   numSamplesLeft = 0;
	int   numSamples;
	int   result;

	/* ADPCM state information. */
	int   stepIndex = 0;
	short previousValue = 0;

	SPMIDI_HostInit();

	PRTMSG( "Play the ADPCM encoded file: " );
	PRTMSG( inputFileName );
	PRTMSG( "\n" );

	/* Get encoded data from a file. */
	data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
	if( data == NULL )
	{
		printf("ERROR could not find MIDI File %s\n", inputFileName );
		goto error;
	}
	
	result = SPMUtil_StartAudio( &device, SAMPLE_RATE, SAMPLES_PER_FRAME );
	if( result < 0 )
	{
		goto error;
	}

	/* Decode and play samples until we run out of data. */
	encodedData = data;
	numSamplesLeft = fileSize * 2;
	while( numSamplesLeft > 0 )
	{
		/* Play a buffer full or whatever is left to play. */
		numSamples = ( numSamplesLeft < SHORTS_PER_BUFFER ) ?
			numSamplesLeft : SHORTS_PER_BUFFER;

		/* Decode a block of ADPCM data into 16 bit samples. */
		stepIndex = IMA_DecodeArray( encodedData,
	                 numSamples,
	                 previousValue,
	                 stepIndex,      /* Value between 0 and 88, typically 0. */
	                 decodedSamples  /* Output array must be large enough for numSamples. */
	               );

		/* Advance to next block of encoded data. */
		encodedData += numSamples/2;      /* Two samples per byte of encodedArray. */
		numSamplesLeft -= numSamples;

		/* Play any samples that we have decoded. */
		if( numSamples > 0 )
		{
			/* Grab last sample to feed into next buffer. */
			previousValue = decodedSamples[ numSamples - 1 ];

			SPMUtil_WriteAudioBuffer( device, decodedSamples, numSamples );
		}
	}

	SPMUtil_StopAudio( device );
	SPMIDI_HostTerm();
	return 0;

error:
	printf("Error exit.\n");
	SPMIDI_HostTerm();
	return 1;
}
