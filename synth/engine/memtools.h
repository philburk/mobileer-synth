#ifndef _MEMTOOLS_H
#define _MEMTOOLS_H
/* $Id: memtools.h,v 1.12 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Miscellaneous Memory Tools.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include "spmidi/include/spmidi_config.h"


#ifdef __cplusplus
extern "C"
{
#endif

    void MemTools_Clear( void *address, int numBytes );

    void MemTools_Copy( void *dest, const void *source, int numBytes );

#ifdef __cplusplus
}
#endif

#endif /* _MEMTOOLS_H */
