/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IMidiSynthBase.h"
#include "ME2000MidiSynth.h"

#include "spmidi/include/spmidi.h"

ME2000MidiSynth::ME2000MidiSynth()
{
    SPMIDI_Initialize();
}

ME2000MidiSynth::~ME2000MidiSynth()
{
    SPMIDI_Terminate();
}

int32_t ME2000MidiSynth::open(int32_t sampleRate)
{
/* Start synthesis engine with default number of voices. */
    SPMIDI_CreateContext(&spmidiContext, 48000 );
}

int32_t ME2000MidiSynth::close()
{
    SPMIDI_DeleteContext(&spmidiContext);
}

int32_t ME2000MidiSynth::getFramesPerBlock()
{
    return SPMIDI_GetFramesPerBuffer();
}

#define SAMPLES_PER_FRAME 2

int32_t ME2000MidiSynth::getSamplesPerFrame()
{
    return SAMPLES_PER_FRAME;
}

int32_t ME2000MidiSynth::writeMidi(uint8_t *data, int32_t numBytes)
{
    // Write bytes individually to the parser.
    for (int i = 0; i < numBytes; i++) {
        SPMIDI_WriteByte( spmidiContext, data[i] );
    }
}

int32_t ME2000MidiSynth::readShortPCM(int16_t *pcm, int32_t numFrames)
{
    // Get 16 bit sample data from the synthesizer.
    return SPMIDI_ReadFrames( spmidiContext, pcm, numFrames, getSamplesPerFrame(), 16 );
}
