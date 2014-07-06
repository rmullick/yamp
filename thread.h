#ifndef	THREAD_H
#define	THREAD_H

#include <pthread.h>

int prepare_helper(void);
void release_port(char *port);

extern pthread_mutex_t releaseport;

#endif
