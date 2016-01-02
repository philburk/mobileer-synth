#ifndef _SPMIDI_HYBRID_H
#define _SPMIDI_HYBRID_H
/* $Id: spmidi_hybrid.h,v 1.41 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 *
 * Hybrid Synthesizer for SPMIDI Engine.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#if SPMIDI_ME3000
#include "dls_parser_internal.h"
#endif /* SPMIDI_ME3000 */
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi_voice.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/engine/compressor.h"
#include "spmidi/engine/instrument_mgr.h"
#include "reverb.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*****************************************************************/
    /********** typedefs *********************************************/
    /*****************************************************************/

#define HYBRID_FLAG_USE_PHASE_MOD       (0x00000001)
#define HYBRID_FLAG_MODOSC_USE_MAINENV  (0x00000002)
#define HYBRID_FLAG_ENABLE_FILTER       (0x00000004)
#define HYBRID_FLAG_USE_RING_MOD        (0x00000008)
#define HYBRID_FLAG_IGNORE_NOTE_OFF     (0x00000010)

    /* We don't need more unique infos than we do voices. */
#define SS_NUM_INFOS    (SPMIDI_MAX_VOICES)

#define SS_MIXER_SHIFT                  (5)
#define SS_MIXER_BUS_RESOLUTION         (27)
#define SS_MIXER_BUS_MAX                ((1 << (SS_MIXER_BUS_RESOLUTION - 1)) - 1)
#define SS_MIXER_BUS_MIN                (0 - (1 << (SS_MIXER_BUS_RESOLUTION - 1)))

    typedef struct HybridChannel_s
    {
        PitchOctave    tuningOctaveOffset;
        PitchOctave    pitchBendOctaveOffset;
        PitchOctave    lfoPitchModDepth;
        unsigned short volExpr; /* premultiplied volume * expression */
        unsigned short insBank; /* 14 bit bank number */
        unsigned char  program;
        unsigned char  volume;     /* CC7 */
        unsigned char  expression; /* CC11 */
        unsigned char  modDepth;   /* CC1 */
        unsigned char  pressure;   /* channel pressure */
        unsigned char  padding;    /* align structure */
    }
    HybridChannel_t;

    typedef struct HybridSynth_s
    {
        /* This SoftSynth structure must be first so we can type cast between them. */
        SoftSynth       softSynth;
        /** Info structure are used to cache preset related information that
         * must be calculated at run-time.
         */
        HybridVoice_Info_t infoCache[ SS_NUM_INFOS ];
        HybridChannel_t channels[ MIDI_NUM_CHANNELS ];
        HybridVoice_t   voices[ SPMIDI_MAX_VOICES ];

        /* Shared Output Buffers ---- */
        FXP31           lfoOutput[SS_BLOCKS_PER_BUFFER];
        FXP31           modEnvOutput[SS_BLOCKS_PER_BUFFER];
        FXP31           mainEnvOutput[SS_BLOCKS_PER_BUFFER];
        FXP31           ampEnvOutput[SS_BLOCKS_PER_BUFFER];

        FXP31           modOscOutput[SS_FRAMES_PER_BLOCK];

        FXP31           voiceOutput[SS_FRAMES_PER_BLOCK];

        /** Interleaved sample mixing buffer. Only use first half of buffer in mono mode. */
        FXP31           mixerBus[SPMIDI_MAX_SAMPLES_PER_FRAME * SS_FRAMES_PER_BUFFER];

        DLS_Orchestra_t *dlsOrchestra;

        int             sampleRate;
        int             controlRate;
        /** Offset for audio rate pitch calculations. 0 if SR = SS_BASE_SAMPLE_RATE */
        PitchOctave     srateOffset;
        SS_AutoStopCallback *autoStopCallback;
        void           *autoStopUserData;

        FXP31           mixAmplitude[2];

        /** Cumulative value for stolen voices. Ramp down continuously. */
        FXP27           stolenVoiceLeft;
        FXP27           stolenVoiceRight;
        /** Based on sample rate. */
        FXP27           stolenVoiceShift;

#if SPMIDI_USE_REVERB
        Reverb_t        reverb;
        FXP31           reverbOutput[SS_FRAMES_PER_BUFFER];
#endif /* SPMIDI_USE_REVERB */

#if SPMIDI_USE_COMPRESSOR

        Compressor_t    compressor;
        unsigned char   isCompressorEnabled;
#endif
        /** Is General MIDI off, allowing use of some test instruments. */
        unsigned char   isGeneralMidiOff;
        spmSInt8        transposition;
    }
    HybridSynth_t;

    typedef enum {
        NO_FLAGS = 0
    } VoiceFlags;

    /***************************************************************************/
    /******** Performance critical routines from spmidi_fast.c *****************/
    /***************************************************************************/
    void SS_SynthesizeVoiceME1000( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame );
    void SS_SynthesizeVoiceME2000( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame );

    /** Allow override of function used to implement SS_SynthesizeVoiceDLS2()
     * This allow modification to code in ROM.
     */
    typedef void (*SS_SynthesizeVoiceDLS2ProcPtr)( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame );
    SS_SynthesizeVoiceDLS2ProcPtr SS_GetProc_SynthesizeVoiceDLS2( void );
    void SS_SetProc_SynthesizeVoiceDLS2( SS_SynthesizeVoiceDLS2ProcPtr proc );

    void SS_SynthesizeVoiceDLS2( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame );

    /***************************************************************************/
    /******* In spmidi_hybrid.c ************************************************/
    /***************************************************************************/
    void SS_AutoStopVoice( SoftSynth *synth, HybridVoice_t *voice );


#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_HYBRID_H */
