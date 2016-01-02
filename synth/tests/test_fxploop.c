/* $Id: test_fxploop.c,v 1.2 2005/05/03 22:04:00 philjmsl Exp $ */
/**
 *
 * Test fixed point math operators.
 * Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>

#define FXP31_MULT(a,b)   (((a)>>15) * ((b)>>16))
#define FXP31_MULT_NEW(a,b)   ((((a)>>16) * ((b)>>16))<<1)

int main(void)
{
    int i;
    long x = 0;
    long y;
    long z;

    printf("Test multiply --------------\n");
    printf("sizeof(int) = %d\n", sizeof(int));
    printf("sizeof(long) = %d\n", sizeof(long));

    y = 0x02468ACE;
    printf("y = 0x%08X\n", y );
    for( i=0; i<10; i++ )
    {
        x += 0x10000000;
        printf("x = 0x%08X\n", x );

        z = FXP31_MULT( x, y );
        printf("  z=x*y    = 0x%08x\n", z );

        z = FXP31_MULT( y, x );
        printf("  z=y*x    = 0x%08x\n", z );

        z = FXP31_MULT_NEW( x, y );
        printf("  z.new    = 0x%08x\n", z );
    }

    return 0;
}
