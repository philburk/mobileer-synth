/* $Id: program_list.c,v 1.1 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * @file program_list.c
 * @brief List of bank/program/pitches used in a set of songs.
 *
 * @author Phil Burk, Copyright 2007 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include <math.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_util.h"

#include "spmidi/engine/memtools.h"
#include "spmidi/include/program_list.h"
#include "spmidi/engine/dbl_list.h"

typedef void SPMIDI_ProgramList;

typedef struct SPMIDI_ProgramBankBits_s
{
    DoubleNode     node;
    spmUInt32      bits[ 128/32 ];
    spmSInt16      bankIndex;
    spmSInt16      programIndex;  // only used with drum banks
} SPMIDI_ProgramBankBits_t;

typedef struct SPMIDI_ProgramList_s
{
    DoubleList melodicBankList;
    DoubleList drumBankList;
} SPMIDI_ProgramList_t;


/********************************************************************************/
/** Free all the banks in the list. */
static void SS_DeleteBanks( DoubleList *bankList )
{
    while( !DLL_IsEmpty( bankList ) )
    {
        SPMIDI_ProgramBankBits_t *programBank = (SPMIDI_ProgramBankBits_t *) DLL_First( bankList );
        DLL_Remove( &programBank->node );
        SPMIDI_FREE_MEM( programBank );
    }
}

/********************************************************************************/
static SPMIDI_ProgramBankBits_t *SS_AllocateProgramBankNode( int bankIndex )
{
    SPMIDI_ProgramBankBits_t *programBank = SPMIDI_ALLOC_MEM( sizeof(SPMIDI_ProgramBankBits_t), "SPMIDI_ProgramBankBits_t" );
    if( programBank == NULL )
    {
        return NULL;
    }
    MemTools_Clear( programBank, sizeof( SPMIDI_ProgramBankBits_t ) );

    DLL_InitNode( &programBank->node );
    programBank->bankIndex = (short) bankIndex;
    programBank->programIndex = (short) 0;
    return programBank;
}

/********************************************************************************/
static SPMIDI_ProgramBankBits_t *SS_FindBank( DoubleList *bankList, int bankIndex, int programIndex )
{
    SPMIDI_ProgramBankBits_t *programBank = NULL;
    SPMIDI_ProgramBankBits_t *candidate;
    DLL_FOR_ALL( SPMIDI_ProgramBankBits_t, candidate, bankList )
    {
        if( (((int)candidate->bankIndex) == bankIndex) && (((int)candidate->programIndex) == programIndex) )
        {
            programBank = candidate;
            break;
        }
    }
    return programBank;
}

#define SETBIT( array, index ) (array)[(index)>>5] |= (1<<((index)&31))
#define CLEARBIT( array, index ) (array)[(index)>>5] &= ~(1<<((index)&31))
#define ISBIT( array, index ) (((array)[(index)>>5] & (1<<((index)&31))) != 0)

/*******************************************************************/
static int SS_SetBankProgramUsed( DoubleList *bankList, int bank, int program, int item )
{
    SPMIDI_ProgramBankBits_t *programBank = SS_FindBank( bankList, bank, program );
    if( (program < 0) || (program >= 128) )
    {
        return SPMIDI_Error_OutOfRange;
    }
    if( (item < 0) || (item >= 128) )
    {
        return SPMIDI_Error_OutOfRange;
    }
    if( programBank == NULL )
    {
        programBank = SS_AllocateProgramBankNode( bank );
        if( programBank == NULL ) return SPMIDI_Error_OutOfMemory;
        programBank->programIndex = (spmSInt16) program;
        DLL_AddTail( bankList, &programBank->node );
    }
    SETBIT( programBank->bits, item );
    return 0;
}

/*******************************************************************/
static int SS_ClearBankProgramUsed( DoubleList *bankList, int bank, int program, int item )
{
    SPMIDI_ProgramBankBits_t *programBank = SS_FindBank( bankList, bank, program );
    if( (program < 0) || (program >= 128) )
    {
        return SPMIDI_Error_OutOfRange;
    }
    if( (item < 0) || (item >= 128) )
    {
        return SPMIDI_Error_OutOfRange;
    }
    if( programBank != NULL )
    {
        CLEARBIT( programBank->bits, item );
    }
    return 0;
}

/*******************************************************************/
static int SS_IsBankProgramUsed( DoubleList *bankList, int bank, int program, int item )
{
    SPMIDI_ProgramBankBits_t *programBank = SS_FindBank( bankList, bank, program );
    if( (program < 0) || (program >= 128) )
    {
        return SPMIDI_Error_OutOfRange;
    }
    if( (item < 0) || (item >= 128) )
    {
        return SPMIDI_Error_OutOfRange;
    }
    if( programBank == NULL )
    {
        return 0;
    }
    return ISBIT( programBank->bits, item );
}

/*******************************************************************/
int SPMIDI_SetProgramUsed( SPMIDI_ProgramList *programList, int bank, int program )
{
    SPMIDI_ProgramList_t *programList_t = (SPMIDI_ProgramList_t *)programList;
    return SS_SetBankProgramUsed( &programList_t->melodicBankList, bank, 0, program );
}
/*******************************************************************/
int SPMIDI_SetDrumUsed( SPMIDI_ProgramList *programList, int bank, int program, int noteIndex )
{
    SPMIDI_ProgramList_t *programList_t = (SPMIDI_ProgramList_t *)programList;
    return SS_SetBankProgramUsed( &programList_t->drumBankList, bank, program, noteIndex );
}
/*******************************************************************/
int SPMIDI_ClearProgramUsed( SPMIDI_ProgramList *programList, int bank, int program )
{
    SPMIDI_ProgramList_t *programList_t = (SPMIDI_ProgramList_t *)programList;
    return SS_ClearBankProgramUsed( &programList_t->melodicBankList, bank, 0, program );
}
/*******************************************************************/
int SPMIDI_ClearDrumUsed( SPMIDI_ProgramList *programList, int bank, int program, int noteIndex )
{
    SPMIDI_ProgramList_t *programList_t = (SPMIDI_ProgramList_t *)programList;
    return SS_ClearBankProgramUsed( &programList_t->drumBankList, bank, program, noteIndex );
}

/*******************************************************************/
int SPMIDI_IsProgramUsed( SPMIDI_ProgramList *programList, int bank, int program )
{
    SPMIDI_ProgramList_t *programList_t = (SPMIDI_ProgramList_t *)programList;
    return SS_IsBankProgramUsed( &programList_t->melodicBankList, bank, 0, program );
}
/*******************************************************************/
int SPMIDI_IsDrumUsed( SPMIDI_ProgramList *programList, int bank, int program, int noteIndex )
{
    SPMIDI_ProgramList_t *programList_t = (SPMIDI_ProgramList_t *)programList;
    return SS_IsBankProgramUsed( &programList_t->drumBankList, bank, program, noteIndex );
}

/*******************************************************************/
int SPMIDI_CreateProgramList( SPMIDI_ProgramList **programListPtr )
{
    SPMIDI_ProgramList_t *programList;

    programList = (SPMIDI_ProgramList_t *)SPMIDI_ALLOC_MEM( sizeof(SPMIDI_ProgramList_t), "SPMIDI_CreateProgramList" ) ;
    if( programList == NULL )
    {
        return SPMIDI_Error_OutOfMemory;
    }

    MemTools_Clear( programList, sizeof( SPMIDI_ProgramList_t ) );
    DLL_InitList( &programList->melodicBankList );
    DLL_InitList( &programList->drumBankList );
    *programListPtr = (SPMIDI_ProgramList *) programList;
    return 0;
}

/********************************************************************************/
void SPMIDI_DeleteProgramList( SPMIDI_ProgramList *programList )
{
    SPMIDI_ProgramList_t *programList_t = (SPMIDI_ProgramList_t *) programList;
    if( programList_t != NULL )
    {
        SS_DeleteBanks( &programList_t->melodicBankList );
        SS_DeleteBanks( &programList_t->drumBankList );
        SPMIDI_FREE_MEM( programList_t );
    }
}

/****************************************************************/
/************* MIDIFile Scanner to build ProgramLists ***********/
/****************************************************************/

typedef struct ChannelStatus_s
{
    SPMIDI_ProgramList *programList;
    spmSInt16 bank[ 16 ];
    spmSInt16 program[ 16 ];
} ChannelStatus_t;

/****************************************************************/
static int ProgramScanner_HandleEvent( MIDIFileParser_t *parser, int ticks,
        int command, int data1, int data2 )
{
    (void) ticks;
    int messageType = command & 0x00F0;
    ChannelStatus_t *channelStatus = (ChannelStatus_t *) parser->userData;
    int channel = command & 0x0F;

    switch( messageType )
    {
    case MIDI_NOTE_ON:
        {
            SPMIDI_ProgramList *programList = (SPMIDI_ProgramList *) channelStatus->programList;
            int bank = channelStatus->bank[ channel ];
            int program = channelStatus->program[ channel ];
            if( channel == MIDI_RHYTHM_CHANNEL_INDEX )
            {
                /* Clip to legal range for GM drums. */
                if( data1 < GMIDI_FIRST_DRUM )
                    data1 = GMIDI_FIRST_DRUM;
                else if( data1 > GMIDI_LAST_DRUM )
                    data1 = GMIDI_LAST_DRUM;
                SPMIDI_SetDrumUsed( programList, bank, program, data1 );
            }
            else
            {
                SPMIDI_SetProgramUsed( programList, bank, program );
            }
        }
        break;

    case MIDI_PROGRAM_CHANGE:
        channelStatus->program[ channel ] = (spmSInt16) data1;
        break;

    case MIDI_CONTROL_CHANGE:
        {
            if( data1 == MIDI_CONTROL_BANK)
            {
                channelStatus->bank[ channel ] = (spmSInt16) (data2 << SPMIDI_BANK_MSB_SHIFT);
            }
            else if( data1 == (MIDI_CONTROL_BANK + MIDI_CONTROL_LSB_OFFSET))
            {
                channelStatus->bank[ channel ] = (spmSInt16) ((channelStatus->bank[ channel ] & ~0x7F) | data2);
            }
        }
        break;

    default:
        break;
    }
    return 0;
}

/****************************************************************/
int MIDIFile_ScanForPrograms( SPMIDI_ProgramList *programList, unsigned char *image, int numBytes  )
{
    MIDIFileParser_t PARSER = { 0 };
    ChannelStatus_t channelStatus = {0};
    MIDIFileParser_t *parser = &PARSER;

    if( programList == NULL ) return SPMIDI_Error_IllegalArgument;

    parser->beginTrackProc = NULL;
    parser->endTrackProc = NULL;
    parser->handleEventProc = ProgramScanner_HandleEvent;
    parser->handleMetaEventProc = NULL;
    parser->handleSysExProc = NULL;

    parser->imageStart = image;
    parser->imageSize = numBytes;

    channelStatus.programList = programList;
    parser->userData = &channelStatus;

    return MIDIFile_Parse( parser );
}
