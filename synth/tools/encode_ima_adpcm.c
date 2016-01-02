/*
 * Application to convert a PCM WAV file to a raw IMA ADPCM file.
 *
 * Author: Phil Burk
 * Copyright 1998 Phil Burk
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/ima_adpcm.h"
#include "spmidi/include/ima_adpcm_wav.h"
#include "spmidi/include/read_wav.h"
#include "spmidi/include/spmidi_load.h"

#include "spmidi/include/spmidi_audio.h"

#define SAMPLES_PER_BLOCK  (512)
#define COMPRESSION_RATIO  (4)
#define ENCODED_SAMPLES_PER_BYTE  (2)
#define ENCODED_BYTES_PER_BLOCK  (SAMPLES_PER_BLOCK/ENCODED_SAMPLES_PER_BYTE)

unsigned char encodedBuffer[ENCODED_BYTES_PER_BLOCK];

/**************************************************************/
int main( int argc, char **argv )
{
    StreamIO *stream;
    AudioSample ASMP;
    AudioSample *asmp;
    FILE  *outFile = NULL;
    short previousSample = 0;
    int stepIndex = 0;
    int numSamplesLeft;

    char *inputFileName = "E:\\nomad\\MIDISynth\\test\\chewymonkeys_11025.wav";
    char *outputFileName = "E:\\nomad\\MIDISynth\\test\\chewymonkeys_11025_ima.raw";
//  char *inputFileName = "E:\\nomad\\MIDISynth\\test\\short_11025.wav";
//  char *outputFileName = "E:\\nomad\\MIDISynth\\test\\short_11025_ima.raw";
    int   fileSize;
    unsigned char *fileImage = NULL;
    short *sampleData = NULL;
    int err;

    if( argc > 1 ) inputFileName = argv[1];
    if( argc > 2 ) outputFileName = argv[2];

    printf("%s - Copyright 2005 Mobileer, Inc.\n", argv[0] );
    printf("Convert %s to raw IMA ADPCM\n", inputFileName );

    asmp = &ASMP;

    SPMIDI_HostInit();

    /* Get encoded data from somewhere. */
    fileImage = SPMUtil_LoadFileImage( inputFileName, &fileSize );
    if( fileImage == NULL )
    {
        printf("ERROR could not find input File %s\n", inputFileName );
        goto error;
    }

    stream = Stream_OpenImage( (char *)fileImage, fileSize );
    if( stream == NULL )
    {
        printf("ERROR: Stream_OpenImage() failed.\n" );
        goto error;
    }
    
    /* Parse the WAV file and get sample info. */
    err = Audio_WAV_ParseSampleStream(
        stream,       /* Stream access to WAV file. */
        asmp          /* Pre-allocated but empty structure to be completely filled in by parser. */
    );
    if( err < 0 )
    {
        printf("ERROR: Audio_WAV_ParseSampleStream returned %d\n", err );
        goto error;
    }
    
    /* Validate WAV sample format. */
    if( asmp->format != AUDIO_FORMAT_PCM )
    {
        printf("Expected PCM file, not %d\n", asmp->format );
        goto error;
    }
    
    if( asmp->bitsPerSample != 16 )
    {
        printf("Expected 16 bit file, not %d\n", asmp->bitsPerSample );
        goto error;
    }

    if( asmp->samplesPerFrame != 1 )
    {
        printf("Expected mono file, not %d\n", asmp->samplesPerFrame );
        goto error;
    }

    /* Open the binary output file. */
    outFile = fopen( outputFileName, "wb" );
    if( outFile == NULL )
    {
        printf("ERROR could not open output File %s\n", outputFileName );
        goto error;
    }

    /* Encode samples until we run out of data. */
    numSamplesLeft = asmp->numberOfFrames * asmp->samplesPerFrame;
    sampleData = (short *) (fileImage + asmp->dataOffset);
    while( numSamplesLeft > 0 )
    {
        int numToWrite;
        int numWritten;

        /* Encode a full block or whatever is left. */
        int numSamples = (numSamplesLeft < SAMPLES_PER_BLOCK) ? numSamplesLeft : SAMPLES_PER_BLOCK;

        /* Encode a block of 16 bit sample data. */
        IMA_EncodeArray(
            sampleData,
            numSamples,
            &previousSample,
            &stepIndex,
            encodedBuffer
            );

        /* Advance to next block. */
        sampleData += numSamples;
        numSamplesLeft -= numSamples;

        /* Write any encoded samples to the output binary file. */
        numToWrite = numSamples / ENCODED_SAMPLES_PER_BYTE;
        numWritten = fwrite( encodedBuffer, 1, numToWrite, outFile );
        if( numWritten != numToWrite )
        {
            printf("ERROR: could not write to file! Is disk full?\n" );
        }
    }

    if( outFile != NULL ) fclose( outFile );
    SPMIDI_HostTerm();
    printf("Encoded output written to %s.\n", outputFileName );
    return 0;

error:
    if( outFile != NULL ) fclose( outFile );
    printf("Error exit.\n");
    SPMIDI_HostTerm();
    return 1;
}
