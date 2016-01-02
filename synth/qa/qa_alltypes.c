/* $Id: qa_alltypes.c,v 1.6 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @brief Verify checksum, min and max on a few buffers of audio.
 *
 * Test a very short segment of audio by comparing a checksum, min and max
 * against an expected result. This is designed for validating a Verilog
 * simulation of a chip running the ME3000.
 *
 * The test uses a very small MXMF file that contains a 16 bit wavetable.
 * it plays three notes, one each of type ME1000, ME2000 and ME3000.
 *
 * To calibrate the expected results, set PRINT_EXPECTED as (1) below.
 * Then run the test on an accurate simulator using exactly the same
 * settings in "spmidi_user_config.h". Also use the same orchestra, etc.
 *
 * Then update the expected results in this file and run the test on the target platform.
 *
 * You can disable the main() function below and call TestAllVoiceTypes()
 * from your own test harness.
 *
 * Author: Phil Burk, Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/song_player.h"
#include "spmidi/qa/qa_tools.h"

/* Set this to (1) if you want to print expected data for a subsequent test.
 * For most devices you can generate the result on a PC.
 * for ARM946 and other Blackfin that use DSP extensions,
 * you will need to generate the expected results on a simulator.
 */
#define PRINT_EXPECTED      (1)

/* Set this to 1 for PC and ARMs without DSP extensions. */
#define TARGET_GENERIC      (1)
/* Set this to 1 for ARMs with DSP extensions like the ARM946. */
#define TARGET_ARMDSP       (0)
/* Set this to 1 for Blackfin DSPs. */
#define TARGET_BLACKFIN     (0)


#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)

/* Define largest buffer needed. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
short samples[SAMPLES_PER_BUFFER];

extern const unsigned char QuickAllTypes[];
extern int QuickAllTypes_size;

typedef struct TestResult_s
{
    int voiceCount;
    int checksum;
    int min;
    int max;
} TestResult_t;

#define EXPECTED_NUM_ALLOCATIONS   (19)

#define EXPECTED_BYTES_ALLOCATED   (9784 + (24 * SPMIDI_MAX_VOICES))

#if TARGET_GENERIC
TestResult_t sExpectedResults[] =
{
    {  3,        0,        0,        0 },
    {  3,   135185,        0,      281 },
    {  3,  3415556,   -11954,    11260 },
    {  3, -7098651,   -15682,    16625 },
    {  3, -2439922,   -16119,    13330 },
};
#elif TARGET_ARMDSP
TestResult_t sExpectedResults[] =
{
    {  3,        0,        0,        0 },
    {  3,   143802,        0,      259 },
    {  3,  1196813,   -13382,     9908 },
    {  3, -5271704,   -15513,    12722 },
    {  3,  -819788,   -18688,    16850 },
};
#endif

#define NUM_BUFFERS  (sizeof(sExpectedResults)/sizeof(TestResult_t))


/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then generate a checksum.
 */
static int ChecksumAudioBuffer(SPMIDI_Context *spmidiContext, TestResult_t *resultPtr )
{
    spmUInt32 checksum = 0;
    int min = 0;
    int max = 0;
    int numSamples;
    int i;

    /* Generate a buffer full of audio data as 16 bit samples. */
    int numFrames = SPMIDI_ReadFrames( spmidiContext, samples, SPMIDI_GetFramesPerBuffer(),
                       SAMPLES_PER_FRAME, 16 );
    if( numFrames < 0 )
    {
        return numFrames;
    }

    numSamples = numFrames * SAMPLES_PER_FRAME;

    for( i=0; i<numSamples; i++ )
    {
        int sample = samples[i];
        int shifter = i & 7;
        checksum += (sample << shifter);

        if( max < sample ) max = sample;
        if( min > sample ) min = sample;
    }

    resultPtr->min = min;
    resultPtr->max = max;
    resultPtr->checksum = checksum;
    return 0;
}

/****************************************************************/
/**
 * Generate a few buffers of MIDI and check the min, max and checksum.
 * Test various aspects other aspects of the SPMIDI System.
 * The test will count success and return a positive number if it passes.
 * If an SPMIDI error is encountered, then that large negative error code will be returned.
 * If one the of QA tests fails, eg a checksum, then the current numSuccesses will be returned.
 * This allows one to deduce what part of the test failed from a single return value.
 *
 * @return positive number on success, negative on error
 */
int TestAllVoiceTypes( void )
{
    int             result;
    SPMIDI_Context *spmidiContext = NULL;
    SongPlayer     *songPlayerContext = NULL;
    int             i;
    TestResult_t  testResult;

    SPMIDI_Initialize();

#define TALLY_RESULT \
    if( result < 0 ) \
    { \
        QA_CountError(); \
        goto error; \
    } \
    QA_CountSuccess();

    /* Start synthesis engine with default number of voices. */
    result = SPMIDI_CreateContext(  &spmidiContext, SAMPLE_RATE );
    TALLY_RESULT;

    /* Create a player for the song */
    result = SongPlayer_Create( &songPlayerContext, spmidiContext, (unsigned char *) QuickAllTypes, QuickAllTypes_size );
    TALLY_RESULT;

    /* Start the songplayer */
    result = SongPlayer_Start( songPlayerContext );
    TALLY_RESULT;

    /* Check number of memory allocations after song loaded.
     * This verifies XMF/DLS parsing and playing of WAV data in place.
     */
#if PRINT_EXPECTED
    printf("#define EXPECTED_NUM_ALLOCATIONS   (%d)\n", SPMIDI_GetMemoryAllocationCount() );
    printf("#define EXPECTED_BYTES_ALLOCATED   (%d)\n", SPMIDI_GetMemoryBytesAllocated() );
#else
    if( SPMIDI_GetMemoryAllocationCount() != EXPECTED_NUM_ALLOCATIONS ) \
    {
        QA_CountError();
        goto error;
    }
    QA_CountSuccess();

    if( SPMIDI_GetMemoryBytesAllocated() != EXPECTED_BYTES_ALLOCATED ) \
    {
        QA_CountError();
        goto error;
    }
    QA_CountSuccess();
#endif

    for( i=0; i<NUM_BUFFERS; i++ )
    {
        result = SongPlayer_PlayFrames( songPlayerContext, SPMIDI_GetFramesPerBuffer() );
        TALLY_RESULT;
        
        result = ChecksumAudioBuffer(spmidiContext, &testResult );
        TALLY_RESULT;

#if PRINT_EXPECTED
        printf("    { %2d, %8d, %8d, %8d },\n", SPMIDI_GetActiveNoteCount(spmidiContext),
                testResult.checksum, testResult.min, testResult.max );
#else

        if( SPMIDI_GetActiveNoteCount(spmidiContext)  != sExpectedResults[i].voiceCount )
        {
            QA_CountError();
            goto error;
        }
        QA_CountSuccess();

#define  CHECK_FIELD(field) \
        if( testResult.field != sExpectedResults[i].field ) \
        { \
            QA_CountError(); \
            goto error; \
        } \
        QA_CountSuccess();

        CHECK_FIELD(checksum);
        CHECK_FIELD(min);
        CHECK_FIELD(max);
#endif
    }

    /* Stop playing */
    result = SongPlayer_Stop( songPlayerContext );
    TALLY_RESULT;

error:
    /* Clean everything up */
    if( songPlayerContext != NULL )
        SongPlayer_Delete( songPlayerContext );
        
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext(spmidiContext);


    SPMIDI_Terminate();

    return result;
}

#if 1
/*******************************************************************/
/* Run test and print the SUCCESS or failure. */
int main(void);
int main(void)
{
    
    QA_Init( "qa_alltypes" );
    
    TestAllVoiceTypes();

    return QA_Term( 14 );;
}
#endif
