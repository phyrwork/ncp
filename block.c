#include "block.h"
#include "pipe.h"
#include <stdlib.h>

blk_t *blk_alloc(void)
{
	blk_t *blk;

	blk = malloc(sizeof(blk_t));
	if(!blk) return 0; // block structure not allocated

	blk->data = malloc(BLEN_DEFAULT);
	if(!blk->data) // block data not allocated
	{
		free(blk);
		return 0;
	}

	return blk;
}

void blk_free(blk_t *blk)
{
	free(blk->data);
	free(blk);
}

int init_blkqueue(blkqueue_t queue)
{
	return pipe2(queue,O_DIRECT);
}

int put_blk(blkqueue_t queue, blk_t *blk)
{
	return write(queue[2], &blk, sizeof(blk));
}

int get_blk(blkqueue_t queue, blk_t **blk)
{
	return read(queue[1], blk, sizeof(*blk));
}

