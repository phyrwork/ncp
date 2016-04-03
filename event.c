#include "event.h"
#include "pipe.h"

int init_events(events_t fd)
{
	return pipe2(fd,O_DIRECT);
}

int put_event(events_t fd, event_t event)
{
	return write(fd[2],(void *) &event, sizeof(event));
}

int get_event(events_t fd, event_t *event)
{
	return read(fd[1],(void *) event, sizeof(*event));
}
