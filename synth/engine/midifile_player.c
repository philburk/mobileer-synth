/* $Id: midifile_player.c,v 1.39 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 * MIDI File parser and player.
 * The player operates directly on an SMF image.
 * It is not expanded to another form in order to avoid memory allocations.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */


#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/engine/memtools.h"

#ifndef SPMIDI_MALLOC_PLAYER
    #define SPMIDI_MALLOC_PLAYER  (0)
#else
    #include <malloc.h>
#endif

#if 0
    #define DBUGMSG(x)   PRTMSG(x)
    #define DBUGNUMD(x)  PRTNUMD(x)
    #define DBUGNUMH(x)  PRTNUMH(x)
#else
    #define DBUGMSG(x)
    #define DBUGNUMD(x)
    #define DBUGNUMH(x)
#endif

#define PRINT_PROGRAM_CHANGES  (0)
#define PRINT_CHANNEL_MESSAGES (0)

typedef struct MIDIFileTrack_s
{
    /** Double Linked List node for sorting tracks. */
    DoubleNode     node;
    /** Time in ticks of next event in track. */
    int            nextEventTime;
    /** Address of track data. */
    const unsigned char *imageStart;
    /** Address as we advance through the image. */
    const unsigned char *imageCursor;
    /** Address of byte after last data. */
    const unsigned char *imageEnd;
    /** Used to parse MIDI running status. */
    unsigned char  lastCommand;
    unsigned char  noteOnEnabled;
    unsigned char  index;
}
MIDIFileTrack_t;

/** Maximum number of tracks in a Format 1 MIDI file that can be played. */
#ifndef MIDIFILE_MAX_TRACKS
    #if SPMIDI_SUPPORT_MALLOC
        #define MIDIFILE_MAX_TRACKS   (512)
    #else
        #define MIDIFILE_MAX_TRACKS   (40)
    #endif
#endif

typedef struct MIDIFilePlayer_s
{
    MIDIFileParser_t parser;

/* Dynamically allocate tracks array if using malloc so we don't run out. */
#if SPMIDI_SUPPORT_MALLOC
    MIDIFileTrack_t *tracks;
#else
    MIDIFileTrack_t  tracks[MIDIFILE_MAX_TRACKS];
#endif

    int              numTracks;
    DoubleList       sortedTracks;   /**< Sorted in order of nextEventTime */
     /** Used to shift ticksPerFrameShifted and adjust tempo. 0x00010000 is normal tempo. */
    int              tempoScaler;
     /** Scaled by 2^16 */
    int              ticksPerFrameShifted;
    /** Current playback frame. */
    int              currentFrame;
    /** Current playback time. */
    int              currentTick;
    /** Unsigned 16 bit fractional tick */
    int              currentTickFraction;
    int              sampleRate;
    MIDIFilePlayer_TextCallback *textCallback;
    void            *textUserData;

    /** MIDI procedure for handling MIDI bytes when they are ready to be played. */
    MIDIFilePlayer_WriteCallback *writeCallback;
    MIDIFilePlayer_ResetCallback *resetCallback;

    unsigned char    noteOnEnabled;
    unsigned char    midiEnabled;
}
MIDIFilePlayer_t;

/****************************************************************/
static int playBeginTrack( MIDIFileParser_t *parser, int index, int size )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) parser->userData;
    MIDIFileTrack_t *track;

    if( index >= MIDIFILE_MAX_TRACKS )
    {
        return MIDIFile_Error_TooManyTracks;
    }

#if SPMIDI_SUPPORT_MALLOC
    if( player->tracks == NULL )
    {
        int numBytes = parser->numTracks * sizeof( MIDIFileTrack_t );
        player->tracks = SPMIDI_ALLOC_MEM( numBytes, "tracks" );
        if( player->tracks == NULL )
        {
            return SPMIDI_Error_OutOfMemory;
        }
    }
#endif

    track = &player->tracks[ index ];

    DBUGMSG("playBeginTrack: init node at ");
    DBUGNUMH( track );
    DBUGMSG("\n");

    DLL_InitNode( &player->tracks[ index ].node );
    player->numTracks += 1;
    track->imageStart = &parser->imageStart[parser->bytesRead];
    track->imageCursor = track->imageStart;
    track->imageEnd = track->imageStart + size;

    track->nextEventTime = MIDIFile_ReadVariableLengthQuantity( &track->imageCursor );

    return 0;
}

/****************************************************************/
static int playEndTrack( MIDIFileParser_t *parser, int index )
{
    (void) parser;
    (void) index;
    return 0;
}

/****************************************************************/
static int playHandleEvent( MIDIFileParser_t *parser, int ticks, int command, int data1, int data2 )
{
    int code = command & 0x0F0;
    (void) ticks;
    (void) data1;
    switch( code )
    {
    case MIDI_NOTE_ON:
        if( data2 == 0 )
            parser->noteOffCount += 1;
        else
            parser->noteOnCount += 1;
        break;
    case MIDI_NOTE_OFF:
        parser->noteOffCount += 1;
        break;
    }
    return 0;
}

#define PLAYER_WRITE( byte )  player->writeCallback( midiContext, byte )
#define PLAYER_RESET()  player->resetCallback( midiContext )

/****************************************************************/
static int MIDIFilePlayer_CalculateTicksPerFrame( MIDIFilePlayer_t *player )
{
    /* We want to shift the ticksPerFrame by 16 and also provide fine adjustment. We can do this by scaling it by a
     * tempo scaler that is nominally 0x10000.
     */
    return (player->parser.ticksPerSecond * player->tempoScaler) / player->sampleRate;
}

/****************************************************************/
SPMIDI_Error MIDIFilePlayer_SetTempoScaler( MIDIFilePlayer *playerExt, int tempoScaler )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    player->tempoScaler = tempoScaler;
    player->ticksPerFrameShifted = MIDIFilePlayer_CalculateTicksPerFrame( player );
    return 0;
}

/****************************************************************/
SPMIDI_Error MIDIFilePlayer_GetTempoScaler( MIDIFilePlayer *playerExt )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    return player->tempoScaler;
}

/****************************************************************/
static void MIDIFilePlayer_HandleMetaEvent( MIDIFilePlayer_t *player,
        MIDIFileTrack_t *track, int ticks, int type,
        const unsigned char *addr, int numBytes )
{
    MIDIFileParser_t *parser = &player->parser;
    (void) ticks;
    /* Call user callback with text if requested. */
    if( (player->textCallback != NULL) && (type >= 1) && (type <= 7) )
    {
        (*player->textCallback)( track->index, type,
                                 (const char *)addr, numBytes, player->textUserData );
    }
    else if( type == MIDI_META_SET_TEMPO )
    {
        /* Change Tempo */
        int microsPerBeat = (addr[0] << 16) | (addr[1] << 8) | addr[2];

        /* Prevent overflow in (parser->ticksPerBeat * scaler). */
        int divisor = microsPerBeat;
        int scaler = 1000000;
        int shifter = 0;
        while( parser->ticksPerBeat > (0x7FFFFFFF / scaler) )
        {
            scaler = scaler >> 1;
            shifter += 1;
        }
        divisor = divisor >> shifter;

        parser->ticksPerSecond = (parser->ticksPerBeat * scaler) / divisor;

        player->ticksPerFrameShifted = MIDIFilePlayer_CalculateTicksPerFrame( player );
    }
}

/****************************************************************/
static int MIDIFile_PlayNextCommand( MIDIFilePlayer_t *player, MIDIFileTrack_t *track,
                                     void *midiContext, int ticks, unsigned char  command )
{
    int      isRunningStatus;
    int      bytesPerMessage;

    unsigned char  data1 = 0;
    unsigned char  data2 = 0;
    int      bytesReadInMessage = 1; /* already read first byte */

    (void) ticks;

    isRunningStatus = ( command < 0x80 ); /* running status? */
    if( isRunningStatus )
    {
        data1 = command;
        command = track->lastCommand;
        bytesReadInMessage = 2;
    }
    track->lastCommand = (unsigned char) command;
    bytesPerMessage = SPMIDI_GetBytesPerMessage( command );
    if( !isRunningStatus )
    {
        if( bytesPerMessage > bytesReadInMessage )
        {
            data1 = *track->imageCursor++;
            bytesReadInMessage++;
        }
    }
    /* Read final byte if needed. */
    if( bytesPerMessage > bytesReadInMessage )
    {
        data2 = *track->imageCursor++;
        bytesReadInMessage++;
    }

    /* printHandleEvent( NULL, ticks, command, data1, data2 ); */
#if PRINT_PROGRAM_CHANGES
    if( (command & 0x0F0) == MIDI_PROGRAM_CHANGE )
    {
        int channel = command & 0x0F;
        PRTMSG("Program: ch = ");
        PRTNUMD( channel + 1 );
        PRTMSG(", tr# = ");
        PRTNUMD( track->index );
        PRTMSG(", p# = ");
        PRTNUMD( data1+1 );
        PRTMSG(" (");
        PRTNUMH( data1 );
        PRTMSG(") ");
        if( channel != MIDI_RHYTHM_CHANNEL_INDEX )
        {
            PRTMSG( MIDI_GetProgramName( data1 ) );
        }
        PRTMSG("\n");
    }
    if( ((command & 0x0F0) == MIDI_CONTROL_CHANGE) && ((data1 == 0) || (data1 == 32)) )
    {
        PRTMSG("Bank Change: ch = ");
        PRTNUMD( (command & 0x0F) + 1 );
        PRTMSG(", tr# = ");
        PRTNUMD( track->index );
        PRTMSG(", cc# = ");
        PRTNUMD( data1 );
        PRTMSG(", d2 = ");
        PRTNUMD( data2 );
        PRTMSG(" (");
        PRTNUMH( data2 );
        PRTMSG(")\n");
    }
#endif

    /* Skip NoteOns if all output or the track is disabled. */
    if( player->midiEnabled  )
    {
        int commandIsNoteOn;
        int commandType = command & 0x0F0;

        /* Convert note offs to note ons to make better use of runing status in MIDI stream. */
        if( commandType == MIDI_NOTE_OFF )
        {
            command = (unsigned char) (MIDI_NOTE_ON + (command & 0x0F));
            data2 = 0;
            commandType = MIDI_NOTE_ON;
        }

        commandIsNoteOn = ((commandType == MIDI_NOTE_ON) && (data2 > 0));

#if PRINT_CHANNEL_MESSAGES

        PRTMSG("tick = ");
        PRTNUMD( player->currentTick );
        PRTMSG(", frame = ");
        PRTNUMD( player->currentFrame );
        PRTMSG(", cmd = ");
        PRTNUMH( command );
        PRTMSG(", data1 = ");
        PRTNUMD( data1 );
        PRTMSG(", data2 = ");
        PRTNUMD( data2 );
        PRTMSG("\n");
#endif

        if( (player->noteOnEnabled && track->noteOnEnabled) ||
                !commandIsNoteOn )
        {
            /* Send individual bytes to MIDI parser in case there are
             * illegally formatted MIDI commands in the file.
             */
            PLAYER_WRITE( command );
            if( bytesPerMessage > 1 )
                PLAYER_WRITE( data1 );
            if( bytesPerMessage > 2 )
                PLAYER_WRITE( data2 );
        }
    }
    return 0;
}

/****************************************************************/
static int MIDIFile_PlayCurrentEvents( MIDIFilePlayer_t *player, MIDIFileTrack_t *track, void *midiContext )
{
    int result = 0;
    int eventLength;
    int ticks = track->nextEventTime;
    int type;
    int bytesLeft;

    while( ticks == track->nextEventTime )
    {
        unsigned char command;

        /* Read message. Might be running status */
        command = *track->imageCursor++;
        switch( command )
        {
        case 0xFF:
            type = *track->imageCursor++;
            eventLength = MIDIFile_ReadVariableLengthQuantity( &track->imageCursor );
            MIDIFilePlayer_HandleMetaEvent( player, track, ticks, type,
                                            track->imageCursor, eventLength );
            track->imageCursor += eventLength;
            break;

        case MIDI_SOX:
            if( player->midiEnabled )
            {
                PLAYER_WRITE( MIDI_SOX );
            }

        /* Handle System Exclusive Event. */
        case 0xF7:
            eventLength = MIDIFile_ReadVariableLengthQuantity( &track->imageCursor );
            if( player->midiEnabled )
            {
                int eb = eventLength;
                const unsigned char *data = track->imageCursor;
                while( eb-- > 0 )
                {
                    PLAYER_WRITE( *data++ );
                }
            }
            track->imageCursor += eventLength;
            break;

        default:
            MIDIFile_PlayNextCommand( player, track, midiContext, ticks, command );
            break;
        }

        /* Are we through with this track? */
        bytesLeft = (int)(track->imageEnd - track->imageCursor);
        if( bytesLeft <= 0)
        {
            result = 1;
            goto finish;
        }

        /* Parse ticks as variable length quantity. */
        ticks += MIDIFile_ReadVariableLengthQuantity( &track->imageCursor );

        /* Sometimes tempo zero may have a ridiculously late EndOfTrack event.
         * If so, just ignore it. This fixes problem with "rhapsody.mid".
         */
        bytesLeft = (int)(track->imageEnd - track->imageCursor);
        if( (bytesLeft <= 3) && (track->index == 0) && (player->parser.format > 0) )
        {
            result = 1;
            goto finish;
        }
    }

finish:
    track->nextEventTime = ticks;
    return result;
}

/****************************************************************/
void MIDIFile_InsertTrack( MIDIFilePlayer_t *player, MIDIFileTrack_t *track )
{
    MIDIFileTrack_t *trackInList;

    DLL_FOR_ALL( MIDIFileTrack_t, trackInList, &player->sortedTracks )
    {
        if( (trackInList->nextEventTime > track->nextEventTime) ||
                /* Sort tracks at the same time in index order to match WinAmp. */
                ((trackInList->nextEventTime == track->nextEventTime) &&
                 (trackInList->index > track->index))
          )
        {
            DLL_InsertBefore( &trackInList->node, &track->node );
            return;
        }
    }

    DLL_AddTail( &player->sortedTracks, &track->node );
}

/****************************************************************/
#if SPMIDI_SUPPORT_MALLOC
#define MIDIFilePlayer_Allocate()      SPMIDI_ALLOC_MEM( sizeof(MIDIFilePlayer_t), "MIDIFilePlayer_t" )
#define MIDIFilePlayer_Free(plr)  SPMIDI_FreeMemory(plr)
#else


/* Pool of players that can be allocated without using malloc. */
static MIDIFilePlayer_t  sMIDIFilePlayerPool[SPMIDI_MAX_NUM_PLAYERS];
/* Array used to keep track of which ones are allocated. */
static char              sMIDIFilePlayersAllocated[SPMIDI_MAX_NUM_PLAYERS] = { 0 };

/* Scan array for unallocated player. Allocate it if free. */
static MIDIFilePlayer_t *MIDIFilePlayer_Allocate( void )
{
    int i;
    MIDIFilePlayer_t *player = NULL;

    /* Protect the following section of code from thread collisions. */
    SPMIDI_EnterCriticalSection();

    for( i=0; i<SPMIDI_MAX_NUM_PLAYERS; i++ )
    {

        /* Warning! This is a non-atomic test and set. It is not thread safe.
        * The application should implement SPMIDI_EnterCriticalSection() if
        * multiple threads can collide in this code.
        */
        if( sMIDIFilePlayersAllocated[i] == 0 )
        {
            sMIDIFilePlayersAllocated[i] = 1;
            player = &sMIDIFilePlayerPool[i];
            break;
        }
    }

    SPMIDI_LeaveCriticalSection();
    return player;
}

/* Scan pool to match player being freed. Mark it as free if found. */
static void MIDIFilePlayer_Free( MIDIFilePlayer_t *player )
{
    int i;
    /* Protect the following section of code from thread collisions. */
    SPMIDI_EnterCriticalSection();
    for( i=0; i<SPMIDI_MAX_NUM_PLAYERS; i++ )
    {
        if( &sMIDIFilePlayerPool[i] == player )
        {
            sMIDIFilePlayersAllocated[i] = 0; /* Mark as free. */
            break;
        }
    }
    SPMIDI_LeaveCriticalSection();
}
#endif

/****************************************************************/
/*********** External API ***************************************/
/****************************************************************/


void MIDIFilePlayer_Delete( MIDIFilePlayer *playerExt )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    if( player != NULL )
    {
#if SPMIDI_SUPPORT_MALLOC
        if( player->tracks != NULL )
        {
            SPMIDI_FreeMemory( player->tracks );
            player->tracks = NULL;
        }
#endif
        MIDIFilePlayer_Free( player );
    }
}

/****************************************************************/
SPMIDI_Error MIDIFilePlayer_Create( MIDIFilePlayer **playerPtr, int sampleRate,
                                    const unsigned char *image, int numBytes )
{
    SPMIDI_Error result = SPMIDI_Error_None;
    MIDIFilePlayer_t *player;

    if( sampleRate < 8000)
    {
        return SPMIDI_Error_UnsupportedRate;
    }

    player = MIDIFilePlayer_Allocate();
    if( player == NULL )
    {
        result = SPMIDI_Error_OutOfMemory;
        goto error1;
    }
    else
    {
        int i;
        MIDIFileParser_t *parser = &player->parser;
        MemTools_Clear( player, sizeof(MIDIFilePlayer_t) );

        /* Set default MIDI handlers. */
        player->writeCallback = SPMIDI_WriteByte;
        player->resetCallback = SPMUtil_Reset;

        player->sampleRate = sampleRate;

        parser->beginTrackProc = playBeginTrack;
        parser->endTrackProc = playEndTrack;
        parser->handleEventProc = playHandleEvent;
        parser->handleMetaEventProc = NULL;

        parser->imageStart = image;
        parser->imageSize = numBytes;
        parser->userData = player;
        player->sampleRate = sampleRate;
        player->tempoScaler = MIDIFILEPLAYER_DEFAULT_TEMPO_SCALER;

        result = MIDIFilePlayer_Rewind( (MIDIFilePlayer *) player );
        if( result < 0 )
        {
            goto error2;
        }

        player->midiEnabled = 1;
        player->noteOnEnabled = 1;
        for( i=0; i<player->numTracks; i++ )
        {
            player->tracks[i].noteOnEnabled = 1;
            player->tracks[i].index = (unsigned char) i;
        }
    }

    *playerPtr = (MIDIFilePlayer *) player;
    return result;

error2:
    MIDIFilePlayer_Delete( (MIDIFilePlayer *) player );
error1:
    *playerPtr = NULL;
    return result;
}

/****************************************************************/
SPMIDI_Error MIDIFilePlayer_Rewind( MIDIFilePlayer *playerExt )
{
    int i;
    int result;
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;

    player->numTracks = 0;
    result = MIDIFile_Parse( &player->parser );
    if( result < 0 )
    {
        return (SPMIDI_Error) result;
    }

    if( player->numTracks > MIDIFILE_MAX_TRACKS )
    {
        return MIDIFile_Error_TooManyTracks;
    }

    player->ticksPerFrameShifted = MIDIFilePlayer_CalculateTicksPerFrame( player );
    player->currentTickFraction = 0;
    player->currentTick = 0;
    player->currentFrame = 0;

    /* Sort tracks by order of next event. */
    DLL_InitList( &player->sortedTracks );
    for( i=0; i<player->numTracks; i++ )
    {
        MIDIFile_InsertTrack( player, &player->tracks[i] );
    }

    return (SPMIDI_Error) result;
}

/****************************************************************/
int MIDIFilePlayer_PlayFrames( MIDIFilePlayer *playerExt, void *midiContext, int numFrames )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;

    /* Accumulate ticks with 16 bit fraction for improved accuracy with small buffer. */
    int deltaTicksShifted = player->currentTickFraction + (player->ticksPerFrameShifted * numFrames);
    player->currentTickFraction = deltaTicksShifted & 0x0000FFFF;
    player->currentTick += deltaTicksShifted >> 16;
    player->currentFrame += numFrames;

    /* Process tracks in sorted order. */
    while( !DLL_IsEmpty(&player->sortedTracks) )
    {
        MIDIFileTrack_t *track = (MIDIFileTrack_t *) DLL_First( &player->sortedTracks );
        /* Check for ready event in next scheduled track. */
        if( track->nextEventTime <= player->currentTick )
        {
            DLL_Remove( &track->node );

            if( MIDIFile_PlayCurrentEvents( player, track, midiContext ) == 0)
            {
                MIDIFile_InsertTrack( player, track );
            }
        }
        else
        {
            /* No more tracks ready to be played at this time. */
            break;  
        }
    }

    return DLL_IsEmpty(&player->sortedTracks);
}

/****************************************************************/
int MIDIFilePlayer_GetDurationInFrames( MIDIFilePlayer *playerExt )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    int approximateFramesPerMsec = player->sampleRate / 1000;
    int originalPosition = player->currentFrame;
    int frames;
    unsigned char savedEnable = player->noteOnEnabled;

    if( approximateFramesPerMsec < 8 )
        approximateFramesPerMsec = 8;

    player->midiEnabled = 0;

    /* Advance by framesPerMsec so that we get an msec accurate measure. */
    MIDIFilePlayer_Rewind( playerExt );
    while( MIDIFilePlayer_PlayFrames( playerExt, NULL, approximateFramesPerMsec ) == 0 )
        ;

    /* Save last position which indicates length of MIDIfile. */
    frames = player->currentFrame;

    /* Restore position before call. */
    MIDIFilePlayer_GoToFrame( playerExt, NULL, originalPosition );

    player->midiEnabled = savedEnable;

    return frames;
}

/****************************************************************/
int MIDIFilePlayer_GetDurationInMilliseconds( MIDIFilePlayer *playerExt )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    int frames = MIDIFilePlayer_GetDurationInFrames( playerExt );
    int msec = SPMUtil_ConvertFramesToMSec( player->sampleRate, frames );
    return msec;
}

/****************************************************************/
int MIDIFilePlayer_GoToFrame( MIDIFilePlayer *playerExt, void *midiContext, int desiredFrame )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    int framesToSkip = player->sampleRate / 1000;
    int go = 1;
    unsigned char savedNoteOnEnable = player->noteOnEnabled;

    player->currentTick = 0;
    player->noteOnEnabled = 0;

    if( desiredFrame < player->currentFrame )
    {
        /* Make sure notes are turned off and controllers reset. */
        SPMUtil_Reset( midiContext ); /* ZZZZ */
        /* Rewind to beginning and move forward to desired point. */
        MIDIFilePlayer_Rewind( playerExt );
    }

    while( go )
    {
        int framesToGo = desiredFrame - player->currentFrame;
        /* Are we there yet? */
        if( framesToGo <= 0 )
            break;
        /* Don't overshoot. */
        if( framesToSkip > framesToGo )
            framesToSkip = framesToGo;
        /* Advance forward. */
        if ( MIDIFilePlayer_PlayFrames( playerExt, midiContext, framesToSkip ) != 0 )
            break; /* Past end! */
    }

    player->noteOnEnabled = savedNoteOnEnable;
    return player->currentFrame;
}

/****************************************************************/
SPMIDI_Error MIDIFilePlayer_SetTrackEnable( MIDIFilePlayer *playerExt, int trackIndex, int onOrOff )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    if( (trackIndex < 0) || (trackIndex >= player->numTracks) )
    {
        return MIDIFile_Error_IllegalTrackIndex;
    }
    player->tracks[ trackIndex ].noteOnEnabled = (unsigned char) (onOrOff != 0);
    return SPMIDI_Error_None;
}

/****************************************************************/
int MIDIFilePlayer_GetTrackEnable( MIDIFilePlayer *playerExt, int trackIndex )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    if( (trackIndex < 0) || (trackIndex >= player->numTracks) )
    {
        return (int) MIDIFile_Error_IllegalTrackIndex;
    }
    else
        return (int) player->tracks[ trackIndex ].noteOnEnabled;
}

/****************************************************************/
int MIDIFilePlayer_GetTrackCount( MIDIFilePlayer *playerExt )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    return player->numTracks;
}

/****************************************************************/
void MIDIFilePlayer_SetTextCallback( MIDIFilePlayer *playerExt,
                                     MIDIFilePlayer_TextCallback *textCallback,
                                     void *userData )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    player->textCallback = textCallback;
    player->textUserData = userData;
}

/****************************************************************/
void MIDIFilePlayer_SetSynthCallbacks( MIDIFilePlayer *playerExt,
                                     MIDIFilePlayer_WriteCallback *writeCallback,
                                     MIDIFilePlayer_ResetCallback *resetCallback )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    player->writeCallback = writeCallback;
    player->resetCallback = resetCallback;
}

/****************************************************************/
int MIDIFilePlayer_GetTickTime( MIDIFilePlayer *playerExt )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    return player->currentTick;
}

/****************************************************************/
int MIDIFilePlayer_GetFrameTime( MIDIFilePlayer *playerExt )
{
    MIDIFilePlayer_t *player = (MIDIFilePlayer_t *) playerExt;
    return player->currentFrame;
}
