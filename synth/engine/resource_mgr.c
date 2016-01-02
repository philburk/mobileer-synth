/* $Id$ */
/**
 *
 * Generic Resource manager. Keep in linked lists.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi/engine/dbl_list.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_print.h"
#include "resource_mgr.h"

void ResourceMgr_InitResource( ResourceTracker_t *resource )
{
    DLL_InitNode( &resource->node );
    resource->token = RESOURCE_UNDEFINED_ID;
    resource->referenceCount = 0;
}

/*********************************************************************/
/** Add in sorted order by token. */
SPMIDI_Error ResourceMgr_Add( DoubleList *list, ResourceTracker_t *resource, spmResourceToken token )
{
    DoubleNode *node;

    resource->token = token;

    for( node = DLL_Last(list);
            !DLL_IsEnd( list, node );
            node = DLL_Previous( node ) )
    {
        /* This works because node is first thing in resource. */
        ResourceTracker_t *rsrcInList = (ResourceTracker_t *) node;
        if( rsrcInList->token == token )
            return SPMIDI_Error_BadToken;
        else if( rsrcInList->token < token )
        {
            DLL_InsertAfter( node, &resource->node );
            return SPMIDI_Error_None;
        }
    }
    /* If we get here then we did not add resource yet so it must be smallest ID yet.
     * So just add it to head. */
    DLL_AddHead( list, &resource->node );
    return SPMIDI_Error_None;
}

/*********************************************************************/
/** Find in list, assuming in sorted order. */
ResourceTracker_t *ResourceMgr_Find( DoubleList *list, spmResourceToken token )
{
    ResourceTracker_t *resource;
    DLL_FOR_ALL( ResourceTracker_t, resource, list  )
    {
        if( resource->token == token )
        {
            return resource;
        }
        else if( resource->token > token )
        {
            /* We have gone past what we are looking for. It must not be here. */
            return NULL;
        }
    }
    return NULL;
}

/*********************************************************************/
/** Remove from list. */
SPMIDI_Error ResourceMgr_Remove( ResourceTracker_t *resource )
{
    DLL_Remove( &resource->node );
    resource->token = RESOURCE_UNDEFINED_ID;
    return SPMIDI_Error_None;
}


