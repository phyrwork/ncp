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

	/* initialize socket */


	/* close pipe */
	sleep(3);
	fprintf(stderr,"Closing pipe @ fd=%d...\n",ctrl->queue[1]);
	int rc = close(ctrl->queue[1]);
	if(rc == 0) fprintf(stderr,"Closed successfully!\n");

	/* deinitialize thread */
	free(ctrl);
	pthread_exit(NULL);
}

void out_main(void *arg)
{
	/* initialize thread */
	stream_ctrl_t *ctrl = (stream_ctrl_t*) arg;

	/* initialize socket */
	//

	/* read blocks from queue */
	int rc;
	blk_t *blk;
	while((rc = get_blk(ctrl->queue,&blk)) > 0)
	{
		/* write data to socket */

		//debug
		fprintf(stderr,"Output block ssn: %d, len: %d, data: %s\n",blk->ssn,blk->len,blk->data);
	}

	/* deinitialize thread */
	free(ctrl);
	pthread_exit(NULL);
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

	return pthread_create(&ctrl->thread.id,NULL,in_main,(void *)ctrl);
}

int start_out(blkqueue_t queue, unsigned long addr, unsigned short port)
{
	stream_ctrl_t *ctrl = malloc(sizeof(*ctrl)); // create persistent memory to pass to thread
	ctrl->thread.type = TSTREAMO;

	/* copy target socket */
	ctrl->addr = addr;
	ctrl->port = port;

	/* duplicate read-end file descriptor */
	ctrl->queue[0] = queue[0];
	ctrl->queue[1] = queue[1];
	ctrl->queue[0] = dup(ctrl->queue[0]);

	return pthread_create(&ctrl->thread.id,NULL,out_main,(void *)ctrl);
}
