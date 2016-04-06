#include "frame.h"
#include "lib/cobs/cobs.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define CEILING(x,y) ((x % y) ? x / y + 1 : x / y)
#define SIZEOF_COBS_MAX(in) (in + CEILING(in,255))

int fbuf_init(fbuf_t *fbuf, int sock, size_t size)
{
	/* initialise ring buffer */
	unsigned char *buf = malloc(SIZEOF_COBS_MAX(size)); // allocate buffer memory
	if(buf == 0) return -1;
	rbuf_init(&fbuf->rbuf,buf,size);
	fbuf->rbuf.mode = RBUF_MODE_BLOCKING; // don't overwrite data

	/* initialise frame buffer params */
	fbuf->sock = sock;

	return 0;
}

int get_frame(fbuf_t *fbuf, void *buf, size_t len)
{
	while(1)
	{
		/* look for a frame in the buffer */
		fprintf(stderr,"get_frame(): looking for frame boundary.\n");
		int rc;
		if((rc = rbuf_find(&fbuf->rbuf,0)) >= 0)
		{
			fprintf(stderr,"get_frame(): Frame boundary found @ pos:%u\n",rc);
			rc = rc + 1; // read amount is position found + 1

			/* decode frame */
			unsigned char *in_tmp = malloc(rc); // allocate cobs_in buffer
			if(in_tmp == 0) return -1;
			rbuf_read(&fbuf->rbuf,in_tmp,rc); // get coded frame

			unsigned char *out_tmp = malloc(rc); // allocate cobs_out buffer - not possible to be larger than input so allocate same size
			if(out_tmp == 0) { free(in_tmp); return -1; }
			rc = cobs_decode(in_tmp,rc,out_tmp); // get decoded frame
			free(in_tmp);
			if(rc == 0) { free(out_tmp); return -1; } // error decoding frame

			/* output frame */
			if(rc > len) { free(out_tmp); return -1; } // frame is too big for buffer
			memcpy(buf,out_tmp,rc);
			free(out_tmp);

			return rc;
		}
		else
		/* if no complete frame found - get more data */
		{
			fprintf(stderr,"get_frame(): Frame boundary not found: reading additional data from socket.\n");
			/* don't read more data from socket than we can write to buffer */
			size_t avail = rbuf_available(&fbuf->rbuf); // allocate tmp buffer
			unsigned char *in_tmp = malloc(avail);
			if(in_tmp == 0) return -1;

			fprintf(stderr,"get_frame(): reading up to %lu additional bytes from socket...",avail);
			rc = read(fbuf->sock,in_tmp,avail); // read the data

			if(rc < 1) // error or EOF
			{
				free(in_tmp); // free resources

				if(rc == 0)
				{
					fprintf(stderr," EOF!: socket is closed.\n");
					return 0; // no more data - can't complete current frame so abandon
				}
				if(rc < 0)
				{
					fprintf(stderr," error!\n");
					return rc; // some error
				}
			}
			fprintf(stderr," read %d bytes.\n",rc);

			// go back and look for the end of a frame
		}

	}
}

int put_frame(fbuf_t *fbuf, void *buf, size_t len)
{
	fprintf(stderr,"put_frame(): Data in: (%lu) ",len);
	for (size_t i=0;i<len;++i)
	{
		fprintf(stderr,"%u ",((unsigned char *)buf)[i]);
	}
	fprintf(stderr,"\n");

	/* cobs encode */
	unsigned char *tmp = malloc(SIZEOF_COBS_MAX(len));
	if(tmp == 0) return -1;
	int rc = cobs_encode(buf,len,tmp);
	tmp[rc++] = 0; // add delimiter

	fprintf(stderr,"put_frame(): Data out: (%d) ",rc);
	for (size_t i=0;i<rc;++i)
	{
		fprintf(stderr,"%u ",((unsigned char *)tmp)[i]);
	}
	fprintf(stderr,"\n");

	rc = write(fbuf->sock,tmp,rc);
	free(tmp);

	return rc;
}

