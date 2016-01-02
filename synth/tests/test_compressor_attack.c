/* $Id: test_compressor_attack.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test attack and pre-delay of compressor.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

//#define PROGRAM             (0x34) /* Choir Ahh */
//#define PROGRAM             (0x07) /* Clav. */
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
#define PROGRAM             (0x51) /* Lead Sawtooth */
//#define PROGRAM             (60) /* French Horn */
//#define PROGRAM             (0x0B) /* Vibraphone. */
//#define PROGRAM             (0x0E) /* Tubular Bells. */
//#define PROGRAM             (0x46) /* Bassoon */
//#define PROGRAM             (0x5D) /* Pad 6 Metallic */
//#define PROGRAM             (0x62) /* FX 3 Crystal */
//#define PROGRAM                (114)  /* Steel Drums */


#define SAMPLE_RATE         (44100)
#define NUM_CHANNELS        (2)
#define VELOCITY            (113)
#define LOWEST_PITCH        (65)
#define PITCH_INCR          (3)
#define NUM_NOTES_IN_CHORD  (5)
#define CHANNEL             (2)
#define DUR_ON              (500)
#define DUR_OFF             (1000)

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
    int j;
//char *fileName = NULL;
char *fileName = "test_compressor.wav";

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, fileName, NUM_CHANNELS );
    if( err < 0 )
        goto error;

    /* Make sure compressor is on, */
    err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 1 );
    if( err < 0 )
        goto error;


    printf("Program #%3d, %s\n", PROGRAM, MIDI_GetProgramName( PROGRAM ) );

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

    err = SPMUtil_PlayMilliseconds( spmidiContext, 500 );
    if( err < 0 )
        goto error;

    for( j = 0; j<3; j++ )
    {

        printf("Start chord -----------\n" );
        /* Notes On */
        for( i=0; i<NUM_NOTES_IN_CHORD; i++ )
        {
            int pitch = LOWEST_PITCH  + (i * PITCH_INCR);
            printf("Pitch = %d\n", pitch );
            SPMUtil_NoteOn( spmidiContext, CHANNEL, pitch, VELOCITY );
        }
        SPMUtil_PlayMilliseconds( spmidiContext, DUR_ON );

        /* Notes Off */
        for( i=0; i<NUM_NOTES_IN_CHORD; i++ )
        {
            int pitch = LOWEST_PITCH  + (i * PITCH_INCR);
            SPMUtil_NoteOff( spmidiContext, CHANNEL, pitch, 0 );

        }
        
        err = SPMUtil_PlayMilliseconds( spmidiContext, DUR_OFF );
        if( err < 0 )
            goto error;
    }

    SPMUtil_PlayMilliseconds( spmidiContext, 1000 );

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    printf("Error in test = %d = %s\n", err, SPMUtil_GetErrorText(err));
    return err;
}
