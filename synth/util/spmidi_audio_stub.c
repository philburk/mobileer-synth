/* $Id: spmidi_audio_stub.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * Stub for audio functions used by the Mobileer examples.
 * This is often used on simulators and can be used as a template
 * for a platform specific impementation.
 *
 * Copyright 2002 Mobileer, Phil Burk, PROPRIETARY and CONFIDENTIAL
 */
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_errors.h"

/****************************************************************/
/* Just keep the linker happy. */
int SPMUtil_StartAudio( SPMIDI_AudioDevice *devicePtr, int sampleRate, int samplesPerFrame )
{
    (void) devicePtr;
    (void) sampleRate;
    (void) samplesPerFrame;
    return SPMIDI_Error_Unsupported;
}

/****************************************************************/
int SPMUtil_WriteAudioBuffer( SPMIDI_AudioDevice device, short *audioSamples, int numFrames )
{
    (void) device;
    (void) audioSamples;
    (void) numFrames;
    return 0;
}

/****************************************************************/
int SPMUtil_StopAudio( SPMIDI_AudioDevice device )
{
    (void) device;
    return 0;
}
