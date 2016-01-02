/* $Id: qa_pitch_excl.c,v 1.7 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_pitch_excl.c
 * @brief Test mutual exclusion of pitches on a channel.
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
#define CHANNEL             (2)

static short samples[ SPMIDI_MAX_FRAMES_PER_BUFFER ];

/*******************************************************************/
static void VerifyChannelNotes( SPMIDI_Context *spmidiContext, int channelIndex, int expectedNoteCount )
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
static void KillTime( SPMIDI_Context *spmidiContext, int numBuffers )
{
    int i;
    for( i=0; i<numBuffers; i++ )
    {
        SPMIDI_ReadFrames( spmidiContext, samples, SPMIDI_MAX_FRAMES_PER_BUFFER, 1, 16 );
    }
}

/*******************************************************************/
/**
 * Turn notes on and off, verify that the expected number of notes are
 * sounding at given times.
 * @return 0 if all tests successful, non-0 if not
 */
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;

    printf("SPMIDI Test. SR = %d\n", SAMPLE_RATE);

    QA_Init( "qa_pitch_excl" );

    err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( err < 0 )
        goto error;

    VerifyChannelNotes( spmidiContext, CHANNEL, 0 );

#define TEST_PITCH( pitch, expected ) \
    SPMUtil_NoteOn( spmidiContext, CHANNEL, (pitch), 64 ); \
    KillTime( spmidiContext, 1 ); /* Let stifled notes die off. */ \
    VerifyChannelNotes( spmidiContext, CHANNEL, (expected) );

    TEST_PITCH( 40, 1 );
    TEST_PITCH( 41, 2 );
    TEST_PITCH( 42, 3 );

    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + CHANNEL, MIDI_CONTROL_ALLNOTESOFF, 0 );
    KillTime( spmidiContext, 1000 ); /* Let all notes die off. */
    VerifyChannelNotes( spmidiContext, CHANNEL, 0 );

    TEST_PITCH( 40, 1 );
    TEST_PITCH( 41, 2 );
    TEST_PITCH( 40, 2 );
    TEST_PITCH( 40, 2 );

    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + CHANNEL, MIDI_CONTROL_ALLNOTESOFF, 0 );
    KillTime( spmidiContext, 1000 ); /* Let notes die off. */
    VerifyChannelNotes( spmidiContext, CHANNEL, 0 );


#define TEST_PITCH_FAST( pitch, expected ) \
    SPMUtil_NoteOn( spmidiContext, CHANNEL, (pitch), 64 ); \
    VerifyChannelNotes( spmidiContext, CHANNEL, (expected) );

    /* Test quick succession of notes so that stifled
     * notes have not had time to die down. */
    TEST_PITCH_FAST( 40, 1 );
    TEST_PITCH_FAST( 41, 2 );
    TEST_PITCH_FAST( 40, 3 );
    TEST_PITCH_FAST( 40, 4 );
    KillTime( spmidiContext, 1 ); /* Let stifled notes die off. */
    TEST_PITCH_FAST( 42, 3 );
    TEST_PITCH_FAST( 42, 4 );

    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + CHANNEL, MIDI_CONTROL_ALLNOTESOFF, 0 );
    KillTime( spmidiContext, 1000 ); /* Let notes die off. */
    VerifyChannelNotes( spmidiContext, CHANNEL, 0 );

    SPMIDI_DeleteContext(spmidiContext);

error:
    return QA_Term( 17 );
}
