/*
 * $Id: stack.c,v 1.3 2007/10/02 16:14:42 philjmsl Exp $
 * Stack operations
 *
 * Implements standard stack operators like push, pop, etc using an array, assuming
 * that maximum stack size is known at startup.
 *
 * Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/include/spmidi_print.h"
#include "spmidi/engine/memtools.h"
#include "spmidi/engine/stack.h"

#if 1
#define DBUGMSG(x)   PRTMSG(x)
#else
#define DBUGMSG(x)
#endif

/**********************************************************************
 ** Stack_Error()
 * Return error code from stack as -ve if an error occurred in a
 * stack operation, as per SPMIDI convention.
 */
SPMIDI_Error Stack_Error( Stack_t* stack )
{
    return( -(stack->err) );
}

/**********************************************************************
 ** Stack_Clear()
 * Initialize stack parms, so that stack is empty and error is cleared.
 */
void Stack_Clear( Stack_t* stack )
{
    stack->tos = -1;
    stack->err = FALSE;
}

/**********************************************************************
 ** Stack_Create()
 * Allocate a stack of 32-bit words, and return a pointer to it.
 */
Stack_t* Stack_Create()
{
    Stack_t* s = NULL;

    s = (Stack_t*)SPMIDI_ALLOC_MEM( sizeof(Stack_t), "Stack_t" );
    if( s != NULL )
    {
        MemTools_Clear( s, sizeof(Stack_t) );
        Stack_Clear( s );
    }
    return( s );
}

/**********************************************************************
 ** Stack_IsEmpty()
 * If the top of stack is negative, stack is empty so return TRUE;
 * otherwise, return FALSE.
 */
spmBoolean Stack_IsEmpty( Stack_t* stack )
{
    return( (spmBoolean)( stack->tos < 0 ) );
}

/**********************************************************************
 ** Stack_Push()
 * Push a value onto the stack.  Set error if overflow occurs.  In this
 * case, stack remains intact.
 *
 * value: to be pushed.
 */
void Stack_Push( Stack_t* stack, spmSInt32 value )
{
    stack->tos++;
    if( stack->tos >= SPMIDI_STACK_SIZE )
    {
        stack->err = TRUE;
        stack->tos = SPMIDI_STACK_SIZE - 1;
    }
    else
    {
        stack->base[stack->tos] = value;
    }
}

/**********************************************************************
 ** Stack_Pop()
 * Pop value from top of stack.  If stack is empty, set error code.
 */
spmSInt32 Stack_Pop( Stack_t* stack )
{
    spmSInt32 value; 
    
    if( Stack_IsEmpty( stack ) )
    {
        stack->err = TRUE;
        value = stack->base[0];
    }
    else
    {
        value = stack->base[stack->tos--];
    }

    return( value );
}

/**********************************************************************
 ** Stack_Delete()
 * Free stack memory.
 */
void Stack_Delete( Stack_t* stack )
{
    if( stack != NULL )
    {
        SPMIDI_FreeMemory( stack );
    }
}

/**********************************************************************
 ** Tests
 */

#if 0
int main( void )
{
    SPMIDI_Error errCode;
    Stack_t *myStack = NULL;
    spmSInt32 value;
    int i;

    myStack = Stack_Create();
    if( myStack == NULL ) goto error;

    /* Simple push and pop */
    Stack_Push( myStack, 200 );
    if( Stack_Error( myStack ) < 0 ) goto error;

    value = Stack_Pop( myStack );
    if( Stack_Error( myStack ) < 0 ) goto error;
    if( value != 200 ) goto error;
    if( !Stack_IsEmpty( myStack ) ) goto error;

    /* Check overflow generates error */
    for( i = 0; i < SPMIDI_STACK_SIZE + 2; i++ )
    {
        Stack_Push( myStack, i );
        if( ( Stack_Error( myStack ) < 0 ) && ( i < SPMIDI_STACK_SIZE ) ) goto error;
        if( ( Stack_Error( myStack ) >= 0 ) && ( i >= SPMIDI_STACK_SIZE ) ) goto error;
    }

    /* Check Stack_Clear(), Stack_IsEmpty() */
    Stack_Clear( myStack );
    for( i = 0; i < SPMIDI_STACK_SIZE; i++ )
    {
        Stack_Push( myStack, i );
        if( Stack_Error( myStack ) < 0 ) goto error;
    }

    for( i = 0; i < 5; i++ )
    {
        value = Stack_Pop( myStack );
        if( Stack_Error( myStack ) < 0 ) goto error;
    }

    Stack_Clear( myStack );
    if( !Stack_IsEmpty( myStack ) ) goto error;

    /* Check underflow generates error */
    Stack_Pop( myStack );
    if( Stack_Error( myStack ) >= 0 ) goto error;

    Stack_Clear( myStack );

error:
    errCode = Stack_Error( myStack );
    if( errCode < 0 )
    {
        DBUGMSG( "Stack Error" );
    }

    Stack_Delete( myStack );
}
#endif /* tests */
