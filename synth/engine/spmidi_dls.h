#ifndef _SPMIDI_DLS_H
#define _SPMIDI_DLS_H
/* $Id: spmidi_dls.h,v 1.11 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * SPMIDI suport for DLS.
 *
 * @author Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "dls_parser_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Lookup an interpolated value in a table of Unsigned Shorts.
     * @param table lookup table
     * @param input is normalized to a 16.16 fraction, high portion becomes index
     */
    spmSInt32 SSDLS_UShortTableLookup( const spmUInt16 *table, int numEntries, spmUInt32 input  );


typedef int (*SSDLS_LoadOrchestraProcPtr)( SoftSynth *synth, DLS_Orchestra_t *dlsOrch );

    /** Allow override of function used to implement SSDLS_LoadDLSOrchestra()
     * This allow modification to code in ROM.
     */
    SSDLS_LoadOrchestraProcPtr SSDLS_GetProc_LoadOrchestra( void );
    void SSDLS_SetProc_LoadOrchestra( SSDLS_LoadOrchestraProcPtr proc );

    /**
     * Load an orchestra for use by SPMIDI.
     */
    int SSDLS_LoadOrchestra( SoftSynth *synth, DLS_Orchestra_t *dlsOrch );

    /**
     * Find first Instrument in Collection that matches the bank and program.
     */
    DLS_Instrument_t *SSDLS_FindInstrument( DLS_Orchestra_t *dlsOrch, int bankIndex, int programIndex );

    /**
     * Find first region in instrument that matches the pitch and velocity.
     */
    DLS_Region_t *SSDLS_FindMatchingRegion( DLS_Instrument_t *dlsIns, int pitch, int velocity );

    /**
     * Note that you can only call this function once for a given context.
     * Do not modify or free that dlsOrch after calling this function
     * until calling SPMIDI_DeleteContext().
     */
    int SPMIDI_LoadDLSOrchestra( SPMIDI_Context *spmidiContext, DLS_Orchestra_t *dlsOrch );
    SoftSynth *SPMIDI_GetSynth( SPMIDI_Context *spmidiContext );

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_DLS_H */
