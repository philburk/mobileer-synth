/* $Id: qa_common.h,v 1.12 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_common.h
 * @brief Common tools for Quality Assurance.
 *
 * @author Robert Marsanyi, Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/dls_parser.h"

#define DEADBEEF              (0xDEADBEEF)

/* SPMIDI_DIR is defined in "midifile_names.h" */
#define QADATA_DIR            SPMIDI_DIR"qa/data/"
#define DETECT_PITCH_FRAMES   (256 * 64)
#define QA_SAMPLE_RATE        (22050)
#define DETECT_AMP_FRAMES     (QA_SAMPLE_RATE / 250 * 4)

typedef struct MemoryCheck
{
    int numAllocs;
    int numBytes;
}
MemoryCheck_t;

/**
 * If c is false, generate a message and set the global success flag to false.
 * @param c expression returning true (non-zero) or false (zero)
 * @param testname name of test being run, printed with message
 * @param msg message to print if expression returns false
 * @param failed set to non-zero if test fails
 */
void TestAssert( int c, const char *testname, const char *msg, int *failed );

/**
 * Snapshot current memory allocations for comparison with after a test.
 * @param memCheck pointer to structure for snapshotting memory status
 * @param success set if memCheck cannot be updated for any reason
 */
void InitMemoryCheck( MemoryCheck_t *memCheck, int *success );

/**
 * Check that current allocations match snapshot.
 * @param memCheck pointer to structure previously filled with InitMemoryCheck()
 * @param testname name of the test being run, printed with message on failure
 * @param success set if snapshot doesn't match current values
 */
void TestMemory( MemoryCheck_t *memCheck, const char *testname, int *success );

/**
 * Generate a random number 0..max.
 * @param max random number is less than this value
 * @return random value 0..max
 */
int ChooseRandom( int max );

/**
 * Measure frequency of a note by counting zero-crossings
 * @param spmidiContext context of MIDI engine
 * @param note note to turn on and measure
 * @return frequency of sounding note
 */
double MeasureFrequency( SPMIDI_Context *spmidiContext, int note );

/**
 * Measure amplitude of output of MIDI engine
 */
int MeasureAmplitude( SPMIDI_Context *spmidiContext, short *samples );
