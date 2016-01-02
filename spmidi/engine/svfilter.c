/* $Id: svfilter.c,v 1.10 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * State Variable Filter.
 * This filter produces both low and high pass outputs.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
 
#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_print.h"
#include "svfilter.h"

#ifdef WIN32
#include <math.h>
#ifndef M_PI
#define M_PI   (3.1415926535897932384626433832795)
#endif
#define TWO_PI   (2.0 * M_PI)
#endif

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif


/*********************************************************************************/
/* Reads and writes same buffer.
*/
void SVFilter_Next( SVFilter_t *filter, FXP31 *buffer )
{
    int i;
    FXP27 lowPass, bandPass, freqParam, highPass, inverseQ;

    lowPass = filter->lowKeep;
    bandPass = filter->bandKeep;
    freqParam = filter->frequencyParam;
    inverseQ = filter->info->inverseQ;

    if( (filter->info->flags & SVFILTER_FLAG_HIGH_PASS) != 0 )
    {
        for( i=0; i<SS_FRAMES_PER_BLOCK; i++)
        {
            lowPass = FXP27_MULT(freqParam, bandPass) + lowPass;

            highPass = (buffer[i] >> (4 + 1)) -
                       FXP27_MULT(inverseQ, bandPass) - lowPass;
            /* PLB050721 - clip highPass instead of lowPass to prevent
             * breakup in cymbals at 8000 Hz. */
            highPass = (highPass > FXP27_MAX_VALUE) ? FXP27_MAX_VALUE :
                      ((highPass < FXP27_MIN_VALUE) ? FXP27_MIN_VALUE : highPass);  /* Prevent blowup. */

            buffer[i] = (highPass << 4);   /* Save HIGHPASS result. */

            bandPass = FXP27_MULT(freqParam, highPass) + bandPass;
        }
    }
    else
    {
        for( i=0; i<SS_FRAMES_PER_BLOCK; i++)
        {
            lowPass = FXP27_MULT(freqParam, bandPass) + lowPass;
            lowPass = (lowPass > FXP27_MAX_VALUE) ? FXP27_MAX_VALUE :
                      ((lowPass < FXP27_MIN_VALUE) ? FXP27_MIN_VALUE : lowPass);  /* Prevent blowup. */


            highPass = (buffer[i] >> (4 + 1)) -
                       FXP27_MULT(inverseQ, bandPass) - lowPass;

            buffer[i] = (lowPass << 4);  /* Save LOWPASS result. */

            bandPass = FXP27_MULT(freqParam, highPass) + bandPass;
        }
    }

    filter->lowKeep = lowPass;
    filter->bandKeep = bandPass;
}

/********************************************************************
 * Filter parameter F1 is ideally defined as:
 * F1 = 2 * sin( PI * F / SR )
 * Assume octavePitch has already been adjusted for sample rate.
 */
void SVFilter_SetPitch( SVFilter_t *filter, FXP16 octavePitch )
{
    if( octavePitch > SS_MAX_PITCH )
    {
        octavePitch = SS_MAX_PITCH;
    }

    {
#if 1
        /* This uses an approximated tuning which seems OK in MIDI range. */
        FXP31 phaseInc = SPMUtil_OctaveToPhaseIncrement( octavePitch );
#define FXP31_PI_OVER_4  (0x6487ED50)

        FXP31 filterTemp = FXP31_MULT( FXP31_PI_OVER_4, phaseInc );
        filter->frequencyParam = (FXP27) (filterTemp >> 2);
#else
        /* This calculation uses floating point! */
        double frequency = FREQUENCY_OCTAVE_ZERO * pow( 2.0, (((double)octavePitch) / (1 << SS_PITCH_SHIFT)) );
        double f1 = 2.0 * sin( (frequency * M_PI) / SS_BASE_SAMPLE_RATE );
#define DOUBLE_TO_FXP27( dbl )    ((FXP27)((dbl) * FXP27_MAX_VALUE))

        filter->frequencyParam = DOUBLE_TO_FXP27( f1 );
#endif
        
    }

    if( filter->frequencyParam > FXP27_MAX_VALUE )
    {
        filter->frequencyParam = FXP27_MAX_VALUE;
    }

}

/********************************************************************/
void SVFilter_Load( const SVFilter_Preset_t *preset, SVFilter_Info_t *info )
{
    info->inverseQ     = preset->inverseQ;
    info->cutoffPitch  = preset->cutoffPitch;
    info->flags        = preset->flags;
}

#if SPMIDI_SUPPORT_LOADING
/***********************************************************************
 * The byte order must match the save() method in the FilterModel class in the HybridEditor.
 */
unsigned char *SVFilter_Define( SVFilter_Preset_t *preset, unsigned char *p )
{
    p = SS_ParseLong( &preset->inverseQ, p );

    preset->flags = 0;
    if( *p++ )
        preset->flags |= SVFILTER_FLAG_HIGH_PASS;

    /* Pitch Model */
    if( *p++ )
        preset->flags |= SVFILTER_FLAG_ABS_PITCH;
    p = SS_ParseLong( &preset->cutoffPitch, p );

    return p;
}
#endif /* SPMIDI_SUPPORT_LOADING */

#if 0
#include "spmidi/include/write_wav.h"

/* Render filter to file for external viewing with wave editor. */
int main( void )
{
    int i;
    short sample;
    WAV_Writer *writer;
    int result;
    FXP31 input[SS_FRAMES_PER_BLOCK];
    FXP31 output[SS_FRAMES_PER_BLOCK];
    SVFilter_t SVF = { 0 };

    printf("Test SVFilter\n");


    /* Open file. */
    result = Audio_WAV_CreateWriter( &writer, "rendered_midi.wav" );
    if( result < 0 )
    {
        printf("Can't open output file rendered_svf.raw\n" );
        return 1;
    }

    result =  Audio_WAV_OpenWriter( writer, 44100, 1 );
    if( result < 0 )
        return result;

    /* impulse train */
    for( i=0; i<SS_FRAMES_PER_BLOCK; i++ )
        input[i] = 0;

    SVF.inverseQ = FXP27_MAX_VALUE / 10;
    SVFilter_SetPitch( &SVF, 0x00147000 );


    for( i=0; i<10; i++ )
    {
        int is;

        /* Occasional impulse. */
        if( (i & 7) == 0 )
            input[5] = FXP31_MAX_VALUE/100;
        else
            input[5] = 0;

        SVFilter_Next( &SVF, input, output );
        /* Write input and output as a stereo pair. */
        for( is=0; is<SS_FRAMES_PER_BLOCK; is++ )
        {
            sample = (short) (input[is] >> (31-15));
            Audio_WAV_WriteShorts( writer, &sample, 1 );

            sample = (short) (output[is] >> (31-15));
            Audio_WAV_WriteShorts( writer, &sample, 1 );
        }
    }

    Audio_WAV_CloseWriter( writer );
    Audio_WAV_DeleteWriter( writer );

    printf("Test complete.\n");

    return 0;
}
#endif
