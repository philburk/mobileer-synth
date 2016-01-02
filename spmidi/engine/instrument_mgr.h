#ifndef _INSTRUMENT_MGR_H
#define _INSTRUMENT_MGR_H

/* $Id$ */
/**
 *
 * Manager for Externally Loaded Instruments.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/spmidi.h"
#include "resource_mgr.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi_voice.h"

#ifdef __cplusplus
extern "C"
{
#endif

void InsManager_InitializePreset( HybridVoice_Preset_t *preset );

#ifdef __cplusplus
}
#endif

#endif /* _INSTRUMENT_MGR_H */

