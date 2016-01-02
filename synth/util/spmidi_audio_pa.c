/* $Id: spmidi_audio_pa.c,v 1.8 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * Platform specific audio functions used by the Mobileer examples.
 * Please replace this PortAudio version with an equivalent
 * for your audio device.
 *
 * This code uses the PABLIO facility of PortAudio.
 * PABLIO provides a blocking write interface to the audio driver.
 *
 * Copyright 2002 Mobileer, Phil Burk, PROPRIETARY and CONFIDENTIAL
 */

#include "pablio.h"
#include "spmidi/include/spmidi_audio.h"

/* This section is used to play SPMIDI in a foreground thread
 * using the PABLIO blocking I/O interface.
 */

/****************************************************************/
/* Open a 16 bit audio stream using PABLIO. */
int SPMUtil_StartAudio( SPMIDI_AudioDevice *devicePtr, int sampleRate, int samplesPerFrame )
{
    /* Open simplified blocking I/O layer on top of PortAudio. */
    return (int) OpenAudioStream( (PABLIO_Stream  **)devicePtr, sampleRate, paInt16,
                                  (PABLIO_WRITE | ((samplesPerFrame == 2) ? PABLIO_STEREO : PABLIO_MONO)) );
}

/****************************************************************/
int SPMUtil_WriteAudioBuffer( SPMIDI_AudioDevice device, short *audioSamples, int numFrames )
{
    return WriteAudioStream( (PABLIO_Stream  *) device, audioSamples, numFrames );
}

/****************************************************************/
int SPMUtil_StopAudio( SPMIDI_AudioDevice device )
{
    CloseAudioStream( (PABLIO_Stream  *) device );
    return 0;
}
