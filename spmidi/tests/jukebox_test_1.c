/* $Id: jukebox_test_1.c,v 1.5 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play a few notes on JukeBox.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_jukebox.h"

short buffer[2048];

/*******************************************************************/
int main(void);
int main(void)
{
    SPMIDI_Error err;
    int i;

    err = JukeBox_Initialize( 22050 );
    if( err < 0 )
        goto error1;

    JukeBox_NoteOn( 1, 0, 61, 70 );
    JukeBox_NoteOn( 2, 0, 62, 70 );
    JukeBox_NoteOn( 5, 0, 65, 70 );
    JukeBox_NoteOn( 4, 0, 64, 70 );

    for( i=0; i<10; i++ )
    {
        printf("Buffer #%d\n", i );
        err = JukeBox_SynthesizeAudioTick( buffer, JukeBox_GetFramesPerTick(), 2 );
        if( err < 0 )
            goto error2;
    }

error2:
    JukeBox_Terminate();

error1:
    printf("Test finished.\n");
    return err;
}
