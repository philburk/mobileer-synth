/*
 * $Id: dumpdls.c,v 1.12 2007/10/02 16:24:50 philjmsl Exp $
 * Dump a DLS file.
 * Copyright 2005 Mobileer
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "spmidi/include/dls_parser.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_errortext.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/engine/spmidi_hybrid.h"

#if 1
/*******************************************************************/
/**
 * Use the DLS Parser to dump out the contents of a DLS file.
 * You need to define DLS_VERBOSITY as 1 in your build environment
 * before compiling in dls_parser.c to turn on the dump listing.
 */
int main(int argc, char* argv[]);
int main(int argc, char* argv[])
{
    DLSParser *dlsParser = NULL;
    char* fileName;
    unsigned char* fileStart;
    spmSInt32 fileSize;

    //#define DATADIR "E:\\nomad\\MIDISynth\\data"
#define DATADIR "C:\\business\\mobileer\\data"

    //  char *DEFAULT_FILENAME = "C:\\WINDOWS\\system32\\drivers\\gm.dls";
    //  char *DEFAULT_FILENAME = DATADIR"\\want_ya.dls";
    //  char *DEFAULT_FILENAME = DATADIR"\\dls\\TestInstruments.dls";
    //  char *DEFAULT_FILENAME = DATADIR"\\dls\\wantcha.dls";
    //  char *DEFAULT_FILENAME = DATADIR"\\dls\\multi_sine_set.dls";
    //  char *DEFAULT_FILENAME = DATADIR"\\dls\\dahdsr_series.dls";
    // char *DEFAULT_FILENAME = DATADIR"\\dls\\charge_alaw.dls";
    char *DEFAULT_FILENAME = DATADIR"\\dls\\ConditionalArticulationTest2.dls";
    int retValue;

    fileName = ( argc < 2 ) ? DEFAULT_FILENAME : argv[1];

    SPMIDI_Initialize();
    fileStart = SPMUtil_LoadFileImage( fileName, (int *)&( fileSize ) );
    if( fileStart != NULL )
    {
        retValue = DLSParser_Create( &dlsParser, fileStart, fileSize );
        if( retValue < 0 )
            goto error;

        retValue = DLSParser_Parse( dlsParser );
        if( retValue < 0 )
            goto error;
    }
    else
    {
        PRTMSG("Error: can't open file ");
        PRTMSG( fileName );
        retValue = -1;
    }

error:
    DLSParser_Delete( dlsParser );
    SPMIDI_Terminate();

    if( retValue < 0 )
        PRTMSG( SPMUtil_GetErrorText( retValue ) );
    return retValue;
}
#endif
