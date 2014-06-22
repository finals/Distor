#ifndef __MPS_H__
#define __MPS_H__

#include "mem_pool.h"

typedef char * mps;

typedef struct mps_s {
    uint32_t len;      // mps当前使用大小
    uint32_t max_len;  // mps所分配的内存大小，即最大的可用空间
    char buf[0]; 
}MpsHdr;
#define MpsHdrSize (sizeof(struct mps_s))

#define mps_get_MpsHdr(s) ((MpsHdr *)((char *)s - MpsHdrSize))

//mps的长度
static inline uint32_t mps_len(const mps s)
{
   return (*(uint32_t *)((char *)s - MpsHdrSize));
}

static inline void mps_set_len(const mps s, uint32_t len)
{
    MpsHdr *dh = mps_get_MpsHdr(s);
    dh->len = len;
}

//mps的最大长度
static inline uint32_t mps_max_len(const mps s)
{
    return ((MpsHdr *)((char *)s - MpsHdrSize))->max_len;
}

mps mps_new_len(MemPool *pool, const char *init, uint32_t len);  //根据长度创建mps
mps mps_new(MemPool *pool, const char *init);   //根据初始化字符串创建mps
mps mps_dup(MemPool *pool, const mps s); //复制mps
void mps_free(MemPool *pool, mps s);      //释放mps结构

// 按长度 len 扩展 mps ，并将 t 拼接到 sds 的末尾
mps mps_cat_len(MemPool *pool, mps s, const char *t, uint32_t len);
// 将一个 char 数组拼接到 mps 末尾 
mps mps_cat(MemPool *pool, mps s, const char *t);

//拷贝字符串到mps
mps mps_copy_len(MemPool *pool, mps s, const char *t, uint32_t len);
mps mps_copy(MemPool *pool, mps s, const char *t);

//比较mps
int32_t mps_cmp(mps s1, mps s2);
int32_t mps_cmp_len(mps s1, mps s2, uint32_t len);

//大小写转换
void mps_tolower(mps s);
void mps_toupper(mps s);

//去除mps开始和结束位置的字符c
void mps_split(mps s, char c);


#endif
