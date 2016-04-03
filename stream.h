#ifndef NCP_STREAM_H
#define NCP_STREAM_H

#include "block.h"

int start_in(blkqueue_t queue, unsigned short port);
int start_out(blkqueue_t queue, unsigned long addr, unsigned short port);

#endif /* NCP_STREAM_H */
