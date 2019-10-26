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

#ifndef ANDROIDME2000_ME2000MIDISYNTH_H
#define ANDROIDME2000_ME2000MIDISYNTH_H


#include "spmidi/include/spmidi.h"

#include "IMidiSynthBase.h"

/**
 * Implementation of MIDI that uses the Mobileer ME2000.
 */
class ME2000MidiSynth  : public IMidiSynthBase {
public:

    ME2000MidiSynth();
    virtual ~ME2000MidiSynth();

    virtual int32_t open(int32_t sampleRate);

    virtual int32_t close();

    virtual int32_t getFramesPerBlock();

    virtual int32_t getSamplesPerFrame();

    virtual int32_t writeMidi(uint8_t *data, int32_t numBytes);

    virtual int32_t readShortPCM(int16_t *pcm, int32_t numFrames);

private:
    SPMIDI_Context *spmidiContext;    // synthesizer context
};


#endif //ANDROIDME2000_ME2000MIDISYNTH_H
