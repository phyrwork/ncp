#include "join.h"
#include "thread.h"

typedef struct {
	thread_t thread;
} join_ctrl_t;

void join_main(void *arg)
{

}

int start_join(void)
{
	join_ctrl_t ctrl;
	ctrl.thread.type = TJOIN;

	return pthread_create(&ctrl.thread.id,NULL,join_main,(void *)&ctrl);
}
