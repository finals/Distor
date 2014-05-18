#include <unistd.h>
#include <stdlib.h>

#include "hub_epoll.h"

int evt_multipexer_create(evtHub *hub)
{
    evtMultipexer *multipexer = (evtMultipexer *)malloc(sizeof(evtMultipexer));
    if(NULL == multipexer)
        return MULTIPEX_ERR;

    multipexer->events = malloc(sizeof(struct epoll_event)*hub->evtsize);
    if(NULL == multipexer->events) {
        free(multipexer);
        return MULTIPEX_ERR;
    }
    
    /* epoll_create的参数仅仅是个暗示，不会限制事件数量 */
    multipexer->epfd = epoll_create(1024);
    if(-1 == multipexer->epfd) {
        free(multipexer->events);
        free(multipexer);
        return MULTIPEX_ERR;
    }
    hub->multipexer = multipexer;
    return MULTIPEX_OK;
}

int evt_multipexer_loop(evtHub *hub, struct timeval *tv)
{
    evtMultipexer *multipexer = hub->multipexer;
    int nfds, mode, i;
    struct epoll_event *ev = NULL;

    nfds = epoll_wait(multipexer->epfd, multipexer->events, hub->evtsize,
        tv ? (tv->tv_sec*1000 + tv->tv_usec/1000) : -1);
    if(MULTIPEX_ERR == nfds || 0 == nfds)
        return nfds;

    for(i = 0; i < nfds; ++i) {
        mode = 0;
        ev = multipexer->events+i;
        
        if(ev->events & EPOLLIN)
            mode |= EVT_READ;
        if(ev->events & EPOLLOUT)
            mode |= EVT_WRITE;
        /*EPOLLHUP,EPOLLERR: epoll_wait(2) will always wait for this event;*/
        if(ev->events & EPOLLHUP) 
            mode |= EVT_WRITE;
        if(ev->events & EPOLLERR)
            mode |= EVT_WRITE;
            
        /* 当epoll监听到某些读写事件后，将事件保存在hub->rdy_events数组中 */
        hub->rdy_events[i].fd = ev->data.fd;
        hub->rdy_events[i].mode = mode;
        
    }
    return nfds;
}

void evt_multipexer_free(evtHub *hub)
{
     evtMultipexer *multipexer = hub->multipexer;

     close(multipexer->epfd);
     free(multipexer->events);
     free(multipexer);

     hub->multipexer = NULL;
}

int evt_event_add(evtHub *hub, int fd, int mode)
{
    evtMultipexer *multipexer = hub->multipexer;
    struct epoll_event ee;
    int op = hub->net_events[fd].mode == 
            EVT_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    mode |= hub->net_events[fd].mode; /* 合并旧事件 */ 
    if(mode & EVT_READ)
        ee.events |= EPOLLIN;
    if(mode & EVT_WRITE)
        ee.events |= EPOLLOUT;
    ee.data.u64 = 0;  /* 避免vargrind告警 , 相当于初始化联合体*/
    ee.data.fd = fd;

    if(-1 == epoll_ctl(multipexer->epfd, op, fd, &ee)) {
        return MULTIPEX_ERR;
    }
    return MULTIPEX_OK;
}

int evt_event_del(evtHub *hub, int fd, int delmode)
{
    evtMultipexer *multipexer = hub->multipexer;
    struct epoll_event ee;
    int mode = hub->net_events[fd].mode & (~delmode);

    ee.events = 0;
    if(mode & EVT_READ)
        ee.events |= EPOLLIN;
    if(mode & EVT_WRITE)
        ee.events |= EPOLLOUT;
    ee.data.u64 = 0;  /* 避免vargrind告警 */
    ee.data.fd = fd;

    if(EVT_NONE != mode) {
        if(-1 == epoll_ctl(multipexer->epfd, EPOLL_CTL_MOD, fd, &ee))
            return MULTIPEX_ERR;
    }
    else {
        if(-1 == epoll_ctl(multipexer->epfd, EPOLL_CTL_DEL, fd, &ee))
            return MULTIPEX_ERR;
    }
    
    return MULTIPEX_OK;
}

char * evt_get_name(void)
{
    return "epoll";
}

