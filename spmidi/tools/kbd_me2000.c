/* $Id: kbd_me2000.c,v 1.3 2007/10/02 16:24:51 philjmsl Exp $ */
/****************************************
 * Play ME2000 using a MIDI Keyboard on Windows.
 *
 * Author: Phil Burk
 * (C) 2006 Mobiler, Inc.
 ***************************************/

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <math.h>

#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/spmidi_play.h"
#include "spmidi/include/spmidi_jukebox.h"

/* PortAudio is an open-source audio API available free from www.portaudio.com */
#include "portaudio.h"

#include "spmidi_jukebox_song_ids.h"

/* If set to -1 then ask user. */
#define DEFAULT_MIDI_DEVICE   (-1)
// #define MSEC_LATENCY          (NULL)
#define MSEC_LATENCY          ("60")
static int sPrintNotes        = 0;

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE          (22050)
#define SAMPLES_PER_FRAME    (1)
#define BITS_PER_SAMPLE      (sizeof(short)*8)
/* Determines audio latency. */
#define NUM_AUDIO_BUFFERS    (2)
#define FRAMES_PER_BUFFER    (JukeBox_GetFramesPerTick() * 4)


/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

/****************************************************************/
/**
 * Get Audio from Jukebox to fill PortAudio buffer..
 */

int JBDemo_Callback(
    void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    PaTimestamp outTime, void *userData )
{
    /* Use information passed from foreground thread. */
    int framesLeft = framesPerBuffer;
    int framesGenerated = 0;
    short *outputPtr = (short *) outputBuffer;
    (void) inputBuffer;
    (void) outTime;
    (void) userData;

    /* The audio buffer is bigger than the synthesizer buffer so we
     * have to call the synthesizer several times to fill it.
     */
    while( framesLeft )
    {
        framesGenerated = JukeBox_SynthesizeAudioTick( outputPtr, framesPerBuffer, SAMPLES_PER_FRAME );
        if( framesGenerated <= 0 )
        {
            PRTMSGNUMH("Error: JukeBox_SynthesizeAudio returned ", framesGenerated );
            return 1; /* Tell PortAudio to stop. */
        }

        /* Advance pointer to next part of large output buffer. */
        outputPtr += SAMPLES_PER_FRAME * framesGenerated;

        /* Calculate how many frames are remaining. */
        framesLeft -= framesGenerated;
    }

    return 0;
}

PortAudioStream *audioStream;

/*******************************************************************/
int JBDemo_StartAudio( void )
{
    int result;

    /* Initialize audio hardware and open an output stream. */
    Pa_Initialize();
    result = Pa_OpenStream( &audioStream,
                       paNoDevice,
                       0,
                       paInt16,
                       NULL,
                       Pa_GetDefaultOutputDeviceID(),
                       SAMPLES_PER_FRAME,
                       paInt16,
                       NULL,
                       (double) SAMPLE_RATE,
                       FRAMES_PER_BUFFER,
                       NUM_AUDIO_BUFFERS,
                       0, /* streamFlags */
                       JBDemo_Callback,
                       NULL );

    if( result < 0 )
    {
        PRTMSG( "Pa_OpenStream returns " );
        PRTMSG( Pa_GetErrorText( result ) );
        PRTMSG( "\n" );

        goto error;
    }

    Pa_StartStream( audioStream );

error:
    return result;
}


/*******************************************************************/
int JBDemo_StopAudio( void )
{
    Pa_StopStream( audioStream );
    Pa_Terminate();
    return 0;
}

/********************************************************************/
void PrintMIDIErrorMsg(unsigned long err)
{
#define BUFFERSIZE 256
    char    buffer[BUFFERSIZE];
    
    if ((err = midiOutGetErrorText(err, &buffer[0], BUFFERSIZE)) == 0)
    {
        printf("%s\r\n", &buffer[0]);
    }
    else
    {
        printf("Unrecognized MIDI error number = %d", err );
    }
}


/**********************************************************************/
static PrintMIDIDevices( void )
{
    MIDIINCAPS     mic;

    unsigned long   numDevs, i;

    /* Get the number of MIDI Out devices in this computer */
    numDevs = midiInGetNumDevs();

    /* Go through all of those devices, displaying their names */
    for (i = 0; i < numDevs; i++)
    {
        /* Get info about the next device */
        if (!midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)))
        {
            /* Display its Device ID and name */
            printf("Device ID #%u: %s\n", i, mic.szPname);
        }
    }
}

/**********************************************************************/
void CALLBACK  MidiCallbackProc(
  HMIDIIN hMidiIn,  
  UINT wMsg,        
  DWORD dwInstance, 
  DWORD dwParam1,   
  DWORD dwParam2    
)
{
    (void) dwParam2;
    (void) hMidiIn;
    (void) dwInstance;

    if( wMsg == MIM_DATA )
    {
        int time = JukeBox_GetTime();
        int cmd = dwParam1 & 0xFF;
        int d1 = (dwParam1 >> 8) & 0xFF;
        int d2 = (dwParam1 >> 16) & 0xFF;

        JukeBox_MIDICommand( time, cmd, d1, d2);

        if( sPrintNotes )
        {
            printf("midi = 0x%02X, %d, %d\n", cmd, d1, d2);
            fflush(stdout);
        }
    }
}

/* SysEx not really used. This is just so the example code compiles. */
unsigned char SysXBuffer[256];

/* A flag to indicate whether I'm currently receiving a SysX message */
unsigned char SysXFlag = 0;

/**********************************************************************/
int main(int argc, char **argv)
{
    HMIDIIN        handle;
    MIDIHDR        midiHdr;
    unsigned long  err = 0;
    int            deviceID = (UINT) DEFAULT_MIDI_DEVICE;
    MIDIINCAPS     moc;
    char           line[256];
    int            i;
    char           *msecLatency = MSEC_LATENCY;


    printf( "Play ME2000 using MIDI keyboard.\n");
    printf( "(C) 2006 Mobileer, Inc. \n");
    printf( "CONFIDENTIAL and PROPRIETARY - Do NOT redistribute this program.\n\n");
    printf( "Usage: %s {-dDeviceID} {-mMsecLatency} {-p}\n\n", argv[0] );

    /* Parse command line. */
    for( i=1; i<argc; i++ )
    {
        char *s = argv[i];

        if( s[0] == '-' )
        {
            char c = s[1];

            switch( c )
            {
            case 'd':
                deviceID = atoi( &s[2] );
                break;

            case 'm':
                msecLatency = &s[2];
                break;

            case 'p':
                sPrintNotes = 1;
                break;
            }
        }
    }

    /* Pass latency setting to PortAudio through environment variable. */
    if( msecLatency != NULL )
    {
        printf("Setting minimum latency to %s\n", msecLatency );
        if( !SetEnvironmentVariable( "PA_MIN_LATENCY_MSEC", msecLatency ) )
        {
            printf( "SetEnvironmentVariable for latency failed.\n" );
            goto error;
        }
    }

    /* Ask for MIDI Device. */
    PrintMIDIDevices();

    if( deviceID < 0 )
    {
        printf( "Enter device ID: " );
        gets( line );
        deviceID = atoi( line );
    }

    /* Get info about the next device */
    if ((err = midiInGetDevCaps(deviceID, &moc, sizeof(MIDIINCAPS))) != MMSYSERR_NOERROR)
    {
        goto error;
    }

    printf( "\nTesting MIDI device ID = %d,  %s\n\n", deviceID, moc.szPname );

    /* Setup Synthesizer. */
    err = JukeBox_Initialize( 22050 );
    if( err < 0 )
    {   goto error;
    }

    JBDemo_StartAudio();


    /* Open default MIDI device. */
    if ((err = midiInOpen(&handle, deviceID, (DWORD) MidiCallbackProc, 0, CALLBACK_FUNCTION)) == MMSYSERR_NOERROR)
    {
        
        /* Store pointer to our input buffer for System Exclusive messages in MIDIHDR */
        midiHdr.lpData = (char *)&SysXBuffer[0];

        /* Store its size in the MIDIHDR */
        midiHdr.dwBufferLength = sizeof(SysXBuffer);

        /* Flags must be set to 0 */
        midiHdr.dwFlags = 0;

        /* Prepare the buffer and MIDIHDR */
        err = midiInPrepareHeader(handle, &midiHdr, sizeof(MIDIHDR));
        if (!err)
        {
            /* Queue MIDI input buffer */
            err = midiInAddBuffer(handle, &midiHdr, sizeof(MIDIHDR));
            if (!err)
            {
                /* Start recording Midi */
                err = midiInStart(handle);
                if (!err)
                {
                    printf("Play MIDI keyboard.\n");
                    printf("Hit ENTER to quit program.\n");
                    getchar();
                }
                
                /* Stop recording */
                midiInReset(handle);
            }
        }

        /* Close the MIDI device. */
        midiInClose(handle);
    }
    else
    {
        printf("Error opening the default MIDI Output device!\n");
        PrintMIDIErrorMsg(err);
    }

    JBDemo_StopAudio();

    JukeBox_Terminate();

    printf("Test complete.\n");
    return 0;

error:
    printf("Error occured, err = %d\n", err );
    return 1;
}
