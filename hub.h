#ifndef __HUB_H__
#define __HUB_H__

/* 网络事件状态 */
#define EVT_NONE    0    /* 未设置 */
#define EVT_READ    1    /* 可读 */
#define EVT_WRITE   2    /* 可写 */

/* 事件执行状态 */
#define EVT_OK      0
#define EVT_ERR    -1

/* 事件执行标识 */
#define EVT_NET       1     /* 网络事件 */
#define EVT_TIME      2     /* 定时事件 */
#define EVT_ALL       (EVT_NET | EVT_TIME)
#define EVT_DONT_WAIT 4
#define EVT_PERM      8     /* 事件是否持续执行 */

struct evtHub;

typedef void (*hubNetEventProc)(struct evtHub *hub, int fd, void *data, int mode);
typedef int  (*hubTimeEventProc)(struct evtHub *hub, void *data);
typedef void (*hubBeforeEventProc)(struct evtHub *hub);

/* 网络事件 */
typedef struct hubNetEvent {
    int mode;     /* 读事件或写事件 */
    hubNetEventProc rnetProc;  /* 读事件处理函数 */
    hubNetEventProc wnetProc;  /* 写事件处理函数 */
    void *data; /* 处理函数的参数 */
}hubNetEvent;

/* 已就绪的网络事件， accept之后，可以读写的事件 */
typedef struct hubReadyEvent {
    int fd;     /* 已就绪文件描述符 */
    int mode;   /* 读事件或写事件 */
}hubReadyEvent;

/* 定时事件 */
typedef struct hubTimeEvent {
    long long id;       /* 事件唯一标识 */
    long seconds;       /* 事件定时时间, 时间戳的秒数 */
    long milliseconds;  /* 时间戳毫秒数 */
    hubTimeEventProc timeProc;  /* 定时处理函数 */ 
    void *data;         /* 处理函数的参数 */ 
    struct hubTimeEvent *next;  /* 下一个定时事件 */
    struct hubTimeEvent *prev;  /* 上一个定时事件 */
}hubTimeEvent;

typedef struct evtHub {
    int maxfd;          /* 该hub中最大的fd */
    int evtsize;        /* 该hub允许注册的事件数量 */
    time_t last_time;   /* 上次处理定时事件的时间 */
    long long time_event_id;         /* 定时事件的id，递增 */
    hubNetEvent   *net_events;       /* 网络事件数组 */
    hubReadyEvent *rdy_events;       /* 已就绪的网络事件数组 */
    hubTimeEvent  *time_event_head;  /* 定时事件链表头部 */
    hubTimeEvent  *time_event_tail;  /* 定时事件链表尾部 */
    int stop;           /* 事件处理开关 */
    void *multipexer;   /* hub使用的多路复用器 */
    hubBeforeEventProc beforeProc;   /* 处理事件前要执行的函数 */
}evtHub;

/* hub相关函数 */
evtHub *hub_create(int evtsize);
void    hub_delete(evtHub *hub);
void    hub_stop(evtHub *hub);

int     hub_create_net_event(evtHub *hub, int fd, int mode,
           hubNetEventProc proc, void *data);
int     hub_delete_net_event(evtHub *hub, int fd, int mode);
hubNetEvent * hub_get_net_event(evtHub *hub, int fd);

long long  hub_create_time_event(evtHub *hub, long long milliseconds,
                 hubTimeEventProc proc, void *data);
hubTimeEvent* hub_get_Time_event(evtHub *hub, long long id);
int     hub_delete_time_event(evtHub *hub, long long id);

void    hub_main(evtHub *hub);
char *  hub_get_multipexer_name(evtHub *hub);

#endif
