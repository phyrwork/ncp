#include "send.h"
#include "thread.h"
#include "block.h"
#include "config.h"
#include "pipe.h"
#include "event.h"
#include "socket.h"
#include "frame.h"

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


	/* initialise frame buffer */
	fbuf_t fbuf; // initialise frame buffer
	fbuf_init(&fbuf,ctrl->sock,sizeof(blk_t) + BLEN_DEFAULT);


	/* write to socket until pipe closed or error */
	int rp;
	blk_t *blk;

	fprintf(stderr,"Stream %lu: Waiting for data.\n",ctrl->thread.id);
	while((rp = get_blk(ctrl->queue,&blk)) > 0) // get block from queue
	{
		// fprintf(stderr,"Stream %lu: Block to send (ssn:%u,len:%u)\n",ctrl->thread.id,blk->ssn,blk->len);
		int rc = put_frame(&fbuf,(char *)blk,sizeof(*blk) + blk->len); // send block via sock
		fprintf(stderr,"Stream %lu: Block sent (ssn:%u)\n",ctrl->thread.id,blk->ssn);

		blk_free(blk); // discard the block
		// fprintf(stderr,"Stream %lu: Block discarded.\n",ctrl->thread.id);

		if(rc <= 0) { notify(ctrl->thread,ESOCK); break; } // check for sock errors
	}
	if(rp == 0) {} // sock_close(ctrl->sock); // no more data - close socket
	else notify(ctrl->thread,EPIPE);


	/* deinitialize thread */
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

	fprintf(stderr,"Split: Waiting for data.\n");
	while((rc = read(STDIN_FILENO,blk->data,BLEN_DEFAULT)) > 0)
	{
		// fprintf(stderr,"Split: Block received - adding metadata.\n");
		blk->ssn = ssn_next++;
		blk->len = rc;

		put_blk(ctrl->queue,blk); // add block to queue
		fprintf(stderr,"Split: Block added to queue (ssn:%u, len:%u).\n",blk->ssn,blk->len);

		blk = blk_alloc(); // allocate a new buffer block
		// fprintf(stderr,"Split: Allocated a new buffer block.\n",blk->ssn,blk->len);
	}

	/* examine reason for stdin read break */
	if(rc == 0) // stdin EOF - no more data to send
	{

		close(ctrl->queue.fd[1]); // close the write end - read end will get EOF
		notify(ctrl->thread,OK);
	}
	else if(rc < 0)
	{
		// should probably examine errno here
	}


	/* deinitialize thread */
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
			sleep(1);
			// need to wait for read queue to be completely emptied first
			// maybe we could join the send threads? need to re-think the graceful shutdown
			exit(0);

		case EPIPE:
			fprintf(stderr,"Send failed: Pipe error!\n");
			exit(0);

		case ESOCK:
			fprintf(stderr,"Send failed: Socket error!\n");
			exit(0);

		default:
			break;
		}
	}

	return 0;
}
