/* $Id: jukebox_demo.c,v 1.6 2007/10/02 16:15:32 philjmsl Exp $ */
/**
 *
 * Use JukeBox to play songs in a queue
 * and play notes scheduled precisely in time.
 * Copyright 2004 Mobileer, Inc., PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <conio.h>

#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_jukebox.h"

/* PortAudio is an open-source audio API available free from www.portaudio.com */
#include "portaudio.h"

#include "spmidi_jukebox_song_ids.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)
#define BITS_PER_SAMPLE     (sizeof(short)*8)

/* Use these macros so that we don't have to hit ENTER. */
#ifdef WIN32
#define GETCH        _getch
#define KBHIT        _kbhit
#define PUTCH(c)     _putch((char)(c))
#endif

#define MELODY_CHANNEL  (14)
#define SFX_CHANNEL     (15)

#define VALUE_UNDEFINED  (-1)

static int ticksPerSecond = 0;

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

/****************************************************************/
/**
 * Get Audio from Jukebox to fill PortAudio buffer..
 */

int JBDemo_Callback(
    void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    PaTimestamp outTime, void *userData )
{
	/* Use information passed from foreground thread. */
	int framesLeft = framesPerBuffer;
	int framesGenerated = 0;
	short *outputPtr = (short *) outputBuffer;
	(void) inputBuffer;
	(void) outTime;
	(void) userData;

	/* The audio buffer is bigger than the synthesizer buffer so we
	 * have to call the synthesizer several times to fill it.
	 */
	while( framesLeft )
	{
		framesGenerated = JukeBox_SynthesizeAudioTick( outputPtr, framesPerBuffer, SAMPLES_PER_FRAME );
		if( framesGenerated <= 0 )
		{
			PRTMSGNUMH("Error: JukeBox_SynthesizeAudio returned ", framesGenerated );
			return 1; /* Tell PortAudio to stop. */
		}

		/* Advance pointer to next part of large output buffer. */
		outputPtr += SAMPLES_PER_FRAME * framesGenerated;

		/* Calculate how many frames are remaining. */
		framesLeft -= framesGenerated;
	}

	return 0;
}

PortAudioStream *audioStream;

/*******************************************************************/
int JBDemo_StartAudio( void )
{
	int result;

	/* Initialize audio hardware and open an output stream. */
	Pa_Initialize();
	result = Pa_OpenDefaultStream( &audioStream,
	                               0, SAMPLES_PER_FRAME,
	                               paInt16,
	                               (double) SAMPLE_RATE,
	                               JukeBox_GetFramesPerTick(),
	                               0,
	                               JBDemo_Callback,
	                               NULL );
	if( result < 0 )
	{
		PRTMSG( "Pa_OpenDefaultStream returns " );
		PRTMSG( Pa_GetErrorText( result ) );
		PRTMSG( "\n" );

		goto error;
	}

	Pa_StartStream( audioStream );

error:
	return result;
}


/*******************************************************************/
int JBDemo_StopAudio( void )
{
	Pa_StopStream( audioStream );
	Pa_Terminate();
	return 0;
}

/*******************************************************************/
void PlayBell( int pitch )
{
	/* Get time so we schedule the bell sound now. */
	int time = JukeBox_GetTime();
	/* Use duration of 1/16th of a second. */
	int dur = ticksPerSecond >> 4;

	JukeBox_ProgramChange( time, SFX_CHANNEL, 9 ); /* Glockenspiel. */

	/* Schedule quick note on and off. */
	JukeBox_NoteOn( time, SFX_CHANNEL, pitch, 64 );
	JukeBox_NoteOff( time+1, SFX_CHANNEL, pitch, 0 );

	/* Schedule next note later in time. */
	time += dur;
	JukeBox_NoteOn( time, SFX_CHANNEL, pitch - 4, 64 );
	JukeBox_NoteOff( time+1, SFX_CHANNEL, pitch - 4, 0 );
}

/*******************************************************************/
void PlaySnare( void )
{
	int i;
	int time = JukeBox_GetTime();
	int dur = (ticksPerSecond * 17) >> 8;

	/* Snare drum roll. */
	time += dur;
	for( i=0; i<8; i++ )
	{
		JukeBox_NoteOn( time, MIDI_RHYTHM_CHANNEL_INDEX, 38, 64 );
		time += dur;
	}
	/* Crash Cymbal */
	JukeBox_NoteOn( time, MIDI_RHYTHM_CHANNEL_INDEX, 49, 64 );
}

/*******************************************************************/
int PlaySwoop( void )
{
	int i;
	int time = JukeBox_GetTime();
	int dur = (ticksPerSecond * 3) >> 7;
	int bend;
	int result = 0;

	/* Start Guitar note and sweep up. */
	JukeBox_ProgramChange( time, SFX_CHANNEL, 30 ); /* Distortion guitar. */
	JukeBox_NoteOn( time, SFX_CHANNEL, 70, 64 );
	time += dur;
	bend = MIDI_BEND_NONE;
	for( i=0; i<50; i++ )
	{
		bend += 200;
		result = JukeBox_PitchBend( time, SFX_CHANNEL, bend );
		if( result < 0 )
		{
			printf("ERROR: could not write pitch bend, %d\n", result);
			return result;
		}
		time += dur;
	}
	/* Turn off note at end of swoop. */
	JukeBox_NoteOff( time, SFX_CHANNEL, 70, 0 );
	if( result < 0 )
	{
		printf("ERROR: could not turn off swoop note, %d\n", result);
		return result;
	}

	return result;
}

/*******************************************************************/
void PlayScale( int channel, int start, int count, int interval, int msec )
{
	int i;
	int time = JukeBox_GetTime();
	int dur = (ticksPerSecond * msec) / 1000;
	int pitch = start;

	/* Advance time so entire sequence has stable timing. */
	time += dur;
	for( i=0; i<count; i++ )
	{
		JukeBox_NoteOn( time, channel, pitch, 64 );
		time += dur;
		JukeBox_NoteOff( time, channel, pitch, 0 );
		pitch += interval;
	}
}

/*******************************************************************/
void QueueSong( int songID, int value )
{
	int numLoops = (value == VALUE_UNDEFINED) ? 1 : value;
	int result = JukeBox_QueueSong( songID, numLoops );
	if( result < 0 )
	{
		printf("ERROR: JukeBox_QueueSong returned 0x%08X\n", result );
	}
}

/*******************************************************************/
void usage( void )
{
	printf("\nJukeBox Demo, (C) 2004 Mobileer, Inc.\n");
	printf("Press single keys to trigger sounds.\n");
	printf("  0-9 = set N\n");
	printf("  sounds --------------\n");
	printf("  q,w,e,r,t = queue songs with N loops\n");
	printf("  a = bell sound\n");
	printf("  s = scale up\n");
	printf("  d = scale down\n");
	printf("  f = snare drum roll\n");
	printf("  g = sweep tone up\n");
	printf("  commands ------------\n");
	printf("  z = clear song queue\n");
	printf("  x = eXit\n");
	printf("  c = pause song\n");
	printf("  v = resume song\n");
	printf("  b = finish looping at end of current song\n");
	printf("  n = stop current song now\n");
	printf("  m = stop sound now\n");
}

/*******************************************************************/
/* Use keyboard commands to trigger songs or note sequences. */
int PlayKeyboard( void )
{
	int c = GETCH();
	int value = VALUE_UNDEFINED;
	while( c != 'x' ) /* Loop until user hits 'x' */
	{
		PUTCH(c);

		/* Parse key input. */
		if( (c >= '0') && (c <= '9') )
		{
			if( value == VALUE_UNDEFINED )
				value = (c - '0');
			else
				value = (value * 10) + (c - '0');
		}
		else if( c == 'a' )
		{
			PlayBell( 80 );
		}
		else if( c == 's' )
		{
			PlayScale( MELODY_CHANNEL, 60, 8, 2, 50 );
		}
		else if( c == 'd' )
		{
			PlayScale( MELODY_CHANNEL, 60, 8, -2, 100 );
		}
		else if( c == 'f' )
		{
			PlaySnare();
		}
		else if( c == 'g' )
		{
			PlaySwoop();
		}
#if 1
		else if( c == 'q' )
		{
			QueueSong( SONG_UPANDDOWN_RT, value );
			value = VALUE_UNDEFINED;
		}
		else if( c == 'w' )
		{
			QueueSong( SONG_ANSWERMENOW_RT, value );
			value = VALUE_UNDEFINED;
		}
		else if( c == 'e' )
		{
			QueueSong( SONG_URBANORE_SP, value );
			value = VALUE_UNDEFINED;
		}
		else if( c == 'r' )
		{
			QueueSong( SONG_FUNTOY8, value );
			value = VALUE_UNDEFINED;
		}
		else if( c == 't' )
		{
			QueueSong( SONG_RINGINREGGAE_RT, value );
			value = VALUE_UNDEFINED;
		}
#endif
		else if( c == 'z' )
		{
			JukeBox_ClearSongQueue();
		}
		else if( c == 'c' )
		{
			JukeBox_PauseSong();
		}
		else if( c == 'v' )
		{
			JukeBox_ResumeSong();
		}
		else if( c == 'b' )
		{
			JukeBox_FinishSong();
		}
		else if( c == 'n' )
		{
			JukeBox_StopSong();
		}
		else if( c == 'm' )
		{
			JukeBox_StopSound();
		}
		else
		{
			usage();
		}

		c = GETCH();
	}
	printf("\n");
	return 0;
}

/*******************************************************************/
int main(void);
int main(void)
{
	SPMIDI_Error err;

	usage();

	err = JukeBox_Initialize( 22050 );
	if( err < 0 )
		goto error1;

	ticksPerSecond = SAMPLE_RATE / JukeBox_GetFramesPerTick();

	JBDemo_StartAudio();

	PlayKeyboard();

	JBDemo_StopAudio();

	JukeBox_Terminate();

error1:
	printf("Test finished.\n");
	return err;
}
