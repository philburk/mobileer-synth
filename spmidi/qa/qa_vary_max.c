/* $Id: qa_vary_max.c,v 1.6 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_vary_max.c
 * @brief Test dynamic voice allocation.
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 * Try to max out voice allocator by playing too many notes rapidly.
 * Compare actual notes active with maximum.
 *
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/qa/qa_tools.h"

#define SAMPLE_RATE         (44100)

#define CHANNEL             (3)
#define PROGRAM             (0x58)
#define MAX_ON              (16)

#define PITCH(n)   (50 + (2*(n)))


void PlayScaleMax( SPMIDI_Context *spmidiContext, int maxNotes )
{
    int numNotes;
    int i;

    SPMIDI_SetMaxVoices( spmidiContext, maxNotes );

    /* Let voices decay and turn themselves off. */
    SPMUtil_PlayMilliseconds( spmidiContext, 1000 );

    for( i=0; i<SPMIDI_MAX_VOICES; i++ )
    {
        SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH(i) , 64 );
        SPMUtil_PlayBuffers( spmidiContext, 5 );
    }

    numNotes = SPMIDI_GetActiveNoteCount(spmidiContext);
    PRTMSG("maxNotes = ");
    PRTNUMD( maxNotes );
    PRTMSG(", numNotes = ");
    PRTNUMD( numNotes );
    PRTMSG("\n");

    if( numNotes != maxNotes )
    {
        PRTMSG("ERROR: numNotes != maxNotes !!!\n");
        QA_CountError();
    }
    else
    {
        QA_CountSuccess();
    }

    for( i=0; i<SPMIDI_MAX_VOICES; i++ )
    {
        SPMUtil_NoteOff( spmidiContext, CHANNEL, PITCH(i) , 0 );
        SPMUtil_PlayBuffers( spmidiContext, 2 );
    }
    /* Let voices decay and turn themselves off. */
    SPMUtil_PlayMilliseconds( spmidiContext, 500 );
}

/*******************************************************************/
/**
 * Play differing numbers of notes very rapidly, check that allocator
 * is correct.
 * @return 0 if all tests are successful, non-0 otherwise
 */
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;
    int i;

    QA_Init( "qa_vary_max" );

    printf("SPMIDI Test: play scale with varying max notes. SR = %d\n", SAMPLE_RATE );

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, "recorded_midi.wav", SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, PROGRAM );

    /* Alternate few and many notes. */
    for( i=1; i<=MAX_ON; i+=2 )
    {
        PlayScaleMax( spmidiContext, i );
        PlayScaleMax( spmidiContext, MAX_ON - i + 1 );
    }

    SPMUtil_Stop(spmidiContext);


    printf("Test finished.\n");

error:
    return QA_Term( 16 );
}
