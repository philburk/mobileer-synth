/* $Id: estimate_midifiles.c,v 1.6 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Estimate multiple MIDI Files amplitudes and generate CSV file.
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

#ifdef WIN32
#include <windows.h>
#endif

#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)

/****************************************************************
 * Estimate a MIDIFIle amplitude and print in CSV format.
 */
int MIDIFile_Estimate( unsigned char *image, int numBytes, const char *fileName )
{
    int result;
    int maxAmplitude;
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
    printf("%s,%d", fileName, maxAmplitude );
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
    printf(",%d\n", maxAmplitude );

error2:
    MIDIFilePlayer_Delete( player );

error1:
    return result;
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
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    result = MIDIFile_Estimate( data, fileSize, inputFileName );
    if( result < 0 )
    {
        printf("Error playing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) );
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
    printf("estimate_midifiles songname.mid ...\n");
}

/****************************************************************/
int main( int argc, char ** argv )
{
    int   i;
    //  const char *s = NULL;
    const char *s = DEFAULT_FILENAME;

    if( s != NULL )
        estimate_midi_file( s );

    /* Play files. */
    for( i=1; i<argc; i++ )
    {
        s = argv[i];
        if( s[0] != '-' )
        {
            estimate_midi_file( s );
        }
    }
}

