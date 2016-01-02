/* $Id$ */
#ifndef _MEMHEAP_H
#define _MEMHEAP_H

/***************************************************************
** Custom Memory Allocator
**
** Author: Phil Burk
** Copyright 2005 Mobileer
***************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif

    /** Initialize the static array that memory will be allocated from. */
    void  MemHeap_Init( void );

    /** Allocate a block of memory from an internal array.
     * Behavior is similar to malloc.
     * @return address or NULL if not enough memory.
     */
    void *MemHeap_Allocate( spmSInt32 NumBytes );

    /**
     * Free a block of memory allocated using MemHeap_Allocate.
     */
    void  MemHeap_Free( void *Mem );

    /**
     * Clean up memheap data structures.
     */
    void  MemHeap_Term( void );

#ifdef __cplusplus
}
#endif


#endif /* _MEMHEAP_H */
