/* $Id: test_armasm.c,v 1.3 2005/05/09 00:27:03 philjmsl Exp $ */
/**
 *
 * Test ARM assembly.
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include "../engine/fxpmath.h"



FXP31 *MIX_ELEMENT_ASM( FXP31 *mixSrc, FXP31 *mixer, FXP31 leftGain, FXP31 rightGain )
{
    FXP31 sample = *mixSrc++;
    register FXP31 leftValue;
    register FXP31 rightValue;
    int temp;
    int shifter = 8;
    __asm
    {
        LDR       leftValue, [mixer, #0]
        LDR       rightValue, [mixer, #4]
        /* LDRD      {R0,R1}, [mixer, #0] */
        SMULWT    temp, sample, leftGain
        ADD       leftValue, leftValue, temp, asr shifter
        SMULWT    temp, sample, rightGain
        ADD       rightValue, rightValue, temp, asr shifter
        STR       leftValue, [mixer], #4
        STR       rightValue, [mixer], #4
    }
    return mixer;
}

#define MIX_SCALE_SHIFT_ADD_C( accum, signal, gain, shifter ) \
    (accum + (FXP31_MULT( signal, gain )  >> shifter))
                
#define MIX_ELEMENT_C \
                { \
                }

void MockMixer( FXP31 *mixSrc, FXP31 *mixer, FXP31 leftGain, FXP31 rightGain )
{
    int shifter = 8;
    FXP31 sample = *mixSrc++;
    FXP31 leftValue = mixer[0];
    FXP31 rightValue = mixer[1];
    leftValue = MIX_SCALE_SHIFT_ADD_C( leftValue, sample, leftGain, shifter );
    rightValue = MIX_SCALE_SHIFT_ADD_C( rightValue, sample, rightGain, shifter );
    /* Write together so we make better use of write buffer. */
    *mixer++ = leftValue;
    *mixer++ = rightValue;
}

FXP31 source[] =
{
    0x12345678,
    0x44332211
};

FXP31 accumulator[] =
{
    0x10000000,
    0x20000000,
    0x30000000,
    0x40000000
};

void TestMixer( void )
{
    MIX_ELEMENT_ASM( &source[0], &accumulator[0], 0x20000000, 0x10000000 );
}


__inline int AddEmUp( int x, int y )
{
    int result;
    __asm
    {
        ADD    result, x, y
    }
    return result;
}

__inline int FXP31_SaturatedDouble( FXP31 x )
{
    int result;
    __asm
    {
        QADD    result, x, x
    }
    return result;
}


int MultiplyAddC( int x, int y, int z )
{
    return (x*y) + z;
}

__inline int InlineMultiplyAddC( int x, int y, int z )
{
    return (x*y) + z;
}

int TestSMULBB( int x, int y )
{
    int result;
    __asm
    {
        SMULBB    result, x, y
        QADD    result, result, result
    }
    return result;
}


FXP31 FXP31_Multiply( FXP31 x, FXP31 y )
{
    FXP31 result;
    __asm
    {
        SMULWT    result, x, y
        QADD    result, result, result
    }
    return result;
}

FXP31 CalculateWaveOutput(
    int previousShifted,
    int phase,
    int delta
)
{
    short phase9 = (short) (phase>>9);
    return previousShifted + ((phase9 * delta) << 1);
}


FXP31 CalculateWaveOutputAsm(
    int previousShifted,
    int phase,
    int delta
)
{
    FXP31 temp;
    FXP31 tempDelta;
    __asm
    {
        MOV     temp, phase, LSR #9
        MOV     tempDelta, delta, ASR #1
        SMULBB  temp, delta, temp
        QDADD   temp, previousShifted, temp
    }
    return temp;
}

void TestInterpolation( int previous, int next, int phase )
{
    int previousShifted = previous << 16;
    int delta;
    int z;
    printf( "TestInterpolation: previous = 0x%08X, next =  0x%08X, phase =  0x%08X\n",
        previous, next, phase );
        
    delta = next - previous;
    z = CalculateWaveOutput( previousShifted, phase, delta );
    printf( "interp = 0x%08X \n", z );

    z = CalculateWaveOutputAsm( previousShifted, phase, delta );
    printf( "interp DSP = 0x%08X \n", z );
}

/*******************************************************************/
int main(void);
int main(void)
{
    int x;
    int y;
    int z;
    int w;
    int previous;
    int next;
    int phase;
    printf( "Test ASM -----------\n" );


    TestMixer();
    
    phase = 0x00800000;
    previous = 0x00001000;
    next = 0x00002000;
    TestInterpolation( previous, next, phase ); 
    
    previous = -0x00003000;
    next = 0x00003000;
    TestInterpolation( previous, next, phase );
    
    previous = -0x00005000;
    next = 0x00005000;
    TestInterpolation( previous, next, phase ); 
    
    phase = 0x00400000;
    previous = -0x00003000;
    next = 0x00003000;
    TestInterpolation( previous, next, phase );
    
    previous = -0x00005000;
    next = 0x00005000;
    TestInterpolation( previous, next, phase ); 

    x = 0x2468ACE2;
    y = 0x40000000;
    z = FXP31_MULT( x, y );
    printf( "FXP31_MULT 0x%08X 0x%08X => 0x%08X\n", x, y, z );

    z = FXP31_Multiply( x, y );
    printf( "FXP31_Multiply 0x%08X 0x%08X => 0x%08X\n", x, y, z );

    x = 7;
    y = 20;
    z = 9;
    w = MultiplyAddC( x, y, z );
    printf( "MultiplyAddC = %d\n", w );

    x = 7;
    y = 20;
    z = 9;
    w = InlineMultiplyAddC( x, y, z );
    printf( "InlineMultiplyAddC = %d\n", w );

    x = 0x0023;
    y = 0x0900;
    z = AddEmUp( x, y );
    printf( "z = 0x%08X\n", z );

    x = 0x00123456;
    z = FXP31_SaturatedDouble( x );
    printf( "sat double = 0x%08X\n", z );

    x = 0x412384AF;
    z = FXP31_SaturatedDouble( x );
    printf( "sat double = 0x%08X\n", z );

    x = 0x40000000;
    z = FXP31_SaturatedDouble( x );
    printf( "sat double = 0x%08X\n", z );

    return 0;
}
