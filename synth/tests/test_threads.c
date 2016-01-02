/* $Id: test_threads.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play two MIDI files simultaneously.
 * Output one on left channel, and the other on the right channel.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_audio.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midifile_player.h"

#define SAMPLE_RATE         (44100)

#define SPMIDI_DIR  "E:\\nomad\\MIDISynth\\"
#define RINGTONE_DIR  SPMIDI_DIR"ringtones/"
#define FILENAME_1    RINGTONE_DIR"phil/LakeTahoe_rt.mid"
#define FILENAME_2    RINGTONE_DIR"jeanne/UrbanOre_rt.mid"

SPMIDI_AudioDevice  sHostAudioDevice; /* PortAudio blocking read/write stream. */

typedef struct SongContext_s
{
    MIDIFilePlayer *player;
    SPMIDI_Context *spmidi;
    void *fileImage;
    int active;
}
SongContext_t;

/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyTextCallback( int trackIndex, int metaEventType,
                     const char *addr, int numChars, void *userData )
{
    int i;
    (void) userData; /* Prevent compiler warnings about unused data. */
    printf("MetaEvent type %d on track %d: ", metaEventType, trackIndex );
    for( i=0; i<numChars; i++ )
        printf("%c", addr[i] );
    printf("\n");
}

/****************************************************************/
int PlayBothSongs( SongContext_t *song1, SongContext_t *song2 )
{
    short mono1[SPMIDI_MAX_FRAMES_PER_BUFFER];
    short mono2[SPMIDI_MAX_FRAMES_PER_BUFFER];
    short stereo[SPMIDI_MAX_FRAMES_PER_BUFFER];
    int im;
    int is;
    int result = 0;

    while( song1->active && song2->active )
    {
        if( song1->active )
        {
            result = MIDIFilePlayer_PlayFrames( song1->player, song1->spmidi, SPMIDI_MAX_FRAMES_PER_BUFFER );
            if( result != 0 )
            {
                song1->active = 0;
            }
        }

        if( song2->active )
        {
            result = MIDIFilePlayer_PlayFrames( song2->player, song2->spmidi, SPMIDI_MAX_FRAMES_PER_BUFFER );
            if( result != 0 )
            {
                song2->active = 0;
            }
        }

        /* Synthesize both songs into mono buffers. */
        SPMIDI_ReadFrames( song1->spmidi, mono1, SPMIDI_MAX_FRAMES_PER_BUFFER, 1, 16 );
        SPMIDI_ReadFrames( song2->spmidi, mono2, SPMIDI_MAX_FRAMES_PER_BUFFER, 1, 16 );

        /* Interleave two mono buffers into stereo buffer */
        is = 0;
        for( im=0; im<SPMIDI_MAX_FRAMES_PER_BUFFER; im++ )
        {
            stereo[is++] = mono1[im];
            stereo[is++] = mono2[im];
        }
        SPMUtil_WriteAudioBuffer( sHostAudioDevice, stereo, SPMIDI_MAX_FRAMES_PER_BUFFER );
    }

    return result;
}

/****************************************************************/
int InitSong( SongContext_t *song, char *fileName )
{
    int  fileSize;
    int   result = -1;
    /* Load MIDI File into a memory image. */
    song->fileImage = SPMUtil_LoadFileImage( fileName, &fileSize );
    if( song->fileImage == NULL )
    {
        printf("ERROR reading MIDI File.\n" );
        return result;
    }
    printf("File: %s\n", fileName );

    /* Create a player_1, parse MIDIFile image and setup tracks. */
    result = MIDIFilePlayer_Create( &song->player, (int) SAMPLE_RATE, song->fileImage, fileSize );
    if( result < 0 )
        return result;

    song->active = 1;

    /* Start synthesis engine with default number of voices. */
    return SPMIDI_CreateContext( &song->spmidi, SAMPLE_RATE );
}


/****************************************************************/
void TermSong( SongContext_t *song )
{
    if( song->spmidi != NULL )
    {
        SPMIDI_DeleteContext(song->spmidi);
        song->spmidi = NULL;
    }

    if( song->player != NULL )
    {
        MIDIFilePlayer_Delete( song->player );
        song->player = NULL;
    }

    if( song->fileImage != NULL )
    {
        SPMUtil_FreeFileImage( song->fileImage );
        song->fileImage = NULL;
    }
}

/****************************************************************/
int RunTest( void )
{
    int   result = -1;
    SongContext_t song1 = {0};
    SongContext_t song2 = {0};

    result = InitSong( &song1, FILENAME_1 );
    if( result < 0 )
        goto error;

    result = InitSong( &song2, FILENAME_2 );
    if( result < 0 )
        goto error;

    /* Open simplified blocking I/O layer on top of PortAudio. */
    result = SPMUtil_StartAudio( &sHostAudioDevice, SAMPLE_RATE, 2 );
    if( result < 0 )
        goto error;


    PlayBothSongs( &song1, &song2);

error:
    TermSong( &song1 );
    TermSong( &song2 );

    if( result != 0 )
        printf("ERROR = %d\n", result );
    else
        printf("SUCCESS\n");

    return result;
}

/****************************************************************/
int main( int argc, char ** argv )
{

    (void) argc;
    (void) argv;

    /* Run test twice if it works. */
    if( RunTest() >= 0 )
    {
        RunTest();
    }
}


