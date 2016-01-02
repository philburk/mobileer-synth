# Makefile for Mobileer MIDI Engine
# (C) 2006 Mobileer, Inc.

include ../build/MakefileVariables

CFLAGS = \
    -DSPMIDI_USER_CONFIG \
    -Wall \
    $(ME2000_INCLUDEPATH) \
    -I../qa \
    -I../examples
         
QA_OBJS = \
     spmidi_errortext.o \
     spmidi_audio_file.o \
     spmidi_play.o \
     spmidi_load.o \
     qa_common.o \
     qa_tools.o \
     QuickAllTypes.mxmf.o \
     write_wav.o
     
QA_TESTS = \
    qa_64v \
    qa_alltypes \
    qa_drum_excl \
    qa_memheap \
    qa_note_on_off \
    qa_pitch \
    qa_pitch_excl \
    qa_smf_errors \
    qa_spmidi \
    qa_spmidi_errors \
    qa_vary_max \
    qa_xmf_corrupt

all: $(QA_TESTS) qa_checksum_song

$(QA_TESTS): $(QA_OBJS) $(SPMIDI_OBJS)

qa_checksum_song: $(QA_OBJS) $(SPMIDI_OBJS)

list_tests:
    @echo $(QA_TESTS)
    
clean:
    rm *.o
    

