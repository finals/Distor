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

/* �ڴ������ */
#define TINY     1      //16 ~ 1024
#define MEDIUM   2      //1025 ~ 65535
#define LARGE    3      //65535 ~ 

#define OUTSET  -1       //��ʾ���ڴ�鲻��set��
#define SET_BOUNDARY  64 //set�ķֽ������±�

/* ��size���ϵ���Ϊ��ӽ�b��������ֵ */
#define mem_align_size(size, b) (((size) + ((b) - 1)) & ~((b) - 1))

/* ��size���ϵ���Ϊset������ӵ�е�ֵ */
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
    int32_t  set_pos;   //set��set_table�е�λ��
    uint32_t size;      //���ÿռ��С
    char  data[0];
}MemChunk;
#define MemChunkSize (sizeof(struct MemChunk))

/* ����chunk��ַ��������ڴ���ʼλ�� */
#define mem_chunk_get_data(pchunk) ((void *)((pchunk)->data))
/* ����data��ַ����chunkָ�� */
#define mem_chunk_get_chunk(pdata) ((MemChunk *)((MemChunk *)(pdata) - 1))
/* ����data��ַ����data��С */
#define mem_chunk_get_data_size(pdata) \
        ((uint32_t)(*((uint32_t *)(pdata) - 1)))

typedef struct MemSet {
    struct MemChunk *chunk_head;  //ջ��
    struct MemChunk *chunk_tail;  //ջ��
//    struct MemChunk *sentinel;
    uint32_t chunk_size;
    uint32_t chunk_count;
    splock_t lock;
}MemSet;
#define MemSetSize (sizeof(struct MemSet))

//�����ڱ���ʱͨ��gcc -DMEM_POOL_DEBUG=1�ķ�ʽ�����Ƿ���debug
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

/* �������ݽṹ�������ԣ������溯�����mem_pool_binary_search_set_by_size */
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