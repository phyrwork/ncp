#ifndef NCP_SEND_H
#define NCP_SEND_H

#include "ncp.h"

int start_split(ncp_opt_t *opt, unsigned long addr);
int ncp_send(int argc, char *argv[]);

#endif /* NCP_SEND_H */
