/* $Id: test_print.c,v 1.4 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Just print some text and numbers.
 *
 * Copyright 2003 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include "spmidi/include/spmidi_print.h"

/*******************************************************************/
void SPTest_Print( void )
{
    PRTMSG("Message from SPTest_Print()\n");
    PRTMSG("1234 = ");
    PRTNUMD( 1234 );
    PRTMSG("\n");
}
