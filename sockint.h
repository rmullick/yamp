#ifndef	SOCKINT_H
#define	SOCKINT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

int udp_open(int *fd, struct sockaddr_in *addr, int port);

#endif
