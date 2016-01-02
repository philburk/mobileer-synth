/* $Id: qa_note_on_off.c,v 1.7 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_note_on_off.c
 * @brief Test AllNotesOff and Sustain controller.
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
 * Turn notes on and off, check that they stop appropriately
 * @return 0 if all tests succeed, non-0 if not
 */
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;

    QA_Init( "qa_note_on_off" );

    printf("SPMIDI Test Note allocation. SR = %d\n", SAMPLE_RATE);

    err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( err < 0 )
        goto error;

    SPMUtil_ProgramChange( spmidiContext, 0, PROGRAM_ORGAN );
    SPMUtil_ProgramChange( spmidiContext, 1, PROGRAM_ORGAN );

    VerifyChannelNotes( spmidiContext, 0, 0 );
    VerifyChannelNotes( spmidiContext, 1, 0 );

    SPMUtil_NoteOn( spmidiContext, 1, 50, 64 );
    KillTime( spmidiContext, 1 );
    VerifyChannelNotes( spmidiContext, 0, 0 );
    VerifyChannelNotes( spmidiContext, 1, 1 );

    SPMUtil_NoteOn( spmidiContext, 1, 52, 64 );
    KillTime( spmidiContext, 1 );
    VerifyChannelNotes( spmidiContext, 0, 0 );
    VerifyChannelNotes( spmidiContext, 1, 2 );

    SPMUtil_NoteOn( spmidiContext, 0, 54, 64 );
    KillTime( spmidiContext, 1 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 2 );

    SPMUtil_NoteOff( spmidiContext, 1, 52, 64 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 2 );
    /* Give notes time to release and be deactivated by synth engine. */
    KillTime( spmidiContext, 50 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 1 );

    SPMUtil_NoteOn( spmidiContext, 1, 56, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 57, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 58, 64 );
    KillTime( spmidiContext, 1 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 4 );

    /* Turn off every note on channel 1 using AllNotesOff message. */
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_ALLNOTESOFF, 0 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 4 );
    KillTime( spmidiContext, 50 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 0 );

    SPMUtil_NoteOn( spmidiContext, 1, 56, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 57, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 58, 64 );
    KillTime( spmidiContext, 1 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 3 );

    /* Sustain OFF because value < 64 */
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_SUSTAIN, 63 );
    SPMUtil_NoteOff( spmidiContext, 1, 56, 64 );
    SPMUtil_NoteOff( spmidiContext, 1, 58, 64 );
    KillTime( spmidiContext, 50 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 1 );

    SPMUtil_NoteOn( spmidiContext, 1, 56, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 57, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 58, 64 );
    KillTime( spmidiContext, 1 );

    /* Sustain ON because value >= 64 */
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_SUSTAIN, 64 );
    SPMUtil_NoteOff( spmidiContext, 1, 56, 64 );
    SPMUtil_NoteOff( spmidiContext, 1, 58, 64 );
    KillTime( spmidiContext, 50 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 3 );

    /* Sustain OFF so all pending notes should turn off. */
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_SUSTAIN, 0 );
    KillTime( spmidiContext, 50 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 1 );

    SPMUtil_NoteOn( spmidiContext, 1, 56, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 57, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 58, 64 );
    KillTime( spmidiContext, 1 );

    /* AllNotesOff commands should also be held until sustain is off. */
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_SUSTAIN, 127 );
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_ALLNOTESOFF, 0 );
    KillTime( spmidiContext, 50 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 3 );

    /* Sustain OFF so all pending notes should turn off. */
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_SUSTAIN, 0 );
    KillTime( spmidiContext, 50 );
    VerifyChannelNotes( spmidiContext, 0, 1 );
    VerifyChannelNotes( spmidiContext, 1, 0 );

    printf("Test AllSoundOff\n");
    SPMUtil_NoteOn( spmidiContext, 0, 55, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 45, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 46, 64 );
    SPMUtil_NoteOn( spmidiContext, 1, 47, 64 );
    KillTime( spmidiContext, 1 );

    printf("AllSoundOff command tells notes to stifle themselves then turn off.\n");
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 1, MIDI_CONTROL_ALLSOUNDOFF, 0 );
    VerifyChannelNotes( spmidiContext, 0, 2 );
    VerifyChannelNotes( spmidiContext, 1, 3 );
    KillTime( spmidiContext, 1 );
    /* Stifled notes should be off by now. */
    VerifyChannelNotes( spmidiContext, 1, 0 );

    /****************************************/
    printf("Test to see if isSustaining flag is cleared properly when note is turned on.\n");
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 2, MIDI_CONTROL_SUSTAIN, 0 );
    KillTime( spmidiContext, 1 );
    SPMUtil_ProgramChange( spmidiContext, 2, PROGRAM_ORGAN );
    printf("Make sure note dies away in 100 ticks.\n");
    SPMUtil_NoteOn( spmidiContext, 2, 52, 96 );
    KillTime( spmidiContext, 2 );
    SPMUtil_NoteOff( spmidiContext, 2, 52, 96 );
    KillTime( spmidiContext, 100 );
    VerifyChannelNotes( spmidiContext, 2, 0 );

    printf("Start note that will get stolen.\n");
    SPMUtil_NoteOn( spmidiContext, 2, 52, 96 );
    KillTime( spmidiContext, 2 );
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 2, MIDI_CONTROL_SUSTAIN, 100 );
    KillTime( spmidiContext, 5 );
    /* Note marked as sustaining. */
    SPMUtil_NoteOff( spmidiContext, 2, 52, 96 );
    KillTime( spmidiContext, 1 );
    VerifyChannelNotes( spmidiContext, 2, 1 );

    printf("Retrigger note.\n");
    SPMUtil_NoteOn( spmidiContext, 2, 52, 96 );
    KillTime( spmidiContext, 1 );
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + 2, MIDI_CONTROL_SUSTAIN, 0 );
    KillTime( spmidiContext, 1 );
    VerifyChannelNotes( spmidiContext, 2, 1 );

    KillTime( spmidiContext, 100 );
    VerifyChannelNotes( spmidiContext, 2, 1 );

    SPMIDI_DeleteContext(spmidiContext);

error:
    return QA_Term( 37 );
}
