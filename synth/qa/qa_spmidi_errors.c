/* $Id: qa_spmidi_errors.c,v 1.5 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_spmidi_errors.c
 * @brief Test error return codes for illegal SPMIDI parameters.
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

#include "spmidi/qa/qa_tools.h"

short buffer[1024];

int TestErrors( void )
{
    int result = 0;
    SPMIDI_Context *spmidiContext = (SPMIDI_Context *) 0xcafebaba;

    SPMIDI_Initialize();

    QA_Assert( SPMIDI_CreateContext( &spmidiContext, 12345 ) == SPMIDI_Error_UnsupportedRate,
        "Trapped unsupported sample rate." );

    QA_Assert( (spmidiContext == NULL),
        "spmidiContext should be NULL after failure." );

    QA_Assert( SPMIDI_CreateContext( &spmidiContext, 96000 ) == SPMIDI_Error_OutOfRange,
        "Trapped over SPMIDI_MAX_SAMPLE_RATE." );

    QA_Assert( SPMIDI_CreateContext( &spmidiContext, 22050 ) == 0,
        "Created good SPMIDI context." );

    QA_Assert( SPMIDI_SetMaxVoices( spmidiContext, 100 ) == SPMIDI_MAX_VOICES,
        "Clipped maxVoices to SPMIDI_MAX_VOICES" );

    QA_Assert( SPMIDI_ReadFrames( spmidiContext, buffer, 512, 2, 33 ) == SPMIDI_Error_IllegalSize,
        "Trapped 33 bits per sample." );
    QA_Assert( SPMIDI_ReadFrames( spmidiContext, buffer, 512, 2, 7 ) == SPMIDI_Error_IllegalSize,
        "Trapped 7 bits per sample." );

    QA_Assert( SPMIDI_ReadFrames( spmidiContext, buffer, 512, 0, 16 ) == SPMIDI_Error_OutOfRange,
        "Trapped 0 samples per frame." );
    QA_Assert( SPMIDI_ReadFrames( spmidiContext, buffer, 512, 3, 16 ) == SPMIDI_Error_OutOfRange,
        "Trapped 0 samples per frame." );

    
    QA_Assert( SPMIDI_GetBytesPerMessage( 0x007F ) == 0,
        "Trapped below MIDI command range." );
    QA_Assert( SPMIDI_GetBytesPerMessage( 0x0100 ) == 0,
        "Trapped above MIDI command range." );
    
    QA_Assert( SPMIDI_SetChannelEnable( spmidiContext, -1, 1 ) == SPMIDI_Error_IllegalChannel,
        "Trapped negative channel." );
    QA_Assert( SPMIDI_SetChannelEnable( spmidiContext, 16, 1 ) == SPMIDI_Error_IllegalChannel,
        "Trapped above channel range." );
    
    
    QA_Assert( SPMIDI_SetParameter( spmidiContext, -1, 0 ) == SPMIDI_Error_UnrecognizedParameter,
        "Trapped negative parameter." );

    QA_Assert( SPMIDI_SetParameter( spmidiContext, 7, 0 ) == SPMIDI_Error_UnrecognizedParameter,
        "Trapped above parameter range." );

    SPMIDI_Terminate();

    return result;

}

int main(void);
int main(void)
{
    QA_Init( "qa_spmidi_errors" );

    TestErrors();

    return QA_Term( 15 );
}

