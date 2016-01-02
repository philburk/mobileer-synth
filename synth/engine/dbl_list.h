#ifndef DBL_LIST_H
#define DBL_LIST_H
/* $Id: dbl_list.h,v 1.7 2005/05/03 22:04:00 philjmsl Exp $ */
/**
 *
 * @brief Doubly Linked List support.
 *
 * @author Phil Burk, Copyright 2000 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

typedef struct DoubleNode
{
    struct DoubleNode      *dln_Next;
    struct DoubleNode      *dln_Previous;
}
DoubleNode;

typedef struct DoubleList
{
    struct DoubleNode      *dll_First;
    struct DoubleNode      *dll_Last;
}
DoubleList;


#define DLL_Next(dln) ((dln)->dln_Next)
#define DLL_Previous(dln) ((dln)->dln_Previous)
#define DLL_AddHead( dll ,dln ) DLL_InsertAfter( (DoubleNode *)(dll), (dln) )
#define DLL_AddTail( dll, dln ) DLL_InsertBefore( (DoubleNode *)(dll), (dln) )
#define DLL_First( dll ) ((dll)->dll_First)
#define DLL_Last( dll ) ((dll)->dll_Last)
#define DLL_IsEnd( dll, dln ) ((dll) == (DoubleList *) (dln))

/* Scan all nodes in a list. */
#define DLL_FOR_ALL( type, node, list ) \
        for( node = (type *) DLL_First(list); \
            !DLL_IsEnd( list, (DoubleNode *) node ); \
            node = (type *) DLL_Next( (DoubleNode *) node ) )


#ifdef __cplusplus
extern "C"
{
#endif

    void DLL_InsertAfter( DoubleNode *dln, DoubleNode *dlnNew );
    void DLL_InsertBefore( DoubleNode *dln, DoubleNode *dlnNew );
    void DLL_Remove( DoubleNode *dln );
    DoubleNode *DLL_RemoveFirst( DoubleList *dll );
    void DLL_InitNode( DoubleNode * dln );
    void DLL_InitList( DoubleList *dll );
    int  DLL_IsEmpty( DoubleList *dll );
    int  DLL_CountNodes( DoubleList *dll );
    int  DLL_GetError( void );

#ifdef __cplusplus
}
#endif

#endif /* DBL_LIST_H */
