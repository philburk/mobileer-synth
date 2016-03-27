/* $Id: play_song_throttle.c,v 1.3 2007/10/10 00:24:04 philjmsl Exp $ */
/**
 *
 * Play a file using the SongPlayer API.
 * In this version we measure the CPU load and adjust
 * the number of voices so that we do not exceed a specified limit
 * for CPU load.
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

/*
 * Adjust these for your system.
 */

/* Maximum desired percent CPU load for MIDI synthesis. */
#define MAX_CPU_LOAD        (50)

#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   ((20 * 1024) / (SAMPLES_PER_FRAME * sizeof(short)) )
#define BITS_PER_SAMPLE     (16)
#define NUM_REPETITIONS     (2)
#define MIN_VOICES          (8)

/* Host independant writable audio device. */
static SPMIDI_AudioDevice  sHostAudioDevice;

#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
short samples[SAMPLES_PER_BUFFER];

/****************************************************************/
/********** CPU Load Control Functions *************************/
/****************************************************************/
/* These three functions need to be implemented for your system. */
int SPMIDICPU_ReadClockStart( void );
int SPMIDICPU_ReadClockStop( int averageVoiceCount );
int SPMIDICPU_GetClockRate( void );

/********* BEGIN FAKE CLOCK IMPLEMENTATION ****************************/
/* Thse are fake implementations of these functions for testing.
 * This is designed to simulate a real clock.
 */
#define FAKE_CPU_MHZ              (100)
#define FAKE_TICKS_PER_SECOND (1000000)
#define FAKE_MHZ_PER_VOICE          (3)
int sLastClock = 0;

/**
 * Return clock rate in Hz for SPMIDICPU_ReadClock().
 * The clock can be any timer in the system that has a steady tick.
 * Also it should be faster than the audio sample rate
 * and slower than the system clock rate.
 */
int SPMIDICPU_GetClockRate( void )
{
	return FAKE_TICKS_PER_SECOND;
}

/**
 * Fake implementation.
 * @return current tick value for hardware timer.
 */
int SPMIDICPU_ReadClockStart( void )
{
	/* Fake clock. In your function you should read a hardware timer. */
	sLastClock += 10000;
	return sLastClock;
}

/**
 * Fake implementation.
 * @return current tick value for hardware timer.
 */
int SPMIDICPU_ReadClockStop( int averageVoiceCount )
{
	/* I am only passing the averageVoiceCount so I can simulate an
	 * elapsed time that is proportional to the number of active voices. *
	/* Estimate how many ticks that would have elapsed. */

	int numMHzForSynth = FAKE_MHZ_PER_VOICE * averageVoiceCount;

	/* Calculate ticks in one buffers worth of audio time. */
	int ticksPerBuffer = (FRAMES_PER_BUFFER * SPMIDICPU_GetClockRate()) / SAMPLE_RATE;
	int ticksElapsed = ticksPerBuffer * numMHzForSynth / FAKE_CPU_MHZ;
	sLastClock += ticksElapsed;
	return sLastClock;

}
/********* END FAKE CLOCK IMPLEMENTATION *********************************/

/*
 * Adjust MaxVoices for Synthesizer based on measured time.
 * If you implement the clock functions then you could use this
 * in an application.
 */
int SPMIDICPU_AdjustMaxVoices( SPMIDI_Context *spmidiContext,
							  int ticksElapsed,
							  int averageVoiceCount,
							  int sampleRate,
							  int framesGenerated,
							  int maxPercentLoad)
{
	int maxVoiceCount;

	/* Calculate ticks in one buffers worth of audio time. */
	int ticksPerBuffer = (framesGenerated * SPMIDICPU_GetClockRate()) / sampleRate;
	/* Calculate measured percent CPU loaded. */
	int actualPercentLoad = ticksElapsed * 100 / ticksPerBuffer;

	/* Calculate maximum voice count that would not exceed maxPercentLoad. */
	if( actualPercentLoad < 1 ) actualPercentLoad = 1;
	maxVoiceCount = averageVoiceCount * maxPercentLoad / actualPercentLoad;

	/* Make sure actual voice count is not out of reasonable range. */
	if( maxVoiceCount > SPMIDI_MAX_VOICES )
	{
		maxVoiceCount = SPMIDI_MAX_VOICES;
	}
	else if( maxVoiceCount < MIN_VOICES )
	{
		maxVoiceCount = MIN_VOICES;
	}

	PRTMSGNUMD("----\nticksElapsed = ", ticksElapsed );
	PRTMSGNUMD("averageVoiceCount = ", averageVoiceCount );
	PRTMSGNUMD("maxVoiceCount = ", maxVoiceCount );
	return SPMIDI_SetMaxVoices( spmidiContext, maxVoiceCount );
}


/****************************************************************/
/**
 * Use ME3000 to synthesize a buffer full of audio.
 * We assume buffer is a multiple of SPMIDI_MAX_FRAMES_PER_BUFFER.
 * @return 1 if done
 */

static int SynthesizeLargeBuffer( SongPlayer *songPlayer, SPMIDI_Context *spmidiContext, int numLoops )
{
	int framesLeft = FRAMES_PER_BUFFER;
	int result = 0;
	int numFramesRead;
	short *outputPtr = (short *) samples;

	int voiceCountSum = 0;
	int numVoiceCounts = 0;
	int tickStart;
	int tickStop;

	/* Read clock before filling buffer. */
	tickStart = SPMIDICPU_ReadClockStart( );

	/* The audio buffer is bigger than the synthesizer buffer so we
	 * have to call the synthesizer several times to fill it.
	 */
	while( framesLeft > 0 )
	{
		/* Measure average number of active voices. */
		voiceCountSum += SPMIDI_GetActiveNoteCount( spmidiContext );
		numVoiceCounts += 1;

		if( numLoops > 0 )
		{
			/* Play one buffer worth of MIDI data. */
			result = SongPlayer_PlayFrames( songPlayer, SPMIDI_GetFramesPerBuffer() );
			//printf("KanaMIDI_SynthesizeBuffer: framesLeft = %d\n", framesLeft );
			if( result > 0 )
			{
				numLoops -= 1;
				if( numLoops > 0 )
				{
					//printf("KanaMIDI_SynthesizeBuffer: rewind, rep %d\n", midiInternal.repeatCount );
					/* Repeat the song by rewinding the MIDI file. */
					SongPlayer_Rewind( songPlayer );
				}
			}
			else if( result < 0 )
			{
				goto error;
			}
		}

		/* Generate (synthesize) a buffer full of audio data as 16 bit samples.
		 * When there are no notes left it will write zeros.
		 */
		numFramesRead = SPMIDI_ReadFrames( spmidiContext, outputPtr, SPMIDI_GetFramesPerBuffer(),
		                            SAMPLES_PER_FRAME, BITS_PER_SAMPLE );
		if( numFramesRead < 0 )
		{
			result = numFramesRead;
			goto error;
		}

		/* Advance pointer to next part of large output buffer. */
		outputPtr += SAMPLES_PER_FRAME * numFramesRead;

		/* Calculate how frames are remaining. */
		framesLeft -= numFramesRead;

	}

	/* Measure elapsed time and adjust max voices. */
	if( numVoiceCounts > 0 )
	{
		int averageVoiceCount;
		int ticksElapsed;

		/* Calculate average voice count for the above loop. */
		averageVoiceCount = voiceCountSum / numVoiceCounts;

		/* Read clock before filling buffer. */
		tickStop = SPMIDICPU_ReadClockStop( averageVoiceCount );
		/* It doesn't matter if the clock wrapped between measurements
		 * because we are difference between the two times.
		 */
		ticksElapsed = tickStop - tickStart;
		

		SPMIDICPU_AdjustMaxVoices( spmidiContext,
							  ticksElapsed,
							  averageVoiceCount,
							  SAMPLE_RATE,
							  FRAMES_PER_BUFFER,
							  MAX_CPU_LOAD );
	}

	return numLoops;

error:
	return result;
}

/****************************************************************/
/**
 * Play a song one audio buffer at a time.
 */
int SongPlayer_Play( SongPlayer *songPlayer, SPMIDI_Context *spmidiContext, int numLoops )
{
	int result;
	int timeout;

	/* Start the songplayer */
	result = SongPlayer_Start( songPlayer );
	if( result < 0 )
		goto error;

	/*
	 * Process one buffer worth of MIDI data each time through the loop.
	 */
	while ( numLoops > 0 )
	{
		/* Generate large buffer full of audio. */
		numLoops = SynthesizeLargeBuffer( songPlayer, spmidiContext, numLoops );

		/* Write audio samples to the audio device. */
		SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, FRAMES_PER_BUFFER );
	}

	/*
	* Continue playing until all of the notes have finished sounding,
	* or for one second, whichever is shorter.
	*/
	timeout = SPMIDI_GetSampleRate( spmidiContext ) / FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		/* Generate large buffer full of audio. */
		numLoops = SynthesizeLargeBuffer( songPlayer, spmidiContext, numLoops );

		/* Write audio samples to the audio device. */
		SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, FRAMES_PER_BUFFER );
	}

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
	int             numLoops = NUM_REPETITIONS;
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


	SPMIDI_Initialize();

	/* Start synthesis engine with default number of voices. */
	result = SPMIDI_CreateContext(  &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
	{
		goto error;
	}


	/* Create a player for the song */
	result = SongPlayer_Create( &songPlayerContext, spmidiContext, fileStart, fileSize );
	if( result < 0 )
	{
		goto error;
	}

	result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
	if( result < 0 )
	{
		goto error;
	}


	/* Play the file */
	result = SongPlayer_Play( songPlayerContext, spmidiContext, numLoops );
	if( result < 0 )
		goto error;

	PRTMSG("File playback complete.\n");

error:
	/* Clean everything up */
	if( songPlayerContext != NULL )
		SongPlayer_Delete( songPlayerContext );
		
	if( sHostAudioDevice != NULL )
		SPMUtil_StopAudio( sHostAudioDevice );
		
	if( spmidiContext != NULL )
		SPMIDI_DeleteContext(spmidiContext);


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
