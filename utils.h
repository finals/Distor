#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

/* 去掉编译器对木有使用的局部变量的警告 */
#define DISTOR_NOTUSED(V) ((void) V)


/* -------------------------- hash functions -------------------------------- */
uint32_t generate_integer_hash(uint32_t key);
uint32_t generate_identity_hash(uint32_t key);
void set_hash_function_seed(uint32_t seed);
uint32_t get_hash_function_seed(void);
uint32_t generate_hash(const void *key, int len);
uint32_t generate_case_hash(const unsigned char *buf, int32_t len);

#endif /* __UTILS_H__ */
