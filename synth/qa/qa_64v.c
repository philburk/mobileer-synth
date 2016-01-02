/* $Id: qa_64v.c,v 1.6 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play 64 voices for your listening pleasure.
 *
 * Purpose: to verify full polyphony.
 *
 * Author: Robert Marsanyi, Phil Burk
 * Copyright 2002-5 Mobileer
 */

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_errortext.h"
#include "spmidi/qa/qa_tools.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)
#define MAX_VOICES          (64)
#define BUFFERS_PER_CHORD   (60)

typedef struct TestData_s
{
    int numChords;
    int notesPerChord;
    int maxNotesPerChord;
    int numChannels;
    int buffersPerChord;
    int checksum;
    int frameCount;
    int maxVoiceCount;
}
TestData_t;


/* Host independant writable audio device. */
static SPMIDI_AudioDevice  sHostAudioDevice;

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 */
static int PlayNextBuffer( SPMIDI_Context *spmidiContext )
{
    int result;
    /* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)

    short samples[SAMPLES_PER_BUFFER];

    /* Generate a buffer full of audio data as 16 bit samples. */
    result = SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
                       SAMPLES_PER_FRAME, 16 );
    if ( result < 0 ) return result;

    /* Write audio samples to the audio device. */
    SPMUtil_WriteAudioBuffer( sHostAudioDevice, samples, FRAMES_PER_BUFFER );

    return result;
}


/****************************************************************/
/**
 * Play lots of voices simultaneously.
 * Turn lots of voices on and off each channel. 
 */
static int PlayManyVoices( TestData_t *data )
{
    SPMIDI_Context *spmidiContext = NULL;
    int result = 0;
    int chordIndex;
    int noteIndex;
    int bufferIndex;
    int channelIndex;

    data->frameCount = 0;
    data->notesPerChord = 1;

    /* Start SP-MIDI synthesis engine using the desired sample rate. */
    result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
    if( result < 0 )
        goto error1;
    
    /* Start audio playback */
    result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, SAMPLES_PER_FRAME );
    if( result < 0 )
        goto error1;

    /* Use scattered collection of pitches and programs. */
#define NOTE_PITCH (30 + ((channelIndex + (chordIndex*5) + (noteIndex * 7)) & 63))
#define DRUM_PITCH (GMIDI_FIRST_DRUM + ((channelIndex + (chordIndex*5) + (noteIndex * 7)) & 31))
#define CALC_PITCH ((channelIndex == MIDI_RHYTHM_CHANNEL_INDEX) ? DRUM_PITCH : NOTE_PITCH)

    for( chordIndex=0; chordIndex < data->numChords; chordIndex++ )
    {
        int numActive, numExpected;

        /* On every channel. */
        for( channelIndex=0; channelIndex < data->numChannels ; channelIndex++ )
        {
            if( (chordIndex & 3) == 0 )
            {
                int programIndex = (chordIndex + (channelIndex * 7)) & 127;
                SPMUtil_ProgramChange( spmidiContext, channelIndex, programIndex );
            }

            /* Turn ON notes. */
            for( noteIndex=0; noteIndex < data->notesPerChord; noteIndex++ )
            {
                int pitch = CALC_PITCH;
                SPMUtil_NoteOn( spmidiContext, channelIndex, pitch, 64 );
            }
        }

        /* Check to make sure that we are really playing all those notes. */
        numActive = SPMIDI_GetActiveNoteCount( spmidiContext );
        numExpected = data->notesPerChord * data->numChannels;
        if( numActive < numExpected )
        {
            PRTMSG("ERROR - num active notes = ");
            QA_CountError();
        }
        else
        {
            PRTMSG("SUCCESS - num active notes = ");
            QA_CountSuccess();
        }
        PRTNUMD( numActive  );
        PRTMSG(", expected at least ");
        PRTNUMD( numExpected  );
        PRTMSG("\n");
        

        /* Let notes play. */
        for( bufferIndex=0; bufferIndex < (data->buffersPerChord/2); bufferIndex++ )
        {
            result = PlayNextBuffer( spmidiContext );
            if( result < 0 ) goto error1;
            data->frameCount += FRAMES_PER_BUFFER;
        }

        /* On every channel. */
        for( channelIndex=0; channelIndex < data->numChannels; channelIndex++ )
        {
            /* Turn OFF notes. */
            for( noteIndex=0; noteIndex < data->notesPerChord; noteIndex++ )
            {
                int pitch = CALC_PITCH;
                SPMUtil_NoteOff( spmidiContext, channelIndex, pitch, 64 );
            }
        }
        /* Let notes die down. */
        for( bufferIndex=0; bufferIndex < (data->buffersPerChord/2); bufferIndex++ )
        {
            result = PlayNextBuffer( spmidiContext );
            if( result < 0 ) goto error1;
            data->frameCount += FRAMES_PER_BUFFER;
        }

        data->maxVoiceCount = SPMIDI_GetMaxNoteCount(spmidiContext);

        {
            int numNotes = (chordIndex >> 3) + 1;
            if( numNotes <= data->maxNotesPerChord )
            {
                data->notesPerChord = numNotes;
            }
        }
    }

error1:
    if( sHostAudioDevice != NULL ) SPMUtil_StopAudio( sHostAudioDevice );
        
    /* Terminate SP-MIDI synthesizer. */
    if( spmidiContext != NULL ) SPMIDI_DeleteContext(spmidiContext);

    return result;
}


/****************************************************************/
int main( int argc, char ** argv )
{
    int result;
    TestData_t DATA = {0};
    (void) argc;
    (void) argv;

    PRTMSG("Test Mobileer Synthesizer\n");
    PRTMSG("(C) 2005 Mobileer\n");

    QA_Init( "qa_64v" );

#if (MAX_VOICES > SPMIDI_MAX_VOICES)
#error ERROR - need to set SPMIDI_MAX_VOICES to 64 in spmidi_config.h or spmidi_user_config.h
#endif

    DATA.buffersPerChord = BUFFERS_PER_CHORD;
    DATA.numChannels = (MAX_VOICES < MIDI_NUM_CHANNELS) ? MAX_VOICES : MIDI_NUM_CHANNELS;
    
    PRTMSG("NumChannels = ");
    PRTNUMD( DATA.numChannels );
    PRTMSG("\n");

    DATA.maxNotesPerChord = MAX_VOICES / DATA.numChannels;

    PRTMSG("maxNotesPerChord = ");
    PRTNUMD( DATA.maxNotesPerChord );
    PRTMSG("\n");

    DATA.numChords = 32;

    result = PlayManyVoices( &DATA );
    if( result < 0 )
    {
        PRTMSG("Error: result = ");
        PRTNUMD( result );
        PRTMSG( SPMUtil_GetErrorText( result ) );
        PRTMSG("\n");
    }
    else
    {
        PRTMSG("NumFrames = ");
        PRTNUMD( DATA.frameCount );
        PRTMSG("\n");

        PRTMSG("MaxVoices = ");
        PRTNUMD( DATA.maxVoiceCount );
        PRTMSG("\n");
    }


    return QA_Term( 32 );
}

