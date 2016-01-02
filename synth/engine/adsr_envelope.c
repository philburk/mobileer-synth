/* $Id: adsr_envelope.c,v 1.22 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * ADSR Envelope.
 * ADSR = Attack/Decay/Sustain/Release
 * An envelope is a contour that can be used to control
 * amplitude, filter cutoff or other synthesis parameters.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/engine/fxpmath.h"
#include "spmidi/engine/adsr_envelope.h"
#include "spmidi/engine/spmidi_synth_util.h"
#include "spmidi/engine/spmidi_synth.h"
#include "spmidi/include/spmidi_print.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

/**
 * Cubic curve to map internal envelope value to output value.<br>
 * output = env**3<br>
 * This is less computation than an exponential curve
 * and gives a natural dynamic range compression compared to an exponential curve.
 */
#define CONVERT_ENV_TO_OUTPUT(env)  ( FXP31_MULT( FXP31_MULT( env, env ), env ) )

/* Output will wrap around to negative when it hits the top. */
#define OVER_THE_TOP  (adsr->current < 0)

/********************************************************************/
/**
 * Generate ADSR contour.
 * Return 0, or 1 if finished contour on this iteration.
 * Envelope is implemented as a state machine, one state for each
 * segment of the envelope.
 */
int ADSR_Next( const EnvelopeADSR_Preset_t *preset, const EnvelopeADSR_Info_t *info, EnvelopeADSR_t *adsr,
              FXP31 *outputBuffer, EnvelopeDAHDSR_Extension_t *extension )
{
    int is = 0;
    int autoStop = 0;
    int flags = preset->flags;
    int framesLeft = SS_FRAMES_PER_BLOCK;

    while( framesLeft > 0 )
    {
        switch( adsr->stage )
        {
            /* Not running */
        case ADSR_STAGE_IDLE:
            while( framesLeft-- > 0 )
            {
                outputBuffer[is++] = 0;
            }
            break;

        case ADSR_STAGE_DELLLAY:
            if( extension == NULL )
            {
                adsr->stage = ADSR_STAGE_ATTACK;
            }
            else
            {
                while( framesLeft-- > 0 )
                {
                    outputBuffer[is++] = 0;

                    /* For delay, we increment the current counter so we can start at zero. */
                    adsr->current += extension->delayIncrement;

                    /* Delay segment ends when we hit the top. */
                    if( OVER_THE_TOP )
                    {
                        adsr->current = 0; /* Prepare to attack. */
                        adsr->stage = ADSR_STAGE_ATTACK;
                        break;
                    }
                }
            }
            break;

            /* Initial rise. */
        case ADSR_STAGE_ATTACK:
            while( framesLeft-- > 0 )
            {
                adsr->current += info->attackIncrement;

                /* Attack segment ends when we hit the top. */
                if( OVER_THE_TOP )
                {
                    adsr->current = FXP31_MAX_VALUE;
                    outputBuffer[is++] = FXP31_MAX_VALUE;
                    adsr->stage = ADSR_STAGE_HOLD;
                    break;
                }
                else
                {
                    outputBuffer[is++] = CONVERT_ENV_TO_OUTPUT( adsr->current );
                }
            }
            break;

        case ADSR_STAGE_HOLD:
            if( extension == NULL )
            {
                if( (preset->flags & ADSR_FLAG_WAIT_DECAY) == 0 )
                {
                    adsr->stage = ADSR_STAGE_DECAY;
                }
                else
                {
                    while( framesLeft-- > 0 )
                    {
                        outputBuffer[is++] = FXP31_MAX_VALUE;
                    }
                }
            }
            else
            {
                while( framesLeft-- > 0 )
                {
                    outputBuffer[is++] = FXP31_MAX_VALUE;

                    /* For hold, we decrement the current counter so we can start at top just like decay. */
                    adsr->current -= extension->holdIncrement;
                    if( adsr->current <= 0 )
                    {
                        adsr->current = FXP31_MAX_VALUE;
                        adsr->stage = ADSR_STAGE_DECAY;
                        break;
                    }
                }
            }
            break;

        /* After hold, the envelope will decay to the sustain value. */
        case ADSR_STAGE_DECAY:
            {
                /* Convert from 10 bit preset format to 31 bit resolution. */
                FXP31 shiftedSustainLevel = preset->sustainLevel << (31 - 10) ;

                while( framesLeft-- > 0 )
                {
                    adsr->current -= adsr->scaledDecayIncrement;
                    /* Decay segment end when we hit the sustain level. */
                    if( adsr->current <= shiftedSustainLevel )
                    {
                        adsr->current = shiftedSustainLevel;
                        outputBuffer[is++] = CONVERT_ENV_TO_OUTPUT( adsr->current );

                        if( (flags & ADSR_FLAG_LOOP_SUSTAIN) != 0 )
                        {
                            /* Loop back to beginning. */
                            adsr->stage = ADSR_STAGE_ATTACK;
                        }
                        else if( adsr->current == 0 )
                        {
                            /* Skip release cuz we are already at zero. */
                            autoStop = 1;
                            adsr->stage = ADSR_STAGE_IDLE;
                        }
                        else if( (flags & ADSR_FLAG_NO_WAIT) != 0 )
                        {
                            /* Don't wait for noteOff, just release now. */
                            adsr->stage = ADSR_STAGE_RELEASE;
                        }
                        else
                        {
                            /* Hold steady and wait for noteOff */
                            adsr->stage = ADSR_STAGE_SUSTAIN;
                        }
                        break;
                    }
                    else
                    {
                        outputBuffer[is++] = CONVERT_ENV_TO_OUTPUT( adsr->current );
                    }
                }
            }
            break;

            /* Hold at this level while note is "on". */
        case ADSR_STAGE_SUSTAIN:
            {
                FXP31 sustainLevel = CONVERT_ENV_TO_OUTPUT( adsr->current );
                while( framesLeft-- > 0 )
                {
                    outputBuffer[is++] = sustainLevel;
                }
            }
            break;

            /* Generally triggered by noteOff */
        case ADSR_STAGE_RELEASE:
            while( framesLeft-- > 0 )
            {
                adsr->current -= adsr->scaledReleaseIncrement;
                outputBuffer[is] = CONVERT_ENV_TO_OUTPUT( adsr->current );
                if( adsr->current <= 0 )
                {
                    adsr->current = 0;
                    outputBuffer[is++] = 0;
                    if( (flags & ADSR_FLAG_LOOP_RELEASE) != 0 )
                    {
                        adsr->stage = ADSR_STAGE_ATTACK;
                    }
                    else
                    {
                        autoStop = 1;
                        adsr->stage = ADSR_STAGE_IDLE;
                    }
                    break;
                }
                is++;
            }
            break;

            /* Used when shutting down a voice ASAP. */
        case ADSR_STAGE_STIFLE:
            while( framesLeft-- > 0 )
            {
                /* Subtract an amount that is adjusted for the block size. */
                adsr->current -= (1 << (32 - (9-SS_BLOCKS_PER_BUFFER_LOG2)));
                outputBuffer[is] = CONVERT_ENV_TO_OUTPUT( adsr->current );
                if( adsr->current <= 0 )
                {
                    adsr->current = 0;
                    outputBuffer[is++] = 0;
                    autoStop = 1;
                    adsr->stage = ADSR_STAGE_IDLE;
                    break;
                }
                is++;
            }
            break;
        default:
            framesLeft = 0;
            break;
        }
    }

    return autoStop;
}

/********************************************************************/
/********************************************************************/
/**
 * Start ADSR contour.
 * Scale decay/release based on pitch.
 */
void ADSR_Start( EnvelopeADSR_t *adsr, const EnvelopeADSR_Preset_t *preset, int pitch, int sampleRate )
{
    /* t = (T * (128 - S*((16384 - (127-P)*(127-P))/16384)))/128 */
    int p1 = (127*127) - ((127-pitch)*(127-pitch)); /* Ranges from 0 to 16129 */
    int p2 = ((128*16129) - (preset->pitchScalar * p1)) >> 7;

    int scaledTime = (preset->decayTime * p2) >> 14;
    if( scaledTime < 1 )
        scaledTime = 1;
    adsr->scaledDecayIncrement = ADSR_MSecToIncrement(scaledTime, sampleRate);

    scaledTime = (preset->releaseTime * p2) >> 14;
    if( scaledTime < 1 )
        scaledTime = 1;
    adsr->scaledReleaseIncrement = ADSR_MSecToIncrement(scaledTime, sampleRate);

    if( (preset->flags & ADSR_FLAG_WAIT_DECAY) == 0 )
    {
        adsr->stage = ADSR_STAGE_DELLLAY;
        adsr->current = 0; /* This is a source of clicks. */
    }
    else
    {
        adsr->stage = ADSR_STAGE_HOLD;
        adsr->current = FXP31_MAX_VALUE; /* This is a source of clicks. */
    }
}

/********************************************************************/
void ADSR_TriggerDecay( EnvelopeADSR_t *adsr )
{
    if( adsr->stage == ADSR_STAGE_HOLD )
    {
        adsr->stage = ADSR_STAGE_DECAY;
    }
}

/********************************************************************/
void ADSR_Release( EnvelopeADSR_t *adsr )
{
    /* For these stages, the current value does not match the output level.
     * So we have to force it to a corresponding value.
     */
    if( adsr->stage == ADSR_STAGE_DELLLAY )
    {
        adsr->current = 0;
    }
    else if( adsr->stage == ADSR_STAGE_HOLD )
    {
        adsr->current = FXP31_MAX_VALUE;
    }
    adsr->stage = ADSR_STAGE_RELEASE;
}


/********************************************************************/
int ADSR_MSecToIncrement( int msec, int envelopeSampleRate )
{
    int increment;
    int numerator;
    if( msec <= 0 ) return FXP31_MAX_VALUE;
/* We have to calculate 
 *     (1000 * FXP31_MAX_VALUE) / (SR * msec)
 * without overflowing.
 */
    /* Check for numerator overflow. */
    if( envelopeSampleRate < 1000 )
    {
        int divisor = envelopeSampleRate * msec;
        if( divisor <= 1000 )
        {
            /* Rather than overflow, just return MAX. */
            increment = FXP31_MAX_VALUE;
        }
        else
        {
            increment = 1000 * ( FXP31_MAX_VALUE / divisor ); /* DIVIDE - load preset */
        }
    }
    else
    {
        numerator = 1000 * ( FXP31_MAX_VALUE / envelopeSampleRate );
        increment = numerator / msec; /* DIVIDE - load preset */
    }
    return increment;
}

/********************************************************************/
/**
 * Setup info structure with information needed to execute envelope.
 */
void ADSR_Load( const EnvelopeADSR_Preset_t *preset, EnvelopeADSR_Info_t *info, int envelopeSampleRate )
{
    info->attackIncrement = ADSR_MSecToIncrement(preset->attackTime, envelopeSampleRate);
}

#if SPMIDI_SUPPORT_LOADING
/********************************************************************/
/**
 * Extract preset information from bulk dump format passed from instrument editor.
 */
unsigned char *ADSR_Define( EnvelopeADSR_Preset_t *preset, unsigned char *p )
{
    p = SS_ParseShort( &preset->attackTime, p );
    p = SS_ParseShort( &preset->decayTime, p );
    p = SS_ParseShort( &preset->sustainLevel, p );
    p = SS_ParseShort( &preset->releaseTime, p );
    preset->pitchScalar = *p++;
    preset->flags = *p++;
    return p;
}
#endif /* SPMIDI_SUPPORT_LOADING */

#if 0
/********************************************************************/
/**
 * Generate an envelope and write it to a file for testing.
 */
int main( void )
{
    FILE *fid;
    EnvelopeADSR_Preset_t PRESET = { 100, 500, 0, 900, 200 };
    EnvelopeADSR_Info_t INFO;
    EnvelopeADSR_t ADSR = { 0 };
    int blockCounter = 0;

    printf("Test ADSR\n");
    ADSR_Load( &PRESET, &INFO, 44100 );

    ADSR_Start( &ADSR, &PRESET, 60, 44100 );

    /* Open file. */
    fid = fopen( "rendered_adsr.raw", "wb" );
    if( fid == NULL )
    {
        printf("Can't open output file rendered_adsr.raw\n" );
        return 1;
    }

    while( ADSR_Next( &INFO, &ADSR, NULL ) == 0 )
    {
        int is;
        for( is=0; is<SS_FRAMES_PER_BLOCK; is++ )
        {
            short sample = (short) (ADSR.output[is] >> 16);
            fwrite( &sample, sizeof(short), 1, fid );
        }

        /* Release envelope after decay. */
        if( blockCounter++ == 22050/8 )
        {
            ADSR.stage = ADSR_STAGE_RELEASE;
        }
    }

    fclose( fid );

    printf("Test complete.\n");

    return 0;
}
#endif
