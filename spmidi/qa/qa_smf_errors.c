/* $Id: qa_smf_errors.c,v 1.4 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_smf_errors.c
 * @brief Test error return codes for illegal MIDIFilePlayer parameters
 * and illegal SMF content.
 *
 * @author Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/midifile_player.h"

#include "spmidi/qa/qa_tools.h"

#define SAMPLE_RATE  (22050)

const unsigned char sGoodSMF[] = {
    0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, /* MThd, 6 bytes in header. */
    0x00, 0x01, 0x00, 0x01, 0x00, 0xC0,             /* Format, nTracks, division */
    0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x15, /* MTrk, length. */
    0x00, 0xFF, 0x03, 0x05, 0x54, 0x69, 0x74, 0x6C, 0x65, /* Track Name */
    0x00, 0x90, 0x59, 0x7F, 0x21, 0x80, 0x59, 0x7F,  /* Note ON and Note OFF */
    0x66, 0xFF, 0x2F, 0x00 /* End Of Track Meta-event. */
    };

/* Bad MThd */
const unsigned char sBadMThdSMF[] = {
    0x4D, 0x55, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, /* MUhd!!!!, 6 bytes in header. */
    0x00, 0x01, 0x00, 0x01, 0x00, 0xC0,             /* Format, nTracks, division */
    0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x15, /* MTrk, length. */
    0x00, 0xFF, 0x03, 0x05, 0x54, 0x69, 0x74, 0x6C, 0x65, /* Track Name */
    0x00, 0x90, 0x59, 0x7F, 0x21, 0x80, 0x59, 0x7F,  /* Note ON and Note OFF */
    0x66, 0xFF, 0x2F, 0x00 /* End Of Track Meta-event. */
    };

/* Bad MThd */
const unsigned char sBadHeaderSizeSMF[] = {
    0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x05, /* MThd, 5 bytes in header!!!! */
    0x00, 0x01, 0x00, 0x01, 0x00, 0xC0,             /* Format, nTracks, division */
    0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x15, /* MTrk, length. */
    0x00, 0xFF, 0x03, 0x05, 0x54, 0x69, 0x74, 0x6C, 0x65, /* Track Name */
    0x00, 0x90, 0x59, 0x7F, 0x21, 0x80, 0x59, 0x7F,  /* Note ON and Note OFF */
    0x66, 0xFF, 0x2F, 0x00 /* End Of Track Meta-event. */
    };

const unsigned char sBadMTrkSMF[] = {
    0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, /* MThd, 6 bytes in header. */
    0x00, 0x01, 0x00, 0x01, 0x00, 0xC0,             /* Format, nTracks, division */
    0x4D, 0x54, 0x72, 0x6C, 0x00, 0x00, 0x00, 0x15, /* MTrl!!!, length. */
    0x00, 0xFF, 0x03, 0x05, 0x54, 0x69, 0x74, 0x6C, 0x65, /* Track Name */
    0x00, 0x90, 0x59, 0x7F, 0x21, 0x80, 0x59, 0x7F,  /* Note ON and Note OFF */
    0x66, 0xFF, 0x2F, 0x00 /* End Of Track Meta-event. */
    };

const unsigned char sMissingEOTSMF[] = {
    0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, /* MThd, 6 bytes in header. */
    0x00, 0x01, 0x00, 0x01, 0x00, 0xC0,             /* Format, nTracks, division */
    0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x11, /* MTrk, length. */
    0x00, 0xFF, 0x03, 0x05, 0x54, 0x69, 0x74, 0x6C, 0x65, /* Track Name */
    0x00, 0x90, 0x59, 0x7F, 0x21, 0x80, 0x59, 0x7F,  /* Note ON and Note OFF */
    };

/* Set numTracks to 2 but only one track. */
const unsigned char sMissingTrackSMF[] = {
    0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, /* MThd, 6 bytes in header. */
    0x00, 0x01, 0x00, 0x02, 0x00, 0xC0,             /* Format, nTracks, division */
    0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x15, /* MTrk, length. */
    0x00, 0xFF, 0x03, 0x05, 0x54, 0x69, 0x74, 0x6C, 0x65, /* Track Name */
    0x00, 0x90, 0x59, 0x7F, 0x21, 0x80, 0x59, 0x7F,  /* Note ON and Note OFF */
    0x66, 0xFF, 0x2F, 0x00 /* End Of Track Meta-event. */
    };

/* Garbage before and after the SMF data. */
const unsigned char sGarbagePadSMF[] = {
    0x05, 0x55, 0x4D, 0xEE, 0x22,
    0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, /* MThd, 6 bytes in header. */
    0x00, 0x01, 0x00, 0x01, 0x00, 0xC0,             /* Format, nTracks, division */
    0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x15, /* MTrk, length. */
    0x00, 0xFF, 0x03, 0x05, 0x54, 0x69, 0x74, 0x6C, 0x65, /* Track Name */
    0x00, 0x90, 0x59, 0x7F, 0x21, 0x80, 0x59, 0x7F,  /* Note ON and Note OFF */
    0x66, 0xFF, 0x2F, 0x00, /* End Of Track Meta-event. */
    0x44, 0x9F
    };

int TestSMFErrors( void )
{
    int result = 0;
    SPMIDI_Context *spmidiContext;
    MIDIFilePlayer *player;

    SPMIDI_Initialize();

    QA_Assert( SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE ) == 0,
        "Created SPMIDI context." );

    QA_Assert( MIDIFilePlayer_Create( &player, SAMPLE_RATE, sBadMThdSMF, sizeof(sBadMThdSMF) )
        == MIDIFile_Error_NotSMF,
        "Trap bad MThd." );

    QA_Assert( MIDIFilePlayer_Create( &player, SAMPLE_RATE, sBadHeaderSizeSMF, sizeof(sBadHeaderSizeSMF) )
        == MIDIFile_Error_IllegalFormat,
        "Trap bad header size." );

    QA_Assert( MIDIFilePlayer_Create( &player, SAMPLE_RATE, sMissingTrackSMF, sizeof(sMissingTrackSMF) )
        == MIDIFile_Error_IllegalFormat,
        "Trap missing track." );

    QA_Assert( MIDIFilePlayer_Create( &player, SAMPLE_RATE, sBadMTrkSMF, sizeof(sBadMTrkSMF) )
        == MIDIFile_Error_IllegalFormat,
        "Trap bad MTrk." );

    QA_Assert( MIDIFilePlayer_Create( &player, SAMPLE_RATE, sMissingEOTSMF, sizeof(sMissingEOTSMF) )
        == MIDIFile_Error_MissingEndOfTrack,
        "Missing EOT." );

    QA_Assert( MIDIFilePlayer_Create( &player, SAMPLE_RATE, sGoodSMF, sizeof(sGoodSMF) ) == 0,
        "Load good SMF." );
    MIDIFilePlayer_Delete( player );

    QA_Assert( MIDIFilePlayer_Create( &player, SAMPLE_RATE, sGarbagePadSMF, sizeof(sGarbagePadSMF) ) == 0,
        "Load padded SMF." );
    MIDIFilePlayer_Delete( player );

    SPMIDI_Terminate();

    return result;

}

int main(void);
int main(void)
{
    
    QA_Init( "qa_smf_errors" );

    TestSMFErrors();

    return QA_Term( 8 );
}

