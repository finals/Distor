#include "config.h"
#ifdef HAVE_EPOLL
#include "hub_epoll.h"
#else
#include "hub_select.h"
#endif
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

static void get_current_time(long *seconds, long *milliseconds)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec/1000;
}

evtHub *hub_create(int evtsize)
{
    evtHub *event_hub;
    int i;
    
    /* ����evtHub�ṹ�ռ� */
    event_hub = malloc(sizeof(evtHub));
    if(NULL == event_hub) {
        goto err;
    }

    /* ���������¼�����;����¼�����ռ� */
    event_hub->net_events = malloc(sizeof(hubNetEvent) * evtsize);
    event_hub->rdy_events = malloc(sizeof(hubReadyEvent) *evtsize);
    if(NULL == event_hub->rdy_events || NULL == event_hub->net_events) { 
        goto err;
    }

    /* ��ʼ��evtHub���ֶ� */
    event_hub->maxfd = -1;
    event_hub->evtsize = evtsize;
    event_hub->time_event_id = 0;
    event_hub->last_time = time(NULL);
    event_hub->stop = -1;
    event_hub->beforeProc = NULL;

    /* ��ʼ����ʱ���¼�����ͷ */
    event_hub->time_event_head = NULL;
    event_hub->time_event_tail = NULL;

    /* ����IO��·������ */
    if(MULTIPEX_ERR == evt_multipexer_create(event_hub)) {
        goto err;
    }

    /* ��ʼ�������¼� */
    for(i = 0; i < evtsize; ++i)
        event_hub->net_events[i].mode = EVT_NONE;

    return event_hub;
    
err:
    if(event_hub) {
        free(event_hub->net_events);
        free(event_hub->rdy_events);
        free(event_hub);
    }
    return NULL;
}

void hub_delete(evtHub *hub)
{
    if(NULL == hub) 
        return;

    evt_multipexer_free(hub);
    free(hub->net_events);
    free(hub->rdy_events);
    free(hub);
}

void hub_stop(evtHub *hub)
{
    hub->stop = 1;
}

int hub_create_net_event(evtHub *hub, int fd, int mode,
        hubNetEventProc proc, void *data)
{
    hubNetEvent *ne = NULL;
    if(fd > hub->evtsize) 
        return EVT_ERR;
    ne = &hub->net_events[fd];

    if(MULTIPEX_ERR == evt_event_add(hub, fd, mode)) {
        return EVT_ERR;
    }

    ne->mode |= mode;
    if(mode & EVT_READ) 
        ne->rnetProc = proc;
    if(mode & EVT_WRITE)
        ne->wnetProc = proc;

    ne->data = data;

    if(fd > hub->maxfd)
        hub->maxfd = fd;

    return EVT_OK;
}

int hub_delete_net_event(evtHub *hub, int fd, int mode)
{
    int i;
    hubNetEvent *ne = NULL;
    if(fd > hub->maxfd) {
        return EVT_ERR;
    }
    ne = &hub->net_events[fd];

    ne->mode = ne->mode & (~mode); /* ȡ��fd��ָ����mode */
    if(!(ne->mode & EVT_READ)) 
        ne->rnetProc = NULL;
    if(!(ne->mode & EVT_WRITE)) 
        ne->wnetProc = NULL;

    /* ����maxfd */
    if(ne->mode == EVT_NONE && fd == hub->maxfd) {
        for(i = fd-1; i >= 0; --i)
            if(EVT_NONE != hub->net_events[i].mode) 
                break;
        hub->maxfd = i;
    }

    /* ȡ��fd��mode�¼� */
    evt_event_del(hub, fd, mode);
    
    return EVT_OK;
}

hubNetEvent * hub_get_net_event(evtHub *hub, int fd)
{
    hubNetEvent *ne = NULL;
    if(fd > hub->maxfd) 
        return NULL;

    ne = &hub->net_events[fd];

    if(ne->mode == EVT_NONE)
        return NULL;

    return ne;
}

long long hub_create_time_event(evtHub *hub, long long milliseconds,
        hubTimeEventProc proc, void *data)
{
    hubTimeEvent *te = NULL;

    te = malloc(sizeof(hubTimeEvent));
    if(NULL == te) 
        return EVT_ERR;

    get_current_time(&te->seconds, &te->milliseconds);
    te->seconds += (int)(milliseconds / 1000);
    te->milliseconds += (int)(milliseconds % 1000);

    te->id = ++(hub->time_event_id);
    te->timeProc = proc;
    te->data = data;

    /* ʱ���¼�����Ϊ�գ�����β������ */
    if(NULL == hub->time_event_tail) {
        hub->time_event_head = te;
        hub->time_event_tail = te;
        te->next = NULL;   /* β����nextָ��NULL */
        te->prev = NULL;   /* ͷ����prevָ��NULL */
    }
    else {
        te->prev = hub->time_event_tail;
        te->next = NULL;
        hub->time_event_tail->next = te;
        hub->time_event_tail = te;
    }
    
    return te->id;
}

hubTimeEvent * hub_get_Time_event(evtHub *hub, long long id)
{
    hubTimeEvent *te = hub->time_event_head;

    while(NULL != te) {
        if(te->id == id) 
            return te;
        te = te->next;
    }
    return NULL;
}

int hub_delete_time_event(evtHub *hub, long long id)
{
    hubTimeEvent *next=NULL, *prev=NULL;    
    hubTimeEvent *te = hub_get_Time_event(hub, id);
    if(NULL == te) 
        return EVT_ERR;
    
    next = te->next;
    prev = te->prev;
    if(NULL != next) 
        next->prev = prev;
    else  /* �������β���Ľڵ㣬�޸�tailָ�� */
        hub->time_event_tail = te->prev;
    if(NULL != prev) 
        prev->next = next;
    else  /* �������ͷ���Ľڵ㣬�޸�headָ�� */
        hub->time_event_head = te->next;

    free(te);
    te = NULL;
    return EVT_OK;
}

char * hub_get_multipexer_name(evtHub *hub)
{
    if(NULL == hub && NULL == hub->multipexer) 
        return NULL;
  
    return evt_get_name();
}

static void add_milliseconds_to_event(long milliseconds, long *sec, long *msec)
{
    long now_sec = 0L, now_msec = 0L;

    get_current_time(&now_sec, &now_msec);

    now_msec += (int)(milliseconds % 1000);
    now_sec  += (int)(now_msec /1000);
    if(now_msec >= 1000)
        now_msec -= 1000;

    *sec  = now_sec;
    *msec = now_msec;
}

static int process_time_event(evtHub *hub)
{
    int processed = 0, retval;
    hubTimeEvent *te = NULL;
    long long maxid;
    long now_sec, now_msec;
    time_t now = time(NULL);

    /* Ϊ�˷�ֹϵͳʱ���޸�Ϊ��ȥ��ʱ��㣬����һЩ��ʱ�¼��ӳ�ִ�У�
       �ڷ�����������������ٴ���ʱ�¼� */
    if(now < hub->last_time) {
        te = hub->time_event_head;
        while(NULL != te) {
            te->seconds = 0;
            te = te->next;
        }
    }
    hub->last_time = now;

    te = hub->time_event_head;
    maxid = hub->time_event_id;
    
    while(NULL != te) {
        /* ����id�쳣�Ķ�ʱ�¼� */
        if(te->id > maxid) {
            te = te->next;
            hub_delete_time_event(hub, te->id);
        }

        get_current_time(&now_sec, &now_msec);
        if(now_sec > te->seconds || 
                (now_sec == te->seconds && now_msec > te->milliseconds)) {
            retval = te->timeProc(hub, te->data);
            ++processed;

            if(EVT_PERM == retval) {
                add_milliseconds_to_event(retval, 
                        &te->seconds, &te->milliseconds);
            }
            else {
                hub_delete_time_event(hub, te->id);
            }

            te = hub->time_event_head;
        }  
        else {
            te = te->next;
        }
    }
    return processed;
}

static int process_net_event(evtHub *hub, struct timeval *tvp)
{
    int event_nums = 0, i = 0;
    int mode, fd, flag = 0;
    int processed = 0;
    hubNetEvent *ne = NULL;
    event_nums = evt_multipexer_loop(hub, tvp);
    if(MULTIPEX_ERR == event_nums && 0 == event_nums) 
        return EVT_ERR;

    for(i = 0; i < event_nums; ++i) {
        ne = &hub->net_events[hub->rdy_events[i].fd];

        mode = hub->rdy_events[i].mode;
        fd = hub->rdy_events[i].fd;
        flag = 0;   /* ����ȷ����д�¼�һ��ֻ��ִ��һ�� */

        /* note: ne->mode & mode & ... code : maybe an already processed
         * event removed an element that fired and we still didn't
         * processed, so we check if the event is still valid. */
        if(ne->mode & mode & EVT_READ) {
            flag = 1;
            ne->rnetProc(hub, fd, ne->data, mode);
        }
        if(ne->mode & mode & EVT_WRITE) {
            if(!flag && ne->rnetProc != ne->wnetProc)
                ne->wnetProc(hub, fd, ne->data, mode);
        }

        ++processed;
    }

    return processed;
}

hubTimeEvent * search_nearest_time_event(evtHub *hub)
{
    hubTimeEvent *te = hub->time_event_head;
    hubTimeEvent *pte= te;
    int min_sec = 0, min_msec = 0;
    if(NULL == te) 
        return NULL;

    min_sec  = te->seconds;
    min_msec = te->milliseconds;
    while(NULL != (te = te->next)) {
        if(te->seconds < min_sec || 
                (te->seconds == min_sec && te->milliseconds < min_msec)) {
           pte = te;
        }
    }
    return pte;
}

static int process_events(evtHub *hub, int flag)
{
    int processed = 0;
    hubTimeEvent *te = NULL;
    long now_sec, now_msec;
    struct timeval tv, *tvp;
    tvp = &tv;
    
    if(!((flag & EVT_TIME) || (flag & EVT_NET))) 
        return EVT_NONE;

    get_current_time(&now_sec, &now_msec);

    /* û��Net�¼���������Time�¼���û������EVT_DONT_WAIT */
    if(-1 == hub->maxfd || ((flag & EVT_TIME) && !(flag & EVT_DONT_WAIT))) {
        te = search_nearest_time_event(hub);
        if(NULL != te) {
            if(te->milliseconds < now_msec) {
                tvp->tv_usec = (te->milliseconds + 1000 - now_msec) * 1000; 
                --te->seconds;
            }
            else {
                tvp->tv_usec = (te->milliseconds - now_msec) * 1000;
            }

            tvp->tv_sec = te->seconds - now_sec;

            if(tvp->tv_sec  < 0) 
                tvp->tv_sec  = 0;
            if(tvp->tv_usec < 0) 
                tvp->tv_usec = 0;
        }
        else {   /* ��ʾû���¼��¼���searchʧ�� */
            if(flag & EVT_DONT_WAIT) { /* ������DONT_WAIT��Net�¼������� */
                tvp->tv_sec  = 0;
                tvp->tv_usec = 0;
            }
            else {   /* Net�¼���������ֱ�����¼�����Ϊֹ */
                tvp = NULL;
            }
        }
    }
    
    /* ����Net�¼� */
    processed = process_net_event(hub, tvp);
    /* ����Time�¼� */
    if(flag & EVT_TIME)
        processed += process_time_event(hub);

    return processed;
}

void hub_main(evtHub *hub)
{
    hub->stop = 0;

    while(1 != hub->stop) {
        if(NULL != hub->beforeProc)
            hub->beforeProc(hub);

        process_events(hub, EVT_ALL);
    }
}
