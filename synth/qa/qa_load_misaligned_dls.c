/* $Id: qa_load_misaligned_dls.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test loading of a DLS file.
 * Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#define SPMIDI_DIR "/nomad/MIDISynth/code/spmidi/"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "spmidi/include/spmidi_config.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/include/spmidi_load.h"

#include "spmidi/engine/memtools.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/include/dls_parser.h"
#include "qa_common.h"

#define DEFAULT_FILENAME    QADATA_DIR"multi_sine_set.dls"
#define PROGRAM_START       (48)

#define DEFAULT_BANK        ((121 << 8) + 0)
#define NUM_PROGRAMS        (3)

#define SAMPLE_RATE         (22050)
#define PITCH               (60)
#define CHANNEL             (0)
#define DURATION            (5 * 1000)

#if 1
/*******************************************************************/
int main(int argc, char **argv);
int main(int argc, char **argv)
{
    SPMIDI_Context *spmidiContext = NULL;
    spmSInt32 err;
    int bank, program;
    int i;

    DLSParser *dlsParser;
    char* fileName;
    unsigned char *fileStart, *moveToStart, *s, *d;
    spmSInt32 fileSize;
    char *outputFileName = NULL;
//  char *outputFileName = "spmidi_output.wav";


    fileName = ( argc < 2 ) ? DEFAULT_FILENAME : argv[1];
    bank = ( argc < 3 ) ? DEFAULT_BANK : atoi( argv[2] );
    program = ( argc < 4 ) ? PROGRAM_START : atoi( argv[3] );

    printf("SPMIDI Test: load DLS file on bank %4x, program %d = %s\n", bank, program, MIDI_GetProgramName( program )  );

    printf("Deliberately misalign the waves by copying them to another location, starting on an odd byte.\n");

    SPMIDI_Initialize();

    fileStart = SPMUtil_LoadFileImage( fileName, (int *)&( fileSize ) );
    if( fileStart != NULL )
    {
        /* Deliberately misalign the waves by copying them to another location, starting on an odd byte */
        moveToStart = SPMIDI_AllocateMemory( fileSize + 1 );
        if( moveToStart != NULL )
        {
            s = fileStart;
            d = ++moveToStart;
            for( i = 0; i < fileSize; i++ )
            {
                *d++ = *s++;
            }
            err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, outputFileName, SPMUTIL_OUTPUT_MONO );
            if( err < 0 )
                goto error;

            /* Turn off compressor so we can see envelopes easier. */
            err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
            if( err < 0 )
                goto error;

            err = DLSParser_Create( &dlsParser, moveToStart, fileSize );
            if( err < 0 )
                goto error;

            err = DLSParser_Parse( dlsParser );
            if( err < 0 )
                goto error;

            err = DLSParser_Load( dlsParser, spmidiContext );
            if( err < 0 )
                goto error;
        }
        else
        {
            printf("Error: out of memory\n");
            err = -1;
            goto error;
        }
    }
    else
    {
        printf("Error: can't open file %s\n", fileName);
        err = -1;
        goto error;
    }

    SPMUtil_BankSelect( spmidiContext, CHANNEL, bank );

    for( i=0; i<NUM_PROGRAMS; i++ )
    {
        int currentProgram = program + i;
        printf("Program 0x%02X\n", currentProgram);
        SPMUtil_ProgramChange( spmidiContext, CHANNEL, currentProgram );

        /* Note On */
        SPMUtil_NoteOn( spmidiContext, CHANNEL, PITCH, 64 );
        SPMUtil_PlayMilliseconds( spmidiContext, DURATION );

        /* Note Off */
        SPMUtil_NoteOff( spmidiContext, CHANNEL, PITCH, 0 );
        SPMUtil_PlayMilliseconds( spmidiContext, 500 );

    }

    DLSParser_Delete( dlsParser );

    SPMUtil_Stop(spmidiContext);

    // FIXME    SPMIDI_Terminate();

    printf("Test finished OK.\n");
    return err;
error:
    printf("Error in test = %d = %s\n", err, SPMUtil_GetErrorText( err ) );
    return err;
}
#endif
