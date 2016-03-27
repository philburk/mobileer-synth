/* $Id: checksum_smid.c,v 1.2 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Render a MIDI Stream file and just print the checksum result.
 * Do not use the audio device.
 *
 * This file is to assist integration on target systems.
 * It demonstrates the basic techniques and is intended as
 * a starting point for platform specific implementation.
 *
 * Author: Phil Burk
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/midistream_player.h"

#define USE_SOURCE  (1)

#if USE_SOURCE
#include "UpAndDown_smid.c"
#else
#include "spmidi/include/spmidi_load.h"
#endif

/*
 * Adjust these for your system.
 */
#define FRAMES_PER_SECOND   (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_TICK     (SPMIDI_GetFramesPerBuffer())

#if USE_SOURCE
/*
 * Compile this program with a file containing a 
 * MIDI Stream that is stored in a char array,
 * for example "UpAndDown_smid.c". Eventually you can replace this with 
 * code that loads the data from a file or a database.
 */
extern const unsigned char midiStreamImage[];
extern const int midiStreamImage_size;
#else
unsigned char *midiStreamImage;
int midiStreamImage_size;
#define DEFAULT_FILENAME "UpAndDown_rt.smid"
//#define DEFAULT_FILENAME "TwoTrills.smid"
#endif

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_TICK)
short samples[SAMPLES_PER_FRAME * 64];
//short samples[SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER];

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then checksum that audio for verification.
 */
static int ChecksumNextBuffer( SPMIDI_Context *spmidiContext, int checkSum )
{
	int i;

	/* Generate a buffer full of audio data as 16 bit samples. */
	SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_TICK,
	                   SAMPLES_PER_FRAME, 16 );

	for( i=0; i<SAMPLES_PER_BUFFER; i++ )
	{
		checkSum += samples[i];
	}

	return checkSum;
}

static	MIDIStreamPlayer smidPlayer;

/****************************************************************/
/**
 * Checksum a MIDI file one audio buffer at a time.
 */
static int MIDIFile_Checksum( const unsigned char *image, int numBytes )
{
	int result;
	int timeout;
	SPMIDI_Context *spmidiContext;
	int checkSum = 0;

	SPMIDI_Initialize();

	result = MIDIStreamPlayer_Setup( &smidPlayer, FRAMES_PER_TICK, FRAMES_PER_SECOND,
		image, numBytes );
	if( result < 0 )
	{
		goto error1;
	}

	/* Start SP-MIDI synthesis engine using the desired sample rate. */
	result = SPMIDI_CreateContext( &spmidiContext, FRAMES_PER_SECOND );
	if( result < 0 )
		goto error2;

	/*
	 * Process one buffer worth of MIDI data each time through the loop.
	 */
	while( MIDIStreamPlayer_PlayTick( &smidPlayer, spmidiContext ) == 0 )
	{
		checkSum  = ChecksumNextBuffer( spmidiContext, checkSum );
	}


	/*
	* Continue playing until all of the notes have finished sounding,
	* or for one second, whichever is shorter.
	*/
	timeout = FRAMES_PER_SECOND / FRAMES_PER_TICK;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		checkSum  = ChecksumNextBuffer( spmidiContext, checkSum );
	}
	result = checkSum & 0x7FFFFFFF;

	/* Terminate SP-MIDI synthesizer. */
	SPMIDI_DeleteContext(spmidiContext);

	/* Free any data structures allocated for playing the MIDI Stream. */
error2:

error1:
	SPMIDI_Terminate();
	return result;
}

/****************************************************************/
int main( int argc, char ** argv )
{
	int result;

#if USE_SOURCE
	(void) argc;
	(void) argv;
#else
	int i;
	char *inputFileName = DEFAULT_FILENAME;


	/* Parse command line. */
	for( i=1; i<argc; i++ )
	{
		char *s = argv[i];
		if( s[0] == '-' )
		{
			switch( s[1] )
			{
			default:
				break;
			}
		}
		else
		{
			inputFileName = argv[i];
		}
	}

	/* Load MIDI Stream File into a memory image. */
	midiStreamImage = SPMUtil_LoadFileImage( inputFileName, &midiStreamImage_size );
	if( midiStreamImage == NULL )
	{
		printf("ERROR reading MIDI File.\n" );
		return 1;
	}
#endif

	PRTMSG("Play MIDI Stream from byte array.\n");
	PRTMSG("(C) 2003 Mobileer\n");

	/*
	 * Play the midifile contained in the midiStreamImage char array.
	 */
	result = MIDIFile_Checksum( midiStreamImage, midiStreamImage_size );
	if( result < 0 )
	{
		PRTMSG("Error playing MIDI File = ");
		PRTNUMD( result );
		PRTMSG( SPMUtil_GetErrorText( result ) );
		PRTMSG("\n");
	}
	else
	{
		PRTMSG("Checksum = ");
		PRTNUMH(result);
		PRTMSG("\n");
	}

#if (USE_SOURCE == 0)
	PRTMSG( inputFileName );
	PRTMSG(" file used.\n");
#endif

	PRTMSG("MIDI Stream playback complete.\n");

	return (result < 0);
}

