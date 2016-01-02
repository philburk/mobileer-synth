/*
 * $Id: xmf_parser.c,v 1.32 2007/10/02 16:14:42 philjmsl Exp $
 * XMF File Parser
 *
 * Author: Robert Marsanyi
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/engine/memtools.h"  /* for MemTools_Clear */
#include "spmidi/include/streamio.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/xmf_parser.h"

#include "xmf_parser_internal.h"
#include "spmidi/include/midifile_player.h"  /* for debugging */

/* Only compile if supporting ME3000 API */
#if SPMIDI_ME3000


#ifndef XMF_VERBOSITY
#define XMF_VERBOSITY (0)
#endif

#if  XMF_VERBOSITY
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#define DBUGMSGNUMD(msg,x) { DBUGMSG(msg); DBUGNUMD(x); DBUGMSG("\n"); }
#define DBUGMSGNUMH(msg,x) { DBUGMSG(msg); DBUGNUMH(x); DBUGMSG("\n"); }

#define TOUCH(value) \
    (void)value

/* Function table for patching DLS parser. */
static XMFParser_FunctionTable_t sXMFParserFunctionTable = { 0 };

static int XMF_ParseNode( XmfParser_t *xmfParser );
static long XMF_ReadLimitedVLQ( XmfParser_t *xmfParser, unsigned long max );
static XMFID XMF_ReadID( XmfParser_t *xmfParser );

/************************************************************************
 * These defines let us seek randomly around the stream.
 * SeekAbs places us an absolute distance from the start of the XMF stream
 * SeekRel advances us forward or backward from our current position
 */

#define XMF_SeekAbs( xmfParser, offset ) \
    ( xmfParser->stream->setPosition( (xmfParser)->stream, (offset) ) )

#define XMF_SeekRel( xmfParser, offset ) \
    ( xmfParser->stream->setPosition( xmfParser->stream, \
        (XMF_Tell(xmfParser) + (offset)) ) )

#define XMF_Tell( xmfParser ) \
    ( xmfParser->stream->getPosition( xmfParser->stream ) )


/************************************************************************/
/************** Vectored Functions *************************************/
/************************************************************************/
SPMIDI_Error XMFParser_Create( XMFParser **parserPtr, unsigned char *fileStart, spmSInt32 fileSize )
{
    return XMFParser_GetFunctionTable()->create( parserPtr, fileStart, fileSize );
}
void XMFParser_Delete( XMFParser *xmfParser )
{
    XMFParser_GetFunctionTable()->delete( xmfParser );
}
static int XMF_ParseNode( XmfParser_t *xmfParser )
{
    return XMFParser_GetFunctionTable()->parseNode( xmfParser );
}
static int XMF_HandleFileNode( XmfParser_t *xmfParser, long resourceFormatID )
{
    return XMFParser_GetFunctionTable()->handleFileNode( xmfParser, resourceFormatID );
}
static int XMF_ParseNodeUnpackers( XmfParser_t *xmfParser )
{
    return XMFParser_GetFunctionTable()->parseNodeUnpackers( xmfParser );
}
static int XMF_ParseFileHeader( XmfParser_t *xmfParser )
{
    return XMFParser_GetFunctionTable()->parseFileHeader( xmfParser );
}
static int XMF_FindNodeType( XmfParser_t *xmfParser )
{
    return XMFParser_GetFunctionTable()->findNodeType( xmfParser );
}

/************************************************************************
 * Stream-reading primitives
 * Basic data types are:
 *    VLQ (Variable Length Quantity),
 *    XString (eXtended String),
 *    ID (4-char string encoded as bytes)
 */

#define VLQ_UNLIMITED 0
/************************************************************************/
/* VLQ code lifted from midifile_player.c */
static long XMF_ReadLimitedVLQ( XmfParser_t *xmfParser, unsigned long max )
{
    long value = 0;
    char data;
    int numRead;
    do
    {
        numRead = xmfParser->stream->read(xmfParser->stream, &data, 1);
        if( numRead == 0 )
        {
            value = XMFParser_Error_ParseError;
            break;
        }

        value = (value << 7) + (long)(data & 0x7F);
        if( ((unsigned)value > max) && (max != VLQ_UNLIMITED) )
        {
            value = XMFParser_Error_VLQTooLarge;
            break;
        }
    }
    while( (data & 0x80) == 0x80 );

    return value;
}

/* We might use this later. */
#if 0
/************************************************************************/
static char *XMF_ReadXString( XmfParser_t *xmfParser );
static char *XMF_ReadXString( XmfParser_t *xmfParser )
{
    int len;

    /* Currently unimplemented. */
    /* For now, just return NULL and skip the right number of bytes. */

    len = XMF_ReadLimitedVLQ( xmfParser, VLQ_UNLIMITED );
    if( len > 0 ) XMF_SeekRel( xmfParser, len );
    return NULL;
}
#endif

/************************************************************************/
static XMFID XMF_ReadID( XmfParser_t *xmfParser )
{
    char pad[4];

    xmfParser->stream->read( xmfParser->stream, pad, sizeof(pad) );
    return ( MakeFourCC(pad[0], pad[1], pad[2], pad[3] ) );
}

/************************************************************************
 * This function handles the table defining internationalization issues
 * in the FileHeader. Since we're only expecting SP-MIDI and DLS data,
 * we'll skip it :)
 */
static int XMF_ParseMetaDataTypesTable( XmfParser_t *xmfParser )
{
    int length;

    DBUGMSGNUMH("XMF_ParseMetaDataTypesTable: offset in file = ", XMF_Tell( xmfParser ) );
    length = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
    DBUGMSGNUMH("XMF_ParseMetaDataTypesTable: length = ", length );
    return( length >= 0 ? XMF_SeekRel( xmfParser, length ) : length );
}

/************************************************************************
 * This function skips the NodeMetaData.
 */
static int XMF_SkipNodeMetaData( XmfParser_t *xmfParser )
{
    int length;

    /* Figure out the dimensions of the NodeMetaData table */
    length = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
    DBUGMSGNUMD("----------\nXMF_SkipNodeMetaData: length = ", length );
    return( length >= 0 ? XMF_SeekRel( xmfParser, length ) : length );
}

/************************************************************************
 * This function scans the NodeMetaData for a Resource Format standard ID
 * and returns it, or XMFParser_Error_ParseError if not found.
 */
static int XMF_FindNodeType_Internal( XmfParser_t *xmfParser )
{
    int start, length, result;
    long fieldIDLength, fieldID;
    long endOfFieldContents;
    long numberOfVersions, fieldContentsLength, stringFormatTypeID, formatTypeID;
    int resourceType = XMFParser_Error_ParseError; // valid types are 0 and up, so -ve means "type not found"

    /* Figure out the dimensions of the NodeMetaData table */
    length = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
    DBUGMSGNUMD("----------\nXMF_FindNodeType: length = ", length );
    if( length < 0 ) return length;

    start = XMF_Tell( xmfParser );
    DBUGMSGNUMD("XMF_FindNodeType: offset in file = ", start );
    if( start < 0 ) return start;

    if( (start + length) >= xmfParser->fileLength )
    {
        return XMFParser_Error_ParseError; /* 050616 */
    }

    /* Iterate over MetaDataItems in the table until you find the format, or run out of table */
    while( ( resourceType < 0 ) && ( XMF_Tell( xmfParser ) < (start + length) ) )
    {
        /* Read and parse the FieldSpecifier */
        fieldIDLength = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
        DBUGMSGNUMD("____ item ___ at offset = ", XMF_Tell( xmfParser ) );
        DBUGMSGNUMD("XMF_FindNodeType: fieldIDLength = ", fieldIDLength );
        if( fieldIDLength < 0 ) return fieldIDLength;

        /* This is just for reading the field specifier. */
        if( fieldIDLength == 0 )  /* it's a standard field */
        {
            fieldID = XMF_ReadLimitedVLQ( xmfParser, 1<<8 );  /* gives the FieldID number */
            DBUGMSGNUMD("XMF_FindNodeType: fieldID = ", fieldID );
            if( fieldID < 0 ) return fieldID;
        }
        else // it's a custom field
        {
            fieldID = -1;  /* non-standard FieldID */
            result = XMF_SeekRel( xmfParser, fieldIDLength );
            if( result < 0 ) return result;
        }

        /* Always read and parse the FieldContents */
        numberOfVersions = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
        DBUGMSGNUMD("XMF_FindNodeType: numberOfVersions = ", numberOfVersions );
        if( numberOfVersions < 0 ) return numberOfVersions;

        fieldContentsLength = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
        DBUGMSGNUMD("XMF_FindNodeType: fieldContentsLength = ", fieldContentsLength );
        if( fieldContentsLength < 0 ) return fieldContentsLength;

        endOfFieldContents = XMF_Tell( xmfParser ) + fieldContentsLength;

        /* if we're looking at the right resource format ID */
        if( fieldID == SRF_RESOURCE_FORMAT )
        {
            /* if the contents are type Universal */
            if( numberOfVersions == 0 )
            {
                /* read the format of the first Universal data item */
                stringFormatTypeID = XMF_ReadLimitedVLQ( xmfParser, 1<<4 );
                DBUGMSGNUMD("XMF_FindNodeType: stringFormatTypeID = ", stringFormatTypeID );
                if( stringFormatTypeID < 0 ) return stringFormatTypeID;

                /* if the first item in the Universal data is binary */
                if( ( stringFormatTypeID == SFID_BINARY_VISIBLE  || \
                    stringFormatTypeID == SFID_BINARY_HIDDEN ) )
                {
                    /* it's the FormatTypeID, so read it */
                    formatTypeID = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
                    DBUGMSGNUMD("XMF_FindNodeType: formatTypeID = ", formatTypeID );
                    if( formatTypeID < 0 ) return formatTypeID;

                    /* if the FormatTypeID is the Standard ResourceFormatID */
                    if( formatTypeID == SRF_STANDARD_RESOURCE_FORMAT )
                    {
                        /* we've found what we're looking for, so read it */
                        /*
                        Note: p32 of XMF Spec v 1.00b Oct 10 2001 states that these
                        will all fit into one byte "for the foreseeable future"
                        */

                        resourceType = XMF_ReadLimitedVLQ( xmfParser, 1<<8 );
                        DBUGMSGNUMD("XMF_FindNodeType: FOUND resourceType = ", resourceType );
                        if( resourceType < 0 ) return resourceType;
                    }
                }
            }
        }
        result = XMF_SeekAbs( xmfParser, endOfFieldContents );
        if( result < 0 ) return XMFParser_Error_ParseError;
    }

    DBUGMSGNUMD("XMF_FindNodeType: bytes left at end of metadata = ", (start + length) - XMF_Tell( xmfParser ) );

    /* skip to end of NodeMetaData in NodeHeader */
    result = XMF_SeekAbs( xmfParser, start + length );
    if( result < 0 ) return XMFParser_Error_ParseError;

    return resourceType;
}

/************************************************************************/
static int XMF_ParseFileHeader_Internal( XmfParser_t *xmfParser )
{
    int result;
    int version, fileTypeID, fileTypeRevisionID;
    unsigned long fileID;

    /* Read the File ID */
    fileID = XMF_ReadID( xmfParser );
    DBUGMSGNUMH("XMF_ParseFileHeader: fileID = ", fileID );
    if( fileID != XMF_FILEID )
    {
        result = XMFParser_Error_NotXMF;
        goto error;
    }

    /* Read the XmfMetaFileFormatVersion */
    version = XMF_ReadID( xmfParser );
    DBUGMSGNUMH("XMF_ParseFileHeader: version = ", version );

    /* We now support format version 1.xx XMF files.
     * The difference is: the XmfFileTypeID and XmfFileTypeRevisionID only appear
     * in the file header for XmfMetaFileFormatVersion 2.xx and higher
     */

    if( ((version >> 24) & 0x7F) >= '2')
    {
        /* Read the XmfFileTypeID and FileTypeRevisionIDs (new for XMF 2.0!) */
        fileTypeID = XMF_ReadID( xmfParser );
        DBUGMSGNUMH("XMF_ParseFileHeader: fileTypeID = ", fileTypeID );

        /* We want to throw an error if the fileTypeID is not 2 (=MobileXMF) */
        if( fileTypeID != XMF_MOBILE_FILETYPE )
        {
            result = XMFParser_Error_WrongType;
            goto error;
        }

        /* Try to support future revisions assuming they are backwards compatible. */
        fileTypeRevisionID = XMF_ReadID( xmfParser );
    }

    /* Read the file length */
    xmfParser->fileLength = XMF_ReadLimitedVLQ( xmfParser, 1<<28 );
    DBUGMSGNUMH("XMF_ParseFileHeader: fileLength = ", xmfParser->fileLength );
    if( xmfParser->fileLength < 0 )
    {
        result = xmfParser->fileLength;
        goto error;
    }
    if( xmfParser->fileLength > xmfParser->fileMaxSize )
    {
        result = XMFParser_Error_SizeError;
        goto error;
    }

    /* Read the MetaDataTypes table */
    result = XMF_ParseMetaDataTypesTable( xmfParser );
    if( result < 0 ) goto error;

    /* Read the TreeStart, TreeEnd values */
    xmfParser->treeStart = XMF_ReadLimitedVLQ( xmfParser, 1<<28 );
    DBUGMSGNUMD("XMF_ParseFileHeader: treeStart = ", xmfParser->treeStart );
    if( xmfParser->treeStart < 0 )
    {
        result = xmfParser->treeStart;
        goto error;
    }
    if( xmfParser->treeStart > xmfParser->fileLength )
    {
        result = XMFParser_Error_SizeError;
        goto error;
    }

    xmfParser->treeEnd = XMF_ReadLimitedVLQ( xmfParser, 1<<28 );
    DBUGMSGNUMD("XMF_ParseFileHeader: treeEnd = ", xmfParser->treeEnd );
    if( xmfParser->treeStart < 0 )
    {
        result = xmfParser->treeStart;
        goto error;
    }
    if( xmfParser->treeEnd > xmfParser->fileLength )
    {
        result = XMFParser_Error_SizeError;
        goto error;
    }

error:
    return result;
}

/************************************************************************/
/**
 * Parse the NodeUnpackers structure to determine content length.
 * The only UnpackerID we recognize currently is 0, "No unpacker".
 * @return contentLength or error
 */
static int XMF_ParseNodeUnpackers_Internal( XmfParser_t *xmfParser )
{
    int result = 0, length, decodedSize, endOfStructure;
    int unpackerTypeID, unpackerID;
    char MMAByte;

    length = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
    if( length < 0 ) return length;
    endOfStructure = XMF_Tell( xmfParser ) + length;

    while( XMF_Tell( xmfParser ) < endOfStructure )
    {
        unpackerTypeID = XMF_ReadLimitedVLQ( xmfParser, 1<<2 );
        if( unpackerTypeID < 0 )
        {
            result = unpackerTypeID;
            goto error;
        }

        switch( unpackerTypeID )
        {
        case UPIDTYPE_STANDARD:
            unpackerID = XMF_ReadLimitedVLQ( xmfParser, 1<<8 );
            break;

        case UPIDTYPE_REGISTERED:
        case UPIDTYPE_NONREGISTERED:
            unpackerID = XMF_ReadLimitedVLQ( xmfParser, VLQ_UNLIMITED );
            break;

        case UPIDTYPE_MMA:
            xmfParser->stream->read(xmfParser->stream, &MMAByte, 1);
            if( MMAByte == 0 )  // MMA ID is 3-byte form
            {
                result = XMF_SeekRel( xmfParser, 2 );  // skip the next two bytes
                if( result < 0 ) goto error;
            }
            unpackerID = XMF_ReadLimitedVLQ( xmfParser, VLQ_UNLIMITED );
            break;

        default:
            result = XMFParser_Error_ParseError;
            goto error;
        }
        if( unpackerID < 0 )
        {
            result = unpackerID;
            goto error;
        }

        /* next is the UnpackerDecodedSize */
        decodedSize = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
        if( decodedSize < 0 )
        {
            result = decodedSize;
            goto error;
        }
        if( (unpackerTypeID == UPIDTYPE_STANDARD) && (unpackerID == UPID_NONE) )
        {
            result = decodedSize;
        }
    }

error:
    DBUGMSGNUMD("XMF_ParseNodeUnpackers returns ", result );
    return result;
}


/************************************************************************
 * Given that the stream is pointing to a NodeContents structure, find the
 * associate resource data.
 */
static int XMF_SeekToResource( XmfParser_t *xmfParser )
{
    int result = 0;
    long refTypeID, resourcePos;

    refTypeID = XMF_ReadLimitedVLQ( xmfParser, 1<<3 );
    DBUGMSGNUMD("XMF_SeekToResource: refTypeID = ", refTypeID );
    if( refTypeID < 0 )
    {
        result = refTypeID;
        goto error;
    }

    switch( refTypeID )
    {
    case REFTYPEID_INLINE:

        /* Resource is at current seek position */
        break;

    case REFTYPEID_INFILE:

        /* Resource is pointed to by following reference data */
        resourcePos = XMF_ReadLimitedVLQ( xmfParser, 1<<28 );
        DBUGMSGNUMD("XMF_SeekToResource: resourcePos = ", resourcePos );
        if( resourcePos < 0 )
        {
            result = resourcePos;
            goto error;
        }

        /* Jump to the resource */
        result = XMF_SeekAbs( xmfParser, resourcePos );
        if( result < 0 ) goto error;
        break;

        /* we don't handle any of the following, which aren't part of MobileXMF */
    case REFTYPEID_INNODE:
    case REFTYPEID_ERF:
    case REFTYPEID_EXMF:
        result = XMFParser_Error_DetachedNodeContentFound;
        goto error;

    default:
        result = XMFParser_Error_ParseError;
        goto error;
    }

error:
    return result;
}

/************************************************************************
 * This function gets called for each FileNode.  The stream is positioned in
 * the NodeHeader at the NodeMetaData structure.  The absolute position of
 * the node's NodeContents is passed as an argument.
 */
static int XMF_HandleFileNode_Internal( XmfParser_t *xmfParser, long resourceFormatID )
{
    int result = 0;

    /* call the handler for this node's type */
    switch( resourceFormatID )
    {
    case RFID_SMF_TYPE0:
    case RFID_SMF_TYPE1:
        if( xmfParser->smfOffset != XMF_INVALID_OFFSET )
        {
            result = XMFParser_Error_ExtraSMF;
        }
        else
        {
            xmfParser->smfOffset = XMF_Tell( xmfParser );
            DBUGMSGNUMD("XMF_HandleFileNode: smfOffset = ", xmfParser->smfOffset );
        }
        break;

        /* Mobile XMF disallows any of the following DLS formats, so we'll return an error
         * but we'll set the dlsOffset anyway so we can use the data if we choose to. */
    case RFID_DLS_1:
    case RFID_DLS_2:
    case RFID_DLS_2_1:
        result = XMFParser_Error_WrongDLSType;

    case RFID_MOBILE_DLS:
        xmfParser->dlsOffset = XMF_Tell( xmfParser );
        DBUGMSGNUMD("XMF_HandleFileNode: dlsOffset = ", xmfParser->dlsOffset );
        break;

    default:
        break;
    }

    return result;
}

/************************************************************************
 * This function parses a node in the tree structure, including the
 * RootNode, by handing it off to a handler.  If the Node is a FolderNode,
 * it then recurses through it's child Nodes.
 */
static int XMF_ParseNode_Internal( XmfParser_t *xmfParser )
{
    int result, i;
    long length, numContainedItems, headerLength;
    long resourceFormatID = -1;
    int startPos, contentsPos, endPos;

    /* Determine starting position in stream */
    startPos = XMF_Tell( xmfParser );
    DBUGMSGNUMD("XMF_ParseNode: node starts at ", startPos );

    /* Parse NodeHeader */
    length            = XMF_ReadLimitedVLQ( xmfParser, 1<<28 );
    if( length < 0 ) return length;

    numContainedItems = XMF_ReadLimitedVLQ( xmfParser, 1<<8 );
    if( numContainedItems < 0 ) return numContainedItems;

    headerLength      = XMF_ReadLimitedVLQ( xmfParser, 1<<16 );
    if( headerLength < 0 ) return headerLength;

    DBUGMSGNUMD("XMF_ParseNode: length = ", length );
    DBUGMSGNUMD("XMF_ParseNode: numContainedItems = ", numContainedItems );
    DBUGMSGNUMD("XMF_ParseNode: headerLength = ", headerLength );

    /* Calculate the absolute position of the node's contents */
    contentsPos = startPos + headerLength;

    /* If the node is a FileNode, find its type */
    if( numContainedItems == 0 )
    {
        /* find node's type by scanning its metadata for the resourceFormatID */
        /* This is guaranteed to be there. */
        result = resourceFormatID = XMF_FindNodeType( xmfParser );
        if( result < 0)
            goto error;
    }
    else
    {
        result = XMF_SkipNodeMetaData( xmfParser );
        if( result < 0)
            goto error;
    }

    /* Do we need the NodeUnpackers to determine content length? */
    result = XMF_ParseNodeUnpackers( xmfParser );
    if( result < 0 )
        goto error;

    /* seek to NodeContent */
    result = XMF_SeekAbs( xmfParser, contentsPos );
    if( result < 0 )
        goto error;

    /* find the node's resource data */
    result = XMF_SeekToResource( xmfParser );
    if( result >= 0)
    {
        /* If we're doing a FileNode, pass it to the handler */
        if( numContainedItems == 0 )
        {
            result = XMF_HandleFileNode( xmfParser, resourceFormatID );
            
            /* if there's no error, or the error is WrongDLSType, we're good.
            * We decided to be lenient and try to process DLS nodes of any
            * type, rather than insisting on MobileDLS */
            if( ( result < 0 ) && ( result != XMFParser_Error_WrongDLSType ) )
                goto error;
        }
        /* We're doing a FolderNode, so recurse with the resource */
        else
        {
            for( i=0; i<numContainedItems; i++ )
            {
                result = XMF_ParseNode( xmfParser );
                if( result < 0 )
                {
                    goto error; /* 050616 */
                }
            }
        }
    }
    else if( result != XMFParser_Error_DetachedNodeContentFound )
        goto error;

    /* Seek to the start of the next node, if there is one,
    * in case parser did not go to end. */
    endPos = startPos + length;
    if( endPos < xmfParser->treeEnd )
    {
        if( XMF_Tell( xmfParser ) != endPos )
        {
            result = XMF_SeekAbs( xmfParser, endPos );
        }
    }

error:
    return result;
}

/************************************************************************/
/**** API calls *********************************************************/
/************************************************************************/

/************************************************************************/
int XMFParser_IsXMF( unsigned char *xmfImage )
{
    XMFID id;

    /* read 4 bytes at xmfImage as a long */
    id = MakeFourCC(xmfImage[0], xmfImage[1], xmfImage[2], xmfImage[3]);

    /* if it's the XMF file id, return 1; otherwise, return 0 */
    return( id == XMF_FILEID ? 1 : 0 );
}

/************************************************************************/
static SPMIDI_Error XMFParser_Create_Internal( XMFParser **parserPtr,
                               unsigned char *xmfImage, spmSInt32 numBytes )
{
    XmfParser_t *xmfParser;
    StreamIO *sio;

    /* Allocate a parser structure */
    xmfParser = (XmfParser_t*)SPMIDI_ALLOC_MEM( sizeof( XmfParser_t ), "XmfParser_t" );
    if( xmfParser == NULL )
        return SPMIDI_Error_OutOfMemory;

    MemTools_Clear( xmfParser, sizeof( XmfParser_t ) );

    /* Open a stream on the image */
    sio = Stream_OpenImage( (char *)xmfImage, numBytes );
    if( sio == 0 )
    {
        XMFParser_Delete( (XMFParser *) xmfParser ); /* Avoid leak of parser memory. */
        return SPMIDI_Error_OutOfMemory;
    }

    xmfParser->stream = sio;
    xmfParser->fileMaxSize = numBytes;
    xmfParser->fileStart = xmfImage;
    xmfParser->smfOffset = XMF_INVALID_OFFSET;
    xmfParser->dlsOffset = XMF_INVALID_OFFSET;

    /* Set user's pointer to new xmfParser */
    *parserPtr = (XMFParser *)xmfParser;

    return 0;
}

/************************************************************************/
SPMIDI_Error XMFParser_Parse( XMFParser *parser )
{
    int result;
    XmfParser_t *xmfParser = (XmfParser_t *)parser;

    if( parser == NULL )
        return( SPMIDI_Error_IllegalArgument );

    /* Record our starting position in the parser's associated stream */
    xmfParser->startingOffset = XMF_Tell( xmfParser );

    /* Parse FileHeader */
    result = XMF_ParseFileHeader( xmfParser );
    if( result < 0 )
        goto error;

    /* Advance to start of tree node */
    result = XMF_SeekAbs( xmfParser, xmfParser->treeStart );
    if( result < 0 )
        goto error;

    /* Parse Root node of tree */
    result = XMF_ParseNode( xmfParser );

error:
    return result;
}

/************************************************************************/
unsigned char *XMFParser_GetSMF( XMFParser *parser, spmSInt32 *maxSizePtr )
{
    unsigned char *result;
    XmfParser_t *xmfParser = (XmfParser_t *)parser;

    if( xmfParser == NULL )
        return NULL;

    if( xmfParser->fileStart && ( xmfParser->smfOffset != -1 ) )
    {
        *maxSizePtr = xmfParser->fileLength - xmfParser->smfOffset;
        result = xmfParser->fileStart + xmfParser->smfOffset;
    }
    else
    {
        result = NULL;
    }

    return result;
}

/************************************************************************/
unsigned char *XMFParser_GetDLS( XMFParser *parser, spmSInt32 *maxSizePtr )
{
    unsigned char *result;
    XmfParser_t *xmfParser = (XmfParser_t *)parser;

    if( xmfParser == NULL )
        return NULL;

    if( xmfParser->fileStart && ( xmfParser->dlsOffset != -1 ) )
    {
        *maxSizePtr = xmfParser->fileLength - xmfParser->dlsOffset;
        result = xmfParser->fileStart + xmfParser->dlsOffset;
    }
    else
    {
        result = NULL;
    }

    return result;
}

/************************************************************************/
static void XMFParser_Delete_Internal( XMFParser *parser )
{
    XmfParser_t *xmfParser = (XmfParser_t *)parser;
    StreamIO *sio;

    if( xmfParser == NULL )
        return;

    /* Close the stream */
    sio = xmfParser->stream;
    if( sio != NULL )
    {
        Stream_Close( sio );
    }

    /* Delete the structure */
    SPMIDI_FreeMemory( parser );
}

/*******************************************************************************/
XMFParser_FunctionTable_t *XMFParser_GetFunctionTable( void )
{
    if( sXMFParserFunctionTable.initialized == 0 )
    {
        /* Load function table at run-time because some relocatable systems
         * cannot resolve compile time pointer initialization.
         */
        sXMFParserFunctionTable.create = XMFParser_Create_Internal;
        sXMFParserFunctionTable.delete = XMFParser_Delete_Internal;
        sXMFParserFunctionTable.parseNode = XMF_ParseNode_Internal;
        sXMFParserFunctionTable.handleFileNode = XMF_HandleFileNode_Internal;
        sXMFParserFunctionTable.parseNodeUnpackers = XMF_ParseNodeUnpackers_Internal;
        sXMFParserFunctionTable.parseFileHeader = XMF_ParseFileHeader_Internal;
        sXMFParserFunctionTable.findNodeType = XMF_FindNodeType_Internal;

        sXMFParserFunctionTable.initialized = 1;
    }
    return &sXMFParserFunctionTable;
}



#if 0
/************************************************************************/
int main( int argc, char *argv[] )
{
    int result;
    XMFParser *xmfParser = NULL;
    unsigned char *fileStart;
    int fileSize;
    unsigned char *smf, *dls;
    long smfSize, dlsSize;

    char *xmfFileName;
    char *DEFAULT_XMFFILE = "C:\\business\\mobileer\\data\\beatnik_xmf.mxmf";

    xmfFileName = ( argc < 2 ) ? DEFAULT_XMFFILE : argv[1];

    PRTMSG( "parsexmf: parse xmf file\n" );

    /* Load the file into memory */
    fileStart = SPMUtil_LoadFileImage( xmfFileName, &( fileSize ) );

    /* Verify the file says it's XMF */
    result = XMFParser_IsXMF( fileStart );
    if( result != 1 )
    {
        result = XMFParser_Error_NotXMF;
        goto error;
    }

    /* Create a parser pointing at the image */
    result = XMFParser_Create( &xmfParser, fileStart, fileSize );
    if( result < 0 )
        goto error;

    /* Parse the image */
    result = XMFParser_Parse( xmfParser );
    if( result < 0 )
        goto error;

    /* Show results */
    dls = XMFParser_GetDLS( xmfParser, &dlsSize );
    PRTMSG( "DLS at " );
    PRTNUMH( dls );
    PRTMSG( ", size " );
    PRTNUMD( dlsSize );
    PRTMSG( "\n" );

    smf = XMFParser_GetSMF( xmfParser, &smfSize );
    PRTMSG( "SMF at " );
    PRTNUMH( smf );
    PRTMSG( ", size " );
    PRTNUMD( smfSize );
    PRTMSG( "\n" );
    MIDIFile_Print( smf, smfSize );

error:
    if( xmfParser != NULL )
        XMFParser_Delete( xmfParser );
    return result;
}
#endif

#endif /* SPMIDI_ME3000 */
