/* $Id: print_midifile.c,v 1.3 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Print a MIDI File specified on command line.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/examples/midifile_names.h"

/****************************************************************/
int main( int argc, char ** argv )
{
    unsigned char *data = NULL;
    int fileSize;
    int result = -1;
    char *fileName = argv[1];

    if( argc > 1 )
    {
        fileName = argv[1];
    }
    else
    {
        fileName = DEFAULT_FILENAME;
    }

    printf("MIDI File: %s\n", fileName );

    data = SPMUtil_LoadFileImage( fileName, &fileSize );
    if( data == NULL )
    {
        printf("Error reading MIDI File.\n" );
        goto error;
    }

    result = MIDIFile_Print( data, fileSize );
    if( result < 0 )
    {
        printf("Error printing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) );
        goto error;
    }

    printf("result = %d\n", result );

error:
    if( data != NULL )
        SPMUtil_FreeFileImage( data );
    return result;
}

