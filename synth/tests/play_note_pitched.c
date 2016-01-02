/* $Id: play_note_pitched.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Extremely simple test that plays a note with pitch bend.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define USE_TEST_INSTRUMENTS       (0)

// #define PROGRAM             (0x00) /* Acoustic Grand. */
// #define PROGRAM             (0x07) /* Celesta. */
// #define PROGRAM             (0x08) /* Celesta. */
// #define PROGRAM             (0x09) /* Glockenspiel. */
// #define PROGRAM             (0x0A) /* Music Box. */
// #define PROGRAM             (0x12) /* RockOrgan. */
// #define PROGRAM             (35) /* Fretless Bass. */
// #define PROGRAM             (40) /* Violin. */
// #define PROGRAM             (0x15) /* Accordian. */
// #define PROGRAM             (0x16) /* Harmonica. */
#define PROGRAM             (0x18) /* Nylon Guitar. */
// #define PROGRAM             (0x30) /* String Ensemble */
// #define PROGRAM             (0x37) /* Orchestra Hit */
// #define PROGRAM             (0x38) /* Trumpet */
//#define PROGRAM             (0x41) /* Alto Sax */
// #define PROGRAM             (0x47) /* Clarinet */
// #define PROGRAM             (0x49) /* Flute */
// #define PROGRAM             (0x0B) /* Vibraphone. */
// #define PROGRAM             (0x0E) /* Tubular Bells. */
// #define PROGRAM             (0x46) /* Bassoon */

// #define PROGRAM             (0x00) /* Test Sine - NonGM. */
// #define PROGRAM             (0x02) /* Test Sawtooth - NonGM. */
// #define PROGRAM             (0x03) /* Test Square - NonGM. */
// #define PROGRAM             (0x04) /* Test Sample - NonGM. */

#define SAMPLE_RATE         (16000)
#define PITCH               (59)
#define CHANNEL             (0)
#define DURATION            (30)

static const unsigned char SysExGMOff[] =
    {
        MIDI_SOX, 0x7E, 0x7F, 0x09, 0x02, MIDI_EOX
    };

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;
    int bend;
    //char *fileName = NULL;
    char *fileName = "D:\\temp\\nylon_bent.wav";
    printf("SPMIDI Test: play_note on program %d = %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM )  );

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    /* Turn off compressor so we hear unmodified instrument sound. */
    err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( err < 0 )
        goto error;

    SPMIDI_SetMasterVolume( spmidiContext, SPMIDI_DEFAULT_MASTER_VOLUME * 8 );

#if USE_TEST_INSTRUMENTS

    SPMIDI_Write( SysExGMOff, sizeof( SysExGMOff ) );
    printf("using test instrument #%d\n", PROGRAM );
#else

    printf("Program #%3d, %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM ) );
#endif

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

    /* Set range to one semitone. */
    SPMUtil_SetBendRange( spmidiContext, CHANNEL, 1, 0);
    
    SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH, 64 );
    SPMUtil_PlayBuffers( spmidiContext, DURATION );
    SPMUtil_NoteOff( spmidiContext, CHANNEL, PITCH, 0 );
    SPMUtil_PlayBuffers( spmidiContext, DURATION );

    /* Set bend to half a semitone. */
    bend = MIDI_BEND_NONE + (MIDI_BEND_MAX - MIDI_BEND_NONE)/2;
    SPMUtil_PitchBend( spmidiContext, CHANNEL, bend );
    
    SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH, 64 );
    SPMUtil_PlayBuffers( spmidiContext, DURATION );
    SPMUtil_NoteOff( spmidiContext, CHANNEL, PITCH, 0 );
    SPMUtil_PlayBuffers( spmidiContext, DURATION );

    /* Set bend to zero and play a semitone higher. */
    bend = MIDI_BEND_NONE;
    SPMUtil_PitchBend( spmidiContext, CHANNEL, bend );
    
    SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH+1, 64 );
    SPMUtil_PlayBuffers( spmidiContext, DURATION );
    SPMUtil_NoteOff( spmidiContext, CHANNEL, PITCH+1, 0 );
    SPMUtil_PlayBuffers( spmidiContext, DURATION );

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
