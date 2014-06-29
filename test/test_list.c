#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../list.h"

#define TEST_TIMES 20
uint32_t add_count = 0, del_count = 0;
uint32_t sleep_time = 5000;
#define sleep(n) usleep(1000*(n))


void * add_node_to_list(void *param) {
    list_t *list = (list_t *)param;
    list_node_t *node = (list_node_t *)malloc(list_node_size);
    uint32_t n = __sync_fetch_and_sub(&sleep_time, 250);
    sleep(n);

    list_tail_push_node(list, node);
    
    __sync_fetch_and_add(&add_count, 1);

    return NULL;
}

void * del_node_from_list(void *param) {
    list_t *list = (list_t *)param;
    list_node_t *node = NULL;
    uint32_t n = __sync_fetch_and_sub(&sleep_time, 250);
    sleep(n);
    
    node = list_head_pop_node(list);
    if(node)
        __sync_fetch_and_add(&del_count, 1);

    return NULL;
}

pthread_t pids[TEST_TIMES];
void test_list()
{
    uint32_t i = 0;
    struct timeval starttime, endtime;
    double timeuse;
    list_t *list;
    MemPool *pool = mem_pool_create(640); //640M
    if(!pool) goto done;

    list = (list_t *)mem_pool_alloc(pool, list_size);
    if(!list) goto done;

    gettimeofday(&starttime,0);
    for(i = 0; i < TEST_TIMES; ++i) {
        if(1) 
            pthread_create(&pids[i], NULL, add_node_to_list, list);
        else
            pthread_create(&pids[i], NULL, del_node_from_list, list);
            
        sleep(250);
    }

    for(i = 0; i < TEST_TIMES; ++i) {
        sleep(2);
        pthread_join(pids[i], NULL);
    }

    gettimeofday(&endtime,0);

done:
    mem_pool_free(pool, list);
    mem_pool_destroy(pool);

    timeuse = 1000000*(endtime.tv_sec - starttime.tv_sec) 
            + endtime.tv_usec - starttime.tv_usec;
    timeuse /=1000;

    printf("cost %lfms\n", timeuse);
}


int main()
{
    test_list();

    return 0;
}
