#ifndef _SPMIDI_ORCHESTRA_H
#define _SPMIDI_ORCHESTRA_H

/* $Id: spmidi_orchestra.h,v 1.9 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 *
 * Support for sets of instruments.
 *
 * @author Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include "spmidi/include/spmidi_config.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_preset.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Determine whether instrument is read-only. */
#if SPMIDI_SUPPORT_LOADING
#define EDITABLE /* */
#define SPMIDI_MAX_PROGRAM_MAP_BANKS (256)
#else
#define EDITABLE const
#endif

typedef struct HybridOrchestra_s
{
    DoubleNode     node;
    struct HybridVoice_Preset_s *hybridSynthPresets;
    DoubleList     melodicBankList;
    DoubleList     drumProgramList;
#if SPMIDI_ME2000
    WaveManager_t  waveManager;
#endif /* SPMIDI_ME2000 */
} HybridOrchestra_t;

/** @return Orchestra compiled from source code. */
HybridOrchestra_t *SS_GetCompiledOrchestra( void );

/** @return Indexed preset structure from instrument library. */
EDITABLE struct HybridVoice_Preset_s *SS_GetSynthPreset( HybridOrchestra_t *orchestra, int presetIndex );

/** @return number of valid preset structures in instrument library */
int SS_GetSynthPresetCount( void );

/** @return preset corresponding to drum bank, program and noteIndex. */
EDITABLE struct HybridVoice_Preset_s *SS_GetSynthDrumPreset( int bank, int program, int noteIndex, HybridOrchestra_t **orchestraPtr );

/** @return preset corresponding to melodic bank and program. */
EDITABLE struct HybridVoice_Preset_s *SS_GetSynthMelodicPreset( int bank, int program, HybridOrchestra_t **orchestraPtr );

/** Map a MIDI program number to an instrument index.
 * This allows multiple programs to be mapped to a single instrument.
 */
int SS_SetInstrumentMap( HybridOrchestra_t *orchestra, int bankIndex, int programIndex, int insIndex );

/** Map a MIDI drum pitch to an instrument index.
 * This allows multiple drums to be mapped to a single instrument.
 */
int SS_SetDrumMap( HybridOrchestra_t * orchestra, int bankIndex, int programIndex, int noteIndex, int insIndex, int pitch );

/**
 * MIDI drums are indexed by noteIndex or "pitch".
 * But their actual played pitch is independant of the pitch used to 
 * select the drum.
 * This maps MIDI note to the acoustic pitch that will be heard.
 * This allows, for example, one Tom preset to be used for several
 * drums by playing it at different tuned pitches.
 * @return playback pitch.
 */
int SS_GetSynthDrumPitch( int bank, int program, int noteIndex );

int SS_SetSynthDrumPitch( int bank, int program, int noteIndex, int pitch );

#if SPMIDI_ME2000
/** Defined by Editor to load waves at runtime. */
void WaveManager_LoadWaves( WaveManager_t *waveManager );
#endif

int SS_Orchestra_Init( void );
void SS_Orchestra_Term( void );

/** This function is defined in the Preset file exported by the
 * Mobileer Instrument Editor. */
void SS_LoadPresetOrchestra( void );

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_ORCHESTRA_H */
