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

#define SAMPLE_RATE   (11025)
#define SAMPLES_PER_FRAME  (1)

#define SHORTS_PER_BUFFER  (2048)
static short decodedSamples[SHORTS_PER_BUFFER];

/**************************************************************/
int main( void )
{
	SPMIDI_AudioDevice device;
	StreamIO *stream;
	AudioSample ASMP;
	AudioSample *asmp;
	IMA_WAV_Coder IMADEC, *imacod;

//	char *inputFileName = "E:\\nomad\\MIDISynth\\test\\short_11025_ima.wav";
	char *inputFileName = "E:\\nomad\\MIDISynth\\test\\chewymonkeys_11025_ima.wav";
	int   fileSize;
	void *data = NULL;

	int numSamples;
	int result;

	int err;

	asmp = &ASMP;
	imacod = &IMADEC;

	SPMIDI_HostInit();

	/* Get encoded data from somewhere. */
	data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
	if( data == NULL )
	{
		printf("ERROR could not find MIDI File %s\n", inputFileName );
		goto error;
	}

	stream = Stream_OpenImage( (char *)data, fileSize );
	if( stream == NULL )
	{
		printf("ERROR: Stream_OpenImage() failed.\n" );
		goto error;
	}
	
	err = Audio_WAV_ParseSampleStream(
	    stream,       /* Stream access to WAV file. */
	    asmp          /* Pre-allocated but empty structure to be completely filled in by parser. */
	);
	if( err < 0 )
	{
		printf("ERROR: Audio_WAV_ParseSampleStream returned %d\n", err );
		goto error;
	}
	
	if( asmp->format != AUDIO_FORMAT_IMA_ADPCM_WAV )
	{
		printf("Expected IMA ADPCM file!\n", asmp->format );
		goto error;
	}

/* Set up structure used for IMA ADPCM decoder. */
	result = IMA_WAV_InitializeCoder( imacod, asmp->samplesPerBlock, stream );
	if( result < 0 )
	{
		printf("Error setting up decoder = 0x%x\n", result );
		goto error;
	}

	/* Position cursor to start of encoded data in stream. */
	stream->setPosition( stream, asmp->dataOffset );

	result = SPMUtil_StartAudio( &device, SAMPLE_RATE, SAMPLES_PER_FRAME );
	if( result < 0 )
	{
		goto error;
	}

	/* Play decoded samples until we run out of data. */
	while( (numSamples = IMA_WAV_DecodeNextBlock( imacod, decodedSamples, SHORTS_PER_BUFFER )) > 0 )
	{
		SPMUtil_WriteAudioBuffer( device, decodedSamples, numSamples );
	}

	SPMUtil_StopAudio( device );

	printf( "SPMIDI bytes allocated before Stream_Close  = %d\n", SPMIDI_GetMemoryBytesAllocated() );
	Stream_Close( stream );
	IMA_WAV_TerminateCoder( imacod );
	printf( "SPMIDI bytes allocated after cleanup  = %d\n", SPMIDI_GetMemoryBytesAllocated() );

	SPMIDI_HostTerm();
	return 0;

error:
	printf("Error exit.\n");
	SPMIDI_HostTerm();
	return 1;
}
