/* $Id: spmidi_jukebox.c,v 1.11 2007/10/02 17:25:29 philjmsl Exp $ */
/**
 * High level song queuing and MIDI scheduling.
 * Public routines are documented in spmidi_jukebox.h
 * @file spmidi_jukebox.c
 * @brief Tools for playing from a selection of songs.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/midi.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/engine/dbl_list.h"

#include "spmidi/jukebox/atomic_fifo.h"
#include "spmidi/jukebox/event_buffer.h"
#include "spmidi/include/spmidi_jukebox.h"

/** This is included from the "export" folder. */
#include "spmidi_jukebox_playlist.h"

#ifndef JUKEBOX_FIFO_SIZE
#define JUKEBOX_FIFO_SIZE   (256)
#endif

#ifndef JUKEBOX_EVBUF_SIZE
#define JUKEBOX_EVBUF_SIZE  (512)
#endif

#ifndef JUKEBOX_SONGQUEUE_SIZE
#define JUKEBOX_SONGQUEUE_SIZE  (64)
#endif

#define JUKEBOX_FOREVER_LOOPS_LEFT  (-1)

#define BITS_PER_SAMPLE   (16)
#define DUMP_MIDI          (0)

#if DUMP_MIDI
static void DumpMIDI( unsigned char *data );
#endif

typedef enum ParserCommands_e
{
    CMD_MIDI=100,
    CMD_QUEUE_SONG,
    CMD_CLEAR_SONG_QUEUE,
    CMD_STOP_SONG,
    CMD_PAUSE_SONG,
    CMD_RESUME_SONG,
    CMD_FINISH_SONG,
    CMD_STOP_SOUND,
    CMD_SET_MAX_VOICES
} ParserCommands_e;

typedef struct JukeBoxCommand_s
{
	int opcode;
	int data;
	unsigned char bytes[4];
}
JukeBoxCommand_t;

typedef struct JukeBoxEvent_s
{
	EventBufferNode ebnode;
	int opcode;
	unsigned char bytes[4];
}
JukeBoxEvent_t;

typedef struct JukeBoxSongNode_s
{
	DoubleNode node;
	JukeBoxEntry_t song;
	int numLoops;
}
JukeBoxSongNode_t;

typedef struct JukeBoxContext_s
{
	int time;
	SPMIDI_Context *spmidiContext;
	MIDIFilePlayer  *songPlayer;
	int songLoopsLeft;
	int framesPerTick;
	volatile int numSongsQueued;
	volatile int numSongsDequeued;
	unsigned char pauseSong;
	unsigned char stopSong;
	AtomicFIFO  fifo;
	DoubleList songQueue;
	DoubleList freeSongNodeList;
	EventBuffer eventBuffer;
	DoubleList freeEventList;
	JukeBoxCommand_t commands[JUKEBOX_FIFO_SIZE];
	JukeBoxSongNode_t sqnodes[JUKEBOX_SONGQUEUE_SIZE];
	JukeBoxEvent_t ebnodes[JUKEBOX_EVBUF_SIZE];
}
JukeBoxContext_t;

static JukeBoxContext_t JBCon;

static SPMIDI_Error  JukeBox_Send0( int opcode );
static SPMIDI_Error  JukeBox_Send4( int opcode, int time, int b0, int b1, int b2, int b3 );

/**********************************************************/
/**
 * Send an opcode with time or data.
 * @param opcode simple command
 */
static SPMIDI_Error  JukeBox_Send0( int opcode )
{
	JukeBoxCommand_t *command;

	/* Are there any available commands in the FIFO? */
	if( (command = (JukeBoxCommand_t *) AFIFO_NextWritable( &JBCon.fifo )) == NULL )
	{
		return -1;
	}
	else
	{
		command->opcode = opcode;

		AFIFO_AdvanceWriter( &JBCon.fifo );
	}
	return SPMIDI_Error_None;
}

/**********************************************************/
/**
 * Send a 4 byte JukeBox command.
 * @param time in ticks when event should occur
 */
static SPMIDI_Error  JukeBox_Send4( int opcode, int time, int b0, int b1, int b2, int b3 )
{
	JukeBoxCommand_t *command;

	/* Are there any available commands in the FIFO? */
	if( (command = (JukeBoxCommand_t *) AFIFO_NextWritable( &JBCon.fifo )) == NULL )
	{
		return -1;
	}
	else
	{
		command->opcode = opcode;
		command->data = time;
		command->bytes[0] = (unsigned char) b0;
		command->bytes[1] = (unsigned char) b1;
		command->bytes[2] = (unsigned char) b2;
		command->bytes[3] = (unsigned char) b3;

		AFIFO_AdvanceWriter( &JBCon.fifo );
	}
	return SPMIDI_Error_None;
}

/**********************************************************/
/**
 * Initialize ME2000 engine and song playlist based on custom include files.
 */
SPMIDI_Error JukeBox_Initialize( int sampleRate )
{
	int i;
	int result=0;

	MemTools_Clear( &JBCon, sizeof(JBCon) );

	JBCon.framesPerTick = SPMIDI_GetFramesPerBuffer();
	if( JBCon.framesPerTick < 32 )
		JBCon.framesPerTick = 32;

	result = AFIFO_Init( &JBCon.fifo, JUKEBOX_FIFO_SIZE,
	                     sizeof(JukeBoxCommand_t), &JBCon.commands[0] );
	if( result < 0 )
		goto error2;

	result = EB_Init( &JBCon.eventBuffer, 0, &JBCon );
	if( result < 0 )
		goto error3;

	DLL_InitList( &JBCon.songQueue );

	/* Keep a list of free song queue nodes. Initially all are free. */
	DLL_InitList( &JBCon.freeSongNodeList );
	for( i=0; i<JUKEBOX_SONGQUEUE_SIZE; i++ )
	{
		JukeBoxSongNode_t *songNode = &JBCon.sqnodes[i];
		DLL_InitNode( &songNode->node );
		DLL_AddTail( &JBCon.freeSongNodeList, &songNode->node );
	}

	/* Keep a list of free events. Initially all are free. */
	DLL_InitList( &JBCon.freeEventList );
	for( i=0; i<JUKEBOX_EVBUF_SIZE; i++ )
	{
		JukeBoxEvent_t *event = &JBCon.ebnodes[i];
		DLL_InitNode( &event->ebnode.ebnd_Node );
		DLL_AddTail( &JBCon.freeEventList, &event->ebnode.ebnd_Node );
	}

	/* Start SP-MIDI synthesis engine using the desired sample rate. */
	result = SPMIDI_CreateContext( &JBCon.spmidiContext, sampleRate );
	if( result < 0 )
		goto error4;

error4:
error3:
error2:
	return result;
}

/**********************************************************/
SPMIDI_Error JukeBox_Terminate( void )
{
	return SPMIDI_DeleteContext( JBCon.spmidiContext );
}

/**********************************************************/
SPMIDI_Context *JukeBox_GetMIDIContext( void )
{
	return JBCon.spmidiContext;
}

/**********************************************************/
SPMIDI_Error JukeBox_QueueSong( int songIndex, int numLoops )
{
#if 1
	if( (songIndex >= JUKEBOX_NUM_SONGS) || (songIndex < 0) )
	{
		return SPMIDI_Error_OutOfRange;
	}
	else if( (numLoops > JUKEBOX_MAX_LOOPS) || (numLoops < 0) )
	{
		return SPMIDI_Error_OutOfRange;
	}
	else
	{
		JBCon.numSongsQueued += 1;
		return JukeBox_Send4( CMD_QUEUE_SONG,
		                      0,
		                      songIndex, numLoops, 0, 0 );
	}
#else
	(void) songIndex;
	(void) numLoops;
	return SPMIDI_Error_OutOfRange;
#endif
}

/**********************************************************/
SPMIDI_Error JukeBox_ClearSongQueue()
{
	return JukeBox_Send0( CMD_CLEAR_SONG_QUEUE );
}

/**********************************************************/
SPMIDI_Error JukeBox_StopSong( void )
{
	return JukeBox_Send0( CMD_STOP_SONG );
}

/**********************************************************/
SPMIDI_Error JukeBox_FinishSong( void )
{
	return JukeBox_Send0( CMD_FINISH_SONG );
}

/**********************************************************/
SPMIDI_Error JukeBox_PauseSong( void )
{
	return JukeBox_Send0( CMD_PAUSE_SONG );
}

/**********************************************************/
SPMIDI_Error JukeBox_ResumeSong( void )
{
	return JukeBox_Send0( CMD_RESUME_SONG );
}

/**********************************************************/
SPMIDI_Error JukeBox_StopSound( void )
{
	return JukeBox_Send0( CMD_STOP_SOUND );
}

/**********************************************************/
SPMIDI_Error JukeBox_SetVolume( int volume )
{
	SPMIDI_SetMasterVolume( JBCon.spmidiContext, volume );
	return 0;
}

/**********************************************************/
SPMIDI_Error JukeBox_SetMaxVoices( int maxVoices )
{
	return JukeBox_Send4( CMD_SET_MAX_VOICES,
	                      0,
	                      maxVoices, 0, 0, 0 );
}

/**********************************************************/
int JukeBox_GetMaxVoices( void )
{
	return SPMIDI_GetMaxVoices( JBCon.spmidiContext );
}

/**********************************************************/
int JukeBox_GetNumSongs( void )
{
	return JUKEBOX_NUM_SONGS;
}

/**********************************************************/
/* We subtract these two variables so we can count songs in
 * both command queue and background song queue.
 */
int JukeBox_GetSongQueueDepth( void )
{
	return JBCon.numSongsQueued - JBCon.numSongsDequeued;
}

/**********************************************************/
int JukeBox_GetTime( void )
{
	return JBCon.time;
}

/**********************************************************/
int JukeBox_GetFramesPerTick( void )
{
	return JBCon.framesPerTick;
}

/**********************************************************/
SPMIDI_Error  JukeBox_SendMIDI2( int time, int cmd, int d1 )
{
	return JukeBox_Send4( CMD_MIDI, time, 2, cmd, d1, 0 );
}

/**********************************************************/
SPMIDI_Error  JukeBox_SendMIDI3( int time, int cmd, int d1, int d2 )
{
	return JukeBox_Send4( CMD_MIDI, time, 3, cmd, d1, d2 );
}

/**********************************************************/
SPMIDI_Error  JukeBox_ClearCommands( void )
{
	/* Terminate and re-initialize event buffer */
	if( &JBCon.eventBuffer )
	{
		EB_Term( &JBCon.eventBuffer );
		return EB_Init( &JBCon.eventBuffer, 0, &JBCon );
	}
	else
	{
		/* buffer not yet set up */
		return 0;
	}
}

/**********************************************************/
SPMIDI_Error  JukeBox_MIDICommand( int time, int cmd, int d1, int d2 )
{
	int numBytes;

	numBytes = SPMIDI_GetBytesPerMessage( cmd );
	return JukeBox_Send4( CMD_MIDI, time, numBytes, cmd, d1, d2 );
}


/*******************************************************************/
SPMIDI_Error JukeBox_SendMIDI( int time, int numBytes, unsigned char *bytes )
{
	if( numBytes < 4 )
	{
		int numBytes = SPMIDI_GetBytesPerMessage( bytes[0] );
		return JukeBox_Send4( CMD_MIDI, time, numBytes, bytes[0], bytes[1], bytes[2] );
	}
	return SPMIDI_Error_Unsupported;
}

/**********************************************************/
SPMIDI_Error  JukeBox_ProgramChange( int time, int channel, int program)
{
	return JukeBox_SendMIDI2( time, MIDI_PROGRAM_CHANGE + channel, program );
}

/**********************************************************/
SPMIDI_Error  JukeBox_NoteOn( int time, int channel, int pitch, int velocity )
{
	return JukeBox_SendMIDI3( time, MIDI_NOTE_ON + channel, pitch, velocity );
}

/**********************************************************/
SPMIDI_Error  JukeBox_NoteOff( int time, int channel, int pitch, int velocity )
{
	return JukeBox_SendMIDI3( time, MIDI_NOTE_OFF + channel, pitch, velocity );
}

/**********************************************************/
SPMIDI_Error  JukeBox_PitchBend( int time, int channel, int bend )
{
	return JukeBox_SendMIDI3( time, MIDI_PITCH_BEND + channel,
	                          bend & 0x007F, /* LSB */
	                          bend >> 7 );  /* MSB */
}

/**********************************************************/
SPMIDI_Error  JukeBox_ControlChange( int time, int channel, int controller, int value )
{
	return JukeBox_SendMIDI3( time, MIDI_CONTROL_CHANGE + channel,
	                          controller,
	                          value );
}

/**********************************************************/
SPMIDI_Error  JukeBox_SetBendRange( int time, int channel, int semitones, int cents)
{
	/* Point to bend range RPN. */
	JukeBox_ControlChange( time, channel, 101, 0 );
	JukeBox_ControlChange( time, channel, 100, 0 );
	/* Set bend range. */
	JukeBox_ControlChange( time, channel, 6, semitones );
	JukeBox_ControlChange( time, channel, 38, cents );
	/* For safety, reset RPN to NULL. */
	JukeBox_ControlChange( time, channel, 101, 127 );
	JukeBox_ControlChange( time, channel, 100, 127 );
	return 0;
}

/**********************************************************/
SPMIDI_Error  JukeBox_AllNotesOff( int time, int channel  )
{
	return JukeBox_ControlChange( time, channel, MIDI_CONTROL_ALLNOTESOFF, 0 );
}

/**********************************************************/
int32 EB_ProcessNode( void *context, EventBufferNode *ebnd )
{
	int result = 0;
	JukeBoxContext_t *jbCon = (JukeBoxContext_t *) context;
	JukeBoxEvent_t *event = (JukeBoxEvent_t *) ebnd;
	int numBytes = event->bytes[0];
#if DUMP_MIDI
	DumpMIDI( &event->bytes[0] );
#endif

	SPMIDI_Write( jbCon->spmidiContext, &event->bytes[1], numBytes );

	result = EB_FreeNode( context, ebnd );
	return result;
}

/**********************************************************/
int32 EB_FreeNode( void *context, EventBufferNode *ebnd )
{
	(void) context;
	/* printf("free node %p\n", ebnd); */
	DLL_AddTail( &JBCon.freeEventList, &ebnd->ebnd_Node );
	return 0;
}

#if DUMP_MIDI
static void DumpMIDI( unsigned char *data )
{
	int i;
	printf("MIDI:");
	printf(" 0x%02X,", data[1] );
	for( i=0; i<(data[0]-1); i++ )
	{
		printf(" %d,", data[i+2] );
	}
	printf("\n");
}
#endif

/**********************************************************/
static void JB_ClearSongQueue( void )
{
	JukeBoxSongNode_t *songNode;
	/* Move all queued song nodes to the freeSongNodeList */
	while( (songNode = (JukeBoxSongNode_t *) DLL_RemoveFirst( &JBCon.songQueue )) != NULL)
	{
		JBCon.numSongsDequeued += 1;
		DLL_AddTail( &JBCon.freeSongNodeList, &songNode->node );
	}
}

/**
 * Process commands in synthesis thread.
 */
static SPMIDI_Error JB_ProcessCommands( void )
{
	int i;
	int result = 0;
	JukeBoxCommand_t *command;
	JukeBoxEvent_t *event;
	JukeBoxSongNode_t *songNode;

	/* Are there any available commands in the FIFO? */
	while( !AFIFO_IsEmpty( &JBCon.fifo ) )
	{
		command = (JukeBoxCommand_t *) AFIFO_NextReadable( &JBCon.fifo );
		AFIFO_AdvanceReader( &JBCon.fifo );

		switch( command->opcode )
		{
		case CMD_MIDI:
			/* Grab MIDI bytes from command and put into the event buffer. */
			event = (JukeBoxEvent_t *) DLL_RemoveFirst( &JBCon.freeEventList );
			if( event == NULL )
			{
				result = -2;
				goto error;
			}
			event->opcode = command->opcode;
			*((int *)&event->bytes[0]) = *((int *)&command->bytes[0]);
			EB_ScheduleNode( &JBCon.eventBuffer,
			                 command->data, /* timestamp */
			                 &event->ebnode );
			break;

		case CMD_QUEUE_SONG:
			/* Grab song id from command and put into the song queue. */
			songNode = (JukeBoxSongNode_t *) DLL_RemoveFirst( &JBCon.freeSongNodeList );
			if( songNode == NULL )
			{
				result = -3;
				goto error;
			}
			songNode->song = jukeBoxSongs[ command->bytes[0] ];
			songNode->numLoops = command->bytes[1];
			DLL_AddTail( &JBCon.songQueue, &songNode->node );
			break;

		case CMD_CLEAR_SONG_QUEUE:
			JB_ClearSongQueue();
			break;

		case CMD_PAUSE_SONG:
			JBCon.pauseSong = 1;
			for( i=0; i<MIDI_NUM_CHANNELS; i++ )
			{
				SPMUtil_AllNotesOff( JBCon.spmidiContext, i );
			}
			break;

		case CMD_RESUME_SONG:
			JBCon.pauseSong = 0;
			break;

			/* Finish after this last loop. */
		case CMD_FINISH_SONG:
			JBCon.songLoopsLeft = 0;
			break;

			/* Stop song immediately. */
		case CMD_STOP_SONG:
			JBCon.stopSong = 1;
			JBCon.songLoopsLeft = 0;
			break;

			/* Stop song immediately. */
		case CMD_STOP_SOUND:
			SPMUtil_Reset( JBCon.spmidiContext );
			break;

		case CMD_SET_MAX_VOICES:
			SPMIDI_SetMaxVoices( JBCon.spmidiContext, command->bytes[0] );
			break;
			
		default:
			break;
		}
	}
error:
	return result;
}

/*******************************************************************/
/*
 * If no song left playing then check the song queue.
 * If a song is playing then play the next part.
 * If it finishes then repeat if looped.
 */
static SPMIDI_Error JB_ProcessSongs( void )
{
	int result = 0;
	;

	if( JBCon.songPlayer == NULL )
	{
		/* Grab song id from command and put into the song queue. */
		JukeBoxSongNode_t *songNode = (JukeBoxSongNode_t *) DLL_RemoveFirst( &JBCon.songQueue );
		if( songNode != NULL )
		{

			JBCon.stopSong = 0;
			JBCon.numSongsDequeued += 1;

			SPMUtil_Reset( JBCon.spmidiContext );

			/* Create a player, parse MIDIFile image and setup tracks. */
			result = MIDIFilePlayer_Create( &JBCon.songPlayer,
			                                SPMIDI_GetSampleRate( JBCon.spmidiContext ),
			                                songNode->song.image, songNode->song.size );
			if( result < 0 )
			{
				JBCon.songLoopsLeft = 0;
				return result;
			}
			else
			{
				JBCon.songLoopsLeft = songNode->numLoops - 1;
			}

			/* Return song queue node to free list. */
			DLL_AddTail( &JBCon.freeSongNodeList, &songNode->node );
		}
	}

	if( JBCon.songPlayer != NULL )
	{
		if( JBCon.stopSong )
		{
			int i;
			result = 1; /* Force deletion. */
			for( i=0; i<MIDI_NUM_CHANNELS; i++ )
			{
				SPMUtil_AllNotesOff( JBCon.spmidiContext, i );
			}
		}
		else
		{
			/* Play one buffer worth of MIDI data. */
			result = MIDIFilePlayer_PlayFrames( JBCon.songPlayer,
			                                    JBCon.spmidiContext, JBCon.framesPerTick  );
		}
		if(result != 0)
		{
			if(JBCon.songLoopsLeft == 0)
			{
				/* Deallocate the MIDI file player. */
				MIDIFilePlayer_Delete( JBCon.songPlayer );
				JBCon.songPlayer = NULL;
			}
			else
			{
				/* Repeat the song by rewinding the MIDI file. */
				SPMUtil_Reset( JBCon.spmidiContext );
				MIDIFilePlayer_Rewind( JBCon.songPlayer );
				if(JBCon.songLoopsLeft > 0)
					JBCon.songLoopsLeft -= 1;
			}
		}
	}
	return 0;
}

/*******************************************************************/
SPMIDI_Error JukeBox_SynthesizeAudioTick(
    short *buffer,
    int maxFrames,
    int channelsPerFrame )
{
	SPMIDI_Error err = SPMIDI_Error_None;

	if( maxFrames < JBCon.framesPerTick )
	{
		return SPMIDI_Error_BufferTooSmall;
	}

	/* Check FIFO for more commands from foreground process. */
	err = JB_ProcessCommands();
	if( err < 0 )
	{
		return err;
	}

	/* Execute all of the current events in the buffer. */
	EB_ExecuteNodes( &JBCon.eventBuffer, JBCon.time );

	if( JBCon.pauseSong == 0 )
	{
		err = JB_ProcessSongs();
		if( err < 0 )
		{
			return err;
		}
	}

	/* Generate a tick worth of audio data as 16 bit samples. */
	err = SPMIDI_ReadFrames( JBCon.spmidiContext, buffer, JBCon.framesPerTick,
	                         channelsPerFrame, BITS_PER_SAMPLE );

	JBCon.time += 1;

	return err;
}

