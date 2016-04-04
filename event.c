#include "event.h"
#include "pipe.h"

events_t events;

int init_events(void)
{
	return pipe2(events,O_DIRECT);
}

int put_event(thread_t thread, event_id_t id)
{
	event_t event;
	event.thread = thread;
	printf("id: %d\n",id);
	event.id = id;

	return write(events[1],(void *) &event, sizeof(event));
}

int get_event(event_t *event)
{
	return read(events[0],(void *) event, sizeof(*event));
}
