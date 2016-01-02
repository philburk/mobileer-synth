/* $Id: midifile_parser.c,v 1.2 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 * Standard MIDI File parser.
 * This parser scans the SMF image and calls the user specified
 8 callback routines as it encounters events.
 *
 * Author: Phil Burk
 * Copyright 2002-2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/midifile_player.h"
#include "spmidi/engine/memtools.h"

#if 0
    #define DBUGMSG(x)   PRTMSG(x)
    #define DBUGNUMD(x)  PRTNUMD(x)
    #define DBUGNUMH(x)  PRTNUMH(x)
#else
    #define DBUGMSG(x)
    #define DBUGNUMD(x)
    #define DBUGNUMH(x)
#endif


/****************************************************************/
/** Read byte from image or -1 if end of image. */
static int bytesLeft(  MIDIFileParser_t *parser )
{
    return parser->imageSize - parser->bytesRead;
}

/****************************************************************/
/** Read byte from image or -1 if end of image. */
static int readByte(  MIDIFileParser_t *parser )
{
    if( parser->bytesRead >= parser->imageSize )
        return -1;
    else
        return (int) parser->imageStart[ parser->bytesRead++ ];
}

/****************************************************************/
/** Read 32 bit signed integer assuming Big Endian byte order. */
static int readIntBig(  MIDIFileParser_t *parser )
{
    int result = readByte(parser) & 0xFF;
    result = (result<<8) | (readByte(parser)&0xFF);
    result = (result<<8) | (readByte(parser)&0xFF);
    result = (result<<8) | (readByte(parser)&0xFF);
    return result;
}

/****************************************************************/
/** Read 16 bit signed short assuming Big Endian byte order. */
static short readShortBig( MIDIFileParser_t *parser )
{
    short result = (short) ((readByte(parser) << 8));
    result |= readByte(parser) & 0xFF;
    return result;
}

/****************************************************************/
/** Parse variable length quentity. Data is stored as 7 bits per byte.
 * Last byte has high bit clear. Preceding bytes have high bit set.
 */
static int parseVariableLengthQuantity( MIDIFileParser_t *parser )
{
    const unsigned char *addr = &parser->imageStart[parser->bytesRead];
    const unsigned char *addrOld = addr;
    int value = MIDIFile_ReadVariableLengthQuantity( &addr );
    parser->bytesRead += addr - addrOld;
    return value;
}


/****************************************************************/
static int parseMetaEvent( MIDIFileParser_t *parser, int ticks )
{
    int type = readByte(parser);
    int eventLength = parseVariableLengthQuantity(parser);
    int valid = 1;

    if( type >= 0x80 )
    {
        /* return MIDIFile_Error_IllegalMetaEventType; */
        /* This is an illegal type. But some files have illegal types
         * and some players ignore them. So we will too. */
        valid = 0;
        DBUGMSG("Warning - SMF MetaEvent type invalid = "); DBUGNUMH( type ); DBUGMSG("\n");
    }
    else if( type == MIDI_META_END_OF_TRACK )
    {
        if( eventLength == 0 )
        {
            parser->gotEndOfTrack = 1;
        }
        else
        {
            return MIDIFile_Error_IllegalFormat;
        }
        /* Let user callback see EOT as well. */
    }

    if( valid && (parser->handleMetaEventProc != NULL) )
    {
        (parser->handleMetaEventProc)( parser, ticks, type,
                                       &parser->imageStart[ parser->bytesRead ], eventLength );
    }

    parser->bytesRead += eventLength;
    return 0;
}


/****************************************************************/
/**
 * @param type is 0xF0 or 0xF7 from the MIDIFile
 */
static int parseSysExEvent( MIDIFileParser_t *parser, int ticks, int type )
{
    int eventLength = parseVariableLengthQuantity(parser);

    if( parser->handleSysExProc != NULL )
    {
        (parser->handleSysExProc)( parser, ticks, type,
                                   &parser->imageStart[ parser->bytesRead ], eventLength );
    }

    parser->bytesRead += eventLength;
    return 0;
}

/****************************************************************/
/** Parse MIDI Event.
 */
static int parseEvent( MIDIFileParser_t *parser, int ticks, int command )
{
    int      isRunningStatus;
    int      bytesPerMessage;

    int      data1 = 0;
    int      data2 = 0;
    int      bytesReadInMessage = 1; /* already read first byte */

    isRunningStatus = ( command < 0x80 ); /* running status? */
    if( isRunningStatus )
    {
        data1 = command;
        command = parser->lastCommand;
        bytesReadInMessage = 2;
    }
    parser->lastCommand = command;
    bytesPerMessage = SPMIDI_GetBytesPerMessage( command );
    if( !isRunningStatus )
    {
        if( bytesPerMessage > bytesReadInMessage )
        {
            data1 = readByte(parser);
            bytesReadInMessage++;
        }
    }
    /* Read final byte if needed. */
    if( bytesPerMessage > bytesReadInMessage )
    {
        data2 = readByte(parser);
        bytesReadInMessage++;
    }

    parser->handleEventProc( parser, ticks, command, data1, data2 );

    return 0;
}

/****************************************************************/
static SPMIDI_Error parseTrack( MIDIFileParser_t *parser, int index )
{
    int deltaTime;
    int chunkLen;
    int byteLimit;
    int ticks = 0;
    int result;

    int ID = readIntBig(parser);
    if( ID != MIDI_MTrk_ID )
        return MIDIFile_Error_IllegalFormat;

    chunkLen = readIntBig(parser);
    byteLimit = chunkLen + parser->bytesRead;
    parser->gotEndOfTrack = 0;

    /* Call handler to start saving new track. */
    if( parser->beginTrackProc != NULL )
    {
        result = (parser->beginTrackProc)( parser, index, chunkLen );
        if( result < 0 )
        {
            return (SPMIDI_Error) result;
        }
    }

    /* Loop on remaining bytes. */
    while( (parser->bytesRead < byteLimit) &&
            (parser->gotEndOfTrack == 0) )
    {
        int command;
        /* Parse ticks as variable length quantity. */
        deltaTime = parseVariableLengthQuantity(parser);
        ticks += deltaTime;
        /* Read message. Might be running status */
        command = readByte(parser);
        switch( command )
        {
        case 0xFF:
            result = parseMetaEvent( parser, ticks );
            if( result < 0 )
                return (SPMIDI_Error) result;
            break;
        case 0xF0:
        case 0xF7:
            result = parseSysExEvent( parser, ticks, command );
            if( result < 0 )
                return (SPMIDI_Error) result;
            break;
        default:
            parseEvent( parser, ticks, command );
            break;
        }
    }

    if( parser->bytesRead <  byteLimit )
    {
        /* An EndOfTrack was found before we ran out of bytes in the track!
         * We used to  we return an error for this condition.
         * But now we just skip the extraneous bytes after the
         * EndOfTrack event.
         */
        /* return MIDIFile_Error_PrematureEndOfTrack; */
        parser->bytesRead = byteLimit;
    }
    else if( parser->gotEndOfTrack == 0 )
    {
        return MIDIFile_Error_MissingEndOfTrack;
    }

    /* Call handler to finish saving new track. */
    if( parser->endTrackProc != NULL )
    {
        (parser->endTrackProc)( parser, index );
    }
    
    return SPMIDI_Error_None;
}


/****************************************************************/
static int parseTracks( MIDIFileParser_t *parser )
{
    int i;
    int result;

    for( i=0; i<parser->numTracks; i++ )
    {
        result = parseTrack( parser, i );
        if( result < 0 )
            return result;
    }
    return 0;
}


/****************************************************************/
static SPMIDI_Error parseHeader( MIDIFileParser_t *parser )
{
    int chunkLen;
    int division;
    int ID = 0;
    /* Scan for start of MIDI file content in case it is wrapped. */
    while( bytesLeft( parser ) > 0 )
    {
        ID = (ID << 8) | readByte(parser);
        if( ID == MIDI_MThd_ID )
            break;
    }
    if( ID != MIDI_MThd_ID )
        return MIDIFile_Error_NotSMF;

    chunkLen = readIntBig(parser);
    if( chunkLen != 6 ) /* TODO, maybe this should be <6 so we allow future header formats. */
    {
        return MIDIFile_Error_IllegalFormat;
    }
    parser->format = readShortBig(parser);
    parser->numTracks = readShortBig(parser);
    division = readShortBig(parser);

    if( (division & 0x00008000) == 0 )
    {
        /* Base time on ticksPerBeat */
        parser->ticksPerBeat = division;
        /* Default assuming 120 beats per minute. */
        parser->ticksPerSecond = 2 * parser->ticksPerBeat;
    }
    else
    {
        /* Base time on SMPTE time code. */
        /* Convert division from negative two's complement to positive. */
        int framesPerSecond = 1 + (((division >> 8) & 0x007F) ^ 0x007F);
        int ticksPerFrame = division & 0x00FF;
        parser->ticksPerSecond = framesPerSecond * ticksPerFrame;
        parser->ticksPerBeat = parser->ticksPerSecond / 2;
    }
    return SPMIDI_Error_None;
}

/****************************************************************/
/** Parse variable length quantity. Data is stored as 7 bits per byte.
 * Last byte has high bit clear. Preceding bytes have high bit set.
 */
int MIDIFile_ReadVariableLengthQuantity( const unsigned char **addrPtr )
{
    const unsigned char *addr = *addrPtr;
    int value = 0;
    int data;
    do
    {
        data = *addr++;
        value = (value << 7) + (data & 0x7F);

        DBUGMSG("VLQ: data = ");
        DBUGNUMD(data);
        DBUGMSG(", value = ");
        DBUGNUMD(value);
        DBUGMSG("\n");
    }
    while( (data & 0x80) == 0x80 );
    *addrPtr = addr;
    return value;
}


/****************************************************************/
int MIDIFile_Parse(  MIDIFileParser_t *parser )
{
    int result;

    parser->noteOnCount = 0;
    parser->noteOffCount = 0;
    parser->bytesRead = 0;

    result = parseHeader( parser );
    if( result < 0 )
    {
        return result;
    }

    DBUGMSG("MIDIFile: format = ");
    DBUGNUMD(parser->format);
    DBUGMSG(", numTracks = ");
    DBUGNUMD(parser->numTracks);
    DBUGMSG(", ticksPerSecond = ");
    DBUGNUMD(parser->ticksPerSecond);
    DBUGMSG("\n");

    return parseTracks( parser );
}
