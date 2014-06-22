#ifndef __MPLIST_H__
#define __MPLIST_H__

#include "mem_pool.h"

typedef struct MpListNode {
    struct MpListNode *next;
    struct MpListNode *prev;
    void *value;
}MpListNode;
#define MpListNodeSize (sizeof(MpListNode))

typedef struct MpListIter {
    MpListNode *next;
    int direction;
}MpListIter;
#define MpListIterSize (sizeof(MpListIter))

typedef struct MpList {
    MpListNode *head;
    MpListNode *tail;
    uint32_t len;
    uint32_t spare0;
    void * (*dup)(void *ptr); //���ƽڵ�ʱ�����õĻص�����
    void (*free)(void *ptr); //ɾ���ڵ�ʱ�����õĻص�����
    int (*match)(void *ptr, void *key); //�ԱȽڵ�ʱ�����õĻص�����
}MpList;
#define MpListSize (sizeof(MpList))

#define mplist_len(mpl) ((mpl)->len)   //��������Ľڵ�����
#define mplist_head(mpl) ((mpl)->head) //��������ı�ͷ�ڵ�
#define mplist_tail(mpl) ((mpl)->tail) //��������ı�β�ڵ�

#define mplist_prev_node(mpn) ((mpn)->prev)   //���ظ����ڵ��ǰһ���ڵ�
#define mplist_next_node(mpn) ((mpn)->next)   //���ظ����ڵ�ĺ�һ���ڵ�
#define mplist_node_value(mpn) ((mpn)->value) //���ظ����ڵ��ֵ

//�����б��dup��free��match����
#define mplist_set_dup_cb(mpl, m) ((mpl)->dup = (m))
#define mplist_set_free_cb(mpl, m) ((mpl)->free = (m))
#define mplist_set_match_cb(mpl, m) ((mpl)->match = (m))

//�����б��dup��free��match����
#define mplist_get_dup_cb(mpl) ((mpl)->dup)
#define mplist_get_free_cb(mpl) ((mpl)->free)
#define mplist_get_match_cb(mpl) ((mpl)->match)

/*Prototypes */
MpList *mplist_create(MemPool *pool);
void mplist_destroy(MemPool *pool, MpList *list);
MpList *mplist_add_node_head(MemPool *pool, MpList *list, void *value);
MpList *mplist_add_node_tail(MemPool *pool, MpList *list, void *value);


MpList *listInsertNode(MpList *list, MpListNode *old_node, void *value, int after);
void listDelNode(MpList *list, MpListNode *node);
MpListIter *listGetIterator(MpList *list, int direction);
MpListNode *listNext(MpListIter *iter);
void listReleaseIterator(MpListIter *iter);
MpList *listDup(MpList *orig);
MpListNode *listSearchKey(MpList *list, void *key);
MpListNode *listIndex(MpList *list, long index);
void listRewind(MpList *list, MpListIter *li);
void listRewindTail(MpList *list, MpListIter *li);
void listRotate(MpList *list);

/* Directions for iterators */
#define MPL_START_HEAD 0
#define MPL_START_TAIL 1

#endif