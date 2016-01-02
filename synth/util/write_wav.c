/* $Id: write_wav.c,v 1.9 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Write WAV formatted audio to a file.
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/wav_format.h"

typedef struct WAV_Writer_s
{
    FILE *fid;
    /* Offset in file for data size. */
    int   dataSizeOffset;
    int   dataSize;
    int   samplesPerFrame;
}
WAV_Writer;

/* Write long word data to a little endian format byte array. */
void WriteLongLE( unsigned char **addrPtr, unsigned long data )
{
    unsigned char *addr = *addrPtr;
    *addr++ =  (unsigned char) data;
    *addr++ =  (unsigned char) (data>>8);
    *addr++ =  (unsigned char) (data>>16);
    *addr++ =  (unsigned char) (data>>24);
    *addrPtr = addr;
}

/* Write short word data to a little endian format byte array. */
void WriteShortLE( unsigned char **addrPtr,  unsigned short data )
{
    unsigned char *addr = *addrPtr;
    *addr++ =  (unsigned char) data;
    *addr++ =  (unsigned char) (data>>8);
    *addrPtr = addr;
}

/* Write IFF ChunkType data to a byte array. */
static void WriteChunkType( unsigned char **addrPtr, unsigned long cktyp )
{
    unsigned char *addr = *addrPtr;
    *addr++ =  (unsigned char) (cktyp>>24);
    *addr++ =  (unsigned char) (cktyp>>16);
    *addr++ =  (unsigned char) (cktyp>>8);
    *addr++ =  (unsigned char) cktyp;
    *addrPtr = addr;
}

#define WAV_HEADER_SIZE (4 + 4 + 4 + /* RIFF+size+WAVE */ \
        4 + 4 + 16 + /* fmt chunk */ \
        4 + 4 ) /* data chunk */


/* Allocate statically to avoid malloc() */
static WAV_Writer sWAV_Writer;

/*********************************************************************************/
int Audio_WAV_CreateWriter( WAV_Writer **writerPtr, const char *fileName )
{
    WAV_Writer *writer = &sWAV_Writer;
    writer->fid = fopen( fileName, "wb" );
    if( writer->fid == NULL )
    {
        return -1;
    }
    else
    {
        *writerPtr = writer;
        return 0;
    }
}
/*********************************************************************************/
void Audio_WAV_DeleteWriter( WAV_Writer *writer )
{
    (void) writer;
}

/*********************************************************************************
 * Open named file and write WAV header to the file.
 * The header includes the DATA chunk type and size.
 * Returns number of bytes written to file or negative error code.
 */
long Audio_WAV_OpenWriter( WAV_Writer *writer, int frameRate, int samplesPerFrame )
{
    unsigned int  bytesPerSecond;
    unsigned char header[ WAV_HEADER_SIZE ];
    unsigned char *addr = header;
    int numWritten;

    writer->dataSize = 0;

    /* Write RIFF header. */
    WriteChunkType( &addr, RIFF_ID );

    /* Write RIFF size as zero for now. Will patch later. */
    WriteLongLE( &addr, 0 );

    /* Write WAVE form ID. */
    WriteChunkType( &addr, WAVE_ID );

    /* Write format chunk based on AudioSample structure. */
    WriteChunkType( &addr, FMT_ID );
    WriteLongLE( &addr, 16 );
    WriteShortLE( &addr, WAVE_FORMAT_PCM );
    bytesPerSecond = frameRate * samplesPerFrame * sizeof( short);
    WriteShortLE( &addr, (short) samplesPerFrame );
    WriteLongLE( &addr, frameRate );
    WriteLongLE( &addr,  bytesPerSecond );
    WriteShortLE( &addr, (short) (samplesPerFrame * sizeof( short)) ); /* bytesPerBlock */
    WriteShortLE( &addr, (short) 16 ); /* bits per sample */

    /* Write ID and size for 'data' chunk. */
    WriteChunkType( &addr, DATA_ID );
    /* Save offset so we can patch it later. */
    writer->dataSizeOffset = (int) (addr - header);
    WriteLongLE( &addr, 0 );

    numWritten = fwrite( header, 1, sizeof(header), writer->fid );
    if( numWritten != sizeof(header) )
        return -1;

    return (int) numWritten;
}

/*********************************************************************************
 * Write to the data chunk portion of a WAV file.
 * Returns bytes written or negative error code.
 */
long Audio_WAV_WriteShorts( WAV_Writer *writer,
                            short *samples,
                            int numSamples
                          )
{
    unsigned char buffer[2];
    unsigned char *bufferPtr;
    int i;
    short *p = samples;
    int numWritten;
    int bytesWritten;

    for( i=0; i<numSamples; i++ )
    {
        bufferPtr = buffer;
        WriteShortLE( &bufferPtr, *p++ );
        numWritten = fwrite( buffer, 1, sizeof( buffer), writer->fid );
        if( numWritten != sizeof(buffer) )
            return -1;
    }
    bytesWritten = numSamples * sizeof(short);
    writer->dataSize += bytesWritten;
    return (int) bytesWritten;
}

/*********************************************************************************
 * Close WAV file.
 * Update chunk sizes so it can be read by audio applications.
 */
long Audio_WAV_CloseWriter( WAV_Writer *writer )
{
    unsigned char buffer[4];
    unsigned char *bufferPtr;
    int numWritten;
    int riffSize;

    /* Go back to beginning of file and update DATA size */
    int result = fseek( writer->fid, writer->dataSizeOffset, SEEK_SET );
    if( result < 0 )
    {
        return result;
    }

    bufferPtr = buffer;
    WriteLongLE( &bufferPtr, writer->dataSize );
    numWritten = fwrite( buffer, 1, sizeof( buffer), writer->fid );
    if( numWritten != sizeof(buffer) )
    {
        return -1;
    }

    /* Update RIFF size */
    result = fseek( writer->fid, 4, SEEK_SET );
    if( result < 0 )
    {
        return result;
    }

    riffSize = writer->dataSize + (WAV_HEADER_SIZE - 8);
    bufferPtr = buffer;
    WriteLongLE( &bufferPtr, riffSize );
    numWritten = fwrite( buffer, 1, sizeof( buffer), writer->fid );
    if( numWritten != sizeof(buffer) )
    {
        return -1;
    }

    fclose( writer->fid );
    writer->fid = NULL;

    return writer->dataSize;
}

/*********************************************************************************
 * Simple test that write a sawtooth waveform to a file.
 */
#if 0
int main( void )
{
    int i;
    WAV_Writer *writer;
    int result;
#define NUM_SAMPLES  (200)

    short data[NUM_SAMPLES];
    short saw = 0;

    for( i=0; i<NUM_SAMPLES; i++ )
    {
        data[i] = saw;
        saw += 293;
    }

    result = Audio_WAV_CreateWriter( &writer, "rendered_midi.wav" );
    if( result < 0 )
        goto error;

    result =  Audio_WAV_OpenWriter( writer, 44100, 1 );
    if( result < 0 )
        goto error;

    for( i=0; i<15; i++ )
    {
        result =  Audio_WAV_WriteShorts( writer, data, NUM_SAMPLES );
        if( result < 0 )
            goto error;
    }

    result =  Audio_WAV_CloseWriter( writer );
    if( result < 0 )
        goto error;

    Audio_WAV_DeleteWriter( writer );

    return 0;

error:
    printf("ERROR: result = %d\n", result );
    return result;
}
#endif
