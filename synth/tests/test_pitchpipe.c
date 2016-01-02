/* $Id: test_pitchpipe.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test playing a steady pure note for use as a pitch pipe.
 *.
 * Copyright 2002-5 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

/* You can use an internal test tone for a pure sine wave with no vibrato.
 * But the test tone bank may change in the future. So do not use this
 * technique in a MIDI file. Use it only in code that can be updated.
 */
#define USE_TEST_INSTRUMENTS       (1)

#if USE_TEST_INSTRUMENTS
    #define PROGRAM             (0x00) /* Test Sine - NonGM. */
#else
    #define PROGRAM             (0x49) /* Flute */
#endif

#define SAMPLE_RATE         (22050)
#define PITCH               (60)
#define CHANNEL             (0)

#define DUR_ON              (4000)
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
    int pitch = PITCH;
    char *fileName = NULL;

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    /* Turn off compressor so we hear unmodified instrument sound. */
    err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( err < 0 )
        goto error;

    /* Set MasterVolume high because compressor is off and will not boost gain automatically. */
    SPMIDI_SetMasterVolume( spmidiContext, SPMIDI_DEFAULT_MASTER_VOLUME * 8 );

#if USE_TEST_INSTRUMENTS
    /* Send a Sysex to turn off General MIDI. */
    SPMIDI_Write( spmidiContext, SysExGMOff, sizeof( SysExGMOff ) );
    /* Select the test instrument bank. */
    SPMUtil_BankSelect( spmidiContext, CHANNEL, SPMIDI_TEST_BANK );
    printf("Using test instrument #%d\n", PROGRAM );
#else

    printf("Program #%3d, %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM ) );
#endif

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

            
    SPMUtil_ControlChange( spmidiContext, CHANNEL, MIDI_CONTROL_MODULATION, 0 );
    
    printf("Pitch = %d\n", pitch );
    SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, 64 );
    SPMUtil_PlayMilliseconds( spmidiContext, DUR_ON );

    /* Note Off */
    SPMUtil_NoteOff( spmidiContext, CHANNEL, pitch, 0 );
    SPMUtil_PlayMilliseconds( spmidiContext, DUR_OFF );

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
