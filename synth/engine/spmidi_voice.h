#ifndef _SPMIDI_VOICE_H
#define _SPMIDI_VOICE_H
/* $Id$ */
/**
 *
 * Hybrid Synthesizer for SPMIDI Engine.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/adsr_envelope.h"
#include "spmidi/engine/oscillator.h"
#include "svfilter.h"
#include "reverb.h"
#include "spmidi/engine/spmidi_preset.h"
#include "dls_parser_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* Program description derived from preset with SW usable parameters. */
    typedef struct HybridVoice_Info_s
    {
        /** Pointer to corresponding preset, NULL when unassigned. */
        const HybridVoice_Preset_t *preset;
        /** When this is zero, the info structure is unused and can be remapped. */
        int                      numUsers;

        EnvelopeADSR_Info_t      modEnv;
        EnvelopeADSR_Info_t      mainEnv;
        EnvelopeADSR_Info_t      ampEnv;
        SVFilter_Info_t          filter;
#if SPMIDI_ME2000
        WaveSet_t               *waveSet; /* Pointer to a structure with an array of possible waveTables. */
#endif
        PitchOctave              lfoCutoffModDepth;
        PitchOctave              envPitchModDepth;
        PitchOctave              envCutoffModDepth;
        unsigned char            flags;
        unsigned char            pad2;
        unsigned char            pad3;
        unsigned char            pad4;
    }
    HybridVoice_Info_t;

    typedef struct HybridVoice_s
    {
        /* Synthesis modules */
        /* In ME1000 instruments, modOsc is used as a second audio oscillator for PM and mixing. */
        /* In ME2000 instruments, modOsc is not used. */
        /* In ME3000 instruments, modOsc is used as the vibrato LFO. */
        Oscillator_t          modOsc;

        Oscillator_t          mainOsc;
        /* In ME3000, lfo is used as the modulation LFO. */
        Oscillator_t          lfo;
        EnvelopeADSR_t        modEnv;
        EnvelopeADSR_t        mainEnv;
        EnvelopeADSR_t        ampEnv;
        SVFilter_t            filter;
        WaveSetRegion_t      *waveSetRegion;
        FXP31                 phaseModDepth;

        /* Point to data that defines how the instrument is played. */
        HybridVoice_Info_t   *info;
        const HybridVoice_Preset_t *preset;

#if SPMIDI_ME3000
#define vibratoLFO modOsc
        DLS_Instrument_t     *dlsInstrument;
        HybridOrchestra_t    *orchestra;
        /* Selected by matching pitch and velocity. */
        DLS_Region_t         *dlsRegion;
        short                 modLFOStartDelayCounter;
        short                 vibratoLFOStartDelayCounter;
#endif

        FXP31                 lastLeftValue;
        FXP31                 lastRightValue;
        FXP31                 leftGain;
        FXP31                 rightGain;
        FXP31                 previousMixerGain; /* For smoothing mix. */
        /** Pitch of base note in octave fraction. */
        PitchOctave           baseNotePitch;
        FXP15                 leftVelocityPan; /* Locked at NoteOn time. */
        FXP15                 rightVelocityPan;

        FXP14                 velocity14; /* 2.14 Velocity after scaling and concave transform. */

        /** Index of voice in voice array. */
        unsigned char         index;
        unsigned char         active;
        unsigned char         isOn;
        /** Index of channel, 0-15 */
        unsigned char         channel;
        unsigned char         pitch;
        unsigned char         isDrum;
    }
    HybridVoice_t;


#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_VOICE_H */
