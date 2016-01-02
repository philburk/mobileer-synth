/* $Id: play_timed.c,v 1.8 2007/10/10 00:26:51 philjmsl Exp $ */
/**
 *
 * Play a MIDIFile with no audio for debugging and benchmarking.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/midifile_player.h"

#include "spmidi/examples/midifile_names.h"


#define READ_FROM_FILE      (1)

#if (READ_FROM_FILE == 0)
#include "urbanore.c"
#endif

#define SAMPLE_RATE         (16000)
#define SAMPLES_PER_FRAME   (1)

#define SAMPLES_PER_BUFFER  (SPMIDI_MAX_SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)

short samples[SAMPLES_PER_BUFFER];

/****************************************************************/
static int GetCheckSum( MIDIFilePlayer *player, SPMIDI_Context *spmidiContext, int samplesPerFrame, int *totalFramesPtr )
{
    int result;
    int i;
    int samplesPerBuffer;
    int checksum = 0;
    int numRead;
    int totalFrames = 0;

    samplesPerBuffer = samplesPerFrame * SPMIDI_GetFramesPerBuffer();

    while( (result = MIDIFilePlayer_PlayFrames( player, spmidiContext, samplesPerBuffer )) == 0)
    {
        /* Synthesize samples and fill buffer. */
        numRead = SPMIDI_ReadFrames( spmidiContext, samples, samplesPerBuffer, samplesPerFrame, 16 );
        for( i=0; i<numRead; i++ )
        {
            checksum += samples[i];
        }
        totalFrames += numRead;
    }

    *totalFramesPtr = totalFrames;

    return checksum;
}

/****************************************************************
 * Estimate a MIDIFIle amplitude and print in CSV format.
 */
static int MIDIFile_CheckSum( unsigned char *image, int numBytes )
{
    int result;
    int checksum = 0;
    int numFrames;
    int totalFrames;
    int msec;
    int seconds;
    int rem_msec;
    MIDIFilePlayer *player;
    SPMIDI_Context *spmidiContext = NULL;

    SPMIDI_Initialize();

    /* Create a player, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &player, (int) SAMPLE_RATE, image, numBytes );
    if( result < 0 )
    {
        goto error;
    }

    msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
    seconds = msec / 1000;
    rem_msec = msec - (seconds * 1000);
    PRTMSG("Song Duration = ");
    PRTNUMD(seconds);
    PRTMSG(" seconds + ");
    PRTNUMD( rem_msec );
    PRTMSG(" msec\n");

    
    numFrames = MIDIFilePlayer_GetDurationInFrames( player );
    PRTMSG(" duration = ");
    PRTNUMD( numFrames );
    PRTMSG(" frames\n");

    result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
    {
        goto error;
    }
    checksum = GetCheckSum( player, spmidiContext, SAMPLES_PER_FRAME, &totalFrames  );

    PRTMSG(" framesPerBuffer = ");
    PRTNUMD( SPMIDI_GetFramesPerBuffer() );
    PRTMSG("\n");

    PRTMSGNUMD("checksum = ", checksum );
    PRTMSGNUMD("measured total frames = ", totalFrames );
    PRTMSGNUMD("Max voices used = ", SPMIDI_GetMaxNoteCount(spmidiContext) );
    result = checksum;

error:
    if( spmidiContext != NULL ) SPMIDI_DeleteContext(spmidiContext);

    if( player != NULL ) MIDIFilePlayer_Delete( player );

    SPMIDI_Terminate();

    return result;
}

#if READ_FROM_FILE
/****************************************************************/
static int estimate_midi_file( const char *inputFileName )
{
    void *data = NULL;
    int  fileSize;
    int   result = -1;

    PRTMSG("measure checksum of ");
    PRTMSG( inputFileName );
    PRTMSG("\n");

    /* Load MIDI File into a memory image. */
    data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( data == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        goto error;
    }

    result = MIDIFile_CheckSum( data, fileSize );

error:
    if( data != NULL )
        SPMUtil_FreeFileImage( data );

    return result;
}
#else
/****************************************************************/
/* Load MIDI file from initialized 'C' source array. */
extern unsigned char midiFileImage[];
extern int midiFileImage_size;

static int estimate_midi_file( const char *inputFileName )
{
    int   result = -1;
    (void) inputFileName;

    PRTMSG("Measure binary image of MIDIFile\n");

    result = MIDIFile_CheckSum( &midiFileImage[0], midiFileImage_size );

    return result;
}
#endif

/****************************************************************/

#ifndef MAIN_ENTRY
#define MAIN_ENTRY main
#endif

int MAIN_ENTRY( void )
{
    char *inputFileName = DEFAULT_FILENAME;
    int result;
    result = estimate_midi_file( inputFileName );
    PRTMSG(" result = ");
    PRTNUMD( result );
    PRTMSG(" \n");
    return result;
}

