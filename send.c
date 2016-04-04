#include "pipe.h"
#include "thread.h"
#include "event.h"
#include "block.h"
#include <stdlib.h>
#include <stdio.h>
#include "send.h"
#include "config.h"
#include <errno.h>
#include <netdb.h>

typedef struct {
	thread_t thread;
	unsigned long addr;
	unsigned short port;
	blkqueue_t queue;
} stream_ctrl_t;

typedef struct {
	thread_t thread;
} split_ctrl_t;

static ssn_t ssn_next = 0;

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

int ncp_send(int argc, char *argv[])
{
	/* initialize events */
	int rc = init_events();

	/* create negotiation socket */
	unsigned short port = atoi(argv[argc-1]); // get port
	unsigned long addr;
	{
		struct hostent *server = gethostbyname(argv[argc-2]);
		if(server == NULL)
		{
			fprintf(stderr,"Cannot send: Unknown host");
			exit(0);
		}
		bcopy((char *)server->h_addr,(char *) &addr,server->h_length);
		addr = ntohl(addr);
	}
	fprintf(stderr,"Initiating negotiation with host %lu:%u\n",addr,port);
	int sock = sock_out(addr,port); // initiate outgoing connection

	/* send connection options */
	void *opt = malloc(SIZEOF_NCP_RESPONSE(NUM_PORTS_MAX));
	((ncp_opt_t *)opt)->blen = BLEN_DEFAULT;
	((ncp_opt_t *)opt)->num_ports = NUM_PORTS_DEFAULT;
	sock_send(sock,(char *)opt,sizeof(*((ncp_opt_t *)opt))); // send options
	rc = sock_recv(sock,(char *)opt,SIZEOF_NCP_RESPONSE(NUM_PORTS_MAX)); // wait for response

	/* start send */
	fprintf(stderr,"Starting split...\n");
	start_split((ncp_opt_t *)opt,addr);

	/* wait for event */
	fprintf(stderr,"Waiting for events...\n");
	event_t event;
	int loop = 1;
	while(loop && (rc = get_event(&event)) > 0)
	{
		switch(event.id)
		{
			case OK:
				loop = 0;
				break;
			case ESOCK:
				loop = 0;
				break;
			default:
				break;
		}
	}
	if(rc > 0) fprintf(stderr,"Breaking event: %d\n",event.id);
	else if(rc < 0) fprintf(stderr,"get_event() errno: %d\n",errno);
	else fprintf(stderr,"get_event(): EOF.\n");

	return 0;
}
