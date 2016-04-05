#include "recv.h"
#include "pipe.h"
#include "thread.h"
#include "event.h"
#include "block.h"
#include "queue.h"
#include "config.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
//#include <errno.h>


typedef struct {
	thread_t thread;
	blkq_t queue;
	int sock;
} in_ctrl_t;

typedef struct {
	thread_t thread;
	blkq_t queue;
} join_ctrl_t;


/* block cache structures */
struct blk_node_s{
	SLIST_ENTRY(blk_node_s) node;
	blk_t *blk;
};
typedef struct blk_node_s blk_node_t;


void in_stream(void *arg)
{
	/* get control structure */
	in_ctrl_t *ctrl = (in_ctrl_t*) arg;


	/* read from socket until closed or error */
	int rc;
	blk_t *blk = blk_alloc();

	fprintf(stderr,"Stream %lu: Waiting for data.\n",ctrl->thread.id);
	while((rc = sock_recv(ctrl->sock,(char *)blk,sizeof(*blk) + BLEN_DEFAULT)) > 0)
	{
		fprintf(stderr,"Stream %lu: Block received (ssn:%u,len:%u)\n",ctrl->thread.id,blk->ssn,blk->len);
		int rp = put_blk(ctrl->queue,blk); // add block to queue
		blk = blk_alloc(); // get an empty block

		if(rp <= 0) { notify(ctrl->thread,EPIPE); break; } // check for pipe errors
		sleep(1);
	}
	if(rc == 0)
	{
		fprintf(stderr,"Stream %lu: Socket closed - closing queue.\n",ctrl->thread.id);
		close(ctrl->queue.fd[1]); // no more data - close queue
	}
	else
	{
		fprintf(stderr,"Stream %lu: Socket error - notifying main thread 'ESOCK'.\n",ctrl->thread.id);
		notify(ctrl->thread,ESOCK);
	}


	/* deinitialize thread */
	//free(ctrl);
	pthread_exit(NULL);
}

void join(void *arg)
{
	/* get control structure */
	join_ctrl_t *ctrl = (join_ctrl_t *) arg;


	/* fetch and output blocks in order */
	int rc;
	static ssn_t ssn_next = 0;
	blk_t *blk;
	SLIST_HEAD(blk_cache_t,blk_node_s) blk_cache = SLIST_HEAD_INITIALIZER(blk_cache);

	fprintf(stderr,"Join: Waiting for data.\n");
	while((rc = get_blk(ctrl->queue,&blk)) > 0)
	{
		fprintf(stderr,"Join: New block received (ssn:%u)\n",blk->ssn);

		/* add to block list in ordered position */
		if(blk->ssn >= ssn_next)
		{
			fprintf(stderr,"Join: Adding block to cache.\n");

			blk_node_t *new_node = malloc(sizeof(blk_node_t)); // allocate new node
			new_node->blk = blk; // associate block with node

			if (SLIST_EMPTY(&blk_cache))
			{
				SLIST_INSERT_HEAD(&blk_cache,new_node,node); // list empty - insert at head
			}
			else
			{
				blk_node_t *iter_node = SLIST_FIRST(&blk_cache);
				if(new_node->blk->ssn < iter_node->blk->ssn)
				{
					SLIST_INSERT_HEAD(&blk_cache,new_node,node); // smallest ssn - insert at head
				}
				else
				{
					while(new_node->blk->ssn < iter_node->blk->ssn) iter_node = SLIST_NEXT(iter_node,node); // find preceding node
					SLIST_INSERT_AFTER(iter_node,new_node,node); // insert at appropriate position
				}
			}
		}
		else
		{
			fprintf(stderr,"Join: Duplicate block received (ssn:%u) - discarding!\n",blk->ssn);
			blk_free(blk);
		}


		/* write out any appropriate blocks */
		fprintf(stderr,"Join: Looking for blocks to write.\n");
		while(!SLIST_EMPTY(&blk_cache) && SLIST_FIRST(&blk_cache)->blk->ssn == ssn_next)
		{
			blk_node_t *node = SLIST_FIRST(&blk_cache);

			/* output the block */
			write(STDOUT_FILENO,blk->data,blk->len);
			fprintf(stderr,"Join: Wrote out a block (ssn:%u)\n",blk->ssn);
			++ssn_next; // advance sequence number

			/* free block resources */
			blk_free(node->blk);
			fprintf(stderr,"Join: Released block resources.\n");

			/* remove block from list */
			SLIST_REMOVE_HEAD(&blk_cache,node);
			free(node); // free node resources
			fprintf(stderr,"Join: Released node resources.\n");
		}
	}

	/* examine reason for read queue break */
	if(rc == 0) // queue EOF - no more data to receive
	{
		fprintf(stderr,"Join: Queue fully closed - notifying main thread 'OK'.\n");
		notify(ctrl->thread,OK);
	}
	else if(rc < 0)
	{
		fprintf(stderr,"Join: Pipe error - notifying main thread 'EPIPE'.\n");
		// should probably examine errno here
	}


	/* deinitialize thread */
	fprintf(stderr,"Join: Exiting.\n");
	pthread_exit(NULL);
}


int ncp_recv(int argc, char *argv[])
{
	/* initialize events */
	int rc = init_events();


	/* configure connection */
	fprintf(stderr,"Main: Starting connection configuration...\n");
	conf_t conf;
	configure_recv(argc,argv,&conf);


	/* initialize queue */
	fprintf(stderr,"Main: Initializing block queue...");
	blkq_t blkq;
	init_blkq(&blkq);
	fprintf(stderr," done!\n");


	/* initalize join */
	fprintf(stderr,"Main: Initializing join...");
	join_ctrl_t *join_ctrl = malloc(sizeof(*join_ctrl));

	join_ctrl->thread.type = TJOIN; // copy parameters
	join_ctrl->queue = copy_blkq(blkq,READ);

	pthread_create(&join_ctrl->thread.id,NULL,join,(void *)join_ctrl); // initialize thread
	fprintf(stderr," done!\n");


	/* initialise streams */
	fprintf(stderr,"Main: Initializing streams...");
	in_ctrl_t *in_ctrl = malloc(conf.socks.len * sizeof(*in_ctrl));
	for(size_t n=0;n<conf.socks.len;++n)
	{
		in_ctrl_t *c = &in_ctrl[n]; // this in_ctrl structure

		c->thread.type = TSTREAMI;
		c->queue = copy_blkq(blkq,WRITE);
		c->sock = conf.socks.sock[n];

		pthread_create(&c->thread.id,NULL,in_stream,(void *)c); // initialize thread
	}
	fprintf(stderr," done!\n");


	/* monitor for events */
	close(blkq.fd[0]);
	close(blkq.fd[1]); // close original block queue descriptors to leave copies as only open ones

	event_t event;
	while(wait_notify(&event) > 0)
	{
		switch(event.id)
		{
		case OK:
			exit(0);

		case EPIPE:
			fprintf(stderr,"Main: Receive failed: Pipe error!\n");
			exit(0);

		case ESOCK:
			fprintf(stderr,"Main: Receive failed: Socket error!\n");
			exit(0);

		default:
			exit(0);
		}
	}


	return 0;
}
