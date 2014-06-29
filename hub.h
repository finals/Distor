#ifndef __HUB_H__
#define __HUB_H__

/* �����¼�״̬ */
#define EVT_NONE    0    /* δ���� */
#define EVT_READ    1    /* �ɶ� */
#define EVT_WRITE   2    /* ��д */

/* �¼�ִ��״̬ */
#define EVT_OK      0
#define EVT_ERR    -1

/* �¼�ִ�б�ʶ */
#define EVT_NET       1     /* �����¼� */
#define EVT_TIME      2     /* ��ʱ�¼� */
#define EVT_ALL       (EVT_NET | EVT_TIME)
#define EVT_DONT_WAIT 4
#define EVT_PERM      8     /* �¼��Ƿ����ִ�� */

struct evtHub;

typedef void (*hubNetEventProc)(struct evtHub *hub, int fd, void *data, int mode);
typedef int  (*hubTimeEventProc)(struct evtHub *hub, void *data);
typedef void (*hubBeforeEventProc)(struct evtHub *hub);

/* �����¼� */
typedef struct hubNetEvent {
    int mode;     /* ���¼���д�¼� */
    hubNetEventProc rnetProc;  /* ���¼������� */
    hubNetEventProc wnetProc;  /* д�¼������� */
    void *data; /* �������Ĳ��� */
}hubNetEvent;

/* �Ѿ����������¼��� accept֮�󣬿��Զ�д���¼� */
typedef struct hubReadyEvent {
    int fd;     /* �Ѿ����ļ������� */
    int mode;   /* ���¼���д�¼� */
}hubReadyEvent;

/* ��ʱ�¼� */
typedef struct hubTimeEvent {
    long long id;       /* �¼�Ψһ��ʶ */
    long seconds;       /* �¼���ʱʱ��, ʱ��������� */
    long milliseconds;  /* ʱ��������� */
    hubTimeEventProc timeProc;  /* ��ʱ������ */ 
    void *data;         /* �������Ĳ��� */ 
    struct hubTimeEvent *next;  /* ��һ����ʱ�¼� */
    struct hubTimeEvent *prev;  /* ��һ����ʱ�¼� */
}hubTimeEvent;

typedef struct evtHub {
    int maxfd;          /* ��hub������fd */
    int evtsize;        /* ��hub����ע����¼����� */
    time_t last_time;   /* �ϴδ���ʱ�¼���ʱ�� */
    long long time_event_id;         /* ��ʱ�¼���id������ */
    hubNetEvent   *net_events;       /* �����¼����� */
    hubReadyEvent *rdy_events;       /* �Ѿ����������¼����� */
    hubTimeEvent  *time_event_head;  /* ��ʱ�¼�����ͷ�� */
    hubTimeEvent  *time_event_tail;  /* ��ʱ�¼�����β�� */
    int stop;           /* �¼������� */
    void *multipexer;   /* hubʹ�õĶ�·������ */
    hubBeforeEventProc beforeProc;   /* �����¼�ǰҪִ�еĺ��� */
}evtHub;

/* hub��غ��� */
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
