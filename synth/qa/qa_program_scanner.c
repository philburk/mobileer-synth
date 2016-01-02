/* $Id: qa_program_scanner.c,v 1.1 2007/10/02 16:23:11 philjmsl Exp $ */
/**
 *
 * @file qa_program_scanner.c
 * @brief Test list of bank/program/pitches used in a MIDIFile.
 *
 * @author Phil Burk, Copyright 2007 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/program_list.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/qa/qa_tools.h"

/*******************************************************************/
/**
 * Test scanning a MIDI File.
 * @return 0 if all tests successful, non-0 if not
 */
int main(void);
int main(void)
{
    int err;
    SPMIDI_ProgramList *programList =  NULL;
    void *data = NULL;
    int  fileSize;
    const char inputFileName[] = "E:\\nomad\\MIDISynth\\data\\midi\\GMSuite\\011Vibraphone.mid";

    QA_Init( "qa_program_scanner" );

    SPMIDI_Initialize();

    err = SPMIDI_CreateProgramList( &programList );
    QA_Assert( (err==0), "Program list created." );
    if( programList == NULL ) goto error;

    /* Load MIDI File into a memory image. */
    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    err = MIDIFile_ScanForPrograms( programList, data, fileSize );
    QA_Assert( (err==0), "MIDIFilePlayer_ScanForPrograms" );

    QA_Assert( (SPMIDI_IsProgramUsed( programList, 0, 10 ) == 0), "Below vibraphone" );
    QA_Assert( (SPMIDI_IsProgramUsed( programList, 0, 11 ) == 1), "Vibraphone" );

    SPMIDI_DeleteProgramList( programList );

error:
    SPMIDI_Terminate();
    return QA_Term( 4 );
}
