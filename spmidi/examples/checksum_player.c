/* $Id: checksum_player.c,v 1.9 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Render a MIDI file and just print the checksum result.
 * Do not use the audio device.
 *
 * This file is to assist integration on target systems.
 * It demonstrates the basic techniques and is intended as
 * a starting point for platform specific implementation.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/midifile_player.h"

#define USE_SOURCE  (1)

#if USE_SOURCE
#include "UpAndDown.c"
#else
#include "spmidi/include/spmidi_load.h"
#endif

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (SPMIDI_GetFramesPerBuffer())

#if USE_SOURCE
/*
 * Compile this program with a file containing a 
 * Standard MIDI File that is stored in a char array,
 * for example "UpAndDown.c". Eventually you can replace this with 
 * code thats load the data from a file or a database.
 */
extern const unsigned char midiFileImage[];
extern const int midiFileImage_size;
#else
unsigned char *midiFileImage;
int midiFileImage_size;
#define DEFAULT_FILENAME "UpAndDown_rt.mid"
//#define DEFAULT_FILENAME "TwoTrills.mid"
#endif

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
short samples[SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER];

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then checksum that audio for verification.
 */
static int ChecksumNextBuffer( SPMIDI_Context *spmidiContext, int checkSum )
{
	int i;

	/* Generate a buffer full of audio data as 16 bit samples. */
	SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
	                   SAMPLES_PER_FRAME, 16 );

	for( i=0; i<SAMPLES_PER_BUFFER; i++ )
	{
		checkSum += samples[i];
	}

	return checkSum;
}

/****************************************************************/
/**
 * Checksum a MIDI file one audio buffer at a time.
 */
static int MIDIFile_Checksum( const unsigned char *image, int numBytes )
{
	int result;
	int timeout;
	MIDIFilePlayer *player;
	SPMIDI_Context *spmidiContext;
	int checkSum = 0;

	SPMIDI_Initialize();

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
	if( result < 0 )
		goto error1;

	/* Start SP-MIDI synthesis engine using the desired sample rate. */
	result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		goto error2;

	/*
	 * Process one buffer worth of MIDI data each time through the loop.
	 */
	while( MIDIFilePlayer_PlayFrames( player, spmidiContext, FRAMES_PER_BUFFER ) == 0 )
	{
		checkSum  = ChecksumNextBuffer( spmidiContext, checkSum );
	}

	/*
	* Continue playing until all of the notes have finished sounding,
	* or for one second, whichever is shorter.
	*/
	timeout = SAMPLE_RATE / FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		checkSum  = ChecksumNextBuffer( spmidiContext, checkSum );
	}
	result = checkSum & 0x7FFFFFFF;

	/* Terminate SP-MIDI synthesizer. */
	SPMIDI_DeleteContext(spmidiContext);

	/* Free any data structures allocated for playing the MIDI file. */
error2:
	MIDIFilePlayer_Delete( player );

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
	midiFileImage = SPMUtil_LoadFileImage( inputFileName, &midiFileImage_size );
	if( midiFileImage == NULL )
	{
		printf("ERROR reading MIDI File.\n" );
		return 1;
	}
#endif

	PRTMSG("Play MIDI File from byte array.\n");
	PRTMSG("(C) 2003 Mobileer\n");

	/*
	 * Play the midifile contained in the midiFileImage char array.
	 */
	result = MIDIFile_Checksum( midiFileImage, midiFileImage_size );
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

	PRTMSG("MIDI File playback complete.\n");

	return (result < 0);
}

