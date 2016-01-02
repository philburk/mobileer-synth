/* $Id: spmidi_dls.c,v 1.25 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * SPMIDI suport for DLS.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_dls.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_hybrid.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#define DBUGMSGNUMH( msg, num ) { DBUGMSG( msg ); DBUGNUMH( num ); DBUGMSG("\n"); }

#define MSEC_PER_SECOND   (1000)

/* Only compile if supporting ME3000 API */
#if SPMIDI_ME3000

/* NULL means use internal function instead of custom function. */
SSDLS_LoadOrchestraProcPtr sLoadOrchestraProc = NULL;

void SSDLS_ApplyArticulationToRegion( DLS_ArticulationTokens token,
                                            spmSInt32 scale, DLS_Region_t *region );

/*******************************************************************************/
/**
 * Iterate through array and apply each articulation to the region.
 */
static void SSDLS_ApplyArticulationArray( DLS_ArticulationTracker_t *articulations,
                                      DLS_Region_t *region )
{
    int i;
    DLS_Articulation_t *articulation;

    for( i=0; i<articulations->numArticulations; i++ )
    {
        articulation = &articulations->articulations[ i ];

        SSDLS_ApplyArticulationToRegion( articulation->token, articulation->scale, region );
    }
}

#define DLS2_ABSPITCH_5HZ     (-55791973)
#define DLS2_RELPITCH_OCTAVE  (78643200)
#define DLS2_ABSTIME_10MSEC   (-522494111)
#define DLS2_ABSTIME_ZERO     (0x80000000)

/* Declare these as const to avoid RW data.
 * These contain linked list nodes but we won't be using them.
 */
const DLS_Articulation_t sDefaultArticulations[] =
{
    { CONN_Z_LFO_FREQUENCY, DLS2_ABSPITCH_5HZ },
    { CONN_Z_VIB_FREQUENCY, DLS2_ABSPITCH_5HZ },
    { CONN_Z_LFO_STARTDELAY, DLS2_ABSTIME_10MSEC },
    { CONN_Z_VIB_STARTDELAY, DLS2_ABSTIME_10MSEC },
    /* The other envelope times are set by the default preset in the instrument manager. */
    { CONN_Z_EG1_DELAYTIME, DLS2_ABSTIME_ZERO },
    { CONN_Z_EG1_HOLDTIME, DLS2_ABSTIME_ZERO },
    { CONN_Z_EG2_DELAYTIME, DLS2_ABSTIME_ZERO },
    { CONN_Z_EG2_HOLDTIME, DLS2_ABSTIME_ZERO }
};

#define NUM_DEFAULT_ARTICULATIONS (sizeof(sDefaultArticulations) / sizeof( DLS_Articulation_t ) )

/*******************************************************************************/
/**
 */
static void SSDLS_ApplyDefaultArticulations( DLS_Region_t *region, int sampleRate )
{
    int i;

    for( i=0; i<NUM_DEFAULT_ARTICULATIONS; i++ )
    {
        const DLS_Articulation_t *articulation = &sDefaultArticulations[i];
        spmSInt32 converted = DLSParser_ConvertArticulationData( articulation, sampleRate );
        SSDLS_ApplyArticulationToRegion( articulation->token, converted, region );
    }
}

/*******************************************************************************/
void SSDLS_ApplyArticulationToRegion( DLS_ArticulationTokens token,
                                            spmSInt32 scale, DLS_Region_t *region )
{
    HybridVoice_Preset_t *preset = &region->hybridPreset;

    switch( token )
    {

    case CONN_Z_TUNING:
        preset->mainOsc.pitchControl = ( scale );
        break;

    case CONN_Z_VIB_LFO_TO_PITCH:
        preset->vibratoPitchModDepth = ( scale );
        break;

    case CONN_Z_MOD_LFO_TO_PITCH:
        preset->lfoPitchModDepth = ( scale );
        break;

    case CONN_Z_EG2_TO_PITCH:
        preset->envPitchModDepth = ( scale );
        break;

    case CONN_Z_LFO_FREQUENCY:
        preset->lfo.pitchControl = ( scale );
        preset->lfo.flags = OSC_FLAG_ABSOLUTE_PITCH;
        break;

    case CONN_Z_VIB_FREQUENCY:
        preset->vibratoLFO.pitchControl = ( scale );
        preset->vibratoLFO.flags = OSC_FLAG_ABSOLUTE_PITCH;
        break;

    case CONN_Z_VIB_STARTDELAY:
        region->vibratoLFOStartDelay = (short)scale;
        break;

    case CONN_Z_LFO_STARTDELAY:
        region->modLFOStartDelay = (short) scale;
        break;

        /* EG1 Destinations */
    case CONN_Z_EG1_DELAYTIME:
        region->eg1Extension.delayIncrement = scale;
        break;
    case CONN_Z_EG1_ATTACKTIME:
        preset->ampEnv.attackTime = (short) scale;
        break;
    case CONN_Z_EG1_HOLDTIME:
        region->eg1Extension.holdIncrement = scale;
        break;
    case CONN_Z_EG1_DECAYTIME:
        preset->ampEnv.decayTime = (short) scale;
        break;
    case CONN_Z_EG1_RELEASETIME:
        preset->ampEnv.releaseTime = (short) scale;
        break;
    case CONN_Z_EG1_SUSTAINLEVEL:
        preset->ampEnv.sustainLevel = (spmSInt16) scale;
        break;

        /* EG2 Destinations */
    case CONN_Z_EG2_DELAYTIME:
        region->eg2Extension.delayIncrement = scale;
        break;
    case CONN_Z_EG2_ATTACKTIME:
        preset->modEnv.attackTime = (short) scale;
        break;
    case CONN_Z_EG2_HOLDTIME:
        region->eg2Extension.holdIncrement = scale;
        break;
    case CONN_Z_EG2_DECAYTIME:
        preset->modEnv.decayTime = (short) scale;
        break;
    case CONN_Z_EG2_RELEASETIME:
        preset->modEnv.releaseTime = (short) scale;
        break;
    case CONN_Z_EG2_SUSTAINLEVEL:
        preset->modEnv.sustainLevel = (spmSInt16) scale;
        break;

    case CONN_Z_KEY_TO_PITCH:
        preset->keyScalar = (spmUInt8) scale;
        break;

    case CONN_Z_MOD_LFO_CC1_TO_PITCH:
        region->modLFOCC1toPitch = ( scale );
        break;

    case CONN_Z_VIB_LFO_CC1_TO_PITCH:
        region->vibLFOCC1toPitch = ( scale );
        break;

    case CONN_Z_MOD_LFO_CPR_TO_PITCH:
        region->modLFOCPRtoPitch = ( scale );
        break;

    case CONN_Z_VIB_LFO_CPR_TO_PITCH:
        region->vibLFOCPRtoPitch = ( scale );
        break;

    default:
        break;
    }
}

/*******************************************************************************/
/*******************************************************************************/
static int SSDLS_LoadOrchestra_Internal( SoftSynth *synth, DLS_Orchestra_t *dlsOrch )
{
    int insIndex;
    HybridSynth_t *hybridSynth = (HybridSynth_t *) synth;

    if( hybridSynth->dlsOrchestra != NULL )
    {
        return SPMIDI_Error_DLSAlreadyLoaded;
    }
    hybridSynth->dlsOrchestra = dlsOrch;

    /* Initialize preset structures */
    for( insIndex=0; insIndex<dlsOrch->numInstruments; insIndex++ )
    {
        int regionIndex;

        DLS_Instrument_t *dlsInstrument = &(dlsOrch->instruments[insIndex]);

        /* Initialize preset structures */
        for( regionIndex=0; regionIndex<dlsInstrument->numRegions; regionIndex++ )
        {
            DLS_Region_t *region = &dlsInstrument->regions[regionIndex];

            HybridVoice_Preset_t *preset = &(region->hybridPreset);

            InsManager_InitializePreset( preset );

            /* Set preset based on DLS instrument. */
            preset->mainOsc.waveform = WAVETABLE;

            DBUGMSG("Apply Default Articulations.\n");
            SSDLS_ApplyDefaultArticulations( region, hybridSynth->sampleRate );

            /* Apply Instrument Articulations. */
            DBUGMSG("Apply Instrument Articulations.\n");
            SSDLS_ApplyArticulationArray( &dlsInstrument->articulations,
                                      region );

            /* Override Instrument Articulations with Region Articulations. */
            DBUGMSG("Apply Region Articulations.\n");
            SSDLS_ApplyArticulationArray( &region->articulations,
                                      region );
        }
    }

    return 0;
}


/*******************************************************************************/
SSDLS_LoadOrchestraProcPtr SSDLS_GetProc_LoadOrchestra( void )
{
    if( sLoadOrchestraProc != NULL ) return sLoadOrchestraProc;
    else return &SSDLS_LoadOrchestra_Internal;
}

/*******************************************************************************/
void SSDLS_SetProc_LoadOrchestra( SSDLS_LoadOrchestraProcPtr proc )
{
    sLoadOrchestraProc = proc;
}

/*******************************************************************************/
int SSDLS_LoadOrchestra( SoftSynth *synth, DLS_Orchestra_t *dlsOrch )
{
    return SSDLS_GetProc_LoadOrchestra()( synth, dlsOrch );
}

/*******************************************************************************/
/** Find matching DLS instrument with same bank and program.
 */
DLS_Instrument_t *SSDLS_FindInstrument( DLS_Orchestra_t *dlsOrch, int bankIndex, int programIndex )
{
    int i;
    for( i=0; i<dlsOrch->numInstruments; i++ )
    {
        DLS_Instrument_t *dlsInstrument = &(dlsOrch->instruments[i]);
        if( (dlsInstrument->bankID == (spmUInt32) bankIndex) &&
                (dlsInstrument->programID == programIndex) )
        {
            return dlsInstrument;
        }
    }
    return NULL;
}

/*******************************************************************************/
/**
 * Find first region in instrument that matches the pitch and velocity.
 */
DLS_Region_t *SSDLS_FindMatchingRegion( DLS_Instrument_t *dlsIns, int pitch, int velocity )
{
    DLS_Region_t *region = NULL;
    DLS_Region_t *match = NULL;
    int i;
    (void) velocity;

    for( i=0; i<dlsIns->numRegions; i++ )
    {
        region = &(dlsIns->regions[i]);
        if( ((pitch >= region->waveSetRegion.lowPitch) && (pitch <= region->waveSetRegion.highPitch)) &&
            ((velocity >= region->waveSetRegion.lowVelocity) && (velocity <= region->waveSetRegion.highVelocity))
          )
        {
            match = region;
            break;
        }
    }
    return match;
}

#endif /* #if SPMIDI_ME3000 */
