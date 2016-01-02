/* $Id$ */
/**
 *
 * Manager for Externally Loaded Instruments.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/engine/dbl_list.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/engine/instrument_mgr.h"


/* Use to create default instrument with reasonable values. */
#define DEFAULT_RISEFALL_TIME   (1)
static const HybridVoice_Preset_t sInstrumentTemplate =
    { /* Pure Sawtooth */
        { SINE, 0, 0x0 },
        { SAWTOOTH, 0, 0x0 },
        { SINE, 0, 0x0 },
        { DEFAULT_RISEFALL_TIME, 1, 1023, DEFAULT_RISEFALL_TIME, 0, 0 | 0 | 0 },
        { DEFAULT_RISEFALL_TIME, 1, 1023, DEFAULT_RISEFALL_TIME, 0, 0 | 0 | 0 },
        { DEFAULT_RISEFALL_TIME, 1, 1023, DEFAULT_RISEFALL_TIME, 0, 0 | 0 | 0 },
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


/* Initialize linked lists and prepare for storing instruments. */
void InsManager_InitializePreset( HybridVoice_Preset_t *preset )
{
    MemTools_Copy( preset, &sInstrumentTemplate, sizeof( sInstrumentTemplate ) );
}
