#ifndef NCP_FRAME_H
#define NCP_FRAME_H

#include "rbuf.h"

#include <stdlib.h>

typedef struct {
	int sock;
	rbuf_t rbuf;
} fbuf_t;

int fbuf_init(fbuf_t *fbuf, int sock, size_t size);

int get_frame(fbuf_t *fbuf, void *buf, size_t len);
int put_frame(fbuf_t *fbuf, void *buf, size_t len);

#endif
