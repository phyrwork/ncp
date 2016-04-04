#include "stream.h"
#include "pipe.h"
#include "thread.h"
#include "event.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
	thread_t thread;
	unsigned long addr;
	unsigned short port;
	blkqueue_t queue;
} stream_ctrl_t;

void in_main(void *arg)
{
	/* initialize thread */
	stream_ctrl_t *ctrl = (stream_ctrl_t*) arg;

	/* close pipe */
	sleep(1);
	printf("Closing pipe @ fd=%d...\n",ctrl->queue[1]);
	int rc = close(ctrl->queue[1]);
	if(rc == 0) printf("Closed successfully!\n");

	/* deinitialize thread */
	free(ctrl);
	pthread_exit(NULL);
}

void out_main(void *arg)
{

}

int start_in(blkqueue_t queue, unsigned short port)
{
	stream_ctrl_t *ctrl = malloc(sizeof(*ctrl)); // create persistent memory to pass to thread
	ctrl->thread.type = TSTREAMI;

	/* copy target socket */
	//ctrl->addr = NULL;
	ctrl->port = port;

	/* duplicate write-end file descriptor */
	ctrl->queue[0] = queue[0];
	ctrl->queue[1] = queue[1];
	ctrl->queue[1] = dup(ctrl->queue[1]);
	printf("Duplicating pipe @ fd=%d to fd=%d\n",queue[1],ctrl->queue[1]);

	return pthread_create(&ctrl->thread.id,NULL,in_main,(void *)ctrl);
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
