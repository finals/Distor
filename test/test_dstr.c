#include <stdio.h>
#include <time.h> 
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../mps.h"
#include "../mem_pool.h"

MemPool *pool = NULL;
#define TEST_TIMES 1000

void test_mps_len()
{
    MpsHdr * dh = NULL;
    mps s = NULL;
    uint32_t i = 0, size=0;

    pool = mem_pool_create(160);
    srand((unsigned)time( NULL )); 

    for(i = 0; i < TEST_TIMES; ++i) {
        size = rand() % 50+16;
        dh = (MpsHdr *)mem_pool_alloc(pool, size
#if MEM_POOL_DEBUG
            , "Dstr"
#endif
        );
        dh->len = size;
        dh->max_len = mem_chunk_get_data_size((void *)dh);
        s = dh->buf;
        printf("%7u: len: %u  max_len: %u\n", i, dh->len, dh->max_len);
        printf("%7u: len: %u  max_len: %u set_pos: %d\n", i, mps_len(s), mps_max_len(s), 
            mem_chunk_get_chunk(dh)->set_pos);
        assert((dh->len == mps_len(s)) && (dh->max_len == mps_max_len(s)));
    }
}

void test_mps_new_len()
{
    mps s = NULL;
    char str[] = "abcdefghijklmnopqrstuvwxyz";

    pool = mem_pool_create(160);
    srand((unsigned)time( NULL )); 
    s = mps_new_len(pool, str, sizeof(str));
    printf("%s len: %u max_len: %u\n", s, mps_len(s), mps_max_len(s));

    mps_free(pool, s);
    mem_pool_destroy(pool);
}

void test_mps_cat_len()
{
    mps s = NULL, s_dup1 = NULL, s_dup2 = NULL;
    char str1[] = "abcdefghijklmnopqrstuvwxyz";
    char str2[8] = "cccccccc";
    char str3[256];
    memset(str3, 'a', 128);
    memset(str3+128, 'b', 128);
    
    pool = mem_pool_create(160);
    srand((unsigned)time( NULL )); 
    s = mps_new_len(pool, str1, sizeof(str1));
    printf("%s len: %u max_len: %u\n", s, mps_len(s), mps_max_len(s));

    s_dup1 = mps_cat(pool, s, str2);
    printf("%s len: %u max_len: %u s: %p s_dup1: %p\n", s_dup1, mps_len(s_dup1), 
        mps_max_len(s_dup1), s, s_dup1);

    s_dup2 = mps_cat(pool, s, str3);
    printf("%s len: %u max_len: %u s: %p s_dup2: %p\n", s_dup2, mps_len(s_dup2), 
        mps_max_len(s_dup2), s, s_dup2);

    mps_free(pool, s_dup2);
    mem_pool_destroy(pool);
}

void test_mps_copy()
{
    mps s = NULL;
    char str1[] = "abcdefghijklmnopqrstuvwxyz";
    char str2[9] = "cccccccc";
    char str3[256];
    memset(str3, 'a', 128);
    memset(str3+128, 'b', 126);
    str3[255] = 0;
    
    pool = mem_pool_create(160);
    s = mps_new_len(pool, str1, sizeof(str1));
    printf("%s len: %u max_len: %u s: %p\n", s, mps_len(s), mps_max_len(s), s);
    s = mps_copy(pool, s, str2);
    printf("%s len: %u max_len: %u s: %p\n", s, mps_len(s), mps_max_len(s), s);
    s = mps_copy(pool, s, str3);
    printf("%s len: %u max_len: %u s: %p\n", s, mps_len(s), mps_max_len(s), s);

    mem_pool_destroy(pool);
}

void test_mps_cmp()
{
    mps s1 = NULL, s2 = NULL, s3 = NULL, s4 = NULL;
    char str1[] = "abcdefg";
    char str2[] = "abcdefgh";
    char str3[] = "abcdef";
    char str4[] = "abcdefg";
    
    pool = mem_pool_create(160);
    s1 = mps_new_len(pool, str1, sizeof(str1));
    s2 = mps_new_len(pool, str2, sizeof(str2));
    s3 = mps_new_len(pool, str3, sizeof(str3));
    s4 = mps_new_len(pool, str4, sizeof(str4));

    printf("s1: %s len %u  s2: %s len %u  cmp: %d\n", s1, mps_len(s1), 
        s2, mps_len(s2), mps_cmp(s1, s2));
    printf("s1: %s len %u  s3: %s len %u  cmp: %d\n", s1, mps_len(s1), 
        s3, mps_len(s3), mps_cmp(s1, s3));
    printf("s1: %s len %u  s4: %s len %u  cmp: %d\n", s1, mps_len(s1), 
        s4, mps_len(s4), mps_cmp(s1, s4));


    printf("s1: %s len %u  s2: %s len %u  cmp: %d\n", s1, mps_len(s1), 
        s2, mps_len(s2), mps_cmp_len(s1, s2, 7));
    printf("s1: %s len %u  s3: %s len %u  cmp: %d\n", s1, mps_len(s1), 
        s3, mps_len(s3), mps_cmp_len(s1, s3, 6));

       
    mps_free(pool, s1);
    mps_free(pool, s2);
    mps_free(pool, s3);
    mps_free(pool, s4);
    
    mem_pool_destroy(pool);
}

void test_tolower_toupper()
{
    mps s1 = NULL, s2 = NULL;
    char str1[] = "aBcdEfg";
    char str2[] = "abCdEFGEFFEDJUIKJQWEabc";

    pool = mem_pool_create(160);
    s1 = mps_new_len(pool, str1, sizeof(str1));
    s2 = mps_new_len(pool, str2, sizeof(str2));

    printf("%s len: %u max_len: %u\n", s1, mps_len(s1), mps_max_len(s1));
    mps_tolower(s1);
    printf("%s len: %u max_len: %u\n", s1, mps_len(s1), mps_max_len(s1));

    printf("%s len: %u max_len: %u\n", s2, mps_len(s2), mps_max_len(s2));
    mps_toupper(s2);
    printf("%s len: %u max_len: %u\n", s2, mps_len(s2), mps_max_len(s2));


    //mps_free(pool, s1);
    //mps_free(pool, s2);
    
    mem_pool_destroy(pool);
}

void test_mps_split()
{
    mps s1 = NULL, s2 = NULL;
    char str1[] = "##aaa ";
    char str2[] = "aaabbbbbbbbabbaabba";

    pool = mem_pool_create(160);
    s1 = mps_new_len(pool, str1, sizeof(str1));
    s2 = mps_new_len(pool, str2, sizeof(str2));

    printf("%s len: %u max_len: %u\n", s1, mps_len(s1), mps_max_len(s1));
    mps_split(s1, '#');
    printf("%s len: %u max_len: %u\n", s1, mps_len(s1), mps_max_len(s1));

    printf("%s len: %u max_len: %u\n", s2, mps_len(s2), mps_max_len(s2));
    mps_split(s2, 'a');
    printf("%s len: %u max_len: %u\n", s2, mps_len(s2), mps_max_len(s2));


    //mps_free(pool, s1);
    //mps_free(pool, s2);
    
    mem_pool_destroy(pool);
}

int main()
{
    test_mps_new_len();
    test_mps_cat_len();
    test_mps_copy();
    test_mps_cmp();
    test_tolower_toupper();
    test_mps_split();
    
    return 0;
}
