#include <ctype.h>
#include "utils.h"

/* -------------------------- hash functions -------------------------------- */
/* Thomas Wang's 32 bit Mix Function */
uint32_t generate_integer_hash(uint32_t key)
{
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}

/* Identity hash function for integer keys */
uint32_t generate_identity_hash(uint32_t key)
{
    return key;
}

static uint32_t hash_function_seed = 5381;

void set_hash_function_seed(uint32_t seed) 
{
    hash_function_seed = seed;
}

uint32_t get_hash_function_seed(void) 
{
    return hash_function_seed;
}

/* MurmurHash2, by Austin Appleby
 * Note - This code makes a few assumptions about how your machine behaves -
 * 1. We can read a 4-byte value from any address without crashing
 * 2. sizeof(int) == 4
 *
 * And it has a few limitations -
 *
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian
 *    machines.
 *
 * 算法的具体信息可以参考 http://code.google.com/p/smhasher/
 */
uint32_t generate_hash(const void *key, int32_t len)
{
    /* 'm' and 'r' are mixing constants generated offline.
     They're not really 'magic', they just happen to work well.  */
    uint32_t seed = hash_function_seed;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (uint32_t)h;
}

/* And a case insensitive hash function (based on djb hash) */
uint32_t generate_case_hash(const unsigned char *buf, int len) 
{
    uint32_t hash = (uint32_t)hash_function_seed;

    while (len--)
        hash = ((hash << 5) + hash) + (tolower(*buf++)); /* hash * 33 + c */
    return hash;
}

