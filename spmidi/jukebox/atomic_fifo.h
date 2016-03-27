#ifndef _ATOMICFIFO_H
#define _ATOMICFIFO_H
/*
 * Atomic FIFO Control System
 *
 * Author: Phil Burk
 * Copyright 1997 Phil Burk
 * All Rights Reserved.
 */

#include <stdlib.h>

typedef struct
{
	int   af_NumElements;  /* Number of elements in FIFO. Power of 2. Set by AFIFO_Init. */
	/* These are declared volatile because they are written by a different thread than the reader. */
	volatile int   af_WriteIndex;   /* Index of next writable element. Set by AFIFO_NextWritable. */
	volatile int   af_ReadIndex;    /* Index of next readable element. Set by AFIFO_NextReadable. */
	char *af_DataPtr;
	int   af_ElementSize;
}
AtomicFIFO;

/*
 * Initialize FIFO.
 * numElements must be power of 2, returns -1 if not.
 */
int AFIFO_Init( AtomicFIFO *af, int numElements, int elementSize, void *dataPtr );

/* Return 1 if full, else return 0. */
int AFIFO_IsFull( AtomicFIFO *af );

/* Return 1 if empty, else return 0. */
int AFIFO_IsEmpty( AtomicFIFO *af );

/* Advance write index.
 * Return 0, or -1 if full.
 */
int AFIFO_AdvanceWriter( AtomicFIFO *af );

/* Advance read index.
 * Return 0, or -1 if empty.
 */
int AFIFO_AdvanceReader( AtomicFIFO *af );

/* Returns address of next element that can be written, or NULL. */
void *AFIFO_NextWritable( AtomicFIFO *af );
/* Returns address of next element that can be read, or NULL. */
void *AFIFO_NextReadable( AtomicFIFO *af );

#endif  /* _ATOMICFIFO_H */
