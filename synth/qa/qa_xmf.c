/* $Id: qa_xmf.c,v 1.19 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * @file qa_xmf.c
 * @brief Test XMF parser, DLS parser, SongPlayer.
 * @author Robert Marsanyi, Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 * Tests include:
 * <ul>
 * <li>checking that good files parse correctly</li>
 * <li>checking that bad files return appropriate errors</li>
 * <li>checking that in all cases there are no memory leaks.</li>
 *
 * The datafiles are specifically munged to test different aspects of the different parsers.
 * NOTE: these tests expect to find specific test data files in a directory
 * QADATA_DIR, which is defined by qa_common.h.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/examples/midifile_names.h"
#include "spmidi/include/xmf_parser.h"
#include "spmidi/include/dls_parser.h"
#include "spmidi/include/song_player.h"
#include "spmidi/engine/memtools.h"

#include "qa_common.h"

#define QABADXMF_FILENAME     QADATA_DIR"textfile.dls"
#define QAMISSINGSMF_FILENAME QADATA_DIR"missingSMFdata.mxmf"
#define QAGOOD_FILENAME       QADATA_DIR"good.mxmf"
#define QATYPE1_FILENAME      QADATA_DIR"type1.xmf"

#define QABADDLS_FILENAME         QADATA_DIR"textfile.dls"
#define QAMISSINGFMT_FILENAME     QADATA_DIR"missingFMTchunk.dls"
#define QATRUNCATEDWSMP_FILENAME  QADATA_DIR"truncatedWSMPchunk.dls"
#define QAGOODDLS_FILENAME        QADATA_DIR"good.dls"

#define NUM_SCRIBBLES         (100)

int sReturn = 0;
MemoryCheck_t gMemoryCheck;

/* Link with QuickAllTypes.mxmf.c */
extern const unsigned char QuickAllTypes[];
extern const int QuickAllTypes_size;

static void InitTest( void )
{
    /* setup for all tests */
    SPMIDI_SetMemoryAllocationLimit( SPMIDI_ALLOCATION_UNLIMITED );
    SPMIDI_SetMemoryAllocationCountdown( SPMIDI_ALLOCATION_UNLIMITED );
}

/***********************************************************************/
static void TestBadXMF( char *filename )
{
    const char *testname = "TestBadXMF";
    XMFParser *xmfParser = NULL;
    unsigned char *fileStart;
    int fileSize;
    char msg[80];

    InitTest();
    InitMemoryCheck( &gMemoryCheck, &sReturn );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( filename, &( fileSize ) );

    /* XMFParser_IsXMF() should report no */
    sprintf( msg, "IsXMF reports %s is bad", filename );
    TestAssert( XMFParser_IsXMF( fileStart ) == 0, testname, msg, &sReturn );

    /* Trying to parse the file should return an error */
    TestAssert( XMFParser_Create( &xmfParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );
    TestAssert( XMFParser_Parse( xmfParser ) < 0, testname, "Parser returns error.", &sReturn );

    XMFParser_Delete( xmfParser );
    SPMUtil_FreeFileImage( fileStart );

    /* Check for memory leaks */
    TestMemory( &gMemoryCheck, testname, &sReturn );
}

/***********************************************************************/
static void TestMissingSMF( void )
{
    const char *testname = "TestMissingSMF";
    XMFParser *xmfParser = NULL;
    unsigned char *fileStart;
    int fileSize;
    spmSInt32 maxSize;

    InitTest();
    InitMemoryCheck( &gMemoryCheck, &sReturn );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( QAMISSINGSMF_FILENAME, &( fileSize ) );

    /* XMFParser_IsXMF() should report yes */
    TestAssert( XMFParser_IsXMF( fileStart ) == 1, testname, "IsXMF reports missingSMFdata.mxmf is OK.", &sReturn );

    /* Trying to parse the file should work */
    TestAssert( XMFParser_Create( &xmfParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );
    TestAssert( XMFParser_Parse( xmfParser ) == 0, testname, "Parser parsed OK.", &sReturn );

    /* Getting an SMF from the result should fail */
    TestAssert( XMFParser_GetSMF( xmfParser, &maxSize ) == NULL, testname, "GetSMF returns NULL.", &sReturn );

    XMFParser_Delete( xmfParser );
    SPMUtil_FreeFileImage( fileStart );

    /* Check for memory leaks */
    TestMemory( &gMemoryCheck, testname, &sReturn );
}

/***********************************************************************/
static void TestWrongVersion( char *filename )
{
    const char *testname = "TestWrongVersion";
    XMFParser *xmfParser = NULL;
    unsigned char *fileStart;
    int fileSize;
    char msg[80];

    InitTest();
    InitMemoryCheck( &gMemoryCheck, &sReturn );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( filename, &( fileSize ) );

    /* XMFParser_IsXMF() should report OK */
    sprintf( msg, "IsXMF reports %s is bad", filename );
    TestAssert( XMFParser_IsXMF( fileStart ) == 1, testname, msg, &sReturn );

    TestAssert( XMFParser_Create( &xmfParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );

    /* Trying to parse the file used to return an error, but parser is now more permissive
     * and does not flag an error just because the XMF is not the right version, so
     * it's indeterminate whether or not we'll get a parse error.  So we remove this assert. */

    /* TestAssert( XMFParser_Parse( xmfParser ) < 0, testname, "Parser returns error.", &sReturn ); */

    XMFParser_Delete( xmfParser );
    SPMUtil_FreeFileImage( fileStart );

    /* Check for memory leaks */
    TestMemory( &gMemoryCheck, testname, &sReturn );
}

/***********************************************************************/
static void TestRandomCorruption( void )
{
    const char *testname = "TestRandomCorruption";
    XMFParser *xmfParser = NULL;
    unsigned char *fileStart, *randomStart;
    int fileSize, i, j;
    spmSInt32 randomSize;

    InitTest();

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( QAGOOD_FILENAME, &( fileSize ) );

    for( i = 0; i < NUM_SCRIBBLES; i++ )
    {
        /* Scribble on it */
        randomStart = fileStart + ( ChooseRandom( fileSize ) );
        randomSize = ChooseRandom( fileSize - (randomStart - fileStart));
        for( j = 0; j < randomSize; j++ )
        {
            randomStart[j] = (char)ChooseRandom( 0x7F );
        }

        /* Try parsing */
        InitMemoryCheck( &gMemoryCheck, &sReturn );
        TestAssert( XMFParser_Create( &xmfParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );
        XMFParser_Parse( xmfParser ) ;
        XMFParser_Delete( xmfParser );
        TestMemory( &gMemoryCheck, testname, &sReturn );
    }

    SPMUtil_FreeFileImage( fileStart );
}

/***********************************************************************/
static void TestBadDLS( char *filename )
{
    const char *testname = "TestBadDLS";
    DLSParser *dlsParser = NULL;
    unsigned char *fileStart;
    int fileSize;

    InitTest();
    InitMemoryCheck( &gMemoryCheck, &sReturn );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( filename, &( fileSize ) );

    /* Trying to parse the file should return an error */
    TestAssert( DLSParser_Create( &dlsParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );
    TestAssert( DLSParser_Parse( dlsParser ) < 0, testname, "Parser returns error.", &sReturn );

    DLSParser_Delete( dlsParser );
    SPMUtil_FreeFileImage( fileStart );

    /* Check for memory leaks */
    TestMemory( &gMemoryCheck, testname, &sReturn );
}

/***********************************************************************/
static void TestGoodDLS( char *filename )
{
    const char *testname = "TestGoodDLS";
    DLSParser *dlsParser = NULL;
    unsigned char *fileStart = NULL;
    int fileSize;
    SPMIDI_Context *spmidiContext = NULL;

    InitTest();
    InitMemoryCheck( &gMemoryCheck, &sReturn );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( filename, &( fileSize ) );

    /* Start the MIDI playback engine */
    TestAssert( SPMIDI_CreateContext( &spmidiContext, 22050 ) == 0, testname, "MIDI context created OK.", &sReturn );

    /* Trying to parse the file should not return an error */
    TestAssert( DLSParser_Create( &dlsParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );
    TestAssert( DLSParser_Parse( dlsParser ) == 0, testname, "Parser OK.", &sReturn );
    TestAssert( DLSParser_Load( dlsParser, spmidiContext ) == 0, testname, "Load OK.", &sReturn );

    DLSParser_Delete( dlsParser );
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext( spmidiContext );
    if( fileStart != NULL )
        SPMUtil_FreeFileImage( fileStart );

    /* Check for memory leaks */
    TestMemory( &gMemoryCheck, testname, &sReturn );
}

/***********************************************************************/
static void TestRandomDLSCorruption( void )
{
    const char *testname = "TestRandomDLSCorruption";
    DLSParser *DLSParser = NULL;
    unsigned char *fileStart = NULL, *randomStart;
    int fileSize, i, j;
    spmSInt32 randomSize;
    SPMIDI_Context *spmidiContext = NULL;

    InitTest();

    /*
     * Set maximum memory allocation allowed, so that we don't successfully allocate huge
     * gobs of RAM and spend forever zeroing them out and bringing the host machine to 
     * its knees.
     */
    SPMIDI_SetMemoryAllocationLimit( 1 << 20 );  /* about 1 Mb */

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( QAGOODDLS_FILENAME, &( fileSize ) );

    /* Start the MIDI playback engine */
    TestAssert( SPMIDI_CreateContext( &spmidiContext, 22050 ) == 0, testname, "MIDI context created OK.", &sReturn );

    for( i = 0; i < NUM_SCRIBBLES; i++ )
    {
        /* Scribble on it */
        randomStart = fileStart + ( ChooseRandom( fileSize ) );
        randomSize = ChooseRandom( fileSize - (randomStart - fileStart) );
        for( j = 0; j < randomSize; j++ )
        {
            randomStart[j] = (char)ChooseRandom( 0x7F );
        }

        /* Try parsing */
        InitMemoryCheck( &gMemoryCheck, &sReturn );
        TestAssert( DLSParser_Create( &DLSParser, fileStart, fileSize ) == 0, testname, "Parser created OK.", &sReturn );
        DLSParser_Parse( DLSParser );
        DLSParser_Load( DLSParser, spmidiContext );
        DLSParser_Delete( DLSParser );
        TestMemory( &gMemoryCheck, testname, &sReturn );
    }

    if( spmidiContext != NULL )
        SPMIDI_DeleteContext( spmidiContext );
    if( fileStart != NULL )
        SPMUtil_FreeFileImage( fileStart );

    SPMIDI_SetMemoryAllocationLimit( SPMIDI_ALLOCATION_UNLIMITED );
}

/***********************************************************************/
static void TestCountdownAllocation( int numAllocs, int maxAllocs )
{
    int i;
    void *ptr;

    InitTest();
    SPMIDI_SetMemoryAllocationCountdown( maxAllocs );
    for( i = 0; i < numAllocs; i++ )
    {
        /*
        PRTMSG( "Attempt #" );
        PRTNUMD( i );
        PRTMSG( "... " );
        */

        ptr = SPMIDI_AllocateMemory( 24 );
        if( i == (maxAllocs) && maxAllocs != SPMIDI_ALLOCATION_UNLIMITED )
        {
            TestAssert( ptr == NULL, "TestCountdownAllocation", "Allocation should fail on maxAllocs.", &sReturn );
        }
        else
        {
            TestAssert( ptr != NULL, "TestCountdownAllocation", "Allocation should succeed.", &sReturn );
        }
        SPMIDI_FreeMemory( ptr );
    }

    SPMIDI_SetMemoryAllocationCountdown( SPMIDI_ALLOCATION_UNLIMITED );
}

/***********************************************************************/
static void TestMultipleIterations( int numIterations )
{
    SongPlayer *songPlayer = NULL;
    SPMIDI_Context *spmidiContext = NULL;
    int i, fileSize;
    unsigned char *fileStart;

    InitTest();

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( QAGOOD_FILENAME, &( fileSize ) );

    InitMemoryCheck( &gMemoryCheck, &sReturn );
    for( i = 0; i < numIterations; i++ )
    {
        /* Create a context */
        TestAssert( SPMIDI_CreateContext( &spmidiContext, QA_SAMPLE_RATE ) == 0, "TestMultipleIterations", "SPMIDI context created OK.", &sReturn );

        /* Create a song, and delete it */
        TestAssert( SongPlayer_Create( &songPlayer, spmidiContext, fileStart, fileSize ) == 0, "TestMultipleIterations", "SongPlayer created OK.", &sReturn );
        SongPlayer_Delete( songPlayer );

        /* Delete the context */
        if( spmidiContext != NULL )
            SPMIDI_DeleteContext( spmidiContext );
    }
    TestMemory( &gMemoryCheck, "TestMultipleIterations", &sReturn );

    if( fileStart != NULL )
        SPMUtil_FreeFileImage( fileStart );
}

/***********************************************************************/
static void TestFailOnCount( void )
{
    SongPlayer *songPlayer = NULL;
    SPMIDI_Context *spmidiContext = NULL;
    int maxAllocs, fileSize, totalAllocationsNeeded;
    unsigned char *fileStart;
    char msg[80];

    InitTest();

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( QAGOOD_FILENAME, &( fileSize ) );

    /* Create a context */
    TestAssert( SPMIDI_CreateContext( &spmidiContext, QA_SAMPLE_RATE ) == 0, "TestFailOnCount", "SPMIDI context created OK.", &sReturn );

    /* Go through cycle once to establish maximum required allocations */
    TestAssert( SongPlayer_Create( &songPlayer, spmidiContext, fileStart, fileSize ) == 0, "TestFailOnCount", "SongPlayer created OK.", &sReturn );
    totalAllocationsNeeded = SPMIDI_GetMemoryAllocationCount();
    SongPlayer_Delete( songPlayer );

    /* Delete the context */
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext( spmidiContext );

    /* Set allocation countdown progressively from 0 to total needed */
    for( maxAllocs = 0; maxAllocs < totalAllocationsNeeded; maxAllocs++ )
    {
        /*
        PRTMSG( "------ Attempt #" );
        PRTNUMD( maxAllocs );
        PRTMSG( ": " );
        */

        SPMIDI_SetMemoryAllocationCountdown( maxAllocs );

        InitMemoryCheck( &gMemoryCheck, &sReturn );
        songPlayer = NULL;
        spmidiContext = NULL;

        if( SPMIDI_CreateContext( &spmidiContext, QA_SAMPLE_RATE ) == 0 )
        {
            sprintf( msg, "SongPlayer_Create() should fail (maxAllocs = %d).", maxAllocs );
            TestAssert( (SongPlayer_Create( &songPlayer, spmidiContext,
                                             fileStart, fileSize ) == SPMIDI_Error_OutOfMemory),
                "TestFailOnCount", msg, &sReturn );
            if( songPlayer != NULL )
            {
                SongPlayer_Delete( songPlayer );
            }
            SPMIDI_DeleteContext( spmidiContext );
        }
        sprintf( msg, "TestFailOnCount (maxAllocs = %d).", maxAllocs );
        TestMemory( &gMemoryCheck, msg, &sReturn );

        /* PRTMSG( "\n" ); */
    }

    if( fileStart != NULL )
        SPMUtil_FreeFileImage( fileStart );
}

/***********************************************************************/
static void TestInvalidVLQ( void )
{
    XMFParser *xmfParser = NULL;
    unsigned char* image = NULL;
    int i;

    InitTest();

    image = SPMIDI_AllocateMemory( QuickAllTypes_size );
    TestAssert( ( image != NULL ), "TestInvalidVLQ", "Allocated XMF image memory", &sReturn );
    MemTools_Copy( image, QuickAllTypes, QuickAllTypes_size );

    /*
    Corrupt FileHeader.FileLength by setting high bits to 1
    The Mobile XMF spec states that the maximum value for file length is
    28bits, or 4 bytes of VLQ.  So we should be able to force a VLQ error
    by setting the high bit for the rest of the file.
    */
    for( i = 16; i < QuickAllTypes_size; i++ )
    {
        image[i] |= 0x80;
    }

    /* Try parsing XMF, check for error */
    TestAssert( XMFParser_Create( &xmfParser, image, QuickAllTypes_size) >= 0, "TestInvalidVLQ", "Created XMF parser", &sReturn );
    TestAssert( XMFParser_Parse( xmfParser ) == XMFParser_Error_VLQTooLarge, "TestInvalidVLQ", "Correct error code thrown", &sReturn );

    /* Clean up */
    if( xmfParser != NULL ) XMFParser_Delete( xmfParser );
    if( image != NULL ) SPMIDI_FreeMemory( image );
}

/*******************************************************************/
/**
 * Run XMF, DLS and SongPlayer tests.
 * @return 0 if all tests succeed, non-0 otherwise
 */
int main(void);
int main(void)
{
    printf("XMF Tests.\n");

    SPMIDI_Initialize();

#if (SPMIDI_SUPPORT_MALLOC == 0)
#error SPMIDI_SUPPORT_MALLOC must be 1 for these tests.
#endif

#if (SPMIDI_USE_COMPRESSOR == 0)
#error SPMIDI_USE_COMPRESSOR should be 1 so we test allocation thoroughly.
#endif

    TestCountdownAllocation( 5, 10 );
    TestCountdownAllocation( 12, 10 );
    TestCountdownAllocation( 5, 5 );
    TestCountdownAllocation( 10, 0 );
    TestCountdownAllocation( 10, SPMIDI_ALLOCATION_UNLIMITED );

    TestBadXMF(QABADXMF_FILENAME);
    TestWrongVersion(QATYPE1_FILENAME);

    TestMissingSMF();
    TestRandomCorruption();

    TestFailOnCount();

    TestMultipleIterations(10);

    TestGoodDLS(QAGOODDLS_FILENAME);
    TestBadDLS(QABADDLS_FILENAME);
    TestBadDLS(QAMISSINGFMT_FILENAME);
    TestBadDLS(QATRUNCATEDWSMP_FILENAME);
    TestRandomDLSCorruption();

    TestInvalidVLQ();

    SPMIDI_Terminate();
    printf("\nqa_xmf: %s, result = %d <<<<<<<<<<<<<<<<<<<<<-!!\n",
        ((sReturn == 0) ? "SUCCESS" : "ERROR"),
        sReturn );

    return sReturn;
}
