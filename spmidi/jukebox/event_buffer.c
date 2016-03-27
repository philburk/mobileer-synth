/*
** evntbufr.c - buffer events for later execution
**
** Designed for use with HMSL based on pForth, or with JSyn
**
** The buffer combines a circular buffer of linked list slots
** that are hashed by time.
** Nodes are added to each slot using a time based insertion
** from the end of the list.
** Nodes are removed from each slot if their time matches
** the currently executing time.
**
** Nodes will be drawn from a pool of prelinked nodes.
**
** Author: Phil Burk
** Copyright 1997 -  Phil Burk
** All Rights Reserved
*/

#include "spmidi/include/spmidi_print.h"
#include "spmidi/jukebox/event_buffer.h"

#define TimeGreaterThan(a,b) (((int32)((a)-(b))) > 0)
#define TimeGreaterThanOrEqual(a,b) (((int32)((a)-(b))) >= 0)
/* Use simple hashing function to spread nodes between many slots. */
#define TimeToSlot(t)  ( ((t)^((t)>>EB_SLOT_SHIFT)) & EB_SLOT_MASK)

static void EB_InsertByTime( DoubleList *dll, EventBufferNode *newNode);
static void EB_ExecuteNextSlot( EventBuffer *ebuf );

#define DBUG(x)  /* { printf x; } */

/* Functions called by foreground. */
int32 EB_Init( EventBuffer *ebuf, uint32 time, void *context )
{
	int i;
	ebuf->ebuf_NextTime = time;
	ebuf->ebuf_UserContext = context;
	for( i=0; i<EB_NUM_SLOTS; i++ )
	{
		DLL_InitList( &ebuf->ebuf_Slots[i] );
	}
	return 0;
}

/***************************************************************
** Clean up event buffer.
*/
void EB_Term( EventBuffer *ebuf )
{
	EB_Clear( ebuf );
}

/***************************************************************
** Clear all pending events from event buffer.
*/
void EB_Clear( EventBuffer *ebuf )
{
	EventBufferNode *ebnd;
	DoubleList *dll;
	int   i;

	/* Free all pending nodes. */
	for( i=0; i<EB_NUM_SLOTS; i++ )
	{
		dll = &ebuf->ebuf_Slots[i];
		while( (ebnd = (EventBufferNode *) DLL_RemoveFirst(dll)) != NULL)
		{
			EB_FreeNode( ebuf->ebuf_UserContext, ebnd );
		}
	}
}

/***************************************************************
** Search list from tail and add after same or earlier node.
*/
static void EB_InsertByTime( DoubleList *dll, EventBufferNode *newNode)
{
	EventBufferNode *ebnd;
	uint32 newTime = newNode->ebnd_Time;

	ebnd = (EventBufferNode *) DLL_Last( dll );

	while( !DLL_IsEnd( dll, &ebnd->ebnd_Node ) && TimeGreaterThan(ebnd->ebnd_Time, newTime) )
	{
		ebnd = (EventBufferNode *) DLL_Previous( &ebnd->ebnd_Node );
	}
	DLL_InsertAfter( &ebnd->ebnd_Node, &newNode->ebnd_Node );
}

/***************************************************************
** 
*/
void EB_ScheduleNode( EventBuffer *ebuf, uint32 time, EventBufferNode *ebnd )
{
	uint32 slotIndex;
	/* If node scheduled in the past, then put in next slot. */
	ebnd->ebnd_Time = ( TimeGreaterThan( ebuf->ebuf_NextTime, time ) ) ? ebuf->ebuf_NextTime : time;
	slotIndex = TimeToSlot( ebnd->ebnd_Time );
	DBUG(("EB_ScheduleNode: time = %d, nodeTime = %d, node = 0x%x\n", time, ebnd->ebnd_Time, ebnd));
	EB_InsertByTime( &ebuf->ebuf_Slots[slotIndex], ebnd );
}

/***************************************************************
** 
*/
static void EB_ExecuteNextSlot( EventBuffer *ebuf )
{
	EventBufferNode *ebnd;
	uint32 slotIndex;
	DoubleList *dll;
	/* Remove nodes from list and process them. */
	slotIndex = TimeToSlot( ebuf->ebuf_NextTime );
	dll = &ebuf->ebuf_Slots[slotIndex];
	ebnd = (EventBufferNode *) DLL_First( dll );

	while( !DLL_IsEnd( dll, &ebnd->ebnd_Node ) && (ebnd->ebnd_Time == ebuf->ebuf_NextTime) )
	{
		DLL_Remove( &ebnd->ebnd_Node );
		DBUG(("EB_ExecuteNextSlot: NextTime = %d, node = 0x%x\n", ebuf->ebuf_NextTime, ebnd));
		EB_ProcessNode( ebuf->ebuf_UserContext, ebnd );
		ebnd = (EventBufferNode *) DLL_First( dll );
	}
	ebuf->ebuf_NextTime++;
}

/***************************************************************
**
*/
void EB_ExecuteNodes( EventBuffer *ebuf, uint32 time )
{
	while( 	TimeGreaterThanOrEqual( time, ebuf->ebuf_NextTime ) )
	{
		EB_ExecuteNextSlot( ebuf );
	}
}
