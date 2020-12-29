# mobileer-synth

General MIDI Synthesizer in portable C by Mobileer.

This synthesizer was used in the Palm Treo and other mobile devices for playing ringtones.

The library is given MIDI bytes and input and produces 16-bit PCM audio as output.
It can be easily connected to any audio system.
Optional output modules can be used to play in real-time using PortAudio or to output a WAV file.

The library has 3 levels:
* ME1000 - is for low memory devices and uses a virtual analog synthesizer instead of wavetables
* ME2000 - adds wavetables for piano, violin, drums and other instruments that are difficult to synthesize
* ME3000 - adds support for DLS and Scalable Polyphonic MIDI ringtone standards

# Dependencies and Requirements

The synthesizer uses 32-bit fixed-point arithmetic for its signal processing. It does not use floating point. 

It has minimal software dependencies and can even run without a memory allocator on embedded devices with no operating system.

It does not require any file I/O. The wavetable instruments are loaded at compile time from a header produced by a custom editor.

The ME1000 requires:
* Memory
  * Code 20-30 KB ROM
  * Instrument Library 14 KB ROM
  * Data 20-30 KB RAM
* 32-bit integer arithmetic, eg ARM-7, Blackfin

The ME2000 adds more ROM for the wavetables, depending on quality. A 1 MB instrument set is included.

* On an ARM946 generating audio at 44100 Hz
  * each wavetable voice requires 1.5 MIPS
  * each synthesized voice requires 2.4 MIPS

# Building

The current build system requires CMake and a C compiler.

This will generate a library for the ME2000 and a few example programs.

    cd spmidi
    mkdir build
    cd build
    cmake ..
    make
    
The libraries will be in the "spmidi/build/lib" folder
and the executables will be in the "spmidi/build/bin" folder.

This will play a diatonic scale and output the results to a WAV file.

    bin/play_scale
    
If you have ALSA installed then you can hear the output:

    aplay spmidi_output.wav
    
You can convert any standard MIDI file to WAV using "play_midifile".
We have included some ringtones that you can use for testing.

    bin/play_midifile ../../data/ringtones/phil/songs/FurryLisa_rt.mid
    
# Contributors

* Phil Burk - primary developer and sound designer
* Robert Marsanyi - DLS and other contributions to the ME3000
* Ryan Fransesconi - contributed to the instrument editor written in Java
