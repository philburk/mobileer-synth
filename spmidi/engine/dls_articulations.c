/* $Id: dls_articulations.c,v 1.2 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * DLS Articulation support.
 * Convert DLS values to ME3000 values.
 *
 * @author Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_dls.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_hybrid.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#define DBUGMSGNUMH( msg, num ) { DBUGMSG( msg ); DBUGNUMH( num ); DBUGMSG("\n"); }

/* Only compile if supporting ME3000 API */
#if SPMIDI_ME3000


#define MSEC_PER_SECOND     (1000)

/*******************************************************************************/
/******* Convert Articulation Data *********************************************/
/*******************************************************************************/

/*
The envelope times are specified using "Absolute Time",
which is defined as:
    
      AbsoluteTime = 1200 * log2( timeInSeconds ) * 65536
 
Thus:
 
    log2( timeInSeconds ) = AbsoluteTime / (1200 * 65536)
    timeInSeconds = 2.0 ** (AbsoluteTime / (1200 * 65536))
 
    timeInMilliseconds = 1000 * (2.0 ** (AbsoluteTime / (1200 * 65536)))
    Q.E.D.
 
We will use a lookup table to convert AbsoluteTime to milliseconds for use by ME3000.
*/

#define SSDLS_ABSTIME_MILLIS_SIZE  (80)
/* All milliseconds below this absTime are zero. */
#define SSDLS_ABSTIME_START  ((spmSInt32) 0xCB000000)
static const spmUInt16 sAbsTimeToMilliseconds[] =
    {
        0, /* 0, 0xCB000000 */
        0, /* 1, 0xCC000000 */
        1, /* 2, 0xCD000000 */
        1, /* 3, 0xCE000000 */
        1, /* 4, 0xCF000000 */
        1, /* 5, 0xD0000000 */
        1, /* 6, 0xD1000000 */
        1, /* 7, 0xD2000000 */
        1, /* 8, 0xD3000000 */
        1, /* 9, 0xD4000000 */
        2, /* 10, 0xD5000000 */
        2, /* 11, 0xD6000000 */
        2, /* 12, 0xD7000000 */
        3, /* 13, 0xD8000000 */
        3, /* 14, 0xD9000000 */
        4, /* 15, 0xDA000000 */
        4, /* 16, 0xDB000000 */
        5, /* 17, 0xDC000000 */
        6, /* 18, 0xDD000000 */
        7, /* 19, 0xDE000000 */
        8, /* 20, 0xDF000000 */
        9, /* 21, 0xE0000000 */
        10, /* 22, 0xE1000000 */
        12, /* 23, 0xE2000000 */
        14, /* 24, 0xE3000000 */
        16, /* 25, 0xE4000000 */
        18, /* 26, 0xE5000000 */
        21, /* 27, 0xE6000000 */
        25, /* 28, 0xE7000000 */
        29, /* 29, 0xE8000000 */
        33, /* 30, 0xE9000000 */
        39, /* 31, 0xEA000000 */
        45, /* 32, 0xEB000000 */
        52, /* 33, 0xEC000000 */
        60, /* 34, 0xED000000 */
        70, /* 35, 0xEE000000 */
        81, /* 36, 0xEF000000 */
        94, /* 37, 0xF0000000 */
        109, /* 38, 0xF1000000 */
        126, /* 39, 0xF2000000 */
        146, /* 40, 0xF3000000 */
        170, /* 41, 0xF4000000 */
        197, /* 42, 0xF5000000 */
        228, /* 43, 0xF6000000 */
        264, /* 44, 0xF7000000 */
        306, /* 45, 0xF8000000 */
        355, /* 46, 0xF9000000 */
        412, /* 47, 0xFA000000 */
        477, /* 48, 0xFB000000 */
        554, /* 49, 0xFC000000 */
        642, /* 50, 0xFD000000 */
        744, /* 51, 0xFE000000 */
        863, /* 52, 0xFF000000 */
        1000, /* 53, 0x00000000 */
        1159, /* 54, 0x01000000 */
        1344, /* 55, 0x02000000 */
        1558, /* 56, 0x03000000 */
        1807, /* 57, 0x04000000 */
        2095, /* 58, 0x05000000 */
        2428, /* 59, 0x06000000 */
        2815, /* 60, 0x07000000 */
        3264, /* 61, 0x08000000 */
        3784, /* 62, 0x09000000 */
        4387, /* 63, 0x0A000000 */
        5086, /* 64, 0x0B000000 */
        5897, /* 65, 0x0C000000 */
        6837, /* 66, 0x0D000000 */
        7926, /* 67, 0x0E000000 */
        9190, /* 68, 0x0F000000 */
        10654, /* 69, 0x10000000 */
        12352, /* 70, 0x11000000 */
        14320, /* 71, 0x12000000 */
        16603, /* 72, 0x13000000 */
        19248, /* 73, 0x14000000 */
        22316, /* 74, 0x15000000 */
        25872, /* 75, 0x16000000 */
        29995, /* 76, 0x17000000 */
        32767, /* 77, 0x18000000 */ /* Only go to 32767 because envelope msec are spmSInt32 */
        32767, /* 78, 0x19000000 */
        32767, /* 79, 0x1A000000 */
    };

/*******************************************************************************/

/**
 * Lookup an interpolated value in a table of Unsigned Shorts.
 * @param table lookup table
 * @param input is normalized to a 16.16 fraction, high portion becomes index
 */
spmSInt32 SSDLS_UShortTableLookup( const spmUInt16 *table, int numEntries, spmUInt32 input  )
{
    const spmUInt16 *tablePtr;
    spmSInt32 tableDelta;
    spmSInt32 result;
    spmSInt32 lowValue, highValue;
    spmSInt32 fraction;

    int index = input >> 16;
    /* Do not interpolate past last value. */
    if( index >= (numEntries-1) )
    {
        return table[numEntries-1];
    }

    /* Calculate fraction for interpolation. */
    fraction = (spmSInt32) (input & 0xFFFF);

    tablePtr = &table[ index ];
    lowValue = (spmSInt32) (*tablePtr++);
    highValue = (spmSInt32) (*tablePtr);

    /* Interpolate between succesive table values. */
    tableDelta = (highValue - lowValue);
    result = lowValue  +
             ((fraction * tableDelta) >> 16);
    return result;
}

/*******************************************************************************/
static spmSInt32 SSDLS_ConvertAbsoluteTimeToMilliseconds( spmSInt32 absTime )
{
    spmSInt32 millis;
    spmUInt32 unsignedAbsTime;

    if( absTime < SSDLS_ABSTIME_START )
        absTime = SSDLS_ABSTIME_START;

    unsignedAbsTime = (spmUInt32) (absTime - SSDLS_ABSTIME_START);

    millis = SSDLS_UShortTableLookup( sAbsTimeToMilliseconds, SSDLS_ABSTIME_MILLIS_SIZE,
                                      (unsignedAbsTime >> 8) );
    return millis;
}

/* Enable this code to rebuild lookup table source. */
#if 0
/*******************************************************************************/
#include <math.h>
/* Calculate on desktop PC using double precision math. */
spmSInt32 SSDLS_CalculateAbsoluteTimeToMilliseconds( spmSInt32 absTime )
{
    double exponent = ((double) absTime) / (1200 * 65536);
    double timeSeconds = pow( 2.0, exponent );
    int milliseconds = (int) ((1000.0 * timeSeconds) + 0.5);
    if( milliseconds > 32767 )
        milliseconds = 32767;
    return milliseconds;
}

/*******************************************************************************/
/* Generate sAbsTimeToMilliseconds table. */
int mainX( void )
{
    int i;
    printf("static const short sAbsTimeToMilliseconds[] = {\n");
    /* Generate a table of values. */
    for( i = 0; i<SSDLS_ABSTIME_MILLIS_SIZE; i++ )
    {
        long absTime = (i << 24) + SSDLS_ABSTIME_START;
        int milliseconds = SSDLS_CalculateAbsoluteTimeToMilliseconds( absTime );
        printf("  %5d, /* %d, 0x%08lX */\n", milliseconds, i, absTime );
    }
    printf("};\n" );
    return 0;
}

/*******************************************************************************/
/* Test SSDLS_ConvertAbsoluteTimeToMilliseconds */
int mainY( void )
{
    int i;
    int absTime, lookupMillis, calcMillis;
    int delta, delta2;
    absTime = -103960656; /* 400 msec */
    lookupMillis = SSDLS_ConvertAbsoluteTimeToMilliseconds( absTime );
    calcMillis = SSDLS_CalculateAbsoluteTimeToMilliseconds( absTime );
    printf("  abst = 0x%08X = %d, lookup = %d, calc = %d\n",
           absTime, absTime, lookupMillis, calcMillis );

    /* test lots of absolute time values. */
    for( i = 0; i<=800; i++ )
    {
        absTime = (i << 22) + 0x80000000;

        lookupMillis = SSDLS_ConvertAbsoluteTimeToMilliseconds( absTime );
        calcMillis = SSDLS_CalculateAbsoluteTimeToMilliseconds( absTime );

        printf("%3d:  abst = 0x%08X = %8d, lookup = %5d, calc = %5d",
               i, absTime, absTime, lookupMillis, calcMillis );
        delta = lookupMillis - calcMillis;
        delta2 = delta * delta;
        if( delta2 > 500 )
        {
            printf(", delta = %d", delta );
            if( delta < 0 )
            {
                printf(", negative!" );
            }
        }
        printf("\n");
    }
    return 0;
}
#endif

/*******************************************************************************/
/**
 * Convert from DLS2 Absolute pitch to pitchOctave.
 * DLS2 AbsolutePitch = ((1200*log2(freq/440)) + 6900) * 65536
 * ME pitchOctave = log2(freq/FREQUENCY_OCTAVE_ZERO) * 65536
 */      
static spmSInt32 SSDLS_ConvertAbsolutePitchToPitchOctave( spmSInt32 absPitch )
{
    return ((absPitch + 600) / 1200) + 524288;
}

/*******************************************************************************/
/**
 * Convert from DLS2 Relative pitch to pitchOctave.
 * DLS2 AbsolutePitch = ((1200*log2(freq/F)) * 65536
 * ME pitchOctave = log2(freq/F) * 65536
 */      
static spmSInt32 SSDLS_ConvertRelativePitchToPitchOctave( spmSInt32 absPitch )
{
    return (absPitch + 600) / 1200;
}

/*******************************************************************************/
spmSInt32 DLSParser_ConvertArticulationData( const DLS_Articulation_t *articulation,
                                            int sampleRate )
{
    int sum;
    int msec;
    spmSInt32 scale = articulation->scale;
    spmSInt32 converted;

    switch( articulation->token )
    {

    case CONN_Z_TUNING:
    case CONN_Z_VIB_LFO_TO_PITCH:
    case CONN_Z_MOD_LFO_TO_PITCH:
    case CONN_Z_EG2_TO_PITCH:
    case CONN_Z_MOD_LFO_CC1_TO_PITCH:
    case CONN_Z_VIB_LFO_CC1_TO_PITCH:
    case CONN_Z_MOD_LFO_CPR_TO_PITCH:
    case CONN_Z_VIB_LFO_CPR_TO_PITCH:
        converted = SSDLS_ConvertRelativePitchToPitchOctave( scale );
        break;

    case CONN_Z_LFO_FREQUENCY:
    case CONN_Z_VIB_FREQUENCY:
        converted = SSDLS_ConvertAbsolutePitchToPitchOctave( scale );
        break;


    case CONN_Z_VIB_STARTDELAY:
    case CONN_Z_LFO_STARTDELAY:
        msec = SSDLS_ConvertAbsoluteTimeToMilliseconds( scale );
        /* Unit analysis: ticks = (msec * frames/second) / (frames/tick * msec/second) */
        /* This should not overflow because msec<10000 and sampleRate<96000 */
        converted = (short)((msec * sampleRate) / (SS_FRAMES_PER_BUFFER * MSEC_PER_SECOND));
        break;

    case CONN_Z_EG1_DELAYTIME:
    case CONN_Z_EG1_HOLDTIME:
    case CONN_Z_EG2_DELAYTIME:
    case CONN_Z_EG2_HOLDTIME:
        /* Convert to envelope increment. */
        {
            int controlRate = sampleRate >> SS_FRAMES_PER_CONTROL_LOG2;
            int msec = SSDLS_ConvertAbsoluteTimeToMilliseconds( scale );
            if( msec > 32767 ) msec = 32767;
            else if ( msec < 0 ) msec = 0;
            converted = ADSR_MSecToIncrement( msec, controlRate );
        }
        break;

    case CONN_Z_EG1_ATTACKTIME:
    case CONN_Z_EG1_DECAYTIME:
    case CONN_Z_EG1_RELEASETIME:
    case CONN_Z_EG2_ATTACKTIME:
    case CONN_Z_EG2_DECAYTIME:
    case CONN_Z_EG2_RELEASETIME:
        /* Convert to envelope time. */
        {
            int msec = SSDLS_ConvertAbsoluteTimeToMilliseconds( scale );
            if( msec > 32767 ) msec = 32767;
            else if ( msec < 0 ) msec = 0;
            converted = (short) msec;
        }
        break;

    case CONN_Z_EG1_SUSTAINLEVEL:
    case CONN_Z_EG2_SUSTAINLEVEL:
        /* DLS specifies full sustain level as 1024, which matches the level for ME2000 */
        sum = (spmSInt16) (scale >> 16);
        if( sum > ADSR_SUSTAIN_MAX )
            sum = ADSR_SUSTAIN_MAX;
        /* Square value to approximate dB conversion. */
        sum = (sum * sum) >> 10;
        converted = (spmSInt16) sum;
        break;

    case CONN_Z_KEY_TO_PITCH:
        {
            int cents = scale >> 23;
            converted = (unsigned char) (((1 << SPMIDI_PITCH_SCALAR_SHIFT) * cents) / 100);
        }
        break;

    default:
        converted = scale;
        break;
    }

    return converted;
}

#endif /* #if SPMIDI_ME3000 */
