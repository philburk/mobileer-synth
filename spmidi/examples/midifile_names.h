#ifndef _MIDIFILE_NAMES_H
#define _MIDIFILE_NAMES_H
/**
 * $Id: midifile_names.h,v 1.93 2011/10/03 21:01:06 phil Exp $
 * Name of most of the MIDI files available for testing.
 * This is mostly used internally by Mobileer for testing.
 * These MIDI files are not part of the release package.
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

/** DATA_DIR is the folder where the ringtones or testmusic folder reside
 * relative to the midifile playing executable.
 * You will probably need to modify the definition for your system.
 */

#define PHIL_PREFIX   "E:\\nomad\\MIDISynth\\data\\"
#define ROBERT_PREFIX "C:\\business\\mobileer\\data\\"

#if defined(WIN32)
#define DATA_DIR   PHIL_PREFIX
#elif defined(MACOSX)
#define DATA_DIR  "../../../"
#else
#define DATA_DIR  PHIL_PREFIX
#endif

#define XMF_DIR        DATA_DIR"xmf\\"
#define RINGTONE_DIR   DATA_DIR"ringtones\\"
#define TESTMUSIC_DIR  DATA_DIR"midi\\"


//#define DEFAULT_FILENAME  ("E:\\nomad\\PHMSL\\ring_loop_1.mid")

#define DEFAULT_FILENAME    DATA_DIR"karaoke/Blackbird_2.mid"

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
//#define DEFAULT_FILENAME    XMF_DIR"charge_alaw/charge_alaw_3.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/DrumKitFallThroughTest.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/DrumKitTest2.mxmf"
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
//#define DEFAULT_FILENAME    XMF_DIR"qa/WaveFineTune.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"qa/VolumeTest.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"DrumBit31/DrumBit31.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"DrumBit31/DrumBit31SetByHand.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"pac/PAC_hiphop.mxmf"

// XMF Misc Tests
//#define DEFAULT_FILENAME    "\\dump.txt"
//#define DEFAULT_FILENAME    XMF_DIR"sine_voice_test_LFO.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"didkovsky/Giggles.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"DrumAndMelody.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"brandburg_sine_orchestra.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"brandburg_sine_orchestra_LFO.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"krivo_horo.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"sine_voice_test.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Tequila.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"test.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"views_of_distant_towns.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"wantya.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"song_krivo_spmidi.xmf"
//#define DEFAULT_FILENAME    XMF_DIR"song_Funtoy8_sp.mxmf"

// Jay's MXMF
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/EdSpeaks.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/ZipKit.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/MusBoxBubble.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/DrumLoop1.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/DrumLoop1_large.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/GentleRing.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/PhoneBell.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051014/XMF FILE OUTPUT/Pulse.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051017/OUTPUT/MXMF FILES/MusBoxPitchTestRATE.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051017/OUTPUT/MXMF FILES/MusBoxPitchTestROOT.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"JayCloidt/Calliope/Calliope.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"JayCloidt/Calliope/Calliope.mid"

//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/BachInvention4.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/BachInvention8.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/Boogie.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/Boogie7K.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/BrassATptch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/BrassEG2ptch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/BrassLFO1and2Ptch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/BrassLFO1SqPtch.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/Calliope_v2.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/DrumLoop3.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/Latiny_v3.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/MyCountryA.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/MyCountryB.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/MyCountryC.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/PleasantCelesta.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/Reggulated.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/Slidem.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/SonOfGoofus.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/TechnoSynth.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"Jay_051117/MXMF/TSaxVswitch.mxmf"

//#define DEFAULT_FILENAME    XMF_DIR"gibbs/TrekXmas/TrekXmas.mxmf"

//#define DEFAULT_FILENAME    XMF_DIR"patrick/PA050517/FJTest1.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"patrick/PA050523/Help22.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"patrick/PA050523/Sample_0-64.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"patrick/PA050523/64_D0G64_0-64.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"patrick/Help/Help64all8k.mxmf"

//#define DEFAULT_FILENAME    XMF_DIR"patrick/PA050517/a_2.mid"
//#define DEFAULT_FILENAME    XMF_DIR"patrick/v1_21.mxmf"

//#define DEFAULT_FILENAME    XMF_DIR"ReverbTest/Reverb_Piano.mxmf"

//#define DEFAULT_FILENAME    XMF_DIR"didkovsky/Baker.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"didkovsky/BombXMF.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"didkovsky/EightNotePolyChords.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"didkovsky/QtrQuitSexSep.mxmf"
//#define DEFAULT_FILENAME    XMF_DIR"didkovsky/19NotePolyChord.mxmf"

//#define DEFAULT_FILENAME    RINGTONE_DIR"No_Frontiers.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"cavalcade.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"speedy.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"wishfish.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"renegades.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"skank.mid"

// Phil Burk
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
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FurryLisa_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/TimeBefore.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/songs/FunToy8.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"test/TestSteal.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"test/Voices64.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"test/sudden_chords.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/BirdChirp_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/FastHorn.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/FastTrumpet.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudArpeggios.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudHello.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudHocket.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/OldPhoneShortLong_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/OldPhoneAscending.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/FourLowFourHigh_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/RingAndTing_rt.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/Tremolo1_Square.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/Tremolo1_SteelDrum.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/WarbleLow.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/WarbleLow_Sax.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/WarbleLow_Square.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/WarbleHigh_Glock.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/WarbleHigh_Guitar.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/TwoTrills.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/TwoTrillsMixed.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/TwoTrillsWiggle.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/Fiver.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/FiverEcho.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/MonoMania.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/RiseUp.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/DuoDing.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LongHighTrill.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LongLowTrill.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LongLowTrill_Trumpet.mid"

// Phil Tests
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudAccordian.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/SoftLoud.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudSquareMono.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudSaxMono.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudSaxQuad.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudSaxOctet.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudTrumpetMono.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudTrumpetQuad.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudTrumpetOctet.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudMonoVarious.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/LoudSquareTestOctet.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/AbuseCompressor.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"phil/rings/TestArpeggio.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"mobileer/FakeOrchestraHit.mid"

// Todd Telford
//#define DEFAULT_FILENAME    RINGTONE_DIR"todd/DarkNight.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"todd/Lantern.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"todd/Expandingaling.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"todd/Ducklings.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"todd/Dingaling.mid"

// Nick Didkovsky
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/rt_beta14ok.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/rt_jonny.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/baker.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/ironwood.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/rt_bomb.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/aaaa.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/plague.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/rt_takeyourears.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"nick/rt_bomb2_pb.mid"

// Larry Polansky
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/fornick.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/gamelan_030309_jazzed.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/nld_theme_plb.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/simpledream.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/chromaticon.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/henrycalling.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/BonyparteCall.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/BonyparteCall_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/ringrang.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/shortringrang.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/microringrang_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/NewHampshireGamelan.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"larry/NewHampshireGamelan_rt.mid"

// David Kendall
//#define DEFAULT_FILENAME    RINGTONE_DIR"kendall/StaccatoBoogie.mid"

// Bill Burk
//#define DEFAULT_FILENAME    RINGTONE_DIR"bill/lizard.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"bill/reptile_stare_2.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"bill/Doppleganger_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"bill/dry_on_a_rainy_day_2.mid"

// Jeanne Parson
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/50s-90sRing.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/AvgWhtGrl.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/BachFInvRingTune.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/MozartDivert.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/UrbanOre.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/UrbanOre_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/RokDriver.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/KenshasaRing.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/6_8WorldBlues.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"jeanne/ChaCha01Ring.mid"

// HandSpring
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/_Treo.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Treo_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Jazz Combo.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/JazzCombo_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Chimes.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Fly By.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Blip.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Escalate.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Euro.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Professional.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Powerful.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Beep Beep 1.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Turca06.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Arabesque_rt.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/waiting_on_you_5_10 m0.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/HotSync_marimba.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/HotSync_musicbox.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/HotSync_celesta.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/alarm_hilo1.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/alarm_hilo2.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/alarm_beep.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/alarm_click.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/connect_ep1.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/connect_ep2.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Message2.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/RadioOn.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/MusicBox.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Splish2.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Beethoven5.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/ToccataFuga.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/oblada.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/oblada_clarinet.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/oblada_fixed.mid"

//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/TreoFantasy.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/TreoAscending.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/TreoTwoBar.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/TreoClassic_SquareLead.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/TreoClassic_SquareTest.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/TreoClassic_Trumpet.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/TreoClassic_FrenchHorn.mid"

// Haitani
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/vivaldistring.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/trillecho_brass.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/technotreo4.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/SlowBlues2.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/rhythm_n_blues4.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/Escalate.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/echochord.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/bluescale.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"handspring/12bar5.mid"

// Subbu
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/01_TSUNAMI_B.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/02_TOKEI.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/03_ELECT.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/05_KIMIOSAG_A.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/05_KIMIOSAG_B.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/06_SIROIKOI.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/07_ITUMONAN.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/08_KAWANONA.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/10_SALUTDAM.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/13_HIDAMARI.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/15_HEYJUDE.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Call09ari90.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Freedom.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Jin01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Jin01_plb.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/TestExportVolume.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Jin09+3.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Jin_02.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Jin_08+2.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/Jin_13.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/02_sakura.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/04_cosmos.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/03_natsuwodaki.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/ac0402_05.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/ac0403_01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/ac0403_03.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"subbu/sakuranbo.mid"

// Frankie
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/sb0.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/sbsp2.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/mms_sound.mid" /* This caused ticksPerBeat overflow! */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/SeperateWays.mid" /* Has extra bytes after the EOT! */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/Message1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/Message3.mid" /* EstimateMaxAmplitude too high! */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/Calendar1.mid" /* EstimateMaxAmplitude too low! */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/Enterprise.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/figaro.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/Rock_Lobster.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/2sexy.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/b29.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/blackoy.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/tequila.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/gfc7ocar.mid"  /* Used to pop before voice stifling code added. */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"frankie/rob_noch11.mid"  /* Missing instrument on ch 11. */

// Valence
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"valence/gm000.mid"

// Jamie
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"jamie/3DWorldX.mid"

// Ning
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"ning/imelody05.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"ning/Jay_Melo1.mid"

// Flurry
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"flurry/Huang5.mid"

// LeapFrog
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"DecayTest3.mid"
//#define DEFAULT_FILENAME      "D:/mobileer_work/LIT_InstSet_v225_061307/LIT_InstSet/testMidi/Neutr_3_noDrums.mid"

// Classical
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"sdrum_xylo_mar.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"sdrum_debug3.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bach_fugadm.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bach_brand5.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bach_tocatta.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bachdblconcerto.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"debussy_rapsodie.mid" /* Piano, clarinet */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tocatta_short.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bach_wtc1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"chpn-p11.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"chpn-p19.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"chpn-p21.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"brahms_n2aminor.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"mussorgsky_pix_3.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"paganini_capric02.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GPacchioni_OP28_04.mid" /* Clarinet, bassoon, oboe */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"fur_elise.mid" /* Piano */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tchai_5.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"aguado1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Paco_de_Lucia_Guajiras_Lucia_Steel.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Lucia_Rio_Ancho.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"pineapple_rag.mid"

//#define DEFAULT_FILENAME    TESTMUSIC_DIR"classical/bach_bwv1004.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"classical/bach_bwv1005.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"classical/bach_bwv1016.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"classical/liz_rhap10.mid"

//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/CoarseFineTune.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/test_cc7.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/test_cc7_seq.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/test_vel.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/qa_bend_1.mid"

// Rock and Folk
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"jig_long.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"jigs_up.mid"   /* Accordion */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"layla.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"layla_short_ma.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bite_dust.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"rhapsody.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"heartofgold.mid" /* Acoustic Grand, Fretless Bass, Harmonica, Bari Sax */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"mdkatakane.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"odin-1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"dance_mix-1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"santana_blackmagicwoman.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"stairway.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"IShotTheSheriff.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Doors_BreakOnThrough.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"zztop_igotamessage.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bowie_suffragettecity.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"bowie_suffragettecitySP.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"police_everylittlethingshedoes.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"REM_Religion.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"RoyOrbisson_PrettyWoman.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"PeterGabriel_SledgeHammer.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"test_smpte_division.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"wizballguitarsolo.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"salsa.mid"  /* Has SP-MIDI MIPS message. */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"kc_talkwind1.mid"

// Americana
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"americana/Grover_TheCavalier.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"americana/BuyABroom.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"americana/OldRosinTheBeau.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"americana/40soar.mid"

/* Jazz */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"TakeFive.mid"  /* Piano, sax */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"BlueMonkSaxQuartet.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"YardBirdSuite.mid" /* Nice! - piano, saxes, trumpets */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"YardBirdSuiteShort.mid" /* Nice! - piano, saxes, trumpets */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"BlackNarcissus.mid" /* Piano, vibe */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"BlackNarcissusSP.mid" /* Piano, vibe */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"RoundMidnight.mid" /* Bass, E Piano, Trombone, Ride */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MyFavoriteThings.mid" /* yucky tuning! */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Impressions.mid" /* Nice! */

/* Sharp */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"sharp/life_in_the_fast_lane.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"sharp/victim_of_love.mid"

/* Trycho test files */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi2.mid" /* Fiddle */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi3.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi4.mid" /* Trumpet, clav, dist guitar */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi5.mid" /* Oboe, piano */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi10.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi20.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi30.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi40.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"tmi50.mid"

//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kwl_jz01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kwl_sy01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kwl_sy02.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kwl_sy03.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_clp01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_clp02.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_clp03.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_clp04.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_clp05.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_clp06.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_fnk01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_fnk02.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_fnk03.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_jaz01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_mel01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_mel02.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_pop01.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/WebTunes/kw_syn01.mid"

//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/CATALINA.MID"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/chrome.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/club_xxx.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/cybrbeat.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/darklady.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/demon16t.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/dreaming.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/drkshore.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/echoes1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/eternium.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/fnkngrvn.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/gatedoom.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/home.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/housrokr.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/island2.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/justblue.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/kinshasa.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/marseill.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/nitefunk.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/olboogie.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/ovrdrive.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/pacific.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/sherwood.mid"   /* Guitar, piano, celesta */
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/skyline.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/smoke.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/sngchild.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/sunset1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/televisn.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/travels.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/TROPICO.MID"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/ultimate.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"Walthius/undrmoon.mid"

// QA
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/play_8/play_programs_113_120.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/play_1/play_ins_010.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/TestSeekNoteOff.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/TestChannelModulation.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"qa/GapInMiddle.mid"

//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/000AcPiano.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/001BrtAcPiano.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/002ElGrandPiano.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/006Harpsichord.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/007Clavichord.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/008Celesta.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/009Glockenspiel.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/010MusicBox.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/011Vibraphone.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/014TubularBells.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/015Dulcimer.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/018RockOrgan.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/019ChurchOrgan.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/021Accordian.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/022Harmonica.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/024AcGuitar.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/025SteelAcGuitar.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/027ElGuitar.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/029OdGuitar.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/030DistGuitar.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/032AcBass.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/033ElBassFinger.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/034ElBassPick.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/035FretlessBass.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/036SlapBass1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/037SlapBass2.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/038SynthBass1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/039SynthBass2.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/040Violin.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/046Harp.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/047Timpani.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/048StringEns1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/056Trumpet.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/065AltoSax.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/064SopSax.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/068Oboe.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/069EngHorn.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/071Clarinet.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/080Lead1Square.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/082Lead2Saw.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/083Lead4Chiff.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/089Pad2Warm.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/090Pad3Polysynth.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/095Pad8Sweep.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/096FX2Rain.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/108Kalimba.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/109Bagpipe.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/114SteelDrums.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"GMSuite/117MelodicTom.mid"

//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/35_AcousticBassDrum.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/36_BassDrum1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/37_SideStick.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/38_AcousticSnare.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/42_ClosedHiHat.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/46_OpenHiHat.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/49_CrashCymbal1.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/54_Tambourine.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/63_OpenHiConga.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/64_LowConga.mid"
//#define DEFAULT_FILENAME    TESTMUSIC_DIR"MIDI_DrumKit/EXC1_HiHat.mid"

/* Broken MIDI files to test error recovery. */
//#define DEFAULT_FILENAME    RINGTONE_DIR"broken/rt_jonny_noEOT.mid"
//#define DEFAULT_FILENAME    RINGTONE_DIR"broken/rt_beta14ok_badmeta.mid"

/* Not a MIDI file! Test bogus file error recovery. */
//#define DEFAULT_FILENAME    "README.txt"

#endif  /* _MIDIFILE_NAMES_H */
