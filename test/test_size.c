#include <stdio.h>
#include "../mem_pool.h"

int main()
{
    //printf("%d\n",sizeof(memPool));
    //printf("%d\n", mem_align(sizeof(memPool)+CHUNK_SIZE, MEM_POOL_ALIGNMENT));
    //printf("%d\n", sizeof(int));

/*
    memLargeData *p = malloc(sizeof(memLargeData) + 1024);
    printf("%p\n", p);
    printf("%p\n", p->begin);
    printf("%p\n", (struct memLargeData *)p->begin);
    printf("%d\n", sizeof(memLargeData));
    printf("%d\n", p->begin - (uchar_t *)p);
 */

   // memPool *pool = mem_create_pool(100, 0.6);
   // getchar();

#ifdef linux
    printf("aaaaaaaaaaaaaa\n");
#else
    printf("bbbbbbbbbbbbbb\n");
#endif
    return 0;
}
