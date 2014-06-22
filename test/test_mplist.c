#include <stdio.h>

#include "../mplist.h"

MemPool *pool = NULL;
#define TEST_TIMES 10

void test_mplist_create()
{
    MpList *list = NULL;
    pool = mem_pool_create(160);

   list = mplist_create(pool);

   //mplist_destroy(pool, list);

   mem_pool_destroy(pool);
}

void test_mplist_add_node()
{
    MpList *list = NULL;
    MpListNode *node = NULL;
    uint32_t i = 0;
    uint32_t *value = NULL;
    
    pool = mem_pool_create(160);
    if(!pool) goto err;

    list = mplist_create(pool);
    if(!list) goto err;

    value = mem_pool_alloc(pool, sizeof(uint32_t)
#if MEM_POOL_DEBUG
        , "test_mplist_add_node"
#endif
    );

    for(i = 0; i < TEST_TIMES; ++i) {
        *value = i;
        if((i & 0xFFFFFFF2) == 0) {
            mplist_add_node_head(pool, list, value);
        }
        else {
            mplist_add_node_tail(pool, list, value);
        }
    }

    for(i = 0, node = list->head; i < list->len; ++i, node=node->next) { 
        *value = i;
        if((i & 0xFFFFFF10) == 0) {
            printf("%d\n", *((int *)(node)->value));
        }
        else {
            printf("%d\n", *((int *)(node)->value));
        }
    }

err:

   mplist_destroy(pool, list);

   mem_pool_destroy(pool);
}

int main()
{
    //test_mplist_create();
    test_mplist_add_node();

    return 0;
}
