#include "join.h"
#include "pipe.h"
#include "thread.h"
#include "event.h"
#include "block.h"
#include "stream.h"
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

typedef struct {
	thread_t thread;
} join_ctrl_t;

struct blk_node_s{
	SLIST_ENTRY(blk_node_s) node;
	blk_t *blk;
};
typedef struct blk_node_s blk_node_t;

SLIST_HEAD(blk_cache_t,blk_node_s) blk_cache = SLIST_HEAD_INITIALIZER(blk_cache);

ssn_t ssn_next = 0; // ssn of next block to write to stdout

int output_blk(blk_t *blk)
{
	printf("ssn:%d, len:%d, data:%s\n",blk->ssn,blk->len,blk->data);
	return 0;
}

void join_main(void *arg)
{
	/* initialize thread */
	join_ctrl_t *ctrl = (join_ctrl_t *) arg;

	/* initialize queue */
	blkqueue_t queue;
	init_blkqueue(queue);

	/* initialize in-stream threads */
	for(int n=0;n<4;++n) start_in(queue,8024+n);
	//close(queue[1]); // close write-end of pipe to leave threads as only remaining write-ends

	// debug - write some unordered test data
	printf("Writing some test data...\n");
	blk_t *test_blk;

	test_blk = blk_alloc();
	test_blk->ssn = 3;
	strncpy(test_blk->data,"sequence 3",10);
	test_blk->len = strnlen(test_blk->data);
	put_blk(queue,test_blk);

	test_blk = blk_alloc();
	test_blk->ssn = 0;
	strncpy(test_blk->data,"sequence 0",10);
	test_blk->len = strnlen(test_blk->data);
	put_blk(queue,test_blk);

	test_blk = blk_alloc();
	test_blk->ssn = 1;
	strncpy(test_blk->data,"sequence 1",10);
	test_blk->len = strnlen(test_blk->data);
	put_blk(queue,test_blk);

	test_blk = blk_alloc();
	test_blk->ssn = 2;
	strncpy(test_blk->data,"sequence 2",10);
	test_blk->len = strnlen(test_blk->data);
	put_blk(queue,test_blk);

	printf("...done!\n");
	////////


	/* fetch and output blocks in order */
	int rc;
	blk_t *blk;

	printf("Waiting for a new block...\n");
	while((rc = get_blk(queue,&blk)) > 0)
	{
		/* add to block list in ordered position */
		printf("Allocating new node...\n");
		blk_node_t *new_node = malloc(sizeof(blk_node_t)); // allocate new node
		new_node->blk = blk; // associate block with node

		printf("Inserting new node...\n");
		if (SLIST_EMPTY(&blk_cache))
		{
			printf("...list is empty: inserting at head.\n");
			SLIST_INSERT_HEAD(&blk_cache,new_node,node); // list empty - insert at head
		}
		else
		{
			blk_node_t *iter_node = SLIST_FIRST(&blk_cache);
			if(new_node->blk->ssn < iter_node->blk->ssn)
			{
				printf("...new node has smallest ssn: inserting at head.\n");
				SLIST_INSERT_HEAD(&blk_cache,new_node,node);
			}
			else
			{
				printf("...mid-list.\n");
				while(new_node->blk->ssn < iter_node->blk->ssn) iter_node = SLIST_NEXT(iter_node,node); // find preceding node
				SLIST_INSERT_AFTER(iter_node,new_node,node);
			}
		}


		/* write out any appropriate blocks */
		printf("Looking for next-in-sequence blocks to write...\n");
		while(!SLIST_EMPTY(&blk_cache) && SLIST_FIRST(&blk_cache)->blk->ssn == ssn_next)
		{
			blk_node_t *node = SLIST_FIRST(&blk_cache);

			/* output the block */
			output_blk(node->blk);
			++ssn_next;

			/* free block resources */
			blk_free(node->blk);

			/* remove block from list */
			SLIST_REMOVE_HEAD(&blk_cache,node);
			free(node); // free node resources

			if(!SLIST_EMPTY(&blk_cache))
				printf("Next block in cache is ssn:%d, len:%d, data:%s\n",
						SLIST_FIRST(&blk_cache)->blk->ssn,
						SLIST_FIRST(&blk_cache)->blk->len,
						SLIST_FIRST(&blk_cache)->blk->data);
			else printf("No more blocks in cache.\n");
		}
		printf("No more blocks to write...\n");
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
