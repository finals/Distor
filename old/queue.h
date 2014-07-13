#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdint.h>
#include "splock.h"
#include "mem_pool.h"

#define QUEUE_OK   0
#define QUEUE_ERR  1

typedef struct queue_node_s {
    struct queue_node_s *next;
    struct queue_node_s *prev;
}queue_node_t;
#define queue_node_size (sizeof(queue_node_t))

typedef struct queue_s {
    queue_node_t *head;
    queue_node_t *tail;
    uint32_t length;
    uint16_t spare0;
    splock_t head_lock;
    splock_t tail_lock;
}queue_t;
#define queue_size (sizeof(queue_t)

uint8_t queue_init(queue_t *queue, MemPool *mem_pool);
void enqueue(queue_t *queue, queue_node_t *node); //入队列
queue_node_t *dequeue(queue_t *queue);   //出队列
queue_node_t *dequeue_blocking(queue_t *queue);  

#endif  /* __QUEUE_H__ */
