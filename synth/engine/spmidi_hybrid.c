/* $Id: spmidi_hybrid.c,v 1.84 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 *
 * Hybrid SPMIDI Synth that uses
 * oscillators, envelopes, filter, and LFO.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#if defined(WIN32) || defined(MACOSX)
#include <math.h>
#endif

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/engine/spmidi_orchestra.h"
#include "spmidi/engine/spmidi_dls.h"
#include "spmidi/engine/compressor.h"
#include "spmidi/engine/adsr_envelope.h"
#include "spmidi/engine/oscillator.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/engine/memtools.h"

#include "spmidi/include/spmidi_editor.h"
#include "spmidi/include/spmidi_print.h"

/* Print sizes of major data structures. */
#define SPMIDI_REPORT_SIZES (0)

#define SPMIDI_TEST_WAVETABLE  (0)

#ifndef SPMIDI_RAMP_STOLEN_VOICES
#define SPMIDI_RAMP_STOLEN_VOICES (0)
#endif

/* Determine whether instrument is read-only. */
#if SPMIDI_SUPPORT_LOADING
#define EDITABLE /* */
#else
#define EDITABLE const
#endif

/* Use detailed estimator that actually runs envelopes. */
#ifndef USE_ENVELOPE_IN_ESTIMATION
#define USE_ENVELOPE_IN_ESTIMATION (0)
#endif



#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#ifndef FALSE
#define FALSE (0)
#define TRUE  (1)
#endif


/** Determines max samples it takes to cross fade
 * from stolen voice value to new value.
 * Max time is MIN_STOLEN_CROSS_FADE_DELTA / frameRate.
 */
#define MIN_STOLEN_CROSS_FADE_DELTA  (FXP31_MAX_VALUE >> 9)

/* Put SYNTH structure at fixed address. */
/* #define SYNTH_AT_FIXED_ADDRESS (0x20025800) */

#define IS_VOICE_DLS( voice ) ((voice)->dlsInstrument != NULL)

/*****************************************************************/
/********** global data ******************************************/
/*****************************************************************/

#if SPMIDI_SUPPORT_MALLOC
#define HybridSynth_Allocate()   SPMIDI_ALLOC_MEM( sizeof(HybridSynth_t), "HybridSynth_t" )
#define HybridSynth_Free(cntxt)  SPMIDI_FreeMemory(cntxt)
#else

/* Pool of synths that can be allocated without using malloc. */
#ifdef SYNTH_AT_FIXED_ADDRESS
#define sHybridSynthPool ((HybridSynth_t *)  SYNTH_AT_FIXED_ADDRESS)
#else /* SYNTH_AT_FIXED_ADDRESS */
static HybridSynth_t  sHybridSynthPool[SPMIDI_MAX_NUM_CONTEXTS];
#endif  /* SYNTH_AT_FIXED_ADDRESS */

/* Array used to keep track of which ones are allocated. */
static char sHybridSynthsAllocated[SPMIDI_MAX_NUM_CONTEXTS] = { 0 };

/* Scan array for unallocated synth. Allocate it if free. */
static HybridSynth_t *HybridSynth_Allocate( void )
{
    int i;
    HybridSynth_t *synth = NULL;

    /* Protect the following section of code from thread collisions. */
    SPMIDI_EnterCriticalSection();

    for( i=0; i<SPMIDI_MAX_NUM_CONTEXTS; i++ )
    {
        /* Warning! This is a non-atomic test and set. It is not thread safe.
         * The application should implement SPMIDI_EnterCriticalSection() if
         * multiple threads can collide in this code.
         */
        if( sHybridSynthsAllocated[i] == 0 )
        {
            sHybridSynthsAllocated[i] = 1;
            synth = &sHybridSynthPool[i];
            break;
        }
    }

    SPMIDI_LeaveCriticalSection();

    return synth;
}

/* Scan pool to match synth being freed. Mark it as free if found. */
static void HybridSynth_Free( HybridSynth_t *synth )
{
    int i;
    /* Protect the following section of code from thread collisions. */
    SPMIDI_EnterCriticalSection();
    for( i=0; i<SPMIDI_MAX_NUM_CONTEXTS; i++ )
    {
        if( &sHybridSynthPool[i] == synth )
        {
            sHybridSynthsAllocated[i] = 0; /* Mark as free. */
            break;
        }
    }
    SPMIDI_LeaveCriticalSection();
}
#endif

/*****************************************************************/
/* Pitch offsets adjust synth tuning to account for different sample rates.*/
/*****************************************************************/
typedef struct SampleRatePitchOffsets_s
{
    int rate;
    PitchOctave pitchOffset;
}
SampleRatePitchOffsets;

static const SampleRatePitchOffsets sSupportedRates[] =
    {
        {
            7350, 169408
        },
        {  8000, 161395 },
        { 11025, 131072 },
        { 12000, 123059 },
        { 16000,  95859 },
        { 22050,  65536 },
        { 24000,  57523 },
        { 32000,  30323 },
        { 44100,      0 },
        { 48000,  -8012 },
        { 96000, -73548 },
    };
#define NUM_SUPPORTED_RATES  (sizeof(sSupportedRates)/sizeof(SampleRatePitchOffsets))
/*
To add support for new sample rates:
1) add an entry to sSupportedRates above with the new rate and a pitchOffset of zero,
2) enable and run the code below to generate the pitch offsets.
*/
#if 0
void main( void );
void main( void )
{
    int i;
    for( i=0; i<NUM_SUPPORTED_RATES; i++ )
    {
        double ratio = (double) SS_BASE_SAMPLE_RATE / sSupportedRates[i].rate;
        double ratioLog2 = log(ratio) / log(2.0);
        PitchOctave pitchOffset = (PitchOctave) ((1<<SS_PITCH_SHIFT) * ratioLog2);
        printf("    { %5d, %6d },\n", sSupportedRates[i].rate, pitchOffset );
    }
}
#endif

/*****************************************************************/
static const short sPanTable[] =
    {
        0,    408,    816,   1225,   1633,   2041,   2448,   2855,
        3262,   3668,   4074,   4479,   4883,   5287,   5689,   6091,
        6492,   6892,   7291,   7689,   8085,   8480,   8874,   9267,
        9658,  10047,  10435,  10822,  11206,  11589,  11971,  12350,
        12727,  13103,  13476,  13847,  14217,  14583,  14948,  15310,
        15670,  16028,  16383,  16735,  17085,  17433,  17777,  18119,
        18458,  18794,  19127,  19457,  19784,  20108,  20429,  20747,
        21062,  21373,  21681,  21986,  22287,  22584,  22879,  23169,
        23456,  23740,  24019,  24295,  24568,  24836,  25100,  25361,
        25618,  25870,  26119,  26364,  26604,  26841,  27073,  27301,
        27525,  27744,  27959,  28170,  28377,  28579,  28776,  28969,
        29158,  29342,  29522,  29696,  29867,  30032,  30194,  30350,
        30501,  30648,  30790,  30928,  31060,  31188,  31311,  31429,
        31542,  31650,  31753,  31852,  31945,  32033,  32117,  32195,
        32269,  32337,  32401,  32459,  32512,  32560,  32604,  32642,
        32675,  32703,  32726,  32744,  32756,  32764,  32766,  32764,
    };

#if 0
/* Generate sPanTable for lookup. */
int main( void )
{
    int i,j,index;
    short panGain;
    printf("static const short sPanTable[] = {\n" );
    /* Generate an sine table for panning. */
    index = 0;
    for( i = 0; i<(128/8); i++ )
    {
        for( j = 0; j<8; j++ )
        {
            int pan126 = (index > 0) ? index - 1 : index;
            panGain = (short) (FXP15_MAX_VALUE * sin( ((M_PI/2) * pan126) / 126 ));
            printf(" %6d,", panGain );
            index += 1;
        }
        printf("\n" );
    }
    printf("};\n" );
}
#endif

#define CUSTOM_RISEFALL_TIME   (10)

#if SPMIDI_ME3000
/* This preset is used to play disabled voices. */
static const HybridVoice_Preset_t sSilentQuickPreset =
{ /* Pure Sine */
    { SINE, 0, 0x0 },

    /* Use absolute Pitch zero in mainOsc so oscillator makes no sound. */
    { SINE, OSC_FLAG_ABSOLUTE_PITCH, 0x0 },

    { SINE, 0, 0x0 },
    { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
    { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },

    /* Use zero times in amp envelope so instrument stops ASAP */
    /* Don't wait for noteOff. Just go straight through. */
    { 0, 0, 1023, 0, 0, 0 | 0 | ADSR_FLAG_NO_WAIT },

    { 0x0, 0x0, 0 },
    0x00000000, /* phaseModDepth */
    0x0, /* lfoPitchModDepth */
    0x0, /* lfoCutoffModDepth */
    0x0, /* envPitchModDepth */
    0x0, /* envCutoffModDepth */
    0 | 0 | 0, /* flags */
    1, /* boostLog2 */
    60, /* keyCenter */
    128, /* keyScalar */
};
#endif

/*****************************************************************/
/* Hand build instruments for testing. */

/* Pointers inside structures must be initialized by code
 * at run-time instead of at compile-time. This is to simplify
 * building relocatable code.
 */

#if SPMIDI_TEST_WAVETABLE
static const short gTestWaveSamples[] = {
    0, 0,
    -30000,
    0, 0, 0, 6000,
    0, 0, 0, 0,
    30000,
};

/* If non-relocatable, then we can make structures const and init with pointers. */
#if SPMIDI_RELOCATABLE
    #define NRLCONST  /* */
#else
    #define NRLCONST  const
#endif

static NRLCONST WaveTable_t gTestWaveTable =
{
    {{0}}, /* tracker */
#if SPMIDI_RELOCATABLE
    (spmSample *) 0,
#else
    (spmSample *) gTestWaveSamples,
#endif
    sizeof(gTestWaveSamples)/sizeof(short),
    SPMUtil_MIDIPitchToOctave(0x564233),
    -157387, /* SR = 8346 */
    2, // loopBegin
    12, // loopEnd
};

#if SPMIDI_USE_REGIONS
    static WaveSetRegion_t gTestWave_Regions[1] = {{0,127,0,127}};
#else
    static NRLCONST WaveTable_t *gTestWave_Tables[1];
#endif

static WaveSet_t gTestWave_Set = {{{0}}};
#define SPMIDI_WAVESET_TestWave   (99)

static void SS_LoadTestWaves( WaveManager_t *waveManager )
{
#if SPMIDI_RELOCATABLE
  gTestWaveTable.samples = (spmSample *) gTestWaveSamples;
#endif
  gTestWave_Set.numTables = 1;
#if SPMIDI_USE_REGIONS
  gTestWave_Regions[0].table = &gTestWaveTable;
  gTestWave_Set.regions = (WaveSetRegion_t *)gTestWave_Regions;
#else
  gTestWave_Tables[0] = &gTestWaveTable;
  gTestWave_Set.tables = (WaveTable_t **)gTestWave_Tables;
#endif
  WaveManager_AddWaveSet( waveManager, &gTestWave_Set, SPMIDI_WAVESET_TestWave );
}
#endif

static const HybridVoice_Preset_t gHybridSynthTestPresets[] =
    {
        { /* Pure Sine */
            { SINE, 0, 0x0 },
            { SINE, 0, 0x0 },
            { SINE, 0, 0x0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { 0x0, 0x0, 0 },
            0x00000000, /* phaseModDepth */
            0x0, /* lfoPitchModDepth */
            0x0, /* lfoCutoffModDepth */
            0x0, /* envPitchModDepth */
            0x0, /* envCutoffModDepth */
            0 | 0 | 0, /* flags */
            1, /* boostLog2 */
            60, /* keyCenter */
            128, /* keyScalar */
        },
        { /* Ringing Filter */
            { WHITENOISE, 0, 0x0 },
            { PARTICLE, 0, 0x0 },
            { SINE, 0, 0x0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 0000, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { 0x0, 0x0, 0 },
            0x01000000, /* phaseModDepth */
            0x0, /* lfoPitchModDepth */
            0x0, /* lfoCutoffModDepth */
            0x0, /* envPitchModDepth */
            0x0, /* envCutoffModDepth */
            0 | 0 | HYBRID_FLAG_ENABLE_FILTER, /* flags */
            6, /* boostLog2 */
            60, /* keyCenter */
            128, /* keyScalar */
        },
        { /* Pure Sawtooth */
            { SINE, 0, 0x0 },
            { SAWTOOTH, 0, 0x0 },
            { SINE, 0, 0x0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { 0x0, 0x0, 0 },
            0x00000000, /* phaseModDepth */
            0x0, /* lfoPitchModDepth */
            0x0, /* lfoCutoffModDepth */
            0x0, /* envPitchModDepth */
            0x0, /* envCutoffModDepth */
            0 | 0 | 0, /* flags */
            1, /* boostLog2 */
            60, /* keyCenter */
            128, /* keyScalar */
        },
        { /* Pure Square */
            { SINE, 0, 0x0 },
            { SQUARE, 0, 0x0 }, /* This is the only oscillator which will contribute to the mix. */
            { SINE, 0, 0x0 },  /* phaseModDepth is zero so this is not heard */
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { 0x0, 0x0, 0 },
            0x00000000, /* phaseModDepth */
            0x0, /* lfoPitchModDepth */
            0x0, /* lfoCutoffModDepth */
            0x0, /* envPitchModDepth */
            0x0, /* envCutoffModDepth */
            0 | 0 | 0, /* flags */
            1, /* boostLog2 */
            60, /* keyCenter */
            128, /* keyScalar */
        },
        { /* Pure Square using raw harsh aliasing implementation. */
            { SINE, 0, 0x0 },
            { SQUARE_HARSH, 0, 0x0 }, /* This is the only oscillator which will contribute to the mix. */
            { SINE, 0, 0x0 },  /* phaseModDepth is zero so this is not heard */
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { 0x0, 0x0, 0 },
            0x00000000, /* phaseModDepth */
            0x0, /* lfoPitchModDepth */
            0x0, /* lfoCutoffModDepth */
            0x0, /* envPitchModDepth */
            0x0, /* envCutoffModDepth */
            0 | 0 | 0, /* flags */
            1, /* boostLog2 */
            60, /* keyCenter */
            128, /* keyScalar */
        },
#if SPMIDI_TEST_WAVETABLE
        { /* Simple wavetable for testing. */
            { SINE, 0, 0x0 },
            { WAVETABLE, 0, 0x0 }, /* This is the only oscillator which will contribute to the mix. */
            { SINE, 0, 0x0 },  /* phaseModDepth is zero so this is not heard */
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { CUSTOM_RISEFALL_TIME, 1, 1023, CUSTOM_RISEFALL_TIME, 0, 0 | 0 | 0 },
            { 0x0, 0x0, 0 },
            0x00000000, /* phaseModDepth */
            0x0, /* lfoPitchModDepth */
            0x0, /* lfoCutoffModDepth */
            0x0, /* envPitchModDepth */
            0x0, /* envCutoffModDepth */
            0 | 0 | 0, /* flags */
            1, /* boostLog2 */
            60, /* keyCenter */
            128, /* keyScalar */
            SPMIDI_WAVESET_TestWave
        },
#endif
    };
#define SS_NUM_TEST_PRESETS   (sizeof(gHybridSynthTestPresets)/sizeof(HybridVoice_Preset_t))
#undef CUSTOM_RISEFALL_TIME

/********************************************************************/
/**************** Functions *****************************************/
/********************************************************************/

static void SS_FreeInfo( HybridVoice_Info_t *info );
static int  SS_SynthesizeBuffer( SoftSynth *synth, void *samples,
                                 int samplesPerFrame, int bitsPerSample );
static int  SS_EstimateMaxAmplitude( SoftSynth *synth, int samplesPerFrame );
static void SS_NoteOn( SoftSynth *synth, int voiceIndex, int channelIndex,
                       int pitch, int velocity, int pan );
static void SS_NoteOff( SoftSynth *synth, int voiceIndex );
static void SS_TriggerDrum( SoftSynth *synth, int voiceIndex, int channelIndex,
                            int pitch, int velocity, int pan );
static void SS_SetAutoStopCallback( SoftSynth *synth,
                                    SS_AutoStopCallback *autoStopCallback,
                                    void *autoStopUserData );
static void SS_StealVoice( SoftSynth *synth, int voiceIndex );
static void SS_StifleVoice( SoftSynth *synth, int voiceIndex );
static void SS_SetChannelPitchBend( SoftSynth *synth,
                                    int channelIndex, FXP16 semitoneOffset16 );
static void SS_AllSoundOff( SoftSynth *synth, int channelIndex );
static void SS_SetChannelAftertouch( SoftSynth *synth, int channelIndex, int aftertouch );
static void SS_SetChannelExpression( SoftSynth *synth, int channelIndex, int expression );
static void SS_SetChannelModDepth( SoftSynth *synth, int channelIndex, int modDepth );
static void SS_SetChannelProgram( SoftSynth *synth, int channelIndex, int program );
static void SS_SetChannelBank( SoftSynth *synth, int channelIndex, int insBank );
static void SS_SetChannelTuning( SoftSynth *synth, int channelIndex, FXP16 octaveOffset16 );
static void SS_SetChannelVolume( SoftSynth *synth, int channelIndex, int volume );
static void SS_SetMasterVolume( SoftSynth *synth, FXP7 volume );
static void SS_SetGeneralMIDIMode( SoftSynth *synth, int isOn );
static int  SS_SetParameter( SoftSynth *synth, SPMIDI_Parameter parameterIndex, int value );
static int  SS_GetParameter( SoftSynth *synth, SPMIDI_Parameter parameterIndex, int *valuePtr );

/********************************************************************/
static void HybridVoice_Init( HybridVoice_t *voice, int index )
{
    voice->info = NULL;
    voice->index = (unsigned char) index;
}

void SS_AutoStopVoice( SoftSynth *synth, HybridVoice_t *voice )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;

    voice->active = FALSE;
    if( hybridSynth->autoStopCallback != NULL )
    {
        (*hybridSynth->autoStopCallback)( voice->index, hybridSynth->autoStopUserData );
    }
    SS_FreeInfo( voice->info );
    voice->info = NULL;
}

/********************************************************************/
#if SPMIDI_REPORT_SIZES
static void SS_ReportSizes( void )
{
    int sum = 0;

#define PRINT_SIZE_NUM(data,num) \
        PRTMSG("size of " #data " = "); PRTNUMD( sizeof(data) ); \
        PRTMSG(", * "); PRTNUMD( num ); \
        PRTMSG(" = "); PRTNUMD( sizeof(data)*(num) ); \
        PRTMSG("\n"); \
        sum += sizeof(data)*(num);

#define PRINT_SIZE(data) \
        PRTMSG("size of " #data " = "); PRTNUMD( sizeof(data) ); \
        PRTMSG("\n"); \
        sum += sizeof(data);

    PRTMSG("\n----------\n");

#if SPMIDI_ME2000

    PRINT_SIZE( WaveTable_t );
    PRINT_SIZE( WaveSet_t );
#endif

    PRINT_SIZE_NUM( HybridVoice_Preset_t, SS_GetSynthPresetCount() );

    PRINT_SIZE( HybridSynth_t );

    PRTMSG("Includes:\n");
    PRINT_SIZE_NUM( HybridVoice_Info_t, SS_NUM_INFOS );
    PRINT_SIZE_NUM( HybridVoice_t, SPMIDI_MAX_VOICES );
    PRINT_SIZE_NUM( HybridChannel_t, MIDI_NUM_CHANNELS );
    PRTMSG("----------\n\n");
}
#else
#define SS_ReportSizes()  /* noop */
#endif

/********************************************************************/
/**
 * Load an info structure with the runtime info for a given preset.
 */
static void SS_UpdateInfo( HybridSynth_t *hybridSynth, HybridOrchestra_t *orchestra, const HybridVoice_Preset_t *preset, HybridVoice_Info_t *info )
{
    info->preset = preset;

    ADSR_Load( &preset->modEnv, &info->modEnv, hybridSynth->controlRate );
    ADSR_Load( &preset->mainEnv, &info->mainEnv, hybridSynth->controlRate );
    ADSR_Load( &preset->ampEnv, &info->ampEnv, hybridSynth->controlRate );

#if SPMIDI_ME2000
    info->waveSet = NULL;
    {
        WaveSet_t *waveSet = NULL;
        if( preset->waveSetID != 0 )
        {
            waveSet = WaveManager_FindWaveSet( &orchestra->waveManager, preset->waveSetID );
            if( waveSet == NULL )
            {
                PRTMSGNUMD("ERROR in SS_UpdateInfo - waveSet not found, ID = ", preset->waveSetID );
                /* To prevent further errors, just use first WaveSet.
                 * This should not happen during production, only when editing.
                 */
                waveSet = WaveManager_GetFirstWaveSet( &orchestra->waveManager );
                if( waveSet == NULL )
                {
                    PRTMSG("ERROR in SS_UpdateInfo - WaveManager_GetFirstWaveSet failed.\n");
                }
            }
        }
        info->waveSet = waveSet;
    }
#endif /* SPMIDI_ME2000 */

    SVFilter_Load( &preset->filter, &info->filter );

    info->lfoCutoffModDepth = preset->lfoCutoffModDepth;
    info->envCutoffModDepth = preset->envCutoffModDepth;
    info->envPitchModDepth = preset->envPitchModDepth;
    info->flags = preset->flags;
}

/********************************************************************/
/**
 * Allocate an info structure for use by a voice.
 * Grab matching one or load an unused one.
 * Bump resource use count.
 * This allocation should never fail because there is an info structure for each voice.
 */
static HybridVoice_Info_t *SS_FindOrAllocateInfo( HybridSynth_t *hybridSynth, HybridOrchestra_t *orchestra, const HybridVoice_Preset_t *preset )
{
    int i;
    HybridVoice_Info_t *match = NULL;
    HybridVoice_Info_t *empty = NULL;
    HybridVoice_Info_t *info;

    /* Scan info structures for a match, or an unused structure. */
    for( i=0; i<SS_NUM_INFOS; i++ )
    {
        info = &hybridSynth->infoCache[i];
        if( info->preset == preset )
        {
            match = info;
            break;
        }
        /* Save empty info as second choice. */
        else if( (empty == NULL) && (info->preset == NULL) )
            empty = info;
    }

    /* If we didn't find a match then use an empty one. */
    if( match == NULL )
    {
        if( empty != NULL )
        {
            match = empty;
            empty->numUsers = 0;
            SS_UpdateInfo( hybridSynth, orchestra, preset, empty );
        }
    }

    /* Add another user. */
    if( match != NULL )
    {
        match->numUsers += 1;
    }

    return match;
}

/********************************************************************/
/**
 * Mark an info structure as being unused so that it can be reclaimed
 * and used by another voice.
 */
static void SS_FreeInfo( HybridVoice_Info_t *info )
{
    info->numUsers -= 1;
    if( info->numUsers <= 0 )
    {
#if SPMIDI_ME2000
        info->waveSet = NULL;
#endif
        info->preset = NULL;
        info->numUsers = 0;
    }
}

/********************************************************************
 * Set function to be called when a voice stops.
 * This allows the voice allocator to move it
 * to the free list.
 */
static void SS_SetAutoStopCallback( SoftSynth *synth,
                                    SS_AutoStopCallback *autoStopCallback,
                                    void *autoStopUserData )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    hybridSynth->autoStopCallback = autoStopCallback;
    hybridSynth->autoStopUserData = autoStopUserData;
}

/********************************************************************
 * Tell voice to shutup ASAP.
 */
static void SS_StifleVoice( SoftSynth *synth, int voiceIndex )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice = &hybridSynth->voices[voiceIndex];
    /* Use envelope that controls overall amplitude to shutdown voice. */
    voice->ampEnv.stage = ADSR_STAGE_STIFLE;
}

/********************************************************************
 * Add last values from stolen voice so they will get ramped down in the mix.
 * We only need to keep one value each output channel because the latched values
 * don't change, and they all ramp down at the same rate.
 */
static void SS_StealVoice( SoftSynth *synth, int voiceIndex )
{
#if SPMIDI_RAMP_STOLEN_VOICES
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice = &hybridSynth->voices[voiceIndex];

    hybridSynth->stolenVoiceLeft += voice->lastLeftValue;
    hybridSynth->stolenVoiceRight += voice->lastRightValue;
#else
    (void) synth;
    (void) voiceIndex;
#endif
}

#if SPMIDI_RAMP_STOLEN_VOICES
/********************************************************************
 * Add values from stolen voices as they are being ramped down.
 */
#define ATTENUATE_STOLEN_LEVEL(var) \
    var = var - ( var >> shifter );

static void SS_RampDownStolenVoices( HybridSynth_t *hybridSynth, FXP31 *samples, int samplesPerFrame )
{
    int is = SS_FRAMES_PER_BUFFER;
    FXP31 *samplePtr = samples;
    int shifter = hybridSynth->stolenVoiceShift;

    if( samplesPerFrame == 1 )
    {
        if( hybridSynth->stolenVoiceLeft != 0 )
        {
            while( is > 0 )
            {
                ATTENUATE_STOLEN_LEVEL( hybridSynth->stolenVoiceLeft );
                *samplePtr += hybridSynth->stolenVoiceLeft;
                samplePtr++;
                is--;
            }

            if( hybridSynth->stolenVoiceLeft < (1<<shifter) )
            {
                hybridSynth->stolenVoiceLeft = 0;
            }
        }
    }
    else
    {
        if( (hybridSynth->stolenVoiceLeft != 0) || (hybridSynth->stolenVoiceRight != 0) )
        {
            while( is > 0 )
            {
                ATTENUATE_STOLEN_LEVEL( hybridSynth->stolenVoiceLeft );
                ATTENUATE_STOLEN_LEVEL( hybridSynth->stolenVoiceRight );
                *samplePtr += hybridSynth->stolenVoiceLeft;
                samplePtr++;
                *samplePtr += hybridSynth->stolenVoiceRight;
                samplePtr++;
                is--;
            }
            
            /* Force level to zero if too close to get there by attenuation. */
            /* We don't have to do this for negative values because shifting
             * a negative number too far will result in -1 and the attenuator will go to zero.
             */
            if( (hybridSynth->stolenVoiceLeft > 0) &&
                (hybridSynth->stolenVoiceLeft < (1<<shifter)) )
            {
                hybridSynth->stolenVoiceLeft = 0;
            }
            if( (hybridSynth->stolenVoiceRight > 0) &&
                (hybridSynth->stolenVoiceRight < (1<<shifter)) )
            {
                hybridSynth->stolenVoiceRight = 0;
            }
        }
    }
}
#endif

/********************************************************************
 * Read PCM audio from the synthesis engine.
 */
static void SS_SynthesizeMixerBus( HybridSynth_t *hybridSynth, int samplesPerFrame )
{
    int iv, is;
    HybridVoice_t *voice;
    int numSamples = SS_FRAMES_PER_BUFFER * samplesPerFrame;

    /* Clear mixer bus. Unroll loop slightly to take advantage of burst writes. */
    for( is=0; is<numSamples; is += 4 )
    {
        hybridSynth->mixerBus[is+0] = 0;
        hybridSynth->mixerBus[is+1] = 0;
        hybridSynth->mixerBus[is+2] = 0;
        hybridSynth->mixerBus[is+3] = 0;
    }

    /* Synthesize and mix all active voices. */
    voice = &hybridSynth->voices[0];
    for( iv=0; iv<SPMIDI_MAX_VOICES; iv++ )
    {
        if( voice->active )
        {
#if SPMIDI_ME3000
            if( IS_VOICE_DLS(voice) )
            {
                SS_SynthesizeVoiceDLS2( hybridSynth, voice, samplesPerFrame );
            }
            else
#endif /* SPMIDI_ME3000 */

#if SPMIDI_ME2000
            if( voice->preset->mainOsc.waveform == WAVETABLE )
            {
                SS_SynthesizeVoiceME2000( hybridSynth, voice, samplesPerFrame );
            }
            else
#endif /* SPMIDI_ME2000 */

            {
                SS_SynthesizeVoiceME1000( hybridSynth, voice, samplesPerFrame );
            }
        }
        voice++;
    }

#if SPMIDI_RAMP_STOLEN_VOICES
    SS_RampDownStolenVoices( hybridSynth, hybridSynth->mixerBus, samplesPerFrame );
#endif

#if SPMIDI_USE_REVERB
    Reverb_Next( &hybridSynth->reverb, hybridSynth->mixerBus,
                 hybridSynth->reverbOutput,
                 SS_FRAMES_PER_BUFFER, (samplesPerFrame == 2) );
    {
        FXP31 rvrb = 0;
        int reverbIndex = 0;

        for( is=0; is<numSamples; is++ )
        {
            if( (samplesPerFrame == 1) || ((is&1) == 0) )
            {
                rvrb = hybridSynth->reverbOutput[reverbIndex++] >> 1; /* TODO - fine tune */
            }

            hybridSynth->mixerBus[is] += rvrb;
        }
    }
#endif

#if SPMIDI_USE_COMPRESSOR
    if( hybridSynth->isCompressorEnabled )
    {
        Compressor_CompressBuffer( &hybridSynth->compressor, hybridSynth->mixerBus,
                                   samplesPerFrame, hybridSynth->softSynth.masterVolume );
    }
    else
#endif

    {
        FXP7 gain = hybridSynth->softSynth.masterVolume;
        /* Scale and clip to mixer bus range. */
        for( is=0; is<numSamples; is++ )
        {
            FXP31 mix = (hybridSynth->mixerBus[is] >> 7) * gain;

#if SPMIDI_USE_SOFTCLIP

            mix = SS_MixerSoftClip( mix );
#else
            /* Clip to range. */
            if( mix > SS_MIXER_BUS_MAX )
            {
                mix = SS_MIXER_BUS_MAX;
            }
            else if( mix < SS_MIXER_BUS_MIN )
            {
                mix = SS_MIXER_BUS_MIN;
            }
#endif /* SPMIDI_USE_SOFTCLIP */
            hybridSynth->mixerBus[is] = mix;
        }
    }
}


/********************************************************************
 * Estimate a single voice amplitude.
 */
#if USE_ENVELOPE_IN_ESTIMATION
/* Set fudge factor based on comparison between full synthesis
 * and estimate for ringtone quite. */
#define ESTIMATOR_FUDGE_FACTOR  ((100*256) / 131)

static void SS_EstimateVoice( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame )
{
    int    ib;
    FXP31 *mixer;
    FXP31  leftGain, rightGain;
    FXP31  leftValue = 0, rightValue = 0;
    HybridChannel_t *channel;
    HybridVoice_Info_t *info = voice->info;
    int    maxAmplitudes[2] = { 0, 0 };
    int    doAutoStop;

    channel = &hybridSynth->channels[ voice->channel ];

    /* ADSR envelopes */
    ADSR_Next( &info->modEnv, &voice->modEnv, hybridSynth->modEnvOutput );
    ADSR_Next( &info->mainEnv, &voice->mainEnv, hybridSynth->mainEnvOutput );
    doAutoStop = ADSR_Next( &info->ampEnv, &voice->ampEnv, hybridSynth->ampEnvOutput );

    mixer = hybridSynth->mixerBus;

    if( samplesPerFrame == 1 )
    {
        leftGain = rightGain = voice->velocity * channel->volExpr << 7;
    }
    else
    {
        leftGain = voice->leftVelocityPan * channel->volExpr;
        rightGain = voice->rightVelocityPan * channel->volExpr;
    }

    /* Calculate audio rate units. ***********************/
    for( ib=0; ib<SS_BLOCKS_PER_BUFFER; ib++ )
    {

        FXP31 modAmplitude;
        FXP31 mixAmplitude;
        FXP31 envGain;

        /* Determine which envelope controls modOsc amplitude. */
        if( (voice->info->flags & HYBRID_FLAG_MODOSC_USE_MAINENV) != 0)
        {
            modAmplitude = FXP31_MULT(hybridSynth->mainEnvOutput[ib], info->phaseModDepth);
        }
        else
        {
            modAmplitude = FXP31_MULT(hybridSynth->modEnvOutput[ib], info->phaseModDepth);
        }

        /* Run oscillators with modEnv phase modulating mainEnv, or independantly. */
        if( (info->flags & HYBRID_FLAG_USE_PHASE_MOD) != 0 )
        {
            mixAmplitude = hybridSynth->mainEnvOutput[ib];
        }
        else
        {
            if( (info->flags & HYBRID_FLAG_USE_RING_MOD) != 0 )
            {
                mixAmplitude = FXP31_MULT( hybridSynth->mainEnvOutput[ib], modAmplitude ) >> 2;
            }
            else
            {
                mixAmplitude = (hybridSynth->mainEnvOutput[ib] >> 1) + (modAmplitude >> 1);
            }
        }

        /* Scale output and mix result with global mixer busses. */
        envGain = FXP31_MULT( hybridSynth->ampEnvOutput[ib], leftGain );

        leftValue = FXP31_MULT( mixAmplitude, envGain );
        leftValue = leftValue >> SS_MIXER_SHIFT;
        if( leftValue > maxAmplitudes[0] )
            maxAmplitudes[0] = leftValue;

        if( samplesPerFrame == 2 )
        {
            envGain = FXP31_MULT( hybridSynth->ampEnvOutput[ib], rightGain );
            rightValue = FXP31_MULT( mixAmplitude, envGain );
            rightValue = rightValue >> SS_MIXER_SHIFT;
            if( rightValue > maxAmplitudes[1] )
                maxAmplitudes[1] = rightValue;
        }

    }
    hybridSynth->mixAmplitude[0] += maxAmplitudes[0];
    hybridSynth->mixAmplitude[1] += maxAmplitudes[1];

    if( doAutoStop )
    {
        SS_AutoStopVoice( (SoftSynth *) hybridSynth, voice );
    }
    /* Return immediately after SS_AutoStopVoice() because voice info not valid. */
}

#else /* USE_ENVELOPE_IN_ESTIMATION */

/* Fast estimator that does not use envelopes. */

/* Set fudge factor based on comparison between full synthesis
* and estimate for ringtone. */

/* This value is based on the average estimate deviation. */
/* #define ESTIMATOR_FUDGE_FACTOR  ((100*256) / 171) */

/* This value is designed so that most ringtones will not clip
* at the expense of being a bit quiter. */
#define ESTIMATOR_FUDGE_FACTOR  ((100*256) / 100)

static void SS_EstimateVoice( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int samplesPerFrame )
{
    FXP31  leftGain, rightGain;
    HybridChannel_t *channel;

    channel = &hybridSynth->channels[ voice->channel ];

    if( voice->isOn )
    {
        if( samplesPerFrame == 1 )
        {
            leftGain = rightGain = voice->velocity14 * channel->volExpr;
        }
        else
        {
            leftGain = voice->leftVelocityPan * channel->volExpr;
            rightGain = voice->rightVelocityPan * channel->volExpr;
        }

        hybridSynth->mixAmplitude[0] += leftGain >> SS_MIXER_SHIFT;
        ;
        hybridSynth->mixAmplitude[1] += rightGain >> SS_MIXER_SHIFT;

    }
    else
    {
        SS_AutoStopVoice( (SoftSynth *) hybridSynth, voice );
    }
    /* Return immediately after SS_AutoStopVoice() because voice info not valid. */
}
#endif /* USE_ENVELOPE_IN_ESTIMATION */

/********************************************************************
 * Read PCM audio from the synthesis engine.
 */
static int SS_EstimateMaxAmplitude( SoftSynth *synth, int samplesPerFrame )
{
    int iv, is;
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice;
    int maxAmplitude = 0;
    int maxAmplitudeFudged = 0;

    if( samplesPerFrame > 2 )
        return SPMIDI_Error_IllegalSize;

    hybridSynth->mixAmplitude[0] = 0;
    hybridSynth->mixAmplitude[1] = 0;

    /* Mix estimated volume of all active voices. */
    voice = &hybridSynth->voices[0];
    for( iv=0; iv<SPMIDI_MAX_VOICES; iv++ )
    {
        if( voice->active )
        {
            SS_EstimateVoice( hybridSynth, voice, samplesPerFrame );
        }
        voice++;
    }

    {
        FXP7 gain;
        FXP31 amplifiedMix;
        FXP31 mixShifted;
        FXP31 maxMix = 0;
        /* Determine max of all channels. */
        for( is=0; is<samplesPerFrame; is++ )
        {
            FXP31 mix = hybridSynth->mixAmplitude[is];
            if( mix > maxMix )
                maxMix = mix;
        }
        /* Apply gain and possible compression. */
        gain = hybridSynth->softSynth.masterVolume;
#if SPMIDI_USE_COMPRESSOR

        if( hybridSynth->isCompressorEnabled )
        {
            amplifiedMix = Compressor_EstimateBuffer( &hybridSynth->compressor, maxMix, gain );
        }
        else
#endif

        {
            amplifiedMix = (maxMix >> 7) * gain;
        }

        mixShifted = amplifiedMix >> (SS_MIXER_BUS_RESOLUTION - 16);
        if( mixShifted > maxAmplitude )
            maxAmplitude = mixShifted;
    }

    /* Fudge estimate based on empirical comparison. */
    maxAmplitudeFudged = (maxAmplitude * ESTIMATOR_FUDGE_FACTOR) >> 8; /* divide by 256 */

    return maxAmplitudeFudged;
}

/********************************************************************
 * Read PCM audio from the synthesis engine.
 */
static int SS_SynthesizeBuffer( SoftSynth *synth, void *samples,
                                int samplesPerFrame, int bitsPerSample )
{
    int is;
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    int numSamples = SS_FRAMES_PER_BUFFER * samplesPerFrame;

    if( (samplesPerFrame < 1) || (samplesPerFrame > 2) )
        return SPMIDI_Error_OutOfRange;

    SS_SynthesizeMixerBus( hybridSynth, samplesPerFrame );

    /* Convert results to proper resolution and clip. */
    if( bitsPerSample < 8 )
    {
        return SPMIDI_Error_IllegalSize;
    }
    else if( bitsPerSample == 8 )
    {
        unsigned char *bytes = (unsigned char *) samples;

        for( is=0; is<numSamples; is++ )
        {
            int mix = hybridSynth->mixerBus[is] >> (SS_MIXER_BUS_RESOLUTION - 8);
            bytes[is] = (unsigned char ) (mix + 128);
        }
    }
    else if( bitsPerSample <= 16 )
    {
        short *shorts = (short *)samples;
        int    shifter = SS_MIXER_BUS_RESOLUTION - bitsPerSample;

        for( is=0; is<numSamples; is++ )
        {
            shorts[is] = (short) (hybridSynth->mixerBus[is] >> shifter);
        }
    }
    else if( bitsPerSample < SS_MIXER_BUS_RESOLUTION )
    {
        long  *longs = (long *)samples;
        int    shifter = SS_MIXER_BUS_RESOLUTION - bitsPerSample;

        for( is=0; is<numSamples; is++ )
        {
            longs[is] = hybridSynth->mixerBus[is] >> shifter;
        }
    }
    else if( bitsPerSample == SS_MIXER_BUS_RESOLUTION )
    {
        long  *longs = (long *)samples;

        for( is=0; is<numSamples; is++ )
        {
            longs[is] = hybridSynth->mixerBus[is];
        }
    }
    else if( bitsPerSample <= 32 )
    {
        long  *longs = (long *)samples;
        int    shifter = bitsPerSample - SS_MIXER_BUS_RESOLUTION;

        for( is=0; is<numSamples; is++ )
        {
            longs[is] = hybridSynth->mixerBus[is] << shifter;
            ;
        }
    }
    else
    {
        return SPMIDI_Error_IllegalSize;
    }

    return 0;
}

/***********************************************************************/
static const HybridVoice_Preset_t *SS_LookupMelodicPresetGM( HybridSynth_t *hybridSynth,
    int bankIndex, int program, HybridOrchestra_t **orchestraPtr )
{
    const HybridVoice_Preset_t *preset;

    /* When General MIDI is turned off, the normal instruments
     * are replaced with simple test instruments. */
    if(( hybridSynth->isGeneralMidiOff ) && ( bankIndex == SPMIDI_TEST_BANK ))
    {
        int presetIndex = program % SS_NUM_TEST_PRESETS;
        preset = &gHybridSynthTestPresets[ presetIndex ];
        *orchestraPtr = NULL;
    }
    else
    {
        preset = SS_GetSynthMelodicPreset( bankIndex, program, orchestraPtr );
    }

    return preset;
}

/***********************************************************************/
static const HybridVoice_Preset_t *SS_LookupDrumPresetGM( HybridSynth_t *hybridSynth,
    int bankIndex, int program, int pitch, HybridOrchestra_t **orchestraPtr )
{
    const HybridVoice_Preset_t *preset;

    /* When General MIDI is turned off, the normal instruments
     * are replaced with simple test instruments. */
    if(( hybridSynth->isGeneralMidiOff ) && ( bankIndex == SPMIDI_TEST_BANK ))
    {
        int presetIndex = program % SS_NUM_TEST_PRESETS;
        preset = &gHybridSynthTestPresets[ presetIndex ];
    }
    else
    {
        preset = SS_GetSynthDrumPreset( bankIndex, program, pitch, orchestraPtr );
    }

    return preset;
}

#if SPMIDI_ME3000
/***********************************************************************/
/**
 * Look for matching DLS Instrument program/bank and region.
 * Set voice->dlsInstrument and voice->dlsRegion if found. Otherwise set to NULL
 */
static const HybridVoice_Preset_t *SS_LookupPresetDLS( HybridSynth_t *hybridSynth,
        HybridVoice_t *voice, int bankIndex, int program, int pitch, int velocity )
{
    const HybridVoice_Preset_t *preset = NULL;

    if( voice != NULL )
    {
        voice->dlsInstrument = NULL;
        voice->dlsRegion = NULL;
    }

    /* Look for a match in the DLS Orchestra */
    if( hybridSynth->dlsOrchestra != NULL )
    {
        DLS_Instrument_t *dlsInstrument = SSDLS_FindInstrument(
                                              hybridSynth->dlsOrchestra, bankIndex, program );
        if( dlsInstrument != NULL )
        {
            DLS_Region_t *region = SSDLS_FindMatchingRegion( dlsInstrument, pitch, velocity );
            if( region == NULL )
            {
                /* If we do not match a region then we should play a silent instrument. */
                /* TODO Eventually we should abort the note in this case to conserve resources. */
                preset = &sSilentQuickPreset;
            }
            else
            {
                preset = &region->hybridPreset;
                if( voice != NULL )
                {
                    voice->dlsInstrument = dlsInstrument;
                    voice->dlsRegion = region;
                }
            }
        }
    }
    return preset;
}
#endif /* SPMIDI_ME3000 */

/********************************************************************/
/** Reduce phase modulation when carrier frequency is near Nyquist Rate.
 * This will reduce aliasing artifacts, particularly for 8000 Hz output.
 */
static void SS_AdjustPhaseMod( HybridVoice_t *voice )
{
    voice->phaseModDepth = voice->preset->phaseModDepth;

    if( (voice->info->flags & HYBRID_FLAG_USE_PHASE_MOD)
        /* For noise modulated sounds, it is OK to modulate near the Nyquist
         * because the partials are completely enharmonic. So aliasing sounds OK.
         * Note that this assumes an order to the WAVEFORM enum. DEPENDENCY001
         */
        && (voice->preset->modOsc.waveform < OSC_FIRST_NOISE_WAVEFORM )
      )
    {
        FXP31 mainPhaseInc = voice->mainOsc.phaseInc;
        /* Reduce phaseModDepth to zero at Nyquist/2 */
        if( mainPhaseInc > (OSC_PHASE_INC_NYQUIST >> 1) )
        {
            voice->phaseModDepth = 0;
        }
        else
        {
            FXP31 modScale = ((OSC_PHASE_INC_NYQUIST>>1) - mainPhaseInc) << 1;
            voice->phaseModDepth = FXP31_MULT( voice->preset->phaseModDepth, modScale );
        }
        // PRTMSGNUMH("phaseModDepth = ", voice->phaseModDepth );
    }
}

/********************************************************************/
/**
 * Modify pitch of a voice.
 */
static int SS_SetBasePitch( HybridSynth_t *hybridSynth, HybridVoice_t *voice, int pitch, int velocity )
{
    WaveSetRegion_t *region = NULL;
    HybridChannel_t *channel = &hybridSynth->channels[ voice->channel ];
    int keyCenter = voice->preset->keyCenter;
    int scaledPitch = ((pitch - keyCenter) * voice->preset->keyScalar) + (keyCenter << SPMIDI_PITCH_SCALAR_SHIFT);
    int normalizedPitch = scaledPitch << (SS_PITCH_SHIFT - SPMIDI_PITCH_SCALAR_SHIFT);
    int lfoPitchOffset = hybridSynth->srateOffset + (SS_FRAMES_PER_BLOCK_LOG2<<SS_PITCH_SHIFT);

    voice->baseNotePitch = SPMUtil_MIDIPitchToOctave( normalizedPitch ) +
                           channel->tuningOctaveOffset;

    /* Adjust for control rate */
    Osc_Start( &voice->preset->lfo, &voice->lfo, voice->baseNotePitch, lfoPitchOffset, NULL );

    /* Set up modOsc, alias vibratoLFO */
#if SPMIDI_ME3000
    if( IS_VOICE_DLS(voice) )
    {
        Osc_Start( &voice->preset->vibratoLFO, &voice->vibratoLFO, voice->baseNotePitch, lfoPitchOffset, NULL );
    }
    else
#endif
    {
        Osc_Start( &voice->preset->modOsc, &voice->modOsc, voice->baseNotePitch, hybridSynth->srateOffset, NULL );
    }

#if SPMIDI_ME2000
    if( voice->preset->mainOsc.waveform == WAVETABLE )
    {
#if SPMIDI_ME3000
        if( IS_VOICE_DLS(voice) )
        {
            region = &voice->dlsRegion->waveSetRegion;
        }
        else 
#endif /* SPMIDI_ME3000 */
        /* This 'if' phrase was moved outside the above #if statement on 6/26/06
         * It prevents a crash in the editor when switching synth targets.
         */
        if (voice->info->waveSet != NULL)
        {
            region = Osc_FindMatchingRegion( voice->info->waveSet,
                                    pitch,
                                    velocity );
            if( region == NULL )
            {
                goto abort_voice;
            }
            
            //PRTMSGNUMH("SS_SetBasePitch: pitch = ", pitch );
            //PRTMSGNUMH("SS_SetBasePitch: voice->baseNotePitch = ", voice->baseNotePitch );
            //PRTMSGNUMH("SS_SetBasePitch: waveSetRegion->basePitch = ", voice->waveSetRegion->basePitch );
        }
        voice->waveSetRegion = region;
    }
#else
    (void) velocity;
#endif /* SPMIDI_ME2000 */


    Osc_Start( &voice->preset->mainOsc, &voice->mainOsc, voice->baseNotePitch, hybridSynth->srateOffset, region );

    SS_AdjustPhaseMod( voice );
    return 0;

#if SPMIDI_ME2000
abort_voice:
    SS_AutoStopVoice( (SoftSynth *) hybridSynth, voice );
    return -1;
#endif
}

/********************************************************************/
/* Turn off a note so that it will decay and eventually stop. */
static void SS_NoteOff( SoftSynth *synth, int voiceIndex )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice = &hybridSynth->voices[voiceIndex];

    DBUGMSG("SS_NoteOff: voiceIndex = ");
    DBUGNUMD(voiceIndex);
    DBUGMSG("\n");

    voice->isOn = 0;

    if( !voice->isDrum )
    {
        if( (voice->info->flags & HYBRID_FLAG_IGNORE_NOTE_OFF) == 0 )
        {
            ADSR_Release( &voice->modEnv );
            ADSR_Release( &voice->ampEnv );
            ADSR_Release( &voice->mainEnv );
#if SPMIDI_ME2000

            if( voice->preset->mainOsc.waveform == WAVETABLE )
            {
                Osc_WaveTable_Release( &voice->mainOsc, voice->waveSetRegion->table );
            }
#endif /* SPMIDI_ME2000 */

        }
    }
}

/********************************************************************/
/* Attenuate bass notes so they don't get too boomy. */
static int SS_ApplyVelocityEQ( HybridSynth_t *hybridSynth, int pitch, int velocity )
{
    int newVelocity = velocity;

    if( pitch < hybridSynth->softSynth.veqBassCutoff )
    {
        int scaler = hybridSynth->softSynth.veqGainAtZero +
                     (((SPMIDI_VEQ_NOMINAL_GAIN - hybridSynth->softSynth.veqGainAtZero) * pitch) / (hybridSynth->softSynth.veqBassCutoff + 1));
        if( scaler < 0 )
            scaler = 0;
        newVelocity = (unsigned char) ((velocity * scaler) >> SPMIDI_VEQ_SHIFT);
    }
    return newVelocity;
}

/********************************************************************/
static void SS_StartVoice( HybridSynth_t *hybridSynth, HybridOrchestra_t *orchestra, HybridVoice_t *voice, const HybridVoice_Preset_t *preset,
                           int channelIndex, int pitch, int velocity14, int pan )
{
    voice->preset = preset;

    /* If we are starting a stolen voice then it will already have an info structure. */
    /* Do NOT optimize this by converting voice->info->preset to voice->preset !! */
    if( (voice->info != NULL) && (voice->info->preset != preset) )
    {
        SS_FreeInfo( voice->info );
        voice->info = NULL;
    }

    if( voice->info == NULL )
    {
        voice->info = SS_FindOrAllocateInfo( hybridSynth, orchestra, voice->preset );
    }

    voice->active = TRUE;
    voice->channel = (unsigned char) channelIndex;
    voice->pitch = (unsigned char) pitch;
    voice->isOn = 1;
    voice->previousMixerGain = 0;

    ADSR_Start( &voice->modEnv,
                &voice->preset->modEnv, pitch,
                hybridSynth->controlRate );

    ADSR_Start( &voice->mainEnv,
                &voice->preset->mainEnv, pitch,
                hybridSynth->controlRate );

    ADSR_Start( &voice->ampEnv,
                &voice->preset->ampEnv, pitch,
                hybridSynth->controlRate );

    voice->filter.info = &voice->info->filter;

    voice->velocity14 = (FXP14) velocity14;

    /* sPanTable is value for right channel. So just reverse index for left channel. */
    voice->leftVelocityPan = (sPanTable[ 127 - pan ] * velocity14) >> 14;
    voice->rightVelocityPan = (sPanTable[ pan ] * velocity14) >> 14;

#if SPMIDI_ME3000
    {
        DLS_Region_t *region = voice->dlsRegion;
        if( region != NULL )
        {
            voice->modLFOStartDelayCounter = region->modLFOStartDelay;
            voice->vibratoLFOStartDelayCounter = region->vibratoLFOStartDelay;
        }
    }
#endif
}

/***********************************************************************/
/* Transform 7 bit MIDI velocity to an FXP14 gain value. */
static FXP14 SS_ApplyVelocityTransform( int velocity )
{
#if SPMIDI_SQUARE_VELOCITY
/* Use squared velocity to match DLS spec.*/
    return velocity * velocity;
#else
    return velocity << 7;
#endif
}

/***********************************************************************/
/**
 * Turn on a note so that it contributes to audio mix.
 * @param pitch MIDI pitch from 0 to 127.
 */
static void SS_NoteOn( SoftSynth *synth, int voiceIndex, int channelIndex,
                       int noteIndex, int velocity, int pan )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice = &hybridSynth->voices[voiceIndex];
    HybridChannel_t *channel = &hybridSynth->channels[ channelIndex ];
    int pitch;
    const HybridVoice_Preset_t *preset;
    FXP14 velocity14;
    HybridOrchestra_t *orchestra;

    voice->isDrum = 0;

    pitch = noteIndex + hybridSynth->transposition; // Transpose by semitones.
    if( hybridSynth->transposition > 0 )
    {
        while( pitch > 127 ) pitch -= 12;
    }
    else if( hybridSynth->transposition < 0 )
    {
        while( pitch < 0 ) pitch += 12;
    }
    
#if SPMIDI_ME3000
    preset = SS_LookupPresetDLS( hybridSynth, voice,
                             channel->insBank, channel->program,
                             pitch, velocity );
    if( preset != NULL )
    {
        orchestra = voice->orchestra;
    }
    else
#endif  
    {
        preset = SS_LookupMelodicPresetGM( hybridSynth, channel->insBank, channel->program, &orchestra );
#if SPMIDI_ME3000
        voice->orchestra = orchestra;
#endif  
        /* For GM sound effects but not bird tweet, force pitch between 60 and 71.
         * Note that 0x78 = 120 and 0x7B = 123.
         */
        if( (channel->program >= 0x78) && (channel->program != 0x7B) )
        {
            pitch = 60 + (noteIndex % 12);
        }
    }
    
    
#if 0
        PRTNUMD( pitch ); PRTMSG( " = pitch in SS_NoteOn()\n" );
        PRTNUMH( synthPitch ); PRTMSG( " = synthPitch\n" );
        PRTNUMH( channelIndex ); PRTMSG( " = channelIndex\n" );
        PRTNUMH( voiceIndex ); PRTMSG( " = voiceIndex\n" );
        PRTNUMH( channel->program ); PRTMSG( " = program\n" );
        PRTNUMH( voice ); PRTMSG( " = voice\n" );
        PRTNUMH( velocity ); PRTMSG( " = velocity\n" );
#endif

    velocity = SS_ApplyVelocityEQ( hybridSynth, pitch, velocity );

#if 0
        PRTNUMH( velocity ); PRTMSG( " = velocity after EQ\n" );
#endif

    velocity14 = SS_ApplyVelocityTransform( velocity );

    SS_StartVoice( hybridSynth, orchestra, voice, preset, channelIndex, pitch, velocity14, pan );
    SS_SetBasePitch( hybridSynth, voice, pitch, velocity );
}

/***********************************************************************/
/**
 * Turn on a Rhythm Instrument so that it contributes to audio mix.
 */
static void SS_TriggerDrum( SoftSynth *synth, int voiceIndex, int channelIndex,
                            int pitch, int velocity, int pan )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice = &hybridSynth->voices[voiceIndex];
    int acousticPitch;
    const HybridVoice_Preset_t *preset;
    HybridChannel_t *channel = &hybridSynth->channels[ channelIndex ];
    int velocity14;
    HybridOrchestra_t *orchestra;

    voice->isDrum = 1;

#if SPMIDI_ME3000
    preset = SS_LookupPresetDLS( hybridSynth, voice,
                             channel->insBank, channel->program,
                             pitch, velocity );
    if( preset != NULL )
    {
        acousticPitch = pitch;
        orchestra = voice->orchestra;
    }
    else
#endif
    {
        preset = SS_LookupDrumPresetGM( hybridSynth, 
            channel->insBank, channel->program, pitch, &orchestra );
#if SPMIDI_ME3000
        voice->orchestra = orchestra;
#endif
        
        acousticPitch = SS_GetSynthDrumPitch( channel->insBank, channel->program, pitch );
    }

    velocity = SS_ApplyVelocityEQ( hybridSynth, acousticPitch, velocity );
    velocity14 = SS_ApplyVelocityTransform( velocity );
/*
    PRTMSGNUMD("SS_TriggerDrum -------------- pitch = ", pitch );
    PRTMSGNUMH("preset = ", preset );
    PRTMSGNUMD("main flags = ", preset->mainEnv.flags );
    PRTMSGNUMD("amp flags = ", preset->mainEnv.flags );
    PRTMSGNUMD("mainOsc waveform = ", preset->mainOsc.waveform );
    PRTMSGNUMD("waveset = ", preset->waveSetID );
*/
    /* Apply FXP7 RhythmVolume to FXP14 velocity14 after transform to gain. */
    velocity14 = (velocity14 * hybridSynth->softSynth.rhythmVolume) >> 7;

    SS_StartVoice( hybridSynth, orchestra, voice, preset, channelIndex, pitch, velocity14, pan );
    SS_SetBasePitch( hybridSynth, voice, acousticPitch, velocity );
}

/***********************************************************************/
static void SS_SetChannelTuning( SoftSynth *synth, int channelIndex, FXP16 octaveOffset16 )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridChannel_t *channel = &hybridSynth->channels[channelIndex];
    channel->tuningOctaveOffset = octaveOffset16;
}

/***********************************************************************/
static void SS_SetChannelPitchBend( SoftSynth *synth, int channelIndex, FXP16 semitoneOffset16 )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridChannel_t *channel = &hybridSynth->channels[channelIndex];
    channel->pitchBendOctaveOffset = SPMUtil_MIDIPitchToOctave( semitoneOffset16 ) -
                                     BASE_MIDI_PITCH;
}

/***********************************************************************
 */
static void SS_SetChannelAftertouch( SoftSynth *synth, int channelIndex, int aftertouch )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridChannel_t *channel = &hybridSynth->channels[channelIndex];
    channel->pressure = (unsigned char) aftertouch;
}

/***********************************************************************/
static void SS_SetChannelProgram( SoftSynth *synth, int channelIndex, int program )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    hybridSynth->channels[channelIndex].program = (unsigned char) program;
}

/***********************************************************************/
static void SS_SetChannelBank( SoftSynth *synth, int channelIndex, int insBank )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    hybridSynth->channels[channelIndex].insBank = (unsigned short) insBank;
}

/***********************************************************************/
static void SS_SetGeneralMIDIMode( SoftSynth *synth, int isOn )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    hybridSynth->isGeneralMidiOff = (unsigned char) !isOn;
}

/***********************************************************************/
static int SS_StopAllVoices( SoftSynth *synth )
{
    int result = 0;
    int i;
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice = &hybridSynth->voices[0];
    HybridVoice_Info_t *info;

    for( i=0; i<SPMIDI_MAX_VOICES; i++ )
    {
        if( voice->active )
        {
            SS_AutoStopVoice( synth, voice );
        }
        voice++;
    }
    
    /* Scan info structures to make sure that all are free. */
    for( i=0; i<SS_NUM_INFOS; i++ )
    {
        info = &hybridSynth->infoCache[i];
        if( (info->preset != NULL) || (info->numUsers > 0) )
        {
            PRTMSGNUMD("SS_StopAllVoices: ERROR - info not free! numUsers = \n", info->numUsers );
            result = -1; // TODO add proper error
        }
    }
    return result;
}

/***********************************************************************/
static void SS_AllSoundOff( SoftSynth *synth, int channelIndex )
{
    int iv;
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridVoice_t *voice = &hybridSynth->voices[0];

    for( iv=0; iv<SPMIDI_MAX_VOICES; iv++ )
    {
        if( voice->active && (voice->channel == channelIndex) )
        {
            SS_StifleVoice( (SoftSynth *) hybridSynth, voice->index );
        }
        voice++;
    }
}

/***********************************************************************/
static void SS_SetChannelVolume( SoftSynth *synth, int channelIndex, int volume )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridChannel_t *channel = &hybridSynth->channels[channelIndex];
    channel->volume = (unsigned char) volume;
    /* Square the fixed-point volume and expression parameters. */
    channel->volExpr = (unsigned short)
        ((volume * volume * channel->expression * channel->expression) >> 14);
}

/***********************************************************************/
static void SS_SetMasterVolume( SoftSynth *synth, FXP7 volume )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    hybridSynth->softSynth.masterVolume = volume;
}

/***********************************************************************
 * At max depth (0x7F) modulate by 50 cents which is 1/24 of an octave.
 * An octave depth is 1<<SS_PITCH_SHIFT.
 * @param modDepth modWheel controller 7 bit MSB
 */
static void SS_SetChannelModDepth( SoftSynth *synth, int channelIndex, int modDepth )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridChannel_t *channel = &hybridSynth->channels[channelIndex];
    channel->modDepth = (unsigned char) modDepth;
    /* Used by GM but not DLS */
    channel->lfoPitchModDepth = (modDepth << (SS_PITCH_SHIFT-7)) / 24; /* DIVIDE - mod depth control */
}

/***********************************************************************/
static void SS_SetChannelExpression( SoftSynth *synth, int channelIndex, int expression )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    HybridChannel_t *channel = &hybridSynth->channels[channelIndex];
    /* Square the fixed-point volume and expression parameters. */
    channel->expression = (unsigned char) expression;
    channel->volExpr = (unsigned short)
        ((expression * expression * channel->volume * channel->volume) >> 14);
}

/***********************************************************************/
static int SS_SetParameter( SoftSynth *synth, SPMIDI_Parameter parameterIndex, int value )
{
    int result = 0;
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    switch( parameterIndex )
    {
#if SPMIDI_USE_COMPRESSOR
    case SPMIDI_PARAM_COMPRESSOR_ON:
        hybridSynth->isCompressorEnabled = (unsigned char) (value != 0);
        break;

    case SPMIDI_PARAM_COMPRESSOR_CURVE:
    case SPMIDI_PARAM_COMPRESSOR_TARGET:
    case SPMIDI_PARAM_COMPRESSOR_THRESHOLD:
        result = Compressor_SetParameter( &hybridSynth->compressor, parameterIndex, value );
        break;
#else

        (void) hybridSynth;
        /* If not supported, allow turning it off. */
    case SPMIDI_PARAM_COMPRESSOR_ON:
        if (value != 0)
            result = SPMIDI_Error_OutOfRange;
        break;
#endif

    case SPMIDI_PARAM_VEQ_GAIN_AT_ZERO:
        hybridSynth->softSynth.veqGainAtZero = (short) value;
        break;

    case SPMIDI_PARAM_VEQ_BASS_CUTOFF:
        if( (value < 0 ) || (value > 127) )
        {
            result = SPMIDI_Error_OutOfRange;
        }
        else
        {
            hybridSynth->softSynth.veqBassCutoff = (unsigned char) value;
        }
        break;

    case SPMIDI_PARAM_RHYTHM_VOLUME:
        if( value > SPMIDI_DEFAULT_MASTER_VOLUME )
        {
            result = SPMIDI_Error_OutOfRange;
            goto error;
        }
        hybridSynth->softSynth.rhythmVolume = value;
        break;

    case SPMIDI_PARAM_TRANSPOSITION:
        if( value > 128 )
        {
            result = SPMIDI_Error_OutOfRange;
            goto error;
        }
        else if( value < -128 )
        {
            result = SPMIDI_Error_OutOfRange;
            goto error;
        }
        hybridSynth->transposition = (spmSInt8) value;
        break;

    default:
        result = SPMIDI_Error_UnrecognizedParameter;
        break;
    }
error:
    return result;
}

/***********************************************************************/
static int SS_GetParameter( SoftSynth *synth, SPMIDI_Parameter parameterIndex, int *valuePtr )
{
    int result = 0;
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;
    switch( parameterIndex )
    {
#if SPMIDI_USE_COMPRESSOR
    case SPMIDI_PARAM_COMPRESSOR_ON:
        *valuePtr = (int) hybridSynth->isCompressorEnabled;
        break;

    case SPMIDI_PARAM_COMPRESSOR_CURVE:
    case SPMIDI_PARAM_COMPRESSOR_TARGET:
    case SPMIDI_PARAM_COMPRESSOR_THRESHOLD:
        result = Compressor_GetParameter( &hybridSynth->compressor, parameterIndex, valuePtr );
        break;
#else

        (void) hybridSynth;
    case SPMIDI_PARAM_COMPRESSOR_ON:
        *valuePtr = 0;
        break;
#endif

    case SPMIDI_PARAM_VEQ_GAIN_AT_ZERO:
        *valuePtr = hybridSynth->softSynth.veqGainAtZero;
        break;

    case SPMIDI_PARAM_VEQ_BASS_CUTOFF:
        *valuePtr = hybridSynth->softSynth.veqBassCutoff;
        break;

    case SPMIDI_PARAM_RHYTHM_VOLUME:
        *valuePtr = hybridSynth->softSynth.rhythmVolume; // Fixed 10/7/07. Was setting result.
        break;

    case SPMIDI_PARAM_TRANSPOSITION:
        *valuePtr = hybridSynth->transposition;
        break;

    default:
        result = SPMIDI_Error_UnrecognizedParameter;
        break;
    }
    return result;
}

/********************************************************************/
/**************** External API **************************************/
/********************************************************************/

/********************************************************************/
/* Create a context for synthesis. */
int SS_Terminate()
{
    SS_Orchestra_Term();
    return SPMIDI_Error_None;
}

/********************************************************************/
/* Delete synthesis context. */
int SS_DeleteSynth( SoftSynth *synth )
{
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;

#if SPMIDI_USE_COMPRESSOR
    Compressor_Term( &hybridSynth->compressor );
#endif

    HybridSynth_Free( hybridSynth );
    return SPMIDI_Error_None;
}

/********************************************************************/
/* Create a context for synthesis. */
int SS_Initialize()
{
    /* Initialize data structures shared by all synths. */
    Osc_Init();

    SS_Orchestra_Init();

    
    SS_LoadPresetOrchestra();

    return SPMIDI_Error_None;
}

/********************************************************************/
/* Create a context for synthesis. */
int SS_CreateSynth( SoftSynth **synthPtr, int sampleRate )
{
    int i;
    SoftSynth *softSynth;
    int result;
    HybridSynth_t *hybridSynth;

    

    hybridSynth = HybridSynth_Allocate();

    if( hybridSynth == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }

    MemTools_Clear( hybridSynth, sizeof( HybridSynth_t ) );
    softSynth = (SoftSynth *) hybridSynth;

    SS_ReportSizes();

    result = SPMIDI_Error_UnsupportedRate; /* In case we don't match. */
    for( i=0; i<NUM_SUPPORTED_RATES; i++ )
    {
        if( sSupportedRates[i].rate == sampleRate )
        {
            hybridSynth->srateOffset = sSupportedRates[i].pitchOffset;
            result = 0;
            break;
        }
    }
    if( result != 0 )
    {
        HybridSynth_Free( hybridSynth );
        hybridSynth->srateOffset = 0;
        return result;
    }
    hybridSynth->sampleRate = sampleRate;
    hybridSynth->controlRate = sampleRate / SS_FRAMES_PER_CONTROL; /* DIVIDE - init */

#if SPMIDI_USE_COMPRESSOR

    result = Compressor_Init( &hybridSynth->compressor, sampleRate );
    if( result < 0 )
    {
        HybridSynth_Free( hybridSynth );
        return result;
    }
    hybridSynth->isCompressorEnabled = 1;
#endif

    /* Set default values for velocity EQ. */
    hybridSynth->softSynth.veqBassCutoff = SPMIDI_VEQ_BASS_CUTOFF;
    hybridSynth->softSynth.veqGainAtZero = SPMIDI_VEQ_BASS_GAIN;

    /* Set function pointers for synth "object". */
    softSynth->AllSoundOff = SS_AllSoundOff;
    softSynth->NoteOff = SS_NoteOff;
    softSynth->NoteOn = SS_NoteOn;
    softSynth->SetAutoStopCallback  = SS_SetAutoStopCallback;
    softSynth->SetChannelAftertouch = SS_SetChannelAftertouch;
    softSynth->SetChannelExpression = SS_SetChannelExpression;
    softSynth->SetChannelModDepth   = SS_SetChannelModDepth;
    softSynth->SetChannelPitchBend  = SS_SetChannelPitchBend;
    softSynth->SetChannelProgram    = SS_SetChannelProgram;
    softSynth->SetChannelBank       = SS_SetChannelBank;
    softSynth->SetChannelTuning     = SS_SetChannelTuning;
    softSynth->SetChannelVolume     = SS_SetChannelVolume;
    softSynth->SetGeneralMIDIMode   = SS_SetGeneralMIDIMode;
    softSynth->SetMasterVolume      = SS_SetMasterVolume;
    softSynth->StealVoice           = SS_StealVoice;
    softSynth->StifleVoice          = SS_StifleVoice;
    softSynth->SynthesizeBuffer     = SS_SynthesizeBuffer;
    softSynth->EstimateMaxAmplitude = SS_EstimateMaxAmplitude;
    softSynth->TriggerDrum          = SS_TriggerDrum;
    softSynth->SetParameter         = SS_SetParameter;
    softSynth->GetParameter         = SS_GetParameter;
    softSynth->StopAllVoices        = SS_StopAllVoices;

    hybridSynth->softSynth.masterVolume = SPMIDI_DEFAULT_MASTER_VOLUME;
    hybridSynth->softSynth.rhythmVolume = SPMIDI_DEFAULT_MASTER_VOLUME;

    /* Calculate shift value for attenuating stolen voice levels. */
    /* Use a larger shifter, and a slower attenuation for higher sample rates. */
    {
        int rateShift = sampleRate;
        hybridSynth->stolenVoiceShift = 2;
        while( rateShift >= 16000 )
        {
            hybridSynth->stolenVoiceShift += 1;
            rateShift = rateShift >> 1;
        }
    }


    for( i=0; i<SS_NUM_INFOS; i++ )
    {
        HybridVoice_Info_t *info = &hybridSynth->infoCache[ i ];
        info->preset = NULL;
        info->numUsers = 0;
    }

    for( i=0; i<SPMIDI_MAX_VOICES; i++ )
    {
        HybridVoice_Init( &hybridSynth->voices[i], i );

    }

    *synthPtr = softSynth;

    return 0;

}

#if SPMIDI_SUPPORT_LOADING

/***********************************************************************/
int SS_SetInstrumentDefinition( HybridOrchestra_t *orchestra, int insIndex, ResourceTokenMap_t *tokenMap, unsigned char *data, int numBytes )
{
    HybridVoice_Preset_t *preset;
    unsigned char *p = data;
    int numParsed;

    DBUGMSG( "-----------------------\n" );
    DBUGNUMH( insIndex );
    DBUGMSG( " = insIndex in SS_SetInstrumentDefinition()\n" );

    if( (insIndex >= SS_MAX_PRESETS) || (insIndex < 0) )
    {
        return -2;
    }
    preset = (HybridVoice_Preset_t *) SS_GetSynthPreset( orchestra, insIndex );
    // Clear preset in case we do not set all fields.
    MemTools_Clear( preset, sizeof( HybridVoice_Preset_t ) );
    
    //PRTMSGNUMH("SS_SetInstrumentDefinition: insIndex = ", insIndex )
    //PRTMSGNUMH("SS_SetInstrumentDefinition: preset = ", preset )

    /* Parse stream header */
    if( *p++ != SPMIDI_BEGIN_STREAM )
    {
        return SPMIDI_Error_BadFormat;
    }
    if( *p++ != SPMIDI_INSTRUMENT_STREAM_ID )
    {
        return SPMIDI_Error_BadFormat;
    }

    p = Osc_Define( &preset->modOsc, p );
    p = Osc_Define( &preset->mainOsc, p );

#if SPMIDI_ME2000
    if( preset->mainOsc.waveform == WAVETABLE )
    {
        // Save waveSetID for later.
        p = SS_ParseLong( &preset->waveSetID, p );
        // PRTMSGNUMD("SS_SetInstrumentDefinition: preset->waveSetID = ", preset->waveSetID);
        if( tokenMap != NULL )
        {
            preset->waveSetID = tokenMap[ preset->waveSetID ].token;
            // PRTMSGNUMD("SS_SetInstrumentDefinition: waveSetID mapped to = ", preset->waveSetID);
        }
        
        // Check now to make sure we are referencing a valid waveset.
        if( preset->waveSetID != 0 )
        {
            WaveSet_t *waveSet = WaveManager_FindWaveSet( &orchestra->waveManager, preset->waveSetID );
            if( waveSet == NULL )
            {
                PRTMSG("ERROR in SS_SetInstrumentDefinition - waveSet not found.");
                return SPMIDI_Error_IllegalArgument;
            }
        }
    }
#endif

    p = Osc_Define( &preset->lfo, p );
    p = ADSR_Define( &preset->modEnv, p );
    p = ADSR_Define( &preset->mainEnv, p );
    p = ADSR_Define( &preset->ampEnv, p );
    p = SVFilter_Define( &preset->filter, p );
    p = SS_ParseLong( &preset->phaseModDepth, p );
    p = SS_ParseLong( &preset->lfoPitchModDepth, p );
    p = SS_ParseLong( &preset->lfoCutoffModDepth, p );
    p = SS_ParseLong( &preset->envPitchModDepth, p );
    p = SS_ParseLong( &preset->envCutoffModDepth, p );
    preset->flags = *p++;
    preset->boostLog2 = (signed char) *p++;
    preset->keyCenter = (unsigned char) *p++;
    preset->keyScalar = (unsigned char) *p++;

    if( *p++ != SPMIDI_END_STREAM )
    {
        PRTMSG( "SS_SetInstrumentDefinition did not see SPMIDI_END_STREAM\n" );
        return SPMIDI_Error_BadFormat;
    }

    numParsed = p - data;
    if( numParsed != numBytes )
    {
        PRTMSG("ERROR in SS_SetInstrumentDefinition - mismatch in data parsed and data sent.\n");
        PRTMSGNUMD("ERROR in SS_SetInstrumentDefinition: numParsed = ", numParsed);
        PRTMSGNUMD("ERROR in SS_SetInstrumentDefinition: numBytes = ", numBytes);
        return SPMIDI_Error_BadFormat;
    }

    /* Check for deadly use of phase modulated waveform without PM flag. */
    if( ((preset->flags & HYBRID_FLAG_USE_PHASE_MOD) == 0) && /* no PM */
            ((preset->mainOsc.waveform == SINE_PM) ||
             (preset->mainOsc.waveform == TRIANGLE_PM) ||
             (preset->mainOsc.waveform == SAWTOOTH_PM) ||
             (preset->mainOsc.waveform == SQUARE_PM)) )
    {
        PRTMSG("ERROR in SS_SetInstrumentDefinition - use of PM waveform without modulator.\n");
        PRTMSGNUMD("SS_SetInstrumentDefinition: preset->mainOsc.waveform = ", preset->mainOsc.waveform );
        PRTMSGNUMD("SS_SetInstrumentDefinition: preset->flags = ", preset->flags );
        PRTMSGNUMD("SS_SetInstrumentDefinition: HYBRID_FLAG_USE_PHASE_MOD = ", HYBRID_FLAG_USE_PHASE_MOD );
        return -1;
    }

    DBUGNUMH( preset->filter.cutoffPitch );
    DBUGMSG( " = filter.cutoffPitch in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->filter.inverseQ );
    DBUGMSG( " = filter.inverseQ in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->filter.flags );
    DBUGMSG( " = filter.flags in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->flags );
    DBUGMSG( " = flags in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->phaseModDepth );
    DBUGMSG( " = phaseModDepth in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->lfoPitchModDepth );
    DBUGMSG( " = lfoPitchModDepth in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->lfoCutoffModDepth );
    DBUGMSG( " = lfoCutoffModDepth in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->envCutoffModDepth );
    DBUGMSG( " = envCutoffModDepth in SS_SetInstrumentDefinition()\n" );
    DBUGNUMH( preset->boostLog2 );
    DBUGMSG( " = boostLog2 in SS_SetInstrumentDefinition()\n" );

    //PRTMSGNUMD("SS_SetInstrumentDefinition: preset->mainOsc.waveform = ", preset->mainOsc.waveform );

    return 0;
}

#endif /* SPMIDI_SUPPORT_LOADING */

