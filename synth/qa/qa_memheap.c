/* $Id: qa_memheap.c,v 1.5 2007/10/02 16:24:50 philjmsl Exp $ */
/**
 *
 * @file qa_memheap.c
 * @brief Test Mobileer Memory Heap.
 * @author Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 * Allocate and reallocate memory blocks.
 *
 * Note; this test requires that the compiler variable
 *       SPMIDI_USE_INTERNAL_MEMHEAP be defined as (1)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "spmidi/include/spmidi.h"
#include "memheap.h"
#include "spmidi/qa/qa_tools.h"

#define MAX_ALLOCATIONS  (1024)
void *aAllocatedBlocks[MAX_ALLOCATIONS] = { NULL };
int sNumAllocations = 0;

/*******************************************************************/
static void MHTest_FreeAllMemory( void )
{
    int i;
    for( i=0; i<sNumAllocations; i++ )
    {
        MemHeap_Free( aAllocatedBlocks[i] );
        aAllocatedBlocks[i] = NULL;
    }
    sNumAllocations = 0;
}

/*******************************************************************/
static int MHTest_AllocateAllMemory( void )
{
    int blockSize = 1<<16;
    sNumAllocations = 0;

    while( (blockSize > 8) && (sNumAllocations < MAX_ALLOCATIONS) )
    {
        void *mem = MemHeap_Allocate( blockSize );
        if( mem == NULL )
        {
            blockSize = blockSize >> 1;
        }
        else
        {
            aAllocatedBlocks[sNumAllocations++] = mem;
        }
    }
    return sNumAllocations;
}

/*******************************************************************/
/**
 * Test to see if memory blocks are properly coalesced when adjacent.
 * @return true if error occured.
 */
static void MHTest_Coalescense( void )
{
    void *p1, *p2, *p3, *p4, *p5;
    void *np2, *np3;
    int size = 5 * 16;

    /* These should all get allocated sequentially. */
    p1 = MemHeap_Allocate( size );
    QA_Assert( (p1 != NULL) , "allocated P1, p1 NULL" );
    p2 = MemHeap_Allocate( size );
    QA_Assert( (p2 != NULL) , "allocated P2, p2 NULL" );
    p3 = MemHeap_Allocate( size );
    QA_Assert( (p3 != NULL) , "allocated P3, p3 NULL" );
    p4 = MemHeap_Allocate( size );
    QA_Assert( (p4 != NULL) , "allocated P4, p4 NULL" );
    p5 = MemHeap_Allocate( size );
    QA_Assert( (p5 != NULL) , "allocated P5, p5 NULL" );

    MHTest_AllocateAllMemory();

    np3 = MemHeap_Allocate( size );
    QA_Assert( (np3 == NULL) , "mem all gone, np3 allocated when mem all gone" );

    /* Free block in middle. */
    MemHeap_Free( p3 );
    np3 = MemHeap_Allocate( size );
    QA_Assert( (np3 == p3) , "reallocated np3, np3 != p3" );

    /* Try to allocate a block twice as big. */
    MemHeap_Free( np3 );
    np3 = MemHeap_Allocate( 2*size );
    QA_Assert( (np3 == NULL) , "reallocated np3, np3 should be too big" );

    /* Free higher memory block. It should be merged with p3 memory so we can allocate a bigger block. */
    MemHeap_Free( p4 );
    np3 = MemHeap_Allocate( 2*size );
    QA_Assert( (np3 != NULL) , "reallocated np3, np3 couldn't use p4 memory" );

    MemHeap_Free( np3 );
    np2 = MemHeap_Allocate( 3*size );
    QA_Assert( (np2 == NULL) , "allocate np2, should not be enough" );

    /* Free lower memory block. It should be merged with p3 memory so we can allocate a bigger block. */
    MemHeap_Free( p2 );
    np2 = MemHeap_Allocate( 3*size );
    QA_Assert( (np2 != NULL) , "allocate np2, should be able to get three blocks" );

    MHTest_FreeAllMemory();

    MemHeap_Free( np2 );
    MemHeap_Free( p1 );
    MemHeap_Free( p5 );

}

/*******************************************************************/
static void MHTest_CheckSameSize( void )
{
    void *p1, *p2;

    p1 = MemHeap_Allocate( 64 );
    QA_Assert( (p1 != NULL) , "allocated P1, p1 NULL" );
    p2 = MemHeap_Allocate( 64 );
    QA_Assert( (p2 != NULL) , "allocated P2, p2 NULL" );

    MemHeap_Free( p1 );
    p1 = MemHeap_Allocate( 64 );
    QA_Assert( (p1 != NULL) , "reallocated P1, p1 NULL" );


    MemHeap_Free( p1 );
    MemHeap_Free( p2 );

}

/*******************************************************************/
/**
 * Allocate and deallocate blocks, and check results
 * @return 0 if all tests succeed, non-0 if not.
 */
int main(void);
int main(void)
{
    int i;

    printf("Memheap Tests 1\n");

    QA_Init( "qa_memheap" );

    MemHeap_Init();

    /* Run tests twice in case they leave memheap in a bad condition. */
    for( i=0; i<2; i++ )
    {
        MHTest_CheckSameSize();
        MHTest_Coalescense();
    }

    MemHeap_Term();

    return QA_Term( 28 );
}
