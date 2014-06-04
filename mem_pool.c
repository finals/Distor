#include "mem_pool.h"

#include "debug.h"
#include <errno.h>

void * mem_alloc(uint_t size)
{
    void *p = malloc(size);
    if(NULL == p) {
        /* ��־��¼ */
    }
    return p;
}

void * mem_calloc(uint_t size)
{
    void *p = mem_alloc(size);
    if(NULL != p) {
        mem_zero(p, size);
    }
    return p;
}

void mem_free(void *ptr)
{
    free(ptr);
    ptr = NULL;
}

#if DISTOR_HAVE_POSIX_MEMALIGN
void *mem_align(uint_t alignment, uint_t size)
{
    void  *p = NULL;
    int err = 0;

    err = posix_memalign(&p, alignment, size);
    if(err) {
        p = NULL;
    }

    return p;
}
#elif DISTOR_HAVE_MEMALIGN
void *mem_align(uint_t alignment, uint_t size)
{
    void  *p = NULL;
    int err = 0;

    p = memalign(alignment, size);
    if(NULL == p) {
        p = NULL;
    }

    return p;
}
#endif

static memDataChunk * mem_create_chunk(memDataSet *set)
{
    memDataChunk *p = NULL;
    int size =  mem_align_size(sizeof(memDataChunk)+CHUNK_SIZE, 
            MEM_POOL_ALIGNMENT);

    p = mem_alloc(size);
    if(NULL == p) {
        return NULL;
    }

    p->set_id = set->id;
    p->use_count = 0;
    p->next = NULL;
    p->prev = NULL;
    p->avail = CHUNK_SIZE;
    p->end = p->begin;

    return p;
}

static uint_t mem_reset_chunk(memDataSet *set, memDataChunk *chunk)
{
    uint_t space = CHUNK_SIZE - chunk->avail;
    set->avail += space;
    set->max = CHUNK_SIZE;
    chunk->avail = CHUNK_SIZE;
    chunk->end = chunk->begin;
    chunk->use_count = 0;

    return space;
}

static void mem_destroy_set(memDataSet *set);

static memDataSet * mem_create_set(memPool *pool, uint_t id)
{
    memDataSet *set = NULL;
    memDataChunk *chunk = NULL;
    int i = 1;

    set = mem_alloc(sizeof(memDataSet));
    if(NULL == set) {
        /* ��־ */
        goto err;
    }
    set->chunk_head = NULL;
    set->chunk_tail = NULL;
    set->chunk_avail = NULL;
    set->large = NULL;
    set->id = id;
    set->avail = 0;
    set->max = 0;
    set->next = NULL;
    set->prev = NULL;

    for(i = 1; i <= SET_SIZE; ++i) {
        chunk = mem_create_chunk(set);
        if(NULL == chunk) {
            /* ��־ */
            goto err;
        }
        if(NULL != set->chunk_head) {  
            set->chunk_tail->next = chunk;
            chunk->prev = set->chunk_tail;
            set->chunk_tail = chunk;
        }
        else {     /* �����һ���ڵ� */
            set->chunk_head = chunk;
            set->chunk_tail = chunk;
        }
    }
    set->avail = CHUNK_SIZE * SET_SIZE;
    set->max = CHUNK_SIZE;
    set->chunk_avail = set->chunk_head;

    return set;

err:
    mem_destroy_set(set);
    return NULL;
}

static void mem_destroy_set(memDataSet *set)
{
    memDataChunk *chunk = NULL, *p = NULL;
    memLargeData *large = NULL, *pl = NULL;

    if(NULL != set) {
        /* ����set�е�chunk���� */
        if(NULL != set->chunk_head) {
            chunk = set->chunk_head;
            while(NULL != chunk) {
                p = chunk->next;
                mem_free(chunk);
                chunk = p;
            }
        }
        /* ����set�е�large���� */
        if(NULL != set->large) {
            large = set->large;
            while(NULL != large) {
                pl = large->next;
                mem_free(large);
                large = pl;
            }
        }
        mem_free(set);
        set = NULL;
    }
}

static uint_t mem_reset_set(memDataSet *set)
{
    memDataChunk *chunk = NULL;
    memLargeData *large = NULL, *p = NULL;
    uint_t space = 0;

    if(NULL != set) {
        if(NULL != set->chunk_head) {
            chunk = set->chunk_head;
            while(NULL != chunk) {
                space += mem_reset_chunk(set, chunk);
            }
            set->avail = CHUNK_SIZE * SET_SIZE;
        }
        /* ����large���������ڴ� */
        large = set->large;
        while(NULL != large) {
            p = large->next;
            mem_free(large);
            large = p;
        }
        set->large = NULL;
    }

    return space;
}

static void mem_restore_set(memDataSet *set)
{
    memDataChunk *chunk = set->chunk_head, *p = NULL; 
    
    set->avail = 0;
    set->max = MEM_POOL_ALIGNMENT;
    set->chunk_avail = NULL;
    while(NULL != chunk) {
        p = chunk->next;
        /* ����chunk */
        if(chunk->begin + chunk->avail != chunk->end) {
            chunk->end = chunk->begin + chunk->avail;
        }
        
        set->avail += chunk->avail;
        if(chunk->avail > set->max) {  
            set->max = chunk->avail;
        }

        /* ��ǰchunkû��ʣ��ռ䣬����ͷ�� */
        if(MEM_POOL_ALIGNMENT > chunk->avail) {
            if(chunk != set->chunk_head) {
                if(chunk == set->chunk_tail) {  /* ָ������β�� */
                    set->chunk_tail = set->chunk_tail->prev;
                    set->chunk_tail->next = NULL;
                }
                else {
                    /* chunk�������� */
                    chunk->prev->next = chunk->next;
                    chunk->next->prev = chunk->prev;     
                }
                /* chunk��������ͷ�� */
                chunk->next = set->chunk_head;
                chunk->prev = NULL;
                set->chunk_head->prev = chunk;
                set->chunk_head = chunk;
            }
        }
        else if(NULL == set->chunk_avail) {
            set->chunk_avail = chunk;
        }
        
        chunk = p;
    }
}

memPool * mem_create_pool(uint_t size, float ratio)
{
    memPool *pool = NULL;
    int len = 0, i = 0;

    //distor_debug(trc, "begin create pool\n")

    pool = mem_alloc(sizeof(memPool));
    if(NULL == pool) {
        /* ��־ */
        distor_debug(trc, "alloc memory pool failed: %s\n", strerror(errno));
        goto err;
    }
    
    pool->size = size * SET_SIZE * CHUNK_SIZE;
    pool->use_id = 0;

    /* ����ratio���ִ��ڴ���С�ڴ��Ŀռ�ռ���� */
    len = (uint_t)(size * ratio);
    pool->set_length = mem_align_size(len, 2); /* set������������2�ı��� */
    pool->lavail = (size - pool->set_length) * SET_SIZE * CHUNK_SIZE;
    pool->avail = pool->size - pool->lavail;    /* �ڴ�صĿ��ÿռ� */
    pool->set_table = mem_alloc(sizeof(memDataSet *) * pool->set_length);
    if(NULL == pool->set_table) {
        /* ��־ */
        distor_debug(trc, "alloc set table failed: %s\n", strerror(errno));
        goto err;
    }
    
    for(i = 0; i < pool->set_length; ++i) {
        pool->set_table[i] = mem_create_set(pool, i);
        if(NULL == pool->set_table[i]) {
            /* ��־ */
            distor_debug(trc, "alloc set failed: %s\n", strerror(errno));
            goto err;
        }
    }

    return pool;

err:
   mem_destroy_pool(pool);
   
   return NULL;
}

void mem_destroy_pool(memPool *pool)
{
    int i = 0;
    
    if(NULL != pool) {
       if(NULL != pool->set_table) {
           for(i = 0; i < pool->set_length; ++i) {
               if(NULL != pool->set_table[i]) {
                   mem_destroy_set(pool->set_table[i]);
               }
           }
           mem_free(pool->set_table);
       }
       mem_free(pool);
   }
}

void mem_reset_pool(memPool *pool)
{
    int i = 0;

    for(i = 0; i < pool->set_length; ++i) {
        if(NULL != pool->set_table[i]) {
            pool->avail += mem_reset_set(pool->set_table[i]);
        }
    }
    
    pool->lavail = pool->size - pool->set_length * SET_SIZE * CHUNK_SIZE;
    pool->avail = pool->size - pool->lavail;
    pool->use_id = 0;
}

static void * mem_pool_alloc_large(memPool *pool, uint_t size)
{
    memLargeData *large = NULL;
   
    memDataSet *set = pool->set_table[pool->use_id];

    large = mem_alloc(size);
    if(NULL == large) {
        /* ��־ */
        return NULL;
    }
    large->size = size;

    /* ����ͷ������ */
    large->next = set->large;
    large->prev = NULL;
    if(NULL != set->large) {
        set->large->prev = large;   
    }
    set->large = large;
    large->set = set;
    mem_pool_next_set(pool);

    return large->begin;
   
}

static int mem_pool_free_large(memPool *pool, uchar_t *ptr)
{
    memLargeData *large = NULL;
    memDataSet *set = NULL;
    memLargeData *pl = NULL;
    int size = 0;

    //distor_debug(trc, "mempool free large\n");
    if(NULL == ptr) {
        return MEMORY_OK;
    }

    /* Ѱ��large�ṹ����ʼλ�� */
    large = (memLargeData *)((uchar_t *)ptr-sizeof(memLargeData));
    set = large->set;
    
    size=large->size;
    pl = set->large;
    while(NULL != pl) {
        if(pl == large) {
            if(pl != set->large) {
                pl->prev->next = pl->next;
            }
            else {
                set->large = NULL;
            }
            if(NULL != pl->next) {
                pl->next->prev = pl->prev;
            }

            mem_free(large);
            ptr = NULL;
            pool->lavail += size;
            return MEMORY_OK;
        }
    }
    ptr = NULL;
    return MEMORY_ERR;
}

static void * mem_pool_alloc_small_old(memPool *pool, uint_t size)
{
    memDataChunk *chunk = NULL, *prev_chunk = NULL;
    void *p = NULL;
    memDataSet *set = NULL;
    uint_t pos = pool->use_id;

    /* �ҵ����Է���size��С��set */
    while(1) {   
        //printf("111111111\n");
        set = pool->set_table[pool->use_id];
        if(set->max < size) {    /* ��setû�пռ���� */
            mem_pool_next_set(pool);
            if(pool->use_id == pos) {   /* ����������setû�к��ʵĿռ� */
                /* ��־ */
                //printf("no set to save\n");
                goto err;
            }
        }
        else {     /* ��set���ռ���� */
            /* Ѱ�Ҹ�set�У����Դ��size��С��chunk */
            chunk = set->chunk_avail;
            //printf("set->max= %d\n", set->max);
            while(NULL != chunk && chunk->avail < size) {  
                //printf("222222222  %d\n", chunk->avail);
                prev_chunk = chunk;
                chunk = chunk->next;
            }
            
            if(NULL == chunk) {   /* ��setû�к��ʵĿռ䣬��set->maxì�� */
                mem_restore_set(set);
                /* ��־ */
                continue;
                //goto err;
            }
            else if(chunk->avail >= size) break;
        }
    }

    //printf("alloc set id= %d, chunk addr= %p\n", chunk->set_id, chunk);
    p = (void *)chunk->end;
    chunk->end = (uchar_t *)(chunk->end) + size;
    chunk->avail -= size;
    ++(chunk->use_count);
    set->avail -= size;
    if(chunk == set->chunk_tail) {   /* �����һ��chunk����ռ䣬����max */
        mem_restore_set(set);
        //printf("restore set set->max= %d\n", set->max);
    }
    else {
        /* �����chunkû�п��ÿռ䣬�������chunk_avail��ǰһ���ڵ� */
        if(MEM_POOL_ALIGNMENT > chunk->avail) {
            if(chunk == set->chunk_avail) {
                set->chunk_avail = set->chunk_avail->next;
            }
            else {
                /* �Ƚ�chunk��������ɾ�� */
                chunk->prev->next = chunk->next;
                chunk->next->prev = chunk->prev;
                
                /* ��chunk����chunk_availǰһ���ڵ� */
                chunk->next = set->chunk_avail;
                if(set->chunk_head == set->chunk_avail) {
                    set->chunk_head = chunk;
                }
                else {
                    chunk->prev = set->chunk_avail->prev;
                    set->chunk_avail->prev->next = chunk;
                }
                set->chunk_avail->prev = chunk;
                
            }
        }
    }
    
    mem_pool_next_set(pool);
    return p;
err:

    return NULL;

}

static void mem_pool_free_small_old(memPool *pool, uchar_t *ptr)
{
    memDataChunk *chunk = NULL;
    memDataSet *set = NULL;
    int i = 0;
    int offset = 0;

    //distor_debug(trc, "mempool free small\n");
    if(NULL == ptr) {
        distor_debug(trc, "mempool free small: NULL\n");
        return;
    }

    for(i = 0; i < pool->set_length; ++i) {
        set = pool->set_table[i];
        chunk = set->chunk_head;
        for( ; NULL != chunk; chunk = chunk->next) {
            offset = ptr - chunk->begin;
            if(offset >=0 && offset <= CHUNK_SIZE) {
                ////printf("chunk= %p, begin=%p, ptr=%p\n", chunk, chunk->begin, ptr);   
                goto find;    
            }
        }
    }

    return;

find:
    --(chunk->use_count);

    ////printf("free set id= %d, chunk addr= %p\n", chunk->set_id, chunk);
    //distor_debug(trc, "chunk use count: %d\n", chunk->use_count);
    if(0 >= chunk->use_count) {  /* ����Ϊ0�� �����ڴ� */
        ////printf("call mem reset chunk\n");
        pool->avail += mem_reset_chunk(set, chunk);
    }

    ptr = NULL;
}

void * mem_pool_alloc(memPool *pool, uint_t size)
{
    int real_size = mem_align_size(size, MEM_POOL_ALIGNMENT);
    void * p = NULL;
    
    if(ALLOC_LIMIT >= real_size) {
        if(pool->avail < real_size) {
            /* ��־ */
            distor_debug(trc, "mempool small is full, left %d.\n", pool->avail);
            return NULL;
        }
        p = mem_pool_alloc_small(pool, real_size);
        if(NULL != p) {
            pool->avail -= real_size;
        }
    }
    else {
        real_size = mem_align_size(sizeof(memLargeData) + real_size, 
                MEM_POOL_ALIGNMENT);
        if(pool->lavail < real_size) {
            /* ��־ */
            distor_debug(trc, "mempool small is full, left %d.\n", pool->lavail);
            return NULL;
        }
        p = mem_pool_alloc_large(pool, real_size);
        if(NULL != p) {
            pool->lavail -= real_size;
        }
    }

    return p;
}

void * mem_pool_calloc(memPool *pool, uint_t size)
{
    int real_size = mem_align_size(size, MEM_POOL_ALIGNMENT);
    void *p = mem_pool_calloc(pool, real_size);

    if(NULL != p) {
        mem_zero(p, real_size);
    }

    return p;
}

void mem_pool_free_old(memPool *pool, void *ptr)
{
    distor_debug(trc, "free type: %d\n", type_large_data(ptr));
    
    if(NULL != ptr && type_large_data(ptr)) {  /* large���ݣ��ͷ��ڴ棬�黹ϵͳ */
        mem_pool_free_large(pool, ptr);      
    }
    else {
        mem_pool_free_small(pool, ptr);
    }
}


void mem_pool_free(memPool *pool, void *ptr) {

}