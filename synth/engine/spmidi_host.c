/* $Id: spmidi_host.c,v 1.24 2007/10/10 00:23:47 philjmsl Exp $ */
/**
 *
 * Host dependencies.
 * See spmidi_host.h for an explanation.
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/include/spmidi_config.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"
#include "spmidi/include/spmidi_print.h"

#ifndef SPMIDI_DEBUG_HOST
#define SPMIDI_DEBUG_HOST  (0)
#endif

#if SPMIDI_DEBUG_HOST
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#define DBUGMSGNUMD( msg, num ) { DBUGMSG( msg ); DBUGNUMD( num ); DBUGMSG("\n"); }

#if SPMIDI_SUPPORT_LOADING || SPMIDI_SUPPORT_MALLOC

#ifdef BUILD_FOR_PALMOS

#include <MemoryMgr.h>

static void *MEHost_AllocateMemory(int numBytes)
{
    return MemPtrNew(numBytes);
}
static void MEHost_FreeMemory( void *ptr)
{
    MemPtrFree(ptr);
}

/* You may want to allocate a critical section object or a semaphore here. */
static int MEHost_Init(void)
{
    return SPMIDI_Error_None;
}
static int MEHost_Term(void)
{
    return SPMIDI_Error_None;
}

#elif SPMIDI_USE_INTERNAL_MEMHEAP

#include "memheap.h"
/* Define memory allocation routines so that they can be easily replaced.
* Feel free to replace these functions with equivalent functions for your system.
*/
static void *MEHost_AllocateMemory(int numBytes)
{
    return MemHeap_Allocate(numBytes);
}
static void MEHost_FreeMemory( void *ptr)
{
    MemHeap_Free(ptr);
}

static int MEHost_Init(void)
{
    MemHeap_Init();
    return 0;
}
static int MEHost_Term(void)
{
    MemHeap_Term();
    return 0;
}

#else

/* Use system malloc() */
#include "stdlib.h"
/* Define memory allocation routines so that they can be easily replaced.
* Feel free to replace these functions with equivalent functions for your system.
*/
static void *MEHost_AllocateMemory(int numBytes)
{
    return malloc(numBytes);
}
static void MEHost_FreeMemory( void *ptr)
{
    free(ptr);
}

static int MEHost_Init(void)
{
    return SPMIDI_Error_None;
}
static int MEHost_Term(void)
{
    return SPMIDI_Error_None;
}

#endif /* BUILD_FOR_PALMOS */

#else /* SPMIDI_SUPPORT_LOADING || SPMIDI_SUPPORT_MALLOC */

/* Stub out memory allocation routines. */
static void *MEHost_AllocateMemory(int numBytes)
{
    return ((void *)(numBytes - numBytes));
}
static void MEHost_FreeMemory( void *ptr)
{
    (void) ptr;
}

static int MEHost_Init(void)
{
    return SPMIDI_Error_None;
}
static int MEHost_Term(void)
{
    return SPMIDI_Error_None;
}

#endif /* SPMIDI_SUPPORT_LOADING || SPMIDI_SUPPORT_MALLOC */


void SPMIDI_EnterCriticalSection(void)
{}
void SPMIDI_LeaveCriticalSection(void)
{}

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/* Track every memory allocation and free to avoid errors. */
#define MEMORY_STATE_ALLOCATED  (0xCAFE5432)
#define MEMORY_STATE_FREED  (0xDEADBEEF)

typedef struct MemoryTracker_s
{
    unsigned long magic;
    unsigned long numBytes;
    unsigned long index;
    const char   *name;
}
MemoryTracker_t;

#define SPMIDI_ALLOCATION_UNLIMITED  (-1)

static int sAllocationLimit; /* Initialized to SPMIDI_ALLOCATION_UNLIMITED by SPMIDI_HostInit() */
static int sFailCountdown; /* Initialized to SPMIDI_ALLOCATION_UNLIMITED by SPMIDI_HostInit() */
static int sNumBytesAllocated = 0;
static int sNumAllocations = 0;
static int sNextAllocationIndex = 0;
static int sMaxBytesAllocated = 0;

/*****************************************************************/
void *SPMIDI_AllocateMemory(int numBytes)
{
    return SPMIDI_AllocateMemoryNamed( numBytes, NULL );
}

/*****************************************************************/
void *SPMIDI_AllocateMemoryNamed(int numBytes, const char *name)
{
    MemoryTracker_t *memTracker;

    /* Check that we're not over the limit */
    if( (sAllocationLimit != SPMIDI_ALLOCATION_UNLIMITED) &&
        ((sNumBytesAllocated + numBytes) > sAllocationLimit) )
        return NULL;

    /* Check countdown fail, if enabled */
    if( sFailCountdown >= 0 )
    {
        /* Only fail once. */
        if( sFailCountdown-- == 0)
        {
            return NULL;
        }
    }

    /* Put special structure in front of user memory. */
    memTracker = (MemoryTracker_t *) MEHost_AllocateMemory(
                     numBytes + sizeof(MemoryTracker_t) );
    if( memTracker != NULL )
    {
        sNumBytesAllocated += numBytes;
        if( sNumBytesAllocated > sMaxBytesAllocated )
        {
            sMaxBytesAllocated = sNumBytesAllocated;
        }
        sNumAllocations += 1;

        memTracker->magic = MEMORY_STATE_ALLOCATED;
        memTracker->numBytes = numBytes;
        memTracker->index = sNextAllocationIndex++;
        memTracker->name = name;
        DBUGMSG("SPMIDI_AllocateMemory, at ");
        DBUGNUMH( memTracker );
        DBUGMSG(", index = ");
        DBUGNUMD( memTracker->index );
        DBUGMSG(", ");
        DBUGNUMD( numBytes );
        if( name != NULL )
        {
            DBUGMSG(", ");
            DBUGMSG( name );
        }
        DBUGMSG("\n");
        return (void *) (memTracker + 1);
    }
    else
    {
        return NULL;
    }
}

/*****************************************************************/
void SPMIDI_FreeMemory( void *ptr)
{
    if( ptr != NULL )
    {
        MemoryTracker_t *memTracker = (MemoryTracker_t *) ptr;
        /* Go back before user memory to our structure. */
        memTracker -= 1;
        
        DBUGMSG("SPMIDI_FreeMemory, at ");
        DBUGNUMH( memTracker );
        DBUGMSG(", index = ");
        DBUGNUMD( memTracker->index );
        DBUGMSG(", ");
        DBUGNUMD( memTracker->numBytes );
        if( memTracker->name != NULL )
        {
            DBUGMSG(", ");
            DBUGMSG( memTracker->name );
        }
        DBUGMSG("\n");

        /* Is this bogus? */
        if( memTracker->magic != MEMORY_STATE_ALLOCATED )
        {
            if( memTracker->magic == MEMORY_STATE_FREED )
            {
                PRTMSGNUMD("SPMIDI_FreeMemory: memory freed twice, index = ", memTracker->index);
                PRTMSGNUMD("SPMIDI_FreeMemory:                     numBytes = ", memTracker->numBytes);
            }
            else
            {
                PRTMSGNUMH("SPMIDI_FreeMemory: attempt to free unallocated memory at ", memTracker+1 );
            }
        }
        else
        {
            /* Mark as freed. */
            memTracker->magic = MEMORY_STATE_FREED;
            sNumAllocations--;
            sNumBytesAllocated -= memTracker->numBytes;
            MEHost_FreeMemory( memTracker );
        }
    }
}

/*****************************************************************/
int SPMIDI_HostInit(void)
{
    sNumAllocations = 0;
    sNumBytesAllocated = 0;
    sMaxBytesAllocated = 0;
    sAllocationLimit = SPMIDI_ALLOCATION_UNLIMITED;
    sFailCountdown = SPMIDI_ALLOCATION_UNLIMITED;
    return MEHost_Init();
}

/*****************************************************************/
int SPMIDI_HostTerm(void)
{
    DBUGMSGNUMD( "SPMIDI_HostTerm: num memory allocations = ", sNumAllocations );
    DBUGMSGNUMD( "SPMIDI_HostTerm: num bytes allocated = ", sNumBytesAllocated );
    DBUGMSGNUMD( "SPMIDI_HostTerm: max bytes allocated = ", sMaxBytesAllocated );
    DBUGMSGNUMD( "SPMIDI_HostTerm: allocation limit = ", sAllocationLimit );
    DBUGMSGNUMD( "SPMIDI_HostTerm: fail countdown = ", sFailCountdown );
    return MEHost_Term();
}

/*****************************************************************/
/**
 * For debugging and QA purposes.
 * @return number of blocks of memory allocated
 */
int SPMIDI_GetMemoryAllocationCount( void )
{
    return sNumAllocations;
}
/**
 * For debugging and QA purposes.
 * @return number of bytes of memory currently allocated
 */
int SPMIDI_GetMemoryBytesAllocated( void )
{
    return sNumBytesAllocated;
}
/**
 * For debugging and QA purposes.
 * @param total number of bytes available for all allocations
 */
void SPMIDI_SetMemoryAllocationLimit( int numBytes )
{
    sAllocationLimit = numBytes;
}
/**
 * For debugging and QA purposes.
 * @param countdown of allocations before failure, or SPMIDI_ALLOCATION_UNLIMITED for no countdown
 */
void SPMIDI_SetMemoryAllocationCountdown( int numAllocs )
{
    sFailCountdown = numAllocs;
}
