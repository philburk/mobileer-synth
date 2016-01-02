/*
 * $Id: mbis_parser.c,v 1.2 2007/10/10 00:23:47 philjmsl Exp $
 * Load an MBIS file containing multiple instruments.
 * This file is only needed if you plan to load Orchestras from .mbis files.
 *
 * Copyright 2007 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
// TODO
// make sure internal tokens do not grow forever and wrap around
// Editor needs to invalidate INFO structures after updating an instrument.
// Prefill maps with invalid instrument index so we can fall through to other orchestras.

#if 0
How does the ProgramList work?

Editor writes out a dependency list for each type.
A dependency list is an array of ID pairs: X depends on Y
PresetMaps contain their own inherent dependency information.
Instruments:WaveSets
WaveSets:WaveTables

Steps --------
Pass #1 through file gathering dependencies, allocating TokenMaps and loading ProgramMaps.
Create dummy TokenMap for Instruments.
Scan ProgramMaps and mark instruments needed.
Scan needed Instruments and mark WaveSets needed.
Scan needed WaveSets and mark WaveTables needed.
Pass #2 through file, skipping or loading resource if needed.

#endif

#include "spmidi/include/streamio.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/include/program_list.h"
#include "spmidi/include/mbis_parser.h"

#include "spmidi/engine/spmidi_host.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/engine/parse_riff.h"
#include "spmidi/engine/mbis_parser_internal.h"
#include "spmidi/engine/memtools.h"

#define DBUGMSGNUMD( msg, num ) { DBUGMSG( msg ); DBUGNUMD( num ); DBUGMSG("\n"); }

#if SPMIDI_SUPPORT_LOADING

static int gNeedIndent = 0;
#define MBIS_VERBOSITY  (1)
#if  MBIS_VERBOSITY
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

typedef struct MBIS_Dependency_s
{
    // The parent contains the child.
    spmSInt32   parentIndex;
    spmSInt32   childIndex;
} MBIS_Dependency_t;

typedef enum MBIS_Mode_e
{
    MBIS_ModeScanning,
    MBIS_ModeLoading
} MBIS_Mode;

typedef struct MBIS_Parser_s
{
    RiffParser_t        riffParser;
    MBIS_Mode           mode;
    spmSInt32           level;
    spmSInt32           abortList;
    MBIS_Orchestra_t    mbisOrchestra;
    // Temporary buffer for loading chunk data.
    spmUInt8           *buffer;
    spmSInt32           bufferSize;

    spmSInt32           numWaveSets;
    spmSInt32           numWaveTables;

    spmSInt32           numTokenMaps;
    ResourceTokenMap_t *tokenMaps;

    spmSInt32           numInstrumentMaps;
    ResourceTokenMap_t *instrumentMaps;

    spmSInt32           numInstrumentDependencies;
    MBIS_Dependency_t  *instrumentDependencies;

    spmSInt32           numWaveSetDependencies;
    MBIS_Dependency_t  *waveSetDependencies;

    SPMIDI_Orchestra   *spmidiOrchestra;
    SPMIDI_ProgramList *programList;
}
MBIS_Parser_t;

/* Use macros in case we want to change how we get the data. */
#define MBIS_READ_SHORT_LITTLE  RIFF_ReadShortLittle( &mbisParser->riffParser )
#define MBIS_READ_INT_LITTLE  RIFF_ReadIntLittle( &mbisParser->riffParser )
#define MBIS_Tell( mbisParser ) \
    ( mbisParser->riffParser.stream->getPosition( mbisParser->riffParser.stream ) )

#define TOUCH(value) \
    (void)value

#if MBIS_VERBOSITY

#define PRT_VALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( mbisParser ); } \
        DBUGMSG(format); \
        DBUGNUMD(value); \
    }
#define PRT_HVALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( mbisParser ); } \
        DBUGMSG(format); \
        DBUGNUMH(value); \
    }
#define PRTLN_VALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( mbisParser ); } \
        DBUGMSG(format); \
        DBUGNUMD(value); \
        DBUGMSG("\n"); \
        gNeedIndent = 1; \
    }
#define PRTLN_HVALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( mbisParser ); } \
        DBUGMSG(format); \
        DBUGNUMH(value); \
        DBUGMSG("\n"); \
        gNeedIndent = 1; \
    }
#define PRTLN_TEXT(text) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( mbisParser ); } \
        DBUGMSG(text); \
        DBUGMSG("\n"); \
        gNeedIndent = 1; \
    }

static void PrintChunk_Indent( MBIS_Parser_t *mbisParser )
{
    int i;
    for( i=0; i<mbisParser->level; i++ )
        DBUGMSG("    ");
    gNeedIndent = 0;
}

#else /* MBIS_VERBOSITY */
#define PRT_VALUE(format, value) TOUCH(value)
#define PRT_HVALUE(format, value) TOUCH(value)
#define PRTLN_VALUE(format, value) TOUCH(value)
#define PRTLN_HVALUE(format, value) TOUCH(value)
#define PRTLN_TEXT(text) TOUCH(text)
#define PrintChunk_Indent( mbisParser )
#endif /* MBIS_VERBOSITY */

static int PrintChunkInfo( MBIS_Parser_t *mbisParser, RIFF_ChunkID chunkID, long chunkSize, char *msg )
{
    char pad[5];
    pad[0] = (char) (chunkID >> 24);
    pad[1] = (char) (chunkID >> 16);
    pad[2] = (char) (chunkID >> 8);
    pad[3] = (char) chunkID;
    pad[4] = 0;

    PrintChunk_Indent( mbisParser );
    printf("%s: %s, %d bytes at %d\n", msg, pad, chunkSize, mbisParser->riffParser.stream->getPosition( mbisParser->riffParser.stream ) );
    return 0;
}

/**********************************************************************/
/* Load chunk into a temporary buffer in the parser. */
static int MBIS_LoadChunkIntoMemory( MBIS_Parser_t *mbisParser, long chunkSize )
{
    // Make the buffer bigger if needed.
    if( mbisParser->bufferSize < chunkSize )
    {
        SPMIDI_FREE_MEM( mbisParser->buffer );
        mbisParser->buffer = SPMIDI_ALLOC_MEM( chunkSize + 32, "MBIS_LoadChunkIntoMemory" );
        mbisParser->bufferSize = chunkSize + 32;
    }
    if( mbisParser->buffer != NULL )
    {
        return mbisParser->riffParser.stream->read( mbisParser->riffParser.stream, (char *) mbisParser->buffer, chunkSize );
    }
    else
    {
        return SPMIDI_Error_OutOfMemory;
    }
}

/**********************************************************************/
/** Load counts for various resources and allocate arrays to track them.
 */
static int MBIS_LoadNumResourcesChunk( MBIS_Parser_t *mbisParser )
{
    int numBytes;
    int err = 0;
    // Is it already loaded?
    if( mbisParser->tokenMaps == NULL )
    {
        mbisParser->numInstrumentMaps = RIFF_ReadIntBig( &mbisParser->riffParser );
        mbisParser->numWaveSets = RIFF_ReadIntBig( &mbisParser->riffParser );
        mbisParser->numWaveTables = RIFF_ReadIntBig( &mbisParser->riffParser );

        mbisParser->numTokenMaps = mbisParser->numWaveSets + mbisParser->numWaveTables;

        // Create an array large enough to hold the tokens we get from loading wave tables and wave sets.
        numBytes = mbisParser->numTokenMaps * sizeof( ResourceTokenMap_t );
        mbisParser->tokenMaps = SPMIDI_ALLOC_MEM( numBytes, "MBIS tokenMaps" );
        if( mbisParser->tokenMaps == NULL )
        {
            return SPMIDI_Error_OutOfMemory;
        }
        else
        {
            MemTools_Clear( mbisParser->tokenMaps, numBytes );
        }
        
        // Allocate maps for the instruments which use an index into an orchestra array.
        numBytes = mbisParser->numInstrumentMaps * sizeof( ResourceTokenMap_t );
        mbisParser->instrumentMaps = SPMIDI_ALLOC_MEM( numBytes, "MBIS instrumentMaps" );
        if( mbisParser->instrumentMaps == NULL )
        {
            return SPMIDI_Error_OutOfMemory;
        }
        else
        {
            MemTools_Clear( mbisParser->instrumentMaps, numBytes );
        }
        
        // Now that we know how many instruments we have, create an Orchestra.
        err = SPMIDI_CreateOrchestra( &mbisParser->spmidiOrchestra, mbisParser->numInstrumentMaps );
        if( err < 0 )
        {
            PRTMSGNUMD("ERROR: SPMIDI_CreateOrchestra returned = ", err );
        }

    }
    return err;
}

/**********************************************************************/
static int MBIS_LoadWaveTableChunk( MBIS_Parser_t *mbisParser, long chunkSize )
{
    int token;
    int result;
    int tableSize;

    int fileIndex = RIFF_ReadIntBig( &mbisParser->riffParser );

    // Bail if we don't need this wavetable.
    if( mbisParser->programList != NULL )
    {
        if( !mbisParser->tokenMaps[ fileIndex ].needed )
        {
            // DBUGMSGNUMD("MBIS_LoadWaveTableChunk: SKIP fileIndex = ", fileIndex );
            return 0;
        }
    }

    tableSize = chunkSize - 4;
    if( (fileIndex < 0) || (fileIndex >= mbisParser->numTokenMaps) )
    {
        return MBISParser_Error_ParseError;
    }

    result = MBIS_LoadChunkIntoMemory( mbisParser, tableSize );
    if( result < 0 ) return result;

    //DBUGMSGNUMD("MBIS_LoadWaveTableChunk: fileIndex = ", fileIndex );
    token = SPMIDI_LoadWaveTable( mbisParser->spmidiOrchestra, mbisParser->buffer, tableSize );
    //DBUGMSGNUMD("MBIS_LoadWaveTableChunk: token = ", token );
    if( token < 0 )
    {
        return token;
    }
    mbisParser->tokenMaps[ fileIndex ].token = token;
    return 0;
}

/**********************************************************************/
static int MBIS_LoadWaveSetChunk( MBIS_Parser_t *mbisParser, long chunkSize )
{
    int token;
    int result;
    int tableSize;

    int fileIndex = RIFF_ReadIntBig( &mbisParser->riffParser );
    
    // Bail if we don't need this wavetable.
    if( mbisParser->programList != NULL )
    {
        if( !mbisParser->tokenMaps[ fileIndex ].needed )
        {
            // DBUGMSGNUMD("MBIS_LoadWaveSetChunk: SKIP fileIndex = ", fileIndex );
            return 0;
        }
    }

    tableSize = chunkSize - 4;
    if( (fileIndex < 0) || (fileIndex >= mbisParser->numTokenMaps) )
    {
        return MBISParser_Error_ParseError;
    }

    result = MBIS_LoadChunkIntoMemory( mbisParser, tableSize );
    if( result < 0 ) return result;

    //DBUGMSGNUMD("MBIS_LoadWaveSetChunk: fileIndex = ", fileIndex );
    token = SPMIDI_LoadWaveSet( mbisParser->spmidiOrchestra, mbisParser->tokenMaps, mbisParser->buffer, tableSize );
    //DBUGMSGNUMD("MBIS_LoadWaveSetChunk: token = ", token );
    if( token < 0 )
    {
        return token;
    }
    mbisParser->tokenMaps[ fileIndex ].token = token;
    return 0;
}
/**********************************************************************/
/** Get instrument index from file. */
static int MBIS_LoadInstrumentChunk( MBIS_Parser_t *mbisParser, long chunkSize )
{
    int result;
    int insIndex = RIFF_ReadIntBig( &mbisParser->riffParser );
    int insSize = chunkSize - 4;
    
    // Bail if we don't need this wavetable.
    if( mbisParser->programList != NULL )
    {
        if( !mbisParser->instrumentMaps[ insIndex ].needed )
        {
            // DBUGMSGNUMD("MBIS_LoadInstrumentChunk: SKIP insIndex = ", insIndex );
            return 0;
        }
    }

    result = MBIS_LoadChunkIntoMemory( mbisParser, insSize );
    if( result < 0 ) return result;
    //DBUGMSGNUMD("MBIS_LoadInstrumentChunk: insIndex = ", insIndex );
    
    return SS_SetInstrumentDefinition( mbisParser->spmidiOrchestra, insIndex, mbisParser->tokenMaps, mbisParser->buffer, insSize );
}

/**********************************************************************/
static int MBIS_ParseBigInteger( spmUInt8 **pp )
{
    spmUInt8 *p = *pp;
    int result = *p++;
    result = (result <<8) | *p++;
    result = (result <<8) | *p++;
    result = (result <<8) | *p++;
    *pp = p;
    return result;
}

/**********************************************************************/
static int MBIS_LoadMelodicMapChunk( MBIS_Parser_t *mbisParser, long chunkSize )
{
    int i;
    int result = MBIS_LoadChunkIntoMemory( mbisParser, chunkSize );
    spmUInt8 *p = mbisParser->buffer;
    if( result < 0 )
    {
        return result;
    }
    else
    {
        int bankMSB = *p++;
        int bankLSB = *p++;
        int bankIndex = (bankMSB << 7) + bankLSB;
        int firstProgramIndex = *p++;
        int numPrograms = *p++;
        for( i=0; i<numPrograms; i++ )
        {
            int insIndex = MBIS_ParseBigInteger( &p );
            
            // Bail if we don't need this instrument.
            if( mbisParser->programList != NULL )
            {
                if( !mbisParser->instrumentMaps[ insIndex ].needed )
                {
                    continue;
                }
            }

            // DBUGMSGNUMD("MBIS_LoadMelodicMapChunk: insIndex = ", insIndex );
            result = SPMIDI_SetInstrumentMap( mbisParser->spmidiOrchestra, bankIndex, firstProgramIndex + i, insIndex );
            if( result < 0 )
            {
                return result;
            }
        }
    }
    return 0;
}
/**********************************************************************/
static int MBIS_LoadDrumMapChunk( MBIS_Parser_t *mbisParser, long chunkSize )
{
    int i;
    int result = MBIS_LoadChunkIntoMemory( mbisParser, chunkSize );
    spmUInt8 *p = mbisParser->buffer;
    if( result < 0 )
    {
        return result;
    }
    else
    {
        int bankMSB = *p++;
        int bankLSB = *p++;
        int bankIndex = (bankMSB << 7) + bankLSB;
        int programIndex = *p++;
        int firstNoteIndex = *p++;
        int numNotes = *p++;
        for( i=0; i<numNotes; i++ )
        {
            int insIndex = MBIS_ParseBigInteger( &p );
            int pitch = *p++;

            // Bail if we don't need this instrument.
            if( mbisParser->programList != NULL )
            {
                if( !mbisParser->instrumentMaps[ insIndex ].needed )
                {
                    continue;
                }
            }

            // DBUGMSGNUMD("MBIS_LoadDrumMapChunk: insIndex = ", insIndex );
            result = SPMIDI_SetDrumMap( mbisParser->spmidiOrchestra, bankIndex, programIndex, firstNoteIndex + i, 
                insIndex, pitch );
            if( result < 0 )
            {
                return result;
            }
        }
    }
    return 0;
}


/**********************************************************************/
/* Mark the instrument referenced by the Map as needed. */
static int MBIS_ScanMelodicMapChunk( MBIS_Parser_t *mbisParser, long chunkSize )
{
    int i;
    int result = MBIS_LoadChunkIntoMemory( mbisParser, chunkSize );
    spmUInt8 *p = mbisParser->buffer;
    if( result < 0 )
    {
        return result;
    }
    else
    {
        SPMIDI_ProgramList *programList = mbisParser->programList;
        int bankMSB = *p++;
        int bankLSB = *p++;
        int bankIndex = (bankMSB << 7) + bankLSB;
        int firstProgramIndex = *p++;
        int numPrograms = *p++;
        for( i=0; i<numPrograms; i++ )
        {
            int insIndex = MBIS_ParseBigInteger( &p );
            if( SPMIDI_IsProgramUsed( programList, bankIndex, firstProgramIndex + i ) )
            {
                mbisParser->instrumentMaps[ insIndex ].needed = 1;
            }
        }
    }
    return 0;
}

/**********************************************************************/
static int MBIS_ScanDrumMapChunk( MBIS_Parser_t *mbisParser, long chunkSize )
{
    int i;
    int result = MBIS_LoadChunkIntoMemory( mbisParser, chunkSize );
    spmUInt8 *p = mbisParser->buffer;
    if( result < 0 )
    {
        return result;
    }
    else
    {
        SPMIDI_ProgramList *programList = mbisParser->programList;
        int bankMSB = *p++;
        int bankLSB = *p++;
        int bankIndex = (bankMSB << 7) + bankLSB;
        int programIndex = *p++;
        int firstNoteIndex = *p++;
        int numNotes = *p++;
        for( i=0; i<numNotes; i++ )
        {
            int insIndex = MBIS_ParseBigInteger( &p );
            p++; // discard pitch
            if( SPMIDI_IsDrumUsed( programList, bankIndex, programIndex, firstNoteIndex + i ) )
            {
                mbisParser->instrumentMaps[ insIndex ].needed = 1;
            }
        }
    }
    return 0;
}

/**********************************************************************/
/** @return Number of dependencies. */
static spmSInt32 MBIS_LoadDependencies( MBIS_Parser_t *mbisParser, long chunkSize, MBIS_Dependency_t **dependencyPtr )
{
    MBIS_Dependency_t *dependencies = NULL;
    int result = MBIS_LoadChunkIntoMemory( mbisParser, chunkSize );
    int numPairs = 0;
    spmUInt8 *p = mbisParser->buffer; // buffer now contains chunk
    if( result < 0 )
    {
        return result;
    }
    else
    {
        int numBytes;
        numPairs = chunkSize / 8;
        numBytes = numPairs * sizeof( MBIS_Dependency_t );

        dependencies = SPMIDI_ALLOC_MEM( numBytes, "MBIS_LoadDependencies" );
        if( dependencies != NULL )
        {
            int i;
            for( i=0; i<numPairs; i++ )
            {
                dependencies[i].parentIndex = MBIS_ParseBigInteger( &p );
                dependencies[i].childIndex = MBIS_ParseBigInteger( &p );
            }
        }
    }
    *dependencyPtr = dependencies;
    return numPairs;
}


/**********************************************************************/
static int MBIS_ScanningChunkHandler( MBIS_Parser_t *mbisParser, RIFF_ChunkID chunkID, long chunkSize )
{
    int result = 0;

    // result = PrintChunkInfo( mbisParser, chunkID, chunkSize, "Chunk" );

    /* If the abort flag is set, skip the chunk. */
    if( !( mbisParser->abortList) )
    {
        mbisParser->level += 1;
        switch( chunkID )
        {
        case FOURCC_NRSC:
            result = MBIS_LoadNumResourcesChunk( mbisParser );
            break;
        case FOURCC_MMAP:
            result = MBIS_ScanMelodicMapChunk( mbisParser, chunkSize );
            break;
        case FOURCC_DMAP:
            result = MBIS_ScanDrumMapChunk( mbisParser, chunkSize );
            break;
        case FOURCC_IDEP:
            mbisParser->numInstrumentDependencies = MBIS_LoadDependencies( mbisParser, chunkSize, &mbisParser->instrumentDependencies );
            if( mbisParser->numInstrumentDependencies < 0 )
            {
                result = mbisParser->numInstrumentDependencies;
            }
            break;
        case FOURCC_WDEP:
            mbisParser->numWaveSetDependencies = MBIS_LoadDependencies( mbisParser, chunkSize, &mbisParser->waveSetDependencies  );
            if( mbisParser->numWaveSetDependencies < 0 )
            {
                result = mbisParser->numWaveSetDependencies;
            }
            break;

        default:
            // PRTLN_HVALUE("UNRECOGNIZED = ", chunkID );
            break;
        }
        mbisParser->level -= 1;
    }

    return result;
}

/**********************************************************************/
static int MBIS_LoadingChunkHandler( MBIS_Parser_t *mbisParser, RIFF_ChunkID chunkID, long chunkSize )
{
    int result = 0;

    // result = PrintChunkInfo( mbisParser, chunkID, chunkSize, "Chunk" );

    /* if the abort flag is set, skip the chunk */
    if( !( mbisParser->abortList) )
    {
        mbisParser->level += 1;
        switch( chunkID )
        {
        case FOURCC_NRSC:
            result = MBIS_LoadNumResourcesChunk( mbisParser );
            break;
        case FOURCC_WTBL:
            result = MBIS_LoadWaveTableChunk( mbisParser, chunkSize );
            break;
        case FOURCC_WVST:
            result = MBIS_LoadWaveSetChunk( mbisParser, chunkSize );
            break;
        case FOURCC_INST:
            result = MBIS_LoadInstrumentChunk( mbisParser, chunkSize );
            break;
        case FOURCC_MMAP:
            result = MBIS_LoadMelodicMapChunk( mbisParser, chunkSize );
            break;
        case FOURCC_DMAP:
            result = MBIS_LoadDrumMapChunk( mbisParser, chunkSize );
            break;

        default:
            // PRTLN_HVALUE("UNRECOGNIZED = ", chunkID );
            break;
        }
        mbisParser->level -= 1;
    }

    return result;
}

/**********************************************************************/
static int MBIS_ChunkHandler( void *userData, RIFF_ChunkID chunkID, long chunkSize )
{
    MBIS_Parser_t *mbisParser = (MBIS_Parser_t *) userData;
    int result = 0;
    switch( mbisParser->mode )
    {
    case MBIS_ModeScanning:
        result = MBIS_ScanningChunkHandler( mbisParser, chunkID, chunkSize );
        break;
    case MBIS_ModeLoading:
        result = MBIS_LoadingChunkHandler( mbisParser, chunkID, chunkSize );
        break;
    default:
        break;
    }
    return result;
}

/**********************************************************************/
static int MBIS_BeginFormHandler( void *userData, RIFF_ChunkID formType, long chunkSize )
{
    MBIS_Parser_t *mbisParser = (MBIS_Parser_t *) userData;
    // PrintChunkInfo( mbisParser, formType, chunkSize, "FORM" );
    (void) chunkSize;
    if( formType != FOURCC_MBIS )
    {
        return MBISParser_Error_NotMBIS;
    }
    mbisParser->level += 1;
    return 0;
}
/**********************************************************************/
static int MBIS_EndFormHandler( void *userData, RIFF_ChunkID formType, long chunkSize )
{
    MBIS_Parser_t *mbisParser = (MBIS_Parser_t *) userData;

    TOUCH(chunkSize);
    TOUCH(formType);
    mbisParser->level -= 1;

    return 0;
}

/**********************************************************************/
static int MBIS_BeginListHandler( void *userData, RIFF_ChunkID listType, long chunkSize )
{
    int result = 0;
    MBIS_Parser_t *mbisParser = (MBIS_Parser_t *) userData;
    // PrintChunkInfo( mbisParser, listType, chunkSize, "LIST" );
    (void) chunkSize;
    mbisParser->level += 1;
    switch( listType )
    {
    case FOURCC_INSS:
        break;
    case FOURCC_MAPS:
        break;
    default:
        break;
    }
    return result;
}

/**********************************************************************/
static int MBIS_EndListHandler( void *userData, RIFF_ChunkID listType, long chunkSize )
{
    MBIS_Parser_t *mbisParser = (MBIS_Parser_t *) userData;

    TOUCH(chunkSize);
    mbisParser->level -= 1;
    switch( listType )
    {
    default:
        break;
    }
    return 0;
}


/**********************************************************************/
/** Create a parser context for MBIS.
 */
SPMIDI_Error MBISParser_Create( MBISParser **parserPtr, StreamIO *sio )
{
    RiffParser_t *riffParser = NULL;
    MBIS_Parser_t *mbisParser = NULL;
    MBIS_Orchestra_t *mbisOrch = NULL;

    mbisParser = (MBIS_Parser_t *) SPMIDI_ALLOC_MEM( sizeof(MBIS_Parser_t), "MBIS_Parser_t" ) ;
    if( mbisParser == NULL )
    {
        goto nomem;
    }
    MemTools_Clear( mbisParser, sizeof(MBIS_Parser_t) );
    mbisOrch = &mbisParser->mbisOrchestra;
    riffParser = &mbisParser->riffParser;

    DLL_InitList( &mbisOrch->waves );

    riffParser->stream = sio;
    riffParser->userData = mbisParser;

    riffParser->handleChunk = MBIS_ChunkHandler;
    riffParser->handleBeginForm = MBIS_BeginFormHandler;
    riffParser->handleEndForm = MBIS_EndFormHandler;
    riffParser->handleBeginList = MBIS_BeginListHandler;
    riffParser->handleEndList = MBIS_EndListHandler;

    *parserPtr = (MBISParser *)mbisParser;

    return SPMIDI_Error_None;

nomem:
    SPMIDI_FreeMemory( mbisParser );
    return SPMIDI_Error_OutOfMemory;
}

/*********************************************************/
/* Scan dependencies and if the parent is needed then mark the child as also being needed. */
SPMIDI_Error MBIS_MarkNeededDependents( ResourceTokenMap_t *parentMaps, spmSInt32 numParentMaps,
            MBIS_Dependency_t *dependencies, spmSInt32 numDependencies,
            ResourceTokenMap_t *childMaps, spmSInt32 numChildMaps )
{
    int dependencyIndex;

    for( dependencyIndex = 0; dependencyIndex < numDependencies; dependencyIndex++ )
    {
        spmSInt32 parentIndex = dependencies[ dependencyIndex ].parentIndex;
        spmSInt32 childIndex = dependencies[ dependencyIndex ].childIndex;
        if( parentIndex >= numParentMaps )
        {
            return MBISParser_Error_ParseError;
        }
        if( childIndex >= numChildMaps )
        {
            return MBISParser_Error_ParseError;
        }
        if( parentMaps[ parentIndex ].needed )
        {
            childMaps[ childIndex ].needed = 1;
        }
    }
    return 0;
}

/*********************************************************/
/**
 * Read MBIS instruments and wavetables from image in Stream,
 * which may be a file or an in-memory image.
 */
SPMIDI_Error MBISParser_Load( MBISParser *parser )
{
    int result;
    RiffParser_t *riffParser = NULL;
    MBIS_Parser_t *mbisParser = NULL;

    mbisParser = (MBIS_Parser_t *)parser;
    if( mbisParser == NULL )
    {
        return SPMIDI_Error_IllegalArgument;
    }

    riffParser = &mbisParser->riffParser;

    if( mbisParser->programList != NULL )
    {
        // Make an initial pass to gather dependencies.
        int originalPosition = riffParser->stream->getPosition(riffParser->stream);
        mbisParser->mode = MBIS_ModeScanning;
        result = RIFF_ParseStream( riffParser );
        if( result < 0 )
        {
            goto error;
        }
        // Rewind for second pass.
        riffParser->stream->setPosition(riffParser->stream, originalPosition );
        
        //Scan Instruments and mark WaveSets needed.
        result = MBIS_MarkNeededDependents( mbisParser->instrumentMaps, mbisParser->numInstrumentMaps,
            mbisParser->instrumentDependencies, mbisParser->numInstrumentDependencies,
            mbisParser->tokenMaps, mbisParser->numTokenMaps );

        //Scan WaveSets and mark WaveTables needed.
        result = MBIS_MarkNeededDependents( mbisParser->tokenMaps, mbisParser->numTokenMaps,
            mbisParser->waveSetDependencies, mbisParser->numWaveSetDependencies,
            mbisParser->tokenMaps, mbisParser->numTokenMaps );

    }
    
    mbisParser->mode = MBIS_ModeLoading;
    result = RIFF_ParseStream( riffParser );
    if( result < 0 )
    {
        goto error;
    }

    return 0;

error:
    return result;

}

/*********************************************************/
/**
 * Unload MBIS instruments from SPMIDI synthesizer.
 */
SPMIDI_Error MBISParser_Unload( MBISParser *parser, SPMIDI_Context *spmidiContext )
{
    /* Unsupported. Once a MBIS instrument is loaded it cannot be unloaded. 
     * Just delete the SPMIDI_Context and create a new one.
     */
    TOUCH(parser);
    TOUCH(spmidiContext);
    return 0;
}

/*********************************************************/
void MBISParser_Delete( MBISParser *parser )
{
    MBIS_Parser_t *mbisParser = (MBIS_Parser_t *)parser;
    if( mbisParser == NULL )
    {
        return;
    }

    if( mbisParser->buffer != NULL )
    {
        SPMIDI_FREE_MEM( mbisParser->buffer );
    }
    if( mbisParser->tokenMaps != NULL )
    {
        SPMIDI_FREE_MEM( mbisParser->tokenMaps );
    }
    if( mbisParser->instrumentMaps != NULL )
    {
        SPMIDI_FREE_MEM( mbisParser->instrumentMaps );
    }
    if( mbisParser->instrumentDependencies != NULL )
    {
        SPMIDI_FREE_MEM( mbisParser->instrumentDependencies );
    }
    if( mbisParser->waveSetDependencies != NULL )
    {
        SPMIDI_FREE_MEM( mbisParser->waveSetDependencies );
    }
    SPMIDI_FreeMemory( mbisParser );

    return;
}

/*******************************************************************/
SPMIDI_Error SPMIDI_LoadOrchestra( StreamIO *instrumentStream, SPMIDI_ProgramList *programList, SPMIDI_Orchestra **orchestraPtr )
{
    SPMIDI_Error err;
    MBISParser *mbisParser = NULL;
    MBIS_Parser_t *mbisParser_t = NULL;
    (void) programList;
    err = MBISParser_Create( &mbisParser, instrumentStream );
    if( err < 0 )
    {
        PRTMSGNUMD("ERROR: MBISParser_Create returned = ", err );
        return err;
    }
    mbisParser_t = (MBIS_Parser_t *) mbisParser;
    mbisParser_t->programList = programList;
    
    // Load the instruments.
    err = MBISParser_Load( mbisParser );
    if( err < 0 )
    {
        PRTMSGNUMD("ERROR: MBISParser_Parse returned = ", err );
        goto error;
    }

error:
    *orchestraPtr = mbisParser_t->spmidiOrchestra;
    MBISParser_Delete( mbisParser );
    return err;
}

#if 0

#include <malloc.h>
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_play.h"

/*******************************************************************/
int main(int argc, char* argv[]);
int main(int argc, char* argv[])
{
    char* fileName;
    unsigned char* fileStart = NULL;
    spmSInt32 fileSize;
    StreamIO *sio = NULL;
    SPMIDI_Context *spmidiContext = NULL;
    SPMIDI_Orchestra *orchestra = NULL;
    const int kSampleRate = 22050;
    const int kChannel = 1;
    SPMIDI_Error err;

    char *DEFAULT_FILENAME = "D:\\mobileer_work\\Trivial\\export\\exported.mbis";

    SPMIDI_Initialize();
        
    fileName = ( argc < 2 ) ? DEFAULT_FILENAME : argv[1];

    // Start synthesizer and audio engine.
    err = SPMUtil_Start( &spmidiContext, kSampleRate, NULL, SPMUTIL_OUTPUT_STEREO );
    if( err < 0 )
    {
        PRTMSGNUMD( "SPMUtil_Start returned %d\n", err );
    }

    // Load file into a memory stream and parse it.
    fileStart = SPMUtil_LoadFileImage( fileName, (int *)&( fileSize ) );
    if( fileStart != NULL )
    {
        int i;

        sio = Stream_OpenImage( (char *)fileStart, fileSize );
        if( sio == NULL )
        {
            goto error;
        }

        err = SPMIDI_LoadOrchestra( sio, NULL, &orchestra );
        if( err < 0 )
        {
            PRTMSGNUMD( "SPMIDI_LoadOrchestra returned ", err );
            goto error;
        }

        SPMUtil_ControlChange( spmidiContext, kChannel, MIDI_CONTROL_BANK, 0 );
        
        /* Note On */
        for( i = 0; i<=10; i++ )
        {
            SPMUtil_ProgramChange( spmidiContext, kChannel, i );
            printf("Program = %d\n", i );
            SPMUtil_NoteOn( spmidiContext, kChannel, 60, 64 );
            SPMUtil_PlayMilliseconds( spmidiContext, 500 );

            /* Note Off */
            SPMUtil_NoteOff( spmidiContext, kChannel, 60, 0 );
            SPMUtil_PlayMilliseconds( spmidiContext, 200 );
        }
    }
    else
    {
        DBUGMSG("Error: can't open file ");
        DBUGMSG( fileName );
        err = -1;
    }


error:
    /* close the associated stream */
    if( sio != NULL )
    {
        Stream_Close( sio );
    }
    free( fileStart );

    SPMUtil_Stop(spmidiContext);
    SPMIDI_DeleteOrchestra( orchestra );


    SPMIDI_Terminate();

    PRTMSGNUMD("returning ", err );
    return err;
}
#endif

#endif /* SPMIDI_SUPPORT_LOADING */
