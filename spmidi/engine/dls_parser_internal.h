
#ifndef _DLS_PARSER_INTERNAL_H
#define _DLS_PARSER_INTERNAL_H
/**
 
   dls_parser_internal.h
 
   Description:
  
   Interface defines and structures for the Instrument Collection Form, RIFF DLS.
 
   Derived from "dls.h" written by Sonic Foundry 1996 and released for public use.
 
   Modifications and extensions by (C) Phil Burk 2002, Mobileer, PROPRIETARY and CONFIDENTIAL
 
*/

#include "spmidi/engine/parse_riff.h"
#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_preset.h"
#include "spmidi/engine/wave_manager.h"
#include "spmidi/include/dls_parser.h"

/*
 
   Layout of an instrument collection:
 
 
   RIFF [] 'DLS ' [colh,INSTLIST,WAVEPOOL,INFOLIST]
 
   INSTLIST
   LIST [] 'lins'
                 LIST [] 'ins ' [insh,RGNLIST,ARTLIST,INFOLIST]
                 LIST [] 'ins ' [insh,RGNLIST,ARTLIST,INFOLIST]
                 LIST [] 'ins ' [insh,RGNLIST,ARTLIST,INFOLIST]
 
   RGNLIST
   LIST [] 'lrgn' 
                 LIST [] 'rgn '  [rgnh,wsmp,wlnk,ARTLIST]
                 LIST [] 'rgn '  [rgnh,wsmp,wlnk,ARTLIST]
                 LIST [] 'rgn '  [rgnh,wsmp,wlnk,ARTLIST]
 
   ARTLIST
   LIST [] 'lart'
           'art1' level 1 Articulation connection graph
           'art2' level 2 Articulation connection graph
           '3rd1' Possible 3rd party articulation structure 1
           '3rd2' Possible 3rd party articulation structure 2 .... and so on
 
   WAVEPOOL 
   ptbl [] [pool table]
   LIST [] 'wvpl'
                 [path],
                 [path],
                 LIST [] 'wave',RIFFWAVE
                 LIST [] 'wave',RIFFWAVE
                 LIST [] 'wave',RIFFWAVE
                 LIST [] 'wave',RIFFWAVE
                 LIST [] 'wave',RIFFWAVE
 
   INFOLIST
   LIST [] 'INFO' 
                 'icmt' 'One of those crazy comments.'
                 'icop' 'Copyright (C) 1996 Sonic Foundry'
  
*/


/*
   FOURCC's used in the DLS file
*/

#define FOURCC_DLS   MakeFourCC('D','L','S',' ')
 #define FOURCC_COLH  MakeFourCC('c','o','l','h')
 #define FOURCC_WVPL  MakeFourCC('w','v','p','l')
 #define FOURCC_PTBL  MakeFourCC('p','t','b','l')
 #define FOURCC_PATH  MakeFourCC('p','a','t','h')
 #define FOURCC_wave  MakeFourCC('w','a','v','e')
 #define FOURCC_LINS  MakeFourCC('l','i','n','s')
 #define FOURCC_INS   MakeFourCC('i','n','s',' ')
 #define FOURCC_INSH  MakeFourCC('i','n','s','h')
 #define FOURCC_LRGN  MakeFourCC('l','r','g','n')
 #define FOURCC_RGN   MakeFourCC('r','g','n',' ')
 #define FOURCC_RGNH  MakeFourCC('r','g','n','h')
 #define FOURCC_LART  MakeFourCC('l','a','r','t')
 #define FOURCC_ART1  MakeFourCC('a','r','t','1')
 #define FOURCC_WLNK  MakeFourCC('w','l','n','k')
 #define FOURCC_WSMP  MakeFourCC('w','s','m','p')
 #define FOURCC_VERS  MakeFourCC('v','e','r','s')
 #define FOURCC_ICOP  MakeFourCC('I','C','O','P')
 #define FOURCC_INAM  MakeFourCC('I','N','A','M')

#define FOURCC_FMT   MakeFourCC('f','m','t',' ')
 #define FOURCC_DATA  MakeFourCC('d','a','t','a')
 #define FOURCC_WAVE  MakeFourCC('w','a','v','e')

#define FOURCC_FACT  MakeFourCC('f','a','c','t')
 #define FOURCC_ICMT  MakeFourCC('I','C','M','T')
 #define FOURCC_IENG  MakeFourCC('I','E','N','G')
 #define FOURCC_ISFT  MakeFourCC('I','S','F','T')


/*
     FOURCC's used in the DLS2 file, in addition to DLS1 chunks
*/

#define FOURCC_RGN2  MakeFourCC('r','g','n','2')
#define FOURCC_LAR2  MakeFourCC('l','a','r','2')
#define FOURCC_ART2  MakeFourCC('a','r','t','2')
#define FOURCC_CDL   MakeFourCC('c','d','l',' ')
#define FOURCC_DLID  MakeFourCC('d','l','i','d')

/*
   Articulation connection graph definitions (DLS 2)
*/

/* Generic Sources */
#define CONN_SRC_NONE              0x0000
#define CONN_SRC_LFO               0x0001
#define CONN_SRC_KEYONVELOCITY     0x0002
#define CONN_SRC_KEYNUMBER         0x0003
#define CONN_SRC_EG1               0x0004
#define CONN_SRC_EG2               0x0005
#define CONN_SRC_PITCHWHEEL        0x0006
#define CONN_SRC_POLYPRESSURE      0x0007
#define CONN_SRC_CHANNELPRESSURE   0x0008
#define CONN_SRC_VIBRATO           0x0009

/* Midi Controllers 0-127 */
#define CONN_SRC_CC1               0x0081
#define CONN_SRC_CC7               0x0087
#define CONN_SRC_CC10              0x008a
#define CONN_SRC_CC11              0x008b
#define CONN_SRC_CC91              0x00db   /* Reverb Send */
#define CONN_SRC_CC93              0x00dd   /* Chorus Send */

/* Registered Parameter Numbers */
#define CONN_SRC_RPN0             0x0100
#define CONN_SRC_RPN1             0x0101
#define CONN_SRC_RPN2             0x0102

/* Generic Destinations */
#define CONN_DST_NONE              0x0000
#define CONN_DST_ATTENUATION       0x0001
#define CONN_DST_RESERVED          0x0002
#define CONN_DST_PITCH             0x0003
#define CONN_DST_PAN               0x0004
#define CONN_DST_GAIN              0x0001   /* Same as CONN_DST_ ATTENUATION, but more appropriate terminology. */
#define CONN_DST_KEYNUMBER         0x0005  /* Key Number Generator */

/* Audio Channel Output Destinations */
#define CONN_DST_LEFT              0x0010   /* Left Channel Send */
#define CONN_DST_RIGHT             0x0011   /* Right Channel Send */
#define CONN_DST_CENTER            0x0012   /* Center Channel Send */
#define CONN_DST_LEFTREAR          0x0013   /* Left Rear Channel Send */
#define CONN_DST_RIGHTREAR         0x0014   /* Right Rear Channel Send */
#define CONN_DST_LFE_CHANNEL       0x0015   /* LFE Channel Send */
#define CONN_DST_CHORUS            0x0080   /* Chorus Send */
#define CONN_DST_REVERB            0x0081   /* Reverb Send */

/* LFO Destinations */
#define CONN_DST_LFO_FREQUENCY     0x0104
#define CONN_DST_LFO_STARTDELAY    0x0105
#define CONN_DST_VIB_FREQUENCY     0x0114   /* Vibrato Frequency */
#define CONN_DST_VIB_STARTDELAY    0x0115   /* Vibrato Start Delay */

/* EG1 Destinations */
#define CONN_DST_EG1_ATTACKTIME    0x0206
#define CONN_DST_EG1_DECAYTIME     0x0207
#define CONN_DST_EG1_RESERVED      0x0208
#define CONN_DST_EG1_RELEASETIME   0x0209
#define CONN_DST_EG1_SUSTAINLEVEL  0x020a
#define CONN_DST_EG1_DELAYTIME     0x020B   /* EG1 Delay Time */
#define CONN_DST_EG1_HOLDTIME      0x020C   /* EG1 Hold Time */
#define CONN_DST_EG1_SHUTDOWNTIME  0x020D   /* EG1 Shutdown Time */

/* EG2 Destinations */
#define CONN_DST_EG2_ATTACKTIME    0x030a
#define CONN_DST_EG2_DECAYTIME     0x030b
#define CONN_DST_EG2_RESERVED      0x030c
#define CONN_DST_EG2_RELEASETIME   0x030d
#define CONN_DST_EG2_SUSTAINLEVEL  0x030e
#define CONN_DST_EG2_DELAYTIME     0x030F   /* EG2 Delay Time */
#define CONN_DST_EG2_HOLDTIME      0x0310   /* EG2 Hold Time */

/* Filter Destinations */
#define CONN_DST_FILTER_CUTOFF     0x0500   /* Filter Cutoff Frequency */
#define CONN_DST_FILTER_Q          0x0501   /* Filter Resonance */

#define CONN_TRN_NONE              0x0000
#define CONN_TRN_CONCAVE           0x0001
#define CONN_TRN_CONVEX            0x0002   /* Convex Transform */
#define CONN_TRN_SWITCH            0x0003   /* Switch Transform */

typedef enum DLS_ArticulationTokens_e
{
    CONN_Z_UNRECOGNIZED,
    CONN_Z_TUNING,
    CONN_Z_VIB_LFO_TO_PITCH,
    CONN_Z_MOD_LFO_TO_PITCH,
    CONN_Z_EG2_TO_PITCH,
    CONN_Z_LFO_FREQUENCY,
    CONN_Z_VIB_FREQUENCY,
    CONN_Z_LFO_STARTDELAY,
    CONN_Z_VIB_STARTDELAY,

    CONN_Z_EG1_DELAYTIME,
    CONN_Z_EG1_ATTACKTIME,
    CONN_Z_EG1_HOLDTIME,
    CONN_Z_EG1_DECAYTIME,
    CONN_Z_EG1_RELEASETIME,
    CONN_Z_EG1_SUSTAINLEVEL,

    CONN_Z_EG2_DELAYTIME,
    CONN_Z_EG2_ATTACKTIME,
    CONN_Z_EG2_HOLDTIME,
    CONN_Z_EG2_DECAYTIME,
    CONN_Z_EG2_RELEASETIME,
    CONN_Z_EG2_SUSTAINLEVEL,

    CONN_Z_KEY_TO_PITCH,
    CONN_Z_VIB_LFO_CC1_TO_PITCH,
    CONN_Z_MOD_LFO_CC1_TO_PITCH,
    CONN_Z_VIB_LFO_CPR_TO_PITCH,
    CONN_Z_MOD_LFO_CPR_TO_PITCH
} DLS_ArticulationTokens;

#define DLS_MAX_MELODIC_REGIONS      (16)
#define DLS_INS_PER_BANK            (128)

/*
Conditional chunk operators
*/
#define DLS_CDL_AND                0x0001
#define DLS_CDL_OR                 0x0002
#define DLS_CDL_XOR                0x0003
#define DLS_CDL_ADD                0x0004
#define DLS_CDL_SUBTRACT           0x0005
#define DLS_CDL_MULTIPLY           0x0006
#define DLS_CDL_DIVIDE             0x0007
#define DLS_CDL_LOGICAL_AND        0x0008
#define DLS_CDL_LOGICAL_OR         0x0009
#define DLS_CDL_LT                 0x000A
#define DLS_CDL_LE                 0x000B
#define DLS_CDL_GT                 0x000C
#define DLS_CDL_GE                 0x000D
#define DLS_CDL_EQ                 0x000E
#define DLS_CDL_NOT                0x000F
#define DLS_CDL_CONST              0x0010
#define DLS_CDL_QUERY              0x0011
#define DLS_CDL_QUERY_SUPPORTED    0x0012

/*
  DLSID queries for <cdl-ck>
*/

typedef struct GUID_s
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID_t;

#define DLSID_GMInHardware     { 0x178f2f24, 0xc364, 0x11d1, { 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12 } }
#define DLSID_GSInHardware     { 0x178f2f25, 0xc364, 0x11d1, { 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12 } }
#define DLSID_XGInHardware     { 0x178f2f26, 0xc364, 0x11d1, { 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12 } }
#define DLSID_SupportsDLS1     { 0x178f2f27, 0xc364, 0x11d1, { 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12 } }
#define DLSID_SupportsDLS2     { 0xf14599e5, 0x4689, 0x11d2, { 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6 } }
#define DLSID_SampleMemorySize { 0x178f2f28, 0xc364, 0x11d1, { 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12 } }
#define DLSID_ManufacturersID  { 0xb03e1181, 0x8095, 0x11d2, { 0xa1, 0xef, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8 } }
#define DLSID_ProductID        { 0xb03e1182, 0x8095, 0x11d2, { 0xa1, 0xef, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8 } }
#define DLSID_SamplePlaybackRate { 0x2a91f713, 0xa4bf, 0x11d2, { 0xbb, 0xdf, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8 } }

#define DLSID_SupportsMobileDLS { 0x3104088d, 0x2fc6, 0x4a6a, { 0xa6, 0xc7, 0x8d, 0x53, 0x3, 0x5a, 0xfe, 0x1a } }
#define DLSID_SupportsMobileDLSOptionalBlocks { 0xed8e8a37, 0x97f9, 0x46cd, { 0x92, 0x2f, 0x6d, 0xd4, 0x36, 0xf8, 0xdb, 0x8d } }

#define DLS_MANUFACTURERS_ID   (0x00000152)
#define DLS_PRODUCT_ID         (0)
#define DLS_DEFAULT_SAMPLERATE (22050)

#define CDLTRUE (-1)
#define CDLFALSE (0)

#ifndef boolean
typedef char   boolean;
#endif

#ifndef TRUE
#define TRUE              (1)
#endif

#ifndef FALSE
#define FALSE             (0)
#endif


/* Describes how to use a DLS_Wave_t
 * Note: If this structure is modified then you must change the
 * static initialization of sDefaultWaveSample!
 */
typedef struct DLS_WaveSample_s
{
    /* from wsmp chunk */
    spmUInt8      basePitch;
    spmUInt8      loopType;
    spmSInt16     fineTune;
    spmSInt32     gain;
/*  spmUInt32     sampleOptions; This is not currently stored because we do not truncate or compress wave data. */
    spmSInt32     loopStart;
    spmSInt32     loopSize;
}
DLS_WaveSample_t;

typedef struct DLS_Wave_s
{
    DoubleNode        node;
    /* Offset from wavePoolBase. Used for matching entries in rawPoolTable. */
    spmUInt32         waveOffset;
    /* If this is NULL, then use default global wavesample.
     * Can be overridden by region waveSample
     */
    DLS_WaveSample_t *wavesample;
    spmSInt32         numSamples;
    /* sample data array */
    void             *samples;
    spmSInt32         sampleRate;
    spmUInt16         format;
    spmUInt8          numChannels; /* TODO This is not used.    PLB051012 */
    spmUInt8          bytesPerSample;
    /* Set this to true if we allocated memory to store the samples. */
    spmUInt8          isAllocated;
}
DLS_Wave_t;

#define WAVE_FORMAT_PCM              0x0001
#define WAVE_FORMAT_ALAW             0x0006
#define WAVE_FORMAT_EXTENSIBLE       0xFFFE

typedef struct DLS_Articulation_s
{
    /* NOTE: if structure modified, update the initialization of sDefaultArticulations */
    DLS_ArticulationTokens token; /* Maps to a specific combination of source+control+destination. */
    spmSInt32           scale;
}
DLS_Articulation_t;

typedef struct DLS_ArticulationTracker_s
{
    DLS_Articulation_t *articulations;
    spmSInt           numArticulations;
}
DLS_ArticulationTracker_t;

typedef struct DLS_Region_s
{
    WaveSetRegion_t   waveSetRegion;
    /* Mobileer WaveTable info for wavetable oscillator.
     * Every region needs a wavetable so we keep a static copy here instead of allocating a pointer.
     * Note that we must set pointer in waveSetRegion.
     */
    WaveTable_t       waveTable;

    DLS_ArticulationTracker_t articulations;

//  DoubleList        voiceArticulations;
//  DoubleList        noteArticulations;
//  DoubleList        midiArticulations;

    DLS_Wave_t       *dlsWave;
    DLS_WaveSample_t *waveSample;
    
    /* Regions can have their own articulations so we need
     * a Preset for each region. */
    HybridVoice_Preset_t  hybridPreset;
    EnvelopeDAHDSR_Extension_t eg1Extension;
    EnvelopeDAHDSR_Extension_t eg2Extension;
    PitchOctave       modLFOCC1toPitch; /* Mod Wheel */
    PitchOctave       modLFOCPRtoPitch; /* Channel Pressure */
    PitchOctave       vibLFOCC1toPitch;
    PitchOctave       vibLFOCPRtoPitch;
    spmSInt32         reserved; /* for patching */

    /* Settings derived from Articulations. */
    /* Delay is in units of ticks where one tick is SS_FRAMES_PER_BUFFER frames. */
    spmUInt16         modLFOStartDelay; 
    spmUInt16         vibratoLFOStartDelay;

    /* from wlnk chunk */
    spmUInt32         linkOptions;
    spmUInt16         phaseGroup;
    spmUInt32         channel;
    spmUInt32         tableIndex;
    spmSInt           WLNKloaded;

    spmUInt16         options;
    spmUInt16         keyGroup;
    spmSInt           RGNHloaded;
}
DLS_Region_t;

typedef struct DLS_Instrument_s
{
    DLS_ArticulationTracker_t articulations;
    spmSInt               numRegions;
    DLS_Region_t         *regions;
    spmUInt16             bankID;
    spmUInt16             programID;
}
DLS_Instrument_t;

typedef struct DLS_Orchestra_s
{
    DoubleList                  waves;
    spmSInt                     numInstruments;
    DLS_Instrument_t           *instruments;
    spmSInt32                   numPoolTableEntries;
    DLS_Wave_t                **poolTable;
    spmUInt32                  *rawPoolTable;
    spmSInt32                   reserved; /* For patching. */
}
DLS_Orchestra_t;

/* Patch table for internal use. */
typedef struct DLSParser_FunctionTable_s
{
    int initialized;

    SPMIDI_Error (*create)( DLSParser **parserPtr,
        unsigned char *fileStart, spmSInt32 fileSize );

    void (*delete)( DLSParser *parser );

    SPMIDI_Error (*parse)( DLSParser *parser );

    SPMIDI_Error (*load)( DLSParser *parser, SPMIDI_Context *spmidiContext );

    int (*resolveRegion)( DLS_Wave_t **poolTable,
                                DLS_Region_t *region);

    void (*deleteInstruments)( DLS_Orchestra_t *orchestra );
    void (*deleteRegion)( DLS_Region_t *region);


}
DLSParser_FunctionTable_t;

/*************************************** DLS File Structures **********/
typedef struct _DLSVERSION
{
    spmUInt32    dwVersionMS;
    spmUInt32    dwVersionLS;
}
DLSVERSION;


typedef struct _CONNECTION
{
    spmUInt16   usSource;
    spmUInt16   usControl;
    spmUInt16   usDestination;
    spmUInt16   usTransform;
    spmUInt32     lScale;
}
CONNECTION;


/* Level 1 Articulation Data */

typedef struct _CONNECTIONLIST
{
    spmUInt32    cbSize;            /* size of the connection list structure */
    spmUInt32    cConnections;      /* count of connections in the list */
}
CONNECTIONLIST;



/*
   Generic type defines for regions and instruments
*/

typedef struct _RGNRANGE
{
    spmUInt16 usLow;
    spmUInt16 usHigh;
}
RGNRANGE;

#define F_INSTRUMENT_DRUMS      0x80000000

typedef struct _MIDILOCALE
{
    spmUInt32 ulBank;
    spmUInt32 ulInstrument;
}
MIDILOCALE;

/*
   Header structures found in an DLS file for collection, instruments, and
   regions.
*/

#define F_RGN_OPTION_SELFNONEXCLUSIVE  0x0001

typedef struct _RGNHEADER
{
    RGNRANGE RangeKey;            /* Key range */
    RGNRANGE RangeVelocity;       /* Velocity Range */
    spmUInt16   fusOptions;          /* Synthesis options for this range */
    spmUInt16   usKeyGroup;          /* Key grouping for non simultaneous play
                                                    0 = no group, 1 up is group 
                                                    for Level 1 only groups 1-15 are allowed */
}
RGNHEADER;

typedef struct _INSTHEADER
{
    spmUInt32      cRegions;          /* Count of regions in this instrument */
    MIDILOCALE Locale;            /* Intended MIDI locale of this instrument */
}
INSTHEADER;

typedef struct _DLSHEADER
{
    spmUInt32      cInstruments;      /* Count of instruments in the collection */
}
DLSHEADER;

/*
   definitions for the Wave link structure
*/

/*****  For level 1 only WAVELINK_CHANNEL_MONO is valid  ****
  ulChannel allows for up to 32 channels of audio with each bit position
  specifiying a channel of playback */

#define WAVELINK_CHANNEL_LEFT    0x0001l
 #define WAVELINK_CHANNEL_RIGHT   0x0002l

#define F_WAVELINK_PHASE_MASTER  0x0001

typedef struct _WAVELINK
{ /* any paths or links are stored right after struct */
    spmUInt16   fusOptions;     /* options flags for this wave */
    spmUInt16   usPhaseGroup;   /* Phase grouping for locking channels */
    spmUInt32    ulChannel;      /* channel placement */
    spmUInt32    ulTableIndex;   /* index into the wave pool table, 0 based */
}
WAVELINK;

#define POOL_CUE_NULL  0xffffffffl

typedef struct _POOLCUE
{
    //  spmUInt32    ulEntryIndex;   /* Index entry in the list */
    spmUInt32    ulOffset;       /* Offset to the entry in the list */
}
POOLCUE;

typedef struct _POOLTABLE
{
    spmUInt32    cbSize;            /* size of the pool table structure */
    spmUInt32    cCues;             /* count of cues in the list */
}
POOLTABLE;

/*
   Structures for the "wsmp" chunk
*/

#define F_WSMP_NO_TRUNCATION     0x0001l
 #define F_WSMP_NO_COMPRESSION    0x0002l


typedef struct _rwsmp
{
    spmUInt32   cbSize;
    spmUInt16  usUnityNote;         /* MIDI Unity Playback Note */
    spmUInt16   sFineTune;           /* Fine Tune in log tuning */
    spmUInt32    lAttenuation;        /* Overall Attenuation to be applied to data */
    spmUInt32   fulOptions;          /* Flag options */
    spmUInt32   cSampleLoops;        /* Count of Sample loops, 0 loops is one shot */
}
WSMPL;


/* This loop type is a normal forward playing loop which is continually
   played until the envelope reaches an off threshold in the release
   portion of the volume envelope */

#define WLOOP_TYPE_FORWARD   0

/* This loop type has a release portion after the loop. */
#define WLOOP_TYPE_RELEASE   1

typedef struct _rloop
{
    spmUInt32 cbSize;
    spmUInt32 ulType;              /* Loop Type */
    spmUInt32 ulStart;             /* Start of loop in samples */
    spmUInt32 ulLength;            /* Length of loop in samples */
}
WLOOP;

spmSInt32 DLSParser_ConvertArticulationData( const DLS_Articulation_t *articulation,
                                            int sampleRate );

/**
 * Get patch table.
 */
DLSParser_FunctionTable_t *DLSParser_GetFunctionTable( void );

/**
 * Get orchestra.
 */
DLS_Orchestra_t *DLSParser_GetOrchestra( DLSParser parser );

#endif /* _DLS_PARSER_INTERNAL_H */
