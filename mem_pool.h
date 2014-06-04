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

/* 将size向上调整为最接近b的整数倍值 */
#define mem_align_size(size, b) (((size) + ((b) - 1)) & ~((b) - 1))
/* 将指针p的值向上调整为b的整数倍 */
#define mem_align_ptr(p, b) \
       (uchar_t *)(((uint_p)(p) + ((uint_p)b - 1)) & ~((uint_p)b - 1))
    
/* 内存置0 */
#define mem_zero(buf, size) (void)memset(buf, 0, size)

typedef struct memDataChunk {
    struct memDataChunk *next;   /* 指向下一个chunk */
    struct memDataChunk *prev;   /* 指向上一个chunk */
    uchar_t *end;      /* 可用数据区开始位置 */
    uint_t spare0;     /* 保留，并占位 */
    uint_t set_id;     /* chunk所属的set在pool */
    int use_count;  /* 该chunk被引用的数量 */
    uint_t avail;      /* 该chunk中可用空间 */
    uchar_t begin[0];  /* 数据区开始位置 */
}memDataChunk;

struct memDataSet;
typedef struct memLargeData {
    struct memLargeData *next;
    struct memLargeData *prev;
    struct memDataSet *set;     /* 该largeData所属的set */
    uint_t spare0;     /* 保留，并占位 */
    uint_t size;       /* 大块数据的大小 */
    uchar_t begin[0];  /* 数据区起始位置 */
}memLargeData;

typedef struct memDataSet {
    struct memDataChunk *chunk_head;
    struct memDataChunk *chunk_tail;
    struct memDataChunk *chunk_avail;  /* chunk链表中有空间的第一个可用chunk */
    struct memLargeData *large;    /* 指向大数据块 */
    uint_t id;       /* 该Set在内存池中的序号 */
    uint_t avail;    /* 该set可用数据区的大小 */
    uint_t max;      /* 当前set最大的一块数据区大小 */
    uint_t keep;     /* 保留，并占位 */
    struct memDataSet   *prev;
    struct memDataSet   *next;
}memDataSet;

typedef struct {
    uint_t size;    /* 内存池大小 */
    uint_t avail;   /* 内存池可用空间 */
    uint_t lavail;   /* 大空间的可用空间 */
    uint_t use_id;  /* 当前分配内存的set id */
    uint_t set_length;  /* set表长度 */
    struct memDataSet **set_table;  /* set表 */
}memPool;

#define CHUNK_SIZE 1048576  /* chunk大小为1M */
#define SET_SIZE   32       /* set大小为1M * 32 = 32M */
#define ALLOC_LIMIT  4096 /* 分配界限 */

/* 内存分配释放 */
void * mem_alloc(uint_t size);
void * mem_calloc(uint_t size);
void mem_free( void *ptr);

#if (DISTOR_HAVE_POSIX_MEMALIGN || DISTOR_HAVE_MEMALIGN)
void *mem_align( uint_t alignment, uint_t size);
#else
#define mem_align(alignment, size)  mem_alloc(size)
#endif

/* 判断是否是large数据 */
#define type_large_data(p) (*(uint_p)((uchar_t *)p-sizeof(uint_t)) > ALLOC_LIMIT)

/* 内存池操作 */
memPool * mem_create_pool(uint_t size, float ratio);        /* 创建 */
void mem_destroy_pool(memPool *pool);          /* 销毁 */
void mem_reset_pool(memPool *pool);            /* 重置 */

void * mem_pool_alloc(memPool *pool, uint_t size);     /* 分配 */
void * mem_pool_calloc(memPool *pool, uint_t size);    /* 分配 初始化为0 */
void mem_pool_free(memPool *pool, void *ptr);          /* 释放 */

static inline void mem_pool_next_set(memPool *pool)
{
    ++(pool->use_id);
    if(pool->use_id >= pool->set_length) {
        pool->use_id = 0;
    }
}

#endif