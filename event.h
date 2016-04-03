#ifndef NCP_EVENT_H
#define NCP_EVENT_H

#include "thread.h"

typedef enum {
	OK,
	ESOCK,
	EPIPE,
	ESPLIT,
	EJOIN
} event_type_t;

typedef struct {
	thread_t thread;
	event_type_t event;
} event_t;

typedef int events_t[2];

int init_events(events_t fd);
int put_event(events_t fd, event_t event);
int get_event(events_t fd, event_t *event);

#endif
