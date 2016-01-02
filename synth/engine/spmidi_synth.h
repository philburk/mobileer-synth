#ifndef _SPMIDI_SYNTH_H
#define _SPMIDI_SYNTH_H

/* $Id: spmidi_synth.h,v 1.33 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 *
 * SPMIDI_SYNTH header.
 * Define API for various synthesizers that can work with the
 * higher level MIDI parser and voice allocator.
 * This is not normally called by an application.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/include/spmidi_config.h"
#include "spmidi/engine/fxpmath.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_orchestra.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* A BUFFER is made of BLOCKS, which is made of FRAMES */

    /** Number of audio frames per processing block.
     * This can be set to 2,3 or 4
     */
#ifndef SS_FRAMES_PER_BLOCK_LOG2
#define SS_FRAMES_PER_BLOCK_LOG2    (SPMIDI_FRAMES_PER_BLOCK_LOG2)
#endif

#if (SS_FRAMES_PER_BLOCK_LOG2 < 2) || (SS_FRAMES_PER_BLOCK_LOG2 > 4)
#error SS_FRAMES_PER_BLOCK_LOG2 out of range, must be 2,3 or 4
#endif

#define SS_FRAMES_PER_BLOCK         (1 << SS_FRAMES_PER_BLOCK_LOG2)

/* TODO - make independant from SS_FRAMES_PER_BLOCK_LOG2. Must modify oscillator loop. */
#define SS_BLOCKS_PER_BUFFER_LOG2   (SS_FRAMES_PER_BLOCK_LOG2)
#define SS_BLOCKS_PER_BUFFER        (1 << SS_BLOCKS_PER_BUFFER_LOG2)
#define SS_FRAMES_PER_BUFFER        (SS_FRAMES_PER_BLOCK * SS_BLOCKS_PER_BUFFER)

    /** Number of audio frames per control frame */
#define SS_FRAMES_PER_CONTROL  (SS_BLOCKS_PER_BUFFER)
#define SS_FRAMES_PER_CONTROL_LOG2  (SS_FRAMES_PER_BLOCK_LOG2)

#define SPMIDI_FREQUENCY_SHIFT      (15)
#define SPMIDI_PITCH_SCALAR_SHIFT   (7)

#define SPMIDI_VEQ_SHIFT         (7)
    /** Default gain for pitch zero. */
#define SPMIDI_VEQ_BASS_GAIN     ((SPMIDI_VEQ_NOMINAL_GAIN)/2)
    /** Pitch below which the amplitude is attenuated. Set to zero to disable VEQ. */
#define SPMIDI_VEQ_BASS_CUTOFF   (60)

    /** We use an unsigned byte array to address them so this is the absolute maximum allowed. */
#define SS_MAX_PRESETS (1<<8)

    /** Expressed as a 17.15 fraction. Giving a range of +-65536 Hz with fractional precision. */
    typedef FXP15 SPMIDIFrequency;

    typedef void (SS_AutoStopCallback)( int voiceIndex, void *userData );

    /********************************************************************/
    /** Define a pseudo-class for a synthesizer. */
    typedef struct SoftSynth_s
    {

        /** Read PCM audio from the synthesis engine. */
        int (*SynthesizeBuffer)( struct SoftSynth_s *synth, void *samples,
                                 int samplesPerFrame, int bitsPerSample );

        /** Estimate max audio amplitude from the synthesis engine. */
        int (*EstimateMaxAmplitude)( struct SoftSynth_s *synth, int samplesPerFrame );

        /* Turn on a note so that it contributes to audio mix. */
        void (*NoteOn)( struct SoftSynth_s *synth, int voiceIndex, int channelIndex,
                        int pitch, int velocity, int pan );

        /* Turn off a note so that it will decay and eventually stop. */
        void (*NoteOff)( struct SoftSynth_s *synth, int voiceIndex );

        void (*TriggerDrum)( struct SoftSynth_s *synth, int voiceIndex, int channelIndex,
                             int pitch, int velocity, int pan );

        /**
         * Set function to be called when a voice stops.
         * This allows the voice allocator to move it
         * to the free list.
         */
        void (*SetAutoStopCallback)( struct SoftSynth_s *synth,
                                     SS_AutoStopCallback *autoStopCallback,
                                     void *autoStopUserData );

        /** Mark a voice as being stolen so that value will get cross faded. */
        void (*StealVoice)( struct SoftSynth_s *synth, int voiceIndex );
        /** Tell voice to shut up ASAP. */
        void (*StifleVoice)( struct SoftSynth_s *synth, int voiceIndex );

        void (*SetChannelPitchBend)( struct SoftSynth_s *synth, int channelIndex, FXP16 semitoneOffset16 );
        void (*SetChannelAftertouch)( struct SoftSynth_s *synth, int channelIndex, int aftertouch );
        void (*SetChannelTuning)( struct SoftSynth_s *synth, int channelIndex, FXP16 octaveOffset16 );
        void (*SetChannelBank)( struct SoftSynth_s *synth, int ci, int b );
        void (*SetChannelProgram)( struct SoftSynth_s *synth, int channelIndex, int program );

        void (*SetChannelModDepth)( struct SoftSynth_s *synth, int channelIndex, int modDepth );
        void (*SetChannelVolume)( struct SoftSynth_s *synth, int channelIndex, int volume );
        void (*SetChannelExpression)( struct SoftSynth_s *synth, int channelIndex, int expression );

        /** Set master volume where 0x080 is unity gain. */
        void (*SetMasterVolume)( struct SoftSynth_s *synth, FXP7 masterVolume );

        /** Turn off all active voices on a channel by asking them to ramp down ASAP.
         * AutoStopCallback functions will be called when they finish ramping down.
         */
        void (*AllSoundOff)( struct SoftSynth_s *synth, int channelIndex );

        /** Immediately kill all voices. Do not ramp down. This will probably click.
         */
        int (*StopAllVoices)( struct SoftSynth_s *synth );

        /**
         * Turn General MIDI on or off.
         * @param isOn Set to 1 to turn on General MIDI node, 0 for off.
         */
        void (*SetGeneralMIDIMode)( struct SoftSynth_s *synth, int isOn );

        /**
         * Set indexed parameter for synthesizer.
         * @return zero or native error code
         */
        int (*SetParameter)( struct SoftSynth_s *synth, SPMIDI_Parameter parameterIndex, int value );

        /**
         * Get indexed parameter value for synthesizer.
         */
        int (*GetParameter)( struct SoftSynth_s *synth, SPMIDI_Parameter parameterIndex, int *valuePtr );

        /** Velocity EQ Bass gain. Gain at pitch zero. */
        short           veqGainAtZero;
        /** Velocity EQ Bass cutoff. Only pitches below this value will be affected. */
        unsigned char   veqBassCutoff;

        FXP7            masterVolume;
        FXP7            rhythmVolume;

    }
    SoftSynth;

    /********************************************************************/

    /** Initialize SoftSynth.
     * This should be called only once before calling SS_CreateSynth().
     */
    int SS_Initialize( void );

    /** Terminate SoftSynth.
     */
    int SS_Terminate( void );


    /* Create a context for synthesis. */
    int SS_CreateSynth( SoftSynth **synthPtr, int sampleRate );
    /* Delete context for synthesis. */
    int SS_DeleteSynth( SoftSynth *synth );

    /** Download an instrument definition as a byte stream.
     * The contents of the definition are specific to the synthesizer in use.
     */
    int SS_SetInstrumentDefinition( HybridOrchestra_t *orchestra, int insIndex, ResourceTokenMap_t *tokenMap, unsigned char *data, int numBytes );


#if SPMIDI_ME2000

    /** Download a WaveTable for internal storage and use.
     * The contents of the definition are specific to the synthesizer in use.
     * Returns negative error or positive waveTable token.
     */
    int SS_LoadWaveTable( HybridOrchestra_t *orchestra, unsigned char *data, int numBytes );

    /* Delete WaveTable if WaveSet reference count is zero. */
    int SS_UnloadWaveTable( HybridOrchestra_t *orchestra, spmSInt32 token );

    /** Download a WaveSet for internal storage and use.
     * The contents of the definition are specific to the synthesizer in use.
     * Returns negative error or positive waveSet token.
     */
    int SS_LoadWaveSet( HybridOrchestra_t *orchestra, ResourceTokenMap_t *tokenMap, unsigned char *data, int numBytes );

    /* Delete WaveSet if instrument reference count is zero. */
    int SS_UnloadWaveSet( HybridOrchestra_t *orchestra, spmSInt32 token );

#endif /* SPMIDI_ME2000 */

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_SYNTH_H */

