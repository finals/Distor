#ifndef __HUB_EPOLL_H__
#define __HUB_EPOLL_H__

#include <sys/epoll.h>
#include <sys/time.h>
#include "hub.h"

#define MULTIPEX_ERR  -1
#define MULTIPEX_OK    0

typedef struct evtMultipexer {
    int epfd;
    struct epoll_event *events;
}evtMultipexer;

int evt_multipexer_create(evtHub *hub);
int evt_multipexer_loop(evtHub *hub, struct timeval *tv);
void evt_multipexer_free(evtHub *hub);
int evt_event_add(evtHub *hub, int fd, int mode);
int evt_event_del(evtHub *hub, int fd, int delmode);
char * evt_get_name(void);

#endif
