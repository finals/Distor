#include "list.h"

uint8_t list_init(list_t *list, MemPool *mem_pool)
{
    list_node_t *head_node = NULL;

    head_node = (list_node_t *)mem_pool_alloc(mem_pool, list_node_size
#if MEM_POOL_DEBUG
        , __FUNCTION__
#endif
    );
    if(!head_node) return LIST_ERR;

    head_node->next = NULL;
    list->head = list->tail = head_node;
    list->length = 0;
    splock_init(&list->head_lock);
    splock_init(&list->tail_lock);

    return LIST_OK;
}

void list_tail_push_node(list_t *list, list_node_t *node)
{
    splock_lock(&list->tail_lock);
    list->tail->next = node;
    list->tail = node;
    node->next = NULL;
    ++list->length;
    splock_unlock(&list->tail_lock);
}

list_node_t *list_head_pop_node(list_t *list)
{
    list_node_t *node = NULL;
    if(splock_try_lock(&list->head_lock)) {
        return NULL;
    }
 
    node = list->head->next;
    if(node) {
        list->head = node;
    }  
    
    --list->length;
    splock_unlock(&list->head_lock);
    return node;
}

list_node_t *list_head_pop_node_blocking(list_t *list)
{
    list_node_t *node = NULL;

    splock_lock(&list->head_lock);

    node = list->head->next;
    if(node) {
        list->head = node;
    }  

    --list->length;
    splock_unlock(&list->head_lock);
    return node;
}

