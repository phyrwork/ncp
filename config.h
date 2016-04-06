#ifndef NCP_CONFIG_H
#define NCP_CONFIG_H

#include "block.h"

#define NUM_PORTS_DEFAULT 4
#define NUM_PORTS_MAX 32
#define BASE_PORT_DEFAULT 61000

typedef struct {
	unsigned short len;
	int *sock;
} sock_list_t;

typedef struct {
	blen_t blen;
	sock_list_t socks;
} conf_t;

int configure_send(int argc, char *argv[], conf_t *conf);
int configure_recv(int argc, char *argv[], conf_t *conf);

#endif /* NCP_CONFIG_H */
