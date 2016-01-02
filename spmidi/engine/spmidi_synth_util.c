/* $Id: spmidi_synth_util.c,v 1.13 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * SPMIDI Utilities for tuning.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/engine/spmidi_hybrid.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/include/spmidi_print.h"

#define USE_TABLE   (1)

#if (USE_TABLE == 0)
#include <math.h>
#endif

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif


#define TABLE_NUM_ENTRIES_LOG2     (8)
#define TABLE_NUM_ENTRIES          (1 << TABLE_NUM_ENTRIES_LOG2)
#define TABLE_NUM_ENTRIES_GUARDED  (TABLE_NUM_ENTRIES + 1)


/** Octave Tuning Table for converting pitch to phase increment. */
static const long sTuningTable[] =
    {
        0x309976DE, /* 0x00130000 */
        0x30BB3251, /* 0x00130100 */
        0x30DD052F, /* 0x00130200 */
        0x30FEEF86, /* 0x00130300 */
        0x3120F168, /* 0x00130400 */
        0x31430AE4, /* 0x00130500 */
        0x31653C0B, /* 0x00130600 */
        0x318784EE, /* 0x00130700 */
        0x31A9E59C, /* 0x00130800 */
        0x31CC5E27, /* 0x00130900 */
        0x31EEEE9F, /* 0x00130A00 */
        0x32119715, /* 0x00130B00 */
        0x32345799, /* 0x00130C00 */
        0x3257303C, /* 0x00130D00 */
        0x327A210E, /* 0x00130E00 */
        0x329D2A21, /* 0x00130F00 */
        0x32C04B86, /* 0x00131000 */
        0x32E3854C, /* 0x00131100 */
        0x3306D786, /* 0x00131200 */
        0x332A4244, /* 0x00131300 */
        0x334DC596, /* 0x00131400 */
        0x3371618F, /* 0x00131500 */
        0x33951640, /* 0x00131600 */
        0x33B8E3B8, /* 0x00131700 */
        0x33DCCA0B, /* 0x00131800 */
        0x3400C948, /* 0x00131900 */
        0x3424E181, /* 0x00131A00 */
        0x344912C8, /* 0x00131B00 */
        0x346D5D2E, /* 0x00131C00 */
        0x3491C0C4, /* 0x00131D00 */
        0x34B63D9C, /* 0x00131E00 */
        0x34DAD3C7, /* 0x00131F00 */
        0x34FF8357, /* 0x00132000 */
        0x35244C5E, /* 0x00132100 */
        0x35492EED, /* 0x00132200 */
        0x356E2B16, /* 0x00132300 */
        0x359340EB, /* 0x00132400 */
        0x35B8707D, /* 0x00132500 */
        0x35DDB9DF, /* 0x00132600 */
        0x36031D22, /* 0x00132700 */
        0x36289A58, /* 0x00132800 */
        0x364E3193, /* 0x00132900 */
        0x3673E2E6, /* 0x00132A00 */
        0x3699AE63, /* 0x00132B00 */
        0x36BF941A, /* 0x00132C00 */
        0x36E59420, /* 0x00132D00 */
        0x370BAE86, /* 0x00132E00 */
        0x3731E35E, /* 0x00132F00 */
        0x375832BB, /* 0x00133000 */
        0x377E9CB0, /* 0x00133100 */
        0x37A5214D, /* 0x00133200 */
        0x37CBC0A7, /* 0x00133300 */
        0x37F27AD0, /* 0x00133400 */
        0x38194FDA, /* 0x00133500 */
        0x38403FD8, /* 0x00133600 */
        0x38674ADC, /* 0x00133700 */
        0x388E70FA, /* 0x00133800 */
        0x38B5B244, /* 0x00133900 */
        0x38DD0ECD, /* 0x00133A00 */
        0x390486A8, /* 0x00133B00 */
        0x392C19E8, /* 0x00133C00 */
        0x3953C8A1, /* 0x00133D00 */
        0x397B92E4, /* 0x00133E00 */
        0x39A378C5, /* 0x00133F00 */
        0x39CB7A57, /* 0x00134000 */
        0x39F397AF, /* 0x00134100 */
        0x3A1BD0DD, /* 0x00134200 */
        0x3A4425F7, /* 0x00134300 */
        0x3A6C9710, /* 0x00134400 */
        0x3A95243A, /* 0x00134500 */
        0x3ABDCD8A, /* 0x00134600 */
        0x3AE69313, /* 0x00134700 */
        0x3B0F74E9, /* 0x00134800 */
        0x3B38731E, /* 0x00134900 */
        0x3B618DC8, /* 0x00134A00 */
        0x3B8AC4F9, /* 0x00134B00 */
        0x3BB418C6, /* 0x00134C00 */
        0x3BDD8941, /* 0x00134D00 */
        0x3C071681, /* 0x00134E00 */
        0x3C30C097, /* 0x00134F00 */
        0x3C5A8798, /* 0x00135000 */
        0x3C846B99, /* 0x00135100 */
        0x3CAE6CAD, /* 0x00135200 */
        0x3CD88AE9, /* 0x00135300 */
        0x3D02C660, /* 0x00135400 */
        0x3D2D1F28, /* 0x00135500 */
        0x3D579554, /* 0x00135600 */
        0x3D8228F8, /* 0x00135700 */
        0x3DACDA2B, /* 0x00135800 */
        0x3DD7A8FE, /* 0x00135900 */
        0x3E029589, /* 0x00135A00 */
        0x3E2D9FDE, /* 0x00135B00 */
        0x3E58C813, /* 0x00135C00 */
        0x3E840E3C, /* 0x00135D00 */
        0x3EAF726F, /* 0x00135E00 */
        0x3EDAF4C0, /* 0x00135F00 */
        0x3F069543, /* 0x00136000 */
        0x3F32540F, /* 0x00136100 */
        0x3F5E3137, /* 0x00136200 */
        0x3F8A2CD1, /* 0x00136300 */
        0x3FB646F3, /* 0x00136400 */
        0x3FE27FB1, /* 0x00136500 */
        0x400ED720, /* 0x00136600 */
        0x403B4D57, /* 0x00136700 */
        0x4067E269, /* 0x00136800 */
        0x4094966E, /* 0x00136900 */
        0x40C16979, /* 0x00136A00 */
        0x40EE5BA1, /* 0x00136B00 */
        0x411B6CFB, /* 0x00136C00 */
        0x41489D9E, /* 0x00136D00 */
        0x4175ED9D, /* 0x00136E00 */
        0x41A35D11, /* 0x00136F00 */
        0x41D0EC0D, /* 0x00137000 */
        0x41FE9AA9, /* 0x00137100 */
        0x422C68F9, /* 0x00137200 */
        0x425A5715, /* 0x00137300 */
        0x42886512, /* 0x00137400 */
        0x42B69306, /* 0x00137500 */
        0x42E4E108, /* 0x00137600 */
        0x43134F2D, /* 0x00137700 */
        0x4341DD8D, /* 0x00137800 */
        0x43708C3C, /* 0x00137900 */
        0x439F5B53, /* 0x00137A00 */
        0x43CE4AE7, /* 0x00137B00 */
        0x43FD5B0E, /* 0x00137C00 */
        0x442C8BE1, /* 0x00137D00 */
        0x445BDD74, /* 0x00137E00 */
        0x448B4FDF, /* 0x00137F00 */
        0x44BAE339, /* 0x00138000 */
        0x44EA9798, /* 0x00138100 */
        0x451A6D14, /* 0x00138200 */
        0x454A63C3, /* 0x00138300 */
        0x457A7BBD, /* 0x00138400 */
        0x45AAB518, /* 0x00138500 */
        0x45DB0FEC, /* 0x00138600 */
        0x460B8C50, /* 0x00138700 */
        0x463C2A5B, /* 0x00138800 */
        0x466CEA25, /* 0x00138900 */
        0x469DCBC5, /* 0x00138A00 */
        0x46CECF53, /* 0x00138B00 */
        0x46FFF4E5, /* 0x00138C00 */
        0x47313C94, /* 0x00138D00 */
        0x4762A678, /* 0x00138E00 */
        0x479432A8, /* 0x00138F00 */
        0x47C5E13B, /* 0x00139000 */
        0x47F7B24B, /* 0x00139100 */
        0x4829A5EE, /* 0x00139200 */
        0x485BBC3D, /* 0x00139300 */
        0x488DF54F, /* 0x00139400 */
        0x48C0513E, /* 0x00139500 */
        0x48F2D021, /* 0x00139600 */
        0x49257210, /* 0x00139700 */
        0x49583724, /* 0x00139800 */
        0x498B1F75, /* 0x00139900 */
        0x49BE2B1B, /* 0x00139A00 */
        0x49F15A30, /* 0x00139B00 */
        0x4A24ACCB, /* 0x00139C00 */
        0x4A582305, /* 0x00139D00 */
        0x4A8BBCF8, /* 0x00139E00 */
        0x4ABF7ABB, /* 0x00139F00 */
        0x4AF35C68, /* 0x0013A000 */
        0x4B276218, /* 0x0013A100 */
        0x4B5B8BE4, /* 0x0013A200 */
        0x4B8FD9E4, /* 0x0013A300 */
        0x4BC44C32, /* 0x0013A400 */
        0x4BF8E2E6, /* 0x0013A500 */
        0x4C2D9E1C, /* 0x0013A600 */
        0x4C627DEA, /* 0x0013A700 */
        0x4C97826C, /* 0x0013A800 */
        0x4CCCABBB, /* 0x0013A900 */
        0x4D01F9EF, /* 0x0013AA00 */
        0x4D376D23, /* 0x0013AB00 */
        0x4D6D0570, /* 0x0013AC00 */
        0x4DA2C2F1, /* 0x0013AD00 */
        0x4DD8A5BE, /* 0x0013AE00 */
        0x4E0EADF2, /* 0x0013AF00 */
        0x4E44DBA7, /* 0x0013B000 */
        0x4E7B2EF6, /* 0x0013B100 */
        0x4EB1A7FB, /* 0x0013B200 */
        0x4EE846CE, /* 0x0013B300 */
        0x4F1F0B8B, /* 0x0013B400 */
        0x4F55F64C, /* 0x0013B500 */
        0x4F8D072A, /* 0x0013B600 */
        0x4FC43E41, /* 0x0013B700 */
        0x4FFB9BAB, /* 0x0013B800 */
        0x50331F82, /* 0x0013B900 */
        0x506AC9E2, /* 0x0013BA00 */
        0x50A29AE4, /* 0x0013BB00 */
        0x50DA92A5, /* 0x0013BC00 */
        0x5112B13E, /* 0x0013BD00 */
        0x514AF6CB, /* 0x0013BE00 */
        0x51836366, /* 0x0013BF00 */
        0x51BBF72B, /* 0x0013C000 */
        0x51F4B236, /* 0x0013C100 */
        0x522D94A0, /* 0x0013C200 */
        0x52669E86, /* 0x0013C300 */
        0x529FD003, /* 0x0013C400 */
        0x52D92933, /* 0x0013C500 */
        0x5312AA31, /* 0x0013C600 */
        0x534C5318, /* 0x0013C700 */
        0x53862404, /* 0x0013C800 */
        0x53C01D12, /* 0x0013C900 */
        0x53FA3E5D, /* 0x0013CA00 */
        0x54348800, /* 0x0013CB00 */
        0x546EFA18, /* 0x0013CC00 */
        0x54A994C2, /* 0x0013CD00 */
        0x54E45818, /* 0x0013CE00 */
        0x551F4438, /* 0x0013CF00 */
        0x555A593E, /* 0x0013D000 */
        0x55959746, /* 0x0013D100 */
        0x55D0FE6C, /* 0x0013D200 */
        0x560C8ECD, /* 0x0013D300 */
        0x56484886, /* 0x0013D400 */
        0x56842BB4, /* 0x0013D500 */
        0x56C03872, /* 0x0013D600 */
        0x56FC6EDF, /* 0x0013D700 */
        0x5738CF16, /* 0x0013D800 */
        0x57755936, /* 0x0013D900 */
        0x57B20D5A, /* 0x0013DA00 */
        0x57EEEBA1, /* 0x0013DB00 */
        0x582BF427, /* 0x0013DC00 */
        0x5869270A, /* 0x0013DD00 */
        0x58A68467, /* 0x0013DE00 */
        0x58E40C5C, /* 0x0013DF00 */
        0x5921BF06, /* 0x0013E000 */
        0x595F9C83, /* 0x0013E100 */
        0x599DA4F0, /* 0x0013E200 */
        0x59DBD86C, /* 0x0013E300 */
        0x5A1A3714, /* 0x0013E400 */
        0x5A58C106, /* 0x0013E500 */
        0x5A977661, /* 0x0013E600 */
        0x5AD65742, /* 0x0013E700 */
        0x5B1563C8, /* 0x0013E800 */
        0x5B549C10, /* 0x0013E900 */
        0x5B94003A, /* 0x0013EA00 */
        0x5BD39064, /* 0x0013EB00 */
        0x5C134CAB, /* 0x0013EC00 */
        0x5C533530, /* 0x0013ED00 */
        0x5C934A11, /* 0x0013EE00 */
        0x5CD38B6B, /* 0x0013EF00 */
        0x5D13F960, /* 0x0013F000 */
        0x5D54940C, /* 0x0013F100 */
        0x5D955B8F, /* 0x0013F200 */
        0x5DD65009, /* 0x0013F300 */
        0x5E177199, /* 0x0013F400 */
        0x5E58C05D, /* 0x0013F500 */
        0x5E9A3C76, /* 0x0013F600 */
        0x5EDBE603, /* 0x0013F700 */
        0x5F1DBD22, /* 0x0013F800 */
        0x5F5FC1F5, /* 0x0013F900 */
        0x5FA1F49A, /* 0x0013FA00 */
        0x5FE45532, /* 0x0013FB00 */
        0x6026E3DC, /* 0x0013FC00 */
        0x6069A0B8, /* 0x0013FD00 */
        0x60AC8BE7, /* 0x0013FE00 */
        0x60EFA588, /* 0x0013FF00 */
        0x6132EDBC, /* 0x00140000 */
    };

/* Convert a fractional octave pitch to a phase increment.
 * Calculated assuming sample rate = SS_BASE_SAMPLE_RATE.
 * So octavePitch must be adjusted for sample rate before calling this function.
 */
FXP31 SPMUtil_OctaveToPhaseIncrement( FXP16 octavePitch )
{
#if USE_TABLE
    /* Calculate fraction of an octave. Mask off low bits for fraction. */
    long octaveFraction = octavePitch & ((1 << SS_PITCH_SHIFT) - 1);

    /* Which octave are we in? Mask off high bits by shifting. */
    long octaveTruncated = octavePitch >> SS_PITCH_SHIFT;

    /* Calculate index into table based on fractional octave. */
    long index = octaveFraction >> (SS_PITCH_SHIFT - TABLE_NUM_ENTRIES_LOG2);
    FXP31 lowPhaseIncr = sTuningTable[ index ];
    /* We use guard point so this is safe. */
    FXP31 highPhaseIncr = sTuningTable[ index + 1 ];

    /* Interpolate between succesive table values. */
    unsigned long tableDelta = (highPhaseIncr - lowPhaseIncr);
    unsigned long tableFraction = octaveFraction &
                                  ((1 << (SS_PITCH_SHIFT - TABLE_NUM_ENTRIES_LOG2)) - 1);
    FXP31 topPhaseIncr = lowPhaseIncr  +
                         ((tableFraction * tableDelta) >> (SS_PITCH_SHIFT - TABLE_NUM_ENTRIES_LOG2));

    /* Shift down into appropriate octave. */
    int shifter = (SS_TOP_OCTAVE - 1) - octaveTruncated;
    FXP31 phaseIncr;
    if( shifter > 0 )
    {
        phaseIncr = (topPhaseIncr >> shifter);
    }
    /* Just for debugging.
        else if( shifter < 0 )
        {
            phaseIncr = topPhaseIncr;
        }
    */
    else
    {
        phaseIncr = topPhaseIncr;
    }
    return phaseIncr;
#else

    double frequency = FREQUENCY_OCTAVE_ZERO *
                       pow( 2.0, (((double)octavePitch) / (1 << SS_PITCH_SHIFT)) );
#define DOUBLE_TO_FXP31( dbl )    ((FXP31)((dbl) * FXP31_MAX_VALUE))

    return DOUBLE_TO_FXP31( (frequency * 2) / SS_BASE_SAMPLE_RATE );
#endif
}

#if SPMIDI_USE_SOFTCLIP
/* Softclip mixer signal so that we can push amplitude without getting too harsh. */
FXP31 SS_MixerSoftClip( FXP31 input )
{
    FXP31 output;
    if( input > SS_MIXER_BUS_MAX )
    {
        output = SS_MIXER_BUS_MAX;
    }
    else if( input < SS_MIXER_BUS_MIN )
    {
        output = SS_MIXER_BUS_MIN;
    }
    else
    {
        /* y = (x - ((x^3)/3)), non-linear amplifier in fixed point. */
        FXP31 xShifted =  input >> (SS_MIXER_BUS_RESOLUTION - 16);
        FXP31 xSquaredShifted =  (xShifted * xShifted) >> (15);
        FXP31 xCubed = (xShifted * xSquaredShifted) >> (31 - SS_MIXER_BUS_RESOLUTION);
        FXP31 x3 = input + (input << 1);
        output = (x3 - xCubed) >> 1;
        /* Output can be out of range because of imprecision in calculation so do hard clip again. */
        if( output > SS_MIXER_BUS_MAX )
        {
            output = SS_MIXER_BUS_MAX;
        }
        else if( output < SS_MIXER_BUS_MIN )
        {
            output = SS_MIXER_BUS_MIN;
        }
    }
    return output;
}
#endif

/** Parse 32 bit integer assuming Big Endian byte order. */
unsigned char *SS_ParseLong( long *data, unsigned char *p)
{
    unsigned long pad = ((unsigned long)(*p++)) << 24;
    pad |= ((unsigned long)(*p++)) << 16;
    pad |= ((unsigned long)(*p++)) << 8;
    pad |= ((unsigned long)(*p++));
    *data = (long) pad;
    return p;
}

/** Parse 16 bit integer assuming Big Endian byte order. */
unsigned char *SS_ParseShort( short *data, unsigned char *p)
{
    unsigned short pad = (unsigned short)(((int)(*p++)) << 8);
    pad |= (unsigned short)(*p++);
    *data = (short) pad;
    return p;
}

#if 0
#include "math.h"
int main( void )
{
    int ioct;
    double midiZeroFreq, octaveZeroFreq;


    {
        /* Calculate pitch offset for wavetable pitch calculation. */
        double da = log((((double)NOMINAL_PHASE_INCREMENT) * SS_BASE_SAMPLE_RATE) / (FXP31_MAX_VALUE * 440.0 * 2)) / log(2.0);
        double db = (da + (MIDI_PITCH_CONCERT_A / 12.0) + MIDI_OCTAVE_OFFSET) * (1 << SS_PITCH_SHIFT);
        long op = (long) db;
        printf("op = 0x%x\n", op  );
    }


    /* Calculate frequency of FREQUENCY_OCTAVE_ZERO
     * A above middle C is 440 Hz.
     */
    midiZeroFreq = 440.0 * pow( 2.0, (- MIDI_PITCH_CONCERT_A / 12.0 ) );
    printf("midi zero frequency = %12.10f\n", midiZeroFreq );

    octaveZeroFreq = midiZeroFreq * pow( 2.0, - MIDI_OCTAVE_OFFSET );
    printf("octave zero frequency = %12.10f\n", octaveZeroFreq );

    {
        int octaveA, phaseincA;
        octaveA = SPMUtil_MIDIPitchToOctave( MIDI_PITCH_CONCERT_A << SS_PITCH_SHIFT );
        printf(" Octave A = 0x%08X\n", octaveA );

        phaseincA = SPMUtil_OctaveToPhaseIncrement( octaveA );
        printf(" Phase Inc A = 0x%08X\n", phaseincA );
    }


    /* Convert misc pitches to phaseIncrement for testing. */
    for( ioct = 0; ioct<10; ioct++ )
    {
        FXP16 octaveBase = SPMUtil_MIDIPitchToOctave( MIDI_PITCH_CONCERT_A << SS_PITCH_SHIFT );
        FXP16 octavePitch = octaveBase + (0x4567 * ioct);
        FXP31 phaseInc = SPMUtil_OctaveToPhaseIncrement( octavePitch );
        printf("pitch = 0x%08X, phinc = 0x%08X\n", octavePitch, phaseInc );
    }

    printf("static const sTuningTable[] = {\n" );
    /* Generate an octave table for tuning. */
    for( ioct = 0; ioct<TABLE_NUM_ENTRIES_GUARDED; ioct++ )
    {
        FXP16 octavePitch = (SS_TOP_OCTAVE << SS_PITCH_SHIFT) +
                            (ioct << (SS_PITCH_SHIFT - TABLE_NUM_ENTRIES_LOG2));

        FXP31 phaseInc = SPMUtil_OctaveToPhaseIncrement( octavePitch );
        printf("    0x%08X, /* 0x%08X */\n", phaseInc, octavePitch );
    }
    printf("};\n" );

}
#endif
