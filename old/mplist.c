#include <stdlib.h>

#include "mplist.h"

MpList *mplist_create(MemPool *pool)
{
    MpList *list = NULL;

    list = mem_pool_alloc(pool, MpListSize
#if MEM_POOL_DEBUG
        , "mplist_create"
#endif
    );
    if(!list) 
        return NULL;

    list->head = NULL;
    list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;

    return list;
}

void mplist_destroy(MemPool *pool, MpList *list)
{
    MpListNode *current = list->head, *next = NULL;
    uint32_t len = list->len;

    while(len--) {
        next = current->next;
        if(list->free) 
            list->free(current->value);
        mem_pool_free(pool, current);
        current = next;
    }
    mem_pool_free(pool, list);
}

MpList *mplist_add_node_head(MemPool *pool, MpList *list, void *value)
{
    MpListNode *node = NULL;

    node = mem_pool_alloc(pool, MpListNodeSize
#if MEM_POOL_DEBUG
        , "mplist_add_node_head"
#endif
    );
    if(!node) return NULL;

    node->value = value;
    if(0 == list->len) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    }
    else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    ++list->len;
    return list;
}


MpList *mplist_add_node_tail(MemPool *pool, MpList *list, void *value)
{
    MpListNode *node = NULL;

    node = mem_pool_alloc(pool, MpListNodeSize
#if MEM_POOL_DEBUG
        , "mplist_add_node_tail"
#endif
    );
    if(!node) return NULL;

    node->value = value;
    if(0 == list->len) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    }
    else {
        node->next = NULL;
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    ++list->len;
    return list;
}
