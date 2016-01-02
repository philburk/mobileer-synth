#ifndef _SPMIDI_HYBRID_PRESETS_H
#define _SPMIDI_HYBRID_PRESETS_H

#include "spmidi/include/spmidi_config.h"
/*
 * Preset definitions for Hybrid SP-MIDI Synthesizer
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */


#if SPMIDI_SUPPORT_LOADING
EDITABLE HybridVoice_Preset_t  gHybridSynthPresets[SS_MAX_PRESETS] =
#else
EDITABLE HybridVoice_Preset_t gHybridSynthPresets[] =
#endif
{
    { /* Claves */
        { 4, 0, 0xffff759e },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 15, 0, 15, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 1, 99, 0, 99, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x5189456, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Open Triangle */
        { 4, 0, 0x8000 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xdc84c },
        { 1, 440, 0, 529, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 29, 915, 1527, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x7348002, /* phaseModDepth */
        0x8f, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        -1, 60, 128 /* boost, center, scale */
    },
    { /* Pad 6 (metallic) */
        { 4, 0, 0x195c0 },
        { 0, 0, 0x10000 },
        { 4, 0, 0xfffcd422 },
        { 29, 496, 838, 2465, 0, 0 | 0 | 0 },
        { 0, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 112, 99, 911, 624, 0, 0 | 0 | 0 },
        { 0x67af312, 0x20000, 0 | 0 },
        0x3e27d794, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x7dc, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xffd5, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Gunshot */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xf9555 },
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0xe5555 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x68db8 },
        { 1, 87, 659, 3908, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 14, 384, 765, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 870, 0, 1258, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x24de442, 0xd5555, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x2d56eb63, /* phaseModDepth */
        0x1afb, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0xfffe, /* envPitchModDepth */
        0x11fe5, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Rock Organ */
        { 6, 0, 0x20083 },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x7cccc },
        { 10, 99, 921, 99, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 931, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x5ea9a205, /* phaseModDepth */
        0x58b, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Drawbar Organ */
        { 5, 0, 0x100c4 },
        { 7, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x70000 },
        { 10, 202, 911, 99, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 20, 264, 941, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x5ab9625c, /* phaseModDepth */
        0x2cd, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Open Hi Conga */
        { 5, 0, 0xffff759e },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xd9ccc },
        { 1, 18, 576, 282, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 23, 716, 630, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4e36ec3, 0x10000, 0 | 0 },
        0x72e3d910, /* phaseModDepth */
        0x4de, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Synth Drum */
        { 4, 0, 0xffff759e },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 202, 492, 365, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 3, 99, 911, 765, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x565ec85, 0x10000, 0 | 0 },
        0x1472e8d0, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x67d5, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Hand Clap */
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0x1084f6 },
        { 8, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xb9ccc },
        { 1, 8, 159, 66, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 0, 6, 185, 54, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 1, 87, 768, 20, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x478e127, 0x106aaa, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x7fffffff, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0xb763, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        4, 60, 128 /* boost, center, scale */
    },
    { /* Trumpet */
        { 4, 0, 0x111 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 2, 412, 896, 153, 0, 0 | 0 | 0 },
        { 10, 10, 1023, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 889, 343, 38, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x7681dcfd, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Distortion Guitar */
        { 7, 0, 0x1006d },
        { 7, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xe9ccc },
        { 9, 10000, 947, 99, 0, 0 | 0 | 0 },
        { 3, 10000, 876, 282, 0, 0 | 0 | 0 },
        { 6, 10000, 761, 106, 0, 0 | 0 | 0 },
        { 0x2c5546a, 0x10000, 0 | 0 },
        0x2a23276e, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Pedal Hi-Hat */
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0x1084f6 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xeef36 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x81555 },
        { 34, 343, 0, 2187, 0, 0 | 0 | 0 },
        { 2, 202, 838, 529, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 202, 0, 190, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4c448e4, 0xf84f6, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x1e088648, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x13a53, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        3, 60, 128 /* boost, center, scale */
    },
    { /* Percussive Organ */
        { 7, 0, 0x100c4 },
        { 5, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x70000 },
        { 3, 218, 525, 99, 0, 0 | 0 | 0 },
        { 5, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 3, 99, 931, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x5ab9625c, /* phaseModDepth */
        0x2cd, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* FX 7 (echoes) */
        { 6, 0, 0x10036 },
        { 5, 0, 0xffffffca },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x3f7f8 },
        { 23, 106, 358, 106, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 20, 127, 441, 119, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 29, 2465, 480, 1037, 0, 0 | 0 | 0 },
        { 0x583c5f8, 0x20000, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x1641, /* lfoCutoffModDepth */
        0x2be, /* envPitchModDepth */
        0x19c06, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* English Horn */
        { 4, 0, 0x111 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 3, 412, 896, 153, 0, 0 | 0 | 0 },
        { 10, 10, 1023, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 889, 365, 38, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x314e5d48, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Tinkle Bell */
        { 4, 0, 0x11742 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x64a62 },
        { 1, 69, 934, 321, 39, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 232, 902, 3499, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 69, 921, 2333, 86, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x0, 0x0, 0 | 0 },
        0xb5c99a9, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Reverse Cymbal */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x1184f6 },
        { 2, OSC_FLAG_ABSOLUTE_PITCH, 0xf1269 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x78c9b },
        { 4, 1922, 781, 39, 0, 0 | 0 | 0 },
        { 195, 99, 1023, 99, 0, 0 | 0 | 0 },
        { 1922, 58, 0, 37, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x0, 0x0, 0 | 0 },
        0x6ccba1c7, /* phaseModDepth */
        0xc5d, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Electric Piano 2 */
        { 6, 0, 0x6d },
        { 4, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 4, 343, 0, 99, 110, 0 | 0 | 0 },
        { 0, 195, 1023, 1527, 0, 0 | 0 | 0 },
        { 5, 3280, 0, 264, 91, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x2116e035, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Celesta */
        { 4, 0, 0x31111 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 2, 816, 218, 1037, 44, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 4, 39, 1023, 3664, 74, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x7a75c7a, /* phaseModDepth */
        0x147, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Sitar */
        { 5, 0, 0x1006d },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 4, 1431, 0, 62, 22, 0 | 0 | 0 },
        { 3, 10000, 941, 218, 0, 0 | 0 | 0 },
        { 2, 1819, 0, 42, 41, 0 | 0 | 0 },
        { 0x2c5546a, 0x10000, 0 | 0 },
        0x2f263dc1, /* phaseModDepth */
        0x1a8, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x5c0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Guitar Fret Noise */
        { 6, 0, 0x30000 },
        { 1, 0, 0x20000 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x95381 },
        { 106, 106, 876, 99, 98, 0 | 0 | 0 },
        { 19, 1037, 781, 5199, 0, 0 | 0 | 0 },
        { 5, 440, 653, 301, 94, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x0, 0x0, 0 | 0 },
        0x662e812f, /* phaseModDepth */
        0x3e9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x1423, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Applause */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xe2aaa },
        { 0, OSC_FLAG_ABSOLUTE_PITCH, 0xf5555 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x87786 },
        { 11, 8790, 1023, 5199, 0, 0 | 0 | 0 },
        { 99, 2333, 1023, 4742, 0, 0 | 0 | 0 },
        { 282, 3908, 883, 2465, 0, 0 | 0 | 0 },
        { 0x22a6a2b, 0xe0000, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x7fffffff, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x792, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Electric Guitar(muted) */
        { 4, 0, 0x5e },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 1, 42, 532, 981, 30, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 3, 301, 474, 529, 52, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x6de9505, 0x10000, 0 | 0 },
        0x5bb1f98b, /* phaseModDepth */
        0x137, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xf7d9, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* High Timbale */
        { 7, 0, 0xe4a1 },
        { 5, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xfef36 },
        { 1, 18, 646, 1258, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 29, 736, 2465, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 218, 492, 2465, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x20ca4ce, 0x10000, 0 | 0 },
        0x7adb0e0f, /* phaseModDepth */
        0x1fe7, /* lfoPitchModDepth */
        0x2212, /* lfoCutoffModDepth */
        0x2a98, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Choir Aahs */
        { 5, 0, 0x111 },
        { 5, 0, 0xfffffeef },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 10, 1037, 850, 99, 0, 0 | 0 | 0 },
        { 10, 282, 934, 2187, 0, 0 | 0 | 0 },
        { 7, 176, 911, 125, 0, 0 | 0 | 0 },
        { 0x39a7887, 0xe1f9a, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x581972f1, /* phaseModDepth */
        0x1c9, /* lfoPitchModDepth */
        0x4e5, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xb14, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Guitar Harmonics */
        { 4, 0, 0x20111 },
        { 0, 0, 0x20000 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 1, 13, 397, 365, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 2465, 1023, 10000, 0, 0 | 0 | 0 },
        { 1, 1527, 486, 218, 0, 0 | 0 | 0 },
        { 0x5c0d83, 0x0, 0 | 0 },
        0x1e088648, /* phaseModDepth */
        0x1a8, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x26ec3, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        -1, 60, 128 /* boost, center, scale */
    },
    { /* Cowbell */
        { 4, 0, 0x6b4e },
        { 0, 0, 0xc000 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xe9ccc },
        { 1, 202, 474, 469, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 2, 127, 454, 717, 26, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x2622472b, /* phaseModDepth */
        0x411, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Pad 5 (bowed) */
        { 6, 0, 0x20111 },
        { 6, 0, 0x111 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 15, 99, 942, 250, 0, 0 | 0 | 0 },
        { 12, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 99, 99, 911, 624, 0, 0 | 0 | 0 },
        { 0xdd279a, 0x20000, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0x1c9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6be4, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Lead 7 (fifths) */
        { 6, 0, 0x95f9 },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x558de },
        { 2, 981, 993, 153, 0, 0 | 0 | 0 },
        { 3, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 5, 99, 931, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x7fffffff, /* phaseModDepth */
        0xcb, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Pizzicato Strings */
        { 6, 0, 0x10111 },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 12, 202, 0, 12, 72, 0 | 0 | 0 },
        { 10, 10000, 0, 10000, 0, 0 | 0 | 0 },
        { 10, 282, 0, 23, 68, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x560033e7, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Side Stick */
        { 8, 0, 0xffff759e },
        { 4, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 386, 0, 624, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10, 0, 10, 0, 0 | 0 | 0 },
        { 1, 99, 0, 99, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x1472e8d0, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* FX 1 (rain) */
        { 7, 0, 0x195c0 },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x43b4f },
        { 29, 47, 204, 343, 108, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 9, 264, 883, 2333, 0, 0 | 0 | 0 },
        { 66, 5597, 0, 1258, 23, 0 | 0 | 0 },
        { 0x194227d, 0x20000, 0 | 0 },
        0x703def90, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x5c0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xe4ae, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Tremolo Strings */
        { 6, 0, 0x36 },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6c6a7 },
        { 81, 1037, 941, 3664, 0, ADSR_FLAG_LOOP_SUSTAIN | 0 | 0 },
        { 69, 870, 909, 10000, 0, ADSR_FLAG_LOOP_SUSTAIN | 0 | 0 },
        { 2, 99, 947, 87, 0, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x51b5ed57, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* FX 6 (goblins) */
        { 8, 0, 0x10111 },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x53b4f },
        { 765, 4742, 627, 765, 0, 0 | 0 | 0 },
        { 54, 1922, 722, 10000, 0, 0 | 0 | 0 },
        { 153, 1258, 793, 928, 0, 0 | 0 | 0 },
        { 0x178e0aa, 0x10000, 0 | 0 },
        0xbc70558, /* phaseModDepth */
        0xcb, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x25338, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Acoustic Guitar(nylon) */
        { 4, 0, 0x0 },
        { 1, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 3, 32, 460, 412, 41, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 1, 1922, 0, 125, 69, 0 | 0 | 0 },
        { 0x6de9505, 0x10000, 0 | 0 },
        0x95395e8, /* phaseModDepth */
        0x9e, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xf7d9, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Pad 1 (new age) */
        { 4, 0, 0xbc },
        { 1, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x43b4f },
        { 2, 43, 474, 1037, 0, 0 | 0 | 0 },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 9, 195, 883, 1527, 59, 0 | 0 | 0 },
        { 0x10a30fe, 0x20036, 0 | 0 },
        0x17e1aa50, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0x4f33, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x12a9e, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Blown Bottle */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x1084f6 },
        { 8, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x53b4f },
        { 5, 195, 0, 19, 0, 0 | 0 | 0 },
        { 10, 1527, 870, 386, 0, 0 | 0 | 0 },
        { 30, 153, 993, 315, 0, 0 | 0 | 0 },
        { 0x32799, 0x0, 0 | 0 },
        0x33160022, /* phaseModDepth */
        0x9e, /* lfoPitchModDepth */
        0x4e5, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x8e8, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Timpani */
        { 4, 0, 0xffff7831 },
        { 0, 0, 0x0 },
        { 9, 0, 0xffff7c45 },
        { 1, 54, 614, 2465, 69, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 112, 876, 2187, 89, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x565ec85, 0x10000, 0 | 0 },
        0x3a37483e, /* phaseModDepth */
        0x1a8, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x75d, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Kalimba */
        { 4, 0, 0xffff4aab },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 43, 287, 248, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 4, 23, 953, 717, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x465b270, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Pad 8 (sweep) */
        { 6, 0, 0x20111 },
        { 6, 0, 0x111 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x43b4f },
        { 1342, 1527, 364, 250, 18, ADSR_FLAG_LOOP_SUSTAIN | 0 | 0 },
        { 16, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 99, 99, 911, 624, 0, 0 | 0 | 0 },
        { 0x22817c6, 0x10000, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0x9e, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x13a53, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Mute Cuica */
        { 4, 0, 0xffff0000 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xc9ccc },
        { 7, 386, 0, 51, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 19, 264, 0, 58, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x464ca0d6, /* phaseModDepth */
        0xb9f, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x216d, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Dulcimer */
        { 6, 0, 0x101d7 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 1, 218, 781, 1342, 91, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 4, 3075, 0, 176, 102, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0xeef36, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x28ba4dae, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xb5cc, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* SynthBrass 1 */
        { 4, 0, 0x0 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 99, 386, 880, 99, 0, 0 | 0 | 0 },
        { 2, 99, 911, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 798, 99, 0, 0 | 0 | 0 },
        { 0x368a46f, 0x20000, 0 | 0 },
        0x3f0a4087, /* phaseModDepth */
        0x1c9, /* lfoPitchModDepth */
        0x436, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xe86d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Open Cuica */
        { 4, 0, 0xffff0000 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xc9ccc },
        { 7, 386, 0, 51, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 19, 264, 0, 58, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x464ca0d6, /* phaseModDepth */
        0xb9f, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x216d, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Seashore */
        { 9, 0, 0x0 },
        { 8, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x83388 },
        { 440, 3908, 486, 2465, 0, 0 | 0 | 0 },
        { 559, 1922, 858, 5597, 0, 0 | 0 | 0 },
        { 106, 3908, 0, 3499, 10, 0 | 0 | 0 },
        { 0x803ad07, 0xd555, 0 | 0 },
        0x703def90, /* phaseModDepth */
        0x2cd, /* lfoPitchModDepth */
        0x1b66, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x108d3, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Open Hi-Hat */
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0xeef36 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x3a0000 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x81555 },
        { 559, 1922, 0, 0, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 9, 386, 0, 440, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x60019c0, 0xf84f6, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0xc817eac, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0xfffe, /* envPitchModDepth */
        0x19639, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Electric Guitar(clean) */
        { 7, 0, 0xbc },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 1, 32, 934, 1922, 71, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 4, 1721, 0, 99, 66, 0 | 0 | 0 },
        { 0x6de9505, 0x10000, 0 | 0 },
        0x175bd3ea, /* phaseModDepth */
        0x147, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x1af, /* envPitchModDepth */
        0xf7d9, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Lead 2 (sawtooth) */
        { 6, 0, 0x36 },
        { 6, 0, 0xffffffca },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x752f5 },
        { 10, 99, 960, 928, 0, 0 | 0 | 0 },
        { 10, 5199, 960, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 986, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x4f4fb3c5, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Acoustic Bass */
        { 6, 0, 0x10036 },
        { 7, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x64a62 },
        { 1, 190, 563, 602, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 3664, 1023, 10000, 0, 0 | 0 | 0 },
        { 2, 559, 0, 51, 25, 0 | 0 | 0 },
        { 0x324279e, 0x0, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* FX 5 (brightness) */
        { 6, 0, 0x36 },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 39, 1258, 850, 1258, 0, 0 | 0 | 0 },
        { 12, 3908, 896, 4168, 0, 0 | 0 | 0 },
        { 24, 3499, 838, 3280, 0, 0 | 0 | 0 },
        { 0x39a7887, 0xe1f9a, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x4b839123, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0x4e5, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xb14, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* High Agogo */
        { 9, 0, 0xffff759e },
        { 1, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xec84c },
        { 1, 30, 0, 47, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 2, 153, 0, 195, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x427ec792, /* phaseModDepth */
        0xa19, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Oboe */
        { 7, 0, 0x36 },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 19, 99, 829, 99, 0, 0 | 0 | 0 },
        { 25, 10, 909, 176, 0, 0 | 0 | 0 },
        { 10, 232, 798, 248, 0, 0 | 0 | 0 },
        { 0x8f765a, 0x20000, 0 | 0 },
        0x537e8274, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0x6ca, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x271d, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Acoustic Guitar(steel) */
        { 6, 0, 0x1005e },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 3, 202, 588, 1179, 36, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 6, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 1, 1922, 0, 125, 98, 0 | 0 | 0 },
        { 0x6de9505, 0x10000, 0 | 0 },
        0x692aea9e, /* phaseModDepth */
        0x167, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xf7d9, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* French Horn */
        { 4, 0, 0x111 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 2, 412, 896, 153, 0, 0 | 0 | 0 },
        { 10, 10, 1023, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 889, 343, 38, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x5f6ad98f, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* SynthStrings 1 */
        { 6, 0, 0x111 },
        { 6, 0, 0xfffffeef },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 10, 99, 911, 99, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 911, 99, 0, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x5ab9625c, /* phaseModDepth */
        0x1c9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Xylophone */
        { 4, 0, 0x759d },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 32, 301, 176, 93, 0 | 0 | 0 },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 1, 1342, 0, 1037, 101, 0 | 0 | 0 },
        { 0x7e1480f, 0x20000, 0 | 0 },
        0x2622472b, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x10173, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Closed Hi-Hat */
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0x1084f6 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xfef36 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x81555 },
        { 58, 343, 0, 2187, 0, 0 | 0 | 0 },
        { 2, 202, 838, 529, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 15, 902, 162, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x54cb806, 0xfef36, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x25a765a3, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x1ccec, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        3, 60, 128 /* boost, center, scale */
    },
    { /* Violin */
        { 4, 0, 0x5 },
        { 2, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 5, 99, 1023, 3664, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 23, 69, 1023, 106, 17, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x4d4b99e, /* phaseModDepth */
        0x1a8, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Lead 6 (voice) */
        { 7, 0, 0x196fa },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x61b2c },
        { 10, 99, 798, 195, 0, 0 | 0 | 0 },
        { 10, 99, 952, 195, 0, 0 | 0 | 0 },
        { 10, 99, 778, 99, 0, 0 | 0 | 0 },
        { 0x1b66f9b, 0xd328c, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x692aea9e, /* phaseModDepth */
        0x1c9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Tubular Bells */
        { 4, 0, 0x1b8f3 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 2, 51, 953, 1037, 54, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 11, 43, 960, 3075, 43, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x86c2a8a, /* phaseModDepth */
        0xcb, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Reed Organ */
        { 5, 0, 0x100c4 },
        { 7, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x70000 },
        { 10, 99, 911, 99, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 931, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x5ab9625c, /* phaseModDepth */
        0x2cd, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Mute Hi Conga */
        { 8, 0, 0xffff759e },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 69, 430, 66, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 32, 0, 29, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4e36ec3, 0x10000, 0 | 0 },
        0x11b0d5fb, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Orchestral Harp */
        { 4, 0, 0x3ac },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 3, 58, 972, 2050, 31, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 6, 3280, 0, 1258, 95, 0 | 0 | 0 },
        { 0x6de9505, 0x10000, 0 | 0 },
        0x57485c6, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xf7d9, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Long Guiro */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xeef36 },
        { 8, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x3a58e },
        { 496, 496, 491, 12, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10, 153, 43, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 1, 496, 552, 10, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x39e393, 0x0, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x1e71d, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Chinese Cymbal */
        { 9, 0, 0xffff2334 },
        { 3, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xe6b7b },
        { 386, 672, 0, 816, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 3280, 0, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 870, 352, 928, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x9428867, 0xe9ccc, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x2859772f, /* phaseModDepth */
        0x1d5d, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x15491, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Church Organ */
        { 6, 0, 0x200c4 },
        { 7, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x69863 },
        { 3, 99, 921, 145, 0, 0 | 0 | 0 },
        { 13, 10000, 1023, 1527, 0, 0 | 0 | 0 },
        { 3, 99, 931, 1106, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x6495a441, /* phaseModDepth */
        0x2cd, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Woodblock */
        { 4, 0, 0xffff8a63 },
        { 0, 0, 0x10000 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 19, 64, 153, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 2465, 1023, 10000, 0, 0 | 0 | 0 },
        { 2, 248, 0, 282, 23, 0 | 0 | 0 },
        { 0x565ec85, 0x10000, 0 | 0 },
        0x1995e41e, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* String Ensembles 1 */
        { 6, 0, 0x111 },
        { 6, 0, 0xfffffeef },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 15, 99, 911, 195, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 12, 99, 911, 315, 0, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x5ab9625c, /* phaseModDepth */
        0x1c9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* FX 2 (soundtrack) */
        { 6, 0, 0x10111 },
        { 6, 0, 0x111 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x53b4f },
        { 386, 765, 931, 765, 0, 0 | 0 | 0 },
        { 15, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 99, 99, 911, 624, 0, 0 | 0 | 0 },
        { 0x1cd5b6f, 0x10000, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0x9e, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x25338, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Lead 3 (calliope) */
        { 7, 0, 0x196fa },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x61b2c },
        { 10, 99, 798, 99, 0, 0 | 0 | 0 },
        { 10, 99, 952, 99, 0, 0 | 0 | 0 },
        { 10, 99, 844, 99, 0, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x692aea9e, /* phaseModDepth */
        0x1c9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Long Whistle */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x110000 },
        { 4, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x76665 },
        { 1, 25, 706, 10, 0, 0 | 0 | 0 },
        { 10, 25, 440, 10000, 0, ADSR_FLAG_LOOP_SUSTAIN | 0 | 0 },
        { 10, 981, 860, 62, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x1a8343cd, /* phaseModDepth */
        0x476, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Hi Bongo */
        { 4, 0, 0xffff0000 },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 29, 249, 208, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 32, 471, 301, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4e36ec3, 0x10000, 0 | 0 },
        0x560033e7, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Cabasa */
        { 9, 0, 0x20000 },
        { 8, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x83388 },
        { 624, 386, 450, 1247, 0, 0 | 0 | 0 },
        { 10, 1922, 1023, 10000, 0, 0 | 0 | 0 },
        { 20, 162, 678, 264, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x422dc06, 0x1aaaa, 0 | 0 },
        0x703def90, /* phaseModDepth */
        0x2cd, /* lfoPitchModDepth */
        0x67fc, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x108d3, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Pad 3 (polysynth) */
        { 7, 0, 0x6d },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x558de },
        { 27, 1247, 1023, 1247, 0, 0 | 0 | 0 },
        { 27, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 24, 195, 931, 1527, 0, 0 | 0 | 0 },
        { 0x20ca4ce, 0x20036, 0 | 0 },
        0x5855d54b, /* phaseModDepth */
        0x137, /* lfoPitchModDepth */
        0xc4b, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x1a313, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Steel Drums */
        { 4, 0, 0x196fa },
        { 0, 0, 0x10000 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x985f2 },
        { 3, 816, 492, 248, 43, 0 | 0 | 0 },
        { 2, 440, 1023, 3075, 0, 0 | 0 | 0 },
        { 2, 1527, 0, 2050, 26, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x13a8df20, /* phaseModDepth */
        0x167, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Flute */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x114f1d },
        { 4, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x76666 },
        { 6, 315, 525, 25, 0, 0 | 0 | 0 },
        { 42, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 19, 99, 1023, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0xa277071, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Lead 1 (square) */
        { 7, 0, 0x20106 },
        { 7, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x7cccc },
        { 10, 99, 921, 99, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 10, 99, 931, 99, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x67423c2a, /* phaseModDepth */
        0x21d, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Short Guiro */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xeef36 },
        { 8, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x3a58e },
        { 99, 99, 235, 12, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 1, 10, 153, 15, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 1, 76, 552, 10, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x39e393, 0x0, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x1bc89, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Bassoon */
        { 5, 0, 0x6d },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 20, 99, 1023, 99, 0, 0 | 0 | 0 },
        { 4, 10000, 1023, 176, 0, 0 | 0 | 0 },
        { 23, 816, 1023, 42, 0, 0 | 0 | 0 },
        { 0x134256, 0xf65a1, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x35d9803e, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        -1, 60, 128 /* boost, center, scale */
    },
    { /* Voice Oohs */
        { 5, 0, 0x111 },
        { 5, 0, 0xfffffeef },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 12, 1037, 850, 99, 0, 0 | 0 | 0 },
        { 9, 282, 934, 2187, 0, 0 | 0 | 0 },
        { 42, 176, 911, 125, 0, 0 | 0 | 0 },
        { 0x462ed43, 0xd328c, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x43ce8984, /* phaseModDepth */
        0x147, /* lfoPitchModDepth */
        0x4e5, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x477, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* FX 4 (atmosphere) */
        { 6, 0, 0x36 },
        { 7, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x53b4f },
        { 24, 1037, 889, 717, 0, 0 | 0 | 0 },
        { 4, 1037, 179, 153, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 6, 99, 921, 624, 0, 0 | 0 | 0 },
        { 0x54cb806, 0x10000, 0 | 0 },
        0x67cd0e34, /* phaseModDepth */
        0x137, /* lfoPitchModDepth */
        0x55c, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x113c1, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Mute Triangle */
        { 4, 0, 0x160e6 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xdc84c },
        { 1, 1614, 0, 559, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 1, 66, 0, 34, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x1995e41e, /* phaseModDepth */
        0x34f, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Recorder */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x1141a0 },
        { 4, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 10, 99, 645, 19, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 25, 99, 1023, 99, 0, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x7a75c7a, /* phaseModDepth */
        0x9e, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Vibraslap */
        { 5, 0, 0xffff4eda },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x3a58e },
        { 1, 19, 286, 12, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 1106, 0, 529, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x2be886, 0x12aaa, 0 | 0 },
        0x6ecb5820, /* phaseModDepth */
        0x1256, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x2b3ff, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        -1, 60, 128 /* boost, center, scale */
    },
    { /* Agogo */
        { 7, 0, 0x1e334 },
        { 1, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xec84c },
        { 1, 119, 0, 76, 63, 0 | 0 | 0 },
        { 1, 1431, 832, 928, 0, 0 | 0 | 0 },
        { 2, 928, 0, 765, 65, 0 | 0 | 0 },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x86c2a8a, /* phaseModDepth */
        0x2cd, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x157, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Bird Tweet */
        { 6, 0, 0x0 },
        { 4, 0, 0x20000 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x81839 },
        { 23, 66, 230, 23, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 20, 27, 266, 29, 0, 0 | ADSR_FLAG_LOOP_RELEASE | ADSR_FLAG_NO_WAIT },
        { 1, 232, 1023, 69, 0, 0 | 0 | 0 },
        { 0x55ca19, 0x20000, 0 | 0 },
        0x0, /* phaseModDepth */
        0x3cce, /* lfoPitchModDepth */
        0xb290, /* lfoCutoffModDepth */
        0x20b8, /* envPitchModDepth */
        0xee83, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Melodic Tom */
        { 4, 0, 0xffff7831 },
        { 0, 0, 0x0 },
        { 9, 0, 0xffff7c45 },
        { 1, 136, 179, 301, 69, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 112, 876, 2333, 89, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x565ec85, 0x10000, 0 | 0 },
        0x2d56eb63, /* phaseModDepth */
        0x1a8, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0xe5c, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Overdriven Guitar */
        { 7, 0, 0x6d },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 4, 5597, 934, 99, 0, 0 | 0 | 0 },
        { 3, 10000, 972, 218, 0, 0 | 0 | 0 },
        { 2, 10000, 966, 99, 0, 0 | 0 | 0 },
        { 0x2c5546a, 0x10000, 0 | 0 },
        0x67423c2a, /* phaseModDepth */
        0x1a8, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Synth Voice */
        { 6, 0, 0x111 },
        { 6, 0, 0xfffffeef },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 25, 765, 675, 153, 0, 0 | 0 | 0 },
        { 15, 99, 890, 386, 0, 0 | 0 | 0 },
        { 19, 99, 911, 125, 0, 0 | 0 | 0 },
        { 0x194227d, 0x10000, 0 | 0 },
        0x5ab9625c, /* phaseModDepth */
        0x3c2, /* lfoPitchModDepth */
        0x68c, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x251a1, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Lead 4 (chiff) */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x10ef36 },
        { 7, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 15, 386, 531, 47, 0, 0 | 0 | 0 },
        { 76, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 38, 99, 931, 99, 0, 0 | 0 | 0 },
        { 0x2361abf, 0x20000, 0 | 0 },
        0x20a7d5a1, /* phaseModDepth */
        0x286, /* lfoPitchModDepth */
        0x873, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x14c9f, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Taiko Drum */
        { 4, 0, 0xffff7831 },
        { 0, 0, 0x0 },
        { 9, 0, 0xffff7c45 },
        { 1, 54, 614, 2465, 69, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 112, 876, 2187, 89, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x565ec85, 0x10000, 0 | 0 },
        0x3a37483e, /* phaseModDepth */
        0x1a8, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x75d, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* FX 3 (crystal) */
        { 4, 0, 0x8000 },
        { 0, 0, 0x36 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x558de },
        { 2, 62, 141, 672, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 29, 3280, 0, 1527, 26, 0 | 0 | 0 },
        { 0x13a3a32, 0x20000, 0 | 0 },
        0xa277071, /* phaseModDepth */
        0x1ea, /* lfoPitchModDepth */
        0xda0, /* lfoCutoffModDepth */
        0x157, /* envPitchModDepth */
        0xb0bf, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Pan Flute */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x106698 },
        { 0, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x76666 },
        { 12, 365, 448, 51, 0, 0 | 0 | 0 },
        { 18, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 12, 10, 1023, 162, 0, 0 | 0 | 0 },
        { 0x1b66f9b, 0x0, 0 | 0 },
        0xa277071, /* phaseModDepth */
        0x20c, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Orchestra Hit */
        { 6, 0, 0x6a3f },
        { 2, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 92, 412, 691, 365, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 13, 1258, 774, 529, 33, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x0, 0x0, 0 | 0 },
        0x1a97c71, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Acoustic Bass Drum */
        { 4, 0, 0xffff759e },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 136, 430, 112, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 365, 761, 440, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4e36ec3, 0x10000, 0 | 0 },
        0x5d6a8e4e, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x873b, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Accordian */
        { 5, 0, 0x0 },
        { 3, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x51716 },
        { 6, 4742, 518, 2050, 0, 0 | 0 | 0 },
        { 1, 4, 799, 10000, 0, 0 | 0 | 0 },
        { 58, 162, 1023, 343, 37, 0 | 0 | 0 },
        { 0xb572c4a, 0x20000, 0 | 0 },
        0x1f106ba4, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Marimba */
        { 4, 0, 0xffff759e },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 2, 39, 115, 248, 67, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 4, 2465, 147, 440, 77, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x20000, 0 | 0 },
        0x464ca0d6, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x10173, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Low Floor Tom */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xfef36 },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 51, 403, 343, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 602, 0, 559, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4e36ec3, 0x10000, 0 | 0 },
        0x31e4d8ae, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x80c5, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Ride Cymbal 2 */
        { 8, 0, 0xfffead97 },
        { 10, 0, 0x0 },
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0xe6b7b },
        { 176, 2857, 0, 717, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 5199, 0, 4446, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 70, 838, 886, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x6e3af69, 0xfc881, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x28ba4dae, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0xfdd, /* envPitchModDepth */
        0x4d52, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Harmonica */
        { 5, 0, 0x1002f },
        { 3, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x73a60 },
        { 5, 4168, 896, 92, 0, 0 | 0 | 0 },
        { 3, 4446, 902, 10000, 0, 0 | 0 | 0 },
        { 27, 4446, 858, 81, 0, 0 | 0 | 0 },
        { 0x707bcdc, 0xfef36, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x19213cfa, /* phaseModDepth */
        0xf9, /* lfoPitchModDepth */
        0x215b, /* lfoCutoffModDepth */
        0x54, /* envPitchModDepth */
        0xc838, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Bright Acoustic */
        { 6, 0, 0x36 },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 1, 3499, 397, 99, 91, 0 | 0 | 0 },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 6, 3664, 0, 232, 101, 0 | 0 | 0 },
        { 0x7e1480f, 0xeef36, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x60f14f9c, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xb5cc, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Ride Cymbal 1 */
        { 8, 0, 0xfffead97 },
        { 9, 0, 0x0 },
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0xe6b7b },
        { 71, 2831, 0, 717, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 5199, 0, 4446, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 69, 921, 1037, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x6de9505, 0xf9d07, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x39e21458, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0xf9c, /* envPitchModDepth */
        0x4d52, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Glockenspiel */
        { 4, 0, 0xe666 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 2, 47, 941, 440, 46, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 4, 39, 947, 3664, 74, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x95395e8, /* phaseModDepth */
        0x147, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Tambourine */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xeef36 },
        { 2, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xc9ccc },
        { 1, 153, 332, 1342, 0, 0 | 0 | 0 },
        { 2, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 14, 202, 685, 34, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x692aea9e, /* phaseModDepth */
        0x3d9e, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0xe9b, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Crash Cymbal 2 */
        { 8, 0, 0xfffead97 },
        { 10, 0, 0x0 },
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0xe6b7b },
        { 264, 2831, 0, 717, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 5199, 0, 4446, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 32, 934, 1342, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x65df189, 0xf9d07, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x45e93681, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0xf9c, /* envPitchModDepth */
        0x5ecc, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Crash Cymbal 1 */
        { 8, 0, 0xfffead97 },
        { 10, 0, 0x0 },
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0xe6b7b },
        { 264, 2831, 0, 717, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 5199, 0, 4446, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 4, 32, 934, 1342, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x65df189, 0xf6bbe, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x30943351, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0xf5b, /* envPitchModDepth */
        0x8852, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Short Whistle */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x110000 },
        { 4, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x76665 },
        { 1, 19, 706, 10, 0, 0 | 0 | 0 },
        { 10, 15, 512, 10000, 0, ADSR_FLAG_LOOP_SUSTAIN | 0 | 0 },
        { 10, 529, 934, 15, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x1a8343cd, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Ride Bell */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xe6b7b },
        { 3, 0, 0x0 },
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0xe6b7b },
        { 23, 2831, 0, 717, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 5199, 0, 4446, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 5, 636, 0, 672, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x6a73783, 0xeef36, SVFILTER_FLAG_ABS_PITCH | SVFILTER_FLAG_HIGH_PASS },
        0x401d6846, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x179d, /* envPitchModDepth */
        0xc838, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Slap Bass 1 */
        { 7, 0, 0xda },
        { 1, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 3, 469, 0, 202, 0, 0 | 0 | 0 },
        { 1, 3280, 1023, 10000, 0, 0 | 0 | 0 },
        { 7, 786, 0, 112, 0, 0 | 0 | 0 },
        { 0x324279e, 0x0, 0 | 0 },
        0x179e83f2, /* phaseModDepth */
        0x22e, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x1db, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Harpsichord */
        { 6, 0, 0x20000 },
        { 6, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 2, 99, 825, 816, 80, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 3, 321, 825, 717, 84, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x7fffffff, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Acoustic Snare */
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0x1084f6 },
        { 4, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x81555 },
        { 1, 81, 557, 1527, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 672, 0, 1179, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 469, 0, 630, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4e36ec3, 0x10000, 0 | 0 },
        0x3eaf600a, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Pad 2 (warm) */
        { 5, 0, 0x6d },
        { 6, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x558de },
        { 162, 1247, 1023, 1247, 0, 0 | 0 | 0 },
        { 27, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 99, 195, 931, 1527, 0, 0 | 0 | 0 },
        { 0x14887c8, 0x20036, 0 | 0 },
        0x7fffffff, /* phaseModDepth */
        0x137, /* lfoPitchModDepth */
        0x1866, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xd12f, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Acoustic Grand */
        { 6, 0, 0x19 },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x5f7f8 },
        { 2, 2630, 0, 321, 96, 0 | 0 | 0 },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 6, 3664, 0, 264, 103, 0 | 0 | 0 },
        { 0x8cfd03f, 0x0, 0 | 0 },
        0x3a37483e, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xb5cc, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Muted Trumpet */
        { 4, 0, 0x111 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 1, 153, 890, 153, 0, 0 | 0 | 0 },
        { 19, 12, 1023, 10000, 0, 0 | 0 | 0 },
        { 30, 153, 798, 153, 0, 0 | 0 | 0 },
        { 0x324279e, 0x6aaa, 0 | 0 },
        0x67cd0e34, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0xb46, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x174c8, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Pad 7 (halo) */
        { 6, 0, 0x10111 },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 15, 99, 942, 250, 0, 0 | 0 | 0 },
        { 12, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 51, 99, 911, 624, 0, 0 | 0 | 0 },
        { 0x12a72bc, 0xe1f9a, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x5bb1f98b, /* phaseModDepth */
        0x147, /* lfoPitchModDepth */
        0x68c, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Clarinet */
        { 4, 0, 0x10000 },
        { 0, 0, 0x195c0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 13, 250, 962, 99, 0, 0 | 0 | 0 },
        { 0, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 24, 99, 941, 365, 0, 0 | 0 | 0 },
        { 0x0, 0x0, 0 | 0 },
        0x224d90df, /* phaseModDepth */
        0xea, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        -1, 60, 128 /* boost, center, scale */
    },
    { /* Soprano Sax */
        { 4, 0, 0x9 },
        { 0, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x6bbf2 },
        { 5, 99, 876, 99, 0, 0 | 0 | 0 },
        { 3, 10, 1023, 10000, 0, 0 | 0 | 0 },
        { 24, 99, 798, 99, 0, 0 | 0 | 0 },
        { 0x19e1596, 0x20000, 0 | 0 },
        0x4b839123, /* phaseModDepth */
        0x187, /* lfoPitchModDepth */
        0x3d7, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xccd, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Helicopter */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xa8000 },
        { 8, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x8e230 },
        { 99, 1, 1023, 10000, 0, 0 | 0 | 0 },
        { 99, 1, 1023, 10000, 0, 0 | 0 | 0 },
        { 99, 1, 1023, 496, 0, 0 | 0 | 0 },
        { 0x115e95f, 0xe0000, SVFILTER_FLAG_ABS_PITCH | 0 },
        0x79495c5b, /* phaseModDepth */
        0x4f00, /* lfoPitchModDepth */
        0xbd52, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Telephone Ring */
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x104fd9 },
        { 0, OSC_FLAG_ABSOLUTE_PITCH, 0xf9555 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x78783 },
        { 1, 43, 147, 51, 0, ADSR_FLAG_LOOP_SUSTAIN | 0 | 0 },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 1, 153, 978, 1527, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x2b2f6430, /* phaseModDepth */
        0x72f, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Maracas */
        { 11, OSC_FLAG_ABSOLUTE_PITCH, 0x1084f6 },
        { 11, OSC_FLAG_ABSOLUTE_PITCH, 0xeef36 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x81555 },
        { 1, 42, 0, 190, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 321, 364, 264, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 218, 594, 1721, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0xece82e, 0x20000, 0 | 0 },
        0x7fffffff, /* phaseModDepth */
        0x2b87, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Low Bongo */
        { 4, 0, 0xffff0000 },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 29, 249, 208, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 34, 576, 870, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x4e36ec3, 0x10000, 0 | 0 },
        0x43ce8984, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x37e3, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Shamisen */
        { 7, 0, 0x0 },
        { 5, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 3, 343, 326, 112, 0, 0 | 0 | 0 },
        { 5, 10000, 1004, 3664, 0, 0 | 0 | 0 },
        { 3, 3075, 0, 176, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x7d67767e, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Breath Noise */
        { 9, 0, 0x30000 },
        { 8, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x53b4f },
        { 10, 99, 0, 10, 0, 0 | 0 | 0 },
        { 99, 250, 870, 624, 0, 0 | 0 | 0 },
        { 99, 125, 931, 153, 0, 0 | 0 | 0 },
        { 0x241edfa, 0x10000, 0 | 0 },
        0x33160022, /* phaseModDepth */
        0x9e, /* lfoPitchModDepth */
        0x4e5, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x2034, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        2, 60, 128 /* boost, center, scale */
    },
    { /* Electric Snare */
        { 8, OSC_FLAG_ABSOLUTE_PITCH, 0x1084f6 },
        { 4, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x81555 },
        { 1, 496, 317, 1247, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 3162, 921, 3908, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 3, 195, 348, 1922, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x1cd5b6f, 0x12aaa, 0 | 0 },
        0x4e072b8d, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x52da, /* envPitchModDepth */
        0x1ccec, /* envCutoffModDepth */
        0 | 0 | HYBRID_FLAG_MODOSC_USE_MAINENV | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        3, 60, 128 /* boost, center, scale */
    },
    { /* Music Box */
        { 4, 0, 0x2d043 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 37, 953, 1614, 48, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 6, 39, 992, 3075, 90, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x2b84a78, /* phaseModDepth */
        0x8f, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Fretless Bass */
        { 7, 0, 0x36 },
        { 4, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x72379 },
        { 2, 1247, 0, 99, 0, 0 | 0 | 0 },
        { 10, 195, 1023, 10000, 0, 0 | 0 | 0 },
        { 25, 1527, 0, 125, 0, 0 | 0 | 0 },
        { 0x324279e, 0x0, 0 | 0 },
        0x581972f1, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0xab2d, /* envCutoffModDepth */
        0 | 0 | 0 | 0 | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Synth Bass 2 */
        { 7, 0, 0x5e },
        { 1, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x64a62 },
        { 1, 1037, 480, 602, 47, 0 | 0 | 0 },
        { 1, 3664, 1023, 2187, 0, 0 | 0 | 0 },
        { 2, 1614, 19, 51, 67, 0 | 0 | 0 },
        { 0xcab97e, 0x20000, 0 | 0 },
        0x18970418, /* phaseModDepth */
        0x2bb, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x174c8, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Synth Bass 1 */
        { 6, 0, 0x10036 },
        { 7, 0, 0x0 },
        { 4, OSC_FLAG_ABSOLUTE_PITCH, 0x64a62 },
        { 1, 232, 218, 602, 0, 0 | 0 | 0 },
        { 1, 3664, 1023, 2187, 0, 0 | 0 | 0 },
        { 2, 928, 210, 51, 57, 0 | 0 | 0 },
        { 0x62637e, 0x20000, 0 | 0 },
        0x5985d939, /* phaseModDepth */
        0x2bb, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x220ba, /* envCutoffModDepth */
        0 | 0 | 0 | HYBRID_FLAG_ENABLE_FILTER | 0, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Low Tom */
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0xa2aaa },
        { 0, 0, 0x0 },
        { 6, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 1, 232, 0, 529, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 1, 10000, 671, 10000, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 248, 0, 1106, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x0, 0x0, 0 | 0 },
        0x38677c5a, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x2457, /* envPitchModDepth */
        0x345f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        1, 60, 128 /* boost, center, scale */
    },
    { /* Vibraphone */
        { 4, 0, 0x35616 },
        { 0, 0, 0x0 },
        { 9, OSC_FLAG_ABSOLUTE_PITCH, 0x62b97 },
        { 11, 66, 1023, 1721, 10, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 2, 26, 941, 10000, 0, 0 | 0 | 0 },
        { 12, 27, 1023, 2050, 45, 0 | 0 | ADSR_FLAG_NO_WAIT },
        { 0x7e1480f, 0x30000, 0 | 0 },
        0x7a75c7a, /* phaseModDepth */
        0x55, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x6a7f, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | HYBRID_FLAG_IGNORE_NOTE_OFF, /* flags */
        0, 60, 128 /* boost, center, scale */
    },
    { /* Clav */
        { 4, 0, 0x15 },
        { 3, 0, 0x0 },
        { 5, OSC_FLAG_ABSOLUTE_PITCH, 0x56666 },
        { 7, 1527, 602, 99, 0, 0 | 0 | 0 },
        { 10, 10000, 1023, 10000, 0, 0 | 0 | 0 },
        { 6, 1247, 174, 38, 0, 0 | 0 | 0 },
        { 0x7e1480f, 0x0, 0 | 0 },
        0x2b2f6430, /* phaseModDepth */
        0x9e, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        HYBRID_FLAG_USE_PHASE_MOD | 0 | 0 | 0 | 0, /* flags */
        -1, 60, 128 /* boost, center, scale */
    },
};
#define SS_NUM_VALID_PRESETS (131)

EDITABLE unsigned char gHybridSynthProgramMap[GMIDI_NUM_PROGRAMS] =
{
    112, /* Acoustic Grand */
    100, /* Bright Acoustic */
    100, /* Electric Grand => Bright Acoustic */
    100, /* Honky-Tonk => Bright Acoustic */
    17, /* Electric Piano 1 => Electric Piano 2 */
    17, /* Electric Piano 2 */
    109, /* Harpsichord */
    130, /* Clav */
    18, /* Celesta */
    102, /* Glockenspiel */
    124, /* Music Box */
    129, /* Vibraphone */
    96, /* Marimba */
    55, /* Xylophone */
    59, /* Tubular Bells */
    41, /* Dulcimer */
    5, /* Drawbar Organ */
    12, /* Percussive Organ */
    4, /* Rock Organ */
    65, /* Church Organ */
    60, /* Reed Organ */
    95, /* Accordian */
    99, /* Harmonica */
    95, /* Tango Accordian => Accordian */
    34, /* Acoustic Guitar(nylon) */
    52, /* Acoustic Guitar(steel) */
    46, /* Electric Guitar(jazz) => Electric Guitar(clean) */
    46, /* Electric Guitar(clean) */
    22, /* Electric Guitar(muted) */
    87, /* Overdriven Guitar */
    10, /* Distortion Guitar */
    25, /* Guitar Harmonics */
    48, /* Acoustic Bass */
    48, /* Electric Bass(finger) => Acoustic Bass */
    48, /* Electric Bass(pick) => Acoustic Bass */
    125, /* Fretless Bass */
    108, /* Slap Bass 1 */
    108, /* Slap Bass 2 => Slap Bass 1 */
    127, /* Synth Bass 1 */
    126, /* Synth Bass 2 */
    57, /* Violin */
    57, /* Viola => Violin */
    57, /* Cello => Violin */
    57, /* Contrabass => Violin */
    32, /* Tremolo Strings */
    29, /* Pizzicato Strings */
    62, /* Orchestral Harp */
    37, /* Timpani */
    67, /* String Ensembles 1 */
    67, /* String Ensembles 2 => String Ensembles 1 */
    54, /* SynthStrings 1 */
    54, /* SynthStrings 2 => SynthStrings 1 */
    24, /* Choir Aahs */
    79, /* Voice Oohs */
    88, /* Synth Voice */
    93, /* Orchestra Hit */
    9, /* Trumpet */
    9, /* Trombone => Trumpet */
    9, /* Tuba => Trumpet */
    113, /* Muted Trumpet */
    53, /* French Horn */
    9, /* Brass Section => Trumpet */
    42, /* SynthBrass 1 */
    42, /* SynthBrass 2 => SynthBrass 1 */
    116, /* Soprano Sax */
    116, /* Alto Sax => Soprano Sax */
    116, /* Tenor Sax => Soprano Sax */
    116, /* Baritone Sax => Soprano Sax */
    51, /* Oboe */
    14, /* English Horn */
    78, /* Bassoon */
    115, /* Clarinet */
    75, /* Piccolo => Flute */
    75, /* Flute */
    82, /* Recorder */
    92, /* Pan Flute */
    36, /* Blown Bottle */
    75, /* Skakuhachi => Flute */
    75, /* Whistle => Flute */
    75, /* Ocarina => Flute */
    76, /* Lead 1 (square) */
    47, /* Lead 2 (sawtooth) */
    69, /* Lead 3 (calliope) */
    89, /* Lead 4 (chiff) */
    69, /* Lead 5 (charang) => Lead 3 (calliope) */
    58, /* Lead 6 (voice) */
    28, /* Lead 7 (fifths) */
    42, /* Lead 8 (bass+lead) => SynthBrass 1 */
    35, /* Pad 1 (new age) */
    111, /* Pad 2 (warm) */
    73, /* Pad 3 (polysynth) */
    58, /* Pad 4 (choir) => Lead 6 (voice) */
    27, /* Pad 5 (bowed) */
    2, /* Pad 6 (metallic) */
    114, /* Pad 7 (halo) */
    39, /* Pad 8 (sweep) */
    31, /* FX 1 (rain) */
    68, /* FX 2 (soundtrack) */
    91, /* FX 3 (crystal) */
    80, /* FX 4 (atmosphere) */
    49, /* FX 5 (brightness) */
    33, /* FX 6 (goblins) */
    13, /* FX 7 (echoes) */
    80, /* FX 8 (sci-fi) => FX 4 (atmosphere) */
    19, /* Sitar */
    109, /* Banjo => Harpsichord */
    121, /* Shamisen */
    109, /* Koto => Harpsichord */
    38, /* Kalimba */
    116, /* Bagpipe => Soprano Sax */
    57, /* Fiddle => Violin */
    51, /* Shanai => Oboe */
    15, /* Tinkle Bell */
    84, /* Agogo */
    74, /* Steel Drums */
    66, /* Woodblock */
    90, /* Taiko Drum */
    86, /* Melodic Tom */
    7, /* Synth Drum */
    16, /* Reverse Cymbal */
    20, /* Guitar Fret Noise */
    122, /* Breath Noise */
    44, /* Seashore */
    85, /* Bird Tweet */
    118, /* Telephone Ring */
    117, /* Helicopter */
    21, /* Applause */
    3, /* Gunshot */
};

EDITABLE unsigned char gHybridSynthDrumMap[GMIDI_NUM_DRUMS] =
{
    94, /* Acoustic Bass Drum */
    94, /* Bass Drum 1 => Acoustic Bass Drum */
    30, /* Side Stick */
    110, /* Acoustic Snare */
    8, /* Hand Clap */
    123, /* Electric Snare */
    97, /* Low Floor Tom */
    56, /* Closed Hi-Hat */
    97, /* High Floor Tom => Low Floor Tom */
    11, /* Pedal Hi-Hat */
    128, /* Low Tom */
    45, /* Open Hi-Hat */
    128, /* Low-Mid Tom => Low Tom */
    128, /* Hi-Mid Tom => Low Tom */
    105, /* Crash Cymbal 1 */
    128, /* High Tom => Low Tom */
    101, /* Ride Cymbal 1 */
    64, /* Chinese Cymbal */
    107, /* Ride Bell */
    103, /* Tambourine */
    104, /* Splash Cymbal => Crash Cymbal 2 */
    26, /* Cowbell */
    104, /* Crash Cymbal 2 */
    83, /* Vibraslap */
    98, /* Ride Cymbal 2 */
    71, /* Hi Bongo */
    120, /* Low Bongo */
    61, /* Mute Hi Conga */
    6, /* Open Hi Conga */
    6, /* Low Conga => Open Hi Conga */
    23, /* High Timbale */
    23, /* Low Timbale => High Timbale */
    50, /* High Agogo */
    50, /* Low Agogo => High Agogo */
    72, /* Cabasa */
    119, /* Maracas */
    106, /* Short Whistle */
    70, /* Long Whistle */
    77, /* Short Guiro */
    63, /* Long Guiro */
    0, /* Claves */
    0, /* Hi Wood Block => Claves */
    0, /* Low Wood Block => Claves */
    40, /* Mute Cuica */
    40, /* Open Cuica => Mute Cuica */
    81, /* Mute Triangle */
    1, /* Open Triangle */
};

EDITABLE unsigned char gHybridSynthDrumPitches[GMIDI_NUM_DRUMS] =
{
    40, /* Acoustic Bass Drum */
    36, /* Bass Drum 1 */
    97, /* Side Stick */
    52, /* Acoustic Snare */
    76, /* Hand Clap */
    60, /* Electric Snare */
    38, /* Low Floor Tom */
    90, /* Closed Hi-Hat */
    42, /* High Floor Tom */
    90, /* Pedal Hi-Hat */
    44, /* Low Tom */
    90, /* Open Hi-Hat */
    50, /* Low-Mid Tom */
    55, /* Hi-Mid Tom */
    90, /* Crash Cymbal 1 */
    66, /* High Tom */
    96, /* Ride Cymbal 1 */
    98, /* Chinese Cymbal */
    96, /* Ride Bell */
    100, /* Tambourine */
    100, /* Splash Cymbal */
    86, /* Cowbell */
    98, /* Crash Cymbal 2 */
    89, /* Vibraslap */
    94, /* Ride Cymbal 2 */
    77, /* Hi Bongo */
    65, /* Low Bongo */
    77, /* Mute Hi Conga */
    64, /* Open Hi Conga */
    54, /* Low Conga */
    77, /* High Timbale */
    70, /* Low Timbale */
    100, /* High Agogo */
    86, /* Low Agogo */
    101, /* Cabasa */
    96, /* Maracas */
    100, /* Short Whistle */
    100, /* Long Whistle */
    89, /* Short Guiro */
    89, /* Long Guiro */
    97, /* Claves */
    83, /* Hi Wood Block */
    75, /* Low Wood Block */
    88, /* Mute Cuica */
    60, /* Open Cuica */
    97, /* Mute Triangle */
    97, /* Open Triangle */
};
#endif /* _SPMIDI_HYBRID_PRESETS_H */
