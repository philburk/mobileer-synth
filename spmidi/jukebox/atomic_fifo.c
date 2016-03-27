/*
 * Atomic FIFO Control System
 *
 * Author: Phil Burk
 * Copyright 1997 Phil Burk
 * All Rights Reserved.
 */

#include "spmidi/jukebox/atomic_fifo.h"

#define AFIFO_CHECK_MASK(fifo)  (((fifo)->af_NumElements*2)-1)
#define AFIFO_PTR_MASK(fifo)  ((fifo)->af_NumElements-1)

/*
 * Initialize FIFO.
 * numElements must be power of 2, returns -1 if not.
 */
int AFIFO_Init( AtomicFIFO *af, int numElements, int elementSize, void *dataPtr )
{
	if( ((numElements-1) & numElements) != 0)
		return -1; /* Not Power of two. */
	af->af_NumElements = numElements;
	af->af_DataPtr = (char *)dataPtr;
	af->af_ElementSize = elementSize;
	af->af_WriteIndex = 0;
	af->af_ReadIndex = 0;
	return 0;
}

/* Return 1 if full, else return 0. */
int AFIFO_IsFull( AtomicFIFO *af )
{
	return ( ((af->af_WriteIndex - af->af_ReadIndex) & AFIFO_CHECK_MASK(af)) == af->af_NumElements );
}

/* Return 1 if empty, else return 0. */
int AFIFO_IsEmpty( AtomicFIFO *af )
{
	return ( af->af_WriteIndex == af->af_ReadIndex);
}

/* Advance write index.
 * Return 0, or -1 if full.
 */
int AFIFO_AdvanceWriter( AtomicFIFO *af )
{
	if( AFIFO_IsFull(af) )
	{
		return -1;
	}
	else
	{
		af->af_WriteIndex = (af->af_WriteIndex + 1) & AFIFO_CHECK_MASK(af);
		return 0;
	}
}

/* Advance read index.
 * Return 0, or -1 if empty.
 */
int AFIFO_AdvanceReader( AtomicFIFO *af )
{
	if( AFIFO_IsEmpty(af) )
	{
		return -1;
	}
	else
	{
		af->af_ReadIndex = (af->af_ReadIndex + 1) & AFIFO_CHECK_MASK(af);
		return 0;
	}
}

/* Returns address of next element that can be written, or NULL.
 * Does not advance, index. Call AFIFO_AdvanceWriter() after reading finished.
 */
void *AFIFO_NextWritable( AtomicFIFO *af )
{
	if( AFIFO_IsFull(af) || (af->af_DataPtr == NULL) )
	{
		return NULL;
	}
	else
	{
		char *ptr = &af->af_DataPtr[ af->af_ElementSize * (af->af_WriteIndex & AFIFO_PTR_MASK(af)) ];
		return (void *) ptr;
	}
}

/* Returns address of next element that can be read, or NULL.
 * Does not advance, index. Call AFIFO_AdvanceReader() after reading finished.
 */
void *AFIFO_NextReadable( AtomicFIFO *af )
{
	if( AFIFO_IsEmpty(af) || (af->af_DataPtr == NULL) )
	{
		return NULL;
	}
	else
	{
		char *ptr = &(af->af_DataPtr[ af->af_ElementSize * (af->af_ReadIndex & AFIFO_PTR_MASK(af)) ] );
		return (void *) ptr;
	}
}

