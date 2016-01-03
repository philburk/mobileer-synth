/* $Id: read_wav_stream.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * WAV parser.
 * Parses a WAV file image from a StreamIO
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#ifdef WIN32
#include <memory.h>
#endif

#include "spmidi/engine/memtools.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/streamio.h"
#include "spmidi/include/read_wav.h"

/** Read 32 bit unsigned integer assuming Little Endian byte order. */
spmUInt32 ParseLongLE( spmUInt8 **addrPtr )
{
    spmUInt8 *addr = *addrPtr;
    int n = *addr++;
    n += (((int) *addr++) << 8);
    n += (((int) *addr++) << 16);
    n += (((int) *addr++) << 24);
    *addrPtr = addr;
    return (spmUInt32) n;
}

/** Read 32 bit unsigned integer assuming Big Endian byte order. */
spmUInt32 ParseLongBE( spmUInt8 **addrPtr )
{
    spmUInt8 *addr = *addrPtr;
    int n = (((int) *addr++) << 24);
    n += (((int) *addr++) << 16);
    n += (((int) *addr++) << 8);
    n += *addr++;
    *addrPtr = addr;
    return (spmUInt32) n;
}

/** Read 16 bit unsigned integer assuming Little Endian byte order. */
spmUInt16 ParseShortLE( spmUInt8 **addrPtr )
{
    spmUInt8 *addr = *addrPtr;
    int n = *addr++;
    n += (((int) *addr++) << 8);
    *addrPtr = addr;
    return (spmUInt16) n;
}

/* Parse a StreamIO of a WAV file and return information in an AudioSample structure.
 * A zero is returned if no error occurs.
 * A negative number is returned if a parsing error occurs.
 */
long Audio_WAV_ParseSampleStream(
    StreamIO *sio,             /* Stream access to WAV file. */
    AudioSample *asmp         /* Pre-allocated but empty structure to be completely filled in by parser. */
    )
{
    spmUInt32     chunkType;
    spmSInt32     chunkSize;
    spmUInt32     extendedSize;
    spmUInt32     formType;
    spmUInt32     bytesLeft;
    int           format;
    int           result;
    spmSInt32     numFactSamples = -1;
    spmUInt32 nextPos;
    int           numRead;
    unsigned char *addr;
#define PAD_SIZE  (128)
    unsigned char pad[PAD_SIZE];  /* Read stream data into this pad. */

    MemTools_Clear( asmp, sizeof(AudioSample) );

    /* Read 12 bytes of FORM header from WAV */
    numRead = sio->read( sio, (char *) pad, 12 );
    if( numRead != 12 )
    {
        goto unexpected_end;
    }
    addr = pad;

    /* Parse RIFF header. */
    chunkType = ParseLongBE( &addr );
    if( chunkType != RIFF_ID )
    {
        return WAV_ERR_FILE_TYPE;
    }

    bytesLeft = ParseLongLE( &addr );

    formType = ParseLongBE( &addr );
    bytesLeft -= 4;
    if( formType != WAVE_ID )
    {
        return WAV_ERR_FILE_TYPE;
    }

    nextPos = sio->getPosition( sio );

    while( bytesLeft > 8 )
    {
        /* Seek to beginning of next chunk. */
        result = sio->setPosition( sio, nextPos );
        if( result < 0 )
        {
            goto unexpected_end;
        }

        /* Read 8 bytes of chunk header from WAV */
        numRead = sio->read( sio, (char *) pad, 8 );
        if( numRead != 8 )
        {
            goto unexpected_end;
        }
        addr = pad;

        chunkType = ParseLongBE( &addr );
        chunkSize = ParseLongLE( &addr );
        bytesLeft -= 8;
        nextPos = sio->getPosition( sio ) + chunkSize;
        bytesLeft -= chunkSize;
        /* Round up if odd. */
        if( (((long)(nextPos)) & 1) == 1)
        {
            nextPos++;
            bytesLeft--;
        }

        /* Handle relevant chunk types encountered in WAV file. */
        switch( chunkType )
        {

            /* Parse format chunk and fill in AudioSample structure. */
        case FMT_ID:
            /* Read chunk from WAV */
            if( chunkSize > PAD_SIZE )
                goto pad_too_small;
            numRead = sio->read( sio, (char *) pad, chunkSize );
            if( numRead != chunkSize )
            {
                goto unexpected_end;
            }
            addr = pad;

            format = ParseShortLE( &addr );
            asmp->samplesPerFrame = (unsigned char) ParseShortLE( &addr );
            asmp->frameRate = ParseLongLE( &addr );
            addr += 4; /* skip dwAvgBytesPerSec */
            asmp->bytesPerBlock = ParseShortLE( &addr ); /* read asmp->bytesPerBlock, is bytesPerFrame for PCM */
            asmp->bitsPerSample = (unsigned char) ParseShortLE( &addr );

            switch(format)
            {
            case WAVE_FORMAT_PCM:
                asmp->format = AUDIO_FORMAT_PCM;
                asmp->samplesPerBlock = (8 * asmp->bytesPerBlock) / asmp->bitsPerSample ;
                break;

            case WAVE_FORMAT_IMA_ADPCM:
                asmp->format = AUDIO_FORMAT_IMA_ADPCM_WAV;
                extendedSize = ParseShortLE( &addr ); /* read size of ADPCM extension */
                asmp->samplesPerBlock = ParseShortLE( &addr );
                break;

            default:
                return WAV_ERR_FORMAT_TYPE;
            }

            break;

            /* Point AudioSample structure to current position in WAV image. */
        case DATA_ID:
            asmp->maxNumberOfBytes = chunkSize;
            asmp->numberOfBytes = chunkSize;
            asmp->dataOffset = sio->getPosition( sio );
            break;

        case FACT_ID:
            /* Read chunk from WAV */
            if( chunkSize > PAD_SIZE )
                goto pad_too_small;
            numRead = sio->read( sio, (char *) pad, chunkSize );
            if( numRead != chunkSize )
            {
                goto unexpected_end;
            }
            addr = pad;

            numFactSamples = ParseLongLE( &addr );
            break;

        default:
            break;
        }

    }

    /* Calculate number of frames. */
    if( asmp->bytesPerBlock == 0 )
        return WAV_ERR_ILLEGAL_VALUE;
    if( numFactSamples > 0 )
    {
        asmp->numberOfFrames = numFactSamples / asmp->samplesPerFrame;
    }
    else
    {
        asmp->numberOfFrames = (asmp->samplesPerBlock * asmp->numberOfBytes) /
                                    (asmp->samplesPerFrame * asmp->bytesPerBlock);
    }

    return 0;

pad_too_small:
    return WAV_ERR_CHUNK_SIZE;

unexpected_end:
    return WAV_ERR_TRUNCATED;
}
