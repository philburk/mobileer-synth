/* $Id: test_waveload.c,v 1.6 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Test loading of wavetable and waveset data.
 * Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_editor.h"


#define PROGRAM             (0x00) /* Acoustic Grand. */

#define SAMPLE_RATE         (22050)
#define PITCH               (60)
#define CHANNEL             (0)
#define DURATION            (600)

/* Encode data using BigEndian format. */
#define EncodeLong(n)  ((n>>24)&0x00FF), ((n>>16)&0x00FF), ((n>>8)&0x00FF), (n&0x00FF)
#define EncodeShort(n)  ((n>>8)&0x00FF), (n&0x00FF)

/* Match internal conversion of MIDI pitch to PitchOctave. */
#define PitchToOctave(pitch)  ((pitch/12) + (8<<16))

/* Define basic wavetable with recognizable shape. */
unsigned char tableStream1[] = {
                                   SPMIDI_BEGIN_STREAM,
                                   SPMIDI_WAVETABLE_STREAM_ID,

                                   EncodeLong( 60 << 16 ), /* 4 FXP16 midiPitch; */
                                   EncodeLong( -75128 ), /* 4   PitchOctave     sampleRateOffset; */
                                   EncodeLong( 4 ), /* 4    int             loopBegin; */
                                   EncodeLong( 8 ), /* 4    int             loopEnd; */
                                   EncodeLong( 8 ), /* 4    int         numSamples */
                                   SPMIDI_WAVE_TYPE_S16,
                                   0, /* velocity */

                                   EncodeShort( 0x0100 ),
                                   EncodeShort( 0x1000 ),
                                   EncodeShort( 0x3000 ),
                                   EncodeShort( 0x7000 ),
                                   EncodeShort( 0x2000 ),
                                   EncodeShort( 0xFEDC ),
                                   EncodeShort( 0xCBA9 ),
                                   EncodeShort( 0xD000 ),


                                   SPMIDI_END_STREAM
                               };

unsigned char tableStream2[] = {
                                   SPMIDI_BEGIN_STREAM,
                                   SPMIDI_WAVETABLE_STREAM_ID,

                                   EncodeLong( 48 << 16 ), /* 4 FXP16 midiPitch; */
                                   EncodeLong( -75128 ), /* 4   PitchOctave     sampleRateOffset; */
                                   EncodeLong( 2 ), /* 4    int             loopBegin; */
                                   EncodeLong( 6 ), /* 4    int             loopEnd; */
                                   EncodeLong( 8 ), /* 4    int         numSamples */
                                   SPMIDI_WAVE_TYPE_S16,
                                   0, /* velocity */

                                   EncodeShort( 0x2000 ), /* Sawtooth wave. */
                                   EncodeShort( 0x4000 ),
                                   EncodeShort( 0x6000 ),
                                   EncodeShort( 0x8000 ),
                                   EncodeShort( 0xA000 ),
                                   EncodeShort( 0xC000 ),
                                   EncodeShort( 0xE000 ),
                                   EncodeShort( 0x0000 ),

                                   SPMIDI_END_STREAM
                               };

unsigned char waveSetStream1[] = {
                                     SPMIDI_BEGIN_STREAM,
                                     SPMIDI_WAVESET_STREAM_ID,

                                     2, /* two tables */
                                     EncodeLong(2), /* anticipated tokens */
                                     EncodeLong(3),

                                     SPMIDI_END_STREAM
                                 };

unsigned char waveSetStreamBad[] = {
                                       SPMIDI_BEGIN_STREAM,
                                       SPMIDI_WAVESET_STREAM_ID,

                                       2, /* two tables */
                                       EncodeLong(2), /* anticipated tokens */
                                       EncodeLong(666), /* Bad token. */

                                       SPMIDI_END_STREAM
                                   };


/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    spmSInt32 err;
    spmSInt32 tableToken1;
    spmSInt32 tableToken2;
    spmSInt32 waveSetToken1;
    spmSInt32 waveSetTokenBad;

    char *fileName = NULL;
    //char *fileName = "test_waveload.wav";
    printf("SPMIDI Test: play_note on program %d = %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM )  );

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    /* Create two tables. */
    tableToken1 = SPMIDI_LoadWaveTable( spmidiContext, tableStream1, sizeof(tableStream1) );
    if( tableToken1 < 0 )
    {
        err = tableToken1;
        goto error;
    }
    tableToken2 = SPMIDI_LoadWaveTable( spmidiContext, tableStream2, sizeof(tableStream2) );
    if( tableToken2 < 0 )
    {
        err = tableToken2;
        goto error;
    }

    /* Load a waveSet containing two valid tables. */
    waveSetToken1 = SPMIDI_LoadWaveSet( NULL, waveSetStream1, sizeof(waveSetStream1) );
    if( waveSetToken1 < 0 )
    {
        err = waveSetToken1;
        goto error;
    }

    /* Load a bad waveSet to test error recovery. */
    waveSetTokenBad = SPMIDI_LoadWaveSet( NULL, waveSetStreamBad, sizeof(waveSetStreamBad) );
    if( waveSetTokenBad != SPMIDI_Error_BadToken )
    {
        printf("Expected SPMIDI_Error_BadToken!\n");
        goto error;
    }

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

    /* Note On */
    SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH, 64 );
    SPMUtil_PlayMilliseconds( spmidiContext, DURATION );

    /* Note Off */
    SPMUtil_NoteOff( spmidiContext, CHANNEL, PITCH, 0 );
    SPMUtil_PlayMilliseconds( spmidiContext, DURATION );


    /* Unload table in use by waveSetToken1 */
    err = SPMIDI_UnloadWaveTable( spmidiContext, tableToken2 );
    if( err < 0 )
    {
        goto error;
    }

    /* Try to unload it a second time, which should fail. */
    err = SPMIDI_UnloadWaveTable( spmidiContext, tableToken2 );
    if( err != SPMIDI_Error_BadToken )
    {
        printf("Expected SPMIDI_Error_BadToken!\n");
        goto error;
    }

    err = SPMIDI_UnloadWaveSet( spmidiContext, waveSetToken1 );
    if( err < 0 )
    {
        goto error;
    }

    /* Try to unload it a second time, which should fail. */
    err = SPMIDI_UnloadWaveTable( spmidiContext, waveSetToken1 );
    if( err != SPMIDI_Error_BadToken )
    {
        printf("Expected SPMIDI_Error_BadToken!\n");
        goto error;
    }

    err = SPMIDI_UnloadWaveSet( spmidiContext, waveSetTokenBad );
    if( err != SPMIDI_Error_BadToken )
    {
        printf("Expected SPMIDI_Error_BadToken!\n");
        goto error;
    }


    SPMUtil_Stop(spmidiContext);

    printf("Test finished OK.\n");
    return err;
error:
    printf("Error in test = %d = %s\n", err, SPMUtil_GetErrorText( err ) );
    return err;
}
