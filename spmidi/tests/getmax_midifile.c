/* $Id: getmax_midifile.c,v 1.6 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Get MIDI File amplitudes.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midifile_player.h"

#include "spmidi/examples/midifile_names.h"

#define PRINT(x)   { printf x; }

#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)

/****************************************************************
 * Estimate a MIDIFIle amplitude and print in CSV format.
 */
int MIDIFile_Estimate( unsigned char *image, int numBytes, const char *fileName )
{
    int result;
    int maxAmplitude = 0;
    int testVolume = 0x040;
    MIDIFilePlayer *player;

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
        goto error1;

    /* Measure maximum amplitude.
     */
    maxAmplitude = SPMUtil_GetMaxAmplitude( player, SAMPLES_PER_FRAME,
                                            testVolume, (int) SAMPLE_RATE  );
    if( maxAmplitude < 0 )
    {
        result = maxAmplitude;
        goto error2;
    }
    PRINT(("%s,%d", fileName, maxAmplitude ));
    MIDIFilePlayer_Rewind( player );

    /* Estimate maximum amplitude for comparison.
     */
    maxAmplitude = SPMUtil_EstimateMaxAmplitude( player, SAMPLES_PER_FRAME,
                   testVolume, (int) SAMPLE_RATE );
    if( maxAmplitude < 0 )
    {
        result = maxAmplitude;
        goto error2;
    }
    PRINT((",%d\n", maxAmplitude ));

error2:
    MIDIFilePlayer_Delete( player );

error1:
    return maxAmplitude;
}


/****************************************************************/
int estimate_midi_file( const char *inputFileName )
{
    void *data = NULL;
    int  fileSize;
    int   result = -1;


    /* Load MIDI File into a memory image. */
    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        PRINT(("ERROR reading MIDI File.\n" ));
        goto error;
    }
    result = MIDIFile_Estimate( data, fileSize, inputFileName );

    if( result < 0 )
    {
        PRINT(("Error playing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) ));
        goto error;
    }

error:
    if( data != NULL )
        SPMUtil_FreeFileImage( data );

    return result;
}

/****************************************************************/
void usage( void )
{
    PRINT(("estimate_midifiles song1.mid ...\n"));
}

/****************************************************************/
int main( int argc, char ** argv )
{
    char *inputFileName = DEFAULT_FILENAME;
    int result;
    usage();
    result = estimate_midi_file( inputFileName );
    (void) argc;
    (void) argv;
    return result;
}

