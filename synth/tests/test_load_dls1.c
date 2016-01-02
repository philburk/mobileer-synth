/* $Id$ */
/**
 *
 * Test loading of a DLS file.
 * Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/include/spmidi_load.h"

#include "spmidi/engine/memtools.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/include/dls_parser.h"

#define DATADIR "E:\\nomad\\MIDISynth\\data\\dls\\ArticulationTests"

//#define DEFAULT_FILENAME  DATADIR"\\ModLFO_Tests\\Mod_LFO_Freq.dls"

//#define DEFAULT_FILENAME  DATADIR"\\ModLFO_Tests\\Mod_LFO_Delay.dls"

//#define DEFAULT_FILENAME  DATADIR"\\VibLFO_Tests\\Vib_LFO_Freq.dls"

//#define DEFAULT_FILENAME  DATADIR"\\VibLFO_Tests\\Vib_LFO_Delay.dls"
//#define PROGRAM_START       (0x09)

//#define DEFAULT_FILENAME  DATADIR"\\VolEnv_Tests\\Vol_Env_Attack.dls"
//#define PROGRAM_START       (15)

#define DEFAULT_FILENAME  DATADIR"\\VolEnv_Tests\\Vol_Env_Decay.dls"
#define PROGRAM_START       (21)

#define DEFAULT_BANK        ((121 << 8) + 0)
#define NUM_PROGRAMS        (3)

#define SAMPLE_RATE         (22050)
#define PITCH               (60)
#define CHANNEL             (0)
#define DURATION            (35 * 1000)

#if 1
/*******************************************************************/
int main(int argc, char **argv);
int main(int argc, char **argv)
{
    SPMIDI_Context *spmidiContext = NULL;
    spmSInt32 err;
    int bankIndex, program;
    int i;

    DLSParser *dlsParser;
    char* fileName;
    unsigned char* fileStart;
    spmSInt32 fileSize;
//  char *outputFileName = NULL;
    char *outputFileName = "spmidi_output.wav";


    fileName = ( argc < 2 ) ? DEFAULT_FILENAME : argv[1];
    bankIndex = ( argc < 3 ) ? DEFAULT_BANK : atoi( argv[2] );
    program = ( argc < 4 ) ? PROGRAM_START : atoi( argv[3] );

    printf("SPMIDI Test: load DLS file on bank %4x, program %d = %s\n", bankIndex, program, MIDI_GetProgramName( program )  );

    SPMIDI_Initialize();

    fileStart = SPMUtil_LoadFileImage( fileName, (int *)&( fileSize ) );
    if( fileStart != NULL )
    {
        err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, outputFileName, SPMUTIL_OUTPUT_MONO );
        if( err < 0 )
            goto error;

        /* Turn off compressor so we can see envelopes easier. */
        err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
        if( err < 0 )
            goto error;

        err = DLSParser_Create( &dlsParser, fileStart, fileSize );
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
        printf("Error: can't open file %s\n", fileName);
        err = -1;
        goto error;
    }

    SPMUtil_BankSelect( spmidiContext, CHANNEL, bankIndex );

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

    SPMIDI_Terminate();

    printf("Test finished OK.\n");
    return err;
error:
    printf("Error in test = %d = %s\n", err, SPMUtil_GetErrorText( err ) );
    return err;
}
#endif
