#ifndef NCP_THREAD_H
#define NCP_THREAD_H

#ifndef _GNU_SOURCE // include before <pthread.h> to avoid 'O_DIRECT undeclared' errors
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <pthread.h>

typedef enum {
	TSPLIT,
	TJOIN,
	TSTREAMI,
	TSTREAMO
} thread_type_t;

typedef struct {
	pthread_t id;
	thread_type_t type;
} thread_t;

#endif /* NCP_THREAD_H */
