#include <stdio.h>
#include <string.h>

#include "../keyvalue.h"
#include "../utils.h"

void test_key_value()
{
    MemPool *pool = NULL;
    key_value_t *kv = NULL;
    key_value_node_t *node = NULL;
    char *keys[5], *vals[5];
    
    pool = mem_pool_create(160); //160M
    kv = key_value_init(pool, 20);

    keys[0] = "abcde";
    vals[0] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    key_value_set(pool, kv, "abcde", strlen("abcde"), 
        vals[0], strlen(vals[0]), 0);
        
    keys[1] = "qwerty";
    vals[1] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    key_value_set(pool, kv, keys[1], strlen(keys[1]), 
        vals[1], strlen(vals[1]), 0);

    keys[2] = "asdf";
    vals[2] = "cccccccccccc";
    key_value_set(pool, kv, keys[2], strlen(keys[2]), 
        vals[2], strlen(vals[2]), 0);

    node = key_value_get(kv, keys[2], strlen(keys[2]));
    printf("get %s node data: %s\n", keys[2], (char *)node->data);

    keys[3] = "asdf";
    vals[3] = "dddddddddddddddddddddddddddddddddddddddddddddddddddd";
    key_value_set(pool, kv, keys[3], strlen(keys[3]), 
        vals[3], strlen(vals[3]), 0);

    node = key_value_get(kv, keys[3], strlen(keys[3]));
    printf("get %s node data: %s\n", keys[3], (char *)node->data);

    keys[4] = "lkjhgfdsa";
    vals[4] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
    key_value_set(pool, kv, keys[4], strlen(keys[4]), 
        vals[4], strlen(vals[4]), 0);

    node = key_value_get(kv, "aaaa", strlen("aaaa"));
    if(node)
        printf("get aaaa node data: %s\n", (char *)node->data);
    else
        printf("get aaaa node data: Not Found\n");


    printf("Key Value: used memory: %u, mem pool used memory: %lu \n", 
        kv->used_memory, pool->used_size);
    printf("Key-Value Pairs number: %u\n", kv->count);
    key_value_destroy(pool, kv);
    mem_pool_destroy(pool);
}

int main()
{

    test_key_value();
    //printf("key value node size %lu\n", KeyValueNodeSize);
    //printf("key value size: %lu\n", KeyValueSize);

//    unsigned int i = 0;
    //char *s = "abcedfge";
    //for(i = 0; i < 10; ++i) {
      //  printf("%s hash: %u\n", s, generate_hash(s, sizeof(s)));
  //  }
 
    return 0;
}
