#include <string.h>
#include <ctype.h>

#include "mps.h"

//根据长度创建mps
mps mps_new_len(MemPool *pool, const char *init, uint32_t len)
{
    MpsHdr *dh = mem_pool_alloc(pool, len + MpsHdrSize
#if MEM_POOL_DEBUG
        , "MPS"
#endif
    );
    if(!dh) {
        goto err;
    }
    if(init[len] == '\0') 
        --len;
    dh->len = len;
    dh->max_len = mem_chunk_get_data_size(dh) - MpsHdrSize;

    if(init) {
        memcpy(dh->buf, init, len);    
    }
    return (mps)dh->buf;
err:
    return NULL;
}

//根据初始化字符串创建mps
mps mps_new(MemPool *pool, const char *init)
{
    uint32_t initlen = (init == NULL) ? 0 : strlen(init);
    return mps_new_len(pool, init, initlen);
}

//复制mps
mps mps_dup(MemPool *pool, const mps s)
{
    uint32_t duplen = mps_len(s);
    return mps_new_len(pool, (char *)s, duplen);
}

//释放mps结构
void mps_free(MemPool *pool, mps s)
{
    MpsHdr *dh = mps_get_MpsHdr(s);
    mem_pool_free(pool, dh);
}

// 按长度 len 扩展 mps ，并将 t 拼接到 sds 的末尾
mps mps_cat_len(MemPool *pool, mps s, const char *t, uint32_t len)
{
    uint32_t s_len = mps_len(s);
    uint32_t new_len = s_len + len;
    mps new_s = s;
    MpsHdr *dh = NULL;

    if(new_len > mps_max_len(s)) {
        new_s = mps_new_len(pool, s, new_len);
        if(!new_s) {
            goto err;
        }
        mps_free(pool, s);
    }

    memcpy(new_s + s_len, t, len);
    dh = mps_get_MpsHdr(s);
    dh->len = len;
    return new_s;

err:
    return NULL;
}

// 将一个 char 数组拼接到 mps 末尾 
mps mps_cat(MemPool *pool, mps s, const char *t)
{
    uint32_t t_len = (t == NULL) ? 0 : strlen(t);
    return mps_cat_len(pool, s, t, t_len);
}

//拷贝字符串到mps
mps mps_copy_len(MemPool *pool, mps s, const char *t, uint32_t len)
{
    uint32_t s_max_len = mps_max_len(s);
    mps new_s = s;
    MpsHdr * dh = NULL;

    if(s_max_len < len) {
        new_s = mps_new_len(pool, t, len);
        if(!new_s) return NULL;
        mps_free(pool, s);
    }
    else {
        memcpy(new_s, t, len);
    }
    new_s[len+1] = '\0';
    dh = mps_get_MpsHdr(s);
    dh->len = len;
    return new_s;
}

mps mps_copy(MemPool *pool, mps s, const char *t)
{
    uint32_t t_len = (t == NULL) ? 0 : strlen(t);
    return mps_copy_len(pool, s, t, t_len);
}

//比较mps, 按照字节比较
int32_t mps_cmp(mps s1, mps s2)
{
    uint32_t s1_len = 0, s2_len = 0, min_len = 0;
    int32_t cmp;

    s1_len = mps_len(s1);
    s2_len = mps_len(s2);
    min_len = s1_len > s2_len ? s2_len : s1_len;

    cmp = memcmp(s1, s2, min_len);
    if(0 == cmp) {
        return s1_len - s2_len;
    }
    return cmp;    
}

//比较两个mps的前len个字节
int32_t mps_cmp_len(mps s1, mps s2, uint32_t len)
{
    uint32_t s1_len = mps_len(s1), 
             s2_len = mps_len(s2);
    int32_t cmp;

    if(s1_len < len && s2_len < len) {
        return -2;
    }

    cmp = memcmp(s1, s2, len);
    return cmp;  
}

//大小写转换
void mps_tolower(mps s)
{
   int len = mps_len(s), j;

    for (j = 0; j < len; j++) 
        s[j] = tolower(s[j]); 
}

void mps_toupper(mps s)
{
    int len = mps_len(s), j;

    for (j = 0; j < len; j++) 
        s[j] = toupper(s[j]);
}

//去除mps开始和结束位置的字符c
void mps_split(mps s, char c)
{
    uint32_t s_len = mps_len(s), start= 0, end = s_len-1, i = 0;

    while( start <= end) {
        if(s[start] == c) ++start;
        else break;
    }
    while(start <= end) {
        if(s[end] == c) --end;
        else break;
    }
    
    mps_set_len(s, end-start+1);
    
    if(start != 0) {
        for(i = 0; start <= end; ++i, ++start) {
            s[i] = s[start];
        }
    }
}
