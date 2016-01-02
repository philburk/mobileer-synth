/* $Id: play_song_patch.c,v 1.5 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Test patching functions in the DLS Parser and Player.
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
#include "spmidi/engine/spmidi_dls.h"
#include "spmidi/include/song_player.h"
#include "spmidi/examples/midifile_names.h"

/* Include internal headers so we can patch functions and
 * access internal data structures.
 */
#include "spmidi/engine/spmidi_hybrid.h"
#include "dls_parser_internal.h"
#include "xmf_parser_internal.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (2)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)

#define SHOW_SIZE(msg,size) {printf( msg "%d\n", (size) );}

/* Host independant writable audio device. */
static SPMIDI_AudioDevice  sHostAudioDevice;


/****************************************************************/
/********* Patch DLS Functions **********************************/
/****************************************************************/

/* Store copies of original function pointers and tables. */
SSDLS_LoadOrchestraProcPtr sOldLoadOrchestraProcPtr;
SS_SynthesizeVoiceDLS2ProcPtr sSynthesizeDLSProcPtr;

static DLSParser_FunctionTable_t sDLSFunctions = { 0 };
static XMFParser_FunctionTable_t sXMFFunctions = { 0 };

/* Count how many times we call the DLS synthesizer. */
int sVoiceDLS_Count = 0;

/****************************************************************/
/********* Custom Replacement Functions *************************/
/****************************************************************/

/* These functions perform a custom operation and then call the original function. */
void SS_SynthesizeVoiceDLS2_Patched( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame )
{
    sVoiceDLS_Count += 1;
    sSynthesizeDLSProcPtr( hybridSynth, voice, samplesPerFrame );
}

int SSDLS_LoadOrchestra_Patched( SoftSynth *synth, DLS_Orchestra_t *dlsOrch )
{
    PRTMSG("Patched version of SSDLS_LoadOrchestra!\n");
    return sOldLoadOrchestraProcPtr( synth, dlsOrch );
}

int ResolveRegionPatch( DLS_Wave_t **poolTable,
                                DLS_Region_t *region)
{
    PRTMSG("Patched version of ResolveRegion!\n");
    return sDLSFunctions.resolveRegion( poolTable, region );
}   

int FindNodeTypePatch( XmfParser_t *xmfParser )
{
    PRTMSG("Patched version of FindNodeType!\n");
    return sXMFFunctions.findNodeType( xmfParser );
}

/****************************************************************/
/* Patch function tables so that parser uses custom functions.
 */
void PatchDLSFunctions( void )
{
    sSynthesizeDLSProcPtr = SS_GetProc_SynthesizeVoiceDLS2();
    SS_SetProc_SynthesizeVoiceDLS2( &SS_SynthesizeVoiceDLS2_Patched );

    sOldLoadOrchestraProcPtr = SSDLS_GetProc_LoadOrchestra();
    SSDLS_SetProc_LoadOrchestra( &SSDLS_LoadOrchestra_Patched );

    /* Make a copy of old function table so we can call the original functions. */
    sDLSFunctions = *DLSParser_GetFunctionTable();

    /* Tell DLS parser to use new function. */
    DLSParser_GetFunctionTable()->resolveRegion = ResolveRegionPatch;

    /* Copy old function table. */
    sXMFFunctions = *XMFParser_GetFunctionTable();
    XMFParser_GetFunctionTable()->findNodeType = FindNodeTypePatch;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static void PlayAudioBuffer(SPMIDI_Context *spmidiContext)
{
    /* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)
    short samples[SAMPLES_PER_BUFFER];

    /* Generate a buffer full of audio data as 16 bit samples. */
    SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
                       SAMPLES_PER_FRAME, 16 );

    /* Write audio samples to the audio device. */
    SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, FRAMES_PER_BUFFER );
}


/****************************************************************/
/**
 * Play a song one audio buffer at a time.
 */
int SongPlayer_Play( SongPlayer *songPlayer, SPMIDI_Context *spmidiContext, int numLoops )
{
    int result;
    int timeout;
    int go = 1;

    /* Start the songplayer */
    result = SongPlayer_Start( songPlayer );
    if( result < 0 )
        goto error;

    /*
     * Process one buffer worth of MIDI data each time through the loop.
     */
    while ( go )
    {
        if( SongPlayer_PlayFrames( songPlayer, FRAMES_PER_BUFFER ) == 0 )
        {
            PlayAudioBuffer(spmidiContext);
        }
        else
        {
            if( --numLoops <= 0 )
            {
                go = 0;
            }
            else
            {
                /* Rewind song. */
                SPMUtil_Reset( spmidiContext );
                result = SongPlayer_Rewind( songPlayer );
                if( result < 0 )
                    goto error;
            }
        }
    }

    /*
    * Continue playing until all of the notes have finished sounding,
    * or for one second, whichever is shorter.
    */
    timeout = SPMIDI_GetSampleRate( spmidiContext ) / FRAMES_PER_BUFFER;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        PlayAudioBuffer(spmidiContext);
    }

    /* Stop playing */
    result = SongPlayer_Stop( songPlayer );

error:
    return result;
}

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

    PatchDLSFunctions();

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

    result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error;


    /* Play the file */
    result = SongPlayer_Play( songPlayerContext, spmidiContext, numLoops );
    if( result < 0 )
        goto error;

    PRTMSG("VoiceDLS call counter = ");
    PRTNUMD( sVoiceDLS_Count );
    PRTMSG("\n");

    PRTMSG("File playback complete.\n");

error:
    /* Clean everything up */
    if( songPlayerContext != NULL )
        SongPlayer_Delete( songPlayerContext );
        
    if( sHostAudioDevice != NULL )
        SPMUtil_StopAudio( sHostAudioDevice );
        
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext(spmidiContext);


    SPMIDI_Terminate();
    SHOW_SIZE("after SPMIDI_Terminate() = ", SPMIDI_GetMemoryBytesAllocated() );

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
