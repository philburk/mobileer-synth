#ifndef _RESOURCE_MGR_H
#define _RESOURCE_MGR_H

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

#ifdef __cplusplus
extern "C"
{
#endif

#define RESOURCE_UNDEFINED_ID   (0)

    typedef spmUInt32 spmResourceToken; // TODO can this be an spmSInt32 instead so we can pass negative errors?

    /** Used to map external IDs to internal resource tokens. */
    typedef struct ResourceTokenMap_s
    {
        /** Internal token allocated by WaveManager. */
        spmResourceToken    token;
        /** Set true if we need to load this resource to play the chosen songs. */
        spmUInt8            needed;
    } ResourceTokenMap_t;

    /** Used to keep track of resources in linked lists.
     * Resource is identified by a token.
     * References are counted to prevent deletion of
     * resources still in use.
     */
    typedef struct ResourceTracker_s
    {
        DoubleNode      node;
        int             referenceCount;
        spmResourceToken   token;
    }
    ResourceTracker_t;

    void ResourceMgr_InitResource( ResourceTracker_t *resource );

    /** Add in sorted order by token.
     * Token must not be zero because that is RESOURCE_UNDEFINED_ID
     */
    SPMIDI_Error ResourceMgr_Add( DoubleList *list, ResourceTracker_t *resource, spmResourceToken token );

    /** Find in list, assuming in sorted order.
     * @return resource or NULL if not found.
     */
    ResourceTracker_t *ResourceMgr_Find( DoubleList *list, spmResourceToken token );

    /** Remove from list. */
    SPMIDI_Error ResourceMgr_Remove( ResourceTracker_t *resource );


#ifdef __cplusplus
}
#endif

#endif /* _RESOURCE_MGR_H */

