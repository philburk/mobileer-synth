/* $Id: spmidi_load.c,v 1.8 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Load a file in to a memory image.
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi_load.h"

void SPMUtil_FreeFileImage( void *image )
{
    free( image );
}

/****************************************************************/
void *SPMUtil_LoadFileImage( const char *fileName, int *sizePtr )
{
    FILE *fid = NULL;
    void *data = NULL;
    int  fileSize;
    int  numRead;

    /* Open file. */
    fid = fopen( fileName, "rb" );
    if( fid == NULL )
    {
        printf("SPMUtil_LoadFileImage: Can't open file %s\n", fileName );
        goto error;
    }

    /* Determine length of file. */
    fseek( fid, 0, SEEK_END );
    fileSize = ftell( fid );
    fseek( fid, 0, SEEK_SET );

    /* Allocate memory for file. */
    data = malloc( fileSize );
    if( data == NULL )
    {
        printf("SPMUtil_LoadFileImage: Can't allocate %d bytes for holding file.\n", fileSize );
        goto error;
    }

    /* Read entire file into memory. */
    numRead = fread( data, 1, fileSize, fid );
    if( numRead != fileSize )
    {
        printf("Could not read entire file. Only read %d bytes.\n", numRead );
        goto error;
    }
    *sizePtr = numRead;

error:
    if( fid != NULL )
        fclose( fid );
    return data;
}


