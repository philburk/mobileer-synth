/* $Id: onepole_filter.c,v 1.1 2011/10/03 21:01:06 phil Exp $ */
/**
 *
 * One Pole resonant filter for DLS2.
 *
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_print.h"
#include "onepole_filter.h"


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
/* Can read and write same buffer.
*/
void OnePoleFilter_Next( OnePoleFilter_t *filter, FXP31 *inputBuffer, FXP31 *outputBuffer)
{
    int i;
    FXP27 zm1, zm2, a0, b1, b2;
    FXP27 output;
    FXP27 input;

    zm1 = filter->zm1;
    zm2 = filter->zm2;
    a0 = filter->a0;
    b1 = filter->b1;
    b2 = filter->b2;

    for( i=0; i<SS_FRAMES_PER_BLOCK; i++)
    {
        input = (*inputBuffer++) >> 4;

        output = FXP27_MULT( a0, input );
        output += FXP27_MULT( b1, zm1 );
        output += FXP27_MULT( b2, zm2 );

        /* Delay line. */
        zm2 = zm1;
        zm1 = output;

        *outputBuffer++ = output << 4;

    }

    filter->zm1 = zm1;
    filter->zm2 = zm2;
}

/********************************************************************
 * Filter parameter F1 is ideally defined as:
 * F1 = 2 * sin( PI * F / SR )
 * Assume octavePitch has already been adjusted for sample rate.
 */
void OnePoleFilter_SetPitch( OnePoleFilter_t *filter, FXP16 octavePitch )
{
    if( octavePitch > SS_MAX_PITCH )
    {
        octavePitch = SS_MAX_PITCH;
    }

#if 1
    {
        double dResonance = filter->info->resonance / ((double)(1<<16));
        /* This calculation uses floating point! */
        double dFrequency = FREQUENCY_OCTAVE_ZERO *
            pow( 2.0, (((double)octavePitch) / (1 << SS_PITCH_SHIFT)) );
        double theta = 3.14159265 * dFrequency / SS_BASE_SAMPLE_RATE;

        double numerator = cos(theta) + (sin(theta) * sqrt( pow( 10.0, dResonance/10.0 ) - 1.0 ));
        double denominator = (sin(theta) * pow(10.0,dResonance/20.0)) + 1.0;
        double radius = numerator / denominator;
        filter->b1 = (FXP27) ((-2.0 * radius * cos( theta )) * (1<<27));
        filter->b2 = (FXP27) ((radius * radius) * (1<<27));
//      filter->a0 = (FXP27) (1<<27);
        filter->a0 = (FXP27) (pow( 10.0, filter->info->resonance / (-40 << 27) ) * (1<<27));

        printf("a0 = 0x%\n", filter->a0 );
        printf("b1 = %g\n", filter->b1 );
        printf("b2 = %g\n", filter->b2 );
    }
#endif

}

/********************************************************************/
void OnePoleFilter_Load( const OnePoleFilter_Preset_t *preset, OnePoleFilter_Info_t *info )
{
    info->resonance     = preset->resonance;
    info->cutoffPitch  = preset->cutoffPitch;
}

#if SPMIDI_SUPPORT_LOADING
/***********************************************************************
 * The byte order must match the save() method in the FilterModel class in the HybridEditor.
 */
unsigned char *OnePoleFilter_Define( OnePoleFilter_Preset_t *preset, unsigned char *p )
{
    p = SS_ParseLong( &preset->resonance, p );

    preset->flags = 0;
    if( *p++ )
        preset->flags |= OnePoleFilter_FLAG_HIGH_PASS;

    /* Pitch Model */
    if( *p++ )
        preset->flags |= OnePoleFilter_FLAG_ABS_PITCH;
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
    int is;
    short sample;
    WAV_Writer *writer;
    int result;
    FXP31 input[SS_FRAMES_PER_BLOCK];
    FXP31 output[SS_FRAMES_PER_BLOCK];
    OnePoleFilter_t OPF = { 0 };
    OnePoleFilter_Info_t OPF_Info = { 0 };

    printf("Test One Pole Filter\n");


    /* Open file. */
    result = Audio_WAV_CreateWriter( &writer, "spmidi_output.wav" );
    if( result < 0 )
    {
        printf("Can't open output file spmidi_output.wav\n" );
        return 1;
    }

    result =  Audio_WAV_OpenWriter( writer, 44100, 2 );
    if( result < 0 )
        return result;

    OPF_Info.resonance = 2 << 25;
    OPF.info = &OPF_Info;

    OnePoleFilter_SetPitch( &OPF, 0x000C0000 );

    for( is=0; is<SS_FRAMES_PER_BLOCK; is++ )
    {
        input[is] = 0;
    }
    input[5] = 0x10000000;

    for( i=0; i<1000; i++ )
    {

        OnePoleFilter_Next( &OPF, input, output );

        /* Write input and output as a stereo pair. */
        for( is=0; is<SS_FRAMES_PER_BLOCK; is++ )
        {
            sample = (short) (input[is] >> (31-15));
            Audio_WAV_WriteShorts( writer, &sample, 1 );

            sample = (short) (output[is] >> (31-15));
            Audio_WAV_WriteShorts( writer, &sample, 1 );
        }

        
        input[5] = 0x00000000;
    }

    Audio_WAV_CloseWriter( writer );
    Audio_WAV_DeleteWriter( writer );

    printf("Test complete.\n");

    return 0;
}
#endif
