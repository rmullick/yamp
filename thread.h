#ifndef	THREAD_H
#define	THREAD_H

#include <pthread.h>

int prepare_helper(void);
void release_port(char *port);
#endif
