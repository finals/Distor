#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include "splock.h"
#include "mem_pool.h"

#define LIST_OK   0
#define LIST_ERR  1

typedef struct list_node_s {
    struct list_node_s *next;
}list_node_t;
#define list_node_size (sizeof(list_node_t))

typedef struct list_s {
    list_node_t *head;
    list_node_t *tail;
    uint32_t length;
    splock_t head_lock;
    splock_t tail_lock;
    uint16_t spare0;
}list_t;
#define list_size (sizeof(list_t))

uint8_t list_init(list_t *list, MemPool *mem_pool);
void list_tail_push_node(list_t *list, list_node_t *node);
list_node_t *list_head_pop_node(list_t *list);
list_node_t *list_head_pop_node_blocking(list_t *list);

#endif /* __LIST_H__ */
