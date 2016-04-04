#include "event.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include "socket.h"
#include "block.h"
#include "ncp.h"
#include "recv.h"
#include "send.h"

typedef enum {
	SEND,
	RECV
} ncp_mode_t;

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




