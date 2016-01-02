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
#define ENCODED_BYTES_PER_BLOCK  (SAMPLES_PER_BLOCK/COMPRESSION_RATIO)

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

//  char *inputFileName = "E:\\nomad\\MIDISynth\\test\\chewymonkeys_11025.wav";
//  char *outputFileName = "E:\\nomad\\MIDISynth\\test\\chewymonkeys_11025_ima.raw";
    char *inputFileName = "E:\\nomad\\MIDISynth\\test\\short_11025.wav";
    char *outputFileName = "E:\\nomad\\MIDISynth\\test\\short_11025_ima.raw";
    int   fileSize;
    unsigned char *fileImage = NULL;
    short *sampleData = NULL;
    int err;
    int numWritten;
    int numToWrite;

    if( argc > 1 ) inputFileName = argv[1];
    if( argc > 2 ) outputFileName = argv[2];

    printf("%s - Copyright 2005 Mobileer, Inc.\n", argv[0] );
    printf("Convert %s to raw IMA ADPCM\n", inputFileName );

    asmp = &ASMP;

    SPMIDI_HostInit();

    outFile = fopen( outputFileName, "wb" );
    if( outFile == NULL )
    {
        printf("ERROR could not open output File %s\n", outputFileName );
        goto error;
    }

    numToWrite = ENCODED_BYTES_PER_BLOCK;
    numWritten = fwrite( encodedBuffer, 1, numToWrite, outFile );
    if( numWritten != numToWrite )
    {
        printf("ERROR: could not write to file! Is disk full?\n" );
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
