#include <stdio.h>
#define DEBUG_LEVEL DEBUG_ERR
#include "../mem_pool.h"
#include <time.h> 
#include <sys/time.h>
#include <unistd.h>

void test_mem_pool()
{
    uint_t size = 10;   
    float ratio = 0.6;
    char *p = NULL;
    memPool *pool = mem_create_pool(size, ratio);
    if(NULL == pool) {
        printf("pool: %p\n", pool);
        goto err;
    }

    size = 1024;
    p = mem_pool_alloc(pool, size);
    if(NULL == pool) {
        printf("first alloc[size= %d] %p\n",size, p);
        goto err;
    }
    printf("first alloc[size= %d]  %p\n",size,  p);
    //strncpy(p, "hello mempool", sizeof("hello mempool"));
    //printf("%s\n", (char *)p);
    mem_pool_free(pool, p);
    printf("first free[size= %d]  %p\n",size,  p);

    size=2048;
    p = mem_pool_alloc(pool, size);
    if(NULL == pool) {
        printf("second alloc[size= %d]  %p\n",size,  p);
        goto err;
    }
    printf("second alloc[size= %d]  %p\n",size,  p);
    mem_pool_free(pool, p);
    printf("second free[size= %d]  %p\n",size,  p);

    size=888;
    p = mem_pool_alloc(pool, size);
    if(NULL == pool) {
        printf("third alloc[size= %d] %p\n",size,  p);
        goto err;
    }
    printf("third alloc[size= %d]  %p\n",size,  p);
    mem_pool_free(pool, p);
    printf("third free[size= %d]  %p\n",size,  p);

    
err:
    mem_destroy_pool(pool);
}


void test_for_mem_pool()
{
    long long i = 0;
    uint_t size = 100;   
    float ratio = 0.6;
    char *p = NULL;
    uint_t fail = 0;
    uint_t org_avail = 0;
    uint_t org_lavail = 0;
    memDataChunk *chunk = NULL;
    uint_t max_chunk = 0;
    struct timeval starttime, endtime;

    memPool *pool = mem_create_pool(size, ratio);
    if(NULL == pool) {
        printf("pool: %p\n", pool);
        goto err;
    }

    org_avail = pool->avail;
    org_lavail = pool->lavail;

    srand( (unsigned)time( NULL ) );  

    gettimeofday(&starttime,0);
    while(1) {
        //printf("**************%d*****************\n", i);
        //size = (uint_t)(rand() % 8080 + 16);
        //size = (uint_t)(rand() % 4096 + 4096);
        size = (uint_t)(rand() % 3000 + 16);
        //printf("size = %d\n", size);
        //printf("Before alloc pool avail= %u\n", pool->avail);
        //printf("Before alloc pool lavail= %u\n", pool->lavail);
        p = mem_pool_alloc(pool, size);
        if(NULL == p) {
            printf("error: %lld alloc %p\n", i, p);
            ++fail;
            goto full;
        }
        //printf("success: %d alloc %p\n", i, p);
        //printf("After alloc pool avail= %u\n", pool->avail);
        //printf("After alloc pool lavail= %u\n", pool->lavail);
        mem_pool_free(pool, p);
        //printf("success: %d free %p\n", i, p);
        //printf("After free pool avail= %u\n", pool->avail);
        //printf("After free pool lavail= %u\n", pool->lavail);


        if(pool->lavail != 4194304 || pool->avail != 6291456){
            // 引发段错误，生成core文件
            //char *ptr="linuxers.cn"; 
            //*ptr = 0;
            ++fail;
            goto full;
        }
       
        printf("+++%lld+++\n", i);
        ++i;
        if(i > 100000) {
            break;
        }
        //usleep(800);
    }
    gettimeofday(&endtime,0);

    double timeuse = 1000000*(endtime.tv_sec - starttime.tv_sec) 
            + endtime.tv_usec - starttime.tv_usec;
    timeuse /=1000;

full:
    printf("pool info: \n");
    printf("pool org avail: %d\n", org_avail);
    printf("pool org lavail: %d\n", org_lavail);
    printf("pool avail: %d\n", pool->avail);
    printf("pool lavail: %d\n", pool->lavail);
    printf("fail times: %u\n", fail);
    for(i = 0; i < pool->set_length; ++i) {
        chunk = pool->set_table[i]->chunk_head;
        max_chunk = chunk->avail;
        printf("set[%lld] set max= %u, avail= %u\n", i, 
             pool->set_table[i]->max, pool->set_table[i]->avail);
    }

    printf("cost %lfms\n", timeuse);
err:
    mem_destroy_pool(pool);
    
}

void test_fill_mem_pool()
{
    long long i = 0;
    uint_t size = 16;   
    float ratio = 0.6;
    char *p = NULL, *keep = NULL;
    uint_t fail = 0;
    uint_t org_avail = 0;
    uint_t org_lavail = 0;
    uint_t remain_small_size = 0;
    uint_t remain_large_size = 0;
    memDataChunk *chunk = NULL;
    uint_t max_chunk = 0;
    struct timeval starttime, endtime;

    memPool *pool = mem_create_pool(size, ratio);
    if(NULL == pool) {
        printf("pool: %p\n", pool);
        goto err;
    }

    org_avail = pool->avail;
    org_lavail = pool->lavail;

    srand( (unsigned)time( NULL ) ); 

    gettimeofday(&starttime,0);
    while(1) {
        //printf("**************%lld*****************\n", i);
        //size = (uint_t)(rand() % 8080 + 16);
        //size = (uint_t)(rand() % 4096 + 4096);
        size = (uint_t)(rand() % 3000 + 16);
        printf("size = %d\n", size);
        p = (char *)mem_pool_alloc(pool, size);
        //p = malloc(size);
        if(NULL == p) {
            ++fail;
            goto full;
        }
        if(0 != i %100) {
            printf("success: %lld alloc %p\n", i, p);
            mem_pool_free(pool, p);
            //free(p);
            printf("success: %lld free %p\n", i, p);
        }
        else {
            if(size > ALLOC_LIMIT)
                remain_large_size += size;
            else {
                //if(NULL == keep) {
                    //keep = p;
                    //strncpy(keep, "aaaaa", sizeof("aaaaa"));
                //}
                remain_small_size += size;
            }
        }
        printf("++++++++++++++%lld+++++++++++++++++\n", i);
        //if(remain_large_size >= pool->lavail && remain_small_size >= pool->avail) {
            //printf("mempool is full.\n");
        //    goto full;
        //}
        
        ++i;
        
        if(i > 100000) {
            break;
        }
        //usleep(800);
    }
    gettimeofday(&endtime,0);

    double timeuse = 1000000*(endtime.tv_sec - starttime.tv_sec) 
            + endtime.tv_usec - starttime.tv_usec;
    timeuse /=1000;

    

full:
    printf("pool info: \n");
    printf("pool org avail: %d\n", org_avail);
    printf("pool org lavail: %d\n", org_lavail);
    printf("pool avail: %d\n", pool->avail);
    printf("pool lavail: %d\n", pool->lavail);
    printf("fail times: %u\n", fail);
    printf("remain small size: %u\n", remain_small_size);
    printf("remain large size: %u\n", remain_large_size);
    for(i = 0; i < pool->set_length; ++i) {
        chunk = pool->set_table[i]->chunk_head;
        max_chunk = chunk->avail;
        printf("set[%lld] set max= %u, avail= %u\n", i, 
             pool->set_table[i]->max, pool->set_table[i]->avail);
    }
    //printf("keep: %s\n", keep);

    printf("cost %lfms\n", timeuse);
  
err:
    mem_destroy_pool(pool);
}

int main()
{
    

    
    //test_for_mem_pool();
    test_fill_mem_pool();
    

    return 0;
}
