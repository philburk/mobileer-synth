/* Stream IO - mapped onto standard file I/O.
**
** Author Phil Burk
** Copyright 1998 Phil Burk
*/

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/streamio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "spmidi/include/streamio.h"

    typedef struct FileStreamIO
    {
        StreamIO      sio;
        FILE         *fid;
    }
    FileStreamIO;


    /******************************************************************
    ** Read from Stream
    ** Return number of bytes read.
    */
    static int FileStream_Read( StreamIO *sio, char *buffer, int numBytes )
    {
        FileStreamIO *fsio = (FileStreamIO *) sio;
        return fread( buffer, 1, numBytes, fsio->fid );
    }


    /******************************************************************
    ** Write to Stream
    ** Return number of bytes written.
    */
    static int FileStream_Write( StreamIO *sio, char *buffer, int numBytes )
    {
        FileStreamIO *fsio = (FileStreamIO *) sio;
        return fwrite( buffer, 1, numBytes, fsio->fid );
    }

    /******************************************************************
    ** Seek forwards or backwards in stream.
    */
    static int FileStream_SetPosition( StreamIO *sio, int offset )
    {
        FileStreamIO *fsio = (FileStreamIO *) sio;
        return fseek( fsio->fid, offset, SEEK_SET );
    }

    /******************************************************************
    ** Report current position in stream.
    */
    static int FileStream_GetPosition( StreamIO *sio )
    {
        FileStreamIO *fsio = (FileStreamIO *) sio;
        return ftell( fsio->fid );
    }

    /******************************************************************
    ** Close StreamIO for reading from file.
    */
    static void FileStream_Close( StreamIO *sio )
    {
        if( sio != NULL )
        {
            FileStreamIO *fsio = (FileStreamIO *) sio;
            if( fsio->fid != NULL )
                fclose( fsio->fid );
            free( sio );
        }
    }

    /******************************************************************
    ** Setup StreamIO for reading from file.
    */
    StreamIO *Stream_OpenFile( char *fileName, char *mode )
    {
        FileStreamIO *fsio;
        FILE *fid = fopen( fileName, mode );
        if( fid == NULL )
            return NULL;

        fsio = malloc( sizeof( FileStreamIO ) );
        if( fsio == NULL )
        {
            fclose( fid );
            return NULL;
        }

        /* Initialize pointers to functions for object like interface. */
        fsio->sio.read  = FileStream_Read;
        fsio->sio.write = FileStream_Write;
        fsio->sio.setPosition  = FileStream_SetPosition;
        fsio->sio.getPosition  = FileStream_GetPosition;
        fsio->sio.close = FileStream_Close;
        fsio->fid = fid;

        return &fsio->sio;
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
        StreamIO SIO, *sio;
        sio = &SIO;
        /* Open RAM Stream. */
        Stream_Open( sio, testData, sizeof(testData) );

        /* Test read of 3 bytes. */
        numRead = sio->read( sio, pad, 3 );
        CHECK_VALUE("numRead", 3, numRead );
        for( i=0; i<3; i++ )
            CHECK_VALUE("1st read", 0+i, pad[i] );

        /* Test read of 5 more bytes. */
        sio->read( sio, pad, 5 );
        for( i=0; i<5; i++ )
            CHECK_VALUE("2nd read", 3+i, pad[i] );

        /* Test Tell */
        pos = sio->tell( sio );
        CHECK_VALUE("tell", 8, pos );

        /* Test Seek */
        pos = sio->seek( sio, -3, SEEK_CUR );
        pos = sio->tell( sio );
        CHECK_VALUE("SEEK_CUR", 5, pos );
        sio->read( sio, pad, 3 );
        for( i=0; i<3; i++ )
            CHECK_VALUE("3rd read", 5+i, pad[i] );

        pos = sio->seek( sio, -2, SEEK_END );
        pos = sio->tell( sio );
        CHECK_VALUE("SEEK_END", sizeof(testData)-2, pos );
        numRead = sio->read( sio, pad, 4 );
        CHECK_VALUE("numRead", 2, numRead );
        for( i=0; i<2; i++ )
            CHECK_VALUE("4th read", sizeof(testData)-2+i, pad[i] );

        pos = sio->seek( sio, 4, SEEK_SET );
        pos = sio->tell( sio );
        CHECK_VALUE("SEEK_SET", 4, pos );
        numRead = sio->read( sio, pad, 2 );
        CHECK_VALUE("numRead", 2, numRead );
        for( i=0; i<2; i++ )
            CHECK_VALUE("5th read", 4+i, pad[i] );

        /* Test write of 3 bytes. */
        pos = sio->seek( sio, 3, SEEK_SET );
        pad[0] = 100;
        pad[1] = 101;
        pad[2] = 102;
        sio->write( sio, pad, 3 );
        /* Read data written and a byte on either side. */
        pos = sio->seek( sio, 2, SEEK_SET );
        numRead = sio->read( sio, pad, 5 );
        CHECK_VALUE("numRead", 5, numRead );
        CHECK_VALUE("before write", 2, pad[0] );
        for( i=0; i<3; i++ )
            CHECK_VALUE("wrote", 100+i, pad[i+1] );
        CHECK_VALUE("after write", 6, pad[4] );

        printf("numErrors = %d\n", numErrors );
    }

#ifdef __cplusplus
};
#endif

#endif

