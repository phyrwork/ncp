#include "block.h"
#include "pipe.h"
#include <stdlib.h>
#include <stdio.h>

blk_t *blk_alloc(void)
{
	blk_t *blk;

	blk = malloc(sizeof(*blk) + BLEN_DEFAULT);
	if(!blk) return 0; // block structure not allocated

	return blk;
}

void blk_free(blk_t *blk)
{
	fprintf(stderr,"blk_free(): Starting block release...\n");
	free(blk);
	fprintf(stderr,"blk_free(): Released block structure.\n");
}

int init_blkq(blkq_t *queue)
{
	return pipe2(queue->fd,O_DIRECT);
}

blkq_t copy_blkq(blkq_t queue, blkq_mode_t mode)
{
	/* duplicate both ends */
	blkq_t copy;
	copy.fd[0] = dup(queue.fd[0]);
	copy.fd[1] = dup(queue.fd[1]);

	switch(mode)
	{
	case READ:
		close(copy.fd[1]); // close the write end to ensure no writes
		break;
	case WRITE:
		close(copy.fd[0]); // close the read end to ensure no reads
		break;
	}

	return copy;
}

int put_blk(blkq_t queue, blk_t *blk)
{
	return write(queue.fd[1], &blk, sizeof(blk));
}

int get_blk(blkq_t queue, blk_t **blk)
{
	return read(queue.fd[0], blk, sizeof(*blk));
}

