/* $Id: qa_xmf_corrupt.c,v 1.9 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_xmf_corrupt.c
 * @brief Test XMF parser, DLS parser, SongPlayer for handling of corrupt files.
 *
 * This test systematically munges a Mobile XMF data file to see if errors are caught correctly.
 * It modifies each byte in the file in sequence. As a result, SongPlayer_Create() should either
 * return an error or zero. It should not crash.
 *
 * @author Robert Marsanyi, Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/xmf_parser.h"
#include "spmidi/include/dls_parser.h"
#include "spmidi/include/song_player.h"

#include "qa_common.h"
#include "spmidi/qa/qa_tools.h"
int sReturn = 0;
MemoryCheck_t gMemoryCheck;

#define SAMPLE_RATE (22050)

/* Link with QuickAllTypes.mxmf.c */
extern const unsigned char QuickAllTypes[];
extern const int QuickAllTypes_size;

static void InitTest( void )
{
    SPMIDI_Initialize();

    /* setup for all tests */
    SPMIDI_SetMemoryAllocationLimit( 100 * 1024 );
    SPMIDI_SetMemoryAllocationCountdown( SPMIDI_ALLOCATION_UNLIMITED );
}

/***********************************************************************/
static int TestCorruptFile( unsigned char *image, int numBytes )
{
    const char *testname = "TestRandomCorruption";
    int result;
    SPMIDI_Context *spmidiContext = NULL;
    SongPlayer     *songPlayerContext = NULL;

    InitMemoryCheck( &gMemoryCheck, &sReturn );

    /* Start synthesis engine with default number of voices. */
    QA_Assert( SPMIDI_CreateContext(  &spmidiContext, SAMPLE_RATE ) == 0, "spmidiContext created OK." );

    /* Create a player for the song. It may fail, which is OK because the file is corrupted. */
    result = SongPlayer_Create( &songPlayerContext, spmidiContext, image, numBytes );
    if( result == 0 )
    {
        SongPlayer_Delete( songPlayerContext );
        PRTMSG(".");
    }
    else
    {
        PRTMSG("X");
    }

    SPMIDI_DeleteContext( spmidiContext );

    TestMemory( &gMemoryCheck, testname, &sReturn );
    
    return sReturn;
}

/***********************************************************************/
/* Corrupt each byte in the image and test for crashes.
 * Each byte is munged in various ways.
 */
static int TestFileCorruption( unsigned char *image, int numBytes )
{
    int  i;
    int result = 0;

    for( i = 0; i < numBytes; i++ )
    {
        unsigned char savedByte = image[i];
        PRTMSG("i = ");
        PRTNUMD(i);
        PRTMSG(" = ");
        PRTNUMH(i);
        PRTMSG(", ");

        
#define CHECK_BYTE(val) \
        image[i] = (unsigned char) (val); \
        result = TestCorruptFile( image, numBytes ); \
        if( result != 0 ) \
        { \
            goto error; \
        }

        CHECK_BYTE(0x00);
        CHECK_BYTE(0x80);
        CHECK_BYTE(savedByte + 1);
        CHECK_BYTE(savedByte - 1);
        CHECK_BYTE(savedByte ^ 0x0055);
        CHECK_BYTE(savedByte ^ 0x00AA);
        CHECK_BYTE(savedByte & 0x00F0);
        CHECK_BYTE(savedByte & 0x000F);
        CHECK_BYTE(savedByte ^ 0x0080);
        CHECK_BYTE(savedByte ^ 0x0008);
        
        PRTMSG("\n");

        /* Restore original value. */
        image[i] = savedByte;
    }
error:
    return result;
}

/*******************************************************************/
/**
 * Run XMF, DLS and SongPlayer tests.
 * @return 0 if all tests succeed, non-0 otherwise
 */
int main(void);
int main(void)
{
    unsigned char *image;
    int result;

    PRTMSG("XMF Corruption Test.\n");
    QA_Init( "qa_xmf_corrupt" );

#if (SPMIDI_SUPPORT_MALLOC == 0)
#error SPMIDI_SUPPORT_MALLOC must be 1 for these tests.
#endif

#if (SPMIDI_USE_COMPRESSOR == 0)
#error SPMIDI_USE_COMPRESSOR should be 1 so we test allocation thoroughly.
#endif

    InitTest();

    image = SPMIDI_AllocateMemory( QuickAllTypes_size );
    if( image == NULL )
    {
        PRTMSG("Could not allocate memory!\n");
        return(-1);
    }
    MemTools_Copy( image, QuickAllTypes, QuickAllTypes_size );

    result = TestFileCorruption( image, QuickAllTypes_size );

    
    PRTMSG("  X = error detected\n");
    PRTMSG("  . = file still playable\n");

    QA_Assert( sReturn == 0, "memory result" );

    return QA_Term( 18951 );
}
