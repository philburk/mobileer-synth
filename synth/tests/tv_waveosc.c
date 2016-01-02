/* $Id: tv_waveosc.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test vector for wavetable oscillator.
 * Setup wave oscillator using a small hand built data set.
 * Run oscillator and capture output and compare with known good output.
 *
 * Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/engine/oscillator.h"

int numGood = 0;
int numBad = 0;

/* Set GEN_DATA to (1) when generating test vector data with known good code. */
/* Set GEN_DATA to (0) when testing code. */
#define GEN_DATA   (0)

/* Set to 0 for bit exact testing.
 * Set to larger number to allow minor differences in low bits. */
#define TOLERANCE  (0x00000000)

/* Phase increment for advancing one sample per frame. */
#define NOMINAL_PHASE_INCREMENT   (0x01000000)

/* Define 16 bit wavetable. */
short sSamples[] = {
    (short) 0x0000, (short) 0x0654, (short) 0x69A3, (short) 0x8533, // attack
    (short) 0xC3E8, (short) 0x177B, (short) 0x7E92, (short) 0x8037, // loop
    (short) 0xC077, (short) 0x2BCC, (short) 0x3912, (short) 0xF1CC, // release
    (short) 0x0000, (short) 0x0000 // tail
};
#define NUM_SAMPLES  (sizeof(sSamples) / sizeof(short))

#define NUM_BLOCKS_ON       (3)
#define NUM_BLOCKS_OFF      (3)
#define NUM_BLOCKS          (NUM_BLOCKS_ON + NUM_BLOCKS_OFF)

/********************************************************************************/
/* These values are generated when GEN_DATA is set to (1). Do not modify by hand. */
FXP31 sExpectedOutputA[] = {
   0x00000000, /* 0 */   0x00000000, /* 1 */   0x00000000, /* 2 */   0x06540000, /* 3 */
   0x69A30000, /* 4 */   0x85330000, /* 5 */   0xC3E80000, /* 6 */   0x177B0000, /* 7 */
   0x7E920000, /* 8 */   0x80370000, /* 9 */   0xC3E80000, /* 10 */   0x177B0000, /* 11 */
   0x7E920000, /* 12 */   0x80370000, /* 13 */   0xC3E80000, /* 14 */   0x177B0000, /* 15 */
   0x7E920000, /* 16 */   0x80370000, /* 17 */   0xC3E80000, /* 18 */   0x177B0000, /* 19 */
   0x7E920000, /* 20 */   0x80370000, /* 21 */   0xC3E80000, /* 22 */   0x177B0000, /* 23 */
   0x7E920000, /* 24 */   0x80370000, /* 25 */   0xC3E80000, /* 26 */   0x177B0000, /* 27 */
   0x7E920000, /* 28 */   0x80370000, /* 29 */   0xC0770000, /* 30 */   0x2BCC0000, /* 31 */
   0x39120000, /* 32 */   0xF1CC0000, /* 33 */   0x00000000, /* 34 */   0x00000000, /* 35 */
   0x00000000, /* 36 */   0x00000000, /* 37 */   0x00000000, /* 38 */   0x00000000, /* 39 */
   0x00000000, /* 40 */   0x00000000, /* 41 */   0x00000000, /* 42 */   0x00000000, /* 43 */
   0x00000000, /* 44 */   0x00000000, /* 45 */   0x00000000, /* 46 */   0x00000000, /* 47 */
};
FXP31 sExpectedOutputB[] = {
   0x00000000, /* 0 */   0x00000000, /* 1 */   0x00000000, /* 2 */   0x00000000, /* 3 */
   0x00000000, /* 4 */   0x032A0000, /* 5 */   0x06540000, /* 6 */   0x37FB8000, /* 7 */
   0x69A30000, /* 8 */   0xF76B0000, /* 9 */   0x85330000, /* 10 */   0xA48D8000, /* 11 */
   0xC3E80000, /* 12 */   0xEDB18000, /* 13 */   0x177B0000, /* 14 */   0x4B068000, /* 15 */
   0x7E920000, /* 16 */   0xFF648000, /* 17 */   0x80370000, /* 18 */   0xA20F8000, /* 19 */
   0xC3E80000, /* 20 */   0xEDB18000, /* 21 */   0x177B0000, /* 22 */   0x4B068000, /* 23 */
   0x7E920000, /* 24 */   0xFF648000, /* 25 */   0x80370000, /* 26 */   0xA20F8000, /* 27 */
   0xC3E80000, /* 28 */   0xEDB18000, /* 29 */   0x177B0000, /* 30 */   0x4B068000, /* 31 */
   0x7E920000, /* 32 */   0xFF648000, /* 33 */   0x80370000, /* 34 */   0xA0570000, /* 35 */
   0xC0770000, /* 36 */   0xF6218000, /* 37 */   0x2BCC0000, /* 38 */   0x326F0000, /* 39 */
   0x39120000, /* 40 */   0x156F0000, /* 41 */   0xF1CC0000, /* 42 */   0xF8E60000, /* 43 */
   0x00000000, /* 44 */   0x00000000, /* 45 */   0x00000000, /* 46 */   0x00000000, /* 47 */
};
FXP31 sExpectedOutputC[] = {
   0x00000000, /* 0 */   0x00000000, /* 1 */   0x00000000, /* 2 */   0x039DAFA0, /* 3 */
   0x30E33D2A, /* 4 */   0x285EF040, /* 5 */   0x8E28373A, /* 6 */   0xC3E80000, /* 7 */
   0x0B8A0304, /* 8 */   0x611D2396, /* 9 */   0xED3A9068, /* 10 */   0x9D397CD6, /* 11 */
   0xDBC8ABAC, /* 12 */   0x2635071E, /* 13 */   0x7E920000, /* 14 */   0xA48ED89C, /* 15 */
   0xB0907ABA, /* 16 */   0xF3A95758, /* 17 */   0x43A9155A, /* 18 */   0x35E64834, /* 19 */
   0x89E27EF2, /* 20 */   0xC3E80000, /* 21 */   0x0B8A0304, /* 22 */   0x611D2396, /* 23 */
   0xED3A9068, /* 24 */   0x9D397CD6, /* 25 */   0xDBC8ABAC, /* 26 */   0x2635071E, /* 27 */
   0x7E920000, /* 28 */   0xA48ED89C, /* 29 */   0xAE1B3680, /* 30 */   0xFDCBA9E8, /* 31 */
   0x317C3DC4, /* 32 */   0x24B50428, /* 33 */   0xF3D369A8, /* 34 */   0x00000000, /* 35 */
   0x00000000, /* 36 */   0x00000000, /* 37 */   0x00000000, /* 38 */   0x00000000, /* 39 */
   0x00000000, /* 40 */   0x00000000, /* 41 */   0x00000000, /* 42 */   0x00000000, /* 43 */
   0x00000000, /* 44 */   0x00000000, /* 45 */   0x00000000, /* 46 */   0x00000000, /* 47 */
};
FXP31 sExpectedOutputD[] = {
   0x00000000, /* 0 */   0x00000000, /* 1 */   0x0437FBC8, /* 2 */   0x69A23962, /* 3 */
   0x9A19AC64, /* 4 */   0xFB9F1D9E, /* 5 */   0x7E9131D2, /* 6 */   0x96C6FB14, /* 7 */
   0xFB9F1D9E, /* 8 */   0x7E9131D2, /* 9 */   0x96C6FB14, /* 10 */   0xFB9F1D9E, /* 11 */
   0x7E9131D2, /* 12 */   0x96C6FB14, /* 13 */   0xFB9F1D9E, /* 14 */   0x7E9131D2, /* 15 */
   0x96C6FB14, /* 16 */   0xFB9F1D9E, /* 17 */   0x7E9131D2, /* 18 */   0x96C6FB14, /* 19 */
   0xFB9F1D9E, /* 20 */   0x7E9131D2, /* 21 */   0x96C6FB14, /* 22 */   0xFB9F1D9E, /* 23 */
   0x7E9131D2, /* 24 */   0x95A15500, /* 25 */   0x0804B872, /* 26 */   0x3911E574, /* 27 */
   0xF687ED10, /* 28 */   0x00000000, /* 29 */   0x00000000, /* 30 */   0x00000000, /* 31 */
   0x00000000, /* 32 */   0x00000000, /* 33 */   0x00000000, /* 34 */   0x00000000, /* 35 */
   0x00000000, /* 36 */   0x00000000, /* 37 */   0x00000000, /* 38 */   0x00000000, /* 39 */
   0x00000000, /* 40 */   0x00000000, /* 41 */   0x00000000, /* 42 */   0x00000000, /* 43 */
   0x00000000, /* 44 */   0x00000000, /* 45 */   0x00000000, /* 46 */   0x00000000, /* 47 */
};
/********************************************************************************/

FXP31 sActualOutput[ NUM_BLOCKS * SS_FRAMES_PER_BLOCK ];

/*******************************************************************/
/* Generate wavetable oscillator output and either
 * compare with known good output,
 * or print source code for known good output.
 *
 * @param phaseIncrement affects pitch of playback
 * @param expected array of known good values
 * @param name of test data set
 */
void TestWaveOscillator( FXP31 phaseIncrement, FXP31 *expected, char *name )
{
    Oscillator_t OSC;
    Oscillator_t *osc = &OSC;

    WaveTable_t  WAVETABLE;
    WaveTable_t *waveTable = &WAVETABLE;

    int index = 0;
    int i;
    int j;

    printf("Test %s\n", name );

    /* Set up structures as needed by wavetable oscillator. */
    waveTable->samples = sSamples;
    waveTable->loopBegin = 4,
    waveTable->loopEnd = 8;
    waveTable->numSamples = NUM_SAMPLES;

    osc->phase = 0;
    osc->phaseInc = phaseIncrement;
    osc->shared.wave.endAt = waveTable->loopEnd;
    osc->shared.wave.next = 0;
    osc->shared.wave.previous = 0;
    osc->shared.wave.sampleIndex = 0;
    osc->shared.wave.waveTable = waveTable;

    /* Generate a few blocks of output including attack and loop. */
    index = 0;
    for( i=0; i<NUM_BLOCKS_ON; i++ )
    {
        Osc_WaveTableS16( osc, &sActualOutput[ index ] );
        index += SS_FRAMES_PER_BLOCK;
    }

    /* Simulate note off event. */
    Osc_WaveTable_Release( osc );

    /* Generate a few blocks of output including some loop and release. */
    for( i=0; i<NUM_BLOCKS_OFF; i++ )
    {
        Osc_WaveTableS16( osc, &sActualOutput[ index ] );
        index += SS_FRAMES_PER_BLOCK;
    }


#if GEN_DATA
    /* Print test vector data as 'C' source code. */
    (void) expected;
    index = 0;
    printf("FXP31 sExpectedOutput%s[] = {\n", name );
    for( i=0; i<NUM_BLOCKS; i++ )
    {
        for( j=0; j<SS_FRAMES_PER_BLOCK; j++ )
        {
            printf("   0x%08X, /* %d */", sActualOutput[ index ], index );
            index += 1;
            if( (index & 3) == 0 ) printf("\n");
        }
    }
    printf("};\n");

    return 0;

#else

    /* Compare actual result with expected result. */
    index = 0;
    for( i=0; i<NUM_BLOCKS; i++ )
    {
        for( j=0; j<SS_FRAMES_PER_BLOCK; j++ )
        {
            FXP31 actual = sActualOutput[ index ];
            FXP31 good = expected[ index ];

            FXP31 diff = actual - good;
            if( diff < 0 ) diff = 0 - diff; // absolute value
            if( diff > TOLERANCE )
            {
                printf("ERROR: expected 0x%08X but got 0x%08X at index = %d\n", good, actual, index );
                numBad += 1;
            }
            else
            {
                numGood += 1;
            }
            index += 1;
        }
    }
#endif
}


/*******************************************************************/
/* Test wave oscillator at varying pitches. */
int main(void);
int main(void)
{
    FXP31 phaseInc;
        
    
    printf("Validate Osc_WaveTableS16() oscillator.\n");
    printf("(C) 2005 Mobileer, Inc.\n");

    /* Output should match the sSamples array because our phase increment
     * is set to advance by exactly one sample.
     */
    phaseInc = NOMINAL_PHASE_INCREMENT;
    TestWaveOscillator( phaseInc, sExpectedOutputA, "A" );

    /* Every other sample of output should match the sSamples array because
     * our phase increment is set to advance by exactly one half sample.
     * Intervening samples should be the interpolated average of the adjacent samples.
     */
    phaseInc = NOMINAL_PHASE_INCREMENT / 2;
    TestWaveOscillator( phaseInc, sExpectedOutputB, "B" );

    /* Advance by a little less than one sample each time. */
    phaseInc = NOMINAL_PHASE_INCREMENT - (NOMINAL_PHASE_INCREMENT / 7);
    TestWaveOscillator( phaseInc, sExpectedOutputC, "C" );

    /* Advance by a little more than one sample each time. */
    phaseInc = NOMINAL_PHASE_INCREMENT + (NOMINAL_PHASE_INCREMENT / 3);
    TestWaveOscillator( phaseInc, sExpectedOutputD, "D" );

    if( numBad == 0 )
    {
        printf("SUCCESS: ");
    }
    else
    {
        printf("ERROR: ");
    }

    printf("numGood = %d, numBad = %d\n", numGood, numBad );
}
