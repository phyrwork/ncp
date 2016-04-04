#ifndef NCP_MAIN_H
#define NCP_MAIN_H

#include "block.h"

typedef struct {
	blen_t blen;
	unsigned char num_ports;
	unsigned short *port;
} ncp_opt_t;

#endif /* NCP_MAIN_H */
