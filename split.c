#include "split.h"
#include "pipe.h"
#include "thread.h"
#include "stream.h"
#include "event.h"
#include "block.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
	thread_t thread;
} split_ctrl_t;

static ssn_t ssn_next = 0;

void split_main(void *arg)
{
	/* initialize thread */
	split_ctrl_t *ctrl = (split_ctrl_t *) arg;

	/* initialize queue */
	blkqueue_t queue;
	init_blkqueue(queue);

	/* initialize out-stream threads */
	for(int n=0;n<4;++n) start_out(queue,ntohl(inet_addr("127.0.0.1")),8024+n);

	/* get stdin in blocks and add to block queue */
	int rc;
	blk_t *blk = blk_alloc();
	fprintf(stderr,"Waiting for stdin...\n");
	while((rc = read(STDIN_FILENO,blk->data,BLEN_DEFAULT)) > 0)
	{
		blk->ssn = ssn_next++;
		blk->len = rc;
		put_blk(queue,blk);

		fprintf(stderr,"Queueing new block ssn: %d, len: %d, data: ",blk->ssn,blk->len);
		write(STDERR_FILENO,blk->data,blk->len);
		fprintf(stderr,"\n");
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

int start_split(ncp_opt_t *opt, unsigned long addr)
{
	split_ctrl_t *ctrl = malloc(sizeof(*ctrl));
	ctrl->thread.type = TSPLIT;

	return pthread_create(&ctrl->thread.id,NULL,split_main,(void *)ctrl);
}
