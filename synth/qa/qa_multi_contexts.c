/* $Id: qa_multi_contexts.c,v 1.4 2007/10/02 16:53:20 philjmsl Exp $ */
/**
 *
 * @file qa_multi_contexts.c
 * @brief Test creating and deleting multiple SPMIDI_Contexts.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/qa/qa_tools.h"

/*******************************************************************/
/**
 * @return 0 if all tests succeed, non-0 otherwise.
 */
int main(void);
int main(void)
{
    int err;
    int i;
    int numContexts = 0;

#define MAX_CONTEXTS   (8)

    SPMIDI_Context *contexts[MAX_CONTEXTS];

    QA_Init( " qa_multi_contexts" );

    printf("Test Multiple Contexts.\n");

    SPMIDI_Initialize();

    for( i=0; i<MAX_CONTEXTS; i++ )
    {
        err = SPMIDI_CreateContext( &contexts[i], 44100 );
        QA_Assert( (err == 0), "SPMIDI_CreateContext" );
        if( err == 0 )
        {
            numContexts++;
            printf("Created %d contexts\n", numContexts);
        }
        else
        {
            break;
        }
    }


    for( i=0; i<numContexts; i++ )
    {
        SPMIDI_DeleteContext(contexts[i]);
    }

    SPMIDI_Terminate();

    return QA_Term(MAX_CONTEXTS);
}
