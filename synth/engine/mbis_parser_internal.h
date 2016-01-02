
#ifndef _MBIS_PARSER_INTERNAL_H
#define _MBIS_PARSER_INTERNAL_H
/**
   mbis_parser_internal.h
 
   Copyright (C) Phil Burk 2007, Mobileer, PROPRIETARY and CONFIDENTIAL
*/

#include "spmidi/engine/parse_riff.h"
#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_preset.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/include/mbis_parser.h"

#define FOURCC_MBIS   MakeFourCC('M','B','I','S')
#define FOURCC_INFO  MakeFourCC('I','N','F','O')
#define FOURCC_DATE  MakeFourCC('D','A','T','E')
#define FOURCC_AUTH  MakeFourCC('A','U','T','H')
#define FOURCC_CPRT  MakeFourCC('C','P','R','T')
#define FOURCC_EDVN  MakeFourCC('E','D','V','N')
#define FOURCC_FMVN  MakeFourCC('F','M','V','N')
#define FOURCC_CMNT  MakeFourCC('C','M','N','T')
#define FOURCC_INSS  MakeFourCC('I','N','S','S')
#define FOURCC_INST  MakeFourCC('I','N','S','T')
#define FOURCC_MAPS  MakeFourCC('M','A','P','S')
#define FOURCC_MMAP  MakeFourCC('M','M','A','P')
#define FOURCC_DMAP  MakeFourCC('D','M','A','P')
#define FOURCC_WVST  MakeFourCC('W','V','S','T')
#define FOURCC_WTBL  MakeFourCC('W','T','B','L')
#define FOURCC_NRSC  MakeFourCC('N','R','S','C')
#define FOURCC_IDEP  MakeFourCC('I','D','E','P')
#define FOURCC_WDEP  MakeFourCC('W','D','E','P')
// #define FOURCC_DATE  MakeFourCC('','','','')

typedef struct MBIS_Instrument_s
{
    spmUInt16             bankID;
    spmUInt16             programID;
}
MBIS_Instrument_t;

typedef struct MBIS_Orchestra_s
{
    DoubleList                  waves;
    spmSInt                     numInstruments;
    MBIS_Instrument_t           *instruments;
    spmSInt32                   reserved; /* For patching. */
}
MBIS_Orchestra_t;


#endif /* _MBIS_PARSER_INTERNAL_H */
