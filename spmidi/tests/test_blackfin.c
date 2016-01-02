/* $Id: test_blackfin.c,v 1.1 2005/05/09 00:05:10 philjmsl Exp $ */
/**
 *
 * Test fixed point math operators.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>

#include <fract.h>

#include "../engine/fxpmath.h"

#ifdef __ADSPBLACKFIN__
#if __ADSPBLACKFIN__

inline int FastMultiply( int x, int y )
{
    int product;
    asm("%0 = %1.H * %2.H;" 
        :"=E"(product)     /* output */ 
        :"r"(x),"r"(y)  /* input  */ 
        );
    return product;
}
#endif
#endif

/* This gets compiled inline but it requires that the result be passed.
 * How can we make this compatible with FXP31_MULT(xx,yy)?
 */
#define FastMultiplyMacro( product, xx, yy ) \
    asm("%0 = %1.H * %2.H;" \
        :"=E"(product)     /* output */ \
        :"r"(xx),"r"(yy)  /* input  */ \
        );


/*******************************************************************/
int main(void);
void TestMult( int x, int y)
{
    register int z;
    register fract32 zf;
    int temp;

    printf( "----------------------\n" );
    z = FXP31_MULT(x,y);
    printf( "FXP31_MULT z = 0x%08X\n", z );
    
    /* This function returns a nice result but is very slow and not inline. */
    zf = mult_fr1x32x32( (fract32) x, (fract32) y);
    printf( "mult_fr1x32x32 zf = 0x%08X\n", zf );
    
    zf = FastMultiply( x, y);
    printf( "FastMultiply zf = 0x%08X\n", zf );
    zf = 0xdeadbeef;
    
    FastMultiplyMacro( zf, x, y);
    printf( "FastMultiplyMacro zf = 0x%08X\n", zf );
    zf = 0xdeadbeef;
    
}

int main(void)
{
    int err = 0;

    int x;
    int y;
    int z;

    x = 0x2468ACEF;
    y = 0x4158347A; // 1/2 in 1.31 format
    TestMult(x,y);

    y = -y;
    TestMult(x,y);

    x = -x;
    TestMult(x,y);


    y = -y;
    TestMult(x,y);

    return err;
}
