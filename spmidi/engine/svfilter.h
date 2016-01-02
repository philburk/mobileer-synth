#ifndef _SVFILTER_H
#define _SVFILTER_H
/* $Id: svfilter.h,v 1.9 2007/10/02 16:14:42 philjmsl Exp $ */
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

#define SVFILTER_FLAG_ABS_PITCH         (0x01)
#define SVFILTER_FLAG_HIGH_PASS         (0x02)

    typedef struct SVFilter_Preset_s
    {
        FXP27                    inverseQ;
        PitchOctave              cutoffPitch; /* Absolute or relative pitchOctave like modOsc */
        unsigned char            flags;
    }
    SVFilter_Preset_t;

    typedef struct SVFilter_Info_s
    {
        FXP27                    inverseQ;
        PitchOctave              cutoffPitch; /* Absolute or relative pitchOctave like modOsc */
        unsigned char            flags;
    }
    SVFilter_Info_t;

    /* Structure of Unit Generator. */
    typedef struct SVFilter_s
    {
        SVFilter_Info_t *info;
        FXP27      lowKeep;
        FXP27      bandKeep;
        FXP27      frequencyParam;
    }
    SVFilter_t;

    void SVFilter_Next( SVFilter_t *filter, FXP31 *buffer );

    void SVFilter_SetPitch( SVFilter_t *filter, FXP16 octavePitch );

    void SVFilter_Load( const SVFilter_Preset_t *preset, SVFilter_Info_t *info );

    unsigned char *SVFilter_Define( SVFilter_Preset_t *preset, unsigned char *p );

#ifdef __cplusplus
}
#endif

#endif /* _SVFILTER_H */
