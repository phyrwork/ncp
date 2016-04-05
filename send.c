#include "send.h"
#include "thread.h"
#include "block.h"
#include "config.h"
#include "pipe.h"
#include "event.h"
#include "socket.h"

#include <stdlib.h>
#include <stdio.h>
//#include <errno.h>
//#include <netdb.h>

typedef struct {
	thread_t thread;
	blkq_t queue;
	int sock;
} out_ctrl_t;

typedef struct {
	thread_t thread;
	blkq_t queue;
} split_ctrl_t;

void out_stream(void *arg)
{
	/* get control structure */
	out_ctrl_t *ctrl = (out_ctrl_t*) arg;


	/* write to socket until pipe closed or error */
	int rp;
	blk_t *blk;

	while((rp = get_blk(ctrl->queue,&blk)) > 0) // get block from queue
	{
		int rc = sock_send(ctrl->sock,(char *)blk,sizeof(*blk) + blk->len);// send block via sock
		blk_free(blk); // discard the block

		if(rc <= 0) { notify(ctrl->thread,ESOCK); break; } // check for sock errors
	}
	if(rp) notify(ctrl->thread,OK);
	else notify(ctrl->thread,EPIPE);


	/* deinitialize thread */
	free(ctrl);
	pthread_exit(NULL);
}

void split(void *arg)
{
	/* get control structure */
	split_ctrl_t *ctrl = (split_ctrl_t *) arg;


	/* get stdin in blocks and add to block queue */
	int rc;
	static ssn_t ssn_next = 0;
	blk_t *blk = blk_alloc();

	while((rc = read(STDIN_FILENO,blk->data,BLEN_DEFAULT)) > 0)
	{
		blk->ssn = ssn_next++;
		blk->len = rc;
		put_blk(ctrl->queue,blk);
	}


	/* examine reason for pipe close */
	switch(rc)
	{
	case 0:
		notify(ctrl->thread,OK);
		break;
	default:
		notify(ctrl->thread,EPIPE);
		break;
	}


	/* deinitialize thread */
	free(ctrl);
	pthread_exit(NULL);
}

int ncp_send(int argc, char *argv[])
{
	/* initialize events */
	int rc = init_events();


	/* configure connection */
	conf_t conf;
	configure_send(argc,argv,&conf);


	/* initialize queue */
	blkq_t blkq;
	init_blkq(&blkq);


	/* initialise streams */
	out_ctrl_t *out_ctrl = malloc(conf.socks.len * sizeof(*out_ctrl));
	for(size_t n=0;n<conf.socks.len;++n)
	{
		out_ctrl_t *c = &out_ctrl[n]; // this in_ctrl structure

		c->thread.type = TSTREAMO;
		c->queue = copy_blkq(blkq,READ);
		c->sock = conf.socks.sock[n];

		pthread_create(&c->thread.id,NULL,out_stream,(void *)c); // initialize thread
	}


	/* initalize split */
	split_ctrl_t *split_ctrl = malloc(sizeof(*split_ctrl));

	split_ctrl->thread.type = TSPLIT; // copy parameters
	split_ctrl->queue = copy_blkq(blkq,WRITE);

	pthread_create(&split_ctrl->thread.id,NULL,split,(void *)split_ctrl); // initialize thread


	/* monitor for events */
	close(blkq.fd[0]);
	close(blkq.fd[1]); // close original block queue descriptors to leave copies as only open ones

	event_t event;
	while(wait_notify(&event) > 0)
	{
		switch(event.id)
		{
		case OK:
			fprintf(stderr,"Send completed successfully!\n");
			break;

		case EPIPE:
			fprintf(stderr,"Send failed: Pipe error!\n");
			break;

		case ESOCK:
			fprintf(stderr,"Send failed: Socket error!\n");
			break;

		default:
			break;
		}
	}

	return 0;
}
