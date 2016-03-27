/* $Id: play_no_io.c,v 1.2 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Call SPMIDI Library without doing any I/O.
 * This is mainly for determining RAM and ROM sizes.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/midifile_player.h"
#include "UpAndDown.c"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)

/* 1 for mono, 2 for stereo */
#define SAMPLES_PER_FRAME   (1)

#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)


#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
short sAudioBuffer[SAMPLES_PER_BUFFER];
	

/****************************************************************/
/**
 * Play the song several times in a loop.
 */
static int MIDIFile_Render( SPMIDI_Context *spmidiContext, MIDIFilePlayer *player )
{
	int timeout;
	int result;
	int go;
	
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
			result = SPMIDI_ReadFrames( spmidiContext, sAudioBuffer, FRAMES_PER_BUFFER,
                   SAMPLES_PER_FRAME, 16 );
			if( result < 0 ) return result;
		}
	}
	
		
	/*
	 * Continue playing until all of the notes have finished sounding,
	 * or for one second, whichever is shorter.
	 */
	timeout = SAMPLE_RATE / FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		/* Still in the middle of the song so synthesize a buffer full of audio. */
		result = SPMIDI_ReadFrames( spmidiContext, sAudioBuffer, FRAMES_PER_BUFFER,
               SAMPLES_PER_FRAME, 16 );
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
	SPMIDI_Context *spmidiContext = (SPMIDI_Context *) 0;

	SPMIDI_Initialize();

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
	if( result < 0 )
		goto error1;

	/* Create an SP-MIDI synthesis engine. */
	result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		goto error2;

	result = MIDIFile_Render( spmidiContext, player );
	if( result < 0 )
		goto error2;

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
	(void) argc;
	(void) argv;

	/*
	 * Play the midifile contained in the midiFileImage char array.
	 */

	return MIDIFile_Play( midiFileImage, midiFileImage_size );
}

