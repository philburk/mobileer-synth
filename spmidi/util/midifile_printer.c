/* $Id: midifile_printer.c,v 1.4 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 * MIDI File parser and player.
 * The player operates directly on an SMF image.
 * It is not expanded to another form in order to avoid memory allocations.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */


#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/midifile_parser.h"
#include "spmidi/engine/memtools.h"

#define PRINTF( parms )          { printf parms; }
#define MIDIFILE_SUPPORT_PRINT   (1)

#define INVALID_PARAM_NUMBER (-1)
static int sNonRegParamNumbers[MIDI_NUM_CHANNELS] = { 0 };
static int sRegParamNumbers[MIDI_NUM_CHANNELS] = { 0 };

/****************************************************************/
void DumpSafeString( const unsigned char *addr, int numBytes )
{
    int i;
    for( i=0; i<numBytes; i++ )
    {
        int c = addr[i];
        PRINTF( ("%c", ((c >= 0x20) && (c < 0x7E)) ? c : '.') );
        (void) c; /* Avoid unused variable warnings. */
    }
}

/****************************************************************/
static void DumpMemory( const unsigned char *addr, int numBytes )
{
    int i;
    while( numBytes > 0 )
    {
        int bytesOnLine = (numBytes > 16) ? 16 : numBytes;
        for( i=0; i<bytesOnLine; i++ )
            PRINTF( (" %02X", addr[i]) );
        PRINTF( ("   ") );
        DumpSafeString( addr, bytesOnLine );
        numBytes -= 16;
        addr += 16;
        PRINTF( ("\n") );
    }
}

/****************************************************************/
static int printBeginTrack( MIDIFileParser_t *parser, int index, int size)
{
    (void) parser;
    (void) index;
    (void) size;
    PRINTF( ("Track #%d, %d --------------------\n", index, size ) );
    return 0;
}

/****************************************************************/
static int printEndTrack( MIDIFileParser_t *parser, int index )
{
    (void) parser;
    (void) index;
    PRINTF( ("------------------------------------ %d\n", index ) );
    return 0;
}



/********************************************************************
 * Handle Registered Parameter Number LSB updates.
 */
static void printRPNData( MIDIFileParser_t *parser, int command, int data1, int data2 )
{
    int channelIndex = command & 0x0F;
    (void) parser;
    (void) data1;
    (void) data2;

    switch( sRegParamNumbers[channelIndex] )
    {
    case MIDI_RPN_BEND_RANGE:
        printf(", RPN_BendRange");
        break;
    case MIDI_RPN_FINE_TUNING:
        printf(", RPN_FineTuning");
        break;
    case MIDI_RPN_COARSE_TUNING:
        printf(", RPN_CoarseTuning");
        break;
    default:
        break;
    }
}

/****************************************************************/
static int printController( MIDIFileParser_t *parser, int command, int data1, int data2 )
{
    char *name = NULL;
    char *bitSignificance = "";
    int channelIndex = command & 0x0F;
    (void) parser;
    (void) command;

    if( (0 <= data1) && (data1 <= 31) )
    {
        bitSignificance = " MSB";
    }
    else if( (32 <= data1) && (data1 <= 63) )
    {
        bitSignificance = " LSB";
        data1 -= MIDI_CONTROL_LSB_OFFSET;
    }

    switch( data1 )
    {
    case MIDI_CONTROL_BANK:
        name = "Bank";
        break;
    case MIDI_CONTROL_MODULATION:
        name = "Modulation";
        break;
    case MIDI_CONTROL_DATA_ENTRY:
        name = "Data Entry";
        printRPNData( parser, command, data1, data2 );
        break;
    case MIDI_CONTROL_VOLUME:
        name = "Volume";
        break;
    case MIDI_CONTROL_PAN:
        name = "Pan";
        break;
    case MIDI_CONTROL_EXPRESSION:
        name = "Expression";
        break;
    case MIDI_CONTROL_SUSTAIN:
        name = "Sustain";
        break;
    case MIDI_CONTROL_NONRPN_LSB:
        name = "NonRPN_LSB";
        sNonRegParamNumbers[channelIndex] = (short) ((sNonRegParamNumbers[channelIndex] & 0x3F80) | data2);
        /* Turn off RPN so we don't respond when data entry occurs. */
        sRegParamNumbers[channelIndex] = INVALID_PARAM_NUMBER;
        break;
    case MIDI_CONTROL_NONRPN_MSB:
        name = "NonRPN_MSB";
        sNonRegParamNumbers[channelIndex] = (short) ((sNonRegParamNumbers[channelIndex] & 0x7F) | (data2 << 7));
        sRegParamNumbers[channelIndex] = INVALID_PARAM_NUMBER;
        break;
    case MIDI_CONTROL_RPN_LSB:
        name = "RPN_LSB";
        sRegParamNumbers[channelIndex] = (short) ((sRegParamNumbers[channelIndex] & 0x3F80) | data2);
        sNonRegParamNumbers[channelIndex] = INVALID_PARAM_NUMBER;
        break;
    case MIDI_CONTROL_RPN_MSB:
        name = "RPN_MSB";
        sRegParamNumbers[channelIndex] = (short) ((sRegParamNumbers[channelIndex] & 0x7F) | (data2 << 7));
        sNonRegParamNumbers[channelIndex] = INVALID_PARAM_NUMBER;
        break;

    case 91:
        name = "Effects 1 (Ext) Depth";
        break;
    case 92:
        name = "Effects 2 (Tremolo) Depth";
        break;
    case 93:
        name = "Effects 3 (Chorus) Depth";
        break;
    case 94:
        name = "Effects 4 (Celeste) Depth";
        break;
    case 95:
        name = "Effects 5 (Phaser) Depth";
        break;

    case MIDI_CONTROL_ALLSOUNDOFF:
        name = "AllSoundOff";
        break;
    case MIDI_CONTROL_ALLNOTESOFF:
        name = "AllNotesOff";
        break;

    default:
        name = "?";
        break;
    }

    PRINTF(( ", Controller # %2d (%s%s) set to %3d\n", data1, name, bitSignificance, data2 ));

    return 0;
}

/****************************************************************/
static int printHandleEvent( MIDIFileParser_t *parser, int ticks, int command, int data1, int data2 )
{
    int numBytes;
    int messageType = command & 0x00F0;
    (void) parser;
    (void) data2;
    (void) data1;
    (void) ticks;

    numBytes = SPMIDI_GetBytesPerMessage( command );
    PRINTF( ("Event: ticks = %4d", ticks ) );
    /* Print hex bytes. */
    PRINTF( (", %02X", command ) );
    if( numBytes > 1 )
    {
        PRINTF( (" %02X", data1 ) );
    }
    if( numBytes > 2 )
    {
        PRINTF( (" %02X", data2 ) );
    }

    switch( messageType )
    {
    case MIDI_NOTE_ON:
        PRINTF( (", NoteOn  pitch = %3d, velocity = %3d\n", data1, data2 ) );
        break;
    case MIDI_NOTE_OFF:
        PRINTF( (", NoteOff pitch = %3d, velocity = %3d\n", data1, data2 ) );
        break;
    case MIDI_PROGRAM_CHANGE:
        PRINTF( (", Program instrument = %d = %s\n", data1, MIDI_GetProgramName( data1 ) ) );
        break;
    case MIDI_CONTROL_CHANGE:
        printController( parser, command, data1, data2 );
        break;
    case MIDI_PITCH_BEND:
        {
            int bend = (data2 << 7) + data1;
            PRINTF( (", PitchBend = 0x%04X, offset = %d\n", bend, (bend - MIDI_BEND_NONE) ) );
        }
        break;
    case MIDI_CHANNEL_AFTERTOUCH:
        PRINTF( (", Channel Aftertouch = %d\n", data1 ) );
        break;
    default:
        PRINTF( ("\n") );
        break;
    }
    return 0;
}

/****************************************************************/
static int printHandleMetaEvent( struct MIDIFileParser_s *parser, int ticks, int type,
                                 const unsigned char *addr, int numBytes )
{
    (void) parser;
    (void) ticks;

    PRINTF( ("MetaEvent: ticks = %4d, type = 0x%02X, len = %d\n",
             ticks, type, numBytes ) );

    switch( type )
    {
    case MIDI_META_TEXT_EVENT:
    case MIDI_META_COPYRIGHT:
        PRINTF( ("   ") );
        DumpSafeString( addr, numBytes );
        PRINTF( ("\n") );
        break;

    case MIDI_META_SET_TEMPO:
        {
            int microsPerBeat;

            microsPerBeat = (addr[0] << 16) | (addr[1] << 8) | addr[2];
            PRINTF( ("   Tempo event: microsPerBeat = %d\n", microsPerBeat ) );
        }
        break;

    default:
        DumpMemory( addr, numBytes );
        break;
    }
    return 0;
}

/****************************************************************/
static int printHandleSysExEvent( struct MIDIFileParser_s *parser, int ticks, int type,
                                  const unsigned char *addr, int numBytes )
{
    (void) parser;
    (void) type;
    (void) ticks;

    PRINTF( ("SysExEvent: ticks = %4d, type = 0x%02X, len = %d\n",
             ticks, type, numBytes ) );
    DumpMemory( addr, numBytes );
    return 0;
}

/****************************************************************/
int MIDIFile_Print( unsigned char *image, int numBytes )
{

    MIDIFileParser_t PARSER = { 0 };
    MIDIFileParser_t *parser = &PARSER;

    parser->beginTrackProc = printBeginTrack;
    parser->endTrackProc = printEndTrack;
    parser->handleEventProc = printHandleEvent;
    parser->handleMetaEventProc = printHandleMetaEvent;
    parser->handleSysExProc = printHandleSysExEvent;

    parser->imageStart = image;
    parser->imageSize = numBytes;

    return MIDIFile_Parse( parser );
}
