/* $Id: minimal_player.c,v 1.16 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play a MIDI file on an audio device that uses blocking writes.
 * The MIDI song file is loaded from a 'C' source array so a
 * file system is not needed.
 *
 * The source arrays are generated using a tool called "bintoc",
 * which is available from Mobileer.
 *
 * This file is to assist integration on target systems.
 * It demonstrates the basic techniques and is intended as
 * the starting point for a platform specific implementation.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/midifile_player.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)

/* 1 for mono, 2 for stereo */
#define SAMPLES_PER_FRAME   (1)

/* How many times do we play the song? */
#define NUM_REPETITIONS   (2)

#define FRAMES_PER_BUFFER   (SPMIDI_GetFramesPerBuffer())

/*
 * Compile this program with Standard MIDI File that is stored in a char array,
 * for example "songs/song_UpAndDown_rt.h". Eventually you can replace this with 
 * code that loads the data from a file or other type of storage.
 */
/* Select a ringtone by setting its 0 to a 1, and the others to 0. */

#if 0
#include "songs/song_FurryLisa.h"
const unsigned char *midiFileImage = song_FurryLisa;
int midiFileImage_size = sizeof(song_FurryLisa);

#elif 0
#include "songs/song_UpAndDown_rt.h"
const unsigned char *midiFileImage = song_UpAndDown_rt;
int midiFileImage_size = sizeof(song_UpAndDown_rt);

#elif 0
#include "songs/song_Bach_Sonata3EMajor.h"
const unsigned char *midiFileImage = song_Bach_Sonata3EMajor;
int midiFileImage_size = sizeof(song_Bach_Sonata3EMajor);

#elif 0
#include "songs/song_FunToy8.h"
const unsigned char *midiFileImage = song_FunToy8;
int midiFileImage_size = sizeof(song_FunToy8);

#else
/* Externally linked file. */
extern const unsigned char midiFileImage[];
extern int midiFileImage_size;
#endif

/* Host independant writable audio device. */
SPMIDI_AudioDevice  sHostAudioDevice;

short sAudioBuffer[SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER];
	
/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static int PlayAudioBuffer(SPMIDI_Context *spmidiContext)
{
	int result;
	
	/* Generate a buffer full of audio data as 16 bit samples. */
	result = SPMIDI_ReadFrames( spmidiContext, sAudioBuffer, FRAMES_PER_BUFFER,
	                   SAMPLES_PER_FRAME, 16 );
	if( result < 0 ) return result;
	                   
	/* Write audio samples to the audio device. */
	return SPMUtil_WriteAudioBuffer( sHostAudioDevice, sAudioBuffer, FRAMES_PER_BUFFER );
}

/****************************************************************/
/**
 * Play the song several times in a loop.
 */
static int MIDIFile_Loop( SPMIDI_Context *spmidiContext, MIDIFilePlayer *player, int numLoops )
{
	int i;
	int timeout;
	int result;
	int go;
	
	for( i=0; i<numLoops; i++ )
	{
		/* Reset MIDI programs, controllers, etc. */
		SPMUtil_Reset( spmidiContext );
		
		/* Rewind song to beginning. */
		MIDIFilePlayer_Rewind( player );
		
		/*
		 * Play the song once.
		 * Process one buffer's worth of MIDI data each time through the loop.
		 */
		go = 1;
		while( go )
		{
			/* Play a small bit of the MIDI file. */
			result = MIDIFilePlayer_PlayFrames( player, spmidiContext, FRAMES_PER_BUFFER );
			if( result > 0 )
			{
				/* Song has finished. */
				go = 0;
			}
			else if( result < 0 )
			{
				/* Error has occured. */
				return result;
			}
			else
			{
				/* Still in the middle of the song so synthesize a buffer full of audio. */
				result = PlayAudioBuffer(spmidiContext);
				if( result < 0 ) return result;
			}
		}
	}
	
		
	/*
	 * Continue playing until all of the notes have finished sounding,
	 * or for one second, whichever is shorter.
	 */
	timeout = SAMPLE_RATE / FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		result = PlayAudioBuffer(spmidiContext);
		if( result < 0 ) return result;
	}
	
	return 0;
}

/****************************************************************/
/**
 * Play a MIDI file one audio buffer at a time.
 */
static int MIDIFile_Play( const unsigned char *image, int numBytes )
{
	int result;
	MIDIFilePlayer *player;
	SPMIDI_Context *spmidiContext = NULL;

	SPMIDI_Initialize();

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
	if( result < 0 )
		goto error1;

	/* Create an SP-MIDI synthesis engine. */
	result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		goto error2;

	/* Initialize audio device. */
	result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
	if( result < 0 )
		goto error3;

	/* Play the song several times. */
	result = MIDIFile_Loop( spmidiContext, player, NUM_REPETITIONS );
	if( result < 0 )
		goto error2;
	
	/* Close audio hardware. */
	SPMUtil_StopAudio( sHostAudioDevice );

	/* Terminate SP-MIDI synthesizer. */
error3:
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
	(void) argc;
	(void) argv;

	PRTMSG("Play MIDI File from byte array.\n");
	PRTMSG("(C) 2003 Mobileer\n");

	/*
	 * Play the midifile contained in the midiFileImage char array.
	 */
	result = MIDIFile_Play( midiFileImage, midiFileImage_size );
	if( result < 0 )
	{
		PRTMSG("Error playing MIDI File = ");
		PRTNUMD( result );
		PRTMSG( SPMUtil_GetErrorText( result ) );
		PRTMSG("\n");
	}

	PRTMSG("MIDI File playback complete.\n");

	return (result < 0);
}

