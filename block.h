#ifndef NCP_BLOCK_H
#define NCP_BLOCK_H

#ifndef _GNU_SOURCE // include before <stdint.h> to avoid 'O_DIRECT undeclared' errors
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <stdint.h>

typedef uint32_t ssn_t;
typedef uint32_t blen_t;
#define SSN_MAX UINT32_MAX
#define BLEN_MAX UINT32_MAX
#define BLEN_DEFAULT 1*1024^2 // 1M

typedef struct {
	ssn_t ssn;
	blen_t len;
	uint8_t *data;
} blk_t;

typedef int blkqueue_t[2];

blk_t *blk_alloc(void);
void blk_free(blk_t *blk);

int init_blkqueue(blkqueue_t queue);

#endif /* NCP_BLOCK_H */
