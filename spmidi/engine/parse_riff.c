/*
 * RIFF File Parser
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer Inc
 */

#include "spmidi/include/streamio.h"
#include "spmidi/engine/parse_riff.h"

static int RIFF_ParseChunk( RiffParser_t *riffParser, RIFF_ChunkID chunkID, int chunkSize );
static int RIFF_ParseForm( RiffParser_t *riffParser, int formSize );

/** Read 32 bit signed integer assuming Little Endian byte order. */
int Stream_ReadIntLittle( StreamIO *stream )
{
    unsigned char pad[4];
    stream->read( stream, (char *) pad, sizeof(pad) );
    return (pad[3] << 24) + (pad[2] << 16) + (pad[1] << 8) + pad[0];
}

/** Read 32 bit signed integer assuming Big Endian byte order. */
int Stream_ReadIntBig( StreamIO *stream )
{
    unsigned char pad[4];
    stream->read( stream, (char *) pad, sizeof(pad) );
    return (pad[0] << 24) + (pad[1] << 16) + (pad[2] << 8) + pad[3];
}

/** Read 16 bit signed integer assuming Little Endian byte order. */
int Stream_ReadShortLittle( StreamIO *stream )
{
    unsigned char pad[2];
    stream->read( stream, (char *) pad, sizeof(pad) );
    return (int)((short)((pad[1] << 8) + pad[0]));
}

/** Read 16 bit signed integer assuming Big Endian byte order. */
int Stream_ReadShortBig( StreamIO *stream )
{
    unsigned char pad[2];
    stream->read( stream, (char *) pad, sizeof(pad) );
    return (int)((short)((pad[0] << 8) + pad[1]));
}

/************************************************************************/
long RIFF_ParseStream( RiffParser_t *riffParser )
{
    RIFF_ChunkID chunkID;
    long chunkSize;

    riffParser->startingOffset = riffParser->stream->getPosition( riffParser->stream );
    chunkID = RIFF_ReadIntBig( riffParser );
    chunkSize = RIFF_ReadIntLittle( riffParser );
    riffParser->total = chunkSize + 8;

    return RIFF_ParseChunk( riffParser, chunkID, chunkSize );
}


/* Parse one chunk from IFF file. After calling handler, make sure stream is positioned
 * at end of chunk.
 */
static int RIFF_ParseChunk( RiffParser_t *riffParser, RIFF_ChunkID chunkID, int chunkSize )
{
    int result = 0;
    long startOffset, endOffset;
    int numRead;

    startOffset = riffParser->stream->getPosition( riffParser->stream );

    if( chunkID == FOURCC_RIFF )
    {
        int type = RIFF_ReadIntBig( riffParser );
        result = riffParser->handleBeginForm( riffParser->userData, type, chunkSize - 4 );
        if( result < 0 )
        {
            goto error;
        }
        result = RIFF_ParseForm( riffParser, chunkSize - 4 );
        if( result < 0 )
        {
            goto error;
        }
        result = riffParser->handleEndForm( riffParser->userData, type, 0 );
        if( result < 0 )
        {
            goto error;
        }
    }
    else if( chunkID == FOURCC_LIST )
    {
        /* Save state for nested lists. */
        int savedState = riffParser->listState;
        int type = RIFF_ReadIntBig( riffParser );
        result = riffParser->handleBeginList( riffParser->userData, type, chunkSize - 4 );
        if( result < 0 )
        {
            goto error;
        }
        result = RIFF_ParseForm( riffParser, chunkSize - 4 );
        if( result < 0 )
        {
            goto error;
        }
        result = riffParser->handleEndList( riffParser->userData, type, 0 );
        if( result < 0 )
        {
            goto error;
        }
        riffParser->listState = savedState;
    }
    else
    {
        result = riffParser->handleChunk( riffParser->userData, chunkID, chunkSize );
        if( result < 0 )
        {
            goto error;
        }
    }

    endOffset = riffParser->stream->getPosition( riffParser->stream );
    numRead = (int)(endOffset - startOffset);
    if( (chunkSize & 1) == 1)
    {
        chunkSize++;  /* even-up */
    }
    if( numRead < chunkSize )
    {
        int numToSkip = chunkSize - numRead;
        if( endOffset + numToSkip < riffParser->total )
        {
            result = riffParser->stream->setPosition( riffParser->stream, endOffset + numToSkip );
        }
    }

error:
    return result;
}

/** Parse the FORM and pass the chunks to the ChunkHandler
 * The cursor should be positioned right after the type field.
 */
static int RIFF_ParseForm( RiffParser_t *riffParser, int formSize )
{
    int result = 0;
    long bytesLeft = formSize;
    while( bytesLeft > 0 )
    {
        int chunkID = RIFF_ReadIntBig( riffParser );
        int size = RIFF_ReadIntLittle( riffParser );
        bytesLeft -= 8;
        if( size <= 0 )
        {
            return size;
        }
        result = RIFF_ParseChunk( riffParser, chunkID, size );
        if( result < 0 )
        {
            goto error;
        }
        if( (size & 1) == 1)
        {
            size++;  /* even-up */
        }
        bytesLeft -= size;
    }
error:
    return result;
}

