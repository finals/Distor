#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include <stdlib.h>   /* for posix_memalign or memalign */
#include <string.h>

/*
 * Linux has memalign() or posix_memalign()
 * Solaris has memalign()
 * FreeBSD 7.0 has posix_memalign(), besides, early version's malloc()
 * aligns allocations bigger than page size at the page boundary
 */
#ifdef linux
#define DISTOR_HAVE_POSIX_MEMALIGN 1
#else
#define DISTOR_HAVE_MEMALIGN 1
#endif 

#define MEMORY_OK    0
#define MEMORY_ERR  -1

#define MEM_POOL_ALIGNMENT 16

typedef unsigned char  uchar_t;
typedef unsigned int   uint_t;
typedef unsigned int * uint_p;

/* ��size���ϵ���Ϊ��ӽ�b��������ֵ */
#define mem_align_size(size, b) (((size) + ((b) - 1)) & ~((b) - 1))
/* ��ָ��p��ֵ���ϵ���Ϊb�������� */
#define mem_align_ptr(p, b) \
       (uchar_t *)(((uint_p)(p) + ((uint_p)b - 1)) & ~((uint_p)b - 1))
    
/* �ڴ���0 */
#define mem_zero(buf, size) (void)memset(buf, 0, size)

typedef struct memDataChunk {
    struct memDataChunk *next;   /* ָ����һ��chunk */
    struct memDataChunk *prev;   /* ָ����һ��chunk */
    uchar_t *end;      /* ������������ʼλ�� */
    uint_t spare0;     /* ��������ռλ */
    uint_t set_id;     /* chunk������set��pool */
    int use_count;  /* ��chunk�����õ����� */
    uint_t avail;      /* ��chunk�п��ÿռ� */
    uchar_t begin[0];  /* ��������ʼλ�� */
}memDataChunk;

struct memDataSet;
typedef struct memLargeData {
    struct memLargeData *next;
    struct memLargeData *prev;
    struct memDataSet *set;     /* ��largeData������set */
    uint_t spare0;     /* ��������ռλ */
    uint_t size;       /* ������ݵĴ�С */
    uchar_t begin[0];  /* ��������ʼλ�� */
}memLargeData;

typedef struct memDataSet {
    struct memDataChunk *chunk_head;
    struct memDataChunk *chunk_tail;
    struct memDataChunk *chunk_avail;  /* chunk�������пռ�ĵ�һ������chunk */
    struct memLargeData *large;    /* ָ������ݿ� */
    uint_t id;       /* ��Set���ڴ���е���� */
    uint_t avail;    /* ��set�����������Ĵ�С */
    uint_t max;      /* ��ǰset����һ����������С */
    uint_t keep;     /* ��������ռλ */
    struct memDataSet   *prev;
    struct memDataSet   *next;
}memDataSet;

typedef struct {
    uint_t size;    /* �ڴ�ش�С */
    uint_t avail;   /* �ڴ�ؿ��ÿռ� */
    uint_t lavail;   /* ��ռ�Ŀ��ÿռ� */
    uint_t use_id;  /* ��ǰ�����ڴ��set id */
    uint_t set_length;  /* set���� */
    struct memDataSet **set_table;  /* set�� */
}memPool;

#define CHUNK_SIZE 1048576  /* chunk��СΪ1M */
#define SET_SIZE   32       /* set��СΪ1M * 32 = 32M */
#define ALLOC_LIMIT  4096 /* ������� */

/* �ڴ�����ͷ� */
void * mem_alloc(uint_t size);
void * mem_calloc(uint_t size);
void mem_free( void *ptr);

#if (DISTOR_HAVE_POSIX_MEMALIGN || DISTOR_HAVE_MEMALIGN)
void *mem_align( uint_t alignment, uint_t size);
#else
#define mem_align(alignment, size)  mem_alloc(size)
#endif

/* �ж��Ƿ���large���� */
#define type_large_data(p) (*(uint_p)((uchar_t *)p-sizeof(uint_t)) > ALLOC_LIMIT)

/* �ڴ�ز��� */
memPool * mem_create_pool(uint_t size, float ratio);        /* ���� */
void mem_destroy_pool(memPool *pool);          /* ���� */
void mem_reset_pool(memPool *pool);            /* ���� */

void * mem_pool_alloc(memPool *pool, uint_t size);     /* ���� */
void * mem_pool_calloc(memPool *pool, uint_t size);    /* ���� ��ʼ��Ϊ0 */
void mem_pool_free(memPool *pool, void *ptr);          /* �ͷ� */

static inline void mem_pool_next_set(memPool *pool)
{
    ++(pool->use_id);
    if(pool->use_id >= pool->set_length) {
        pool->use_id = 0;
    }
}

#endif