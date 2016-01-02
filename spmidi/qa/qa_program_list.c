/* $Id: qa_program_list.c,v 1.1 2007/10/02 16:23:11 philjmsl Exp $ */
/**
 *
 * @file qa_program_list.c
 * @brief Test list of bank/program/pitches used.
 *
 * @author Phil Burk, Copyright 2007 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/program_list.h"
#include "spmidi/qa/qa_tools.h"

/*******************************************************************/
/**
 * Test adding to ProgramList
 * @return 0 if all tests successful, non-0 if not
 */
int main(void);
int main(void)
{
    int i,j;
    int err;
    SPMIDI_ProgramList *programList =  NULL;
    QA_Init( "qa_program_list" );

    SPMIDI_Initialize();

    err = SPMIDI_CreateProgramList( &programList );
    QA_Assert( (err==0), "Program list created." );
    if( programList == NULL ) goto error;

    QA_Assert( (SPMIDI_IsProgramUsed( programList, 3, 128 ) == SPMIDI_Error_OutOfRange), "Program too high");
    QA_Assert( (SPMIDI_IsProgramUsed( programList, 3, -1 ) == SPMIDI_Error_OutOfRange), "Program too low");

    QA_Assert( (SPMIDI_SetProgramUsed( programList, 3, 17 ) == 0), "Set 3,17" );
    QA_Assert( (SPMIDI_IsProgramUsed( programList, 3, 16 ) == 0), "Same bank.");
    QA_Assert( (SPMIDI_IsProgramUsed( programList, 3, 17 ) == 1), "Just set.");
    QA_Assert( (SPMIDI_IsProgramUsed( programList, 2, 17 ) == 0), "Different bank.");

    // Walking ones test.
    for( i=0; i<128l; i++ )
    {
        SPMIDI_SetProgramUsed( programList, 2, i );
        for( j=0; j<128l; j++ )
        {
            int flag = SPMIDI_IsProgramUsed( programList, 2, j );
            int expected = ( i == j );
            if( flag == expected )
            {
                QA_CountSuccess();
            }
            else
            {
                printf("ERROR in walking ones test. i = %d, j = %d\n", i, j );
                QA_CountError();
            }
        }
        
        SPMIDI_ClearProgramUsed( programList, 2, i );
    }

    QA_Assert( (SPMIDI_IsDrumUsed( programList, 5, 128, 45 ) == SPMIDI_Error_OutOfRange), "Drum program too high");
    QA_Assert( (SPMIDI_IsDrumUsed( programList, 5, -1, 45 ) == SPMIDI_Error_OutOfRange), "Drum program too low");
    QA_Assert( (SPMIDI_IsDrumUsed( programList, 5, 7, 128 ) == SPMIDI_Error_OutOfRange), "Drum note too high");
    QA_Assert( (SPMIDI_IsDrumUsed( programList, 5, 7, -1 ) == SPMIDI_Error_OutOfRange), "Drum note too low");

    QA_Assert( (SPMIDI_SetDrumUsed( programList, 5, 1, 69 ) == 0), "Set 5, 69" );
    QA_Assert( (SPMIDI_IsDrumUsed( programList, 5, 1, 63 ) == 0), "Same bank.");
    QA_Assert( (SPMIDI_IsDrumUsed( programList, 5, 1, 69 ) == 1), "Just set.");
    QA_Assert( (SPMIDI_IsDrumUsed( programList, 4, 1, 69 ) == 0), "Different bank.");

error:
    SPMIDI_Terminate();
    return QA_Term( (128*128) + 7 + 8 );
}
