#include "stream.h"
#include "pipe.h"
#include "thread.h"
#include <string.h>
#include <stdio.h>

typedef struct {
	thread_t thread;
	unsigned long addr;
	unsigned short port;
	blkqueue_t queue;
} stream_ctrl_t;

void in_main(void *arg)
{
	/* initialize thread */
	stream_ctrl_t ctrl = *((stream_ctrl_t*) arg);
	ctrl.thread.id = pthread_self();

	while(1)
	{
		printf("Stream thread (id: %d) running...\n", (unsigned int) ctrl.thread.id);
		sleep(5);
	}
}

void out_main(void *arg)
{

}

int start_in(blkqueue_t queue, unsigned short port)
{
	stream_ctrl_t ctrl;
	ctrl.thread.type = TSTREAMI;

	/* copy target socket */
	//ctrl.addr = NULL;
	ctrl.port = port;

	/* duplicate write-end file descriptor */
	memcpy(queue,ctrl.queue,sizeof(ctrl.queue));
	ctrl.queue[2] = dup(ctrl.queue[2]);

	return pthread_create(&ctrl.thread.id,NULL,in_main,(void *)&ctrl);
}

int start_out(blkqueue_t queue, unsigned long addr, unsigned short port)
{
	stream_ctrl_t ctrl;
	ctrl.thread.type = TSTREAMI;

	/* copy target socket */
	ctrl.addr = addr;
	ctrl.port = port;

	/* duplicate write-end file descriptor */
	memcpy(queue,ctrl.queue,sizeof(ctrl.queue));
	ctrl.queue[1] = dup(ctrl.queue[1]);

	return pthread_create(&ctrl.thread.id,NULL,out_main,(void *)&ctrl);
}
