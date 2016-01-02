/* $Id: memtools.c,v 1.11 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Memory Tools.
 * Provided to avoid dependence on external libraries.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include "spmidi/engine/memtools.h"

#ifndef SPMIDI_USE_STDLIB
#define SPMIDI_USE_STDLIB (0)
#endif

#if SPMIDI_USE_STDLIB
#include <memory.h>
#endif

/**
 * Clear a region of memory.
 */
void MemTools_Clear( void *address, int numBytes )
{
#if SPMIDI_USE_STDLIB
    memset( address, 0, numBytes );
#else
    char *p = (char *) address;
    int i;
    for( i=0; i<numBytes; i++ )
    {
        p[i] = 0;
    }
#endif
}

/**
 * Copy a region of memory to another.
 */
void MemTools_Copy( void *dest, const void *source, int numBytes )
{
#if SPMIDI_USE_STDLIB
    memcpy( dest, source, numBytes );
#else
    char *da = (char *) dest;
    char *sa = (char *) source;
    int i;
    if( (da-sa) > 0 )
    {
        /* If dest is after source then copy from beginning of source. OK if overlaps. */
        for( i=0; i<numBytes; i++ )
        {
            da[i] = sa[i];
        }
    }
    else
    {
        /* If source is after dest then copy from end of source. OK if overlaps. */
        for( i=numBytes-1; i>=0; i-- )
        {
            da[i] = sa[i];
        }
    }
#endif
}
