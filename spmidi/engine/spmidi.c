/* $Id: spmidi.c,v 1.60 2007/11/06 08:22:02 philjmsl Exp $ */
/**
 *
 * Scaleable Polyphonic MIDI Engine.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "dls_parser_internal.h"
#include "spmidi/engine/spmidi_orchestra.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/engine/dbl_list.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/engine/spmidi_dls.h"

#ifdef WIN32
    #include <assert.h>
#else
    #define assert(x) /* never mind */
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

#define CHANNEL_FREE           (0xFF)
#define SPM_NO_PRIORITY        (0xFF)
#define SPM_LOWEST_PRIORITY    (MIDI_NUM_CHANNELS-1)
#define SPM_HIGHEST_PRIORITY   (0)

/* This was defined as 8 before we supported multiple banks. */
#define SPMIDI_BANK_MSB_SHIFT    (7)

/* 15 bit number that is impossible when combining two 7 bit values. */
#define SPM_INVALID_PARAM_NUMBER (0x7FFF)

#define SEMITONES_PER_OCTAVE   (12)

#ifndef FALSE
#define FALSE (0)
#define TRUE  (1)
#endif

#if (SPMIDI_MAX_SAMPLE_RATE > 96000)
#error SPMIDI_MAX_SAMPLE_RATE is too high. Must be less than 96000.
#endif

#if (SPMIDI_MAX_SAMPLES_PER_FRAME < 1) || (SPMIDI_MAX_SAMPLES_PER_FRAME > 2)
#error SPMIDI_MAX_SAMPLES_PER_FRAME must be 1 or 2.
#endif

#if (SPMIDI_MAX_VOICES < 1) || (SPMIDI_MAX_VOICES > 64)
#error SPMIDI_MAX_VOICES must be between 1 and 64 inclusive.
#endif

/**********************************************************/
/********** Structures ************************************/
/**********************************************************/
typedef struct SPMIDIVoiceTracker_s
{
    DoubleNode      node;
    int             timeOn;  /* Time the voiceTracker got a NoteOn command */
    int             timeOff; /* Time the voiceTracker got a NoteOn command */
    unsigned char   id;      /* For referencing synthesizer voice. */
    unsigned char   channel; /* CHANNEL_FREE if unassigned */
    unsigned char   noteIndex;  /* MIDI note index. Actual pitch may be transposed. */
    unsigned char   velocity;
    /** True if between NoteOn and NoteOff. False if after NoteOff */
    unsigned char   isOn;
    /** True if note off occured while sustain pedal on. */
    unsigned char   isSustaining;
    /** General MIDI drums are mutually exclusive. Zero if not in exclusion group. */
    unsigned char   exclusionGroup;
    /** Set if note has already been stifled. */
    unsigned char   isStifled;
}
SPMIDIVoiceTracker_t;

/* Define real SPMIDI Channel. */
typedef struct SPMIDIChannel_s
{
    DoubleList      voiceList;
    long            bend;         /* Signed 14 bit pitch bend value for channel. */
    long            bendRange;    /* 8.8 bit pitch bend range in semitones. */
    short           nonRegParamNumber;  /* NonRPN, non-registered parameter number specified using control 98/99 */
    short           regParamNumber;  /* RPN, registered parameter number specified using control 100/101 */
    /** Current bank. */
    short           insBank;
    unsigned char   isDrum;       /* True if this channel should be reated as a drum channel. */
    unsigned char   isMuted;      /* True if channel muted because of SP. */
    unsigned char   isDisabled;   /* True if channel disabled by external call. */
    unsigned char   isVibrating;  /* True program and bank match Phone Vibrator. */
    unsigned char   isSustained;  /* True if sustain pedal is on. */
    unsigned char   priority;     /* Priority of zero is highest, 15 is lowest. */
    unsigned char   numVoices;    /* Number of voices active. */
    unsigned char   program;      /* Program Change Index */
    unsigned char   pan;          /* Pan Controller setting. */
    unsigned char   tuningFineLSB; /* Tuning byte. */
    unsigned char   tuningFineMSB; /* Tuning byte. */
    unsigned char   tuningCoarseMSB; /* Tuning byte. */
}
SPMIDIChannel_t;

/* Define real SPMIDI context. */
typedef struct SPMIDIContext_s
{
    DoubleNode      node; // So we can keep track of and operate on all contexts.

    /* Synthesizer interface. */
    SoftSynth      *synth;

    int             frameCount; /* Number of frames generated. */
    int             sampleRate;
    /** Call this function when a telephone ring vibrator message is received. */
    SPMIDI_VibratorCallback *vibratorCallback;
    /** Data passed back to user as a callback parameter. */
    void           *vibratorUserData;

    SPMIDIChannel_t channels[MIDI_NUM_CHANNELS];
    /* Voice Allocation */
    SPMIDIVoiceTracker_t   voiceTrackers[SPMIDI_MAX_VOICES];
    DoubleList      freeVoiceList;

    /* The first channel listed is the highest priority.
     * This data comes from the Scaleable Polyphony message. */
    unsigned char   channelsInPriorityOrder[MIDI_NUM_CHANNELS];
    unsigned char   cumulativeMIPsInPriorityOrder[MIDI_NUM_CHANNELS];
    /** Number of Priorities specified in a bona fide ScaleablePolyphony SysEx message. */
    unsigned char   numPrioritiesSpecified;

    unsigned char   maxVoices;  /* Maximum allowable active voices. Set dynamically. */
    unsigned char   numVoices;  /**< Number of voices currently active. */
    unsigned char   maxVoicesUsed;  /**< Historical max of numVoices. Set dynamically. */

    unsigned char   lastCommand;
    unsigned char   lastByte;
    unsigned char   bytesNeeded;
    unsigned char   lastCommandSize;
    unsigned char   sysexParserState; /* State machine for system exclusive messages. */
}
SPMIDIContext_t;

typedef enum SysExStates_e
{
    /** Not parsing a sysex message. */
    SYSEX_STATE_IDLE = 0,
    /** Got F0 of a sysex message. */
    SYSEX_STATE_START,
    /** Ignore remainder of message because it does not apply to us. */
    SYSEX_STATE_IGNORE,
    /** Got 7F of a Universal Real Time SysEx header. */
    SYSEX_STATE_URT,
    /** Matched Device ID of Universal Real Time SysEx header. */
    SYSEX_STATE_URT_MATCHED,
    /** Got Scaleable Polyphony sub-ID#1 */
    SYSEX_STATE_URT_SP_1,
    /** Wait for the next channel number. */
    SYSEX_STATE_URT_SP_WAIT_CHANNEL,
    /** Wait for the next channel number. */
    SYSEX_STATE_URT_SP_WAIT_MIP,
    /** UNiversal NON Real Time */
    SYSEX_STATE_UNRT,
    /** Matched Device ID of Universal NON-Real Time SysEx header. */
    SYSEX_STATE_UNRT_MATCHED,
    /** General MIDI Message */
    SYSEX_STATE_UNRT_GM
} SysExStates_t;


/**********************************************************/
/********** Constant data for ROM *************************/
/**********************************************************/

/** Number of bytes in a message nc from 8c to Ec */
static const int channelByteLengths[] =
    {
        3, 3, 3, 3, 2, 2, 3
    };

/** Number of bytes in a message Fn from F0 to FF */
static const int systemByteLengths[] =
    {
        1, 2, 3, 2, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1
    };

static const unsigned char defaultChannelsInPriorityOrder[] =
    {
        9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15
    };

/** Some drums are in exclusion groups like the 3 hi-hat sounds. */
static const char sDrumExclusionGroups[ GMIDI_NUM_DRUMS ] =
    {
        0, 0, 0, 0, 0,   0, 0, 1, 0, 1, /* 35-44 */
        0, 1, 0, 0, 0,   0, 0, 0, 0, 0, /* 45-54 */
        0, 0, 0, 0, 0,   0, 0, 0, 0, 0, /* 55-64 */
        0, 0, 0, 0, 0,   0, 2, 2, 3, 3, /* 65-74 */
        0, 0, 0, 4, 4,   5, 5,          /* 75-81 */
    };

/** Drums have their own pan settings independant of the channel setting. */
static const char sDrumPan[ GMIDI_NUM_DRUMS ] =
    {
        64, 64, 64, 64, 54,   64, 34, 84, 46, 84, /* 35-44 */
        58, 84, 70, 82, 84,   94, 44, 44, 44, 74, /* 45-54 */
        54, 84, 44, 29, 44,   99, 99, 39, 39, 44, /* 55-64 */
        84, 84, 29, 29, 29,   24, 99, 99, 94, 94, /* 65-74 */
        84, 99, 99, 44, 44,   24, 24,          /* 75-81 */
    };

typedef void (SPMIDI_CommandHandler)( SPMIDIContext_t *spmc, int channelIndex, int data1, int data2 );

/****************************************************************/
/***** Prototypes for static functions. *************************/
/****************************************************************/
static void SPMIDI_HandleDummyCommand( SPMIDIContext_t *spmc, int channelIndex, int data1, int data2 );
static void SPMIDI_HandleNoteOn( SPMIDIContext_t *spmc, int channelIndex, int pitch, int velocity );
static void SPMIDI_HandleNoteOff( SPMIDIContext_t *spmc, int channelIndex, int pitch, int velocity );
static void SPMIDI_HandleProgramChange( SPMIDIContext_t *spmc, int channelIndex, int program, int dummy );
static void SPMIDI_HandleChannelAftertouch( SPMIDIContext_t *spmc, int channelIndex, int program, int dummy );
static void SPMIDI_HandleControlChange( SPMIDIContext_t *spmc, int channelIndex, int controller, int data );
static void SPMIDI_HandlePitchBend( SPMIDIContext_t *spmc, int channelIndex, int lsb, int msb );

/****************************************************************/
/***** Zero initialized read/writeable memory. ******************/
/****************************************************************/

/* Function tables for parsing MIDI. */
static SPMIDI_CommandHandler *ChannelCommandHandlers[7];
static SPMIDI_CommandHandler *SystemCommandHandlers[16];

/* Have we called SPMIDI_Initialize()? */
static int sIsInitialized = 0;
DoubleList sContextList;

/**********************************************************/
/********** Allocate SPMIDIContext ************************/
/**********************************************************/
#if SPMIDI_SUPPORT_MALLOC
#define SPMIDIContext_Allocate()   SPMIDI_ALLOC_MEM( sizeof(SPMIDIContext_t), "SPMIDIContext_t" )
#define SPMIDIContext_Free(cntxt)  SPMIDI_FreeMemory(cntxt)
#else

/* Pool of players that can be allocated without using malloc. */
static SPMIDIContext_t  sSPMIDIContextPool[SPMIDI_MAX_NUM_CONTEXTS];
/* Array used to keep track of which ones are allocated. */
static char              sSPMIDIContextsAllocated[SPMIDI_MAX_NUM_CONTEXTS] = { 0 };

/* Scan array for unallocated player. Allocate it if free. */
static SPMIDIContext_t *SPMIDIContext_Allocate( void )
{
    int i;
    SPMIDIContext_t * context = NULL;

    /* Protect the following section of code from thread collisions. */
    SPMIDI_EnterCriticalSection();

    for( i=0; i<SPMIDI_MAX_NUM_CONTEXTS; i++ )
    {
        /* Warning! This is a non-atomic test and set. It is not thread safe.
        * The application should implement SPMIDI_EnterCriticalSection() if
        * multiple threads can collide in this code.
        */
        if( sSPMIDIContextsAllocated[i] == 0 )
        {
            sSPMIDIContextsAllocated[i] = 1;
            context = &sSPMIDIContextPool[i];
            break;
        }
    }

    SPMIDI_LeaveCriticalSection();

    return context;
}

/* Scan pool to match player being freed. Mark it as free if found. */
static void SPMIDIContext_Free( SPMIDIContext_t *player )
{
    int i;
    /* Protect the following section of code from thread collisions. */
    SPMIDI_EnterCriticalSection();
    for( i=0; i<SPMIDI_MAX_NUM_CONTEXTS; i++ )
    {
        if( &sSPMIDIContextPool[i] == player )
        {
            sSPMIDIContextsAllocated[i] = 0; /* Mark as free. */
            break;
        }
    }
    SPMIDI_LeaveCriticalSection();
}
#endif

/**********************************************************/
/********** Parser and Voice Allocator ********************/
/**********************************************************/

/********************************************************************
 * Default Dummy Handler
 */
static void SPMIDI_HandleDummyCommand( SPMIDIContext_t *spmc, int channelIndex, int data1, int data2 )
{
    (void) spmc;
    (void) channelIndex;
    (void) data1;
    (void) data2;
    return;
}

/********************************************************************/
/**
 * Mark a voice as available for allocation
 * and place it in the freeVoiceList.
 */
static void SPMIDI_UnassignVoiceFromChannel( SPMIDIContext_t *spmc, SPMIDIVoiceTracker_t *voiceTracker )
{
    SPMIDIChannel_t *channel = &spmc->channels[voiceTracker->channel];
    DBUGMSG("SPMIDI_UnassignVoiceFromChannel( ");
    DBUGNUMH( (long) voiceTracker );
    DBUGMSG(" )\n");

    /* Remove from Channel list. */
    DLL_Remove( &voiceTracker->node );
    voiceTracker->channel = CHANNEL_FREE;
    voiceTracker->isSustaining = 0;

    DLL_AddTail( &spmc->freeVoiceList, &voiceTracker->node );

    channel->numVoices -= 1;
    spmc->numVoices -= 1;
}

/********************************************************************/
/**
 * Mark a voice as in-use on a channel.
 */
static void SPMIDI_AssignVoiceToChannel( SPMIDIContext_t *spmc, SPMIDIVoiceTracker_t *voiceTracker, int channelIndex )
{
    SPMIDIChannel_t *channel = &spmc->channels[channelIndex];
    DBUGMSG("SPMIDI_AssignVoiceToChannel( ");
    DBUGNUMH( (long) voiceTracker );
    DBUGMSG(" )\n");

    DLL_Remove( &voiceTracker->node );
    DLL_AddTail( &channel->voiceList, &voiceTracker->node );
    assert( voiceTracker->channel == CHANNEL_FREE );
    voiceTracker->channel = (unsigned char) channelIndex;

    channel->numVoices += 1;
    spmc->numVoices += 1;
    if( spmc->numVoices > spmc->maxVoicesUsed )
        spmc->maxVoicesUsed = spmc->numVoices;
}

/********************************************************************/
/**
 * Move voice to free list when it finishes.
 */
static void SPMIDI_AutoStopCallback( int voiceIndex, void *userData )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) userData;
    SPMIDIVoiceTracker_t *voiceTracker = &spmc->voiceTrackers[voiceIndex];
    SPMIDI_UnassignVoiceFromChannel( spmc, voiceTracker );
}

/********************************************************************/
/**
 * Turn on a voiceTracker.
 */
static void SPMIDI_VoiceOn( SPMIDIContext_t *spmc, SPMIDIVoiceTracker_t *voiceTracker,
                            int channelIndex, int noteIndex, int velocity )
{
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    voiceTracker->timeOn = spmc->frameCount;
    voiceTracker->isOn = TRUE;
    /* Clear this because OFF is no longer pending. */
    voiceTracker->isSustaining = FALSE;
    voiceTracker->isStifled = FALSE;

    if( channel->isDrum )
    {
        int pan;
        int drumIndex;

        /* Clip to legal range. */
        if( noteIndex < GMIDI_FIRST_DRUM )
            noteIndex = GMIDI_FIRST_DRUM;
        else if( noteIndex > GMIDI_LAST_DRUM )
            noteIndex = GMIDI_LAST_DRUM;

        /* Each drum has a specific pan setting. */
        drumIndex = noteIndex - GMIDI_FIRST_DRUM;
        pan = sDrumPan[ drumIndex ];
        voiceTracker->exclusionGroup = sDrumExclusionGroups[ drumIndex ];

        spmc->synth->TriggerDrum( spmc->synth, voiceTracker->id, channelIndex, noteIndex, velocity, pan );
    }
    else
    {
        spmc->synth->NoteOn( spmc->synth, voiceTracker->id, channelIndex,
                             noteIndex, velocity, channel->pan );
    }
    voiceTracker->noteIndex = (unsigned char) noteIndex;
    voiceTracker->velocity = (unsigned char) velocity;
    
/*
    PRTMSG("SPMIDI_VoiceOn: id = "); PRTNUMD( voiceTracker->id );
    PRTMSG(", chan = "); PRTNUMD( channelIndex );
    PRTMSG(", frame = "); PRTNUMD( spmc->frameCount );
    PRTMSG(", noteIndex = "); PRTNUMD( noteIndex );
    PRTMSG("\n");
*/
}

/********************************************************************/
/**
 * Turn off a voiceTracker.
 */
static void SPMIDI_VoiceOff( SPMIDIContext_t *spmc, SPMIDIVoiceTracker_t *voiceTracker )
{
    voiceTracker->timeOff = spmc->frameCount;
    voiceTracker->isOn = FALSE;

    spmc->synth->NoteOff( spmc->synth, voiceTracker->id );
/*
    PRTMSG("SPMIDI_VoiceOff: id = "); PRTNUMD( voiceTracker->id );
    PRTMSG(", frame = "); PRTNUMD( spmc->frameCount );
    PRTMSG(", timeOn = "); PRTNUMD( voiceTracker->timeOn );
    PRTMSG(", chan = "); PRTNUMD( voiceTracker->channel );
    PRTMSG(", pitch = "); PRTNUMD( voiceTracker->pitch );
    PRTMSG("\n");
*/
}

#define STEALMSG(x) /* PRTMSG(x) */
#define STEALNUMD(x) /* PRTNUMD(x) */

/********************************************************************/
/**
 * Steal a voice using a combination of weighted factors which include:
 * <ul>
 *  <li>Scaleable Polyphony MIP message</li>
 *  <li>Current note volume if after NoteOff</li>
 *  <li>Whether after NoteOff</li>
 *  <li>Whether currently stifled</li>
 *  <li>Channel Priority</li>
 * </ul>
 */
static SPMIDIVoiceTracker_t *SPMIDI_SelectVoiceToSteal( SPMIDIContext_t *spmc, int requestingChannelIndex )
{
    SPMIDIVoiceTracker_t *bestVoice = NULL;
    int priority;
    int limitedNoteCount = 0;
    int excessNotes;
    SPMIDIChannel_t *bestSPMChannel = NULL;
    int i;
    int maxScore = 0;

    if( spmc->numPrioritiesSpecified > 0 )
    {
        /* Scaleable Polyphony
         * Scan channel from highest to lowest priority, counting voices over quota.
         * Steal the first voice on a channel that exceeds the limit set by
         * the MIP table.
         */
        for( priority=SPM_HIGHEST_PRIORITY; priority<=SPM_LOWEST_PRIORITY; priority++ )
        {
            int channelIndex = spmc->channelsInPriorityOrder[ priority ];
            SPMIDIChannel_t *channel = &spmc->channels[channelIndex];
            int cumulativePolyphony = spmc->cumulativeMIPsInPriorityOrder[ priority ];
            int numVoices = channel->numVoices;
            /* Count one extra voice on this channel for the new note being added. */
            if( channelIndex == requestingChannelIndex )
                numVoices += 1;

            limitedNoteCount += numVoices;
            excessNotes = limitedNoteCount - cumulativePolyphony;
            if( excessNotes > 0 )
            {
                limitedNoteCount -= excessNotes;
                bestSPMChannel = channel;
            }
        }
    }

#define ALLOC_WEIGHT_OFF       (100)
#define ALLOC_WEIGHT_TIMEOFF   (1)
#define MAX_TIMEOFF_COUNTED    (300)
#define ALLOC_WEIGHT_STIFLED   (1000)
#define ALLOC_WEIGHT_VELOCITY  (200)
#define ALLOC_WEIGHT_SAMECHAN  (90)
#define ALLOC_WEIGHT_CHANPRI   (4)
#define ALLOC_WEIGHT_SPMIPS    (300)

    STEALMSG("--- steal -------------------------\n");
    for( i=0; i<SPMIDI_MAX_VOICES; i++ )
    {
        SPMIDIVoiceTracker_t *voiceTracker = &spmc->voiceTrackers[i];

        if( voiceTracker->channel != CHANNEL_FREE )
        {
            SPMIDIChannel_t *channel = &spmc->channels[voiceTracker->channel];
            int score = 1; /* So we always select a voice. */
            int temp;

            STEALMSG(", Channel = ");
            STEALNUMD( voiceTracker->channel );

            /* Account for result of Scaleable Polyphony determination. */
            if( channel == bestSPMChannel )
            {
                temp = ALLOC_WEIGHT_SPMIPS;
                STEALMSG(", SPMIPS = ");
                STEALNUMD( temp );
                score += temp;
            }

            /* Note Off? */
            if( !voiceTracker->isOn )
            {
                int elapsedOff;

                temp = ALLOC_WEIGHT_OFF;
                STEALMSG(", NoteOff = ");
                STEALNUMD( temp );
                score += temp;

                elapsedOff = (spmc->frameCount - voiceTracker->timeOff);
                if( elapsedOff > MAX_TIMEOFF_COUNTED )
                    elapsedOff = MAX_TIMEOFF_COUNTED;
                temp = elapsedOff * ALLOC_WEIGHT_TIMEOFF;
                STEALMSG(", TimeOff = ");
                STEALNUMD( temp );
                score += temp;
            }

            /* Note on same channel as new note needs to be? */
            if( voiceTracker->channel == requestingChannelIndex )
            {
                temp = ALLOC_WEIGHT_SAMECHAN;
                STEALMSG(", SameChan = ");
                STEALNUMD( temp );
                score += temp;
            }

            /* Note being stifled? */
            if( voiceTracker->isStifled )
            {
                temp = ALLOC_WEIGHT_STIFLED;
                STEALMSG(", Stifled = ");
                STEALNUMD( temp );
                score += temp;
            }

            /* Channel Priority? */
            {
                temp = (MIDI_NUM_CHANNELS - channel->priority) * ALLOC_WEIGHT_CHANPRI;
                score += temp;
                STEALMSG(", ChanPri = ");
                STEALNUMD( temp );
            }

            /* Note quiet? */
            temp = ((127 - voiceTracker->velocity) * ALLOC_WEIGHT_VELOCITY) >> 7;
            STEALMSG(", Velocity =");
            STEALNUMD( temp );
            score += temp;

            STEALMSG(", Score = ");
            STEALNUMD( score );
            if( score > maxScore )
            {
                maxScore = score;
                bestVoice = voiceTracker;
                STEALMSG("   best so far **");
            }
            STEALMSG("\n");
        }
    }

    return bestVoice;
}

/********************************************************************/
/**
 * Assign a voice for a new note.
 * First look for the same note already playing.
 * Then look for a mutually exclusive drum voice.
 * If either of those are found, and free voices are available,
 * then stifle the old note and use the new one.
 *
 * Otherwise allocate a free voice or steal a voice if none free.
 */
static SPMIDIVoiceTracker_t *SPMIDI_AllocateVoice( SPMIDIContext_t *spmc, int channelIndex, int noteIndex )
{

    SPMIDIChannel_t *channel = &spmc->channels[channelIndex];
    SPMIDIVoiceTracker_t *voiceToUse = NULL;
    SPMIDIVoiceTracker_t *voiceToStifle = NULL;
    SPMIDIVoiceTracker_t *next;

    /* Look for a voice playing the same pitch on that channel. */
    DLL_FOR_ALL( SPMIDIVoiceTracker_t, next, &channel->voiceList )
    {
        /* Don't stifle a voice that is already stifled because we
         * we need to find matching voice that is playing the same note.
         */
        if( (next->noteIndex == noteIndex) && (next->isStifled == FALSE) )
        {
            voiceToStifle = next;
            break;
        }
    }

    /* If we didn't find one yet, then check for voices in same drum exclusion group. */
    if(( voiceToStifle == NULL ) && ( channel->isDrum ))
    {
        /* Look for voice in same drum exclusion group
         * such as short or long whistle.
         */
        int drumIndex = noteIndex - GMIDI_FIRST_DRUM;
        int exclusionGroup = sDrumExclusionGroups[ drumIndex ];
        if( exclusionGroup > 0 )
        {
            DLL_FOR_ALL( SPMIDIVoiceTracker_t, next, &channel->voiceList )
            {
                if( (next->exclusionGroup == exclusionGroup) && (next->isStifled == FALSE) )
                {
                    voiceToStifle = next;
                    break;
                }
            }
        }
    }

    /* Did we find a voice based on matching pitch or exclusion group? */
    if( voiceToStifle != NULL )
    {
        /* Have we run out of free voices? */
        if( DLL_IsEmpty( &spmc->freeVoiceList )  || (spmc->numVoices >= spmc->maxVoices) )
        {
            /* We must use the same voice over again because we have no extras. */
            voiceToUse = voiceToStifle;
            spmc->synth->StealVoice( spmc->synth, voiceToUse->id );
        }
        else
        {
            /* We have extra voices so just stifle this voice and use another. */
            voiceToStifle->isStifled = 1;
            spmc->synth->StifleVoice( spmc->synth, voiceToStifle->id );
            /* Grab first voice in free list which we know is not empty. */
            voiceToUse = (SPMIDIVoiceTracker_t *) DLL_First( &spmc->freeVoiceList );
        }
    }
    else
    {
        /* Have we run out of free voices? */
        if( DLL_IsEmpty( &spmc->freeVoiceList )  || (spmc->numVoices >= spmc->maxVoices) )
        {
            /* No other voices free so steal an active voice. */
            voiceToUse = SPMIDI_SelectVoiceToSteal( spmc, channelIndex );
            if( voiceToUse != NULL )
            {
                spmc->synth->StealVoice( spmc->synth, voiceToUse->id );
                SPMIDI_VoiceOff( spmc, voiceToUse );
                SPMIDI_UnassignVoiceFromChannel( spmc, voiceToUse );
            }
        }
        else
        {
            /* Grab first voice in free list which we know is not empty. */
            voiceToUse = (SPMIDIVoiceTracker_t *) DLL_First( &spmc->freeVoiceList );
        }

    }

    /* Did we find a voice that is not assigned? */
    if( (voiceToUse != NULL) && (voiceToUse->channel == CHANNEL_FREE) )
    {
        SPMIDI_AssignVoiceToChannel( spmc, voiceToUse, channelIndex );
    }

    return voiceToUse;
}


/********************************************************************/
/**
 * Call application function that responds to telephone vibrator commands.
 */
static void SPMIDI_CallVibrator( SPMIDIContext_t *spmc, int noteIndex, int velocity )
{
    if( spmc->vibratorCallback != NULL )
    {
        (*(spmc->vibratorCallback))( spmc->vibratorUserData, noteIndex, velocity );
    }
}

/********************************************************************/
/**
 * Handle NoteOn Commands
 */
static void SPMIDI_HandleNoteOn( SPMIDIContext_t *spmc, int channelIndex, int noteIndex, int velocity )
{
    printf("SPMIDI_HandleNoteON(%d) ========\n", noteIndex);
    if( velocity == 0 )
    {
        SPMIDI_HandleNoteOff( spmc, channelIndex, noteIndex, velocity );
    }
    else
    {
        SPMIDIChannel_t *channel = &spmc->channels[channelIndex];

        if( channel->isVibrating )
        {
            SPMIDI_CallVibrator( spmc, noteIndex, velocity );
        }
        else
        {
            if( !channel->isMuted && !channel->isDisabled )
            {
                SPMIDIVoiceTracker_t *voiceTracker = SPMIDI_AllocateVoice( spmc, channelIndex, noteIndex );
                /* Did we get a voice? If not then it must not have been an important channel. */
                if( voiceTracker != NULL )
                {
                    SPMIDI_VoiceOn( spmc, voiceTracker, channelIndex, noteIndex, velocity );
                }
            }
        }
    }
}

/********************************************************************/
/**
 * Handle NoteOff Commands
 */
static void SPMIDI_HandleNoteOff( SPMIDIContext_t *spmc, int channelIndex, int noteIndex, int velocity )
{
    SPMIDIChannel_t *channel = &spmc->channels[channelIndex];
    SPMIDIVoiceTracker_t *voice = NULL;
    (void) velocity;

    printf("SPMIDI_HandleNoteOFF(%d) =========\n", noteIndex);

    /* Just call app if this is a vibrating channel. */
    if( channel->isVibrating )
    {
        SPMIDI_CallVibrator( spmc, noteIndex, 0 );
        return;
    }

    /* Find voice that is on and playing the pitch. */
    DLL_FOR_ALL( SPMIDIVoiceTracker_t, voice, &channel->voiceList )
    {
        if( (voice->noteIndex == noteIndex) && voice->isOn && !voice->isSustaining)
        {
            if( channel->isSustained )
            {
                /* Mark for turning off later. */
                voice->isSustaining = 1;
            }
            else
            {
                SPMIDI_VoiceOff( spmc, voice );
            }
            break;
        }
    }
}

/********************************************************************
 * Update Tuning information on a channel.
 */
static void SPMIDI_UpdateTuning( SPMIDIContext_t *spmc, int channelIndex )
{
    FXP16 octaveOffset16;
    spmSInt32 semitoneOffset16;
    spmSInt32 fineOffset13, coarseOffset13;
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    if( channelIndex != MIDI_RHYTHM_CHANNEL_INDEX )
    {
        /* Fine tuning with a resolution of 1/8192 semitones. */
        fineOffset13 = ((channel->tuningFineMSB << 7) + channel->tuningFineLSB) - (0x40<<7);
        /* Semitone offset shifted to same resolution as FineTuning. */
        coarseOffset13 = (channel->tuningCoarseMSB - 0x40) << 13;
        semitoneOffset16 = (coarseOffset13 + fineOffset13) << 3;
        /* Convert from semitone offset to fractional octave offset. */
        octaveOffset16 = semitoneOffset16 / SEMITONES_PER_OCTAVE;  /* DIVIDE - tuning control */
        /* Update synthesizer channel. */
        spmc->synth->SetChannelTuning( spmc->synth, channelIndex, octaveOffset16 );
        
        DBUGMSG("SPMIDI_UpdateTuning: channel = ");
        DBUGNUMD( channelIndex );
        DBUGMSG(", semitoneOffset16 = ");
        DBUGNUMH( semitoneOffset16 );
        DBUGMSG("\n");
    }
}

/********************************************************************
 * Update Pitch Bend information on a channel.
 */
static void SPMIDI_UpdatePitchBend( SPMIDIContext_t *spmc, int channelIndex )
{
    spmSInt32 semitoneOffset16;
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    semitoneOffset16 = ((spmSInt32) channel->bendRange * (spmSInt32) channel->bend) >> 5;
    spmc->synth->SetChannelPitchBend( spmc->synth, channelIndex, semitoneOffset16 );
}

/********************************************************************
 * Send noteOff commands for all notes on a channel.
 */
static void SPMIDI_AllNotesOff( SPMIDIContext_t *spmc, int channelIndex )
{
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    SPMIDIVoiceTracker_t *voice = NULL;
    /* Turn off each voice */
    DLL_FOR_ALL( SPMIDIVoiceTracker_t, voice, &channel->voiceList )
    {
        SPMIDI_HandleNoteOff( spmc, channelIndex, voice->noteIndex, 0 );
    }
}

/********************************************************************
 * Reset all controllers to their default state.
 */
static void SPMIDI_ResetAllControllers( SPMIDIContext_t *spmc, int channelIndex )
{
    SPMIDI_HandleControlChange( spmc, channelIndex,  MIDI_CONTROL_MODULATION,   0 ); /* Modulation */
    SPMIDI_HandleControlChange( spmc, channelIndex,  MIDI_CONTROL_EXPRESSION, 127 );
    SPMIDI_HandleControlChange( spmc, channelIndex,  MIDI_CONTROL_SUSTAIN,   0 );
    SPMIDI_HandleControlChange( spmc, channelIndex,  65,   0 );
    SPMIDI_HandleControlChange( spmc, channelIndex,  66,   0 );
    SPMIDI_HandleControlChange( spmc, channelIndex,  67,   0 );

    /* Set default coarse tuning. */
    SPMIDI_HandleControlChange( spmc, channelIndex, MIDI_CONTROL_RPN_MSB, 0 );
    SPMIDI_HandleControlChange( spmc, channelIndex, MIDI_CONTROL_RPN_LSB, MIDI_RPN_COARSE_TUNING );
    SPMIDI_HandleControlChange( spmc, channelIndex, MIDI_CONTROL_DATA_ENTRY,   MIDI_OFFSET_NONE ); /* Data Entry MSB */

    /* Set default fine tuning. */
    SPMIDI_HandleControlChange( spmc, channelIndex, MIDI_CONTROL_RPN_LSB, MIDI_RPN_FINE_TUNING );
    SPMIDI_HandleControlChange( spmc, channelIndex, MIDI_CONTROL_DATA_ENTRY,  MIDI_OFFSET_NONE ); /* Data Entry MSB */
    SPMIDI_HandleControlChange( spmc, channelIndex, 
        MIDI_CONTROL_DATA_ENTRY + MIDI_CONTROL_LSB_OFFSET,   0 ); /* Data Entry LSB */

    /* Set RPN to NULL parameter. */
    SPMIDI_HandleControlChange( spmc, channelIndex, MIDI_CONTROL_RPN_LSB, 127 ); /* RPN LSB */
    SPMIDI_HandleControlChange( spmc, channelIndex, MIDI_CONTROL_RPN_MSB, 127 ); /* RPN MSB */

    /* Important; Data Entry values must be set to zero AFTER setting RPN to NULL param. */
    SPMIDI_HandleControlChange( spmc, channelIndex,  MIDI_CONTROL_DATA_ENTRY,   0 ); /* Data Entry MSB */
    SPMIDI_HandleControlChange( spmc, channelIndex, 
        MIDI_CONTROL_DATA_ENTRY + MIDI_CONTROL_LSB_OFFSET,   0 ); /* Data Entry LSB */

    SPMIDI_HandlePitchBend( spmc, channelIndex, 0, 0x40 );
}

/********************************************************************
 * Handle Registered Parameter Number LSB updates.
 */
static void SMPIDI_HandleParamLSB( SPMIDIContext_t *spmc, int channelIndex, int value )
{
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    int temp;
    switch( channel->regParamNumber )
    {
    case MIDI_RPN_BEND_RANGE: /* Convert Pitch Bend Range LSB from cents to binary fraction. */
        temp = (value << 8) / 100;  /* DIVIDE - pitch bend */
        channel->bendRange = (channel->bendRange & 0xFF00) | temp;
        SPMIDI_UpdatePitchBend( spmc, channelIndex );
        break;
    case MIDI_RPN_FINE_TUNING: /* Update Fine Tuning. */
        channel->tuningFineLSB = (unsigned char) value;
        SPMIDI_UpdateTuning( spmc, channelIndex );
        break;
    /* There is no LSB for Coarse Tuning. */
    default:
        break;
    }
}

/********************************************************************
 * Handle Registered Parameter Number MSB updates.
 */
static void SMPIDI_HandleParamMSB( SPMIDIContext_t *spmc, int channelIndex, int value )
{
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    switch( channel->regParamNumber )
    {
    case MIDI_RPN_BEND_RANGE: /* Pitch Bend Range */
        channel->bendRange = value << 8;
        SPMIDI_UpdatePitchBend( spmc, channelIndex );
        break;
    case MIDI_RPN_FINE_TUNING: /* Update Fine Tuning. */
        channel->tuningFineMSB = (unsigned char) value;
        SPMIDI_UpdateTuning( spmc, channelIndex );
        break;
    case MIDI_RPN_COARSE_TUNING: /* Update Coarse Tuning. */
        channel->tuningCoarseMSB = (unsigned char) value;
        SPMIDI_UpdateTuning( spmc, channelIndex );
        break;
    default:
        break;
    }
}

/********************************************************************
 * Handle sustain pedal.
 * When pedal turned off, then turn off all pending notes.
 */
static void SPMIDI_HandleSustainPedal( SPMIDIContext_t *spmc, SPMIDIChannel_t *channel, int value )
{
    channel->isSustained = (unsigned char) (value >= 64);
    if( !channel->isSustained ) /* OFF */
    {
        /* Turn off all pending notes. */
        SPMIDIVoiceTracker_t *voice = NULL;
        DLL_FOR_ALL( SPMIDIVoiceTracker_t, voice, &channel->voiceList )
        {
            if( voice->isSustaining )
            {
                voice->isSustaining = 0;
                SPMIDI_VoiceOff( spmc, voice );
            }
        }

    }
}

/********************************************************************
 * Test whether we are in special SP-MIDI vibrator mode where the TelephoneRing
 * instrument causes the vibrator to vibrate.
 */
static void SPMIDI_UpdateVibrator( SPMIDIChannel_t *channel )
{
    channel->isVibrating = (unsigned char) ((channel->program == SPMIDI_VIBRATOR_PROGRAM) &&
                                            (channel->insBank == SPMIDI_VIBRATOR_BANK));
}

/********************************************************************
 * Handle Program Change Commands
 */
static void SPMIDI_HandleProgramChange( SPMIDIContext_t *spmc, int channelIndex, int program, int dummy )
{
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    (void) dummy;

    DBUGMSG("SPMIDI_HandleProgramChange( ");
    DBUGNUMD( channelIndex );
    DBUGMSG(", ");
    DBUGNUMD( program );
    DBUGMSG(")\n");

    spmc->synth->SetChannelProgram( spmc->synth, channelIndex, program );
    channel->program = (unsigned char) program;
    SPMIDI_UpdateVibrator( channel );
}


/********************************************************************
 * Handle Channel Aftertouch Commands
 */
static void SPMIDI_HandleChannelAftertouch( SPMIDIContext_t *spmc, int channelIndex, int value, int dummy )
{
    (void) dummy;
    spmc->synth->SetChannelAftertouch( spmc->synth, channelIndex, value );
}

/********************************************************************/
/**
 * Bank has changed so update the channel.
 */
static void SPMIDI_SetChannelBank( SPMIDIContext_t *spmc, int channelIndex, int bankIndex )
{
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    
    //PRTMSGNUMD("SPMIDI_SetChannelBank: channelIndex = ", channelIndex );
    //PRTMSGNUMD("SPMIDI_SetChannelBank: bankIndex = ", bankIndex );
    
    channel->insBank = (short) bankIndex;

    /* Any channel can be a drum channel if GMIDI_RHYTHM_BANK_MSB is specified.
     * And the Drum channel can be a melody channel if GMIDI_MELODY_BANK_MSB is specified.
     */
    if( channelIndex == MIDI_RHYTHM_CHANNEL_INDEX )
    {
        channel->isDrum = (unsigned char) (channel->insBank != (GMIDI_MELODY_BANK_MSB << SPMIDI_BANK_MSB_SHIFT) );
    }
    else
    {
        channel->isDrum = (unsigned char) (channel->insBank == (GMIDI_RHYTHM_BANK_MSB << SPMIDI_BANK_MSB_SHIFT) );
    }

    spmc->synth->SetChannelBank( spmc->synth, channelIndex, channel->insBank );
    SPMIDI_UpdateVibrator( channel );
}

/********************************************************************
 * Handle Control Change Commands
 */
static void SPMIDI_HandleControlChange( SPMIDIContext_t *spmc, int channelIndex, int controller, int value )
{
    int temp;
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    DBUGNUMD( controller );
    DBUGMSG( " = controller\n" );

    switch( controller )
    {
    case MIDI_CONTROL_BANK:
        /* LSB should not be preserved and should be set to zero. */
        temp = (short) (value << SPMIDI_BANK_MSB_SHIFT);
        SPMIDI_SetChannelBank( spmc, channelIndex, temp );
        break;
    case MIDI_CONTROL_MODULATION: /* Modulation Depth. */
        spmc->synth->SetChannelModDepth( spmc->synth, channelIndex, value );
        break;
    case MIDI_CONTROL_DATA_ENTRY: /* Data entry MSB. */
        SMPIDI_HandleParamMSB( spmc, channelIndex, value );
        break;
    case MIDI_CONTROL_VOLUME:
        spmc->synth->SetChannelVolume( spmc->synth, channelIndex, value );
        break;
    case MIDI_CONTROL_PAN:
        channel->pan = (unsigned char) value;
        break;
    case MIDI_CONTROL_EXPRESSION:
        spmc->synth->SetChannelExpression( spmc->synth, channelIndex, value );
        break;
    case (MIDI_CONTROL_BANK + MIDI_CONTROL_LSB_OFFSET):
        /* Preserve MSB with mask and set LSB */
        temp = (short) ((channel->insBank & ~0x7F) | value);
        SPMIDI_SetChannelBank( spmc, channelIndex, temp );
        break;
    case (MIDI_CONTROL_DATA_ENTRY + MIDI_CONTROL_LSB_OFFSET): /* Data entry LSB. */
        SMPIDI_HandleParamLSB( spmc, channelIndex, value );
        break;
    case MIDI_CONTROL_SUSTAIN:
        SPMIDI_HandleSustainPedal( spmc, channel, value );
        break;
    case MIDI_CONTROL_NONRPN_LSB: /* NonRPM LSB */
        channel->nonRegParamNumber = (short) ((channel->nonRegParamNumber & 0x3F80) | value);
        /* Turn off RPN so we don't respond when data entry occurs. */
        channel->regParamNumber = SPM_INVALID_PARAM_NUMBER;
        break;
    case MIDI_CONTROL_NONRPN_MSB: /* NonRPM MSB */
        channel->nonRegParamNumber = (short) ((channel->nonRegParamNumber & 0x7F) | (value << 7));
        channel->regParamNumber = SPM_INVALID_PARAM_NUMBER;
        break;
    case MIDI_CONTROL_RPN_LSB: /* RPM LSB */
        channel->regParamNumber = (short) ((channel->regParamNumber & 0x3F80) | value);
        channel->nonRegParamNumber = SPM_INVALID_PARAM_NUMBER;
        break;
    case MIDI_CONTROL_RPN_MSB: /* RPM MSB */
        channel->regParamNumber = (short) ((channel->regParamNumber & 0x7F) | (value << 7));
        channel->nonRegParamNumber = SPM_INVALID_PARAM_NUMBER;
        break;
    case MIDI_CONTROL_ALLSOUNDOFF:
        spmc->synth->AllSoundOff( spmc->synth, channelIndex );
        break;
    case 121:
        SPMIDI_ResetAllControllers( spmc, channelIndex );
        break;
    case MIDI_CONTROL_ALLNOTESOFF:
        SPMIDI_AllNotesOff( spmc, channelIndex );
        break;
    default:
        break;
    }
}

/********************************************************************
 * Handle PitchBend Commands
 */
static void SPMIDI_HandlePitchBend( SPMIDIContext_t *spmc, int channelIndex, int lsb, int msb )
{
    SPMIDIChannel_t *channel = &spmc->channels[ channelIndex ];
    channel->bend = ((msb << 7) | lsb) - MIDI_BEND_NONE;
    SPMIDI_UpdatePitchBend( spmc, channelIndex );
}

/*******************************************************************/
/**
 * Call this when a Scaleable Polyphony message is received,
 * or the maxVoices changes after an SP message is received.
 *
 * On entry, these must be set:
 *     spmc->numPrioritiesSpecified
 *     spmc->channelsInPriorityOrder[]
 *     spmc->cumulativeMIPsInPriorityOrder[];
 *     spmc->maxVoices
 *
 */
static int SPMIDI_UpdateScaleablePolyphony( SPMIDIContext_t *spmc )
{
    int i, priority;
    SPMIDIChannel_t *channel;
    int cumulativePolyphony = 0;

    /* Disable all channels so any not specified are muted. */
    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        channel = &spmc->channels[i];
        /* PRTMSGNUMD("SPMIDI_UpdateScaleablePolyphony: mute channel", i ); */
        channel->isMuted = TRUE;
        /* Mark priority so we can assign channels to undefined priorities. */
        channel->priority = SPM_NO_PRIORITY;
    }

    /*
        PRTMSGNUMD("SPMIDI_UpdateScaleablePolyphony: numPrioritiesSpecified = ", spmc->numPrioritiesSpecified );
        PRTMSGNUMD("SPMIDI_UpdateScaleablePolyphony: maxVoices = ", spmc->maxVoices );
    */
    /* Unmute any channels whose MIP fit under the maxVoices. */
    for( priority=0; priority < spmc->numPrioritiesSpecified; priority++ )
    {
        int channelIndex = spmc->channelsInPriorityOrder[ priority ];
        cumulativePolyphony = spmc->cumulativeMIPsInPriorityOrder[ priority ];

        channel = &spmc->channels[ channelIndex ];
        channel->priority = (unsigned char) priority;

        if( cumulativePolyphony <= spmc->maxVoices )
        {
            channel->isMuted = FALSE;
            /* PRTMSGNUMD("SPMIDI_UpdateScaleablePolyphony: UNmute channel #", channelIndex ); */
        }
    }

    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        channel = &spmc->channels[i];

        /* Fix tables in case all channels were not specified. */
        if( channel->priority == SPM_NO_PRIORITY )
        {
            channel->priority = (unsigned char) priority;
            spmc->channelsInPriorityOrder[ priority ] = (unsigned char) i;
            spmc->cumulativeMIPsInPriorityOrder[ priority ] = (unsigned char) (cumulativePolyphony + 1);
            channel->isMuted = SPMIDI_MUTE_UNSPECIFIED_CHANNELS;
            /*
            PRTMSGNUMD("SPMIDI_UpdateScaleablePolyphony: channel not specified #", i );
            PRTMSGNUMD("SPMIDI_UpdateScaleablePolyphony: set to priority", priority );
            */
            priority++;
        }

        /* Turn off any voices playing on a muted channel. */
        if( channel->isMuted )
        {
            /* Turn off any voices playing this channel.*/
            SPMIDI_AllNotesOff( spmc, i );
        }
    }

    return 0;
}

/******************************************************************/
/**
 * Set scaleable polyphony tables to reasonable defaults
 * for traditional voice allocation behavior.
 */
static void SPMIDI_ResetScaleablePolyphony( SPMIDIContext_t *spmc )
{
    SPMIDIChannel_t *channel;
    int priority;

    for( priority=SPM_HIGHEST_PRIORITY; priority<=SPM_LOWEST_PRIORITY; priority++ )
    {
        int channelIndex = defaultChannelsInPriorityOrder[ priority ];
        channel = &spmc->channels[ channelIndex ];
        channel->priority = (unsigned char) priority;
        spmc->channelsInPriorityOrder[priority] = (unsigned char) channelIndex;
        spmc->cumulativeMIPsInPriorityOrder[priority] = (unsigned char) spmc->maxVoices;
        channel->isMuted = FALSE;
    }
    spmc->numPrioritiesSpecified = 0;
}

/*******************************************************************/
static void SPMIDI_GeneralOn( SPMIDIContext_t *spmc )
{
    int i;

    DBUGMSG("SPMIDI_GeneralOn()\n");
    spmc->synth->SetGeneralMIDIMode( spmc->synth, 1 );

    SPMIDI_ResetScaleablePolyphony(spmc);

    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
#if SPMIDI_ME3000
        int defaultBank = (i == MIDI_RHYTHM_CHANNEL_INDEX) ?
            GMIDI_RHYTHM_BANK_MSB :
            GMIDI_MELODY_BANK_MSB;
#else
        int defaultBank = 0; // TODO - how should we handle default bank?
#endif
        SPMIDI_HandleControlChange( spmc, i, MIDI_CONTROL_BANK, defaultBank );

        SPMIDI_HandleProgramChange( spmc, i, 0, 0 );

        SPMIDI_HandleControlChange( spmc, i, MIDI_CONTROL_VOLUME, 100 );
        SPMIDI_HandleControlChange( spmc, i, MIDI_CONTROL_PAN,  64 );

        /* Set pitch bend range back to 2. */
        /* Set RPN to 0,0 */
        SPMIDI_HandleControlChange( spmc, i, MIDI_CONTROL_RPN_LSB,  0 );
        SPMIDI_HandleControlChange( spmc, i, MIDI_CONTROL_RPN_MSB,  0 );
        SPMIDI_HandleControlChange( spmc, i, MIDI_CONTROL_DATA_ENTRY,  2 );

        SPMIDI_ResetAllControllers( spmc, i );
    }
}

/*******************************************************************/
static void SPMIDI_GeneralOff( SPMIDIContext_t *spmc )
{
    spmc->synth->SetGeneralMIDIMode( spmc->synth, 0 );
}

/*******************************************************************/
static void SPMIDI_ParseSysExByte( SPMIDIContext_t *spmc, int byte )
{
    /* If sysex is for another device then ignore remainder of message. */
    int nextState = SYSEX_STATE_IGNORE;

    DBUGNUMH( byte );
    DBUGMSG( " = sysex\n" );
    DBUGNUMH( spmc->sysexParserState );
    DBUGMSG( " = sysex parser state\n" );

    switch( spmc->sysexParserState )
    {
        /* Got F0 of a sysex message. */
    case SYSEX_STATE_START:
        if( byte == 0x7F )
            nextState = SYSEX_STATE_URT;
        else if( byte == 0x7E )
            nextState = SYSEX_STATE_UNRT;
        break;

        /* Got 7F of a Universal Real Time SysEx header. Match device. 7F is all devices. */
    case SYSEX_STATE_URT:
        if( byte == 0x7F )
            nextState = SYSEX_STATE_URT_MATCHED; /* TODO - should we match on all! */
        break;

        /* Matched Device ID of Universal Real Time SysEx header. */
    case SYSEX_STATE_URT_MATCHED:
        if( byte == 0x0B )
            nextState = SYSEX_STATE_URT_SP_1; /* Scaleable Polyphony */
        break;

        /* Got Scaleable Polyphony sub-ID#1 */
    case SYSEX_STATE_URT_SP_1:
        if( byte == 0x01 )
        {
            spmc->numPrioritiesSpecified = 0;
            nextState = SYSEX_STATE_URT_SP_WAIT_CHANNEL; /* Start MIP chan/mip pairs*/

            DBUGMSG("SPMIDI_ParseSysExByte: MIPS message ---------------\n");
        }
        break;

        /* Wait for the next channel number for Scaleable Polyphony. */
    case SYSEX_STATE_URT_SP_WAIT_CHANNEL:
        if( byte == MIDI_EOX )
        {
            SPMIDI_UpdateScaleablePolyphony(spmc);
        }
        else if( (spmc->numPrioritiesSpecified >= MIDI_NUM_CHANNELS) ||
                 (byte < 0) ||
                 (byte >= MIDI_NUM_CHANNELS) )
        {
            /* Error - too many pairs or bad channel. */
            spmc->numPrioritiesSpecified = 0;
            nextState = SYSEX_STATE_IDLE;
        }
        else
        {
            /* Save channel index for this priority level. */
            DBUGMSG("SPMIDI_ParseSysExByte: channelIndex = ");
            DBUGNUMD( byte );
            spmc->channelsInPriorityOrder[ spmc->numPrioritiesSpecified ] = (unsigned char) byte;
            nextState = SYSEX_STATE_URT_SP_WAIT_MIP;
        }
        break;

        /* Wait for the next max polyphony for Scaleable Polyphony. */
    case SYSEX_STATE_URT_SP_WAIT_MIP:
        if( byte == MIDI_EOX )
        {
            /* Error - premature EOX. */
            spmc->numPrioritiesSpecified = 0;
        }
        else
        {
            /* Save Max Instantaneous Polyphony value */
            spmc->cumulativeMIPsInPriorityOrder[ spmc->numPrioritiesSpecified++ ] = (unsigned char) byte;
            nextState = SYSEX_STATE_URT_SP_WAIT_CHANNEL;
        }
        break;

        /* Got 7F of a Universal NONReal Time SysEx header. Match device. 7F is all devices. */
    case SYSEX_STATE_UNRT:
        if( byte == 0x7F )
            nextState = SYSEX_STATE_UNRT_MATCHED; /* TODO - should we match on all! */
        break;

        /* Matched Device ID of Universal Real Time SysEx header. */
    case SYSEX_STATE_UNRT_MATCHED:
        if( byte == 0x09 )
            nextState = SYSEX_STATE_UNRT_GM; /* General MIDI */
        break;

        /* General MIDI Message */
    case SYSEX_STATE_UNRT_GM:
        if( byte == 0x01 )
            SPMIDI_GeneralOn(spmc); /* General MIDI On */
        else if( byte == 0x02 )
            SPMIDI_GeneralOff(spmc); /* General MIDI Off */
        break;

    default:
        break;
    }

    /* End of SysEx */
    if( byte == MIDI_EOX )
    {
        nextState = SYSEX_STATE_IDLE;
    }

    spmc->sysexParserState = (unsigned char) nextState;
}

/*********************************************************************/
/**************** External API ***************************************/
/*********************************************************************/
#define SPMIDI_CHECK_START_VOID      if( (spmc == NULL) || (spmc->synth == NULL) ) return;
#define SPMIDI_CHECK_START      if( (spmc == NULL) || (spmc->synth == NULL) ) return SPMIDI_Error_NotStarted;

/* The following functions are documented in "spmidi.h". */
/*******************************************************************/
void SPMIDI_WriteCommand( SPMIDI_Context *context, int command, int data1, int data2 )
{
    int channelIndex;
    int index;
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;

    SPMIDI_CHECK_START_VOID;

    DBUGMSG("SPMIDI_WriteCommand( ");
    DBUGNUMH( command );
    DBUGMSG(", ");
    DBUGNUMD( data1 );
    DBUGMSG(", ");
    DBUGNUMD( data2 );
    DBUGMSG(" )\n");

    channelIndex = command & 0x0F;
    index = (command >> 4) & 0x07;

    if( index < 7 )
        ChannelCommandHandlers[index]( spmc, channelIndex, data1, data2 );
    else
        SystemCommandHandlers[channelIndex]( spmc, channelIndex, data1, data2 );
}

/********************************************************************/
int SPMIDI_GetBytesPerMessage( int command )
{
    if( (command < 0x80) || (command > 0xFF) )
        return 0;
    else if( command >= 0xF0 )
        return systemByteLengths[ command & 0x0F ];
    else
        return channelByteLengths[ (command >> 4) - 8 ];
}

/********************************************************************/
void SPMIDI_Write( SPMIDI_Context *context, const unsigned char *data, int numBytes )
{
    while( numBytes-- > 0 )
        SPMIDI_WriteByte( context, *data++ );
}

/********************************************************************/
void SPMIDI_WriteByte( SPMIDI_Context *context, int byte )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    SPMIDI_CHECK_START_VOID;
    if( byte >= 0x80 ) /* Command. */
    {
        /* System Real Time Status Bytes can come any time!*/
        if( byte >= 0xF8 )
        {
            /* Ignore it. TODO - what about SystemReset? Not likely in SMF. */
        }
        else
        {
            /* Any other command during a SysEx ends SysEx. */
            if( spmc->sysexParserState != SYSEX_STATE_IDLE )
            {
                SPMIDI_ParseSysExByte( spmc, MIDI_EOX );
            }

            if( byte == MIDI_SOX ) /* Start System Exclusive Command. */
            {
                spmc->sysexParserState = SYSEX_STATE_START;
            }
            else
            {
                /* Setup byte counter so we can wait for a complete message. */
                spmc->lastCommand = (unsigned char) byte;
                spmc->lastCommandSize = (unsigned char) SPMIDI_GetBytesPerMessage( (unsigned char) byte );
                spmc->bytesNeeded = (unsigned char) (spmc->lastCommandSize - 1);
            }
        }
    }
    else /* Data byte. */
    {
        /* Are we in the middle of a long sysex message? */
        if( spmc->sysexParserState != SYSEX_STATE_IDLE )
        {
            SPMIDI_ParseSysExByte( spmc, byte );
        }
        else if( --spmc->bytesNeeded == 0 )
        {
            if( spmc->lastCommandSize == 2 )
            {
                /* Just use latest byte. */
                SPMIDI_WriteCommand( context, spmc->lastCommand, byte, 0 );
            }
            else
            {
                /* Use stored byte. */
                SPMIDI_WriteCommand( context, spmc->lastCommand, spmc->lastByte, byte );
            }
            /* Prepare for running status. */
            spmc->bytesNeeded = (unsigned char) (spmc->lastCommandSize - 1);
        }
        else
        {
            spmc->lastByte = (unsigned char) byte;
        }
    }
}

/*********************************************************************/
int SPMIDI_SetMaxVoices( SPMIDI_Context *context, int maxNumVoices )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    SPMIDI_CHECK_START;
    if((maxNumVoices <= 0 ) || ( maxNumVoices > SPMIDI_MAX_VOICES ))
    {
        maxNumVoices = SPMIDI_MAX_VOICES;
    }
    spmc->maxVoices = (unsigned char) maxNumVoices;
    if( spmc->numPrioritiesSpecified > 0 )
    {
        SPMIDI_UpdateScaleablePolyphony(spmc);
    }
    return maxNumVoices;
}

/*********************************************************************/
int SPMIDI_GetMaxVoices( SPMIDI_Context *context )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    return spmc->maxVoices;
}

/** Initialize SPMIDI Library.
 * This should be called only once before calling SPMIDI_CreateContext().
 */
int SPMIDI_Initialize( void )
{
    int i;

    if( sIsInitialized == 0 )
    {
        SPMIDI_HostInit();

        /* Initialize list for tracking contexts. */
        DLL_InitList( &sContextList );

        /* Initialize function table here to prevent ARM compiler warnings. */
        i = 0;
        ChannelCommandHandlers[i++] = SPMIDI_HandleNoteOff;       /* 0x80 */
        ChannelCommandHandlers[i++] = SPMIDI_HandleNoteOn;        /* 0x90 */
        ChannelCommandHandlers[i++] = SPMIDI_HandleDummyCommand;  /* 0xA0 */
        ChannelCommandHandlers[i++] = SPMIDI_HandleControlChange; /* 0xB0 */
        ChannelCommandHandlers[i++] = SPMIDI_HandleProgramChange; /* 0xC0 */
        ChannelCommandHandlers[i++] = SPMIDI_HandleChannelAftertouch;  /* 0xD0 */
        ChannelCommandHandlers[i++] = SPMIDI_HandlePitchBend;      /* 0xE0 */

        /* Initialize function table here to prevent ARM compiler warnings. */
        for( i=0; i<16; i++ )
        {
            SystemCommandHandlers[i] = SPMIDI_HandleDummyCommand;
        }

        SS_Initialize();

        sIsInitialized += 1;
    }

    return 0;
}

/** Terminate SPMIDI Library.
 */
int SPMIDI_Terminate( void )
{
    if( sIsInitialized  )
    {
        SS_Terminate();
        SPMIDI_HostTerm();
        sIsInitialized = 0;
    }
    return 0;
}

/*********************************************************************/
int SPMIDI_CreateContext( SPMIDI_Context **contextPtr, int sampleRate )
{
    int i;
    int result;
    SPMIDIContext_t *spmc;

    if( sampleRate > SPMIDI_MAX_SAMPLE_RATE )
    {
        return SPMIDI_Error_OutOfRange;
    }

    /* Call it just in case it hadn't been called yet. */
    SPMIDI_Initialize();

    spmc = SPMIDIContext_Allocate();
    if( spmc == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }

    MemTools_Clear( (void *) spmc, sizeof(SPMIDIContext_t) );

    DBUGMSG("SPMIDI Engine (C) Mobileer, built " __DATE__ "\n");

    DLL_InitNode( &spmc->node );
    DLL_AddTail( &sContextList, &spmc->node );

    spmc->sampleRate = sampleRate;
    spmc->maxVoices = SPMIDI_MAX_VOICES;

    DBUGMSG("SPMIDI_CreateContext: createSynth\n");
    result = SS_CreateSynth( &spmc->synth, sampleRate );
    if( result < 0 )
    {
        goto error;
    }

    /* Initialize Voice Allocator */
    DBUGMSG("SPMIDI_CreateContext: init list\n");
    DLL_InitList( &spmc->freeVoiceList );
    for( i=0; i<SPMIDI_MAX_VOICES; i++ )
    {
        SPMIDIVoiceTracker_t *voiceTracker = &spmc->voiceTrackers[i];
        DLL_InitNode( &voiceTracker->node );
        DLL_AddTail( &spmc->freeVoiceList, &voiceTracker->node );
        voiceTracker->channel = CHANNEL_FREE;
        voiceTracker->id = (unsigned char) i;
    }

    /* Initialize Channels */
    DBUGMSG("SPMIDI_CreateContext: init channels\n");
    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        SPMIDIChannel_t *channel = &spmc->channels[i];
        DLL_InitList( &channel->voiceList );
        /* Set initial bend range to 2 semitones. */
        channel->bendRange = 2 << 8;
    }

    /* Request NO callback when voice finishes playing. */
    DBUGMSG("SPMIDI_CreateContext: set callback\n");
    spmc->synth->SetAutoStopCallback( spmc->synth, SPMIDI_AutoStopCallback, spmc );

    /* This causes a reset of the controllers and polyphony tables. */
    SPMIDI_GeneralOn(spmc);

    *contextPtr = (SPMIDI_Context *) spmc;
    return 0;

error:
    if( spmc != NULL )
    {
        SPMIDI_DeleteContext( (SPMIDI_Context *) spmc );
    }
    *contextPtr = NULL;
    return result;
}


/******************************************************************/
int SPMIDI_ReadFrames( SPMIDI_Context *context, void *samples, int numFrames, int samplesPerFrame, int bitsPerSample  )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    int result;
    int initialFrameCount;
    int bytesPerSample;
    int bufferIncrement;
    char *buffer = (char *) samples;

    SPMIDI_CHECK_START;
    if( samplesPerFrame > SPMIDI_MAX_SAMPLES_PER_FRAME )
    {
        return SPMIDI_Error_OutOfRange;
    }

    initialFrameCount = spmc->frameCount;

    bytesPerSample = (bitsPerSample <= 8) ? 1 :
                     ((bitsPerSample <= 16) ? 2 : 4);

    bufferIncrement = samplesPerFrame * SS_FRAMES_PER_BUFFER * bytesPerSample;

    while( numFrames >= SS_FRAMES_PER_BUFFER )
    {
        result = spmc->synth->SynthesizeBuffer( spmc->synth, buffer, samplesPerFrame, bitsPerSample );
        if( result < 0 )
            return result;
        buffer += bufferIncrement;
        numFrames -= SS_FRAMES_PER_BUFFER;
        spmc->frameCount += SS_FRAMES_PER_BUFFER;
    }

    return spmc->frameCount - initialFrameCount;
}


/******************************************************************/
int SPMIDI_EstimateMaxAmplitude( SPMIDI_Context *context, int numFrames, int samplesPerFrame )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    int result;
    int maxAmplitude = 0;

    SPMIDI_CHECK_START;

    while( numFrames >= SS_FRAMES_PER_BUFFER )
    {
        result = spmc->synth->EstimateMaxAmplitude( spmc->synth, samplesPerFrame );
        if( result < 0 )
            return result;
        numFrames -= SS_FRAMES_PER_BUFFER;
        spmc->frameCount += SS_FRAMES_PER_BUFFER;
        if( result > maxAmplitude )
        {
            maxAmplitude = result;
        }
    }

    return maxAmplitude;
}

/*******************************************************************/
void SPMIDI_SetMasterVolume( SPMIDI_Context *context, int masterVolume )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    spmc->synth->SetMasterVolume( spmc->synth, masterVolume );
}

/*******************************************************************/
int SPMIDI_StopAllVoices( void )
{
    int result = 0;
    int err;
    SPMIDIContext_t *spmc;

    if( sIsInitialized )
    {
        /* Look for a voice playing the same pitch on that channel. */
        DLL_FOR_ALL( SPMIDIContext_t, spmc, &sContextList )
        {
            err = spmc->synth->StopAllVoices( spmc->synth );
            if( err < 0 )
            {
                result = err;
            }
        }
    }
    return result;
}

/*******************************************************************/
int SPMIDI_SetParameter( SPMIDI_Context *context, SPMIDI_Parameter parameterIndex, int value )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    return spmc->synth->SetParameter( spmc->synth, parameterIndex, value );
}

/*******************************************************************/
int SPMIDI_GetParameter( SPMIDI_Context *context, SPMIDI_Parameter parameterIndex, int *valuePtr )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    return spmc->synth->GetParameter( spmc->synth, parameterIndex, valuePtr );
}

/*******************************************************************/
int SPMIDI_GetFrameCount( SPMIDI_Context *context )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    return spmc->frameCount;
}

/********************************************************************/
int SPMIDI_GetSampleRate( SPMIDI_Context *context )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    return spmc->sampleRate;
}

/********************************************************************/
int SPMIDI_GetActiveNoteCount( SPMIDI_Context *context )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    return spmc->numVoices;
}

/********************************************************************/
int SPMIDI_GetMaxNoteCount( SPMIDI_Context *context )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    return spmc->maxVoicesUsed;
}

/********************************************************************/
void SPMIDI_ResetMaxNoteCount( SPMIDI_Context *context )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    spmc->maxVoicesUsed = 0;
}

/********************************************************************/
int SPMIDI_GetChannelActiveNoteCount( SPMIDI_Context *context, int channelIndex )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    SPMIDIChannel_t *channel;
    if( (channelIndex < 0) || (channelIndex >= MIDI_NUM_CHANNELS) )
    {
        return SPMIDI_Error_IllegalChannel;
    }
    channel = &spmc->channels[channelIndex];
    return channel->numVoices;
}

/********************************************************************/
int SPMIDI_GetChannelEnable( SPMIDI_Context *context, int channelIndex )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    SPMIDIChannel_t *channel;
    if( (channelIndex < 0) || (channelIndex >= MIDI_NUM_CHANNELS) )
    {
        return SPMIDI_Error_IllegalChannel;
    }
    channel = &spmc->channels[channelIndex];
    /* Return opposite of internal flag. */
    return !channel->isDisabled;
}

/********************************************************************/
int SPMIDI_SetChannelEnable( SPMIDI_Context *context, int channelIndex, int onOrOff )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    SPMIDIChannel_t *channel;
    if( (channelIndex < 0) || (channelIndex >= MIDI_NUM_CHANNELS) )
    {
        return SPMIDI_Error_IllegalChannel;
    }
    channel = &spmc->channels[channelIndex];
    channel->isDisabled = (unsigned char) !onOrOff;
    return 0;
}

/********************************************************************/
int SPMIDI_DeleteContext( SPMIDI_Context *context )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    if( spmc->synth != NULL )
    {
        SS_DeleteSynth( spmc->synth );
        spmc->synth = NULL;
    }
    
    DLL_Remove( &spmc->node );

    SPMIDIContext_Free(spmc);
    return 0;
}

/********************************************************************/
int SPMIDI_GetFramesPerBuffer( void )
{
    return SS_FRAMES_PER_BUFFER;
}

/********************************************************************/
int SPMIDI_SetVibratorCallback( SPMIDI_Context *context, SPMIDI_VibratorCallback *vibratorCallback,
                                void *vibratorUserData )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    SPMIDI_CHECK_START;
    spmc->vibratorCallback = vibratorCallback;
    spmc->vibratorUserData = vibratorUserData;
    return 0;
}

/********************************************************************/
/** This routine is only used for debugging on desktop machines.
 * Do not bother to port it to embedded systems.
 */
void SPMIDI_PrintStatus( SPMIDI_Context *context )
{
#if defined(WIN32) || defined(MACOSX)
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) context;
    int chan;
    printf("--------------------------\n");
    printf("SPMIDI status: numFree = %d", DLL_CountNodes( &spmc->freeVoiceList ) );
    printf(", numVoices = %d", spmc->numVoices );
    printf(", maxUsed = %d", spmc->maxVoicesUsed );
    printf(", maxVoices = %d\n", spmc->maxVoices );
    for( chan=0; chan<MIDI_NUM_CHANNELS; chan++ )
    {
        SPMIDIChannel_t *channel = &spmc->channels[ chan ];
        if( channel->numVoices > 0 )
        {
            SPMIDIVoiceTracker_t *voice;
            printf("Channel: %d, #v = %d", chan, channel->numVoices );
            printf(", pri = %d", channel->priority);
            printf(", ins = %d (0x%X)", channel->program + 1, channel->program );
            if( chan != MIDI_RHYTHM_CHANNEL_INDEX )
            {
                printf( ", %s", MIDI_GetProgramName( channel->program ) );
            }
            printf("\n");
            /* printf("         numOn = %d, numOff = %d\n", channel->numOnMessages, channel->numOffMessages ); */
            /* Print voices on channel. */
            DLL_FOR_ALL( SPMIDIVoiceTracker_t, voice, &channel->voiceList )
            {
                printf("    Voice: noteIndex = %d, vel = %d, onAt %d", voice->noteIndex, voice->velocity, voice->timeOn );
                if( !voice->isOn )
                {
                    printf(", offAt %d", voice->timeOff );
                }
                if( voice->channel != chan )
                {
                    printf("ERROR - voice->channel = %d\n", voice->channel );
                }
                if( chan == MIDI_RHYTHM_CHANNEL_INDEX )
                {
                    printf( ", %s", MIDI_GetDrumName( voice->noteIndex ) );
                }
                printf("\n");
            }
        }
    }
#else

    (void) context;
#endif
}

#if SPMIDI_ME3000
int SPMIDI_LoadDLSOrchestra( SPMIDI_Context *spmidiContext, DLS_Orchestra_t *dlsOrch )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) spmidiContext;
    return SSDLS_LoadOrchestra( spmc->synth, dlsOrch );
}

SoftSynth *SPMIDI_GetSynth( SPMIDI_Context *spmidiContext )
{
    SPMIDIContext_t *spmc = (SPMIDIContext_t *) spmidiContext;
    return spmc->synth;
}
#endif /* SPMIDI_ME3000 */

#if SPMIDI_SUPPORT_LOADING
/** Download an instrument definition as a byte stream.
 * The contents of the definition are specific to the synthesizer in use.
 */
int SPMIDI_SetInstrumentDefinition( SPMIDI_Orchestra *orchestra, int insIndex, ResourceTokenMap_t *tokenMap, unsigned char *data, int numBytes )
{
    SPMIDI_StopAllVoices( );
    return SS_SetInstrumentDefinition( orchestra, insIndex, tokenMap, data, numBytes );
}

/** Map a MIDI program number to an instrument index.
 * This allows multiple programs to be mapped to a single instrument.
 */
int SPMIDI_SetInstrumentMap( SPMIDI_Orchestra *orchestra, int bankIndex, int programIndex, int insIndex )
{
    return SS_SetInstrumentMap( (HybridOrchestra_t *) orchestra, bankIndex, programIndex, insIndex );
}

/** Map a MIDI drum pitch to an instrument index.
 * This allows multiple drums to be mapped to a single instrument.
 */
int SPMIDI_SetDrumMap( SPMIDI_Orchestra *orchestra, int bankIndex, int programIndex, int noteIndex, int insIndex, int pitch )
{
    return SS_SetDrumMap( (HybridOrchestra_t *) orchestra, bankIndex, programIndex, noteIndex, insIndex, pitch );
}

/** Return Orchestra compiled with synthesizer.
 */
SPMIDI_Orchestra *SPMIDI_GetCompiledOrchestra( void )
{
    return (SPMIDI_Orchestra *) SS_GetCompiledOrchestra();
}

/* Delete WaveTable if WaveSet reference count is zero. */
int SPMIDI_UnloadWaveTable( SPMIDI_Orchestra *orchestra, spmSInt32 token )
{
    SPMIDI_StopAllVoices( );
    return SS_UnloadWaveTable( (HybridOrchestra_t *) orchestra, token );
}

/* Delete WaveSet if reference count is zero. */
int SPMIDI_UnloadWaveSet( SPMIDI_Orchestra *orchestra, spmSInt32 token )
{
    SPMIDI_StopAllVoices( );
    return SS_UnloadWaveSet( (HybridOrchestra_t *) orchestra, token );
}

#endif /* SPMIDI_SUPPORT_LOADING */
