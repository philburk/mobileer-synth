#ifndef  _EVENT_BUFFER_H
#define  _EVENT_BUFFER_H
/*
** Event Buffer
**
** Author: Phil Burk
** Copyright 1997 Phil Burk
** All Rights Reserved.
*/

#include "spmidi/engine/dbl_list.h"

#define EB_SLOT_SHIFT (8)
#define EB_NUM_SLOTS (1<<EB_SLOT_SHIFT)
#define EB_SLOT_MASK (EB_NUM_SLOTS-1)

#ifndef int32
typedef long int32;
#endif
#ifndef uint32
typedef unsigned long uint32;
#endif

typedef struct  EventBufferNode
{
	DoubleNode       ebnd_Node;
	uint32           ebnd_Time;   /* time to execute event */
}
EventBufferNode;

typedef struct EventBuffer
{
	uint32            ebuf_NextTime;      /* Time associated with next slot to be processed. */
	DoubleList        ebuf_Slots[EB_NUM_SLOTS];  /* Nodes hashed, ready for execution. */
	void             *ebuf_UserContext;   /* Gets passed to EB_ProcessNode() and EB_FreeNode(). */
}
EventBuffer;


#ifdef __cplusplus
extern "C"
{
#endif

	/* Functions called by foreground. */
	int32 EB_Init( EventBuffer *ebuf, uint32 time, void *context );
	void  EB_Term( EventBuffer *ebuf );
	void  EB_Clear( EventBuffer *ebuf );
	void  EB_ScheduleNode( EventBuffer *ebuf, uint32 time, EventBufferNode *ebnd );

	/* Functions called by background. */
	void EB_ExecuteNodes( EventBuffer *ebuf, uint32 time );

	/* Functions defined by client! */
	int32 EB_ProcessNode( void *context, EventBufferNode *ebnd );
	int32 EB_FreeNode( void *context, EventBufferNode *ebnd );

#ifdef __cplusplus
}
#endif


#endif /* _EVENT_BUFFER_H */
