#include "recv.h"
#include "send.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	/* parse arguments */
	     if(!strcmp(argv[1],"send")) ncp_send(argc-2,&argv[2]);
	else if(!strcmp(argv[1],"recv")) ncp_recv(argc-2,&argv[2]);
	else
	{
		fprintf(stderr,"Mode '%s', not recognised.",argv[1]);
	}

	return 0;
}




