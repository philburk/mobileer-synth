/* $Id: render_midifiles.c,v 1.4 2007/10/02 16:24:51 philjmsl Exp $ */
/**
 *
 * Render a MIDI File whose name is passed on the command line.
 * Use the Mobileer ME2000 Synthesizer.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/midifile_player.h"

#ifdef WIN32
#include <windows.h>
#endif

#define SAMPLE_RATE         (44100)
#define SAMPLES_PER_FRAME   (2)

typedef struct PlayConfig_s
{
    unsigned char *image;
    int numBytes;
    int numLoops;
    int loopMult;
    int sampleRate;
    int maxVoices;
    int drumVolume;
    int samplesPerFrame;
    char maximizeVolume;
} PlayConfig_t;

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( PlayConfig_t *config, char *outputFileName )
{
    SPMIDI_Context *spmidiContext = NULL;
    int result;
    int timeout;
    int msec;
    int seconds;
    int rem_msec;
    int maxAmplitude;
    int testVolume = 0x040;
    int masterVolume = SPMIDI_DEFAULT_MASTER_VOLUME;
    int i;
    int activeNoteSum = 0;
    int activeNoteLoops = 0;
    MIDIFilePlayer *player;

    SPMIDI_Initialize();

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, config->sampleRate,
        config->image, config->numBytes );
    if( result < 0 )
        goto error;

    msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
    seconds = msec / 1000;
    rem_msec = msec - (seconds * 1000);
    printf("Duration = %d.%03d seconds\n", seconds, rem_msec );

    if( config->maximizeVolume )
    {
        /* Estimate maximum amplitude so that we can calculate
         * optimal volume.
         */
        maxAmplitude = SPMUtil_GetMaxAmplitude( player, config->samplesPerFrame,
                                                testVolume, config->sampleRate );
        if( maxAmplitude < 0 )
            goto error;
        printf("At masterVolume %d, maxAmplitude is %d\n", testVolume, maxAmplitude );
        MIDIFilePlayer_Rewind( player );

        /* Calculate masterVolume that will maximize the signal amplitude without clipping. */
        masterVolume = (testVolume * 32767) / maxAmplitude;
        printf("New masterVolume = %d\n", masterVolume );
    }

    /*
     * Initialize SPMIDI Synthesizer.
     * Output to audio or a file.
     */
    result = SPMUtil_Start( &spmidiContext, config->sampleRate, outputFileName, config->samplesPerFrame );
    if( result < 0 )
        goto error;

    fflush(stdout); /* Needed for Mac OS X */

    if( config->maximizeVolume )
    {
        SPMIDI_SetMasterVolume( spmidiContext, masterVolume );
    }

    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, config->drumVolume );
    if( result < 0 )
    {
        printf("\nERROR: Illegal value for Rhythm Volume = %d\n", config->drumVolume );
    }

    SPMIDI_SetMaxVoices( spmidiContext, config->maxVoices );

    for( i=0; i<(config->numLoops * config->loopMult); i++ )
    {
        /*
         * Play until we hit the end of all tracks.
         * Tell the MIDI player to move forward on all tracks
         * by a time equivalent to a buffers worth of frames.
         * Generate one buffers worth of data and write it to the output stream.
         */
        while( SPMUtil_PlayFileBuffer( player, spmidiContext ) == 0 )
        {
            activeNoteSum += SPMIDI_GetActiveNoteCount(spmidiContext);
            activeNoteLoops += 1;
        }
            

        /* Rewind song. */
        SPMUtil_Reset( spmidiContext );
        MIDIFilePlayer_Rewind( player );
    }

    /* Play until sound stops or a timeout occurs. */
    timeout = (config->sampleRate * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        SPMUtil_PlayBuffers( spmidiContext, 1 );
    }

    /* If still an active note then print status to see what is stuck. */
    if(SPMIDI_GetActiveNoteCount(spmidiContext) > 0)
    {
        printf("ERROR: notes still active!\n");
        SPMIDI_PrintStatus(spmidiContext);
    }

    printf("STATS, %s, %d, %d\n", outputFileName,
        SPMIDI_GetMaxNoteCount(spmidiContext),
        (activeNoteSum / activeNoteLoops) );

    SPMUtil_Stop(spmidiContext);

    MIDIFilePlayer_Delete( player );

error:
    return result;
}


/****************************************************************/
int play_midi_file( PlayConfig_t *config, const char *inputFileName )
{
    int   result = -1;
    int   len;
    char *outputFileName;

    len = strlen( inputFileName );
    outputFileName = malloc( len + 1 );
    if( outputFileName == NULL )
        return -1;

    config->loopMult = ( strcmp( &inputFileName[len-7] , "_rt.mid" ) == 0 ) ? 2 : 1;

    /* Build output file name. */
    strcpy( outputFileName, inputFileName );
    outputFileName[ len - 3 ] = 'w';
    outputFileName[ len - 2 ] = 'a';
    outputFileName[ len - 1 ] = 'v';

    /* Load MIDI File into a memory image. */
    config->image = SPMUtil_LoadFileImage( inputFileName, &config->numBytes );
    if( config->image == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    printf("-------------------------------\n");
    printf("Input MIDI File: %s\n", inputFileName );
    printf("Output WAV File: %s\n", outputFileName );

    result = MIDIFile_Play( config, outputFileName );
    if( result < 0 )
    {
        printf("Error playing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) );
        goto error;
    }

error:
    if( config->image != NULL )
        SPMUtil_FreeFileImage( config->image );
    if( outputFileName != NULL )
        free( outputFileName );

    return result;
}

/****************************************************************/
void usage( void )
{
    printf("render_midifiles -l[numLoops] -r[sampleRate] -v[drumVolume] -m[maxVoices] -n song1.mid ...\n");
}

/****************************************************************/
int main( int argc, char ** argv )
{
    int   i;
    const char *s;
    PlayConfig_t CONFIG;
    PlayConfig_t *config = &CONFIG;

    printf("sizeof(int) = %d\n", (int)sizeof(int));
    printf("sizeof(long) = %d\n", (int)sizeof(long));
    printf("sizeof(FXP31) = %d\n", (int)sizeof(FXP31));
    config->numLoops = 1;
    config->sampleRate = SAMPLE_RATE;
    config->maxVoices = SPMIDI_MAX_VOICES;
    config->drumVolume = SPMIDI_DEFAULT_MASTER_VOLUME;
    config->maximizeVolume = 0;
    config->samplesPerFrame = SAMPLES_PER_FRAME;

    /* Parse command line switches. */
    for( i=1; i<argc; i++ )
    {
        s = argv[i];
        if( s[0] == '-' )
        {
            switch( (int) s[1] )
            {
            case 'l':
                config->numLoops = atoi( &s[2] );
                break;
            case 'c':
                config->samplesPerFrame = atoi( &s[2] );
                break;
            case 'r':
                config->sampleRate = atoi( &s[2] );
                break;
            case 'm':
                config->maxVoices = atoi( &s[2] );
                break;
            case 'v':
                config->drumVolume = atoi( &s[2] );
                break;
            case 'n':
                config->maximizeVolume = 1;
                break;
            case 'h':
            case '?':
            default:
                usage();
                exit(1);
            }
        }
    }

    /* Play files. */
    for( i=1; i<argc; i++ )
    {
        s = argv[i];
        if( s[0] != '-' )
        {
            play_midi_file( config, s );
        }
    }
}

