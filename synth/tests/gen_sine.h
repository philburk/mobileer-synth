#ifndef _GEN_SINE_H
#define _GEN_SINE_H

/* $Id$ */
/**
 *
 * @file gen_sine.h
 * @brief Generate a sine wave for testing audio device.
 * @author Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Fill a buffer with a mono or stereo sinewave signal.
 * This can be used for testing an audio device driver.
 *
 * @param samples pointer to buffer to receive sine wave samples
 * @param framesPerBuffer number of mono sample or stereo pairs
 * @param samplesPerFrame 1 for mono, 2 for stereo
 * @return zero or negative error code
 */
int FillBufferWithSines( short *samples, int framesPerBuffer, int samplesPerFrame );


#ifdef __cplusplus
}
#endif

#endif /* _GEN_SINE_H */

