/* $Id: qa_drum_excl.c,v 1.7 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_drum_excl.c
 * @brief Test mutual exclusion of related drum sound in General MIDI set.
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 * For example "OpenHiHat", "ClosedHiHat" and "PedalHiHat" are mutually exclusive because
 * they are 3 sounds from the same physical instrument.
 * There are 5 such exclusion groups for related sounds from the same instrument
 * such as mute and open cuica.
 *
 */

#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/qa/qa_tools.h"

#define SAMPLE_RATE         (44100)

#define BASS_DRUM           (36)
#define COW_BELL            (56)

#define CLOSED_HIHAT_EX_1   (42)
#define PEDAL_HIHAT_EX_1    (44)
#define OPEN_HIHAT_EX_1     (46)

#define SHORT_WHISTLE_EX_2  (71)
#define LONG_WHISTLE_EX_2   (72)

#define SHORT_GUIRO_EX_3    (73)
#define LONG_GUIRO_EX_3     (74)

#define MUTE_CUICA_EX_4     (78)
#define OPEN_CUICA_EX_4     (79)

#define MUTE_TRIANGLE_EX_5  (80)
#define OPEN_TRIANGLE_EX_5  (81)

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
 * Test drums
 * @return Negative error code or zero.
 */
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;

    QA_Init( "qa_drum_excl" );

    printf("SPMIDI Test AllNotesOff. SR = %d\n", SAMPLE_RATE);

    err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( err < 0 )
    {
        goto error;
    }

    VerifyChannelNotes( spmidiContext, 9, 0 );

#define TEST_DRUM( pitch, expected ) \
    SPMUtil_NoteOn( spmidiContext, 9, (pitch), 64 ); \
    KillTime( spmidiContext, 1 ); /* Let stifled drums die off. */ \
    VerifyChannelNotes( spmidiContext, 9, (expected) );

    TEST_DRUM( BASS_DRUM, 1 );
    TEST_DRUM( CLOSED_HIHAT_EX_1, 2 );
    TEST_DRUM( PEDAL_HIHAT_EX_1, 2 );   /* Replaces CLOSED_HIHAT_EX_1 */
    TEST_DRUM( OPEN_HIHAT_EX_1, 2 );  /* Replaces PEDAL_HIHAT_EX_1 */
    TEST_DRUM( COW_BELL, 3 );

    KillTime( spmidiContext, 1000 ); /* Let drums die off. */
    VerifyChannelNotes( spmidiContext, 9, 0 );

    TEST_DRUM( BASS_DRUM, 1 );
    TEST_DRUM( COW_BELL, 2 );
    TEST_DRUM( CLOSED_HIHAT_EX_1, 3 );
    TEST_DRUM( SHORT_WHISTLE_EX_2, 4 );
    TEST_DRUM( PEDAL_HIHAT_EX_1, 4 );
    TEST_DRUM( LONG_WHISTLE_EX_2, 4 );

    KillTime( spmidiContext, 1000 ); /* Let drums die off. */
    VerifyChannelNotes( spmidiContext, 9, 0 );

    TEST_DRUM( BASS_DRUM, 1 );
    TEST_DRUM( SHORT_GUIRO_EX_3, 2 );
    TEST_DRUM( LONG_GUIRO_EX_3, 2 );
    TEST_DRUM( LONG_GUIRO_EX_3, 2 );
    TEST_DRUM( COW_BELL, 3 );
    TEST_DRUM( MUTE_CUICA_EX_4, 4 );
    TEST_DRUM( OPEN_CUICA_EX_4, 4 );

    KillTime( spmidiContext, 1000 ); /* Let drums die off. */
    VerifyChannelNotes( spmidiContext, 9, 0 );

    TEST_DRUM( BASS_DRUM, 1 );
    TEST_DRUM( MUTE_CUICA_EX_4, 2 );
    TEST_DRUM( OPEN_CUICA_EX_4, 2 );
    TEST_DRUM( COW_BELL, 3 );
    TEST_DRUM( MUTE_TRIANGLE_EX_5, 4 );
    TEST_DRUM( OPEN_TRIANGLE_EX_5, 4 );

    SPMIDI_DeleteContext(spmidiContext);

    return QA_Term( 28 );

error:
    printf("ERROR: test did not run, err = %d = %s\n", err,
               SPMUtil_GetErrorText( err ) );
    return QA_Term( 28 );
}
