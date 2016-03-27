/* $Id: minimal_player_pa.c,v 1.7 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Play a MIDI file using the PortAudio callback interface.
 *
 * Please note that if the Mobileer synthesizer is playing
 * in a background thread or interrupt then you should be very careful
 * about calling SPMIDI functions from another thread.
 * In general the SPMIDI routines are not thread-safe so you 
 * should use appropriate synchronization techniques such as
 * semaphores or command queues when controlling the SPMIDI code.
 *
 * This file is to assist integration on target systems.
 * It demonstrates the basic techniques and is intended as
 * a starting point for platform specific implementation.
 *
 * Author: Phil Burk
 * Copyright 2002-4 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/midifile_player.h"

/* PortAudio is an open-source audio API available free from www.portaudio.com */
#include "portaudio.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)
#define BITS_PER_SAMPLE     (sizeof(short)*8)
/* In your program you can repeat as long as you want.
 * In a mobile phohne you would repeat the song until
 * the user answered the phone, or it switched to 
 * recording the voice message.
 */
#define MAX_REPEATS         (4)
/*
 * We may need to make our audio buffer larger than the SPMIDI buffer size.
 * Otherwise we may get an audio underflow on some devices.
 */
#define AUDIO_FRAMES_PER_BUFFER   (8 * SPMIDI_MAX_FRAMES_PER_BUFFER)

/* Include a MIDIfile in the form of a byte array. */
#include "UpAndDown.c"

/*
 * Compile this program with a file containing a 
 * Standard MIDI File that is stored in a char array,
 * for example "UpAndDown.c". Eventually you can replace this with 
 * code thats loads the data from a file or a database.
 */
extern const unsigned char midiFileImage[];
extern int midiFileImage_size;

/*
 * Store all application data in one structure so it can
 * be passed to the callback function.
 */
typedef struct MyPlayerData_s
{
	MIDIFilePlayer  *player;
	SPMIDI_Context  *spmidiContext;
	PortAudioStream *stream;
	int              numRepeats;
}
MyPlayerData_t;

/****************************************************************/
/**
 * Use SPMIDI to synthesize a buffer full of audio.
 * This function is called from the audio system software
 * when it needs a buffer full of audio.
 */

int MyPlayerCallback(
    void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    PaTimestamp outTime, void *userData )
{
	/* Use information passed from foreground thread. */
	MyPlayerData_t *playerData = (MyPlayerData_t *) userData;
	int framesLeft = framesPerBuffer;
	int returnCode = 0;
	short *outputPtr = (short *) outputBuffer;
	(void) inputBuffer;
	(void) outTime;

	/* The audio buffer is bigger than the synthesizer buffer so we
	 * have to call the synthesizer several times to fill it.
	 */
	while( framesLeft )
	{
		if( playerData->numRepeats < MAX_REPEATS )
		{
			int result = MIDIFilePlayer_PlayFrames( playerData->player,
			                                        playerData->spmidiContext, SPMIDI_MAX_FRAMES_PER_BUFFER );
			/* Play one buffer worth of MIDI data. */
			if( result != 0 )
			{
				/* Repeat the song by rewinding the MIDI file. */
				MIDIFilePlayer_Rewind( playerData->player );
				playerData->numRepeats += 1;
			}
		}

		/* Generate a buffer full of audio data as 16 bit samples. */
		result = SPMIDI_ReadFrames( playerData->spmidiContext, outputPtr, SPMIDI_MAX_FRAMES_PER_BUFFER,
		                   SAMPLES_PER_FRAME, BITS_PER_SAMPLE );
		if( result < 0 ) return result;

		/* Advance pointer to next part of large output buffer. */
		outputPtr += SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER;

		/* Calculate how frames are remaining. */
		framesLeft -= SPMIDI_MAX_FRAMES_PER_BUFFER;
	}

	return returnCode;
}

/****************************************************************/
/**
 * Play a MIDI file one audio buffer at a time.
 */
static int MIDIFile_Play( const unsigned char *image, int numBytes )
{
	int result;
	MyPlayerData_t PLAYERDATA = {0};
	MyPlayerData_t *playerData = &PLAYERDATA;

	SPMIDI_Initialize();

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &playerData->player, (int) SAMPLE_RATE, image, numBytes );
	if( result < 0 )
		goto error1;

	/* Start SP-MIDI synthesis engine using the desired sample rate. */
	result = SPMIDI_CreateContext( &playerData->spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		goto error2;

	/* Initialize audio hardware and open an output stream. */
	Pa_Initialize();
	result = Pa_OpenDefaultStream( &playerData->stream,
	                               0, SAMPLES_PER_FRAME,
	                               paInt16,
	                               (double) SAMPLE_RATE,
	                               AUDIO_FRAMES_PER_BUFFER,
	                               0,
	                               MyPlayerCallback,
	                               playerData );
	if( result < 0 )
	{
		PRTMSG( "Pa_OpenDefaultStream returns " );
		PRTMSG( Pa_GetErrorText( result ) );
		PRTMSG( "\n" );

		goto error3;
	}

	playerData->numRepeats = 0;

	Pa_StartStream( playerData->stream );

	/* In your program you can repeat as long as you want.
	 * In a mobile phone you would repeat the song until
	 * the user answered the phone, or it switched to 
	 * recording the voice message.
	 */
	while( playerData->numRepeats < MAX_REPEATS )
		Pa_Sleep(100);

	Pa_StopStream( playerData->stream );

	/* Terminate SP-MIDI synthesizer. */
error3:
	Pa_Terminate();
	SPMIDI_DeleteContext(playerData->spmidiContext);

	/* Free any data structures allocated for playing the MIDI file. */
error2:
	MIDIFilePlayer_Delete( playerData->player );

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

	PRTMSG("Play MIDI File using Mobileer Synthesizer.\n");
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

