#include <string.h>
#include <stdio.h>
#include "keyvalue.h"
#include "utils.h"

void *key_value_rbtree_insert_value(rbtree_node_t *root, rbtree_node_t *node, 
    rbtree_node_t *sentinel)
{
    rbtree_node_t *temp = root;
    key_value_node_t *n, *t;
    rbtree_node_t **p;
    void *old_data = NULL;

    n = (key_value_node_t *)node;
    for( ; ; ) {
    
        t = (key_value_node_t *)temp;
    
        if(node->key != temp->key) {
            p = (node->key > temp->key) ? &temp->right : &temp->left;
        }
        else { //有重复key，修改原来的
            t->len = n->len;
            old_data = t->data;
            t->data = n->data;
            t->expire = n->expire;
            return old_data;
        }

        if(*p == sentinel) 
            break;

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;

    rbt_red(node);

    return NULL;

}

key_value_t *key_value_init(MemPool *pool, uint32_t max_memory)
{
    key_value_t *kv = NULL;
    rbtree_node_t *sentinel = NULL;
    uint32_t i = 0;

    kv = mem_pool_alloc(pool, KeyValueSize
#if MEM_POOL_DEBUG
        , "key_value_init"
#endif
    );
    if(!kv) {
        goto err;
    }   
    sentinel = mem_pool_alloc(pool, RbtreeNodeSize
#if MEM_POOL_DEBUG
        , "key_value_init sentinel" 
#endif
    );
    if(!sentinel) {
        goto err;
    }

    kv->count = 0;
    kv->used_memory = 0;
    kv->max_memory = max_memory << 20;
    kv->pool = pool;

    for(i = 0; i < KEY_VALUE_SIZE; ++i) {
        rbtree_init(kv->key_value_table + i, sentinel, 
            key_value_rbtree_insert_value);
    }
    
    return kv;

err:
    if(sentinel) {
        mem_pool_free(pool, sentinel);
    }
    if(kv) {
        mem_pool_free(pool, kv);
    }
    return NULL;
}

void rbtree_destroy(MemPool *pool, volatile rbtree_t *tree)
{
    rbtree_node_t *temp = tree->root, *parent = NULL;

    while(temp) {
        while(temp->left != tree->sentinel || temp->right != tree->sentinel) {
            if(temp->left != tree->sentinel) {
                temp = temp->left;
                if(!temp) break;
                continue;
            }
            if(temp->right != tree->sentinel) {
                temp = temp->right;
                if(!temp) break;
                continue;
            }

            break;
        }

        if(!temp) break;
        
        parent = temp->parent;  //parent为NULL表示是根节点
        if(parent) {
            if(parent->left == temp) 
                parent->left = tree->sentinel;
            else
                parent->right = tree->sentinel;
        }

        mem_pool_free(pool, ((key_value_node_t *)temp)->data);
        mem_pool_free(pool, temp);
        temp = parent;
        
    }
    
}

void key_value_destroy(MemPool *pool, key_value_t *kv)
{
    rbtree_node_t *sentinel = NULL;
    rbtree_t *tree = NULL;
    uint32_t i = 0;

    if(!kv) return;

    sentinel = kv->key_value_table[0].sentinel;

    for(i = 0; i < KEY_VALUE_SIZE; ++i) {
        tree = kv->key_value_table + i;
        rbtree_destroy(pool, tree);
    }

    mem_pool_free(pool, sentinel);
    mem_pool_free(pool, kv);

}

static uint32_t key_value_get_pos_by_key(void *key, int32_t key_len, 
    rbtree_key_t *int_key)
{
     uint32_t pos = 0;
     
     *int_key = generate_hash(key, key_len);
     pos = *int_key % KEY_VALUE_SIZE;
     return pos;
}

int8_t key_value_set(MemPool *pool, key_value_t *kv, void *key, int32_t key_len,
    void *val, int32_t val_len, uint32_t expire_after)
{
    key_value_node_t *node = NULL;
    uint32_t pos = 0;
    rbtree_key_t int_key = 0;
    time_t now;
    int8_t ret = KV_SUCCESS;
    void *old_data = NULL;
    
    if(kv->used_memory + val_len > kv->max_memory)
        return KV_NO_MEMORY;

    if(!key || key_len <= 0)
        return KV_KEY_INVALID;

    if(!val || val_len <= 0)
        return KV_VAL_INVALID;

    if(expire_after < 0)
        expire_after = 0;

    pos = key_value_get_pos_by_key(key, key_len, &int_key);
    node = (key_value_node_t *)mem_pool_alloc(pool, KeyValueNodeSize
#if MEM_POOL_DEBUG
        , "key_value_set"
#endif
    );
    if(!node) {
        ret = KV_ALLOC_FAILED;
        goto err;
    }
    node->node.key = int_key;
    node->data = (key_value_node_t *)mem_pool_alloc(pool, val_len
#if MEM_POOL_DEBUG
        , "key_value_set"
#endif
    ); 
    if(!node->data) {
        goto err;
    }
    strncpy(node->data, val, val_len);
    node->len = val_len;
    time(&now);
    node->expire = now + expire_after;
    node->node.parent = NULL;
    old_data = rbtree_insert(&(kv->key_value_table[pos]), (rbtree_node_t *)node);
    kv->used_memory += mem_chunk_get_data_size(node->data);
    if(old_data) {  //找到相同的key
        kv->used_memory -= mem_chunk_get_data_size(old_data);
        mem_pool_free(pool, node);
        mem_pool_free(pool, old_data);  
    }
    else {
        ++kv->count;
    }
    
    return KV_SUCCESS;
    
err:
    if(node) {
        if(node->data)
            mem_pool_free(pool, node->data);
        mem_pool_free(pool, node);
    }
    return KV_ALLOC_FAILED;
}

key_value_node_t * key_value_get(key_value_t *kv, void *key, int32_t key_len)
{
    key_value_node_t *n;
    rbtree_node_t *node, *sentinel;
    rbtree_t *tree = NULL;
    rbtree_key_t int_key;
    uint32_t pos = -1;

    if(!key || key_len <= 0)
        return NULL;

    pos = key_value_get_pos_by_key(key, key_len, &int_key);
    tree = kv->key_value_table + pos;

    node = tree->root;
    sentinel = tree->sentinel;

    while(node != sentinel) {
        n = (key_value_node_t *)node;

        if(int_key != node->key) {
            node = (int_key > node->key) ? node->right : node->left;
            continue;
        }

        return n;
    }

    return NULL;
}

