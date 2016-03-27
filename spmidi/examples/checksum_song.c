/* $Id: checksum_song.c,v 1.4 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Process a file using the SongPlayer API.
 * Do not play the song using audio, just generate a checksum.
 *
 * In order for song_player support to be compiled,
 * The compiler flag SPMIDI_ME3000 must be defined as (1).
 *
 * Based on play_song.c MIDI file player example.
 *
 * Author: Phil Burk, Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_errortext.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/song_player.h"



//#define SONG_IMAGE midiFileImage
//#define SONG_IMAGE_SIZE midiFileImage_size

// link with "examples/TalkinReggae_mxmf.c"
#define SONG_IMAGE gTalkinReggaeData
#define SONG_IMAGE_SIZE gTalkinReggaeData_size

/*
 * Compile this program with a file containing a 
 * Standard MIDI File that is stored in a char array,
 * for example "UpAndDown.c". Eventually you can replace this with 
 * code thats load the data from a file or a database.
 */
extern const unsigned char SONG_IMAGE[];
extern int SONG_IMAGE_SIZE;

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)

static spmSInt32 numFramesSynthesized = 0;

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static spmUInt32 ChecksumAudioBuffer(SPMIDI_Context *spmidiContext)
{
	int i;
	spmUInt32 checksum = 0;

	/* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)

	short samples[SAMPLES_PER_BUFFER];

	/* Generate a buffer full of audio data as 16 bit samples. */
	SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
	                   SAMPLES_PER_FRAME, 16 );
	numFramesSynthesized += FRAMES_PER_BUFFER;

	for( i=0; i<SAMPLES_PER_BUFFER; i++ )
	{
		int shifter = i & 7;
		checksum += (samples[i] << shifter);
	}

	return checksum;
}


/****************************************************************/
/**
 * Play a song one audio buffer at a time.
 */
static int SongPlayer_Play( SongPlayer *songPlayer, SPMIDI_Context *spmidiContext,
                            int numLoops, spmUInt32 *checksumPtr )
{
	int result;
	int timeout;
	int go = 1;
	spmUInt32 checksum = 0;

	/* Start the songplayer */
	result = SongPlayer_Start( songPlayer );
	if( result < 0 )
		goto error;

	/*
	 * Process one buffer worth of MIDI data each time through the loop.
	 */
	while ( go )
	{
		if( SongPlayer_PlayFrames( songPlayer, FRAMES_PER_BUFFER ) == 0 )
		{
			checksum += ChecksumAudioBuffer(spmidiContext);
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
		checksum += ChecksumAudioBuffer(spmidiContext);
	}

	/* Stop playing */
	result = SongPlayer_Stop( songPlayer );

	*checksumPtr = checksum;
	return result;

error:
	return result;
}

#if 1
/*******************************************************************/
int main(void)
{
	SPMIDI_Error   result;
	SPMIDI_Context *spmidiContext = NULL;
	SongPlayer     *songPlayerContext = NULL;
	int             numLoops = 1;
	spmUInt32        checksum = 0;


	PRTMSG( "play_song: play song file\n" );

	/* Start SP-MIDI synthesis engine using the desired sample rate. */
	result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		goto error;

	/* Create a player for the song */
	result = SongPlayer_Create( &songPlayerContext, spmidiContext, (unsigned char *) SONG_IMAGE, SONG_IMAGE_SIZE );
	if( result < 0 )
		goto error;

	SPMIDI_ResetMaxNoteCount( spmidiContext );

	/* Play the file */
	result = SongPlayer_Play( songPlayerContext, spmidiContext, numLoops, &checksum );
	if( result < 0 )
		goto error;

	PRTMSG("Song playback complete.\n");
	PRTMSGNUMD("max voices used = ", SPMIDI_GetMaxNoteCount( spmidiContext ) );
	PRTMSGNUMD("numFramesSynthesized = ", numFramesSynthesized);
	PRTMSGNUMH("Checksum = ", checksum);

error:
	if( songPlayerContext != NULL )
		SongPlayer_Delete( songPlayerContext );

	/* Terminate SP-MIDI synthesizer. */
	if( spmidiContext != NULL )
		SPMIDI_DeleteContext(spmidiContext);


	if( result < 0 )
	{
		PRTMSG("Error playing song = ");
		PRTNUMD( result );
		PRTMSG( SPMUtil_GetErrorText(result) );
		PRTMSG("\n");
	}

	return (result < 0);
}

#endif
