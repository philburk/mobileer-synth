cmake_minimum_required(VERSION 2.4)

project(spmidi)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/engine)
include_directories(${PROJECT_SOURCE_DIR}/export_1mb)
include_directories(${PROJECT_SOURCE_DIR}/jukebox)
include_directories(${PROJECT_SOURCE_DIR}/util)
include_directories(${PROJECT_SOURCE_DIR}/../ )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (c_options
    -DSPMIDI_USE_PRINTF
    -DSPMIDI_ME3000=0
    -Wall
#    -Werror
    -g
    -Ofast
    )

# Unused source
#    engine/mbis_parser.c   # for loading instrument files
    
set (me3000_sources
    engine/dls_articulations.c
    engine/dls_parser.c
    engine/onepole_filter.c
    engine/parse_riff.c
    engine/spmidi_dls.c
    engine/xmf_parser.c
    )

set (me2000_sources
    engine/adsr_envelope.c
    engine/compressor.c
    engine/dbl_list.c
    engine/instrument_mgr.c
    engine/memheap.c
    engine/memtools.c
    engine/midifile_parser.c
    engine/midifile_player.c
    engine/midi_names.c
    engine/oscillator.c
    engine/program_list.c
    engine/resource_mgr.c
    engine/reverb.c
    engine/song_player.c
    engine/spmidi.c
    engine/spmidi_fast.c
    engine/spmidi_host.c
    engine/spmidi_hybrid.c
    engine/spmidi_orchestra.c
    engine/spmidi_synth_util.c
    engine/spmidi_util.c
    engine/stack.c
    engine/streamio.c
    engine/streamio_ram.c
    engine/svfilter.c
    engine/wave_manager.c
    engine/wavetable.c
    )

add_library(me2000 ${me2000_sources})
target_compile_options(me2000 PRIVATE ${c_options})
    
set (example_sources
    util/spmidi_play.c
    util/spmidi_audio_file.c
    util/write_wav.c
    )

add_executable(play_scale
    ${example_sources}
    examples/play_scale.c
    )
target_compile_options(play_scale PRIVATE ${c_options})
target_link_libraries(play_scale me2000)

add_executable(play_midifile
    ${example_sources}
    util/spmidi_errortext.c # SPMUtil_GetErrorText(errcode)
    util/spmidi_load.c      # SPMUtil_LoadFileImage(filename)
    examples/play_midifile.c
    )
target_compile_options(play_midifile PRIVATE ${c_options})
target_link_libraries(play_midifile me2000)

