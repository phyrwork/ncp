#include "recv.h"
#include "pipe.h"
#include "thread.h"
//#include "event.h"
#include "block.h"
//#include "queue.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
//#include <errno.h>

typedef struct {
	thread_t thread;
	blkq_t queue;
	int socket;
} in_ctrl_t;

typedef struct {
	thread_t thread;
	blkq_t queue;
} join_ctrl_t;

//struct blk_node_s{
//	SLIST_ENTRY(blk_node_s) node;
//	blk_t *blk;
//};
//typedef struct blk_node_s blk_node_t;
//
//SLIST_HEAD(blk_cache_t,blk_node_s) blk_cache = SLIST_HEAD_INITIALIZER(blk_cache);
//
//static ssn_t ssn_next = 0; // ssn of next block to write to stdout

void in_stream(void *arg)
{

}
//void in_main(void *arg)
//{
//	/* initialize thread */
//	stream_ctrl_t *ctrl = (stream_ctrl_t*) arg;
//
//	/* initialize socket */
//
//
//	/* close pipe */
//	sleep(3);
//	fprintf(stderr,"Closing pipe @ fd=%d...\n",ctrl->queue[1]);
//	int rc = close(ctrl->queue[1]);
//	if(rc == 0) fprintf(stderr,"Closed successfully!\n");
//
//	/* deinitialize thread */
//	free(ctrl);
//	pthread_exit(NULL);
//}
//
//int start_in(blkqueue_t queue, unsigned short port)
//{
//	stream_ctrl_t *ctrl = malloc(sizeof(*ctrl)); // create persistent memory to pass to thread
//	ctrl->thread.type = TSTREAMI;
//
//	/* copy target socket */
//	//ctrl->addr = NULL;
//	ctrl->port = port;
//
//	/* duplicate write-end file descriptor */
//	ctrl->queue[0] = queue[0];
//	ctrl->queue[1] = queue[1];
//	ctrl->queue[1] = dup(ctrl->queue[1]);
//
//	return pthread_create(&ctrl->thread.id,NULL,in_main,(void *)ctrl);
//}

void join(void *arg)
{

}

//void join_main(void *arg)
//{
//	/* initialize thread */
//	join_ctrl_t *ctrl = (join_ctrl_t *) arg;
//
//	/* initialize queue */
//	blkqueue_t queue;
//	init_blkqueue(queue);
//
//	/* initialize in-stream threads */
//	fprintf(stderr,"Initializing in streams...\n");
//
//	for(int n=0; n<4; ++n) start_in(queue,NULL);
//	close(queue[1]); // close write-end of pipe to leave threads as only remaining write-ends
//
//	/* fetch and output blocks in order */
//	int rc;
//	blk_t *blk;
//
////	printf("Waiting for a new block...\n");
//	while((rc = get_blk(queue,&blk)) > 0)
//	{
//		/* add to block list in ordered position */
////		fprintf(stderr,"Allocating new node...\n");
//		blk_node_t *new_node = malloc(sizeof(blk_node_t)); // allocate new node
//		new_node->blk = blk; // associate block with node
//
////		fprintf(stderr,"Inserting new node...\n");
//		if (SLIST_EMPTY(&blk_cache))
//		{
////			fprintf(stderr,"...list is empty: inserting at head.\n");
//			SLIST_INSERT_HEAD(&blk_cache,new_node,node); // list empty - insert at head
//		}
//		else
//		{
//			blk_node_t *iter_node = SLIST_FIRST(&blk_cache);
//			if(new_node->blk->ssn < iter_node->blk->ssn)
//			{
////				fprintf(stderr,"...new node has smallest ssn: inserting at head.\n");
//				SLIST_INSERT_HEAD(&blk_cache,new_node,node);
//			}
//			else
//			{
////				fprintf(stderr,"...mid-list.\n");
//				while(new_node->blk->ssn < iter_node->blk->ssn) iter_node = SLIST_NEXT(iter_node,node); // find preceding node
//				SLIST_INSERT_AFTER(iter_node,new_node,node);
//			}
//		}
//
//
//		/* write out any appropriate blocks */
////		fprintf(stderr,"Looking for next-in-sequence blocks to write...\n");
//		while(!SLIST_EMPTY(&blk_cache) && SLIST_FIRST(&blk_cache)->blk->ssn == ssn_next)
//		{
//			blk_node_t *node = SLIST_FIRST(&blk_cache);
//
//			/* output the block */
//			write(STDOUT_FILENO,blk->data,blk->len);
//			++ssn_next; // advance sequence number
//
//			/* free block resources */
//			blk_free(node->blk);
//
//			/* remove block from list */
//			SLIST_REMOVE_HEAD(&blk_cache,node);
//			free(node); // free node resources
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
//int start_join(ncp_opt_t *opt)
//{
//	join_ctrl_t *ctrl = malloc(sizeof(*ctrl));
//	ctrl->thread.type = TJOIN;
//
//	return pthread_create(&ctrl->thread.id,NULL,join_main,(void *)ctrl);
//}

int ncp_recv(int argc, char *argv[])
{
	/* configure connection */
	fprintf(stderr,"Starting connection configuration...\n");
	conf_t conf;
	configure_recv(argc,argv,&conf);


	/* initialize queue */
	fprintf(stderr,"Initializing block queue...");
	blkq_t blkq;
	init_blkq(&blkq);
	fprintf(stderr," done!\n");


	/* initalize join */
	fprintf(stderr,"Initializing join...");
	join_ctrl_t *join_ctrl = malloc(sizeof(*join_ctrl));

	join_ctrl->thread.type = TJOIN; // copy parameters
	join_ctrl->queue = copy_blkq(blkq,READ);

	pthread_create(&join_ctrl->thread.id,NULL,join,(void *)join_ctrl); // initialize thread
	fprintf(stderr," done!\n");


	/* initialise streams */
	fprintf(stderr,"Initializing streams...");
	in_ctrl_t *in_ctrl = malloc(conf.socks.len * sizeof(*in_ctrl));
	for(size_t n=0;n<conf.socks.len;++n)
	{
		in_ctrl_t *c = &in_ctrl[n]; // this in_ctrl structure

		c->thread.type = TSTREAMI;
		c->queue = copy_blkq(blkq,WRITE);
		c->socket = conf.socks.sock[n];

		pthread_create(&c->thread.id,NULL,in_stream,(void *)c); // initialize thread
	}
	fprintf(stderr," done!\n");


	/* monitor for events */
	close(blkq.fd[0]);
	close(blkq.fd[1]); // close original block queue descriptors to leave copies as only open ones


	return 0;
}

//int ncp_recv(int argc, char *argv[])
//{
//	/* initialize events */
//	int rc = init_events();
//
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
