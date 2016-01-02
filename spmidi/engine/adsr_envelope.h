#ifndef _ADSR_ENVELOPE_H
#define _ADSR_ENVELOPE_H
/* $Id: adsr_envelope.h,v 1.19 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * ADSR Envelope.
 * ADSR = Attack/Decay/Sustain/Release
 * Envelopes are used to provide contour for amplitude control
 * and other types of modulation.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct EnvelopeADSR_s
    {
        /** One of ADSR_STAGES */
        spmSInt  stage;
        /** Current internal value before translation to external curve. */
        FXP31    current;
        /** These are pitch dependant so they need to be here
        * instead of in the envelope info structure. */
        FXP31    scaledDecayIncrement;
        FXP31    scaledReleaseIncrement;
    }
    EnvelopeADSR_t;


    /** Loop back to attack segment when envelope hits sustain level.
     * Start at current level, not zero.
     */
#define ADSR_FLAG_LOOP_SUSTAIN  (1<<0)

    /** Loop to beginning when envelop hits end of release. */
#define ADSR_FLAG_LOOP_RELEASE  (1<<1)

    /** Do not wait for noteOff. Go from Decay to Release when hit Sustain level. */
#define ADSR_FLAG_NO_WAIT       (1<<2)

    /** Hold indefinitely before DECAY, waiting for external trigger like a wavetable loop.
     * This lets us use the natural attack of the wavetable and then decay once in the wave loop.
     */
#define ADSR_FLAG_WAIT_DECAY    (1<<3)

    /** Value in Preset that corresponds to maximum sustain level. */
#define ADSR_SUSTAIN_MAX        (1023)

    typedef struct EnvelopeADSR_Preset_s
    {
        /** Time for attack segment to go from zero to full in milliseconds. */
        spmSInt16  attackTime;
        spmSInt16  decayTime;
        /** Hold level. 10 bit resolution, 0 to 1023 */
        spmSInt16  sustainLevel;
        spmSInt16  releaseTime;
        /** Amount attack, decay and release time are modified at highest pitch, 0 to 127. */
        spmUInt8   pitchScalar;
        spmUInt8   flags;
    }
    EnvelopeADSR_Preset_t;

    /** Contain runtime parameters that cannot be calculated at compile time. */
    typedef struct EnvelopeADSR_Info_s
    {
        FXP31  attackIncrement;
    }
    EnvelopeADSR_Info_t;

    typedef struct EnvelopeDAHDSR_Extension_s
    {
        FXP31  delayIncrement;
        FXP31  holdIncrement;
    }
    EnvelopeDAHDSR_Extension_t;

    typedef enum ADSR_STAGES_E
    {
        ADSR_STAGE_IDLE = 0,
        ADSR_STAGE_DELLLAY, /* Spell it strangely because it looks too much like DECAY. */
        ADSR_STAGE_ATTACK,
        ADSR_STAGE_HOLD,
        ADSR_STAGE_DECAY,
        ADSR_STAGE_SUSTAIN,
        ADSR_STAGE_RELEASE,
        ADSR_STAGE_STIFLE
    } ADSR_STAGES;

    /**
     * Convert milliseconds to an envelope increment per sample.
     * Note that envelopes are generally calculated at a lower sample
     * rate then oscillators because they change more slowly.
     */
    int ADSR_MSecToIncrement( int msec, int envelopeSampleRate );


    /**
     * Generate next block of envelope data.
     */
    int ADSR_Next( const EnvelopeADSR_Preset_t *preset, const EnvelopeADSR_Info_t *info, EnvelopeADSR_t *adsr,
           FXP31 *outputBuffer, EnvelopeDAHDSR_Extension_t *extension );

    /**
     * Setup info structure with information needed to execute envelope.
     */
    void ADSR_Load( const EnvelopeADSR_Preset_t *preset, EnvelopeADSR_Info_t *info, int sampleRate );

    /**
     * Start attack stage of ADSR envelope.
     * Scale decay based on pitch.
     */
    void ADSR_Start( EnvelopeADSR_t *adsr, const EnvelopeADSR_Preset_t *preset, int pitch, int sampleRate);

    /**
     * Trigger final release phase of envelope.
     */
    void ADSR_Release( EnvelopeADSR_t *adsr );
    
    /**
     * Trigger transition from HOLD to DECAY state.
     */
    void ADSR_TriggerDecay( EnvelopeADSR_t *adsr );

    /**
     * Extract preset information from bulk dump format passed from instrument editor.
     */
    unsigned char *ADSR_Define( EnvelopeADSR_Preset_t *preset, unsigned char *p );

#ifdef __cplusplus
}
#endif

#endif /* _ADSR_ENVELOPE_H */
