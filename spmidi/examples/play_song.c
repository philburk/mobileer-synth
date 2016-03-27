/* $Id: play_song.c,v 1.27 2007/10/02 17:24:58 philjmsl Exp $ */
/**
 *
 * Play a file using the SongPlayer API.
 * This can automatically detect and play either SMF or XMF files.
 *
 * In order for song_player support to be compiled,
 * The compiler flag SPMIDI_ME3000 must be defined as (1).
 *
 * Author: Phil Burk, Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/song_player.h"
#include "spmidi/examples/midifile_names.h"

#ifndef DEFAULT_FILENAME
#define DEFAULT_FILENAME  ("carumba.mxmf")
//#define DEFAULT_FILENAME  ("AllTypesTest.mxmf")
//#define DEFAULT_FILENAME  ("VoiceLa.mxmf")
//#define DEFAULT_FILENAME  ("QuickAllTypes.mxmf")
//#define DEFAULT_FILENAME  ("ZipKit.mxmf")
//#define DEFAULT_FILENAME  ("FurryLisa_rt.mid")
#endif

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define SHOW_VOICES         (0)

#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)

#define SHOW_SIZE(msg,size) /* {printf( msg "%d\n", (size) );} */

/* Host independant writable audio device. */
static SPMIDI_AudioDevice  sHostAudioDevice;
static spmUInt32 sCheckSum = 0;

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static void PlayAudioBuffer(SPMIDI_Context *spmidiContext)
{
	int i;
	
	/* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
	short samples[SAMPLES_PER_BUFFER];

	/* Generate a buffer full of audio data as 16 bit samples. */
	SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
	                   SAMPLES_PER_FRAME, 16 );

	/* Generate CheckSum for audio data. */
	for( i=0; i<SAMPLES_PER_BUFFER; i++ )
	{
		int shifter = i & 7;
		sCheckSum += (samples[i] << shifter);
	}
	
	/* Write audio samples to the audio device. */
	SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, FRAMES_PER_BUFFER );
}


/****************************************************************/
/**
 * Play a song one audio buffer at a time.
 */
int SongPlayer_Play( SongPlayer *songPlayer, SPMIDI_Context *spmidiContext, int numLoops )
{
	int result;
	int timeout;
	int go = 1;
#if SHOW_VOICES
	int loopCount = 0;
#endif

	/* Start the songplayer */
	result = SongPlayer_Start( songPlayer );
	if( result < 0 )
	{
		goto error;
	}
	sCheckSum = 0;
	
	/*
	 * Process one buffer worth of MIDI data each time through the loop.
	 */
	while ( go )
	{
#if SHOW_VOICES	    
		if( (loopCount & 63) == 0 )
		{
			int ic;
			PRTMSG( "#" );
			PRTNUMD( loopCount );
			PRTMSG( ", --- " );
			for( ic=0; ic<16; ic++ )
			{
				int numNotes = SPMIDI_GetChannelActiveNoteCount( spmidiContext, ic );
				if( numNotes > 0 )
				{
					
					PRTNUMD( ic );
					PRTMSG( ":" );
					PRTNUMD( numNotes );
					PRTMSG( ", " );
				}
			}
			PRTMSG( ", T = " );
			PRTNUMD( SPMIDI_GetActiveNoteCount( spmidiContext ) );
			PRTMSG( "\n" );
		}
		loopCount += 1;
#endif
		
		if( SongPlayer_PlayFrames( songPlayer, FRAMES_PER_BUFFER ) == 0 )
		{
			PlayAudioBuffer(spmidiContext);
		}
		else
		{
			if( --numLoops <= 0 )
			{
				go = 0;
			}
			else
			{
				/* Rewind song. */
				SPMUtil_Reset( spmidiContext );
				result = SongPlayer_Rewind( songPlayer );
				if( result < 0 )
					goto error;
			}
		}
		
	}

	/*
	* Continue playing until all of the notes have finished sounding,
	* or for one second, whichever is shorter.
	*/
	timeout = SPMIDI_GetSampleRate( spmidiContext ) / FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		PlayAudioBuffer(spmidiContext);
	}

	
	PRTMSG( "play_song: max voices = " );
	PRTNUMD( SPMIDI_GetMaxNoteCount( spmidiContext ) );
	PRTMSG( "\n" );
	
	PRTMSG( "play_song: checksum = " );
	PRTNUMH( sCheckSum );
	PRTMSG( "\n" );

	/* Stop playing */
	result = SongPlayer_Stop( songPlayer );

error:
	return result;
}

/****************************************************************/
static void usage( void )
{
	printf("play_song [-nNUMREPS] fileName\n");
	fflush(stdout);
}

#if 1
/*******************************************************************/
int main(int argc, char **argv);
int main(int argc, char **argv)
{
	int             result;
	SPMIDI_Context *spmidiContext = NULL;
	SongPlayer     *songPlayerContext = NULL;
	unsigned char  *fileStart;
	int             fileSize;
	int             numLoops = 1;
	int             i;

	char *fileName = DEFAULT_FILENAME;

	/* Parse command line. */
	for( i=1; i<argc; i++ )
	{
		char *s = argv[i];
		if( s[0] == '-' )
		{
			switch( s[1] )
			{
			case 'n':
				numLoops = atoi( &s[2] );
				break;
			case 'h':
			case '?':
				usage();
				return 0;
			}
		}
		else
		{
			fileName = argv[i];
		}
	}
	
	PRTMSG( "play_song: play " ); PRTMSG( fileName ); PRTMSG( "\n" );

	/* Load the file into memory */
	fileStart = SPMUtil_LoadFileImage( fileName, &( fileSize ) );
	if( fileStart == NULL )
	{
		printf("ERROR: file %s not found.\n", fileName );
		return 1;
	}

	SHOW_SIZE( "file size = ", fileSize );

	SPMIDI_Initialize();
	SHOW_SIZE("after SPMIDI_Initialize() = ", SPMIDI_GetMemoryBytesAllocated() );

	/* Start synthesis engine with default number of voices. */
	result = SPMIDI_CreateContext(  &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
	{
		goto error;
	}
	SHOW_SIZE("after SPMIDI_CreateContext() = ", SPMIDI_GetMemoryBytesAllocated() );


	/* Create a player for the song */
	result = SongPlayer_Create( &songPlayerContext, spmidiContext, fileStart, fileSize );
	if( result < 0 )
		goto error;
	SHOW_SIZE("after SongPlayer_Create() = ", SPMIDI_GetMemoryBytesAllocated() );

	result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
	if( result < 0 )
	{
		goto error;
	}

	/* Play the file */
	result = SongPlayer_Play( songPlayerContext, spmidiContext, numLoops );
	if( result < 0 )
		goto error;

error:
	/* Clean everything up */
	if( songPlayerContext != NULL )
		SongPlayer_Delete( songPlayerContext );
		
	if( sHostAudioDevice != NULL )
		SPMUtil_StopAudio( sHostAudioDevice );
		
	if( spmidiContext != NULL )
		SPMIDI_DeleteContext(spmidiContext);

	SHOW_SIZE("before SPMIDI_Terminate() = ", SPMIDI_GetMemoryBytesAllocated() );
	if( SPMIDI_GetMemoryBytesAllocated() > 0 )
	{
		PRTMSGNUMD("ERROR - memory not freed!!!!!!!!!!! bytes = ", SPMIDI_GetMemoryBytesAllocated() );
	}

	SPMIDI_Terminate();

	SPMUtil_FreeFileImage( fileStart );

	if( result < 0 )
	{
		PRTMSG("Error playing file = ");
		PRTNUMD( result );
		PRTMSG( SPMUtil_GetErrorText( (SPMIDI_Error)result ) );
		PRTMSG("\n");
	}

	return (result < 0);
}

#endif
