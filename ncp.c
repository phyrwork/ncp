#include "event.h"
#include "join.h"
#include "split.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include "socket.h"
#include "block.h"
#include "ncp.h"

#define NUM_PORTS_DEFAULT 4
#define NUM_PORTS_MAX 32
#define BASE_PORT_DEFAULT 8024

typedef enum {
	SEND,
	RECV
} ncp_mode_t;

#define SIZEOF_NCP_RESPONSE(num_ports) (sizeof(ncp_opt_t) + (num_ports-1)*sizeof(unsigned short))

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

	/* call send/recv method */
	switch(mode)
	{
	case SEND:
		ncp_send(argc-2,&argv[2]);
		break;
	case RECV:
		ncp_recv(argc-2,&argv[2]);
		break;
	}

	return 0;
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

int ncp_recv(int argc, char *argv[])
{
	/* initialize events */
	int rc = init_events();

	/* create negotiation socket */
	unsigned short port = atoi(argv[argc-1]);
	fprintf(stderr,"Awaiting negotiation on port %u...\n",port);
	int sock = sock_in(port); // wait for incoming connection
	fprintf(stderr,"...connected!\n");

	/* get connection options */
	fprintf(stderr,"Awaiting connection options...\n");
	void *opt = malloc(SIZEOF_NCP_RESPONSE(NUM_PORTS_MAX));
	sock_recv(sock,(char *)opt,sizeof(*((ncp_opt_t *)opt))); // await connection options
	fprintf(stderr,"...received!\n");

	/* initialize receive */
	unsigned char num_ports = 0;
	while(num_ports < ((ncp_opt_t *)opt)->num_ports)
	{

	}

	fprintf(stderr,"Initializing receive...\n");
	start_join((ncp_opt_t *)opt);

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
