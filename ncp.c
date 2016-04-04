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
	if(rc > 0) printf("Breaking event: %d\n",event.id);
	else if(rc < 0) printf("get_event() errno: %d\n",errno);
	else printf("EOF.\n");

	printf("Exiting recv...\n");
	return 0;
}
