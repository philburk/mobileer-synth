/* $Id: load_sdls.c,v 1.2 2007/10/02 16:24:51 philjmsl Exp $
 *
 * Reconstruct DLS Orchestra from SDLS stream.  This can then be loaded into ME3000.
 *
 * Author: Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/dls_parser.h"
#include "dls_parser_internal.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_errors.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/engine/spmidi_dls.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/engine/instrument_mgr.h"
#include "spmidi/engine/memtools.h"

#include "sdls_parser_internal.h"
#include <stdio.h>

typedef struct SDLSParser_s
{
    DLS_Orchestra_t    *orchestra;
    spmSInt32           numWaves;
    DLS_Wave_t         *waves;
    spmUInt32           sampleRate;  /* from stream */
} SDLSParser;

/* From spmidi_dls, routine to apply converted articulation to region */
void SSDLS_ApplyArticulationToRegion( DLS_ArticulationTokens token,
                                            spmSInt32 scale, DLS_Region_t *region );

/**
 * SDLSParser_Create
 * Create an SDLSParser structure
 */

int SDLSParser_Create( SDLSParser **parser )
{
    void *mem;

    mem = SPMIDI_ALLOC_MEM( sizeof(SDLSParser), "SDLSParser" );
    if( mem == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
    MemTools_Clear( mem, sizeof(SDLSParser) );

    *parser = (SDLSParser *)mem;
    return 0;
}

/**
 * Parse_Header()
 * parse header chunk
 * Format is:
 *   ID: 'MXMF'
 *   majorVersion number
 *   minorVersion number
 *   sampleRate
 */
int Parse_Header( StreamIO *inputStream, SDLSParser *parser )
{
    int chunkID;
    int majorVersion, minorVersion;
    void *mem;

    chunkID = Stream_ReadIntBig( inputStream );
    if( chunkID != FOURCC_MXMF ) return SDLSStream_Error_NotSDLS;

    majorVersion = Stream_ReadShortLittle( inputStream );
    if( majorVersion != SDLS_MAJOR_VERSION ) return SDLSStream_Error_WrongVersion;

    minorVersion = Stream_ReadShortLittle( inputStream );
    parser->sampleRate = Stream_ReadIntLittle( inputStream );

    /* Allocate DLS Orchestra, point parser at it */
    mem = SPMIDI_ALLOC_MEM( sizeof(DLS_Orchestra_t), "DLS_Orchestra_t" );
    if( mem == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
    MemTools_Clear( mem, sizeof(DLS_Orchestra_t) );

    parser->orchestra = (DLS_Orchestra_t*)mem;

    return 0;
}

/**
 * Parse_Waves()
 * Build pool table, data table from wave info in stream.  Waves are indexed sequentially, and
 * index numbers are referenced by downstream instruments/regions.
 */
int Parse_Waves( StreamIO *inputStream, SDLSParser *parser )
{
    DLS_Wave_t       *dlsWave;
    int               i, numSampleBytes;

    /* Verify chunk ID */
    if( Stream_ReadIntBig( inputStream ) != FOURCC_WAVG ) return DLSParser_Error_ParseError;

    /* Allocate wave table */
    parser->numWaves = Stream_ReadIntLittle( inputStream );
    parser->waves = SPMIDI_ALLOC_MEM( parser->numWaves * sizeof(DLS_Wave_t), "dls wave table" );
    if( parser->waves == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
    MemTools_Clear( parser->waves, parser->numWaves * sizeof(DLS_Wave_t) );

    /* Iterate over pool table, consuming information for each referenced DLS_Wave_t */
    for( i=0; i<parser->numWaves; i++ )
    {
        if( Stream_ReadIntBig( inputStream ) != FOURCC_WAVE ) return DLSParser_Error_ParseError;

        /* Use ith entry in wave table */
        dlsWave = &(parser->waves[i]);

        dlsWave->sampleRate = Stream_ReadIntLittle( inputStream );
        dlsWave->format = (spmUInt16)Stream_ReadShortLittle( inputStream );
        dlsWave->numChannels = (spmUInt8)Stream_ReadShortLittle( inputStream );
        dlsWave->bytesPerSample = (spmUInt8)Stream_ReadShortLittle( inputStream );

        /* Consume the sample count and data */
        dlsWave->numSamples = Stream_ReadIntLittle( inputStream );
        numSampleBytes = dlsWave->numSamples * dlsWave->bytesPerSample;
        
        /* Even-up the count to whole 16-bit words */
        if( numSampleBytes % 2 == 1 ) numSampleBytes += 1;

        dlsWave->samples = SPMIDI_ALLOC_MEM( numSampleBytes, "wavetable" );
        if( dlsWave->samples == NULL )
        {
            return SPMIDI_Error_OutOfMemory;
        }
        inputStream->read( inputStream, dlsWave->samples, numSampleBytes );
    }
    return 0;
}

/**
 * Parse_Instruments()
 * Allocate and fill DLS_Instruments, DLS_Regions.  Resolve pointers to associated
 * waves.
 */
int Parse_Instruments( StreamIO *inputStream, SDLSParser *parser )
{
    DLS_Orchestra_t  *orchestra = parser->orchestra;
    DLS_Instrument_t *dlsInst;
    DLS_Region_t     *dlsRegion;
    DLS_Wave_t       *dlsWave;
    WaveTable_t      *waveTable;
    int i, j;
    int waveIndex;

    /* Verify chunk ID */
    if( Stream_ReadIntBig( inputStream ) != FOURCC_INSG ) return DLSParser_Error_ParseError;

    /* Allocate instrument table */
    orchestra->numInstruments = Stream_ReadIntLittle( inputStream );
    orchestra->instruments = SPMIDI_ALLOC_MEM( orchestra->numInstruments * sizeof(DLS_Instrument_t), "instrument table" );
    if( orchestra->instruments == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
    MemTools_Clear( orchestra->instruments, orchestra->numInstruments * sizeof(DLS_Instrument_t) );

    /* Consume each instrument */
    for( i=0; i<orchestra->numInstruments; i++)
    {
        /* Verify chunk ID */
        if( Stream_ReadIntBig( inputStream ) != FOURCC_INST ) return DLSParser_Error_ParseError;

        dlsInst = &(orchestra->instruments[i]);
        dlsInst->bankID = (spmUInt16)Stream_ReadShortLittle( inputStream );
        dlsInst->programID = (spmUInt16)Stream_ReadShortLittle( inputStream );

        /* Allocate region table for this instrument */
        dlsInst->numRegions = Stream_ReadIntLittle( inputStream );
        dlsInst->regions = SPMIDI_ALLOC_MEM( dlsInst->numRegions * sizeof(DLS_Region_t), "region table" );
        if( dlsInst->regions == NULL )
        {
            return SPMIDI_Error_OutOfMemory;
        }
        MemTools_Clear( dlsInst->regions, dlsInst->numRegions * sizeof(DLS_Region_t) );

        /* Consume each region */
        for( j=0; j<dlsInst->numRegions; j++ )
        {
            /* Verify chunk ID */
            if( Stream_ReadIntBig( inputStream ) != FOURCC_REGN ) return DLSParser_Error_ParseError;

            dlsRegion = &(dlsInst->regions[j]);

            /* Find the associated DLS_Wave, and set the region pointer to it */
            waveIndex = Stream_ReadShortLittle( inputStream );
            if( waveIndex > parser->numWaves ) goto errExit;
            dlsWave = &(parser->waves[waveIndex]);
            dlsRegion->dlsWave = dlsWave;

            /* Consume WaveTable info */
            waveTable = &(dlsRegion->waveTable);
            waveTable->samples = dlsWave->samples;
            waveTable->numSamples = Stream_ReadIntLittle( inputStream  );
            waveTable->basePitch = Stream_ReadIntLittle( inputStream );
            waveTable->sampleRateOffset = Stream_ReadIntLittle( inputStream );
            waveTable->loopBegin = Stream_ReadIntLittle( inputStream );
            waveTable->loopEnd = Stream_ReadIntLittle( inputStream );
            waveTable->type = (spmSInt8)Stream_ReadShortLittle( inputStream );

            /* Consume remaining Region info */
            dlsRegion->lowestNote = (spmSInt8)Stream_ReadShortLittle( inputStream );
            dlsRegion->highestNote = (spmSInt8)Stream_ReadShortLittle( inputStream );
            dlsRegion->lowestVelocity = (spmSInt8)Stream_ReadShortLittle( inputStream );
            dlsRegion->highestVelocity = (spmSInt8)Stream_ReadShortLittle( inputStream );
        }
    }

    return 0;

errExit:
    return DLSParser_Error_ParseError;
}

/**
 * Parse_Articulations()
 * First we get the global articulations, and apply them to all regions.  Then
 * we get the instrument articulations, and apply them to regions within the
 * indicated instruments.  Then we get region articulations, and apply them within
 * to indicated regions.
 */
int Parse_Articulations( StreamIO *inputStream, SDLSParser *parser )
{
    DLS_Orchestra_t  *dlsOrchestra = parser->orchestra;
    DLS_Instrument_t *dlsInst;
    DLS_Region_t     *dlsRegion;
    HybridVoice_Preset_t *preset;
    int i,j,k;
    int numArticulations;
    DLS_ArticulationTokens token;
    spmSInt32 scale;

    /* Verify chunk ID */
    if( Stream_ReadIntBig( inputStream ) != FOURCC_ARTG ) return DLSParser_Error_ParseError;

    /* Consume default articulations */
    if( Stream_ReadIntBig( inputStream ) != FOURCC_ARTD ) return DLSParser_Error_ParseError;
    numArticulations = Stream_ReadIntLittle( inputStream );
    for( i=0; i<numArticulations; i++)
    {
        token = Stream_ReadIntLittle( inputStream );
        scale = Stream_ReadIntLittle( inputStream );
        
        /* iterate over instruments, regions */
        for( j=0; j<dlsOrchestra->numInstruments; j++ )
        {
            dlsInst = &(dlsOrchestra->instruments[j]);
            for( k=0; k<dlsInst->numRegions; k++ )
            {
                dlsRegion = &(dlsInst->regions[k]);

                preset = &(dlsRegion->hybridPreset);
                InsManager_InitializePreset( preset );

                /* Set preset based on DLS instrument. */
                preset->mainOsc.waveform = WAVETABLE;

                SSDLS_ApplyArticulationToRegion( token, scale, dlsRegion );
            }
        }
    }

    /* Consume instrument articulations */
    for( i=0; i<dlsOrchestra->numInstruments; i++ )
    {
        dlsInst = &(dlsOrchestra->instruments[i]);

        if( Stream_ReadIntBig( inputStream ) != FOURCC_ARTI ) return DLSParser_Error_ParseError;
        numArticulations = Stream_ReadIntLittle( inputStream );

        for( j=0; j<numArticulations; j++ )
        {
            token = Stream_ReadIntLittle( inputStream );
            scale = Stream_ReadIntLittle( inputStream );
        
            for( k=0; k<dlsInst->numRegions; k++ )
            {
                dlsRegion = &(dlsInst->regions[k]);
                SSDLS_ApplyArticulationToRegion( token, scale, dlsRegion );
            }
        }

        /* Consume region articulations for this instrument */
        for( j=0; j<dlsInst->numRegions; j++ )
        {
            dlsRegion = &(dlsInst->regions[j]);

            if( Stream_ReadIntBig( inputStream ) != FOURCC_ARTR ) return DLSParser_Error_ParseError;
            numArticulations = Stream_ReadIntLittle( inputStream );

            for( k=0; k<numArticulations; k++ )
            {
                token = Stream_ReadIntLittle( inputStream );
                scale = Stream_ReadIntLittle( inputStream );
                SSDLS_ApplyArticulationToRegion( token, scale, dlsRegion );
            }
        }
    }

    return 0;
}

/**
 * Parse_Footer()
 */
int Parse_Footer( StreamIO *inputStream )
{
    /* Verify chunk ID */
    if( Stream_ReadIntBig( inputStream ) != FOURCC_EOMX ) return DLSParser_Error_ParseError;

    return 0;
}

/**
 * SDLSParser_Parse
 * Parse input stream into a DLS Orchestra, assuming SDLS format
 */
int SDLSParser_Parse( StreamIO *inputStream, SDLSParser *parser )
{
    int result;

    /* Consume header */
    if( (result = Parse_Header( inputStream, parser )) < 0 ) goto errExit;

    /* Consume wave array */
    if( (result = Parse_Waves( inputStream, parser )) < 0 ) goto errExit;

    /* Consume instrument array */
    if( (result = Parse_Instruments( inputStream, parser )) < 0 ) goto errExit;

    /* Consume articulations */
    if( (result = Parse_Articulations( inputStream, parser )) < 0 ) goto errExit;

    /* Consume footer */
    if( (result = Parse_Footer( inputStream )) < 0 ) goto errExit;

errExit:
    return result;
}

/**
 * SDLSParser_Load
 * Load the orchestra into the midi context ready to play.
 */
int SDLSParser_Load( SDLSParser *sdlsParser, SPMIDI_Context *spmidiContext )
{
    DLS_Orchestra_t *dlsOrch = sdlsParser->orchestra;
    HybridSynth_t *hybridSynth;

    if( dlsOrch == NULL ) return DLSParser_Error_NotParsed;

    hybridSynth = (HybridSynth_t *)SPMIDI_GetSynth( spmidiContext );
    if( hybridSynth == NULL ) return SPMIDI_Error_NotStarted;

    hybridSynth->dlsOrchestra = dlsOrch;

    return 0;
}

/**
 * SDLSParser_Delete
 * Delete the parser
 */
void SDLSParser_Delete( SDLSParser *sdlsParser )
{
    if( sdlsParser != NULL )
    {
        if( sdlsParser->waves != NULL ) SPMIDI_FreeMemory( sdlsParser->waves );
        if( sdlsParser->orchestra != NULL ) SPMIDI_FreeMemory( sdlsParser->orchestra );

        SPMIDI_FreeMemory( sdlsParser );
    }
}

#if 1
/****************************************************************/
#define SAMPLES_PER_FRAME   (2)
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
static short sSampleBuffer[SAMPLES_PER_BUFFER];

static void usage( void )
{
    printf("loadsdls inFile\n");
    printf("Parses SDLS file and plays result\n");
    fflush(stdout);
}

/* from play_midifile.c example */
static int PlayFile( SPMIDI_Context *spmidiContext, void *fileStart, int fileSize )
{
    MIDIFilePlayer *player;
    int sampleRate = SPMIDI_GetSampleRate( spmidiContext );
    int result, go = 1, timeout;

    /* Create MIDIFilePlayer */
    if( (result = MIDIFilePlayer_Create( &player, sampleRate, fileStart, fileSize )) < 0 ) goto errExit;
    
    /* Start audio */
    if( (result = SPMUtil_StartVirtualAudio( sampleRate, NULL, SAMPLES_PER_FRAME )) < 0 ) goto errExit;

    /*
     * Play until we hit the end of all tracks.
     * Tell the MIDI player to move forward on all tracks
     * by a time equivalent to a buffers worth of frames.
     * Generate one buffers worth of data and write it to the output stream.
     */
    while( go )
    {
    
        result = MIDIFilePlayer_PlayFrames( player, spmidiContext, SPMIDI_GetFramesPerBuffer()  );
        if( result < 0 )
        {
            goto errExit;
        }
        else if( result > 0 )
        {
            go = 0;
        }

        /* Synthesize samples and fill buffer. */
        SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_GetFramesPerBuffer(), SAMPLES_PER_FRAME, 16 );
        SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, SPMIDI_GetFramesPerBuffer() );
    }

    /* Wait for sound to die out. */
    timeout = (sampleRate * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
    while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
    {
        SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, SPMIDI_GetFramesPerBuffer(), SAMPLES_PER_FRAME, 16 );

        SPMUtil_WriteVirtualAudio( sSampleBuffer, SAMPLES_PER_FRAME, SPMIDI_GetFramesPerBuffer() );
    }

errExit:
    SPMUtil_StopVirtualAudio();

    /* Delete MIDI player, if created */
    if( player != NULL ) MIDIFilePlayer_Delete( player );

    return result;
}

int main(int argc, char *argv[])
{
    int             result;
    SPMIDI_Context *spmidiContext;
    SDLSParser     *sdlsParser = NULL;
    unsigned char  *midiFileStart = NULL;
    int             midiFileSize;
    StreamIO       *inputStream = NULL;

#define DATADIR "C:\\business\\mobileer\\data"
    char *DEFAULT_SDLS_FILENAME = DATADIR"\\sdls\\output.sdls";
    char *DEFAULT_MIDI_FILENAME = DATADIR"\\sdls\\views3.mid";

    char *sdlsFileName = DEFAULT_SDLS_FILENAME;
    char *midiFileName = DEFAULT_MIDI_FILENAME;

    /* Parse command line. */
    if( argc == 3 )
    {
        sdlsFileName = argv[1];
        midiFileName = argv[2];
    }
    else
    {
        if( argc != 1 )
        {
            usage();
            return 1;
        }
    }

    SPMIDI_Initialize();

    /* Load the file into memory */
    midiFileStart = SPMUtil_LoadFileImage( midiFileName, &( midiFileSize ) );
    if( midiFileStart == NULL )
    {
        printf("ERROR: MIDI file %s not found.\n", midiFileName );
        return 1;
    }

    /* Open input file as stream */
    inputStream = Stream_OpenFile( sdlsFileName, "rb" );
    if( inputStream == NULL )
    {
        printf("ERROR: can't open input stream on file %s.\n", sdlsFileName );
        result = 1;
        goto errExit;
    }

    /* Create SDLS parser */
    if( (result = SDLSParser_Create( &sdlsParser )) < 0 ) goto errExit;

    /* Parse input file */
    if( (result = SDLSParser_Parse( inputStream, sdlsParser )) < 0 ) goto errExit;

    /* Start synthesis engine with default number of voices. */
    result = SPMIDI_CreateContext(  &spmidiContext, sdlsParser->sampleRate );
    if( result < 0 )
    {
        goto errExit;
    }

    /* Load result to ME3000 */
    if( (result = SDLSParser_Load( sdlsParser, spmidiContext )) < 0 ) goto errExit;

    /* Play the file */
    if( (result = PlayFile( spmidiContext, midiFileStart, midiFileSize )) < 0 ) goto errExit;

    printf("File playback complete.\n");

errExit:
    if( result < 0 )
    {
        fprintf( stderr, "ERROR %i: %s\n", result, SPMUtil_GetErrorText( result ) );
    }

    /* Delete SDLS parser, if created */
    if( sdlsParser != NULL ) SDLSParser_Delete( sdlsParser );

    /* Terminate SPMIDI */
    SPMIDI_Terminate();

    /* Close input stream if open */
    if( inputStream != NULL ) Stream_Close( inputStream );

    /* Unload midifile if loaded */
    if( midiFileStart != NULL ) SPMUtil_FreeFileImage( midiFileStart );

    return result;
}
#endif
