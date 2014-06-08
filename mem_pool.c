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
    chunk = NULL;
}

void mem_set_destroy(MemSet *set)
{
    MemChunk *chunk = set->chunk_head, *next = NULL;

    if(!set) {
        while(!chunk) {
            next = chunk->next;
            mem_chunk_destroy(chunk);
            chunk = next;
        }

        free(set);
        set = NULL;
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
        //链表头插入
        chunk->next = set->chunk_head;
        chunk->prev = NULL;
        if(set->chunk_head)
            set->chunk_head->prev = chunk;
        set->chunk_head = chunk;
        if(!set->chunk_tail)
            set->chunk_tail = chunk;
    }

    set->chunk_count = chunk_count;
    set->chunk_stack_top = set->chunk_head;
    set->chunk_size = size;

    return set;

err:
    mem_set_destroy(set);
    return NULL;
    
}

#if MEM_POOL_DEBUG
static void mem_pool_register_debug(MemPool *pool, MemChunk *chunk, 
    const char8_t * module)
{

    uint32_t i = 0;
    char8_t *msg = "[module]%s [chunk]%p [size]%u";

    for(i = 0; i < DEBUG_SIZE; ++i) {
        if(!pool->debug_table[i].alloc_chunk) {
            pool->debug_table[i].alloc_chunk = chunk;
            snprintf(pool->debug_table[i].message, MESSAGE_SIZE, msg, 
                    module, chunk, chunk->size);
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
#if MEM_POOL_DEBUG
        mem_pool_output_debug(pool); 
#endif
        for(i = 0; i < SETSIZE; ++i) {
            if(!pool->set_table[i]) {
                mem_set_destroy(pool->set_table[i]);
            }
        }

        free(pool);
        pool = NULL;
    }
}

MemPool *mem_pool_create(uint32_t max_size)
{
    uint32_t i = 0, chunk_size = MINSIZE;
    MemPool *pool = NULL;

    pool = (MemPool *)malloc(MemPoolSize);
    if(!pool) {
        goto err;  
    }
    pool->max_size = max_size * 1024 * 1024;
    pool->used_size = 0;
    
    for(i = 0; i < SETSIZE; ++i) {
        pool->set_table[i] = mem_set_create(chunk_size, MAXSIZE/chunk_size, i);
        if(!pool->set_table[i]) {
            goto err;
        }
        chunk_size = chunk_size << 1; 
    }

#if MEM_POOL_DEBUG
    for(i = 0; i < DEBUG_SIZE; ++i) {
        pool->debug_table[i].alloc_chunk = NULL;
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
    uint32_t chunk_size = 0;

    if(size > MAXSIZE) return OUTSET;

    for(i = 0, j = SETSIZE - 1; i <= j; ) {
        mid = (i + j) >> 1;
        chunk_size = pool->set_table[mid]->chunk_size;
        if(size <= chunk_size && size > chunk_size >> 1) {
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
    , const char8_t *module
#endif
)
{
    uint32_t align_size = mem_align_size(size, ALIGN_SIZE);
    int32_t pos = -1;
    MemChunk *palloc = NULL;

    if(pool->used_size + align_size > pool->max_size)
        return NULL;

    if(align_size <= MAXSIZE) {
        pos = mem_pool_binary_search_set_by_size(pool, align_size);
        if(-1 != pos) {
            palloc = pool->set_table[pos]->chunk_stack_top;
            if(palloc) {
                //chunk_stack_top指向下一个元素，如果它指向NULL，表示栈满
                pool->set_table[pos]->chunk_stack_top = palloc->next;
            }
        }
    }
    
    if(!palloc) {
        palloc = mem_chunk_create(align_size, OUTSET);
    }
    
    if(palloc) {
#if MEM_POOL_DEBUG
        mem_pool_register_debug(pool, palloc, module);
#endif
        //printf("pool->used_size: %lu  alloc size: %u\n", pool->used_size, align_size);
        pool->used_size += palloc->size;
        
        return mem_chunk_get_data(palloc);
    }
    return NULL;
}

void mem_pool_free(MemPool *pool, void *p)
{
    MemChunk *chunk = NULL;
    MemSet *set = NULL;
    int32_t pos = OUTSET;

    if(!p) return;

    chunk = mem_chunk_get_chunk(p);
    //printf("pool->used_size: %lu  free size: %u\n", pool->used_size, chunk->size);
    pool->used_size -= chunk->size;
    
    if(chunk->size <= MAXSIZE) {
        if(OUTSET != chunk->set_pos) {  //chunk在set中
            set = pool->set_table[chunk->set_pos];
            //chunk是尾节点
            if(chunk == set->chunk_tail) {
                set->chunk_stack_top = chunk;
                goto done;
            }
            //chunk从set链表中解开
            if(!chunk->prev) { //chunk是头结点
                set->chunk_head = chunk->next;
                chunk->next = NULL;
            }
            else{
                chunk->prev->next = chunk->next;
                chunk->next->prev = chunk->prev;
            }
            //chunk加入set链表尾部
            chunk->prev = set->chunk_tail;
            chunk->next = NULL;
            set->chunk_tail->next = chunk;
            set->chunk_tail = chunk;
            
        }
        else {  //chunk不在set中
            pos = mem_pool_binary_search_set_by_size(pool, chunk->size); 
            set = pool->set_table[pos];
            //将chunk加入对应的set链表中
            chunk->prev = set->chunk_tail;
            set->chunk_tail->next = chunk;
            set->chunk_tail = chunk;
            if(!set->chunk_stack_top) {  //如果set满
                set->chunk_stack_top = chunk;
            }
            ++set->chunk_count;  //set增加一个chunk
            chunk->size = set->chunk_size;
        }
    }
    else {
        mem_chunk_destroy(chunk);
    }

done: 
#if MEM_POOL_DEBUG
        mem_pool_unregister_debug(pool, chunk);
#endif
    return;

}

void *mem_pool_calloc(MemPool *pool, uint32_t size
#if MEM_POOL_DEBUG
    , const char8_t *module
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
    , const char8_t *module
#endif
)
{
    uint32_t old_size = MINSIZE;
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

