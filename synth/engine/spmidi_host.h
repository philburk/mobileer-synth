#ifndef _SPMIDI_HOST_H
#define _SPMIDI_HOST_H
/* $Id: spmidi_host.h,v 1.11 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Host dependencies.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include "spmidi/include/spmidi_config.h"


#ifdef __cplusplus
extern "C"
{
#endif

    /**
    * If passed to SPMIDI_SetMemoryAllocationCountdown(), will not fail on nth allocation
    */
#define SPMIDI_ALLOCATION_UNLIMITED  (-1)

#if SPMIDI_PRODUCTION
#define SPMIDI_ALLOC_MEM( numBytes, name ) SPMIDI_AllocateMemory( numBytes )
#else
#define SPMIDI_ALLOC_MEM( numBytes, name ) SPMIDI_AllocateMemoryNamed(numBytes, name)
#endif
#define SPMIDI_FREE_MEM  SPMIDI_FreeMemory

    /**
     * Allocate a block of memory from the heap.
     * This may be redefined as needed for the system.
     * If the system does not support memory allocation
     * then define SPMIDI_SUPPORT_MALLOC as (0)
     * and memory will be allocated from an internal pool.
     */
    void *SPMIDI_AllocateMemory(int numBytes);

    /** Allocate memory and give block a name for help with debugging. */
    void *SPMIDI_AllocateMemoryNamed(int numBytes, const char *name);

    void SPMIDI_FreeMemory( void *ptr);

    /**
     * For debugging and QA purposes.
     * @return number of blocks of memory allocated
     */
    int SPMIDI_GetMemoryAllocationCount( void );
    /**
     * For debugging and QA purposes.
     * @return number of bytes of memory currently allocated
     */
    int SPMIDI_GetMemoryBytesAllocated( void );
    /**
     * For debugging and QA purposes.
     * @param total number of bytes available for all allocations
     */
    void SPMIDI_SetMemoryAllocationLimit( int numBytes );
    /**
     * For debugging and QA purposes ONLY.
     * @param countdown of allocations before failure, or SPMIDI_ALLOCATION_UNLIMITED for no countdown
     */
    void SPMIDI_SetMemoryAllocationCountdown( int numAllocs );
    /**
     * Enter a section of code that must be synchronized.
     * This is only required if multiple threads will be sharing
     * the SPMIDI code.
     * If another thread is inside the critical section, then this function
     * will block until it is able to get a lock.
     */
    void SPMIDI_EnterCriticalSection(void);

    /**
     * Free the lock and exit the critical section of code.
     */
    void SPMIDI_LeaveCriticalSection(void);

    /**
     * Perform any necessary host initialization.
     * This will be called once by SPMIDI_Initialize().
     */
    int SPMIDI_HostInit(void);
    int SPMIDI_HostTerm(void);

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_HOST_H */
