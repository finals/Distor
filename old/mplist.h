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
    void * (*dup)(void *ptr); //复制节点时，调用的回调函数
    void (*free)(void *ptr); //删除节点时，调用的回调函数
    int (*match)(void *ptr, void *key); //对比节点时，调用的回调函数
}MpList;
#define MpListSize (sizeof(MpList))

#define mplist_len(mpl) ((mpl)->len)   //返回链表的节点数量
#define mplist_head(mpl) ((mpl)->head) //返回链表的表头节点
#define mplist_tail(mpl) ((mpl)->tail) //返回链表的表尾节点

#define mplist_prev_node(mpn) ((mpn)->prev)   //返回给定节点的前一个节点
#define mplist_next_node(mpn) ((mpn)->next)   //返回给定节点的后一个节点
#define mplist_node_value(mpn) ((mpn)->value) //返回给定节点的值

//设置列表的dup、free和match函数
#define mplist_set_dup_cb(mpl, m) ((mpl)->dup = (m))
#define mplist_set_free_cb(mpl, m) ((mpl)->free = (m))
#define mplist_set_match_cb(mpl, m) ((mpl)->match = (m))

//返回列表的dup、free和match函数
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