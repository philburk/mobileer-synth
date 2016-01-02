/* $Id: load_song.c,v 1.3 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Load a file using the SongPlayer API.
 *
 * In order for song_player support to be compiled,
 * The compiler flag SPMIDI_ME3000 must be defined as (1).
 *
 * Based on minimal_player.c MIDI file player example.
 *
 * Author: Phil Burk, Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/song_player.h"
#include "spmidi/examples/midifile_names.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)

#define SHOW_SIZE(msg,size) {printf( msg "%d\n", (size) );}

/****************************************************************/
static void usage( void )
{
    printf("play_song [-nNUMREPS] fileName\n");
    fflush(stdout);
}

#ifndef DEFAULT_FILENAME
#define DEFAULT_FILENAME  ("carumba.mxmf")
#endif

#if 1
/*******************************************************************/
int main(int argc, char **argv);
int main(int argc, char **argv)
{
    int             result;
    SPMIDI_Context *spmidiContext = NULL;
    SongPlayer     *songPlayerContext = NULL;
    unsigned char  *fileStart;
    int             fileSize;
    int             numLoops = 1;
    int             i;

    char *fileName = DEFAULT_FILENAME;

    /* Parse command line. */
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];
        if( s[0] == '-' )
        {
            switch( s[1] )
            {
            case 'n':
                numLoops = atoi( &s[2] );
                break;
            case 'h':
            case '?':
                usage();
                return 0;
            }
        }
        else
        {
            fileName = argv[i];
        }
    }
    
    PRTMSG( "play_song: play " ); PRTMSG( fileName ); PRTMSG( "\n" );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( fileName, &( fileSize ) );
    if( fileStart == NULL )
    {
        printf("ERROR: file %s not found.\n", fileName );
        return 1;
    }

    SHOW_SIZE( "file size = ", fileSize );

    SPMIDI_Initialize();
    SHOW_SIZE("after SPMIDI_Initialize() = ", SPMIDI_GetMemoryBytesAllocated() );

    /* Start synthesis engine with default number of voices. */
    result = SPMIDI_CreateContext(  &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
        goto error;
    SHOW_SIZE("after SPMIDI_CreateContext() = ", SPMIDI_GetMemoryBytesAllocated() );


    /* Create a player for the song */
    result = SongPlayer_Create( &songPlayerContext, spmidiContext, fileStart, fileSize );
    if( result < 0 )
        goto error;
    SHOW_SIZE("after SongPlayer_Create() = ", SPMIDI_GetMemoryBytesAllocated() );


    PRTMSG("File playback complete.\n");

error:
    /* Clean everything up */
    if( songPlayerContext != NULL )
        SongPlayer_Delete( songPlayerContext );
        
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext(spmidiContext);

    SHOW_SIZE("before SPMIDI_Terminate() = ", SPMIDI_GetMemoryBytesAllocated() );
    if( SPMIDI_GetMemoryBytesAllocated() > 0 )
    {
        PRTMSGNUMD("ERROR - memory not freed!!!!!!!!!!! bytes = ", SPMIDI_GetMemoryBytesAllocated() );
    }

    SPMIDI_Terminate();

    SPMUtil_FreeFileImage( fileStart );

    if( result < 0 )
    {
        PRTMSG("Error playing file = ");
        PRTNUMD( result );
        PRTMSG( SPMUtil_GetErrorText( (SPMIDI_Error)result ) );
        PRTMSG("\n");
    }

    return (result < 0);
}

#endif
