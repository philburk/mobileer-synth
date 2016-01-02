/* $Id: midistream_player.c,v 1.3 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Play a MIDI Stream File whose name is passed on the command line.
 * Use the ME2000 Synthesizer.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/midistream_player.h"

/*****************************************************************/
/**
 * Setup a player for a MIDI stream file image.
 */
SPMIDI_Error MIDIStreamPlayer_Setup( MIDIStreamPlayer *player, int playFramesPerPlayTick, int playFramesPerSecond,
                                    const unsigned char *image, int numBytes )
{
    int streamFramesPerSecond;

    /* Validate header. */
    /* Is it an SMID file? */
    if( (image[0] != 'S') ||
        (image[1] != 'M') ||
        (image[2] != 'I') ||
        (image[3] != 'D')
      )
    {
        return MIDIStream_Error_NotSMID;
    }

    player->streamFramesPerStreamTick = (image[4] << 8) | image[5];
    streamFramesPerSecond = (image[6] << 8) | image[7];

    /* Account for mismatch in stream buffer sizes and playback buffer sizes.
     * and stream frameRate and playback frameRate. */
    player->streamFramesPerPlayTick = (playFramesPerPlayTick * streamFramesPerSecond) / playFramesPerSecond;

    player->image = image;
    player->numBytes = numBytes;

    /* Position cursor to beginning of song. */
    return MIDIStreamPlayer_Rewind( player );
}

#define GET_NEXT_BYTE  ((int) player->image[ player->cursor++ ])

/*****************************************************************/
SPMIDI_Error MIDIStreamPlayer_Rewind( MIDIStreamPlayer *player )
{
    player->cursor = MIDISTREAM_HEADER_SIZE;
    player->currentFrame = 0;
    player->nextFrameToPlay = 0;

    /* Get first duration from first packet. */
    player->nextFrameToPlay += (GET_NEXT_BYTE * player->streamFramesPerStreamTick);

    return SPMIDI_Error_None;
}

/*****************************************************************/
/**
 * Play one ticks worth of packets.
 * @return 0 if more events are available to play. 1 if at end of stream.
 */
int MIDIStreamPlayer_PlayTick( MIDIStreamPlayer *player, SPMIDI_Context *spmidiContext )
{
    /* Advance frame counter. */
    player->currentFrame += player->streamFramesPerPlayTick;

    /* Play any packets from MIDI stream up to that frame time. */
    while( (player->cursor < player->numBytes) &&
           (player->currentFrame > player->nextFrameToPlay)
         )
    {
        int numToPlay = GET_NEXT_BYTE;

        /* Is it an empty packet used to keep time? Or a data packet. */
        if( numToPlay > 0 )
        {
            /* Write block of MIDI data to synthesizer and advance cursor. */
            SPMIDI_Write( spmidiContext,
                &player->image[ player->cursor ], numToPlay );
            // DumpMemory( &player->image[ player->cursor ], numToPlay );

            player->cursor += numToPlay;
        }

        if(player->cursor < player->numBytes)
        {
            /* Get duration from next packet. */
            player->nextFrameToPlay += (GET_NEXT_BYTE * player->streamFramesPerStreamTick);
        }
    }
    
    return (player->cursor < player->numBytes) ? 0 : 1;
}

