/***************************************************************
** Alternative memory allocator.
**
** Author: Phil Burk
** Copyright 2005 Mobileer
***************************************************************/

#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"

#if SPMIDI_USE_INTERNAL_MEMHEAP

static char  *gMemPoolPtr = NULL;
static spmSInt32 gMemPoolSize;

/* Prevent fragmentation into pieces smaller than 16.
 * Must be big enough for the MemListNode. */
#define MEMHEAP_BLOCK_SIZE (16)

#define ALIGN_UP_TO_BLOCK(x) \
    ((((spmUInt32)(x)) + MEMHEAP_BLOCK_SIZE - 1) & ~(MEMHEAP_BLOCK_SIZE - 1))

static char sMemoryPool[SPMIDI_MEMHEAP_SIZE];

static DoubleList gMemList;

typedef struct MemListNode
{
    DoubleNode  node; /* Assume two pointers so 8 bytes. Could break on 64 bit processor. */
    spmSInt32   size;
}
MemListNode;

/***************************************************************
** Insert into a linked list.
** Smaller blocks are near the beginning so we preserve larger blocks.
**
*/
static void MemHeap_InsertFreeNode( MemListNode *freeNode )
{
    MemListNode *mln;
    MemListNode *nextBiggest = NULL;

    /* Scan list from low to high looking for next largest node. */
    DLL_FOR_ALL( MemListNode, mln, &gMemList )
    {
        if( mln->size >= freeNode->size )
        {
            nextBiggest = mln;
            break;
        }
    }

    DLL_InitNode( &freeNode->node );
    if( nextBiggest == NULL )
    {
        /* Nothing bigger so add to end of list. */
        DLL_AddTail( &gMemList, &freeNode->node );
    }
    else
    {
        /* Add this node before the next biggest one we found. */
        DLL_InsertBefore( &nextBiggest->node, &freeNode->node );
    }
}

/***************************************************************
** Free memory of any size.
*/
static void MemHeap_FreeRawMem( char *allocated, spmSInt32 numBytes )
{
    MemListNode *mln;
    MemListNode *freeNode;
    MemListNode *AdjacentLower = NULL;
    MemListNode *AdjacentHigher = NULL;

    /* Check memory alignment. */
    if( ( ((spmSInt32)allocated) & (MEMHEAP_BLOCK_SIZE - 1)) != 0)
    {
        PRTMSGNUMH("MemHeap_FreeRawMem: misaligned allocated = 0x", (spmSInt32) allocated );
        return;
    }

    /* Scan list from low to high looking for various nodes. */
    DLL_FOR_ALL( MemListNode, mln, &gMemList )
    {
        if( (((char *) mln) + mln->size) == allocated )
        {
            AdjacentLower = mln;
            if( AdjacentHigher != NULL )
                break; /* We have it bracketed. */
        }
        else if( ((char *) mln) == ( allocated + numBytes ))
        {
            AdjacentHigher = mln;
            if( AdjacentLower != NULL )
                break; /* We have it bracketed. */
        }
    }

    freeNode = (MemListNode *) allocated;

    /* Check to see if we can merge with higher node. */
    if( AdjacentHigher )
    {
        numBytes += AdjacentHigher->size;
        DLL_Remove( &AdjacentHigher->node);
    }

    /* Check to see if we can merge with lower node. */
    if( AdjacentLower )
    {
        numBytes += AdjacentLower->size;
        freeNode = AdjacentLower;
        /* Remove it so we can re-insert it in sorted order. */
        DLL_Remove( &AdjacentLower->node);
    }

    freeNode->size = numBytes;
    MemHeap_InsertFreeNode( freeNode );

}

/***************************************************************
** Setup memory list. Initialize allocator.
*/
static void MemHeap_InitMemBlock( void *addr, spmUInt32 poolSize )
{
    char *AlignedMemory;
    spmSInt32 AlignedSize;

    /* Set globals. */
    gMemPoolPtr = addr;
    gMemPoolSize = poolSize;

    DLL_InitList( &gMemList );

    /* Adjust to next highest aligned memory location. */
    AlignedMemory = (char *) ALIGN_UP_TO_BLOCK(gMemPoolPtr);

    /* Adjust size to reflect aligned memory. */
    AlignedSize = gMemPoolSize - (AlignedMemory - gMemPoolPtr);

    /* Align size of pool. */
    AlignedSize = AlignedSize & ~(MEMHEAP_BLOCK_SIZE - 1);

    /* Free to pool. */
    MemHeap_FreeRawMem( AlignedMemory, AlignedSize );

}

/***************************************************************
** Allocate mem from list of free nodes.
*/
static char *MemHeap_AllocRawMem( spmSInt32 numBytes )
{
    char *allocated = NULL;
    MemListNode *mln;

    if( gMemPoolPtr == NULL )
        return NULL;

    /* PRTMSGNUMD("MemHeap_AllocRawMem: numBytes = ", numBytes ); */

    /* Scan list from low to high until we find a node big enough. */
    DLL_FOR_ALL( MemListNode, mln, &gMemList )
    {
        if( mln->size >= numBytes )
        {
            spmSInt32 RemSize;

            allocated = (char *) mln;

            /* Remove this node from list. */
            DLL_Remove( (DoubleNode *) mln );

            /* Is there enough left in block to make it worth splitting? */
            RemSize = mln->size - numBytes;
            if( RemSize >= MEMHEAP_BLOCK_SIZE )
            {
                MemListNode *freeNode;

                /* Put leftover memory back in list. */
                freeNode = (MemListNode *) (allocated + numBytes);
                freeNode->size = RemSize;
                MemHeap_InsertFreeNode( freeNode );
            }
            else if( RemSize != 0 )
            {
                /* Some piece of this block will not get used. */
                /* This should never happen if allocation is in whole blocks. */
                PRTMSG("ERROR in MemHeap_AllocRawMem: fragment lost.\n");
                return NULL;
            }
            break;
        }

    }

    return allocated;
}

/***************************************************************
** Store block size at first cell.
*/
void *MemHeap_Allocate( spmSInt32 numBytes )
{
    spmSInt32 *headerMem;

    if( numBytes <= 0 )
        return NULL;

    /* Allocate an extra cell for size. */
    numBytes += sizeof(spmSInt32);

    /* Allocate in whole blocks of 16 bytes */
    numBytes = ALIGN_UP_TO_BLOCK(numBytes);

    headerMem = (spmSInt32 *)MemHeap_AllocRawMem( numBytes );

    /* Set raw size at beginning of block and pont to user area. */
    if( headerMem != NULL )
    {
        *headerMem++ = numBytes;
    }
    /*
        else
        {
            PRTMSG("MemHeap_Allocate() failed.\n");
        }
    */
    return (void *) headerMem;
}

/***************************************************************
** Free mem with mem size at first cell.
*/
void MemHeap_Free( void *allocated )
{
    spmSInt32 *headerMem;
    spmSInt32 numBytes;

    if( allocated == NULL )
        return;

    /* Allocate an extra cell for size. */
    headerMem = (spmSInt32 *) allocated;
    headerMem--;
    numBytes = *headerMem;

    MemHeap_FreeRawMem( (char *) headerMem, numBytes );

}

void MemHeap_Init( void )
{
    MemHeap_InitMemBlock( sMemoryPool, SPMIDI_MEMHEAP_SIZE );
}

void MemHeap_Term( void )
{
    gMemPoolPtr = NULL;
}

#else /* SPMIDI_USE_INTERNAL_MEMHEAP */
int sMemHeapPreventCompilerWarning; /* Some compilers will complain about an empty file. */
#endif /* SPMIDI_USE_INTERNAL_MEMHEAP */

