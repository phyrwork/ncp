#include "join.h"
#include "pipe.h"
#include "thread.h"
#include "event.h"
#include "block.h"
#include "stream.h"
#include <stdio.h>

typedef struct {
	thread_t thread;
} join_ctrl_t;

blkqueue_t queue;

void join_main(void *arg)
{
	/* initialize thread */
	join_ctrl_t ctrl = *((join_ctrl_t *) arg);
	ctrl.thread.id = pthread_self();

	/* initialize queue */
	init_blkqueue(queue);

	/* initialize in-stream threads */
	for(int n=0;n<4;++n) start_in(queue,8024+n);

	// debug
	while(1)
	{
		printf("Join thread (id: %d) running...\n", (unsigned int) ctrl.thread.id);
		sleep(2);
	}
}

int start_join(void)
{
	join_ctrl_t ctrl;
	ctrl.thread.type = TJOIN;

	return pthread_create(&ctrl.thread.id,NULL,join_main,(void *)&ctrl);
}
