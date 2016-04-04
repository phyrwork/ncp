#include "join.h"
#include "pipe.h"
#include "thread.h"
#include "event.h"
#include "block.h"
#include "stream.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	thread_t thread;
} join_ctrl_t;

void join_main(void *arg)
{
	/* initialize thread */
	join_ctrl_t *ctrl = (join_ctrl_t *) arg;

	/* initialize queue */
	blkqueue_t queue;
	init_blkqueue(queue);

	/* initialize in-stream threads */
	for(int n=0;n<4;++n) start_in(queue,8024+n);
	close(queue[1]); // close write-end of pipe to leave threads as only remaining write-ends

	/* fetch and output blocks in order */
	int rc;
	blk_t *blk;
	while((rc = get_blk(queue,&blk)) > 0)
	{
		sleep(1);
	}

	/* examine reason for pipe close */
	switch(rc)
	{
	case 0:
		put_event(ctrl->thread,OK);
		break;
	default:
		put_event(ctrl->thread,EPIPE);
		break;
	}

	/* deinitialize thread */
	free(ctrl);
	pthread_exit(NULL);
}

int start_join(void)
{
	join_ctrl_t *ctrl = malloc(sizeof(*ctrl));
	ctrl->thread.type = TJOIN;

	return pthread_create(&ctrl->thread.id,NULL,join_main,(void *)ctrl);
}
