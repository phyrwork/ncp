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
#define BLEN_DEFAULT (1*1024^2) // 1M
// #define BLEN_DEFAULT (16) // 16B

typedef struct {
	ssn_t ssn;
	blen_t len;
	uint8_t data[];
} blk_t;

typedef struct {
	int fd[2];
} blkq_t;

typedef enum {
	READ,
	WRITE
} blkq_mode_t;

blk_t *blk_alloc(void);
void blk_free(blk_t *blk);

int init_blkq(blkq_t *queue);
blkq_t copy_blkq(blkq_t queue, blkq_mode_t mode);

int put_blk(blkq_t queue, blk_t *blk);
int get_blk(blkq_t queue, blk_t **blk);

#endif /* NCP_BLOCK_H */
