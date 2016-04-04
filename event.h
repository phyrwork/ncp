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

int init_events(void);
int notify(thread_t thread, event_id_t event);
int monitor(void);

#endif
