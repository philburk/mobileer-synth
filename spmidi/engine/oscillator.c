/* $Id: oscillator.c,v 1.40 2007/11/13 07:38:45 philjmsl Exp $ */
/**
 *
 * Oscillator with multiple waveforms.
 * Each waveform is implemented using a separate function.
 * The function is chosen when the oscillator is started.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/engine/oscillator.h"

/** Enable variable slope waveforms, which reduces
 * high harmonics and aliasing in sawtooth and square waves.
 */
#define USE_MOLDED_WAVEFORMS  (1)

#ifdef WIN32
#include <math.h>
#ifndef M_PI
#define M_PI   (3.1415926535897932384626433832795)
#endif
#define TWO_PI   (2.0 * M_PI)
#endif

/* Set to 1 to enable debugging macros. */
#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#ifndef FALSE
#define FALSE (0)
#define TRUE  (1)
#endif


/********************************************************************/
/* Lookup table for sine wave generator.
 * Contains one full cycle scaled to range of signed short.
 */
#define TABLE_NUM_ENTRIES_LOG2     (7)
#define TABLE_NUM_ENTRIES          (1 << TABLE_NUM_ENTRIES_LOG2)
#define TABLE_NUM_ENTRIES_GUARDED  (TABLE_NUM_ENTRIES + 1)

static const short sSineTable[] =
    {
        0, 1607, 3211, 4807, 6392, 7961, 9511, 11038,
        12539, 14009, 15446, 16845, 18204, 19519, 20787, 22004,
        23169, 24278, 25329, 26318, 27244, 28105, 28897, 29621,
        30272, 30851, 31356, 31785, 32137, 32412, 32609, 32727,
        32766, 32727, 32609, 32412, 32137, 31785, 31356, 30851,
        30272, 29621, 28897, 28105, 27244, 26318, 25329, 24278,
        23169, 22004, 20787, 19519, 18204, 16845, 15446, 14009,
        12539, 11038, 9511, 7961, 6392, 4807, 3211, 1607,
        0, -1607, -3211, -4807, -6392, -7961, -9511, -11038,
        -12539, -14009, -15446, -16845, -18204, -19519, -20787, -22004,
        -23169, -24278, -25329, -26318, -27244, -28105, -28897, -29621,
        -30272, -30851, -31356, -31785, -32137, -32412, -32609, -32727,
        -32766, -32727, -32609, -32412, -32137, -31785, -31356, -30851,
        -30272, -29621, -28897, -28105, -27244, -26318, -25329, -24278,
        -23169, -22004, -20787, -19519, -18204, -16845, -15446, -14009,
        -12539, -11038, -9511, -7961, -6392, -4807, -3211, -1607,
        0,
    };

/********************************************************************/
#define INITIAL_RANDOM_SEED   (23456)
static spmUInt32 sRandomState; /* Initialized to INITIAL_RANDOM_SEED by Osc_Init() */

/* Calculate pseudo-random 32 bit number based on linear congruential method. */
#define NEXT_RANDOM  (sRandomState = (sRandomState * 196314165) + 907633515)

/********************************************************************/
/** Use macros to define oscillator functions so that we can
 * use the same code in the modulated and unmodulated version.
 * Define common beginning and ending of every oscillator function.
 */

#define OSC_PREAMBLE \
    FXP31  oscOutput; \
    int    is; \
    FXP31  oscPhaseInc = osc->phaseInc; \
    FXP31  oscPhase = osc->phase; \
    is = SS_FRAMES_PER_BLOCK; \
    while( --is >= 0) \
    {
    
#define OSC_PREAMBLE_NOMOD \
    OSC_PREAMBLE \
        FXP31 phase = oscPhase;

#define OSC_PREAMBLE_MOD \
    OSC_PREAMBLE \
        FXP31 phase = oscPhase  + (*modulator++ << 2);

#define OSC_POSTAMBLE \
        *output++ = FXP31_MULT( oscOutput, amplitude ); \
    } \
    (void) modulator; \
    osc->phase = oscPhase;
    
#define OSC_POSTAMBLE_INC \
        oscPhase =oscPhase + oscPhaseInc; /* FXP31 automatically wraps. */ \
        OSC_POSTAMBLE
        

/********************************************************************/
/* Use lookup table to generate equivalent of the following code.
        ((FXP31) (FXP31_MAX_VALUE * sin( (M_PI * ((double)phase / OSC_PHASE_INC_NYQUIST)) )))
*/
#define SINE_INTERP_ORIGINAL \
    { \
        /* Convert phase to integer index into sine table. */ \
        int index = ((spmUInt32) phase) >> (32 - TABLE_NUM_ENTRIES_LOG2); \
        const spmSInt16 *lowSinePtr = &sSineTable[ index ]; \
        FXP31 lowSine = *lowSinePtr++; \
        FXP31 highSine = *lowSinePtr; \
        /* Convert phase to fraction between entries in sine table. */ \
        spmUInt32 tableFraction = (((spmUInt32) phase) << TABLE_NUM_ENTRIES_LOG2) >> TABLE_NUM_ENTRIES_LOG2; \
        spmSInt32 tableDelta = (highSine - lowSine); \
        tableFraction = tableFraction >> (16 - TABLE_NUM_ENTRIES_LOG2); \
        oscOutput = (lowSine << 16)  + (tableFraction * tableDelta); \
    }

#define SINE_INTERP \
    { \
        /* Convert phase to integer index into sine table. */ \
        int index = ((spmUInt32) phase) >> (32 - TABLE_NUM_ENTRIES_LOG2); \
        const spmSInt16 *lowSinePtr = &sSineTable[ index ]; \
        FXP31 lowSine = *lowSinePtr++; \
        FXP31 highSine = *lowSinePtr; \
        /* Convert phase to fraction between entries in sine table. */ \
        spmUInt32 tableFraction = (((spmUInt32) phase) << TABLE_NUM_ENTRIES_LOG2) >> 16; \
        spmSInt32 tableDelta = (highSine - lowSine); \
        oscOutput = (lowSine << 16)  + (tableFraction * tableDelta); \
    }

void Osc_Next_Sine( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_NOMOD
    SINE_INTERP
    OSC_POSTAMBLE_INC
}

void Osc_Next_SinePM( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_MOD;
    SINE_INTERP
    OSC_POSTAMBLE_INC
}

/********************************************************************/
void OSC_Next_Triangle( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_NOMOD
    {
        FXP31 phasePositive = phase ^ (phase >> 31); /* Fast pseudo-absolute-value. */
        oscOutput = (phasePositive - 0x40000000) << 1;
    }
    OSC_POSTAMBLE_INC
}

void OSC_Next_TrianglePM( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_MOD

    {
        FXP31 phasePositive = phase ^ (phase >> 31); /* Fast pseudo-absolute-value. */
        oscOutput = (phasePositive - 0x40000000) << 1;
    }

    OSC_POSTAMBLE_INC
}

/********************************************************************/
#if USE_MOLDED_WAVEFORMS
#define OSC_SAWTOOTH \
    oscOutput = phase; \
    if( phase < -osc->shared.square.level ) \
    { \
        oscOutput = ((-OSC_PHASE_INC_NYQUIST - phase) >> 8) * osc->shared.square.scale; \
    } \
    else if( phase > osc->shared.square.level ) \
    { \
        oscOutput = ((OSC_PHASE_INC_NYQUIST - phase) >> 8) * osc->shared.square.scale; \
    }
#else
#define OSC_SAWTOOTH \
    oscOutput = phase;
#endif

void OSC_Next_Sawtooth( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_NOMOD
    OSC_SAWTOOTH
    OSC_POSTAMBLE_INC
}

void OSC_Next_SawtoothPM( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_MOD
    OSC_SAWTOOTH
    OSC_POSTAMBLE_INC
}

/********************************************************************/
#if USE_MOLDED_WAVEFORMS
#define OSC_SQUARE \
    { \
        int negative = 0; \
        if( phase < 0 ) \
        { \
            negative = 1; \
            phase += 0x80000000; \
        } \
        if( phase < osc->shared.square.level ) \
        { \
            oscOutput = (((phase >> 8) * osc->shared.square.scale) - (FXP31_MAX_VALUE/2)) << 1; \
        } \
        else oscOutput = FXP31_MAX_VALUE; \
        if( negative ) oscOutput ^= -1; \
    }
#else
#define OSC_SQUARE \
    oscOutput = ( phase < 0 ) ? FXP31_MAX_VALUE : FXP31_MIN_VALUE;
#endif

void OSC_Next_Square( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_NOMOD
    OSC_SQUARE
    OSC_POSTAMBLE_INC
}

void OSC_Next_SquarePM( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_MOD
    OSC_SQUARE
    OSC_POSTAMBLE_INC
}

/* These harsh square waves have aliasing artifacts and are designed for use in shrill
 * "in your face" ringtones that you can hear in a loud airport.
 */
void OSC_Next_SquareHarsh( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_NOMOD
    oscOutput = ( phase < 0 ) ? FXP31_MAX_VALUE : FXP31_MIN_VALUE;
    OSC_POSTAMBLE_INC
}

void OSC_Next_SquareHarshPM( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE_MOD
    oscOutput = ( phase < 0 ) ? FXP31_MAX_VALUE : FXP31_MIN_VALUE;
    OSC_POSTAMBLE_INC
}

/********************************************************************
 * Full frequency noise, one random value per sample.
 */
void OSC_Next_WhiteNoise( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    FXP31 oscOutput;
    int is;
    (void) modulator;
    (void) osc;

    for ( is = 0; is < SS_FRAMES_PER_BLOCK; is++ )
    {
        oscOutput = (FXP31) NEXT_RANDOM;
        output[is] = FXP31_MULT( oscOutput, amplitude );
    }
}

/********************************************************************
 * Generate random segments by interpolating between random values.
 */
void OSC_Next_RedNoise( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE
    
    FXP31 nextPhase = oscPhase + oscPhaseInc; /* FXP31 automatically wraps. */

    if ( (oscPhase <= 0) && (nextPhase > 0) )
    {
        FXP31 next;
        osc->shared.interp.previous += osc->shared.interp.delta << 16;
        next = (FXP31) NEXT_RANDOM;
        osc->shared.interp.delta = (next >> 17) - (osc->shared.interp.previous >> 17);
    }
    oscOutput = osc->shared.interp.previous + ((((spmUInt32)nextPhase) >> 16) * osc->shared.interp.delta);

    oscPhase = nextPhase;
    OSC_POSTAMBLE
}

/********************************************************************
 * Output a random impulse if random value exceeds threshold.
 * Average frequency of pulses is frequency of oscillator.
 */
void OSC_Next_Particle( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    FXP31 oscOutput;
    int is;
    FXP31 oscPhaseInc = osc->phaseInc;
    for ( is = 0; is < SS_FRAMES_PER_BLOCK; is++ )
    {
        oscOutput = (NEXT_RANDOM < (spmUInt32) oscPhaseInc) ?
                    (FXP31) NEXT_RANDOM : 0;

        output[is] = FXP31_MULT( oscOutput, amplitude );
    }
    (void) modulator;
}

/********************************************************************
 * Also known as "Sample and Hold"
 */
void OSC_Next_RandomHold( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator )
{
    OSC_PREAMBLE
    
    FXP31 nextPhase =oscPhase + oscPhaseInc; /* FXP31 automatically wraps. */

    if ( (oscPhase <= 0) && (nextPhase > 0) )
    {
        /* Pick new random phase when phase crosses zero. */
        osc->shared.interp.previous = oscOutput = (FXP31) NEXT_RANDOM;
    }
    else
    {
        /* Hold previous value. */
        oscOutput = osc->shared.interp.previous;
    }
    oscPhase = nextPhase;

    OSC_POSTAMBLE
}

/********************************************************************/

static SS_OscillatorNextProc *sNextProcs[NUM_WAVEFORMS];

void Osc_Init( void )
{
    /*
    ** The function table is initialized by code to prevent
    ** linker warnings for ARM compilers.
    */
    sNextProcs[SINE_PM] = (SS_OscillatorNextProc *) Osc_Next_SinePM;
    sNextProcs[TRIANGLE_PM] = (SS_OscillatorNextProc *) OSC_Next_TrianglePM;
    sNextProcs[SAWTOOTH_PM] = (SS_OscillatorNextProc *) OSC_Next_SawtoothPM,
    sNextProcs[SQUARE_PM] = (SS_OscillatorNextProc *) OSC_Next_SquarePM;
    sNextProcs[SINE] = (SS_OscillatorNextProc *) Osc_Next_Sine;
    sNextProcs[TRIANGLE] = (SS_OscillatorNextProc *) OSC_Next_Triangle;
    sNextProcs[SAWTOOTH] = (SS_OscillatorNextProc *) OSC_Next_Sawtooth;
    sNextProcs[SQUARE] = (SS_OscillatorNextProc *) OSC_Next_Square;
    sNextProcs[WHITENOISE] = (SS_OscillatorNextProc *) OSC_Next_WhiteNoise;
    sNextProcs[REDNOISE] = (SS_OscillatorNextProc *) OSC_Next_RedNoise;
    sNextProcs[RANDOMHOLD] = (SS_OscillatorNextProc *) OSC_Next_RandomHold;
    sNextProcs[PARTICLE] = (SS_OscillatorNextProc *) OSC_Next_Particle;
    sNextProcs[SQUARE_HARSH] = (SS_OscillatorNextProc *) OSC_Next_SquareHarsh;
    sNextProcs[SQUARE_HARSH_PM] = (SS_OscillatorNextProc *) OSC_Next_SquareHarshPM;
#if SPMIDI_ME2000
    sNextProcs[WAVETABLE] = (SS_OscillatorNextProc *) OSC_Next_SquareHarshPM;
#endif

    sRandomState = INITIAL_RANDOM_SEED;
}

/********************************************************************/
void Osc_Term( void )
{}


/***********************************************************************/
#if SPMIDI_ME2000

#if SPMIDI_USE_REGIONS
/*******************************************************************************/
/**
 * Find first region in instrument that matches the pitch and velocity.
 */
WaveSetRegion_t *Osc_FindMatchingRegion( const WaveSet_t *waveSet, int pitch, int velocity )
{
    WaveSetRegion_t *candidate = NULL;
    WaveSetRegion_t *match = NULL;
    WaveSetRegion_t *bestTableIgnoringVelocity = NULL;
    int i;
    (void) velocity;

    //PRTMSGNUMD("Osc_FindMatchingRegion pitch = ", pitch );
    //PRTMSGNUMH("Osc_FindMatchingRegion numTables = ", waveSet->numTables );
    for( i=0; i<waveSet->numTables; i++ )
    {
        candidate = &(waveSet->regions[i]);
        //PRTMSGNUMD("Osc_FindMatchingRegion lowPitch = ", candidate->lowPitch );
        //PRTMSGNUMD("Osc_FindMatchingRegion highPitch = ", candidate->highPitch );
        if( (pitch >= candidate->lowPitch) && (pitch <= candidate->highPitch))
        {
            bestTableIgnoringVelocity = candidate;
            //PRTMSGNUMH("Osc_FindMatchingRegion bestTableIgnoringVelocity = ", bestTableIgnoringVelocity );
            if( (velocity >= candidate->lowVelocity) && (velocity <= candidate->highVelocity) )
            {
                match = candidate;
                //PRTMSGNUMH("Osc_FindMatchingRegion matching region = ", match );
                break;
            }
        }
    }
    
    // Just in case no match for velocity range.
    if( match == NULL )
    {
        match = bestTableIgnoringVelocity;
    }
    //PRTMSGNUMH("Osc_FindMatchingRegion using table = ", match->table );
    return match;
}
#else
WaveTable_t *Osc_SelectNearestWaveTable( const WaveSet_t *waveSet, FXP16 octavePitch, int velocityNote )
{
    /* Scan for closest table in pitch to starting note. */
    WaveTable_t **tablePtr = waveSet->tables;
    int i;
    WaveTable_t *bestTable = NULL;
    spmSInt32 bestDelta = 0x7FFFFFFF;
    WaveTable_t *bestTableIgnoringVelocity = NULL;
    spmSInt32 bestDeltaIgnoringVelocity = 0x7FFFFFFF;


    for ( i = 0; i < waveSet->numTables; i++ )
    {
        WaveTable_t *nextTable = *tablePtr++;
        PitchOctave pitch = nextTable->basePitch;
        PitchOctave pitchDelta = (pitch - octavePitch);
        int highVelocityWave = nextTable->highVelocity;

        if ( pitchDelta < 0 )
        {
            pitchDelta = -pitchDelta; /* Absolute value. */
        }

        /* Is this the closest pitch so far if we ignore velocity? */
        if ( pitchDelta < bestDeltaIgnoringVelocity )
        {
            bestTableIgnoringVelocity = nextTable;
            bestDeltaIgnoringVelocity = pitchDelta;
        }

        if( highVelocityWave == 0 )
        {
            highVelocityWave = 127;
        }

        /* Is this the closest pitch so far that is in velocity range? */
        if ( (pitchDelta < bestDelta) &&
             (velocityNote >= nextTable->lowVelocity) &&
             (velocityNote <= highVelocityWave)
           )
        {
            bestTable = nextTable;
            bestDelta = pitchDelta;
        }
    }

    // Just in case no match for velocity range.
    if( bestTable == NULL )
    {
        bestTable = bestTableIgnoringVelocity;
    }

#if 0
    PRTMSG( "pitch: note = " ); PRTNUMH( octavePitch );
    PRTMSG( "wave = " ); PRTNUMH( bestTable->basePitch );
    PRTMSG( "; vel: note = " ); PRTNUMD( velocityNote );
    PRTMSG( "wave = " ); PRTNUMD( bestTable->lowVelocity );
    PRTMSG( " to " ); PRTNUMD( bestTable->highVelocity ); PRTMSG( "\n" );
#endif

    return bestTable;
}
#endif
#endif /* SPMIDI_ME2000 */

/***********************************************************************/
/** Called when a NoteOn is processed.
 * Set the pitch.
 * For Sawtooth and square waves we also calculate variable slope
 * parameters based on pitch.
 */
void Osc_Start( const Oscillator_Preset_t *preset, Oscillator_t *osc,
                FXP16 octavePitch, FXP16 srateOffset, WaveSetRegion_t *waveSetRegion )
{
    int divisor;
    int bestWaveForm;

    osc->phase = 0;

#if SPMIDI_ME2000
    if( preset->waveform == WAVETABLE )
    {
        Osc_SetWavePitch( preset, osc, octavePitch, srateOffset, waveSetRegion );
    }
    else
#else
    (void) waveSetRegion;
#endif
    {
        Osc_SetPitch( preset, osc, octavePitch, srateOffset );
    }

    bestWaveForm = preset->waveform;

    /* If we are too close to the Nyquist rate then just use a sine wave.
     * All the higher partials will be aliased anyway.
     */
    // PRTMSGNUMH("phaseInc = ", osc->phaseInc );
    if( osc->phaseInc > (OSC_PHASE_INC_NYQUIST >> 2) )
    {
        switch( preset->waveform )
        {
        case TRIANGLE_PM:
        case SAWTOOTH_PM:
        case SQUARE_PM:
            DBUGMSG("use SINE_PM\n" );
            bestWaveForm = SINE_PM;
            break;

        case TRIANGLE:
        case SAWTOOTH:
        case SQUARE:
            DBUGMSG("use SINE\n" );
            bestWaveForm = SINE;
            break;

        default:
            break;
        }
    }

    /* Set function that creates the desired waveform. */
    if( bestWaveForm < NUM_WAVEFORMS )
    {
        osc->nextProc = sNextProcs[ bestWaveForm ];
    }
    else
    {
        osc->nextProc = OSC_Next_Sawtooth;
    }

    /* For low frequencies, use straight square.
     * For higher frequencies, use a more sloped waveform.
     */
    /* Make sure shift will stay in range. */
    switch ( bestWaveForm )
    {
    case SAWTOOTH:
    case SAWTOOTH_PM:
        {
            const FXP31 PHASE_INC_LIMIT = (OSC_PHASE_INC_NYQUIST >> 4);
            FXP31 clippedInc = (osc->phaseInc > PHASE_INC_LIMIT) ? PHASE_INC_LIMIT : osc->phaseInc;
            osc->shared.square.level = OSC_PHASE_INC_NYQUIST - ((clippedInc) << 3);
        }
        divisor = (OSC_PHASE_INC_NYQUIST - osc->shared.square.level) >> 8;
        if ( divisor <= 0 )
        {
            divisor = 1;
        }
        osc->shared.square.scale = osc->shared.square.level / divisor; /* DIVIDE - noteOn */
        break;

    case SQUARE:
    case SQUARE_PM:
/* This determines the frequency at which a square wave becomes a triangle wave.
 * Original value was 4. Lower values are brighter.
 */
#define SQUARE_SHIFT  (4)
        if ( osc->phaseInc > (OSC_PHASE_INC_NYQUIST >> SQUARE_SHIFT))
        {
            osc->shared.square.level = OSC_PHASE_INC_NYQUIST;
        }
        else
        {
            osc->shared.square.level = osc->phaseInc << SQUARE_SHIFT;
        }
        divisor = osc->shared.square.level >> 8;
        if ( divisor <= 0 )
        {
            divisor = 1;
        }
        osc->shared.square.scale = OSC_PHASE_INC_NYQUIST / divisor; /* DIVIDE - noteOn */
        break;

#if SPMIDI_ME2000

    case WAVETABLE:
        /* Prevent pops by clearing at start of oscillation. */
        osc->shared.wave.previous = 0;
        osc->shared.wave.next = 0;
        osc->shared.wave.sampleIndex = 0;

        /* Determine when the wave oscillator should stop and decide what to do next. */
        if ( waveSetRegion->table->loopEnd > 0 )
        {
            /* Stop when we get to the end of the loop
             * so we can jump back to start of loop. */
            osc->shared.wave.endAt = waveSetRegion->table->loopEnd;
        }
        else
        {
            /* No loop so stop at end of the sample. */
            osc->shared.wave.endAt = waveSetRegion->table->numSamples;
        }
        
        break;
#endif

    }
}

/***********************************************************************/
/**
 * Set pitch of oscillator by calculating phaseIncrement.
 * @param srateOffset Add an offset if the sample rate of the calculation is less than SS_BASE_SAMPLE_RATE.
 */
void Osc_SetPitch( const Oscillator_Preset_t *preset, Oscillator_t *osc,
                   FXP16 octavePitch, FXP16 srateOffset )
{
    if ( preset->flags & OSC_FLAG_ABSOLUTE_PITCH )
    {
        octavePitch = preset->pitchControl; /* absolute pitch, ignore noteOn pitch */
    }
    else
    {
        octavePitch += preset->pitchControl; /* pitch offset relative to noteOn pitch */
    }
    octavePitch += srateOffset;
    osc->phaseInc = SPMUtil_OctaveToPhaseIncrement( octavePitch );
}

#if SPMIDI_ME2000
/***********************************************************************/
/**
 * Set pitch of oscillator by calculating phaseIncrement.
 * @param srateOffset Add an offset if the sample rate of the calculation is less than SS_BASE_SAMPLE_RATE.
 */
void Osc_SetWavePitch( const Oscillator_Preset_t *preset, Oscillator_t *osc,
                   FXP16 octavePitch, FXP16 srateOffset, WaveSetRegion_t *waveSetRegion )
{
    FXP16 wavePitch;
    if ( preset->flags & OSC_FLAG_ABSOLUTE_PITCH )
    {
        octavePitch = preset->pitchControl; /* absolute pitch, ignore noteOn pitch */
    }
    else
    {
        octavePitch += preset->pitchControl; /* pitch offset relative to noteOn pitch */
    }
    octavePitch += srateOffset;

    /* SS_WAVE_OCTAVE_BASE is calculated in "spmidi_synth_util.c".
     * It is the octavePitch that will result in an equivalent value of 1.0 for the wave phase increment.
     */
#define SS_WAVE_OCTAVE_BASE (0x000c65aa)
    wavePitch = SS_WAVE_OCTAVE_BASE
                      + waveSetRegion->table->sampleRateOffset /* Adjust because samples are recorded at various rates. */
                      + octavePitch
                      - waveSetRegion->basePitch;
    osc->phaseInc = SPMUtil_OctaveToPhaseIncrement( wavePitch );
}
#endif

#if SPMIDI_SUPPORT_LOADING
/***********************************************************************/
/** Parse preset information from the byte stream.
 */
unsigned char *Osc_Define( Oscillator_Preset_t *preset, unsigned char *p )
{
    preset->waveform = *p++;
    preset->flags = *p++;
    p = SS_ParseLong( &preset->pitchControl, p );
    return p;
}
#endif /* SPMIDI_SUPPORT_LOADING */

/***********************************************************************/
/************** Tests and Tools ****************************************/
/***********************************************************************/
#if 0
/* Render oscillator to file for external viewing with wave editor. */
int main( void )
{
    int i;
    FILE *fid;
    FXP31 buffer[SS_FRAMES_PER_BLOCK];
    Oscillator_Preset_t PRESET = { TRIANGLE, 0, 0 };
    Oscillator_Info_t INFO;
    Oscillator_t OSC = { 0 };

    printf("Test OSC\n");

    Osc_SetPitch( &INFO, &OSC, 0x00DC000, 0 );

    /* Open file. */
    fid = fopen( "rendered_osc.raw", "wb" );
    if ( fid == NULL )
    {
        printf("Can't open output file rendered_osc.raw\n" );
        return 1;
    }

    for ( i = 0; i < 10; i++ )
    {
        int is;
        (*INFO.nextProc)( &OSC, FXP31_MAX_VALUE, buffer, NULL, 0 );
        for ( is = 0; is < SS_FRAMES_PER_BLOCK; is++ )
        {
            short sample = (short) (buffer[is] >> 16);
            fwrite( &sample, sizeof(short), 1, fid );
        }
    }

    fclose( fid );

    printf("Test complete.\n");

    return 0;
}
#endif

#if 0
/* Generate sSineTable for lookup. */
int main( void )
{
    int i, j, index;
    short sineValue;
    printf("static const short sSineTable[] = {\n" );
    /* Generate an octave table for tuning. */

    index = 0;
    for ( i = 0; i < TABLE_NUM_ENTRIES_GUARDED / 8; i++ )
    {
        for ( j = 0; j < 8; j++ )
        {

            sineValue = FXP15_MAX_VALUE * sin( (TWO_PI * index) / TABLE_NUM_ENTRIES );
            printf(" %6d,", sineValue );
            index += 1;
        }
        printf("\n" );
    }
    printf("      0,\n");
    printf("};\n" );
}
#endif
