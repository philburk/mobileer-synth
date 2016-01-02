#ifndef _SPMIDI_JUKEBOX_PLAYLIST_H
#define _SPMIDI_JUKEBOX_PLAYLIST_H
/**
 * Song playlist for JukeBox.
 * Generated automatically by Mobileer Editor
 * Do NOT edit by hand!
 * (C) Mobileer, Inc. CONFIDENTIAL and PROPRIETARY
 */

#include "songs/song_FurryLisa_rt.h"

typedef struct JukeBoxEntry_s
{
    const unsigned char *image;
    int size;
} JukeBoxEntry_t;


    JukeBoxEntry_t jukeBoxSongs[] =
{
    { song_FurryLisa_rt, sizeof(song_FurryLisa_rt) },
};

#define JUKEBOX_NUM_SONGS  (sizeof(jukeBoxSongs)/sizeof(JukeBoxEntry_t))


#endif /* _SPMIDI_JUKEBOX_PLAYLIST_H */
