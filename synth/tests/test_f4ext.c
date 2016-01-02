/* $Id: test_f4ext.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * Test to see how parser handles extended length F4 messages.
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

#define SAMPLE_RATE  (44100)
#define CHANNEL      (1)

static const unsigned char EDMessage[] =
{
    0xF4, 0x01, 0x23, 0x40, 0x32, 0x72
};

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Context *spmidiContext = NULL;
    int err;

    err = SPMUtil_Start( &spmidiContext, SAMPLE_RATE, NULL, SPMUTIL_OUTPUT_MONO );
    if( err < 0 )
        goto error;

    SPMUtil_ProgramChange( spmidiContext, CHANNEL, 41 );

    SPMUtil_NoteOn( spmidiContext, CHANNEL, 60, 64 );
    SPMUtil_PlayMilliseconds( spmidiContext, 1000 );
    SPMUtil_NoteOff( spmidiContext, CHANNEL, 60, 0 );
    SPMUtil_PlayMilliseconds( spmidiContext, 200 );

    SPMIDI_Write( spmidiContext, EDMessage, sizeof( EDMessage ) );

    SPMUtil_NoteOn( spmidiContext, CHANNEL, 62, 64 );
    SPMUtil_PlayMilliseconds( spmidiContext, 1000 );
    SPMUtil_NoteOff( spmidiContext, CHANNEL, 62, 0 );
    SPMUtil_PlayMilliseconds( spmidiContext, 200 );

    SPMUtil_Stop(spmidiContext);

    printf("Test finished.\n");
    return err;
error:
    return err;
}
