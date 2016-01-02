/* $Id: streamdls.c,v 1.2 2007/10/02 16:24:51 philjmsl Exp $
 *
 * Use DLS Parser to generate a stream in proprietary StreamDLS format.
 * This can be played by a StreamDLS player.
 *
 * Author: Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/dls_parser.h"
#include "dls_parser_internal.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"

#include "sdls_parser_internal.h"
#include <stdio.h>

#define SAMPLE_RATE 22050

/* FIXME: this is a duplication of the structure in dls_parser.c, which should be shared */
static DLS_WaveSample_t sDefaultDLSWaveSample =
    {
        60, /* basePitch */
        0, /* loopType */
        0, /* fineTune */
        0, /* gain */
        -1, /* loopStart */
        0 /* loopSize */
    };

#define DLS2_ABSPITCH_5HZ     (-55791973)
#define DLS2_RELPITCH_OCTAVE  (78643200)
#define DLS2_ABSTIME_10MSEC   (-522494111)
#define DLS2_ABSTIME_ZERO     (0x80000000)

/* FIXME: This is duplicated from spmidi_dls.c */
/* Declare these as const to avoid RW data.
 * These contain linked list nodes but we won't be using them.
 */
const DLS_Articulation_t sDefaultDLSArticulations[] =
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

#define NUM_DEFAULT_ARTICULATIONS (sizeof(sDefaultDLSArticulations) / sizeof( DLS_Articulation_t ) )

int Stream_WriteIntLittle( StreamIO *stream, spmSInt32 value )
{
    unsigned char pad[4];
    pad[0] = (unsigned char)( value & 0xFF );
    pad[1] = (unsigned char)( value >> 8 ) & 0xFF;
    pad[2] = (unsigned char)( value >> 16 ) & 0xFF;
    pad[3] = (unsigned char)( value >> 24 ) & 0xFF;
    return stream->write( stream, (char *) pad, sizeof(pad) );
}

int Stream_WriteIntBig( StreamIO *stream, spmSInt32 value )
{
    unsigned char pad[4];
    pad[3] = (unsigned char)( value & 0xFF );
    pad[2] = (unsigned char)( value >> 8 ) & 0xFF;
    pad[1] = (unsigned char)( value >> 16 ) & 0xFF;
    pad[0] = (unsigned char)( value >> 24 ) & 0xFF;
    return stream->write( stream, (char *) pad, sizeof(pad) );
}

int Stream_WriteShortLittle( StreamIO *stream, spmSInt16 value )
{
    unsigned char pad[2];
    pad[0] = (unsigned char)( value & 0xFF );
    pad[1] = (unsigned char)( value >> 8 ) & 0xFF;
    return stream->write( stream, (char *) pad, sizeof(pad) );
}

/**
 * StreamDLS_Header()
 * Send header chunk
 * Format is:
 *   ID: 'MXMF'
 *   majorVersion number
 *   minorVersion number
 *   sampleRate
 */
int StreamDLS_Header( spmSInt32 sampleRate, StreamIO *outputStream )
{
    int result;

    result = Stream_WriteIntBig( outputStream, FOURCC_MXMF );
    result += Stream_WriteShortLittle( outputStream, SDLS_MAJOR_VERSION );
    result += Stream_WriteShortLittle( outputStream, SDLS_MINOR_VERSION );
    result += Stream_WriteIntLittle( outputStream, sampleRate );
    if( result == sizeof(spmSInt32) + sizeof(spmSInt16) + sizeof(spmSInt16) + sizeof(spmSInt32) )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * StreamDLS_Waves()
 * Send wave chunks
 */
int StreamDLS_Waves( DLS_Orchestra_t *dlsOrchestra, StreamIO *outputStream )
{
    int i;
    char nullByte = 0x00;
    DLS_Wave_t *dlsWave;

    /* Send WAVG header with count of waves */
    Stream_WriteIntBig( outputStream, FOURCC_WAVG );
    Stream_WriteIntLittle( outputStream, dlsOrchestra->numPoolTableEntries );

    /* Iterate over pool table, sending information for each referenced DLS_Wave_t */
    for( i=0; i<dlsOrchestra->numPoolTableEntries; i++ )
    {
        Stream_WriteIntBig( outputStream, FOURCC_WAVE );

        dlsWave = dlsOrchestra->poolTable[i];

        Stream_WriteIntLittle( outputStream, dlsWave->sampleRate );
        Stream_WriteShortLittle( outputStream,dlsWave->format );
        Stream_WriteShortLittle( outputStream, dlsWave->numChannels );
        Stream_WriteShortLittle( outputStream, dlsWave->bytesPerSample );

        /* Send the sample count and data */
        Stream_WriteIntLittle( outputStream, dlsWave->numSamples );
        outputStream->write( outputStream, dlsWave->samples, dlsWave->numSamples * dlsWave->bytesPerSample );

        /* if sample data is odd number of bytes, send a 0x00 pad to even it up */
        if( (dlsWave->numSamples * dlsWave->bytesPerSample) % 2 == 1 )
        {
            outputStream->write( outputStream, &(nullByte), 1 );
        }
    }

    return 0;
}

/**
 * StreamDLS_LookupWave()
 * Match wave to pooltable, return index or -1 if not found
 */
int StreamDLS_LookupWave( DLS_Orchestra_t *dlsOrchestra, DLS_Wave_t *dlsWave )
{
    int i;

    for( i=0; i<dlsOrchestra->numPoolTableEntries; i++ )
    {
        if( dlsOrchestra->poolTable[i] == dlsWave ) break;
    }

    return ( i<dlsOrchestra->numPoolTableEntries ? i : -1 );
}

/**
 * StreamDLS_Instruments()
 * Send instrument chunks with regions
 */
int StreamDLS_Instruments( DLS_Orchestra_t *dlsOrchestra, StreamIO *outputStream )
{
    DLS_Instrument_t *dlsInst;
    DLS_Region_t     *dlsRegion;
    DLS_Wave_t       *dlsWave;
    WaveTable_t      *waveTable;
    int i, j;
    int waveIndex;

    /* Send INSG header */
    Stream_WriteIntBig( outputStream, FOURCC_INSG );
    Stream_WriteIntLittle( outputStream, dlsOrchestra->numInstruments );

    /* Send each instrument */
    for( i=0; i<dlsOrchestra->numInstruments; i++)
    {
        Stream_WriteIntBig( outputStream, FOURCC_INST );

        dlsInst = &(dlsOrchestra->instruments[i]);
        Stream_WriteShortLittle( outputStream, dlsInst->bankID );
        Stream_WriteShortLittle( outputStream, dlsInst->programID );
        Stream_WriteIntLittle( outputStream, dlsInst->numRegions );

        /* Send each region */
        for( j=0; j<dlsInst->numRegions; j++ )
        {
            Stream_WriteIntBig( outputStream, FOURCC_REGN );

            dlsRegion = &(dlsInst->regions[j]);

            /* Send index of associated DLS_Wave */
            dlsWave = dlsRegion->dlsWave;
            waveIndex = StreamDLS_LookupWave( dlsOrchestra, dlsWave );
            if( waveIndex < 0 ) goto errExit;
            Stream_WriteShortLittle( outputStream, (spmSInt16)waveIndex );
            
            /* Send associated WaveTable_t information */
            waveTable = &(dlsRegion->waveTable);
            Stream_WriteIntLittle( outputStream, waveTable->numSamples );
            Stream_WriteIntLittle( outputStream, waveTable->basePitch );
            Stream_WriteIntLittle( outputStream, waveTable->sampleRateOffset );
            Stream_WriteIntLittle( outputStream, waveTable->loopBegin);
            Stream_WriteIntLittle( outputStream, waveTable->loopEnd );
            Stream_WriteShortLittle( outputStream, waveTable->type );

            /* Send remaining region info */
            Stream_WriteShortLittle( outputStream, dlsRegion->lowestNote );
            Stream_WriteShortLittle( outputStream, dlsRegion->highestNote );
            Stream_WriteShortLittle( outputStream, dlsRegion->lowestVelocity );
            Stream_WriteShortLittle( outputStream, dlsRegion->highestVelocity );
        }
    }

    return 0;

errExit:
    return DLSParser_Error_ParseError;

}

/**
 * StreamDLS_Articulations()
 * Send articulation chunks
 */
int StreamDLS_Articulations( DLS_Orchestra_t *dlsOrchestra, spmUInt32 sampleRate, StreamIO *outputStream )
{
    int i,j,k;
    spmSInt32 converted;
    DLS_Instrument_t *inst;
    DLS_ArticulationTracker_t *instArts, *regionArts;
    DLS_Region_t *region;

    Stream_WriteIntBig( outputStream, FOURCC_ARTG );

    /* Send default articulations: note - they need to be converted to ME3000 values */
    Stream_WriteIntBig( outputStream, FOURCC_ARTD );
    Stream_WriteIntLittle( outputStream, NUM_DEFAULT_ARTICULATIONS );
    for( i=0; i<NUM_DEFAULT_ARTICULATIONS; i++)
    {
        Stream_WriteIntLittle( outputStream, sDefaultDLSArticulations[i].token );
        converted = DLSParser_ConvertArticulationData( &(sDefaultDLSArticulations[i]), sampleRate );
        Stream_WriteIntLittle( outputStream, converted );
    }

    /* For each instrument, send instrument articulations followed by region articulations */
    /* These were converted already when the articulations were loaded in ART chunks */
    for( i=0; i<dlsOrchestra->numInstruments; i++ )
    {
        inst = &(dlsOrchestra->instruments[i]);
        instArts = &(inst->articulations);

        Stream_WriteIntBig( outputStream, FOURCC_ARTI );
        Stream_WriteIntLittle( outputStream, instArts->numArticulations );

        for( j=0; j<instArts->numArticulations; j++ )
        {
            Stream_WriteIntLittle( outputStream, (instArts->articulations)[j].token );
            Stream_WriteIntLittle( outputStream, (instArts->articulations)[j].scale );
        }

        /* Now send region articulations */
        for( j=0; j<inst->numRegions; j++ )
        {
            region = &(inst->regions[j]);
            regionArts = &(region->articulations);

            Stream_WriteIntBig( outputStream, FOURCC_ARTR );
            Stream_WriteIntLittle( outputStream, regionArts->numArticulations );

            for( k=0; k<regionArts->numArticulations; k++ )
            {
                Stream_WriteIntLittle( outputStream, (regionArts->articulations)[k].token );
                Stream_WriteIntLittle( outputStream, (regionArts->articulations)[k].scale );
            }
        }
    }

    return 0;
}

/**
 * StreamDLS_Footer()
 * Send footer chunk
 * Format is:
 *   ID: 'EOMX'
 *   checksum
 */
int StreamDLS_Footer( StreamIO *outputStream )
{
    Stream_WriteIntBig( outputStream, FOURCC_EOMX );
    
    /* TODO: checksum */
    return 0;
}

/**
 * StreamDLS()
 * Given a parsed DLS parser in memory and an open output stream,
 * parse the DLS file, then traverse the result and output
 * it in StreamDLS format.
 */
int StreamDLS( DLSParser *dlsParser, StreamIO *outputStream )
{
    int result;
    spmSInt32 sampleRate;
    DLS_Orchestra_t *dlsOrchestra;
    
    /* Retrieve the orchestra from the parser internal structure */
    if( (dlsOrchestra = DLSParser_GetOrchestra( dlsParser )) == NULL )
    {
        result = DLSParser_Error_NotParsed;
        goto errExit;
    }

    /* Send header */
    if( (result = DLSParser_GetSampleRate( dlsParser )) < 0 ) goto errExit;
    sampleRate = result;
    if( (result = StreamDLS_Header( sampleRate, outputStream )) < 0 ) goto errExit;

    /* Send wave array */
    if( (result = StreamDLS_Waves( dlsOrchestra, outputStream )) < 0 ) goto errExit;

    /* Send instrument array */
    if( (result = StreamDLS_Instruments( dlsOrchestra, outputStream )) < 0 ) goto errExit;

    /* Send articulations */
    if( (result = StreamDLS_Articulations( dlsOrchestra, sampleRate, outputStream )) < 0 ) goto errExit;

    /* Send footer */
    if( (result = StreamDLS_Footer( outputStream )) < 0 ) goto errExit;

errExit:
    return result;
}

#if 1
/****************************************************************/
static void usage( void )
{
    printf("streamdls inFile\n");
    printf("Parses DLS file and outputs result in StreamDLS format\n");
    fflush(stdout);
}

/*******************************************************************/
int main(int argc, char **argv);
int main(int argc, char **argv)
{
    int             result;
    DLSParser      *dlsParser = NULL;
    unsigned char  *inputFileStart = NULL;
    int             inputFileSize;
    StreamIO       *outputStream = NULL;

#define DATADIR "C:\\business\\mobileer\\data"

    char *DEFAULT_FILENAME = DATADIR"\\dls\\ConditionalArticulationTest.dls";
    char *inputFileName = DEFAULT_FILENAME;
    char *outputFileName;

    /* Parse command line. */
    if( argc != 3 )
    {
        usage();
        return 1;
    }
    else
    {
        inputFileName = argv[1];
        outputFileName = argv[2];
    }

    SPMIDI_Initialize();

    /* Load input file into memory */
    inputFileStart = SPMUtil_LoadFileImage( inputFileName, &( inputFileSize ) );
    if( inputFileStart == NULL )
    {
        printf("ERROR: file %s not found.\n", inputFileName );
        result = 1;
        goto errExit;
    }

    /* Open output stream */
    outputStream = Stream_OpenFile( outputFileName, "wb" );
    if( outputStream == NULL )
    {
        printf("ERROR: can't open output stream on file %s.\n", outputFileName );
        result = 1;
        goto errExit;
    }

    /* Create DLS Parser */
    if( (result = DLSParser_Create( &dlsParser, inputFileStart, inputFileSize )) < 0 ) goto errExit;

    /* Set sample rate */
    if( (result = DLSParser_SetSampleRate( dlsParser, SAMPLE_RATE )) < 0 ) goto errExit;

    /* Parse input file */
    if( (result = DLSParser_Parse( dlsParser )) < 0 ) goto errExit;

    /* Stream result to output stream */
    if( (result = StreamDLS( dlsParser, outputStream )) < 0 ) goto errExit;

errExit:
    if( result < 0 )
    {
        fprintf( stderr, "ERROR %i: %s\n", result, SPMUtil_GetErrorText( result ) );
    }

    /* Delete DLS parser, if created */
    if( dlsParser != NULL ) DLSParser_Delete( dlsParser );

    /* Terminate SPMIDI */
    SPMIDI_Terminate();

    /* Close output stream if open */
    if( outputStream != NULL ) Stream_Close( outputStream );

    /* Unload input file if loaded */
    if( inputFileStart != NULL ) SPMUtil_FreeFileImage( inputFileStart );

    return result;
}
#endif
