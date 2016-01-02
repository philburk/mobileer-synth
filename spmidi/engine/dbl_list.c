/* $Id: dbl_list.c,v 1.6 2007/10/02 16:14:42 philjmsl Exp $ */
/**
 *
 * Doubly Linked List
 *
 * Author: Phil Burk
 * Copyright 1996 Phil Burk and Mobileer
 * All Rights Reserved.
 * PROPRIETARY and CONFIDENTIAL
 */

#include "spmidi/engine/dbl_list.h"

#define PRINT(x)  /* { printf x; fflush(stdout); }; */

static void DLL_Connect( DoubleNode *dln1, DoubleNode *dln2);

#define STOP_DEBUG  { *((int *)NULL) = 1; }  /* Trigger a CPU exception. */

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifdef PARANOID
/* Paranoid versions of DL link routines that check list sanity before
 * doing anything dangerous. These are used for debugging because
 * linked lists are a common potential point of failure.
 */
int DLL_CheckNodeInList( DoubleNode *dln )
{
    /* Check to see whether node is connected to neighbors. */
    if( (DLL_Next(DLL_Previous(dln)) != dln) |
            (DLL_Previous(DLL_Next(dln)) != dln) )
    {
        PRINT(("DLL node not properly in list! 0x%x\n", dln));
        return 1;
    }
    else
    {
        return 0;
    }
}
#define CHECK_NODE(n) { if( DLL_CheckNodeInList(n) ) STOP_DEBUG; }

int DLL_CheckNodeInit( DoubleNode *dln )
{
    /* Check to see whether node is in initial state. */
    if( (DLL_Next(dln) != dln) |
            (DLL_Previous(dln) != dln) )
    {
        PRINT(("DLL node not initialized! 0x%x\n", dln));
        return 1;
    }
    else
    {
        return 0;
    }
}
#define CHECK_NODE_INIT(n) { if( DLL_CheckNodeInit(n) ) STOP_DEBUG; }

int DLL_CheckPreviousLink( DoubleNode *dlnPrev, DoubleNode *dln )
{
    /* Check to see whether node is connected to previous. */
    if( (DLL_Previous(dln) != dlnPrev) )
    {
        PRINT(("DLL_CheckPreviousNode: list broken! 0x%x 0x%x\n", dlnPrev, dln ));
        return 1;
    }
    else
    {
        return 0;
    }
}
#define CHECK_PREVIOUS_LINK(n,p) { if( DLL_CheckPreviousLink(n,p) ) STOP_DEBUG; }

int DLL_CheckNextLink( DoubleNode *dln, DoubleNode *dlnNext )
{
    /* Check to see whether node is connected to neighbors. */
    if( (DLL_Next(dln) != dlnNext) )
    {
        PRINT(("DLL_CheckNextLink: list broken! 0x%x 0x%x\n", dln, dlnNext));
        return 1;
    }
    else
    {
        return 0;
    }
}
#define CHECK_NEXT_LINK(n,nx) { if( DLL_CheckNextLink(n,nx) ) STOP_DEBUG; }
#else
/* Disable checks. */
#define CHECK_NODE(n)
#define CHECK_NODE_INIT(n)
#define CHECK_PREVIOUS_LINK(n,p)
#define CHECK_NEXT_LINK(n,nx)
#endif

/* connect dn2 after dn1 */
static void DLL_Connect( DoubleNode *dln1, DoubleNode *dln2)
{
    (dln1)->dln_Next = dln2;
    (dln2)->dln_Previous = dln1;
}

void DLL_InsertAfter( DoubleNode *dln, DoubleNode *dlnNew )
{
    DoubleNode *dlnTemp = DLL_Next(dln);
    CHECK_NODE_INIT(dlnNew);
    CHECK_PREVIOUS_LINK(dln,dlnTemp);
    /* Connect new one first so other tasks that only use next can still read safely. */
    DLL_Connect( dlnNew, dlnTemp );
    DLL_Connect( dln, dlnNew );
    CHECK_NODE(dlnNew);
}

void DLL_InsertBefore( DoubleNode *dln, DoubleNode *dlnNew )
{
    DoubleNode *dlnTemp = DLL_Previous(dln);
    CHECK_NODE_INIT(dlnNew);
    CHECK_NEXT_LINK(dlnTemp,dln);
    DLL_Connect( dlnNew, dln );
    DLL_Connect( dlnTemp, dlnNew );
    CHECK_NODE(dlnNew);
}

void DLL_Remove( DoubleNode *dln )
{
    CHECK_NODE(dln);
    DLL_Connect( DLL_Previous(dln), DLL_Next(dln) );
    DLL_InitNode(dln);
}

/* Remove first element or return NULL if empty. */
DoubleNode *DLL_RemoveFirst( DoubleList *dll )
{
    DoubleNode *dln;
    if( !((dln = dll->dll_First) == (DoubleNode *) dll))
    {
        DLL_Remove( dln );
    }
    else
    {
        dln = NULL;
    }
    return dln;
}

void DLL_InitNode( DoubleNode *dln )
{
    dln->dln_Next = dln;
    dln->dln_Previous = dln;
}

/* Point it to itself in a degenerate loop. */
void DLL_InitList( DoubleList *dll )
{
    DLL_InitNode( (DoubleNode *) dll );
}

int DLL_IsEmpty( DoubleList *dll )
{
    return ( dll->dll_First == (DoubleNode *) dll );
}

/*******************************************************
 * Count Nodes in list.
 */
int DLL_CountNodes( DoubleList *dll )
{
    DoubleNode *dln;
    int count = 0;
    dln = DLL_First( dll );
    while( !DLL_IsEnd( dll, dln ) )
    {
        count++;
        dln = DLL_Next( dln );
    }
    return count;
}
