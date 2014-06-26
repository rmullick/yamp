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

struct servinfo {
	struct sockaddr_in saddr, caddr[2];
	int flags;
};

struct pollfd *pfdp;
int *fdp;
struct servinfo *sinfo;

int udp_open(int *fd, struct sockaddr_in *addr, int port);
int startport, *freeports, endport, freeidx, active, portrange;

#endif
