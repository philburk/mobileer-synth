/* $Id: spmidi_errortext.c,v 1.10 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Error handling and reporting functions
 *
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_errortext.h"

#define TOUCH(value) \
    (void)value

#if SPMIDI_PRODUCTION  /* if we're in production mode, don't use text because it takes RAM */

/******************************************************************/
const char *SPMUtil_GetErrorText( SPMIDI_Error errorCode )
{
    TOUCH( errorCode );
    return "No error text. SPMUtil_GetErrorText() disabled.";
}

#else /* if we're in development mode, use full error text */

/******************************************************************/
const char *SPMUtil_GetErrorText( SPMIDI_Error errorCode )
{
    const char *text;
    switch( errorCode )
    {
    case 0:
        text = "No error.";
        break;
    case MIDIFile_Error_NotSMF:
        text = "The file being parsed is not a standard MIDI file!";
        break;
    case MIDIFile_Error_IllegalFormat:
        text = "The MIDI file may be damaged.";
        break;
    case MIDIFile_Error_IllegalTrackIndex:
        text = "TrankIndex is out of range.";
        break;
    case MIDIFile_Error_MissingEndOfTrack:
        text = "We ran out of bytes in the track before finding the EndOfTrack MetaEvent.";
        break;
    case MIDIFile_Error_PrematureEndOfTrack:
        text = "We still had bytes left in the track after finding the EndOfTrack MetaEvent.";
        break;
    case MIDIFile_Error_IllegalMetaEventType:
        text = "MetaEvent type not in range 0-127.";
        break;
    case MIDIFile_Error_TooManyTracks:
        text = "More than MIDIFILE_MAX_TRACKS tracks.";
        break;
    case SPMIDI_Error_IllegalChannel:
        text = "The channelIndex is out of range. Must be 0-15.";
        break;
    case SPMIDI_Error_NotStarted:
        text = "SPMIDI_CreateContext() not called before using SPMIDI.";
        break;
    case SPMIDI_Error_IllegalSize:
        text = "Size parameter out of range.";
        break;
    case SPMIDI_Error_UnsupportedRate:
        text = "Sample rate not supported. Please add entry to \"spmidi_hybrid.c\".";
        break;
    case SPMIDI_Error_UnrecognizedParameter:
        text = "Parameter index not supported by SPMIDI_SetParameter().";
        break;
    case SPMIDI_Error_OutOfRange:
        text = "Value is out of range.";
        break;
    case SPMIDI_Error_OutOfMemory:
        text = "Could not allocate memory.";
        break;
    case SPMIDI_Error_BadFormat:
        text = "Incorrect data format.";
        break;
    case SPMIDI_Error_BadToken:
        text = "Illegal resource token.";
        break;
    case SPMIDI_Error_DLSAlreadyLoaded:
        text = "One can only load one DLS orchestra per SPMIDI context.";
        break;

    case MIDIStream_Error_NotSMID:
        text = "The file is not Mobileer MIDIStream.";
        break;

#if SPMIDI_ME3000
        /*
        * These error are only used by the ME3000 API.
        */
    case DLSParser_Error_NotDLS:
        text = "The file is not Mobile DLS.";
        break;
    case DLSParser_Error_ParseError:
        text = "The DLS parser cannot parse this file.";
        break;
    case DLSParser_Error_UnsupportedSampleFormat:
        text = "The sample information is not in a supported format.";
        break;
    case DLSParser_Error_NotParsed:
        text = "You cannot load a DLS file into the engine without parsing it first.";
        break;
    case XMFParser_Error_StreamError:
        text = "Error processing the XMF stream.";
        break;
    case XMFParser_Error_NotXMF:
        text = "The file is not XMF.";
        break;
    case XMFParser_Error_WrongType:
        text = "The file is not a type 2 Mobile XMF file.";
        break;
    case XMFParser_Error_ParseError:
        text = "The XMF parser cannot parser this file.";
        break;
    case XMFParser_Error_SizeError:
        text = "The XMF file's stated size does not agree with the actual image size.";
        break;
    case XMFParser_Error_WrongDLSType:
        text = "XMF requires a MobileDLS format.";
        break;
    case XMFParser_Error_ExtraSMF:
        text = "MobileXMF supports only one SMF per file.";
        break;
    case XMFParser_Error_DetachedNodeContentFound:
        text = "XMF Node contains detached content, but MobileXMF does not allow this.";
        break;
    case XMFParser_Error_VLQTooLarge:
        text = "VLQ exceeds stated maximum";
        break;
#endif /* SPMIDI_ME3000 */

    default:
        text = "Unrecognized error.";
        break;
    }
    return text;
}

#endif
