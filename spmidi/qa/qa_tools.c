/* $Id: qa_tools.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * QA - Quality Assurance Tools
 *
 * @author Phil Burk, Copyright 1996 Phil Burk and Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/qa/qa_tools.h"

static const char *gTestName = "QA_Init() not called!";
static int gQA_NumSuccesses;
static int gQA_NumErrors;

/******************************************/
void QA_Init( const char *testName )
{
    gQA_NumSuccesses = 0;
    gQA_NumErrors = 0;
    gTestName = testName;
}

/******************************************/
void QA_CountSuccess( void )
{
    gQA_NumSuccesses++;
}
/******************************************/
void QA_CountError( void )
{
    gQA_NumErrors++;
}

/******************************************/
void QA_Assert( int ok, const char *message )
{
    if( ok )
    {
        QA_CountSuccess();
    }
    else
    {

        PRTMSG("QA: ERROR = " );
        PRTMSG( message );
        PRTMSG( "\n" );
        QA_CountError();
    }
}

/******************************************/
int QA_Term( int expectedSuccesses )
{
    int result = 0;

    PRTMSG("QA: successes = " );
    PRTNUMD( gQA_NumSuccesses );
    PRTMSG( ", errors = ");
    PRTNUMD( gQA_NumErrors );
    PRTMSG( "\n" );
    if( expectedSuccesses != gQA_NumSuccesses )
    {
        PRTMSG("QA: FAILED " );
        PRTMSG( gTestName );
        PRTMSG( ", ");
        PRTNUMD( gQA_NumSuccesses );
        PRTMSG( "successes, expected ");
        PRTNUMD( expectedSuccesses );
        PRTMSG( "\n" );
        result = 1;
    }
    if( gQA_NumErrors > 0 )
    {
        PRTMSG("QA: FAILED " );
        PRTMSG( gTestName );
        PRTMSG( ", ");
        PRTNUMD( gQA_NumErrors );
        PRTMSG( " errors in test.\n" );
        result = 2;
    }
    if( result == 0 )
    {
        PRTMSG("QA: PASSED " );
        PRTMSG( gTestName );
        PRTMSG( "\n" );
    }

    return result;
}
