#ifndef __HUB_SELECT_H__
#define __HUB_SELECT_H__

#include <sys/select.h>
#include <sys/time.h>
#include "hub.h"

#define MULTIPEX_ERR  -1
#define MULTIPEX_OK    0

typedef struct evtMultipexer {
    fd_set rfds, wfds;
    /* We need to have a copy of the fd sets as it's not safe to reuse
     * FD sets after select(). */
    fd_set _rfds, _wfds;
}evtMultipexer;

int evt_multipexer_create(evtHub *hub);
int evt_multipexer_loop(evtHub *hub, struct timeval *tv);
void evt_multipexer_free(evtHub *hub);
int evt_event_add(evtHub *hub, int fd, int mode);
int evt_event_del(evtHub *hub, int fd, int delmode);
char * evt_get_name(void);

#endif
