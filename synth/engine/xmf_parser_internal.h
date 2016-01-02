#ifndef _PARS_XMF_H
#define _PARS_XMF_H
/*
 * XMF parser.
 * Parses an XMF image from an in-memory image.
 *
 * This code is only used by the ME3000 API.
 * It is not used by the ME1000 or ME2000.
 *
 * Author: Robert Marsanyi, Phil Burk
 * Copyright 2005 Mobileer
 */
#include "spmidi/include/streamio.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/xmf_parser.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* File Header constants are four-character strings, which we represent as integers */
#define MakeFourCC(a,b,c,d)  (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))
#define XMF_FILEID       MakeFourCC('X', 'M', 'F', '_')
#define XMF_MOBILE_FILETYPE     (2)
#define XMF_INVALID_OFFSET      (-1)

    /* MetaData Standard FieldIDs */
#define RFID_SMF_TYPE0   (0)
#define RFID_SMF_TYPE1   (1)
#define RFID_DLS_1       (2)
#define RFID_DLS_2       (3)
#define RFID_DLS_2_1     (4)
#define RFID_MOBILE_DLS  (5) // Added with XMF 2.1

    /* NodeContent ReferenceTypeIDs */
#define REFTYPEID_INLINE (1)
#define REFTYPEID_INFILE (2)
#define REFTYPEID_INNODE (3)
#define REFTYPEID_ERF    (4)
#define REFTYPEID_EXMF   (5)

    /* StringFormatTypeIDs */
#define SFID_BINARY_VISIBLE   (6)
#define SFID_BINARY_HIDDEN    (7)

    /* ResourceFormat ID and Standard ResourceFormat ID */
#define SRF_RESOURCE_FORMAT            (3)
#define SRF_STANDARD_RESOURCE_FORMAT   (0)

    /* UnpackerIDTypes */
#define UPIDTYPE_STANDARD      (0)
#define UPIDTYPE_MMA           (1)
#define UPIDTYPE_REGISTERED    (2)
#define UPIDTYPE_NONREGISTERED (3)

    /* UnpackerIDs */
#define UPID_NONE             (0)

    typedef long XMFID;

    typedef struct XmfParser_s
    {
        StreamIO               *stream;
        unsigned char          *fileStart;
        spmSInt32               fileMaxSize;
        spmSInt32               startingOffset;
        /** Parsed from XMF image. */
        long                    fileLength;
        long                    treeStart;
        long                    treeEnd;

        long                    dlsOffset;
        long                    smfOffset;
        long                    reserved;
    }
    XmfParser_t;

    /* Patch table for internal use. */
    typedef struct XMFParser_FunctionTable_s
    {
        int initialized;

        SPMIDI_Error (*create)( XMFParser **parserPtr,
                               unsigned char *xmfImage, spmSInt32 numBytes );

        void (*delete)( XMFParser *parser );

        int (*parseNode)( XmfParser_t *xmfParser );

        int (*handleFileNode)( XmfParser_t *xmfParser, long resourceFormatID );

        int (*parseNodeUnpackers)( XmfParser_t *xmfParser );

        int (*parseFileHeader)( XmfParser_t *xmfParser );

        int (*findNodeType)( XmfParser_t *xmfParser );

    }
    XMFParser_FunctionTable_t;


    /**
     * Get patch table.
     */
    XMFParser_FunctionTable_t *XMFParser_GetFunctionTable( void );

#endif

