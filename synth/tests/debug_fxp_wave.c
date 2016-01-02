/* $Id: debug_fxp_wave.c,v 1.5 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play a note using a very simple waveform to
 * debug fixed point wave oscillator.
 * Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL, Phil Burk
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define USE_TEST_INSTRUMENTS       (1)

#define PROGRAM             (0x04) /* SimpleWave - NonGM. */

#define SAMPLE_RATE         (22050)
#define PITCH               (60)
#define CHANNEL             (0)
#define DURATION            (800)

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
    //char *fileName = NULL;
    char *fileName = "debug_fxp_wave.wav";
    printf("SPMIDI Test: debug_fxp_wave on program %d = %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM )  );

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
    printf("using test instrument #%d\n", PROGRAM );
#else

    printf("Program #%3d, %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM ) );
#endif

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

    SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH, 64 );
    /* Just play one buffer so we can examine printouts. */
    SPMUtil_PlayMilliseconds(spmidiContext, 1 );

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
