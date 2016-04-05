#include "send.h"
#include "thread.h"
#include "block.h"
#include "config.h"
#include "pipe.h"
//#include "event.h"

#include <stdlib.h>
//#include <stdio.h>
//#include <errno.h>
//#include <netdb.h>

typedef struct {
	thread_t thread;
	blkq_t queue;
	int socket;
} out_ctrl_t;

typedef struct {
	thread_t thread;
	blkq_t queue;
} split_ctrl_t;

void out_stream(void *arg)
{

}
//void out_main(void *arg)
//{
//	/* initialize thread */
//	stream_ctrl_t *ctrl = (stream_ctrl_t*) arg;
//
//	/* initialize socket */
//	//
//
//	/* read blocks from queue */
//	int rc;
//	blk_t *blk;
//	while((rc = get_blk(ctrl->queue,&blk)) > 0)
//	{
//		/* write data to socket */
//
//		//debug
//		fprintf(stderr,"Output block ssn: %d, len: %d, data: %s\n",blk->ssn,blk->len,blk->data);
//	}
//
//	/* deinitialize thread */
//	free(ctrl);
//	pthread_exit(NULL);
//}
//
//int start_out(blkqueue_t queue, unsigned long addr, unsigned short port)
//{
//	stream_ctrl_t *ctrl = malloc(sizeof(*ctrl)); // create persistent memory to pass to thread
//	ctrl->thread.type = TSTREAMO;
//
//	/* copy target socket */
//	ctrl->addr = addr;
//	ctrl->port = port;
//
//	/* duplicate read-end file descriptor */
//	ctrl->queue[0] = queue[0];
//	ctrl->queue[1] = queue[1];
//	ctrl->queue[0] = dup(ctrl->queue[0]);
//
//	return pthread_create(&ctrl->thread.id,NULL,out_main,(void *)ctrl);
//}

void split(void *arg)
{

}

//void split_main(void *arg)
//{
//	/* initialize thread */
//	split_ctrl_t *ctrl = (split_ctrl_t *) arg;
//
//	/* initialize queue */
//	blkqueue_t queue;
//	init_blkqueue(queue);
//
//	/* initialize out-stream threads */
//	for(int n=0;n<4;++n) start_out(queue,ntohl(inet_addr("127.0.0.1")),8024+n);
//
//	/* get stdin in blocks and add to block queue */
//	int rc;
//	blk_t *blk = blk_alloc();
//	fprintf(stderr,"Waiting for stdin...\n");
//	while((rc = read(STDIN_FILENO,blk->data,BLEN_DEFAULT)) > 0)
//	{
//		blk->ssn = ssn_next++;
//		blk->len = rc;
//		put_blk(queue,blk);
//
//		fprintf(stderr,"Queueing new block ssn: %d, len: %d, data: ",blk->ssn,blk->len);
//		write(STDERR_FILENO,blk->data,blk->len);
//		fprintf(stderr,"\n");
//	}
//
//	/* examine reason for pipe close */
//	switch(rc)
//	{
//	case 0:
//		put_event(ctrl->thread,OK);
//		break;
//	default:
//		put_event(ctrl->thread,EPIPE);
//		break;
//	}
//
//	/* deinitialize thread */
//	free(ctrl);
//	pthread_exit(NULL);
//}
//
//int start_split(ncp_opt_t *opt, unsigned long addr)
//{
//	split_ctrl_t *ctrl = malloc(sizeof(*ctrl));
//	ctrl->thread.type = TSPLIT;
//
//	return pthread_create(&ctrl->thread.id,NULL,split_main,(void *)ctrl);
//}

int ncp_send(int argc, char *argv[])
{
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

		c->thread.type = TSTREAMI;
		c->queue = copy_blkq(blkq,READ);
		c->socket = conf.socks.sock[n];

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

	return 0;
}

//int ncp_send(int argc, char *argv[])
//{
//	/* initialize events */
//	int rc = init_events();
//
//	/* wait for event */
//	fprintf(stderr,"Waiting for events...\n");
//	event_t event;
//	int loop = 1;
//	while(loop && (rc = get_event(&event)) > 0)
//	{
//		switch(event.id)
//		{
//			case OK:
//				loop = 0;
//				break;
//			case ESOCK:
//				loop = 0;
//				break;
//			default:
//				break;
//		}
//	}
//	if(rc > 0) fprintf(stderr,"Breaking event: %d\n",event.id);
//	else if(rc < 0) fprintf(stderr,"get_event() errno: %d\n",errno);
//	else fprintf(stderr,"get_event(): EOF.\n");
//
//	return 0;
//}
