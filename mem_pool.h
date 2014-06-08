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
#define OUTSET  -1       //��ʾ���ڴ�鲻��set��

/* ��size���ϵ���Ϊ��ӽ�b��������ֵ */
#define mem_align_size(size, b) (((size) + ((b) - 1)) & ~((b) - 1))

typedef struct MemChunk {
    struct MemChunk *next;
    struct MemChunk *prev;
    uint64_t spare0;
    int32_t  set_pos;   //set��set_table�е�λ��
    uint32_t size;      //���ÿռ��С
    char8_t data[0];
}MemChunk;
#define MemChunkSize (sizeof(struct MemChunk))

/* ����chunk��ַ��������ڴ���ʼλ�� */
#define mem_chunk_get_data(pchunk) ((void *)(pchunk->data))
/* ����data��ַ����chunkָ�� */
#define mem_chunk_get_chunk(pdata) ((MemChunk *)((MemChunk *)pdata - 1))
/* ����data��ַ����data��С */
#define mem_chunk_get_data_size(pdata) \
        ((uint32_t)(*((uint32_t *)pdata - 1)))

typedef struct MemSet {
    struct MemChunk *chunk_head;  //ջ��
    struct MemChunk *chunk_tail;  //ջ��
    struct MemChunk *chunk_stack_top;  //�ڴ�ջ�ɷ���Ԫ��ָ��
    uint32_t chunk_size;
    uint32_t chunk_count;
}MemSet;
#define MemSetSize (sizeof(struct MemSet))

//�����ڱ���ʱͨ��gcc -DMEM_POOL_DEBUG=1�ķ�ʽ�����Ƿ���debug
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