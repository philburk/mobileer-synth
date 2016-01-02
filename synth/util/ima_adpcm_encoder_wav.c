/* $Id: ima_adpcm_encoder_wav.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
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
 * Setup an AudioSample to receive encoded ADPCM data.
 */
void IMA_WAV_SetupSample( AudioSample *asmp,
                        int bytesPerBlock, /* Typically 0x100 */
                        long frameRate
                        )
{
    asmp->data = NULL;
    asmp->bitsPerSample = 4;
    asmp->bytesPerBlock = bytesPerBlock;
    asmp->format = AUDIO_FORMAT_IMA_ADPCM_WAV;
    asmp->frameRate = frameRate;
    asmp->numberOfBytes = 0;
    asmp->numberOfFrames = 0;
    asmp->samplesPerFrame = 1;
    asmp->samplesPerBlock = ((bytesPerBlock - 4)*2) + 1;
    asmp->maxNumberOfBytes  = 0;
}

/****************************************************************************/
int IMA_WAV_CalculateEncodedSize(
        int bytesPerBlock, /* Typically 0x100 */
        int minSamples /* Number of blocks will be rounded up to accomodate all samples. */
        )
{
    int numBlocks;
    int samplesPerBlock = IMA_WAV_SamplesPerBlock( bytesPerBlock );
    numBlocks = (minSamples + samplesPerBlock - 1) / samplesPerBlock;
    return numBlocks * bytesPerBlock;
}

/****************************************************************************/
int IMA_WAV_EncodeNextBlock_VOID( void *imacod_void, unsigned char *buffer )
{
    return IMA_WAV_EncodeNextBlock((IMA_WAV_Coder *) imacod_void, buffer );
}

/****************************************************************************
 * Encode next block of an ADPCM stream.
 * Return -1 if write fails.
 */
int IMA_WAV_EncodeNextBlock( IMA_WAV_Coder *imacod, unsigned char *buffer )
{
    int   numWritten = 0;
    short *decodedData = (short *) buffer;

    unsigned char *encoded = imacod->encodedBlock;

/* Write value and stepIndex to beginning of block. */
    imacod->previousValue = decodedData[0];
    WriteShortLE( &encoded, imacod->previousValue );
    WriteShortLE( &encoded, (short) imacod->stepIndex );

/* Encode remaining samples in block. */
    IMA_EncodeArray( &decodedData[1],
        imacod->samplesPerBlock - 1,  /* Cuz we already wrote one sample to header. */
        &imacod->previousValue,
        &imacod->stepIndex, encoded );

/* Write encoded block to Stream. */
    numWritten = imacod->sio->write( imacod->sio, (char *) imacod->encodedBlock,
        imacod->bytesPerBlock );
    if( numWritten != imacod->bytesPerBlock )
    {
        return -1;
    } /* end if( numWritten != imacod->bytesPerBlock ) */

/* Fix zero length data chunks. */
    imacod->bytesEncoded += imacod->bytesPerBlock;

    return 0;
}
