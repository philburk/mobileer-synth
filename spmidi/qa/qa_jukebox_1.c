/* $Id: qa_jukebox_1.c,v 1.7 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_jukebox_1.c
 * @brief Self tests for JukeBox.
 *
 * Tests song queues, note scheduling.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>

#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_jukebox.h"

/*
 * Adjust these for your system.
 */
#define PROGRAM             (13)
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)
#define BITS_PER_SAMPLE     (sizeof(short)*8)

static int numErrors = 0;

#define ERROR(msg) { numErrors++; printf("ERROR: " msg "\n"); }


/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

short audioBuffer[SPMIDI_MAX_FRAMES_PER_BUFFER];
/****************************************************************/
/**
 * Get one ticks worth of Audio from Jukebox and return maximum amplitude
 */

int SynthesizeOneTick( void )
{
    int i;
    int returnCode = 0;
    int max = 0;

    returnCode = JukeBox_SynthesizeAudioTick( audioBuffer, SPMIDI_MAX_FRAMES_PER_BUFFER, SAMPLES_PER_FRAME );
    if( returnCode < 0 )
    {
        PRTMSGNUMH("Error: JukeBox_SynthesizeAudio returned ", returnCode );
        return -1;
    }

    for( i=0; i<returnCode; i++ )
    {
        int sample = audioBuffer[i];
        if(sample > max)
            max = sample;
        else if( -sample > max )
            max = -sample;
    }

    return max;
}

/* Queue two notes and watch for them to appear. */
void TestNoteSchedule( void )
{
    int i;
    int time;
    int max;
    const int dur1 = 5;
    const int dur2 = 200;
    const int channel = 0;

    printf("Test scheduling of notes.\n");

    time = JukeBox_GetTime();

    /* Set to xylophone. */
    JukeBox_ProgramChange( time, channel, PROGRAM );

    time += dur1;
    JukeBox_NoteOn( time, channel, 60, 64 );
    JukeBox_NoteOff( time + 1, channel, 60, 0 );

    for( i=0; i<dur1; i++ )
    {
        max = SynthesizeOneTick();
        if( max > 0 )
            ERROR( "note played too soon" );
    }

    max = SynthesizeOneTick();
    if( max <= 0 )
        ERROR( "note did not play on time." );

    time += dur2;
    JukeBox_NoteOn( time, channel, 61, 64 );
    JukeBox_NoteOff( time + 1, channel, 61, 0 );

    for( i=1; ((i<dur2) && (max>0)); i++ )
    {
        max = SynthesizeOneTick();
    }
    if( max > 0 )
        ERROR( "first note did not die soon enough" );

    for( ; i<dur2; i++ )
    {
        max = SynthesizeOneTick();
        if( max > 0 )
            ERROR( "note played too soon" );
    }

    max = SynthesizeOneTick();
    if( max <= 0 )
        ERROR( "note did not play on time." );

}

/*******************************************************************/
static void TestSongQueue1( void )
{
    int result;
    printf("Test queuing of songs.\n");
    /* Queue up some songs and make sure they show up in the queue. */
    result = JukeBox_QueueSong( 0, 1 );
    if( result < 0 )
        ERROR( "could not queue song" );
    result = JukeBox_QueueSong( 1, 1 );
    if( result < 0 )
        ERROR( "could not queue song" );
    result = JukeBox_QueueSong( 0, 1 );
    if( result < 0 )
        ERROR( "could not queue song" );

    if( JukeBox_GetSongQueueDepth() != 3 )
        ERROR("expected 3 in song queue");

    /* Read first song out of queue and start playing. */
    SynthesizeOneTick();
    if( JukeBox_GetSongQueueDepth() != 2 )
        ERROR("expected 2 in song queue");

    /* Clear queue, which does not show up right away because it is done in the synth thread. */
    JukeBox_ClearSongQueue();
    SynthesizeOneTick();
    if( JukeBox_GetSongQueueDepth() != 0 )
        ERROR("expected 0 in song queue after clear");

}

/*******************************************************************/
static void TestSongQueue2( void )
{
    int i;
    int j;
    int result;
    printf("Test queuing of lots of songs to check for node leaks.\n");

    for( i=0; i<100; i++ )
    {
        for( j=0; j<10; j++  )
        {
            result = JukeBox_QueueSong( 0, 1 );
            if( result < 0 )
                ERROR( "could not queue song" );
        }

        if( JukeBox_GetSongQueueDepth() != 10 )
            ERROR("expected 3 in song queue");

        /* Read first song out of queue and start playing. */
        SynthesizeOneTick();

        /* Clear queue, which does not show up right away because it is done in the synth thread. */
        JukeBox_ClearSongQueue();
        SynthesizeOneTick();
        if( JukeBox_GetSongQueueDepth() != 0 )
            ERROR("expected 0 in song queue after clear");
    }
}


/*******************************************************************/
/**
 * Test Jukebox
 * @return 0 if test has no errors, or 1 if there were errors
 */
int main(void);
int main(void)
{
    SPMIDI_Error err;
    SPMIDI_Context *spmidiContext;
    int result;

    err = JukeBox_Initialize( 22050 );
    if( err < 0 )
        goto error1;

    spmidiContext = JukeBox_GetMIDIContext();

    /* Turn off compressor so we do not delay the sound. */
    result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, 0 );
    if( result < 0 )
        goto error1;

    TestNoteSchedule();
    TestSongQueue1();
    TestSongQueue2();

    JukeBox_Terminate();

error1:
    printf("Test finished. numErrors = %d\n", numErrors);

    return (numErrors > 0);
}
