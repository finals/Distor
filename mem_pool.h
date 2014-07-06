#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include <stdint.h>
#include "config.h"
#include "splock.h"

#define TINY_MIN_SIZE 16   
#define TINY_MAX_SIZE 1024
#define TINY_STEP 16
#define MEDIUM_MIN_SIZE 2048
#define MEDIUM_MAX_SIZE 65536   // 64KB
#define MEDIUM_STEP 1024
#define SETSIZE 127
#define ALIGN_SIZE 16
#define SET_MAX_SIZE 1048576  //1M

/* 内存块类型 */
#define TINY     1      //16 ~ 1024
#define MEDIUM   2      //1025 ~ 65535
#define LARGE    3      //65535 ~ 

#define OUTSET  -1       //表示该内存块不在set里
#define SET_BOUNDARY  64 //set的分界数组下标

/* 将size向上调整为最接近b的整数倍值 */
#define mem_align_size(size, b) (((size) + ((b) - 1)) & ~((b) - 1))

/* 将size向上调整为set表中所拥有的值 */
static inline uint32_t mem_align_size_match_set(size)
{                                    
    uint32_t align_size;
    
    align_size = (size <= TINY_MAX_SIZE) ? TINY_MIN_SIZE : TINY_MAX_SIZE; 
    return mem_align_size(size, align_size);                                
}

typedef struct MemChunk {
    struct MemChunk *next;
    struct MemChunk *prev;
    uint64_t spare0;
    int32_t  set_pos;   //set在set_table中的位置
    uint32_t size;      //可用空间大小
    char  data[0];
}MemChunk;
#define MemChunkSize (sizeof(struct MemChunk))

/* 根据chunk地址计算可用内存起始位置 */
#define mem_chunk_get_data(pchunk) ((void *)((pchunk)->data))
/* 根据data地址计算chunk指针 */
#define mem_chunk_get_chunk(pdata) ((MemChunk *)((MemChunk *)(pdata) - 1))
/* 根据data地址计算data大小 */
#define mem_chunk_get_data_size(pdata) \
        ((uint32_t)(*((uint32_t *)(pdata) - 1)))

typedef struct MemSet {
    struct MemChunk *chunk_head;  //栈底
    struct MemChunk *chunk_tail;  //栈顶
//    struct MemChunk *sentinel;
    uint32_t chunk_size;
    uint32_t chunk_count;
    splock_t lock;
}MemSet;
#define MemSetSize (sizeof(struct MemSet))

//允许在编译时通过gcc -DMEM_POOL_DEBUG=1的方式设置是否开启debug
#ifndef MEM_POOL_DEBUG   
#define MEM_POOL_DEBUG 0
#endif

#if MEM_POOL_DEBUG
#define DEBUG_SIZE 10000
#define MESSAGE_SIZE 80

typedef struct MemDebug {
    MemChunk *alloc_chunk;
    char  message[MESSAGE_SIZE];
    splock_t lock;
}MemDebug;
#define MemDebugSize (sizeof(struct MemDebug))
#endif  //#if MEM_POOL_DEBUG


typedef struct MemPool {
    struct MemSet *set_table[SETSIZE];
    uint64_t max_size;
    uint64_t used_size;
#if MEM_POOL_DEBUG
    struct MemDebug debug_table[DEBUG_SIZE];
#endif
}MemPool;
#define MemPoolSize (sizeof(struct MemPool))


MemPool *mem_pool_create(uint32_t size);
void mem_pool_destroy(MemPool *pool);

void *mem_pool_alloc(MemPool *pool, uint32_t size
#if MEM_POOL_DEBUG
    , const char  *module
#endif
);

void *mem_pool_calloc(MemPool *pool, uint32_t size
#if MEM_POOL_DEBUG
    , const char  *module
#endif
);

void *mem_pool_realloc(MemPool *pool, void *pold, uint32_t size
#if MEM_POOL_DEBUG
    , const char  *module
#endif
);

void mem_pool_free(MemPool *pool, void *p);

/* 由于数据结构的特殊性，用下面函数替代mem_pool_binary_search_set_by_size */
static inline int32_t mem_pool_get_set_by_size(uint32_t size)
{
    if(size <= TINY_MAX_SIZE) {
        return (size >> 4) - 1;    
    }
    else {
        return (size >> 10) + 62;
    }
}

#endif  /* __MEM_POOL_H__ */