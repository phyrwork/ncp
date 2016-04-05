#include "event.h"
#include "pipe.h"

int events[2];

int init_events(void)
{
	return pipe2(events,O_DIRECT);
}

int notify(thread_t thread, event_id_t event)
{
	event_t e;
	e.thread = thread;
	e.id = event;

	return write(events[1],(void *) &e, sizeof(e));
}

int wait_notify(event_t *event)
{
	return read(events[0],(void *) event, sizeof(*event));
}

//int put_event(thread_t thread, event_id_t id)
//{

//}
//
//int get_event(event_t *event)
//{
//
//}
