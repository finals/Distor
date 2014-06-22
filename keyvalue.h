#ifndef __KEYVALUE_H__
#define __KEYVALUE_H__

#include <stdint.h>
#include <time.h>

#include "rbtree.h"
#include "mem_pool.h"

#define KEY_VALUE_SIZE 32   //hashÉ¢ÁÐ

typedef struct key_value_node_s {
    rbtree_node_t node;
    time_t expire;
    uint32_t len;
    void *data;
}key_value_node_t;
#define KeyValueNodeSize (sizeof(key_value_node_t))

typedef struct key_value_s {
    rbtree_t key_value_table[KEY_VALUE_SIZE];
    uint32_t count;
    uint32_t used_memory;
    uint32_t max_memory;
    MemPool *pool;
}key_value_t;
#define KeyValueSize (sizeof(key_value_t))

#define key_value_node_get_len(node) ((node)->len)
#define key_value_node_get_data(node) ((node)->data)
#define key_value_node_get_expire(node) ((node)->expire)

#define key_value_get_count(kv) ((kv)->count)
#define key_value_get_used_memory(kv) ((kv)->used_memory)

key_value_t *key_value_init(MemPool *pool, uint32_t max_memory);
void rbtree_destroy(MemPool *pool, volatile rbtree_t *tree);
void key_value_destroy(MemPool *pool, key_value_t *kv);

/* ²Ù×÷key_value×´Ì¬ */
#define KV_FAILED      -1
#define KV_SUCCESS      0
#define KV_NO_MEMORY    1
#define KV_KEY_INVALID  2
#define KV_VAL_INVALID  4
#define KV_ALLOC_FAILED 8

int8_t key_value_set(MemPool *pool, key_value_t *kv, void *key, int32_t key_len,
    void *val, int32_t val_len, uint32_t expire_after);
key_value_node_t * key_value_get(key_value_t *kv, void *key, int32_t key_len);

#endif  /* __KEYVALUE_H__ */