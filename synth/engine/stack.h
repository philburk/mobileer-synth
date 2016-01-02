#ifndef _STACK_H
#define _STACK_H
/* $Id: stack.h,v 1.5 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 * Stack operations
 *
 * Implements standard stack operators like push, pop, etc using an array, assuming
 * that maximum stack size is known at startup.
 *
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/engine/spmidi_host.h"

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define SPMIDI_STACK_SIZE 16

typedef struct Stack_s
{
    int          tos;               /* points to current top of stack */
    spmBoolean   err;               /* TRUE if an error occurred, otherwise FALSE */
    spmSInt32    base[SPMIDI_STACK_SIZE];  /* allocated stack memory */
} Stack_t;

/**********************************************************************
 ** Stack_Error()
 * Return error code from stack as -ve if an error occurred in a
 * stack operation, as per SPMIDI convention.
 */
SPMIDI_Error Stack_Error( Stack_t* stack );

/**********************************************************************
 ** Stack_Clear()
 * Initialize stack parms, so that stack is empty and error is cleared.
 */
void Stack_Clear( Stack_t* stack );

/**********************************************************************
 ** Stack_Create()
 * Allocate a stack of 32-bit words, and return a pointer to it.
 */
Stack_t* Stack_Create( void );

/**********************************************************************
 ** Stack_IsEmpty()
 * If the top of stack is negative, stack is empty so return TRUE;
 * otherwise, return FALSE.
 */
spmBoolean Stack_IsEmpty( Stack_t* stack );

/**********************************************************************
 ** Stack_Push()
 * Push a value onto the stack.  Set error if overflow occurs.  In this
 * case, stack remains intact.
 *
 * value: to be pushed.
 */
void Stack_Push( Stack_t* stack, spmSInt32 value );

/**********************************************************************
 ** Stack_Pop()
 * Pop value from top of stack.  If stack is empty, set error code.
 */
spmSInt32 Stack_Pop( Stack_t* stack );

/**********************************************************************
 ** Stack_Delete()
 * Free stack memory.
 */
void Stack_Delete( Stack_t* stack );

#endif /* _STACK_H */
