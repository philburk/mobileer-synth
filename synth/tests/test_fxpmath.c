/* $Id: test_fxpmath.c,v 1.7 2005/05/09 00:27:14 philjmsl Exp $ */
/**
 *
 * Test fixed point math operators on BlackFin DSP.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include "../engine/fxpmath.h"

/*******************************************************************/
int main(void);
int main(void)
{
    int err = 0;

    int x;
    int y;
    int z;

    x = 0x2468ACE0;
    y = 0x40000000; // 1/2 in 1.31 format

    z = FXP31_MULT(x,y);
    printf( "FXP31_MULT z = 0x%08X\n", z );

    y = -y;
    z = FXP31_MULT(x,y);
    printf( "FXP31_MULT z = 0x%08X\n", z );

    x = -x;
    z = FXP31_MULT(x,y);
    printf( "FXP31_MULT z = 0x%08X\n", z );


    y = -y;
    z = FXP31_MULT(x,y);
    printf( "FXP31_MULT z = 0x%08X\n", z );

    return err;
}
