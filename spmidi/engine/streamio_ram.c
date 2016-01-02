/* $Id: streamio_ram.c,v 1.7 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 * Stream IO - emulate file I/O from in-memory char arrays.
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/streamio.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"


typedef struct ImageStreamIO_s
{
    StreamIO    sio;
    char       *bytePtr;
    int         numBytes;
    int         cursor;
}
ImageStreamIO_t;

static int ImageStream_Read( StreamIO *sio, char *buffer, int numBytes );
static int ImageStream_Write( StreamIO *sio, char *buffer, int numBytes );
static int ImageStream_GetPosition( StreamIO *sio );
static int ImageStream_SetPosition( StreamIO *sio, int offset );
static void ImageStream_Close( StreamIO *sio );
static char *ImageStream_GetAddress( StreamIO *sio );

/******************************************************************
** Setup StreamIO for reading from RAM.
*/
StreamIO *Stream_OpenImage( char *dataPtr, int numBytes )
{
    ImageStreamIO_t *ramsio;
    StreamIO *sio;

    ramsio = (ImageStreamIO_t *) SPMIDI_ALLOC_MEM( sizeof( ImageStreamIO_t ), "ImageStreamIO_t" );
    if( ramsio == NULL )
    {
        return NULL;
    }

    sio = (StreamIO *) ramsio;
    /* Initialize pointers to functions for object like interface. */
    sio->read  = ImageStream_Read;
    sio->write = ImageStream_Write;
    sio->setPosition  = ImageStream_SetPosition;
    sio->getPosition  = ImageStream_GetPosition;
    sio->close = ImageStream_Close;
    sio->getAddress = ImageStream_GetAddress;
    /* Initialize data for RAM I/O. */
    ramsio->numBytes = numBytes;
    ramsio->cursor = 0;
    ramsio->bytePtr = dataPtr;

    return sio;
}
/******************************************************************
** Close StreamIO for reading from file.
*/
static void ImageStream_Close( StreamIO *sio )
{
    if( sio != NULL )
    {
        SPMIDI_FreeMemory( sio );
    }
}

/******************************************************************
** Read from Stream
** Return number of bytes read.
*/
static int ImageStream_Read( StreamIO *sio, char *buffer, int numBytes )
{
    ImageStreamIO_t *ramsio = (ImageStreamIO_t *) sio;
    char *s,*d;
    int i;
    int numRead = ramsio->numBytes - ramsio->cursor;
    if( numRead > numBytes )
        numRead = numBytes;
    if( ramsio->bytePtr == NULL )
        return -1;
    d = buffer;
    s = &ramsio->bytePtr[ramsio->cursor];
    for( i=0; i<numRead; i++ )
        *d++ = *s++;
    ramsio->cursor += numRead;
    return numRead;
}


/******************************************************************
** Write to Stream
** Return number of bytes written.
*/
static int ImageStream_Write( StreamIO *sio, char *buffer, int numBytes )
{
    ImageStreamIO_t *ramsio = (ImageStreamIO_t *) sio;
    char *s,*d;
    int i;
    int numWritten = ramsio->numBytes - ramsio->cursor;
    if( numWritten > numBytes )
        numWritten = numBytes;
    if( ramsio->bytePtr == NULL )
        return -1;
    s = buffer;
    d = &ramsio->bytePtr[ramsio->cursor];
    for( i=0; i<numWritten; i++ )
        *d++ = *s++;
    ramsio->cursor += numWritten;
    return numWritten;
}

/******************************************************************
** Seek forwards or backwards in stream.
*/
static int ImageStream_SetPosition( StreamIO *sio, int offset )
{
    ImageStreamIO_t *ramsio = (ImageStreamIO_t *) sio;
    int original = ramsio->cursor;
    if( (offset < 0) || (offset >= ramsio->numBytes) )
        return -1;
    ramsio->cursor = offset;
    return original;
}

/******************************************************************
** Report current position in stream.
*/
static int ImageStream_GetPosition( StreamIO *sio )
{
    ImageStreamIO_t *ramsio = (ImageStreamIO_t *) sio;
    return ramsio->cursor;
}

/******************************************************************
** Report current position in stream.
*/
static char *ImageStream_GetAddress( StreamIO *sio )
{
    ImageStreamIO_t *ramsio = (ImageStreamIO_t *) sio;
    return ramsio->bytePtr;
}

#if 0
/* Test StreamIO
*/

char testData[] = { 0,1,2,3,4,5,6,7,8,9,
                    10,11,12,13,14,15,16,17,18,19 };

#define CHECK_VALUE( msg, expected, actual  ) \
if( actual == expected ) printf("SUCCESS: %s, %d \n", msg, actual ); \
else printf("ERROR #%d: %s, expect %d, got %d\n", numErrors++, msg, expected, actual );

int main( int argc, char **argv )
{
    int numErrors = 0;
    int i,pos, numRead;
    unsigned char pad[8];
    StreamIO  *sio;
    /* Open RAM Stream. */
    sio = Stream_OpenImage( testData, sizeof(testData) );

    /* Test read of 3 bytes. */
    numRead = sio->read( sio, pad, 3 );
    CHECK_VALUE("numRead", 3, numRead );
    for( i=0; i<3; i++ )
        CHECK_VALUE("1st read", 0+i, pad[i] );

    /* Test read of 5 more bytes. */
    Stream_Read( sio, pad, 5 );
    for( i=0; i<5; i++ )
        CHECK_VALUE("2nd read", 3+i, pad[i] );

    /* Test Tell */
    pos = sio->getPosition( sio );
    CHECK_VALUE("tell", 8, pos );

    /* Test Seek */
    pos = sio->setPosition( sio, pos - 3 );
    pos = sio->getPosition( sio );
    CHECK_VALUE("SEEK pos-3", 5, pos );
    sio->read( sio, pad, 3 );
    for( i=0; i<3; i++ )
        CHECK_VALUE("3rd read", 5+i, pad[i] );


    pos = sio->setPosition( sio, 4 );
    pos = sio->getPosition( sio );
    CHECK_VALUE("SEEK 4", 4, pos );
    numRead = sio->read( sio, pad, 2 );
    CHECK_VALUE("numRead", 2, numRead );
    for( i=0; i<2; i++ )
        CHECK_VALUE("5th read", 4+i, pad[i] );

    /* Test write of 3 bytes. */
    pos = sio->setPosition( sio, 3 );
    pad[0] = 100;
    pad[1] = 101;
    pad[2] = 102;
    sio->write( sio, pad, 3 );
    /* Read data written and a byte on either side. */
    pos = sio->setPosition( sio, 2 );
    numRead = sio->read( sio, pad, 5 );
    CHECK_VALUE("numRead", 5, numRead );
    CHECK_VALUE("before write", 2, pad[0] );
    for( i=0; i<3; i++ )
        CHECK_VALUE("wrote", 100+i, pad[i+1] );
    CHECK_VALUE("after write", 6, pad[4] );

    printf("numErrors = %d\n", numErrors );
}

#endif


