/* $Id: midifile_names_public.h,v 1.2 2006/06/13 02:22:17 philjmsl Exp $ */
#ifndef _MIDIFILE_NAMES_H
#define _MIDIFILE_NAMES_H
/**
 * Names of most of the MIDI files available for testing.
 * This is mostly used internally by Mobileer for testing.
 * These MIDI files are not part of the release package.
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

/** DATA_DIR is the folder where the ringtones or testmusic folder reside
 * relative to the midifile playing executable.
 * You will probably need to modify the definition for your system.
 */

#define DATA_DIR  "E:\\nomad\\MIDISynth\\data\\"

#define XMF_DIR        DATA_DIR"xmf\\"
#define RINGTONE_DIR   DATA_DIR"ringtones\\"
#define TESTMUSIC_DIR  DATA_DIR"midi\\"

// XMF Test Suite
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_BadFormat.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_DetachedNodeContentFound.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_IllegalFormat.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_MissingEndOfTrack.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_NotDLS.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_NotSMF.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_NotXMF.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_ParseErrorNoIns.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_ParseErrorNoPTBL.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_SizeError.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_UnsupportedSampleFormat.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_WrongType.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Bad_XMFParseError.mxmf"

//#define DEFAULT_FILENAME    XMF_DIR"qa/2RegionTest.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/AllTypesTest.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/AnyoneHome.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/BankMelDrumSwap.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/carumba.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/charge.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/charge_alaw.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/DrumKitFallThroughTest.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/DrumKitTest2.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/DrumPitch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/EnvelopeDelay.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/EnvelopeDAHDSR.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/EnvelopeHold.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Key_to_Pitch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/LFOTest1.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/ModEnvToPitch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/MOD_LFO_CC1_to_Pitch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/MOD_LFO_CPress_to_Pitch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/QuickAllTypes.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/TalkinReggae.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Tuning.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/VelocityMatch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Vib_LFO_CC1_to_Pitch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Vib_LFO_CPress_to_Pitch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/voiceLa.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/voiceLa8bit.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/Vol_Env_Sustain2.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"pac/PAC_hiphop.mxmf"

//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/subdiv.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/rel_tuning.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/string_4tet.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"test/string_4tet_bad.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/EightNotes.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/Belgique_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/LakeTahoe_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/RingBop_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/UpAndDown_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/EchoEcho_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/RockAndRing_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/RinginReggae_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/VibratingReggae_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/AnswerMeNow_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/AuldToonOfAyr.mid"  /* violin, acordian, snare */

//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/Strasbourg.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/Bach_PartitaDMinor.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/Bach_Sonata3EMajor.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurElise_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurEliseSteelDrum_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurEliseCelesta_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurEliseTrumpet_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurEliseOrgan_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurEliseSquare_rt.mid"
#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurryLisa_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/TimeBefore.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FunToy8.mid"

#endif  /* _MIDIFILE_NAMES_H */
