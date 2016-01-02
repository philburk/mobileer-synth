/* $Id: qa_spmidi.c,v 1.6 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_spmidi.c
 * @brief Test note stealing based on the example in the Scaleable Polyphony MIDI spec.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/qa/qa_tools.h"

#define SAMPLE_RATE         (44100)
/* Use Drawbar organ because it sustains, and has short attack and release times. */
#define PROGRAM_ORGAN       (17)

static short samples[ SPMIDI_MAX_FRAMES_PER_BUFFER ];

static SPMIDI_Context *spmidiContext = NULL;

const unsigned char spSysExMessage1[] =
    {
        MIDI_SOX, 0x7F, 0x7F, 0x0B, 0x01, /* SysEx header */
        0, 3, /* 3 voices, { Channel, MIP}  pair */
        1, 6, /* 3 voices */
        2, 8, /* 2 voices */
        3, 9, /* 1 voice */
        4, 14, /* 5 voices */
        MIDI_EOX
    };
const unsigned char spSysExMessage2[] =
    {
        MIDI_SOX, 0x7F, 0x7F, 0x0B, 0x01, /* SysEx header */
        0, 2, /* { Channel, MIP}  pair */
        1, 6, /* Define a group. */
        2, 6,
        3, 9,
        MIDI_EOX
    };

/*******************************************************************/
static void VerifyChannelNotes( int channelIndex, int expectedNoteCount )
{
    int actualNoteCount = SPMIDI_GetChannelActiveNoteCount( spmidiContext, channelIndex );
    if( actualNoteCount != expectedNoteCount )
    {
        printf( "ERROR: on channel %d, expected %d notes, got %d !!!!\n",
                channelIndex, expectedNoteCount, actualNoteCount );
        QA_CountError();
    }
    else
    {
        printf( "SUCCESS: on channel %d, got %d notes\n",
                channelIndex, expectedNoteCount );
        QA_CountSuccess();
    }
}

/*******************************************************************/
static void KillTime( int numBuffers )
{
    int i;
    for( i=0; i<numBuffers; i++ )
    {
        SPMIDI_ReadFrames( spmidiContext, samples, SPMIDI_MAX_FRAMES_PER_BUFFER, 1, 16 );
    }
}

#define NOTE(ch) \
    SPMUtil_NoteOn( spmidiContext, ch, pitch++, 64 ); \
    KillTime( 2 );

#define NOTELOOP(ch,many) \
    for( i=0; i<many; i++ ) SPMUtil_NoteOn( spmidiContext, ch, pitch++, 64 ); \
    KillTime( 2 );


/*******************************************************************/
/**
 * Make sure notes can be played on all channels.
 */
static void TestReset( void )
{
    int pitch = 48;

    printf( "Test Reset =============\n" );

    SPMIDI_Write( spmidiContext, spSysExMessage1, sizeof( spSysExMessage1 ) );

    /* This reset should erase previous polyphony setting. */
    SPMUtil_Reset(spmidiContext);
    KillTime( 50 );

    /* Should now be using default polyphony settings. */
    SPMIDI_SetMaxVoices( spmidiContext, 3 );
    NOTE( 0 );
    NOTE( 1 );
    NOTE( 2 );
    VerifyChannelNotes( 0, 1 );
    VerifyChannelNotes( 1, 1 );
    VerifyChannelNotes( 2, 1 );
    VerifyChannelNotes( 3, 0 );
    NOTE( 3 );
    VerifyChannelNotes( 0, 0 );
    VerifyChannelNotes( 1, 1 );
    VerifyChannelNotes( 2, 1 );
    VerifyChannelNotes( 3, 1 );
}

/*******************************************************************/
/**
 * Make sure notes on muted channels are not activated.
 */
static void TestChannelMuting( void )
{
    int pitch = 48;

    printf( "Test Channel Muting =============\n" );

    SPMUtil_Reset(spmidiContext);
    KillTime( 50 );
    SPMIDI_Write( spmidiContext, spSysExMessage1, sizeof( spSysExMessage1 ) );

    /* Only channel 0 should be able to play notes. */
    SPMIDI_SetMaxVoices( spmidiContext, 3 );
    NOTE( 0 );
    NOTE( 1 );
    NOTE( 2 );
    NOTE( 3 );
    VerifyChannelNotes( 0, 1 );
    VerifyChannelNotes( 1, 0 );
    VerifyChannelNotes( 2, 0 );
    VerifyChannelNotes( 3, 0 );

    /* Only channel 0 and 1 should be able to play notes. */
    SPMIDI_SetMaxVoices( spmidiContext, 7 );
    NOTE( 0 );
    NOTE( 1 );
    NOTE( 2 );
    NOTE( 3 );
    VerifyChannelNotes( 0, 2 );
    VerifyChannelNotes( 1, 1 );
    VerifyChannelNotes( 2, 0 );
    VerifyChannelNotes( 3, 0 );

    /* Only channel 0,1 and 2 should be able to play notes. */
    SPMIDI_SetMaxVoices( spmidiContext, 8 );
    NOTE( 0 );
    NOTE( 1 );
    NOTE( 2 );
    NOTE( 3 );
    VerifyChannelNotes( 0, 3 );
    VerifyChannelNotes( 1, 2 );
    VerifyChannelNotes( 2, 1 );
    VerifyChannelNotes( 3, 0 );

    /* Channel 2 can no longer play notes so its notes will be turned off. */
    SPMIDI_SetMaxVoices( spmidiContext, 7 );
    KillTime( 50 );
    VerifyChannelNotes( 0, 3 );
    VerifyChannelNotes( 1, 2 );
    VerifyChannelNotes( 2, 0 );
    VerifyChannelNotes( 3, 0 );

    /* Channel 1 can no longer play notes so its notes will be turned off. */
    SPMIDI_SetMaxVoices( spmidiContext, 4 );
    KillTime( 50 );
    VerifyChannelNotes( 0, 3 );
    VerifyChannelNotes( 1, 0 );
    VerifyChannelNotes( 2, 0 );
    VerifyChannelNotes( 3, 0 );
}


/*******************************************************************/
/**
 * Turn on notes and check to see which channel the note was stolen from.
 * This is based on the example in the SPMIDI spec.
 */
static void TestStealing( void )
{
    int pitch = 48;

    printf( "Test Note Stealing =============\n" );

    SPMUtil_Reset(spmidiContext);
    KillTime( 50 );

    SPMIDI_Write( spmidiContext, spSysExMessage1, sizeof( spSysExMessage1 ) );
    SPMIDI_SetMaxVoices( spmidiContext, 10 );

    NOTE( 1 );
    NOTE( 1 );
    NOTE( 3 );
    NOTE( 0 );
    NOTE( 1 );
    NOTE( 0 );
    NOTE( 1 );
    NOTE( 1 );
    NOTE( 2 );

    /* This is the tenth and last note that can be assigned without stealing. */
    NOTE( 1 );

    VerifyChannelNotes( 0, 2 );
    VerifyChannelNotes( 1, 6 );
    VerifyChannelNotes( 2, 1 );
    VerifyChannelNotes( 3, 1 );

    /* This note is too many and should steal from 1 because it has excess notes. */
    NOTE( 2 );

    VerifyChannelNotes( 0, 2 );
    VerifyChannelNotes( 1, 5 ); /* Steal from 1 and add to 2 */
    VerifyChannelNotes( 2, 2 );
    VerifyChannelNotes( 3, 1 );

    /* This note is too many and should steal from 2 because it now has an excess note. */
    NOTE( 2 );

    VerifyChannelNotes( 0, 2 );
    VerifyChannelNotes( 1, 5 );
    VerifyChannelNotes( 2, 2 ); /* Steal from 2 and add to 2 */
    VerifyChannelNotes( 3, 1 );
    VerifyChannelNotes( 4, 0 );

    printf("This note should not sound because too few voices to enable channel 4.\n");
    NOTE( 4 );

    VerifyChannelNotes( 0, 2 );
    VerifyChannelNotes( 1, 5 );
    VerifyChannelNotes( 2, 2 );
    VerifyChannelNotes( 3, 1 );
    VerifyChannelNotes( 4, 0 );


    printf("SPMIDI_MUTE_UNSPECIFIED_CHANNELS = %d\n", SPMIDI_MUTE_UNSPECIFIED_CHANNELS );
    NOTE( 5 );
#if SPMIDI_MUTE_UNSPECIFIED_CHANNELS

    printf("This note is past end of MIPS record and should be muted.\n");

    VerifyChannelNotes( 0, 2 );
    VerifyChannelNotes( 1, 5 );
    VerifyChannelNotes( 2, 2 );
    VerifyChannelNotes( 3, 1 );
    VerifyChannelNotes( 4, 0 );
    VerifyChannelNotes( 5, 0 );
#else

    printf("This note is past end of MIPS record but will still make sound..\n");

    VerifyChannelNotes( 0, 2 );
    VerifyChannelNotes( 1, 4 ); /* Stolen from here because 2+5>6 so excess note. */
    VerifyChannelNotes( 2, 2 );
    VerifyChannelNotes( 3, 1 );
    VerifyChannelNotes( 4, 0 );
    VerifyChannelNotes( 5, 1 );
#endif

}


/*******************************************************************/
/**
 * Turn on notes and check to see which channel the note was stolen from.
 */
static void TestGroup( void )
{
    int pitch = 48;
    int i;

    printf( "Test Group =============\n" );

    SPMUtil_Reset(spmidiContext);
    KillTime( 50 );

    // MIP
    // 0, 2,
    // 1, 6,
    // 2, 6,
    // 3, 9,
    SPMIDI_Write( spmidiContext, spSysExMessage2, sizeof( spSysExMessage2 ) );
    SPMIDI_SetMaxVoices( spmidiContext, 9 );

    NOTELOOP( 3, 9 );
    VerifyChannelNotes( 0, 0 );
    VerifyChannelNotes( 1, 0 );
    VerifyChannelNotes( 2, 0 );
    VerifyChannelNotes( 3, 9 );

    NOTELOOP( 2, 9 );
    VerifyChannelNotes( 0, 0 );
    VerifyChannelNotes( 1, 0 );
    VerifyChannelNotes( 2, 6 );  /* only 6 here cuz that is max MIP */
    VerifyChannelNotes( 3, 3 );

    NOTELOOP( 1, 3 );
    VerifyChannelNotes( 0, 0 );
    VerifyChannelNotes( 1, 3 );
    VerifyChannelNotes( 2, 3 ); /* Take from here cuz cumulative is above 6. */
    VerifyChannelNotes( 3, 3 );

    NOTE( 3 );
    VerifyChannelNotes( 0, 0 );
    VerifyChannelNotes( 1, 3 );
    VerifyChannelNotes( 2, 3 );
    VerifyChannelNotes( 3, 3 ); /* cuz lowest priority */

    NOTE( 0 );
    VerifyChannelNotes( 0, 1 );
    VerifyChannelNotes( 1, 3 );
    VerifyChannelNotes( 2, 2 ); /* Take from here cuz cumulative is above 6. */
    VerifyChannelNotes( 3, 3 );
}

/*******************************************************************/
/**
 * Run all tests verifying that notes are correctly sounded and muted
 * @return 0 if all tests succeed, non-0 otherwise.
 */
int main(void);
int main(void)
{
    int err;
    int i;

    QA_Init( " qa_spmidi" );

    printf("Test Scaleable Polyphony.\n");

    err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( err < 0 )
        goto error;

    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        SPMUtil_ProgramChange( spmidiContext, i, PROGRAM_ORGAN );
    }

    TestChannelMuting();
    TestStealing();
    TestReset();
    TestGroup();

    SPMIDI_DeleteContext(spmidiContext);

error:
    return QA_Term(72);
}
