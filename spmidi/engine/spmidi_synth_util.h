#ifndef _SPMIDI_SYNTH_UTIL_H
#define _SPMIDI_SYNTH_UTIL_H
/* $Id: spmidi_synth_util.h,v 1.13 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Internal tools for use by synthesizer.
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Nominal sample rate for pitch calculations.
     * When converting pitch to phase increment for other sample rates,
     * the pitch can simply be offset to adjust for
     * the different sample rate. Offset is 0.0 for 44100 Hz sample rate.
     */
#define SS_BASE_SAMPLE_RATE   (44100)

    /** Octave index of MIDI note 0.
     * We extend below the lowest MIDI pitch so that we can specify
     * LFO rates in PitchOctave format.
     */
#define MIDI_OCTAVE_OFFSET   (8)

#define MIDI_PITCH_CONCERT_A  (69)

    /** Bits to shift a 1 to get 1.0 fixed point shift. */
#define SS_PITCH_SHIFT        (16)

    /** Frequency of Octave Zero. Lowest possible pitch in PitchOctave format.
     */
#define FREQUENCY_OCTAVE_ZERO   (440.0 * pow( 2.0, - ((MIDI_PITCH_CONCERT_A / 12.0) + MIDI_OCTAVE_OFFSET) ))
    /* #define FREQUENCY_OCTAVE_ZERO   (0.0319367145) */

    /** Fractional Octave pitch of MIDI note 0 */
#define BASE_MIDI_PITCH  ( MIDI_OCTAVE_OFFSET << SS_PITCH_SHIFT )

    /** Fractional octave representation for pitch.
     * pitchOctave = log2(freq/FREQUENCY_OCTAVE_ZERO) * 65536
     */
    typedef FXP16 PitchOctave;

#define SS_TOP_OCTAVE (19)
#define SS_MAX_PITCH   ((SS_TOP_OCTAVE-1) << SS_PITCH_SHIFT)

    /********************************************************************
     * Calculate an octave based pitch based on a MIDI pitch fraction in 16.16 format.
     */
#define SPMUtil_MIDIPitchToOctave( midiPitch ) (((midiPitch) / 12) + BASE_MIDI_PITCH)


    /**
     * Convert a fractional octave pitch to a phase increment.
     * Assume adjusted for SS_BASE_SAMPLE_RATE.
     *
     * relFreq = 2 ^ ( octavePitch /  (1 << SS_PITCH_SHIFT))
     * freq = baseFreq * relFreq;
     */
    FXP31 SPMUtil_OctaveToPhaseIncrement( FXP16 octavePitch );

    /**
     * Non-linear amplifier that whill clip a signal to the mixer range
     * with soft corners. This will sound better than a simple clipper.
     * This allows us to push the amplitude without getting too harsh.
     * The algorithm is based on the equation:
     *
     *     y = 1.5 * (x - ((x^3)/3))
     */
    FXP31 SS_MixerSoftClip( FXP31 input );

    /** Parse 32 bit integer assuming Big Endian byte order. */
    unsigned char *SS_ParseLong( long *data, unsigned char *p);

    /** Parse 16 bit integer assuming Big Endian byte order. */
    unsigned char *SS_ParseShort( short *data, unsigned char *p);

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_SYNTH_UTIL_H */
