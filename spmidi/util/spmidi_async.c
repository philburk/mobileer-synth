/* $Id: spmidi_async.c,v 1.7 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Utility functions for playing notes Asynchronously, etc.
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

#include <stdio.h>
#if defined(WIN32) || defined(MACOSX)
#include "pablio.h"
#endif
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/write_wav.h"
#include "spmidi/include/midifile_player.h"

#if defined(WIN32) || defined(MACOSX)

/*****************************************************************/
/********** Asynchronous Interface *******************************/
/*****************************************************************/

/* This section is used to play SPMIDI in a background thread
 * using the PortAudio callback.
 */
static PortAudioStream *stream;
static RingBuffer       ringBuffer;
static unsigned char    byteBuffer[1024];

/* This routine will be called by the PortAudio engine when audio is needed.
*/
static int paSynthCallback(   void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              PaTimestamp outTime, void *userData )
{
    void  *dataPtr1;
    void  *dataPtr2;
    long   size1;
    long   size2;
    long   numBytes;
    SPMIDI_Context *spmidiContext = (SPMIDI_Context *) userData;

    (void) outTime; /* Prevent unused variable warnings. */
    (void) inputBuffer;

    /* Read any available bytes from the ringbuffer and parse them. */
    numBytes = RingBuffer_GetReadAvailable( &ringBuffer );
    if( numBytes > 0 )
    {
        int got = RingBuffer_GetReadRegions( &ringBuffer, numBytes,
                                             &dataPtr1, &size1,
                                             &dataPtr2, &size2 );
        if( size1 > 0 )
        {
            SPMIDI_Write( spmidiContext, dataPtr1, size1 );
            if( size2 > 0 )
            {
                SPMIDI_Write( spmidiContext, dataPtr2, size2 );
            }
        }
        RingBuffer_AdvanceReadIndex( &ringBuffer, got );
    }


    /* Synthesize samples and fill buffer. */
    SPMIDI_ReadFrames( spmidiContext, outputBuffer, framesPerBuffer, 2, 16 );
    return 0;
}


/****************************************************************/
int SPMUtil_StartAsync( SPMIDI_Context **contextPtr, int sampleRate )
{
    PaError err;

    /* Start synthesis engine with default number of voices. */
    err = SPMIDI_CreateContext( contextPtr, sampleRate );
    if( err < 0 )
        return err;

    RingBuffer_Init( &ringBuffer, sizeof(byteBuffer), byteBuffer );

    err = Pa_Initialize();
    if( err != paNoError )
        goto error;

    err = Pa_OpenStream(
              &stream,
              paNoDevice,/* default input device */
              0,              /* no input */
              paInt16,
              NULL,
              Pa_GetDefaultOutputDeviceID(), /* default output device */
              2,          /* stereo output */
              paInt16,
              NULL,
              sampleRate,
              SPMIDI_MAX_FRAMES_PER_BUFFER,
              0,              /* number of buffers, if zero then use default minimum */
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              paSynthCallback,
              *contextPtr );
    if( err != paNoError )
        goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError )
        goto error;

error:
    return err;
}

/****************************************************************/
int SPMUtil_StopAsync( SPMIDI_Context *context )
{
    int err;

    err = Pa_StopStream( stream );
    if( err != paNoError )
        goto error;

    err = Pa_CloseStream( stream );
    if( err != paNoError )
        goto error;

    Pa_Terminate();
    err = SPMIDI_DeleteContext(context);

error:
    return err;
}

/****************************************************************/
int SPMUtil_WriteByteAsync( int data )
{
    unsigned char byte = (unsigned char) data;
    int numWritten = RingBuffer_Write( &ringBuffer, &byte, 1 );
    return ((numWritten == 1) ? -1 : 0 );
}
#endif /* WIN32 */

