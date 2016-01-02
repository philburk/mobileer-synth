/* $Id: spmidi_fast.c,v 1.18 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Mixer and high level voice synthesis.
 * This is performance critical code.
 * This code may be placed in tightly coupled memory to optimize performance.
 *
 * @author Copyright 2002-2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#if defined(WIN32) || defined(MACOSX)
#include <math.h>
#endif

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/midi.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/engine/spmidi_dls.h"
#include "spmidi/engine/compressor.h"
#include "spmidi/engine/adsr_envelope.h"
#include "spmidi/engine/oscillator.h"
#include "dls_parser_internal.h"

/* This macro is a code mixing operation.
 * It scales the input signal by the gain and adds it to an accumulator.
 */
#if SPMIDI_DSP_ARM946
/* Inline function for optimized ARM version.
 * Note this macro is missing the QADD so we have to adjust shiftby accordingly.
 */
__inline FXP31 MIX_SCALE_SHIFT_ADD( FXP31 accum, FXP31 signal, FXP31 gain, int shiftby )
{
    int temp;
    __asm
    {
        SMULWT    temp, signal, gain
        ADD       temp, accum, temp, asr shiftby
    }
    return temp;
}
#else

/* Portable 'C' version of core mixing element. */
#define MIX_SCALE_SHIFT_ADD( accum, signal, gain, shiftby ) \
    (accum +  (FXP31_MULT( signal, gain )  >> shiftby))
    
#endif

/********************************************************************
 * Mix output of voice into common mixing buffer.
 * @param input Mono signal from voice.
 * @param voice single instrument voice state structure
 * @param mixer output accumulator
 */
FXP31 *SS_MixVoiceBlock( FXP31 *input, HybridVoice_t *voice,
                         int samplesPerFrame, FXP31 ampGain,
                         FXP31 *mixer )
{
    FXP31  leftValue = 0, rightValue = 0;
    int shifter = SS_MIXER_SHIFT - voice->preset->boostLog2;
    int is;
    FXP31 leftEnvGain;

#if SPMIDI_SMOOTH_MIXER_GAIN
    /* Interpolate between adjacent gain values to remove zipper noise.
     * This is particularly important if running at low sample rates.
     */
    FXP31 leftEnvDelta;
    leftEnvGain = FXP31_MULT( voice->previousMixerGain, voice->leftGain );
    leftEnvDelta = FXP31_MULT( ampGain, voice->leftGain ) - leftEnvGain;
    leftEnvDelta = leftEnvDelta >> SS_FRAMES_PER_BLOCK_LOG2;

    /* Define macro to increase gain slightly for each output value. */
    #define BUMP_LEFT_GAIN  {leftEnvGain += leftEnvDelta;}
#else
    leftEnvGain = FXP31_MULT( ampGain, voice->leftGain );
    #define BUMP_LEFT_GAIN  /* Noop */
#endif

    /* Scale output and mix result with global mixer busses. */
    if( samplesPerFrame == 1 )
    {
#if SPMIDI_DSP_ARM946
        /* The ARM946 version of MIX_SCALE_SHIFT_ADD saves an instruction by
         * not doing the QADD. The QADD is an effective left shift by 1.
         * To account for that we need to adjust our right shifter by 1.
         */
        shifter -= 1;
#endif /* SPMIDI_DSP_ARM946 */

        /* Unroll loop. Assume block size is multiple of four. */
        is=SS_FRAMES_PER_BLOCK;
        while( is > 0 )
        {
            FXP31 m0,m1,m2;
            m0 = MIX_SCALE_SHIFT_ADD( mixer[0], *input++, leftEnvGain, shifter );
            BUMP_LEFT_GAIN;
            m1 = MIX_SCALE_SHIFT_ADD( mixer[1], *input++, leftEnvGain, shifter );
            BUMP_LEFT_GAIN;
            m2 = MIX_SCALE_SHIFT_ADD( mixer[2], *input++, leftEnvGain, shifter );
            BUMP_LEFT_GAIN;
            leftValue = MIX_SCALE_SHIFT_ADD( mixer[3], *input++, leftEnvGain, shifter );
            BUMP_LEFT_GAIN;

            /* Perform writes quickly to try and consolidate them into a single burst write.
             * This will improve speed on many architectures.
             */
            *mixer++ = m0;
            *mixer++ = m1;
            *mixer++ = m2;
            *mixer++ = leftValue;

            /* Decrement 4 at a time in this unrolled loop. */
            is -= 4;
        }
    }
    else
    {
        /* STEREO mix */
        FXP31 rightEnvGain;
#if SPMIDI_SMOOTH_MIXER_GAIN
        FXP31 rightEnvDelta;
        rightEnvGain = FXP31_MULT( voice->previousMixerGain, voice->rightGain );
        rightEnvDelta = FXP31_MULT( ampGain, voice->rightGain ) - rightEnvGain;
        rightEnvDelta = rightEnvDelta >> SS_FRAMES_PER_BLOCK_LOG2;
    #define BUMP_RIGHT_GAIN  {rightEnvGain += rightEnvDelta;}
#else
        rightEnvGain = FXP31_MULT( ampGain, voice->rightGain );
    #define BUMP_RIGHT_GAIN  /* Noop */
#endif

#if SPMIDI_DSP_ARM946
        /* The ARM946 version of MIX_SCALE_SHIFT_ADD saves an instruction by
         * not doing the QADD. The QADD is an effective left shift by 1.
         * To account for that we need to adjust our right shifter by 1.
         */
        shifter -= 1;
#endif /* SPMIDI_DSP_ARM946 */

        /* Unroll loop. Assume block size is multiple of four. */
        is=SS_FRAMES_PER_BLOCK;
        while( is > 0 )
        {               
/* Define macro to mix a mono signal to a stereo output accumulator. */
#define MIX_STEREO \
            { \
            FXP31 sample = *input++; \
            leftValue = mixer[0]; \
            rightValue = mixer[1]; \
            leftValue = MIX_SCALE_SHIFT_ADD( leftValue, sample, leftEnvGain, shifter ); \
            BUMP_LEFT_GAIN; \
            rightValue = MIX_SCALE_SHIFT_ADD( rightValue, sample, rightEnvGain, shifter ); \
            BUMP_RIGHT_GAIN; \
            /* Write together so we make better use of write buffer. */ \
            *mixer++ = leftValue; \
            *mixer++ = rightValue; \
            }

            MIX_STEREO;
            MIX_STEREO;
            MIX_STEREO;
            MIX_STEREO;

            /* Decrement 4 at a time in this unrolled loop. */
            is -= 4;
        }
    }

    /* Save last values for stolen crossfade. */
    voice->lastLeftValue = leftValue;
    voice->lastRightValue = rightValue;

#if SPMIDI_SMOOTH_MIXER_GAIN
    /* Save last gain for smoothing. */
    voice->previousMixerGain = ampGain;
#endif

    return mixer;
}


/********************************************************************
 * Synthesize a single complete voice using Hybrid Synthesis.
 */
void SS_SynthesizeVoiceME1000( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame )
{
    int    ib, is;
    FXP16  basePitch;
    FXP31 *mixer;
    HybridChannel_t *channel;
    HybridVoice_Info_t *info = voice->info;
    const HybridVoice_Preset_t *preset = voice->preset;
    int    isPitchModulated = 0;
    int    doAutoStop;
    PitchOctave lfoPitchModDepth;

    channel = &hybridSynth->channels[ voice->channel ];
    basePitch = voice->baseNotePitch + channel->pitchBendOctaveOffset;

    /* Combine setting from preset and ModWheel */
    lfoPitchModDepth = info->preset->lfoPitchModDepth + channel->lfoPitchModDepth;

    /* Calculate control rate units. ********************/
    /* LFO */
    if( (lfoPitchModDepth != 0) || (info->lfoCutoffModDepth != 0) )
    {
        (*voice->lfo.nextProc)( &voice->lfo,
                               FXP31_MAX_VALUE,
                               &hybridSynth->lfoOutput[0],
                               NULL );
    }

    isPitchModulated = (lfoPitchModDepth != 0) || (info->envPitchModDepth != 0);

    if( !isPitchModulated )
    {
        /* If no LFO then just set pitch once at start of block so we pick up pitchBend. */
        Osc_SetPitch( &info->preset->modOsc, &voice->modOsc, basePitch, hybridSynth->srateOffset );
        Osc_SetPitch( &info->preset->mainOsc, &voice->mainOsc, basePitch, hybridSynth->srateOffset );
    }

    /* ADSR envelopes */
    ADSR_Next( &preset->modEnv, &info->modEnv, &voice->modEnv, hybridSynth->modEnvOutput, NULL );
    ADSR_Next( &preset->mainEnv, &info->mainEnv, &voice->mainEnv, hybridSynth->mainEnvOutput, NULL );
    doAutoStop = ADSR_Next( &preset->ampEnv, &info->ampEnv, &voice->ampEnv, hybridSynth->ampEnvOutput, NULL );

    if( samplesPerFrame == 1 )
    {
        voice->leftGain = voice->rightGain = voice->velocity14 * channel->volExpr;
    }
    else
    {
        voice->leftGain = voice->leftVelocityPan * channel->volExpr;
        voice->rightGain = voice->rightVelocityPan * channel->volExpr;
    }

    /* Calculate audio rate units. ***********************/
    mixer = hybridSynth->mixerBus;
    for( ib=0; ib<SS_BLOCKS_PER_BUFFER; ib++ )
    {

        FXP31 modAmplitude;

        /* Determine which envelope controls modOsc amplitude. */
        if( (info->flags & HYBRID_FLAG_MODOSC_USE_MAINENV) != 0)
        {
            modAmplitude = FXP31_MULT(hybridSynth->mainEnvOutput[ib], voice->phaseModDepth);
        }
        else
        {
            modAmplitude = FXP31_MULT(hybridSynth->modEnvOutput[ib], voice->phaseModDepth);
        }

        /* Set pitches based on LFO pitch modulation. */
        if( isPitchModulated )
        {
            PitchOctave pitchSum = basePitch;
            pitchSum += FXP_MULT_16_31_16( hybridSynth->lfoOutput[ib], lfoPitchModDepth );
            pitchSum += FXP_MULT_16_31_16( hybridSynth->modEnvOutput[ib], info->envPitchModDepth );

            Osc_SetPitch( &info->preset->modOsc, &voice->modOsc, pitchSum, hybridSynth->srateOffset );
            Osc_SetPitch( &info->preset->mainOsc, &voice->mainOsc, pitchSum, hybridSynth->srateOffset );
        }

        if( (info->flags & HYBRID_FLAG_USE_PHASE_MOD) != 0 )
        {

            /* Run oscillators with modOsc phase modulating mainOsc. */
            (*voice->modOsc.nextProc)(
                &voice->modOsc,
                modAmplitude,
                &hybridSynth->modOscOutput[0],
                NULL );

            (*voice->mainOsc.nextProc)(
                &voice->mainOsc,
                hybridSynth->mainEnvOutput[ib],
                &hybridSynth->voiceOutput[0],
                &hybridSynth->modOscOutput[0] );
        }
        else
        {
            /* No PhaseModulation so mix two oscillators at half amplitude. */
            FXP31 *bufMod = &hybridSynth->modOscOutput[0];
            FXP31 *bufMix = &hybridSynth->voiceOutput[0];

            (*voice->modOsc.nextProc)(
                &voice->modOsc,
                modAmplitude >> 1, /* half amplitude */
                bufMod,
                NULL );

            (*voice->mainOsc.nextProc)(
                &voice->mainOsc,
                hybridSynth->mainEnvOutput[ib] >> 1, /* half amplitude */
                bufMix,
                NULL );

            if( (info->flags & HYBRID_FLAG_USE_RING_MOD) != 0 )
            {
                /* Multiply output of two oscillators. */
                for( is=0; is<SS_FRAMES_PER_BLOCK; is++ )
                {
                    FXP31 a = *bufMod++;
                    FXP31 b = *bufMix;
                    *bufMix++ = FXP31_MULT( a, b );
                }
            }
            else
            {
                /* Mix output of two oscillators. */
                for( is=0; is<SS_FRAMES_PER_BLOCK; is++ )
                {
                    FXP31 b = *bufMix;
                    *bufMix++ = *bufMod++ + b;
                }
            }
        }

        /* FILTER mix of two oscillators. */
        if( (info->flags & HYBRID_FLAG_ENABLE_FILTER) != 0)
        {
            FXP16 filterPitch = info->filter.cutoffPitch;
            if( (info->filter.flags & SVFILTER_FLAG_ABS_PITCH) == 0 )
            {
                filterPitch += basePitch;
            }
            filterPitch += FXP_MULT_16_31_16( hybridSynth->modEnvOutput[ib], info->envCutoffModDepth );
            filterPitch += FXP_MULT_16_31_16( hybridSynth->lfoOutput[ib], info->lfoCutoffModDepth );
            filterPitch += hybridSynth->srateOffset;

            SVFilter_SetPitch( &voice->filter, filterPitch );
            SVFilter_Next( &voice->filter, &hybridSynth->voiceOutput[0] );

        }

        mixer = SS_MixVoiceBlock( hybridSynth->voiceOutput, voice, samplesPerFrame,
                                  hybridSynth->ampEnvOutput[ib],
                                  mixer );

    }

    if( doAutoStop )
    {
        SS_AutoStopVoice( (SoftSynth *) hybridSynth, voice );
    }
    /* Return immediately after SS_AutoStopVoice() because voice info not valid. */
}


#if SPMIDI_ME2000

/********************************************************************
 * Synthesize a single complete voice using Hybrid Synthesis.
 * This is called then the mainOsc waveform is WAVETABLE.
 */
void SS_SynthesizeVoiceME2000( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame )
{
    int    ib;
    FXP16  basePitch;
    FXP31 *mixer;
    HybridChannel_t *channel;
    HybridVoice_Info_t *info = voice->info;
    const HybridVoice_Preset_t *preset = voice->preset;
    int    isPitchModulated = 0;
    int    doAutoStop;
    PitchOctave lfoPitchModDepth;

    channel = &hybridSynth->channels[ voice->channel ];
    basePitch = voice->baseNotePitch + channel->pitchBendOctaveOffset;

    /* Combine setting from preset and ModWheel */
    lfoPitchModDepth = info->preset->lfoPitchModDepth + channel->lfoPitchModDepth;

    /* Calculate control rate units. ********************/
    /* LFO */
    if( (lfoPitchModDepth != 0) || (info->lfoCutoffModDepth != 0) )
    {
        (*voice->lfo.nextProc)( &voice->lfo,
                               FXP31_MAX_VALUE,
                               &hybridSynth->lfoOutput[0],
                               NULL );
    }

    isPitchModulated = (lfoPitchModDepth != 0) || (info->envPitchModDepth != 0);

    if( !isPitchModulated )
    {
        /* If no LFO then just set pitch once at start of block so we pick up pitchBend. */
        Osc_SetWavePitch( &info->preset->mainOsc, &voice->mainOsc, basePitch, hybridSynth->srateOffset, voice->waveSetRegion );
    }

    /* ADSR envelopes */
    ADSR_Next( &preset->modEnv, &info->modEnv, &voice->modEnv, hybridSynth->modEnvOutput, NULL );
    doAutoStop = ADSR_Next( &preset->ampEnv, &info->ampEnv, &voice->ampEnv, hybridSynth->ampEnvOutput, NULL );

    if( samplesPerFrame == 1 )
    {
        voice->leftGain = voice->rightGain = ((int)voice->velocity14) * (int)channel->volExpr ;
    }
    else
    {
        voice->leftGain = voice->leftVelocityPan * channel->volExpr;
        voice->rightGain = voice->rightVelocityPan * channel->volExpr;
    }

    /* Calculate audio rate units. ***********************/
    mixer = hybridSynth->mixerBus;
    for( ib=0; ib<SS_BLOCKS_PER_BUFFER; ib++ )
    {

        /* Set pitches based on LFO pitch modulation. */
        if( isPitchModulated )
        {
            
            PitchOctave pitchSum = basePitch;
            pitchSum += FXP_MULT_16_31_16( hybridSynth->lfoOutput[ib], lfoPitchModDepth );
            pitchSum += FXP_MULT_16_31_16( hybridSynth->modEnvOutput[ib], info->envPitchModDepth );

            Osc_SetWavePitch( &info->preset->mainOsc, &voice->mainOsc, pitchSum, hybridSynth->srateOffset, voice->waveSetRegion );
        }

        if( voice->waveSetRegion->table->type == SPMIDI_WAVE_TYPE_ALAW )
        {
            /* Generate one block of wavetable output. */
            Osc_WaveTableALaw( &voice->mainOsc, voice->waveSetRegion->table, &hybridSynth->voiceOutput[0] );
        }
        else
        {
            /* Generate one block of wavetable output. */
            Osc_WaveTableS16( &voice->mainOsc, voice->waveSetRegion->table, &hybridSynth->voiceOutput[0] );

            /* If we have hit the loop then start the decay portion. */
            if( ((preset->ampEnv.flags & ADSR_FLAG_WAIT_DECAY) != 0) &&
                (voice->mainOsc.shared.wave.sampleIndex >= voice->waveSetRegion->table->loopBegin) )
            {
                ADSR_TriggerDecay( &voice->ampEnv );
            }
        }

        /* FILTER mix of two oscillators. */
        if( (info->flags & HYBRID_FLAG_ENABLE_FILTER) != 0)
        {
            FXP16 filterPitch = info->filter.cutoffPitch;
            if( (info->filter.flags & SVFILTER_FLAG_ABS_PITCH) == 0 )
            {
                filterPitch += basePitch;
            }
            filterPitch += FXP_MULT_16_31_16( hybridSynth->modEnvOutput[ib], info->envCutoffModDepth );
            filterPitch += FXP_MULT_16_31_16( hybridSynth->lfoOutput[ib], info->lfoCutoffModDepth );
            filterPitch += hybridSynth->srateOffset;

            SVFilter_SetPitch( &voice->filter, filterPitch );
            SVFilter_Next( &voice->filter, &hybridSynth->voiceOutput[0] );
        }

        mixer = SS_MixVoiceBlock( hybridSynth->voiceOutput, voice, samplesPerFrame,
                                  hybridSynth->ampEnvOutput[ib],
                                  mixer );
    }

    /* Is the wavetable playback finished? */
    doAutoStop |= (voice->mainOsc.shared.wave.sampleIndex < 0);

    if( doAutoStop )
    {
        SS_AutoStopVoice( (SoftSynth *) hybridSynth, voice );
    }
    /* Return immediately after SS_AutoStopVoice() because voice info not valid. */
}
#endif /* SPMIDI_ME2000 */


#if SPMIDI_ME3000
/********************************************************************
 * Synthesize a single complete voice using DLS2 Architecture.
 * Generates SS_FRAMES_PER_BUFFER frames per call.
 */
void SS_SynthesizeVoiceDLS2_Internal( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame )
{
    int    ib;
    FXP16  basePitch;
    FXP31 *mixer;
    HybridChannel_t *channel;
    HybridVoice_Info_t *info = voice->info;
    const HybridVoice_Preset_t *preset = voice->preset;
    int    isPitchModulated = 0;
    int    doAutoStop;
    PitchOctave modLFOPitchModDepth;
    PitchOctave vibLFOPitchModDepth;

    channel = &hybridSynth->channels[ voice->channel ];
    /* For wavetable oscillator. */
    basePitch = voice->baseNotePitch + channel->pitchBendOctaveOffset;

    /* Combine setting from preset, ModWheel and channel pressure. */
    modLFOPitchModDepth = (voice->dlsRegion->modLFOCC1toPitch * channel->modDepth) >> 7;
    modLFOPitchModDepth += (voice->dlsRegion->modLFOCPRtoPitch * channel->pressure) >> 7;
    modLFOPitchModDepth += info->preset->lfoPitchModDepth;

    /* Calculate control rate units. ********************/
    /* LFO */
    if( (modLFOPitchModDepth != 0) ||
        (info->lfoCutoffModDepth != 0) )
    {
        if( voice->modLFOStartDelayCounter == 0 )
        {
            /* DLS2 only uses Sine waveform. */
            Osc_Next_Sine( &voice->lfo,
                                   FXP31_MAX_VALUE,
                                   &hybridSynth->lfoOutput[0],
                                   NULL );
        }
        else
        {
            voice->modLFOStartDelayCounter -= 1;
            /* Since we are not running the oscillator, we need to clear the buffer
             * to prevent leftover signal from causing modulation.
             */
            MemTools_Clear( &hybridSynth->lfoOutput[0], sizeof( hybridSynth->lfoOutput ) );
        }
    }

    /* Combine setting from preset, ModWheel and channel pressure. */
    vibLFOPitchModDepth = (voice->dlsRegion->vibLFOCC1toPitch * channel->modDepth) >> 7;
    vibLFOPitchModDepth += (voice->dlsRegion->vibLFOCPRtoPitch * channel->pressure) >> 7;
    vibLFOPitchModDepth += voice->vibratoPitchModDepth;

    if( vibLFOPitchModDepth != 0 )
    {
        if( voice->vibratoLFOStartDelayCounter == 0 )
        {
            /* DLS2 only uses Sine waveform. */
            /* We are using the ModOsc as the Vibrato LFO. */
            Osc_Next_Sine( &voice->vibratoLFO,
                                   FXP31_MAX_VALUE,
                                   &hybridSynth->modOscOutput[0],
                                   NULL );
        }
        else
        {
            voice->vibratoLFOStartDelayCounter -= 1;
            /* Since we are not running the oscillator, we need to clear the buffer
             * to prevent leftover signal from causing modulation.
             */
            MemTools_Clear( &hybridSynth->modOscOutput[0], sizeof( hybridSynth->modOscOutput ) );
        }
    }

    isPitchModulated = (modLFOPitchModDepth != 0) || (vibLFOPitchModDepth != 0) || (info->envPitchModDepth != 0);

    /* If no PITCH mod then just set oscillator pitch once at start of block so we pick up pitchBend. */
    if( !isPitchModulated )
    {
        Osc_SetWavePitch( &info->preset->mainOsc, &voice->mainOsc, basePitch, hybridSynth->srateOffset, &voice->dlsRegion->waveSetRegion );
    }

    /* ADSR envelopes */
    ADSR_Next( &preset->modEnv, &info->modEnv, &voice->modEnv, hybridSynth->modEnvOutput,
        &voice->dlsRegion->eg2Extension );
    doAutoStop = ADSR_Next( &preset->ampEnv, &info->ampEnv, &voice->ampEnv, hybridSynth->ampEnvOutput,
        &voice->dlsRegion->eg1Extension );

    if( samplesPerFrame == 1 )
    {
        voice->leftGain = voice->rightGain = ((int)voice->velocity14) * (int)channel->volExpr ;
    }
    else
    {
        voice->leftGain = voice->leftVelocityPan * channel->volExpr;
        voice->rightGain = voice->rightVelocityPan * channel->volExpr;
    }

    /* Calculate audio rate units. ***********************/
    mixer = hybridSynth->mixerBus;
    for( ib=0; ib<SS_BLOCKS_PER_BUFFER; ib++ )
    {

        /* Set pitches based on LFO pitch modulation. */
        if( isPitchModulated )
        {
            PitchOctave pitchSum = basePitch;
            pitchSum += FXP_MULT_16_31_16( hybridSynth->lfoOutput[ib], modLFOPitchModDepth );
            /* We are using the ModOsc as the Vibrato LFO. */
            pitchSum += FXP_MULT_16_31_16( hybridSynth->modOscOutput[ib], vibLFOPitchModDepth );
            pitchSum += FXP_MULT_16_31_16( hybridSynth->modEnvOutput[ib], info->envPitchModDepth );

            Osc_SetWavePitch( &info->preset->mainOsc, &voice->mainOsc, pitchSum, hybridSynth->srateOffset, &voice->dlsRegion->waveSetRegion );
        }

        /* Generate one block of wavetable output. */
        if( voice->dlsRegion->dlsWave->bytesPerSample == 1 )
        {
            
            if( voice->dlsRegion->dlsWave->format == WAVE_FORMAT_ALAW )
            {
                Osc_WaveTableALaw( &voice->mainOsc, voice->waveSetRegion->table, &hybridSynth->voiceOutput[0] );
            }
            else
            {
                Osc_WaveTableU8( &voice->mainOsc, voice->waveSetRegion->table, &hybridSynth->voiceOutput[0] );
            }
        }
        else
        {
            Osc_WaveTableS16( &voice->mainOsc, voice->waveSetRegion->table, &hybridSynth->voiceOutput[0] );
        }

        /* FILTER osc output. */
        if( (info->flags & HYBRID_FLAG_ENABLE_FILTER) != 0)
        {
            FXP16 filterPitch = info->filter.cutoffPitch;
            if( (info->filter.flags & SVFILTER_FLAG_ABS_PITCH) == 0 )
            {
                filterPitch += basePitch;
            }
            filterPitch += FXP_MULT_16_31_16( hybridSynth->modEnvOutput[ib], info->envCutoffModDepth );
            filterPitch += FXP_MULT_16_31_16( hybridSynth->lfoOutput[ib], info->lfoCutoffModDepth );
            filterPitch += hybridSynth->srateOffset;

            SVFilter_SetPitch( &voice->filter, filterPitch );
            SVFilter_Next( &voice->filter, &hybridSynth->voiceOutput[0] );
        }

        mixer = SS_MixVoiceBlock( hybridSynth->voiceOutput, voice, samplesPerFrame,
                                  hybridSynth->ampEnvOutput[ib],
                                  mixer );
    }

    /* Is the wavetable playback finished? */
    doAutoStop |= (voice->mainOsc.shared.wave.sampleIndex < 0);

    if( doAutoStop )
    {
        SS_AutoStopVoice( (SoftSynth *) hybridSynth, voice );
    }
    /* Return immediately after SS_AutoStopVoice() because voice info not valid. */
}

/*******************************************************************************/
/* Allow patching of this function for customization. */

/* NULL means use internal function instead of custom function. */
SS_SynthesizeVoiceDLS2ProcPtr sSynthesizeDLSProc = NULL;

/*******************************************************************************/
SS_SynthesizeVoiceDLS2ProcPtr SS_GetProc_SynthesizeVoiceDLS2( void )
{
    if( sSynthesizeDLSProc != NULL ) return sSynthesizeDLSProc;
    else return &SS_SynthesizeVoiceDLS2_Internal;
}

/*******************************************************************************/
void SS_SetProc_SynthesizeVoiceDLS2( SS_SynthesizeVoiceDLS2ProcPtr proc )
{
    sSynthesizeDLSProc = proc;
}

/*******************************************************************************/
void SS_SynthesizeVoiceDLS2( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame )
{
    SS_GetProc_SynthesizeVoiceDLS2()( hybridSynth, voice, samplesPerFrame );
}

#endif /* SPMIDI_ME3000 */
