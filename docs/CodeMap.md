[HOME](docs/README.md)

# Map of Code

## Folders

* engine = source for the library, synth, players, DLS
* examples
* export_1mb = header files that contain a General MIDI Instrument library
* include = exported headers
* jukebox = high level player for queuing songs
* qa = unit tests
* tests = executables for debugging and manual tests
* tools = file conversion, etc.
* util = general purpose utilities

## Synthesizer

* engine/spmidi_voice.h = defines basic voice architecture for an SPMIDI voice
* engine/oscillator.c = oscillator with multiple waveforms and reduced aliasing
* engine/svfilter.c = State Variable Filter
* engine/wavetable.c = wavetable oscillator with bilinear interpolation

## Standard MIDI File Support

* engine/midifile_parser.c = analyze the file and construct a player context
* engine/midifile_player.c = play the tracks of the MIDI file in time order

## DLS Parser

* engine/dls_articulations.c = convert DLS articulation value to values specific to ME3000
* engine/dls_parser.c = main file parser
* engine/parse_riff.c = parser for WAV and DLS files
* engine/spmidi_dls.c = DLS loader for SPMIDI orchestras

## XMF File Support

* engine/song_player.c = parse XMF file, load instruments and play it
* engine/xmf_parser.c = XMF file parser

