#ifndef _SPMIDI_USER_CONFIG_H
#define _SPMIDI_USER_CONFIG_H

/* $Id: spmidi_user_config.h,v 1.1 2006/05/20 01:50:57 philjmsl Exp $ */
/**
 *
 * @file spmidi_conig.h
 * @brief Configuration file to select compile time options.
 * @author Phil Burk, Copyright 2002 Mobileer PROPRIETARY and CONFIDENTIAL
 */

/** By defining SPMIDI_USER_CONFIG you can set your compile time configuration variables
 * in file called "spmidi_user_config.h" instead of passing them on the command line.
 */

#define SPMIDI_USE_PRINTF  (1)

#define SPMIDI_USE_INTERNAL_MEMHEAP  (1)

 /** Turn on or off ME3000, which adds DLS and Mobile XMF support. */
#define SPMIDI_ME3000   (1)

 /** Turn on or off ME2000, which adds Wavetable support. */
#define SPMIDI_ME2000   (1)

/** Absolute maximum number of voices allowed. Do not set higher than 64.
 * Internal structures will be allocated based on this value.
 * The actual maximum number of voices can be lowered dynamically by passing a value to SPMIDI_SetMaxVoices().
 */
#define SPMIDI_MAX_VOICES          (64)

/* Set to 0 for mobile devices for more compression, 1 for studio instruments for more dynamic range. */
#define SPMIDI_SQUARE_VELOCITY     (0)

/** Set SPMIDI_LEAVE_DLS_WAVES_IN_IMAGE to (1) to play directly from the DLS image. */
#define SPMIDI_LEAVE_DLS_WAVES_IN_IMAGE  (1)

/** Define this as zero to disable the dynamic range compressor. */
#define SPMIDI_USE_COMPRESSOR        (1)

/** Define this as one to enable dynamic memory allocation.
 * This is only needed if you want to play an arbitrary number of synth contexts simultaneously. */
#define SPMIDI_SUPPORT_MALLOC    (0)

#define SPMIDI_MAX_SAMPLE_RATE   (44100)


/** User can select mono or stereo synthesis when calling SPMIDI_ReadFrames().
 * One can save some memory by setting this to (1) at compile time but that
 * will prevent calling SPMIDI_ReadFrames() with samplesPerFrame greater than one.
 */
#define SPMIDI_MAX_SAMPLES_PER_FRAME   (2)

/** Define the maximum number of SPMIDI_Contexts that can be created.
 * This is only used when SPMIDI_SUPPORT_MALLOC is zero.
 * It determines the number of context data structures that
 * are statically allocated at compile time.
 */
#define SPMIDI_MAX_NUM_CONTEXTS     (1)
#define SPMIDI_MAX_NUM_PLAYERS      (1)

/**
 * Setting this to (0) will save about 2KB of RW memory. But if your system
 * requires dynamically relocatable code then you will have to
 * set SPMIDI_RELOCATABLE to (1).
 */
#define SPMIDI_RELOCATABLE          (0)

#define SPMIDI_SMOOTH_MIXER_GAIN    (1)

/** Define this as one to enable soft clipping.
 * This results in a gentle distortion when the volume is
 * raised above the normal limits.
 */
#define SPMIDI_USE_SOFTCLIP         (0)

#define SPMIDI_USE_REVERB           (0)

/** Use 3 instead of 4 for better MIDI timing at low sample rates and to use less RAM.
 * Use 4 to reduce CPU load.
 */
#define SPMIDI_FRAMES_PER_BLOCK_LOG2   (3)

#endif /* _SPMIDI_USER_CONFIG_H */

