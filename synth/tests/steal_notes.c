/* $Id: steal_notes.c,v 1.8 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test note stealing.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "pablio.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define SAMPLE_RATE         (44100)
#define DURATION            (80)

//#define PROGRAM             (0x00) /* Acoustic Grand. */
// #define PROGRAM             (0x07) /* Clav. */
// #define PROGRAM             (0x08) /* Celesta. */
// #define PROGRAM             (0x09) /* Glockenspiel. */
// #define PROGRAM             (0x0A) /* Music Box. */
// #define PROGRAM             (0x12) /* RockOrgan. */
// #define PROGRAM             (35) /* Fretless Bass. */
// #define PROGRAM             (40) /* Violin. */
// #define PROGRAM             (0x15) /* Accordian. */
// #define PROGRAM             (0x16) /* Harmonica. */
// #define PROGRAM             (0x37) /* Orchestra Hit */
// #define PROGRAM             (0x38) /* Trumpet */
// #define PROGRAM             (0x41) /* Alto Sax */
// #define PROGRAM             (0x47) /* Clarinet */
#define PROGRAM             (0x49) /* Flute */
// #define PROGRAM             (0x50) /* Lead Square */
// #define PROGRAM             (0x0B) /* Vibraphone. */
// #define PROGRAM             (0x0E) /* Tubular Bells. */
// #define PROGRAM             (0x46) /* Bassoon */
// #define PROGRAM             (0x5D) /* Pad 6 Metallic */
// #define PROGRAM             (0x62) /* FX 3 Crystal */

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;
    int i;
//  char *outputFileName = "steal_cross_fade.wav";
    char *outputFileName = NULL;

    printf("Test Note Stealing: SR = %d\n", SAMPLE_RATE);

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, outputFileName, SPMUTIL_OUTPUT_STEREO );
    if( err < 0 )
        goto error;

    SPMIDI_SetMaxVoices( spmidiContext, 1 );

    SPMUtil_ProgramChange( spmidiContext, 0, PROGRAM );
    for( i=0; i<40; i++ )
    {
        int duration;
        int pitch = 50 + (i&15);

        /* Note On */
        SPMUtil_NoteOn( spmidiContext, 0, pitch, 64 );

        duration = ((i&1)==0) ? i : DURATION + i;
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION + i );
    }

    SPMUtil_PlayMilliseconds( spmidiContext, 200 );

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
error:
    return err;
}
