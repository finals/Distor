#include <stdio.h>
#include <time.h> 
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "../mem_pool.h"

extern MemChunk * mem_chunk_create(uint32_t size, uint32_t set_pos);
extern void mem_chunk_destroy(MemChunk * chunk);
extern void mem_set_destroy(MemSet *set);
extern MemSet *mem_set_create(uint32_t size, uint32_t chunk_count, uint32_t pos);
extern int32_t mem_pool_binary_search_set_by_size(MemPool *pool, uint32_t size);

#define TEST_TIMES 2000

void test_mem_align_size()
{
    uint32_t i = 0, size = 0, align_size = 0, error_count = 0;
    srand((unsigned)time( NULL )); 

    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 65535;
        align_size = mem_align_size(size, ALIGN_SIZE);
        //
        if(align_size % ALIGN_SIZE){
            printf("%d  %d  %d\n", size, align_size, align_size % ALIGN_SIZE); 
            ++error_count;
        }
    }
    if(error_count)
        printf("test_mem_align_size error_count: %u\n", error_count);
    else
        printf("test_mem_align_size   [OK]\n");
}

void test_mem_chunk_get_data()
{
    uint32_t i = 0, size = 0, align_size = 0, error_count = 0;
    MemChunk *chunk = NULL;

    srand((unsigned)time( NULL )); 

    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 65535;
        align_size = mem_align_size(size, ALIGN_SIZE);
        chunk = (MemChunk *)malloc(MemChunkSize + align_size);

        if(chunk->data != mem_chunk_get_data(chunk)) {
            printf("chunk: %p  chunk->data: %p  data: %p  equal: %d\n", 
                chunk, chunk->data, mem_chunk_get_data(chunk), 
                chunk->data == mem_chunk_get_data(chunk));
            ++error_count;
        }
        free(chunk);
        chunk = NULL;
    }
    if(error_count)
        printf("test_mem_chunk_get_data error_count: %u\n", error_count);
    else
        printf("test_mem_chunk_get_data   [OK]\n");
}

void test_mem_chunk_get_chunk()
{
    uint32_t i = 0, size = 0, align_size = 0, error_count = 0;
    MemChunk *chunk = NULL;
    void *p = NULL;

    srand((unsigned)time( NULL )); 

    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 65535;
        align_size = mem_align_size(size, ALIGN_SIZE);
        chunk = (MemChunk *)malloc(MemChunkSize + align_size);
        p = mem_chunk_get_data(chunk);
        
        if(chunk != mem_chunk_get_chunk(p)) {
            printf("chunk: %p  get_chunk: %p equal: %d\n", 
                chunk, mem_chunk_get_chunk(p),
                chunk == mem_chunk_get_chunk(p)
            );
            ++error_count;
        }
        free(chunk);
        chunk = NULL;
    }
    if(error_count)
        printf("test_mem_chunk_get_chunk error_count: %u\n", error_count);
    else
        printf("test_mem_chunk_get_chunk   [OK]\n");
}

void test_mem_chunk_get_data_size()
{
    uint32_t i = 0, size = 0, align_size = 0, error_count = 0;
    MemChunk *chunk = NULL;
    void *p = NULL;

    srand((unsigned)time( NULL )); 

    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 65535;
        align_size = mem_align_size(size, ALIGN_SIZE);
        chunk = (MemChunk *)malloc(MemChunkSize + align_size);
        chunk->size = size;
        p = mem_chunk_get_data(chunk);
        
        if(size != mem_chunk_get_data_size(p)) {
            printf("chunk->size: %u  size: %u  get_size: %u  equal: %d\n", 
                chunk->size, size,  mem_chunk_get_data_size(p),
                chunk->size == mem_chunk_get_data_size(p)
            );
            ++error_count;
        }
        free(chunk);
        chunk = NULL;
    }
    if(error_count)
        printf("test_mem_chunk_get_data_size error_count: %u\n", error_count);
    else
        printf("test_mem_chunk_get_data_size   [OK]\n");
}

void test_mem_chunk()
{
    uint32_t i = 0, size = 0, align_size = 0, error_count = 0;
    MemChunk *chunk = NULL;
    void *p = NULL;

    srand((unsigned)time( NULL )); 

    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 65535;
        align_size = mem_align_size(size, ALIGN_SIZE);
        chunk = mem_chunk_create(size, OUTSET);
        p = mem_chunk_get_data(chunk);
        
        if(chunk->data != mem_chunk_get_data(chunk)) {
            printf("chunk: %p  chunk->data: %p  data: %p  equal: %d\n", 
                chunk, chunk->data, mem_chunk_get_data(chunk), 
                chunk->data == mem_chunk_get_data(chunk));
            ++error_count;
        }
        if(size != mem_chunk_get_data_size(p)) {
            printf("chunk->size: %u  size: %u  get_size: %u  equal: %d\n", 
                chunk->size, size,  mem_chunk_get_data_size(p),
                chunk->size == mem_chunk_get_data_size(p)
            );
            ++error_count;
        }
        if(chunk != mem_chunk_get_chunk(p)) {
            printf("chunk: %p  get_chunk: %p equal: %d\n", 
                chunk, mem_chunk_get_chunk(p),
                chunk == mem_chunk_get_chunk(p)
            );
            ++error_count;
        }
        mem_chunk_destroy(chunk);
        chunk = NULL;
    }
    if(error_count)
        printf("test_mem_chunk error_count: %u\n", error_count);
    else
        printf("test_mem_chunk   [OK]\n");
}

void test_mem_chunk_args(MemChunk *chunk, uint32_t *error_c)
{
    uint32_t i = 0, error_count = *error_c;
    void *p = NULL;

    srand((unsigned)time(NULL)); 

    if(!chunk)
        goto err;

    for(i = 0; i < TEST_TIMES; ++i) {
        p = mem_chunk_get_data(chunk);
        
        if(chunk->data != mem_chunk_get_data(chunk)) {
            printf("chunk: %p  chunk->data: %p  data: %p  equal: %d\n", 
                chunk, chunk->data, mem_chunk_get_data(chunk), 
                chunk->data == mem_chunk_get_data(chunk));
            ++error_count;
        }
        if(chunk->size != mem_chunk_get_data_size(p)) {
            printf("chunk->size: %u  get_size: %u  equal: %d\n", 
                chunk->size,  mem_chunk_get_data_size(p),
                chunk->size == mem_chunk_get_data_size(p)
            );
            ++error_count;
        }
        if(chunk != mem_chunk_get_chunk(p)) {
            printf("chunk: %p  get_chunk: %p equal: %d\n", 
                chunk, mem_chunk_get_chunk(p),
                chunk == mem_chunk_get_chunk(p)
            );
            ++error_count;
        }
    }
    *error_c = error_count;
    return;
err:
    ++error_count;
    *error_c = error_count;
    printf("chunk: %p\n", chunk);
    return;
}


void test_mem_set()
{
    MemSet *set = NULL;
    MemChunk *chunk = NULL;
    uint32_t i = 0, size = 0, align_size = 0, error_count = 0;

    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 524272 + 16;
        align_size = mem_align_size(size, ALIGN_SIZE);
        set = mem_set_create(align_size, (uint32_t)(524288/size), -1);
        for(chunk=set->chunk_head; chunk; chunk=chunk->next) {
            if(chunk->size != align_size || chunk->set_pos != -1) {
                 printf("chunk: %p chunk->size: %u align_size: %u chunk->set_pos: %d\n",
                     chunk, chunk->size, align_size, chunk->set_pos);
                 ++error_count;
            }
            if(chunk)
                test_mem_chunk_args(chunk, &error_count);
        }
        mem_set_destroy(set);
    }
    if(error_count)
        printf("test_mem_set error_count: %u\n", error_count);
    else
        printf("test_mem_set   [OK]\n");

}

void test_mem_set_args(MemSet *set, uint32_t *error_count, int32_t pos)
{
    MemChunk *chunk = NULL;
    uint32_t i = 0;

    if(!set)
        goto err;

    for(i = 0; i < TEST_TIMES; ++i) {
        for(chunk=set->chunk_head; chunk; chunk=chunk->next) {
            if(chunk->size != set->chunk_size || chunk->set_pos != pos) {
                 printf("chunk: %p set->chunk_size: %u  chunk->size: %u chunk->set_pos: %d pos: %d\n",
                     chunk, set->chunk_size, chunk->size, chunk->set_pos, pos);
                 ++(*error_count);
            }
            if(chunk)
                test_mem_chunk_args(chunk, error_count);
        }
    }
        
    return;

err:
    ++(*error_count);

    printf("set: %p\n", set);
    return;

}

void test_mem_pool_create()
{
    MemPool *pool = NULL;
    MemSet *set = NULL;
    uint32_t i = 0, j = 0, size = 0, align_size = 0, error_count = 0;

    srand((unsigned)time(NULL)); 
    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 200 + 16;
        align_size = mem_align_size(size, ALIGN_SIZE);

        pool = mem_pool_create(align_size);
        if(!pool)
            goto err;

        printf("pool size: %u\n", align_size);

        for(j = 0; j < SETSIZE; ++j) {
            set = pool->set_table[j];
            test_mem_set_args(set, &error_count, j);
        }
    }

    if(error_count)
        printf("test_mem_pool error_count: %u\n", error_count);
    else
        printf("test_mem_pool   [OK]\n");
    return;

err:
    ++error_count;
    printf("pool: %p\n", pool);
    return;

}

void test_mem_pool_set_size(MemPool *pool, uint32_t *error_count)
{
    uint32_t i = 0;
    MemChunk *chunk = NULL;
    MemSet *set = NULL;

    for(i = 0; i < SETSIZE; ++i) {
        set = pool->set_table[i];
        for(chunk = set->chunk_head; chunk; chunk = chunk->next) {
            if(set->chunk_size != chunk->size) {
                ++(*error_count);
                printf("set->chunk_size: %u  chunk->size: %u\n", 
                    set->chunk_size, chunk->size);
            }
        }
    }
}

void print_mem_pool_info(MemPool *pool)
{
    MemSet *set = NULL;
    uint32_t i = 0;

    printf("\n-----------Memory Pool Information------------\n");
    printf("Pool Size:      \t%llu\n", pool->max_size);
    printf("pool Used Size: \t%llu\n", pool->used_size);
    printf("pool set info:  \n");
    for(i = 0; i < SETSIZE; ++i) {
        set = pool->set_table[i];
        printf("set[%2u]", i);
        printf("\t chunk size:  \t%u\n", set->chunk_size);
        printf("\t chunk count: \t%u\n", set->chunk_count);
    }
}

void *p[TEST_TIMES];
void test_mem_pool_alloc()
{
    MemPool *pool = NULL;
    uint32_t i = 0, size = 0, align_size = 640, error_count = 0;
    uint64_t used_size = 0;
    
    pool = mem_pool_create(align_size); //160M
    print_mem_pool_info(pool);
    srand((unsigned)time(NULL)); 
    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 524272 + 16;  //16 ~ 2M
        p[i] = mem_pool_alloc(pool, size
#if MEM_POOL_DEBUG
        , __FUNCTION__
#endif
        );
        if(!p[i]) {
            ++error_count;
            goto err;
        }
        used_size += mem_align_size(size, ALIGN_SIZE);
        //printf("%u pool->used_size: %llu used_size: %llu\n", i, pool->used_size, used_size);
        
        //mem_pool_free(pool, p[i]);
        used_size -= mem_align_size(size, ALIGN_SIZE);
     
    }
    print_mem_pool_info(pool);
    for(i = 0; i < TEST_TIMES; ++i) {
       mem_pool_free(pool, p[i]); 
    }

    test_mem_pool_set_size(pool, &error_count);

err:
    if(pool->used_size != used_size)
        ++error_count;
    if(error_count)
        printf("test_mem_pool_alloc pool->max_size: %llu pool->used_size: %llu used_size: %llu error: %u\n",
            pool->max_size, pool->used_size, used_size, error_count);
    else {
        printf("test_mem_pool_alloc pool->max_size: %llu pool->used_size: %llu used_size: %llu \n",
            pool->max_size, pool->used_size, used_size);
        printf("test_mem_pool_alloc times:%u      [OK]\n", i);
    }
    print_mem_pool_info(pool);
        
    mem_pool_destroy(pool);
    
}

void test_mem_pool_binary_search_set_by_size()
{
    MemPool *pool = NULL;
    uint32_t i = 0, j = 0, size = 0, align_size = 160, error_count = 0;
    int32_t pos = OUTSET;
    uint32_t size_map[] = {
        16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536,
        131072, 262144, 524288
    };

    pool = mem_pool_create(align_size);

    srand((unsigned)time(NULL)); 
    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 524272 + 16;  //16 ~ 512K
        pos = mem_pool_binary_search_set_by_size(pool, size);
        for(j = 0; j < 16; ++j) {
            if(size_map[j] >= size)
                break;
        }
        if(pool->set_table[pos]->chunk_size < size) {
            printf("size: %u  match_size: %u  pos: %d chunk_size: %u size_map: %u\n",
                size, size_map[j], pos, pool->set_table[pos]->chunk_size, size_map[pos]);
            ++error_count;
        }
        else
            printf("size: %u pos: %d chunk_size: %u size_map: %u\n",
                size, pos, pool->set_table[pos]->chunk_size, size_map[pos]);
        
    }
    if(error_count)
        printf("test_mem_pool_binary_search_set_by_size  error: %u   [Failed]\n", error_count);
    else
        printf("test_mem_pool_binary_search_set_by_size   [OK]\n");
}

void test_mem_pool_realloc()
{
    MemPool *pool = NULL;
    MemSet *set = NULL;
    MemChunk *chunk = NULL;
    uint32_t i = 0, size = 0, align_size = 160, error_count = 0;
    int32_t pos = OUTSET; 
    void *p = NULL, *p_new = NULL;

    pool = mem_pool_create(align_size); //160M

    srand((unsigned)time(NULL)); 
    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 1048576 + 16;  //16 ~ 1M
        p = mem_pool_alloc(pool, size
#if MEM_POOL_DEBUG
        , __FUNCTION__
#endif
        );
        if(!p) {
            ++error_count;
            goto err;
        }

        if(i % 2) {
            align_size = size + rand() % 128000;
        }
        else {
            align_size = 2 * size + rand() % 128000;
        }
 
        p_new = mem_pool_realloc(pool, p, align_size);

        chunk = mem_chunk_get_chunk(p_new);
        if(align_size <= MAXSIZE) {
            pos = mem_pool_binary_search_set_by_size(pool, align_size);
            set = pool->set_table[pos];
            if(set->chunk_size < align_size || 
                align_size < set->chunk_size >> 1) {
                ++error_count;
                printf("p_old: %p old_size: %u p_new: %p new_size: %u set->chunk_size: %u\n",
                    p, size, p_new, align_size,
                    set->chunk_size);
            }
  
        }
        else {
            if(mem_align_size(align_size, ALIGN_SIZE) != chunk->size) {
                ++error_count;
                printf("p_old: %p old_size: %u p_new: %p new_size: %u\n",
                    p, mem_chunk_get_data_size(p), p_new, 
                    mem_chunk_get_data_size(p_new));
            }

        }
        mem_pool_free(pool, p_new);
    }
    if(error_count)
        printf("test_mem_pool_realloc pool->max_size: %llu pool->used_size: %llu error: %u\n",
            pool->max_size, pool->used_size,  error_count);
    else
        printf("test_mem_pool_realloc pool->max_size: %llu pool->used_size: %llu \n",
            pool->max_size, pool->used_size);

err:
    return;
    
}

int main()
{
    struct timeval starttime, endtime;
    double timeuse;
    gettimeofday(&starttime,0);
    
    //test_mem_align_size();
    //test_mem_chunk_get_data();
    //test_mem_chunk_get_chunk();
    //test_mem_chunk_get_data_size();
    //test_mem_chunk();
    //test_mem_set();
    //test_mem_pool_create();
    test_mem_pool_alloc();
    //test_mem_pool_binary_search_set_by_size();
    //test_mem_pool_realloc();
    
    gettimeofday(&endtime,0);

    timeuse = 1000000*(endtime.tv_sec - starttime.tv_sec) 
            + endtime.tv_usec - starttime.tv_usec;
    timeuse /=1000;

    printf("cost %lfms\n", timeuse);
    
    //printf("Chunk Struct size: %llu\n", MemChunkSize);    
    //printf("Set Struct size: %llu\n", MemSetSize);   
    //printf("Pool Struct size: %llu\n", MemPoolSize);   

    return 0;
}
