/* $Id: wave_manager.c,v 1.25 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 *
 * WaveTable manager.
 *
 * Copyright 2004 Mobileer, Phil Burk, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/engine/oscillator.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/engine/memtools.h"

#if SPMIDI_ME2000

#if SPMIDI_SUPPORT_LOADING
static WaveTable_t *WaveManager_UnreferenceWaveTable( WaveTable_t *waveTable );
#endif /* SPMIDI_SUPPORT_LOADING */

static void WaveManager_FreeWaveTable( WaveTable_t *waveTable );
static void WaveManager_FreeWaveSet( WaveSet_t *waveSet );

/* If the reference count is zero, then the object is in ROM and should not be freed.
 * So don't count the references.
 */
#define REFERENCE_START_TRACKING(trkr) { trkr.referenceCount = 1; }
#define REFERENCE_IS_TRACKED(trkr) ( trkr.referenceCount > 0 )
#define REFERENCE_IS_UNUSED(trkr) ( trkr.referenceCount == 1 )
#define REFERENCE_INCREMENT(trkr) { if( trkr.referenceCount > 0 ) trkr.referenceCount += 1; }
#define REFERENCE_DECREMENT(trkr) { if( trkr.referenceCount > 1 ) trkr.referenceCount -= 1; }

#define GET_NEXT_ID   (waveManager->nextID++)

/****************************************************************/
/* Initialize linked lists and prepare for storing tables. */
spmSInt32  WaveManager_Init( WaveManager_t *waveManager )
{
    DLL_InitList( &waveManager->waveTableList );
    DLL_InitList( &waveManager->waveSetList );
    /* Start at 1 so we know zero means no ID. */
    waveManager->nextID = 1;
    waveManager->totalWaveBytes = 0;
    return 0;
}

/****************************************************************/
/* Free all lingering data. */
spmSInt32 WaveManager_Term( WaveManager_t *waveManager )
{
    while( !DLL_IsEmpty( &waveManager->waveSetList ) )
    {
        WaveSet_t *waveSet = (WaveSet_t *) DLL_First( &waveManager->waveSetList );
        DLL_Remove( &waveSet->tracker.node );
        if( REFERENCE_IS_TRACKED( waveSet->tracker ) )
        {
            WaveManager_FreeWaveSet(waveSet);
        }
    }
    while( !DLL_IsEmpty( &waveManager->waveTableList ) )
    {
        WaveTable_t *waveTable = (WaveTable_t *) DLL_First( &waveManager->waveTableList );
        DLL_Remove( &waveTable->tracker.node );
        if( REFERENCE_IS_TRACKED( waveTable->tracker ) )
        {
            WaveManager_FreeWaveTable(waveTable);
        }
    }
    return SPMIDI_Error_None;
}

/****************************************************************/
/** 
 */
static spmSInt32 WaveManager_JustAddWaveSet( WaveManager_t *waveManager, WaveSet_t *waveSet, spmResourceToken id )
{

    if( id == RESOURCE_UNDEFINED_ID )
    {
        id = GET_NEXT_ID; /* TODO review calls to this function and nextID */
    }
    if( waveManager->nextID <= id )
    {
        waveManager->nextID = id + 1;
    }
    ResourceMgr_Add( &waveManager->waveSetList, &waveSet->tracker, id );

    return id;
}

/****************************************************************/
/** Add WaveSet to managed storage.
 * There is potential conflict if a caller passes a low ID.
 * So this should only be called internally when wavetables are loaded.
 */
spmSInt32 WaveManager_AddWaveSet( WaveManager_t *waveManager, WaveSet_t *waveSet, spmResourceToken id )
{
    ResourceMgr_InitResource( &waveSet->tracker );
    return WaveManager_JustAddWaveSet( waveManager, waveSet, id );
}


WaveSet_t * WaveManager_FindWaveSet( WaveManager_t *waveManager, spmResourceToken token )
{
    return (WaveSet_t *) ResourceMgr_Find( &waveManager->waveSetList, token );
}

WaveSet_t * WaveManager_GetFirstWaveSet( WaveManager_t *waveManager )
{
    return (WaveSet_t *) DLL_First( &waveManager->waveSetList );
}


#if SPMIDI_SUPPORT_LOADING

static void WaveManager_FreeWaveTable( WaveTable_t *waveTable )
{
    if( waveTable->samples != NULL )
    {
        SPMIDI_FreeMemory( waveTable->samples ); /* MemoryTracking - Free #2 */
    }
    SPMIDI_FreeMemory( waveTable );     /* MemoryTracking -  Free #1 */
}
static void WaveManager_FreeWaveSet( WaveSet_t *waveSet )
{
    SPMIDI_FreeMemory( waveSet );     /* MemoryTracking - Free #3 */
}

/****************************************************************/
/** Just add the WaveTable without initializing the resource tracker.
 * Used when adding a downloaded WaveTable that has already been reference counted.
 */
static spmSInt32 WaveManager_JustAddWaveTable( WaveManager_t *waveManager, WaveTable_t *waveTable )
{
    ResourceMgr_Add( &waveManager->waveTableList, &waveTable->tracker, GET_NEXT_ID );
    waveManager->totalWaveBytes += waveTable->numSamples * sizeof(spmSInt16);
    return waveTable->tracker.token;
}

/****************************************************************/
/** Download a ready WaveTable for internal storage and use.
 * Return negative error or positive token.
 */
spmSInt32 WaveManager_AddWaveTable( WaveManager_t *waveManager, WaveTable_t *waveTable )
{
    ResourceMgr_InitResource( &waveTable->tracker );
    return WaveManager_JustAddWaveTable( waveManager, waveTable );
}

/****************************************************************/
/* If reference count goes down to 1 then delete waveTable and return NULL
 * Return waveTable if not deleted.
 */
static WaveTable_t *WaveManager_UnreferenceWaveTable( WaveTable_t *waveTable )
{
    if( waveTable == NULL )
    {
        return NULL;
    }
    
    REFERENCE_DECREMENT(waveTable->tracker);
    PRTMSGNUMD("WaveManager_UnreferenceWaveTable: token = ", waveTable->tracker.token );
    PRTMSGNUMD("WaveManager_UnreferenceWaveTable: count after decrement = ", waveTable->tracker.referenceCount );
    /* Is everyone done using this? */
    if( REFERENCE_IS_UNUSED( waveTable->tracker ) )
    {
        PRTMSGNUMH("WaveManager_UnreferenceWaveTable: free wavetable at ", (long) waveTable );
        DLL_Remove( &waveTable->tracker.node );
        WaveManager_FreeWaveTable( waveTable );
        waveTable = NULL;
    }
    return waveTable;
}

/****************************************************************/
/* Delete WaveTable if WaveSet reference count is zero. */
spmSInt32 WaveManager_UnloadWaveTable( WaveManager_t *waveManager, spmResourceToken token )
{
    WaveTable_t *waveTable = (WaveTable_t *) ResourceMgr_Find( &waveManager->waveTableList, token );
    if( waveTable == NULL )
    {
        return SPMIDI_Error_BadToken;
    }

    /* Mark as being invalid for lookup by this routine. */
    waveTable->tracker.token = 0;
    WaveManager_UnreferenceWaveTable( waveTable );

    return SPMIDI_Error_None;
}


/****************************************************************/
/** Download a WaveTable for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 * Data is in the form of a byte stream so it can come from Java.
 * Return negative error or positive token.
 */
spmSInt32 WaveManager_LoadWaveTable( WaveManager_t *waveManager, unsigned char *data,
                                     int numBytes )
{
    WaveTable_t *waveTable;
    int err = SPMIDI_Error_BadFormat;
    unsigned char *p = data;
    int numSampleBytes;
    int bytesRead;
    int i;

    /* Parse stream header */
    /*
        Must match stream builder in Java editor and test routines.
        WaveTable byte stream
        1   SPMIDI_BEGIN_STREAM
        1   SPMIDI_WAVETABLE_STREAM_ID
        4   PitchOctave      sampleRateOffset;
        4   int              loopBegin;
        4   int              loopEnd;
        4   int              numSamples
        1   byte             velocity
        1   byte             type; 0 for signed 16, 1 for ALaw

        finally
          numSamples*2 short[]  samples; if S16
        OR
          numSamples byte[] samples; if ALaw

        1   SPMIDI_END_STREAM
    */
    if( *p++ != SPMIDI_BEGIN_STREAM )
        goto error;
    if( *p++ != SPMIDI_WAVETABLE_STREAM_ID )
        goto error;

    /* Allocate WaveTable */
    waveTable = SPMIDI_AllocateMemory(sizeof(WaveTable_t));  /* MemoryTracking - Allocation #1 */
    if( waveTable == NULL )
    {
        err = SPMIDI_Error_OutOfMemory;
        goto error;
    }
    MemTools_Clear( waveTable, sizeof(WaveTable_t) );
    DLL_InitNode( &waveTable->tracker.node );
    REFERENCE_START_TRACKING( waveTable->tracker );
    REFERENCE_INCREMENT( waveTable->tracker );

    p = SS_ParseLong( &waveTable->sampleRateOffset, p );
    p = SS_ParseLong( &waveTable->loopBegin, p );
    p = SS_ParseLong( &waveTable->loopEnd, p );

    p = SS_ParseLong( &waveTable->numSamples, p );

    waveTable->lowVelocity = *p++;
    waveTable->highVelocity = *p++;

    if( *p++ == SPMIDI_WAVE_TYPE_ALAW )
    {
        unsigned char *samples8;

        numSampleBytes = waveTable->numSamples; /* ALaw is one byte per sample. */
        //PRTMSGNUMD("Unpacking ALAW, numSampleBytes = ", numSampleBytes );
        samples8 = SPMIDI_AllocateMemory(numSampleBytes);     /* MemoryTracking - Allocation #2 */
        if( samples8 == NULL )
        {
            err = SPMIDI_Error_OutOfMemory;
            goto cleanup;
        }

        /* Move bytes from stream to waveTable. */
        waveTable->samples = samples8;
        for( i=0; i<numSampleBytes; i++ )
        {
            *samples8++ = *p++;
        }
        waveTable->type = SPMIDI_WAVE_TYPE_ALAW;
        //PRTMSGNUMD("Finished ALAW, numSampleBytes = ", numSampleBytes );
    }
    else
    {
        spmSample *samples;
        numSampleBytes = waveTable->numSamples * sizeof(spmSample);
        samples = SPMIDI_AllocateMemory(numSampleBytes);     /* MemoryTracking - Allocation #2 */
        if( samples == NULL )
        {
            err = SPMIDI_Error_OutOfMemory;
            goto cleanup;
        }

        /* Parse from BigEndian data format to CPU native format. */
        waveTable->samples = samples;
        for( i=0; i<waveTable->numSamples; i++ )
        {
            p = SS_ParseShort( samples++, p );
        }
        waveTable->type = SPMIDI_WAVE_TYPE_S16;
    }

    if( *p++ != SPMIDI_END_STREAM )
        goto cleanup;
    bytesRead = p - data;
    if( bytesRead != numBytes )
    {
        err = SPMIDI_Error_IllegalSize;
        goto cleanup;
    }

    return WaveManager_JustAddWaveTable( waveManager, waveTable );

cleanup:
    WaveManager_UnreferenceWaveTable( waveTable );
error:
    return err;
}

/****************************************************************/
/** Download a WaveSet for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 */
spmSInt32 WaveManager_LoadWaveSet( WaveManager_t *waveManager, ResourceTokenMap_t *tokenMap, unsigned char *data, int numBytes )
{
    WaveSetRegion_t *regionArray;
    WaveSet_t *waveSet = NULL;
    int err = SPMIDI_Error_BadFormat;
    int numTables;
    unsigned char *p = data;
    int bytesRead;
    int i;
    int waveSetSize;
    FXP16 midiPitch;

    /* Parse stream header */
    if( *p++ != SPMIDI_BEGIN_STREAM )
        goto error;
    if( *p++ != SPMIDI_WAVESET_STREAM_ID )
        goto error;

    /* Parse table tokens from stream, find them and fill array. */
    numTables = *p++;
    if( numTables == 0 )
    {
        err = SPMIDI_Error_OutOfRange;
        goto error;
    }

    /* Allocate WaveSet with WaveSetRegion array at end. */
    waveSetSize = sizeof(WaveSet_t) + (numTables * sizeof(WaveSetRegion_t));
    waveSet = SPMIDI_AllocateMemory(waveSetSize);   /* MemoryTracking - Allocation #3 */
    if( waveSet == NULL )
    {
        err = SPMIDI_Error_OutOfMemory;
        goto error;
    }
    MemTools_Clear( waveSet, sizeof(WaveSet_t) );
    DLL_InitNode( &waveSet->tracker.node );
    REFERENCE_START_TRACKING( waveSet->tracker );
    WaveManager_ReferenceWaveSet( waveSet );

    /* Region array is immediately after the waveset. */
    regionArray = (WaveSetRegion_t *)(waveSet + 1);
    waveSet->regions = regionArray;
    for( i=0; i<numTables; i++ )
    {
        WaveTable_t *waveTable;
        spmResourceToken token;
        WaveSetRegion_t *region = &regionArray[i];
        
        p = SS_ParseLong( &midiPitch, p );
        
        //PRTMSGNUMH( "WaveManager_LoadWaveSet: midiPitch = ", midiPitch );
        region->basePitch = SPMUtil_MIDIPitchToOctave( midiPitch );
        region->lowPitch = *p++;
        //PRTMSGNUMH( "WaveManager_LoadWaveSet: low Pitch = ", region->lowPitch );
        region->highPitch = *p++;
        //PRTMSGNUMH( "WaveManager_LoadWaveSet: high Pitch = ", region->highPitch );
        region->lowVelocity = *p++;
        region->highVelocity = *p++;

        p = SS_ParseLong( (long *) &token, p );

        // If we have a token map then map external token to internal token.
        if( tokenMap != NULL )
        {
            token = tokenMap[ token ].token;
        }
        waveTable = (WaveTable_t *) ResourceMgr_Find( &waveManager->waveTableList, token );
        if( waveTable == NULL )
        {
            //PRTMSGNUMH( "WaveManager_LoadWaveSet lowPitch = ", region->lowPitch );
            PRTMSGNUMH( "WaveManager_LoadWaveSet got bad token = ", token );
            err = SPMIDI_Error_BadToken;
            goto cleanup;
        }
        //PRTMSGNUMH("WaveManager_LoadWaveSet referenced WaveTable at ", waveTable );
        REFERENCE_INCREMENT(waveTable->tracker);
        waveSet->numTables += 1;
#if SPMIDI_USE_REGIONS
        region->table = waveTable;
#else
        tableArray[i] = waveTable;
#endif
    }

    if( *p++ != SPMIDI_END_STREAM )
    {
        PRTMSG( "WaveManager_LoadWaveSet did not see SPMIDI_END_STREAM\n" );
        goto cleanup;
    }
    bytesRead = p - data;
    if( bytesRead != numBytes )
    {
        err = SPMIDI_Error_IllegalSize;
        goto cleanup;
    }

    /* Assign ID and add WaveWrapper to manager. Don't init reference counter. */
    WaveManager_JustAddWaveSet( waveManager, waveSet, GET_NEXT_ID );

    return waveSet->tracker.token;

cleanup:
    WaveManager_UnreferenceWaveSet( waveSet );
error:
    return err;
}

/** Reference WaveSet so it does not get deleted while in use. */
void WaveManager_ReferenceWaveSet( WaveSet_t *waveSet )
{
    REFERENCE_INCREMENT( waveSet->tracker );
}

/****************************************************************/
/* If reference count == 1 then delete waveSet and return NULL
 * Return waveSet if not deleted.
 */
WaveSet_t * WaveManager_UnreferenceWaveSet( WaveSet_t *waveSet )
{
    int i;
    if( waveSet == NULL )
        return NULL;
    REFERENCE_DECREMENT(waveSet->tracker);
    PRTMSGNUMD("WaveManager_UnreferenceWaveSet: token = ", waveSet->tracker.token );
    PRTMSGNUMD("WaveManager_UnreferenceWaveSet: count after decrement = ", waveSet->tracker.referenceCount );

    /* Is everyone done using this WaveSet? */
    if( REFERENCE_IS_UNUSED( waveSet->tracker ) )
    {
        for( i=0; i<waveSet->numTables; i++ )
        {
#if SPMIDI_USE_REGIONS
            WaveSetRegion_t *region = &waveSet->regions[i];
            //PRTMSGNUMH( "DEREFERENCE WaveTable at ", region->table );
            WaveManager_UnreferenceWaveTable( (WaveTable_t *) region->table );
#else
            WaveTable_t *waveTable = waveSet->tables[i];
            WaveManager_UnreferenceWaveTable( waveTable );
#endif
        }
        
        PRTMSGNUMH("WaveManager_UnreferenceWaveSet: free waveset at ", waveSet );
        ResourceMgr_Remove( &waveSet->tracker );
        SPMIDI_FreeMemory(waveSet);
        waveSet = NULL;
    }
    return waveSet;
}

/****************************************************************/
/* Delete WaveSet if reference count is zero. */
spmSInt32 WaveManager_UnloadWaveSet( WaveManager_t *waveManager, spmResourceToken token )
{
    WaveSet_t *waveSet = (WaveSet_t *) ResourceMgr_Find( &waveManager->waveSetList, token );
    if( waveSet == NULL )
    {
        return SPMIDI_Error_BadToken;
    }
    /* Mark as being invalid for lookup by this routine. */
    waveSet->tracker.token = 0;
    WaveManager_UnreferenceWaveSet( waveSet );

    return SPMIDI_Error_None;
}
#else /* SPMIDI_SUPPORT_LOADING */
/* Stubs for runtime.
* These are required because simpler stubs result in compiler warnings.
*/
static void WaveManager_FreeWaveTable( WaveTable_t *waveTable )
{
    (void) waveTable;
}
static void WaveManager_FreeWaveSet( WaveSet_t *waveSet )
{
    (void) waveSet;
}
#endif /* SPMIDI_SUPPORT_LOADING */


#endif  /* SPMIDI_ME2000 */
