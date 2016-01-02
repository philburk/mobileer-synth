/* $Id: spmidi_orchestra.c,v 1.10 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 *
 * The Synthesizer can play instruments from an ordered list of Orchestras.
 * Each Orchestra contains an array of up to 255 Instrument Presets
 * and a list of Maps for mapping Bank/Program/Pitch information to those Presets.
 * It also contains the WaveTables and WaveSets used by those Instruments.
 *
 * Objects in one Orchestra cannot reference objects in another Orchestra.
 *
 * Copyright 2002-2007 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#if defined(WIN32) || defined(MACOSX)
#include <math.h>
#endif

#include "spmidi/engine/dbl_list.h"
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
//#include "spmidi/engine/compressor.h"
//#include "spmidi/engine/adsr_envelope.h"
//#include "spmidi/engine/oscillator.h"
#include "spmidi/engine/wave_manager.h"

#include "spmidi/engine/spmidi_orchestra.h"

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/* Define these here because they are needed by the following
 * include of spmidi_presets_*.h */
/* These are specific to a melodic bank or drum program. */
typedef unsigned char ProgramMapIndex;

#define SS_PROGRAM_MAP_ALLOCATED   (1<<0)

/** Use same structure for both MelodicMaps and DrumPrograms
 * Note that this structure is initialized from the Mobileer Editor so do NOT change its structure.
 * Adding to it is OK.
 */
typedef struct ProgramBankMap_s
{
    DoubleNode node;
    short bankIndex;
    unsigned char programIndex;
    unsigned char flags;
    ProgramMapIndex *presetMap;
    ProgramMapIndex *pitches;
} ProgramBankMap_t;


static DoubleList sOrchestraList;

/* These are used for the orchestra compiled from the source put out by the editor. */
static HybridOrchestra_t sCompiledOrchestra;

void SS_AddMelodicBankToList( ProgramBankMap_t *melodicBank );
void SS_AddDrumProgramToList( ProgramBankMap_t *drumProgram );

#define MELODIC_STUB_INDEX  (0xFF)
#define DRUM_STUB_INDEX     (0xFE)

/*********************************************************************/
/******* Load Preset Orchestra ***************************************/
/*********************************************************************/
/* Include Preset data instead of linking. */
#if SPMIDI_ME2000
    #if SPMIDI_SUPPORT_EDITING
        #define SPMIDI_PRESET_FILENAME_H "spmidi_presets_editor_me2000.h"

    #elif defined(SPMIDI_PRESETS_CUSTOM)
        #define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom.h"

    #elif defined(SPMIDI_PRESETS_CUSTOM_1)
        #define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_1.h"

    #elif defined(SPMIDI_PRESETS_CUSTOM_2)
        #define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_2.h"

    #elif defined(SPMIDI_PRESETS_CUSTOM_3)
        #define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_3.h"

    #elif defined(SPMIDI_PRESETS_CUSTOM_4)
        #define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_4.h"

    #elif defined(SPMIDI_PRESETS_TEST)
        #define SPMIDI_PRESET_FILENAME_H "spmidi_presets_test.h"

    #else
        #define SPMIDI_PRESET_FILENAME_H "spmidi_hybrid_presets_me2000.h"

    #endif
#else /* SPMIDI_ME2000 */
    #define SPMIDI_PRESET_FILENAME_H "spmidi_hybrid_presets_me1000.h"
#endif /* SPMIDI_ME2000 */

#include SPMIDI_PRESET_FILENAME_H

/* Verify that the included preset file is at the necessary revision level. */
#ifndef SPMIDI_EDITOR_VERSION
    #define SPMIDI_EDITOR_VERSION (0)
#endif

#define SPMIDI_EARLIEST_COMPATIBLE_VERSION (225)

#if  SPMIDI_EDITOR_VERSION < SPMIDI_EARLIEST_COMPATIBLE_VERSION
    #error Included preset file is obsolete. Please use newly exported instrument set.
#endif


/** @return Number of entries in DrumMap */
static int SS_GetSynthDrumMapSize( void );

/********************************************************************************/
HybridOrchestra_t *SS_GetCompiledOrchestra( void )
{
    return &sCompiledOrchestra;
}

/********************************************************************************/
static void SS_FreeMelodicBank( ProgramBankMap_t *melodicBank )
{
    if( (melodicBank->flags & SS_PROGRAM_MAP_ALLOCATED) != 0 )
    {
        SPMIDI_FREE_MEM( melodicBank );
    }
}

/********************************************************************************/
static void SS_FreeDrumProgram( ProgramBankMap_t *drumProgram )
{
    if( (drumProgram->flags & SS_PROGRAM_MAP_ALLOCATED) != 0 )
    {
        SPMIDI_FREE_MEM( drumProgram->pitches );
        SS_FreeMelodicBank( drumProgram );
    }
}

/********************************************************************************/
/** Used by editor.
 * Allocate presets array right after structure.
 * Do not add to list because it may be a drum.
 */
static ProgramBankMap_t *SS_AllocateMelodicBankNode( int bankIndex )
{
    int i;
    int numBytes = sizeof(ProgramBankMap_t) + 128;
    ProgramBankMap_t *melodicBank = SPMIDI_ALLOC_MEM( numBytes, "ProgramBankMap_t" );
    if( melodicBank == NULL )
    {
        return NULL;
    }

    melodicBank->presetMap = (unsigned char *) &melodicBank[1]; // point to memory after structure
    melodicBank->bankIndex = (short) bankIndex;
    melodicBank->programIndex = (short) 0;
    melodicBank->flags |= SS_PROGRAM_MAP_ALLOCATED;

    /* Fill map tables. */
    for( i=0; i<GMIDI_NUM_PROGRAMS; i++ )
    {
        melodicBank->presetMap[i] = MELODIC_STUB_INDEX;
    }

    return melodicBank;
}

/********************************************************************************/
static void SS_AddMelodicBankToOrchestra( HybridOrchestra_t *orchestra, ProgramBankMap_t *melodicBank )
{
    DLL_InitNode( &melodicBank->node );
    DLL_AddTail( &orchestra->melodicBankList, &melodicBank->node );
}

/********************************************************************************/
static void SS_AddDrumProgramToOrchestra( HybridOrchestra_t *orchestra, ProgramBankMap_t *drumProgram )
{
    DLL_InitNode( &drumProgram->node );
    DLL_AddTail( &orchestra->drumProgramList, &drumProgram->node );
}

/********************************************************************************/
void SS_AddMelodicBankToList( ProgramBankMap_t *melodicBank )
{
    SS_AddMelodicBankToOrchestra( SS_GetCompiledOrchestra(), melodicBank );
}

/********************************************************************************/
void SS_AddDrumProgramToList( ProgramBankMap_t *drumProgram )
{
    SS_AddDrumProgramToOrchestra( SS_GetCompiledOrchestra(), drumProgram );
}

/********************************************************************************/
static ProgramBankMap_t *SS_AllocateMelodicBank( HybridOrchestra_t *orchestra, int bankIndex )
{
    ProgramBankMap_t *melodicBank = SS_AllocateMelodicBankNode( bankIndex );
    if( melodicBank != NULL )
    {
        SS_AddMelodicBankToOrchestra( orchestra, melodicBank );
    }
    return melodicBank;
}

/********************************************************************************/
static ProgramBankMap_t *SS_AllocateDrumProgram( HybridOrchestra_t *orchestra, int bankIndex, int programIndex )
{
    int i;
    ProgramBankMap_t *drumProgram = SS_AllocateMelodicBankNode( bankIndex );
    if( drumProgram == NULL )
    {
        return NULL;
    }

    // Add pitch array.
    drumProgram->pitches = SPMIDI_ALLOC_MEM( SS_GetSynthDrumMapSize(), "DrumPitches" );
    if( drumProgram->pitches == NULL )
    {
        SS_FreeMelodicBank( drumProgram );
        return NULL;
    }

    drumProgram->programIndex = (unsigned char) programIndex;

    for( i=0; i<GMIDI_NUM_DRUMS; i++ )
    {
        drumProgram->presetMap[i] = DRUM_STUB_INDEX;
        drumProgram->pitches[i] = 60; /* Middle C */
    }
    
    SS_AddDrumProgramToOrchestra( orchestra, drumProgram );
    
    return drumProgram;
}


/********************************************************************************/
static ProgramBankMap_t *SS_FindMelodicBank( HybridOrchestra_t *orchestra, int bankIndex )
{
    ProgramBankMap_t *melodicBank = NULL;
    ProgramBankMap_t *candidate;
    DoubleList *bankList = &orchestra->melodicBankList;
    DLL_FOR_ALL( ProgramBankMap_t, candidate, bankList )
    {
        if( candidate->bankIndex == bankIndex )
        {
            melodicBank = candidate;
            break;
        }
    }
    return melodicBank;
}

/********************************************************************************/
/* Always return some bank so that we always have an instrument to play. */
static ProgramBankMap_t *SS_FindMelodicBankSafe( int bankIndex, HybridOrchestra_t **orchestraPtr )
{
    ProgramBankMap_t *melodicBank = NULL;
    HybridOrchestra_t *orchestra;
    DLL_FOR_ALL( HybridOrchestra_t, orchestra, &sOrchestraList )
    {
        melodicBank = SS_FindMelodicBank( orchestra, bankIndex );
        if( melodicBank != NULL )
        {
            *orchestraPtr = orchestra;
            break;
        }
    }

    //PRTMSGNUMD("SS_FindMelodicBankSafe: bankIndex = ", bankIndex );
    //PRTMSGNUMD("SS_FindMelodicBankSafe: melodicBank = ", (int)melodicBank );
    if( melodicBank == NULL )
    {
        orchestra = (HybridOrchestra_t *) DLL_Last( &sOrchestraList );
        *orchestraPtr = orchestra;
        if( DLL_IsEmpty( &orchestra->melodicBankList ) )
        {
            melodicBank = SS_AllocateMelodicBank( orchestra, 0 );
        }
        else
        {
            melodicBank = (ProgramBankMap_t *) DLL_First( &orchestra->melodicBankList );
        }
    }
    //PRTMSGNUMD("SS_FindMelodicBankSafe: melodicBank.bankIndex = ", melodicBank->bankIndex );
    return melodicBank;
}

/********************************************************************************/
static ProgramBankMap_t * SS_FindDrumProgram( HybridOrchestra_t *orchestra, int bankIndex, int programIndex )
{
    ProgramBankMap_t *drumProgram = NULL;
    ProgramBankMap_t *candidate;
    DoubleList *programList = &orchestra->drumProgramList;
    DLL_FOR_ALL( ProgramBankMap_t, candidate, programList )
    {
        if( (candidate->bankIndex == bankIndex) && (candidate->programIndex == programIndex))
        {
            drumProgram = candidate;
            break;
        }
    }
    return drumProgram;
}

/********************************************************************************/
/* Always return some bank so that we always have an instrument to play. */
static ProgramBankMap_t *SS_FindDrumProgramSafe( int bankIndex, int programIndex, HybridOrchestra_t **orchestraPtr )
{
    ProgramBankMap_t *drumProgram = NULL;
    HybridOrchestra_t *orchestra;
    DLL_FOR_ALL( HybridOrchestra_t, orchestra, &sOrchestraList )
    {
        drumProgram = SS_FindDrumProgram( orchestra, bankIndex, programIndex );
        if( drumProgram != NULL )
        {
            *orchestraPtr = orchestra;
            break;
        }
    }

    //PRTMSGNUMD("SS_FindDrumProgramSafe: bankIndex = ", bankIndex );
    //PRTMSGNUMD("SS_FindDrumProgramSafe: programIndex = ", programIndex );
    //PRTMSGNUMD("SS_FindDrumProgramSafe: drumProgram = ", (int)drumProgram );
    if( drumProgram == NULL )
    {
        orchestra = (HybridOrchestra_t *) DLL_Last( &sOrchestraList );
        *orchestraPtr = orchestra;
        if( DLL_IsEmpty( &orchestra->drumProgramList ) )
        {
            drumProgram = SS_AllocateDrumProgram( orchestra, 0, 0 );
        }
        else
        {
            drumProgram = (ProgramBankMap_t *) DLL_First( &orchestra->drumProgramList );
        }
    }
    //PRTMSGNUMD("SS_FindDrumProgramSafe: drumProgram->bankIndex = ", drumProgram->bankIndex );
    //PRTMSGNUMD("SS_FindDrumProgramSafe: drumProgram->programIndex = ", drumProgram->programIndex );
    return drumProgram;
}

/********************************************************************************/
static int SS_GetSynthDrumMapSize( void )
{
    return GMIDI_NUM_DRUMS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
EDITABLE HybridVoice_Preset_t *SS_GetSynthPreset( HybridOrchestra_t *orchestra, int presetIndex )
{
    if( presetIndex == MELODIC_STUB_INDEX )
    {
        return &gHybridSynthPresets[ 0 ];
    }
    else if( presetIndex == DRUM_STUB_INDEX )
    {
        return &gHybridSynthPresets[ 1 ];
    }
    else
    {
        return &orchestra->hybridSynthPresets[ presetIndex ];
    }
}

/********************************************************************************/
EDITABLE HybridVoice_Preset_t *SS_GetSynthMelodicPreset( int bankIndex, int programIndex, HybridOrchestra_t **orchestraPtr )
{
    HybridOrchestra_t *orchestra;
    ProgramBankMap_t *melodicBank = SS_FindMelodicBankSafe( bankIndex, &orchestra );
    int presetIndex = melodicBank->presetMap[programIndex];
    *orchestraPtr = orchestra;
    return SS_GetSynthPreset( orchestra, presetIndex );
}

/********************************************************************************/
EDITABLE HybridVoice_Preset_t *SS_GetSynthDrumPreset( int bankIndex, int programIndex, int noteIndex, HybridOrchestra_t **orchestraPtr )
{
    int presetIndex;
    HybridOrchestra_t *orchestra;
    ProgramBankMap_t *drumProgram;
    int drumIndex = noteIndex - GMIDI_FIRST_DRUM;
    if( (drumIndex < 0) ||
        (drumIndex >= SS_GetSynthDrumMapSize()) )
    {
        drumIndex = 0;
    }
    drumProgram = SS_FindDrumProgramSafe( bankIndex, programIndex, &orchestra );
    presetIndex = drumProgram->presetMap[drumIndex];
    *orchestraPtr = orchestra;
    return SS_GetSynthPreset( orchestra, presetIndex );
}

/********************************************************************************/
int SS_GetSynthPresetCount( void )
{
    return SS_NUM_VALID_PRESETS;
}

/********************************************************************************/
int SS_GetSynthDrumPitch( int bankIndex, int programIndex, int noteIndex )
{
    HybridOrchestra_t *orchestra;
    ProgramBankMap_t *drumProgram = SS_FindDrumProgramSafe( bankIndex, programIndex, &orchestra );
    int drumIndex = noteIndex - GMIDI_FIRST_DRUM;
    if( (drumIndex < 0) ||
            (drumIndex >= SS_GetSynthDrumMapSize()) )
    {
        drumIndex = 0;
    }
    return drumProgram->pitches[drumIndex];
}

#if SPMIDI_SUPPORT_LOADING
/********************************************************************************/
int SS_SetInstrumentMap( HybridOrchestra_t * orchestra, int bankIndex, int programIndex, int insIndex )
{
    ProgramBankMap_t *melodicBank;

    if( (programIndex < 0) || (programIndex > 127) )
        return SPMIDI_Error_OutOfRange;
    if( (insIndex >= SS_MAX_PRESETS) || (insIndex < 0) )
        return SPMIDI_Error_OutOfRange;
    
    melodicBank = SS_FindMelodicBank( orchestra, bankIndex );
    if( melodicBank == NULL )
    {
        // Make sure we have a fallback bank of zero.
        if( DLL_IsEmpty( &orchestra->melodicBankList ) && (bankIndex != 0) )
        {
            SS_AllocateMelodicBank( orchestra, 0 );
        }
        melodicBank = SS_AllocateMelodicBank( orchestra, bankIndex );
    }

    melodicBank->presetMap[programIndex] = (unsigned char) insIndex;
    return 0;
}

/********************************************************************************/
int SS_SetDrumMap( HybridOrchestra_t * orchestra, int bankIndex, int programIndex, int noteIndex, int insIndex, int pitch )
{
    ProgramBankMap_t *drumProgram;
    int drumIndex = noteIndex - GMIDI_FIRST_DRUM;
    
    if( (drumIndex >= GMIDI_NUM_DRUMS) || (drumIndex < 0) )
        return SPMIDI_Error_OutOfRange;
    if( (insIndex >= SS_MAX_PRESETS) || (insIndex < 0) )
        return SPMIDI_Error_OutOfRange;
    if( (pitch > 127) || (pitch < 0) )
        return SPMIDI_Error_OutOfRange;

    drumProgram = SS_FindDrumProgram( orchestra, bankIndex, programIndex );
    if( drumProgram == NULL )
    {
        // Make sure we have a fallback bank of zero.
        if( DLL_IsEmpty( &orchestra->drumProgramList ) && (bankIndex != 0) && (programIndex != 0) )
        {
            SS_AllocateDrumProgram( orchestra, 0, 0 );
        }
        drumProgram = SS_AllocateDrumProgram( orchestra, bankIndex, programIndex );
    }

    drumProgram->presetMap[drumIndex] = (unsigned char) insIndex;
    drumProgram->pitches[drumIndex] = (unsigned char) pitch;
    
    return 0;
}

/********************************************************************************/
void SS_ChangeMelodicMap( int oldBankIndex, int newBankIndex )
{
    // Only used by editor.
    ProgramBankMap_t *melodicMap = SS_FindMelodicBank( SS_GetCompiledOrchestra(), oldBankIndex );
    //PRTMSGNUMD("SS_ChangeMelodicMap: oldBankIndex = ", oldBankIndex );
    //PRTMSGNUMD("SS_ChangeMelodicMap: newBankIndex = ", newBankIndex );
    //PRTMSGNUMD("SS_ChangeMelodicMap: melodicMap = ", (int)melodicMap );
    if( melodicMap != NULL )
    {
        melodicMap->bankIndex = (short) newBankIndex;
    }
}

/********************************************************************************/
void SS_ChangeDrumMap( int oldBankIndex, int oldProgramIndex, int newBankIndex, int newProgramIndex )
{
    ProgramBankMap_t *drumProgram = SS_FindDrumProgram( SS_GetCompiledOrchestra(), oldBankIndex, oldProgramIndex );
    
    //PRTMSGNUMD("SS_ChangeDrumMap: oldBankIndex = ", oldBankIndex );
    //PRTMSGNUMD("SS_ChangeDrumMap: oldProgramIndex = ", oldProgramIndex );
    //PRTMSGNUMD("SS_ChangeDrumMap: newBankIndex = ", newBankIndex );
    //PRTMSGNUMD("SS_ChangeDrumMap: newProgramIndex = ", newProgramIndex );
    //PRTMSGNUMD("SS_ChangeDrumMap: drumProgram = ", (int)drumProgram );
    if( drumProgram != NULL )
    {
        drumProgram->bankIndex = (short) newBankIndex;
        drumProgram->programIndex = (unsigned char) newProgramIndex;
    }
}

/********************************************************************************/
void SS_RemoveMelodicMap( int bankIndex )
{
    ProgramBankMap_t *melodicBank;

    melodicBank = SS_FindMelodicBank( SS_GetCompiledOrchestra(), bankIndex );
    //PRTMSGNUMD("SS_RemoveMelodicMap: bankIndex = ", bankIndex );
    //PRTMSGNUMD("SS_RemoveMelodicMap: melodicBank = ", (int)melodicBank );
    if( melodicBank != NULL )
    {
        DLL_Remove( &melodicBank->node );
        SS_FreeMelodicBank( melodicBank );
    }
}
/********************************************************************************/
void SS_RemoveDrumMap( int bankIndex, int programIndex )
{
    ProgramBankMap_t *drumProgram;

    drumProgram = SS_FindDrumProgram( SS_GetCompiledOrchestra(), bankIndex, programIndex );
    //PRTMSGNUMD("SS_RemoveDrumMap: bankIndex = ", bankIndex );
    //PRTMSGNUMD("SS_RemoveDrumMap: programIndex = ", programIndex );
    //PRTMSGNUMD("SS_RemoveDrumMap: drumProgram = ", (int)drumProgram );
    if( drumProgram != NULL )
    {
        DLL_Remove( &drumProgram->node );
        SS_FreeDrumProgram( drumProgram );
    }
}

/***********************************************************************/
/** Download a WaveTable for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 */
int SPMIDI_AddWaveTable( SPMIDI_Orchestra *orchestra, WaveTable_t *waveTable )
{
    return WaveManager_AddWaveTable( &((HybridOrchestra_t *) orchestra)->waveManager, waveTable );
}

/***********************************************************************/
/* Delete WaveTable if WaveSet reference count is zero. */
int SS_UnloadWaveTable( HybridOrchestra_t *orchestra, spmSInt32 token )
{
    return WaveManager_UnloadWaveTable( &orchestra->waveManager, token );
}

/***********************************************************************/
/** Download a WaveSet for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 */
int SPMIDI_LoadWaveSet( SPMIDI_Orchestra *orchestra, ResourceTokenMap_t *tokenMap, unsigned char *data, int numBytes )
{
    return WaveManager_LoadWaveSet( &((HybridOrchestra_t *) orchestra)->waveManager, tokenMap, data, numBytes );
}

/***********************************************************************/
/** Add a WaveSet for internal storage and use.
 */
int SPMIDI_AddWaveSet( SPMIDI_Orchestra *orchestra, WaveSet_t *waveSet, int id )
{
    return WaveManager_AddWaveSet( &((HybridOrchestra_t *) orchestra)->waveManager, waveSet, id );
}

/***********************************************************************/
/* Delete WaveSet if instrument reference count is zero. */
int SS_UnloadWaveSet( HybridOrchestra_t *orchestra, spmSInt32 token )
{
    return WaveManager_UnloadWaveSet( &orchestra->waveManager, token );
}

/***********************************************************************/
/** Download a WaveTable for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 */
int SPMIDI_LoadWaveTable( SPMIDI_Orchestra *orchestra, unsigned char *data, int numBytes )
{
    return WaveManager_LoadWaveTable( &((HybridOrchestra_t *) orchestra)->waveManager, data, numBytes );
}

#endif /* SPMIDI_SUPPORT_LOADING */

/********************************************************************************/
int SS_Orchestra_InitOrchestra( HybridOrchestra_t * orchestra )
{
    MemTools_Clear( orchestra, sizeof( HybridOrchestra_t ) );
    DLL_InitNode( &orchestra->node );
    DLL_InitList( &orchestra->melodicBankList );
    DLL_InitList( &orchestra->drumProgramList );
    
#if SPMIDI_ME2000
    WaveManager_Init( &orchestra->waveManager );
#endif
    return 0;
}

/********************************************************************************/
void SS_Orchestra_TermOrchestra( HybridOrchestra_t * orchestra )
{
    
#if SPMIDI_ME2000
    WaveManager_Term( &orchestra->waveManager );
#endif

    while( !DLL_IsEmpty( &orchestra->melodicBankList ) )
    {
        ProgramBankMap_t *melodicBank = (ProgramBankMap_t *) DLL_First( &orchestra->melodicBankList );
        DLL_Remove( &melodicBank->node );
        SS_FreeMelodicBank( melodicBank );
    }
    while( !DLL_IsEmpty( &orchestra->drumProgramList ) )
    {
        ProgramBankMap_t *drumProgram = (ProgramBankMap_t *) DLL_First( &orchestra->drumProgramList );
        DLL_Remove( &drumProgram->node );
        SS_FreeDrumProgram( drumProgram );
    }
}

/********************************************************************************/
int SPMIDI_CreateOrchestra( SPMIDI_Orchestra **spmidiOrchestraPtr, spmSInt32 numInstruments )
{
    HybridOrchestra_t *orchestra = SPMIDI_ALLOC_MEM( sizeof( HybridOrchestra_t ), "HybridOrchestra" );
    if( orchestra == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }

    SS_Orchestra_InitOrchestra( orchestra );

    orchestra->hybridSynthPresets = SPMIDI_ALLOC_MEM( numInstruments * sizeof(HybridVoice_Preset_t), "PresetArray" );
    if( orchestra->hybridSynthPresets == NULL )
    {
        SPMIDI_FREE_MEM( orchestra );
        return SPMIDI_Error_OutOfMemory;
    }

    /* Add to the head so it will be seen before other orchestras and override them. */
    DLL_AddHead( &sOrchestraList, &orchestra->node );

    *spmidiOrchestraPtr = orchestra;
    return 0;
}

/********************************************************************************/
void SPMIDI_DeleteOrchestra( SPMIDI_Orchestra *spmidiOrchestra )
{
    HybridOrchestra_t * orchestra = (HybridOrchestra_t *) spmidiOrchestra;
    if( orchestra == NULL )
    {
        return;
    }
    DLL_Remove( &orchestra->node );

    SS_Orchestra_TermOrchestra( orchestra );
    if( orchestra->hybridSynthPresets != NULL )
    {
        SPMIDI_FREE_MEM( orchestra->hybridSynthPresets );
    }
    SPMIDI_FREE_MEM( orchestra );
}


/********************************************************************************/
int SS_Orchestra_Init( void )
{
    DLL_InitList( &sOrchestraList );
    SS_Orchestra_InitOrchestra( &sCompiledOrchestra );
    sCompiledOrchestra.hybridSynthPresets = (struct HybridVoice_Preset_s *) gHybridSynthPresets;
    DLL_AddTail( &sOrchestraList, &sCompiledOrchestra.node );
    
#if SPMIDI_ME2000
    WaveManager_LoadWaves( &sCompiledOrchestra.waveManager );
    #if SPMIDI_TEST_WAVETABLE
        /* Load simple waveform for testing. */
        SS_LoadTestWaves( &sCompiledOrchestra.waveManager );
    #endif
#endif

    return 0;
}

/********************************************************************************/
void SS_Orchestra_Term( void )
{
    while( !DLL_IsEmpty( &sOrchestraList ) )
    {
        HybridOrchestra_t * orchestra = (HybridOrchestra_t *) DLL_First( &sOrchestraList );
        if( orchestra == &sCompiledOrchestra )
        {
            DLL_Remove( &orchestra->node );
        }
        else
        {
            SPMIDI_DeleteOrchestra( (SPMIDI_Orchestra *) orchestra );
        }
    }
}
