/* $Id: ima_adpcm_decoder_wav.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * High level WAV file format IMA Intel/DVI ADPCM decoder and encoder
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi_errors.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/read_wav.h"
#include "spmidi/include/write_wav.h"
#include "spmidi/include/ima_adpcm.h"
#include "spmidi/include/ima_adpcm_wav.h"

#define PRINT(x) /* { printf x; } */

/****************************************************************************
 * Setup Coder structure for block-by-block decoding or encoding of an ADPCM stream.
 * Block size is specified by samplesPerBlock which is normally
 * read from a WAV file.
 */
int IMA_WAV_InitializeCoder( IMA_WAV_Coder *imacod, int samplesPerBlock, StreamIO *sio )
{
    MemTools_Clear( imacod, sizeof(IMA_WAV_Coder) );
    imacod->sio = sio;
    imacod->samplesPerBlock = samplesPerBlock;
    imacod->bytesPerBlock = IMA_WAV_BytesPerBlock( imacod->samplesPerBlock );
    imacod->encodedBlock = SPMIDI_AllocateMemory( imacod->bytesPerBlock );
    if( imacod->encodedBlock == NULL ) return SPMIDI_Error_OutOfMemory;

    PRINT(("IMA_WAV_SetupCoder: imacod->samplesPerBlock = %i, imacod->bytesPerBlock = %i,\n",
        imacod->samplesPerBlock, imacod->bytesPerBlock ));

    return 0;
}

/****************************************************************************
 * Delete memory allocated for Coder.
 */
void IMA_WAV_TerminateCoder( IMA_WAV_Coder *imacod )
{
    if( imacod->encodedBlock ) SPMIDI_FreeMemory( imacod->encodedBlock ); 
    imacod->encodedBlock = NULL;
}

/****************************************************************************/
int IMA_WAV_DecodeNextBlock( IMA_WAV_Coder *imacod, short *decodedData, int maxSamples )
{
    int   numSamples = 0;
    short initialValue;
    int   initialStepIndex;
    int   numRead;
    unsigned char *encoded;

    if( maxSamples < imacod->samplesPerBlock )
    {
        return SPMIDI_Error_BufferTooSmall;
    }

    /* Read next encoded block from stream. */
    numRead = imacod->sio->read( imacod->sio, (char *) imacod->encodedBlock,
        imacod->bytesPerBlock );

    encoded = imacod->encodedBlock;

    if( numRead == imacod->bytesPerBlock )
    {
        /* Parse value and stepIndex from beginning of block. */
        initialValue = ParseShortLE( &encoded );
        initialStepIndex = *encoded++;
        encoded++; /* Skip reserved byte. */
        decodedData[0] = initialValue;
        
        PRINT(("initialValue = %d,\n", initialValue ));
        PRINT(("initialStepIndex = %d,\n", initialStepIndex ));

        /* Decode remaining samples in block. */
        numSamples = imacod->samplesPerBlock - 1;  /* Cuz we already got one sample from header. */
        PRINT(("numSamples = %d,\n", numSamples ));
        IMA_DecodeArray( encoded, numSamples,
            initialValue, initialStepIndex, &decodedData[1] );
        return numSamples + 1;
    }
    else
    {
        return -1;
    }
}
