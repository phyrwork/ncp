#ifndef NCP_EVENT_H
#define NCP_EVENT_H

#include "thread.h"

typedef enum {
	OK,
	ESOCK,
	EPIPE,
	ESPLIT,
	EJOIN,
	EMALLOC
} event_id_t;

typedef struct {
	thread_t thread;
	event_id_t id;
} event_t;

typedef int events_t[2];

int init_events(void);
int put_event(thread_t thread, event_id_t id);
int get_event(event_t *event);

#endif
