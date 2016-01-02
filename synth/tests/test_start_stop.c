/* $Id: test_start_stop.c,v 1.4 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test Creating and Deleting an SPMIDI_Context.
 * The original API used "start" and "stop" function names.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

#define SAMPLE_RATE         (22050)

/*******************************************************************/
int main(void);
int main(void)
{
    int err = 0;
    int i;
    SPMIDI_Context *spmidiContext;

    printf("SPMIDI Test: start and stop\n" );

    for( i=0; i<4; i++ )
    {
        err = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
        if( err < 0 )
            goto error;

        SPMIDI_DeleteContext(spmidiContext);
    }

    printf("Test finished.\n");
    return err;
error:
    return err;
}
