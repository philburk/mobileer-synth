/* $Id: spmidi_play.c,v 1.21 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Utility functions for playing notes, etc.
 * These functions are used to simplify the example programs.
 * Their use is not required by any application.
 * So this code should be considered as example code.
 *
 * This file currently has dependencies on PortAudio,
 * a portable audio I/O library.
 * 
 * For more information, or the latest code, please visit the
 * <a href="http://www.portaudio.com/">PortAudio website</a>.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/write_wav.h"
#include "spmidi/include/midifile_player.h"


/*****************************************************************/
/********** Synchronous Interface *******************************/
/*****************************************************************/

/* This section is used to play SPMIDI in a foreground thread
 * using the PABLIO blocking I/O interface.
 */

/* This static data makes SPMUtil_Start(),
 * SPMUtil_PlayBuffers() and SPMUtil_Stop() only useable by one thread.
 */
static SPMIDI_AudioDevice  sHostAudioDevice; /* PortAudio blocking read/write stream. */
static WAV_Writer *sRenderedFile;
static int   sSamplesPerFrame;

/* Use a static array instead of a dynamic array to reduce stack requirements. */
#define SAMPLES_PER_BUFFER  (SPMIDI_MAX_SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
static short sSampleBuffer[SAMPLES_PER_BUFFER];

/****************************************************************/
int SPMUtil_StartVirtualAudio( int sampleRate, const char *fileName, int samplesPerFrame )
{
    int err;

    sSamplesPerFrame = samplesPerFrame;

    if( fileName != NULL )
    {
        sHostAudioDevice = NULL;
        /* Open file for writing raw sample data. */
        err = Audio_WAV_CreateWriter( &sRenderedFile, fileName );
        if( err < 0 )
        {
            printf("Can't open output file %s\n", fileName );
            return err;
        }

        err =  Audio_WAV_OpenWriter( sRenderedFile, sampleRate, samplesPerFrame );
        if( err < 0 )
        {
            Audio_WAV_DeleteWriter( sRenderedFile );
            return err;
        }
        return 0;
    }
    else
    {
        sRenderedFile = NULL;
        /* Open simplified blocking I/O layer on top of PortAudio. */
        return (int) SPMUtil_StartAudio( &sHostAudioDevice, sampleRate, samplesPerFrame );
    }
}


/****************************************************************/
int SPMUtil_Start( SPMIDI_Context **contextPtr, int sampleRate, const char *fileName, int samplesPerFrame )
{
    int err;

    /* Start synthesis engine with default number of voices. */
    err = SPMIDI_CreateContext( contextPtr, sampleRate );
    if( err < 0 )
        return err;

    return SPMUtil_StartVirtualAudio( sampleRate, fileName, samplesPerFrame );
}

/****************************************************************/
int SPMUtil_WriteVirtualAudio( short *samples, int samplesPerFrame, int numFrames )
{
    if( sRenderedFile != NULL )
    {
        return Audio_WAV_WriteShorts( sRenderedFile, samples,
                               samplesPerFrame * numFrames );
    }
    else if( sHostAudioDevice != NULL )
    {
        /* Write samples to the audio device. */
        return SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, numFrames );
    }
    return 0;
}

/****************************************************************/
int SPMUtil_PlayBuffers( SPMIDI_Context *spmidiContext, int numBuffers )
{
    int i;
    int result = 0;

    for( i=0; i<numBuffers; i++ )
    {
        /* Synthesize samples and fill buffer. */
        result = SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_MAX_FRAMES_PER_BUFFER, sSamplesPerFrame, 16 );
        if( result < 0 )
        {
            return result;
        }
        SPMUtil_WriteVirtualAudio( sSampleBuffer, sSamplesPerFrame, SPMIDI_MAX_FRAMES_PER_BUFFER );
    }
    return result;
}

/****************************************************************/
int SPMUtil_StopVirtualAudio( void )
{
    if( sRenderedFile != NULL )
    {
        Audio_WAV_CloseWriter( sRenderedFile );
        Audio_WAV_DeleteWriter( sRenderedFile );
        sRenderedFile = NULL;
    }

    if( sHostAudioDevice != NULL )
        SPMUtil_StopAudio( sHostAudioDevice );

    return 0;
}


/****************************************************************/
int SPMUtil_Stop( SPMIDI_Context *spmidiContext )
{
    SPMUtil_StopVirtualAudio();

    SPMIDI_DeleteContext(spmidiContext);

    return 0;
}

/****************************************************************/
/* Generate one buffers worth of data and write it to the output stream. */
int SPMUtil_PlayFileBuffer( MIDIFilePlayer *player, SPMIDI_Context *spmidiContext )
{
    int result = MIDIFilePlayer_PlayFrames( player, spmidiContext, SPMIDI_MAX_FRAMES_PER_BUFFER );
    SPMUtil_PlayBuffers( spmidiContext, 1 );
    return result;
}

/****************************************************************/
int SPMUtil_PlayMilliseconds( SPMIDI_Context *spmidiContext, int msec )
{
    int result = 0;
    int numBuffers = msec *  SPMIDI_GetSampleRate(spmidiContext) / (1000 * SPMIDI_MAX_FRAMES_PER_BUFFER);
    if( numBuffers < 1 )
        numBuffers = 1;
    result = SPMUtil_PlayBuffers( spmidiContext, numBuffers );
    if( result < 0 )
    {
        return result;
    }
    return numBuffers;
}
