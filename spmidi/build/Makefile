# Makefile for Mobileer MIDI Engine Examples
# (C) 2006 Mobileer, Inc.

# Use shared file to define object lists.
include ../build/MakefileVariables

CFLAGS = \
	-DSPMIDI_USE_PRINTF \
	-DSPMIDI_ME3000=1 \
	-Wall \
	$(ME2000_INCLUDEPATH) \
	-I../examples \
	-I../qa
	
DEMO_ME2000_SONG = ../data/FurryLisa_rt.mid
DEMO_ME3000_SONG = ../data/ZipKit.mxmf
		 
# Use simple players from examples folder.
DEMO_ME2000_EXE = play_midifile
DEMO_ME3000_EXE = play_song

# Link with utility files that load the MIDI data and output the audio as a WAV file.
DEMO_OBJS = \
	 spmidi_errortext.o \
	 spmidi_audio_file.o \
	 spmidi_play.o \
	 spmidi_load.o \
	 write_wav.o
	 
all: me2000.wav me3000.wav
	@echo ""
	@echo "ME2000 Output written to me2000.wav file."
	@echo "ME3000 Output written to me3000.wav file."

$(DEMO_ME2000_EXE): $(DEMO_OBJS) $(SPMIDI_OBJS)

$(DEMO_ME3000_EXE): $(DEMO_OBJS) $(SPMIDI_OBJS)

libme2000.a:  $(DEMO_OBJS) $(SPMIDI_OBJS)
	@echo 'Building target: $@'
	ar -r "libme2000.a" $(DEMO_OBJS) $(SPMIDI_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Run the demo that converts a MIDI file to a WAV file.
me2000.wav: $(DEMO_ME2000_EXE)
	./$(DEMO_ME2000_EXE) $(DEMO_ME2000_SONG)
	mv spmidi_output.wav me2000.wav
	@echo "ME2000 Output written to me2000.wav file."

# Run the demo that converts a MIDI or MXMF file to a WAV file.
me3000.wav: $(DEMO_ME3000_EXE)
	./$(DEMO_ME3000_EXE) $(DEMO_ME3000_SONG)
	mv spmidi_output.wav me3000.wav
	@echo "ME3000 Output written to me3000.wav file."
	
clean:
	@rm *.o
	@rm $(DEMO_ME2000_EXE)
	@rm $(DEMO_ME3000_EXE)
	@rm *.wav
	
	

