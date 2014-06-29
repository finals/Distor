#include "queue.h"

uint8_t queue_init(queue_t *queue, MemPool *mem_pool)
{
    queue_node_t *head_node = NULL;

    head_node = (queue_node_t *)mem_pool_alloc(mem_pool, queue_node_size
#if MEM_POOL_DEBUG
        , __FUNCTION__
#endif
    );
    if(!head_node) return QUEUE_ERR;

    head_node->next = head_node->prev = NULL;
    queue->length = 0;
    queue->head = queue->tail = head_node;
    splock_init(&queue->head_lock);
    splock_init(&queue->tail_lock);

    return QUEUE_OK;
}

void enqueue(queue_t *queue, queue_node_t *node)
{
    splock_lock(&queue->tail_lock);
    node->prev = queue->tail;
    node->next = NULL;
    queue->tail->next = node;
    queue->tail = node;
    ++queue->length;
    splock_unlock(&queue->tail_lock);
}

queue_node_t *dequeue(queue_t *queue)
{
    queue_node_t *node = NULL;

    if(splock_try_lock(&queue->head_lock)) {
        return NULL;  //ËøÊ§°Ü£¬·µ»ØNULL
    }

    node = queue->head->next;
    if(node) {
        queue->head = node;
    }

    --queue->length;
    splock_unlock(&queue->head_lock);
    return node;
}

queue_node_t *dequeue_blocking(queue_t *queue)
{
    queue_node_t *node = NULL;

    splock_lock(&queue->head_lock);

    node = queue->head->next;
    if(node) {
        queue->head = node;
    }

    --queue->length;
    splock_unlock(&queue->head_lock);
    return node;
}