#ifndef _ONEPOLE_FILTER_H
#define _ONEPOLE_FILTER_H
/* $Id: onepole_filter.h,v 1.1 2011/10/03 21:01:06 phil Exp $ */
/**
 *
 * State Variable Filter.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/engine/spmidi_synth_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct OnePoleFilter_Preset_s
    {
        FXP27                    resonance;
        PitchOctave              cutoffPitch; /* Absolute or relative pitchOctave like modOsc */
    }
    OnePoleFilter_Preset_t;

    typedef struct OnePoleFilter_Info_s
    {
        FXP27                    resonance;
        PitchOctave              cutoffPitch; /* Absolute or relative pitchOctave like modOsc */
    }
    OnePoleFilter_Info_t;

    /* Structure of Unit Generator. */
    typedef struct OnePoleFilter_s
    {
        OnePoleFilter_Info_t *info;
        FXP27      zm1;
        FXP27      zm2;
        FXP27      b1;
        FXP27      b2;
        FXP27      a0;
    }
    OnePoleFilter_t;

    //void OnePoleFilter_Next( OnePoleFilter_t *filter, FXP31 *buffer );
void OnePoleFilter_Next( OnePoleFilter_t *filter, FXP31 *inputBuffer, FXP31 *outputBuffer);

    void OnePoleFilter_SetPitch( OnePoleFilter_t *filter, FXP16 octavePitch );

    void OnePoleFilter_Load( const OnePoleFilter_Preset_t *preset, OnePoleFilter_Info_t *info );

    unsigned char *OnePoleFilter_Define( OnePoleFilter_Preset_t *preset, unsigned char *p );

#ifdef __cplusplus
}
#endif

#endif /* _ONEPOLE_FILTER_H */
