/* $Id: playmf_me3000.c,v 1.4 2007/10/02 17:25:46 philjmsl Exp $ */
/**
 *
 * Play a MIDI File or an MXMF file using the ME3000 Synthesizer.
 * Input filename is passed on the command line.
 * Playback can be controlled using various key commands.
 * Status can be dumped by hitting 's'.
 * Demonstrates track enable and other features.
 * 
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "portaudio.h"

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/include/song_player.h"
#include "spmidi/examples/midifile_names.h"

/* Set velocity based EQ which attenuates low notes if < 64. */
/* #define VEQ_GAIN_AT_ZERO   (0) */
#define VEQ_GAIN_AT_ZERO   (32)

#define MAX_VOICES          (SPMIDI_MAX_VOICES)
#define SAMPLES_PER_FRAME   (2)
#define SAMPLE_RATE         (22050)
#define MASTER_VOLUME       (1 * SPMIDI_DEFAULT_MASTER_VOLUME)
#define COMPRESSOR_ONOFF    (1)
#define FRAMES_PER_BUFFER   SPMIDI_MAX_FRAMES_PER_BUFFER

//#define SOLO_TRACK          (1)

/* Use these macros so that we don't block in getchar(). */
#ifdef WIN32
#define GETCH        _getch
#define KBHIT        _kbhit
#define PUTCH(c)     _putch((char)(c))
#else
#error You need to define these keyboard macros (GETCH,KBHIT PUTCH) for any non-Windows platform.
#endif

static SPMIDI_AudioDevice  sHostAudioDevice;

/****************************************************************/
static void usage( void )
{
    printf("playmf [-rRATE] [-mMAXVOICES]\n");
    printf("       [-nNUMREPS] [-cCHANNELS] [-vRHYTHMVOLUME] songFileName\n");
    printf("Hit keys to control playback:\n");
    printf(" 0-9 = enter index\n");
    printf(" c = Show number of voices on each Channel\n");
    printf(" d = Disable indexed channel\n");
    printf(" D = Disable all channels\n");
    printf(" e = Enable indexed channel\n");
    printf(" E = Enable all channels\n");
    printf(" g = Go to indexed frame position.\n");
    printf(" m = set Max voices to index\n");
    printf(" p = Pause\n");
    printf(" r = Resume\n");
    printf(" s = print Status of instruments and notes playing\n");
    printf(" t = disable indexed Track\n");
    printf(" T = enable indexed Track\n");
    printf(" v = set rhythm volume to index\n");
    printf(" ? = print this message\n");
    printf(" q = quit\n");
    fflush(stdout);
}

/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyTextCallback( int trackIndex, int metaEventType,
                     const char *addr, int numChars, void *userData )
{
    int i;
    (void)userData;
    printf("MetaEvent type %d on track %d: ", metaEventType, trackIndex );
    for( i=0; i<numChars; i++ )
        printf("%c", addr[i] );
    printf("\n");
}

/****************************************************************/
static void ShowTrackStatus( MIDIFilePlayer *player )
{
    int i;
    int numTracks = MIDIFilePlayer_GetTrackCount( player );
    printf("\n");
    for( i=0; i<numTracks; i++ )
    {
        int c = (MIDIFilePlayer_GetTrackEnable( player, i ) > 0) ? '+' : '-';
        printf("%2d%c,", i, c );
    }
    printf("\n");
    fflush(stdout);
}

/****************************************************************/
static void ShowChannelEnableStatus( SPMIDI_Context *spmidiContext )
{
    int i;
    printf("\n");
    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        int c = (SPMIDI_GetChannelEnable( spmidiContext, i ) > 0) ? '+' : '-';
        printf("%2d%c,", i, c );
    }
    printf("\n");
    fflush(stdout);
}

/****************************************************************/
static void ShowChannelNoteCounts( SPMIDI_Context *spmidiContext )
{
    int i;
    printf("\n");
    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        int numNotes = SPMIDI_GetChannelActiveNoteCount( spmidiContext, i );
        if( numNotes > 0 )
            printf("%d:%d, ", i, numNotes );
    }
    printf("\n");
    fflush(stdout);
}

/****************************************************************/
static void EnableOrDisableAllChannels( SPMIDI_Context *spmidiContext, int onOrOff )
{
    int i;
    for( i=0; i<MIDI_NUM_CHANNELS; i++ )
    {
        SPMIDI_SetChannelEnable( spmidiContext, i, onOrOff );
    }
}


/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then play that audio using the audio device.
 */
static void PlayAudioBuffer(SPMIDI_Context *spmidiContext, int numChannels )
{   
    /* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
    short samples[SAMPLES_PER_BUFFER];

    /* Generate a buffer full of audio data as 16 bit samples. */
    SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
                       numChannels, 16 );
    
    /* Write audio samples to the audio device. */
    SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, FRAMES_PER_BUFFER );
}


/****************************************************************/
static int PlaySong( unsigned char *image, int numBytes,
                   int sampleRate, int maxVoices, int numLoops, int numChannels,
                   int rhythmVolume )
{
    int result;
    int go = 1;
    int quit = 0;
    int paused = 0;
    int timeout;
    SPMIDI_Context *spmidiContext = NULL;
    SongPlayer *songPlayer = NULL;
    MIDIFilePlayer smfPlayer = NULL;

    int value = 0;
    int oldValue, newValue;

    SPMIDI_Initialize();

    /* Start synthesis engine with default number of voices. */
    result = SPMIDI_CreateContext(  &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
    {
        goto error;
    }
    
    /* Create a player for the song */
    result = SongPlayer_Create( &songPlayer, spmidiContext, image, numBytes );
    if( result < 0 )
    {
        goto error;
    }

    smfPlayer = SongPlayer_GetMIDIFilePlayer( songPlayer );
    /*
     * Set function to be called when a text MetaEvent is
     * encountered while playing. Includes Lyric events.
     */
    MIDIFilePlayer_SetTextCallback( smfPlayer, MyTextCallback, NULL );

    result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, numChannels );
    if( result < 0 )
    {
        goto error;
    }
    
    result = SongPlayer_Start( songPlayer );
    if( result < 0 )
    {
        goto error;
    }

#ifdef SOLO_TRACK

    EnableOrDisableAllTracks( player, 0 ); // disable all
    result = MIDIFilePlayer_SetTrackEnable( player, SOLO_TRACK, 1 );
    if( result < 0 )
        goto error;
#endif

    result = SPMIDI_SetMaxVoices( spmidiContext, maxVoices );
    if( result < 0 )
        goto error;

    SPMIDI_SetMasterVolume( spmidiContext, MASTER_VOLUME );

    /* Turn compressor on or off and report final setting. */
    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, COMPRESSOR_ONOFF );
    if( result < 0 )
        goto error;

    result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, &newValue );
    if( result < 0 )
        goto error;
    printf("SPMIDI_PARAM_COMPRESSOR_ON: %d\n", newValue );

    /* Set velocity equalization parameters to reduce contribution from low notes.
     * On a mobile phone you can't hear very low notes but they
     * are in the mix and compete with higher notes.
     */
    result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_BASS_CUTOFF, &oldValue );
    if( result < 0 )
        goto error;

    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_VEQ_BASS_CUTOFF, 60 );
    if( result < 0 )
        goto error;

    result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_BASS_CUTOFF, &newValue );
    if( result < 0 )
        goto error;
    printf("VEQ_BASS_CUTOFF: %d => %d\n", oldValue, newValue );

    result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_GAIN_AT_ZERO, &oldValue );
    if( result < 0 )
        goto error;

    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_VEQ_GAIN_AT_ZERO, VEQ_GAIN_AT_ZERO );
    if( result < 0 )
        goto error;

    result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_GAIN_AT_ZERO, &newValue );
    if( result < 0 )
        goto error;
    printf("VEQ_GAIN_AT_ZERO: %d => %d\n", oldValue, newValue );

    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, rhythmVolume );
    if( result < 0 )
    {
        printf("\nERROR: Illegal value for Rhythm Volume = %d\n", rhythmVolume );
    }
    else
    {
        printf("SPMIDI_PARAM_RHYTHM_VOLUME set to %d\n", rhythmVolume );
    }

    usage();

    /* Accumulate numberic digit as track or frame number in value. */
    value = 0;
    while( go )
    {
        if( paused )
        {
            Pa_Sleep( 100 ); /* So we don't hog the CPU completely. */
        }
        else
        {
            /*
             * Play until we hit the end of all tracks.
             * Tell the MIDI player to move forward on all tracks
             * by a time equivalent to a buffers worth of frames.
             */
            
            if( SongPlayer_PlayFrames( songPlayer, FRAMES_PER_BUFFER ) == 0 )
            {
                
                PlayAudioBuffer(spmidiContext, numChannels );
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
                    SongPlayer_Rewind( songPlayer );
                }
            }
        }

        if( KBHIT() ) /* Has a key been hit? */
        {
            int c = GETCH();
            PUTCH(c);
            /* Parse key input. */
            if( (c >= '0') && (c <= '9') )
            {
                value = (value * 10) + (c - '0');
            }
            else if( c == 't' )
            {
                /* Disable track. */
                MIDIFilePlayer_SetTrackEnable( smfPlayer, value, 0 );
                ShowTrackStatus( smfPlayer );
                value = 0;
            }
            else if( c == 'T' )
            {
                /* Enable track. */
                MIDIFilePlayer_SetTrackEnable( smfPlayer, value, 1 );
                ShowTrackStatus( smfPlayer );
                value = 0;
            }
            else if( c == 'd' )
            {
                /* Disable channel. */
                SPMIDI_SetChannelEnable( spmidiContext, value, 0 );
                ShowChannelEnableStatus( spmidiContext );
                value = 0;
            }
            else if( (c == 'D') || (c == 'a') )
            {
                EnableOrDisableAllChannels( spmidiContext, 0 ); // disable all
                ShowChannelEnableStatus( spmidiContext );
            }
            else if( c == 'e' )
            {
                /* Enable channel. */
                SPMIDI_SetChannelEnable( spmidiContext, value, 1 );
                ShowChannelEnableStatus( spmidiContext );
                value = 0;
            }
            else if( (c == 'E') || (c == 'A') )
            {
                EnableOrDisableAllChannels( spmidiContext, 1 ); // enable all
                ShowChannelEnableStatus( spmidiContext );
            }
            else if( c == 'g' )
            {
                /* Seek to specified frame value. */
                MIDIFilePlayer_GoToFrame( smfPlayer, spmidiContext, value );
                value = 0;
            }
            else if( c == 'c' )
            {
                ShowChannelNoteCounts(spmidiContext);
                value = 0;
            }
            else if( c == 's' )
            {
                SPMIDI_PrintStatus(spmidiContext);
                value = 0;
            }
            else if( c == 'm' )
            {
                SPMIDI_SetMaxVoices( spmidiContext, value );
                value = 0;
                printf("\nMax voices = %d\n", SPMIDI_GetMaxVoices(spmidiContext) );
            }
            else if( c == 'v' )
            {
                int err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, value );
                if( err < 0 )
                {
                    printf("\nERROR: Illegal value for Rhythm Volume = %d\n", value );
                }
                SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, &value);
                printf("\nRhythm Volume = %d\n", value );
                value = 0;
            }
            else if( c == 'p' )
            {
                paused = 1;
            }
            else if( c == 'r' )
            {
                paused = 0;
            }
            else if( c == 'q' )
            {
                quit = 1;
                break;
            }
            else
            {
                usage();
            }

            if( value == 0 )
            {
                int centiseconds = MIDIFilePlayer_GetFrameTime( smfPlayer ) * 100 / sampleRate;
                int fraction = centiseconds % 100;
                int seconds = centiseconds / 100;
                int minutes = seconds / 60;
                seconds -= minutes * 60;

                printf("\nt=%d:%d.%d, fr=%d, mtks=%d > ",
                       minutes, seconds, fraction,
                       MIDIFilePlayer_GetFrameTime( smfPlayer ),
                       MIDIFilePlayer_GetTickTime( smfPlayer )
                      );
                fflush(stdout);
            }
        }
    }

    printf("\nPlayback complete.\n");
    fflush(stdout);

    if( quit == 0 )
    {
        timeout = (sampleRate * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
        while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
        {
            PlayAudioBuffer( spmidiContext, numChannels );
        }
    }
    /* Play a few extra buffers to make sure we get last tail of sound. */
    PlayAudioBuffer( spmidiContext, numChannels );
    PlayAudioBuffer( spmidiContext, numChannels );
    PlayAudioBuffer( spmidiContext, numChannels );
    PlayAudioBuffer( spmidiContext, numChannels );

    SPMIDI_PrintStatus(spmidiContext);
error:
    /* Clean everything up */
    if( songPlayer != NULL )
        SongPlayer_Delete( songPlayer );
        
    if( sHostAudioDevice != NULL )
        SPMUtil_StopAudio( sHostAudioDevice );
        
    if( spmidiContext != NULL )
        SPMIDI_DeleteContext(spmidiContext);

    SPMIDI_Terminate();

    return result;
}

/****************************************************************/
int main( int argc, char ** argv )
{
    int   i;
    void *data = NULL;
    int   fileSize;
    int   result = -1;
    int   maxVoices = MAX_VOICES;
    int   numLoops = 1;
    int   numChannels = SAMPLES_PER_FRAME;
    int   rhythmVolume = SPMIDI_DEFAULT_MASTER_VOLUME;
    int   sampleRate = SAMPLE_RATE;
    char *inputFileName = DEFAULT_FILENAME;

    printf("(C) 2004 Mobileer, Inc. All Rights Reserved\n");

    /* Parse command line. */
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];
        if( s[0] == '-' )
        {
            switch( s[1] )
            {
            case 'r':
                sampleRate = atoi( &s[2] );
                break;
            case 'c':
                numChannels = atoi( &s[2] );
                break;
            case 'm':
                maxVoices = atoi( &s[2] );
                break;
            case 'n':
                numLoops = atoi( &s[2] );
                break;
            case 'v':
                rhythmVolume = atoi( &s[2] );
                break;
            case 'h':
            case '?':
                usage();
                break;
            }
        }
        else
        {
            inputFileName = argv[i];
        }
    }

    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        printf("ERROR could not find MIDI File %s\n", inputFileName );
        goto error;
    }

    if( inputFileName != NULL )
        printf("Input MIDI File: %s\n", inputFileName );

    printf("Sample Rate: %d\n", sampleRate );

    result = PlaySong( data, fileSize,
                       sampleRate, maxVoices,
                       numLoops, numChannels, rhythmVolume );
    if( result < 0 )
    {
        printf("Error playing MIDI File = %d = %s\n", result,
               SPMUtil_GetErrorText( result ) );
        goto error;
    }

    printf("\n");
    fflush(stdout);

error:
    if( data != NULL )
        free( data );
    return result;
}
