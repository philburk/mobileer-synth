/* $Id: qa_articulation.c,v 1.6 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * @file qa_articulation.c
 * @brief Test DLS articulations.
 *
 * @author Phil Burk, Robert Marsanyi, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/examples/midifile_names.h"
#include "spmidi/include/xmf_parser.h"
#include "spmidi/include/dls_parser.h"

#include "qa_common.h"

#define QA_DLSFILENAME        QADATA_DIR"multi_sine_set.dls"
#define CHANNEL               (1)
#define SINES_BANK            (121)
#define MAX_VELOCITY          (127)
#define ERR_ALLOWED           (0.1)  /* allowed slop in measurement of attack or decay, as a multiplier */
#define NUM_TESTS             (10)

static int sReturn = 0;
static double sAttacks[6] = { 0.180, 0.454, 0.454, 0.086, 0.454, 0.311 };
static int sPrograms[6] = { 40, 48, 49, 59, 68, 73 };
static short samples[DETECT_AMP_FRAMES];

MemoryCheck_t gMemoryCheck;

static double CalculateAttackTime( SPMIDI_Context *spmidiContext )
{
    short newMaximum = 0, oldMaximum = 0;
    int numFrames = 0;

    /* while the amplitude is going up, keep track of how many frames we've done */
    do
    {
        oldMaximum = newMaximum;
        newMaximum = MeasureAmplitude( spmidiContext, samples );
        numFrames += DETECT_AMP_FRAMES;
    }
    while( (newMaximum > oldMaximum) && (newMaximum > 0) );

    /* at this point, we've stabilized.  Calculate how much actual time elapsed. */
    return (double)(numFrames) / QA_SAMPLE_RATE;
}

static int TestAttack( SPMIDI_Context *spmidiContext, double expected )
{
    double attackTime;

    /* Measure time to maximum amplitude */
    attackTime = CalculateAttackTime( spmidiContext );

    /* Compare with expected value */
    return (fabs(attackTime - expected) <= (expected * ERR_ALLOWED));
}

static void TestEnvelopes( void )
{
    const char *testname = "TestEnvelopes";
    DLSParser *dlsParser = NULL;
    SPMIDI_Context *spmidiContext = NULL;
    unsigned char *fileStart;
    int fileSize, i;

    InitMemoryCheck( &gMemoryCheck, &sReturn );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( QA_DLSFILENAME, &( fileSize ) );

    /* Create an SPMIDI context */
    SPMUtil_Start( &spmidiContext, QA_SAMPLE_RATE, NULL, 1 );

    /* Parse and load the DLS */
    TestAssert( DLSParser_Create( &dlsParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );
    TestAssert( DLSParser_Parse( dlsParser ) == 0, testname, "Parser parses OK.", &sReturn );
    TestAssert( DLSParser_Load( dlsParser, spmidiContext ) == 0, testname, "Parser loads OK.", &sReturn );

    /* Test some ADSRs */
    for( i = 0; i < NUM_TESTS; i++ )
    {
        /* Turn on test note */
        SPMUtil_BankSelect( spmidiContext, CHANNEL, SINES_BANK );
        SPMUtil_ProgramChange( spmidiContext, CHANNEL, sPrograms[0] - 1 );
        SPMUtil_NoteOn( spmidiContext, CHANNEL, 60, MAX_VELOCITY );

        // TestAssert( TestAttack( spmidiContext, sAttacks[ 0 ] ), testname, "Attack time within expected parms.", &sReturn );
        TestAssert( TestAttack( spmidiContext, 0.01 ), testname, "Attack time within expected parms.", &sReturn );
        // SPMUtil_PlayMilliseconds( spmidiContext, 1000 );
        /* TestDecay( spmidiContext, random( 127 ) ); */
    }

    /* Shut down */
    DLSParser_Delete( dlsParser );
    SPMUtil_Stop( spmidiContext );
    SPMUtil_FreeFileImage( fileStart );

    /* Check for memory leaks */
    TestMemory( &gMemoryCheck, testname, &sReturn );
}

/*******************************************************************/
/**
 * Test that articulations are being applied correctlyby measuring
 * attack times against expected values.
 * @return zero if all tests succeed, non-zero otherwise
 */
int main(void);
int main(void)
{
    printf("XMF Tests.\n");

    TestEnvelopes();

    return sReturn;
}
