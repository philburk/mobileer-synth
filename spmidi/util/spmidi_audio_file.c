/* $Id: spmidi_audio_file.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * Generic audio functions used by the Mobileer examples.
 * This implementation writes the audio to a file.
 *
 * Copyright 2002 Mobileer, Phil Burk, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/write_wav.h"
#include "spmidi/include/spmidi_audio.h"

#ifndef SPMIDI_DEFAULT_WAVE_FILE
#define SPMIDI_DEFAULT_WAVE_FILE  ("spmidi_output.wav")
#endif

/* This static data makes this code only useable by one thread.
 */
static int   sSamplesPerFrame;

/****************************************************************/
/* Open a 16 bit audio WAV file */
int SPMUtil_StartAudio( SPMIDI_AudioDevice *devicePtr, int sampleRate, int samplesPerFrame )
{
    int err;
    WAV_Writer *sRenderedFile;
    /* Open file for writing raw sample data. */
    err = Audio_WAV_CreateWriter( &sRenderedFile, SPMIDI_DEFAULT_WAVE_FILE );
    if( err < 0 )
    {
        printf("Can't open output file %s\n", SPMIDI_DEFAULT_WAVE_FILE );
        return err;
    }

    err =  Audio_WAV_OpenWriter( sRenderedFile, sampleRate, samplesPerFrame );
    if( err < 0 )
    {
        Audio_WAV_DeleteWriter( sRenderedFile );
        return err;
    }
    sSamplesPerFrame = samplesPerFrame;
    *devicePtr = (SPMIDI_AudioDevice *) sRenderedFile;

    return 0;
}

/****************************************************************/
int SPMUtil_WriteAudioBuffer( SPMIDI_AudioDevice device, short *audioSamples, int numFrames )
{
    return Audio_WAV_WriteShorts( (WAV_Writer *) device, audioSamples,
                           sSamplesPerFrame * numFrames );
}

/****************************************************************/
int SPMUtil_StopAudio( SPMIDI_AudioDevice device )
{
    Audio_WAV_CloseWriter( (WAV_Writer *) device );
    Audio_WAV_DeleteWriter( (WAV_Writer *) device );
    return 0;
}
