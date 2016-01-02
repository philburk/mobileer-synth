/* $Id: play_note.c,v 1.31 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Extremely simple test that just plays a few notes.
 * This is mostly for stepping through synth code.
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

#if USE_TEST_INSTRUMENTS

//#define PROGRAM             (0x00) /* Test Sine - NonGM. */
//#define PROGRAM             (0x02) /* Test Sawtooth - NonGM. */
//#define PROGRAM             (0x03) /* Test Square - NonGM. */
//#define PROGRAM             (0x04) /* Test SquareHarsh - NonGM. */
//#define PROGRAM             (0x05) /* Test WaveTable. */

#else

//#define PROGRAM             (0x00) /* Acoustic Grand. */
#define PROGRAM             (0x07) /* Clav. */
//#define PROGRAM             (0x08) /* Celesta. */
//#define PROGRAM             (0x09) /* Glockenspiel. */
//#define PROGRAM             (0x0A) /* Music Box. */
//#define PROGRAM             (0x12) /* RockOrgan. */
//#define PROGRAM             (0x13) /* ChurchOrgan. */
//#define PROGRAM             (35) /* Fretless Bass. */
//#define PROGRAM             (40) /* Violin. */
//#define PROGRAM             (0x15) /* Accordian. */
//#define PROGRAM             (0x16) /* Harmonica. */
//#define PROGRAM             (0x18) /* NylonGuitar. */
//#define PROGRAM             (0x37) /* Orchestra Hit */
//#define PROGRAM             (0x38) /* Trumpet */
//#define PROGRAM             (0x41) /* Alto Sax */
//#define PROGRAM             (0x47) /* Clarinet */
//#define PROGRAM             (0x49) /* Flute */
//#define PROGRAM             (0x50) /* Lead Square */
//#define PROGRAM             (60) /* French Horn */
//#define PROGRAM             (0x0B) /* Vibraphone. */
//#define PROGRAM             (0x0E) /* Tubular Bells. */
//#define PROGRAM             (0x46) /* Bassoon */
//#define PROGRAM             (0x5D) /* Pad 6 Metallic */
//#define PROGRAM             (0x62) /* FX 3 Crystal */
//#define PROGRAM                (114)  /* Steel Drums */
#endif

#define SAMPLE_RATE         (44100)
#define LOWEST_PITCH        (60)
#define PITCH_INCR          (3)
#define HIGHEST_PITCH       (73)
#define CHANNEL             (2)
#define DUR_ON              (200)
#define DUR_OFF             (200)

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
    int i;
char *fileName = NULL;
//char *fileName = "play_note_piano2.wav";

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    /* Turn off compressor so we hear unmodified instrument sound. */
    err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( err < 0 )
        goto error;

    SPMIDI_SetMasterVolume( spmidiContext, SPMIDI_DEFAULT_MASTER_VOLUME * 8 );

#if USE_TEST_INSTRUMENTS

    SPMIDI_Write( spmidiContext, SysExGMOff, sizeof( SysExGMOff ) );
    SPMUtil_BankSelect( spmidiContext, CHANNEL, SPMIDI_TEST_BANK );
    printf("Using test instrument #%d\n", PROGRAM );
#else

    printf("Program #%3d, %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM ) );
#endif

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );
    SPMUtil_ControlChange( spmidiContext, CHANNEL, MIDI_CONTROL_BANK, 0 );

    /* Note On */
    for( i = LOWEST_PITCH; i<=HIGHEST_PITCH; i+=PITCH_INCR )
    {
        printf("Pitch = %d\n", i );
        SPMUtil_NoteOn( spmidiContext, CHANNEL, i, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, DUR_ON );

        /* Note Off */
        SPMUtil_NoteOff( spmidiContext, CHANNEL, i, 0 );
        SPMUtil_PlayMilliseconds( spmidiContext, DUR_OFF );
    }

    SPMUtil_PlayMilliseconds( spmidiContext, 1000 );

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
