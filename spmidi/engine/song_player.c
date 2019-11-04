/* $Id: song_player.c,v 1.15 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Load XMF file and play it
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 * Author: Robert Marsanyi, Phil Burk
 */

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_editor.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/include/song_player.h"

#include "spmidi/engine/memtools.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/include/dls_parser.h"
#include "spmidi/include/xmf_parser.h"

/* Only compile if supporting ME3000 API */
#if SPMIDI_ME3000


#define TOUCH(value) \
    (void)value

typedef struct SP_Context_s
{
    char               *fileStart;              /* location of file image in RAM */
    int                 fileSize;               /* size of file image */
    SPMIDI_Context     *spmidiContext;          /* everything MIDI needs to play */
    XMFParser          *xmfParser;              /* the xmf parser context */
    DLSParser          *dlsParser;              /* the dls parser context */
    MIDIFilePlayer     *midiFilePlayer;         /* plays the result */
}
SP_Context_t;

/*******************************************************************/
static int SP_ParseXMF( SP_Context_t *spContext )
{
    int result;
    XMFParser *xmfParser;
    DLSParser *dlsParser = NULL;
    SPMIDI_Context* spmidiContext = spContext->spmidiContext;

    unsigned char *dls;
    long dlsSize;

    /* Create an XMF parser */
    result = XMFParser_Create( &xmfParser, (unsigned char *)spContext->fileStart, spContext->fileSize );
    if( result < 0 )
        goto error;
    spContext->xmfParser = xmfParser;

    /* Parse the image */
    result = XMFParser_Parse( xmfParser );
    if( result < 0 )
        goto error;

    /* Get the DLS portion, and parse it.
     * Mobile XMF does not require the DLS chunk so it may be missing.
     */
    dls = XMFParser_GetDLS( xmfParser, &dlsSize );
    if( dls != NULL )
    {
        result = DLSParser_Create( &dlsParser, dls, dlsSize );
        if( result < 0 )
            goto error;
        spContext->dlsParser = dlsParser;

        result = DLSParser_SetSampleRate( dlsParser, SPMIDI_GetSampleRate( spmidiContext ) );
        if( result < 0 )
            goto error;

        result = DLSParser_Parse( dlsParser );
        if( result < 0 )
            goto error;

        result = DLSParser_Load( dlsParser, spContext->spmidiContext );
        if( result < 0 )
            goto error;
    }

    return 0;

error:
    return result;
}

/*******************************************************************
 * API Calls
 *******************************************************************/
SongPlayer_Type SongPlayer_GetType( unsigned char *songImage, spmSInt32 numBytes )
{
    TOUCH( numBytes );
    return( XMFParser_IsXMF( songImage ) ? SongPlayer_Type_XMF : SongPlayer_Type_SMF );
}

/*******************************************************************/
SPMIDI_Error SongPlayer_Create( SongPlayer **playerPtr, SPMIDI_Context *spmidiContext,
                                unsigned char *songImage, spmSInt32 numBytes )
{
    int result = 0;
    SP_Context_t *songPlayer;
    unsigned char *midiStart;
    spmSInt32 midiSize;
    int rate;

    /* Check parms for sanity */
    if( playerPtr == NULL || spmidiContext == NULL || songImage == NULL || numBytes <= 0 )
        return SPMIDI_Error_IllegalArgument;

    /* Allocate context structure */
    songPlayer = SPMIDI_ALLOC_MEM( sizeof(SP_Context_t), "SP_Context_t" );
    if( songPlayer == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }
    MemTools_Clear( songPlayer, sizeof(SP_Context_t) );

    /* Set structure elements */
    songPlayer->spmidiContext= spmidiContext;
    songPlayer->fileStart = (char *)songImage;
    songPlayer->fileSize = numBytes;

    /* If fileimage is XMF, parse it */
    if( SongPlayer_GetType( songImage, numBytes ) == SongPlayer_Type_XMF )
    {
        result = SP_ParseXMF( songPlayer );
        if( result < 0 )
            goto error;

        midiStart = XMFParser_GetSMF( songPlayer->xmfParser, &( midiSize ) );
        
        /* Verify that we have found an SMF file. */
        if( midiStart == NULL )
        {
            result = MIDIFile_Error_NotSMF; /* 050616 */
            goto error;
        }
    }
    else
    {
        midiStart = songImage;
        midiSize = numBytes;
    }

    /* Create MidiFilePlayer.  This will return an error "not SMF" if it can't parse the file */
    rate = SPMIDI_GetSampleRate( spmidiContext );
    result = MIDIFilePlayer_Create( &( songPlayer->midiFilePlayer ), rate, midiStart, midiSize );
    if( result < 0 )
        goto error;

    *playerPtr = (SongPlayer *)songPlayer;
    return 0;

error:
    if( songPlayer != NULL )
        SongPlayer_Delete( (SongPlayer *)songPlayer );
    return result;
}

/*******************************************************************/
SPMIDI_Error SongPlayer_Start( SongPlayer *player )
{
    TOUCH( player );
    return 0;
}

/*******************************************************************/
SPMIDI_Error SongPlayer_Rewind( SongPlayer *player )
{
    SP_Context_t *songPlayer = (SP_Context_t *)player;

    return( MIDIFilePlayer_Rewind( songPlayer->midiFilePlayer ) );
}

/*******************************************************************/
SPMIDI_Error SongPlayer_Stop( SongPlayer *player )
{
    TOUCH( player );
    return 0;
}

/*******************************************************************/
MIDIFilePlayer *SongPlayer_GetMIDIFilePlayer( SongPlayer *player )
{
    SP_Context_t *songPlayer = (SP_Context_t *)player;

    return( songPlayer->midiFilePlayer );
}

/*******************************************************************/
int SongPlayer_PlayFrames( SongPlayer *player, int numFrames )
{
    SP_Context_t *songPlayer = (SP_Context_t *)player;

    return( MIDIFilePlayer_PlayFrames( songPlayer->midiFilePlayer, songPlayer->spmidiContext, numFrames ) );
}

/*******************************************************************/
void SongPlayer_Delete( SongPlayer *player )
{
    SP_Context_t *songPlayer = (SP_Context_t *)player;

    if( songPlayer != NULL )
    {
        if( songPlayer->dlsParser != NULL )
        {
            /* DLSParser_Unload( songPlayer->dlsParser, songPlayer->spmidiContext ); */
            DLSParser_Delete( songPlayer->dlsParser );
        }
        if( songPlayer->xmfParser != NULL )
        {
            XMFParser_Delete( songPlayer->xmfParser );
        }
        if( songPlayer->midiFilePlayer != NULL )
        {
            MIDIFilePlayer_Delete( songPlayer->midiFilePlayer );
        }

        SPMIDI_FreeMemory( songPlayer );
    }
}

#endif /* #if SPMIDI_ME3000 */
