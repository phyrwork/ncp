#include "event.h"
#include "join.h"
#include <errno.h>

int ncp_send();
int ncp_recv();

int main(void)
{
	printf("Initializing event monitoring...\n");
	int rc = init_events();

	ncp_recv();

	return 0;
}

int ncp_send()
{


	return 0;
}

int ncp_recv()
{
	printf("Starting join...\n");
	start_join();

	printf("Waiting for events...\n");
	int rc;
	event_t event;
	while((rc = get_event(&event)) > 0)
	{

		sleep(5);
	}
	printf("get_event(): %d, errno: %d\n",rc,errno);

	printf("Exiting recv...\n");
	return 0;
}
