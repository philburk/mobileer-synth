/* $Id: test_memalloc.c,v 1.2 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * Test ARM assembly.
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"

#define SIZE_P  (500)
#define SIZE_S  (173)


/*******************************************************************/
int main(void);
int main(void)
{
    unsigned char *p;
    unsigned char *s;
    int numAllocations1;
    int numAllocations2;
    
    SPMIDI_Initialize();
    
    numAllocations1 = SPMIDI_GetMemoryAllocationCount();
    printf("numAllocations1 = %d\n", numAllocations1 );
    printf("bytes allocated = %d\n", SPMIDI_GetMemoryBytesAllocated() );
    
    p = SPMIDI_AllocateMemory( SIZE_P );
    if( p == NULL ) goto error;
    printf("p = %p\n", p );

    s = SPMIDI_AllocateMemory( SIZE_S );
    if( s == NULL ) goto error;
    printf("s = %p\n", s );

    numAllocations2 = SPMIDI_GetMemoryAllocationCount();
    if( (numAllocations2 - numAllocations1) != 2 )
    {
        printf("ERROR - SPMIDI_GetMemoryAllocationCount() failed. SHould be two.\n");
    }
    printf("numAllocations2 = %d\n", numAllocations2 );
    printf("bytes allocated = %d\n", SPMIDI_GetMemoryBytesAllocated() );
    
    SPMIDI_FreeMemory( p );
    SPMIDI_FreeMemory( s );
    
    numAllocations2 = SPMIDI_GetMemoryAllocationCount();
    if( (numAllocations2 - numAllocations1) != 0 )
    {
        printf("ERROR - SPMIDI_GetMemoryAllocationCount() failed. Should be zero.\n");
    }
    printf("numAllocations2 = %d\n", numAllocations2 );
    printf("bytes allocated = %d\n", SPMIDI_GetMemoryBytesAllocated() );
    
    return 0;

error:
    printf("ERROR - SPMIDI_AllocateMemory() failed.\n");
    return 1;
}
