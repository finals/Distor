#ifndef __MPS_H__
#define __MPS_H__

#include "mem_pool.h"

typedef char * mps;

typedef struct mps_s {
    uint32_t len;      // mps��ǰʹ�ô�С
    uint32_t max_len;  // mps��������ڴ��С�������Ŀ��ÿռ�
    char buf[0]; 
}MpsHdr;
#define MpsHdrSize (sizeof(struct mps_s))

#define mps_get_MpsHdr(s) ((MpsHdr *)((char *)s - MpsHdrSize))

//mps�ĳ���
static inline uint32_t mps_len(const mps s)
{
   return (*(uint32_t *)((char *)s - MpsHdrSize));
}

static inline void mps_set_len(const mps s, uint32_t len)
{
    MpsHdr *dh = mps_get_MpsHdr(s);
    dh->len = len;
}

//mps����󳤶�
static inline uint32_t mps_max_len(const mps s)
{
    return ((MpsHdr *)((char *)s - MpsHdrSize))->max_len;
}

mps mps_new_len(MemPool *pool, const char *init, uint32_t len);  //���ݳ��ȴ���mps
mps mps_new(MemPool *pool, const char *init);   //���ݳ�ʼ���ַ�������mps
mps mps_dup(MemPool *pool, const mps s); //����mps
void mps_free(MemPool *pool, mps s);      //�ͷ�mps�ṹ

// ������ len ��չ mps ������ t ƴ�ӵ� sds ��ĩβ
mps mps_cat_len(MemPool *pool, mps s, const char *t, uint32_t len);
// ��һ�� char ����ƴ�ӵ� mps ĩβ 
mps mps_cat(MemPool *pool, mps s, const char *t);

//�����ַ�����mps
mps mps_copy_len(MemPool *pool, mps s, const char *t, uint32_t len);
mps mps_copy(MemPool *pool, mps s, const char *t);

//�Ƚ�mps
int32_t mps_cmp(mps s1, mps s2);
int32_t mps_cmp_len(mps s1, mps s2, uint32_t len);

//��Сдת��
void mps_tolower(mps s);
void mps_toupper(mps s);

//ȥ��mps��ʼ�ͽ���λ�õ��ַ�c
void mps_split(mps s, char c);


#endif
