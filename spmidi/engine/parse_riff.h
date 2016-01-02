#ifndef _PARS_RIFF_H
#define _PARS_RIFF_H
/*
 * RIFF parser.
 * Parses a RIFF file image from a stream.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer
 */
#include "spmidi/include/streamio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MakeFourCC(a,b,c,d)  (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))

    /* Define WAV Chunk and FORM types as 4 byte integers. */
#define FOURCC_RIFF   MakeFourCC('R','I','F','F')
#define FOURCC_LIST   MakeFourCC('L','I','S','T')

    typedef unsigned long RIFF_ChunkID;
    typedef int (RiffParserChunkHandler)( void *userData, RIFF_ChunkID chunkID, long chunkSize );

    typedef struct RiffParser_s
    {
        StreamIO *stream;
        RiffParserChunkHandler  *handleBeginForm;
        RiffParserChunkHandler  *handleEndForm;
        RiffParserChunkHandler  *handleBeginList;
        RiffParserChunkHandler  *handleEndList;
        RiffParserChunkHandler  *handleChunk;
        long      total;
        long      startingOffset;
        int       listState;  /* Can be set by user when starting to edit a list. */
        void     *userData;
    }
    RiffParser_t;

    /* Parse a StreamIO of a RIFF file and return information.
     * Call user function with chunks.
     * A zero is returned if no error occurs.
     * A negative number is returned if a parsing error occurs.
     */
    long RIFF_ParseStream( RiffParser_t *riffParser );

    int Stream_ReadIntLittle( StreamIO *stream );

    /** Read 32 bit signed integer assuming Big Endian byte order. */
    int Stream_ReadIntBig( StreamIO *stream );

    /** Read 16 bit signed integer assuming Little Endian byte order. */
    int Stream_ReadShortLittle( StreamIO *stream );

    /** Read 16 bit signed integer assuming Big Endian byte order. */
    int Stream_ReadShortBig( StreamIO *stream );

    /* Parse data from a Little Endian byte stream. */
    /** Read 32 bit signed integer assuming Little Endian byte order. */
#define RIFF_ReadIntLittle( riffParser ) Stream_ReadIntLittle( (riffParser)->stream )

    /** Read 32 bit signed integer assuming Big Endian byte order. */
#define RIFF_ReadIntBig( riffParser ) Stream_ReadIntBig( (riffParser)->stream )

    /** Read 16 bit signed integer assuming Little Endian byte order. */
#define RIFF_ReadShortLittle( riffParser ) Stream_ReadShortLittle( (riffParser)->stream )

    /** Read 16 bit signed integer assuming Big Endian byte order. */
#define RIFF_ReadShortBig( riffParser ) Stream_ReadShortBig( (riffParser)->stream )

#ifdef __cplusplus
};
#endif

#endif /* _PARS_RIFF_H */
