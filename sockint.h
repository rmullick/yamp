#ifndef	SOCKINT_H
#define	SOCKINT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <errno.h>

#define	NRSERV	100

int udp_open(int *fd, struct sockaddr_in *addr, int port);
int startport, *freeports, endport, freeidx;

#endif
