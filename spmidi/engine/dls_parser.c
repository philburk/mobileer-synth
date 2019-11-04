/*
 * $Id: dls_parser.c,v 1.60 2007/10/02 16:14:42 philjmsl Exp $
 * Load a DLS file.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include "spmidi/include/streamio.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/engine/spmidi_dls.h"
#include "spmidi/engine/parse_riff.h"
#include "spmidi/engine/stack.h"
#include "spmidi/include/dls_parser.h"
#include "dls_parser_internal.h"

#include "spmidi/engine/memtools.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/engine/instrument_mgr.h"

/* Only compile if supporting ME3000 API */
#if SPMIDI_ME3000

#define DLS_TRACK_ART_ALLOCS  (0)
#if DLS_TRACK_ART_ALLOCS
static spmSInt sNumRegions = 0;
static spmSInt sNumInstruments = 0;
static spmSInt sNumArtConnTotal = 0;
static spmSInt sNumArtART1 = 0;
static spmSInt sNumArtART2 = 0;
static spmSInt sNumArtAllocs = 0;
static spmSInt sNumArtConnUsed = 0;
static spmSInt sNumArtBytes = 0;
#endif

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

#ifndef DLS_VERBOSITY
#define DLS_VERBOSITY (0)
#endif

#ifndef DLS_PRINT_INSTRUMENTS
#define DLS_PRINT_INSTRUMENTS (0)
#endif

#if  DLS_VERBOSITY
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#define STATE_IDLE           (0)
#define STATE_PARSING_LINS   (1)
#define STATE_PARSING_LRGN   (2)
#define STATE_PARSING_WAVE   (3)

typedef struct DLS_Parser_s
{
    RiffParser_t        riffParser;
    DLS_Orchestra_t     orchestra;
    Stack_t             cdlStack;
    StreamIO           *sio;
    int                 level;
    DLS_Instrument_t   *currentInstrument;
    int                 nextInsIndex;
    int                 nextRegionIndex;
    DLS_Region_t       *currentRegion;
    DLS_Wave_t         *currentWave;
    /* Offset within file. */
    spmUInt32           wavePoolBase;
    spmBoolean          abortList;
    spmBoolean          artIsLocal;
    spmSInt32           sampleRate;
}
DLS_Parser_t;

typedef struct DLSID_Query_s
{
    GUID_t              guid;
    int                 result;
} DLSID_Query_t;

#define DLSID_NUM_QUERIES 11

const DLSID_Query_t sDLSID_Queries[DLSID_NUM_QUERIES] =
{
    { DLSID_SampleMemorySize, 0 },
    { DLSID_SamplePlaybackRate, 0 }, /* these are resolved at runtime */
    { DLSID_SupportsMobileDLS, TRUE },
    { DLSID_SupportsMobileDLSOptionalBlocks, TRUE },
    { DLSID_SupportsDLS1, TRUE },
    { DLSID_SupportsDLS2, TRUE },
    { DLSID_ManufacturersID, DLS_MANUFACTURERS_ID },
    { DLSID_ProductID, DLS_PRODUCT_ID },
    { DLSID_GMInHardware, TRUE },
    { DLSID_GSInHardware, FALSE },
    { DLSID_XGInHardware, FALSE }
};

#define DLSID_SAMPLEMEM_SLOT          (0)
#define DLSID_SAMPLEPLAYBACKRATE_SLOT (1)

static spmBoolean gNeedIndent = 0;

static const DLS_WaveSample_t sDefaultWaveSample =
    {
        60, /* basePitch */
        0, /* loopType */
        0, /* fineTune */
        0, /* gain */
        -1, /* loopStart */
        0 /* loopSize */
    };

/* Use macros in case we want to change how we get the data. */
#define DLS_READ_SHORT_LITTLE  RIFF_ReadShortLittle( &dlsParser->riffParser )
#define DLS_READ_INT_LITTLE  RIFF_ReadIntLittle( &dlsParser->riffParser )
#define DLS_Tell( dlsParser ) \
    ( dlsParser->sio->getPosition( dlsParser->sio ) )

#define TOUCH(value) \
    (void)value

#if DLS_VERBOSITY

#define PRT_VALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( dlsParser ); } \
        DBUGMSG(format); \
        DBUGNUMD(value); \
    }
#define PRT_HVALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( dlsParser ); } \
        DBUGMSG(format); \
        DBUGNUMH(value); \
    }
#define PRTLN_VALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( dlsParser ); } \
        DBUGMSG(format); \
        DBUGNUMD(value); \
        DBUGMSG("\n"); \
        gNeedIndent = 1; \
    }
#define PRTLN_HVALUE(format, value) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( dlsParser ); } \
        DBUGMSG(format); \
        DBUGNUMH(value); \
        DBUGMSG("\n"); \
        gNeedIndent = 1; \
    }
#define PRTLN_TEXT(text) \
    { \
        if( gNeedIndent ) { PrintChunk_Indent( dlsParser ); } \
        DBUGMSG(text); \
        DBUGMSG("\n"); \
        gNeedIndent = 1; \
    }

static void PrintChunk_Indent( DLS_Parser_t *dlsParser )
{
    int i;
    for( i=0; i<dlsParser->level; i++ )
        DBUGMSG("    ");
    gNeedIndent = 0;
}
#else /* DLS_VERBOSITY */
#define PRT_VALUE(format, value) TOUCH(value)
#define PRT_HVALUE(format, value) TOUCH(value)
#define PRTLN_VALUE(format, value) TOUCH(value)
#define PRTLN_HVALUE(format, value) TOUCH(value)
#define PRTLN_TEXT(text) TOUCH(text)
#define PrintChunk_Indent( dlsParser )
#endif /* DLS_VERBOSITY */

typedef struct DLS_ArticulationMap_s
{

    spmUInt16           source;
    spmUInt16           control;
    spmUInt16           destination;
    spmUInt16           token;
}
DLS_ArticulationMap_t;

static const DLS_ArticulationMap_t sArticulationTokenMap[] = 
{
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_PITCH, CONN_Z_TUNING },
    { CONN_SRC_VIBRATO, CONN_SRC_NONE, CONN_DST_PITCH, CONN_Z_VIB_LFO_TO_PITCH },
    { CONN_SRC_LFO, CONN_SRC_NONE, CONN_DST_PITCH, CONN_Z_MOD_LFO_TO_PITCH },
    { CONN_SRC_EG2, CONN_SRC_NONE, CONN_DST_PITCH, CONN_Z_EG2_TO_PITCH },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_LFO_FREQUENCY, CONN_Z_LFO_FREQUENCY },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_VIB_FREQUENCY, CONN_Z_VIB_FREQUENCY },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_LFO_STARTDELAY, CONN_Z_LFO_STARTDELAY },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_VIB_STARTDELAY, CONN_Z_VIB_STARTDELAY },

    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_DELAYTIME, CONN_Z_EG1_DELAYTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_ATTACKTIME, CONN_Z_EG1_ATTACKTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_HOLDTIME, CONN_Z_EG1_HOLDTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_DECAYTIME, CONN_Z_EG1_DECAYTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_RELEASETIME, CONN_Z_EG1_RELEASETIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_SUSTAINLEVEL, CONN_Z_EG1_SUSTAINLEVEL },

    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_DELAYTIME, CONN_Z_EG2_DELAYTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_ATTACKTIME, CONN_Z_EG2_ATTACKTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_HOLDTIME, CONN_Z_EG2_HOLDTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_DECAYTIME, CONN_Z_EG2_DECAYTIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_RELEASETIME, CONN_Z_EG2_RELEASETIME },
    { CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_SUSTAINLEVEL, CONN_Z_EG2_SUSTAINLEVEL },

    { CONN_SRC_KEYNUMBER, CONN_SRC_NONE,          CONN_DST_PITCH, CONN_Z_KEY_TO_PITCH },
    { CONN_SRC_VIBRATO, CONN_SRC_CC1,             CONN_DST_PITCH, CONN_Z_VIB_LFO_CC1_TO_PITCH },
    { CONN_SRC_LFO,     CONN_SRC_CC1,             CONN_DST_PITCH, CONN_Z_MOD_LFO_CC1_TO_PITCH },
    { CONN_SRC_VIBRATO, CONN_SRC_CHANNELPRESSURE, CONN_DST_PITCH, CONN_Z_VIB_LFO_CPR_TO_PITCH },
    { CONN_SRC_LFO,     CONN_SRC_CHANNELPRESSURE, CONN_DST_PITCH, CONN_Z_MOD_LFO_CPR_TO_PITCH }
    
};

/* Function table for patching DLS parser. */
static DLSParser_FunctionTable_t sDLSParserFunctionTable = { 0 };

#define ART_MAP_SIZE  (sizeof(sArticulationTokenMap)/sizeof(DLS_ArticulationMap_t))

/*********** static prototypes ****************************************/
static int DLSParser_ResolveInstrument( DLS_Orchestra_t *orchestra, int insIndex);
static int DLSParser_ResolveRegion( DLS_Wave_t **poolTable,
                                    DLS_Region_t *region);

#if SPMIDI_LEAVE_DLS_WAVES_IN_IMAGE
/**********************************************************************/
/** Figure out whether the Host is LittleEndian by fetching the addressed byte of a long.
 * @return 1 if CPU is Little Endian
 */
static int IsHostLittleEndian( void )
{
    spmSInt32 pad = 1;
    return ((char *)(&pad))[0];
}

/**********************************************************************/
static char *GetCurrentStreamAddress( StreamIO *sio )
{
    int offset = Stream_GetPosition( sio );
    char *image = Stream_GetAddress( sio );
    if( image == NULL ) return NULL;
    return ( image + offset );
}

#endif


/*******************************************************************************/
/********** Vectorred Functions ************************************************/
/*******************************************************************************/
SPMIDI_Error DLSParser_Create( DLSParser **parserPtr, unsigned char *fileStart, spmSInt32 fileSize )
{
    return DLSParser_GetFunctionTable()->create( parserPtr, fileStart, fileSize );
}
/*******************************************************************************/
void DLSParser_Delete( DLSParser *parser )
{
    DLSParser_GetFunctionTable()->delete( parser );
}
/*******************************************************************************/
SPMIDI_Error DLSParser_Parse( DLSParser *parser )
{
    return DLSParser_GetFunctionTable()->parse( parser );
}
/*******************************************************************************/
SPMIDI_Error DLSParser_Load( DLSParser *parser, SPMIDI_Context *spmidiContext )
{
    return DLSParser_GetFunctionTable()->load( parser, spmidiContext );
}
/*******************************************************************************/
static int DLSParser_ResolveRegion( DLS_Wave_t **poolTable,
                                    DLS_Region_t *region)
{
    return DLSParser_GetFunctionTable()->resolveRegion( poolTable, region );
}
/*******************************************************************************/
static void DeleteInstruments( DLS_Orchestra_t *orchestra )
{
    DLSParser_GetFunctionTable()->deleteInstruments( orchestra );
}
/*******************************************************************************/
static void DeleteRegion( DLS_Region_t *region)
{
    DLSParser_GetFunctionTable()->deleteRegion( region );
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
static void *CreateWaveTable( spmUInt32 numBytes )
{
    void *waveTable;

    waveTable = SPMIDI_ALLOC_MEM( numBytes, "wavetable" );
    if( waveTable != NULL )
    {
        MemTools_Clear( waveTable, numBytes );
    }

    return waveTable;
}

/**********************************************************************/
static void DeleteWaveTable ( spmSInt16 *waveTable )
{
    SPMIDI_FreeMemory( waveTable );
}

/**********************************************************************/
static DLS_WaveSample_t * CreateWaveSample( void )
{
    DLS_WaveSample_t *waveSample;

    waveSample = (DLS_WaveSample_t *)SPMIDI_ALLOC_MEM( sizeof( DLS_WaveSample_t ), "DLS_WaveSample_t" );
    if( waveSample != NULL )
    {
        MemTools_Clear( waveSample, sizeof( DLS_WaveSample_t ) );
    }

    return waveSample;
}

/**********************************************************************/
static void DeleteWaveSample ( DLS_WaveSample_t *waveSample )
{
    SPMIDI_FreeMemory( waveSample );
}

/**********************************************************************/
static DLS_Wave_t* CreateWave( DLS_Parser_t *dlsParser )
{
    DLS_Orchestra_t  *orchestra = &dlsParser->orchestra;
    DLS_Wave_t *dlsWave;

    dlsWave = SPMIDI_ALLOC_MEM( sizeof(DLS_Wave_t), "DLS_Wave_t" );
    if( dlsWave != NULL )
    {
        MemTools_Clear( dlsWave, sizeof(DLS_Wave_t) );
        DLL_InitNode( &dlsWave->node );
        DLL_AddTail( &orchestra->waves, &dlsWave->node );

        dlsParser->currentWave = dlsWave;
    }

    return dlsWave;
}

/**********************************************************************/
static void DeleteWave ( DLS_Wave_t *dlsWave )
{
    if( dlsWave != NULL )
    {
        DeleteWaveSample( dlsWave->wavesample );
        if( dlsWave->isAllocated )
        {
            DeleteWaveTable( dlsWave->samples );
        }
        SPMIDI_FreeMemory( dlsWave );
    }
}

/**********************************************************************/
static int FindArticulationToken( spmUInt16 source, spmUInt16 control, spmUInt16 destination )
{
    int i;
    spmUInt16 token;

    /* Find token in map of legal articulations. */
    token = CONN_Z_UNRECOGNIZED;
    for( i=0; i<ART_MAP_SIZE; i++ )
    {
        const DLS_ArticulationMap_t *artMap = &sArticulationTokenMap[i];
        if( (destination == artMap->destination ) &&
            (source == artMap->source ) &&
            (control == artMap->control )
          )
        {
            token = artMap->token;
            break;
        }
    }

    return token;
}

/**********************************************************************/
static int SetArticulationFlag( DLS_Parser_t *dlsParser )
{
    int result;

    /* if we're doing a region, articulations are local to region
     * otherwise, articulations are global to instrument
     * if neither of these, we've got an error in parsing */

    switch( dlsParser->riffParser.listState )
    {
    case STATE_PARSING_LRGN:
        if( dlsParser->currentRegion == NULL )
        {
            result = DLSParser_Error_ParseError;  /* should be a current region, if we're in a region list */
        }
        else
        {
            dlsParser->artIsLocal = TRUE;
            result = 0;
        }
        break;
    case STATE_PARSING_LINS:
        if( dlsParser->currentInstrument == NULL )
        {
            result = DLSParser_Error_ParseError;  /* should be a current instrument, if we're in an instrument list */
        }
        else
        {
            dlsParser->artIsLocal = FALSE;
            result = 0;
        }
        break;
    default:
        result = DLSParser_Error_ParseError;
        break;
    }

    return result;
}

/**********************************************************************/
static void DeleteArticulationArray( DLS_ArticulationTracker_t *artTracker )
{

    if( artTracker->articulations != NULL )
    {
        SPMIDI_FreeMemory( artTracker->articulations );
        artTracker->articulations = NULL;
        artTracker->numArticulations = 0;
    }
}

/**********************************************************************/
static void DeleteRegion_Internal( DLS_Region_t *region)
{
    if( region != NULL )
    {
        DeleteWaveSample( region->waveSample );

        DeleteArticulationArray( &(region->articulations) );
    }
}

/**********************************************************************/
static void DeleteInstruments_Internal( DLS_Orchestra_t *orchestra )
{
    int i, j;
    DLS_Instrument_t *ins;

    if( orchestra != NULL )
    {
        if( orchestra->instruments != NULL )
        {
            /* delete each instrument in table */
            for( i=0; i<orchestra->numInstruments; i++ )
            {
                ins = &orchestra->instruments[ i ];
                if( ins != NULL )
                {
                    /* delete the regions */
                    if( ins->regions != NULL )
                    {
                        for( j=0; j<ins->numRegions; j++ )
                        {
                            DeleteRegion( &ins->regions[j] );
                        }
                        SPMIDI_FreeMemory( ins->regions );
                    }

                    /* delete any global articulations */
                    DeleteArticulationArray( &(ins->articulations) );
                }
            }
        }
    }
}

/**********************************************************************/
/**
 * Collection Header tells us how many instruments will be loaded.
 * Preallocate memory for an array of instruments.
 * NOTE - we are expecting this chunk before the LINS list.
 */
static int LoadChunk_COLH( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Orchestra_t  *orchestra = &dlsParser->orchestra;

    TOUCH(chunkSize);

    orchestra->numInstruments = DLS_READ_INT_LITTLE;
    PRTLN_VALUE("Number of instruments = ", orchestra->numInstruments );

    orchestra->instruments = SPMIDI_ALLOC_MEM( orchestra->numInstruments * sizeof(DLS_Instrument_t), "DLS_Instrument_t" );
    if( orchestra->instruments == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
    MemTools_Clear( orchestra->instruments, orchestra->numInstruments * sizeof(DLS_Instrument_t) );
    dlsParser->nextInsIndex = 0;

    return 0;
}

/**********************************************************************/
static int ReadGUID( DLS_Parser_t *dlsParser, GUID_t* guid )
{
    int numRead;

    guid->Data1 = DLS_READ_INT_LITTLE;
    guid->Data2 = (spmUInt16)DLS_READ_SHORT_LITTLE;
    guid->Data3 = (spmUInt16)DLS_READ_SHORT_LITTLE;

    numRead = dlsParser->sio->read( dlsParser->sio, (char *) guid->Data4, 8 );
    
    return ( numRead == 8 ? 0 : DLSParser_Error_ParseError );
}

/**********************************************************************
 * GUIDIsEqual compares parts of a GUID_t structure and returns TRUE
 * if all parts are the same, or FALSE if not
 */
static spmBoolean GUIDIsEqual( GUID_t* guid1, const GUID_t* guid2 )
{
    spmBoolean result;
    int i;

    if( ( guid1->Data1 == guid2->Data1 ) && \
        ( guid1->Data2 == guid2->Data2 ) && \
        ( guid1->Data3 == guid2->Data3 ) )
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    /* test Data4 portion, if we need to */
    if( result == TRUE )
    {
        for ( i=0; i<8; i++ )
        {
            if ( guid1->Data4[i] != guid2->Data4[i] )
            {
                result = FALSE;
                break;
            }
        }
    }

    return( result );
}

/**********************************************************************
 * CDLQuery evaluates a CDL DLSID and returns 0 if found,
 * or a (negative) error if the query is not supported.  If found, result
 * is set to the result of the query.
 */
static int CDLQuery( DLS_Parser_t *dlsParser, GUID_t* guid, int *result )
{
    int i;
    int queryIndex = -1;

    /* look up GUID in DLSID table */
    for( i = 0; i < DLSID_NUM_QUERIES; i++ )
    {
        if( GUIDIsEqual( guid, &( sDLSID_Queries[i].guid ) ) )
        {
            queryIndex = i;
            break;
        }
    }
    /* Bail if we don't match any Queries. */
    if( queryIndex < 0 ) return -1;

    switch( queryIndex )
    {
    case DLSID_SAMPLEMEM_SLOT:
        /* This allows one to substitute a function that returns available memory. */
        *result = SPMIDI_MAX_SAMPLEMEM;
        break;

    case DLSID_SAMPLEPLAYBACKRATE_SLOT:
        *result = DLSParser_GetSampleRate( (DLSParser*)dlsParser );
        break;

    default:
        *result = sDLSID_Queries[i].result;
        break;
    }

    return( 0 );
}

/**********************************************************************
 * CDLEvaluate evaluates a sequence of CDL operations from a CDL chunk
 * using a stack, and returns TRUE if the result is true, FALSE if false,
 * or an error value if there's an error.
 */
static int CDLEvaluate( DLS_Parser_t *dlsParser, long chunkSize )
{
    Stack_t *cdlStack = &( dlsParser->cdlStack );
    long bytesLeft = chunkSize;
    spmUInt16 opcode;
    int err = 0, result = TRUE;
    int query;
    GUID_t dlsid;

    /* All CDL data should be short or int or GUID, so there should never be an odd number
     * of bytes left.
     */
    if( bytesLeft > 1 )
    {
        /* clear stack */
        Stack_Clear( cdlStack );

        /* while we're not at the end of the chunk */
        while( bytesLeft > 1 )
        {
            /* read opcode */
            opcode = (spmUInt16)DLS_READ_SHORT_LITTLE;
            bytesLeft -= sizeof(spmUInt16);

            /* process opcode */
            switch( opcode )
            {
            case DLS_CDL_AND:
                Stack_Push( cdlStack, Stack_Pop( cdlStack ) & Stack_Pop( cdlStack ) );
                break;

            case DLS_CDL_OR:
                Stack_Push( cdlStack, Stack_Pop( cdlStack ) | Stack_Pop( cdlStack ) );
                break;

            case DLS_CDL_XOR:
                Stack_Push( cdlStack, Stack_Pop( cdlStack ) ^ Stack_Pop( cdlStack ) );
                break;

            case DLS_CDL_ADD:
                Stack_Push( cdlStack, Stack_Pop( cdlStack ) + Stack_Pop( cdlStack ) );
                break;

            case DLS_CDL_SUBTRACT:
                Stack_Push( cdlStack, Stack_Pop( cdlStack ) - Stack_Pop( cdlStack ) );
                break;

            case DLS_CDL_MULTIPLY:
                Stack_Push( cdlStack, Stack_Pop( cdlStack ) * Stack_Pop( cdlStack ) );
                break;

            case DLS_CDL_DIVIDE:
                Stack_Push( cdlStack, Stack_Pop( cdlStack ) / Stack_Pop( cdlStack ) );
                break;

            case DLS_CDL_LOGICAL_AND:
                Stack_Push( cdlStack, ( Stack_Pop( cdlStack ) && Stack_Pop( cdlStack ) ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_LOGICAL_OR:
                Stack_Push( cdlStack, ( Stack_Pop( cdlStack ) || Stack_Pop( cdlStack ) ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_LT:
                Stack_Push( cdlStack, ( Stack_Pop( cdlStack ) < Stack_Pop( cdlStack ) ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_LE:
                Stack_Push( cdlStack, ( Stack_Pop( cdlStack ) <= Stack_Pop( cdlStack ) ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_GT:
                Stack_Push( cdlStack, ( Stack_Pop( cdlStack ) > Stack_Pop( cdlStack ) ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_GE:
                Stack_Push( cdlStack, ( Stack_Pop( cdlStack ) >= Stack_Pop( cdlStack ) ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_EQ:
                Stack_Push( cdlStack, ( Stack_Pop( cdlStack ) == Stack_Pop( cdlStack ) ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_NOT:
                Stack_Push( cdlStack, !Stack_Pop( cdlStack ) ? CDLTRUE : CDLFALSE );
                break;

            case DLS_CDL_CONST:
                Stack_Push( cdlStack, DLS_READ_INT_LITTLE );
                bytesLeft -= sizeof( spmUInt32 );
                break;

            case DLS_CDL_QUERY:
                ReadGUID( dlsParser, &dlsid );
                bytesLeft -= sizeof( GUID_t );

                err = CDLQuery( dlsParser, &dlsid, &query );

                Stack_Push( cdlStack, ( err == 0 ) ? query : CDLFALSE );
                break;

            case DLS_CDL_QUERY_SUPPORTED:
                ReadGUID( dlsParser, &dlsid );
                bytesLeft -= sizeof( GUID_t );

                Stack_Push( cdlStack, ( CDLQuery( dlsParser, &dlsid, &query ) >= 0 ) ? CDLTRUE : CDLFALSE );
                break;

            default:
                err = DLSParser_Error_ParseError;
                goto errExit;
            }
            if( Stack_Error( cdlStack ) < 0 ) break;
        }

        /* read top of stack */
        result = ( Stack_Pop( cdlStack ) ? TRUE : FALSE );
        if( Stack_Error( cdlStack ) < 0 ) err = DLSParser_Error_ParseError;
    }

errExit:
    /* return tos value or error */
    return( err < 0 ? err : result );
}

/**********************************************************************/
static int LoadChunk_CDL( DLS_Parser_t *dlsParser, long chunkSize )
{
    int result;

    /* CDLEvaluate returns: -ve (error), TRUE or FALSE */
    result = CDLEvaluate( dlsParser, chunkSize );

    if( result == FALSE )
    {
        dlsParser->abortList = TRUE;
    }

    return result;
}

/**********************************************************************/
static int LoadList_INS( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Orchestra_t  *orchestra = &dlsParser->orchestra;
    DLS_Instrument_t *ins;

    TOUCH(chunkSize);

    if( dlsParser->nextInsIndex >= orchestra->numInstruments )
    {
        DBUGMSG("ERROR in LoadList_INS: too many instruments.\n");
        return DLSParser_Error_ParseError;
    }
    ins = &orchestra->instruments[ dlsParser->nextInsIndex++ ];
    dlsParser->currentInstrument = ins;

    return 0;
}

/**********************************************************************/
static int LoadList_RGN( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Instrument_t *ins = dlsParser->currentInstrument;
    DLS_Region_t *region;

    TOUCH( chunkSize );
    if( dlsParser->nextRegionIndex >= ins->numRegions )
    {
        DBUGMSG("ERROR in LoadList_RGN: too many regions.\n");
        return DLSParser_Error_ParseError;
    }
    region = &ins->regions[ dlsParser->nextRegionIndex++ ];
    dlsParser->currentRegion = region;

    return 0;
}

/**********************************************************************/
static int LoadList_WAVE( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Wave_t *dlsWave;

    TOUCH(chunkSize);
    dlsWave = CreateWave( dlsParser );
    if( dlsWave == NULL )
        return SPMIDI_Error_OutOfMemory;

    if( dlsParser->wavePoolBase == 0 )
    {
        /* Get offset within DLS file of WavePool. */
        dlsParser->wavePoolBase = dlsParser->sio->getPosition( dlsParser->sio );
    }

    dlsWave->waveOffset = dlsParser->sio->getPosition( dlsParser->sio ) - dlsParser->wavePoolBase;

    return 0;
}

/**********************************************************************/
static int LoadChunk_INSH( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Instrument_t *ins = dlsParser->currentInstrument;

    TOUCH(chunkSize);

    if( ins == NULL )
    {
        return DLSParser_Error_ParseError; /* 050616 */
    }

    ins->numRegions = DLS_READ_INT_LITTLE;
    PRTLN_VALUE("Number of regions = ", ins->numRegions );

    /* The file contains the combined bank ID with MSB and LSB
     * and perhaps the DrumBit 31.
     * The Drum Bit 31 must be ignored so we mask it off here.
     * This is described on page 8 of the Mobile XMF spec.
     * Prior to 11/23/05, the bit was not masked off.
     * We also changed bankID to a short because we only care
     * about the low 16 bits.
     * On 6/9/07 We changed the way we pack the msb and lsb of the bank.
     * Instead of packing as two 8 bit bytes we pack them as two 7 bit fields
     * just like we do internally in the ME2000.
     */
    {
        int ulBank = DLS_READ_INT_LITTLE;
        int msb = (ulBank >> 8) & 0x007F; /* DLS positions the MSB at bit 8, not 7. */
        int lsb = ulBank & 0x007F;
        ins->bankID = (spmUInt16) ((msb << SPMIDI_BANK_MSB_SHIFT) | lsb);
    }
    PRTLN_HVALUE("Bank Index = ", ins->bankID );
#if DLS_PRINT_INSTRUMENTS
    PRTNUMH( ins->bankID );
    PRTMSG( " = Bank Index\n" );
#endif

    ins->programID = (spmUInt8)( DLS_READ_INT_LITTLE & 0x7F );
    PRTLN_VALUE("Program Index = ", ins->programID );
#if DLS_PRINT_INSTRUMENTS
    PRTNUMH( ins->programID );
    PRTMSG( " = Program Index\n" );
#endif

    ins->regions = SPMIDI_ALLOC_MEM( ins->numRegions * sizeof(DLS_Region_t), "regions" );
    if( ins->regions == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
    MemTools_Clear(ins->regions, ins->numRegions * sizeof(DLS_Region_t) );
    dlsParser->nextRegionIndex = 0;

    return 0;
}

/**********************************************************************/
static int LoadChunk_RGNH( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Region_t *region = dlsParser->currentRegion;

    TOUCH(chunkSize);

    if( region == NULL )
    {
        return DLSParser_Error_ParseError; /* 050616 */
    }

    region->waveSetRegion.lowPitch = (unsigned char) DLS_READ_SHORT_LITTLE;
    PRT_VALUE("Key Range = ", region->waveSetRegion.lowPitch);
    region->waveSetRegion.highPitch = (unsigned char) DLS_READ_SHORT_LITTLE;
    PRTLN_VALUE(" to ", region->waveSetRegion.highPitch);

    region->waveSetRegion.lowVelocity = (unsigned char) DLS_READ_SHORT_LITTLE;
    PRT_VALUE("Velocity Range = ", region->waveSetRegion.lowVelocity);
    region->waveSetRegion.highVelocity = (unsigned char) DLS_READ_SHORT_LITTLE;
    PRTLN_VALUE(" to ", region->waveSetRegion.highVelocity);

    region->options = (unsigned short) DLS_READ_SHORT_LITTLE;
    PRT_VALUE("Options = ", region->options);
    region->keyGroup = (unsigned short) DLS_READ_SHORT_LITTLE;
    PRTLN_VALUE(", Key Group = ", region->keyGroup);

    region->RGNHloaded = 1;

    return 0;
}

/**********************************************************************/
static int LoadChunk_WSMP( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Region_t *region;
    DLS_Wave_t *dlsWave;
    DLS_WaveSample_t *waveSample;
    int numLoops;

    TOUCH(chunkSize);

    /* Get the current region (or dlsWave) waveSample, or create it if it doesn't exist */
    switch( dlsParser->riffParser.listState )
    {
    case STATE_PARSING_LRGN:
        region = dlsParser->currentRegion;
        if( region == NULL )
            return DLSParser_Error_ParseError;  /* should be a current region, if we're in a region list */
        if( region->waveSample == NULL )
            region->waveSample = CreateWaveSample();
        waveSample = region->waveSample;
        break;
    case STATE_PARSING_WAVE:
        dlsWave = dlsParser->currentWave;
        if( dlsWave == NULL )
            return DLSParser_Error_ParseError;  /* should be a current dlsWave, if we're in a dlsWave list */
        if( dlsWave->wavesample == NULL )
            dlsWave->wavesample = CreateWaveSample();
        waveSample = dlsWave->wavesample;
        break;
    default:
        return DLSParser_Error_ParseError;
        break;
    }

    if( waveSample == NULL )
        return SPMIDI_Error_OutOfMemory;

    /* throw away the size value */
    DLS_READ_INT_LITTLE;

    waveSample->basePitch = (unsigned char) DLS_READ_SHORT_LITTLE;
    PRT_VALUE("Note = ", waveSample->basePitch );
    waveSample->fineTune = (unsigned short) DLS_READ_SHORT_LITTLE;
    PRTLN_VALUE(", FineTune = ", waveSample->fineTune);

    waveSample->gain = DLS_READ_INT_LITTLE;
    PRT_VALUE("Attenuation = ", waveSample->gain);

    DLS_READ_INT_LITTLE; /* Discard, not used. */
/*  waveSample->sampleOptions = DLS_READ_INT_LITTLE; */
/*  PRTLN_VALUE(", Options = ", waveSample->sampleOptions); */

    numLoops = DLS_READ_INT_LITTLE;
    if( numLoops > 1 )
    {
        PRTLN_VALUE("Error in LoadChunk_WSMP: numLoops > 1, = ", numLoops );
        return DLSParser_Error_ParseError;
    }
    else if( numLoops == 1 )
    {
        /* throw away the size value */
        DLS_READ_INT_LITTLE;
        /* loopType defines whether the portion after the loop is played.
         * It seems redundant because in ME2000 we just play audio after 
         * loop if it exists. If release not played then discard before
         * loading into ME3000.
         */
        waveSample->loopType = (spmUInt8) DLS_READ_INT_LITTLE;
        PRTLN_VALUE(", loopType = ", waveSample->loopType);
        waveSample->loopStart = DLS_READ_INT_LITTLE;
        PRTLN_VALUE(", loopStart = ", waveSample->loopStart);
        waveSample->loopSize = DLS_READ_INT_LITTLE;
        PRTLN_VALUE(", loopSize = ", waveSample->loopSize);
    }
    else if( numLoops == 0 )
    {
        waveSample->loopStart = -1;
    }
    return 0;
}

/**********************************************************************/
/** WLNK chunk points to a dlsWave using the index of a cue entry in
 * the dlsWave pool table.
 */
static int LoadChunk_WLNK( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Region_t *region;

    TOUCH(chunkSize);

    region = dlsParser->currentRegion;
    /* NOTE - we may not need these for Mobile DLS */
    region->linkOptions = (unsigned short) DLS_READ_SHORT_LITTLE;
    region->phaseGroup = (unsigned short) DLS_READ_SHORT_LITTLE;

    region->channel = DLS_READ_INT_LITTLE;
    /* Important link to dlsWave. */
    region->tableIndex = DLS_READ_INT_LITTLE;
    PRTLN_VALUE("tableIndex = ", region->tableIndex);

    region->WLNKloaded = 1;

    return 0;
}

/**********************************************************************/
static int LoadChunk_ART( DLS_Parser_t *dlsParser, long chunkSize )
{
    int numConnections, i;
    DLS_ArticulationTokens token;
    DLS_Articulation_t *articulation;
    spmUInt16 source, control, destination, transform;
    spmSInt32 scale;
    int endOfChunk;
    DLS_Instrument_t *ins = dlsParser->currentInstrument;
    DLS_Region_t *region = dlsParser->currentRegion;

    DLS_ArticulationTracker_t *artTracker = NULL;
    
    /*
     * Is the articulation local, or global to the instrument? and
     * is it voice or note-related?
     */
    if( dlsParser->artIsLocal )
    {
        artTracker = &(region->articulations);
    }
    else
    {
        artTracker = &(ins->articulations);
    }

    endOfChunk = DLS_Tell(dlsParser) + chunkSize;

    DLS_READ_INT_LITTLE; /* size */

    numConnections = DLS_READ_INT_LITTLE;
    PRTLN_VALUE(", numConnections = ", numConnections);

#if DLS_TRACK_ART_ALLOCS
        sNumArtConnTotal += numConnections;
#endif

    if( numConnections > 0 )
    {
        /* Allocate memory to store the Articulation data. */
        if( artTracker->articulations != NULL )
        {
            /* We already have some articulations, so extend the array
             * by allocating bigger array copying old data to beginning of array.
             */
            spmSInt numBytes = sizeof( DLS_Articulation_t ) * (numConnections + artTracker->numArticulations);
            articulation = SPMIDI_ALLOC_MEM( numBytes, "articulations");
            if( articulation == NULL )
            {
                return SPMIDI_Error_OutOfMemory;
            }
            /* Copy previous articulations into new memory. */
            MemTools_Copy( articulation, artTracker->articulations,
                sizeof( DLS_Articulation_t ) * artTracker->numArticulations );
            /* Free old articulation array. */
            SPMIDI_FreeMemory( artTracker->articulations );
            /* Clear remaining articulations. */
            MemTools_Clear( &articulation[artTracker->numArticulations],
                sizeof( DLS_Articulation_t ) * numConnections );
        }
        else
        {
            spmSInt numBytes = sizeof( DLS_Articulation_t ) * numConnections;
            articulation = SPMIDI_ALLOC_MEM( numBytes, "articulations" );
            if( articulation == NULL )
            {
                return SPMIDI_Error_OutOfMemory;
            }
            /* Clear all articulations. */
            MemTools_Clear( articulation,
                sizeof( DLS_Articulation_t ) * numConnections );
        }
        artTracker->articulations = articulation;
    }

#if DLS_TRACK_ART_ALLOCS
    sNumArtAllocs += 1;
    sNumArtBytes += sizeof(DLS_Articulation_t) * numConnections;
#endif

    for( i=0; i<numConnections; i++ )
    {
        /* Don't read past the end of the chunk data.
         * This test will protect against a corrupted value for numConnections.
         */
        if( DLS_Tell(dlsParser) >= endOfChunk )
        {
            return DLSParser_Error_ParseError; /* 050616 */
        }

        source = (spmUInt16)DLS_READ_SHORT_LITTLE;
        control = (spmUInt16)DLS_READ_SHORT_LITTLE;
        destination = (spmUInt16)DLS_READ_SHORT_LITTLE;
        transform = (spmUInt16)DLS_READ_SHORT_LITTLE;
        scale = (spmSInt32)DLS_READ_INT_LITTLE;

        token = FindArticulationToken( source, control, destination );
        
        /* Did we find it? */
        if( token != CONN_Z_UNRECOGNIZED )
        {
            articulation = &artTracker->articulations[ artTracker->numArticulations ];
            
            artTracker->numArticulations += 1;
            
            articulation->token = token;
            articulation->scale = scale;

            /* Convert from DLS number to ME3000 number. */
            articulation->scale = DLSParser_ConvertArticulationData( articulation, dlsParser->sampleRate );
            
#if DLS_TRACK_ART_ALLOCS
            sNumArtConnUsed += 1;
#endif
        }
        
        PRTLN_VALUE("------ ", i );
        PRT_HVALUE("Source      = ", source);
        PRT_HVALUE(", Control = ", control);
        PRT_HVALUE(", Destination = ", destination);
        PRTLN_VALUE(", Iscale = ", scale);
    }

    return 0;
}

/**********************************************************************/
static int LoadChunk_FMT( DLS_Parser_t *dlsParser, long chunkSize )
{
    spmUInt16 extraSize;
    spmUInt32 startOfExtraStructure;
    int reserved;
    int channelMask;
    GUID_t guid;

    DLS_Wave_t *dlsWave = dlsParser->currentWave;

    TOUCH(chunkSize);
    if( dlsWave == NULL )
    {
        DBUGMSG("ERROR: No current dlsWave; we should be in a 'LIST wave'");
        return DLSParser_Error_ParseError;
    }

    dlsWave->format = (spmUInt16) DLS_READ_SHORT_LITTLE;
    PRTLN_VALUE("Format = ", dlsWave->format );

    dlsWave->numChannels = (spmUInt8)DLS_READ_SHORT_LITTLE;
    PRT_VALUE("Channels = ", dlsWave->numChannels );

    dlsWave->sampleRate = DLS_READ_INT_LITTLE;
    PRTLN_VALUE(", Sample Rate = ", dlsWave->sampleRate );
    if( dlsWave->sampleRate < 1 )
    {
        /* Should we throw a parser error here instead? */
        /* We clip this so that later pitch calculations do not fail. */
        dlsWave->sampleRate = 1;
    }

    DLS_READ_INT_LITTLE; /* AvgByteRate */
    DLS_READ_SHORT_LITTLE; /* Block align */

    /* next value is bitsPerSample; divide by 8 to get bytesPerSample */
    dlsWave->bytesPerSample = (unsigned char) (DLS_READ_SHORT_LITTLE / 8);
    PRTLN_VALUE("BytesPerSample = ", dlsWave->bytesPerSample );

    if( dlsWave->format == WAVE_FORMAT_EXTENSIBLE )
    {
        /* NOTE - these are values are not currently used so we
         * may just seek() over them.
         */
        extraSize = (spmUInt16)DLS_READ_SHORT_LITTLE;
        startOfExtraStructure = dlsParser->sio->getPosition( dlsParser->sio );

        reserved = DLS_READ_SHORT_LITTLE;
        PRTLN_VALUE("Reserved Value = ", reserved );

        channelMask = DLS_READ_INT_LITTLE;
        PRTLN_VALUE("ChannelMask = ", channelMask );

        ReadGUID( dlsParser, &guid );

        if( (dlsParser->sio->getPosition( dlsParser->sio ) - startOfExtraStructure) != extraSize )
        {
            return DLSParser_Error_ParseError;
        }
    }

    if ( !((( dlsWave->bytesPerSample == 1 ) || ( dlsWave->bytesPerSample == 2 )) && \
            (( dlsWave->format == WAVE_FORMAT_PCM ) || ( dlsWave->format == WAVE_FORMAT_ALAW ))) )
    {
        DBUGMSG("ERROR: Unsupported sample format\n");
        return DLSParser_Error_UnsupportedSampleFormat;
    }

    return 0;
}

/**********************************************************************/
static int LoadChunk_PTBL( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Orchestra_t *orchestra = &dlsParser->orchestra;
    int i;

    TOUCH(chunkSize);
    /* Throw away cbSize. We may use this in the future to determine whether optional
     * data has been added to the chunk.
     */
    DLS_READ_INT_LITTLE; /* Size */

    orchestra->numPoolTableEntries = DLS_READ_INT_LITTLE;
    PRTLN_VALUE("NumberOfPoolTableEntries = ", orchestra->numPoolTableEntries );

    /* The rawPoolTable contains offsets into the waves in the DLS file.
     * When we parse the waves we match up the offsets with the entries
     * in the rawPoolTable and make a new entry in the poolTable
     * that contains pointers to the wave structures.
     */
    orchestra->poolTable = SPMIDI_ALLOC_MEM( orchestra->numPoolTableEntries * sizeof(void *), "pooltable" );
    if( orchestra->poolTable == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }

    orchestra->rawPoolTable = SPMIDI_ALLOC_MEM( orchestra->numPoolTableEntries * sizeof(spmUInt32), "rawpooltable" );
    if( orchestra->rawPoolTable == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }

    for( i=0; i<orchestra->numPoolTableEntries; i++ )
    {
        orchestra->poolTable[i] = NULL;
        orchestra->rawPoolTable[i] = DLS_READ_INT_LITTLE;
    }
    return 0;
}

/**********************************************************************/
static int LoadChunk_Text( DLS_Parser_t *dlsParser, long chunkSize )
{
    int numToRead, numRead;
    char pad[256];
    if( chunkSize > (sizeof(pad)-1) )
        numToRead = (sizeof(pad)-1);
    else
        numToRead = chunkSize;
    numRead = dlsParser->sio->read( dlsParser->sio, pad, numToRead );
    pad[numRead] = 0;
    PRTLN_TEXT( pad );
    return 0;
}

/**********************************************************************/
static char * GetWaveAddressIfOK( DLS_Wave_t *dlsWave, StreamIO *sio )
{
#if SPMIDI_LEAVE_DLS_WAVES_IN_IMAGE
    char *image = NULL;
    if( dlsWave->format == WAVE_FORMAT_PCM )
    {
        image = GetCurrentStreamAddress( sio );
        if( image != NULL )
        {
            /* 8 bit data is OK. */
            /* 16 bit data has more restrictions. */
            if( dlsWave->bytesPerSample == 2 )
            {
                /* If we are doing 16 bit data then address must be even
                 * and CPU must be LittleEndian */
                if( ((((int)image) & 1) != 0 ) || !IsHostLittleEndian() )
                {
                    image = NULL; /* Sorry. Samples not arranged in memory correctly. */
                }
            }
        }
    }
    return image;
#else
    (void) dlsWave;
    (void) sio;
    return NULL;
#endif
}


/**********************************************************************/
static int LoadChunk_Data( DLS_Parser_t *dlsParser, long chunkSize )
{
    DLS_Wave_t *dlsWave = dlsParser->currentWave;
    spmSInt16 *waveTable;
    spmUInt32 i;
    spmUInt32 numSamples;
    char *waveImageInPlace = NULL;

    /* If we're not inside a WAVE chunk, something's really awry */
    if( dlsWave == NULL )
        return DLSParser_Error_ParseError;

    /* DLS mandates that we should have already loaded a FMT chunk, so check this is true */
    if( dlsWave->bytesPerSample  == 0 )
    {
        return DLSParser_Error_ParseError;
    }

    numSamples = chunkSize / dlsWave->bytesPerSample;
    dlsWave->numSamples = numSamples;

    /* If we can leave the WAVE in the DLS file then we can save memory.
     * Otherwise we have to allocate memory and transfer the
     * wave to a playable location.
     */
    waveImageInPlace = GetWaveAddressIfOK( dlsWave, dlsParser->sio );

    waveTable = NULL;
    if( waveImageInPlace != NULL )
    {
        waveTable = (short *) waveImageInPlace;
        dlsWave->isAllocated = 0;
    }
    
    switch( dlsWave->format )
    {
    case WAVE_FORMAT_ALAW:
//      PRTMSG("ALAW WAVETABLE!\n");
    case WAVE_FORMAT_PCM:
        if( waveImageInPlace == NULL )
        {
            /* Allocate U8 or S16 PNM data without changing the size. */
            waveTable = CreateWaveTable( numSamples * dlsWave->bytesPerSample );
            if( waveTable == NULL )
            {
                return SPMIDI_Error_OutOfMemory;
            }
            dlsWave->isAllocated = 1;

            if( dlsWave->bytesPerSample == 1 )
            {

                dlsParser->sio->read( dlsParser->sio,
                    (char *) waveTable, numSamples );
            }
            else if( dlsWave->bytesPerSample == 2 )
            {
                /* load so we have correct byte order for 16-bit values */
                for( i=0; i<numSamples; i++ )
                {
                    waveTable[i] = (short) DLS_READ_SHORT_LITTLE;
                }
            }
            else
            {
                return DLSParser_Error_UnsupportedSampleFormat;
            }
        }
        break;

    default:
        return DLSParser_Error_UnsupportedSampleFormat;
    }

    dlsWave->samples = (void*)waveTable;
    return 0;
}

/**********************************************************************/
static int PrintChunkInfo( DLS_Parser_t *dlsParser, RIFF_ChunkID chunkID, long chunkSize, char *msg )
{
    char pad[5];
    pad[0] = (char) (chunkID >> 24);
    pad[1] = (char) (chunkID >> 16);
    pad[2] = (char) (chunkID >> 8);
    pad[3] = (char) chunkID;
    pad[4] = 0;

    PrintChunk_Indent( dlsParser );
    DBUGMSG(msg);
    DBUGMSG(": ");
    DBUGMSG(pad);
    PRT_VALUE(", size = ", chunkSize );
    PRTLN_VALUE(" at ", dlsParser->sio->getPosition( dlsParser->sio ) );
    gNeedIndent = 1;

    TOUCH(msg);
    return 0;
}

/**********************************************************************/
int MyChunkHandler( void *userData, RIFF_ChunkID chunkID, long chunkSize )
{
    int result;
    DLS_Parser_t *dlsParser = (DLS_Parser_t *) userData;

    result = PrintChunkInfo( dlsParser, chunkID, chunkSize, "Chunk" );

    /* if the abort flag is set, skip the chunk */
    if( !( dlsParser->abortList) )
    {
        dlsParser->level += 1;
        switch( chunkID )
        {
        case FOURCC_COLH:
            result = LoadChunk_COLH( dlsParser, chunkSize );
            break;
        case FOURCC_CDL:
            result = LoadChunk_CDL( dlsParser, chunkSize );
            break;

        case FOURCC_INSH:
#if DLS_TRACK_ART_ALLOCS
        sNumInstruments += 1;
#endif
            result = LoadChunk_INSH( dlsParser, chunkSize );
            break;

        case FOURCC_RGNH:
#if DLS_TRACK_ART_ALLOCS
        sNumRegions += 1;
#endif
            result = LoadChunk_RGNH( dlsParser, chunkSize );
            break;

        case FOURCC_ART1:
#if DLS_TRACK_ART_ALLOCS
        sNumArtART1 += 1;
#endif
            result = LoadChunk_ART( dlsParser, chunkSize );
            break;

        case FOURCC_ART2:
#if DLS_TRACK_ART_ALLOCS
        sNumArtART2 += 1;
#endif
            result = LoadChunk_ART( dlsParser, chunkSize );
            break;

        case FOURCC_WSMP:
            result = LoadChunk_WSMP( dlsParser, chunkSize );
            break;
        case FOURCC_FMT:
            result = LoadChunk_FMT( dlsParser, chunkSize );
            break;
        case FOURCC_DATA:
            result = LoadChunk_Data( dlsParser, chunkSize );
            break;
        case FOURCC_WLNK:
            result = LoadChunk_WLNK( dlsParser, chunkSize );
            break;
        case FOURCC_PTBL:
            result = LoadChunk_PTBL( dlsParser, chunkSize );
            break;
        case FOURCC_INAM:
            result = LoadChunk_Text( dlsParser, chunkSize );
            break;

        case FOURCC_ISFT:
        case FOURCC_ICOP:
        case FOURCC_FACT:
        case FOURCC_ICMT:
        case FOURCC_IENG:
        case FOURCC_DLID:
        case FOURCC_VERS:
            /* Just ignore these. They are used for editing on desktops, etc. */
            break;

        default:
            PRTLN_HVALUE("UNRECOGNIZED = ", chunkID );
            PrintChunkInfo( dlsParser, chunkID, chunkSize, "Chunk" );
            break;
        }
        dlsParser->level -= 1;
    }

    return result;
}

/**********************************************************************/
int MyBeginFormHandler( void *userData, RIFF_ChunkID formType, long chunkSize )
{
    DLS_Parser_t *dlsParser = (DLS_Parser_t *) userData;
    PrintChunkInfo( dlsParser, formType, chunkSize, "FORM" );
    if( formType != FOURCC_DLS )
        return DLSParser_Error_NotDLS;
    dlsParser->level += 1;
    return 0;
}
/**********************************************************************/
int MyEndFormHandler( void *userData, RIFF_ChunkID formType, long chunkSize )
{
    DLS_Parser_t *dlsParser = (DLS_Parser_t *) userData;
    DLS_Orchestra_t *orchestra = &dlsParser->orchestra;

    TOUCH(chunkSize);
    dlsParser->level -= 1;
    if( formType == FOURCC_DLS )
    {
        /* make sure we've got our pooltable and our instrument information */
        if( orchestra->poolTable == NULL || dlsParser->currentInstrument == NULL || dlsParser->wavePoolBase == 0 )
        {
            return( DLSParser_Error_ParseError );
        }
    }

    return 0;
}

/**********************************************************************/
static int MyBeginListHandler( void *userData, RIFF_ChunkID listType, long chunkSize )
{
    int result = 0;
    DLS_Parser_t *dlsParser = (DLS_Parser_t *) userData;
    PrintChunkInfo( dlsParser, listType, chunkSize, "LIST" );
    dlsParser->level += 1;
    switch( listType )
    {
    case FOURCC_LINS:
        dlsParser->riffParser.listState = STATE_PARSING_LINS;
        break;
    case FOURCC_LRGN:
        dlsParser->riffParser.listState = STATE_PARSING_LRGN;
        break;
    case FOURCC_INS:
        result = LoadList_INS( dlsParser, chunkSize );
        break;
    case FOURCC_RGN:
    case FOURCC_RGN2:
        result = LoadList_RGN( dlsParser, chunkSize );
        break;
    case FOURCC_WAVE:
        dlsParser->riffParser.listState = STATE_PARSING_WAVE;
        result = LoadList_WAVE( dlsParser, chunkSize );
        break;
    case FOURCC_LART:
    case FOURCC_LAR2:
        SetArticulationFlag( dlsParser );
        break;
    default:
        break;
    }
    return result;
}

/**********************************************************************/
static int MyEndListHandler( void *userData, RIFF_ChunkID listType, long chunkSize )
{
    DLS_Parser_t *dlsParser = (DLS_Parser_t *) userData;

    TOUCH(chunkSize);
    dlsParser->level -= 1;
    switch( listType )
    {
    case FOURCC_LINS:
        dlsParser->riffParser.listState = STATE_IDLE;

        /* must have at least one instrument */
        if( dlsParser->currentInstrument == NULL )
        {
            return( DLSParser_Error_ParseError );
        }
        /* We need to have found all the regions that were declared. */
        if( dlsParser->nextRegionIndex < dlsParser->currentInstrument->numRegions )
        {
            DBUGMSG("ERROR in LoadList_RGN: not enough regions found.\n");
            return DLSParser_Error_ParseError; /* 050616 */
        }
        break;
    case FOURCC_LRGN:
        dlsParser->riffParser.listState = STATE_PARSING_LINS;
        break;
    case FOURCC_RGN:
    case FOURCC_RGN2:
        if( dlsParser->abortList )
        {
            DeleteRegion( dlsParser->currentRegion );
            dlsParser->nextRegionIndex--;

            dlsParser->abortList = FALSE;
        }
        else
        {
            /* make sure we've got what we need */
            if ( !( dlsParser->currentRegion->WLNKloaded && dlsParser->currentRegion->RGNHloaded ) )
                return( DLSParser_Error_ParseError );
        }
        break;
    case FOURCC_LART:
    case FOURCC_LAR2:
        if( dlsParser->abortList )
        {
            if( dlsParser->artIsLocal )
            {
                DeleteArticulationArray( &(dlsParser->currentRegion->articulations) );
            }
            else
            {
                DeleteArticulationArray( &(dlsParser->currentInstrument->articulations) );
            }
            dlsParser->abortList = FALSE;
        }
        break;
    default:
        break;
    }
    return 0;
}


/**********************************************************************/
/** Create a parser context for DLS.
 */
static SPMIDI_Error DLSParser_Create_Internal( DLSParser **parserPtr, unsigned char *fileStart, spmSInt32 fileSize )
{
    RiffParser_t *riffParser = NULL;
    DLS_Parser_t *dlsParser = NULL;
    DLS_Orchestra_t *dlsOrch = NULL;

    dlsParser = (DLS_Parser_t *) SPMIDI_ALLOC_MEM( sizeof(DLS_Parser_t), "DLS_Parser_t" ) ;
    if( dlsParser == NULL )
    {
        goto nomem;
    }
    MemTools_Clear( dlsParser, sizeof(DLS_Parser_t) );
    dlsOrch = &dlsParser->orchestra;
    riffParser = &dlsParser->riffParser;

    dlsParser->sio = Stream_OpenImage( (char *)fileStart, fileSize );
    if( dlsParser->sio == NULL )
    {
        goto nomem;
    }

    DLL_InitList( &dlsOrch->waves );
    dlsParser->sampleRate = DLS_DEFAULT_SAMPLERATE;

    riffParser->stream = dlsParser->sio;
    riffParser->userData = dlsParser;

    riffParser->handleChunk = MyChunkHandler;
    riffParser->handleBeginForm = MyBeginFormHandler;
    riffParser->handleEndForm = MyEndFormHandler;
    riffParser->handleBeginList = MyBeginListHandler;
    riffParser->handleEndList = MyEndListHandler;

    *parserPtr = (DLSParser *)dlsParser;

    return SPMIDI_Error_None;

nomem:
    SPMIDI_FreeMemory( dlsParser );
    return SPMIDI_Error_OutOfMemory;
}

/**********************************************************************/
/*
 * Resolves any references in disparate structures,
 * because chunks may be in any order.
 */

static int DLSParser_ResolvePoolTable( DLS_Parser_t *parser )
{
    DLS_Wave_t *dlsWave;
    DLS_Orchestra_t *orchestra = &parser->orchestra;
    int i;

    /* fill in poolTable with actual pointers, using information in rawPoolTable */
    DLL_FOR_ALL( DLS_Wave_t, dlsWave, &orchestra->waves )
    {
        DLS_Wave_t *match = NULL;
        /* Scan rawPoolTable and fill in poolTable with pointers to loaded wave */
        for( i=0; i<orchestra->numPoolTableEntries; i++ )
        {
            if( dlsWave->waveOffset == orchestra->rawPoolTable[i] )
            {
                match = dlsWave;
                break;
            }
        }
        /* Make sure we found one. */
        if( match == NULL )
        {
            return SPMIDI_Error_BadFormat;
        }
        else
        {
            orchestra->poolTable[i] = match;
        }

    }
    return 0;
}

/*********************************************************/
/**
 * Set sample rate for DLS parsing.  This is used by conditional chunks
 * when querying device capabilities.
 */
SPMIDI_Error DLSParser_SetSampleRate( DLSParser *parser, spmSInt32 sampleRate )
{
    DLS_Parser_t* dlsParser = (DLS_Parser_t *)parser;
    if( dlsParser == NULL )
        return SPMIDI_Error_IllegalArgument;

    dlsParser->sampleRate = sampleRate;

    return 0;
}

/*********************************************************/
/**
 * Get sample rate for DLS parsing.  This is used by conditional chunks
 * when querying device capabilities.
 */
spmSInt32 DLSParser_GetSampleRate( DLSParser *parser )
{
    DLS_Parser_t* dlsParser = (DLS_Parser_t *)parser;
    if( dlsParser == NULL )
        return SPMIDI_Error_IllegalArgument;

    return ( dlsParser->sampleRate );
}

/*******************************************************************************/
/**
 * Get internal orchestra structure from parser.  This is used after parsing
 * by StreamDLS to do something other than just load the orchestra to ME3000.
 */
DLS_Orchestra_t *DLSParser_GetOrchestra( DLSParser parser )
{
    DLS_Parser_t* dlsParser = (DLS_Parser_t *)parser;
    if( dlsParser == NULL )
        return NULL;

    return( &(dlsParser->orchestra) );
}

/*********************************************************/
/**
 * Read DLS instruments and wavetables from image in Stream,
 * which may be a file or an in-memory image.
 */
static SPMIDI_Error DLSParser_Parse_Internal( DLSParser *parser )
{
    int result;
    int i;
    RiffParser_t *riffParser = NULL;
    DLS_Parser_t *dlsParser = NULL;
    DLS_Orchestra_t *dlsOrch;

    dlsParser = (DLS_Parser_t *)parser;
    if( dlsParser == NULL )
        return SPMIDI_Error_IllegalArgument;

    riffParser = &dlsParser->riffParser;
    dlsOrch = &dlsParser->orchestra;

    result = RIFF_ParseStream( riffParser );
    if( result < 0 )
        goto error;

    /* Check that we found at least one instrument */
    if( dlsOrch->numInstruments == 0 )
    {
        result = DLSParser_Error_ParseError;
        goto error;
    }

    result =  DLSParser_ResolvePoolTable( dlsParser );
    if( result < 0 )
        goto error;

    /* Finish loading instruments, connect waves, convert articulations. */
    for( i=0; i<dlsOrch->numInstruments; i++ )
    {
        result = DLSParser_ResolveInstrument( dlsOrch, i );
        if( result < 0 )
            goto error;
    }
    
#if DLS_TRACK_ART_ALLOCS
    PRTMSGNUMD("sNumArtAllocs = ", sNumArtAllocs );
    PRTMSGNUMD("sNumArtBytes = ", sNumArtBytes );
    PRTMSGNUMD("sNumRegions = ", sNumRegions );
    PRTMSGNUMD("sNumInstruments = ", sNumInstruments );
    PRTMSGNUMD("sNumArtConnTotal = ", sNumArtConnTotal );
    PRTMSGNUMD("sNumArtConnUsed = ", sNumArtConnUsed );
    PRTMSGNUMD("sNumArtART1 = ", sNumArtART1 );
    PRTMSGNUMD("sNumArtART2 = ", sNumArtART2 );
#endif

    return 0;

error:
    return result;

}

#define SSDLS_LOG2_SCALE  (128)
#define SSDLS_LOG2_SIZE   (SSDLS_LOG2_SCALE+1)
static const spmUInt16 sLogBase2_1to2[] =
    {
        0, /* 0, 1.0000 */
        736, /* 1, 1.0078 */
        1466, /* 2, 1.0156 */
        2190, /* 3, 1.0234 */
        2909, /* 4, 1.0313 */
        3623, /* 5, 1.0391 */
        4331, /* 6, 1.0469 */
        5034, /* 7, 1.0547 */
        5732, /* 8, 1.0625 */
        6425, /* 9, 1.0703 */
        7112, /* 10, 1.0781 */
        7795, /* 11, 1.0859 */
        8473, /* 12, 1.0938 */
        9146, /* 13, 1.1016 */
        9814, /* 14, 1.1094 */
        10477, /* 15, 1.1172 */
        11136, /* 16, 1.1250 */
        11791, /* 17, 1.1328 */
        12440, /* 18, 1.1406 */
        13086, /* 19, 1.1484 */
        13727, /* 20, 1.1563 */
        14363, /* 21, 1.1641 */
        14996, /* 22, 1.1719 */
        15624, /* 23, 1.1797 */
        16248, /* 24, 1.1875 */
        16868, /* 25, 1.1953 */
        17484, /* 26, 1.2031 */
        18096, /* 27, 1.2109 */
        18704, /* 28, 1.2188 */
        19308, /* 29, 1.2266 */
        19909, /* 30, 1.2344 */
        20505, /* 31, 1.2422 */
        21098, /* 32, 1.2500 */
        21687, /* 33, 1.2578 */
        22272, /* 34, 1.2656 */
        22854, /* 35, 1.2734 */
        23433, /* 36, 1.2813 */
        24007, /* 37, 1.2891 */
        24579, /* 38, 1.2969 */
        25146, /* 39, 1.3047 */
        25711, /* 40, 1.3125 */
        26272, /* 41, 1.3203 */
        26830, /* 42, 1.3281 */
        27384, /* 43, 1.3359 */
        27936, /* 44, 1.3438 */
        28484, /* 45, 1.3516 */
        29029, /* 46, 1.3594 */
        29571, /* 47, 1.3672 */
        30109, /* 48, 1.3750 */
        30645, /* 49, 1.3828 */
        31178, /* 50, 1.3906 */
        31707, /* 51, 1.3984 */
        32234, /* 52, 1.4063 */
        32758, /* 53, 1.4141 */
        33279, /* 54, 1.4219 */
        33797, /* 55, 1.4297 */
        34312, /* 56, 1.4375 */
        34825, /* 57, 1.4453 */
        35334, /* 58, 1.4531 */
        35841, /* 59, 1.4609 */
        36346, /* 60, 1.4688 */
        36847, /* 61, 1.4766 */
        37346, /* 62, 1.4844 */
        37842, /* 63, 1.4922 */
        38336, /* 64, 1.5000 */
        38827, /* 65, 1.5078 */
        39316, /* 66, 1.5156 */
        39802, /* 67, 1.5234 */
        40286, /* 68, 1.5313 */
        40767, /* 69, 1.5391 */
        41246, /* 70, 1.5469 */
        41722, /* 71, 1.5547 */
        42196, /* 72, 1.5625 */
        42667, /* 73, 1.5703 */
        43137, /* 74, 1.5781 */
        43603, /* 75, 1.5859 */
        44068, /* 76, 1.5938 */
        44530, /* 77, 1.6016 */
        44990, /* 78, 1.6094 */
        45448, /* 79, 1.6172 */
        45904, /* 80, 1.6250 */
        46357, /* 81, 1.6328 */
        46809, /* 82, 1.6406 */
        47258, /* 83, 1.6484 */
        47705, /* 84, 1.6563 */
        48150, /* 85, 1.6641 */
        48593, /* 86, 1.6719 */
        49034, /* 87, 1.6797 */
        49472, /* 88, 1.6875 */
        49909, /* 89, 1.6953 */
        50344, /* 90, 1.7031 */
        50776, /* 91, 1.7109 */
        51207, /* 92, 1.7188 */
        51636, /* 93, 1.7266 */
        52063, /* 94, 1.7344 */
        52488, /* 95, 1.7422 */
        52911, /* 96, 1.7500 */
        53332, /* 97, 1.7578 */
        53751, /* 98, 1.7656 */
        54169, /* 99, 1.7734 */
        54584, /* 100, 1.7813 */
        54998, /* 101, 1.7891 */
        55410, /* 102, 1.7969 */
        55820, /* 103, 1.8047 */
        56229, /* 104, 1.8125 */
        56635, /* 105, 1.8203 */
        57040, /* 106, 1.8281 */
        57443, /* 107, 1.8359 */
        57845, /* 108, 1.8438 */
        58245, /* 109, 1.8516 */
        58643, /* 110, 1.8594 */
        59039, /* 111, 1.8672 */
        59434, /* 112, 1.8750 */
        59827, /* 113, 1.8828 */
        60219, /* 114, 1.8906 */
        60609, /* 115, 1.8984 */
        60997, /* 116, 1.9063 */
        61384, /* 117, 1.9141 */
        61769, /* 118, 1.9219 */
        62152, /* 119, 1.9297 */
        62534, /* 120, 1.9375 */
        62915, /* 121, 1.9453 */
        63294, /* 122, 1.9531 */
        63671, /* 123, 1.9609 */
        64047, /* 124, 1.9688 */
        64421, /* 125, 1.9766 */
        64794, /* 126, 1.9844 */
        65166, /* 127, 1.9922 */
        65535, /* 128, 2.0000 */
    };

/* Convert from sample rate as frequency to the pitch domain.
 * Use Log base 2 lookup table.
 * Express as offset from SS_BASE_SAMPLE_RATE.
 */
PitchOctave ConvertSampleRateOffset( spmSInt32 sampleRateHertz )
{
    /* Map Hertz into table 16.16. */
    /* Order the operations carefully to avoid overflow. */
    spmUInt32 driver;
    PitchOctave offset;
    int octaveDelta = 0;

    while( sampleRateHertz >= (SS_BASE_SAMPLE_RATE*2) )
    {
        sampleRateHertz = sampleRateHertz >> 1;
        octaveDelta += 1;
    }
    if( sampleRateHertz < 1 )
    {
        /* Prevent infinite loop in new statement. */
        sampleRateHertz = 1;
    }
    while( sampleRateHertz < SS_BASE_SAMPLE_RATE )
    {
        sampleRateHertz = sampleRateHertz << 1;
        octaveDelta -= 1;
    }

    driver = sampleRateHertz    - SS_BASE_SAMPLE_RATE;
    /* We want to map SS_BASE_SAMPLE_RATE to (128<<16). So:
     *     SS_BASE_SAMPLE_RATE*N/M = (128<<16)
     * If M = CSR2P_DENOM, N = ((128<<16)*CSR2P_DENOM)/SS_BASE_SAMPLE_RATE)
     * To calculate MAX_DENOM:
     *   0xFFFFFFFF = SS_BASE_SAMPLE_RATE * ((128<<16)*CSR2P_DENOM)/SS_BASE_SAMPLE_RATE)
     *   0xFFFFFFFF = (128<<16)*CSR2P_DENOM
     *   0xFFFFFFFF/(128<<16) = CSR2P_DENOM
     *   0xFFFFFFFF/(128<<16) = CSR2P_DENOM
     *   1<<32 / 1<<(7+16)
     *   1<<(32-25)
     *   1<<7 so use 1<<6 to be safe
     */
#define  CSR2P_DENOM_SHIFT  (6)

    driver = driver * (((SSDLS_LOG2_SCALE<<16) << CSR2P_DENOM_SHIFT) / SS_BASE_SAMPLE_RATE);
    driver = driver >> CSR2P_DENOM_SHIFT;

    offset = SSDLS_UShortTableLookup( sLogBase2_1to2, SSDLS_LOG2_SIZE,
                                      driver );

    /* Adjust for octave scaling. */
    offset += (octaveDelta * 0x00010000);

    return offset;
}

#if 0

/* Convert from sample rate as frequency to the pitch domain.
 * Express as offset from SS_BASE_SAMPLE_RATE.
 */
PitchOctave CalculateSampleRateOffset( spmSInt32 sampleRateHertz )
{
    /* calculate sample rate offset from SS_BASE_SAMPLE_RATE */
    double log2scaler = log(sampleRateHertz / (double)SS_BASE_SAMPLE_RATE) / log(2.0);
    return (PitchOctave) ((log2scaler * (1<<SS_PITCH_SHIFT)) + 0.5);
}

/*******************************************************************************/
/* Generate sLogBase2_1to2 table. */
int mainXX( void )
{
    int i;
    printf("static const spmUInt16 sLogBase2_1to2[] = {\n");
    /* Generate a table with one extra value as a guard point. */
    for( i = 0; i<SSDLS_LOG2_SIZE; i++ )
    {
        double fInput = 1.0 + (((double) i) / SSDLS_LOG2_SCALE);
        double log2scaler = log(fInput) / log(2.0);
        spmSInt32 iInput = (FXP16) ((log2scaler * 0x010000) + 0.5);
        if( iInput > (spmSInt32)0x0000FFFF )
            iInput = (spmSInt32)0x0000FFFF;
        printf("  %5d, /* %d, %6.4f */\n", iInput, i, fInput );
    }
    printf("};\n" );
    return 0;
}

/*******************************************************************************/
/* Test ConvertSampleRateOffset table lookup. */
int mainYY( void )
{
    int lookupResult, calcResult;
    int delta, delta2;
    double fSampleRate = 8000.0;
    double multiplier = 1.012345;

    /* test lots of absolute time values. */
    while( fSampleRate < 90000.0)
    {
        spmSInt32 sampleRate = (spmSInt32) (fSampleRate + 0.5);

        lookupResult = ConvertSampleRateOffset( sampleRate );
        calcResult = CalculateSampleRateOffset( sampleRate );
        printf("sampleRate = 0x%08X = %8d, lookup = %5d, calc = %5d",
               sampleRate, sampleRate, lookupResult, calcResult );
        delta = lookupResult - calcResult;
        delta2 = delta * delta;
        if( delta2 > 20 )
        {
            printf(" - delta = %d", delta );
        }
        printf("\n");

        fSampleRate *= multiplier;
    }
    return 0;
}
#endif

/*********************************************************/
/* Convert MIDI pitch to internal ME2000 pitch format. */
static PitchOctave CalculatePitchOctave(int midiPitch, int fineTune)
{
    /* Despite what the Mobile DLS spec says, the fine tune info seems to be in cents! */
    /* You have to subtract because the value in the file is the amount you have to 
     * subtract to get the listed midiPitch.
     */
    spmSInt32 pitchInCents = (midiPitch * 100) - fineTune;
    /* Convert to fixed point fractional octave value. */
#define CENTS_PER_OCTAVE (1200)
    PitchOctave octavePitch = (pitchInCents << SS_PITCH_SHIFT) / CENTS_PER_OCTAVE;

    /* Offset so we can specify very low LFO pitches. */
    octavePitch += (8 << SS_PITCH_SHIFT);

    return octavePitch;
}

/*********************************************************/
static int DLSParser_ResolveRegion_Internal( DLS_Wave_t **poolTable,
                                    DLS_Region_t *region)
{
    DLS_Wave_t *dlsWave;
    const DLS_WaveSample_t *waveSample;
    WaveTable_t *waveTable = &region->waveTable;
    /* Point to structure in DLS region. */
    region->waveSetRegion.table = waveTable;

    dlsWave = poolTable[region->tableIndex];
    region->dlsWave = dlsWave;

    /* if the region has local wavesample info, use it */
    if( region->waveSample != NULL )
    {
        waveSample = region->waveSample;
    }
    else if( dlsWave->wavesample != NULL )
        /* if the associated dlsWave has wavesample info, use it */
    {
        waveSample = dlsWave->wavesample;
    }
    else
        /* use the default wavesample info */
    {
        waveSample = &sDefaultWaveSample;
    }

    /* Calculate the base pitch for proper tuning on playback. */
    region->waveSetRegion.basePitch = CalculatePitchOctave( waveSample->basePitch, waveSample->fineTune );

    /* point to sample data */
    waveTable->samples = dlsWave->samples;
    waveTable->numSamples = dlsWave->numSamples;

    /* set up loop, if present */
    if( waveSample->loopStart == -1 )
    {
        waveTable->loopBegin = -1;
        waveTable->loopEnd = -1;
    }
    else
    {
        waveTable->loopBegin = waveSample->loopStart;
        waveTable->loopEnd = waveSample->loopStart + waveSample->loopSize;
        if( waveSample->loopType == WLOOP_TYPE_FORWARD )
        {
            /* Tell ME3000 to loop forever by not seeing data past loop end. */
            waveTable->numSamples = waveTable->loopEnd;
        }
    }

    waveTable->sampleRateOffset = ConvertSampleRateOffset( dlsWave->sampleRate );


    return 0;
}

/*********************************************************/
static int DLSParser_ResolveInstrument( DLS_Orchestra_t *orchestra, int insIndex)
{
    DLS_Instrument_t *ins;
    DLS_Region_t *region;
    DLS_Wave_t  **poolTable = orchestra->poolTable;
    int i, result;

    DBUGMSG("Loading instrument ...\n");
    ins = &(orchestra->instruments[insIndex]);

    /* Load Regions for instrument. */
    for( i=0; i<ins->numRegions; i++ )
    {
        DBUGMSG("Loading region ...\n");
        region = &(ins->regions[i]);
        if( ( region == NULL ) ||
            ( region->tableIndex >= (spmUInt32) orchestra->numPoolTableEntries ) )
        {
            return DLSParser_Error_ParseError; /* 050616 */
        }
        result = DLSParser_ResolveRegion( poolTable, region );
        if( result < 0 )
            goto error;
    }
    return SPMIDI_Error_None;

error:
    return result;
}

/*********************************************************/
/**
 * Load DLS instruments into SPMIDI synthesizer.
 */
static SPMIDI_Error DLSParser_Load_Internal( DLSParser *parser, SPMIDI_Context *spmidiContext )
{
    int result;
    DLS_Parser_t *dlsParser;
    DLS_Orchestra_t *dlsOrch;

    if( parser == NULL )
        return SPMIDI_Error_IllegalArgument;

    if( spmidiContext == NULL )
        return SPMIDI_Error_IllegalArgument;

    dlsParser = (DLS_Parser_t *)parser;
    dlsOrch = &dlsParser->orchestra;

    if( dlsOrch->poolTable == NULL )
    {
        result = DLSParser_Error_NotParsed;
        goto error;
    }

    if( dlsOrch->poolTable[0] == NULL )
    {
        result = DLSParser_Error_NotParsed;
        goto error;
    }

    SPMIDI_LoadDLSOrchestra( spmidiContext, dlsOrch );

    result = 0;

error:
    return result;
}

/**
 * Unload DLS instruments from SPMIDI synthesizer.
 */
SPMIDI_Error DLSParser_Unload( DLSParser *parser, SPMIDI_Context *spmidiContext )
{
    /* Unsupported. Once a DLS instrument is loaded it cannot be unloaded. 
     * Just delete the SPMIDI_Context and create a new one.
     */
    TOUCH(parser);
    TOUCH(spmidiContext);
    return 0;
}

/*********************************************************/
static void DLSParser_Delete_Internal( DLSParser *parser )
{
    RiffParser_t *riffParser = NULL;
    DLS_Parser_t *dlsParser = NULL;
    DLS_Orchestra_t *dlsOrch = NULL;
    DLS_Wave_t *dlsWave;

    dlsParser = (DLS_Parser_t *)parser;
    if( dlsParser == NULL )
        return;
    riffParser = &dlsParser->riffParser;
    dlsOrch = &dlsParser->orchestra;

    /* delete instruments in orchestra instrument table */
    DeleteInstruments( dlsOrch );

    /* free instruments table in orchestra */
    SPMIDI_FreeMemory( dlsOrch->instruments );

    /* free waves in orchestra waves list, and their wavetables */
    while( !DLL_IsEmpty( &dlsOrch->waves ) )
    {
        dlsWave = (DLS_Wave_t*)DLL_First( &dlsOrch->waves );
        DLL_RemoveFirst( &dlsOrch->waves );
        DeleteWave( dlsWave );
    }

    /* free poolTable in orchestra */
    SPMIDI_FreeMemory( dlsOrch->poolTable );

    /* free rawPoolTable in orchestra */
    SPMIDI_FreeMemory( dlsOrch->rawPoolTable );

    /* close the associated stream */
    if( dlsParser->sio != NULL )
        Stream_Close( dlsParser->sio );

    SPMIDI_FreeMemory( dlsParser );

    return;
}

/*******************************************************************************/
DLSParser_FunctionTable_t *DLSParser_GetFunctionTable( void )
{
    if( sDLSParserFunctionTable.initialized == 0 )
    {
        /* Load function table at run-time because some relocatable systems
         * cannot resolve compile time pointer initialization.
         */
        sDLSParserFunctionTable.create = DLSParser_Create_Internal;
        sDLSParserFunctionTable.delete = DLSParser_Delete_Internal;
        sDLSParserFunctionTable.parse = DLSParser_Parse_Internal;
        sDLSParserFunctionTable.load = DLSParser_Load_Internal;
        sDLSParserFunctionTable.resolveRegion = DLSParser_ResolveRegion_Internal;
        sDLSParserFunctionTable.deleteInstruments = DeleteInstruments_Internal;
        sDLSParserFunctionTable.deleteRegion = DeleteRegion_Internal;

        sDLSParserFunctionTable.initialized = 1;
    }
    return &sDLSParserFunctionTable;
}

#if 0

/*******************************************************************/
int main(int argc, char* argv[]);
int main(int argc, char* argv[])
{
    DLSParser *dlsParser = NULL;
    char* fileName;
    unsigned char* fileStart;
    spmSInt32 fileSize;

    /* char *DEFAULT_FILENAME = "C:\\WINDOWS\\system32\\drivers\\gm.dls"; */
    /* char *DEFAULT_FILENAME = "E:\\nomad\\MIDISynth\\dls\\want_ya.dls"; */
    /* char *DEFAULT_FILENAME = "E:\\nomad\\MIDISynth\\data\\dls\\TestInstruments.dls"; */
    /* char *DEFAULT_FILENAME = "C:\\business\\mobileer\\data\\wantcha.dls"; */
    char *DEFAULT_FILENAME = "C:\\business\\mobileer\\data\\multi_sine_set.dls";

    int retValue;

    SPMIDI_Initialize();

    PRTMSGNUMD("DLS_WaveSample_t = ", sizeof(DLS_WaveSample_t) );
    PRTMSGNUMD("DLS_Wave_t = ", sizeof(DLS_Wave_t) );
    PRTMSGNUMD("DLS_Articulation_t = ", sizeof(DLS_Articulation_t) );
    PRTMSGNUMD("DLS_Region_t = ", sizeof(DLS_Region_t) );
    PRTMSGNUMD("DLS_Instrument_t = ", sizeof(DLS_Instrument_t) );
    PRTMSGNUMD("DLS_Orchestra_t = ", sizeof(DLS_Orchestra_t) );
        
    fileName = ( argc < 2 ) ? DEFAULT_FILENAME : argv[1];

    fileStart = SPMUtil_LoadFileImage( fileName, (int *)&( fileSize ) );
    if( fileStart != NULL )
    {
        retValue = DLSParser_Create( &dlsParser, fileStart, fileSize );
        if( retValue < 0 )
        {
            PRTMSGNUMD("ERROR: DLSParser_Create returned = ", retValue );
            goto error;
        }
        retValue = DLSParser_Parse( dlsParser );
        if( retValue < 0 )
        {
            PRTMSGNUMD("ERROR: DLSParser_Parse returned = ", retValue );
            goto error;
        }
    }
    else
    {
        DBUGMSG("Error: can't open file ");
        DBUGMSG( fileName );
        retValue = -1;
    }


error:
    DLSParser_Delete( dlsParser );

    SPMIDI_Terminate();

    PRTMSGNUMD("retValue = ", retValue );
    return retValue;
}
#endif

#endif /* #if SPMIDI_ME3000 */
