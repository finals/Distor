#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#define char8_t     char
#define uchar8_t    unsigned char
#define int32_t     int
#define uint32_t    unsigned int
#define int64_t     long long
#define uint64_t    unsigned long long

#define FACTOR 2
#define MINSIZE 16       //2^4
#define MAXSIZE 524288   //2^19   512KB
#define SETSIZE 16
#define ALIGN_SIZE 16
#define OUTSET  -1       //表示该内存块不在set里

/* 将size向上调整为最接近b的整数倍值 */
#define mem_align_size(size, b) (((size) + ((b) - 1)) & ~((b) - 1))

typedef struct MemChunk {
    struct MemChunk *next;
    struct MemChunk *prev;
    uint64_t spare0;
    int32_t  set_pos;   //set在set_table中的位置
    uint32_t size;      //可用空间大小
    char8_t data[0];
}MemChunk;
#define MemChunkSize (sizeof(struct MemChunk))

/* 根据chunk地址计算可用内存起始位置 */
#define mem_chunk_get_data(pchunk) ((void *)(pchunk->data))
/* 根据data地址计算chunk指针 */
#define mem_chunk_get_chunk(pdata) ((MemChunk *)((MemChunk *)pdata - 1))
/* 根据data地址计算data大小 */
#define mem_chunk_get_data_size(pdata) \
        ((uint32_t)(*((uint32_t *)pdata - 1)))

typedef struct MemSet {
    struct MemChunk *chunk_head;  //栈底
    struct MemChunk *chunk_tail;  //栈顶
    struct MemChunk *chunk_stack_top;  //内存栈可分配元素指针
    uint32_t chunk_size;
    uint32_t chunk_count;
}MemSet;
#define MemSetSize (sizeof(struct MemSet))

//允许在编译时通过gcc -DMEM_POOL_DEBUG=1的方式设置是否开启debug
#ifndef MEM_POOL_DEBUG   
#define MEM_POOL_DEBUG 0
#endif

#if MEM_POOL_DEBUG
#define MESSAGE_SIZE 80

typedef struct MemDebug {
    MemChunk *alloc_chunk;
    char8_t message[MESSAGE_SIZE];
}MemDebug;
#define MemDebugSize (sizeof(struct MemDebug))
#endif  //#if MEM_POOL_DEBUG


#define DEBUG_SIZE 10000
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
    , const char8_t *module
#endif
);

void *mem_pool_calloc(MemPool *pool, uint32_t size
#if MEM_POOL_DEBUG
    , const char8_t *module
#endif
);

void *mem_pool_realloc(MemPool *pool, void *pold, uint32_t size
#if MEM_POOL_DEBUG
    , const char8_t *module
#endif
);

void mem_pool_free(MemPool *pool, void *p);

#endif