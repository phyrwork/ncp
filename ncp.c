#include "event.h"
#include "join.h"
#include "split.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

typedef enum {
	SEND,
	RECV
} ncp_mode_t;

int ncp_send();
int ncp_recv();

int main(int argc, char *argv[])
{
	/* parse arguments */
	ncp_mode_t mode;
	     if(!strcmp(argv[1],"send")) mode = SEND;
	else if(!strcmp(argv[1],"recv")) mode = RECV;
	else
	{
		fprintf(stderr,"Mode '%s', not recognised.",argv[1]);
	}

	/* initialize events */
	int rc = init_events();

	/* begin send/recv */
	switch(mode)
	{
	case SEND:
		ncp_send();
		break;
	case RECV:
		ncp_recv();
		break;
	}

	return 0;
}

int ncp_send()
{
	fprintf(stderr,"Starting split...\n");
	start_split();

	fprintf(stderr,"Waiting for events...\n");
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
	if(rc > 0) fprintf(stderr,"Breaking event: %d\n",event.id);
	else if(rc < 0) fprintf(stderr,"get_event() errno: %d\n",errno);
	else fprintf(stderr,"get_event(): EOF.\n");

	return 0;
}

int ncp_recv()
{
	fprintf(stderr,"Starting join...\n");
	start_join();

	fprintf(stderr,"Waiting for events...\n");
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
	if(rc > 0) fprintf(stderr,"Breaking event: %d\n",event.id);
	else if(rc < 0) fprintf(stderr,"get_event() errno: %d\n",errno);
	else fprintf(stderr,"get_event(): EOF.\n");

	return 0;
}
