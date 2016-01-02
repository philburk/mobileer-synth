/* $Id: qa_load_orchestra.c,v 1.2 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * @file qa_load_orchestra.c
 * @brief Test list of bank/program/pitches used in a MIDIFile.
 *
 * @author Phil Burk, Copyright 2007 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/program_list.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/qa/qa_tools.h"

/*******************************************************************/
/**
 * Test scanning a MIDI File and loading the Orchestra.
 * @return 0 if all tests successful, non-0 if not
 */
int main( void );
int main( void )
{
    int err;
    void *orchestraImage = NULL;
    int  orchestraFileSize;
    void *midiImage = NULL;
    int  midiFileSize;
    int preOrchestraCount = 0;

    StreamIO *sio = NULL;
    SPMIDI_Orchestra *orchestra = NULL;
    
    SPMIDI_ProgramList *programList =  NULL;

    const char orchestraFileName[] = "D:\\mobileer_work\\A_Orchestra\\exports\\exported.mbis";
    const char midiFileName[] = "E:\\nomad\\MIDISynth\\data\\midi\\GMSuite\\025SteelAcGuitar.mid";

    QA_Init( "qa_program_scanner" );


    SPMIDI_Initialize();

    PRTMSGNUMD( "Initial  num blocks allocated = ", SPMIDI_GetMemoryAllocationCount() );
    // Load MIDI file into an image.
    midiImage = SPMUtil_LoadFileImage( midiFileName, (int *)&( midiFileSize ) );
    if( midiImage == NULL )
    {
        PRTMSG("Error: can't open file ");
        PRTMSG( midiFileName );
        goto error;
    }

    PRTMSGNUMD( "Num blocks allocated before SPMIDI_CreateProgramList = ", SPMIDI_GetMemoryAllocationCount() );
    err = SPMIDI_CreateProgramList( &programList );
    if( err < 0 ) goto error;

    // Scan the MIDIFile to see what instruments we should load.
    err = MIDIFile_ScanForPrograms( programList, midiImage, midiFileSize );
    if( err < 0 ) goto error;

    // Load an Orchestra file into a memory stream and parse it.
    orchestraImage = SPMUtil_LoadFileImage( orchestraFileName, (int *)&( orchestraFileSize ) );
    if( orchestraImage == NULL )
    {
        PRTMSG("Error: can't open file ");
        PRTMSG( orchestraFileName );
        goto error;
    }

    // Create a stream object for reading the orchestra.
    sio = Stream_OpenImage( (char *)orchestraImage, orchestraFileSize );
    if( sio == NULL )
    {
        goto error;
    }

    preOrchestraCount = SPMIDI_GetMemoryAllocationCount();
    PRTMSGNUMD( "Num blocks allocated before SPMIDI_LoadOrchestra = ", preOrchestraCount );
    // Load just the instruments we need from the orchestra t play the song.
    err = SPMIDI_LoadOrchestra( sio, programList, &orchestra );
    if( err < 0 )
    {
        PRTMSGNUMD( "SPMIDI_LoadOrchestra returned ", err );
        goto error;
    }

error:
    PRTMSGNUMD( "Num blocks allocated before SPMIDI_DeleteOrchestra = ", SPMIDI_GetMemoryAllocationCount() );
    SPMIDI_DeleteOrchestra( orchestra );
    PRTMSGNUMD( "Num blocks allocated after SPMIDI_DeleteOrchestra = ", SPMIDI_GetMemoryAllocationCount() );
    QA_Assert( (SPMIDI_GetMemoryAllocationCount() == preOrchestraCount), "back to preOrchestraCount" );

    // Close the orchestra stream.
    if( sio != NULL )
    {
        Stream_Close( sio );
    }
    free( orchestraImage );
    free( midiImage );

    SPMIDI_DeleteProgramList( programList );
    PRTMSGNUMD( "Num blocks after SPMIDI_DeleteProgramList = ", SPMIDI_GetMemoryAllocationCount() );

    SPMIDI_Terminate();
    PRTMSGNUMD( "Num blocks allocated after SPMIDI_Terminate = ", SPMIDI_GetMemoryAllocationCount() );

    QA_Assert( (SPMIDI_GetMemoryAllocationCount() == 0), "Final mem count." );
    return QA_Term( 2 );
}
