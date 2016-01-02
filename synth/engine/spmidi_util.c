/* $Id: spmidi_util.c,v 1.27 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Utility functions for playing notes, etc.
 * These functions are used to simplify the example programs.
 * Their use is not required by any application.
 * So this code should be considered as example code.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"

/* For documentation, see "spmidi_util.h" */

static const unsigned char SysExGMOn[] =
    {
        MIDI_SOX, 0x7E, 0x7F, 0x09, 0x01, MIDI_EOX
    };

/*******************************************************************/
void SPMUtil_GeneralMIDIOn( SPMIDI_Context *spmidiContext )
{
    SPMIDI_Write( spmidiContext, SysExGMOn, sizeof( SysExGMOn ) );
}

/*******************************************************************/
void SPMUtil_ControlChange( SPMIDI_Context *spmidiContext, int channel, int controller, int value )
{
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + channel, controller, value );
}

/*******************************************************************/
/* Use variable named "bankIndex" instead of "bank" to prevent strange Blackfin compiler errors. */
void SPMUtil_BankSelect( SPMIDI_Context *spmidiContext, int channel, int bankIndex)
{
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + channel,
        MIDI_CONTROL_BANK, ( bankIndex >> SPMIDI_BANK_MSB_SHIFT ) & 0x7F );
    /* Write LSB after MSB according to MIDI spec. */
    SPMIDI_WriteCommand( spmidiContext, MIDI_CONTROL_CHANGE + channel,
        MIDI_CONTROL_BANK + MIDI_CONTROL_LSB_OFFSET, ( bankIndex ) & 0x7F );
}

/*******************************************************************/
void SPMUtil_ProgramChange( SPMIDI_Context *spmidiContext, int channel, int program)
{
    SPMIDI_WriteCommand( spmidiContext, MIDI_PROGRAM_CHANGE + channel, program, 0 );
}

/*******************************************************************/
void SPMUtil_NoteOn( SPMIDI_Context *spmidiContext, int channel, int pitch, int velocity )
{
    SPMIDI_WriteCommand( spmidiContext, MIDI_NOTE_ON + channel, pitch, velocity );
}

/*******************************************************************/
void SPMUtil_NoteOff( SPMIDI_Context *spmidiContext, int channel, int pitch, int velocity )
{
    SPMIDI_WriteCommand( spmidiContext, MIDI_NOTE_OFF + channel, pitch, velocity );
}

/*******************************************************************/
void SPMUtil_AllNotesOff( SPMIDI_Context *spmidiContext, int channel  )
{
    SPMUtil_ControlChange( spmidiContext, channel, MIDI_CONTROL_ALLNOTESOFF, 0 );
}

/*******************************************************************/
void SPMUtil_Reset( SPMIDI_Context *spmidiContext )
{
    if( spmidiContext != NULL )
    {
        int i;
        for( i=0; i<MIDI_NUM_CHANNELS; i++ )
        {
            SPMUtil_AllNotesOff( spmidiContext, i );
        }

        /* This will cause all controllers to be reset. */
        SPMUtil_GeneralMIDIOn(spmidiContext);
    }
}

/*******************************************************************/
void SPMUtil_SetBendRange( SPMIDI_Context *spmidiContext, int channel, int semitones, int cents)
{
    /* Point to bend range RPN. */
    SPMUtil_ControlChange( spmidiContext, channel, 101, 0 ); /* MSB */
    SPMUtil_ControlChange( spmidiContext, channel, 100, 0 );
    /* Set bend range. Send MSB first cuz LSB reset when MSB received. */
    SPMUtil_ControlChange( spmidiContext, channel, 6, semitones ); /* MSB */
    SPMUtil_ControlChange( spmidiContext, channel, (6+32), cents );
    /* For safety, reset RPN to NULL. */
    SPMUtil_ControlChange( spmidiContext, channel, 101, 127 );
    SPMUtil_ControlChange( spmidiContext, channel, 100, 127 );
}

/******************************************************************/
void SPMUtil_PitchBend( SPMIDI_Context *spmidiContext, int channel, int bend )
{
    SPMIDI_WriteCommand( spmidiContext, MIDI_PITCH_BEND + channel,
                         bend & 0x007F, /* LSB */
                         bend >> 7 );  /* MSB */
}

/******************************************************************/
unsigned long SPMUtil_ConvertFramesToMSec( unsigned long frameRate, unsigned long numFrames )
{
    /* Prevent numeric overflow by avoiding (numFrames * 1000) */
    unsigned long seconds = numFrames / frameRate;
    unsigned long rem = (numFrames - (seconds * frameRate));
    unsigned long bigMSec = seconds * 1000;
    unsigned long smallMSec = (rem * 1000) / frameRate;
    unsigned long msec = bigMSec + smallMSec;
    return msec;
}


/******************************************************************/
unsigned long SPMUtil_ConvertMSecToFrames( unsigned long frameRate, unsigned long msec )
{
    /* Prevent numeric overflow by avoiding (msec * frameRate) */
    unsigned long seconds = msec / 1000;
    unsigned long rem = (msec - (seconds * 1000));
    unsigned long bigFrames = seconds * frameRate;
    unsigned long smallFrames = (rem * frameRate) / 1000;
    unsigned long frames = bigFrames + smallFrames;
    return frames;
}

/****************************************************************/
int SPMUtil_GetMaxAmplitude( MIDIFilePlayer *player, int samplesPerFrame,
                             int masterVolume, int sampleRate )
{
    int result;
    SPMIDI_Context *spmidiContext;

    result = SPMIDI_CreateContext( &spmidiContext, sampleRate );
    if( result < 0 )
        return result;

    SPMIDI_SetMasterVolume( spmidiContext, masterVolume );

    result = SPMUtil_MeasureMaxAmplitude( player, spmidiContext, samplesPerFrame );

    SPMIDI_DeleteContext(spmidiContext);

    return result;
}

/****************************************************************/
int SPMUtil_MeasureMaxAmplitude( MIDIFilePlayer *player, SPMIDI_Context *spmidiContext, int samplesPerFrame )
{
#define SAMPLES_PER_BUFFER  (SPMIDI_MAX_SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
    short samples[SAMPLES_PER_BUFFER];
    int i;
    int samplesPerBuffer = samplesPerFrame * SPMIDI_MAX_FRAMES_PER_BUFFER;
    int maxAmplitude = 0;
    int minAmplitude = 0;

    while( MIDIFilePlayer_PlayFrames( player, spmidiContext, SPMIDI_MAX_FRAMES_PER_BUFFER ) == 0)
    {
        /* Synthesize samples and fill buffer. */
        int result = SPMIDI_ReadFrames( spmidiContext, samples, SPMIDI_MAX_FRAMES_PER_BUFFER, samplesPerFrame, 16 );
        if( result < 0 )
        {
            return result;
        }
        for( i=0; i<samplesPerBuffer; i++ )
        {
            int sample = samples[i];
            if( sample > maxAmplitude )
            {
                maxAmplitude = sample;
            }
            else if( sample < minAmplitude )
            {
                minAmplitude = sample;
            }
        }
    }

    if( -minAmplitude > maxAmplitude )
        maxAmplitude = -minAmplitude;

    return maxAmplitude;
}

/****************************************************************/
int SPMUtil_EstimateMaxAmplitude( MIDIFilePlayer *player, int samplesPerFrame,
                                  int masterVolume, int sampleRate )
{
    int result;
#define SAMPLES_PER_BUFFER  (SPMIDI_MAX_SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)

    int maxAmplitude = 0;
    int amp;
    SPMIDI_Context *spmidiContext;

    result = SPMIDI_CreateContext( &spmidiContext, sampleRate );
    if( result < 0 )
        return result;

    SPMIDI_SetMasterVolume( spmidiContext, masterVolume );

    while( (result = MIDIFilePlayer_PlayFrames( player, spmidiContext, SPMIDI_MAX_FRAMES_PER_BUFFER )) == 0)
    {
        /* Partially synthesize result, estimate amplitude. */
        amp = SPMIDI_EstimateMaxAmplitude( spmidiContext, SPMIDI_MAX_FRAMES_PER_BUFFER, samplesPerFrame );
        if( amp > maxAmplitude )
        {
            maxAmplitude = amp;
        }
    }

    SPMIDI_DeleteContext(spmidiContext);

    return maxAmplitude;
}
