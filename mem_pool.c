#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "mem_pool.h"

inline MemChunk *mem_chunk_create(uint32_t size, uint32_t set_pos)
{
    MemChunk *chunk = NULL;
    
    chunk = (MemChunk *)malloc(MemChunkSize + size);
    if(chunk) {
        chunk->next = NULL;
        chunk->prev = NULL;
        chunk->size = size;
        chunk->set_pos = set_pos;
    }

    return chunk;
}

inline void mem_chunk_destroy(MemChunk *chunk)
{
    free(chunk);
}

void mem_set_destroy(MemSet *set)
{
    MemChunk *chunk = NULL, *next = NULL;

    if(set) {
        chunk = set->chunk_head;
        while(chunk) {
            next = chunk->next;
            mem_chunk_destroy(chunk);
            chunk = next;
        }
        /*
        if(set->sentinel) {
            free(set);
        }
        */
        free(set);
    }
}

MemSet *mem_set_create(uint32_t size, uint32_t chunk_count, uint32_t pos)
{
    MemSet *set = NULL;
    MemChunk * chunk = NULL;
    uint32_t i = 0;

    set = (MemSet *)malloc(MemSetSize);
    if(!set) {
        goto err;
    }
    set->chunk_head = NULL;
    set->chunk_tail = NULL;

    for(i = 0; i < chunk_count; ++i) {
        chunk = mem_chunk_create(size, pos);
        if(!chunk) {
            goto err;
        }
        //链表尾插入
        if(!set->chunk_head) {
            chunk->prev = NULL;
            set->chunk_head = chunk;
        }
        else {
            set->chunk_tail->next = chunk;
            chunk->prev = set->chunk_tail;
        }
        set->chunk_tail = chunk;
    }

    set->chunk_count = chunk_count;
    set->chunk_size = size;
    splock_init(&set->lock);

    return set;

err:
    mem_set_destroy(set);
    return NULL;
    
}

#if MEM_POOL_DEBUG
static void mem_pool_register_debug(MemPool *pool, MemChunk *chunk, 
    const char  * module)
{

    uint32_t i = 0;
    char  *msg = "[module]%s [chunk]%p [size]%u";

    for(i = 0; i < DEBUG_SIZE; ++i) {
        if(!pool->debug_table[i].alloc_chunk) {
            splock_lock(&pool->debug_table[i].lock);
            if(!pool->debug_table[i].alloc_chunk) {
                splock_unlock(&pool->debug_table[i].lock);
                continue;
            }
            pool->debug_table[i].alloc_chunk = chunk;
            snprintf(pool->debug_table[i].message, MESSAGE_SIZE, msg, 
                    module, chunk, chunk->size);
            splock_unlock(&pool->debug_table[i].lock);
            return;
        }
    }
    
}

static void mem_pool_unregister_debug(MemPool *pool, MemChunk *chunk)
{
    uint32_t i = 0;

    for(i = 0; i < DEBUG_SIZE; ++i) {
        if(pool->debug_table[i].alloc_chunk == chunk) {
            pool->debug_table[i].alloc_chunk = NULL;
            return;
        }
    }
}

static void mem_pool_output_debug(MemPool *pool)
{
    uint32_t i = 0;

    printf("Memory Pool Debug statistics:\n");
    for(i = 0; i < DEBUG_SIZE; ++i) {
        if(pool->debug_table[i].alloc_chunk) {
            printf("pos %5u: alloc: %p %s\n", i, pool->debug_table[i].alloc_chunk
                ,pool->debug_table[i].message);
        }
    }
}
#endif

void mem_pool_destroy(MemPool *pool)
{
    uint32_t i = 0;
    
    if(pool) {

        for(i = 0; i < SETSIZE; ++i) {
            mem_set_destroy(pool->set_table[i]);
        }
#if MEM_POOL_DEBUG
        mem_pool_output_debug(pool); 
#endif

        free(pool);
    }
}

MemPool *mem_pool_create(uint32_t max_size)
{
    uint32_t i = 0, chunk_size = 0, chunk_count = 0;
    MemPool *pool = NULL;

    pool = (MemPool *)malloc(MemPoolSize);
    if(!pool) {
        goto err;  
    }
    pool->max_size = max_size * 1024 * 1024;
    pool->used_size = 0;

    for(i = 0, chunk_size = TINY_MIN_SIZE; i < SET_BOUNDARY; ++i) {
        chunk_count = SET_INIT_MAX_SIZE / chunk_size;
        pool->set_table[i] = mem_set_create(chunk_size, chunk_count, i);
        if(!pool->set_table[i]) {
            goto err;
        }
        pool->used_size += chunk_size * chunk_count;
        chunk_size += TINY_STEP; 
    }

    for(i = SET_BOUNDARY, chunk_size = MEDIUM_MIN_SIZE; i < SETSIZE; ++i) {
        chunk_count = SET_INIT_MAX_SIZE / chunk_size;
        pool->set_table[i] = mem_set_create(chunk_size, chunk_count, i);
        if(!pool->set_table[i]) {
            goto err;
        }
        pool->used_size += chunk_size * chunk_count;
        chunk_size += MEDIUM_STEP; 
    }

#if MEM_POOL_DEBUG
    for(i = 0; i < DEBUG_SIZE; ++i) {
        pool->debug_table[i].alloc_chunk = NULL;
        splock_init(&pool->debug_table[i].lock);
    }

#endif

    return pool;

err:
    mem_pool_destroy(pool);
    return NULL;
}

int32_t mem_pool_binary_search_set_by_size(MemPool *pool, uint32_t size)
{
    int32_t i = 0, j = 0, mid = 0;
    uint32_t chunk_size = 0, step = TINY_STEP, low, high;

    if(size > MEDIUM_MAX_SIZE) return OUTSET;

    if(size <= TINY_MAX_SIZE) {
        low = 0;
        high = SET_BOUNDARY - 1;
        step = TINY_STEP;
    }
    else {
        low = SET_BOUNDARY;
        high = SETSIZE - 1;
        step = MEDIUM_STEP;
    }

    //二分搜索，low和high差距为64，最多搜索6次，加上前面的if else 1次，共7次
    for(i = low, j = high; i <= j; ) {
        mid = (i + j) >> 1;
        chunk_size = pool->set_table[mid]->chunk_size;
        if(size <= chunk_size && size > chunk_size - step) {
            return mid;
        }
        if(chunk_size > size) {
            j = mid - 1;
        }
        else {
            i = mid + 1;
        }   
    }
    return i;
}

void *mem_pool_alloc(MemPool *pool, uint32_t size
#if MEM_POOL_DEBUG
    , const char  *module
#endif
)
{
    uint32_t align_size = size;
    int32_t pos = OUTSET;
    MemChunk *palloc = NULL;
    MemSet *set = NULL;

    if(pool->used_size + size > pool->max_size || 0 >= size) {
        goto fail; 
    }

    if(size <= MEDIUM_MAX_SIZE) {
        pos = mem_pool_get_set_by_size(size);
        set = pool->set_table[pos];
        align_size = set->chunk_size;

        splock_lock(&set->lock);
        palloc = set->chunk_head;
        //chunk_head指向下一个元素，如果它指向NULL，表示栈满
        if(palloc) {
            set->chunk_head = palloc->next;
            if(!palloc->next) { //该节点是链表中最后一个节点，更新tail指针
                set->chunk_tail = NULL;
            }
        }
        splock_unlock(&set->lock);
    }
    
    if(!palloc) {
        palloc = mem_chunk_create(align_size, OUTSET);
        if(palloc) {
            __sync_fetch_and_add(&pool->used_size, palloc->size);
            goto success;
        }
        else {
            goto fail;     
        }
    }

success:
#if MEM_POOL_DEBUG
    mem_pool_register_debug(pool, palloc, module);
#endif    
    return mem_chunk_get_data(palloc);

fail:
    return NULL;
}

void mem_pool_free(MemPool *pool, void *p)
{
    MemChunk *chunk = NULL;
    MemSet *set = NULL;
    int32_t pos = OUTSET;

    if(!p) return;

    chunk = mem_chunk_get_chunk(p);
    
    if(chunk->size <= MEDIUM_MAX_SIZE) {
        //将chunk加入链表，分两种情况，原来是链表的元素和新malloc的chunk
        if(chunk->set_pos > OUTSET) {
            set = pool->set_table[chunk->set_pos];
        }
        else {  
            //pos = mem_pool_binary_search_set_by_size(pool, chunk->size); 
            pos = mem_pool_get_set_by_align_size(chunk->size);
            set = pool->set_table[pos];
            //if(set->chunk_count * set->chunk_size >= SET_INIT_MAX_SIZE) {
            if(pool->used_size > pool->max_size * FREE_BOUNDARY) {
                goto overflow;
            }
            chunk->set_pos = pos;   
            __sync_fetch_and_add(&set->chunk_count, 1);  //set增加一个chunk
        }
        
        splock_lock(&set->lock);
        if(!set->chunk_head) {   //当前chunk链表为空
            //chunk->prev = NULL;
            set->chunk_head = chunk;
            set->chunk_tail = chunk;
        }
        else {
            set->chunk_tail->next = chunk;
            chunk->prev = set->chunk_tail;
        }
        chunk->next = NULL;
        set->chunk_tail = chunk;
        splock_unlock(&set->lock);
    }
    else {
overflow:
        __sync_fetch_and_sub(&pool->used_size, chunk->size);
        mem_chunk_destroy(chunk);
    }

#if MEM_POOL_DEBUG
        mem_pool_unregister_debug(pool, chunk);
#endif
    return;
}

void *mem_pool_calloc(MemPool *pool, uint32_t size
#if MEM_POOL_DEBUG
    , const char  *module
#endif
)
{
    void *p = NULL;
    p = mem_pool_alloc(pool, size 
#if MEM_POOL_DEBUG    
        , module
#endif
    );

    if(p) {
        memset(p, 0, size);
    }

    return p;
}

void *mem_pool_realloc(MemPool *pool, void *pold, uint32_t new_size
#if MEM_POOL_DEBUG
    , const char  *module
#endif
)
{
    uint32_t old_size = TINY_MIN_SIZE;
    void *pnew = NULL;

    old_size = mem_chunk_get_data_size(pold);
    if(mem_align_size(old_size, ALIGN_SIZE) >= new_size)
        return pold;

    pnew = mem_pool_alloc(pool, new_size
#if MEM_POOL_DEBUG
        , module
#endif
    );
    if(!pnew) {
        goto err;
    }

    memcpy(pnew, pold, new_size > old_size ? old_size : new_size);
    mem_pool_free(pool, pold);

    return pnew;
    
err:
    return NULL;
}

